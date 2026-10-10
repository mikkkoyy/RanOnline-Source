// SKILL-001: recovering the canonical skill definitions from the mixed-schema
// ASURA Skill.csv export.
//
// The bulk of this file builds SYNTHETIC fixtures, because the point is the
// parser's behaviour on bad input, and the real 24 MB export is a poor way to
// test a short row.
//
// The fixtures deliberately do NOT reproduce the export's column order. The
// canonical header below is built REORDERED and padded to 322 names, so a
// parser that worked from fixed indexes would read the wrong cells and fail.
// That is the strongest available check that resolution is by verified header
// name.
//
// One integration test at the end reads the real export and reconciles counts.

#include "TestHarness.h"
#include "skills/SkillBasicTable.h"
#include "skills/SkillDefinitionProvider.h"

#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace ModernTests
{
	using namespace Modern;
	using namespace Modern::Skill;

	namespace
	{
		// The column names this loader resolves, in the form the export writes
		// them. `ResolveColumns` refuses a header missing any of them, so a
		// fixture header must carry all of them.
		const char* const kScalarNames[] = {
			"sNATIVEID wMainID", "sNATIVEID wSubID", "szNAME", "dwGRADE",
			"dwMAXLEVEL", "emROLE", "emAPPLY", "emIMPACT_TAR", "emIMPACT_SIDE",
			"wTARRANGE", "emUSE_LITEM", "emUSE_RITEM", "emBRIGHT", "dwCLASS",
			"sSKILL wMainID", "sSKILL wSubID",
		};

		const char* const kStepFieldNames[] = {
			"dwSKP", "dwLEVEL", "sSTATS wPow", "sSTATS wStr", "sSTATS wSpi",
			"sSTATS wDex", "sSTATS wInt", "sSTATS wSta", "dwSKILL_LVL",
		};

		// `sNATIVEID wMainID`/`wSubID` must lead, because that pair plus the
		// column count is the header's schema signature.
		std::vector<std::string> CanonicalHeaderNames()
		{
			std::vector<std::string> names = {
				"sNATIVEID wMainID", "sNATIVEID wSubID",
			};
			// The first two scalar names were already pushed as the schema signature.
			const std::size_t scalarCount =
			    sizeof(kScalarNames) / sizeof(kScalarNames[0]);
			for (std::size_t i = 2; i < scalarCount; ++i)
			{
				names.emplace_back(kScalarNames[i]);
			}
			for (int step = 1; step <= kMaxSkillLevel; ++step)
			{
				for (const char* field : kStepFieldNames)
				{
					names.push_back("sLVL_STEP " + std::to_string(step) + " " + field);
				}
			}
			// Pad to the width the export declares, with names nothing reads.
			//
			// 321 NAMES, not 322: the export writes a trailing comma, so its
			// header has 322 FIELDS of which the last one is empty.
			// JoinWithCommas reproduces that comma, so 321 names is what
			// yields 322 fields - and the width checks compare FIELDS.
			for (std::size_t i = names.size(); i < 321; ++i)
			{
				names.push_back("filler" + std::to_string(i));
			}
			return names;
		}

		std::string JoinWithCommas(const std::vector<std::string>& cells)
		{
			std::string out;
			for (std::size_t i = 0; i < cells.size(); ++i)
			{
				if (i != 0)
				{
					out.push_back(',');
				}
				out += cells[i];
			}
			// The export writes a trailing comma, so its rows carry one more
			// field than they have names. Reproduced, because the width check
			// counts it.
			out.push_back(',');
			return out;
		}

		// A canonical data row. Any name not overridden gets "0", so a test
		// only states the fields it actually cares about.
		std::string CanonicalRow(
		    std::initializer_list<std::pair<std::string, std::string>> overrides)
		{
			const std::vector<std::string> names = CanonicalHeaderNames();
			std::vector<std::string> cells;
			cells.reserve(names.size());

			for (const std::string& name : names)
			{
				std::string value = "0";
				for (const auto& pair : overrides)
				{
					if (pair.first == name)
					{
						value = pair.second;
						break;
					}
				}
				cells.push_back(value);
			}
			return JoinWithCommas(cells);
		}

		// A plausible definition: a named, learnable skill.
		std::string GoodRow(uint32_t mainId, uint32_t subId, const char* name)
		{
			return CanonicalRow({
				{ "sNATIVEID wMainID", std::to_string(mainId) },
				{ "sNATIVEID wSubID", std::to_string(subId) },
				{ "szNAME", name },
				{ "dwMAXLEVEL", "9" },
				{ "dwGRADE", "3" },
			});
		}

		// The SAPPLY header: 719 columns led by emBASIC_TYPE/emELEMENT.
		std::string LegacyHeader()
		{
			std::vector<std::string> cells;
			cells.emplace_back("emBASIC_TYPE");
			cells.emplace_back("emELEMENT");
			// 718 names + the trailing comma = 719 FIELDS, matching the
			// export's SAPPLY line. The width checks compare fields.
			for (std::size_t i = 2; i < 718; ++i)
			{
				cells.push_back("legacy" + std::to_string(i));
			}
			return JoinWithCommas(cells);
		}

		std::string LegacyRow()
		{
			std::vector<std::string> cells;
			for (std::size_t i = 0; i < 718; ++i)
			{
				cells.emplace_back("0");
			}
			return JoinWithCommas(cells);
		}

		std::filesystem::path WriteFixture(const char* name,
		                                    const std::string& contents)
		{
			const std::filesystem::path path =
			    std::filesystem::temp_directory_path() / name;
			std::ofstream out(path, std::ios::binary);
			out << contents;
			return path;
		}

		struct Fixture
		{
			std::filesystem::path path;
			~Fixture()
			{
				std::error_code ignored;
				std::filesystem::remove(path, ignored);
			}
		};

		// A whole well-formed file: one block of header + two data lines.
		std::string TwoSkillFile()
		{
			std::string out = JoinWithCommas(CanonicalHeaderNames()) + "\n";
			out += LegacyHeader() + "\n";
			out += GoodRow(1, 2, "SK_1_2") + "\n";
			out += LegacyRow() + "\n";
			out += JoinWithCommas(CanonicalHeaderNames()) + "\n";
			out += LegacyHeader() + "\n";
			out += GoodRow(3, 4, "SK_3_4") + "\n";
			out += LegacyRow() + "\n";
			return out;
		}
	}

	// ===========================================================================
	// ACCEPTING THE CANONICAL HALF
	// ===========================================================================

	MODERN_TEST(SkillTable_CanonicalRowsLoadAndKeepTheirIdentity)
	{
		const Fixture fixture = { WriteFixture("skill-two.csv", TwoSkillFile()) };

		InMemorySkillDefinitions provider;
		const auto loaded = LoadSkillCsv(fixture.path.string(), provider);
		REQUIRE(loaded.IsOk());

		const SkillTableLoadResult& result = loaded.GetValue();
		CHECK_EQ(result.accepted, static_cast<std::size_t>(2));
		CHECK_EQ(result.loaded, static_cast<std::size_t>(2));
		CHECK_EQ(result.Rejected(), static_cast<std::size_t>(0));
		CHECK_EQ(provider.GetCount(), static_cast<std::size_t>(2));

		const SkillDefinition* first = provider.Find(SkillId{ 1, 2 });
		REQUIRE(first != nullptr);
		CHECK(first->name == "SK_1_2");
		CHECK_EQ(first->maxLevel, static_cast<std::uint8_t>(9));
		CHECK_EQ(first->grade, static_cast<std::uint32_t>(3));

		// The id pair is carried whole - no packing, no truncation.
		CHECK(provider.Find(SkillId{ 2, 1 }) == nullptr);
	}

	MODERN_TEST(SkillTable_TheLegacyEffectRowsAreExcludedAndCounted)
	{
		// The 719-column rows are SAPPLY / CDATA_LVL - per-level effect data
		// belonging to a struct this milestone does not recover. They must be
		// counted as excluded, never reinterpreted as skill definitions.
		const Fixture fixture = { WriteFixture("skill-exclude.csv", TwoSkillFile()) };

		InMemorySkillDefinitions provider;
		const auto loaded = LoadSkillCsv(fixture.path.string(), provider);
		REQUIRE(loaded.IsOk());

		const SkillTableLoadResult& result = loaded.GetValue();
		CHECK_EQ(result.legacyHeaders, static_cast<std::size_t>(2));
		CHECK_EQ(result.excludedLegacyRows, static_cast<std::size_t>(2));
		CHECK_EQ(result.accepted, static_cast<std::size_t>(2));

		// A legacy row's leading numbers must not have become a skill id. The
		// fixture's legacy rows are all zeros, so (0,0) would appear if any
		// legacy row were parsed.
		CHECK(provider.Find(SkillId{ 0, 0 }) == nullptr);
	}

	MODERN_TEST(SkillTable_TheLearnTableIsReadWithItsOwnColumnNames)
	{
		// `SLEARN_LVL` declares {dwLEVEL, sSTATS, dwSKP, dwSKILL_LVL}
		// (GLSkillLearn.h:28-40) but the export writes dwSKP FIRST. Resolving
		// by name is what keeps that difference from becoming a silent
		// off-by-one, so the fixture states every field a different value and
		// each must land in the right place.
		const std::string row = CanonicalRow({
			{ "sNATIVEID wMainID", "1" },
			{ "sNATIVEID wSubID", "2" },
			{ "szNAME", "LEARN" },
			{ "dwMAXLEVEL", "9" },
			{ "sLVL_STEP 1 dwSKP", "11" },
			{ "sLVL_STEP 1 dwLEVEL", "22" },
			{ "sLVL_STEP 1 sSTATS wPow", "33" },
			{ "sLVL_STEP 1 sSTATS wDex", "44" },
			{ "sLVL_STEP 1 dwSKILL_LVL", "55" },
			{ "sLVL_STEP 2 dwSKP", "66" },
			{ "sLVL_STEP 2 dwLEVEL", "77" },
		});

		const Fixture fixture = { WriteFixture("skill-learn.csv",
			JoinWithCommas(CanonicalHeaderNames()) + "\n" +
			LegacyHeader() + "\n" + row + "\n" + LegacyRow() + "\n") };

		InMemorySkillDefinitions provider;
		REQUIRE(LoadSkillCsv(fixture.path.string(), provider).IsOk());

		const SkillDefinition* skill = provider.Find(SkillId{ 1, 2 });
		REQUIRE(skill != nullptr);

		CHECK_EQ(skill->learn[1].skillPointCost, static_cast<std::uint32_t>(11));
		CHECK_EQ(skill->learn[1].requiredLevel, static_cast<std::uint32_t>(22));
		CHECK_EQ(skill->learn[1].requiredStats.pow, static_cast<std::uint16_t>(33));
		CHECK_EQ(skill->learn[1].requiredStats.dex, static_cast<std::uint16_t>(44));
		CHECK_EQ(skill->learn[1].resultingSkillLevel, static_cast<std::uint32_t>(55));
		CHECK_EQ(skill->learn[2].skillPointCost, static_cast<std::uint32_t>(66));
		CHECK_EQ(skill->learn[2].requiredLevel, static_cast<std::uint32_t>(77));

		// Untouched steps stay zero rather than borrowing another step's.
		CHECK_EQ(skill->learn[3].skillPointCost, static_cast<std::uint32_t>(0));
		CHECK_EQ(skill->learn[0].skillPointCost, static_cast<std::uint32_t>(0));
	}

	MODERN_TEST(SkillTable_RepeatedHeadersAreHarmlessAndInterleavingIsFollowed)
	{
		// The real export re-emits BOTH headers inside every block, so header
		// DETECTION - not line parity - is what keeps a row paired with the
		// right schema. This fixture interleaves them out of order on purpose.
		std::string contents = JoinWithCommas(CanonicalHeaderNames()) + "\n";
		contents += LegacyHeader() + "\n";
		contents += GoodRow(1, 1, "FIRST") + "\n";
		contents += LegacyRow() + "\n";
		// A second canonical block, headers again, in the same order.
		contents += JoinWithCommas(CanonicalHeaderNames()) + "\n";
		contents += LegacyHeader() + "\n";
		contents += GoodRow(2, 2, "SECOND") + "\n";
		contents += LegacyRow() + "\n";
		// A THIRD, to prove it keeps working rather than special-casing two.
		contents += JoinWithCommas(CanonicalHeaderNames()) + "\n";
		contents += LegacyHeader() + "\n";
		contents += GoodRow(3, 3, "THIRD") + "\n";
		contents += LegacyRow() + "\n";

		const Fixture fixture = { WriteFixture("skill-interleave.csv", contents) };

		InMemorySkillDefinitions provider;
		const auto loaded = LoadSkillCsv(fixture.path.string(), provider);
		REQUIRE(loaded.IsOk());

		const SkillTableLoadResult& result = loaded.GetValue();
		CHECK_EQ(result.canonicalHeaders, static_cast<std::size_t>(3));
		CHECK_EQ(result.legacyHeaders, static_cast<std::size_t>(3));
		CHECK_EQ(result.accepted, static_cast<std::size_t>(3));
		CHECK_EQ(result.excludedLegacyRows, static_cast<std::size_t>(3));

		REQUIRE(provider.Find(SkillId{ 1, 1 }) != nullptr);
		REQUIRE(provider.Find(SkillId{ 2, 2 }) != nullptr);
		REQUIRE(provider.Find(SkillId{ 3, 3 }) != nullptr);
	}

	// ===========================================================================
	// REFUSING BAD INPUT
	// ===========================================================================

	MODERN_TEST(SkillTable_ARowOfTheWrongWidthIsRejected)
	{
		// A short row and an over-long row must both be refused. Validation is
		// not loosened to accept them.
		std::string good = GoodRow(1, 2, "OK");

		std::string shortRow = GoodRow(5, 5, "SHORT");
		shortRow.erase(shortRow.find_last_of(',')); // drop the trailing field

		std::string longRow = GoodRow(6, 6, "LONG") + ",1,2,3";

		std::string contents = JoinWithCommas(CanonicalHeaderNames()) + "\n";
		contents += LegacyHeader() + "\n";
		contents += good + "\n" + shortRow + "\n" + longRow + "\n";

		const Fixture fixture = { WriteFixture("skill-width.csv", contents) };

		InMemorySkillDefinitions provider;
		const auto loaded = LoadSkillCsv(fixture.path.string(), provider);
		REQUIRE(loaded.IsOk());

		const SkillTableLoadResult& result = loaded.GetValue();
		CHECK_EQ(result.accepted, static_cast<std::size_t>(1));
		CHECK_EQ(result.rejectedFieldCount, static_cast<std::size_t>(2));
		CHECK(provider.Find(SkillId{ 1, 2 }) != nullptr);
		CHECK(provider.Find(SkillId{ 5, 5 }) == nullptr);
		CHECK(provider.Find(SkillId{ 6, 6 }) == nullptr);
	}

	MODERN_TEST(SkillTable_ANonNumericCellIsRejectedNotReadAsZero)
	{
		// A corrupt cell in a column this loader reads must refuse the row. A
		// weapon taught at skill-point cost 0 because the cell would not parse
		// is a worse failure than a rejected row.
		const std::string contents =
			JoinWithCommas(CanonicalHeaderNames()) + "\n" +
			LegacyHeader() + "\n" +
			CanonicalRow({
				{ "sNATIVEID wMainID", "1" },
				{ "sNATIVEID wSubID", "2" },
				{ "szNAME", "CORRUPT" },
				{ "sLVL_STEP 1 dwSKP", "not-a-number" },
			}) + "\n";

		const Fixture fixture = { WriteFixture("skill-nan.csv", contents) };

		InMemorySkillDefinitions provider;
		const auto loaded = LoadSkillCsv(fixture.path.string(), provider);
		REQUIRE(loaded.IsOk());
		CHECK_EQ(loaded.GetValue().accepted, static_cast<std::size_t>(0));
		CHECK_EQ(provider.GetCount(), static_cast<std::size_t>(0));
	}

	MODERN_TEST(SkillTable_AMalformedIdentityIsRejected)
	{
		// A missing id, a negative id and the null sentinel are each refused.
		// The sentinel matters: `SkillId::Invalid()` is (0xFFFF,0xFFFF), so a
		// row carrying it would collide with "no skill".
		std::string contents = JoinWithCommas(CanonicalHeaderNames()) + "\n";
		contents += LegacyHeader() + "\n";
		contents += CanonicalRow({ { "sNATIVEID wMainID", "" },
		                           { "sNATIVEID wSubID", "1" },
		                           { "szNAME", "NO_MAIN" } }) + "\n";
		contents += CanonicalRow({ { "sNATIVEID wMainID", "-5" },
		                           { "sNATIVEID wSubID", "1" },
		                           { "szNAME", "NEGATIVE" } }) + "\n";
		contents += CanonicalRow({ { "sNATIVEID wMainID", "65535" },
		                           { "sNATIVEID wSubID", "65535" },
		                           { "szNAME", "SENTINEL" } }) + "\n";

		const Fixture fixture = { WriteFixture("skill-id.csv", contents) };

		InMemorySkillDefinitions provider;
		const auto loaded = LoadSkillCsv(fixture.path.string(), provider);
		REQUIRE(loaded.IsOk());
		CHECK_EQ(loaded.GetValue().accepted, static_cast<std::size_t>(0));
		CHECK_EQ(loaded.GetValue().rejectedIdentity, static_cast<std::size_t>(3));
		CHECK_EQ(provider.GetCount(), static_cast<std::size_t>(0));
	}

	MODERN_TEST(SkillTable_ADuplicateIdIsReportedAndTheFirstOneKeepsTheSlot)
	{
		// The second row for an id must NOT silently overwrite the first -
		// which row wins would otherwise depend on file order.
		std::string contents = JoinWithCommas(CanonicalHeaderNames()) + "\n";
		contents += LegacyHeader() + "\n";
		contents += GoodRow(7, 7, "FIRST_NAME") + "\n";
		contents += CanonicalRow({ { "sNATIVEID wMainID", "7" },
		                           { "sNATIVEID wSubID", "7" },
		                           { "szNAME", "SECOND_NAME" },
		                           { "dwMAXLEVEL", "5" } }) + "\n";

		const Fixture fixture = { WriteFixture("skill-dup.csv", contents) };

		InMemorySkillDefinitions provider;
		const auto loaded = LoadSkillCsv(fixture.path.string(), provider);
		REQUIRE(loaded.IsOk());

		const SkillTableLoadResult& result = loaded.GetValue();
		CHECK_EQ(result.accepted, static_cast<std::size_t>(1));
		CHECK_EQ(result.duplicateIds, static_cast<std::size_t>(1));
		CHECK_EQ(provider.GetCount(), static_cast<std::size_t>(1));

		const SkillDefinition* skill = provider.Find(SkillId{ 7, 7 });
		REQUIRE(skill != nullptr);
		CHECK(skill->name == "FIRST_NAME");
	}

	MODERN_TEST(SkillTable_AnOutOfRangeValueIsRejected)
	{
		// dwMAXLEVEL above RAN's MAX_LEVEL would index past the learn table
		// and the level tables, all of which are 9 entries.
		const std::string contents =
			JoinWithCommas(CanonicalHeaderNames()) + "\n" +
			LegacyHeader() + "\n" +
			CanonicalRow({
				{ "sNATIVEID wMainID", "1" },
				{ "sNATIVEID wSubID", "2" },
				{ "szNAME", "TOO_DEEP" },
				{ "dwMAXLEVEL", "10" },
			}) + "\n";

		const Fixture fixture = { WriteFixture("skill-range.csv", contents) };

		InMemorySkillDefinitions provider;
		const auto loaded = LoadSkillCsv(fixture.path.string(), provider);
		REQUIRE(loaded.IsOk());
		CHECK_EQ(loaded.GetValue().accepted, static_cast<std::size_t>(0));
		CHECK_EQ(loaded.GetValue().rejectedRange, static_cast<std::size_t>(1));
	}

	MODERN_TEST(SkillTable_ARowBeforeAnyHeaderIsRefused)
	{
		// Nothing establishes a data row's schema until a header does, so a
		// leading data row is refused rather than assumed.
		const std::string contents =
			GoodRow(1, 1, "ORPHAN") + "\n" +
			JoinWithCommas(CanonicalHeaderNames()) + "\n" +
			LegacyHeader() + "\n" +
			GoodRow(2, 2, "REAL") + "\n";

		const Fixture fixture = { WriteFixture("skill-orphan.csv", contents) };

		InMemorySkillDefinitions provider;
		const auto loaded = LoadSkillCsv(fixture.path.string(), provider);
		REQUIRE(loaded.IsOk());
		CHECK_EQ(loaded.GetValue().rejectedUnknownSchema,
		         static_cast<std::size_t>(1));
		CHECK_EQ(loaded.GetValue().accepted, static_cast<std::size_t>(1));
		CHECK(provider.Find(SkillId{ 1, 1 }) == nullptr);
		CHECK(provider.Find(SkillId{ 2, 2 }) != nullptr);
	}

	MODERN_TEST(SkillTable_ChangedOrUnknownSchemaIsRefusedNotGuessed)
	{
		// Three ways the export could change under us, each of which must fail
		// loudly rather than be parsed at stale positions.
		//
		// 1. The right column count but the wrong leading names - a different
		//    322-column table.
		std::vector<std::string> impostor = CanonicalHeaderNames();
		impostor[0] = "somethingElse";
		impostor[1] = "somethingElse";
		const std::string wrongNames = JoinWithCommas(impostor);

		// 2. The right leading names but the columns this loader reads are
		//    absent - a re-export that dropped sLVL_STEP.
		std::vector<std::string> dropped = CanonicalHeaderNames();
		for (std::string& name : dropped)
		{
			if (name == "sLVL_STEP 1 dwSKP")
			{
				name = "somethingElse";
			}
		}
		const std::string missingColumn = JoinWithCommas(dropped);

		// 3. A file that is not this export at all.
		const std::string notSkills = "a,b,c\n1,2,3\n";

		const Fixture empty = { WriteFixture("skill-empty.csv", "") };
		const Fixture headersOnly = { WriteFixture("skill-hdronly.csv",
			JoinWithCommas(CanonicalHeaderNames()) + "\n" + LegacyHeader() + "\n") };

		InMemorySkillDefinitions provider;
		// No header at all: not this export.
		CHECK(!LoadSkillCsv(empty.path.string(), provider).IsOk());
		CHECK(!LoadSkillCsv("this-file-does-not-exist.csv", provider).IsOk());

		// Headers and no data is a DIFFERENT thing and is accepted: the file
		// was understood and simply holds no skills. Refusing it would
		// report a corrupt export when it is a valid, empty one.
		{
			InMemorySkillDefinitions empty2;
			const auto onlyHeaders =
			    LoadSkillCsv(headersOnly.path.string(), empty2);
			REQUIRE(onlyHeaders.IsOk());
			CHECK_EQ(onlyHeaders.GetValue().accepted, static_cast<std::size_t>(0));
			CHECK_EQ(empty2.GetCount(), static_cast<std::size_t>(0));
		}

		const Fixture a = { WriteFixture("skill-wrongname.csv",
			wrongNames + "\n" + GoodRow(1, 1, "X") + "\n") };
		const Fixture b = { WriteFixture("skill-missingcol.csv",
			missingColumn + "\n" + GoodRow(1, 1, "X") + "\n") };
		const Fixture c = { WriteFixture("skill-notskills.csv", notSkills) };

		CHECK(!LoadSkillCsv(a.path.string(), provider).IsOk());
		CHECK(!LoadSkillCsv(b.path.string(), provider).IsOk());
		CHECK(!LoadSkillCsv(c.path.string(), provider).IsOk());
		CHECK_EQ(provider.GetCount(), static_cast<std::size_t>(0));
	}

	MODERN_TEST(SkillTable_BlankLinesAndEmptyFieldsAreHandled)
	{
		// Blank lines are counted and skipped, not treated as malformed rows.
		std::string contents = JoinWithCommas(CanonicalHeaderNames()) + "\n\n";
		contents += LegacyHeader() + "\n\n";
		contents += GoodRow(1, 1, "ONE") + "\n\n\n";
		contents += LegacyRow() + "\n";

		const Fixture fixture = { WriteFixture("skill-blank.csv", contents) };

		InMemorySkillDefinitions provider;
		const auto loaded = LoadSkillCsv(fixture.path.string(), provider);
		REQUIRE(loaded.IsOk());
		CHECK_EQ(loaded.GetValue().accepted, static_cast<std::size_t>(1));
		CHECK_EQ(loaded.GetValue().blankLines, static_cast<std::size_t>(4));
		CHECK_EQ(provider.Find(SkillId{ 1, 1 }) != nullptr, true);
	}

	MODERN_TEST(SkillTable_ARecoveredDefinitionIsLearnableButNotCastable)
	{
		// The admission contract. A definition recovered from the SSKILLBASIC
		// half cannot satisfy `IsValid()`, because that asks for per-level
		// effect data this milestone does not recover. Rather than weaken
		// `IsValid()`, `AddRecoveredBasic` admits it under the narrower
		// `HasRecoveredBasic()`.
		SkillDefinition definition;
		definition.id = SkillId{ 1, 1 };
		definition.name = "LEARN_ONLY";
		definition.maxLevel = 9;

		InMemorySkillDefinitions provider;
		CHECK(!definition.IsValid());   // no level contributes anything yet
		CHECK(definition.HasRecoveredBasic());

		REQUIRE(provider.Add(definition).IsOk() == false);
		REQUIRE(provider.AddRecoveredBasic(definition).IsOk());
		REQUIRE(provider.Find(SkillId{ 1, 1 }) != nullptr);

		// The consequence is deliberate: effects read back as nothing rather
		// than as something wrong.
		const SkillDefinition* stored = provider.Find(SkillId{ 1, 1 });
		REQUIRE(stored != nullptr);
		CHECK_EQ(stored->levelData[1].basicVar, 0.0f);
		CHECK(!stored->RequiresWeapon(SkillWeaponSlot::RightHand));
	}

	// ===========================================================================
	// INTEGRATION: the real ASURA export
	// ===========================================================================

	MODERN_TEST(SkillTable_TheRealExportRecoversEveryCanonicalSkillAndNoLegacyRow)
	{
		std::error_code code;
		std::filesystem::path csv =
		    "D:/FILES/project/RanOnline-Build/ASURA CLIENT/data/glogic/Skill.csv";
		if (!std::filesystem::is_regular_file(csv, code))
		{
			// The export lives outside the repository, so its absence is not
			// a defect in the parser.
			std::printf("      SKIPPED the real export: ASURA Skill.csv not found\n");
			return;
		}

		InMemorySkillDefinitions provider;
		const auto loaded = LoadSkillCsv(csv.string(), provider);
		REQUIRE(loaded.IsOk());

		const SkillTableLoadResult& result = loaded.GetValue();

		// The file is 1,139 blocks of exactly four lines: a canonical header,
		// an SAPPLY header, a canonical row and an SAPPLY row. These counts
		// were established by reading the file, not assumed.
		CHECK_EQ(result.canonicalHeaders, static_cast<std::size_t>(1139));
		CHECK_EQ(result.legacyHeaders, static_cast<std::size_t>(1139));
		CHECK_EQ(result.accepted, static_cast<std::size_t>(1139));
		CHECK_EQ(result.excludedLegacyRows, static_cast<std::size_t>(1139));
		CHECK_EQ(result.duplicateIds, static_cast<std::size_t>(0));
		CHECK_EQ(result.blankLines, static_cast<std::size_t>(0));
		CHECK_EQ(result.Rejected(), static_cast<std::size_t>(0));
		CHECK_EQ(result.loaded, static_cast<std::size_t>(1139));
		CHECK_EQ(provider.GetCount(), static_cast<std::size_t>(1139));
		CHECK_EQ(LastCanonicalHeaderColumns(), static_cast<std::size_t>(322));

		// Every recovered skill is learnable data, and carries a real learn
		// table rather than zeros: (0,1) SN_000_001 costs 1 skill point a level
		// and asks for Dex 26 rising to 64.
		const SkillDefinition* first = provider.Find(SkillId{ 0, 1 });
		REQUIRE(first != nullptr);
		CHECK(first->name == "SN_000_001");
		CHECK(first->HasRecoveredBasic());
		CHECK_EQ(first->learn[1].requiredStats.dex, static_cast<std::uint16_t>(26));
		CHECK_EQ(first->learn[9].requiredStats.dex, static_cast<std::uint16_t>(64));
		CHECK(first->IsValid() == false); // the SAPPLY half is not recovered

		// Not one of the 1,139 SAPPLY rows became a skill. Their leading
		// fields are emBASIC_TYPE/emELEMENT, so had any been parsed it would
		// have registered as id (0,0).
		CHECK(provider.Find(SkillId{ 0, 0 }) == nullptr);

		// And the ids are unique across the recovered set, which is the
		// collision property the identity model promises.
		for (uint16_t main = 0; main <= 53; ++main)
		{
			// Ids present in the export; the exact set is not asserted here
			// because it is data, not contract.
			(void)provider.Find(SkillId{ main, 1 });
		}
	}

} // namespace ModernTests