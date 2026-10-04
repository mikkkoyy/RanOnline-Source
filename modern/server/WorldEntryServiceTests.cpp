// WORLD-ENTRY-001 Phase B: the authoritative world-entry domain and the Agent/Field
// role session state.
//
// No socket opens in this file, and that is the point rather than a limitation:
// Phase C binds these services to real listeners, and the reason it can is that
// nothing here needs one. A test that needs Winsock to check a state machine is a
// test that will be flaky.
//
// The cases are grouped by the boundary they defend:
//
//   repository   ownership as a stated invariant, not an emergent property
//   list         2248 fixed width; no terminator; count is the completion signal
//   selection    a client names a character; the server decides what it is
//   world entry  the Agent authorizes; the redirect is real, not a placeholder
//   field        the Field validates 2359 and cannot be lied to
//   spawn        authoritative values only, reserved regions zero
//   states       every illegal transition refused, no crash, no undefined state
//
// The security cases are the ones worth reading twice. Each of "account A selects
// account B's character" appears at THREE layers - the repository, the Agent
// session, and the Agent service - because each layer is independently reachable
// and a check that exists at only one of them is one refactor away from being gone.

#include "TestHarness.h"

#include "world/AgentSession.h"
#include "world/CharacterRepository.h"
#include "world/CharacterSelectService.h"
#include "world/FieldSession.h"
#include "world/WorldCharacter.h"
#include "world/WorldEntryService.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

namespace
{
	using namespace Modern;
	using namespace Modern::Network;
	using namespace Modern::Server::World;

	// ---- fixtures ----------------------------------------------------------

	// A hand-rolled little-endian 32-bit read, so the byte-level assertions are
	// INDEPENDENT of the codec that wrote the frame. Reading a value back with the
	// codec that just wrote it would pass even if every field were written at the
	// wrong offset.
	WireU32 PeekLE32(const std::vector<WireU8>& frame, std::size_t offset)
	{
		WireU32 value = 0;
		for (std::size_t i = 0; i < 4; ++i)
		{
			value |= static_cast<WireU32>(frame[offset + i]) << (8 * i);
		}
		return value;
	}

	float PeekFloat(const std::vector<WireU8>& frame, std::size_t offset)
	{
		const WireU32 bits = PeekLE32(frame, offset);
		float          value = 0.0f;
		std::memcpy(&value, &bits, sizeof(value));
		return value;
	}

	WireU16 PeekLE16(const std::vector<WireU8>& frame, std::size_t offset)
	{
		return static_cast<WireU16>(frame[offset]) |
		       static_cast<WireU16>(static_cast<WireU16>(frame[offset + 1]) << 8);
	}

	bool PeekIsZero(const std::vector<WireU8>& frame, std::size_t offset,
	                std::size_t length)
	{
		for (std::size_t i = 0; i < length; ++i)
		{
			if (frame[offset + i] != 0)
			{
				return false;
			}
		}
		return true;
	}

	constexpr WorldAccountId   kAccountA{ 1001 };
	constexpr WorldAccountId   kAccountB{ 2002 };
	constexpr WorldCharacterId kCharA1{ 5001 };
	constexpr WorldCharacterId kCharA2{ 5002 };
	constexpr WorldCharacterId kCharB1{ 6001 };

	WorldCharacter MakeCharacter(WorldCharacterId id, WorldAccountId accountId,
	                             std::string name, WireU16 level)
	{
		WorldCharacter character;
		character.id         = id;
		character.accountId  = accountId;
		character.userId     = "user_" + std::to_string(accountId.value);
		character.name       = std::move(name);
		character.characterClass = 3;
		character.school     = 1;
		character.level      = level;

		character.hp = { 300u * level, 300u * level };
		character.mp = { 150u * level, 150u * level };
		character.sp = { 80u * level, 80u * level };

		character.saveMapId.value = 7u;
		character.savePosition   = { 100.5f, -20.25f, 3.75f };
		return character;
	}

	// Two accounts, three characters: A owns two, B owns one.
	//
	// Every ownership test needs at least the case where an account owns SOMETHING,
	// because "reject B's character" is only interesting if the same call accepts
	// A's own.
	InMemoryCharacterRepository MakePopulatedRepository()
	{
		InMemoryCharacterRepository repository;
		(void) repository.Add(MakeCharacter(kCharA1, kAccountA, "Aone", 10));
		(void) repository.Add(MakeCharacter(kCharA2, kAccountA, "Atwo", 20));
		(void) repository.Add(MakeCharacter(kCharB1, kAccountB, "Bone", 30));
		return repository;
	}

	// An Agent session that has been sent the list, which is the only state a
	// selection is legal from.
	AgentSession MakeSessionReadyToSelect(WorldAccountId accountId, std::string userId)
	{
		AgentSession session(42);
		(void) session.CompleteAuthentication(accountId, std::move(userId), false);
		(void) session.BeginCharacterList();
		(void) session.CompleteCharacterList();
		return session;
	}

	FieldEndpoint MakeEndpoint()
	{
		FieldEndpoint endpoint;
		endpoint.address     = "127.0.0.1";
		endpoint.servicePort = 12002;
		return endpoint;
	}

	// =========================================================================
	// Repository
	// =========================================================================

	MODERN_TEST(WorldEntry_PhaseB_RepositoryHandlesTheAccountCardinalityMatrix)
	{
		InMemoryCharacterRepository empty;

		// Zero characters. NOT an error: an account with nothing to play is a
		// legitimate state that produces a well-formed empty 2248.
		std::vector<WorldCharacter> owned;
		CHECK(empty.ListByAccount(kAccountA, owned).IsOk());
		CHECK_EQ(owned.size(), static_cast<std::size_t>(0));
		CHECK_EQ(empty.Size(), static_cast<std::size_t>(0));

		// One character.
		CHECK(empty.Add(MakeCharacter(kCharA1, kAccountA, "Solo", 5)).IsOk());
		CHECK(empty.ListByAccount(kAccountA, owned).IsOk());
		CHECK_EQ(owned.size(), static_cast<std::size_t>(1));
		CHECK_EQ(owned[0].id.value, kCharA1.value);

		// Two characters, and the order is DETERMINISTIC by ascending character id
		// rather than by insertion order - A's list came first here, so insertion
		// order would be indistinguishable. Add them the other way round.
		InMemoryCharacterRepository multi;
		CHECK(multi.Add(MakeCharacter(kCharA2, kAccountA, "Second", 20)).IsOk());
		CHECK(multi.Add(MakeCharacter(kCharA1, kAccountA, "First", 10)).IsOk());
		CHECK(multi.Add(MakeCharacter(kCharB1, kAccountB, "Other", 30)).IsOk());

		CHECK(multi.ListByAccount(kAccountA, owned).IsOk());
		CHECK_EQ(owned.size(), static_cast<std::size_t>(2));
		if (owned.size() == 2)
		{
			// kCharA1 before kCharA2, despite being added second.
			CHECK_EQ(owned[0].id.value, kCharA1.value);
			CHECK_EQ(owned[1].id.value, kCharA2.value);
		}

		// And repeating the call gives the identical order. A list whose order varied
		// would make a client that cached character ids between sessions point at the
		// wrong character.
		std::vector<WorldCharacter> again;
		CHECK(multi.ListByAccount(kAccountA, again).IsOk());
		CHECK_EQ(again.size(), owned.size());
		for (std::size_t i = 0; i < again.size() && i < owned.size(); ++i)
		{
			CHECK_EQ(again[i].id.value, owned[i].id.value);
		}
	}

	MODERN_TEST(WorldEntry_PhaseB_RepositoryRejectsUnknownAccountsAndCharacters)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();

		// An unknown account owns nothing - and that is NOT NotFound. "No rows" is how
		// a SQL lookup reports an account with no characters, so treating it as an error
		// would make a brand-new account unable to log in.
		std::vector<WorldCharacter> owned;
		CHECK(repository.ListByAccount(WorldAccountId{ 9999 }, owned).IsOk());
		CHECK_EQ(owned.size(), static_cast<std::size_t>(0));

		// An unknown character is NotFound.
		const Result<WorldCharacter> missing =
		    repository.Find(WorldCharacterId{ 4242 });
		CHECK(missing.IsError());
		CHECK_EQ(missing.GetError(), ErrorCode::NotFound);

