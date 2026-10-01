// CORE-002: RAN stat calculation.
//
// The expected values here come from two sources, deliberately separate:
//
//   1. An oracle in this file that re-derives each result from the documented
//      RAN expression, written independently of the calculator. It shares no
//      code with modern/core/stats, so a change to the calculator's structure
//      or its ordering shows up as a mismatch rather than as two agreeing
//      mistakes.
//   2. Literal numbers, for the cases where the arithmetic is exact and can be
//      read off the formula by hand. These are the regression fixtures: they
//      pin behaviour that an oracle written by the same author could share a
//      misreading with.
//
// The literal cases were chosen to sit away from floating-point boundaries,
// and two of them exist specifically to discriminate the truncations:
//
//   - a fractional level-up rate that truncates to zero over several levels
//     (Stats_LevelUpFractionTruncatesPerField), which fails if the level term
//     is rounded or carried as a float;
//   - a resource maximum where the two truncations disagree (106 against 107),
//     which fails if either is folded away.
//
// Links Modern and nothing else: no renderer, no socket, no database, no legacy
// library.

#include "TestHarness.h"

#include "stats/BaseStats.h"
#include "stats/Contributions.h"
#include "stats/DerivedStats.h"
#include "stats/StatCalculator.h"
#include "engine/GameCharacterCalculations.h"
#include "types/Result.h"

#include <cmath>
#include <limits>

using namespace Modern;
using namespace Modern::Stats;

namespace
{
	// ------------------------------------------------------------------------
	// The oracle.
	// ------------------------------------------------------------------------
	//
	// Written from the legacy source, statement by statement. The point is
	// that it is *not* the calculator: it is a second transcription of
	// GLogixExPC.cpp:286, using the same casts and the same order, so agreement
	// between the two is evidence and disagreement is a bug report.

	uint16_t WordTrunc(float value)
	{
		return static_cast<uint16_t>(value < 0.0f ? 0.0f : value);
	}

	uint32_t DwordTrunc(float value)
	{
		return static_cast<uint32_t>(value < 0.0f ? 0.0f : value);
	}

	int IntTrunc(float value)
	{
		return static_cast<int>(value);
	}

	BaseStats OracleTotalStats(const StatCalculationInput& in)
	{
		const int zb = static_cast<int>(in.level) - 1;
		const BaseStats& b = in.classConstants.beginStats;
		const StatLevelUp& u = in.classConstants.levelUpStats;
		const BaseStats& a = in.allocatedStats;
		const BaseStats& e = in.items.stats;

		BaseStats s;
		s.pow   = static_cast<uint16_t>(b.pow   + WordTrunc(u.pow   * zb) + a.pow   + e.pow);
		s.str   = static_cast<uint16_t>(b.str   + WordTrunc(u.str   * zb) + a.str   + e.str);
		s.spi   = static_cast<uint16_t>(b.spi   + WordTrunc(u.spi   * zb) + a.spi   + e.spi);
		s.dex   = static_cast<uint16_t>(b.dex   + WordTrunc(u.dex   * zb) + a.dex   + e.dex);
		s.intel = static_cast<uint16_t>(b.intel + WordTrunc(u.intel * zb) + a.intel + e.intel);
		s.sta   = static_cast<uint16_t>(b.sta   + WordTrunc(u.sta   * zb) + a.sta   + e.sta);
		return s;
	}

