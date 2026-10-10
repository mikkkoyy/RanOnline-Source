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
		// Added to every range this rule computes, whether the range came from a
		// resolved weapon or from the prototype constant, because the slack is
		// legacy's and it is not conditional on anything.
		inline constexpr float kLegacyRangeSlackUnits = 7.0f;

		// The attacker's and a PC target's body radius:
		// `GLCONST_CHAR::wBODYRADIUS` = 4 (GLogicData.cpp:245).
		inline constexpr float kPcBodyRadiusUnits = 4.0f;

		// The fixed legacy term in the range rule: `... + GETATTACKRANGE() + 2`
		// (GLCharMsg.cpp:345).
		inline constexpr float kLegacyRangeFixedTerm = 2.0f;

		// `GETATTACKRANGE()` when the right hand is empty:
		// `GLCONST_CHAR::wMAXATRANGE_SHORT` = 2 (GLogicData.cpp:237), applied by
		// GLogixExPC.cpp:416-417.
		//
		// Recovered for an unarmed attacker, which makes the unarmed case exact
		// too: 4 + 4 + 2 + 2 + 7 = 19 units. It is still a FALLBACK in the sense
		// that the server cannot tell an unarmed character from one whose weapon
		// has not been resolved - see `WeaponRangeView::hasWeapon`.
		inline constexpr float kUnarmedAttackRangeUnits = 2.0f;

		// PROTOTYPE. Stands in for the whole item-derived sum when no weapon has
		// been resolved, i.e. when there is no item data to read a range from.
		//
		// NOT legacy-verified and NOT derived from item data. It is kept only
		// because a server with no item table cannot compute the legacy sum, and
		// the options are "no range check at all" or "one clearly-labelled
		// placeholder" - and a missing check would let a client attack from
		// anywhere, which is a worse defect than a placeholder constant.
		//
		// A resolved weapon does NOT use this: that is the whole of 002M's change
		// to this rule. See `AttackService::AllowedDistanceFor`.
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

			// The target's body radius, legacy `pTARGET->GetBodyRadius()`.
			//
			// For a PC this is `GLCONST_CHAR::wBODYRADIUS` = 4 (GLogicData.cpp:245),
			// which is the default. It is a parameter rather than a constant
			// because the value is the TARGET's, and a crow target's comes from its
			// crow data (`m_sAction.m_wBodyRadius`) rather than from the character
			// table. PC-vs-PC is therefore 4 + 4, which is the case this server
			// fights today.
			float bodyRadius = 4.0f;
		};

		// What the attacker is holding, for the range rule.
		//
		// Legacy's rule is (GLCharMsg.cpp:343-347):
		//
		//     wAttackRange   = pTARGET->GetBodyRadius() + GETBODYRADIUS()
		//                      + GETATTACKRANGE() + 2
		//     if ( ISLONGRANGE_ARMS() )  wAttackRange += GETSUM_TARRANGE()
		//     wAttackAbleDis = wAttackRange + 7
		//
		// `GETATTACKRANGE()` is `m_wATTRANGE`, which is the equipped weapon's
		// `wAttRange` - or `GLCONST_CHAR::wMAXATRANGE_SHORT` (= 2) when the right
		// hand is empty (GLogixExPC.cpp:411-418).
		struct WeaponRangeView
		{
			// Whether a weapon was resolved and its range read. False leaves the
			// prototype constant in charge, which is the documented fallback for
			// "no item data, or no weapon in hand".
			bool  hasWeapon = false;

			// The weapon's `wAttRange`, in world units. Meaningful only when
			// `hasWeapon` is true; 0 is a declared zero and is kept, because a
			// weapon really can declare a zero reach.
			float attackRange = 0.0f;

			// The attacker's own body radius, `GETBODYRADIUS()` = 4 for a PC.
			float attackerBodyRadius = 4.0f;
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

			// Which limit the comparison used, and why. A test that asks "did the
			// rule use the weapon's range or the prototype" can answer it without
			// re-deriving the number.
			bool usedWeaponRange = false;

			// Why a request was refused, in one line. Empty on success.
			std::string detail;
		};

		// Decides one 3036. Pure: no I/O, no state, no threads.
		class AttackService
		{
		public:
			// The prototype range limit: the placeholder plus the verified slack.
			//
			// This is what is used when no weapon is resolved. It is kept because
			// existing tests and the existing test seams use it, and because a
			// server without item data has nothing better.
			static float AllowedDistance() noexcept;

			// The legacy rule, computed from what the caller resolved.
			//
			//     targetBodyRadius + attackerBodyRadius + weaponRange + 2 + 7
			//
			// `GetSUM_TARRANGE()` is deliberately NOT added: it is the long-range
			// target-range bonus from passives and skills, and this server has
			// neither, so adding a recovered zero would be the same as omitting it -
			// and the day one arrives it has to come from its own source rather
			// than from a default here.
			static float AllowedDistanceFor(const TargetView& target,
			                                const WeaponRangeView& weapon) noexcept;

			// Evaluates `request` for an attacker at `attackerPosition`.
			//
			// `attackerGaeaId` is the attacker's own authorized id. `target` is what
			// the caller resolved; this rule does not look anything up. `weapon` is
			// the caller's resolved weapon range, or the default for "none
			// resolved", which keeps the prototype constant in charge.
			AttackResult Evaluate(WireU32 attackerGaeaId, bool attackerSpawned,
			                      const Vector3& attackerPosition,
			                      const AttackRequest& request,
			                      const TargetView& target,
			                      const WeaponRangeView& weapon = {}) const;
		};
} // namespace Modern::Server::World