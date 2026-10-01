// VERTICAL-011: active skill resolution. See ActiveSkill.h.
//
// Every rule below cites the legacy line it comes from. Nothing here is
// inferred, and a behaviour the source does not prove is refused rather than
// approximated.

#include "skills/ActiveSkill.h"

#include "combat/CombatCalculator.h"
#include "combat/CombatConstants.h"
#include "skills/SkillFactTypes.h"

#include <cmath>

namespace Modern::Skills
{
	// VERTICAL-017: the two damage-protection sources compete rather than add.
	//
	// Legacy seeds `m_sDamageSpec` from the passive total and then max-accumulates
	// the FACT specs into it (GLogixExPC.cpp:2228, :2380-2401), so the larger of
	// the two values is what survives. Applied to the amount and the rate alike,
	// because legacy reads them as one pair from one spec.
	static float Stronger(float current, float candidate) noexcept
	{
		return (candidate > current) ? candidate : current;
	}
	// VERTICAL-013: `SRESIST::GetElement(emELMT)` (GLogixExPC.cpp:1515).
	//
	// `Stats::Resistances` carries only the five elements RAN actually models
	// as separate resistance axes (fire, ice, electric, poison, spirit). The
	// remaining EMELEMENT values - stone, mad, curse, zen - have no field, so
	// they read 0 rather than silently borrowing another element's number.
	// Widening `Resistances` is a data-model change, not a combat change, and
	// guessing those four values here would be inventing balance data.
	static int32_t ResolveResistance(const Stats::Resistances& resistances,
	                                 SkillElement element) noexcept
	{
		switch (element)
		{
			case SkillElement::Fire:     return resistances.fire;
			case SkillElement::Ice:      return resistances.ice;
			case SkillElement::Electric: return resistances.electric;
			case SkillElement::Poison:   return resistances.poison;
			case SkillElement::Spirit:   return resistances.spirit;

			// No modelled resistance axis.
			case SkillElement::Stone:
			case SkillElement::Mad:
			case SkillElement::Curse:
			case SkillElement::Zen:      return 0;

			// Already resolved before this call; belt and braces.
			case SkillElement::ArmWeapon: return 0;
		}
		return 0;
	}
	namespace ActiveSkillResolver
	{
		namespace
		{
			ActiveSkillResult Refuse(ActiveSkillFailure failure) noexcept
			{
				ActiveSkillResult result;
				result.failure = failure;
				return result;
			}
		}

		uint16_t RequiredSP(const ActiveSkillInput& input,
		                    const SkillDefinition& definition) noexcept
		{
			// GLogixExPC.cpp:4254-4256
			//   WORD wDisSP = sSKILL_DATA.wUSE_SP;
			//   if ( pRHAND )  wDisSP += pRHAND->sSuitOp.wReqSP;
			//   if ( pLHAND )  wDisSP += pLHAND->sSuitOp.wReqSP;
			//
			// VERTICAL-010 established that the two hand terms are the
			// equipment contribution, and that it arrives in
			// `equipmentRequiredSP`. `basicAttackSP` is not added here: a skill
			// pays `wUSE_SP`, not `wBASIC_DIS_SP`, and the legacy line above
			// shows only `wUSE_SP` plus the hands. It is carried on the input
			// so a caller building the context cannot silently omit the base
			// cost that a basic attack would pay, and so the difference between
			// the two paths is visible in one place.
			(void)input.basicAttackSP;

			return static_cast<uint16_t>(
				input.equipmentRequiredSP + definition.GetLevelData(input.level).useSp);
		}

		LowSpState EvaluateLowSp(uint32_t currentSp, uint16_t requiredSP) noexcept
		{
			// GLogixExPC.cpp:4258 - `m_sSP.dwNow < wDisSP * wStrikeNum`.
			// Strictly less than, and the pool is the caster's own.
			return currentSp < static_cast<uint32_t>(requiredSP)
			           ? LowSpState::Low
			           : LowSpState::Normal;
		}