	DerivedStats Oracle(const StatCalculationInput& in)
	{
		const int zb = static_cast<int>(in.level) - 1;
		const ClassConstants& cc = in.classConstants;
		const BaseStats s = OracleTotalStats(in);

		DerivedStats d;
		d.totalStats = s;

		d.attackPoint  = WordTrunc((cc.beginAttackPoint  + cc.levelUpAttackPoint  * zb) * cc.attackPointConversion);
		d.defensePoint = WordTrunc((cc.beginDefensePoint + cc.levelUpDefensePoint * zb) * cc.defensePointConversion);

		int pa = WordTrunc((cc.beginMeleePower + cc.levelUpMeleePower * zb) * cc.meleePowerConversion);
		pa += WordTrunc(static_cast<float>(s.pow) * cc.meleePerPow +
		               static_cast<float>(s.dex) * cc.meleePerDex);
		int sa = WordTrunc((cc.beginShootPower + cc.levelUpShootPower * zb) * cc.shootPowerConversion);
		sa += WordTrunc(static_cast<float>(s.pow) * cc.shootPerPow +
		               static_cast<float>(s.dex) * cc.shootPerDex);
		const int ma = WordTrunc(static_cast<float>(s.dex) * cc.magicPerDex +
		                           static_cast<float>(s.spi) * cc.magicPerSpi +
		                           static_cast<float>(s.intel) * cc.magicPerIntel);

		auto variation = [](int now, int value)
		{
			int n = now + value;
			if (n < 0)    { n = 0; }
			if (n > 65535) { n = 65535; }
			return n;
		};
		d.meleePower  = static_cast<uint16_t>(variation(pa, in.items.meleePower + in.passives.meleePower + static_cast<int>(in.codex.meleePower)));
		d.shootPower  = static_cast<uint16_t>(variation(sa, in.items.shootPower + in.passives.shootPower + static_cast<int>(in.codex.shootPower)));
		d.magicAttack = static_cast<uint16_t>(variation(ma, in.items.magicAttack + in.passives.magicAttack + static_cast<int>(in.codex.magicAttack)));

		auto resource = [&](uint16_t stat, float coefficient, int32_t itemFlat,
		                   int32_t passiveFlat, float rate, uint32_t codexFlat)
		{
			const float raw = stat * coefficient + itemFlat + passiveFlat;
			uint32_t maximum = DwordTrunc(raw);
			maximum = DwordTrunc(maximum * (1.0f + rate) * in.confPointRate);
			maximum += codexFlat;
			return maximum;
		};
		d.maxHp = resource(s.str, cc.hpPerStr, in.items.hp, in.passives.hp, in.passives.hpRate, in.codex.hp);
		d.maxMp = resource(s.spi, cc.mpPerSpi, in.items.mp, in.passives.mp, in.passives.mpRate, in.codex.mp);
		d.maxSp = resource(s.sta, cc.spPerSta, in.items.sp, in.passives.sp, in.passives.spRate, in.codex.sp);

		const int hitRaw   = IntTrunc(s.dex * cc.hitPerDex   + in.items.hit   + in.passives.hit   + static_cast<int>(in.codex.hit));
		const int avoidRaw = IntTrunc(s.dex * cc.avoidPerDex + in.items.avoid + in.passives.avoid + static_cast<int>(in.codex.avoid));
		d.hit   = IntTrunc(hitRaw   * (100.0f + in.items.hitRatePercent)   * 0.01f);
		d.avoid = IntTrunc(avoidRaw * (100.0f + in.items.avoidRatePercent) * 0.01f);

		d.defenseBody = IntTrunc(static_cast<float>(d.defensePoint) +
		                           static_cast<float>(s.dex) * cc.defensePerDex);
		d.defense = d.defenseBody + in.items.defense + in.passives.defense + static_cast<int>(in.codex.defense);
	d.defense += in.facts.defense;

	// VERTICAL-021: m_fDefenseRate multiplies the summed flat defence, and the
	// clamp is esult < 0 so a zero defence stays zero.
	d.defenseRate = 1.0f + in.passives.defenseRate + in.facts.defenseRate;
	d.defense = Modern::Engine::ApplyDefenseRate(d.defense, d.defenseRate);

		const int damageBase = IntTrunc(static_cast<float>(d.attackPoint) +
		                                   static_cast<float>(in.passives.damage) +
		                                   static_cast<float>(in.codex.attack));
		int low  = damageBase + in.items.damageLow;
		int high = damageBase + in.items.damageHigh;
		const int applied = static_cast<int>(d.meleePower);
		low  = (low  + applied) < 1 ? 1 : (low  + applied);
		high = (high + applied) < 1 ? 1 : (high + applied);
		d.physicalDamage.low  = static_cast<uint32_t>(low);
		d.physicalDamage.high = static_cast<uint32_t>(high);

		d.resistances.fire     = in.passives.resistances.fire     + in.items.resistances.fire     + static_cast<int>(in.codex.resistance);
		d.resistances.ice      = in.passives.resistances.ice      + in.items.resistances.ice      + static_cast<int>(in.codex.resistance);
		d.resistances.electric = in.passives.resistances.electric + in.items.resistances.electric + static_cast<int>(in.codex.resistance);
		d.resistances.poison   = in.passives.resistances.poison   + in.items.resistances.poison   + static_cast<int>(in.codex.resistance);
		d.resistances.spirit   = in.passives.resistances.spirit   + in.items.resistances.spirit   + static_cast<int>(in.codex.resistance);
		d.resistances.ClampNonNegative();

		d.hpRecoveryRate = RecoveryRateConstant::kHp + in.items.hpRecoveryRate + in.passives.hpRecoveryRate;
		d.mpRecoveryRate = RecoveryRateConstant::kMp + in.items.mpRecoveryRate + in.passives.mpRecoveryRate;
		d.spRecoveryRate = RecoveryRateConstant::kSp + in.items.spRecoveryRate + in.passives.spRecoveryRate;

		return d;
	}

	// ------------------------------------------------------------------------
	// A class row with every field a formula reads, so a test never has to
	// remember to set one.
	// ------------------------------------------------------------------------

	ClassConstants StandardClass()
	{
		ClassConstants cc;
		cc.beginStats.pow = 10; cc.beginStats.str = 20;
		cc.beginStats.spi = 15; cc.beginStats.dex = 25;
		cc.beginStats.intel = 8; cc.beginStats.sta = 12;

		cc.beginAttackPoint = 10;
		cc.beginDefensePoint = 5;
		cc.beginMeleePower = 3;
		cc.beginShootPower = 4;

		cc.attackPointConversion  = 1.0f;
		cc.defensePointConversion = 1.0f;
		cc.meleePowerConversion   = 1.0f;
		cc.shootPowerConversion   = 1.0f;

		cc.hpPerStr = 5.0f;
		cc.mpPerSpi = 4.0f;
		cc.spPerSta = 2.0f;

		cc.hitPerDex     = 2.0f;
		cc.avoidPerDex   = 1.0f;
		cc.defensePerDex = 3.0f;

		cc.meleePerPow = 1.0f;  cc.meleePerDex = 0.5f;
		cc.shootPerPow = 1.0f;  cc.shootPerDex = 0.5f;
		cc.magicPerDex = 1.0f;  cc.magicPerSpi = 1.0f;  cc.magicPerIntel = 2.0f;
		return cc;
	}

