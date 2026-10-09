// WORLD-ENTRY-002L-A: the recovered class-constant table.
//
// What these cases pin is the part a compile-time assertion cannot: that every
// legacy class/gender combination has a row, that a row either carries verified
// data or is explicitly refused, and that the validation refuses the three
// ways a row could otherwise lie - a bad index, a non-finite coefficient, and
// provenance that cannot be stated.
//
// The hand-computed arithmetic at the end is arithmetic, NOT RAN data. The row
// it uses is built by the test and labelled as a test fixture, because the
// repository contains no `.classconst` rows to recover (see the table header).
// What that case proves is that Stats::Calculate and the table wiring agree,
// so that when a real row is filled in the same numbers come out the other
// end - not that any particular number is RAN's.

#include "TestHarness.h"
#include "stats/BaseStats.h"
#include "stats/ClassConstantTable.h"
#include "stats/Contributions.h"
#include "stats/DerivedStats.h"
#include "stats/StatCalculator.h"

#include <cstdio>
#include <limits>
#include <string>

namespace ModernTests
{
	using namespace Modern;
	using namespace Modern::Stats;

	namespace
	{
		// A test-fixture row. Every field is set, and the values are chosen so
		// the arithmetic is checkable by hand (multipliers of 1 and 0.5, a
		// level-up growth that stays integral). NOT legacy data.
		ClassConstantRow MakeTestRow(CharClassIndex index)
		{
			ClassConstantRow row;
			row.index  = index;
			row.source = CoefficientSource::Recovered;
			row.note   = "test fixture: ClassConstantTableTests, not RAN data";

			ClassConstants& cc = row.constants;
			cc.beginStats.pow   = 10;
			cc.beginStats.str   = 20;
			cc.beginStats.spi   = 15;
			cc.beginStats.dex   = 25;
			cc.beginStats.intel = 8;
			cc.beginStats.sta   = 12;

			cc.beginAttackPoint  = 10;
			cc.beginDefensePoint = 5;
			cc.beginMeleePower    = 3;
			cc.beginShootPower    = 4;

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

			// Per-level growth of 0.5 dex: at level 3 the growth term is
			// 0.5 * (3-1) = 1.0, which truncates to 1 and keeps the hand
			// arithmetic exact.
			cc.levelUpStats.dex = 0.5f;

			return row;
		}

		StatCalculationInput TestInput(CharClassIndex index, uint16_t level)
		{
			StatCalculationInput in;
			in.characterClass = index;
			in.level          = level;
			in.confPointRate  = 1.0f;
			return in;
		}
	}

	// ---- the shipped table -------------------------------------------------

	MODERN_TEST(ClassConstantTable_EveryLegacyClassHasARow)
	{
		const ClassConstantTable& table = ClassConstantTable::Verified();

		CHECK_EQ(table.kRowCount, static_cast<std::size_t>(16));

		// All sixteen legacy EMCHARINDEX values resolve, one-for-one. This is
		// the loop over the enum rather than a list of sixteen literals, so a
		// new index cannot be silently dropped.
		for (uint8_t raw = 0; raw < static_cast<uint8_t>(CharClassIndex::TrickerFemale) + 1;
		     ++raw)
		{
			const auto index = static_cast<CharClassIndex>(raw);
			const ClassConstantRow* row = table.Find(index);
			REQUIRE(row != nullptr);
			CHECK_EQ(static_cast<int>(row->index), static_cast<int>(index));
		}
	}

	MODERN_TEST(ClassConstantTable_RowAtWalksEveryRowWithoutDuplicates)
	{
		const ClassConstantTable& table = ClassConstantTable::Verified();

		bool seen[ClassConstantTable::kRowCount] = {};

		for (std::size_t position = 0; position < ClassConstantTable::kRowCount; ++position)
		{
			const ClassConstantRow& row = table.RowAt(position);
			const auto raw = static_cast<uint8_t>(row.index);
			REQUIRE(raw < ClassConstantTable::kRowCount);
			// A duplicate index would mean two rows claim one class and a
			// caller cannot tell which is authoritative.
			CHECK(!seen[raw]);
			seen[raw] = true;
		}

		for (std::size_t position = 0; position < ClassConstantTable::kRowCount; ++position)
		{
			CHECK(seen[position]);
		}
	}

	MODERN_TEST(ClassConstantTable_AnOutOfRangeIndexIsRefusedNotGuessed)
	{
		const ClassConstantTable& table = ClassConstantTable::Verified();

		// 16 and 255 are not legacy EMCHARINDEX values. A nullptr here is the
		// table saying "no such class", which is different from "no data for a
		// known class".
		for (uint8_t raw : { static_cast<uint8_t>(16), static_cast<uint8_t>(17),
		                     static_cast<uint8_t>(255) })
		{
			CHECK(table.Find(static_cast<CharClassIndex>(raw)) == nullptr);
		}
	}

