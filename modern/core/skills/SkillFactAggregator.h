#pragma once

// VERTICAL-015: FACT modifier aggregation.
//
// Reproduces the single pass in `GLogixExPC.cpp:2276-2410`. Two structural
// facts about that function drive this design:
//
//   1. The accumulators are reset to their defaults at the TOP of the function
//      (`m_bProhibitSkill = false;` at :2256, and the rest just above), and the
//      whole set is rebuilt from the live pool on every call. So expiry needs no
//      "restore" step - a fact that disappears simply stops contributing, and
//      the next pass produces the base values again.
//
//   2. Ticking and aggregation are the SAME loop. A fact is decremented and
//      disabled at :2292-2295, but the switch statements below still run for
//      that iteration, because `DISABLESKEFF` only nulls the skill id and the
//      loop already passed its `continue` guard. **A fact therefore still
//      contributes on the tick it expires.** That off-by-one is real behaviour
//      and is reproduced rather than tidied away.
//
// Only the specs and impacts whose consumption is fully traced are aggregated.
// Everything else is stored faithfully but ignored here, which is recorded in
// the investigation as DEFERRED rather than guessed at.

#include "SkillFactContainer.h"

namespace Modern::Skills
{
	// The aggregated, authoritative modifier state.
	//
	// Every field here corresponds to a traced legacy accumulation. Nothing is
	// carried speculatively.
	struct SkillFactModifiers
	{
		// EMSPECA_MOVEVELO: `m_fSKILL_MOVE += fSPECVAR1`
		// (GLogixExPC.cpp:2360). Plain additive - a positive value speeds up.
		float moveVelocity = 0.0f;

		// EMSPECA_ATTACKVELO: `m_fATTVELO -= fSPECVAR1`
		// (GLogixExPC.cpp:2364). SIGN-INVERTED: a positive fSPECVAR1 SLOWS the
		// attacker. Legacy's own comment records that a -0.1 value is entered to
		// make things 10% faster, which only makes sense with the subtraction.
		float attackVelocity = 0.0f;

		// EMSPECA_NONBLOW: `m_dwHOLDBLOW = dwSPECFLAG`
		// (GLogixExPC.cpp:2357). An ASSIGNMENT, not an OR: with two such specs
		// the last one aggregated wins outright. Reproduced as-is.
		//
		// This is the value VERTICAL-014's StatusEffectResolver consumes as
		// `targetDisorderMask`. It is an immunity mask, unrelated to which
		// states are currently active.
		uint32_t statusImmunityMask = 0u;

		// EMSPECA_PROHIBIT_SKILL: `m_bProhibitSkill = true` (:2377)
		bool prohibitSkill = false;

		// EMSPECA_PROHIBIT_POTION: `m_bProhibitPotion = true` (:2374)
		bool prohibitPotion = false;

		// EMSPECA_PSY_DAMAGE_REDUCE / EMSPECA_MAGIC_DAMAGE_REDUCE
		// (:2379-2387): `if ( current < fSPECVAR1 ) current = fSPECVAR1`.
		// The strongest wins; values do NOT stack.
		float psyDamageReduce   = 0.0f;
		float magicDamageReduce = 0.0f;

		// EMSPECA_PSY_DAMAGE_REFLECTION / EMSPECA_MAGIC_DAMAGE_REFLECTION
		// (:2389-2401): the amount AND the rate are taken together from the
		// single strongest spec, and only when that spec's amount beats the
		// current one. The two are never mixed between different facts.
		float psyDamageReflection      = 0.0f;
		float psyDamageReflectionRate  = 0.0f;
		float magicDamageReflection      = 0.0f;
		float magicDamageReflectionRate  = 0.0f;

		// VERTICAL-017: attack-power impacts.
		//
		// GLogixExPC.cpp:2343-2345, SUM with `int()` truncation. Kept as signed
		// accumulators because legacy's `nSUM_*` are `int` and the final sum is
		// folded into a 16-bit power by `VariationClamped` in the stat layer,
		// which is where the wrap/clamp behaviour lives.
		//
		// These are NOT the same thing as any passive, item or base
		// contribution - see the struct comment above.
		int32_t meleePower = 0;
		int32_t shootPower = 0;
		int32_t magicAttack = 0;

		// VERTICAL-019: the hit/avoid accumulators and the range damage.
		//
		// `hit` is attacker-side (it is the caster's own hit), `avoid` is
		// defender-side. Both feed the stat pipeline, exactly like the powers.
		//
		// `damage` is NOT a damage figure: legacy stores it as a VAR_PARAM on
		// `m_gdDAMAGE_SKILL`, so it is carried as the signed integer the legacy
		// `int()` produced and applied to both range ends at the combat boundary.
		int32_t hit = 0;
		int32_t avoid = 0;
		int32_t damage = 0;

		// VERTICAL-020: flat total defence and the all-axes resistance bonus.
		int32_t defense = 0;
		int32_t resist = 0;
	};

	struct SkillFactAdvanceResult
	{
		SkillFactModifiers modifiers{};
		uint32_t expiredCount = 0;
	};

