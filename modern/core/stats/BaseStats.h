#pragma once

// CORE-002: RAN base statistics and the per-class constant table.
//
// The vocabulary here is RAN's own, taken from the legacy source and not from
// any other MMORPG's stat list. RAN has six base stats and they are not
// STR/DEX/INT/VIT/WIL/LUK:
//
//   pow  힘      raw physical power
//   str  체력    constitution, feeds HP
//   spi  정신    spirit, feeds MP
//   dex  민첩    dexterity, feeds hit, avoid, defense and the attack powers
//   int  지력    intellect, feeds magic attack
//   sta  근력    stamina, feeds SP
//
// Legacy origin, and the reason for every field below:
//
//   legacy/Lib_Client/G-Logic/GLCharDefine.h:373  SCHARSTATS   six WORD stats
//   legacy/Lib_Client/G-Logic/GLCharDefine.h:328  FCHARSTATS   the same six as float
//   legacy/Lib_Client/G-Logic/GLCharDefine.h:235  EMCHARINDEX  sixteen class values
//   legacy/Lib_Client/G-Logic/GLogicData.h:58     GLCONST_CHARCLASS
//
// Every base stat is a 16-bit unsigned integer in RAN, and the stat *sum* is
// 16-bit unsigned arithmetic, which wraps. That is reproduced rather than
// widened, because a character whose allocation overflows 65535 in RAN wraps,
// and a modern build that saturates instead would silently disagree with a
// shipped client.

#include <cstdint>
#include <limits>

namespace Modern::Stats
{
	// RAN's class table index. Class and gender are encoded together here
	// because that is what indexes the legacy per-class constant table; the
	// core does not interpret the gender half.
	//
	// Values are the legacy EMCHARINDEX, verified one-for-one against
	// GLCharDefine.h:235. GLCI_NUM_8CLASS (16) is the count, not a member.
	enum class CharClassIndex : uint8_t
	{
		BrawlerMale    = 0,
		SwordsmanMale  = 1,
		ArcherFemale   = 2,
		ShamanFemale   = 3,
		ExtremeMale    = 4,
		ExtremeFemale  = 5,
		BrawlerFemale  = 6,
		SwordsmanFemale= 7,
		ArcherMale     = 8,
		ShamanMale     = 9,
		GunnerMale     = 10,
		GunnerFemale   = 11,
		AssassinMale   = 12,
		AssassinFemale = 13,
		TrickerMale    = 14,
		TrickerFemale  = 15,
	};

	constexpr uint8_t kClassCount = 16;

	constexpr bool IsValidClass(CharClassIndex value) noexcept
	{
		return static_cast<uint8_t>(value) < kClassCount;
	}

	// The six base stats, 16-bit unsigned, in RAN's order.
	//
	// Addition and accumulation wrap at 65535. That is RAN's behaviour
	// (SCHARSTATS members are WORD and operator+ adds them in place), and a
	// widened accumulator would diverge from a shipped client on exactly the
	// characters most likely to be rolled over.
	struct BaseStats
	{
		uint16_t pow  = 0;
		uint16_t str  = 0;
		uint16_t spi  = 0;
		uint16_t dex  = 0;
		uint16_t intel = 0;
		uint16_t sta  = 0;

		constexpr bool operator==(const BaseStats& other) const noexcept
		{
			return pow == other.pow && str == other.str && spi == other.spi &&
			       dex == other.dex && intel == other.intel && sta == other.sta;
		}

		constexpr uint16_t Total() const noexcept
		{
			// Matches SCHARSTATS::GetTotal, which sums in WORD and widens at the
			// end, so the intermediate additions wrap.
			return static_cast<uint16_t>(static_cast<uint16_t>(pow + str) +
			                             static_cast<uint16_t>(spi + dex) +
			                             static_cast<uint16_t>(intel + sta));
		}

		constexpr bool IsZero() const noexcept
		{
			return (pow + str + spi + dex + intel + sta) == 0;
		}
	};

	// The same six stats as single-precision floats.
	//
	// This is RAN's level-up table (FCHARSTATS). It is floating point in the
	// legacy source and the derived sum truncates it back to WORD per field, so
	// the float width is part of the arithmetic and not incidental.
	struct StatLevelUp
	{
		float pow   = 0.0f;
		float str   = 0.0f;
		float spi   = 0.0f;
		float dex   = 0.0f;
		float intel = 0.0f;
		float sta   = 0.0f;