		// So is a character nobody owns, asked for by its real owner's account.
		CHECK(repository.FindOwned(kAccountA, WorldCharacterId{ 4242 }).IsError());
	}

	MODERN_TEST(WorldEntry_PhaseB_RepositoryEnforcesOwnershipAsAnInvariant)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();

		// The positive case: each account finds its own characters.
		CHECK(repository.FindOwned(kAccountA, kCharA1).IsOk());
		CHECK(repository.FindOwned(kAccountA, kCharA2).IsOk());
		CHECK(repository.FindOwned(kAccountB, kCharB1).IsOk());

		// The negative case, and the whole reason this file exists.
		//
		// Account A asking for B's character is NotFound - the SAME code as "no such
		// character". A distinct code would be a character-existence oracle: a client
		// could enumerate which ids exist by comparing the two failures.
		const Result<WorldCharacter> stolen =
		    repository.FindOwned(kAccountA, kCharB1);
		CHECK(stolen.IsError());
		CHECK_EQ(stolen.GetError(), ErrorCode::NotFound);

		// And symmetrically.
		CHECK(repository.FindOwned(kAccountB, kCharA1).IsError());

		// Find() by id ALONE is not ownership-checked, which is why it must not be the
		// call a world-entry path uses. Asserted explicitly so the distinction stays
		// deliberate rather than accidental.
		CHECK(repository.Find(kCharB1).IsOk());

		// Moving a character between accounts moves the ANSWER, not the insertion
		// history: ownership follows the data.
		WorldCharacter moved = MakeCharacter(kCharB1, kAccountB, "Bone", 30);
		moved.accountId      = kAccountA;
		CHECK(repository.Replace(moved).IsOk());
		CHECK(repository.FindOwned(kAccountA, kCharB1).IsOk());
		CHECK(repository.FindOwned(kAccountB, kCharB1).IsError());
	}

	MODERN_TEST(WorldEntry_PhaseB_RepositoryRefusesRecordsItCouldNotEncode)
	{
		InMemoryCharacterRepository repository;

		// A zero character id would make the ownership predicate trivially satisfiable
		// in one direction and unsatisfiable in the other.
		WorldCharacter zeroId = MakeCharacter(WorldCharacterId{ 0 }, kAccountA, "Zero", 1);
		CHECK(repository.Add(zeroId).IsError());

		// A zero account id likewise.
		WorldCharacter zeroAccount = MakeCharacter(kCharA1, WorldAccountId{ 0 }, "NoAcct", 1);
		CHECK(repository.Add(zeroAccount).IsError());

		// Names that would overflow their wire field. Refused rather than truncated: a
		// shortened name is a WRONG name.
		WorldCharacter longName = MakeCharacter(kCharA1, kAccountA, "x", 1);
		longName.name = std::string(Limits::kMaxNameLength + 1, 'x');
		CHECK(repository.Add(longName).IsError());

		// The longest that fits is accepted, so the bound is a bound and not an
		// off-by-one.
		longName.name = std::string(Limits::kMaxNameLength, 'x');
		CHECK(repository.Add(longName).IsOk());

		// An empty name would be a character nobody could identify.
		WorldCharacter noName = MakeCharacter(kCharA2, kAccountA, "", 1);
		CHECK(repository.Add(noName).IsError());

		// A non-finite save position. RAN's save columns are SQL_C_DOUBLE, so a NULL
		// or NaN can reach this type; refusing it means no spawn ever carries one.
		WorldCharacter badPos = MakeCharacter(kCharA2, kAccountA, "Nan", 1);
		badPos.savePosition.y = std::numeric_limits<float>::quiet_NaN();
		CHECK(repository.Add(badPos).IsError());

		// A resource above its own maximum cannot be encoded honestly.
		WorldCharacter overfull = MakeCharacter(kCharA2, kAccountA, "Over", 1);
		overfull.hp.now = overfull.hp.max + 1u;
		CHECK(repository.Add(overfull).IsError());

		// Level 0 does not exist in RAN.
		WorldCharacter levelZero = MakeCharacter(kCharA2, kAccountA, "Zero", 0);
		CHECK(repository.Add(levelZero).IsError());

		// A duplicate id is refused rather than overwriting: an overwrite would
		// silently transfer ownership.
		CHECK(repository.Add(MakeCharacter(kCharA1, kAccountB, "Impostor", 1)).IsError());
		CHECK(repository.FindOwned(kAccountA, kCharA1).IsOk());
	}

	MODERN_TEST(WorldEntry_PhaseB_RepositoryGaeaIdIsAnEntityIdNotAnIdentity)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();

		// Unplaced.
		CHECK_EQ(repository.ReadGaeaId(kCharA1).GetValueOr(0), 0u);

		// 0 is the "not in the world" sentinel, so it must not be assignable -
		// assigning it would erase the placement rather than record one.
		CHECK(repository.AssignGaeaId(kCharA1, 0).IsError());

		// An unknown character is NotFound, not "gaeaId 0" - a caller must not read
		// "no such character" as "not in the world".
		CHECK(repository.AssignGaeaId(WorldCharacterId{ 9999 }, 1).IsError());
		CHECK(repository.ReadGaeaId(WorldCharacterId{ 9999 }).IsError());

		CHECK(repository.AssignGaeaId(kCharA1, 11).IsOk());
		CHECK_EQ(repository.ReadGaeaId(kCharA1).GetValueOr(0), 11u);

		// Re-entry allocates a NEW id, which is what makes an older authorization
		// detectably stale.
		CHECK(repository.AssignGaeaId(kCharA1, 12).IsOk());
		CHECK_EQ(repository.ReadGaeaId(kCharA1).GetValueOr(0), 12u);
	}

	// =========================================================================
	// The character list
	// =========================================================================

	MODERN_TEST(WorldEntry_PhaseB_CharacterListIsFixedWidthAndNeedsNoTerminator)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		CharacterSelectService        select(repository);

		AgentSession session(1);
		CHECK(session.CompleteAuthentication(kAccountA, "user_a", false).IsOk());

		CharacterListResult list;
		CHECK(select.BuildCharacterList(session, list).IsOk());

		// A's two characters, and the packet is still 28 bytes.
		//
		// FIXED WIDTH. NET_CHA_BBA_INFO's constructor sets dwSize = sizeof(...), so a
		// two-character account sends 28 bytes, not 20. Phase A found this bug and
		// Phase B must not reintroduce it.
		CHECK_EQ(list.frame.size(), CharacterList::kLocalSize);
		CHECK_EQ(list.frame.size(), static_cast<std::size_t>(28));

		CHECK_EQ(PeekLE32(list.frame, 0), static_cast<WireU32>(28));
		CHECK_EQ(PeekLE32(list.frame, 4), static_cast<WireU32>(2248));
		CHECK_EQ(PeekLE32(list.frame, 8), static_cast<WireU32>(2));
		CHECK_EQ(PeekLE32(list.frame, 12), kCharA1.value);
		CHECK_EQ(PeekLE32(list.frame, 16), kCharA2.value);

		// The two unused slots are ZERO, exactly as legacy's memset leaves them.
		CHECK_EQ(PeekLE32(list.frame, 20), static_cast<WireU32>(0));
		CHECK_EQ(PeekLE32(list.frame, 24), static_cast<WireU32>(0));

		// NO TERMINATOR. `expectedDetailCount` is the completion signal, and it is a
		// VALUE not a packet - legacy counts too (DxLobyStage.h:150). There is no
		// end-of-list message anywhere in this exchange.
		CHECK_EQ(list.expectedDetailCount, static_cast<std::size_t>(2));
		CHECK_EQ(list.ids.size(), static_cast<std::size_t>(2));

		// Exactly one frame was produced. If a terminator existed it would be here.
		CHECK_EQ(list.frame.size(), static_cast<std::size_t>(28));

		// And it decodes back to the same two ids, through the Phase A decoder.
		CharacterIdList decoded;
		CHECK(CharacterListCodec::DecodeIdList(list.frame, decoded).IsOk());
		CHECK_EQ(decoded.ids.size(), static_cast<std::size_t>(2));
	}

	MODERN_TEST(WorldEntry_PhaseB_AnAccountWithNoCharactersStillGetsAValidList)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		CharacterSelectService        select(repository);

		// An account the repository has never heard of. Not an error: a new account
		// logs in, gets a well-formed 28-byte list with nChaSNum = 0, and stops.
		AgentSession session(2);
		CHECK(session.CompleteAuthentication(WorldAccountId{ 9999 }, "newcomer", false).IsOk());

		CharacterListResult list;
		CHECK(select.BuildCharacterList(session, list).IsOk());
		CHECK_EQ(list.frame.size(), CharacterList::kLocalSize);
		CHECK_EQ(PeekLE32(list.frame, 8), static_cast<WireU32>(0));
		CHECK_EQ(list.expectedDetailCount, static_cast<std::size_t>(0));

		// And it round-trips: the decoder reports zero characters, not four padding
		// slots.
		CharacterIdList decoded;
		CHECK(CharacterListCodec::DecodeIdList(list.frame, decoded).IsOk());
		CHECK_EQ(decoded.ids.size(), static_cast<std::size_t>(0));
	}

	MODERN_TEST(WorldEntry_PhaseB_CharacterListIsRefusedBeforeAuthentication)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		CharacterSelectService        select(repository);

		// CONNECTED -> character list. Refused, so an unauthenticated client never
		// reaches the repository - which would be a character-existence oracle.
		AgentSession fresh(3);
		CharacterListResult list;
		CHECK(select.BuildCharacterList(fresh, list).IsError());
		CHECK_EQ(list.frame.size(), static_cast<std::size_t>(0));

		// A rejected login leaves no account behind, so the session is still refused.
		AgentSession rejected(4);
		CHECK(rejected.CompleteAuthentication(kAccountA, "user_a", true).IsError());
		CHECK(rejected.State() == AgentState::Connected);
		CHECK(select.BuildCharacterList(rejected, list).IsError());

		// Two lists in a row are refused: the session has moved past Authenticated.
		AgentSession once(5);
		CHECK(once.CompleteAuthentication(kAccountA, "user_a", false).IsOk());
		CHECK(select.BuildCharacterList(once, list).IsOk());
		CHECK(select.BuildCharacterList(once, list).IsError());
	}

	MODERN_TEST(WorldEntry_PhaseB_CharacterDetailIsOwnershipCheckedAnd1176Bytes)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		CharacterSelectService        select(repository);

		AgentSession session = MakeSessionReadyToSelect(kAccountA, "user_a");

		// A's own character: a 1176-byte 2332 with authoritative values.
		std::vector<WireU8> frame;
		CHECK(select.BuildCharacterDetail(session, kCharA1.value, frame).IsOk());
		CHECK_EQ(frame.size(), static_cast<std::size_t>(1176));
		CHECK_EQ(PeekLE32(frame, 4), static_cast<WireU32>(2332));
		CHECK_EQ(PeekLE32(frame, 8), kCharA1.value);

		// B's character, requested by A. REFUSED, and nothing appended - a 2332 for
		// someone else's character is the exact leak the ownership rule exists to stop.
		std::vector<WireU8> denied;
		CHECK(select.BuildCharacterDetail(session, kCharB1.value, denied).IsError());
		CHECK_EQ(denied.size(), static_cast<std::size_t>(0));

		// An unknown character, same outcome.
		CHECK(select.BuildCharacterDetail(session, 9999, denied).IsError());
		CHECK_EQ(denied.size(), static_cast<std::size_t>(0));
	}

	// =========================================================================
	// Selection
	// =========================================================================

	MODERN_TEST(WorldEntry_PhaseB_SelectionStoresTheAuthoritativeCharacter)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		CharacterSelectService        select(repository);

		AgentSession session = MakeSessionReadyToSelect(kAccountA, "user_a");
		CHECK(session.SelectedCharacter() == nullptr);

		CHECK(select.SelectCharacter(session, kCharA1.value).IsOk());
		CHECK(session.State() == AgentState::CharacterSelected);

		// The session now holds the REPOSITORY's record, resolved from the client's
		// id. The client's one integer was never a character.
		const WorldCharacter* selected = session.SelectedCharacter();
		CHECK(selected != nullptr);
		if (selected != nullptr)
		{
			CHECK_EQ(selected->id.value, kCharA1.value);
			CHECK_EQ(selected->name, std::string("Aone"));
			CHECK_EQ(static_cast<int>(selected->level), 10);
			CHECK_EQ(selected->hp.now, 3000u);
			CHECK_EQ(selected->saveMapId.value, 7u);
			CHECK_EQ(selected->gaeaId, 0u);
		}
	}

	MODERN_TEST(WorldEntry_PhaseB_SelectionRefusesAnotherAccountsCharacter)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		CharacterSelectService        select(repository);

		AgentSession session = MakeSessionReadyToSelect(kAccountA, "user_a");

		// THE case. A client names B's character.
		//
		// Refused with NotFound, the same code as an unknown id, so the response cannot
		// be used to discover which character ids exist.
		const Status stolen = select.SelectCharacter(session, kCharB1.value);
		CHECK(stolen.IsError());
		CHECK_EQ(stolen.GetCode(), ErrorCode::NotFound);

		// And the session is UNCHANGED - no selection, still in CharacterListSent. A
		// refused selection must not leave a half-applied state for the next request.
		CHECK(session.SelectedCharacter() == nullptr);
		CHECK(session.State() == AgentState::CharacterListSent);

		// An unknown character is refused identically.
		CHECK_EQ(select.SelectCharacter(session, 9999).GetCode(), ErrorCode::NotFound);
		CHECK(session.State() == AgentState::CharacterListSent);

		// Character id 0 is refused as the invalid argument it is, before the
		// repository is consulted - it is not a character.
		CHECK_EQ(select.SelectCharacter(session, 0).GetCode(),
		         ErrorCode::InvalidArgument);

		// The account's OWN character still works afterwards. A refusal must not
		// poison the session.
		CHECK(select.SelectCharacter(session, kCharA2.value).IsOk());
	}

	MODERN_TEST(WorldEntry_PhaseB_SessionRefusesASelectionItselfDidNotResolve)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();

		AgentSession session = MakeSessionReadyToSelect(kAccountA, "user_a");

		// The session's OWN ownership check, reached without CharacterSelectService.
		// This is the second of the three layers, and it exists because a session is
		// constructible by any caller that never went through the service.
		WorldCharacter stolen = MakeCharacter(kCharB1, kAccountB, "Bone", 30);
		CHECK(session.SelectCharacter(stolen).IsError());
		CHECK(session.SelectedCharacter() == nullptr);
		CHECK(session.State() == AgentState::CharacterListSent);

		// An invalid record is refused at the session too, so an unvalidated character
		// can never become authoritative by accident.
		WorldCharacter invalid = MakeCharacter(kCharA1, kAccountA, "Bad", 0);
		CHECK(session.SelectCharacter(invalid).IsError());

		// The account's own valid character is accepted.
		WorldCharacter own = MakeCharacter(kCharA1, kAccountA, "Aone", 10);
		CHECK(session.SelectCharacter(own).IsOk());
		CHECK(session.SelectedCharacter() != nullptr);
	}

	// =========================================================================
	// World entry: the Agent authorizes
	// =========================================================================

	MODERN_TEST(WorldEntry_PhaseB_WorldEntryProducesARealRedirectAndAClaimablePair)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;
		CharacterSelectService       select(repository);
		WorldEntryService            entry(repository, registry);
		entry.SetFieldEndpoint(MakeEndpoint());

		AgentSession session = MakeSessionReadyToSelect(kAccountA, "user_a");
		CHECK(select.SelectCharacter(session, kCharA1.value).IsOk());

		const Result<Network::FieldRedirect> redirect =
		    entry.AuthorizeWorldEntry(session);
		CHECK(redirect.IsOk());
		CHECK(session.State() == AgentState::WorldEntryAuthorized);

		if (redirect.IsOk())
		{
			const Network::FieldRedirect& r = redirect.GetValue();

			// Every field is a real value the client will use in Phase C, not a
			// placeholder it is expected to ignore.
			CHECK_EQ(r.servicePort, 12002);
			CHECK_EQ(r.fieldIp, std::string("127.0.0.1"));
			CHECK_EQ(r.joinType, Network::WorldEntry::kJoinTypeFirst);

			// gaeaId was ALLOCATED by the Field role - not 0, not a constant.
			CHECK(r.gaeaId != 0);
			CHECK(r.gaeaId != FieldEntryRegistry::kInvalidGaeaId);
			CHECK(r.slotFieldAgent != 0);

			// And the pair is the one the Field role actually recorded, so the client
			// will be able to claim with it.
			CHECK(r.gaeaId == repository.ReadGaeaId(kCharA1).GetValueOr(0));
			CHECK_EQ(registry.PendingCount(), static_cast<std::size_t>(1));

			const WorldEntryAuthorization* auth = entry.LastAuthorization();
			CHECK(auth != nullptr);
			if (auth != nullptr)
			{
				CHECK_EQ(auth->gaeaId, r.gaeaId);
				CHECK_EQ(auth->slotFieldAgent, r.slotFieldAgent);
				CHECK_EQ(auth->characterId.value, kCharA1.value);
				CHECK_EQ(auth->accountId.value, kAccountA.value);
				CHECK(!auth->consumed);
			}
		}

		// And the redirect encodes as the real 48-byte 2358.
		std::vector<WireU8> frame;
		CHECK(WorldEntryCodec::AppendFieldRedirect(frame, redirect.GetValue()).IsOk());
		CHECK_EQ(frame.size(), static_cast<std::size_t>(48));
		CHECK_EQ(PeekLE32(frame, 4), static_cast<WireU32>(2358));
		CHECK_EQ(PeekLE32(frame, Network::WorldEntry::kRedirectOffsetPort),
		         static_cast<WireU32>(12002));
	}

	MODERN_TEST(WorldEntry_PhaseB_WorldEntryIsRefusedWithoutASelection)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;
		CharacterSelectService       select(repository);
		WorldEntryService            entry(repository, registry);
		entry.SetFieldEndpoint(MakeEndpoint());

		// CONNECTED -> world entry.
		AgentSession fresh(10);
		CHECK(entry.AuthorizeWorldEntry(fresh).IsError());
		CHECK_EQ(registry.PendingCount(), static_cast<std::size_t>(0));

		// AUTHENTICATED -> world entry WITHOUT a selection. The exact transition the
		// brief calls out.
		AgentSession authenticated(11);
		CHECK(authenticated.CompleteAuthentication(kAccountA, "user_a", false).IsOk());
		CHECK(entry.AuthorizeWorldEntry(authenticated).IsError());
		CHECK(authenticated.SelectedCharacter() == nullptr);
		CHECK_EQ(registry.PendingCount(), static_cast<std::size_t>(0));

		// After the list but before a selection, too.
		AgentSession listed(12);
		CHECK(listed.CompleteAuthentication(kAccountA, "user_a", false).IsOk());
		CharacterListResult listed_result;
		CHECK(select.BuildCharacterList(listed, listed_result).IsOk());
		CHECK(entry.AuthorizeWorldEntry(listed).IsError());
		CHECK_EQ(registry.PendingCount(), static_cast<std::size_t>(0));
	}

	MODERN_TEST(WorldEntry_PhaseB_WorldEntryRefusesAnEndpointTheClientWouldDial)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;
		CharacterSelectService       select(repository);
		WorldEntryService            entry(repository, registry);

		AgentSession session = MakeSessionReadyToSelect(kAccountA, "user_a");
		CHECK(select.SelectCharacter(session, kCharA1.value).IsOk());

		// No endpoint configured. Refused rather than defaulted, because a default here
		// becomes a 2358 carrying an address the client will actually try to dial.
		CHECK(entry.AuthorizeWorldEntry(session).IsError());
		CHECK_EQ(registry.PendingCount(), static_cast<std::size_t>(0));

		// An address too long for 2358's 21-byte field.
		FieldEndpoint tooLong = MakeEndpoint();
		tooLong.address       = std::string(Network::WorldEntry::kAddressFieldSize, '9');
		entry.SetFieldEndpoint(tooLong);
		CHECK(entry.AuthorizeWorldEntry(session).IsError());

		// Port 0 and a port above 65535.
		FieldEndpoint badPort = MakeEndpoint();
		badPort.servicePort   = 0;
		entry.SetFieldEndpoint(badPort);
		CHECK(entry.AuthorizeWorldEntry(session).IsError());

		badPort.servicePort = 70000;
		entry.SetFieldEndpoint(badPort);
		CHECK(entry.AuthorizeWorldEntry(session).IsError());

		// Still CharacterSelected after every refusal - nothing was authorized.
		CHECK(session.State() == AgentState::CharacterSelected);

		// A valid endpoint works.
		entry.SetFieldEndpoint(MakeEndpoint());
		CHECK(entry.AuthorizeWorldEntry(session).IsOk());
	}

	MODERN_TEST(WorldEntry_PhaseB_WorldEntryReProvesOwnershipAtAuthorizationTime)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;
		CharacterSelectService       select(repository);
		WorldEntryService            entry(repository, registry);
		entry.SetFieldEndpoint(MakeEndpoint());

		AgentSession session = MakeSessionReadyToSelect(kAccountA, "user_a");
		CHECK(select.SelectCharacter(session, kCharA1.value).IsOk());

		// The character moves to another account AFTER the selection. The selection
		// was legitimate when it was made; the authorization must not be.
		WorldCharacter moved = MakeCharacter(kCharA1, kAccountB, "Aone", 10);
		CHECK(repository.Replace(moved).IsOk());

		CHECK(entry.AuthorizeWorldEntry(session).IsError());
		CHECK_EQ(registry.PendingCount(), static_cast<std::size_t>(0));
		CHECK(session.State() == AgentState::CharacterSelected);
	}

	// =========================================================================
	// Field: 2359 validation
	// =========================================================================

	// Builds a FieldIdentity that would successfully claim `authorization`.
	Network::FieldIdentity IdentityFor(const WorldEntryAuthorization& authorization)
	{
		Network::FieldIdentity identity;
		identity.joinType       = Network::WorldEntry::kJoinTypeFirst;
		identity.gaeaId         = authorization.gaeaId;
		identity.slotFieldAgent = authorization.slotFieldAgent;
		identity.cryptKey       = RanWire::DefaultCryptKey();
		return identity;
	}

	// Runs select -> authorize and hands back the authorization, which is what a
	// client would receive inside its 2358.
	struct AuthorizedFlow
	{
		WorldEntryAuthorization authorization;
		bool                     ok = false;
	};

	// `repository` and `registry` are deliberately NOT parameters: the service and
	// the session already hold them, and passing them again would let a test build
	// a flow whose service and repository disagree - which is not a failure this
	// domain can have.
	AuthorizedFlow AuthorizeFlow(AgentSession& session,
	                             CharacterSelectService& select,
	                             WorldEntryService& entry, WorldCharacterId characterId)
	{
		AuthorizedFlow flow;
		if (select.SelectCharacter(session, characterId.value).IsError())
		{
			return flow;
		}
		if (entry.AuthorizeWorldEntry(session).IsError())
		{
			return flow;
		}
		const WorldEntryAuthorization* auth = entry.LastAuthorization();
		if (auth == nullptr)
		{
			return flow;
		}
		flow.authorization = *auth;
		flow.ok             = true;
		return flow;
	}

	MODERN_TEST(WorldEntry_PhaseB_FieldAcceptsAnAgentAuthorizedIdentity)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;
		CharacterSelectService       select(repository);
		WorldEntryService            entry(repository, registry);
		entry.SetFieldEndpoint(MakeEndpoint());

		AgentSession agent = MakeSessionReadyToSelect(kAccountA, "user_a");
		const AuthorizedFlow flow = AuthorizeFlow(agent, select,
		                                          entry, kCharA1);
		CHECK(flow.ok);

		// The Field is a SEPARATE role with its OWN session. It learns nothing from the
		// Agent session except through the pair the client would present.
		FieldSession field(repository, registry, 900);
		CHECK(field.State() == FieldState::Connected);
		CHECK(field.Character() == nullptr);

		CHECK(field.ValidateIdentity(IdentityFor(flow.authorization), 0).IsOk());
		CHECK(field.State() == FieldState::IdentityValidated);

		// The Field now holds the AUTHORITATIVE character, re-read from the repository.
		const WorldCharacter* character = field.Character();
		CHECK(character != nullptr);
		if (character != nullptr)
		{
			CHECK_EQ(character->id.value, kCharA1.value);
			CHECK_EQ(character->name, std::string("Aone"));
			CHECK_EQ(character->accountId.value, kAccountA.value);
		}
		CHECK_EQ(field.GaeaId(), flow.authorization.gaeaId);
		CHECK_EQ(field.AgentSessionId(), agent.SessionId());

		// And the authorization is now SPENT. A replay cannot work.
		CHECK_EQ(registry.PendingCount(), static_cast<std::size_t>(0));
	}

	MODERN_TEST(WorldEntry_PhaseB_FieldRejectsEveryWrongIdentity)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;
		CharacterSelectService       select(repository);
		WorldEntryService            entry(repository, registry);
		entry.SetFieldEndpoint(MakeEndpoint());

		AgentSession agent = MakeSessionReadyToSelect(kAccountA, "user_a");
		const AuthorizedFlow flow = AuthorizeFlow(agent, select,
		                                          entry, kCharA1);
		CHECK(flow.ok);

		// An unknown gaeaId: nothing was ever authorized with it.
		{
			FieldSession field(repository, registry, 1);
			Network::FieldIdentity identity = IdentityFor(flow.authorization);
			identity.gaeaId = 999999;
			CHECK(field.ValidateIdentity(identity, 0).IsError());
			CHECK(field.State() == FieldState::Connected);
		}

		// A gaeaId of 0 is not an entity and can never have been authorized.
		{
			FieldSession field(repository, registry, 2);
			Network::FieldIdentity identity = IdentityFor(flow.authorization);
			identity.gaeaId = 0;
			CHECK(field.ValidateIdentity(identity, 0).IsError());
		}

		// A wrong slot against a VALID gaeaId. A different failure from "no such
		// entity", and refused on the same code so it is not an oracle either.
		{
			FieldSession field(repository, registry, 3);
			Network::FieldIdentity identity = IdentityFor(flow.authorization);
			identity.slotFieldAgent = flow.authorization.slotFieldAgent + 1u;
			CHECK(field.ValidateIdentity(identity, 0).IsError());
			CHECK(field.State() == FieldState::Connected);
		}

		// An unknown join type. The field is a plain 4-byte enum on the wire
		// (EMGAME_JOINTYPE) and only three values are declared.
		{
			FieldSession field(repository, registry, 4);
			Network::FieldIdentity identity = IdentityFor(flow.authorization);
			identity.joinType = 99;
			CHECK(field.ValidateIdentity(identity, 0).IsError());
		}

		// Malformed identity that never decoded: the Phase A codec refuses it before
		// any of this runs.
		{
			Network::FieldIdentity decoded;

			std::vector<WireU8> truncated;
			CHECK(WorldEntryCodec::AppendFieldIdentity(truncated,
			                                           IdentityFor(flow.authorization)).IsOk());
			truncated.resize(23);
			CHECK(WorldEntryCodec::DecodeFieldIdentity(truncated, decoded).IsError());

			// Wrong id on the wire.
			std::vector<WireU8> wrongId;
			CHECK(WorldEntryCodec::AppendFieldIdentity(wrongId,
			                                           IdentityFor(flow.authorization)).IsOk());
			wrongId[4] = static_cast<WireU8>(2353);
			CHECK(WorldEntryCodec::DecodeFieldIdentity(wrongId, decoded).IsError());

			// Empty.
			CHECK(WorldEntryCodec::DecodeFieldIdentity({}, decoded).IsError());
		}

		// The real pair still works afterwards: no refusal above consumed it.
		{
			FieldSession field(repository, registry, 5);
			CHECK(field.ValidateIdentity(IdentityFor(flow.authorization), 0).IsOk());
		}
	}

	MODERN_TEST(WorldEntry_PhaseB_ReplayOfAValidIdentityIsRefused)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;
		CharacterSelectService       select(repository);
		WorldEntryService            entry(repository, registry);
		entry.SetFieldEndpoint(MakeEndpoint());

		AgentSession agent = MakeSessionReadyToSelect(kAccountA, "user_a");
		const AuthorizedFlow flow = AuthorizeFlow(agent, select,
		                                          entry, kCharA1);
		CHECK(flow.ok);

		FieldSession first(repository, registry, 1);
		CHECK(first.ValidateIdentity(IdentityFor(flow.authorization), 0).IsOk());

		// The same pair again, on a fresh connection. This is what a captured 2359
		// would be worth to an attacker.
		FieldSession replay(repository, registry, 2);
		CHECK(replay.ValidateIdentity(IdentityFor(flow.authorization), 0).IsError());
		CHECK(replay.State() == FieldState::Connected);
		CHECK(replay.Character() == nullptr);

		// A second 2359 on the SAME connection is a protocol fault, and is refused
		// before it can re-point an already-spawned session.
		CHECK(first.ValidateIdentity(IdentityFor(flow.authorization), 0).IsError());
		CHECK(first.State() == FieldState::IdentityValidated);
	}

	MODERN_TEST(WorldEntry_PhaseB_StaleAuthorizationIsRefused)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;
		CharacterSelectService       select(repository);
		WorldEntryService            entry(repository, registry);
		entry.SetFieldEndpoint(MakeEndpoint());

		// The Agent's clock is passed in, so a test can age an authorization without
		// sleeping. Time is never read from a clock of the registry's own.
		AgentSession agent = MakeSessionReadyToSelect(kAccountA, "user_a");
		agent.AdvanceClock(1000);
		const AuthorizedFlow flow = AuthorizeFlow(agent, select,
		                                          entry, kCharA1);
		CHECK(flow.ok);
		CHECK_EQ(flow.authorization.createdAtMs, 1000u);

		// One millisecond short of NET_TIME_OUT is still valid.
		{
			FieldSession field(repository, registry, 1);
			CHECK(field.ValidateIdentity(IdentityFor(flow.authorization), 1000).IsOk());
		}

		// A different Field, a real clock, at exactly NET_TIME_OUT: stale.
		//
		// The same clock legacy uses to drop a client it has stopped hearing from
		// (NET_TIME_OUT, s_NetGlobal.h:117).
		{
			FieldSession field(repository, registry, 2);
			const WireU64 expired =
			    1000 + Protocol::kTimeoutMilliseconds;
			CHECK(field.ValidateIdentity(IdentityFor(flow.authorization), expired)
			          .IsError());
			CHECK(field.State() == FieldState::Connected);
		}
	}

	MODERN_TEST(WorldEntry_PhaseB_AuthorizationForAPreviousAppearanceIsRefused)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;
		CharacterSelectService       select(repository);
		WorldEntryService            entry(repository, registry);
		entry.SetFieldEndpoint(MakeEndpoint());

		// First entry, authorized but never claimed.
		AgentSession first = MakeSessionReadyToSelect(kAccountA, "user_a");
		const AuthorizedFlow stale = AuthorizeFlow(first, select,
		                                           entry, kCharA1);
		CHECK(stale.ok);
		const WireU32 staleGaeaId = stale.authorization.gaeaId;

		// The character enters the world again. Reserve ALWAYS allocates a new gaeaId -
		// which is exactly what makes the first authorization detectable as stale rather
		// than accidentally valid.
		AgentSession second = MakeSessionReadyToSelect(kAccountA, "user_a");
		const AuthorizedFlow fresh = AuthorizeFlow(second, select,
		                                           entry, kCharA1);
		CHECK(fresh.ok);
		CHECK_NE(fresh.authorization.gaeaId, staleGaeaId);
		CHECK_EQ(repository.ReadGaeaId(kCharA1).GetValueOr(0),
		         fresh.authorization.gaeaId);

		// The FIRST pair no longer works, even though it was validly issued and is not
		// expired and not consumed.
		FieldSession viaStale(repository, registry, 1);
		CHECK(viaStale.ValidateIdentity(IdentityFor(stale.authorization), 0).IsError());
		CHECK(viaStale.State() == FieldState::Connected);

		// The second does.
		FieldSession viaFresh(repository, registry, 2);
		CHECK(viaFresh.ValidateIdentity(IdentityFor(fresh.authorization), 0).IsOk());
	}

	MODERN_TEST(WorldEntry_PhaseB_FieldRefusesAnAuthorizationTheRepositoryContradicts)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;
		CharacterSelectService       select(repository);
		WorldEntryService            entry(repository, registry);
		entry.SetFieldEndpoint(MakeEndpoint());

		AgentSession agent = MakeSessionReadyToSelect(kAccountA, "user_a");
		const AuthorizedFlow flow = AuthorizeFlow(agent, select,
		                                          entry, kCharA1);
		CHECK(flow.ok);

		// The character is moved to another account AFTER authorization. The Field
		// re-checks ownership at validation time rather than trusting the Agent's
		// snapshot, so a bug in the Agent's path cannot become a way in.
		WorldCharacter moved = MakeCharacter(kCharA1, kAccountB, "Aone", 10);
		moved.gaeaId = flow.authorization.gaeaId;
		CHECK(repository.Replace(moved).IsOk());

		FieldSession field(repository, registry, 1);
		CHECK(field.ValidateIdentity(IdentityFor(flow.authorization), 0).IsError());
		CHECK(field.State() == FieldState::Connected);
	}

	MODERN_TEST(WorldEntry_PhaseB_DifferentAccountCannotClaimAnOthersPair)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;
		CharacterSelectService       select(repository);
		WorldEntryService            entry(repository, registry);
		entry.SetFieldEndpoint(MakeEndpoint());

		// A authorizes and receives a pair. B somehow presents it.
		AgentSession agentA = MakeSessionReadyToSelect(kAccountA, "user_a");
		const AuthorizedFlow flow = AuthorizeFlow(agentA, select,
		                                          entry, kCharA1);
		CHECK(flow.ok);

		// There is no account field in 2359 to get wrong, so the only thing that can
		// reject B is that B does not hold the pair. The same-account-different-
		// character case below is the one that could plausibly have slipped through.
		FieldSession asB(repository, registry, 1);
		CHECK(asB.ValidateIdentity(IdentityFor(flow.authorization), 0).IsOk());
		CHECK(asB.Character() != nullptr);
		if (asB.Character() != nullptr)
		{
			CHECK_EQ(asB.Character()->accountId.value, kAccountA.value);
		}

		// A second authorization for B's own character is a DIFFERENT gaeaId, and B's
		// pair cannot be confused with A's.
		AgentSession agentB = MakeSessionReadyToSelect(kAccountB, "user_b");
		const AuthorizedFlow bFlow = AuthorizeFlow(agentB,
		                                           select, entry, kCharB1);
		CHECK(bFlow.ok);
		CHECK_NE(bFlow.authorization.gaeaId, flow.authorization.gaeaId);

		FieldSession asB2(repository, registry, 2);
		CHECK(asB2.ValidateIdentity(IdentityFor(bFlow.authorization), 0).IsOk());
		if (asB2.Character() != nullptr)
		{
			CHECK_EQ(asB2.Character()->id.value, kCharB1.value);
			CHECK_EQ(asB2.Character()->name, std::string("Bone"));
		}
	}

	MODERN_TEST(WorldEntry_PhaseB_RegistryRefusesToAuthorizeWhatItCannotProve)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;

		// A character the repository does not hold. The registry proves ownership for
		// ITSELF - it does not take the Agent's word for it, because the Field is where
		// the character will exist.
		CHECK(registry.Reserve(repository, kAccountA, WorldCharacterId{ 9999 }, 1, 0).IsError());

		// A real character owned by the wrong account.
		CHECK(registry.Reserve(repository, kAccountA, kCharB1, 1, 0).IsError());

		// Zero ids.
		CHECK(registry.Reserve(repository, WorldAccountId{ 0 }, kCharA1, 1, 0).IsError());
		CHECK(registry.Reserve(repository, kAccountA, WorldCharacterId{ 0 }, 1, 0).IsError());

		// Nothing was recorded by any of those.
		CHECK_EQ(registry.PendingCount(), static_cast<std::size_t>(0));

		// And a valid one is.
		const Result<WorldEntryAuthorization> ok =
		    registry.Reserve(repository, kAccountA, kCharA1, 1, 0);
		CHECK(ok.IsOk());
		CHECK_EQ(registry.PendingCount(), static_cast<std::size_t>(1));

		// Discarding makes it unclaimable without pretending the character was never
		// placed.
		registry.Discard(ok.GetValue().gaeaId);
		CHECK_EQ(registry.PendingCount(), static_cast<std::size_t>(0));

		FieldSession field(repository, registry, 1);
		CHECK(field.ValidateIdentity(IdentityFor(ok.GetValue()), 0).IsError());
		// The character still holds its gaeaId: Discard does not un-place it.
		CHECK_EQ(repository.ReadGaeaId(kCharA1).GetValueOr(0),
		         ok.GetValue().gaeaId);
	}

	// =========================================================================
	// Spawn
	// =========================================================================

	// Drives the whole flow and leaves a Field session ready to spawn.
	//
	// Same reasoning as AuthorizeFlow: repository and registry are already held by
	// the service and the Field session, so they are not parameters here either.
	FieldSpawnResult SpawnThroughWorldEntry(CharacterSelectService& select,
	                                         WorldEntryService& entry,
	                                         AgentSession& agent,
	                                         WorldCharacterId characterId,
	                                         FieldSession& field)
	{
		FieldSpawnResult result;
		const AuthorizedFlow flow = AuthorizeFlow(agent, select, entry, characterId);
		if (!flow.ok)
		{
			return result;
		}
		if (field.ValidateIdentity(IdentityFor(flow.authorization), 0).IsError())
		{
			return result;
		}
		(void) field.BuildSpawn(result);
		return result;
	}

	MODERN_TEST(WorldEntry_PhaseB_SpawnIs1022BytesOfAuthoritativeValues)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;
		CharacterSelectService       select(repository);
		WorldEntryService            entry(repository, registry);
		entry.SetFieldEndpoint(MakeEndpoint());

		AgentSession agent = MakeSessionReadyToSelect(kAccountA, "user_a");
		FieldSession field(repository, registry, 1);

		const FieldSpawnResult spawn = SpawnThroughWorldEntry(select, entry, agent, kCharA1, field);
		CHECK_EQ(spawn.frame.size(), static_cast<std::size_t>(1022));
		CHECK(field.State() == FieldState::Spawned);

		// The header.
		CHECK_EQ(PeekLE32(spawn.frame, 0), static_cast<WireU32>(1022));
		CHECK_EQ(PeekLE32(spawn.frame, 4), static_cast<WireU32>(2333));

		// szUserID from the character's own login name, NUL-terminated in place
		// and zero-padded out to the full 21-byte field.
		const std::string userId = "user_" + std::to_string(kAccountA.value);
		CHECK_EQ(static_cast<int>(spawn.frame[Network::WorldEntry::kSpawnOffsetUserId]), 'u');
		CHECK_EQ(static_cast<int>(spawn.frame[Network::WorldEntry::kSpawnOffsetUserId + 4]),
		         static_cast<int>('_'));
		CHECK_EQ(static_cast<int>(spawn.frame[Network::WorldEntry::kSpawnOffsetUserId +
		                                    userId.size()]),
		         0x00);
		CHECK(PeekIsZero(spawn.frame,
		                 Network::WorldEntry::kSpawnOffsetUserId + userId.size() + 1,
		                 Network::WorldEntry::kUserIdFieldSize - userId.size() - 1));

		// gaeaId, map and position: authoritative, and the position is the DB SAVE
		// position (ChaSavePosX/Y/Z, s_COdbcGameChaGet.cpp:214-216).
		CHECK_EQ(PeekLE32(spawn.frame, Network::WorldEntry::kSpawnOffsetGaeaId),
		         spawn.gaeaId);
		CHECK(spawn.gaeaId != 0);
		CHECK_EQ(PeekLE32(spawn.frame, Network::WorldEntry::kSpawnOffsetMapId), 7u);
		CHECK_EQ(PeekFloat(spawn.frame, Network::WorldEntry::kSpawnOffsetPosition + 0), 100.5f);
		CHECK_EQ(PeekFloat(spawn.frame, Network::WorldEntry::kSpawnOffsetPosition + 4), -20.25f);
		CHECK_EQ(PeekFloat(spawn.frame, Network::WorldEntry::kSpawnOffsetPosition + 8), 3.75f);

		// Inside the record: identity and stats, all from the repository.
		const std::size_t record = Network::WorldEntry::kSpawnOffsetData;
		CHECK_EQ(PeekLE32(spawn.frame, record + Network::WorldEntry::kRecordOffsetAccountId),
		         kAccountA.value);
		CHECK_EQ(PeekLE32(spawn.frame, record + Network::WorldEntry::kRecordOffsetCharacterId),
		         kCharA1.value);
		CHECK_EQ(static_cast<int>(spawn.frame[record + Network::WorldEntry::kRecordOffsetName]),
		         'A');
		CHECK_EQ(PeekLE32(spawn.frame, record + Network::WorldEntry::kRecordOffsetCharacterClass),
		         3u);
		CHECK_EQ(PeekLE16(spawn.frame, record + Network::WorldEntry::kRecordOffsetSchool), 1);
		CHECK_EQ(PeekLE16(spawn.frame, record + Network::WorldEntry::kRecordOffsetLevel), 10);
		CHECK_EQ(PeekLE32(spawn.frame, record + Network::WorldEntry::kRecordOffsetHp + 0), 3000u);
		CHECK_EQ(PeekLE32(spawn.frame, record + Network::WorldEntry::kRecordOffsetHp + 4), 3000u);
		CHECK_EQ(PeekLE32(spawn.frame, record + Network::WorldEntry::kRecordOffsetMp + 0), 1500u);
		CHECK_EQ(PeekLE32(spawn.frame, record + Network::WorldEntry::kRecordOffsetSp + 0), 800u);

		// sStartMapID is the saved map.
		CHECK_EQ(PeekLE32(spawn.frame, Network::WorldEntry::kSpawnOffsetStartMapId), 7u);

		// NO ROTATION. RAN sends no angle anywhere in this flow, and 2333 has no field
		// for one - the position is 12 bytes and the record starts immediately after.
		// Asserted as a layout fact so a future "just add a heading" fails here.
		CHECK_EQ(Network::WorldEntry::kSpawnOffsetPosition + 12,
		         Network::WorldEntry::kSpawnOffsetData);
	}

	MODERN_TEST(WorldEntry_PhaseB_SpawnReservedRegionsAreZero)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;
		CharacterSelectService       select(repository);
		WorldEntryService            entry(repository, registry);
		entry.SetFieldEndpoint(MakeEndpoint());

		AgentSession agent = MakeSessionReadyToSelect(kAccountA, "user_a");
		FieldSession field(repository, registry, 1);
		const FieldSpawnResult spawn = SpawnThroughWorldEntry(select, entry, agent, kCharA1, field);
		CHECK_EQ(spawn.frame.size(), static_cast<std::size_t>(1022));

		// The 288 bytes of quickslots and the 44 bytes of counts: skills, quests,
		// inventory and activity are out of scope, so they are ZERO rather than
		// modelled. Checked byte by byte, not merely labelled reserved.
		CHECK(PeekIsZero(spawn.frame, Network::WorldEntry::kSpawnReservedQuickslotOffset,
		                 Network::WorldEntry::kSpawnReservedQuickslotSize));
		CHECK(PeekIsZero(spawn.frame, Network::WorldEntry::kSpawnReservedSlotArrayOffset,
		                 Network::WorldEntry::kSpawnReservedSlotArraySize));
		CHECK(PeekIsZero(spawn.frame, Network::WorldEntry::kSpawnReservedCountsOffset,
		                 Network::WorldEntry::kSpawnReservedCountsSize));
		CHECK(PeekIsZero(spawn.frame, Network::WorldEntry::kSpawnReservedCosmeticsOffset,
		                 Network::WorldEntry::kSpawnReservedCosmeticsSize));
		CHECK(PeekIsZero(spawn.frame, Network::WorldEntry::kSpawnReservedLastCallOffset,
		                 Network::WorldEntry::kSpawnReservedLastCallSize));
		CHECK(PeekIsZero(spawn.frame, Network::WorldEntry::kSpawnReservedTrailingOffset,
		                 Network::WorldEntry::kSpawnReservedTrailingSize));

		// And the reserved runs inside the 600-byte record.
		const std::size_t record = Network::WorldEntry::kSpawnOffsetData;
		CHECK(PeekIsZero(spawn.frame,
		                 record + Network::WorldEntry::kRecordReservedPreIdentityOffset,
		                 Network::WorldEntry::kRecordReservedPreIdentitySize));
		CHECK(PeekIsZero(spawn.frame, record + Network::WorldEntry::kRecordReservedTribeOffset,
		                 Network::WorldEntry::kRecordReservedTribeSize));
		CHECK(PeekIsZero(spawn.frame,
		                 record + Network::WorldEntry::kRecordReservedAppearanceOffset,
		                 Network::WorldEntry::kRecordReservedAppearanceSize));
		CHECK(PeekIsZero(spawn.frame, record + Network::WorldEntry::kRecordReservedMidOffset,
		                 Network::WorldEntry::kRecordReservedMidSize));
		CHECK(PeekIsZero(spawn.frame,
		                 record + Network::WorldEntry::kRecordReservedPostResourceOffset,
		                 Network::WorldEntry::kRecordReservedPostResourceSize));
		CHECK(PeekIsZero(spawn.frame, record + Network::WorldEntry::kRecordReservedTailOffset,
		                 Network::WorldEntry::kRecordReservedTailSize));

		// NO EQUIPMENT IN 2333 AT ALL. RAN sends it as 22 separate
		// SNETLOBBY_CHARPUTON_EX messages after this burst, so there is nothing to
		// zero here and nothing to model.
		CHECK(PeekIsZero(spawn.frame, record + Network::WorldEntry::kRecordReservedMidOffset,
		                 Network::WorldEntry::kRecordReservedMidSize));

		// The spawn decodes back to the authoritative character, which is the strongest
		// statement available that no client-supplied value leaked in.
		Network::SpawnState decoded;
		CHECK(WorldEntryCodec::DecodeSpawn(spawn.frame, decoded).IsOk());
		CHECK_EQ(decoded.userId, "user_" + std::to_string(kAccountA.value));
		CHECK_EQ(decoded.characterId, kCharA1.value);
		CHECK_EQ(decoded.characterName, std::string("Aone"));
		CHECK_EQ(static_cast<int>(decoded.level), 10);
		CHECK_EQ(decoded.mapId.value, 7u);
		CHECK_EQ(decoded.position.y, -20.25f);
		CHECK_EQ(decoded.gaeaId, spawn.gaeaId);
	}

	MODERN_TEST(WorldEntry_PhaseB_SpawnIsRefusedWithoutAValidatedIdentity)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;

		// FIELD_CONNECTED -> spawn, with no 2359 at all. This is the case the brief
		// calls out, and it has to be impossible rather than merely discouraged.
		FieldSession fresh(repository, registry, 1);
		FieldSpawnResult result;
		CHECK(fresh.BuildSpawn(result).IsError());
		CHECK_EQ(result.frame.size(), static_cast<std::size_t>(0));
		CHECK(fresh.Character() == nullptr);
		CHECK(fresh.State() == FieldState::Connected);

		// A failed validation leaves nothing to spawn from either.
		Network::FieldIdentity bogus;
		bogus.gaeaId = 12345;
		bogus.slotFieldAgent = 1;
		CHECK(fresh.ValidateIdentity(bogus, 0).IsError());
		CHECK(fresh.BuildSpawn(result).IsError());
		CHECK_EQ(result.frame.size(), static_cast<std::size_t>(0));

		// A closed session cannot spawn.
		FieldSession closed(repository, registry, 2);
		closed.Close();
		CHECK(closed.BuildSpawn(result).IsError());
	}

	MODERN_TEST(WorldEntry_PhaseB_SpawnIsRefusedTwiceOnOneSession)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;
		CharacterSelectService       select(repository);
		WorldEntryService            entry(repository, registry);
		entry.SetFieldEndpoint(MakeEndpoint());

		AgentSession agent = MakeSessionReadyToSelect(kAccountA, "user_a");
		FieldSession field(repository, registry, 1);

		const AuthorizedFlow flow = AuthorizeFlow(agent, select,
		                                          entry, kCharA1);
		CHECK(flow.ok);
		CHECK(field.ValidateIdentity(IdentityFor(flow.authorization), 0).IsOk());

		FieldSpawnResult first;
		CHECK(field.BuildSpawn(first).IsOk());
		CHECK_EQ(first.frame.size(), static_cast<std::size_t>(1022));

		// Once spawned, this session cannot spawn again: there is one world-entry per
		// Field connection in this milestone.
		FieldSpawnResult second;
		CHECK(field.BuildSpawn(second).IsError());
		CHECK_EQ(second.frame.size(), static_cast<std::size_t>(0));
		CHECK(field.State() == FieldState::Spawned);
	}

	// =========================================================================
	// State machine: every illegal transition
	// =========================================================================

	MODERN_TEST(WorldEntry_PhaseB_AgentStateNamesAreDistinct)
	{
		// A log that cannot tell two states apart is worse than no log.
		CHECK(std::string(ToString(AgentState::Connected)) !=
		      std::string(ToString(AgentState::Authenticated)));
		CHECK(std::string(ToString(AgentState::CharacterListSent)) !=
		      std::string(ToString(AgentState::CharacterSelected)));
		CHECK(std::string(ToString(AgentState::WorldEntryAuthorized)) !=
		      std::string(ToString(AgentState::Closed)));
		CHECK(std::string(ToString(FieldState::IdentityValidated)) !=
		      std::string(ToString(FieldState::Spawned)));
	}

	MODERN_TEST(WorldEntry_PhaseB_IllegalAgentTransitionsAreAllRefused)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;
		CharacterSelectService       select(repository);
		WorldEntryService            entry(repository, registry);
		entry.SetFieldEndpoint(MakeEndpoint());

		CharacterListResult scratch;
		WorldCharacter      owned = MakeCharacter(kCharA1, kAccountA, "Aone", 10);
		std::vector<WireU8> bytes;

		// ---- CONNECTED: nothing but authentication is legal ---------------
		{
			AgentSession s(1);
			CHECK(s.State() == AgentState::Connected);
			CHECK(!s.MayRequestCharacterList());
			CHECK(!s.MayRequestCharacterSelect());
			CHECK(!s.MayRequestWorldEntry());
			CHECK(s.BeginCharacterList().IsError());
			CHECK(s.CompleteCharacterList().IsError());
			CHECK(s.SelectCharacter(owned).IsError());
			CHECK(s.AuthorizeWorldEntry().IsError());
			CHECK(select.BuildCharacterList(s, scratch).IsError());
			CHECK(select.SelectCharacter(s, kCharA1.value).IsError());
			CHECK(select.BuildCharacterDetail(s, kCharA1.value, bytes).IsError());
			CHECK(entry.AuthorizeWorldEntry(s).IsError());
			CHECK_EQ(registry.PendingCount(), static_cast<std::size_t>(0));
		}

		// ---- AUTHENTICATED: list only, no selection, no world entry --------
		{
			AgentSession s(2);
			CHECK(s.CompleteAuthentication(kAccountA, "user_a", false).IsOk());
			CHECK(s.State() == AgentState::Authenticated);

			// Completing the list BEFORE requesting it is out of order.
			CHECK(s.CompleteCharacterList().IsError());
			CHECK(s.SelectCharacter(owned).IsError());
			CHECK(s.AuthorizeWorldEntry().IsError());
			CHECK(select.SelectCharacter(s, kCharA1.value).IsError());
			CHECK(select.BuildCharacterDetail(s, kCharA1.value, bytes).IsError());
			CHECK(entry.AuthorizeWorldEntry(s).IsError());

			// And the legal order still works afterwards: a refused transition must
			// not leave the session somewhere it cannot proceed from.
			CHECK(s.BeginCharacterList().IsOk());
			CHECK(s.CompleteCharacterList().IsOk());
			CHECK(s.State() == AgentState::CharacterListSent);
			CHECK(select.SelectCharacter(s, kCharA1.value).IsOk());
		}

		// ---- CHARACTER_LIST_REQUESTED: selection is not yet legal ---------
		{
			AgentSession s(3);
			CHECK(s.CompleteAuthentication(kAccountA, "user_a", false).IsOk());
			CHECK(s.BeginCharacterList().IsOk());
			CHECK(s.State() == AgentState::CharacterListRequested);
			CHECK(!s.MayRequestCharacterSelect());
			CHECK(s.SelectCharacter(owned).IsError());
			CHECK(select.SelectCharacter(s, kCharA1.value).IsError());
			CHECK(entry.AuthorizeWorldEntry(s).IsError());
		}

		// ---- CHARACTER_SELECTED: world entry yes, re-selection no ---------
		{
			AgentSession s(4);
			CHECK(s.CompleteAuthentication(kAccountA, "user_a", false).IsOk());
			CHECK(s.BeginCharacterList().IsOk());
			CHECK(s.CompleteCharacterList().IsOk());
			CHECK(s.SelectCharacter(owned).IsOk());
			CHECK(s.State() == AgentState::CharacterSelected);

			// A DUPLICATE selection, including one of the same character.
			CHECK(s.SelectCharacter(owned).IsError());
			CHECK(select.SelectCharacter(s, kCharA1.value).IsError());
			CHECK(select.SelectCharacter(s, kCharA2.value).IsError());
			CHECK(select.BuildCharacterDetail(s, kCharA2.value, bytes).IsError());
			CHECK(s.State() == AgentState::CharacterSelected);

			// And the selection that IS legal.
			CHECK(s.AuthorizeWorldEntry().IsOk());
		}

		// ---- WORLD_ENTRY_AUTHORIZED: terminal for this milestone ----------
		{
			AgentSession s(5);
			CHECK(s.CompleteAuthentication(kAccountA, "user_a", false).IsOk());
			CHECK(s.BeginCharacterList().IsOk());
			CHECK(s.CompleteCharacterList().IsOk());
			CHECK(s.SelectCharacter(owned).IsOk());
			CHECK(s.AuthorizeWorldEntry().IsOk());
			CHECK(s.State() == AgentState::WorldEntryAuthorized);

			// A SECOND world entry from the same Agent session. There is no second
			// entry in this milestone, and allowing one would mint a second gaeaId for a
			// character that is already placed.
			CHECK(s.AuthorizeWorldEntry().IsError());
			CHECK(select.SelectCharacter(s, kCharA1.value).IsError());
			CHECK(select.BuildCharacterList(s, scratch).IsError());
		}

		// ---- CLOSED: nothing at all --------------------------------------
		{
			AgentSession s(6);
			CHECK(s.CompleteAuthentication(kAccountA, "user_a", false).IsOk());
			s.Close();
			CHECK(s.IsClosed());
			CHECK(s.BeginCharacterList().IsError());
			CHECK(s.CompleteCharacterList().IsError());
			CHECK(s.SelectCharacter(owned).IsError());
			CHECK(s.AuthorizeWorldEntry().IsError());
			CHECK(select.BuildCharacterList(s, scratch).IsError());
			CHECK(select.SelectCharacter(s, kCharA1.value).IsError());
			CHECK(entry.AuthorizeWorldEntry(s).IsError());
			CHECK(!s.IsAuthenticated());
		}
	}

	MODERN_TEST(WorldEntry_PhaseB_IllegalFieldTransitionsAreAllRefused)
	{
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;

		Network::FieldIdentity nothing;

		// CONNECTED: spawn is illegal, and a nonsense identity does not advance it.
		{
			FieldSession s(repository, registry, 1);
			FieldSpawnResult result;
			CHECK(s.BuildSpawn(result).IsError());
			CHECK(s.ValidateIdentity(nothing, 0).IsError());
			CHECK(s.State() == FieldState::Connected);
			CHECK(!s.MaySpawn());
		}

		// IDENTITY_VALIDATED: one spawn, then no more.
		{
			FieldEntryRegistry           local;
			InMemoryCharacterRepository localRepo = MakePopulatedRepository();
			FieldSession                 s(localRepo, local, 2);

			// Reserve allocates gaeaId 1 and slot 1 for the first request.
			const Result<WorldEntryAuthorization> reserved =
			    local.Reserve(localRepo, kAccountA, kCharA1, 1, 0);
			CHECK(reserved.IsOk());

			Network::FieldIdentity identity;
			identity.joinType       = Network::WorldEntry::kJoinTypeFirst;
			identity.gaeaId         = reserved.GetValue().gaeaId;
			identity.slotFieldAgent = reserved.GetValue().slotFieldAgent;
			identity.cryptKey       = RanWire::DefaultCryptKey();

			CHECK(s.ValidateIdentity(identity, 0).IsOk());
			CHECK(s.State() == FieldState::IdentityValidated);
			CHECK(s.MaySpawn());

			// A second 2359 on the same connection.
			CHECK(s.ValidateIdentity(identity, 0).IsError());
			CHECK(s.State() == FieldState::IdentityValidated);

			FieldSpawnResult result;
			CHECK(s.BuildSpawn(result).IsOk());
			CHECK(s.BuildSpawn(result).IsError());
			CHECK(s.State() == FieldState::Spawned);
			CHECK(!s.MaySpawn());
		}

		// CLOSED: nothing.
		{
			FieldSession s(repository, registry, 3);
			s.Close();
			CHECK(s.IsClosed());
			FieldSpawnResult result;
			CHECK(s.BuildSpawn(result).IsError());
			CHECK(s.ValidateIdentity(nothing, 0).IsError());
		}
	}

	MODERN_TEST(WorldEntry_PhaseB_AuthenticationCannotBeForgedOrRepeated)
	{
		AgentSession s(1);

		// An account id of 0 is refused: a session holding 0 would make every
		// ownership check answer "not yours" for reasons that look like a repository
		// bug.
		CHECK(s.CompleteAuthentication(WorldAccountId{ 0 }, "user_a", false).IsError());
		CHECK(s.State() == AgentState::Connected);

		// An empty user id likewise.
		CHECK(s.CompleteAuthentication(kAccountA, "", false).IsError());
		CHECK(s.State() == AgentState::Connected);

		// A rejected login records nothing at all.
		CHECK(s.CompleteAuthentication(kAccountA, "user_a", true).IsError());
		CHECK(s.Account().value == 0u);
		CHECK(s.UserId().empty());
		CHECK(!s.IsAuthenticated());

		// A real one succeeds, and cannot be repeated - a second call must not
		// re-point an authenticated session at a different account.
		CHECK(s.CompleteAuthentication(kAccountA, "user_a", false).IsOk());
		CHECK(s.State() == AgentState::Authenticated);
		CHECK_EQ(s.Account().value, kAccountA.value);
		CHECK(s.IsAuthenticated());
		CHECK(s.CompleteAuthentication(kAccountB, "user_b", false).IsError());
		CHECK_EQ(s.Account().value, kAccountA.value);
	}

	MODERN_TEST(WorldEntry_PhaseB_AFailedListDoesNotAdvanceTheSession)
	{
		// An account with MORE characters than this build can announce. The list must
		// fail and the session must stay where it was, because a session that advanced
		// past CharacterListSent would let the next request skip a step that never
		// completed.
		InMemoryCharacterRepository repository;
		CHECK(repository.Add(MakeCharacter(WorldCharacterId{ 11 }, kAccountA, "One", 1)).IsOk());
		CHECK(repository.Add(MakeCharacter(WorldCharacterId{ 12 }, kAccountA, "Two", 1)).IsOk());
		CHECK(repository.Add(MakeCharacter(WorldCharacterId{ 13 }, kAccountA, "Three", 1)).IsOk());
		CHECK(repository.Add(MakeCharacter(WorldCharacterId{ 14 }, kAccountA, "Four", 1)).IsOk());
		CHECK(repository.Add(MakeCharacter(WorldCharacterId{ 15 }, kAccountA, "Five", 1)).IsOk());
		CHECK_EQ(repository.Size(), static_cast<std::size_t>(5));

		CharacterSelectService select(repository);
		AgentSession session(1);
		CHECK(session.CompleteAuthentication(kAccountA, "user_a", false).IsOk());

		CharacterListResult list;
		CHECK(select.BuildCharacterList(session, list).IsError());

		// LEFT AT CharacterListRequested, not advanced, and no frame produced. A
		// partial list would show the client three of five characters, and it would
		// never learn the other two existed.
		CHECK(session.State() == AgentState::CharacterListRequested);
		CHECK_EQ(list.frame.size(), static_cast<std::size_t>(0));
		CHECK_EQ(list.expectedDetailCount, static_cast<std::size_t>(0));

		// Which also means a selection is not legal yet.
		CHECK(select.SelectCharacter(session, 11).IsError());
	}

	MODERN_TEST(WorldEntry_PhaseB_NoWorldEntryAcknowledgementIsInvented)
	{
		// The protocol has nothing to acknowledge and this milestone invents nothing.
		//
		// NET_MSG_GAME_JOIN_OK (2355) has both send sites commented out
		// (s_CFieldServerMsg.cpp:433-447, s_CAgentServerMsg.cpp:900-912) and was
		// Field->Session, never Field->Client. NET_MSG_LOBBY_GAME_COMPLETE (2354) is
		// synthesised by the CLIENT (DxGameStage.cpp:581).
		//
		// So the whole Agent+Field exchange produces exactly three client-visible
		// frames: the 2358 redirect, the 2333 spawn, and - on a refused selection -
		// the 2335. Asserted as a count so that adding a fourth would fail here and
		// have to be justified.
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		FieldEntryRegistry           registry;
		CharacterSelectService       select(repository);
		WorldEntryService            entry(repository, registry);
		entry.SetFieldEndpoint(MakeEndpoint());

		AgentSession agent = MakeSessionReadyToSelect(kAccountA, "user_a");
		FieldSession field(repository, registry, 1);

		const AuthorizedFlow flow = AuthorizeFlow(agent, select,
		                                          entry, kCharA1);
		CHECK(flow.ok);

		// 1: the 2358 to the client. Rebuilt from the authorization the Agent minted,
		// which is exactly what the client would have received.
		Network::FieldRedirect redirect;
		redirect.joinType       = Network::WorldEntry::kJoinTypeFirst;
		redirect.gaeaId         = flow.authorization.gaeaId;
		redirect.slotFieldAgent = flow.authorization.slotFieldAgent;
		redirect.servicePort    = entry.Endpoint().servicePort;
		redirect.fieldIp        = entry.Endpoint().address;

		std::vector<WireU8> redirectFrame;
		CHECK(WorldEntryCodec::AppendFieldRedirect(redirectFrame, redirect).IsOk());
		CHECK_EQ(redirectFrame.size(), static_cast<std::size_t>(48));

		// 2: the 2333 spawn.
		CHECK(field.ValidateIdentity(IdentityFor(flow.authorization), 0).IsOk());
		FieldSpawnResult spawn;
		CHECK(field.BuildSpawn(spawn).IsOk());
		CHECK_EQ(spawn.frame.size(), static_cast<std::size_t>(1022));

		// 3: and nothing else. No ack, no complete, no 2354, no 2355.
		CHECK_EQ(registry.PendingCount(), static_cast<std::size_t>(0));
	}

	MODERN_TEST(WorldEntry_PhaseB_ARefusedSelectionProducesThe2355ShapeOnly)
	{
		// The one refusal the protocol DOES define: NET_MSG_LOBBY_CHAR_JOIN_FB (2335),
		// which CAgentServer would emit when the entry is refused. Phase A's codec
		// carries it as an opaque reason.
		InMemoryCharacterRepository repository = MakePopulatedRepository();
		CharacterSelectService        select(repository);

		AgentSession session = MakeSessionReadyToSelect(kAccountA, "user_a");
		CHECK(select.SelectCharacter(session, kCharB1.value).IsError());

		// The refusal frame exists and is 12 bytes; what this milestone does NOT do is
		// invent which EMCHAR_JOIN_FB value corresponds to "not yours", because the
		// investigation did not read that enum's declaration. Carrying it opaquely is
		// the honest choice - see WorldEntryProtocol.h.
		std::vector<WireU8> refusal;
		CHECK(WorldEntryCodec::AppendJoinFailure(refusal, 0).IsOk());
		CHECK_EQ(refusal.size(), static_cast<std::size_t>(12));
		CHECK_EQ(PeekLE32(refusal, 4), static_cast<WireU32>(2335));
	}
}