#pragma once

// WORLD-ENTRY-001 Phase B: the Agent-side character list and selection.
//
// ---------------------------------------------------------------------------
// WHAT LIVES HERE, AND WHY IT IS NOT IN THE SESSION
// ---------------------------------------------------------------------------
//
// Two responsibilities, both on the Agent:
//
//   1. answer 2247 with 2248, and 2244 with 2332, from the repository;
//   2. turn a client-named character id into an AUTHORITATIVE WorldCharacter that
//      the Agent session stores.
//
// The session decides whether the request is legal. This service decides what it
// means. Splitting them is what lets the state-machine tests run with no repository
// and the repository tests run with no session.
//
// ---------------------------------------------------------------------------
// THE 2248 WIDTH IS NOT THIS CLASS'S BUSINESS
// ---------------------------------------------------------------------------
//
// `CharacterListCodec::AppendIdList` already emits this build's fixed width
// (CharacterList::kLocalSize == 28) with the unused slots zero-filled, and refuses
// more ids than kLocalSlots. This service passes the ids straight through and adds
// ONE check of its own: that the account's character count fits the local width.
//
// That check is not redundant with the codec's. The codec refuses when handed too
// many IDS; it cannot know that an account with five characters would therefore be
// unrepresentable on this build. The service knows the account, so the account's
// characters must fit - and it fails as a repository problem (the account has more
// characters than this server can announce) rather than as a codec surprise.
//
// Explicitly NOT done here: emitting a variable-width list. Phase A found and fixed
// exactly that bug - NET_CHA_BBA_INFO's constructor sets dwSize = sizeof(...), so a
// two-character account sends 28 bytes, not 20 - and re-introducing it would put a
// packet on the wire that neither this codebase nor a real client can parse.
//
// ---------------------------------------------------------------------------
// NO TERMINATOR
// ---------------------------------------------------------------------------
//
// There is none, and adding one would be a protocol invention. Legacy completes the
// list by COUNTING: `IsStartReady() { return m_nStartCharNum == m_nStartCharLoad; }`
// (DxLobyStage.h:150). `BuildCharacterList` therefore returns the number of 2332
// records the caller should expect, which is `ExpectedDetailCount` on the decoded
// list - a value, not a packet.

#include "CharacterListProtocol.h"
#include "NetworkTypes.h"
#include "types/Result.h"
#include "world/AgentSession.h"
#include "world/CharacterRepository.h"
#include "world/WorldCharacter.h"

#include <cstddef>
#include <vector>

namespace Modern::Server::World
{
	// What one 2247 request produced.
	//
	// `expectedDetailCount` is the replacement for LOGIN-001's terminator: how many
	// 2332 records the client should now ask for. Carried as a value rather than
	// encoded, because it is a property of the exchange and not a message.
	struct CharacterListResult
	{
		// The 2248 frame: this build's fixed width, unused slots zeroed.
		std::vector<Network::WireU8> frame;

		// How many 2332 records follow, i.e. `nChaSNum`.
		std::size_t expectedDetailCount = 0;

		// The ids in the order they were sent. Kept because a test asserting on the
		// frame's bytes would otherwise have to re-derive them.
		std::vector<Network::WireU32> ids;
	};

	class CharacterSelectService
	{
	public:
		// Borrowed, not owned: the repository outlives every session and every
		// request, exactly as legacy's database manager outlives a client slot.
		explicit CharacterSelectService(const ICharacterRepository& repository) noexcept
			: m_repository(repository)
		{
		}

		// ---- the character list ---------------------------------------------

		// Answers a 2247 for `session`.
		//
		// Requires AgentState::Authenticated. The account comes from the session, never
		// from the caller: there is no overload that takes an account id, so there is
		// no way to ask for another account's list.
		//
		// Moves the session Authenticated -> CharacterListRequested -> CharacterListSent
		// on success, because a list that was built IS a list that was sent. The
		// session is left exactly as it was on failure - a half-advanced session would
		// let the next request skip a step that never completed.
		Status BuildCharacterList(AgentSession& session, CharacterListResult& out);

		// Answers a 2244 for one character with a 2332.
		//
		// Requires a session that has been sent the list, which is also the state a
		// selection is legal from - the client cannot ask about a character it was
		// never told about.
		//
		// Ownership-checked through FindOwned, so a 2244 for another account's
		// character returns NotFound and appends nothing.
		Status BuildCharacterDetail(AgentSession& session,
		                            Network::WireU32 characterId,
		                            std::vector<Network::WireU8>& out);

		// ---- selection -------------------------------------------------------

		// Handles a 2353: resolves the client-named id into an authoritative character
		// and stores it in the session, moving it CharacterListSent ->
		// CharacterSelected.
		//
		// The client supplies ONE integer. Everything else - level, HP, MP, SP, map,
		// position, gaeaId - comes from the repository, and the service has no
		// parameter through which a caller could supply them. That is the structural
		// form of "client-provided character stats are never authoritative": there is
		// no code path that would accept them.
		//
		// Refuses, leaving the session untouched:
		//   - a session that is not in CharacterListSent (unauthenticated, no list
		//     sent, already selected, already authorized, closed);
		//   - a character id that does not exist;
		//   - a character id owned by a different account;
		//   - a character id of 0, which no valid account can own.
		Status SelectCharacter(AgentSession& session, Network::WireU32 characterId);

	private:
		const ICharacterRepository& m_repository;
	};
}