// CORE-002: the RAN stat calculator.
//
// See StatCalculator.h for the provenance of every rule encoded here. Each
// function below names the legacy line it reproduces.

#include "stats/StatCalculator.h"

#include <cmath>
#include <limits>

namespace Modern::Stats
{
	namespace
	{
		// ------------------------------------------------------------------
		// Saturating conversions
		// ------------------------------------------------------------------
		//
		// RAN performs several C casts whose value is out of the destination
		// range when a character is extreme or a data row is corrupt, which is
		// undefined behaviour. These two perform the same truncation for
		// in-range values and saturate outside it, so a bounded input always
		// produces a bounded output.

		uint16_t ToWordTruncating(float value) noexcept
		{
			// Truncation toward zero, as the C cast does.
			const float truncated = std::trunc(value);
			if (!(truncated > 0.0f))          // also catches NaN
			{
				return 0;
			}
			if (truncated >= 65535.0f)
			{
				return 65535;
			}
			return static_cast<uint16_t>(truncated);
		}

		uint32_t ToDwordTruncating(float value) noexcept
		{
			const float truncated = std::trunc(value);
			if (!(truncated > 0.0f))          // also catches NaN
			{
				return 0;
			}
			if (truncated >= 4294967295.0f)
			{
				return 4294967295u;
			}
			return static_cast<uint32_t>(truncated);
		}

		int32_t ToIntTruncating(float value) noexcept
		{
			const float truncated = std::trunc(value);
			if (!(truncated > -2147483648.0f))
			{
				return std::numeric_limits<int32_t>::min();
			}
			if (truncated >= 2147483647.0f)
			{
				return std::numeric_limits<int32_t>::max();
			}
			return static_cast<int32_t>(truncated);
		}

		// int32 + uint32 in RAN, where the int is promoted to DWORD and the sum
		// wraps. Returned as int32 because that is how RAN's `m_nDEFENSE` and
		// friends are declared.
		int32_t WrapAdd(int32_t value, uint32_t bonus) noexcept
		{
			const uint32_t sum = static_cast<uint32_t>(value) + bonus;
			return static_cast<int32_t>(sum);
		}

		// GLOGICEX::VARIATION(wNow, USHRT_MAX, nValue) with a maximum of
		// USHRT_MAX, which is how SUM_ADDITION clamps the attack powers:
		// add in int arithmetic, then clamp to [0, 65535].
		//
		// legacy/Lib_Engine/Common/GameCharacterCalculations.cpp:142
		uint16_t VariationClamped(uint16_t now, int32_t value) noexcept
		{
			const int64_t sum = static_cast<int64_t>(now) + value;
			if (sum < 0)
			{
				return 0;
			}
			if (sum > 65535)
			{
				return 65535;
			}
			return static_cast<uint16_t>(sum);
		}

		// ------------------------------------------------------------------
		// The stat sum
		// ------------------------------------------------------------------
		//
		// GLogixExPC.cpp:306
		//
		//   m_sSUMSTATS = sBEGIN_STATS + sLVLUP_STATS*ZBLEVEL + m_sStats
		//                                                        + m_sSUMITEM.sStats
		//
		// `SCHARSTATS + FCHARSTATS` (GLCharDefine.h:497) adds
		// `static_cast<WORD>(float)` per field, so the float term is truncated
		// before it is added. The two following `+` are `SCHARSTATS + SCHARSTATS`
		// (GLCharDefine.h:452), which add WORD to WORD and wrap.
		BaseStats SumStats(const StatCalculationInput& input, int zeroBasedLevel) noexcept
		{
			const BaseStats&  begin = input.classConstants.beginStats;
			const StatLevelUp& up   = input.classConstants.levelUpStats;

			auto field = [&](uint16_t beginValue, float upValue,
			                 uint16_t allocated, uint16_t fromItems) noexcept -> uint16_t
			{
				// The level term is truncated per field before being added.
				uint16_t total = static_cast<uint16_t>(
					beginValue + ToWordTruncating(upValue * static_cast<float>(zeroBasedLevel)));
				// Both remaining adds are 16-bit and wrap, as in RAN.
				total = static_cast<uint16_t>(total + allocated);
				total = static_cast<uint16_t>(total + fromItems);
				return total;
			};

			BaseStats total;
			total.pow   = field(begin.pow,   up.pow,   input.allocatedStats.pow,   input.items.stats.pow);
			total.str   = field(begin.str,   up.str,   input.allocatedStats.str,   input.items.stats.str);
			total.spi   = field(begin.spi,   up.spi,   input.allocatedStats.spi,   input.items.stats.spi);
			total.dex   = field(begin.dex,   up.dex,   input.allocatedStats.dex,   input.items.stats.dex);
			total.intel = field(begin.intel, up.intel, input.allocatedStats.intel, input.items.stats.intel);
			total.sta   = field(begin.sta,   up.sta,   input.allocatedStats.sta,   input.items.stats.sta);
			return total;
		}

