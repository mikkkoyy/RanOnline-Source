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
					// VERTICAL-024: GLogixExPC.cpp:1042
					//   m_sSUM_PASSIVE.m_fDAMAGE_RATE += fADDON;
					// Added raw, and it is an increment on the 1.0 that seeds
					// `m_fDamageRate` (:2219), not a percentage. Legacy consumes it
					// in CALCDAMAGE on the damage range, so it is a combat-boundary
					// value and never reaches `DerivedStats`.
					c.damageRate += value;
					break;
				case PassiveImpactType::DefenseRate:
					// VERTICAL-021: GLogixExPC.cpp:1046
					//   m_sSUM_PASSIVE.m_fDEFENSE_RATE += fADDON;
					// Added raw. Legacy seeds the accumulator at 1.0f, so this is a
					// multiplier around 1.0 rather than a percentage, and legacy does
					// not normalise it.
					c.defenseRate += value;
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

	// SKILL-013: a recovered spec type's value into its contribution field.
	//
	// Mirrors `AddImpactToContribution` but WITHOUT an int cast: legacy adds
	// `sSPEC.fVAR1` straight into a float accumulator
	// (GLogixExPC.cpp:1093, :1142, :1147) and truncates only later, at the
	// consumer. An unsupported or empty type matches no case and leaves the
	// contribution untouched, which is what keeps the slots the loader
	// refused genuinely inert.
	void AddSpecToContribution(Stats::PassiveContribution& c,
	                           PassiveSpecType type, float value)
	{
		switch (type)
		{
		case PassiveSpecType::TarRange:
			c.tarRange += value;
			break;
		case PassiveSpecType::ChangeAttackRange:
			c.skillAttackRange += value;
			break;
		case PassiveSpecType::ChangeApplyRange:
			c.skillApplyRange += value;
			break;
		default:
			// PassiveSpecType::None, and any spec type the loader refused.
			break;
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

			// SKILL-016: only EMROLE_PASSIVE contributes.
			//
			// Legacy GLogixExPC.cpp:878, inside SUM_PASSIVE, is the SECOND
			// statement of the loop body - immediately after the definition
			// lookup, and ahead of the weapon check, the per-level read and
			// every accumulation below. So this is where it belongs here too.
			//
			// `SkillRole::Passive` (SkillDefinition.h:350) mirrors
			// `SKILL::EMROLE_PASSIVE = 1` (GLSkillBasic.h:133). Both enums are
			// 0/1 and the loader range-checks the column to 0..1 before casting
			// (SkillBasicTable.cpp:1004, :1020), so no cast is needed here.
			//
			// Skipped SILENTLY, like the weapon check below: having learned an
			// active skill is not a data fault.
			//
			// The previous revision of this file declined this check on the
			// stated ground that all definitions in this system are passive by
			// construction. That is false of a definition loaded from the
			// export, where emROLE is a real column.
			if (def->role != SkillRole::Passive)
			{
				continue;
			}

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

			// SKILL-013: the three range specs.
			//
			// Legacy GLogixExPC.cpp:1082-1148 walks ALL `MAX_SPEC` slots of
			// `m_sAPPLY.sSpecs` and, for each, reads
			// `sSpecs[nSpec].sSPEC[sCharSkill.wLevel].fVAR1` - the LEARNED
			// level, not level 1 and not a flat value. There is no
			// `isPhysicalLongRange` or `affectsEnemy` test here: those
			// conditions gate CONSUMPTION in SkillTargetRange/SkillApplyRange,
			// not accumulation, so applying them here would drop bonuses
			// legacy keeps.
			//
			// Accumulation is `+=` with no cast and no clamp, so a negative
			// range spec reduces the bonus exactly as legacy does.
			//
			// Every slot is visited - a character may carry more than one of
			// these types, and the relevant one is not guaranteed to be slot
			// zero. Slots the loader left empty are inert because
			// `PassiveSpecType::None` matches no case.
			for (const SkillSpec& spec : def->specs)
			{
				if (!spec.IsValid())
				{
					continue;
				}
				// A zero value at the learned level contributes nothing but is
				// still added, so the result is identical either way and no
				// special case is needed for it.
				AddSpecToContribution(result.contribution, spec.type,
				                      spec.values[level]);
			}
		}

		return result;
	}
}