#pragma once

// CORE-002: the three contribution sources RAN's stat pipeline sums.
//
// These are the inputs, not the systems that produce them. CORE-002 does not
// implement equipment, passive skills or the codex; it reproduces the
// arithmetic that consumes what they produce, so those systems can be built
// later against a verified contract.
//
// The three exist because the verified legacy chain has exactly three
// independent summations feeding the derived stats, plus one table. No
// stacking, priority or ordering rule is implemented between them, because
// none was found: RAN simply adds them at the point of use.
//
// Legacy origin:
//
//   legacy/Lib_Client/G-Logic/GLogicEx.h:104   SSUM_ITEM
//   legacy/Lib_Client/G-Logic/GLCharData.h:1123 SPASSIVE_SKILL_DATA
//   legacy/Lib_Client/G-Logic/GLCharData.h:721  the eleven m_dw*Increase fields
//   legacy/Lib_Client/G-Logic/GLCharDefine.h:~700 SRESIST
//
// Only the fields the verified formulas read are carried. The rest of
// SSUM_ITEM and SPASSIVE_SKILL_DATA (movement speed, attack speed, critical,
// crushing blow, damage reduction, pierce, skill ranges, summoning time) are
// not part of this pipeline and are not reproduced here.

#include "stats/BaseStats.h"

#include <cstdint>

namespace Modern::Stats
{
	// The five elemental resistances RAN tracks, in legacy SRESIST order.
	//
	// RAN clamps each element to zero or greater after summing
	// (SRESIST::LIMIT), so Resistances::ClampNonNegative reproduces that and
	// the calculator applies it.
	struct Resistances
	{
		int32_t fire     = 0;
		int32_t ice      = 0;
		int32_t electric = 0;
		int32_t poison   = 0;
		int32_t spirit   = 0;

		constexpr bool operator==(const Resistances& other) const noexcept
		{
			return fire == other.fire && ice == other.ice && electric == other.electric &&
			       poison == other.poison && spirit == other.spirit;
		}

		constexpr void ClampNonNegative() noexcept
		{
			if (fire     < 0) { fire     = 0; }
			if (ice      < 0) { ice      = 0; }
			if (electric < 0) { electric = 0; }
			if (poison   < 0) { poison   = 0; }
			if (spirit   < 0) { spirit   = 0; }
		}

		// SRESIST::operator+=(int) adds the same scalar to all five.
		constexpr void AddAll(int32_t value) noexcept
		{
			fire     += value;
			ice      += value;
			electric += value;
			poison   += value;
			spirit   += value;
		}
	};

	// What equipped items add, as RAN's SUM_ITEM accumulates it.
	//
	// The recovery block has two independent halves, and VERTICAL-005 found
	// that the second one had been documented away rather than modelled.
	//
	// `hpRecoveryRate` (fIncR_HP) is the *rate*: a fraction of the resource
	// maximum, added to the class constant to form `DerivedStats::hpRecoveryRate`.
	//
	// `hpRecoveryFlat` (fInc_HP) is the *absolute* per-unit amount, added
	// separately in the same expression (GLogixExPC.cpp:3020-3022). An earlier
	// comment here said fInc_HP "is not part of the stat pipeline"; it is the
	// flat half of the pipeline's recovery term, and it is now summed into
	// `DerivedStats::hpRecoveryFlat` alongside `GLCONST_CHAR::fHP_INC`.
	struct ItemContribution
	{
		// sStats: flat bonuses to the six base stats. These are added into the
		// stat sum as 16-bit unsigned values, so they wrap like everything else.
		BaseStats stats;

		// Flat resource bonuses: nHP, nMP, nSP.
		int32_t hp = 0;
		int32_t mp = 0;
		int32_t sp = 0;

		// Recovery rates: fIncR_HP, fIncR_MP, fIncR_SP. Added to the class
		// constants to form the derived recovery rate.
		float hpRecoveryRate = 0.0f;
		float mpRecoveryRate = 0.0f;
		float spRecoveryRate = 0.0f;

		// VERTICAL-005: the absolute per-unit recovery, fInc_HP / fInc_MP /
		// fInc_SP. See the note above. Added to `GLCONST_CHAR::fHP_INC` and
		// friends, not to the rate.
		float hpRecoveryFlat = 0.0f;
		float mpRecoveryFlat = 0.0f;
		float spRecoveryFlat = 0.0f;

		// Attack-power style flat bonuses: nPA, nSA, nMA. Applied through
		// RAN's VARIATION clamp against the stat-derived value.
		int32_t meleePower  = 0;
		int32_t shootPower  = 0;
		int32_t magicAttack = 0;

