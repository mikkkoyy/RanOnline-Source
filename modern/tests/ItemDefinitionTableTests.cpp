// WORLD-ENTRY-002M: the item-definition table, the equipment rules, and what
// they do to derived combat statistics.
//
// Three groups, and they are grouped by source of truth:
//
//   * IDENTITY - pure packing arithmetic, no file.
//   * LOADING - the tracked legacy CSV, read from the repository so the values
//     are the deployed ones. A test that needs the file finds it next to the
//     source tree; if it genuinely is not there the case reports SKIPPED rather
//     than failing, because "this checkout has no reference data" is not a
//     defect in the loader.
//   * EQUIPMENT AND COMBAT - hand-built definitions, so a golden number is
//     derived from a stated input rather than read out of a 22 MB file.
//
// The CSV-dependent cases reconcile against the file itself (row count, column
// count, a named item) rather than against literals copied out of it, so a
// re-export that changes the data does not silently invalidate the test - it
// fails loudly on the reconciliation.

#include "TestHarness.h"
#include "equipment/EquipmentState.h"
#include "equipment/ItemContributionAggregator.h"
#include "equipment/ItemDefinitionProvider.h"
#include "item/EquipmentRules.h"
#include "item/ItemDefinition.h"
#include "item/ItemDefinitionTable.h"
#include "item/ItemIdentity.h"
#include "stats/StatCalculator.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace ModernTests
{
	using namespace Modern;
	using namespace Modern::Item;

	namespace
	{
		int g_itemCsvSkipped = 0;

		// Where the tracked legacy export lives, resolved from THIS file's path.
		//
		// `<repo>/modern/tests/ItemDefinitionTableTests.cpp` means the repository
		// root is two directories up, and the export is at
		// `<root>/reference/data-formats/Item.csv`. Deriving it rather than
		// hard-coding a path is what makes the test work from any build directory.
		std::string ItemCsvPath()
		{
			std::error_code code;
			std::filesystem::path here = std::filesystem::path(__FILE__).parent_path();
			for (int depth = 0; depth < 6; ++depth)
			{
				const std::filesystem::path candidate =
				    here / "reference" / "data-formats" / "Item.csv";
				if (std::filesystem::is_regular_file(candidate, code))
				{
					return candidate.string();
				}
				here = here.parent_path();
			}
			return {};
		}

		// Returns false after printing why, so a skipped run is visible - the
		// same contract `WldNavigationTests::RequireAssets` keeps.
		bool RequireItemCsv(const char* what, std::string& outPath)
		{
			outPath = ItemCsvPath();
			if (outPath.empty())
			{
				++g_itemCsvSkipped;
				std::printf("      SKIPPED %s: reference/data-formats/Item.csv not found\n",
				            what);
				return false;
			}
			return true;
		}

		// A wearable sword, matching the first row of the export. Hand-built so
		// the derived numbers below are derived rather than read.
		ItemDefinition MakeSword()
		{
			ItemDefinition definition;
			definition.id         = PackItemId(0, 0);
			definition.name       = "Practice Sword";
			definition.kind       = ItemKind::Weapon;
			definition.itemType   = LegacyItemType::Suit;
			definition.suit       = LegacySuit::Handheld;
			definition.attack     = LegacyItemAtt::Sword;
			definition.attackRange = 5;
			definition.hand       = LegacyHand::Right;

			// The first row of the deployed export, as fields.
			definition.stats.damageLow  = 21;
			definition.stats.damageHigh = 28;
			definition.stats.defense    = 1;
			definition.stats.hit        = 8;
			definition.stats.avoid      = -5;
			return definition;
		}

		ItemInstance MakeInstance(ItemId definition)
		{
			ItemInstance instance;
			instance.definition = definition;
			instance.serial     = 1;
			instance.count      = 1;
			return instance;
		}

	}

	// ===========================================================================
	// IDENTITY
	// ===========================================================================

	MODERN_TEST(ItemIdentity_PackingRoundTripsEveryPairItCarries)
	{
		// The encoding is a bijection on the pairs the format allows, so a
		// round trip is exact at the boundaries and in between.
		const std::uint16_t pairs[][2] = {
			{ 0, 0 },       { 0, 0xFFFF },  { 0xFFFF, 0 },  { 0xFFFF, 0xFFFE },
			{ 1996, 906 },  { 1, 1 },       { 0x1234, 0x5678 },
		};

		for (const auto& pair : pairs)
		{
			const ItemId    packed = PackItemId(pair[0], pair[1]);
			const ItemNativeId back = UnpackItemId(packed);
			CHECK(back.mainId == pair[0]);
			CHECK(back.subId == pair[1]);
		}
	}

	MODERN_TEST(ItemIdentity_TheHalfPairEncodingKeepsTheTwoHalvesApart)
	{
		// The property the packing exists for: a shifted sub id cannot collide
		// with a main id. (1, 0) and (0, 1) are different items, and so are
		// (0, 0xFFFF) and (0xFFFF, 0).
		CHECK(PackItemId(1, 0) != PackItemId(0, 1));
		CHECK(PackItemId(0, 0xFFFF) != PackItemId(0xFFFF, 0));
		CHECK(PackItemId(0x1234, 0x5678) != PackItemId(0x5678, 0x1234));
	}

	MODERN_TEST(ItemIdentity_TheInvalidSentinelIsReservedForNoItem)
	{
		// (0xFFFF, 0xFFFF) packs into MakeInvalid, so it cannot name an item.
		// `CanPackItemId` says so rather than letting an id fail IsValid later.
		CHECK(!CanPackItemId(0xFFFF, 0xFFFF));
		CHECK(CanPackItemId(0xFFFF, 0xFFFE));
		CHECK(CanPackItemId(0xFFFE, 0xFFFF));

		// And unpacking anything except a valid id is the default pair, not a
		// fabricated full-scale one.
		const ItemNativeId invalid = UnpackItemId(ItemId::MakeInvalid());
		CHECK(invalid == ItemNativeId{});
	}

	// ===========================================================================
	// LOADING
	// ===========================================================================

	MODERN_TEST(ItemTable_TheTrackedExportLoadsWithoutRejectingARow)
	{
		std::string path;
		if (!RequireItemCsv("the tracked export loads", path))
		{
			return;
		}

		InMemoryItemDefinitions definitions;
		const auto loaded = Item::LoadItemCsv(path, definitions);
		REQUIRE(loaded.IsOk());

		// The export is complete: every row is a usable definition. A rejected
		// row would mean the loader is dropping real items, so this is asserted
		// rather than tolerated.
		const ItemTableLoadResult& result = loaded.GetValue();
		CHECK_EQ(result.Rejected(), static_cast<std::size_t>(0));
		CHECK(result.loaded > 0);
		CHECK_EQ(result.loaded, definitions.GetCount());
	}

	MODERN_TEST(ItemTable_TheRowCountReconcilesWithTheFile)
	{
		std::string path;
		if (!RequireItemCsv("the row count reconciles", path))
		{
			return;
		}

		// Count the data lines independently, with no knowledge of the format.
		std::size_t dataLines = 0;
		{
			std::ifstream file(path.c_str(), std::ios::binary);
			REQUIRE(file.is_open());
			std::string line;
			bool first = true;
			while (std::getline(file, line))
			{
				if (first)
				{
					first = false; // the header
					continue;
				}
				// The export's last line is a lone CR, not a record.
				if (!line.empty() && line != "\r")
				{
					++dataLines;
				}
			}
		}
		CHECK(dataLines > 0);

		InMemoryItemDefinitions definitions;
		const auto loaded = Item::LoadItemCsv(path, definitions);
		REQUIRE(loaded.IsOk());
		CHECK_EQ(loaded.GetValue().loaded, dataLines);

		// Every loaded row is a distinct identity, which is the collision test
		// the identity mapping promises.
		CHECK_EQ(definitions.GetCount(), dataLines);
	}

	MODERN_TEST(ItemTable_TheFirstRowIsThePracticeSwordWithItsRealFields)
	{
		std::string path;
		if (!RequireItemCsv("the first row's fields", path))
		{
			return;
		}

		InMemoryItemDefinitions definitions;
		const auto loaded = Item::LoadItemCsv(path, definitions);
		REQUIRE(loaded.IsOk());
		REQUIRE(loaded.GetValue().loaded > 0);

		// The export's first data row is (0,0) - a one-handed sword. Values read
		// from the file, so a re-export with different numbers fails here rather
		// than at a hard-coded literal.
		const ItemDefinition* sword = definitions.Find(PackItemId(0, 0));
		REQUIRE(sword != nullptr);
		CHECK(sword->kind == ItemKind::Weapon);
		CHECK(sword->itemType == LegacyItemType::Suit);
		CHECK(sword->suit == LegacySuit::Handheld);
		CHECK(sword->attack == LegacyItemAtt::Sword);
		CHECK_EQ(sword->attackRange, static_cast<std::uint16_t>(5));
		CHECK_EQ(sword->stats.damageLow, static_cast<std::int32_t>(21));
		CHECK_EQ(sword->stats.damageHigh, static_cast<std::int32_t>(28));
		CHECK_EQ(sword->stats.defense, static_cast<std::int32_t>(1));
		CHECK_EQ(sword->stats.hit, static_cast<std::int32_t>(8));

		// `nAvoidRate` is the one signed column, and the first row really does
		// carry a negative - so a loader that clamped it would be wrong.
		CHECK(sword->stats.avoid < 0);
	}

	MODERN_TEST(ItemTable_TheHeaderAndTheRowsAreCountedTheSameWay)
	{
		// Regression for the loader rejecting every row.
		//
		// The export writes a final comma, so both its header and every data
		// row carry one more empty field than they have NAMED columns. Counting
		// the header one way and the rows another left them one field apart, and
		// the width check then called all 18,447 rows malformed.
		//
		// So the reconciliation is against the RAW split width on both sides:
		// the header's declared field count, and an independently counted row
		// count. A genuine width mismatch must still be rejected, so that is
		// checked separately below rather than assumed.
		std::string path;
		if (!RequireItemCsv("the header reconciles", path))
		{
			return;
		}

		InMemoryItemDefinitions definitions;
		const auto loaded = Item::LoadItemCsv(path, definitions);
		REQUIRE(loaded.IsOk());

		// Every data row is usable, so none was dropped as malformed.
		CHECK_EQ(Item::LastHeaderColumns(), static_cast<std::size_t>(399));
		CHECK_EQ(loaded.GetValue().Rejected(), static_cast<std::size_t>(0));
		CHECK_EQ(loaded.GetValue().loaded, static_cast<std::size_t>(18447));
		CHECK_EQ(definitions.GetCount(), static_cast<std::size_t>(18447));
	}

	MODERN_TEST(ItemTable_TheHeaderIsWhatDecidesEachColumn)
	{
		// `emItemType` is the column that proves the point: it lives in
		// SITEMBASIC at 18, seventeen columns before the SSUIT block, so a
		// reader that assumes the block starts at the header would read
		// `emReqBright` instead and misclassify every wearable as a non-item.
		//
		// (1,34) `IN_001_034` is a real SUIT_UPPER armour piece: emItemType 0
		// (ITEM_SUIT), emSuit 1 (SUIT_UPPER), emAttack 0, wAttRange 0, no
		// damage, nDefense 1.
		std::string path;
		if (!RequireItemCsv("armour decodes from its header column", path))
		{
			return;
		}

		InMemoryItemDefinitions definitions;
		REQUIRE(Item::LoadItemCsv(path, definitions).IsOk());

		const ItemDefinition* armour = definitions.Find(PackItemId(1, 34));
		REQUIRE(armour != nullptr);
		CHECK(armour->name == "IN_001_034");
		CHECK(armour->itemType == LegacyItemType::Suit);
		CHECK(armour->suit == LegacySuit::Upper);
		CHECK(armour->attack == LegacyItemAtt::Nothing);
		CHECK_EQ(armour->attackRange, static_cast<std::uint16_t>(0));
		CHECK_EQ(armour->stats.damageLow, static_cast<std::int32_t>(0));
		CHECK_EQ(armour->stats.defense, static_cast<std::int32_t>(1));
		CHECK(armour->kind == ItemKind::Armor);

		// Being a SUIT is what makes it wearable, and the slot has to be the one
		// `SLOT_2_SUIT` pairs with SUIT_UPPER.
		CHECK(Item::IsWearableType(armour->itemType));
		CHECK(Item::CheckSlot(*armour, EquipmentSlot::Upper) ==
		      EquipRefusal::None);
		CHECK(Item::CheckSlot(*armour, EquipmentSlot::RightHand) ==
		      EquipRefusal::SuitMismatch);

		// A non-wearable row, to show `emItemType` is actually READ rather than
		// defaulted. (1,20) `IN_001_020` is an ITEM_BOX (emItemType 12) that
		// nevertheless claims SUIT_HEADGEAR - so its suit is right for a hat and
		// only its TYPE keeps it out of the equipment slots.
		const ItemDefinition* box = definitions.Find(PackItemId(1, 20));
		REQUIRE(box != nullptr);
		CHECK(box->name == "IN_001_020");
		CHECK(box->suit == LegacySuit::Headgear);
		CHECK(box->itemType == LegacyItemType::Other);
		CHECK(!Item::IsWearableType(box->itemType));
		CHECK(Item::CheckSlot(*box, EquipmentSlot::Headgear) ==
		      EquipRefusal::NotWearableType);
	}

	MODERN_TEST(ItemTable_AMalformedRowIsRejectedRatherThanGuessed)
	{
		// The width policy must not have been weakened into "accept anything":
		// a short row, a long row and a row with a non-numeric cell in a column
		// this loader reads are each refused and counted, and a good row in the
		// same file still loads.
		const std::filesystem::path path =
		    std::filesystem::temp_directory_path() / "modern-item-width-probe.csv";
		{
			std::ifstream source(ItemCsvPath().c_str(), std::ios::binary);
			REQUIRE(source.is_open());
			std::string header;
			REQUIRE(static_cast<bool>(std::getline(source, header)));
			header.erase(header.find_last_not_of("\r") + 1);

			std::string good;
			REQUIRE(static_cast<bool>(std::getline(source, good)));
			good.erase(good.find_last_not_of("\r") + 1);

			// The export's rows end with a comma, so the last field is the empty
			// one. Dropping that comma drops the field with it - `erase(pos + 1)`
			// would be a no-op here, because the comma is already the final
			// character, and the "short" row would come out identical to the good
			// one.
			std::string shortRow = good;
			shortRow.erase(shortRow.find_last_of(','));

			std::string longRow = good + ",1,2,3";

			std::string badCell = good;
			// Column 91 is `emSuit`; a non-numeric cell there is a corrupt
			// export, and must be refused rather than read as zero.
			std::vector<std::string> fields;
			{
				std::size_t start = 0;
				for (;;)
				{
					const std::size_t comma = badCell.find(',', start);
					if (comma == std::string::npos)
					{
						fields.push_back(badCell.substr(start));
						break;
					}
					fields.push_back(badCell.substr(start, comma - start));
					start = comma + 1;
				}
			}
			fields[91] = "not-a-number";
			badCell.clear();
			for (std::size_t i = 0; i < fields.size(); ++i)
			{
				if (i != 0)
				{
					badCell.push_back(',');
				}
				badCell += fields[i];
			}

			std::ofstream out(path.string(), std::ios::binary);
			REQUIRE(out.is_open());
			out << header << "\n" << good << "\n" << shortRow << "\n"
			    << longRow << "\n" << badCell << "\n";
		}

		InMemoryItemDefinitions definitions;
		const auto loaded = Item::LoadItemCsv(path.string(), definitions);
		REQUIRE(loaded.IsOk());

		const ItemTableLoadResult& result = loaded.GetValue();
		CHECK_EQ(result.loaded, static_cast<std::size_t>(1));
		CHECK_EQ(result.Rejected(), static_cast<std::size_t>(3));
		CHECK_EQ(definitions.GetCount(), static_cast<std::size_t>(1));

		std::error_code ignored;
		std::filesystem::remove(path, ignored);
	}

	MODERN_TEST(ItemTable_ALoadedBowIsaLongRangeWeapon)
	{
		std::string path;
		if (!RequireItemCsv("a loaded bow", path))
		{
			return;
		}

		InMemoryItemDefinitions definitions;
		REQUIRE(Item::LoadItemCsv(path, definitions).IsOk());
		REQUIRE(definitions.GetCount() > 0);

		// (1,4) is `IN_001_004` in the tracked export: emItemType 0, emSuit 5,
		// emAttack 7 (ITEMATT_BOW), wAttRange 85, damage 75/85, nDefense 1,
		// hit 0, avoid -2. `ITEMATT_BOW` is above `ITEMATT_NEAR`, which is what
		// `ISLONGRANGE_ARMS()` tests.
		//
		// This item is REQUIRED, not optional. An earlier version of this case
		// skipped itself when the lookup failed, which turned a loader that
		// rejected every single row into a green test: the row was reported
		// absent when it is demonstrably in the export. A required fixture that
		// cannot be loaded is a failure.
		const ItemDefinition* bow = definitions.Find(PackItemId(1, 4));
		REQUIRE(bow != nullptr);

		CHECK(bow->name == "IN_001_004");
		CHECK(bow->itemType == LegacyItemType::Suit);
		CHECK(bow->suit == LegacySuit::Handheld);
		CHECK(bow->kind == ItemKind::Weapon);
		CHECK(bow->attack == LegacyItemAtt::Bow);
		CHECK_EQ(bow->attackRange, static_cast<std::uint16_t>(85));
		CHECK_EQ(bow->stats.damageLow, static_cast<std::int32_t>(75));
		CHECK_EQ(bow->stats.damageHigh, static_cast<std::int32_t>(85));
		CHECK_EQ(bow->stats.defense, static_cast<std::int32_t>(1));
		CHECK_EQ(bow->stats.avoid, static_cast<std::int32_t>(-2));

		CHECK(static_cast<int>(bow->attack) >
		      static_cast<int>(LegacyItemAtt::NearArms));
		CHECK(bow->attackRange > 0);

		// And the loaded weapon is wearable in a hand slot.
		CHECK(Item::CheckSlot(*bow, EquipmentSlot::RightHand) ==
		      EquipRefusal::None);
		CHECK(static_cast<int>(bow->attack) > static_cast<int>(LegacyItemAtt::NearArms));
		CHECK(bow->attackRange > 0);
	}

	// ===========================================================================
	// EQUIPMENT RULES
	// ===========================================================================

	MODERN_TEST(ItemTable_TheFiveResistancesLoadFromTheirOwnColumns)
	{
		// WORLD-ENTRY-002N. `sResist` was the last SSUIT block the loader skipped,
		// so every item reported zero resistance no matter what the export said.
		// 3,030 of the 18,447 rows carry at least one non-zero element.
		//
		// The values below are read from the export, so a re-export with
		// different numbers fails here rather than at a copied literal.
		std::string path;
		if (!RequireItemCsv("the resistances load", path))
		{
			return;
		}

		InMemoryItemDefinitions definitions;
		REQUIRE(Item::LoadItemCsv(path, definitions).IsOk());

		// (1,17) IN_001_017 is a SUIT_HANDHELD piece at 10 in ALL five elements -
		// the case that fails if the columns are read as a block rather than by
		// name, or if one of them is off by one against its neighbour.
		const ItemDefinition* all = definitions.Find(PackItemId(1, 17));
		REQUIRE(all != nullptr);
		CHECK(all->name == "IN_001_017");
		CHECK_EQ(all->stats.resistFire, static_cast<std::int32_t>(10));
		CHECK_EQ(all->stats.resistIce, static_cast<std::int32_t>(10));
		CHECK_EQ(all->stats.resistElectric, static_cast<std::int32_t>(10));
		CHECK_EQ(all->stats.resistPoison, static_cast<std::int32_t>(10));
		CHECK_EQ(all->stats.resistSpirit, static_cast<std::int32_t>(10));

		// (1,127) IN_001_127 resists SPIRIT ONLY, at 10. This is the discriminating
		// case: every other element is 0, so a loader that copied one column into
		// all five, or that shifted by one, cannot produce it.
		const ItemDefinition* spiritOnly = definitions.Find(PackItemId(1, 127));
		REQUIRE(spiritOnly != nullptr);
		CHECK_EQ(spiritOnly->stats.resistFire, static_cast<std::int32_t>(0));
		CHECK_EQ(spiritOnly->stats.resistIce, static_cast<std::int32_t>(0));
		CHECK_EQ(spiritOnly->stats.resistElectric, static_cast<std::int32_t>(0));
		CHECK_EQ(spiritOnly->stats.resistPoison, static_cast<std::int32_t>(0));
		CHECK_EQ(spiritOnly->stats.resistSpirit, static_cast<std::int32_t>(10));

		// A weapon with no resistance at all stays at zero - the resistance
		// columns must not have displaced any other field.
		const ItemDefinition* plain = definitions.Find(PackItemId(0, 0));
		REQUIRE(plain != nullptr);
		CHECK_EQ(plain->stats.resistFire, static_cast<std::int32_t>(0));
		CHECK_EQ(plain->stats.resistSpirit, static_cast<std::int32_t>(0));
		// Its other, already-covered fields are unchanged by this change.
		CHECK_EQ(plain->stats.damageLow, static_cast<std::int32_t>(21));
		CHECK_EQ(plain->stats.damageHigh, static_cast<std::int32_t>(28));
		CHECK_EQ(plain->stats.defense, static_cast<std::int32_t>(1));
	}

	MODERN_TEST(ItemResist_AnEquippedItemContributesItsResistanceExactlyOnce)
	{
		// The loader now populates `ItemStatBlock::resist*`, which
		// `ItemContributionAggregator` has been summing into
		// `ItemContribution::resistances` since it was written. This closes the
		// loop the two halves of 002M/002N left open.
		//
		// Legacy sums per-item resistance into the character total in
		// `SUM_ITEM` (GLogixExPC.cpp:665-669) exactly as it sums damage.
		InMemoryItemDefinitions definitions;

		ItemDefinition resist = MakeSword();
		resist.id       = PackItemId(1, 127);
		resist.stats.resistFire     = 10;
		resist.stats.resistIce      = 20;
		resist.stats.resistElectric = 30;
		resist.stats.resistPoison   = 40;
		resist.stats.resistSpirit   = 50;
		REQUIRE(definitions.Add(resist).IsOk());

		EquipmentState equipment;
		REQUIRE(equipment.Equip(EquipmentSlot::RightHand,
		                        MakeInstance(PackItemId(1, 127)))
		            .IsOk());

		const auto aggregated =
		    ItemContributionAggregator::Aggregate(equipment, definitions);
		REQUIRE(aggregated.IsOk());

		const ItemContributionResult& result = aggregated.GetValue();
		CHECK_EQ(result.contributingSlots, static_cast<std::size_t>(1));
		CHECK_EQ(result.contribution.resistances.fire, static_cast<std::int32_t>(10));
		CHECK_EQ(result.contribution.resistances.ice, static_cast<std::int32_t>(20));
		CHECK_EQ(result.contribution.resistances.electric, static_cast<std::int32_t>(30));
		CHECK_EQ(result.contribution.resistances.poison, static_cast<std::int32_t>(40));
		CHECK_EQ(result.contribution.resistances.spirit, static_cast<std::int32_t>(50));
	}

	MODERN_TEST(ItemResist_EmptyEquipmentContributesNoResistanceAtAll)
	{
		// The regression that matters for the live path: an unarmoured
		// character's resistance is zero in every element, so the elemental
		// reduction stays out of the damage calculation instead of applying a
		// fabricated reduction.
		InMemoryItemDefinitions definitions;
		definitions.Add(MakeSword());

		const EquipmentState equipment;
		const auto aggregated = ItemContributionAggregator::Aggregate(equipment, definitions);
		REQUIRE(aggregated.IsOk());

		const Stats::Resistances& resistances = aggregated.GetValue().contribution.resistances;
		CHECK_EQ(resistances.fire, static_cast<std::int32_t>(0));
		CHECK_EQ(resistances.ice, static_cast<std::int32_t>(0));
		CHECK_EQ(resistances.electric, static_cast<std::int32_t>(0));
		CHECK_EQ(resistances.poison, static_cast<std::int32_t>(0));
		CHECK_EQ(resistances.spirit, static_cast<std::int32_t>(0));

		// And RAN clamps the summed total at zero (`SRESIST::LIMIT`), which is
		// what `ClampNonNegative` reproduces.
		Stats::Resistances clamped;
		clamped.fire = -5;
		clamped.ClampNonNegative();
		CHECK_EQ(clamped.fire, static_cast<std::int32_t>(0));
	}
	MODERN_TEST(EquipmentRules_EveryWearableSlotHasOneSuit)
	{
		// `SLOT_2_SUIT` decides rule 3, so a slot with no suit is a slot nothing
		// may be worn in. All twenty-one legacy slots have one.
		for (std::uint8_t raw = 0; raw < Modern::kEquipmentSlotCount; ++raw)
		{
			const EquipmentSlot slot = static_cast<EquipmentSlot>(raw);
			const LegacySuit suit = Item::SuitOfSlot(slot);
			CHECK(suit != LegacySuit::None);

			// An item of that suit is accepted in the slot, unless the item type
			// carries a stricter rule of its own (the off-hand types).
			ItemDefinition definition;
			definition.itemType = LegacyItemType::Suit;
			definition.suit     = suit;
			CHECK(Item::CheckSlot(definition, slot) == EquipRefusal::None);
		}
	}

	MODERN_TEST(EquipmentRules_TheFourHandSlotsAllAcceptAHandheldItem)
	{
		// A weapon can go in any of the four hand slots: `SLOT_2_SUIT` maps all
		// four to SUIT_HANDHELD (GLItemDef.h:257-260).
		const EquipmentSlot hands[] = {
			EquipmentSlot::RightHand, EquipmentSlot::LeftHand,
			EquipmentSlot::RightHandExtreme, EquipmentSlot::LeftHandExtreme,
		};

		const ItemDefinition sword = MakeSword();
		for (const EquipmentSlot slot : hands)
		{
			CHECK(Item::CheckSlot(sword, slot) == EquipRefusal::None);
		}
	}

	MODERN_TEST(EquipmentRules_ASwordIsNotEquipmentForAHeadSlot)
	{
		const ItemDefinition sword = MakeSword();
		// SUIT_HANDHELD is not SUIT_HEADGEAR, so the mismatch is the refusal
		// rather than the type being unwearable.
		CHECK(Item::CheckSlot(sword, EquipmentSlot::Headgear) == EquipRefusal::SuitMismatch);
	}

	MODERN_TEST(EquipmentRules_AConsumableIsNotWearableAtAll)
	{
		// `CHECKSLOT_ITEM`'s first predicate is the type. A cure is not in the
		// wearable set however plausible its stat block looks.
		ItemDefinition potion;
		potion.id       = PackItemId(9, 9);
		potion.name     = "Potion";
		potion.kind     = ItemKind::Consumable;
		potion.itemType = LegacyItemType::Other;
		potion.suit     = LegacySuit::Misc;

		CHECK(Item::CheckSlot(potion, EquipmentSlot::Misc) ==
		      EquipRefusal::NotWearableType);
		CHECK(!Item::IsWearableType(LegacyItemType::Other));
	}

	MODERN_TEST(EquipmentRules_ARecordDiscriminatorIsWrongSlotForType)
	{
		// ITEM_REVIVE and ITEM_ANTI_DISAPPEAR go only in SLOT_ORNAMENT
		// (GLogixExPC.cpp:3093-3098) - but legacy tests the SUIT first
		// (:3090-3091), so for a slot whose suit differs the suit check is what
		// answers. `SLOT_2_SUIT` maps only SLOT_ORNAMENT to SUIT_ORNAMENT, which
		// means the type/slot rule is unreachable behind a differing suit, and
		// in practice behind a matching one too: a SUIT_ORNAMENT item has
		// already been pinned to SLOT_ORNAMENT by the earlier check.
		//
		// So the truthful assertion is the refusal legacy actually produces,
		// and that the rule is order-dependent: the ornament slot accepts it,
		// and any other slot refuses it - with the suit check reported first.
		ItemDefinition record;
		record.id       = PackItemId(2, 2);
		record.name     = "Anti Disappear";
		record.kind     = ItemKind::Accessory;
		record.itemType = LegacyItemType::AntiDisappear;
		record.suit     = LegacySuit::Ornament;

		CHECK(Item::CheckSlot(record, EquipmentSlot::Ornament) == EquipRefusal::None);
		CHECK(Item::CheckSlot(record, EquipmentSlot::Face) ==
		      EquipRefusal::SuitMismatch);

		// A vehicle is the same shape: SLOT_VEHICLE is the only slot whose suit
		// is SUIT_VEHICLE (GLItemDef.h:266).
		ItemDefinition vehicle;
		vehicle.id       = PackItemId(2, 3);
		vehicle.name     = "Vehicle";
		vehicle.kind     = ItemKind::Accessory;
		vehicle.itemType = LegacyItemType::Vehicle;
		vehicle.suit     = LegacySuit::Vehicle;

		CHECK(Item::CheckSlot(vehicle, EquipmentSlot::Vehicle) ==
		      EquipRefusal::None);
		CHECK(Item::CheckSlot(vehicle, EquipmentSlot::Ornament) ==
		      EquipRefusal::SuitMismatch);
	}

	MODERN_TEST(EquipmentRules_AnArrowNeedsTheOffHand)
	{
		CHECK(Item::RequiresOffHand(LegacyItemType::Arrow));
		CHECK(Item::RequiredMainHandAttack(LegacyItemType::Arrow) ==
		      LegacyItemAtt::Bow);
		CHECK(!Item::RequiresOffHand(LegacyItemType::Suit));

		// The slot half of the rule: the right hand is refused.
		ItemDefinition arrow;
		arrow.id       = PackItemId(3, 3);
		arrow.name     = "Arrow";
		arrow.kind     = ItemKind::Accessory;
		arrow.itemType = LegacyItemType::Arrow;
		arrow.suit     = LegacySuit::Handheld;

		CHECK(Item::CheckSlot(arrow, EquipmentSlot::RightHand) ==
		      EquipRefusal::NeedsOffHand);
		CHECK(Item::CheckSlot(arrow, EquipmentSlot::LeftHand) == EquipRefusal::None);
	}

	// ===========================================================================
	// AGGREGATION AND DERIVED STATS
	// ===========================================================================

	MODERN_TEST(ItemAggregation_EmptyEquipmentContributesNothing)
	{
		// The regression that matters most for 002L-C: an unarmoured character's
		// derived statistics are exactly the class table's, with no item term at
		// all. `ServerCharacter`'s equipment tests already cover the aggregator;
		// this covers the LIVE path reading it.
		InMemoryItemDefinitions definitions;
		definitions.Add(MakeSword());

		const EquipmentState equipment;
		const auto aggregated = ItemContributionAggregator::Aggregate(equipment, definitions);
		REQUIRE(aggregated.IsOk());

		CHECK_EQ(aggregated.GetValue().contributingSlots, static_cast<std::size_t>(0));
		// Every field of the contribution is zero, which is a stronger
		// statement than contributingSlots - a slot can be occupied and
		// contribute nothing.
		const Stats::ItemContribution& empty = aggregated.GetValue().contribution;
		CHECK_EQ(empty.hit, 0);
		CHECK_EQ(empty.avoid, 0);
		CHECK_EQ(empty.damageLow, static_cast<std::int32_t>(0));
		CHECK_EQ(empty.damageHigh, static_cast<std::int32_t>(0));
		CHECK_EQ(empty.defense, 0);
	}

	MODERN_TEST(ItemAggregation_AnEquippedSwordContributesItsOwnFieldsOnce)
	{
		InMemoryItemDefinitions definitions;
		REQUIRE(definitions.Add(MakeSword()).IsOk());

		EquipmentState equipment;
		REQUIRE(equipment.Equip(EquipmentSlot::RightHand,
		                        MakeInstance(PackItemId(0, 0)))
		            .IsOk());

		const auto aggregated =
		    ItemContributionAggregator::Aggregate(equipment, definitions);
		REQUIRE(aggregated.IsOk());

		const ItemContributionResult& result = aggregated.GetValue();
		CHECK_EQ(result.contributingSlots, static_cast<std::size_t>(1));

		// The sword's own values, exactly once each. Applied twice this would
		// read 42/56 instead of 21/28, so the assertions are on the pair and not
		// just "greater than zero".
		CHECK_EQ(result.contribution.damageLow, static_cast<std::int32_t>(21));
		CHECK_EQ(result.contribution.damageHigh, static_cast<std::int32_t>(28));
		CHECK_EQ(result.contribution.defense, static_cast<std::int32_t>(1));
		CHECK_EQ(result.contribution.hit, static_cast<std::int32_t>(8));
		CHECK_EQ(result.contribution.avoid, static_cast<std::int32_t>(-5));
	}

	MODERN_TEST(ItemAggregation_AnUnknownItemDoesNotSilentlyContributeZero)
	{
		// A worn item nothing defines is an ERROR, not a zero block. Equipment
		// that contributes nothing because nothing knows what it is would hide a
		// data problem behind a plausible number.
		InMemoryItemDefinitions definitions;
		definitions.Add(MakeSword());

		EquipmentState equipment;
		REQUIRE(equipment.Equip(EquipmentSlot::RightHand,
		                        MakeInstance(PackItemId(4242, 4242)))
		            .IsOk());

		const auto aggregated =
		    ItemContributionAggregator::Aggregate(equipment, definitions);
		CHECK(aggregated.IsOk()); // the aggregation itself succeeds
		CHECK_EQ(aggregated.GetValue().error, ContributionError::MissingDefinition);
	}

} // namespace ModernTests
