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

		// ---- the runtime context must be authoritative ------------------------
		//
		// Legacy reads the attacker's level and HP and the target's level on every
		// hit (GETHP/GETMAXHP/GETLEVEL at GLogixExPC.cpp:1387-1388, and the
		// critical kernel at :1615-1619). There is no legacy path that computes
		// those, so a caller that cannot supply them is not in a position to
		// resolve an attack at all - and substituting a level here would produce a
		// number the caller could not explain.
		//
		// Checked BEFORE any roll, so a caller with no context learns that rather
		// than learning that its attack missed.
		if (!input.context.authoritative)
		{
			result.outcome = DamageOutcome::Refused;
			result.detail  = "the runtime context is not authoritative: no "
			                 "attacker/target level and HP were supplied";
			return result;
		}

		// A maxHP of zero divides by zero inside CriticalBaseRate. The resource
		// owner guarantees a positive maximum for a registered character, so this
		// is a caller fault and is reported as one.
		if (input.context.attackerMaxHP == 0)
		{
			result.outcome = DamageOutcome::Refused;
			result.detail  = "attacker maxHP is zero";
			return result;
		}
		if (input.context.attackerCurrentHP > input.context.attackerMaxHP)
		{
			result.outcome = DamageOutcome::Refused;
			result.detail  = "attacker currentHP exceeds maxHP";
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

		// ---- the live per-attack context -------------------------------------
		//
		// 002L-C: these were hardcoded (level 1, HP 100/100) and are now the
		// caller's authoritative values. `CriticalBaseRate` divides by maxHP, so
		// these are the values that make the critical rate character-specific
		// rather than a constant.
		damageInput.attackerLevel       = input.context.attackerLevel;
		damageInput.attackerMaxHP       = input.context.attackerMaxHP;
		damageInput.attackerCurrentHP   = input.context.attackerCurrentHP;
		damageInput.targetLevel         = input.context.targetLevel;
		damageInput.attackerCriticalBonus = 0;
		damageInput.attackerCrushingBonus = 0;

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