		constexpr bool operator==(const StatLevelUp& other) const noexcept
		{
			return pow == other.pow && str == other.str && spi == other.spi &&
			       dex == other.dex && intel == other.intel && sta == other.sta;
		}
	};

	// One class's row of the constant table: the values RAN loads from
	// `default.charclass` into GLCONST_CHARCLASS.
	//
	// Legacy origin: GLogicData.h:58 declares the struct, and
	// GLogicDataLoad.cpp:1169-1214 parses exactly these fields out of the file.
	// Only the fields the verified stat pipeline reads are carried here; the
	// rest of GLCONST_CHARCLASS is animation, head and hair data.
	//
	// The numbers are *data*, not code. RAN ships them in a data file that is
	// not in this repository, so nothing here is hard-coded to RAN's shipped
	// values — a caller supplies the row for the class being calculated. The
	// arithmetic is the part that is reproduced, and it is reproduced in
	// StatCalculator.
	struct ClassConstants
	{
		// sBEGIN_STATS: the stats a level-1 character of this class starts with.
		BaseStats beginStats;

		// sLVLUP_STATS: stats gained per level, as floats. Multiplied by
		// (level - 1) and then truncated per field, exactly as RAN does.
		StatLevelUp levelUpStats;

		// wBEGIN_AP / wBEGIN_DP / wBEGIN_PA / wBEGIN_SA: the level-1 values of
		// the stat points, defence points, melee power and shoot power.
		uint16_t beginAttackPoint   = 0;
		uint16_t beginDefensePoint  = 0;
		uint16_t beginMeleePower    = 0;
		uint16_t beginShootPower    = 0;

		// fLVLUP_AP / DP / PA / SA: the per-level growth of the same four, as
		// floats, again multiplied by (level - 1).
		float levelUpAttackPoint  = 0.0f;
		float levelUpDefensePoint = 0.0f;
		float levelUpMeleePower   = 0.0f;
		float levelUpShootPower   = 0.0f;

		// fCONV_AP / DP / PA / SA: the conversion applied after the level
		// growth, i.e. the points a level-up point is worth.
		float attackPointConversion  = 0.0f;
		float defensePointConversion = 0.0f;
		float meleePowerConversion   = 0.0f;
		float shootPowerConversion   = 0.0f;

		// Resource conversion: a point of the feeding stat yields this much of
		// the resource. fHP_STR, fMP_SPI, fSP_STA.
		float hpPerStr = 0.0f;
		float mpPerSpi = 0.0f;
		float spPerSta = 0.0f;

		// fHIT_DEX / fAVOID_DEX / fDEFENSE_DEX: a point of dexterity yields
		// this much hit, avoid and defence.
		float hitPerDex    = 0.0f;
		float avoidPerDex  = 0.0f;
		float defensePerDex= 0.0f;

		// fPA_POW / fPA_DEX / fSA_POW / fSA_DEX: how power and dexterity build
		// melee and shoot power.
		float meleePerPow = 0.0f;
		float meleePerDex = 0.0f;
		float shootPerPow = 0.0f;
		float shootPerDex = 0.0f;

		// fMA_DEX / fMA_SPI / fMA_INT: how dexterity, spirit and intellect
		// build magic attack.
		float magicPerDex  = 0.0f;
		float magicPerSpi  = 0.0f;
		float magicPerIntel= 0.0f;
	};

	// The widest level RAN allows. GLCONST_CHAR::wMAX_LEVEL is 255
	// (GLogicData.cpp), and the level-1 offset means the level-up term runs
	// over [0, 254].
	constexpr uint16_t kMinLevel = 1;
	constexpr uint16_t kMaxLevel = 255;

	constexpr bool IsValidLevel(uint16_t level) noexcept
	{
		return level >= kMinLevel && level <= kMaxLevel;
	}

	// True when every float in a level-up row and in the class constants is
	// finite. RAN's loader accepts whatever the data file contains; a
	// non-finite coefficient would make the whole derived set non-finite, so
	// the calculator refuses the row rather than emitting NaN.
	bool IsFinite(const StatLevelUp& value) noexcept;
	bool IsFinite(const ClassConstants& value) noexcept;
}
