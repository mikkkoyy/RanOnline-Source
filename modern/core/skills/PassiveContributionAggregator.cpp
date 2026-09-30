// VERTICAL-003: passive skill contribution aggregation.
//
// Field provenance for every line below is in
// docs/reference/client/VERTICAL-003_SKILL_INVESTIGATION.md, which traces
// each one to GLCHARLOGIC::SUM_PASSIVE (GLogixExPC.cpp:863).

#include "skills/PassiveContributionAggregator.h"

#include <cmath>
#include <limits>

namespace Modern
{
	namespace
	{
		// Check if a passive skill's weapon requirements are satisfied by
		// the currently equipped items. Mirrors the logic in
		// GLCHARLOGIC::SUM_PASSIVE (GLogixExPC.cpp:882-914).
		bool CheckWeaponRequirements(const SkillDefinition& def, const EquipmentState& equipment)
		{
			// Left hand requirement
			if (def.RequiresWeapon(SkillWeaponSlot::LeftHand))
			{
				const auto requiredType = def.GetRequiredWeapon(SkillWeaponSlot::LeftHand);
				const EquipmentEntry& entry = equipment.Get(EquipmentSlot::LeftHand);
				if (!entry.HasItem())
				{
					return false;
				}
				// We need to check the item's weapon type. For now, since we
				// don't have item weapon type in ItemDefinition, we'll assume
				// the requirement is met if the slot is occupied. A future
				// system can add weapon type to ItemDefinition.
				// TODO: Add weapon type to ItemDefinition when item system is extended.
				(void)requiredType;
			}

			// Right hand requirement
			if (def.RequiresWeapon(SkillWeaponSlot::RightHand))
			{
				const auto requiredType = def.GetRequiredWeapon(SkillWeaponSlot::RightHand);
				const EquipmentEntry& entry = equipment.Get(EquipmentSlot::RightHand);
				if (!entry.HasItem())
				{
					return false;
				}
				(void)requiredType;
			}

			return true;
		}

		// Add a value to the contribution field, with type-appropriate handling.
		// Legacy SUM_PASSIVE uses DWORD cast for HP/MP/SP and int cast for others.
		void AddToContribution(Stats::PassiveContribution& c,
		                        PassiveApplyType type, float value)
		{
			switch (type)
			{
				// Flat resource bonuses: DWORD cast (unsigned).
				case PassiveApplyType::Hp:
					c.hp += static_cast<int32_t>(value);
					break;
				case PassiveApplyType::Mp:
					c.mp += static_cast<int32_t>(value);
					break;
				case PassiveApplyType::Sp:
					c.sp += static_cast<int32_t>(value);
					break;

				// Recovery rate additions: float.
				case PassiveApplyType::VarHp:
					c.hpRecoveryRate += value;
					break;
				case PassiveApplyType::VarMp:
					c.mpRecoveryRate += value;
					break;
				case PassiveApplyType::VarSp:
					c.spRecoveryRate += value;
					break;

				// Flat defense: int cast.
				case PassiveApplyType::Defense:
					c.defense += static_cast<int32_t>(value);
					break;

				// Flat hit/avoid: int cast.
				case PassiveApplyType::HitRate:
					c.hit += static_cast<int32_t>(value);
					break;
				case PassiveApplyType::AvoidRate:
					c.avoid += static_cast<int32_t>(value);
					break;

				// All three resource rates: float.
				case PassiveApplyType::VarAp:
					c.hpRecoveryRate += value;
					c.mpRecoveryRate += value;
					c.spRecoveryRate += value;
					break;

				// Flat damage/attack power: int cast.
				case PassiveApplyType::VarDamage:
					c.damage += static_cast<int32_t>(value);
					break;
				case PassiveApplyType::VarDefense:
					c.defense += static_cast<int32_t>(value);
					break;

				// Flat attack powers: int cast.
				case PassiveApplyType::Pa:
					c.meleePower += static_cast<int32_t>(value);
					break;
				case PassiveApplyType::Sa:
					c.shootPower += static_cast<int32_t>(value);
					break;
				case PassiveApplyType::Ma:
					c.magicAttack += static_cast<int32_t>(value);
					break;

				// Resource maximum multiplicative rates: float.
				case PassiveApplyType::HpRate:
					c.hpRate += value;
					break;
				case PassiveApplyType::MpRate:
					c.mpRate += value;
					break;
				case PassiveApplyType::SpRate:
					c.spRate += value;
					break;

				// Resistances: all five elements equally, int cast.
				case PassiveApplyType::Resist:
				{
					const int32_t resistVal = static_cast<int32_t>(value);
					c.resistances.fire     += resistVal;
					c.resistances.ice      += resistVal;
					c.resistances.electric += resistVal;
					c.resistances.poison   += resistVal;
					c.resistances.spirit   += resistVal;
					break;
				}

			// Summon time: not in stat pipeline.
			case PassiveApplyType::SummonTime:
				break;

			// VERTICAL-007: combat modifiers.
			case PassiveApplyType::CriticalRate:
				c.criticalRate += value;
				break;
			case PassiveApplyType::CrushingBlow:
				c.crushingBlow += value;
				break;
			case PassiveApplyType::DamageReduce:
				c.damageReduce += value;
				break;
			case PassiveApplyType::DamageReflection:
				c.damageReflection += value;
				break;
			case PassiveApplyType::DamageReflectionRate:
				c.damageReflectionRate += value;
				break;
			}
		}

