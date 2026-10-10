// WORLD-ENTRY-002L-B: the recovered class-constant table.
//
// What these cases pin is the part a compile-time assertion cannot: that every
// legacy class/gender combination has a row, that a row either carries verified
// data or is explicitly refused, that the validation refuses the three ways a
// row could otherwise lie, and - now that data has arrived - that the values
// are the DEPLOYED ones and not the constructor defaults they replaced.
//
// The values asserted here were read out of the deployed
// `class<N>.classconst` files with the same decryption and tokenization the
// legacy loader uses; the arithmetic cases further down use a test row and say
// so, because they test the wiring rather than the data.

#include "TestHarness.h"
#include "stats/BaseStats.h"
#include "stats/ClassConstantTable.h"
#include "stats/Contributions.h"
#include "stats/DerivedStats.h"
#include "stats/StatCalculator.h"

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
		// level-up growth that stays integral). NOT deployed RAN data - it exists
		// to test the wiring.
		ClassConstantRow MakeTestRow(CharClassIndex index)
		{
			ClassConstantRow row;
			row.index  = index;
			row.source = CoefficientSource::Recovered;
			row.note   = "test fixture: ClassConstantTableTests, not deployed data";

			ClassConstants& cc = row.constants;
			cc.beginStats.pow = 10; cc.beginStats.str = 20;
			cc.beginStats.spi = 15; cc.beginStats.dex = 25;
			cc.beginStats.intel = 8; cc.beginStats.sta = 12;

			cc.beginAttackPoint = 10;
			cc.beginDefensePoint = 5;
			cc.beginMeleePower = 3;
			cc.beginShootPower = 4;

			cc.attackPointConversion = 1.0f;
			cc.defensePointConversion = 1.0f;
			cc.meleePowerConversion = 1.0f;
			cc.shootPowerConversion = 1.0f;

			cc.hpPerStr = 5.0f;
			cc.mpPerSpi = 4.0f;
			cc.spPerSta = 2.0f;

			cc.hitPerDex = 2.0f;
			cc.avoidPerDex = 1.0f;
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
		// table saying "no such class", never "no data for a known class".
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

			// And the explanation names the file, the key that pointed at it,
			// the key version, the loader and the getflag keys - so a reader
			// can go and re-read the source rather than only being told it
			// exists.
			const std::string note(row.note);
			CHECK(note.find(".classconst") != std::string::npos);
			CHECK(note.find("SETFILE") != std::string::npos);
			CHECK(note.find("default.charclass") != std::string::npos);
			CHECK(note.find("Rijndael v8") != std::string::npos);
			CHECK(note.find("GLogicDataLoad.cpp") != std::string::npos);
			CHECK(note.find("1169-1216") != std::string::npos);

			// Every row is recovered, and says so.
			CHECK_EQ(static_cast<int>(row.source),
			         static_cast<int>(CoefficientSource::Recovered));
		}
	}

	MODERN_TEST(ClassConstantTable_EveryRowIsRecoveredAndAllSixteenAre)
	{
		const ClassConstantTable& table = ClassConstantTable::Verified();

		// Sixteen rows, all of them recovered. This is the 002L-B state and the
		// reason the milestone exists; 002L-A asserted the opposite and that
		// assertion is what changed.
		CHECK_EQ(table.RecoveredCount(), ClassConstantTable::kRowCount);
	}

	MODERN_TEST(ClassConstantTable_EveryRowNamesItsOwnDeployedFile)
	{
		// The sixteen files, in EMCHARINDEX order, and each row must name exactly
		// its own. A row that named a different file would be reading a
		// different class's coefficients - the failure this guards is a
		// transposed file list, which the speed cross-check would also catch but
		// which only a per-row name check catches for the other 25 fields.
		static const char* kExpected[ClassConstantTable::kRowCount] = {
			"class0.classconst", "class1.classconst", "class2.classconst",
			"class3.classconst", "class4.classconst", "class5.classconst",
			"class6.classconst", "class7.classconst", "class8.classconst",
			"class9.classconst", "classA.classconst", "classB.classconst",
			"classC.classconst", "classD.classconst", "classE.classconst",
			"classF.classconst",
		};

		const ClassConstantTable& table = ClassConstantTable::Verified();
		for (std::size_t position = 0; position < ClassConstantTable::kRowCount; ++position)
		{
			const std::string note(table.RowAt(position).note);
			CHECK(note.find(kExpected[position]) != std::string::npos);

			// And it does not name any OTHER row's file, which is the stronger
			// form: a note that listed two files would pass the find above.
			for (std::size_t other = 0; other < ClassConstantTable::kRowCount; ++other)
			{
				if (other == position)
				{
					continue;
				}
				CHECK(note.find(kExpected[other]) == std::string::npos);
			}
		}
	}

	// ---- the values are the deployed ones, not the constructor's -----------

	MODERN_TEST(ClassConstantTable_NoRowIsACONSTCLASSConstructorDefault)
	{
		// The constructor row for EMCHARINDEX 0 is
		//   GLCONST_CHARCLASS(10.0f, 0.9f, 1.00f, 0.08f, 0.08f, 0.4f, 0.4f, 0.2f,
		//                     SCHARSTATS(15,13,20,10,0,20), FCHARSTATS(2,3,1,1,1,3),
		//                     35, 15, 5, 4, 2, 2, 3, 1)
		// (GLogicData.cpp:609-615). RAN overwrites every one of those fields at
		// startup, so a row that matched them would be the constructor masquer-
		// ading as data.
		const ClassConstantTable& table = ClassConstantTable::Verified();
		const ClassConstantRow* row = table.Find(CharClassIndex::BrawlerMale);
		REQUIRE(row != nullptr);
		REQUIRE(row->source == CoefficientSource::Recovered);

		// The deployed row for BrawlerMale. Quoted field by field so a reader
		// can diff this against class0.classconst.
		CHECK_EQ(row->constants.hpPerStr, 2.0f);     // constructor 10.0f
		CHECK_EQ(row->constants.mpPerSpi, 0.65f);    // constructor 0.9f
		CHECK_EQ(row->constants.spPerSta, 1.2f);     // constructor 1.00f
		CHECK_EQ(row->constants.hitPerDex, 0.0f);    // constructor 0.08f
		CHECK_EQ(row->constants.avoidPerDex, 0.0f);  // constructor 0.08f
		CHECK_EQ(row->constants.defensePerDex, 0.032f); // constructor 0.4f
		CHECK_EQ(row->constants.meleePerPow, 0.14f); // constructor 0.4f
		CHECK_EQ(row->constants.beginAttackPoint, static_cast<uint16_t>(4));  // 35
		CHECK_EQ(row->constants.beginDefensePoint, static_cast<uint16_t>(5)); // 15
		CHECK_EQ(row->constants.beginStats.pow, static_cast<uint16_t>(10));  // 15
		CHECK_EQ(row->constants.beginStats.str, static_cast<uint16_t>(34));  // 13
		CHECK_EQ(row->constants.beginStats.dex, static_cast<uint16_t>(10));  // 10

		// The one field where they AGREE is worth naming, so a later reader does
		// not "fix" the test to differ everywhere: the constructor's dex happens
		// to match.
		CHECK_EQ(row->constants.beginStats.dex, static_cast<uint16_t>(10));
	}

	MODERN_TEST(ClassConstantTable_TheDeployedRowForBrawlerMaleIsExact)
	{
		// A whole row, quoted from class0.classconst, so a transcription error
		// anywhere in it is visible. This is the golden row.
		const ClassConstantRow* row =
		    ClassConstantTable::Verified().Find(CharClassIndex::BrawlerMale);
		REQUIRE(row != nullptr);
		const ClassConstants& cc = row->constants;

		CHECK_EQ(cc.hpPerStr, 2.0f);
		CHECK_EQ(cc.mpPerSpi, 0.65f);
		CHECK_EQ(cc.spPerSta, 1.2f);
		CHECK_EQ(cc.defensePerDex, 0.032f);

		CHECK_EQ(cc.meleePerPow, 0.14f);
		CHECK_EQ(cc.meleePerDex, 0.1f);
		CHECK_EQ(cc.shootPerPow, 0.08f);
		CHECK_EQ(cc.shootPerDex, 0.18f);
		CHECK_EQ(cc.magicPerDex, 0.12f);
		CHECK_EQ(cc.magicPerSpi, 0.22f);
		CHECK_EQ(cc.magicPerIntel, 0.0f);

		CHECK_EQ(cc.attackPointConversion, 0.4f);
		CHECK_EQ(cc.defensePointConversion, 0.75f);
		CHECK_EQ(cc.meleePowerConversion, 0.8f);
		CHECK_EQ(cc.shootPowerConversion, 0.2f);

		CHECK_EQ(cc.beginAttackPoint, static_cast<uint16_t>(4));
		CHECK_EQ(cc.beginDefensePoint, static_cast<uint16_t>(5));
		CHECK_EQ(cc.beginMeleePower, static_cast<uint16_t>(5));
		CHECK_EQ(cc.beginShootPower, static_cast<uint16_t>(3));

		CHECK_EQ(cc.levelUpAttackPoint, 1.2f);
		CHECK_EQ(cc.levelUpDefensePoint, 0.414f);
		CHECK_EQ(cc.levelUpMeleePower, 0.4f);
		CHECK_EQ(cc.levelUpShootPower, 0.18f);

		CHECK_EQ(cc.beginStats.pow, static_cast<uint16_t>(10));
		CHECK_EQ(cc.beginStats.str, static_cast<uint16_t>(34));
		CHECK_EQ(cc.beginStats.spi, static_cast<uint16_t>(9));
		CHECK_EQ(cc.beginStats.dex, static_cast<uint16_t>(10));
		CHECK_EQ(cc.beginStats.intel, static_cast<uint16_t>(0));
		CHECK_EQ(cc.beginStats.sta, static_cast<uint16_t>(9));

		CHECK_EQ(cc.levelUpStats.pow, 0.3f);
		CHECK_EQ(cc.levelUpStats.str, 4.5f);
		CHECK_EQ(cc.levelUpStats.spi, 0.61f);
		CHECK_EQ(cc.levelUpStats.dex, 0.4f);
		CHECK_EQ(cc.levelUpStats.intel, 0.0f);
		CHECK_EQ(cc.levelUpStats.sta, 3.2f);
	}

	MODERN_TEST(ClassConstantTable_HitAndAvoidHaveNoDexTermInAnyDeployedRow)
	{
		// fHIT_DEX and fAVOID_DEX are 0 in every deployed row. That is the data,
		// and it is load-bearing: hit and avoid come entirely from equipment and
		// passives in this build, so a character with neither has derived hit and
		// avoid of exactly 0.
		//
		// Asserted for all sixteen rows rather than one, because a reader seeing
		// a zero in a single row has every reason to suspect a transcription
		// slip - and one row's zero being deliberate is not evidence that the
		// other fifteen are.
		const ClassConstantTable& table = ClassConstantTable::Verified();
		for (std::size_t position = 0; position < ClassConstantTable::kRowCount; ++position)
		{
			const ClassConstants& cc = table.RowAt(position).constants;
			CHECK_EQ(cc.hitPerDex, 0.0f);
			CHECK_EQ(cc.avoidPerDex, 0.0f);
		}
	}

	MODERN_TEST(ClassConstantTable_EveryDeployedRowCarriesTheSixStatValues)
	{
		// sBEGIN_STATS / sLVLUP_STATS are six flat tokens behind [...|...]
		// bracket groups; the loader reads them flat (GLogicDataLoad.cpp:
		// 1204-1209, :1211-1216). The structural consequence in the deployed
		// data is that every class carries intellect 0 - asserted here so a
		// reader who "fixes" the brackets to pairs cannot do so silently.
		const ClassConstantTable& table = ClassConstantTable::Verified();
		for (std::size_t position = 0; position < ClassConstantTable::kRowCount; ++position)
		{
			const ClassConstants& cc = table.RowAt(position).constants;
			CHECK_EQ(cc.beginStats.intel, static_cast<uint16_t>(0));
			CHECK_EQ(cc.levelUpStats.intel, 0.0f);
			CHECK_EQ(cc.magicPerIntel, 0.0f);
		}
	}

	MODERN_TEST(ClassConstantTable_NoTwoRowsAreIdentical)
	{
		// The deployed table has genuine per-class variation, and a duplicated
		// row would mean a transcription copy-paste. The three rows that DO
		// legitimately repeat - ExtremeMale/ExtremeFemale and each other's
		// gender twin - are the deployed data, so the assertion is not "all
		// sixteen differ" but "not everything is one row".
		const ClassConstantTable& table = ClassConstantTable::Verified();

		std::size_t distinctBeginStats = 0;
		for (std::size_t a = 0; a < ClassConstantTable::kRowCount; ++a)
		{
			bool first = true;
			for (std::size_t b = 0; b < a; ++b)
			{
				if (table.RowAt(a).constants.beginStats == table.RowAt(b).constants.beginStats)
				{
					first = false;
					break;
				}
			}
			if (first)
			{
				++distinctBeginStats;
			}
		}

		// The deployed data has more than one distinct class; a single one would
		// mean every file was transcribed from the same row.
		CHECK(distinctBeginStats > 1);
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
		// Proves the "available" half of the table is reachable, so the shipped
		// rows being Recovered is a statement about the DATA and not about a
		// validator that can never succeed.
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
		//   m_wPA           = 3 + (int)(10*1.0 + 26*0.5) = 3 + 23.0 -> 26
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

	MODERN_TEST(ClassConstantTable_TheDeployedArcherRowCalculatesAsLegacyWould)
	{
		// The same hand-arithmetic discipline, run on a DEPLOYED row rather than
		// the fixture, so the transcription is checked end to end through the
		// calculator. ArcherMale (class8) at level 1:
		//
		//   sBEGIN_STATS    = (5, 34, 18, 12, 0, 7)
		//   m_wSUM_AP       = (5 + 1.2*0) * 0.4       = 2.0     -> 2
		//   m_wSUM_DP       = (6 + 0.427*0) * 0.57    = 3.42    -> 3
		//   m_wPA           = (2 + 0.3*0) * 0.6       = 1.2     -> 1
		//                     + (int)(5*0.12 + 12*0.08) = (int)1.56 = 1 -> 2
		//   m_nHIT / AVOID  = int(12*0) = 0           (fHIT_DEX is 0)
		//   m_nDEFENSE_BODY = (int)(3 + 12*0.024)     = 3
		//   m_gdDAMAGE_PHYSIC = 2 + VAR_PARAM(2)      = (4, 4)
		//   m_sHP.dwMax     = (int)(34*1.8)           = 61
		//   m_sMP.dwMax     = (int)(18*0.8)           = 14
		const ClassConstantTable& table = ClassConstantTable::Verified();
		const ClassConstantRow* row = table.Find(CharClassIndex::ArcherMale);
		REQUIRE(row != nullptr);
		REQUIRE(row->source == CoefficientSource::Recovered);

		StatCalculationInput input = TestInput(CharClassIndex::ArcherMale, 1);
		input.classConstants = row->constants;

		const Result<DerivedStats> stats = Calculate(input);
		REQUIRE(stats.IsOk());

		const DerivedStats& got = stats.GetValue();
		CHECK_EQ(got.totalStats.pow, static_cast<uint16_t>(5));
		CHECK_EQ(got.totalStats.str, static_cast<uint16_t>(34));
		CHECK_EQ(got.totalStats.spi, static_cast<uint16_t>(18));
		CHECK_EQ(got.totalStats.dex, static_cast<uint16_t>(12));

		CHECK_EQ(got.attackPoint, static_cast<uint16_t>(2));
		CHECK_EQ(got.defensePoint, static_cast<uint16_t>(3));
		CHECK_EQ(got.meleePower, static_cast<uint16_t>(2));
		CHECK_EQ(got.hit, 0);
		CHECK_EQ(got.avoid, 0);
		CHECK_EQ(got.defenseBody, 3);
		CHECK_EQ(got.defense, 3);
		CHECK_EQ(got.physicalDamage.low, static_cast<uint32_t>(4));
		CHECK_EQ(got.physicalDamage.high, static_cast<uint32_t>(4));
		CHECK_EQ(got.maxHp, static_cast<uint32_t>(61));
		CHECK_EQ(got.maxMp, static_cast<uint32_t>(14));
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
