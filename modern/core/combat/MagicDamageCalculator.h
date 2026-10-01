#pragma once

// VERTICAL-013: magic / elemental combat.
//
// This is deliberately NOT `CalculatePhysicalDamage` with a `bool isMagic`.
// The two channels agree from the damage roll onwards and disagree entirely
// before it:
//
//   Physical (GLogixExPC.cpp:1447-1470, 1572-1597)
//     weapon item damage IS added to the range
//     attack power is PA (melee) or SA (ranged)
//     resistance is applied to the ROLLED damage, multiplicatively
//     physical defense, body defense and item defense all apply
//     ranged suppresses reflection
//
//   Magic (GLogixExPC.cpp:1473-1487)
//     weapon item damage is NOT added to the range at all
//     attack power is MA
//     resistance is applied to the RANGE, subtractively, before the roll
//     defense, body defense and item defense are all forced to zero
//     reflection uses the magic reduction and magic reflection values
//
// Those are different formulas over the same range type. Forcing them through
// one function with flags would have hidden exactly the distinctions the
// milestone exists to establish. What IS shared - the roll, the level
// adjustment, the low-seed branch, critical, crushing, the reduction
// calculation, the reflection calculation and the minimum clamp - is reused
// from `GameCharacterCalculations` and `CombatConstants` rather than
// reimplemented here.

#include "CombatTypes.h"

namespace Modern::Combat
{
	// Inputs for the magic damage calculation.
	struct MagicDamageInput
	{
		// ── Attacker ────────────────────────────────────────────────────────
		uint16_t magicAttack = 0;           // m_wSUM_MA
		float    skillBasicVar = 0.0f;       // sSKILL_DATA.fBASIC_VAR
		float    damageRate = 1.0f;          // m_fDamageRate
		int32_t  skillCrushingBonus = 0;     // EMSPECA_CRUSHING_BLOW, *100

		uint32_t attackerLevel = 1;
		uint32_t attackerMaxHP = 0;
		uint32_t attackerCurrentHP = 0;
		int32_t  attackerCriticalBonus = 0;
		int32_t  attackerCrushingBonus = 0;

		// The pre-roll range. This is `m_gdDAMAGE_SKILL`, which for magic gets
		// NO weapon item damage added (contrast physical at :1448/:1460).
		Stats::DamageRange skillRange{};

		// ── Target ─────────────────────────────────────────────────────────
		int32_t targetLevel = 1;
		int32_t resistElement = 0;          // already clamped by the caller
		float   magicDamageReduce = 0.0f;    // m_fMagicDamageReduce
		float   magicDamageReflection = 0.0f;      // m_fMagicDamageReflection
		float   magicDamageReflectionRate = 0.0f; // m_fMagicDamageReflectionRate
		float   targetDamageDecrease = 0.0f; // GetDecR_DamageMagicSkill
		float   stateDamage = 1.0f;

		bool    lowSP = false;
		bool    isPK = false;

		// ── Environment ────────────────────────────────────────────────────
		// GLOGICEX::WEATHER_ELEMENT_POW. Kept as an injected deterministic value
		// because no world/weather provider exists yet; 1.0 is the verified
		// no-weather result (GameCharacterCalculations.cpp:461-462).
		float weatherElementPower = 1.0f;

		// ── Random values, all [0.0, 1.0] ──────────────────────────────────
		float damageRoll = 0.0f;
		float criticalRoll = 1.0f;
		float crushingRoll = 1.0f;
		float reflectionRoll = 1.0f;
	};