		// Hit and avoid: nHitRate, nAvoidRate.
		int32_t hit   = 0;
		int32_t avoid = 0;

		// Percentage modifiers on the already-computed hit and avoid:
		// fRateHit_Per, fRateAvoid_Per. RAN applies
		// int(value * (100 + percent) * 0.01f), so these are percents, not
		// fractions, and a value of 10 means +10%.
		float hitRatePercent   = 0.0f;
		float avoidRatePercent = 0.0f;

		// Defence: nDefense.
		int32_t defense = 0;

		// Damage range added to the computed attack: gdDamage.dwLow, dwMax.
		int32_t damageLow  = 0;
		int32_t damageHigh = 0;

		// Resistances: sResist.
		Resistances resistances;

		// VERTICAL-007: combat modifiers from items.
		//
		// Legacy: m_sSUMITEM.fIncR_Critical, m_sSUMITEM.fIncR_CrushingBlow
		// (GLogixExPC.cpp:570-574, 595-596)
		//
		// These are rates (floats), not percentage points. The legacy
		// conversion happens at the point of use.
		float criticalRate = 0.0f;
		float crushingBlow = 0.0f;

		// VERTICAL-007: damage reduction and reflection from items.
		//
		// Legacy: sDamageSpec.m_fPsyDamageReduce, m_fPsyDamageReflection,
		// m_fPsyDamageReflectionRate (GLogixExPC.cpp:1409-1411)
		float damageReduce = 0.0f;
		float damageReflection = 0.0f;
		float damageReflectionRate = 0.0f;

		// VERTICAL-010: the equipment part of the required SP, the modern
		// equivalent of legacy m_wSUM_DisSP's wReqSP terms.
		//
		// Legacy: GLogixExPC.cpp:428-434
		//     m_wSUM_DisSP = m_wACCEPTP;
		//     if ( pRHAND )  m_wSUM_DisSP += pRHAND->sSuitOp.wReqSP;
		//     if ( pLHAND )  m_wSUM_DisSP += pLHAND->sSuitOp.wReqSP;
		//
		// Only the two hand slots contribute, so the aggregator fills this
		// selectively rather than summing every equipped item. `WORD`
		// arithmetic in legacy wraps at 16 bits and so does this, matching the
		// six base stats summed alongside it.
		//
		// m_wACCEPTP is deliberately absent: it is a stat-deficit penalty
		// computed from per-item requirement fields the modern item model does
		// not carry, and the legacy low-SP gate does not use it either. See
		// docs/reference/client/VERTICAL-010_REQUIRED_SP_INVESTIGATION.md.
		uint16_t requiredSP = 0;

		constexpr bool operator==(const ItemContribution& other) const noexcept
		{
			return stats == other.stats && hp == other.hp && mp == other.mp && sp == other.sp &&
			       hpRecoveryRate == other.hpRecoveryRate && mpRecoveryRate == other.mpRecoveryRate &&
			       spRecoveryRate == other.spRecoveryRate &&
			       hpRecoveryFlat == other.hpRecoveryFlat &&
			       mpRecoveryFlat == other.mpRecoveryFlat &&
			       spRecoveryFlat == other.spRecoveryFlat &&
			       meleePower == other.meleePower &&
			       shootPower == other.shootPower && magicAttack == other.magicAttack &&
			       hit == other.hit && avoid == other.avoid &&
			       hitRatePercent == other.hitRatePercent &&
			       avoidRatePercent == other.avoidRatePercent && defense == other.defense &&
			       damageLow == other.damageLow && damageHigh == other.damageHigh &&
			       resistances == other.resistances &&
			       criticalRate == other.criticalRate &&
			       crushingBlow == other.crushingBlow &&
			       damageReduce == other.damageReduce &&
			       damageReflection == other.damageReflection &&
			       damageReflectionRate == other.damageReflectionRate &&
			       requiredSP == other.requiredSP;
		}
	};

	// What learned passive skills add, as RAN's SUM_PASSIVE accumulates it.
	//
	// SUM_PASSIVE is driven by SKILL::EMBASIC_TYPE, and every case converts
	// the skill's float effect with a cast before storing: DWORD for the
	// resource bonuses and int for the rest. Those truncations happen while
	// summing, upstream of this calculation, so the values here are already
	// integral and the calculator adds them as-is.
	struct PassiveContribution
	{
		// m_nHP, m_nMP, m_nSP: flat resource bonuses.
		int32_t hp = 0;
		int32_t mp = 0;
		int32_t sp = 0;

		// m_fHP_RATE, m_fMP_RATE, m_fSP_RATE: multiplicative rate on the
		// resource maximum, applied as (1 + rate).
		float hpRate = 0.0f;
		float mpRate = 0.0f;
		float spRate = 0.0f;

