#pragma once

// WORLD-ENTRY-002i: the ATTACK validation rule.
//
// WHAT THIS IS, AND WHAT IT DELIBERATELY IS NOT
//
// Legacy RAN validates an attack in GLChar::MsgAttack (GLCharMsg.cpp:326). The
// first half of that function - lines 327 through 366 - is:
//
//   1. m_bEmptyMsg anti-spam refusal            (NOT modelled; see below)
//   2. IsValidBody()                           (the codec's size check)
//   3. m_pLandMan->IsPeaceZone() -> refuse     (NOT available; see below)
//   4. GetTarget(landMan, targetId) -> E_FAIL if absent
//   5. a distance test, and on failure AVOID (3041) + AVOID_BRD (3042)
//
// and then, from line 368, the PvP and battle-mode block: free-PK maps, guid
// battle, school free-PK, club battle and deathmatch, tyranny, school wars,
// capture-the-flag, and PK-bright penalties.
//
// This service is lines 2, 4 and 5 ONLY. That is a deliberate boundary:
//
//   * it is the part that is fully readable in this repository's legacy tree;
//   * it is the part that depends on NOTHING this milestone excludes - no
//     damage, no SP, no HP, no battle modes, no items;
//   * it is the part a client can observe end to end, because a refusal is a
//     3041 and an acceptance is a 3037.
//
// THERE IS NO DAMAGE HERE. Nothing in this file mutates a character, and the
// 3037 it authorises is an ANIMATION and a target, not a hit. The resolution
// half - GLChar::DamageProc (GLChar.cpp:2484-2539), 3043/3044, the SP spend and
// 3049 - is a later milestone and is excluded on purpose.
//
// TWO RANGE CONSTANTS, AND WHY THEY ARE NOT THE SAME KIND OF THING
//
// Legacy's test is (GLCharMsg.cpp:343-347):
//
//   vTarPos    = pTARGET->GetPosition();
//   fDist      = |m_vPos - vTarPos|
//   wAttackRange  = pTARGET->GetBodyRadius() + GETBODYRADIUS() + GETATTACKRANGE() + 2
//   if ( ISLONGRANGE_ARMS() )  wAttackRange += GETSUM_TARRANGE()
//   wAttackAbleDis = wAttackRange + 7
//   if ( fDist > wAttackAbleDis )  -> AVOID
//
// The `+ 7` slack is a VERIFIED legacy constant and is reproduced exactly.
// The rest is ITEM-DERIVED - a body radius from the target's crowd data, a body
// radius and an attack range from the character's equipment - and `item.csv` is
// not decoded into any modern ItemDefinition, so those values cannot be
// obtained and are NOT invented here.
//
// Instead there is ONE prototype constant standing in for the whole
// item-derived sum. It is a placeholder chosen to make the prototype playable
// and to keep the boundary testable, and it is NOT a claim about RAN. It is
// named kPrototypeAttackableDistanceUnits precisely so that no reader can
// mistake it for a recovered legacy value. When item data lands, this constant
// is replaced by the real terms and the rule itself does not change.

#include "math/Vector3.h"
#include "AttackProtocol.h"
#include "types/Result.h"

#include <cstdint>
#include <string>

namespace Modern::Server::World{
		using Network::WireU32;

		// The VERIFIED legacy slack: `wAttackAbleDis = wAttackRange + 7`
		// (GLCharMsg.cpp:347).
		//
		// Added on top of kPrototypeAttackableDistanceUnits so the shape of the
		// legacy formula survives even while its item-derived terms do not.
		inline constexpr float kLegacyRangeSlackUnits = 7.0f;

		// PROTOTYPE. Stands in for
		//     targetBodyRadius + GETBODYRADIUS() + GETATTACKRANGE() + 2
		// and, for a long-range-armed character, `+ GETSUM_TARRANGE()`.
		//
		// NOT legacy-verified and NOT derived from item data. No RAN item,
		// equipment or crowd value has been decoded, so the honest options were
		// "no range check at all" or "one clearly-labelled placeholder" - and a
		// missing check would let a client attack from anywhere, which is a worse
		// defect than a placeholder constant. The exact value is arbitrary within
		// reason and is expected to change.
		inline constexpr float kPrototypeAttackableDistanceUnits = 20.0f;

		// What one 3036 did.
	enum class AttackOutcome : std::uint8_t
		{
			// The request was valid and in range. A 3037 goes out. NO damage.
			Accepted,

