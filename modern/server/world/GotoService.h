#pragma once

// WORLD-ENTRY-002f: the GOTO rule, GLChar::MsgGoto transliterated.
//
// `GLChar::MsgGoto` (GLCharMsg.cpp:224-324) is 100 lines of ordered checks over
// four inputs: the authoritative state word, the authoritative position, the
// client's claim of where it is, and the client's requested destination. This is
// that function with the socket and the GLChar removed, so it can be tested against
// a real navigation mesh without a client.
//
// ---------------------------------------------------------------------------
// THE ORDER IS THE CONTRACT, AND THREE OF THE CHECKS ARE NOT MODELLABLE HERE
// ---------------------------------------------------------------------------
//
// GLCharMsg.cpp, in order:
//
//     :226  m_bEmptyMsg          -> S_OK, do nothing        NOT MODELLED
//     :227  m_sPMarket.IsOpen()  -> E_FAIL                  NOT MODELLED
//     :230  m_bSTATE_STUN        -> Stop(), E_FAIL          NOT MODELLED
//     :238  IsSTATE(EM_ACT_DIE)  -> 3031 to own client, E_FAIL   MODELLED (the refusal)
//     :258  apply EM_ACT_RUN from the packet          ->   MODELLED
//     :266  |m_vPos - vCurPos| > 60                   ->   MODELLED
//     :291  m_TargetID.vPos = vTarPos ; TurnAction(MOVE)    MODELLED
//     :293  GotoLocation(vTarPos +-10 Y)              ->   MODELLED (Actor)
//     :304  if it hit: SetMaxSpeed(GetMoveVelo()) ; 3035     MODELLED
//     :321  MsgSendUpdateState(false,false,true)             NOT MODELLED
//
// The four NOT MODELLED lines are named here rather than silently dropped, because
// each is a real guard and a reader needs to know it is missing and why:
//
//   * `m_bEmptyMsg` is GLGaeaServer's anti-spam counter (a client that sent too many
//     messages in one frame). WORLD-ENTRY-002a did not model the anti-spam system
//     either, and inventing a message-count threshold would be inventing a rate limit
//     RAN's own threshold for.
//   * `m_sPMarket` is the private-market subsystem, explicitly out of scope.
//   * `m_bSTATE_STUN` is a server-side state flag with no 3034/3032 route that sets
//     it. Modelling it would mean inventing a way to be stunned.
//   * `MsgSendUpdateState` sends `SNETPC_UPDATE_STATE`, the HP/MP/SP packet. Those
//     resources exist on `WorldCharacter` but that packet is not in the modern wire,
//     and adding it would be a new network message - which this milestone is told not
//     to introduce.
//
// The 3031 that the EM_ACT_DIE branch sends is likewise not modelled, for the same
// reason. What IS modelled is the branch's EFFECT on the world: the GOTO is refused.
//
// ---------------------------------------------------------------------------
// THE 60-UNIT CHECK IS THE ANTI-TELEPORT, AND IT IS 3D
// ---------------------------------------------------------------------------
//
// GLCharMsg.cpp:264-266:
//
//     D3DXVECTOR3 vDist = m_vPos - pNetMsg->vCurPos;
//     float fDist = D3DXVec3Length(&vDist);
//     if ( fDist > 60.0f ) { ... }
//
// Three details that are easy to get wrong and all change behaviour:
//
//   * It compares against the SERVER's `m_vPos`, and the CLIENT's `vCurPos`. It is
//     the only place a 3034's `vCurPos` is read at all.
//   * It is the FULL 3D length, not XZ. A client 40 units below its server position
//     is 40 units out and refused.
//   * The comparison is STRICTLY greater than, so exactly 60.0 passes.
//
// On failure legacy does one of two things, and the difference is visible:
//
//   * if `m_Action` is GLAT_TALK or GLAT_GATHERING: nothing is sent at all.
//   * otherwise: `TurnAction(GLAT_IDLE)`, then `SNET_GM_MOVE2GATE_FB` (3830) to the
//     moving client and `SNETPC_JUMP_POS_BRD` (3064) to everyone around it, then S_OK.
//
// GLAT_TALK and GLAT_GATHERING have no modern equivalent - there is no action model -
// so only the second branch is reachable, and this implementation performs its
// EFFECT on the world (the actor stops) and reports the outcome distinctly.
//
// 3830 and 3064 are NOT implemented. They are named deferrals, and the consequence
// is stated rather than hidden: a client that has drifted is refused its GOTO and is
// NOT told to correct its position, because neither correction packet is in the
// modern wire. The server's position is still authoritative and still unchanged.
//
// ---------------------------------------------------------------------------
// THE RUN BIT IS APPLIED BEFORE THE 60-UNIT CHECK, AND THAT IS THE POINT
// ---------------------------------------------------------------------------
//
// GLCharMsg.cpp:258-262 runs BEFORE :266. So a client that sends a >60-unit GOTO with
// a changed run flag still gets its run flag applied - the desynchronisation check
// governs MOVEMENT, not STATE.
//
// Reproduced, and reproduced BEFORE the check rather than after. The order also
// matters relative to the EM_ACT_DIE check at :238, which runs BEFORE the run bit:
// a dying character refuses the GOTO without its run state changing. That asymmetry -
// die beats run-flag, run-flag beats distance - is reproduced exactly.
//
// Note this is a DIFFERENT rule from `MovementStateService::ApplyMoveState`, which
// handles 3032 and applies FOUR bits (with the USER_GM3 gate). A 3034 applies ONE.
// Routing a 3034 through the 3032 service would let it touch the two visibility bits,
// which MsgGoto never reads - so this file carries its own single-bit rule, quoted,
// rather than reusing the wider one.

