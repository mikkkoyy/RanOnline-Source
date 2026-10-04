#include "world/MovementStateService.h"

namespace Modern::Server::World
{
	using namespace Modern::Network;

	namespace
	{
		// Legacy's SetSTATE / ReSetSTATE (GLChar.h:420-421), verbatim:
		//     void SetSTATE   ( DWORD dwState ) { m_dwActState |=  dwState; }
		//     void ReSetSTATE ( DWORD dwState ) { m_dwActState &= ~dwState; }
		//
		// Reproduced as functions rather than inlined at each call site because the
		// asymmetry is the whole point: one SETS a bit whatever else is there, the other
		// CLEARS it. `&= ~flag` on its own would also clear every other bit if written
		// as an assignment.
		void SetFlag(Network::WireU32& word, Network::WireU32 flag) noexcept
		{
			word |= flag;
		}

		void ResetFlag(Network::WireU32& word, Network::WireU32 flag) noexcept
		{
			word &= ~flag;
		}

		// Applies `flag` to `word` according to whether `requested` carries it.
		//
		// This is the two-line if/else legacy writes out four times, and it is written
		// once here because writing it four times is four chances to write it wrong.
		void ApplyFlagFrom(Network::WireU32& word, Network::WireU32 requested, Network::WireU32 flag) noexcept
		{
			if ((requested & flag) != 0)
			{
				SetFlag(word, flag);
			}
			else
			{
				ResetFlag(word, flag);
			}
		}
	}

	Status MovementStateService::ApplyMoveState(WorldCharacter& character,
	                                            Network::WireU32 requestedActState,
	                                            MovementStateChange& out) const
	{
		out = MovementStateChange{};

		// Server-side corruption check, not a request check.
		//
		// The AUTHORITATIVE word must only ever contain bits RAN defines. If it does
		// not, something else wrote the character wrongly, and masking the stray bits
		// here would hide that bug instead of reporting it. A CLIENT is allowed to send
		// unknown bits - legacy ignores them - so nothing is refused for that.
		if ((character.actState & ~MovementState::kKnownFlags) != 0)
		{
			return Status(ErrorCode::InvalidState);
		}

		const Network::WireU32 previous = character.actState;
		Network::WireU32       derived  = previous;

		// ---- the GM visibility gate -----------------------------------------
		//
		// GLCharMsg.cpp:188-195. Inside the gate because a client below USER_GM3 must
		// not be able to touch these bits AT ALL - not set them and not clear them.
		// Outside the gate, a normal client could unset a flag the server had set.
		//
		// USER_GM3 is 20 and the ordering is inverted (USER_GM4 is 19, USER_GM1 is 22),
		// so this is a threshold, not a rank comparison.
		if (character.accountLevel >= MovementState::kUserGm3Level)
		{
			ApplyFlagFrom(derived, requestedActState, MovementState::kReqVisibleNone);
			ApplyFlagFrom(derived, requestedActState, MovementState::kReqVisibleOff);
		}

		// ---- the two flags every client controls ---------------------------
		// GLCharMsg.cpp:197-201, in legacy's order.
		ApplyFlagFrom(derived, requestedActState, MovementState::kActRun);
		ApplyFlagFrom(derived, requestedActState, MovementState::kActPeaceMode);

		// ---- change detection -----------------------------------------------
		// GLCharMsg.cpp:203: the WHOLE word compared, before and after.
		const bool changed = (previous != derived);

		character.actState = derived;

		out.previousActState = previous;
		out.actState         = derived;
		out.changed          = changed;
		out.gaeaId           = character.gaeaId;

		if (changed)
		{
			if (m_speedProvider != nullptr)
			{
				// Legacy: `float fVelo = GetMoveVelo (); m_actorMove.SetMaxSpeed (fVelo);`
				// (GLCharMsg.cpp:206-208).
				//
				// The result is deliberately DISCARDED. There is no authoritative speed
				// field on WorldCharacter yet, because adding one and filling it from an
				// unimplemented provider would be storing a fabricated number. The call
				// is made so that the SEAM is exercised and the point at which speed will
				// land is already fixed.
				(void)m_speedProvider->MaxSpeedFor(character);
				out.speedRecomputed = true;
			}
			else
			{
				// The state changed and a speed WAS due, but `default.charclass` and the
				// worn-item bonus are out of this milestone. Recorded, not hidden.
				out.speedUnavailable = true;
			}
		}

		return Ok();
	}
}