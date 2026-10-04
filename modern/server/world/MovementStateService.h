#pragma once

// WORLD-ENTRY-002a: the authoritative movement-STATE rule.
//
// This is GLChar::MsgMoveState (GLCharMsg.cpp:182-219) translated into a pure
// function over a WorldCharacter, with no socket, no packet and no actor.
//
// ---------------------------------------------------------------------------
// THE SERVER NEVER STORES THE CLIENT'S VALUE
// ---------------------------------------------------------------------------
//
// The single most important property, and the easiest to get wrong. Legacy does NOT
// do `m_dwActState = pNetMsg->dwActState`. It applies FOUR individual bits to a
// persistent word:
//
//     if ( dwUserLvl >= USER_GM3 )                       // account privilege gate
//     {
//         if ( requested & EM_REQ_VISIBLENONE ) SetSTATE ( EM_REQ_VISIBLENONE );
//         else                                   ReSetSTATE ( EM_REQ_VISIBLENONE );
//         if ( requested & EM_REQ_VISIBLEOFF  ) SetSTATE ( EM_REQ_VISIBLEOFF  );
//         else                                   ReSetSTATE ( EM_REQ_VISIBLEOFF  );
//     }
//     if ( requested & EM_ACT_RUN       ) SetSTATE ( EM_ACT_RUN       );
//     else                               ReSetSTATE ( EM_ACT_RUN       );
//     if ( requested & EM_ACT_PEACEMODE ) SetSTATE ( EM_ACT_PEACEMODE );
//     else                               ReSetSTATE ( EM_ACT_PEACEMODE );
//
// Three consequences, each of which is a security property:
//
//  1. Bits the client does not own - EM_ACT_DIE, EM_REQ_GATEOUT, EM_REQ_LOGOUT,
//     EM_ACT_WAITING, EM_ACT_CONFT_WIN, EM_ACT_PK_MODE - are PRESERVED. A client
//     cannot set "dead" or "log me out" by putting the bit in a 3032, because the
//     server never reads it.
//
//  2. Below USER_GM3 the two visibility flags are left ALONE, not cleared. A normal
//     client can neither set nor unset them. This is why those two tests are inside
//     the level gate rather than outside it.
//
//  3. The broadcast carries the whole authoritative word, not the request. A client
//     that only learned the delta would have to guess the server-owned bits.
//
// ---------------------------------------------------------------------------
// CHANGE DETECTION, AND WHY IT IS THE WHOLE WORD
// ---------------------------------------------------------------------------
//
// Legacy compares the full 32-bit word before and after:
//
//     DWORD dwOldActState = m_dwActState;   // :186
//     ...apply bits...
//     if ( dwOldActState != m_dwActState )  // :203
//     {
//         float fVelo = GetMoveVelo ();
//         m_actorMove.SetMaxSpeed ( fVelo );
//         ...broadcast...
//     }
//
// So a request that asks for the state the character is already in produces NO
// broadcast and NO speed recomputation. That is observable behaviour, not an
// optimisation, and this milestone reproduces it.
//
// ---------------------------------------------------------------------------
// THE SPEED DEPENDENCY IS A SEAM, NOT A VALUE
// ---------------------------------------------------------------------------
//
// Legacy recomputes `GetMoveVelo()` on every change, and that reads base walk/run
// velocity from `cCONSTCLASS` - loaded from `default.charclass`, a data file NOT in
// this repository - plus `GLCHARLOGIC::GETMOVE_ITEM()`, i.e. worn equipment, which
// WORLD-ENTRY-001 explicitly excludes.
//
// So the interface is declared and NO IMPLEMENTATION IS SUPPLIED. When no provider is
// configured the state change still applies and the broadcast still goes out;
// `speedRecomputed` is false and `speedUnavailable` explains why. The alternative -
// inventing a speed number - would put a fabricated value on the authoritative
// server and make the deferral invisible.

#include "MovementStateProtocol.h"
#include "types/Result.h"
#include "world/WorldCharacter.h"

#include <cstdint>

namespace Modern::Server::World
{
	// Where an authoritative movement speed would come from, if one were available.
	//
	// Declared so the dependency is NAMED at the boundary rather than smuggled in as a
	// constant. WORLD-ENTRY-002a supplies no implementation, because doing so would
	// mean fabricating either `default.charclass` values or an equipment speed bonus.
	class IMovementSpeedProvider
	{
	public:
		virtual ~IMovementSpeedProvider() = default;

		// The maximum speed implied by `actState`, in the same units RAN's
		// `GetMoveVelo()` would return. Never consulted in this milestone.
		virtual float MaxSpeedFor(const WorldCharacter& character) const = 0;
	};

	// What applying a 3032 actually did.
	struct MovementStateChange
	{
		// Whether the authoritative word CHANGED.
		//
		// False means no broadcast and no speed recomputation, which is the behaviour
		// legacy's `dwOldActState != m_dwActState` produces.
		bool changed = false;

		// The authoritative word AFTER the request was applied - the whole word,
		// including bits the client did not touch.
		Network::WireU32 actState = 0;

		// The word BEFORE, so a caller can see exactly which bits moved.
		Network::WireU32 previousActState = 0;

		// The gaeaId of the character the state belongs to. Taken from the
		// AUTHORITATIVE character, never from a packet: 3032 carries no id at all.
		Network::WireU32 gaeaId = 0;

		// Whether the speed seam was actually consulted.
		bool speedRecomputed = false;

		// True when a speed was due (the state changed) but no provider was
		// configured. Recorded rather than hidden, so a caller can tell "speed is
		// unchanged because it cannot be computed" from "speed is unchanged because
		// the state did not change".
		bool speedUnavailable = false;
	};

	// Applies one 3032 to `character`, in place.
	//
	// Returns InvalidArgument only for a request the protocol cannot express: a
	// dwActState with bits outside EMCHAR_ACTSTATE. Legacy never validates this - it
	// simply ignores bits it does not own - and this matches that by ACCEPTING such a
	// request and ignoring the unknown bits, because a client that sets a bit RAN does
	// not define is not making a protocol error, it is making a request this server
	// has no opinion about.
	//
	// What is refused is a character whose EXISTING authoritative word is corrupt,
	// because that is server state, not a request, and silently masking it would hide
	// a bug.
	class MovementStateService
	{
	public:
		// `speedProvider` may be null. See the header for why that is a supported
		// configuration rather than a missing feature.
		explicit MovementStateService(const IMovementSpeedProvider* speedProvider = nullptr) noexcept
			: m_speedProvider(speedProvider)
		{
		}

		// Returns whether a speed provider is configured, for diagnostics.
		bool HasSpeedProvider() const noexcept { return m_speedProvider != nullptr; }

		Status ApplyMoveState(WorldCharacter& character, Network::WireU32 requestedActState,
		                     MovementStateChange& out) const;

	private:
		const IMovementSpeedProvider* m_speedProvider = nullptr;
	};
}