	StatCalculationInput StandardInput()
	{
		StatCalculationInput in;
		in.characterClass = CharClassIndex::BrawlerMale;
		in.level          = kMinLevel;
		in.classConstants = StandardClass();
		in.confPointRate  = 1.0f;
		return in;
	}

	// Asserts the calculator and the oracle agree, field by field, so a
	// mismatch says which value moved.
	void CheckMatchesOracle(const StatCalculationInput& input, const char* what)
	{
		const Result<DerivedStats> actual = Calculate(input);
		CHECK(actual.IsOk());
		if (actual.IsError())
		{
			std::printf("      (%s refused)\n", what);
			return;
		}
		const DerivedStats expected = Oracle(input);
		const DerivedStats& got = actual.GetValue();
		if (!(got == expected))
		{
			std::printf("      oracle mismatch in %s\n", what);
		}
		CHECK(got == expected);
	}
}

// ---------------------------------------------------------------------------
// Base stats
// ---------------------------------------------------------------------------

MODERN_TEST(Stats_BaseStatsDefaultsToZero)
{
	const BaseStats stats;
	CHECK_EQ(stats.pow, 0);
	CHECK_EQ(stats.str, 0);
	CHECK_EQ(stats.spi, 0);
	CHECK_EQ(stats.dex, 0);
	CHECK_EQ(stats.intel, 0);
	CHECK_EQ(stats.sta, 0);
	CHECK(stats.IsZero());
	CHECK_EQ(stats.Total(), 0);
}

MODERN_TEST(Stats_BaseStatsTotalSumsAllSix)
{
	BaseStats stats;
	stats.pow = 1; stats.str = 2; stats.spi = 4;
	stats.dex = 8; stats.intel = 16; stats.sta = 32;
	CHECK_EQ(stats.Total(), 63);
	CHECK(!stats.IsZero());
}

MODERN_TEST(Stats_BaseStatsTotalWrapsLikeRan)
{
	// SCHARSTATS::GetTotal sums in WORD, so a total past 65535 wraps. A
	// widened accumulator would report 65536 here.
	BaseStats stats;
	stats.pow = 60000; stats.str = 6000;
	stats.spi = 0; stats.dex = 0; stats.intel = 0; stats.sta = 0;
	CHECK_EQ(stats.Total(), static_cast<uint16_t>(60000 + 6000));
	CHECK_EQ(stats.Total(), 464);
}

MODERN_TEST(Stats_ClassIndexCoversTheSixteenRanValues)
{
	// Values verified one-for-one against legacy EMCHARINDEX.
	CHECK_EQ(static_cast<uint8_t>(CharClassIndex::BrawlerMale), 0);
	CHECK_EQ(static_cast<uint8_t>(CharClassIndex::SwordsmanMale), 1);
	CHECK_EQ(static_cast<uint8_t>(CharClassIndex::ArcherFemale), 2);
	CHECK_EQ(static_cast<uint8_t>(CharClassIndex::ShamanFemale), 3);
	CHECK_EQ(static_cast<uint8_t>(CharClassIndex::ExtremeMale), 4);
	CHECK_EQ(static_cast<uint8_t>(CharClassIndex::ExtremeFemale), 5);
	CHECK_EQ(static_cast<uint8_t>(CharClassIndex::BrawlerFemale), 6);
	CHECK_EQ(static_cast<uint8_t>(CharClassIndex::SwordsmanFemale), 7);
	CHECK_EQ(static_cast<uint8_t>(CharClassIndex::ArcherMale), 8);
	CHECK_EQ(static_cast<uint8_t>(CharClassIndex::ShamanMale), 9);
	CHECK_EQ(static_cast<uint8_t>(CharClassIndex::GunnerMale), 10);
	CHECK_EQ(static_cast<uint8_t>(CharClassIndex::GunnerFemale), 11);
	CHECK_EQ(static_cast<uint8_t>(CharClassIndex::AssassinMale), 12);
	CHECK_EQ(static_cast<uint8_t>(CharClassIndex::AssassinFemale), 13);
	CHECK_EQ(static_cast<uint8_t>(CharClassIndex::TrickerMale), 14);
	CHECK_EQ(static_cast<uint8_t>(CharClassIndex::TrickerFemale), 15);
	CHECK_EQ(kClassCount, 16);

	CHECK(IsValidClass(CharClassIndex::TrickerFemale));
	CHECK(!IsValidClass(static_cast<CharClassIndex>(16)));
}

MODERN_TEST(Stats_LevelRangeMatchesRan)
{
	CHECK_EQ(kMinLevel, 1);
	CHECK_EQ(kMaxLevel, 255);
	CHECK(IsValidLevel(1));
	CHECK(IsValidLevel(255));
	CHECK(!IsValidLevel(0));
}

// ---------------------------------------------------------------------------
// Validation
// ---------------------------------------------------------------------------

