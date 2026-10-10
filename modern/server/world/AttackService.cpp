#include "AttackService.h"

#include "AttackProtocol.h"

#include <cmath>

namespace Modern::Server::World{
	const char* ToString(AttackOutcome outcome) noexcept
	{
		switch (outcome)
		{
		case AttackOutcome::Accepted:                  return "Accepted";
		case AttackOutcome::RefusedNotSpawned:         return "RefusedNotSpawned";
		case AttackOutcome::RefusedUnsupportedTargetCrow: return "RefusedUnsupportedTargetCrow";
		case AttackOutcome::RefusedNoTargetId:         return "RefusedNoTargetId";
		case AttackOutcome::RefusedUnknownTarget:      return "RefusedUnknownTarget";
		case AttackOutcome::RefusedOutOfRange:         return "RefusedOutOfRange";
		}
		return "Unrecognised";
	}

	float AttackService::AllowedDistance() noexcept
	{
		// The legacy formula, with its item-derived terms collapsed into the one
		// prototype constant:
		//
		//   wAttackAbleDis = targetBodyRadius + GETBODYRADIUS() + GETATTACKRANGE()
		//                   + 2 (+ GETSUM_TARRANGE())
		//                   + 7
		//
		// The `+2` and the `+7` are both fixed in legacy and the `+7` is carried
		// explicitly as kLegacyRangeSlackUnits; the four item-derived terms are the
		// prototype placeholder. See the header for why that substitution is
		// recorded rather than silently made.
		//
		// 002M: a resolved weapon does NOT arrive here. It goes through
		// `AllowedDistanceFor`, which computes the legacy sum from the weapon's
		// actual `wAttRange`. This entry point remains for the no-item-data case.
		return kPrototypeAttackableDistanceUnits + kLegacyRangeSlackUnits;
	}

	float AttackService::AllowedDistanceFor(const TargetView& target,
	                                        const WeaponRangeView& weapon) noexcept
	{
		// GLCharMsg.cpp:343-347, term by term:
		//
		//   wAttackRange   = pTARGET->GetBodyRadius()
		//                  + GETBODYRADIUS()
		//                  + GETATTACKRANGE()
		//                  + 2
		//   wAttackAbleDis = wAttackRange + 7
		//
		// `ISLONGRANGE_ARMS()` adds `GETSUM_TARRANGE()`, which is the passive and
		// skill target-range bonus. This server has neither, so it is omitted
		// rather than added as a recovered zero - the day one exists it has to
		// come from its own source.
		if (!weapon.hasWeapon)
		{
			// No weapon resolved. Two cases, deliberately distinguished:
			//
			//   * no item data at all, so no weapon can be resolved. The
			//     prototype stands in for the whole sum.
			//   * a weapon IS resolved and simply declares a reach, which may be
			//     0. That is a real value and is computed, not replaced.
			//
			// They are told apart by `hasWeapon`, which only the caller can set,
			// because only the caller knows whether an item table was consulted.
			return AllowedDistance();
		}

		float range = target.bodyRadius;
		range += weapon.attackerBodyRadius;
		range += weapon.attackRange;
		range += kLegacyRangeFixedTerm;
		range += kLegacyRangeSlackUnits;

		// A NaN or negative reach cannot come from a real item (the loader refuses
		// it), but the comparison is written so a NaN falls on the SAFE side: a
		// non-finite limit refuses every attack rather than accepting them all.
		if (!(range > 0.0f))
		{
			return 0.0f;
		}
		return range;
	}

	AttackResult AttackService::Evaluate(WireU32 attackerGaeaId, bool attackerSpawned,
	                                     const Vector3& attackerPosition,
	                                     const AttackRequest& request,
	                                     const TargetView& target,
	                                     const WeaponRangeView& weapon) const
	{
		AttackResult result;
		result.attackerGaeaId = attackerGaeaId;
		result.targetCrow     = request.targetCrow;
		result.targetId       = request.targetId;
		result.aniSel         = request.aniSel;

		// Every refusal below returns through this one place, so `accepted` and
		// `announced` cannot be left inconsistently set on an early exit. Both
		// default to false and only Accepted / RefusedOutOfRange ever change them.
		const auto refuse = [&result](AttackOutcome outcome, const char* detail) {
			result.outcome = outcome;
			result.accepted = false;
			result.detail   = detail;
			return result;
		};

		// Not a legacy step: the Field dispatch already routes 3036 only for a
		// spawned peer, so this cannot be reached from the network. It is checked
		// because a rule that silently assumes a precondition is a rule that breaks
		// the day the caller stops guaranteeing it.
		if (!attackerSpawned)
		{
			return refuse(AttackOutcome::RefusedNotSpawned,
			              "attacker has not spawned");
		}

		// Legacy's target lookup is GetTarget(landMan, {emCrow, dwID}), so the crowd
		// type is part of the identity. The modern world resolves only CROW_PC,
		// because a spawned player character is the only entity type that exists
		// today - there is no NPC or mob. Refused here, and NOT by the codec: the
		// wire format is legacy's and does not get to second-guess it.
		if (request.targetCrow != Network::Attack::kCrowPc)
		{
			return refuse(AttackOutcome::RefusedUnsupportedTargetCrow,
			              "target is not a player character");
		}

		// Legacy reaches this as `GetTarget` returning NULL. Zero is separated out
		// because "the client asked for nobody" and "the client asked for someone
		// who is not here" are different faults that happen to be silent together.
		if (request.targetId == 0)
		{
			return refuse(AttackOutcome::RefusedNoTargetId, "target id is zero");
		}

		// GLCharMsg.cpp:339-340: `GLACTOR* pTARGET = GetTarget(...); if (!pTARGET)
		// return E_FAIL;`
		if (!target.exists)
		{
			return refuse(AttackOutcome::RefusedUnknownTarget,
			              "no live character carries that id");
		}

		// GLCharMsg.cpp:343-344:
		//   D3DXVECTOR3 vTarPos = pTARGET->GetPosition();
		//   float fDist = D3DXVECTORLength(&D3DXVECTOR3(m_vPos - vTarPos));
		//
		// The attacker's position is the SERVER's, taken from the movement runtime -
		// never a client claim. There is no client-supplied position on a 3036 to
		// trust in the first place, which is why this check needs no anti-teleport
		// companion the way 3034's 60-unit rule does.
		const Vector3 delta{ attackerPosition.x - target.position.x,
		                     attackerPosition.y - target.position.y,
		                     attackerPosition.z - target.position.z };
		const float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y +
		                                 delta.z * delta.z);

		result.distance        = distance;
		result.allowedDistance = AllowedDistanceFor(target, weapon);
		result.usedWeaponRange = weapon.hasWeapon;

		// The one refusal legacy ANNOUNCES (GLCharMsg.cpp:349-366).
		//
		// A non-finite distance cannot reach here from real positions - the movement
		// runtime's positions come from navigation, which refuses non-finite input -
		// but the comparison is written so that a NaN falls on the SAFE side. With
		// `!(distance <= limit)` rather than `distance > limit`, a NaN is refused
		// instead of comparing false and being accepted, which is the same trap
		// GotoProtocol's 60-unit rule has to guard against for vCurPos.
		if (!(distance <= result.allowedDistance))
		{
			result.outcome   = AttackOutcome::RefusedOutOfRange;
			result.accepted   = false;
			result.announced  = true;
			result.detail     = "target is out of attack range";
			return result;
		}

		result.outcome  = AttackOutcome::Accepted;
		result.accepted = true;
		return result;
	}
} // namespace Modern::Server::World