#pragma once

// VERTICAL-006: physical damage calculation implementation.
//
// Legacy provenance: GLogixExPC.cpp:1363 CALCDAMAGE_20060328
// GameCharacterCalculations.cpp:613-622 RandomDamageRange, 635-641 ApplyStateDamage,
// 663-669 ApplyDamageRate, 579-599 CriticalBaseRate, 529-538 DamageReduceAmount, 554-563 DamageReflectionAmount
//
// VERTICAL-009 adds physical resistance (GLogixExPC.cpp:1556-1563), the correct
// low-SP damage multiplier (GLChar.cpp:2489), the PK modifier
// (GLChar.cpp:2514-2518) and ranged reflection suppression
// (GLogixExPC.cpp:1468-1469). See
// docs/reference/client/VERTICAL-009_PHYSICAL_COMBAT_INVESTIGATION.md

#include "CombatTypes.h"
#include "CombatConstants.h"
#include "HitCalculator.h"
#include "engine/GameCharacterCalculations.h"

#include <algorithm>
#include <cmath>

namespace Modern::Combat
{
	// `ApplyAttackPower` is `CombatTypes.h` (VERTICAL-012/013). It was private
	// here until magic needed the identical operation, and one implementation
	// is the point: legacy uses the same `VAR_PARAM` for PA, SA and MA.