MODERN_TEST(Stats_RejectsInvalidClass)
{
	StatCalculationInput input = StandardInput();
	input.characterClass = static_cast<CharClassIndex>(16);
	const Result<DerivedStats> result = Calculate(input);
	CHECK(result.IsError());
	CHECK_EQ(result.GetError(), ErrorCode::InvalidArgument);
}

MODERN_TEST(Stats_RejectsOutOfRangeLevel)
{
	for (const uint16_t level : { uint16_t{0}, uint16_t{256}, uint16_t{1000} })
	{
		StatCalculationInput input = StandardInput();
		input.level = level;
		const Result<DerivedStats> result = Calculate(input);
		CHECK(result.IsError());
		CHECK_EQ(result.GetError(), ErrorCode::InvalidArgument);
	}
}

MODERN_TEST(Stats_RejectsNonFiniteCoefficients)
{
	StatCalculationInput input = StandardInput();
	input.classConstants.hpPerStr = std::numeric_limits<float>::quiet_NaN();
	CHECK(Calculate(input).IsError());

	input = StandardInput();
	input.classConstants.levelUpStats.dex = std::numeric_limits<float>::infinity();
	CHECK(Calculate(input).IsError());

	input = StandardInput();
	input.classConstants.meleePerPow = -std::numeric_limits<float>::infinity();
	CHECK(Calculate(input).IsError());
}

MODERN_TEST(Stats_RejectsNonFiniteContributions)
{
	StatCalculationInput input = StandardInput();
	input.items.hitRatePercent = std::numeric_limits<float>::quiet_NaN();
	CHECK(Calculate(input).IsError());

	input = StandardInput();
	input.passives.hpRate = std::numeric_limits<float>::infinity();
	CHECK(Calculate(input).IsError());

	input = StandardInput();
	input.confPointRate = std::numeric_limits<float>::quiet_NaN();
	CHECK(Calculate(input).IsError());
}

// ---------------------------------------------------------------------------
// Literal regression fixtures
// ---------------------------------------------------------------------------

MODERN_TEST(Stats_LevelOneNoContributions)
{
	// Read off the formula by hand, with every coefficient a whole number and
	// no percentage modifier, so the float arithmetic is exact.
	//
	//   totalStats = beginStats, because level 1 means the growth term is zero
	//   attackPoint  = WORD((10 + 0*0) * 1)                 = 10
	//   defensePoint = WORD((5  + 0*0) * 1)                 = 5
	//   meleePower   = 3 + WORD(10*1.0 + 25*0.5)            = 3 + 22 = 25
	//   shootPower   = 4 + WORD(10*1.0 + 25*0.5)            = 4 + 22 = 26
	//   magicAttack  = WORD(25*1 + 15*1 + 8*2)              = 56
	//   maxHp        = DWORD(20*5)                         = 100
	//   maxMp        = DWORD(15*4)                         = 60
	//   maxSp        = DWORD(12*2)                         = 24
	//   hit          = int(25*2) then int(50*100*0.01)     = 50
	//   avoid        = int(25*1) then int(25*100*0.01)     = 25
	//   defenseBody  = int(5 + 25*3)                       = 80
	//   damage       = int(10) + 25 (melee power)          = 35
	const Result<DerivedStats> result = Calculate(StandardInput());
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	const DerivedStats& d = result.GetValue();

	CHECK_EQ(d.totalStats.pow, 10);
	CHECK_EQ(d.totalStats.str, 20);
	CHECK_EQ(d.totalStats.spi, 15);
	CHECK_EQ(d.totalStats.dex, 25);
	CHECK_EQ(d.totalStats.intel, 8);
	CHECK_EQ(d.totalStats.sta, 12);

	CHECK_EQ(d.attackPoint, 10);
	CHECK_EQ(d.defensePoint, 5);
	CHECK_EQ(d.meleePower, 25);
	CHECK_EQ(d.shootPower, 26);
	CHECK_EQ(d.magicAttack, 56);

	CHECK_EQ(d.maxHp, 100u);
	CHECK_EQ(d.maxMp, 60u);
	CHECK_EQ(d.maxSp, 24u);

	CHECK_EQ(d.hit, 50);
	CHECK_EQ(d.avoid, 25);
	CHECK_EQ(d.defenseBody, 80);
	CHECK_EQ(d.defense, 80);
	CHECK_EQ(d.physicalDamage.low, 35u);
	CHECK_EQ(d.physicalDamage.high, 35u);

	CHECK_EQ(d.resistances.fire, 0);
	CHECK_EQ(d.resistances.spirit, 0);

	CHECK_EQ(d.hpRecoveryRate, 0.3f * 0.01f);
	CHECK_EQ(d.mpRecoveryRate, 0.3f * 0.01f);
	CHECK_EQ(d.spRecoveryRate, 0.5f * 0.01f);
}