		// ------------------------------------------------------------------
		// Points and powers
		// ------------------------------------------------------------------
		//
		// GLogixExPC.cpp:309-332

		uint16_t BeginPlusLevelScaled(uint16_t begin, float levelUp,
		                              int zeroBasedLevel, float conversion) noexcept
		{
			return ToWordTruncating(
				(static_cast<float>(begin) + levelUp * static_cast<float>(zeroBasedLevel)) * conversion);
		}

		// The stat points and defence points. GLogixExPC.cpp:309-310.
		void SumPoints(const StatCalculationInput& input, int zeroBasedLevel,
		               uint16_t& attackPoint, uint16_t& defensePoint) noexcept
		{
			attackPoint = BeginPlusLevelScaled(
				input.classConstants.beginAttackPoint,
				input.classConstants.levelUpAttackPoint,
				zeroBasedLevel,
				input.classConstants.attackPointConversion);

			defensePoint = BeginPlusLevelScaled(
				input.classConstants.beginDefensePoint,
				input.classConstants.levelUpDefensePoint,
				zeroBasedLevel,
				input.classConstants.defensePointConversion);
		}

		// The three attack powers, including the stat-derived term and the
		// VARIATION clamp over item, passive and codex. GLogixExPC.cpp:313-332.
		void SumAttackPowers(const StatCalculationInput& input, const BaseStats& stats,
		                     uint16_t& meleePower, uint16_t& shootPower,
		                     uint16_t& magicAttack) noexcept
		{
			const ClassConstants& cc = input.classConstants;
			const int zeroBasedLevel = static_cast<int>(input.level) - 1;

			// m_wPA = WORD( (wBEGIN_PA + fLVLUP_PA*ZBLEVEL) * fCONV_PA )
			// m_wPA += WORD( wPow*fPA_POW + wDex*fPA_DEX )
			uint16_t pa = BeginPlusLevelScaled(cc.beginMeleePower, cc.levelUpMeleePower,
			                                   zeroBasedLevel, cc.meleePowerConversion);
			pa = static_cast<uint16_t>(pa + ToWordTruncating(
				static_cast<float>(stats.pow) * cc.meleePerPow +
				static_cast<float>(stats.dex) * cc.meleePerDex));

			// The shoot power is identical in shape.
			uint16_t sa = BeginPlusLevelScaled(cc.beginShootPower, cc.levelUpShootPower,
			                                   zeroBasedLevel, cc.shootPowerConversion);
			sa = static_cast<uint16_t>(sa + ToWordTruncating(
				static_cast<float>(stats.pow) * cc.shootPerPow +
				static_cast<float>(stats.dex) * cc.shootPerDex));

			// Magic attack has no level term and no VARIATION clamp; RAN assigns
			// it directly from the three stats.
			const uint16_t ma = ToWordTruncating(
				static_cast<float>(stats.dex) * cc.magicPerDex +
				static_cast<float>(stats.spi) * cc.magicPerSpi +
				static_cast<float>(stats.intel) * cc.magicPerIntel);

			meleePower  = VariationClamped(pa, WrapAdd(input.items.meleePower,
			                                          WrapAdd(input.passives.meleePower,
			                                                  static_cast<int32_t>(input.codex.meleePower))));
			shootPower  = VariationClamped(sa, WrapAdd(input.items.shootPower,
			                                          WrapAdd(input.passives.shootPower,
			                                                  static_cast<int32_t>(input.codex.shootPower))));
			magicAttack = VariationClamped(ma, WrapAdd(input.items.magicAttack,
			                                          WrapAdd(input.passives.magicAttack,
			                                                  static_cast<int32_t>(input.codex.magicAttack))));
		}

