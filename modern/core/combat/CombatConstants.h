#pragma once

// VERTICAL-006: combat constants and configuration.
//
// Every field is classified:
//   SOURCE-VERIFIED  = verified from repository legacy source
//   PUBLIC-REFERENCE = found in public RAN sources but not in this repo
//   UNAVAILABLE      = not found in any source
//
// Values with SOURCE-VERIFIED classification match the repository's
// GLogicData.cpp defaults exactly.

namespace Modern::Combat
{
	// Constants and configuration for combat resolution.
	// Every field is classified:
	//   SOURCE-VERIFIED  = verified from repository legacy source
	//   PUBLIC-REFERENCE = found in public RAN sources but not in this repo
	//   UNAVAILABLE      = not found in any source
	//
	// Values with SOURCE-VERIFIED classification match the repository's
	// GLogicData.cpp defaults exactly.
	struct CombatConstants
	{
		// Low SP modifiers (SOURCE-VERIFIED from GLogicData.cpp)
		float lowSPHitDrop = 0.25f;        // fLOWSP_HIT_DROP
		float lowSPDamage = 0.50f;         // fLOWSP_DAMAGE (used in legacy as fLOW_SEED_DAMAGE)

		// Damage grade scaling (SOURCE-VERIFIED from GLogicData.cpp)
		float damageGradeK = 10.0f;        // fDAMAGE_GRADE_K

		// Damage decay (SOURCE-VERIFIED from GLogicData.cpp)
		float damageDecayRate = 40000.0f;  // fDAMAGE_DEC_RATE

		// Physical resistance scaling (SOURCE-VERIFIED from GLogicData.cpp)
		float resistPhysicG = 0.5f;        // fRESIST_PHYSIC_G

		// Critical hit (SOURCE-VERIFIED from GLogicData.cpp)
		uint32_t criticalDamage = 120;     // dwCRITICAL_DAMAGE (120%)
		uint32_t criticalMax = 40;         // dwCRITICAL_MAX (40% cap)

		// Crushing blow (SOURCE-VERIFIED from GLogicData.cpp)
		uint32_t crushingBlowDamage = 150; // dwCRUSHING_BLOW_DAMAGE (150%)
		uint32_t crushingBlowMax = 20;     // dwCRUSHING_BLOW_MAX (20% cap)
		float crushingBlowRange = 10.0f;   // fCRUSH_BLOW_RANGE

		// Low seed damage (SOURCE-VERIFIED from GLogicData.cpp)
		float lowSeedDamage = 0.05f;       // fLOW_SEED_DAMAGE

		// Hit rate bounds (SOURCE-VERIFIED from GameCharacterCalculations.cpp)
		uint32_t maxHitRate = 99;
		uint32_t minHitRate = 20;
		uint32_t basicHitRate = 100;

		// Maximum resistance cap (SOURCE-VERIFIED from GLogicData.cpp)
		float maxResist = 0.8f;            // fMAX_RESIST = 0.8f

		// Damage reduction/decay (SOURCE-VERIFIED from GLogicData.cpp / GLogixExPC.cpp)
		float damageDecRate = 40000.0f;    // fDAMAGE_DEC_RATE

		// Maximum level for damage reduction/reflection scaling (SOURCE-VERIFIED)
		uint32_t maxLevel = 300;           // wMAX_LEVEL

		// Critical hit rate constants (SOURCE-VERIFIED from GameCharacterCalculations.cpp)
		uint32_t criticalHitRateBase = 5;      // CRITICALHIT_RATE
		uint32_t cleanHitRateBase = 1;         // CLEANHIT_RATE
		uint32_t minDamageForShock = 6;        // MIN_DAMAGE
		uint32_t minLevelDiffForShock = 5;     // MIN_DXLEVEL

		// Brightness modifiers (SOURCE-VERIFIED from GameCharacterCalculations.cpp)
		int brightnessModDis = -10;  // BFB_DIS
		int brightnessModAver = 0;   // BFB_AVER
		int brightnessModAdv = 10;   // BFB_ADV

		// Environment defense factors (SOURCE-VERIFIED from GameCharacterCalculations.cpp)
		float envFactorDis = 0.8f;
		float envFactorAver = 1.0f;
		float envFactorAdv = 1.2f;

		// Low SP damage multiplier formula constant (SOURCE-VERIFIED from GLogixExPC.cpp)
		float lowSeedDamageBase = 1.0f;

		// Defense skill minimum (SOURCE-VERIFIED from GLogixExPC.cpp:2975)
		int32_t defenseSkillMinimum = 1;

		constexpr bool operator==(const CombatConstants& other) const noexcept
		{
			return lowSPHitDrop == other.lowSPHitDrop &&
			       lowSPDamage == other.lowSPDamage &&
			       damageGradeK == other.damageGradeK &&
			       damageDecayRate == other.damageDecayRate &&
			       resistPhysicG == other.resistPhysicG &&
			       criticalDamage == other.criticalDamage &&
			       criticalMax == other.criticalMax &&
			       crushingBlowDamage == other.crushingBlowDamage &&
			       crushingBlowMax == other.crushingBlowMax &&
			       crushingBlowRange == other.crushingBlowRange &&
			       lowSeedDamage == other.lowSeedDamage &&
			       maxHitRate == other.maxHitRate &&
			       minHitRate == other.minHitRate &&
			       basicHitRate == other.basicHitRate &&
			       maxResist == other.maxResist &&
			       damageDecRate == other.damageDecRate &&
			       maxLevel == other.maxLevel &&
			       criticalHitRateBase == other.criticalHitRateBase &&
			       cleanHitRateBase == other.cleanHitRateBase &&
			       minDamageForShock == other.minDamageForShock &&
			       minLevelDiffForShock == other.minLevelDiffForShock &&
			       brightnessModDis == other.brightnessModDis &&
			       brightnessModAver == other.brightnessModAver &&
			       brightnessModAdv == other.brightnessModAdv &&
			       envFactorDis == other.envFactorDis &&
			       envFactorAver == other.envFactorAver &&
			       envFactorAdv == other.envFactorAdv &&
			       lowSeedDamageBase == other.lowSeedDamageBase &&
			       defenseSkillMinimum == other.defenseSkillMinimum &&
			       maxResist == other.maxResist &&
			       damageDecRate == other.damageDecRate &&
			       maxLevel == other.maxLevel;
		}
	};
}