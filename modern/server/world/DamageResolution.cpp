#include "DamageResolution.h"

#include "combat/HitCalculator.h"
#include "combat/PhysicalDamageCalculator.h"

#include <cmath>

namespace Modern::Server::World
{
	const char* ToString(DamageOutcome outcome) noexcept
	{
		switch (outcome)
		{
		case DamageOutcome::Avoided: return "Avoided";
		case DamageOutcome::Hit:     return "Hit";
		case DamageOutcome::Refused: return "Refused";
		}
		return "Unrecognised";
	}

	namespace
	{
		// A roll is usable only if it is a real number in [0,1].
		//
		// NaN is the reason this exists. Legacy's comparison is
		// `nHitRate >= (RANDOM_POS * 100)` (GLogixExPC.cpp:1327); with a NaN roll
		// that is false, which would quietly turn a bad roll into an AVOIDED
		// rather than a refusal - so a broken caller would look like a working
		// game with an unlucky hit rate. Refusing at the boundary makes the fault
		// visible instead.
		bool IsUsableRoll(float value) noexcept
		{
			return std::isfinite(value) && value >= 0.0f && value <= 1.0f;
		}
	}

	DamageResult DamageResolution::Resolve(bool attackerPresent, bool targetPresent,
	                                       const DamageInput& input)
	{
		DamageResult result;

		// Identity first, so a caller with no attacker learns that rather than
		// learning that its roll was bad.
		if (!attackerPresent)
		{
			result.outcome = DamageOutcome::Refused;
			result.detail  = "attacker is not in the world";
			return result;
		}

		if (!targetPresent)
		{
			result.outcome = DamageOutcome::Refused;
			result.detail  = "target is not in the world";
			return result;
		}

		// A gaeaId of 0 is never a real identity. Checked here as well as in the
		// peer registry, so the rule refuses it even if a caller hands one in.
		if (input.attackerGaeaId == 0)
		{
			result.outcome = DamageOutcome::Refused;
			result.detail  = "attacker has no gaeaId";
			return result;
		}

		if (input.targetGaeaId == 0)
		{
			result.outcome = DamageOutcome::Refused;
			result.detail  = "target has no gaeaId";
			return result;
		}

		if (!IsUsableRoll(input.hitRoll) || !IsUsableRoll(input.damageRoll) ||
		    !IsUsableRoll(input.criticalRoll) || !IsUsableRoll(input.crushingRoll) ||
		    !IsUsableRoll(input.reflectionRoll))
		{
			result.outcome = DamageOutcome::Refused;
			result.detail  = "a roll is not a number in [0,1]";
			return result;
		}

		// A highDamage below lowDamage would make RandomDamageRange interpolate
		// backwards. Legacy's GDAMAGE pair is built with a low clamp, so refuse
		// rather than emit a figure derived from an inverted range.
		if (input.stats.highDamage < input.stats.lowDamage)
		{
			result.outcome = DamageOutcome::Refused;
			result.detail  = "damage range is inverted";
			return result;
		}

		const Combat::CombatConstants constants;

		// ---- the hit roll ---------------------------------------------------
		//
		// GLChar::PreStrikeProc rolls once per strike through CHECKHIT
		// (GLChar.cpp:2416) and caches the result in the SSTRIKE array; the
		// branch is taken later, at AttackProcess (:2848 hit / :2852 avoid).
		// Both halves live here, so a roll and its consequence cannot drift.
		Combat::HitInput hitInput;
		hitInput.attackerHit = input.stats.hit;
		hitInput.targetAvoid = input.stats.avoid;
		hitInput.brightnessFB = Engine::GameBrightFB::Aver;
		hitInput.lowSP        = false;
		hitInput.hitRoll      = input.hitRoll;

		const Combat::HitResult hit = Combat::CalculateHit(hitInput, constants);
		result.hitRate = hit.hitRate;

		if (!hit.hit)
		{
			// Avoided. 002i already routes 3041/3042 for a refusal, and legacy's
			// AvoidProc sends exactly those - so the caller reuses that path
			// rather than this emitting anything itself.
			result.outcome = DamageOutcome::Avoided;
			return result;
		}

		// ---- the damage figure ----------------------------------------------
		Combat::PhysicalDamageInput damageInput;
		damageInput.physicalDamage.low  = input.stats.lowDamage;
		damageInput.physicalDamage.high = input.stats.highDamage;
		damageInput.meleePower          = input.stats.meleePower;
		damageInput.shootPower          = input.stats.shootPower;
		damageInput.attackType          = Combat::AttackType::Melee;
		damageInput.factDamage          = 0;
		damageInput.damageRate          = input.stats.damageRate;
		damageInput.stateDamage         = input.stats.stateDamage;
		damageInput.damageReduce        = input.stats.damageReduce;
		damageInput.damageReflection    = 0.0f;
		damageInput.damageReflectionRate = 0.0f;
		damageInput.defense             = input.stats.defense;
		damageInput.defenseBody         = input.stats.defenseBody;
		damageInput.defenseItem         = input.stats.defenseItem;
		damageInput.level               = input.stats.level;
		damageInput.resistElement       = input.stats.resistElement;

		// ---- the HP figures the critical-rate kernel divides by --------------
		//
		// `CriticalBaseRate` computes `(currentHP*100)/maxHP` and only floors the
		// RESULT at 10 (GameCharacterCalculations.cpp:583, legacy
		// GLogixExPC.cpp:1603-1613) - the division happens first, so a maxHP of 0
		// is a divide by zero and not a "very high crit rate". These are
		// PROTOTYPE values for the same reason the damage range is: the modern
		// Field path has no derived HP yet. They are non-zero so the kernel is
		// exercised rather than faulting.
		damageInput.attackerLevel       = 1;
		damageInput.attackerMaxHP       = 100;
		damageInput.attackerCurrentHP   = 100;
		damageInput.attackerCriticalBonus = 0;
		damageInput.attackerCrushingBonus = 0;
		damageInput.targetLevel         = 1;
		damageInput.targetMaxHP         = 100;

		damageInput.brightnessFB        = Engine::GameBrightFB::Aver;
		damageInput.weatherElementPower = 1.0f;
		damageInput.targetResistElement = 0;
		damageInput.fDamageReduce       = 0.0f;
		damageInput.fDamageReflection   = 0.0f;
		damageInput.fDamageReflectionRate = 0.0f;

		damageInput.lowSP               = false;
		damageInput.stateDamageMultiplier = 1.0f;
		damageInput.requiredSP          = 0;
		damageInput.isPK                = false;

		damageInput.hitRoll        = input.hitRoll;
		damageInput.damageRoll     = input.damageRoll;
		damageInput.criticalRoll   = input.criticalRoll;
		damageInput.crushingRoll   = input.crushingRoll;
		damageInput.reflectionRoll = input.reflectionRoll;

		// VERTICAL-025: `skillCast` is legacy's `if (pSkill)` discriminator
		// (GLogixExPC.cpp:1417). This is a BASIC attack, so it is false, and the
		// whole physical-resistance block stays as legacy leaves it for basic
		// attacks.
		damageInput.skillCast = false;

		const Combat::DamageResult damage =
		    Combat::CalculatePhysicalDamage(damageInput, constants);

		result.outcome          = DamageOutcome::Hit;
		result.requestedDamage  = damage.damage;
		result.damageFlag       = damage.critical ? Network::Attack::kDamageTypeCritical
		                        : Network::Attack::kDamageTypeNone;

		// `appliedDamage` is left at kUnapplied on purpose. The caller owns HP and
		// is the only thing that can say what was really removed; a caller that
		// forgets must not accidentally ship `requestedDamage`.
		return result;
	}
} // namespace Modern::Server::World