		float CooldownSeconds(const SkillDefinition& definition, uint8_t level,
		                     uint16_t attackerLevel) noexcept
		{
			// GameCharacterCalculations.cpp:118-126
			//   return static_cast<float>(dwSKILL_GRADE * wSKILL_LEV)
			//        / static_cast<float>(wCHAR_LEVEL) + fDelay;
			//
			// The product is integer in legacy, and is kept integer here so the
			// two agree on overflow. A zero character level would divide by
			// zero in legacy; the guard below returns the bare delay instead of
			// an infinity, which is a documented departure rather than a
			// matching crash.
			if (attackerLevel == 0)
			{
				return definition.GetLevelData(level).delayTime;
			}

			const uint32_t product = definition.grade * static_cast<uint32_t>(level);
			return static_cast<float>(product) / static_cast<float>(attackerLevel) +
			       definition.GetLevelData(level).delayTime;
		}

		ActiveSkillResult Resolve(const ActiveSkillInput& input) noexcept
		{
			// ---- 1. The definition, the level, the role ----
			//
			// GLogixExPC.cpp:4085 rejects an unknown skill; :4092 rejects
			// anything whose emROLE is not EMROLE_NORMAL. Both are checked
			// before anything is read out of the definition.

			if (input.definition == nullptr)
			{
				return Refuse(ActiveSkillFailure::UnknownSkill);
			}

			const SkillDefinition& definition = *input.definition;

			if (input.level == 0 || input.level > definition.maxLevel ||
			    input.level > kMaxSkillLevel)
			{
				return Refuse(ActiveSkillFailure::InvalidLevel);
			}

			if (definition.role != SkillRole::Normal)
			{
				return Refuse(ActiveSkillFailure::NotCastable);
			}

			// ---- 2. Caster state that forbids the cast outright ----
			//
			// GLogixExPC.cpp:4060-4063: `m_bProhibitSkill` and
			// `EMBLOW`/stun both return EMSKILL_PROHIBIT before the skill is
			// even looked up. Order is preserved because a prohibited caster
			// learns nothing about whether the skill exists.

			if (input.skillProhibited || input.stunned)
			{
				return Refuse(ActiveSkillFailure::NotCastable);
			}

			// ---- 3. Cooldown ----
			//
			// GLogixExPC.cpp:4082-4083: a `m_SKILLDELAY` entry for this skill
			// returns EMSKILL_DELAYTIME. The map lives on the server; the
			// boolean arrives here.

			if (input.onCooldown)
			{
				return Refuse(ActiveSkillFailure::InCooldown);
			}

			// ---- 4. What this slice can and cannot execute ----
			//
			// Every one of these is a refusal with a reason, never a zero
			// result. A skill the modern server cannot run honestly must say so.

			// VERTICAL-012/013: the apply channel decides which attack power the
			// combat pipeline selects, and which calculator runs.
			//
			//   PhysicalMelee / PhysicalRanged -> CalculatePhysicalDamage
			//   Magic                          -> CalculateMagicDamage
			//
			// Magic is not a third flag on the physical formula; see
			// MagicDamageCalculator.h for the differences that make it a
			// separate function.
			if (definition.apply != SkillApply::PhysicalMelee &&
			    definition.apply != SkillApply::PhysicalRanged &&
			    definition.apply != SkillApply::Magic)
			{
				return Refuse(ActiveSkillFailure::UnsupportedApply);
			}

			// VERTICAL-013: the executed magic slice is narrow and says so.
			//
			//   EMAPPLY_MAGIC + EMFOR_HP + fBASIC_VAR < 0 + TAR_SPEC + SIDE_ENEMY
			//
			// Everything else in the magic switch (GLChar.cpp:3075-3123) is a
			// refusal with its own reason, never a zero result and never a
			// silently reinterpreted damage number.
			if (definition.apply == SkillApply::Magic)
			{
				const SkillLevelData& magicLevel = definition.GetLevelData(input.level);

				// A positive fBASIC_VAR is a heal (GLChar.cpp:3087-3091). It is
				// not negative damage, and representing it as such would invert
				// the sign convention for no gain.
				if (magicLevel.basicVar > 0.0f)
				{
					return Refuse(ActiveSkillFailure::UnsupportedEffect);
				}

				// EMFOR_MP / EMFOR_SP (GLChar.cpp:3094-3122) never reach
				// CALCDAMAGE at all - they are their own arithmetic and would
				// need their own tests, not a reuse of the HP path.
				if (definition.applyType != PassiveApplyType::Hp)
				{
					return Refuse(ActiveSkillFailure::UnsupportedEffect);
				}

				// A heal can also hide in the side: legacy EMFOR_HP with a
				// friendly impact is a restorative, not damage.
				if (definition.impactSide != SkillImpactSide::Enemy)
				{
					return Refuse(ActiveSkillFailure::UnsupportedEffect);
				}
			}

			if (definition.targetKind != SkillTargetKind::Spec &&
			    definition.targetKind != SkillTargetKind::Self)
			{
				// TAR_SELF_TOSPEC is a pierce line, TAR_ZONE is an area, and
				// TAR_SPECIFIC names a list. All three need positions and an
				// entity registry the modern server has not got.
				return Refuse(ActiveSkillFailure::UnsupportedTarget);
			}

			if (definition.impactSide != SkillImpactSide::Enemy)
			{
				// SIDE_OUR and SIDE_ANYBODY are buffs. Applying one is a
				// resource-change path, not a combat path.
				return Refuse(ActiveSkillFailure::UnsupportedSide);
			}

			if (definition.applyType != PassiveApplyType::Hp)
			{
				// GLChar.cpp:3094-3123 handles EMFOR_MP and EMFOR_SP with a
				// resistance-reduced drain. It is a different branch with its
				// own element lookup, so it is not folded in here.
				return Refuse(ActiveSkillFailure::UnsupportedEffect);
			}

			// ---- 5. The damage magnitude ----
			//
			// GLChar.cpp:3077-3085 reads a negative `fBASIC_VAR` as damage and
			// a non-negative one as a heal. The heal branch is not this slice.
			//
			// GLogixExPC.cpp:1521-1524 is where the magnitude is formed:
			//   float fSKILL_VAR = sSKILL_DATA.fBASIC_VAR;
			//   int nVAR = abs ( int(fSKILL_VAR*fPOWER) );

			const SkillLevelData& level = definition.GetLevelData(input.level);

			if (!std::isfinite(level.basicVar))
			{
				return Refuse(ActiveSkillFailure::NonFiniteData);
			}

			if (level.basicVar >= 0.0f)
			{
				return Refuse(ActiveSkillFailure::UnsupportedEffect);
			}

			ActiveSkillResult result;
			result.level = input.level;

			const int32_t magnitude = static_cast<int32_t>(
				level.basicVar * input.weatherElementPower);
			result.basicDamage = magnitude < 0 ? -magnitude : magnitude;

			if (result.basicDamage <= 0)
			{
				// Legacy would charge and deal zero. Refusing is the honest
				// outcome for a skill with no magnitude: it says the data is
				// not usable rather than reporting a successful empty cast.
				return Refuse(ActiveSkillFailure::NoDamageMagnitude);
			}

			// ---- 6. Resource validation ----
			//
			// GLogixExPC.cpp:4240-4241. Note the operators differ and both are
			// reproduced: HP is refused when the pool is *equal to or below*
			// the cost, MP when it is strictly below.
			//
			//   if ( m_sHP.dwNow <= sSKILL_DATA.wUSE_HP*wStrikeNum )  return EMSKILL_NOTHP;
			//   if ( m_sMP.dwNow <  sSKILL_DATA.wUSE_MP*wStrikeNum )  return EMSKILL_NOTMP;
			//
			// SP is *not* checked here. A short SP pool does not refuse a cast,
			// it degrades one.

			if (input.currentHp <= level.useHp)
			{
				return Refuse(ActiveSkillFailure::InsufficientHp);
			}

			if (input.currentMp < level.useMp)
			{
				return Refuse(ActiveSkillFailure::InsufficientMp);
			}

			// ---- 7. The target ----

			if (!input.hasTarget)
			{
				return Refuse(ActiveSkillFailure::UnsupportedTarget);
			}

			// ---- 8. SP cost, and the low-SP flag ----
			//
			// GLogixExPC.cpp:4254-4258 for the cost, and the same line for the
			// comparison. VERTICAL-010 fixed the ownership: the pool is the
			// caster's, never the target's.

			result.requiredSP = RequiredSP(input, definition);
			result.lowSp = EvaluateLowSp(input.currentSp, result.requiredSP);

			// ---- 9. What the cast charges ----
			//
			// HP and MP are drawn by ACCOUNTSKILL (GLogixExPC.cpp:4296-4299)
			// as `wUSE_x * wStrikeNum`. Every call site passes a literal 1 for
			// the strike count, so the multiplier is 1 here; see the
			// investigation for why the animation division is not the value
			// RAN actually multiplies by.
			result.hpCost = level.useHp;
			result.mpCost = level.useMp;

			// SP is drawn by SkillProc (GLChar.cpp:3003-3009) and only when the
			// cast is not low-SP:
			//   if ( !bLowSP ) { WORD wDisSP = m_wSUM_DisSP + sSKILL_DATA.wUSE_SP;
			//                     m_sSP.DECREASE ( wDisSP ); }
			//
			// A departure is recorded here. Legacy charges `m_wSUM_DisSP`, which
			// also contains `m_wACCEPTP`; VERTICAL-010 established that the
			// modern model has no ACCEPTP and that the low-SP *gate* does not
			// use it either. The charge is therefore the same value the gate
			// measured - hand contribution plus `wUSE_SP` - so the two agree.
			// It is one term smaller than legacy's.
			result.spCost = result.IsLowSp() ? 0 : result.requiredSP;

			result.cooldownSeconds = CooldownSeconds(definition, input.level,
			                                         input.attackerLevel);

			// ---- 10. The damage, through the existing combat pipeline ----
			//
			// GLogixExPC.cpp:1527-1530 forms the skill's contribution to the
			// damage range:
			//   gdDamage.dwLow  += DWORD (nVAR + ((float) gdDamage.dwLow  * fGrade));
			//   gdDamage.dwHigh += DWORD (nVAR + ((float) gdDamage.dwHigh * fGrade));
			//
			// The grade term is absent: `wGRADE` is
			// `GET_GRADE(EMGRINDING_DAMAGE)` on the right-hand item, and the
			// modern item model has no grade field. So only `nVAR` is added and
			// the attacker's own range is carried through unchanged.
			//
			// Everything after that - defence, critical, crushing, damage
			// reduction, reflection, and the low-SP hit and damage penalties -
			// is VERTICAL-006 through VERTICAL-009's pipeline, which is exactly
			// what a physical skill reuses.

			Stats::DamageRange range = input.attacker.physicalDamage;
			range.low  = static_cast<int32_t>(range.low) + result.basicDamage;
			range.high = static_cast<int32_t>(range.high) + result.basicDamage;

			Combat::CombatInput combat;
			combat.attackerHit            = input.attacker.hit;
			combat.attackerAvoid          = input.attacker.avoid;
			combat.attackerMeleePower     = input.attacker.meleePower;
			combat.attackerShootPower     = input.attacker.shootPower;
			combat.attackerPhysicalDamage = range;
			combat.attackerLevel          = input.attackerLevel;
			combat.attackerMaxHP          = input.attacker.maxHp;
			combat.attackerCurrentHP      = input.currentHp;
			combat.attackerCriticalBonus  = static_cast<int32_t>(input.attacker.criticalRate * 100.0f);
			combat.attackerCrushingBonus  = static_cast<int32_t>(input.attacker.crushingBlow * 100.0f);

			// VERTICAL-012/013: the apply channel carries through to the
			// combat boundary, so the pipeline selects the shoot power for a
			// ranged physical skill, the melee power for a melee one, and the
			// magic calculator for magic.
			combat.attackType = (definition.apply == SkillApply::PhysicalRanged)
			                        ? Combat::AttackType::Ranged
			                        : (definition.apply == SkillApply::Magic)
			                              ? Combat::AttackType::Magic
			                              : Combat::AttackType::Melee;

			// VERTICAL-013: magic. m_wSUM_MA, the skill's own magnitude, and
			// the magic halves of DAMAGE_SPEC.
			combat.attackerMagicPower = input.attacker.magicAttack;
			combat.skillBasicVar      = level.basicVar;
			combat.skillCrushingBonus = static_cast<int32_t>(input.skillCrushingBonus * 100.0f);

			combat.targetHit                 = input.target.hit;
			combat.targetAvoid               = input.target.avoid;
			combat.targetDefense             = input.target.defense;
			combat.targetDefenseBody         = input.target.defenseBody;
			combat.targetDefenseItem         = 0;
			combat.targetLevel               = input.targetLevel;
			combat.targetMaxHP               = input.target.maxHp;
			combat.targetCurrentHP           = input.targetCurrentHp;
			combat.targetStateDamage         = 1.0f;
			combat.targetDamageReduce        = input.target.damageReduce;
			combat.targetDamageReflection    = input.target.damageReflection;
			combat.targetDamageReflectionRate = input.target.damageReflectionRate;

			// VERTICAL-013: element selection and the magic halves of
			// DAMAGE_SPEC.
			//
			// GLogixExPC.cpp:1504-1513:
			//
			//   EMELEMENT emELMT( EMELEMENT_SPIRIT );
			//   if ( emELEMENT == EMELEMENT_ARM ) {
			//       SITEM* pITEM = GET_ELMT_ITEM();
			//       if ( pITEM ) emELMT = STATE_TO_ELEMENT(pITEM->sSuitOp.sBLOW.emTYPE);
			//   } else {
			//       emELMT = emELEMENT;
			//   }
			//
			// The default is Spirit and ArmWeapon only overrides it when a weapon
			// element is actually available, so a missing weapon yields Spirit
			// rather than a refusal.
			SkillElement emELMT = definition.element;
			if (emELMT == SkillElement::ArmWeapon)
			{
				emELMT = (input.weaponElement == SkillElement::ArmWeapon)
				             ? SkillElement::Spirit
				             : input.weaponElement;
			}

			// sRESIST.GetElement(emELMT) (:1515). The clamp to fMAX_RESIST is
			// applied inside the magic calculator.
			combat.targetResistElement = ResolveResistance(input.target.resistances, emELMT);

			combat.targetMagicDamageReduce  = input.targetMagicDamageReduce;
			combat.targetMagicDamageReflection     = input.targetMagicDamageReflection;
			combat.targetMagicDamageReflectionRate = input.targetMagicDamageReflectionRate;
			combat.targetDamageDecrease     = input.targetDamageDecrease;

			// VERTICAL-017: the target's FACT contributions, combined with its own
			// DAMAGE_SPEC values by MAXIMUM, not by sum.
			//
			// GLogixExPC.cpp:2224-2228 rebuilds `m_sDamageSpec` every tick from
			// the passive total, then :2380-2401 max-accumulate the FACT specs
			// into it. Two buffs therefore do not stack - the strongest one wins,
			// and an expired buff simply stops contributing because the whole
			// structure is rebuilt rather than restored.
			//
			// MAX is reproduced here so the reduction and the reflection stay
			// consistent with each other; taking the larger independently would
			// pair a weak amount with a strong rate from a different fact.
			combat.targetDamageReduce =
				Stronger(combat.targetDamageReduce, input.factDamageReduce);
			combat.targetDamageReflection =
				Stronger(combat.targetDamageReflection, input.factDamageReflection);
			combat.targetDamageReflectionRate =
				Stronger(combat.targetDamageReflectionRate, input.factDamageReflectionRate);
			combat.targetMagicDamageReduce =
				Stronger(combat.targetMagicDamageReduce, input.factMagicDamageReduce);
			combat.targetMagicDamageReflection =
				Stronger(combat.targetMagicDamageReflection, input.factMagicDamageReflection);
			combat.targetMagicDamageReflectionRate =
				Stronger(combat.targetMagicDamageReflectionRate, input.factMagicDamageReflectionRate);

			// VERTICAL-010's rule, applied to the caster's own pool.
			combat.attackerRequiredSP = result.requiredSP;
			combat.attackerCurrentSP  = input.currentSp;

			combat.isPK = false;

			combat.brightnessFB       = Modern::Engine::GameBrightFB::Aver;
			combat.weatherElementPower = input.weatherElementPower;

			combat.hitRoll        = input.hitRoll;
			combat.damageRoll     = input.damageRoll;
			combat.criticalRoll   = input.criticalRoll;
			combat.crushingRoll   = input.crushingRoll;
			combat.reflectionRoll = input.reflectionRoll;

			result.combat = Combat::ResolveCombat(combat);
		result.attackTypeUsed = combat.attackType;

		// VERTICAL-014: the state blow verdict.
		//
		// GLChar.cpp:3355-3397 decides `bBLOW` and, if it landed, fills
		// `sSKILLACTEX.sSTATEBLOW` for the target to store. It does not store it
		// locally, and neither does this: the resolver reports the verdict and
		// `ServerCharacter` applies it to the target. That split is what keeps
		// status lifetime out of the cast path.
		if (definition.stateBlow != StatusEffect::StatusEffectType::None)
		{
			StatusEffect::StatusApplicationInput statusInput;
			statusInput.type             = definition.stateBlow;
			statusInput.actRate         = level.blowRate;
			statusInput.lifetime        = level.life;
			statusInput.weatherPower    = input.weatherBlowPower;
			statusInput.var1            = level.blowVar1;
			statusInput.var2            = level.blowVar2;
			statusInput.attackerLevel   = input.attackerLevel;
			statusInput.targetLevel     = input.targetLevel;
			statusInput.targetResist    = input.targetBlowResist;
			statusInput.targetDisorderMask = input.targetDisorderMask;
			statusInput.randomRoll      = input.statusRandomRoll;

			result.statusApplication = StatusEffect::ResolveStatusApplication(statusInput);
			result.hasStatusApplication = true;
		}

		// VERTICAL-015: build the FACT record this cast produces.
		//
		// Mirrors `GLChar::RECEIVE_SKILLFACT` (GLChar.cpp:6519-6613): copy the
		// basic type/value, every impact and every spec at this level, then
		// stamp the identity, the level and the lifetime. The `bHOLD` gate is
		// re-evaluated here so a definition claiming a FACT but carrying nothing
		// produces no record.
		if (definition.createsFact)
		{
			SkillFact fact;
			fact.skillId           = definition.id;
			fact.level             = input.level;
			fact.remainingLifetime = level.life;   // sSKILL_DATA.fLIFE
			fact.basicType         = definition.applyType;
			fact.basicValue        = level.basicVar;

			for (uint8_t i = 0; i < kSkillFactMaxImpacts && i < kMaxSkillImpacts; ++i)
			{
				const SkillFactImpactEntry& src = definition.factImpacts[i];
				if (src.type == SkillFactImpactType::None)
				{
					continue;
				}
				fact.impacts[i].type  = src.type;
				fact.impacts[i].value = src.values[input.level];
			}

			for (uint8_t i = 0; i < kSkillFactMaxSpecs && i < kMaxSkillSpecs; ++i)
			{
				const SkillFactSpecEntry& src = definition.factSpecs[i];
				if (src.type == SkillFactSpecType::None)
				{
					continue;
				}
				fact.specs[i].type     = src.type;
				fact.specs[i].var1     = src.var1[input.level];
				fact.specs[i].var2     = src.var2[input.level];
				fact.specs[i].specFlag = src.specFlag[input.level];
				fact.specs[i].nativeId = src.nativeId[input.level];
			}

			// Legacy stamps the caster from the network message
			// (`_wCasterCrow`, `_dwCasterID`, GLChar.cpp:6516-6517). There is no
			// transport here, so the caster comes in on the input and defaults to
			// unset rather than being invented.
			fact.casterCrow = input.factCasterCrow;
			fact.casterId   = input.factCasterId;

			// The `bHOLD` gate.
			if (FactHoldsAnything(fact))
			{
				result.skillFact    = fact;
				result.hasSkillFact = true;
			}
		}

			return result;
		}
	}
}
