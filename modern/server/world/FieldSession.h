#pragma once

// WORLD-ENTRY-001 Phase B: the Field role's per-connection state.
//
// ---------------------------------------------------------------------------
// WHAT THE FIELD OWNS, AND WHY IT IS A SEPARATE ROLE
// ---------------------------------------------------------------------------
//
// The investigation §2.1: Agent owns "authentication, the character list, and the
// entry DECISION"; Field owns "the world, and spawn", and it is the Field that
// hosts the GLChar and emits 2333 (s_CFieldServerMsg.cpp:159 ->
// `GLChar::MsgGameJoin()`).
//
// So this role does two things the Agent must not: it checks the client's 2359
// claim against what the Agent authorized, and it emits the spawn. It cannot do
// either without the Field's own registry, and it must not be able to authorize
// anything - which is why WorldEntryService holds a FieldEntryRegistry by
// reference and this class holds one by reference and only ever reads through
// Claim().
//
// ---------------------------------------------------------------------------
// A FIELD SESSION ACCEPTS NO CHARACTER DATA
// ---------------------------------------------------------------------------
//
// There is no setter for a character's level, resources, map or position anywhere
// in this class, and no parameter to `BuildSpawn` through which a client could
// supply one. The only input is the 2359 identity, and everything else comes from
// `ICharacterRepository` re-read at validation time.
//
// That is the structural answer to "do not let the Field session accept arbitrary
// character data from the client": there is no code path that would accept it.
//
// ---------------------------------------------------------------------------
// NO ROTATION, BECAUSE THE WIRE HAS NOWHERE TO PUT IT
// ---------------------------------------------------------------------------
//
// RAN sends no rotation or angle anywhere in the post-login protocol - the
// investigation searched repo-wide for `vAngle` and `SNETPC_ANGLE` and found
// neither. 2333's layout has a position at offset 41 and nothing after it before
// the character record. SpawnState has three floats and no fourth, so a heading
// could not be encoded even if someone wanted one.
//
// ---------------------------------------------------------------------------
// NO WORLD-ENTRY ACKNOWLEDGEMENT
// ---------------------------------------------------------------------------
//
// None is emitted, and none should be. NET_MSG_GAME_JOIN_OK (2355) has both send
// sites commented out (s_CFieldServerMsg.cpp:433-447,
// s_CAgentServerMsg.cpp:900-912) and was Field->Session, never Field->Client;
// NET_MSG_LOBBY_GAME_COMPLETE (2354) is synthesised by the CLIENT
// (DxGameStage.cpp:581). A modern server has nothing to acknowledge here, and
// inventing an ack because it looks tidy would be reviving dead protocol.

#include "WorldEntryProtocol.h"
#include "types/Result.h"
#include "world/CharacterRepository.h"
#include "world/WorldCharacter.h"
#include "world/MovementStateService.h"
#include "world/WorldEntryService.h"

#include <cstdint>
#include <string>

namespace Modern::Server::World
{
	enum class FieldState : std::uint8_t
	{
		// TCP accepted on the second connection. The Field has no idea who this is:
		// 2359 has not arrived, so nothing about this session may be spawned.
		Connected = 0,

		// A 2359 was accepted and its authorization was CONSUMED. The character is
		// committed to this session and the pair cannot be replayed.
		IdentityValidated,

		// 2333 has been produced. Terminal for this milestone: there is no movement,
		// no world tick and no gameplay message handling yet.
		Spawned,

		// Terminal.
		Closed,
	};

	const char* ToString(FieldState state) noexcept;

	// What a successful world entry produced.
	struct FieldSpawnResult
	{
		// The 2333 frame: exactly 1022 bytes, reserved regions zeroed.
		std::vector<Network::WireU8> frame;

		// The authoritative values that went into it, re-read from the repository.
		// Carried so a caller can log what it actually spawned rather than what it
		// was asked for.
		WorldCharacter character;

		// The entity id the spawn carried.
		Network::WireU32 gaeaId = 0;
	};

	class FieldSession
	{
	public:
		FieldSession(ICharacterRepository& repository,
		             FieldEntryRegistry& registry,
		             Network::WireU64 sessionId = 0) noexcept
			: m_repository(repository)
			, m_registry(registry)
			, m_sessionId(sessionId)
		{
		}