	// ---- provenance --------------------------------------------------------

	MODERN_TEST(ClassConstantTable_EveryRowStatesItsProvenance)
	{
		const ClassConstantTable& table = ClassConstantTable::Verified();

		for (std::size_t position = 0; position < ClassConstantTable::kRowCount; ++position)
		{
			const ClassConstantRow& row = table.RowAt(position);

			// A row that cannot explain itself has not been recorded.
			REQUIRE(row.note != nullptr);
			CHECK(row.note[0] != '\0');

			// And the explanation names the missing artifact and the legacy
			// loader that would have supplied it, so a reader knows what to go
			// and find rather than only being told it is missing.
			const std::string note(row.note);
			CHECK(note.find("classconst") != std::string::npos);
			CHECK(note.find("GLogicDataLoad.cpp") != std::string::npos);
		}
	}

	MODERN_TEST(ClassConstantTable_NoRowIsLabelledRecoveredWhenItIsNot)
	{
		const ClassConstantTable& table = ClassConstantTable::Verified();

		// The invariant the whole milestone rests on. The repository has no
		// `.classconst` data, so NOTHING may present itself as recovered - and a
		// row that did would be a constructor default wearing the wrong label.
		CHECK_EQ(table.RecoveredCount(), static_cast<std::size_t>(0));

		for (std::size_t position = 0; position < ClassConstantTable::kRowCount; ++position)
		{
			CHECK_EQ(static_cast<int>(table.RowAt(position).source),
			         static_cast<int>(CoefficientSource::Unavailable));
		}
	}

	MODERN_TEST(ClassConstantTable_TheConstructorValuesAreNotSmuggledIn)
	{
		// GLogicData.cpp:609's first row is brawler-male with
		// SCHARSTATS(15,13,20,10,0,20), FCHARSTATS(2,3,1,1,1,3),
		// wBEGIN_AP=35, fCONV_AP=2 etc. If any of that ever leaked into an
		// Unavailable row, this case fails - which is the whole point of
		// zero-filling an unavailable row instead of defaulting it to the
		// legacy initialiser.
		const ClassConstantTable& table = ClassConstantTable::Verified();

		const ClassConstantRow* row = table.Find(CharClassIndex::BrawlerMale);
		REQUIRE(row != nullptr);
		REQUIRE(row->source == CoefficientSource::Unavailable);

		CHECK(row->constants.beginStats.IsZero());
		CHECK_EQ(row->constants.beginAttackPoint, static_cast<uint16_t>(0));
		CHECK_EQ(row->constants.beginDefensePoint, static_cast<uint16_t>(0));
		CHECK_EQ(row->constants.beginMeleePower, static_cast<uint16_t>(0));
		CHECK_EQ(row->constants.beginShootPower, static_cast<uint16_t>(0));
		CHECK_EQ(row->constants.hpPerStr, 0.0f);
		CHECK_EQ(row->constants.hitPerDex, 0.0f);
		CHECK_EQ(row->constants.meleePerPow, 0.0f);
		CHECK_EQ(row->constants.levelUpAttackPoint, 0.0f);
		CHECK_EQ(row->constants.attackPointConversion, 0.0f);
	}

	MODERN_TEST(ClassConstantTable_CoefficientSourceNamesItself)
	{
		// `ToString` exists so a log line can name a provenance rather than
		// printing a number nobody can read back.
		CHECK_EQ(std::string(ToString(CoefficientSource::Unavailable)),
		         std::string("Unavailable"));
		CHECK_EQ(std::string(ToString(CoefficientSource::Recovered)),
		         std::string("Recovered"));

		// And an out-of-range value does not fall off the switch.
		const auto bogus = static_cast<CoefficientSource>(200);
		CHECK(ToString(bogus) != nullptr);
	}

	// ---- row validation ----------------------------------------------------

	MODERN_TEST(ClassConstantTable_AFullySpecifiedTestRowValidates)
	{
		// Proves the "available" half of the table is reachable, so the
		// `RecoveredCount() == 0` above is a statement about the DATA and not
		// about a validator that can never succeed.
		ClassConstantRow row = MakeTestRow(CharClassIndex::SwordsmanMale);
		REQUIRE(ValidateRow(row));
		CHECK_EQ(static_cast<int>(row.source),
		         static_cast<int>(CoefficientSource::Recovered));
	}