		// m_fINCR_HP, m_fINCR_MP, m_fINCR_SP: flat additions to the recovery
		// rate, on top of the class constant and the item contribution.
		float hpRecoveryRate = 0.0f;
		float mpRecoveryRate = 0.0f;
		float spRecoveryRate = 0.0f;

		// m_nPA, m_nSA, m_nMA: flat attack-power bonuses.
		int32_t meleePower  = 0;
		int32_t shootPower  = 0;
		int32_t magicAttack = 0;

		// m_nHIT, m_nAVOID, m_nDEFENSE, m_nDAMAGE.
		int32_t hit     = 0;
		int32_t avoid   = 0;
		int32_t defense = 0;
		int32_t damage  = 0;

		// m_sSUMRESIST.
		Resistances resistances;

		// VERTICAL-007: combat modifiers from passive skills.
		//
		// Legacy: sSKILL_SPEC.fVAR2 for crushing blow
		// (GLogixExPC.cpp:1499), and passive contributions to
		// DAMAGE_SPEC fields (GLogixExPC.cpp:1409-1411).
		float criticalRate = 0.0f;
		float crushingBlow = 0.0f;
		float damageReduce = 0.0f;
		float damageReflection = 0.0f;
		float damageReflectionRate = 0.0f;

		constexpr bool operator==(const PassiveContribution& other) const noexcept
		{
			return hp == other.hp && mp == other.mp && sp == other.sp &&
			       hpRate == other.hpRate && mpRate == other.mpRate && spRate == other.spRate &&
			       hpRecoveryRate == other.hpRecoveryRate && mpRecoveryRate == other.mpRecoveryRate &&
			       spRecoveryRate == other.spRecoveryRate && meleePower == other.meleePower &&
			       shootPower == other.shootPower && magicAttack == other.magicAttack &&
			       hit == other.hit && avoid == other.avoid && defense == other.defense &&
			       damage == other.damage && resistances == other.resistances &&
			       criticalRate == other.criticalRate &&
			       crushingBlow == other.crushingBlow &&
			       damageReduce == other.damageReduce &&
			       damageReflection == other.damageReflection &&
			       damageReflectionRate == other.damageReflectionRate;
		}
	};

	// What the codex adds: eleven flat, unsigned bonuses.
	//
	// Every one of these is added at the very end of its own formula, after
	// any percentage and after the truncation to the destination type, so a
	// codex bonus is a flat lift that cannot be diluted by a rate and is never
	// truncated away. They are unsigned in RAN (DWORD), so a codex contribution
	// can only raise a value; modelling them as signed would let a caller lower
	// one, which the legacy type cannot express.
	struct CodexContribution
	{
		uint32_t hp           = 0;
		uint32_t mp           = 0;
		uint32_t sp           = 0;
		uint32_t attack       = 0;
		uint32_t defense      = 0;
		uint32_t shootPower   = 0;
		uint32_t meleePower   = 0;
		uint32_t magicAttack  = 0;
		uint32_t resistance   = 0;
		uint32_t hit          = 0;
		uint32_t avoid        = 0;

		constexpr bool operator==(const CodexContribution& other) const noexcept
		{
			return hp == other.hp && mp == other.mp && sp == other.sp && attack == other.attack &&
			       defense == other.defense && shootPower == other.shootPower &&
			       meleePower == other.meleePower && magicAttack == other.magicAttack &&
			       resistance == other.resistance && hit == other.hit && avoid == other.avoid;
		}
	};

	// True when every float in a contribution is finite. The calculator refuses
	// a contribution containing NaN or an infinity rather than propagating it
	// into a derived value.
	bool IsFinite(const ItemContribution& value) noexcept;
	bool IsFinite(const PassiveContribution& value) noexcept;

	// True when a contribution asks for nothing.
	//
	// Needed where a parameter exists only for source compatibility and must be
	// refused when it carries a value: the check has to name every field, and a
	// check that silently misses a newly added one is worse than none. Written
	// as a comparison against a default-constructed value so it cannot fall
	// behind the struct.
	bool IsZero(const ItemContribution& value) noexcept;
	bool IsZero(const PassiveContribution& value) noexcept;

	// Same check for the codex block, and needed for the same reason as the other
	// two. VERTICAL-001 accepted a caller-supplied codex contribution, and its
	// VERTICAL-004 successor refuses one: the completed codex set is the only
	// source for it, so a caller offering a value has to be told no rather than
	// have it accepted and dropped.
	bool IsZero(const CodexContribution& value) noexcept;
}
