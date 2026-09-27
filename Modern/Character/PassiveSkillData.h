#pragma once

#include "../Core/Types.h"
#include "../Item/ItemData.h"

#include <cstdint>

namespace Modern
{
	// Forward declaration
	class Character;

	// Passive skill contribution - represents aggregated passive skill effects
	// Matches legacy SPASSIVE_SKILL_DATA fields that affect character stats
	struct PassiveSkillContribution
	{
		// Flat HP/MP/SP (added before rate multiplication)
		int32_t hp = 0;
		int32_t mp = 0;
		int32_t sp = 0;

		// Rate multipliers for HP/MP/SP (applied as (1 + rate) * conftRate)
		// Legacy: m_fHP_RATE, m_fMP_RATE, m_fSP_RATE
		float hpRate = 0.0f;
		float mpRate = 0.0f;
		float spRate = 0.0f;

		// Recovery rate bonuses
		// Legacy: m_fINCR_HP, m_fINCR_MP, m_fINCR_SP
		float hpRecoveryRate = 0.0f;
		float mpRecoveryRate = 0.0f;
		float spRecoveryRate = 0.0f;

		// Combat stat bonuses
		// Legacy: m_nPA, m_nSA, m_nMA
		int32_t pa = 0;
		int32_t sa = 0;
		int32_t ma = 0;

		// Damage/Defense/Hit/Avoid
		// Legacy: m_nDAMAGE, m_nDEFENSE, m_nHIT, m_nAVOID
		int32_t damage = 0;
		int32_t defense = 0;
		int32_t hitRate = 0;
		int32_t avoidRate = 0;

		// Damage/Defense rate multipliers
		// Legacy: m_fDAMAGE_RATE, m_fDEFENSE_RATE
		float damageRate = 0.0f;
		float defenseRate = 0.0f;

		// Resistances
		// Legacy: m_sSUMRESIST (nFire, nIce, nElectric, nPoison, nSpirit)
		int32_t resistFire = 0;
		int32_t resistIce = 0;
		int32_t resistElec = 0;
		int32_t resistPoison = 0;
		int32_t resistSpirit = 0;

		// Movement/Attack velocity
		// Legacy: m_fMOVEVELO, m_fATTVELO
		float moveVelocity = 0.0f;
		float attackVelocity = 0.0f;

		// Skill delay
		// Legacy: m_fSKILLDELAY
		float skillDelay = 0.0f;

		// Skill range bonuses
		// Legacy: m_fTARRANGE, m_fSUM_SKILL_ATTACKRANGE, m_fSUM_SKILL_APPLYRANGE
		float targetRange = 0.0f;
		float skillAttackRange = 0.0f;
		float skillApplyRange = 0.0f;

		// Pierce
		// Legacy: m_nPIERCE
		int32_t pierce = 0;

		// Summon time
		// Legacy: m_nSummonTime
		int32_t summonTime = 0;

		// Damage spec (reflection/reduction)
		// Legacy: m_sDamageSpec
		// Note: Complex nested structure - only expose fields confirmed used
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
		}
	};

	// Provider interface for passive skill contributions
	class IPassiveSkillProvider
	{
	public:
		virtual ~IPassiveSkillProvider() = default;

		// Get passive contribution for a character
		// Parameters match legacy calculation context: class, school, level
		virtual PassiveSkillContribution GetContribution(
			uint32_t classId,
			uint16_t schoolId,
			uint16_t level
		) const = 0;

		// Get contribution for a character instance (for context that needs entity)
		virtual PassiveSkillContribution GetContribution(
			const Character& character
		) const
		{
			return GetContribution(character.GetClassId(), character.GetSchool(), character.GetLevel());
		}
	};

	// Test provider with deterministic values
	class TestPassiveSkillProvider : public IPassiveSkillProvider
	{
	public:
		TestPassiveSkillProvider() = default;

		PassiveSkillContribution GetContribution(
			uint32_t classId,
			uint16_t schoolId,
			uint16_t level
		) const override
		{
			PassiveSkillContribution c;

			// Simple deterministic values based on level
			if (level >= 10)
			{
				c.hp = 50;
				c.mp = 30;
				c.sp = 20;
				c.hpRate = 0.1f;  // +10%
				c.mpRate = 0.05f; // +5%
				c.spRate = 0.05f; // +5%
				c.hpRecoveryRate = 0.01f;
				c.mpRecoveryRate = 0.01f;
				c.spRecoveryRate = 0.01f;
				c.pa = 5;
				c.sa = 3;
				c.ma = 2;
				c.damage = 10;
				c.defense = 15;
				c.hitRate = 5;
				c.avoidRate = 3;
				c.damageRate = 0.05f;
				c.defenseRate = 0.03f;
				c.resistFire = 10;
				c.resistIce = 10;
				c.resistElec = 10;
				c.resistPoison = 10;
				c.resistSpirit = 10;
				c.moveVelocity = 0.5f;
				c.attackVelocity = 0.1f;
				c.skillDelay = -0.1f;
				c.targetRange = 2.0f;
				c.skillAttackRange = 3.0f;
				c.skillApplyRange = 3.0f;
				c.pierce = 2;
				c.summonTime = 60;
			}

			return c;
		}
	};
}