MODERN_TEST(Stats_LevelUpFractionTruncatesPerField)
{
	// The discriminator for the level term. A growth rate of 0.5 per level
	// truncates: at level 4 (ZBLEVEL 3) it contributes WORD(1.5) = 1, not 1.5,
	// and at level 2 (ZBLEVEL 1) it contributes WORD(0.5) = 0.
	//
	//   level 2: WORD(0.5 * 1) = 0
	//   level 4: WORD(0.5 * 3) = WORD(1.5) = 1
	//   level 8: WORD(0.5 * 7) = WORD(3.5) = 3
	//
	// Rounding, or carrying the float through, would give 1, 2 and 4.
	StatCalculationInput input = StandardInput();
	input.classConstants.levelUpStats.dex = 0.5f;
	input.allocatedStats.dex = 0;

	struct LevelCase
	{
		uint16_t level;
		uint16_t expectedDex;
	};
	for (const LevelCase& levelCase : { LevelCase{ 2, 0 }, LevelCase{ 4, 1 }, LevelCase{ 8, 3 } })
	{
		input.level = levelCase.level;
		const Result<DerivedStats> result = Calculate(input);
		CHECK(result.IsOk());
		if (result.IsError())
		{
			continue;
		}
		// beginStats.dex is 25, so the total is 25 plus the truncated growth.
		CHECK_EQ(result.GetValue().totalStats.dex,
		         static_cast<uint16_t>(25 + levelCase.expectedDex));
	}
}

MODERN_TEST(Stats_ResourceMaxKeepsBothTruncations)
{
	// The discriminator for the resource maximum. RAN truncates to 32 bits
	// twice: once on the raw sum, once after the rate and point-rate multiply.
	//
	//   stat 205, coefficient 0.35: raw = 71.75
	//     keeping both truncations: DWORD(71.75) = 71, then
	//                               DWORD(71 * 1.5 * 1.0) = DWORD(106.5) = 106
	//     dropping the first:     DWORD(71.75 * 1.5)     = DWORD(107.625) = 107
	//
	// The answer is 106. A build that folds the two together reports 107 and
	// disagrees with a shipped client on every such character.
	StatCalculationInput input = StandardInput();
	input.classConstants.beginStats.str = 0;
	input.classConstants.hpPerStr = 0.35f;
	input.classConstants.levelUpStats.str = 0.0f;
	input.allocatedStats.str = 205;
	input.passives.hpRate = 0.5f;
	input.confPointRate = 1.0f;

	const Result<DerivedStats> result = Calculate(input);
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	CHECK_EQ(result.GetValue().totalStats.str, 205);
	CHECK_EQ(result.GetValue().maxHp, 106u);
}

MODERN_TEST(Stats_CodexFlatIsAddedAfterTruncation)
{
	// The codex bonus is added last, in 32-bit unsigned, so it is never
	// diluted by the rate and never truncated away. Adding it before the rate
	// would give 75 * 1.5 = 112 instead of 100 + 50 = 150.
	StatCalculationInput input = StandardInput();
	input.codex.hp = 50;
	const Result<DerivedStats> result = Calculate(input);
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	CHECK_EQ(result.GetValue().maxHp, 150u);
}

MODERN_TEST(Stats_StatSumWrapsAtSixteenBits)
{
	// The stat sum is 16-bit and wraps. 60000 begin + 6000 allocated is 12000
	// after wrapping, not 66000; a widened sum would change every downstream
	// derived value that reads it.
	StatCalculationInput input = StandardInput();
	input.classConstants.beginStats.str = 0;
	input.classConstants.levelUpStats.str = 0.0f;
	input.allocatedStats.str = 60000;
	input.items.stats.str = 6000;

	const Result<DerivedStats> result = Calculate(input);
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	CHECK_EQ(result.GetValue().totalStats.str, static_cast<uint16_t>(60000 + 6000));
	CHECK_EQ(result.GetValue().totalStats.str, 464);
}

MODERN_TEST(Stats_AttackPowerClampsInsteadOfWrapping)
{
	// A large equipment bonus goes through RAN's VARIATION, which clamps to
	// [0, 65535]. Adding it as 16-bit arithmetic would wrap instead, so 60000
	// would become 60000 - 65535 = 60001 rather than the clamp.
	StatCalculationInput input = StandardInput();
	input.items.meleePower = 70000;
	const Result<DerivedStats> result = Calculate(input);
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	CHECK_EQ(result.GetValue().meleePower, 65535);

	// A large negative bonus clamps at zero rather than wrapping.
	input = StandardInput();
	input.items.meleePower = -70000;
	const Result<DerivedStats> floored = Calculate(input);
	CHECK(floored.IsOk());
	if (floored.IsError())
	{
		return;
	}
	CHECK_EQ(floored.GetValue().meleePower, 0);
}

MODERN_TEST(Stats_DamageRangeHasAFloorOfOne)
{
	// GLDWDATA::VAR_PARAM never lets a damage range fall below 1, even when the
	// attack power is small enough to drive it negative.
	StatCalculationInput input = StandardInput();
	input.classConstants.beginAttackPoint = 0;
	input.classConstants.levelUpAttackPoint = 0.0f;
	input.classConstants.attackPointConversion = 1.0f;
	input.passives.damage = 0;
	input.codex.attack = 0;
	input.classConstants.beginMeleePower = 0;
	input.classConstants.meleePerPow = 0.0f;
	input.classConstants.meleePerDex = 0.0f;

	const Result<DerivedStats> result = Calculate(input);
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	CHECK_EQ(result.GetValue().meleePower, 0);
	CHECK_EQ(result.GetValue().physicalDamage.low, 1u);
	CHECK_EQ(result.GetValue().physicalDamage.high, 1u);
}

