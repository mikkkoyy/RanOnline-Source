#pragma once

// CORE-002: the derived statistics RAN computes in one pass.
//
// Every field here is an output of a verified line in
// legacy/Lib_Client/G-Logic/GLogixExPC.cpp:286 `GLCHARLOGIC::SUM_ADDITION`.
// Nothing is present because it is common in an MMORPG; the provenance of each
// field is given.
//
// Note what is deliberately absent. Combat point, movement speed, attack
// speed, critical, pierce, skill and apply ranges, `m_wACCEPTP` (stat cost of
// the worn weapons) and `m_wSUM_DisSP` (SP consumed) all appear in the same
// legacy function, and are all absent here:
//
//   - combat point is a fixed constant with no contribution source, so there
//     is nothing to calculate;
//   - the rest are equipment, weapon and animation state, or belong to a
//     future movement or progression system.
//
// The stat *sum* and the attack/defence *points* are present because they are
// inputs to other derived values in the same pass and a caller cannot verify
// the pipeline without seeing them.

#include "stats/BaseStats.h"
#include "stats/Contributions.h"

#include <cstdint>

namespace Modern::Stats
{
	// A low/high pair, RAN's GLDWDATA damage range.
	struct DamageRange
	{
		uint32_t low  = 0;
		uint32_t high = 0;

		constexpr bool operator==(const DamageRange& other) const noexcept
		{
			return low == other.low && high == other.high;
		}
	};

	// The result of one stat calculation.
	//
	// Types are the legacy destination types, because the truncation that
	// produces them is part of the behaviour. In particular `meleePower`,
	// `shootPower` and `magicAttack` are 16-bit and have already passed through
	// RAN's VARIATION clamp, so a caller that widened them would be reporting
	// something RAN never computed.
	struct DerivedStats
	{
		// m_sSUMSTATS: the six base stats after class begin stats, the
		// per-level growth, the character's allocated points and equipment.
		// 16-bit and wrapping, as in RAN.
		BaseStats totalStats;

		// m_sHP.dwMax / m_sMP.dwMax / m_sSP.dwMax.
		// The truncation to 32 bits happens twice in RAN — once on the raw sum
		// and again after the rate and point-rate multiplication — and both are
		// reproduced.
		uint32_t maxHp = 0;
		uint32_t maxMp = 0;
		uint32_t maxSp = 0;

		// m_fINCR_HP / m_fINCR_MP / m_fINCR_SP: a rate, not a per-second
		// amount. RAN adds the class constant, the item contribution and the
		// passive contribution; the consumer that turns a rate into an amount
		// is the resource system, VERTICAL-005.
		float hpRecoveryRate = 0.0f;
		float mpRecoveryRate = 0.0f;
		float spRecoveryRate = 0.0f;

		// VERTICAL-005: the absolute per-unit half of the same recovery term,
		// `GLCONST_CHAR::fHP_INC + m_sSUMITEM.fInc_HP` and its MP and SP
		// counterparts (GLogixExPC.cpp:3020-3022).
		//
		// The legacy recovery amount is a *sum of two different units*:
		//
		//   fINC_HP = fElap * ( maxHP * hpRecoveryRate + hpRecoveryFlat )
		//
		// so it cannot be folded into `hpRecoveryRate` without changing what the
		// number means. They are reported separately for the same reason
		// `maxHp` and `physicalDamage` are reported rather than summarised.
		//
		// The three `GLCONST_CHAR::*_INC` constants are zero in the shipped
		// data (GLogicData.cpp:256-258), so in practice this is the item sum.
		float hpRecoveryFlat = 0.0f;
		float mpRecoveryFlat = 0.0f;
		float spRecoveryFlat = 0.0f;

		// m_wSUM_AP / m_wSUM_DP: stat points and defence points after the class
		// table's conversion. The derived damage and defence are built on these,
		// so they are reported.
		uint16_t attackPoint  = 0;
		uint16_t defensePoint = 0;

		// m_wPA / m_wSA / m_wMA after the VARIATION clamp to [0, 65535].
		// These are the melee, ranged and magic attack values the rest of the
		// client reads; combat resolution is not implemented.
		uint16_t meleePower  = 0;
		uint16_t shootPower  = 0;
		uint16_t magicAttack = 0;

		// m_nHIT / m_nAVOID after the percentage modifier.
		int32_t hit   = 0;
		int32_t avoid = 0;

		// m_nDEFENSE_BODY: defence before equipment, passive and codex.
		int32_t defenseBody = 0;

		// m_nDEFENSE (== m_nDEFENSE_SKILL): the final defence.
		int32_t defense = 0;

		// m_gdDAMAGE_PHYSIC: the physical attack range, with the attack power
		// added and RAN's floor of 1 applied.
		//
		// RAN picks melee or shoot power here based on the equipped weapon's
		// range. CORE-002 has no equipment, so the melee branch is used; the
		// ranged branch differs only in substituting shootPower. See
		// StatCalculator.h.
		DamageRange physicalDamage;

		// m_sSUMRESIST after summing and clamping each element to zero or more.
		Resistances resistances;

		// VERTICAL-007: combat modifiers derived from items and passives.
		//
		// Legacy: m_sSUMITEM.fIncR_Critical + m_sSUM_PASSIVE critical rate
		// (GLogixExPC.cpp:570, 1620)
		//
		// These are rates (floats), not percentage points. The legacy
		// conversion happens at the point of use in combat.
		float criticalRate = 0.0f;
		float crushingBlow = 0.0f;
		float damageReduce = 0.0f;
		float damageReflection = 0.0f;
		float damageReflectionRate = 0.0f;

		constexpr bool operator==(const DerivedStats& other) const noexcept
		{
			return totalStats == other.totalStats && maxHp == other.maxHp &&
			       maxMp == other.maxMp && maxSp == other.maxSp &&
			       hpRecoveryRate == other.hpRecoveryRate &&
			       mpRecoveryRate == other.mpRecoveryRate &&
			       spRecoveryRate == other.spRecoveryRate &&
			       hpRecoveryFlat == other.hpRecoveryFlat &&
			       mpRecoveryFlat == other.mpRecoveryFlat &&
			       spRecoveryFlat == other.spRecoveryFlat &&
			       attackPoint == other.attackPoint && defensePoint == other.defensePoint &&
			       meleePower == other.meleePower && shootPower == other.shootPower &&
			       magicAttack == other.magicAttack && hit == other.hit && avoid == other.avoid &&
			       defenseBody == other.defenseBody && defense == other.defense &&
			       physicalDamage == other.physicalDamage && resistances == other.resistances &&
			       criticalRate == other.criticalRate &&
			       crushingBlow == other.crushingBlow &&
			       damageReduce == other.damageReduce &&
			       damageReflection == other.damageReflection &&
			       damageReflectionRate == other.damageReflectionRate;
		}
	};
}