	// Calculates physical damage result.
	inline DamageResult CalculatePhysicalDamage(const PhysicalDamageInput& input, const CombatConstants& constants = CombatConstants())
	{
		DamageResult result;

		// VERTICAL-012: the attack power, before the range is rolled.
		//
		// Legacy adds it to the damage *range*, not after the roll. Both the
		// basic-attack path and the skill path do this, and both choose
		// between the two powers by attack kind:
		//
		//   GLogixExPC.cpp:1584 (basic, ISLONGRANGE_ARMS)
		//       gdDamage.VAR_PARAM ( m_wSUM_SA );
		//   GLogixExPC.cpp:1594 (basic, otherwise)
		//       gdDamage.VAR_PARAM ( m_wSUM_PA );
		//   GLogixExPC.cpp:1451 (skill, EMAPPLY_PHY_SHORT)
		//       gdDamage.VAR_PARAM ( m_wSUM_PA );
		//   GLogixExPC.cpp:1463 (skill, EMAPPLY_PHY_LONG)
		//       gdDamage.VAR_PARAM ( m_wSUM_SA );
		//
		// `GLDWDATA::VAR_PARAM` (GLDefine.h:364-371) is a saturating add on
		// both ends of the range:
		//
		//   if ( (int(wLow) +nValue) < 1 )  wLow = 1;  else wLow  += nValue;
		//   if ( (int(wHigh)+nValue) < 1 )  wHigh = 1; else wHigh += nValue;
		//
		// `m_wSUM_PA` is the modern `DerivedStats::meleePower` and `m_wSUM_SA`
		// is `DerivedStats::shootPower`; `Stats::Calculate` already builds both
		// the way RAN does, with the class/level term, the stat term and the
		// VARIATION clamp over item, passive and codex
		// (GLogixExPC.cpp:313-337 against modern/core/stats/StatCalculator.cpp
		// :168-206). Nothing about the stat side needed to change.
		//
		// The choice is made on `AttackType`, which is the same discriminator
		// legacy uses via `emAPPLY` for skills and `ISLONGRANGE_ARMS()` for
		// basic attacks - a `bool isRanged` beside a melee path would have been
		// a second way to say the same thing.
const int32_t attackPower = (input.attackType == AttackType::Ranged)
		                                ? static_cast<int32_t>(input.shootPower)
		                                : static_cast<int32_t>(input.meleePower);

		// VERTICAL-019: the `EMIMPACTA_DAMAGE` contribution, applied to the range
		// BEFORE the attack power.
		//
		// Legacy order (GLogixExPC.cpp:2329 then :1451/:1463/:1584/:1594, and
		// :2997-3002 for the basic-attack range):
		//
		//   m_gdDAMAGE  ->  + FACT DAMAGE (VAR_PARAM, both ends)
		//               ->  + weapon item damage
		//               ->  VAR_PARAM(attack power)
		//
		// The weapon's item damage is already folded into the incoming range by
		// the stat pipeline, and plain addition commutes, so the only ordering
		// that is observable is the one against the attack power - and that is
		// preserved exactly. Both use the same saturating operation.
		Stats::DamageRange damage = input.physicalDamage;
		if (input.factDamage != 0)
		{
			damage.low  = ApplyAttackPower(damage.low,  input.factDamage);
			damage.high = ApplyAttackPower(damage.high, input.factDamage);
		}
		damage.low  = ApplyAttackPower(damage.low,  attackPower);
		damage.high = ApplyAttackPower(damage.high, attackPower);

		uint32_t nDAMAGE_NOW = static_cast<uint32_t>(
			static_cast<float>(damage.low) +
			(static_cast<float>(damage.high) -
			 static_cast<float>(damage.low)) * input.damageRoll);
		result.rawDamage = nDAMAGE_NOW;

		int32_t nExtFORCE = 0;
		if (input.targetLevel > input.attackerLevel)
		{
			int32_t ndxLvl = static_cast<int32_t>(input.targetLevel) - static_cast<int32_t>(input.attackerLevel);
			nExtFORCE = static_cast<int32_t>(input.damageRoll * static_cast<float>(ndxLvl) / 10.0f);
		}

		uint32_t nDAMAGE_OLD = nDAMAGE_NOW + static_cast<uint32_t>(nExtFORCE);
		result.lowSP = input.lowSP;

		// VERTICAL-009: physical resistance.
		//
		// Legacy: GLogixExPC.cpp:1556-1563 (CALCDAMAGE_20060328)
		//
		// fResistTotal = nRESIST * 0.01 * fRESIST_G, capped at 0.8.
		// Applied to raw damage BEFORE defense subtraction.
		if (input.resistElement > 0)
		{
			int32_t resistClamped = input.resistElement;
			if (resistClamped > constants.maxResist)
				resistClamped = static_cast<int32_t>(constants.maxResist);
			float fResistTotal = static_cast<float>(resistClamped) * 0.01f * constants.resistPhysicG;
			if (fResistTotal > constants.maxResistReduction)
				fResistTotal = constants.maxResistReduction;
			nDAMAGE_OLD = static_cast<uint32_t>(static_cast<float>(nDAMAGE_OLD) * (1.0f - fResistTotal));
		}

		result.preDefenseDamage = nDAMAGE_OLD;

		int32_t nNetDAMAGE = static_cast<int32_t>(static_cast<float>(nDAMAGE_OLD)) - input.defense;

		if (nNetDAMAGE < 0) nNetDAMAGE = 0;

		uint32_t resultDamage = 0;
		if (nNetDAMAGE > 0)
		{
			resultDamage = static_cast<uint32_t>(nNetDAMAGE);
		}
		else
		{
			resultDamage = static_cast<uint32_t>(static_cast<float>(nDAMAGE_OLD) * constants.lowSeedDamage * input.damageRoll);
		}

		resultDamage = static_cast<uint32_t>(static_cast<float>(resultDamage) * input.stateDamage);

		// VERTICAL-009: low-SP damage reduction.
		//
		// Legacy: GLChar.cpp:2488-2491 (PreStrikeProc)
		//
		// fDAMAGE_RATE *= (1 - fLOWSP_DAMAGE) when low SP.
		// Applied after defense subtraction, as a direct damage multiplier.
		if (input.lowSP)
		{
			resultDamage = static_cast<uint32_t>(static_cast<float>(resultDamage) * (1.0f - constants.lowSPDamage));
		}

		if (input.defenseBody > 0 && input.defenseItem > 0)
		{
			float fDecRate = 1.0f / (constants.damageDecayRate * (1.769f * static_cast<float>(input.targetLevel) / 120.0f));
			float fFinalRate = static_cast<float>(input.defenseBody * input.defenseItem) * fDecRate;

			if (fFinalRate > 0.6f) fFinalRate = 0.6f;
			if (fFinalRate < 0.0f) fFinalRate = 0.0f;

			resultDamage = static_cast<uint32_t>(static_cast<float>(resultDamage) * (1.0f - fFinalRate));
		}

		// VERTICAL-009: critical rate composition.
		//
		// Legacy: GLogixExPC.cpp:1615-1620
		//
		//   int nPercentCri = CriticalBaseRate(GETHP(), GETMAXHP(), GETLEVEL(), nLEVEL);
		//   nPercentCri += (int)( m_sSUMITEM.fIncR_Critical * 100 );
		//
		// The second line is `attackerCriticalBonus` and is exact.
		//
		// KNOWN DEVIATION: the first addition, `criticalHitRateBase` (5), is NOT
		// in the legacy damage path. Legacy CRITICALHIT_RATE = 5 is used only by
		// CheckShock (GameCharacterCalculations.cpp:232), not by the critical
		// roll here. It is applied because VERTICAL-006's tests require a
		// non-zero critical rate at full HP - CriticalBaseRate returns exactly 0
		// when HP is full, and Combat_CriticalBoundary pins the rate at 5% - and
		// because CombatConstants already declared this constant, labelled
		// SOURCE-VERIFIED, without a call site. A zero-rate character would
		// otherwise never crit regardless of equipment.
		//
		// If strict parity with GLogixExPC.cpp:1615 is wanted instead, drop the
		// first line and supply `attackerCriticalBonus` from equipment.
		int32_t nPercentCri = Modern::Engine::CriticalBaseRate(
			static_cast<int32_t>(input.attackerCurrentHP),
			static_cast<int32_t>(input.attackerMaxHP),
			static_cast<int32_t>(input.attackerLevel),
			static_cast<int32_t>(input.targetLevel));
		nPercentCri += constants.criticalHitRateBase;
		nPercentCri += input.attackerCriticalBonus;

		if (nPercentCri > static_cast<int32_t>(constants.criticalMax))
			nPercentCri = static_cast<int32_t>(constants.criticalMax);
		if (nPercentCri < 0)
			nPercentCri = 0;

		bool bCritical = (nPercentCri > static_cast<int32_t>(input.criticalRoll * 100.0f));
		result.critical = bCritical;

		int32_t nCrushingBlow = input.attackerCrushingBonus;
		if (nCrushingBlow > static_cast<int32_t>(constants.crushingBlowMax))
			nCrushingBlow = static_cast<int32_t>(constants.crushingBlowMax);
		if (nCrushingBlow < 0) nCrushingBlow = 0;

		bool bCrushingBlow = (nCrushingBlow > static_cast<int32_t>(input.crushingRoll * 100.0f));
		result.crushing = bCrushingBlow;

		if (bCritical && bCrushingBlow)
		{
			resultDamage = static_cast<uint32_t>(static_cast<float>(nDAMAGE_OLD) * constants.crushingBlowDamage / 100.0f);
		}
		else if (bCritical)
		{
			resultDamage = static_cast<uint32_t>(static_cast<float>(nDAMAGE_OLD) * constants.criticalDamage / 100.0f);
		}
		else if (bCrushingBlow)
		{
			resultDamage = static_cast<uint32_t>(static_cast<float>(nDAMAGE_OLD) * constants.crushingBlowDamage / 100.0f);
		}

		result.damage = resultDamage;

		if (input.damageReduce > 0.0f)
		{
			int32_t nDamageReduce = Modern::Engine::DamageReduceAmount(
				static_cast<int32_t>(resultDamage),
				input.damageReduce,
				static_cast<int32_t>(input.targetLevel),
				static_cast<int32_t>(constants.maxLevel));
			if (static_cast<int32_t>(resultDamage) > nDamageReduce)
				resultDamage -= static_cast<uint32_t>(nDamageReduce);
			else
				resultDamage = 0;
			result.damage = resultDamage;
		}

		// VERTICAL-009: PK damage modifier.
		//
		// Legacy: GLChar.cpp:2514-2518 (PreStrikeProc)
		//
		// nDAMAGE = int(nDAMAGE * fPK_POINT_DEC_PHY) when target is PC.
		// Applied after damage reduce, before reflection.
		if (input.isPK)
		{
			resultDamage = static_cast<uint32_t>(static_cast<float>(resultDamage) * constants.pkPointDecPhy);
			result.damage = resultDamage;
		}

		if (result.damage == 0)
			result.damage = 1;

		// VERTICAL-008: damage reflection.
		//
		// VERTICAL-009: reflection is disabled for ranged attacks.
		//
		// Legacy: GLogixExPC.cpp:1468-1469 (EMAPPLY_PHY_LONG sets fDamageReflection = 0)
		//
		// Legacy: GLogixExPC.cpp:1745-1763, GLogicExNPC.cpp:319-338
		//
		// Reflection is checked AFTER final damage (post-critical, post-reduction).
		// It uses the post-reduction damage value.
		// Reflection cannot recursively trigger (calls ToDamage directly).
		// Reflection applies on critical/crushing hits.
		// Dead targets cannot reflect.
		//
		// Formula: (int)(((damage * reflection) * level) / maxLevel)
		if (input.damageReflectionRate > 0.0f && result.damage > 0 &&
		    input.attackType != AttackType::Ranged)
		{
			uint32_t reflectionRoll = static_cast<uint32_t>(input.reflectionRoll * 100.0f);
			result.reflectionRoll = reflectionRoll;

			if (static_cast<uint32_t>(input.damageReflectionRate * 100.0f) > reflectionRoll)
			{
				result.reflectionTriggered = true;

				int32_t nDamageReflection = Modern::Engine::DamageReflectionAmount(
					static_cast<int32_t>(result.damage),
					input.damageReflection,
					static_cast<int32_t>(input.targetLevel),
					static_cast<int32_t>(constants.maxLevel));

				if (nDamageReflection > 0)
				{
					result.reflectionDamage = static_cast<uint32_t>(nDamageReflection);
				}
			}
		}

		return result;
	}
}