		void AddImpactToContribution(Stats::PassiveContribution& c,
		                              PassiveImpactType type, float value)
		{
			switch (type)
			{
				case PassiveImpactType::HitRate:
					c.hit += static_cast<int32_t>(value);
					break;
				case PassiveImpactType::AvoidRate:
					c.avoid += static_cast<int32_t>(value);
					break;
				case PassiveImpactType::Damage:
					c.damage += static_cast<int32_t>(value);
					break;
				case PassiveImpactType::Defense:
					c.defense += static_cast<int32_t>(value);
					break;
				case PassiveImpactType::VarHp:
					c.hpRecoveryRate += value;
					break;
				case PassiveImpactType::VarMp:
					c.mpRecoveryRate += value;
					break;
				case PassiveImpactType::VarSp:
					c.spRecoveryRate += value;
					break;
				case PassiveImpactType::VarAp:
					c.hpRecoveryRate += value;
					c.mpRecoveryRate += value;
					c.spRecoveryRate += value;
					break;
				case PassiveImpactType::DamageRate:
					// Not in PassiveContribution — legacy has m_fDAMAGE_RATE but
					// it's not in the stat pipeline.
					break;
				case PassiveImpactType::DefenseRate:
					// Not in PassiveContribution.
					break;
				case PassiveImpactType::Pa:
					c.meleePower += static_cast<int32_t>(value);
					break;
				case PassiveImpactType::Sa:
					c.shootPower += static_cast<int32_t>(value);
					break;
				case PassiveImpactType::Ma:
					c.magicAttack += static_cast<int32_t>(value);
					break;
				case PassiveImpactType::HpRate:
					c.hpRate += value;
					break;
				case PassiveImpactType::MpRate:
					c.mpRate += value;
					break;
				case PassiveImpactType::SpRate:
					c.spRate += value;
					break;
				case PassiveImpactType::Resist:
				{
					const int32_t resistVal = static_cast<int32_t>(value);
					c.resistances.fire     += resistVal;
					c.resistances.ice      += resistVal;
					c.resistances.electric += resistVal;
					c.resistances.poison   += resistVal;
					c.resistances.spirit   += resistVal;
					break;
				}
			default:
				break;

			// VERTICAL-007: combat modifiers from impacts.
			case PassiveImpactType::CriticalRate:
				c.criticalRate += value;
				break;
			case PassiveImpactType::CrushingBlow:
				c.crushingBlow += value;
				break;
			case PassiveImpactType::DamageReduce:
				c.damageReduce += value;
				break;
			case PassiveImpactType::DamageReflection:
				c.damageReflection += value;
				break;
			case PassiveImpactType::DamageReflectionRate:
				c.damageReflectionRate += value;
				break;
			}
		}

		// Check if a skill definition has any non-finite values.
		bool IsDefinitionFinite(const SkillDefinition& def)
		{
			for (uint8_t lvl = 1; lvl <= def.maxLevel; ++lvl)
			{
				if (!std::isfinite(def.levelData[lvl].basicVar))
				{
					return false;
				}
			}
			for (const auto& imp : def.impacts)
			{
				if (imp.IsValid())
				{
					for (uint8_t lvl = 1; lvl <= def.maxLevel; ++lvl)
					{
						if (!std::isfinite(imp.values[lvl]))
						{
							return false;
						}
					}
				}
			}
			for (const auto& spec : def.specs)
			{
				if (spec.IsValid())
				{
					for (uint8_t lvl = 1; lvl <= def.maxLevel; ++lvl)
					{
						if (!std::isfinite(spec.values[lvl]))
						{
							return false;
						}
					}
				}
			}
			return true;
		}
	}

	Result<PassiveContributionResult> PassiveContributionAggregator::Aggregate(
		const SkillState& skills,
		const SkillDefinitionProvider& provider,
		const EquipmentState& equipment)
	{
		PassiveContributionResult result;

		// Iterate in SkillId order (deterministic).
		for (const auto& [skillId, learned] : skills.GetAllSkills())
		{
			if (!learned.IsLearned())
			{
				continue;
			}

			const SkillDefinition* def = provider.Find(skillId);
			if (def == nullptr)
			{
				result.error = PassiveAggregationError::MissingDefinition;
				return result;
			}

			// Skip active skills (only passive skills contribute to stats).
			// In the modern model, we don't track role on the definition;
			// the caller should only pass passive skills. But for defense
			// in depth, we could check if the definition has any passive
			// apply type. Since all definitions in this system are passive
			// by construction, we don't need this check.

			// Check if the definition has non-finite values.
			if (!IsDefinitionFinite(*def))
			{
				result.error = PassiveAggregationError::NonFinite;
				return result;
			}

			// Check equipment requirements for this passive.
			if (!CheckWeaponRequirements(*def, equipment))
			{
				continue;  // Skill not active due to equipment mismatch.
			}

			const uint8_t level = learned.level;
			if (level == 0 || level > def->maxLevel)
			{
				continue;  // Invalid level, skip silently.
			}

			// Get the basic apply value for this level.
			const float basicVar = def->levelData[level].basicVar;
			if (basicVar != 0.0f)
			{
				AddToContribution(result.contribution, def->applyType, basicVar);
				++result.contributingSkills;
			}

			// Process impacts (addons).
			for (const auto& imp : def->impacts)
			{
				if (!imp.IsValid())
				{
					continue;
				}
				const float impVal = imp.values[level];
				if (impVal != 0.0f)
				{
					AddImpactToContribution(result.contribution, imp.type, impVal);
					// Don't increment contributingSkills for impacts — it's
					// per-skill, not per-impact.
				}
			}

			// Process specs. Currently no specs feed the stat pipeline.
			// If we add specs that do, handle them here.
			(void)def->specs;
		}

		return result;
	}
}