#include "world/FieldSession.h"

namespace Modern::Server::World
{
	using namespace Modern::Network;

	const char* ToString(FieldState state) noexcept
	{
		switch (state)
		{
		case FieldState::Connected:         return "Connected";
		case FieldState::IdentityValidated: return "IdentityValidated";
		case FieldState::Spawned:           return "Spawned";
		case FieldState::Closed:            return "Closed";
		}

		return "<invalid FieldState>";
	}

	Status FieldSession::ValidateIdentity(const Network::FieldIdentity& identity,
	                                      WireU64 nowMs)
	{
		// Only a fresh session may validate. A second 2359 on the same connection is a
		// protocol fault, and allowing it would let a client re-point an already
		// spawned session at a different authorization.
		if (m_state != FieldState::Connected)
		{
			return Status(ErrorCode::NotAllowed);
		}

		// Zero gaeaId is not an entity and can never have been authorized, because
		// Reserve starts at 1. Checked before the registry so the reason is precise.
		if (identity.gaeaId == 0)
		{
			return Status(ErrorCode::NotAllowed);
		}

		// The Field's own check, and the Field's own registry. The Agent is not asked
		// whether the claim is good; it is asked for nothing at all.
		const Result<ValidatedWorldEntry> claimed =
		    m_registry.Claim(m_repository, identity, nowMs);
		if (claimed.IsError())
		{
			// The session stays Connected. A client that sent a wrong slot has not
			// spent its authorization, and it should be able to try again rather than
			// lose the character it legitimately owns.
			return claimed.GetStatus();
		}

		const ValidatedWorldEntry& validated = claimed.GetValue();

		m_character       = validated.character;
		m_hasCharacter    = true;
		m_gaeaId          = validated.authorization.gaeaId;
		m_authorizationId = validated.authorization.authorizationId;
		m_agentSessionId  = validated.authorization.agentSessionId;
		m_accountId       = validated.authorization.accountId;

		m_state = FieldState::IdentityValidated;
		return Ok();
	}

	Status FieldSession::BuildSpawn(FieldSpawnResult& out)
	{
		out.frame.clear();
		out.gaeaId = 0;

		// The gate. There is no path to a spawn that has not been through a validated
		// 2359, so "FIELD_CONNECTED -> spawn" is refused here rather than producing a
		// spawn for a character this session has not proved it may have.
		if (!m_hasCharacter || m_state != FieldState::IdentityValidated)
		{
			return Status(ErrorCode::NotAllowed);
		}

		const WorldCharacter& character = m_character;

		// The spawn's identity, all from the repository.
		Network::SpawnState spawn;

		// szUserID on the wire, from the character's own login name. Not the session's
		// connection id and not anything the client sent.
		spawn.userId = character.userId;

		// dwClientID. RAN's client id is a per-connection counter
		// (CClientManager allocates it); there is no such counter in this milestone,
		// and inventing one would put a number on the wire that nothing else in the
		// process could agree with. Zero is the honest value, and it is what legacy's
		// own constructor default leaves for a field the caller does not set.
		spawn.clientId = 0;

		// The entity id, map and position: authoritative, and the position is the DB
		// save position (ChaSavePosX/Y/Z, s_COdbcGameChaGet.cpp:214-216). No
		// rotation: the protocol has no field for one.
		spawn.gaeaId         = character.gaeaId;
		spawn.mapId          = character.saveMapId;
		spawn.position       = character.savePosition;

		// Inside the 600-byte record.
		spawn.accountId      = character.accountId.value;
		spawn.characterId    = character.id.value;
		spawn.characterName  = character.name;
		spawn.characterClass = character.characterClass;
		spawn.school         = character.school;
		spawn.level          = character.level;
		spawn.hp             = character.hp;
		spawn.mp             = character.mp;
		spawn.sp             = character.sp;

		// sStartMapID / dwStartGate.
		//
		// Both are the entry point for this appearance, which for a first entry is the
		// saved position's map. The gate is left at 0: RAN's constructor default is
		// UINT_MAX (s_NetGlobal.h:4215), but that value is a "no gate chosen" marker
		// whose interpretation lives in GLAgentServerMsg.cpp:87-240 - the map fallback
		// logic this milestone does not model. Emitting 0 says "gate 0 of the saved
		// map", which is a position the Field can act on, rather than a sentinel whose
		// only correct handling is in code that does not exist here.
		spawn.startMapId = character.saveMapId;
		spawn.startGate  = 0;

		// Everything not assigned above stays zero in the Phase A codec, which is what
		// zeroes the 288 bytes of quickslots, the 44 bytes of counts, both cosmetics,
		// last-call state and the trailing flags. Equipment is not in 2333 at all: RAN
		// sends it as 22 separate SNETLOBBY_CHARPUTON_EX messages after this burst.
		if (const Status status = WorldEntryCodec::AppendSpawn(out.frame, spawn);
		    status.IsError())
		{
			out.frame.clear();
			return status;
		}

		out.character = character;
		out.gaeaId    = character.gaeaId;

		m_state = FieldState::Spawned;
		return Ok();
	}

	// ---------------------------------------------------------------------------
	// WORLD-ENTRY-002a: movement state
	// ---------------------------------------------------------------------------

	Status FieldSession::ApplyMoveState(Network::WireU32 requestedActState, MovementStateChange& out)
	{
		out = MovementStateChange{};

		// The gate. A connection that has not been spawned has no authoritative
		// character, so there is nothing whose state could be updated.
		//
		// Legacy's equivalent is implicit - GLChar::MsgMoveState is only reachable
		// through pChar->MsgProcess, and there is no GLChar until the 2359 has built
		// one. Making it a state rule here means a 3032 arriving early is refused
		// rather than quietly dropped by a lookup that finds nothing.
		if (!m_hasCharacter || m_state != FieldState::Spawned)
		{
			return Status(ErrorCode::NotAllowed);
		}

		if (m_movement == nullptr)
		{
			// A wiring error rather than a runtime condition: the runtime installs the
			// service before it serves anything. Reported rather than defaulted,
			// because a default rule would be an invented movement model.
			return Status(ErrorCode::InvalidState);
		}

		// The session's OWN copy of the authoritative record, taken from the repository
		// at 2359 validation. The service mutates that in place, so the session's view
		// and the repository's copy are the same object rather than two that could
		// drift apart.
		WorldCharacter& character = m_character;

		if (const Status status = m_movement->ApplyMoveState(character, requestedActState, out);
		    status.IsError())
		{
			return status;
		}

		// The identity in the reply is the AUTHORITATIVE one, read back off the
		// character after the update. Nothing in a 3032 could have influenced it,
		// because a 3032 carries no id at all.
		out.gaeaId = character.gaeaId;

		return Ok();
	}
}