#include "GotoProtocol.h"
#include "math/Vector3.h"
#include "movement/Actor.h"
#include "types/Result.h"
#include "world/MovementStateService.h"
#include "world/WorldCharacter.h"

#include <cstdint>
#include <string>

namespace Modern::Server::World
{
	// GLCharMsg.cpp:266. The desynchronisation threshold, in world units, measured
	// over the FULL 3D distance.
	//
	// Not a modern tolerance chosen to be generous. It is RAN's number, and a
	// different one would let a client claim a position further from the server's own
	// than RAN ever permitted.
	inline constexpr float kGotoPositionTolerance = 60.0f;

	enum class GotoOutcome : std::uint8_t
	{
		// The ±10 probe hit. Legacy sends 3035 and returns S_OK.
		//
		// Note this does NOT promise the character will arrive. See the `pathActive`
		// field of GotoResult: a probe that hits and a pathfind that fails still lands
		// here, because MsgGoto's `bSucceed` is the probe's.
		Accepted,

		// The character is not in a state where it can be told to move:
		// EM_ACT_DIE is set (GLCharMsg.cpp:238).
		RejectedDead,

		// |serverPos - claimedCurPos| > 60 (GLCharMsg.cpp:266).
		//
		// The actor has been STOPPED, which is the part of the branch that affects the
		// world.
		RejectedPositionDesync,

		// The character's map has no navigation mesh: unregistered, missing `.lev` or
		// `.wld`, or `bExist == 0`.
		//
		// RAN cannot produce this - a field server always has its meshes - so it is a
		// modern condition. It is reported as its own outcome rather than folded into
		// "destination unreachable", because the two have different fixes: one is a
		// deployment problem and the other is a player's click.
		RejectedNoMesh,

		// The ±10 vertical probe found no floor under the destination.
		//
		// Legacy's silent rejection: `bSucceed == FALSE` means no 3035, no movement,
		// and the client is told nothing at all. Reproduced, including the silence -
		// inventing an error packet would be a new network message.
		RejectedDestination,
	};

	const char* ToString(GotoOutcome outcome) noexcept;

	// A 3034, expressed in the types the rule works in.
	//
	// Built from `Network::Goto::GotoRequest` by the Field role. `claimedCurrent` is
	// the client's assertion and is used for exactly one thing - the 60-unit check.
	struct GotoRequest
	{
		// `pNetMsg->dwActState`. Only EM_ACT_RUN is read.
		Network::WireU32 requestedActState = 0;

		// `pNetMsg->vCurPos`. The CLAIM.
		Vector3 claimedCurrent{};

		// `pNetMsg->vTarPos`. The REQUEST.
		Vector3 requestedTarget{};
	};

	// What applying one 3034 did.
	struct GotoResult
	{
		GotoOutcome outcome = GotoOutcome::RejectedDestination;

		// Whether a 3035 goes out. True only for Accepted.
		bool accepted = false;

		// Whether the authoritative word CHANGED because of this request.
		//
		// Legacy does not branch on this in MsgGoto - it applies the run bit
		// unconditionally and compares nothing - but reporting it makes "a GOTO that
		// also changed run" distinguishable from "a GOTO that did not", which is the
		// difference a test needs and which the wire does not carry.
		bool actStateChanged = false;

		// The AUTHORITATIVE word after the run bit was applied. Never the request.
		Network::WireU32 actState         = 0;
		Network::WireU32 previousActState = 0;

		// |serverPosition - claimedCurrent|, before any change. 3D, GLCharMsg.cpp:265.
		float claimedDistance = 0.0f;

		// The server's position at the moment of the decision. This is what 3035's
		// `vCurPos` carries.
		Vector3 authoritativeCurrent{};

		// The RAW requested target.
		//
		// Not the point the ±10 probe resolved to. GLCharMsg.cpp:290 stores the client's
		// value and :315 broadcasts `m_TargetID.vPos`, so peers are told where the mover
		// ASKED to go while the server paths to a point up to 10 units away in Y.
		// Reproduced: see the header.
		Vector3 authoritativeTarget{};

		// `SetMaxSpeed(GetMoveVelo())`, i.e. what the actor's speed becomes. Zero when
		// the request was refused, because legacy only calls it on the success path
		// (GLCharMsg.cpp:306).
		float maxSpeed = 0.0f;

		// Whether the actor will ACTUALLY walk: `PathIsActive()` after the request.
		//
		// False with `accepted == true` is a real and reachable state - the probe hit,
		// the A* did not - and the character stops on the next tick
		// (GLChar.cpp:6091-6095). Exposed so the difference is observable rather than
		// inferred from a position that never changed.
		bool pathActive = false;

		// Why a request was refused, in one line. Empty on success.
		std::string detail;
	};

	// Applies one GOTO to an actor and the authoritative character.
	//
	// STATELESS. It holds no position, no destination and no mesh of its own: the
	// actor owns the movement and the character owns the state word. That is what
	// makes it safe to call from a Field worker while a movement tick advances other
	// actors, and it is why the same instance can serve every Field connection.
	//
	// The three inputs are mutated in place and that is deliberate:
	// `character.actState` may gain or lose EM_ACT_RUN, and `actor` may gain a path.
	// Both are the SERVER's authoritative state, and a GOTO is one of the two ways
	// RAN lets a client influence them (the other being 3032).
	class GotoService
	{
	public:
		// The rule, and the one rule it needs. `movement` supplies the speed through
		// its own seam, and is borrowed.
		explicit GotoService(const MovementStateService& movement) noexcept
			: m_movement(&movement)
		{
		}

		GotoResult Apply(Movement::Actor& actor, WorldCharacter& character,
		                 const GotoRequest& request) const;

	private:
		const MovementStateService* m_movement = nullptr;
	};
}