MODERN_TEST(Stats_ResistanceFloorsAtZero)
{
	// SRESIST::LIMIT floors each element at zero after summing.
	StatCalculationInput input = StandardInput();
	input.passives.resistances.fire = 5;
	input.items.resistances.fire = -20;
	const Result<DerivedStats> result = Calculate(input);
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	CHECK_EQ(result.GetValue().resistances.fire, 0);
	CHECK_EQ(result.GetValue().resistances.ice, 0);
}

MODERN_TEST(Stats_HitPercentageScalesTheComputedValue)
{
	// RAN's ordering is int(v * (100 + percent) * 0.01f). A negative percent
	// reduces, and the truncation is toward zero.
	StatCalculationInput input = StandardInput();
	input.items.hitRatePercent = 10.0f;
	Result<DerivedStats> result = Calculate(input);
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	CHECK_EQ(result.GetValue().hit, 55);   // 50 * 110 * 0.01

	input = StandardInput();
	input.items.avoidRatePercent = -10.0f;
	result = Calculate(input);
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	CHECK_EQ(result.GetValue().avoid, 22);  // 25 * 90 * 0.01 = 22.5 -> 22
}

MODERN_TEST(Stats_ZeroContributionsReproduceTheBaseCase)
{
	// The default-constructed contributions must not perturb anything, which
	// is what makes the level-1 fixture above a baseline.
	StatCalculationInput plain = StandardInput();
	StatCalculationInput zeroed = StandardInput();
	zeroed.items = ItemContribution();
	zeroed.passives = PassiveContribution();
	zeroed.codex = CodexContribution();

	const Result<DerivedStats> a = Calculate(plain);
	const Result<DerivedStats> b = Calculate(zeroed);
	CHECK(a.IsOk());
	CHECK(b.IsOk());
	if (a.IsError() || b.IsError())
	{
		return;
	}
	CHECK(a.GetValue() == b.GetValue());
}

// ---------------------------------------------------------------------------
// Oracle agreement across the ranges
// ---------------------------------------------------------------------------

MODERN_TEST(Stats_AgreesWithOracleOnALevelSweep)
{
	// Every level from 1 to 255 against the oracle, which is what catches a
	// level term that is off by one.
	for (uint16_t level = 1; level <= 255; ++level)
	{
		StatCalculationInput input = StandardInput();
		input.level = level;
		input.classConstants.levelUpStats.pow = 1.5f;
		input.classConstants.levelUpStats.str = 2.5f;
		input.classConstants.levelUpStats.dex = 0.5f;
		input.classConstants.levelUpAttackPoint = 0.25f;
		input.allocatedStats.pow = 3;
		input.allocatedStats.dex = 2;
		input.items.stats.spi = 1;
		input.items.hp = 17;
		input.items.defense = 5;
		input.passives.hit = 3;
		input.codex.attack = 9;
		CheckMatchesOracle(input, "level sweep");
	}
}

MODERN_TEST(Stats_AgreesWithOracleOnContributionCombinations)
{
	// Every contribution source at once, and each alone, against the oracle.
	StatCalculationInput base = StandardInput();
	base.level = 40;
	base.classConstants.levelUpStats.str = 1.5f;
	base.classConstants.hpPerStr = 3.7f;
	base.classConstants.mpPerSpi = 0.35f;
	base.classConstants.defensePerDex = 1.25f;
	base.allocatedStats.str = 11;

	ItemContribution items;
	items.stats.dex = 4;
	items.hp = 23; items.mp = -7; items.sp = 5;
	items.meleePower = 9; items.shootPower = -3; items.magicAttack = 2;
	items.hit = 6; items.avoid = -4;
	items.hitRatePercent = 12.5f; items.avoidRatePercent = -7.5f;
	items.defense = 8;
	items.damageLow = -20; items.damageHigh = 40;
	items.hpRecoveryRate = 0.001f; items.mpRecoveryRate = 0.002f; items.spRecoveryRate = 0.003f;
	items.resistances.fire = 3; items.resistances.ice = -9;

	PassiveContribution passives;
	passives.hp = 15; passives.mp = -4; passives.sp = 2;
	passives.hpRate = 0.25f; passives.mpRate = -0.1f; passives.spRate = 0.05f;
	passives.hpRecoveryRate = 0.004f; passives.mpRecoveryRate = 0.005f; passives.spRecoveryRate = 0.006f;
	passives.meleePower = 7; passives.shootPower = 11; passives.magicAttack = -3;
	passives.hit = 2; passives.avoid = 3; passives.defense = -5; passives.damage = 6;
	passives.resistances.poison = 4; passives.resistances.spirit = -12;

	CodexContribution codex;
	codex.hp = 100; codex.mp = 50; codex.sp = 25;
	codex.attack = 30; codex.defense = 20;
	codex.shootPower = 12; codex.meleePower = 14; codex.magicAttack = 6;
	codex.resistance = 3; codex.hit = 7; codex.avoid = 4;

	CheckMatchesOracle(base, "no contributions");
	CheckMatchesOracle([&] { auto i = base; i.items = items; return i; }(), "items only");
	CheckMatchesOracle([&] { auto i = base; i.passives = passives; return i; }(), "passives only");
	CheckMatchesOracle([&] { auto i = base; i.codex = codex; return i; }(), "codex only");
	CheckMatchesOracle([&] { auto i = base; i.items = items; i.passives = passives; return i; }(), "items and passives");
	CheckMatchesOracle([&] { auto i = base; i.items = items; i.codex = codex; return i; }(), "items and codex");
	CheckMatchesOracle([&] { auto i = base; i.passives = passives; i.codex = codex; return i; }(), "passives and codex");
	CheckMatchesOracle([&] { auto i = base; i.items = items; i.passives = passives; i.codex = codex; return i; }(), "all three");
}