	MODERN_TEST(ClassConstantTable_ARowOutsideTheSixteenIsRefused)
	{
		for (uint8_t raw : { static_cast<uint8_t>(16), static_cast<uint8_t>(200),
		                     static_cast<uint8_t>(255) })
		{
			ClassConstantRow row = MakeTestRow(static_cast<CharClassIndex>(raw));
			CHECK(!ValidateRow(row));
			CHECK_EQ(static_cast<int>(row.source),
			         static_cast<int>(CoefficientSource::Unavailable));
			REQUIRE(row.note != nullptr);
			CHECK(std::string(row.note).find("refused") != std::string::npos);
		}
	}

	MODERN_TEST(ClassConstantTable_ANonFiniteCoefficientDemotesTheRow)
	{
		// A NaN anywhere in the row makes every derived stat NaN. The row is
		// DEMOTED, not repaired: a table that quietly fixes itself cannot be
		// audited.
		const float nan = std::numeric_limits<float>::quiet_NaN();

		{
			ClassConstantRow row = MakeTestRow(CharClassIndex::BrawlerMale);
			row.constants.hpPerStr = nan;
			CHECK(!ValidateRow(row));
			CHECK_EQ(static_cast<int>(row.source),
			         static_cast<int>(CoefficientSource::Unavailable));
			REQUIRE(row.note != nullptr);
			CHECK(std::string(row.note).find("non-finite") != std::string::npos);
		}

		{
			// An infinity is equally unusable, and a different code path in a
			// naive check.
			ClassConstantRow row = MakeTestRow(CharClassIndex::ArcherMale);
			row.constants.levelUpStats.dex = std::numeric_limits<float>::infinity();
			CHECK(!ValidateRow(row));
		}
	}

	MODERN_TEST(ClassConstantTable_ARecoveredRowMustNameItsSource)
	{
		// Provenance that cannot be stated cannot be audited, so a value-bearing
		// row with no note is not a recovered row.
		ClassConstantRow row = MakeTestRow(CharClassIndex::ShamanFemale);
		row.note = "";
		CHECK(!ValidateRow(row));
		CHECK_EQ(static_cast<int>(row.source),
		         static_cast<int>(CoefficientSource::Unavailable));
		REQUIRE(row.note != nullptr);
		CHECK(std::string(row.note).find("refused") != std::string::npos);

		// A null note is the same refusal, exercised separately because the
		// check has to dereference it.
		ClassConstantRow nulled = MakeTestRow(CharClassIndex::ShamanFemale);
		nulled.note = nullptr;
		CHECK(!ValidateRow(nulled));
		CHECK_EQ(static_cast<int>(nulled.source),
		         static_cast<int>(CoefficientSource::Unavailable));
	}

	MODERN_TEST(ClassConstantTable_AnUnavailableRowIsLeftAlone)
	{
		// Validation of an already-unavailable row returns false and does not
		// edit the row into looking recovered, nor erase a caller's note.
		ClassConstantRow row;
		row.index  = CharClassIndex::ExtremeMale;
		row.source = CoefficientSource::Unavailable;
		row.note   = "caller's own explanation";

		CHECK(!ValidateRow(row));
		CHECK_EQ(static_cast<int>(row.source),
		         static_cast<int>(CoefficientSource::Unavailable));
		REQUIRE(row.note != nullptr);
		CHECK_EQ(std::string(row.note), std::string("caller's own explanation"));
	}

	MODERN_TEST(ClassConstantTable_AnUnavailableRowWithNoNoteIsGivenTheReason)
	{
		// The table must never present a blank explanation.
		ClassConstantRow row;
		row.index  = CharClassIndex::GunnerFemale;
		row.source = CoefficientSource::Unavailable;
		row.note   = nullptr;

		CHECK(!ValidateRow(row));
		REQUIRE(row.note != nullptr);
		CHECK(row.note[0] != '\0');
	}

	// ---- Stats::Calculate against hand-computed arithmetic ----------------

