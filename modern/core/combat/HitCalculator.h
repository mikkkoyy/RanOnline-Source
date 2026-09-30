// VERTICAL-006: hit calculation implementation.
//
// Legacy provenance: GLogixExPC.cpp:1291 CHECKHIT, GLogixExPC.cpp:1322-1326
// GameCharacterCalculations.cpp:68-85 HitRate

#include "CombatTypes.h"
#include "CombatConstants.h"
#include "engine/GameCharacterCalculations.h"

#include <algorithm>

namespace Modern::Combat
{
	// Calculates hit rate using the legacy formula.
	static uint32_t CalculateHitRate(const HitInput& input, const CombatConstants& constants)
	{
		int32_t brightnessMod = 0;
		switch (input.brightnessFB)
		{
			case Modern::Engine::GameBrightFB::Dis:  brightnessMod = constants.brightnessModDis; break;
			case Modern::Engine::GameBrightFB::Adv:  brightnessMod = constants.brightnessModAdv; break;
			default:                               brightnessMod = constants.brightnessModAver; break;
		}

		int32_t hitRate = constants.basicHitRate + input.attackerHit - input.targetAvoid + brightnessMod;

		if (hitRate > static_cast<int32_t>(constants.maxHitRate))
			hitRate = constants.maxHitRate;
		else if (hitRate < static_cast<int32_t>(constants.minHitRate))
			hitRate = constants.minHitRate;

		if (input.lowSP)
		{
			hitRate = static_cast<int32_t>(hitRate * (1.0f - constants.lowSPHitDrop));
		}

		return static_cast<uint32_t>(hitRate);
	}

	// Determines if an attack hits.
	static bool CheckHit(const HitInput& input, const CombatConstants& constants, HitResult& outResult)
	{
		uint32_t hitRate = CalculateHitRate(input, constants);
		uint32_t hitRoll = static_cast<uint32_t>(input.hitRoll * 100.0f);

		outResult.hit = (hitRate >= hitRoll);
		outResult.hitRate = hitRate;
		outResult.hitRoll = hitRoll;

		return outResult.hit;
	}

	// Public interface
	HitResult CalculateHit(const HitInput& input, const CombatConstants& constants = CombatConstants())
	{
		HitResult result;
		CheckHit(input, constants, result);
		return result;
	}
}