	// The one-pass advance + aggregate, equivalent to legacy's function.
	//
	// `container` is taken by reference because the legacy loop mutates the pool
	// (the `fAGE -=` at :2292 and `DISABLESKEFF` at :2295) and accumulates in
	// the same breath.
	inline SkillFactAdvanceResult AdvanceSkillFacts(SkillFactContainer& container,
	                                                float elapsedSeconds) noexcept
	{
		// The accumulators are rebuilt from their defaults on every call
		// (GLogixExPC.cpp:2256 and the lines above it). Nothing persists.
		SkillFactModifiers modifiers{};
		uint32_t expired = 0;

		for (uint8_t i = 0; i < kSkillFactSlotCount; ++i)
		{
			SkillFact& fact = *container.MutableAt(i);
			if (!fact.Occupied())
			{
				continue;
			}

			// GLogixExPC.cpp:2292-2295.
			fact.remainingLifetime -= elapsedSeconds;

			bool expiredThisTick = false;
			if (fact.remainingLifetime <= 0.0f)
			{
				// DISABLESKEFF nulls the id; the modifiers below still run for
				// this iteration, which is the off-by-one described at the top
				// of this file. The slot is emptied here so the NEXT pass skips
				// it at its `continue` guard.
				expiredThisTick = true;
				++expired;
			}

			// ── Impacts ───────────────────────────────────────────────────────
			//
			// VERTICAL-017: PA/SA/MA are aggregated now, because the consumer is
			// PROVEN and an authoritative owner exists.
			//
			// GLogixExPC.cpp:2343-2345:
			//   case EMIMPACTA_PA:  nSUM_PA += int(fADDON_VAR);  break;
			//   case EMIMPACTA_SA:  nSUM_SA += int(fADDON_VAR);  break;
			//   case EMIMPACTA_MA:  nSUM_MA += int(fADDON_VAR);  break;
			//
			// SUM, and `int()`-truncated, so two facts add rather than compete.
			// Legacy sums these into a variable that is separate from
			// `m_sSUM_PASSIVE.m_nMA` and only adds them at the point of use
			// (:2970-2972), which is why they must stay in their own
			// contribution rather than being merged into a passive one.
			for (const SkillFactImpact& impact : fact.impacts)
			{
				switch (impact.type)
				{
					case SkillFactImpactType::Pa:
						modifiers.meleePower += static_cast<int32_t>(impact.value);
						break;
					case SkillFactImpactType::Sa:
						modifiers.shootPower += static_cast<int32_t>(impact.value);
						break;
					case SkillFactImpactType::Ma:
						modifiers.magicAttack += static_cast<int32_t>(impact.value);
						break;

					// VERTICAL-019, the remaining three proven consumers.
					//
					// GLogixExPC.cpp:2327-2329
					//   m_nSUM_HIT   += int(fADDON_VAR);
					//   m_nSUM_AVOID += int(fADDON_VAR);
					//   m_gdDAMAGE_SKILL.VAR_PARAM( int(fADDON_VAR) );
					//
					// All three are SUM. The first two are flat integers; DAMAGE
					// is a range modifier, carried as an integer here and applied
					// through `ApplyAttackPower` at the combat boundary, because
					// legacy modifies BOTH ends of the range BEFORE the roll -
					// not the rolled figure.
					case SkillFactImpactType::HitRate:
						modifiers.hit += static_cast<int32_t>(impact.value);
						break;
					case SkillFactImpactType::AvoidRate:
						modifiers.avoid += static_cast<int32_t>(impact.value);
						break;
					case SkillFactImpactType::Damage:
						modifiers.damage += static_cast<int32_t>(impact.value);
						break;

					// VERTICAL-020: GLogixExPC.cpp:2330 and :2349
					//   m_nDEFENSE_SKILL    += int(fADDON_VAR);
					//   m_sSUMRESIST_SKILL  += int(fADDON_VAR);
					// Both SUM with int() truncation.
					case SkillFactImpactType::Defense:
						modifiers.defense += static_cast<int32_t>(impact.value);
						break;
					case SkillFactImpactType::Resist:
						modifiers.resist += static_cast<int32_t>(impact.value);
						break;

					default:
						break;
				}
			}

			// Every OTHER EMIMPACTA_* consumer at GLogixExPC.cpp:2327-2349 is
			// still deferred: DAMAGE_RATE feeds the damage pipeline, DEFENSE and
			// RESIST feed
			// subsystems whose modern boundary is undecided, and the VAR*/RATE*
			// families feed a recovery loop and maximum recalculation this
			// milestone has not proven. The six recovery/CP impacts have no case
			// in the switch at all. Values are preserved in the record so a later
			// slice needs no re-derivation. See the VERTICAL-016 and
			// VERTICAL-017 investigations.

			// ── Specs (GLogixExPC.cpp:2353-2408) ───────────────────────────
			for (const SkillFactSpec& spec : fact.specs)
			{
				switch (spec.type)
				{
					case SkillFactSpecType::NonBlow:
						// Assignment, not OR. See the field comment.
						modifiers.statusImmunityMask = spec.specFlag;
						break;

					case SkillFactSpecType::MoveVelo:
						modifiers.moveVelocity += spec.var1;
						break;

					case SkillFactSpecType::AttackVelo:
						// Subtracted. A positive value slows the attacker.
						modifiers.attackVelocity -= spec.var1;
						break;

					case SkillFactSpecType::ProhibitSkill:
						modifiers.prohibitSkill = true;
						break;

					case SkillFactSpecType::ProhibitPotion:
						modifiers.prohibitPotion = true;
						break;

					case SkillFactSpecType::PsyDamageReduce:
						if (modifiers.psyDamageReduce < spec.var1)
						{
							modifiers.psyDamageReduce = spec.var1;
						}
						break;

					case SkillFactSpecType::MagicDamageReduce:
						if (modifiers.magicDamageReduce < spec.var1)
						{
							modifiers.magicDamageReduce = spec.var1;
						}
						break;

					case SkillFactSpecType::PsyDamageReflection:
						if (modifiers.psyDamageReflection < spec.var1)
						{
							modifiers.psyDamageReflection     = spec.var1;
							modifiers.psyDamageReflectionRate = spec.var2;
						}
						break;

					case SkillFactSpecType::MagicDamageReflection:
						if (modifiers.magicDamageReflection < spec.var1)
						{
							modifiers.magicDamageReflection     = spec.var1;
							modifiers.magicDamageReflectionRate = spec.var2;
						}
						break;

					default:
						break;
				}
			}

			// The fact is removed only AFTER its contribution has been
			// collected, which is what produces the extra tick.
			if (expiredThisTick)
			{
				container.Remove(fact.skillId);
			}
		}

		SkillFactAdvanceResult result;
		result.modifiers    = modifiers;
		result.expiredCount = expired;
		return result;
	}