	MODERN_TEST(ClassConstantTable_CalculateMatchesHandComputedArithmeticAtLevelOne)
	{
		// The fixture row, so every number below is derived on paper from the
		// values in MakeTestRow:
		//
		//   m_sSUMSTATS = sBEGIN_STATS + sLVLUP_STATS*(level-1) + allocated(0)
		//               = (10, 20, 15, 25, 8, 12)          at level 1
		//   m_wSUM_AP   = (wBEGIN_AP + fLVLUP_AP*(level-1)) * fCONV_AP
		//               = (10 + 0*1) * 1                   = 10
		//   m_wSUM_DP   = (5  + 0*1) * 1                   = 5
		//   m_wPA       = (3  + 0*1) * 1 + (pow*fPA_POW + dex*fPA_DEX)
		//               = 3 + (10*1.0 + 25*0.5)            = 3 + 22.5 -> 25
		//   m_nHIT      = int(dex*fHIT_DEX + 0)            = int(25*2.0) = 50
		//   m_nAVOID    = int(dex*fAVOID_DEX)              = int(25*1.0) = 25
		//   m_nDEFENSE_BODY = m_wSUM_DP + dex*fDEFENSE_DEX  = 5 + 25*3.0 = 80
		ClassConstantRow row = MakeTestRow(CharClassIndex::SwordsmanMale);
		REQUIRE(ValidateRow(row));

		StatCalculationInput input = TestInput(CharClassIndex::SwordsmanMale, 1);
		input.classConstants = row.constants;

		const Result<DerivedStats> stats = Calculate(input);
		REQUIRE(stats.IsOk());

		const DerivedStats& got = stats.GetValue();
		CHECK_EQ(got.totalStats.pow, static_cast<uint16_t>(10));
		CHECK_EQ(got.totalStats.dex, static_cast<uint16_t>(25));

		CHECK_EQ(got.attackPoint, static_cast<uint16_t>(10));
		CHECK_EQ(got.defensePoint, static_cast<uint16_t>(5));

		// m_wPA truncates its float product to WORD: (int)(3 + 22.5) == 25.
		CHECK_EQ(got.meleePower, static_cast<uint16_t>(25));
		// m_wSA is the same shape from its own begin value:
		//   (4 + 0) + (int)(10*1.0 + 25*0.5) = 4 + 22 = 26
		CHECK_EQ(got.shootPower, static_cast<uint16_t>(26));

		CHECK_EQ(got.hit, 50);
		CHECK_EQ(got.avoid, 25);
		CHECK_EQ(got.defenseBody, 80);
		CHECK_EQ(got.defense, 80); // + no item/passive defence

		// m_gdDAMAGE_PHYSIC = m_gdDAMAGE(dwLow == dwMax == m_wSUM_AP) + weapon
		// damage (none) then VAR_PARAM(m_wPA): both endpoints gain PA.
		CHECK_EQ(got.physicalDamage.low, static_cast<uint32_t>(35));
		CHECK_EQ(got.physicalDamage.high, static_cast<uint32_t>(35));

		// Resource maxima from the six stats.
		CHECK_EQ(got.maxHp, static_cast<uint32_t>(20 * 5));
		CHECK_EQ(got.maxMp, static_cast<uint32_t>(15 * 4));
		CHECK_EQ(got.maxSp, static_cast<uint32_t>(12 * 2));
	}

	MODERN_TEST(ClassConstantTable_CalculateMovesWithLevelExactlyAsLegacyDoes)
	{
		// Level 3, same fixture. The level term is (level - 1), NOT level, and
		// the float growth is truncated PER FIELD:
		//
		//   m_sSUMSTATS.dex = 25 + (int)(0.5 * 2) = 26
		//   m_wSUM_AP       = (10 + 0*2) * 1      = 10   (fLVLUP_AP is 0)
		//   m_wPA           = 3 + (10*1.0 + 26*0.5) = 3 + 23.0 -> 26
		//   m_nHIT          = int(26*2.0)        = 52
		//   m_nAVOID        = int(26*1.0)        = 26
		//   m_nDEFENSE_BODY = 5 + 26*3.0         = 83
		ClassConstantRow row = MakeTestRow(CharClassIndex::SwordsmanMale);
		REQUIRE(ValidateRow(row));

		StatCalculationInput input = TestInput(CharClassIndex::SwordsmanMale, 3);
		input.classConstants = row.constants;

		const Result<DerivedStats> stats = Calculate(input);
		REQUIRE(stats.IsOk());

		const DerivedStats& got = stats.GetValue();
		CHECK_EQ(got.totalStats.dex, static_cast<uint16_t>(26));
		CHECK_EQ(got.meleePower, static_cast<uint16_t>(26));
		CHECK_EQ(got.hit, 52);
		CHECK_EQ(got.avoid, 26);
		CHECK_EQ(got.defenseBody, 83);
		CHECK_EQ(got.physicalDamage.low, static_cast<uint32_t>(36));
	}

	MODERN_TEST(ClassConstantTable_TheSameRowAndLevelAlwaysCalculateTheSame)
	{
		// Purity: the table is data and the calculator is stateless, so the
		// same inputs give the same derived stats. A provider that resolved
		// twice and got two answers would make every damage assertion flaky.
		ClassConstantRow row = MakeTestRow(CharClassIndex::BrawlerFemale);
		REQUIRE(ValidateRow(row));

		StatCalculationInput input = TestInput(CharClassIndex::BrawlerFemale, 7);
		input.classConstants = row.constants;

		const Result<DerivedStats> first  = Calculate(input);
		const Result<DerivedStats> second = Calculate(input);
		REQUIRE(first.IsOk());
		REQUIRE(second.IsOk());
		CHECK(first.GetValue() == second.GetValue());
	}
} // namespace ModernTests