		// One resource maximum. GLogixExPC.cpp:342-355 for HP; MP and SP are the
		// same shape over fMP_SPI and fSP_STA.
		//
		// The two truncations are deliberate and are not the same operation:
		//
		//   max = DWORD( stat*coefficient + itemFlat + passiveFlat )
		//   max = DWORD( max * (1 + rate) * confPointRate )
		//   max += codexFlat
		//
		// Folding the first truncation away, or applying the codex before the
		// rate, would produce a different number.
		uint32_t SumResourceMax(uint16_t stat, float coefficient,
		                        int32_t itemFlat, int32_t passiveFlat, float rate,
		                        float confPointRate, uint32_t codexFlat) noexcept
		{
			const float raw = static_cast<float>(stat) * coefficient +
			                  static_cast<float>(itemFlat) +
			                  static_cast<float>(passiveFlat);

			// First truncation, to 32 bits.
			uint32_t maximum = ToDwordTruncating(raw);

			// Rate and configuration point rate, then the second truncation.
			maximum = ToDwordTruncating(
				static_cast<float>(maximum) * (1.0f + rate) * confPointRate);

			// The codex bonus is added last, in 32-bit unsigned, so it is never
			// diluted by the rate and never truncated away.
			maximum += codexFlat;
			return maximum;
		}
	}

	bool IsFinite(const StatLevelUp& value) noexcept
	{
		return std::isfinite(value.pow) && std::isfinite(value.str) && std::isfinite(value.spi) &&
		       std::isfinite(value.dex) && std::isfinite(value.intel) && std::isfinite(value.sta);
	}

	bool IsFinite(const ClassConstants& value) noexcept
	{
		return IsFinite(value.levelUpStats) &&
		       std::isfinite(value.levelUpAttackPoint)  && std::isfinite(value.levelUpDefensePoint) &&
		       std::isfinite(value.levelUpMeleePower)   && std::isfinite(value.levelUpShootPower) &&
		       std::isfinite(value.attackPointConversion) &&
		       std::isfinite(value.defensePointConversion) &&
		       std::isfinite(value.meleePowerConversion) && std::isfinite(value.shootPowerConversion) &&
		       std::isfinite(value.hpPerStr)   && std::isfinite(value.mpPerSpi) &&
		       std::isfinite(value.spPerSta)   && std::isfinite(value.hitPerDex) &&
		       std::isfinite(value.avoidPerDex) && std::isfinite(value.defensePerDex) &&
		       std::isfinite(value.meleePerPow) && std::isfinite(value.meleePerDex) &&
		       std::isfinite(value.shootPerPow) && std::isfinite(value.shootPerDex) &&
		       std::isfinite(value.magicPerDex) && std::isfinite(value.magicPerSpi) &&
		       std::isfinite(value.magicPerIntel);
	}

	bool IsFinite(const ItemContribution& value) noexcept
	{
		return std::isfinite(value.hpRecoveryRate) && std::isfinite(value.mpRecoveryRate) &&
		       std::isfinite(value.spRecoveryRate) && std::isfinite(value.hitRatePercent) &&
		       std::isfinite(value.avoidRatePercent);
	}

	bool IsFinite(const PassiveContribution& value) noexcept
	{
		return std::isfinite(value.hpRate) && std::isfinite(value.mpRate) && std::isfinite(value.spRate) &&
		       std::isfinite(value.hpRecoveryRate) && std::isfinite(value.mpRecoveryRate) &&
		       std::isfinite(value.spRecoveryRate);
	}

	bool IsZero(const ItemContribution& value) noexcept
	{
		return value == ItemContribution();
	}

	bool IsZero(const PassiveContribution& value) noexcept
	{
		return value == PassiveContribution();
	}