	// Calculates magic damage. See the file header for why this is separate
	// from `CalculatePhysicalDamage`.
	inline DamageResult CalculateMagicDamage(const MagicDamageInput& input,
	                                         const CombatConstants& constants = CombatConstants())
	{
		DamageResult result;

		// ── 1. Attack power onto the range (GLogixExPC.cpp:1477) ────────────
		//
		// gdDamage.VAR_PARAM(m_wSUM_MA) - same operation as PA and SA, and
		// magic does not add the weapon's item damage the way physical does.
		Stats::DamageRange damage = input.skillRange;
		damage.low  = ApplyAttackPower(damage.low,  static_cast<int32_t>(input.magicAttack));
		damage.high = ApplyAttackPower(damage.high, static_cast<int32_t>(input.magicAttack));

		// ── 2. Skill magnitude (GLogixExPC.cpp:1520-1524) ──────────────────
		//
		//   float fSKILL_VAR = sSKILL_DATA.fBASIC_VAR;
		//   int nVAR = abs ( int(fSKILL_VAR*fPOWER) );
		//
		// Truncation happens on the float product BEFORE abs, so -0.5 becomes 0
		// and does not become 1.
		const int32_t nVAR = static_cast<int32_t>(std::abs(
			static_cast<int>(input.skillBasicVar * input.weatherElementPower)));

		// DEPARTURE: legacy then adds `nVAR + dwLow * (wGRADE/fDAMAGE_GRADE_K)`
		// to both ends (GLogixExPC.cpp:1527-1531). The item grade
		// (`GETGRADE(EMGRINDING_DAMAGE)`) has no representation in the modern
		// item model, so the grade term is omitted rather than invented. The
		// skill magnitude itself is included; only the grade-weighted part is
		// missing.
		damage.low  += static_cast<uint32_t>(nVAR);
		damage.high += static_cast<uint32_t>(nVAR);

		// ── 3. Target's own damage decrease (GLogixExPC.cpp:1545-1553) ─────
		//
		//   gdDamage.dwLow  -= int(float(gdDamage.dwLow) * fDec_Damage);
		//
		// Magic reads GetDecR_DamageMagicSkill, a target property distinct from
		// the damage reduction applied much later at :1734.
		if (input.targetDamageDecrease != 0.0f)
		{
			damage.low  -= static_cast<uint32_t>(static_cast<float>(damage.low)  * input.targetDamageDecrease);
			damage.high -= static_cast<uint32_t>(static_cast<float>(damage.high) * input.targetDamageDecrease);
		}

		// ── 4. Resistance, on the RANGE (GLogixExPC.cpp:1556-1564) ─────────
		//
		//   float fResistTotal = (float)((float) nRESIST * 0.01f * fRESIST_G);
		//   fResistTotal = fResistTotal > 0.8f ? 0.8f : fResistTotal;
		//   gdDamage.dwLow  -= (DWORD) ((float) gdDamage.dwLow  * fResistTotal);
		//   gdDamage.dwHigh -= (DWORD) ((float) gdDamage.dwHigh * fResistTotal);
		//
		// Two things are deliberately faithful here and both differ from the
		// physical path:
		//
		//   - It runs on the range, BEFORE the roll. Physical applies resistance
		//     to the already-rolled value.
		//   - It is a subtraction of the truncated product, not a multiplication
		//     by (1 - f). For a roll these differ, and this one is not the
		//     tidier-looking formula.
		//
		// Magic uses the general fRESIST_G, not the physical fRESIST_PHYSIC_G.
		if (input.resistElement > 0)
		{
			int32_t resistClamped = input.resistElement;
			if (resistClamped > constants.maxResist)
			{
				resistClamped = static_cast<int32_t>(constants.maxResist);
			}
			float fResistTotal = static_cast<float>(resistClamped) * 0.01f * constants.resistGeneralG;
			if (fResistTotal > constants.maxResistReduction)
			{
				fResistTotal = constants.maxResistReduction;
			}
			damage.low  -= static_cast<uint32_t>(static_cast<float>(damage.low)  * fResistTotal);
			damage.high -= static_cast<uint32_t>(static_cast<float>(damage.high) * fResistTotal);
		}

		// GLogixExPC.cpp:1566-1570 clamps the ends at 0, not at 1. Only the
		// pre-roll VAR_PARAM floor is 1.
		if (damage.low == 0u)  damage.low = 0u;
		if (damage.high == 0u) damage.high = 0u;

		// ── 5. Damage rate (GLogixExPC.cpp:1600-1603) ─────────────────────
		damage.low  = Modern::Engine::ApplyDamageRate(damage.low,  input.damageRate);
		damage.high = Modern::Engine::ApplyDamageRate(damage.high, input.damageRate);

		// ── 6. Level adjustment (GLogixExPC.cpp:1606-1608) ────────────────
		int32_t nExtFORCE = 0;
		{
			const int32_t ndxLvl = input.targetLevel - static_cast<int32_t>(input.attackerLevel);
			if (ndxLvl > 0)
			{
				nExtFORCE = static_cast<int32_t>(input.damageRoll * static_cast<float>(ndxLvl) / 10.0f);
			}
		}

		// ── 7. Roll (GLogixExPC.cpp:1672-1674) ────────────────────────────
		const uint32_t nDAMAGE_NOW = static_cast<uint32_t>(
			Modern::Engine::RandomDamageRange(damage.low, damage.high, input.damageRoll));
		result.rawDamage = nDAMAGE_NOW;

		const uint32_t nDAMAGE_OLD = nDAMAGE_NOW + static_cast<uint32_t>(nExtFORCE);
		result.preDefenseDamage = nDAMAGE_OLD;

		// ── 8. Low-seed branch (GLogixExPC.cpp:1678-1686) ─────────────────
		//
		// nDEFENSE is 0 for magic (:1474), so the comparison reduces to the
		// low-seed threshold alone. The `else` branch is the "damage too small to
		// survive the seed" case and is scaled by a fresh roll.
		const int32_t nNetDAMAGE = static_cast<int32_t>(
			static_cast<float>(nDAMAGE_OLD) * (1.0f - constants.lowSeedDamage) - 0.0f);

		uint32_t resultDamage = 0;
		if (nNetDAMAGE > 0)
		{
			resultDamage = nDAMAGE_OLD;
		}
		else
		{
			resultDamage = static_cast<uint32_t>(
				static_cast<float>(nDAMAGE_OLD) * constants.lowSeedDamage * input.damageRoll);
		}

		resultDamage = Modern::Engine::ApplyStateDamage(
			static_cast<int32_t>(resultDamage), input.stateDamage);

		// ── 9. Low SP (GLogixExPC.cpp has none here; see the note) ─────────
		//
		// `CALCDAMAGE` does NOT apply a low-SP damage penalty anywhere.
		// Basic attacks halve once in `GLChar::DamageProc` (GLChar.cpp:2489);
		// skills halve once in `SkillProc` (GLChar.cpp:3131), AFTER this
		// function returns. `fFLOWSP_DAMAGE` appears nowhere in GLogixExPC.cpp.
		//
		// VERTICAL-009 models that single factor inside the calculator (see
		// PhysicalDamageCalculator.h), and this follows the same choice so both
		// channels behave identically. The cost is that the low-SP halving
		// happens here rather than after the minimum-damage clamp; the flag is
		// applied exactly once either way, and applying it in both places would
		// halve twice.
		result.lowSP = input.lowSP;
		if (input.lowSP)
		{
			resultDamage = static_cast<uint32_t>(
				static_cast<float>(resultDamage) * (1.0f - constants.lowSPDamage));
		}

		// ── 10. Critical (GLogixExPC.cpp:1615-1620, 1652-1656) ────────────
		//
		// Identical to physical, including the `criticalHitRateBase` addition
		// that VERTICAL-009 already documents as a deviation. Magic has no
		// separate critical rule in legacy.
		int32_t nPercentCri = Modern::Engine::CriticalBaseRate(
			static_cast<int32_t>(input.attackerCurrentHP),
			static_cast<int32_t>(input.attackerMaxHP),
			static_cast<int32_t>(input.attackerLevel),
			input.targetLevel);
		nPercentCri += constants.criticalHitRateBase;
		nPercentCri += input.attackerCriticalBonus;

		if (nPercentCri > static_cast<int32_t>(constants.criticalMax))
			nPercentCri = static_cast<int32_t>(constants.criticalMax);
		if (nPercentCri < 0)
			nPercentCri = 0;

		const bool bCritical = nPercentCri > static_cast<int32_t>(input.criticalRoll * 100.0f);
		result.critical = bCritical;

		// ── 11. Crushing (GLogixExPC.cpp:1494-1501, 1659-1660) ────────────
		//
		// The skill's EMSPECA_CRUSHING_BLOW spec is added in a loop that sits
		// OUTSIDE the apply switch, so it applies to magic too.
		int32_t nCrushingBlow = input.attackerCrushingBonus + input.skillCrushingBonus;
		if (nCrushingBlow > static_cast<int32_t>(constants.crushingBlowMax))
			nCrushingBlow = static_cast<int32_t>(constants.crushingBlowMax);
		if (nCrushingBlow < 0)
			nCrushingBlow = 0;

		const bool bCrushingBlow = nCrushingBlow > static_cast<int32_t>(input.crushingRoll * 100.0f);
		result.crushing = bCrushingBlow;

		// ── 12. Critical / crushing damage (GLogixExPC.cpp:1725-1731) ─────
		//
		// Legacy multiplies `rResultDAMAGE`, the value as it stands here.
		if (bCritical && bCrushingBlow)
		{
			resultDamage = static_cast<uint32_t>(static_cast<float>(resultDamage) * constants.crushingBlowDamage / 100.0f);
		}
		else if (bCritical)
		{
			resultDamage = static_cast<uint32_t>(static_cast<float>(resultDamage) * constants.criticalDamage / 100.0f);
		}
		else if (bCrushingBlow)
		{
			resultDamage = static_cast<uint32_t>(static_cast<float>(resultDamage) * constants.crushingBlowDamage / 100.0f);
		}

		// ── 13. Magic damage reduction (GLogixExPC.cpp:1734-1742) ─────────
		//
		// Same calculation as physical; the difference is the source value,
		// `m_fMagicDamageReduce` rather than `m_fPsyDamageReduce` (:1482).
		if (input.magicDamageReduce > 0.0f)
		{
			const int32_t nDamageReduce = Modern::Engine::DamageReduceAmount(
				static_cast<int32_t>(resultDamage),
				input.magicDamageReduce,
				input.targetLevel,
				static_cast<int32_t>(constants.maxLevel));

			if (static_cast<int32_t>(resultDamage) > nDamageReduce)
			{
				resultDamage -= static_cast<uint32_t>(nDamageReduce);
			}
			else
			{
				resultDamage = 0;
			}
		}

		// ── 14. PK modifier ────────────────────────────────────────────────
		if (input.isPK)
		{
			resultDamage = static_cast<uint32_t>(
				static_cast<float>(resultDamage) * constants.pkPointDecPhy);
		}

		result.damage = resultDamage != 0u ? resultDamage : 1u;

		// ── 15. Magic reflection (GLogixExPC.cpp:1746-1763) ────────────────
		//
		// Magic reflection is NOT the ranged-physical suppression of
		// VERTICAL-012. Legacy reads `m_fMagicDamageReflection` /
		// `m_fMagicDamageReflectionRate` for magic (:1483-1484) and, because
		// `bPsyDamage` is false (:1485), tags the hit with
		// DAMAGE_TYPE_MAGIC_REFLECTION (:1755-1756) rather than the psy flag.
		//
		// The amount formula is the shared one.
		if (input.magicDamageReflectionRate > 0.0f && result.damage > 0)
		{
			const uint32_t reflectionRoll = static_cast<uint32_t>(input.reflectionRoll * 100.0f);
			result.reflectionRoll = reflectionRoll;

			if (static_cast<uint32_t>(input.magicDamageReflectionRate * 100.0f) > reflectionRoll)
			{
				result.reflectionTriggered = true;

				const int32_t nDamageReflection = Modern::Engine::DamageReflectionAmount(
					static_cast<int32_t>(result.damage),
					input.magicDamageReflection,
					input.targetLevel,
					static_cast<int32_t>(constants.maxLevel));

				if (nDamageReflection > 0)
				{
					result.reflectionDamage = static_cast<uint32_t>(nDamageReflection);
				}
			}
		}

		return result;
	}
}