		Network::WireU64 SessionId() const noexcept { return m_sessionId; }
		FieldState       State() const noexcept { return m_state; }
		bool             IsClosed() const noexcept { return m_state == FieldState::Closed; }

		// Handles a 2359: validates the identity against the Field's registry,
		// CONSUMES the authorization, and moves Connected -> IdentityValidated.
		//
		// `nowMs` is the Agent-side clock the session is advanced with, passed in so
		// the staleness rule needs no clock of its own. In Phase C the Field listener
		// will pass the same monotonic time source the Agent uses for its own session.
		//
		// On any failure the session stays Connected, so a client that mistyped its
		// slot can be told why and try again - it has not spent its one claim.
		Status ValidateIdentity(const Network::FieldIdentity& identity,
		                        Network::WireU64 nowMs);

		// Produces the authoritative 2333 spawn, moving IdentityValidated ->
		// Spawned.
		//
		// Requires IdentityValidated, which is what makes "spawn without 2359
		// validation" unreachable rather than merely discouraged.
		Status BuildSpawn(FieldSpawnResult& out);

		// ---- identity -------------------------------------------------------

		// The character this session is committed to, or nullptr before validation.
		const WorldCharacter* Character() const noexcept
		{
			return m_hasCharacter ? &m_character : nullptr;
		}
		Network::WireU32 GaeaId() const noexcept { return m_gaeaId; }
		Network::WireU64 AuthorizationId() const noexcept { return m_authorizationId; }
		Network::WireU64 AgentSessionId() const noexcept { return m_agentSessionId; }
		WorldAccountId          Account() const noexcept { return m_accountId; }

		// ---- authorisation --------------------------------------------------

		bool MaySpawn() const noexcept
		{
			return m_state == FieldState::IdentityValidated;
		}

		// ---- movement state (WORLD-ENTRY-002a) ------------------------------
		//
		// Handles a 3032 on THIS connection: derive the authoritative state and report
		// whether it changed.
		//
		// `changed == false` means legacy sends NOTHING back (GLCharMsg.cpp:203), so
		// the caller must not broadcast. That is observable protocol behaviour rather
		// than an optimisation, and it is why the result distinguishes the two cases
		// instead of always sending.
		//
		// The gaeaId in the result comes from this session's OWN authorized character.
		// A 3032 carries no id at all, so there is nothing for a client to spoof - and
		// `out.gaeaId` is filled from the repository record, never from a parameter.
		//
		// Requires Spawned: a connection that has completed 2359 but has not yet been
		// spawned has no authoritative character to move, and one that has not
		// completed 2359 has no character at all.
		Status ApplyMoveState(Network::WireU32 requestedActState, MovementStateChange& out);

		// The movement-state rule this session applies. Borrowed, not owned: the
		// service holds no per-session state and the repository does.
		void SetMovementStateService(const MovementStateService& service) noexcept
		{
			m_movement = &service;
		}

		// ---- lifecycle ------------------------------------------------------

		// Terminal. Does NOT hand the gaeaId back: the character was placed, and
		// clearing it would let a later entry look like a first. The consumed
		// authorization simply stays consumed.
		void Close() noexcept { m_state = FieldState::Closed; }

		// A closed session that had claimed an authorization leaves the pair spent.
		// Exposed so a Field listener can settle the claim on disconnect rather than
		// relying on the staleness timeout alone.
		void SettleClaim() noexcept
		{
			if (m_gaeaId != 0)
			{
				m_registry.Discard(m_gaeaId);
			}
		}

	private:
		ICharacterRepository& m_repository;
		FieldEntryRegistry&   m_registry;
		Network::WireU64      m_sessionId = 0;

		WorldCharacter m_character;
		bool           m_hasCharacter = false;

		WorldAccountId          m_accountId;
		Network::WireU32   m_gaeaId = 0;
		Network::WireU64   m_authorizationId = 0;
		Network::WireU64   m_agentSessionId = 0;

		const MovementStateService* m_movement = nullptr;

		FieldState m_state = FieldState::Connected;
	};
}