	Result<DerivedStats> Calculate(const StatCalculationInput& input) noexcept
	{
		if (!IsValidClass(input.characterClass))
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (!IsValidLevel(input.level))
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (!IsFinite(input.classConstants) || !IsFinite(input.items) || !IsFinite(input.passives) ||
		    !std::isfinite(input.confPointRate))
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// RAN's ZBLEVEL. Zero-based, so level 1 contributes no growth.
		const int zeroBasedLevel = static_cast<int>(input.level) - 1;

		DerivedStats stats;

		// GLogixExPC.cpp:306
		stats.totalStats = SumStats(input, zeroBasedLevel);

		// GLogixExPC.cpp:309-310
		SumPoints(input, zeroBasedLevel, stats.attackPoint, stats.defensePoint);

		// GLogixExPC.cpp:313-332
		SumAttackPowers(input, stats.totalStats, stats.meleePower, stats.shootPower,
		                stats.magicAttack);

		// GLogixExPC.cpp:342-355
		stats.maxHp = SumResourceMax(stats.totalStats.str, input.classConstants.hpPerStr,
		                              input.items.hp, input.passives.hp, input.passives.hpRate,
		                              input.confPointRate, input.codex.hp);
		stats.maxMp = SumResourceMax(stats.totalStats.spi, input.classConstants.mpPerSpi,
		                              input.items.mp, input.passives.mp, input.passives.mpRate,
		                              input.confPointRate, input.codex.mp);
		stats.maxSp = SumResourceMax(stats.totalStats.sta, input.classConstants.spPerSta,
		                              input.items.sp, input.passives.sp, input.passives.spRate,
		                              input.confPointRate, input.codex.sp);

		// GLogixExPC.cpp:365-366, then the percentage modifier at 370-371.
		//
		//   hit = int( wDex*fHIT_DEX + item + passive + codex )
		//   hit = int( hit * (100.0f + itemPercent) * 0.01f )
		//
		// The grouping is preserved: the value is multiplied by (100+percent)
		// before it is scaled by 0.01f.
		const int32_t hitRaw = ToIntTruncating(
			static_cast<float>(stats.totalStats.dex) * input.classConstants.hitPerDex +
			static_cast<float>(input.items.hit) +
			static_cast<float>(input.passives.hit) +
			static_cast<float>(input.codex.hit));
		stats.hit = ToIntTruncating(
			static_cast<float>(hitRaw) * (100.0f + input.items.hitRatePercent) * 0.01f);

		const int32_t avoidRaw = ToIntTruncating(
			static_cast<float>(stats.totalStats.dex) * input.classConstants.avoidPerDex +
			static_cast<float>(input.items.avoid) +
			static_cast<float>(input.passives.avoid) +
			static_cast<float>(input.codex.avoid));
		stats.avoid = ToIntTruncating(
			static_cast<float>(avoidRaw) * (100.0f + input.items.avoidRatePercent) * 0.01f);

		// GLogixExPC.cpp:372 then 376.
		stats.defenseBody = ToIntTruncating(
			static_cast<float>(stats.defensePoint) +
			static_cast<float>(stats.totalStats.dex) * input.classConstants.defensePerDex);
		stats.defense = WrapAdd(stats.defenseBody, input.items.defense);
		stats.defense = WrapAdd(stats.defense, input.passives.defense);
		stats.defense = WrapAdd(stats.defense, static_cast<int32_t>(input.codex.defense));

		// GLogixExPC.cpp:380-389. RAN chooses melee or shoot power here from
		// the equipped weapon's range; with no equipment the melee branch is
		// used, and the ranged branch differs only in the value passed to
		// VAR_PARAM.
		const int32_t damageBase = ToIntTruncating(
			static_cast<float>(stats.attackPoint) +
			static_cast<float>(input.passives.damage) +
			static_cast<float>(input.codex.attack));

		int32_t low  = WrapAdd(damageBase, input.items.damageLow);
		int32_t high = WrapAdd(damageBase, input.items.damageHigh);

		// GLDWDATA::VAR_PARAM, GLDefine.h:471 — add, but never below 1.
		const int32_t applied = static_cast<int32_t>(stats.meleePower);
		low  = ((low  + applied) < 1) ? 1 : (low  + applied);
		high = ((high + applied) < 1) ? 1 : (high + applied);

		stats.physicalDamage.low  = static_cast<uint32_t>(low);
		stats.physicalDamage.high = static_cast<uint32_t>(high);

		// GLogixExPC.cpp:394, then SRESIST::LIMIT which floors each element at
		// zero (GLCharDefine.h:788).
		stats.resistances = input.passives.resistances;
		stats.resistances.fire     += input.items.resistances.fire;
		stats.resistances.ice      += input.items.resistances.ice;
		stats.resistances.electric += input.items.resistances.electric;
		stats.resistances.poison   += input.items.resistances.poison;
		stats.resistances.spirit   += input.items.resistances.spirit;
		stats.resistances.AddAll(static_cast<int32_t>(input.codex.resistance));
		stats.resistances.ClampNonNegative();

		// GLogixExPC.cpp:397-399. A rate, not an amount per second.
		stats.hpRecoveryRate = RecoveryRateConstant::kHp + input.items.hpRecoveryRate +
		                       input.passives.hpRecoveryRate;
		stats.mpRecoveryRate = RecoveryRateConstant::kMp + input.items.mpRecoveryRate +
		                       input.passives.mpRecoveryRate;
		stats.spRecoveryRate = RecoveryRateConstant::kSp + input.items.spRecoveryRate +
		                       input.passives.spRecoveryRate;

		return DerivedStats(std::move(stats));
	}
}
