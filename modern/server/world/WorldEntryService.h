#pragma once

// WORLD-ENTRY-001 Phase B: world-entry authorization, the Agent's redirect, and
// the Field role's validation of the second connection.
//
// ---------------------------------------------------------------------------
// THE FLOW THIS REPRODUCES
// ---------------------------------------------------------------------------
//
//   client  2353 {nChaNum}  ──►  Agent
//            Agent resolves + ownership-checks the character
//            Agent ──2356──► Field      server-to-server, client never sees it
//            Field allocates gaeaId, replies 2357 {gaeaId, slotFieldAgent}
//            Agent ──2358──► client      {ip, port, gaeaId, slotFieldAgent}
//   client opens TCP #2
//   client  2359 {gaeaId, slot, ck}  ──►  Field
//            Field ──2333──► client      {gaeaId, map, pos, SCHARDATA}
//
// Legacy evidence: `CAgentServer::MsgGameJoin` (s_CAgentServerMsg.cpp:586-660) and
// `GLChar::MsgGameJoin` (reached from s_CFieldServerMsg.cpp:159).
//
// ---------------------------------------------------------------------------
// 2356 AND 2357 BECOME ONE IN-PROCESS CALL - AND THAT IS THE POINT
// ---------------------------------------------------------------------------
//
// The two messages are NOT modelled, deliberately and with one exception noted
// below. `FieldEntryRegistry::Reserve` is their replacement:
//
//   - 2356 (NET_GAME_JOIN_FIELDSVR, s_NetGlobal.h:4165-4241) carries emType,
//     dwSlotAgentClient, szUID, nUserNum, dwUserLvl, nChaNum, dwGaeaID, sStartMap,
//     dwStartGate, vStartPos, dwActState, premium dates, cafe classes and more.
//     NOTE dwGaeaID is 0 in the constructor: the Agent does not know the entity id
//     yet, it is the Field that allocates it.
//   - 2357 (NET_GAME_JOIN_FIELDSVR_FB, :4244-4266) returns dwSlotFieldAgent,
//     dwFieldSVR, dwGaeaID, nChaNum.
//
// So the only thing 2357 adds is the ALLOCATION, and the only thing 2356 carries
// that matters here is the character identity. A process boundary is a transport,
// and this milestone has chosen not to open one; the two roles are separate objects
// in one process, so the exchange is a direct call and the client-visible packets
// (2358, 2359, 2333) are untouched. This is the one documented simplification of the
// design agreed in the investigation §9.
//
// The fields of 2356 that ARE represented: the character identity (nUserNum,
// nChaNum), the start map and position - which the Field needs and which this
// repository already holds authoritatively - and the gaeaId, which is allocated.
// Everything else in 2356 belongs to subsystems this milestone excludes.
//
// ---------------------------------------------------------------------------
// WHY THE AUTHORIZATION IS A (gaeaId, slot) PAIR, AND WHY THAT IS NOT A FAKE TOKEN
// ---------------------------------------------------------------------------
//
// NET_GAME_JOIN_FIELD_IDENTITY (2359, s_NetGlobal.h:4301-4319) is:
//
//     { nmg; EMGAME_JOINTYPE emType; DWORD dwGaeaID; DWORD dwSlotFieldAgent;
//       CRYPT_KEY ck; }
//
// There is NO account id and NO character id on it. That is the whole protocol
// design, and it is why the brief's central requirement - "Field must be able to
// prove that the 2359 identity corresponds to a character that the Agent already
// authorized" - is satisfiable at all: the client proves who it is by presenting
// the pair that only the 2358 it received could have given it.
//
// dwSlotFieldAgent is exactly that correlator in legacy, and gaeaId is the entity
// the Field itself allocated. So `FieldEntryRegistry` is a registry of pending
// allocations, keyed by gaeaId, cross-checked against slot. No secret, no
// signature, no crypto - CRYPT_KEY is {1,1} and protects nothing (Phase A,
// RanWirePrimitives.h).
//
// What makes that safe is not secrecy, it is EXCLUSIVITY and SINGLE USE:
//   - the pair is minted only after ownership has been proven;
//   - it is consumed on first successful validation, so a replay fails;
//   - it expires on NET_TIME_OUT, so an abandoned entry attempt dies;
//   - the Field re-checks ownership against the repository at validation time, so
//     the Agent cannot have authorized a character the store would now refuse.
//
// ---------------------------------------------------------------------------
// GAEAID IS RE-ALLOCATED ON EVERY ENTRY, ON PURPOSE
// ---------------------------------------------------------------------------
//
// A fresh Reserve always allocates a new gaeaId. That makes a second, older
// authorization for the same character detectable: its snapshot names a gaeaId the
// character no longer holds, and validation rejects it as stale. Reusing the id
// would make a stale authorization accidentally valid, which is the failure this
// design exists to prevent.