			// The connection has not earned a spawn, so there is no character to
			// attack from. Not reachable through the Field dispatch, which routes
			// 3036 only for spawned peers; present so the rule is testable alone.
			RefusedNotSpawned,

			// The target is not a player character.
			//
			// Legacy supports CROW_MOB and CROW_NPC (GLContrlPcMsg.h:767 defaults
			// to CROW_MOB). The modern world has NO NPC or mob entity at all, so a
			// mob request is refused here rather than silently resolving to nobody.
			RefusedUnsupportedTargetCrow,

			// `dwTarID` is zero. Legacy's `GetTarget` would simply fail to find it;
			// naming the case is what makes "no id" distinguishable from "an id that
			// is not here".
			RefusedNoTargetId,

			// No live target carries that id. Legacy's `GetTarget(...)` returning
			// NULL and the `E_FAIL` at GLCharMsg.cpp:340.
			RefusedUnknownTarget,

			// `fDist > wAttackAbleDis`. The ONLY refusal that is announced on the
			// wire, matching GLCharMsg.cpp:352-363: a 3041 to the attacker and a
			// 3042 to everyone else.
			//
			// Every other refusal is SILENT, exactly as legacy is: an unknown target
			// produces no packet at all, so on the wire it is indistinguishable from
			// "the server ignored it". That asymmetry is legacy's, not an omission
			// here, and inventing a rejection packet would be a new message.
			RefusedOutOfRange,
		};

		const char* ToString(AttackOutcome outcome) noexcept;

		// A 3036, expressed in the types the rule works in.
		//
		// `targetId` is a gaeaId: for CROW_PC on the Field, legacy reaches the
		// target through GLGaeaServer::GetChar (GLCharMsg.cpp:380) and
		// DamageProc reaches the same record the same way (GLChar.cpp:2495).
		struct AttackRequest
		{
			WireU32 targetCrow = Network::Attack::kCrowPc;
			WireU32 targetId   = 0;
			WireU32 aniSel     = 0;
			WireU32 flags      = 0;
		};

		// What the rule could learn about the target.
		//
		// The rule does NOT resolve targets itself - it is handed the answer, so it
		// stays a pure function and the Field role keeps ownership of who is
		// connected. `exists` false is "no live peer carries this gaeaId".
		struct TargetView
		{
			bool     exists = false;
			Vector3  position{};
		};

		struct AttackResult
		{
			AttackOutcome outcome = AttackOutcome::RefusedUnknownTarget;

			// Whether a 3037 goes out. True only for Accepted.
			bool accepted = false;

			// Whether this refusal is the ANNOUNCED kind - 3041 to the attacker and
			// 3042 to everyone else.
			//
			// Only RefusedOutOfRange is true. Legacy announces exactly one branch,
			// and every other refusal is silent on the wire.
			bool announced = false;

			// The attacker's own gaeaId, echoed into the result so the caller does
			// not have to thread it separately. The broadcast's `dwGaeaID` is always
			// the ATTACKER's id, never the target's.
			WireU32 attackerGaeaId = 0;

			// The target as named by the request, carried so the caller can build a
			// 3041/3042 without re-reading the request. Never substituted.
			WireU32 targetCrow = Network::Attack::kCrowPc;
			WireU32 targetId   = 0;

			// `dwAniSel`, echoed for the 3037. Never interpreted.
			WireU32 aniSel = 0;

			// |attackerPosition - targetPosition|, the legacy `fDist`. Zero when
			// there is no target to measure against.
			float distance = 0.0f;

			// The distance limit the comparison used, so a test can assert on the
			// boundary rather than on a private constant.
			float allowedDistance = 0.0f;

			// Why a request was refused, in one line. Empty on success.
			std::string detail;
		};

		// Decides one 3036. Pure: no I/O, no state, no threads.
		class AttackService
		{
		public:
			// The prototype range limit: the placeholder plus the verified slack.
			static float AllowedDistance() noexcept;

			// Evaluates `request` for an attacker at `attackerPosition`.
			//
			// `attackerGaeaId` is the attacker's own authorized id. `target` is what
			// the caller resolved; this rule does not look anything up.
			AttackResult Evaluate(WireU32 attackerGaeaId, bool attackerSpawned,
			                      const Vector3& attackerPosition,
			                      const AttackRequest& request,
			                      const TargetView& target) const;
		};
} // namespace Modern::Server::World