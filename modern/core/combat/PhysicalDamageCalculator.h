// VERTICAL-006: physical damage calculation implementation.
//
// Legacy provenance: GLogixExPC.cpp:1363 CALCDAMAGE_20060328
// GameCharacterCalculations.cpp:613-622 RandomDamageRange, 635-641 ApplyStateDamage,
// 663-669 ApplyDamageRate, 579-599 CriticalBaseRate, 529-538 DamageReduceAmount, 554-563 DamageReflectionAmount

#include "CombatTypes.h"
#include "CombatConstants.h"
#include "HitCalculator.h"
#include "engine/GameCharacterCalculations.h"

#include <algorithm>
#include <cmath>

namespace Modern::Combat
{
	// Calculates physical damage result.
	DamageResult CalculatePhysicalDamage(const PhysicalDamageInput& input, const CombatConstants& constants = CombatConstants())
	{
		DamageResult result;

		DamageRange damage = input.physicalDamage;

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
		result.preDefenseDamage = nDAMAGE_OLD;

		float defenseUsed = 1.0f;
		if (input.lowSP)
		{
			defenseUsed = 1.0f - constants.lowSeedDamage;
		}

		int32_t nNetDAMAGE = static_cast<int32_t>(static_cast<float>(nDAMAGE_OLD) * defenseUsed) - input.defense;

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

		if (input.defenseBody > 0 && input.defenseItem > 0)
		{
			float fDecRate = 1.0f / (constants.damageDecayRate * (1.769f * static_cast<float>(input.targetLevel) / 120.0f));
			float fFinalRate = static_cast<float>(input.defenseBody * input.defenseItem) * fDecRate;

			if (fFinalRate > 0.6f) fFinalRate = 0.6f;
			if (fFinalRate < 0.0f) fFinalRate = 0.0f;

			resultDamage = static_cast<uint32_t>(static_cast<float>(resultDamage) * (1.0f - fFinalRate));
		}

		int32_t nPercentCri = Modern::Engine::CriticalBaseRate(
			static_cast<int32_t>(input.attackerCurrentHP),
			static_cast<int32_t>(input.attackerMaxHP),
			static_cast<int32_t>(input.attackerLevel),
			static_cast<int32_t>(input.targetLevel));

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

		if (result.damage == 0)
			result.damage = 1;

		return result;
	}
}