MODERN_TEST(Stats_AgreesWithOracleOnEveryClass)
{
	// The class row is data, so all sixteen must behave identically given the
	// same row. This guards against a class value that is used as an index into
	// something sized differently.
	for (uint8_t index = 0; index < kClassCount; ++index)
	{
		StatCalculationInput input = StandardInput();
		input.characterClass = static_cast<CharClassIndex>(index);
		input.level = 17;
		input.classConstants.hpPerStr = 4.2f;
		input.classConstants.levelUpStats.str = 1.1f;
		input.allocatedStats.str = 30;
		input.items.hp = 9;
		input.codex.hp = 20;
		CheckMatchesOracle(input, "class sweep");
	}
}

MODERN_TEST(Stats_AgreesWithOracleOnConfPointRateSweep)
{
	for (const float rate : { 0.0f, 0.25f, 0.5f, 1.0f, 2.0f, 10.0f })
	{
		StatCalculationInput input = StandardInput();
		input.confPointRate = rate;
		input.level = 12;
		input.classConstants.hpPerStr = 2.5f;
		input.allocatedStats.str = 44;
		input.passives.hpRate = 0.15f;
		input.items.hp = 7;
		input.codex.hp = 3;
		CheckMatchesOracle(input, "conf point rate sweep");
	}
}

MODERN_TEST(Stats_AgreesWithOracleAtExtremeContributions)
{
	// Large and negative values, including the places where RAN's casts are
	// undefined and this implementation saturates. Agreement with the oracle
	// here confirms both saturate the same way.
	for (const int32_t magnitude : { 0, 1, 1000, 100000, 2000000000 })
	{
		StatCalculationInput input = StandardInput();
		input.level = 255;
		input.classConstants.levelUpStats.str = 3.3f;
		input.classConstants.hpPerStr = 12.5f;
		input.classConstants.mpPerSpi = 40.0f;
		input.allocatedStats.str = 60000;
		input.allocatedStats.spi = 60000;
		input.items.hp = magnitude;
		input.items.meleePower = magnitude;
		input.items.defense = -magnitude;
		input.passives.hpRate = 0.9f;
		input.codex.hp = 100000;
		input.codex.meleePower = 60000;
		CheckMatchesOracle(input, "extreme contributions");
	}
}

// ---------------------------------------------------------------------------
// Determinism
// ---------------------------------------------------------------------------

MODERN_TEST(Stats_RepeatedCalculationIsIdentical)
{
	const StatCalculationInput input = [&] {
		StatCalculationInput i = StandardInput();
		i.level = 63;
		i.classConstants.hpPerStr = 6.25f;
		i.classConstants.levelUpStats.dex = 1.5f;
		i.allocatedStats.dex = 17;
		i.items.hp = 55;
		i.items.hitRatePercent = 13.5f;
		i.passives.hpRate = 0.2f;
		i.codex.attack = 17;
		return i;
	}();

	const Result<DerivedStats> first = Calculate(input);
	CHECK(first.IsOk());
	if (first.IsError())
	{
		return;
	}
	for (int repeat = 0; repeat < 64; ++repeat)
	{
		const Result<DerivedStats> again = Calculate(input);
		CHECK(again.IsOk());
		if (again.IsError())
		{
			return;
		}
		CHECK(again.GetValue() == first.GetValue());
	}
}

MODERN_TEST(Stats_InputIsNotMutated)
{
	StatCalculationInput input = StandardInput();
	input.level = 30;
	input.classConstants.hpPerStr = 4.5f;
	input.allocatedStats.str = 12;
	input.items.hp = 3;
	input.passives.hpRate = 0.1f;
	input.codex.hp = 5;

	const BaseStats         beforeStats = input.allocatedStats;
	const ClassConstants    beforeClass = input.classConstants;
	const ItemContribution  beforeItems = input.items;
	const PassiveContribution beforePassives = input.passives;
	const CodexContribution beforeCodex = input.codex;
	const uint16_t         beforeLevel = input.level;
	const float            beforeRate = input.confPointRate;

	const Result<DerivedStats> result = Calculate(input);
	CHECK(result.IsOk());

	CHECK(input.allocatedStats == beforeStats);
	CHECK(input.level == beforeLevel);
	CHECK(input.confPointRate == beforeRate);
	CHECK(input.items == beforeItems);
	CHECK(input.passives == beforePassives);
	CHECK(input.codex == beforeCodex);
	CHECK(input.classConstants.hpPerStr == beforeClass.hpPerStr);
	CHECK(input.classConstants.beginStats.str == beforeClass.beginStats.str);
}

