#include "world/GotoService.h"

#include "MovementStateProtocol.h"

#include <cmath>

namespace Modern::Server::World
{
	namespace
	{
		// GLCharMsg.cpp:254-262, verbatim in behaviour:
		//
		//     BOOL bRun   = IsSTATE ( EM_ACT_RUN );
		//     BOOL bToRun = pNetMsg->dwActState & EM_ACT_RUN;
		//     if ( bRun != bToRun )
		//     {
		//         if ( bToRun )  SetSTATE   ( EM_ACT_RUN );
		//         else           ReSetSTATE ( EM_ACT_RUN );
		//     }
		//
		// ONE bit, and only this one. This is deliberately NOT
		// `MovementStateService::ApplyMoveState`, which is 3032's rule and touches
		// four bits behind the USER_GM3 gate. MsgGoto never reads EM_ACT_PEACEMODE and
		// never reads either visibility bit, so routing a 3034 through the 3032 rule
		// would grant a GOTO authority it does not have.
		//
		// The `bRun != bToRun` guard is preserved as a no-op-equivalent rather than as a
		// branch: setting a bit that is already set and clearing one that is already
		// clear both leave the word unchanged, which is the same result. `changed` is
		// computed from the whole word afterwards so the caller can report it.
		void ApplyRunFlag(Network::WireU32& actState, Network::WireU32 requested,
		                  bool& changed)
		{
			const Network::WireU32 before = actState;

			if ((requested & Network::MovementState::kActRun) != 0)
			{
				actState |= Network::MovementState::kActRun;
			}
			else
			{
				actState &= ~Network::MovementState::kActRun;
			}

			changed = actState != before;
		}
	}

	const char* ToString(GotoOutcome outcome) noexcept
	{
		switch (outcome)
		{
		case GotoOutcome::Accepted:                return "Accepted";
		case GotoOutcome::RejectedDead:            return "RejectedDead";
		case GotoOutcome::RejectedPositionDesync:  return "RejectedPositionDesync";
		case GotoOutcome::RejectedNoMesh:          return "RejectedNoMesh";
		case GotoOutcome::RejectedDestination:     return "RejectedDestination";
		}
		return "Unrecognised";
	}

	GotoResult GotoService::Apply(Movement::Actor& actor, WorldCharacter& character,
	                              const GotoRequest& request) const
	{
		GotoResult result;

		result.previousActState = character.actState;
		result.actState         = character.actState;

		// The server's authoritative position, captured ONCE and used for both the
		// 60-unit comparison and the 3035 payload. Reading it after the check instead
		// would be the same value today and a different one the moment anything else
		// moved the actor, so it is read here and carried.
		const Vector3 serverPosition = actor.Position();

		result.authoritativeCurrent = serverPosition;

		// GLCharMsg.cpp:238, the EM_ACT_DIE check.
		//
		// Runs BEFORE the run bit is applied, so a dying character refuses the GOTO
		// without its run state changing. That ordering is load-bearing and is
		// preserved; see the header.
		if ((character.actState & Network::MovementState::kActDie) != 0)
		{
			result.outcome = GotoOutcome::RejectedDead;
			result.detail  = "the character is dead";
			return result;
		}

		// GLCharMsg.cpp:258-262. Before the distance check, after the die check.
		ApplyRunFlag(character.actState, request.requestedActState, result.actStateChanged);
		result.actState = character.actState;

		// GLCharMsg.cpp:264-266.
		//
		// The FULL 3D length between the SERVER's position and the CLIENT's claim,
		// strictly greater than 60.
		const Vector3 delta = serverPosition - request.claimedCurrent;
		result.claimedDistance = delta.Length();

		if (result.claimedDistance > kGotoPositionTolerance)
		{
			// GLCharMsg.cpp:269-270 refuses silently for a TALKING or GATHERING
			// character. There is no modern action model, so that sub-branch is
			// unreachable here rather than emulated; what remains is the branch that
			// stops the actor and corrects clients. See the header for the two
			// correction packets this milestone does not send.
			actor.Stop();

			result.outcome = GotoOutcome::RejectedPositionDesync;
			result.detail  = "the client's claimed position is " +
			                 std::to_string(result.claimedDistance) +
			                 " units from the server's, over the 60-unit tolerance";
			result.pathActive = actor.PathIsActive();
			return result;
		}

		// The character is walking away from here, so a map with no navigation mesh
		// cannot answer.
		//
		// RAN cannot reach this: its field servers always have the mesh, and the probe
		// simply hits. A modern server without a loaded map registry can, so it is
		// reported as its own outcome rather than as "destination unreachable" - the
		// two have different fixes.
		if (actor.Mesh() == nullptr)
		{
			result.outcome = GotoOutcome::RejectedNoMesh;
			result.detail  = "the character has no navigation mesh";
			return result;
		}

		// GLCharMsg.cpp:290-291.
		//
		// The RAW requested target is stored - not the probe's resolution - and the
		// action becomes MOVE. The action itself has no modern representation; what it
		// buys is the path the actor is about to be given.
		result.authoritativeTarget = request.requestedTarget;

		// GLCharMsg.cpp:293-297: the ±10 vertical probe, about the DESTINATION.
		//
		// `from` is target + 10 in Y and `to` is target - 10, matching the argument
		// order of `Actor::GotoDestination`, whose `IsCollision` walks `from` towards
		// `to`.
		const Vector3 probeFrom{ request.requestedTarget.x, request.requestedTarget.y + 10.0f,
			                     request.requestedTarget.z };
		const Vector3 probeTo{ request.requestedTarget.x, request.requestedTarget.y - 10.0f,
			                   request.requestedTarget.z };

		const bool probeHit = actor.GotoDestination(probeFrom, probeTo);

		result.pathActive = actor.PathIsActive();

		if (!probeHit)
		{
			// GLCharMsg.cpp:299-302. The block is an empty `if` with a commented-out
			// log: no state change, no early return, S_OK. And because `bSucceed` is
			// FALSE, no 3035. The client is told NOTHING - which is the observable
			// behaviour, and inventing an error packet would be a new network message.
			result.outcome = GotoOutcome::RejectedDestination;
			result.detail =
			    "no navigation surface within 10 units of the requested destination";
			return result;
		}

		// GLCharMsg.cpp:304-307: the speed is set only on the success path, through
		// 002a's own seam, so a 3034 and a 3032 cannot disagree about what the
		// character walks at.
		if (m_movement != nullptr && m_movement->HasSpeedProvider())
		{
			result.maxSpeed = m_movement->SpeedFor(character);
			actor.SetMaxSpeed(result.maxSpeed);
		}

		// GLCharMsg.cpp:311-318.
		//
		//     dwGaeaID   = m_dwGaeaID      - the server's entity id
		//     dwActState = m_dwActState    - the SERVER's word, after the run bit
		//     vCurPos    = m_vPos          - the server's position
		//     vTarPos    = m_TargetID.vPos - the RAW requested target
		//     fDelay     = 0.0f
		//
		// `dwGaeaID` is filled by the Field role from the session's own authorized
		// character, never from the packet: a 3034 carries no id to forge.
		result.accepted = true;
		result.outcome  = GotoOutcome::Accepted;
		return result;
	}
}
