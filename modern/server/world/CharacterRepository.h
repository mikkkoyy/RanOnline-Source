#pragma once

// WORLD-ENTRY-001 Phase B: the authoritative character store, with ownership
// enforced as a stated invariant.
//
// ---------------------------------------------------------------------------
// OWNERSHIP IS THE POINT OF THIS FILE
// ---------------------------------------------------------------------------
//
// In legacy the rule "a client may only enter its own character" is not written
// down anywhere in C++. `CAgentServer::MsgGameJoin` (s_CAgentServerMsg.cpp:586-650)
// checks exactly two things - `IsAccountPass` and that the message pointer is not
// null - records whatever `nChaNum` arrived, and hands it to the database:
//
//     FROM ChaInfo WHERE ChaNum=%d AND UserNum=%d
//     (s_COdbcGameChaGet.cpp:41, bound to the session's own UserNum at :650)
//
// So the enforcement lives entirely in one SQL predicate. It is correct, and it is
// invisible: nothing in the C++ says "this character must belong to this account",
// and a store with a different query shape would enforce something else without
// anyone noticing.
//
// This repository therefore makes the predicate an INTERFACE METHOD with no
// separate "find" that callers could use instead. `FindOwned` is the only way to
// get a character for the purpose of entering the world, and it is the one that
// checks. `Find` exists for the Agent to answer "does this id exist at all", and
// is deliberately NOT sufficient for world entry - the distinction is what makes
// "unknown character" and "someone else's character" different failures rather
// than the same silent success.
//
// Note the investigation's own conclusion (��4.3): the modern store "must make
// this explicit and testable rather than leave it implicit in a query - it should
// be a stated invariant, not an emergent property of a storage engine." That is
// this interface.
//
// ---------------------------------------------------------------------------
// WHY IN-MEMORY, AND WHY THAT IS NOT A COP-OUT
// ---------------------------------------------------------------------------
//
// No database is introduced. The brief forbids it without evidence, and the
// evidence points the other way: the query above is a single-table lookup on a
// primary key, with no transaction, no ordering guarantee and no join. Everything
// it does, a map does.
//
// The consequence to be honest about: an in-memory store loses characters on
// restart, and RAN's real store does not. That is a persistence milestone, not a
// world-entry one, and the seam is `ICharacterRepository` - swapping in a real
// implementation is a new class, not a rewrite of anything above it.
//
// Determinism is a requirement, not a convenience: the 2248 list must come back in
// a stable order or the character ids a client caches between sessions would shift.
// `ListByAccount` sorts by character id rather than trusting insertion order.

#include "types/Result.h"
#include "world/WorldCharacter.h"

#include <map>
#include <mutex>
#include <vector>

namespace Modern::Server::World
{
	class ICharacterRepository
	{
	public:
		virtual ~ICharacterRepository() = default;

		// Every character owned by `WorldAccountId`, ordered by ascending character id.
		//
		// An unknown account is NOT an error: it is an account with no characters,
		// which is a legitimate state that produces a well-formed empty 2248. Legacy
		// returns the same thing, because "no rows" is how a SQL lookup reports it.
		virtual Status ListByAccount(WorldAccountId accountId,
		                             std::vector<WorldCharacter>& out) const = 0;

		// Looks a character up by id alone.
		//
		// NOT sufficient for world entry: this is the call that would happily return
		// another account's character, and it is public because the Agent needs to
		// distinguish "no such character" from "not yours" in its logs. Every path
		// that acts on a selection uses FindOwned instead.
		virtual Result<WorldCharacter> Find(WorldCharacterId id) const = 0;

		// OWNERSHIP-AUTHORITATIVE. The modern equivalent of
		//   WHERE ChaNum = ? AND UserNum = ?
		//
		// NotFound when the character does not exist, and NotFound when it exists but
		// belongs to a different account. The two are deliberately the same code:
		// distinguishing them would let a caller probe for the existence of other
		// accounts' character ids, which is an information leak with no protocol
		// benefit - the client gets 2335 either way.
		virtual Result<WorldCharacter> FindOwned(WorldAccountId accountId,
		                                         WorldCharacterId characterId) const = 0;

		// Records the entity id the Field role allocated for this character's current
		// appearance.
		//
		// Refuses a zero gaeaId: 0 is the "not in the world" sentinel, and writing it
		// would erase the fact that the character was placed.
		virtual Status AssignGaeaId(WorldCharacterId characterId, Network::WireU32 gaeaId) = 0;

		// Reads the entity id back. NotFound when the character does not exist, so a
		// caller cannot mistake "no character" for "not in the world".
		virtual Result<Network::WireU32> ReadGaeaId(WorldCharacterId characterId) const = 0;
	};

	// A deterministic in-memory repository.
	//
	// Ordered containers throughout: ListByAccount walks the character map in key
	// order rather than filtering a separate per-account index, because with four
	// characters per account a linear walk over a std::map is not a performance
	// question and one source of truth is worth more than the lookup.
	class InMemoryCharacterRepository final : public ICharacterRepository
	{
	public:
		// Adds a character.
		//
		// Refuses a duplicate character id with AlreadyExists: two accounts claiming
		// one character id would make ownership depend on which copy a lookup found,
		// which is precisely the property this file exists to make impossible.
		//
		// Refuses an invalid record with InvalidArgument, via WorldCharacter::Validate.
		Status Add(const WorldCharacter& character);

		// Replaces an existing character's data, keeping its id. Refuses an unknown id.
		// Exists so a test can move a character between accounts and assert that the
		// ownership check follows the data rather than the insertion history.
		Status Replace(const WorldCharacter& character);

		Status ListByAccount(WorldAccountId accountId,
		                     std::vector<WorldCharacter>& out) const override;
		Result<WorldCharacter> Find(WorldCharacterId id) const override;
		Result<WorldCharacter> FindOwned(WorldAccountId accountId,
		                                 WorldCharacterId characterId) const override;
		Status AssignGaeaId(WorldCharacterId characterId,
		                    Network::WireU32 gaeaId) override;
		Result<Network::WireU32> ReadGaeaId(WorldCharacterId characterId) const override;

		// Total characters held, regardless of owner. For tests and for the Agent's
		// list assembly, which must not confuse "this account owns nothing" with
		// "the store is empty".
		//
		// Takes the lock, and is therefore noexcept-unsafe to call concurrently with a
		// mutation: the count is read under the mutex like every other accessor.
		std::size_t Size() const
		{
			const std::lock_guard<std::mutex> lock(m_mutex);
			return m_characters.size();
		}

	private:
		// Character id -> character. std::map, so iteration is id-ordered for free.
		std::map<Network::WireU32, WorldCharacter> m_characters;

		// Guards m_characters.
		//
		// Added in WORLD-ENTRY-002a, and the reason is that the Field role now serves
		// connections concurrently: two clients entering the world at once each call
		// Find and FindOwned from their own worker thread while the Agent thread may be
		// writing. An unsynchronised std::map under concurrent read and write is not
		// merely "racy in theory" - rebalancing can be observed half-done, so a lookup
		// can miss a character that exists, and a writer can lose one.
		//
		// Deliberately a coarse lock held only for the duration of one map operation,
		// with no I/O inside it. Character counts here are small and the operations are
		// microseconds, so finer-grained locking would buy contention problems instead
		// of removing them. A reader returning a COPY of the record is what keeps the
		// lock scope this short: nothing escapes holding a reference into the map.
		mutable std::mutex m_mutex;
	};
}