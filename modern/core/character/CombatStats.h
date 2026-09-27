#pragma once

#include "../types/Types.h"
#include "../item/ItemData.h"

#include <cstdint>

namespace Modern
{
	// Modern combat stats - derived values representing character combat capabilities
	// Matches legacy GLOGICEX derived combat stat calculations
	struct CombatStats
	{
		// Hit / Avoid
		// Legacy: m_nHIT, m_nAVOID
		// Calculation: (DEX * factor + itemFlat + passiveFlat) * itemRatePercent -> int
		int32_t hitRate = 0;
		int32_t avoidRate = 0;

		// Defense
		// Legacy: m_nDEFENSE, m_nDEFENSE_BODY, m_nDEFENSE_SKILL
		// Calculation: (DP + DEX * factor + itemFlat + passiveFlat) * defenseRate -> int
		int32_t defense = 0;
		int32_t defenseBody = 0;  // Before item/passive/rate
		int32_t defenseSkill = 0; // After skill buffs (not yet implemented)

		// Damage
		// Legacy: m_gdDAMAGE, m_gdDAMAGE_SKILL, m_gdDAMAGE_PHYSIC
		// Calculation: (AP + passiveFlat) -> base damage, + itemDamage -> physical, * damageRate -> final
		struct DamageRange
		{
			int32_t low = 0;
			int32_t high = 0;
		};
		DamageRange baseDamage;      // AP + passive
		DamageRange skillDamage;     // After skill buffs (not yet implemented)
		DamageRange physicalDamage;  // After item + PA/SA + damageRate

		// Damage / Defense rate multipliers
		// Legacy: m_fDamageRate, m_fDefenseRate
		// Base 1.0 + passive + skill/buff/pet/land effects
		float damageRate = 1.0f;
		float defenseRate = 1.0f;

		// Resistances
		// Legacy: m_sSUMRESIST, m_sSUMRESIST_SKILL
		// Calculation: passive + item + codex + skill/buff/pet/land -> LIMIT()
		int32_t resistFire = 0;
		int32_t resistIce = 0;
		int32_t resistElec = 0;
		int32_t resistPoison = 0;
		int32_t resistSpirit = 0;

		// Pierce
		// Legacy: m_nSUM_PIERCE
		// Calculation: passive + skill
		int32_t pierce = 0;

		// Range
		// Legacy: m_fTARRANGE, m_fSUM_SKILL_ATTACKRANGE, m_fSUM_SKILL_APPLYRANGE
		float targetRange = 0.0f;
		float skillAttackRange = 0.0f;
		float skillApplyRange = 0.0f;

		// Velocity
		// Legacy: m_fMOVEVELO, m_fATTVELO
		// Calculation: passive + state + pet/land effects
		float moveVelocity = 0.0f;
		float attackVelocity = 0.0f;

		// Skill delay
		// Legacy: m_fSKILLDELAY, m_fSTATE_DELAY
		// Calculation: passive + state + skill/quest/pet effects
		float skillDelay = 0.0f;

		// Damage specification (reflection/reduction)
		// Legacy: m_sDamageSpec
		struct DamageSpec
		{
			float psyDamageReduce = 0.0f;
			float magicDamageReduce = 0.0f;
			float psyDamageReflection = 0.0f;
			float psyDamageReflectionRate = 0.0f;
			float magicDamageReflection = 0.0f;
			float magicDamageReflectionRate = 0.0f;
			float damageCurse = 0.0f;
			// EffRate fields omitted - legacy uses them but modern calc path unclear
		};
		DamageSpec damageSpec;

		void Reset()
		{
			*this = {};
			damageRate = 1.0f;
			defenseRate = 1.0f;
		}

		// Legacy SRESIST::LIMIT() behavior: clamp negative values to 0
		// No upper bound clamp in legacy.
		static int32_t ApplyResistanceLimit(int32_t value)
		{
			return value < 0 ? 0 : value;
		}
	};
}