MODERN_TEST(Stats_DerivedValueIsIndependentOfInput)
{
	// The result must not alias the caller's data: a later change to the
	// input cannot reach into an already-returned value.
	StatCalculationInput input = StandardInput();
	input.level = 20;
	input.allocatedStats.str = 15;

	Result<DerivedStats> result = Calculate(input);
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	const uint32_t capturedHp = result.GetValue().maxHp;
	const uint16_t capturedStr = result.GetValue().totalStats.str;

	input.classConstants.hpPerStr = 9999.0f;
	input.allocatedStats.str = 1;

	CHECK_EQ(result.GetValue().maxHp, capturedHp);
	CHECK_EQ(result.GetValue().totalStats.str, capturedStr);
}

// ═══════════════════════════════════════════════════════════════════════
// VERTICAL-021: the defence-rate axis
// ═══════════════════════════════════════════════════════════════════════

namespace
{
	// A minimal, fully specified stat input so only the defence axis can move.
	StatCalculationInput MakeDefenseInput(int32_t itemDefense, int32_t passiveDefense,
	                                     int32_t factDefense,
	                                     float passiveRate, float factRate)
	{
		StatCalculationInput in;
		in.level          = 1;
		in.characterClass = CharClassIndex::BrawlerMale;
		in.classConstants.avoidPerDex    = 0.0f;
		in.classConstants.hitPerDex      = 0.0f;
		in.classConstants.defensePerDex  = 0.0f;
		in.classConstants.beginDefensePoint = 0;
		in.classConstants.levelUpDefensePoint = 0.0f;
		in.classConstants.defensePointConversion = 1.0f;

		in.items.defense      = itemDefense;
		in.passives.defense   = passiveDefense;
		in.facts.defense      = factDefense;
		in.passives.defenseRate = passiveRate;
		in.facts.defenseRate    = factRate;
		return in;
	}

	int32_t DefenseOf(const StatCalculationInput& in)
	{
		const Result<DerivedStats> r = Calculate(in);
		CHECK(r.IsOk());
		return r.IsOk() ? r.GetValue().defense : -1;
	}
}

// The no-buff baseline must NOT move. VERTICAL-020 deferred this axis on the
// belief that legacy forced a minimum of 1; that reading was wrong - the clamp
// is `result < 0`, so zero stays zero.
MODERN_TEST(DefenseRate_NoBuffBaselineIsUnchanged)
{
	CHECK_EQ(DefenseOf(MakeDefenseInput(0, 0, 0, 0.0f, 0.0f)), 0);
	CHECK_EQ(DefenseOf(MakeDefenseInput(0, 0, 0, 1.0f, 0.0f)), 0);
}

MODERN_TEST(DefenseRate_NormalDefenseIsUnchangedAtRateOne)
{
	CHECK_EQ(DefenseOf(MakeDefenseInput(50, 0, 0, 0.0f, 0.0f)), 50);
}

MODERN_TEST(DefenseRate_PositiveRateMultipliesTheSummedDefence)
{
	// 50 * 1.5 = 75
	CHECK_EQ(DefenseOf(MakeDefenseInput(50, 0, 0, 0.0f, 0.5f)), 75);
}

MODERN_TEST(DefenseRate_ZeroDefenceStaysZeroEvenWithAPositiveRate)
{
	// The clamp is `result < 0`, not `<= 0`, so 0 * 1.5 is 0 and survives.
	CHECK_EQ(DefenseOf(MakeDefenseInput(0, 0, 0, 0.0f, 0.5f)), 0);
}

// ORDERING: the rate multiplies the ALREADY-SUMMED defence including the FACT
// flat bonus - not the base before the bonuses.
MODERN_TEST(DefenseRate_RateAppliesAfterFlatBonusesIncludingTheFact)
{
	// (50 + 10) * 1.5 = 90. If the rate applied to the base alone and the FACT
	// were added afterwards it would be 50*1.5 + 10 = 85.
	CHECK_EQ(DefenseOf(MakeDefenseInput(50, 0, 10, 0.0f, 0.5f)), 90);
}

MODERN_TEST(DefenseRate_TruncationIsTowardZero)
{
// The rate field is the INCREMENT added to 1.0, not the multiplier itself,
	// so this is a 1.333 multiplier: int(50 * 1.333) = int(66.65) = 66.
	CHECK_EQ(DefenseOf(MakeDefenseInput(50, 0, 0, 0.0f, 0.333f)), 66);
}

MODERN_TEST(DefenseRate_NegativeResultBecomesOne)
{
// 10 * (1 - 1.5) = -5, which is < 0, so it becomes 1.
	CHECK_EQ(DefenseOf(MakeDefenseInput(10, 0, 0, 0.0f, -1.5f)), 1);
	// ...but a result of exactly 0 is NOT below zero and stays 0:
	// 10 * (1 - 1.0) = 0.
	CHECK_EQ(DefenseOf(MakeDefenseInput(10, 0, 0, 0.0f, -1.0f)), 0);
}

MODERN_TEST(DefenseRate_PermanentAndFactRatesSum)
{
	// (40 + 4) * (1 + 0.1 + 0.2) = int(44 * 1.3) = int(57.2) = 57
	CHECK_EQ(DefenseOf(MakeDefenseInput(40, 0, 4, 0.1f, 0.2f)), 57);
}