	// Read-only aggregation over a pool that has already been ticked.
	//
	// Provided for callers that tick through `SkillFactContainer::Tick` instead.
	// It differs from `AdvanceSkillFacts` in exactly the way legacy does: a fact
	// whose lifetime has gone negative is skipped here, because the slot was
	// already emptied.
	inline SkillFactModifiers AggregateSkillFacts(const SkillFactContainer& container) noexcept
	{
		SkillFactModifiers modifiers{};

		for (uint8_t i = 0; i < kSkillFactSlotCount; ++i)
		{
			const SkillFact* slot = container.At(i);
			if (slot == nullptr || !slot->Occupied())
			{
				continue;
			}

			for (const SkillFactImpact& impact : slot->impacts)
			{
				switch (impact.type)
				{
					case SkillFactImpactType::Pa:
						modifiers.meleePower += static_cast<int32_t>(impact.value);
						break;
					case SkillFactImpactType::Sa:
						modifiers.shootPower += static_cast<int32_t>(impact.value);
						break;
					case SkillFactImpactType::Ma:
						modifiers.magicAttack += static_cast<int32_t>(impact.value);
						break;
					case SkillFactImpactType::HitRate:
						modifiers.hit += static_cast<int32_t>(impact.value);
						break;
					case SkillFactImpactType::AvoidRate:
						modifiers.avoid += static_cast<int32_t>(impact.value);
						break;
					case SkillFactImpactType::Damage:
						modifiers.damage += static_cast<int32_t>(impact.value);
						break;
					case SkillFactImpactType::Defense:
						modifiers.defense += static_cast<int32_t>(impact.value);
						break;
					case SkillFactImpactType::Resist:
						modifiers.resist += static_cast<int32_t>(impact.value);
						break;
					default:
						break;
				}
			}

			for (const SkillFactSpec& spec : slot->specs)
			{
				switch (spec.type)
				{
					case SkillFactSpecType::NonBlow:
						modifiers.statusImmunityMask = spec.specFlag;
						break;
					case SkillFactSpecType::MoveVelo:
						modifiers.moveVelocity += spec.var1;
						break;
					case SkillFactSpecType::AttackVelo:
						modifiers.attackVelocity -= spec.var1;
						break;
					case SkillFactSpecType::ProhibitSkill:
						modifiers.prohibitSkill = true;
						break;
					case SkillFactSpecType::ProhibitPotion:
						modifiers.prohibitPotion = true;
						break;
					case SkillFactSpecType::PsyDamageReduce:
						if (modifiers.psyDamageReduce < spec.var1)
						{
							modifiers.psyDamageReduce = spec.var1;
						}
						break;
					case SkillFactSpecType::MagicDamageReduce:
						if (modifiers.magicDamageReduce < spec.var1)
						{
							modifiers.magicDamageReduce = spec.var1;
						}
						break;
					case SkillFactSpecType::PsyDamageReflection:
						if (modifiers.psyDamageReflection < spec.var1)
						{
							modifiers.psyDamageReflection     = spec.var1;
							modifiers.psyDamageReflectionRate = spec.var2;
						}
						break;
					case SkillFactSpecType::MagicDamageReflection:
						if (modifiers.magicDamageReflection < spec.var1)
						{
							modifiers.magicDamageReflection     = spec.var1;
							modifiers.magicDamageReflectionRate = spec.var2;
						}
						break;
					default:
						break;
				}
			}
		}

		return modifiers;
	}
}