#include "WorldEntryProtocol.h"
#include "types/Result.h"
#include "world/AgentSession.h"
#include "world/CharacterRepository.h"
#include "world/WorldCharacter.h"

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace Modern::Server::World
{
	// Where the client should open the second connection.
	//
	// Supplied by configuration, never invented here. Phase C's listener binds this
	// address and Phase C's client dials it; this phase only puts it on the wire in
	// 2358 so that the value is real rather than a placeholder the client is
	// expected to ignore.
	struct FieldEndpoint
	{
		std::string       address;  // at most kAddressFieldSize-1 characters
		Network::WireI32  servicePort = 0;
	};

	// One pending world entry: the Field role's record that the Agent has authorized
	// a specific character, and the only pair a client may present to claim it.
	struct WorldEntryAuthorization
	{
		// Monotonic, so two authorizations are distinguishable in a log even when
		// they name the same character - which happens on a retry.
		Network::WireU64 authorizationId = 0;

		// The Agent session that requested it. Held so the Field side can reject an
		// authorization whose Agent connection has since gone away.
		Network::WireU64 agentSessionId = 0;

		// The snapshot of what was authorized. Deliberately a copy rather than a
		// pointer into the repository: validation compares the character's CURRENT
		// gaeaId against this one, and a live reference would make that comparison
		// vacuous.
		WorldAccountId     accountId;
		WorldCharacterId   characterId;
		std::string   userId;
		Network::WireU32 gaeaId = 0;
		Network::WireU32 slotFieldAgent = 0;

		// The Agent's clock at mint time, for the staleness rule.
		Network::WireU64 createdAtMs = 0;

		// Set once a 2359 has successfully claimed it. A second claim fails on this
		// alone, before any comparison - which is what makes a replay cheap to refuse
		// and impossible to confuse with a mistyped id.
		bool consumed = false;
	};

	// What the Field role concludes from a validated 2359.
	//
	// Carries the AUTHORITATIVE character, re-read from the repository at validation
	// time. Not the Agent's snapshot: if the store changed since authorization, the
	// store wins.
	struct ValidatedWorldEntry
	{
		WorldEntryAuthorization authorization;
		WorldCharacter          character;
	};

	// ---------------------------------------------------------------------------
	// The Field role's side of the in-process hop.
	// ---------------------------------------------------------------------------
	//
	// Owns the pending authorizations and the gaeaId allocation. Deliberately a
	// separate object from WorldEntryService: in legacy the Agent and the Field are
	// different processes with different owners, and collapsing them into one class
	// would let the Agent's code mint or consume its own proof of authorization.
	// Keeping them apart means an authorization can only be created by the Field
	// role and only checked by the Field role.
	class FieldEntryRegistry
	{
	public:
		// The 2356-equivalent. Allocates a gaeaId and a slot, records a pending
		// authorization, and returns it to the Agent.
		//
		// `nowMs` is passed rather than read: the registry owns no clock, which is
		// what lets a test make an authorization stale without sleeping.
		//
		// Refuses a zero character id or account id, and a character the repository
		// does not hold. The Agent is not trusted to have checked ownership - the
		// registry re-proves it, because the Field is where the character will exist.
		Result<WorldEntryAuthorization> Reserve(ICharacterRepository& repository,
		                                        WorldAccountId accountId,
		                                        WorldCharacterId characterId,
		                                        Network::WireU64 agentSessionId,
		                                        Network::WireU64 nowMs);

		// The 2359-equivalent check. Does NOT consume: `peek` semantics are provided
		// separately so a caller can inspect without spending.
		//
		// Rejects, in this order:
		//   - an unknown gaeaId (nothing was ever authorized with this entity id);
		//   - a slot that does not match that gaeaId's authorization;
		//   - a join type outside the known EMGAME_JOINTYPE values;
		//   - an already-consumed authorization (a replay);
		//   - an authorization older than NET_TIME_OUT;
		//   - a character the repository no longer holds;
		//   - a character whose CURRENT gaeaId differs from the snapshot's - i.e. the
		//     character has entered the world again since, so this authorization is
		//     from a previous appearance;
		//   - a character no longer owned by the authorized account.
		Result<ValidatedWorldEntry> Validate(const ICharacterRepository& repository,
		                                     const Network::FieldIdentity& identity,
		                                     Network::WireU64 nowMs);

		// Validates and consumes in one step, which is what a Field session does.
		Result<ValidatedWorldEntry> Claim(const ICharacterRepository& repository,
		                                  const Network::FieldIdentity& identity,
		                                  Network::WireU64 nowMs);

		// Marks an authorization consumed without validating it. Used when the
		// connection dies mid-entry, so an abandoned entry does not stay claimable.
		void Discard(Network::WireU32 gaeaId) noexcept;

		// Pending (unconsumed) authorizations. For tests and for the Agent's
		// diagnostics.
		std::size_t PendingCount() const noexcept;

		// Total gaeaIds ever allocated. Never decreases, so an id is not reused
		// within a process lifetime - see the note at the top of the file.
		Network::WireU32 Allocations() const noexcept { return m_nextGaeaId; }

		// Refuses a gaeaId that would be 0 or UINT32_MAX.
		//
		// UINT32_MAX matters: `NET_GAME_JOIN_OK`'s constructor initialises
		// dwGaeaID = -1 (s_NetGlobal.h:4339), so a sentinel of that value appears in
		// this protocol's vocabulary. Allocating it would make "unset" and "allocated"
		// the same number.
		static constexpr Network::WireU32 kFirstGaeaId = 1;
		static constexpr Network::WireU32 kInvalidGaeaId = 0xFFFFFFFFu;

	private:
		// gaeaId -> authorization. A std::map because gaeaIds are allocated in
		// increasing order and the registry is small; ordered iteration makes the
		// PendingCount sweep deterministic.
		std::map<Network::WireU32, WorldEntryAuthorization> m_pending;

		// The next gaeaId to hand out. Monotonic, never decremented.
		Network::WireU32 m_nextGaeaId = kFirstGaeaId;

		// The next dwSlotFieldAgent to hand out. Legacy's slot is a connection slot
		// number on the Field; here it is a correlation value, and it must not collide
		// with a gaeaId in the same registry lookup, so it counts independently.
		Network::WireU32 m_nextSlot = 1;

		// Authorization ids count independently of gaeaIds too. They exist for logs
		// and for telling two authorizations of the SAME character apart - which is
		// exactly what a retry produces - and they must keep doing that after
		// Wraparound, so they are WireU64 and never reused within a process lifetime.
		Network::WireU64 m_nextAuthorizationId = 1;
	};

	// ---------------------------------------------------------------------------
	// The Agent role's side.
	// ---------------------------------------------------------------------------
	//
	// Produces the authoritative world-entry state and the 2358 redirect that
	// carries it. Holds a reference to the Field role's registry, because that is
	// where an authorization must be created - the Agent cannot mint its own proof.
	class WorldEntryService
	{
	public:
		WorldEntryService(ICharacterRepository& repository,
		                  FieldEntryRegistry& registry) noexcept
			: m_repository(repository)
			, m_registry(registry)
		{
		}

		// The configuration surface. Set before use; an unset endpoint is refused at
		// AuthorizeWorldEntry rather than defaulting to something a client might
		// actually dial.
		void SetFieldEndpoint(FieldEndpoint endpoint) noexcept
		{
			m_endpoint = std::move(endpoint);
		}
		// Named `Endpoint()` rather than `FieldEndpoint()` for the same reason
		// AgentSession's account accessor is `Account()`: a member function called
		// FieldEndpoint would shadow the FieldEndpoint TYPE, and the
		// `FieldEndpoint m_endpoint;` member below would stop compiling.
		const FieldEndpoint& Endpoint() const noexcept { return m_endpoint; }

		// Authorizes `session`'s selected character to enter the world, and returns
		// the 2358 the client needs.
		//
		// Requires AgentState::CharacterSelected, moves it to WorldEntryAuthorized, and
		// on success leaves the Field role holding a claimable authorization.
		//
		// Refuses, leaving both the session and the registry untouched:
		//   - a session with no selection, or not in CharacterSelected (the state rule);
		//   - an unset endpoint, or an address too long for 2358's 21-byte field;
		//   - a port outside 1..65535;
		//   - a selected character the repository no longer holds or no longer lists
		//     under this account - re-proven here so that a character moved between
		//     accounts after the selection cannot be authorized.
		Result<Network::FieldRedirect> AuthorizeWorldEntry(AgentSession& session);

		// The authorization just minted for `session`, or nullptr.
		//
		// Exposed because the Field side, not this service, validates it - so a test
		// of the whole flow needs to carry it across, exactly as the client would.
		const WorldEntryAuthorization* LastAuthorization() const noexcept
		{
			return m_hasLastAuthorization ? &m_lastAuthorization : nullptr;
		}

	private:
		ICharacterRepository& m_repository;
		FieldEntryRegistry&    m_registry;

		FieldEndpoint m_endpoint;

		WorldEntryAuthorization m_lastAuthorization;
		bool                     m_hasLastAuthorization = false;
	};
}