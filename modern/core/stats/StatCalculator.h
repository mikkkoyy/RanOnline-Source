#pragma once

// CORE-002: the deterministic RAN stat calculator.
//
// One function, one input value, one output value. Stateless: no globals, no
// clock, no I/O, no renderer, no database, no network, no legacy headers, and
// no randomness. The same input always produces the same output.
//
// The arithmetic is RAN's, including its truncations. The rules that matter,
// each verified against legacy/Lib_Client/G-Logic/GLogixExPC.cpp:286:
//
//   1. The level term is (level - 1), not level. RAN names it ZBLEVEL.
//   2. The per-level growth is a float product truncated **per field** to 16
//      bits before it is added, not rounded and not carried as a float.
//   3. The stat sum is 16-bit unsigned and wraps. It is not widened, so a
//      character past 65535 in a stat agrees with a shipped client.
//   4. A resource maximum is truncated to 32 bits, then multiplied by the
//      passive rate and the configuration point rate, then truncated to 32
//      bits again. Both truncations are reproduced; collapsing them into one
//      would change the result.
//   5. The codex bonus is added last, after the truncations, as a flat
//      unsigned value.
//   6. Hit and avoid apply their percentage as
//      int(value * (100 + percent) * 0.01f) — the multiply happens before
//      the scale, and the grouping is preserved.
//   7. Attack powers are clamped to [0, 65535] by RAN's VARIATION, which is
//      what stops a large equipment bonus from wrapping the stat instead.
//
// Where RAN has undefined behaviour this does something defined, and says so
// in the source: a C cast from a float outside the destination range is
// undefined, and RAN performs several. Every such cast here saturates, so a
// hostile or corrupt input produces a bounded result instead of an arbitrary
// one. For every in-range input the result is identical to RAN's.

#include "stats/BaseStats.h"
#include "stats/Contributions.h"
#include "stats/DerivedStats.h"
#include "types/Result.h"

namespace Modern::Stats
{
	// Everything one stat calculation needs.
	//
	// The class row is a value rather than a lookup because RAN's coefficients
	// are loaded from `default.charclass` at runtime and that data file is not
	// part of this repository. The arithmetic is the part reproduced here, so
	// a caller that has loaded the row supplies it.
	struct StatCalculationInput
	{
		// The class whose constant row is used. Must be one of the sixteen
		// RAN class indices.
		CharClassIndex characterClass = CharClassIndex::BrawlerMale;

		// 1..255. RAN's wMAX_LEVEL.
		uint16_t level = kMinLevel;

		// The class's row of GLCONST_CHARCLASS.
		ClassConstants classConstants;

		// m_sStats: the stats allocated onto the character. Not a base value —
		// this is what the player has spent points into, and it is added after
		// the class and level terms.
		BaseStats allocatedStats;

		ItemContribution     items;
		PassiveContribution  passives;
		CodexContribution    codex;

		// fCONFT_POINT_RATE: the configuration point rate applied to every
		// resource maximum. RAN's callers supply it; a value of 1.0f is "no
		// adjustment".
		float confPointRate = 1.0f;
	};

	// The class-constant recovery rates, which RAN holds as globals rather than
	// per class.
	//
	// legacy/Lib_Client/G-Logic/GLogicData.cpp:252-254, written as the products
	// shown so the intent stays readable:
	//
	//   fHP_INC_PER = 0.3f * 0.01f
	//   fMP_INC_PER = 0.3f * 0.01f
	//   fSP_INC_PER = 0.5f * 0.01f
	namespace RecoveryRateConstant
	{
		inline constexpr float kHp = 0.3f * 0.01f;
		inline constexpr float kMp = 0.3f * 0.01f;
		inline constexpr float kSp = 0.5f * 0.01f;
	}

	// The absolute half of RAN's recovery term: `GLCONST_CHAR::fHP_INC`,
	// `fMP_INC` and `fSP_INC`.
	//
	// legacy/Lib_Client/G-Logic/GLogicData.cpp:256-258, which is the definition
	// site, not a transcription of a shipped data file:
	//
	//   fHP_INC = 0
	//   fMP_INC = 0
	//   fSP_INC = 0
	//
	// They are runtime-configurable (GLogicDataLoad.cpp reads them from a
	// script), and zero is the shipped default, so zero is the correct value to
	// reproduce. They are named here rather than folded into the rate because
	// they are added in a different term of the same expression - see
	// `DerivedStats::hpRecoveryFlat`.
	namespace RecoveryFlatConstant
	{
		inline constexpr float kHp = 0.0f;
		inline constexpr float kMp = 0.0f;
		inline constexpr float kSp = 0.0f;
	}

	// `GLCONST_CHAR::fUNIT_TIME` (GLogicData.cpp:251), the divisor that turns
	// elapsed time into recovery units. It is a recovery-system input rather
	// than a stat, so the resource system owns it; the constant is published
	// here because that is where the verified value lives and because a caller
	// assembling a `ResourceState` has no other reason to know about it.
	namespace RecoveryTiming
	{
		inline constexpr float kUnitTime = 1.0f;
	}

	// Calculates one character's derived statistics.
	//
	// Fails with InvalidArgument if the class index is out of range, the level
	// is outside 1..255, or any float in the class row or the contributions is
	// not finite. It never throws and never reads global state.
	Result<DerivedStats> Calculate(const StatCalculationInput& input) noexcept;
}
