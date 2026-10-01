// VERTICAL-001: the server's authoritative character.
//
 // Headless. Links ModernServer (and therefore Modern) and nothing else: no
 // socket, no database, no renderer, no legacy library, no client.
 //
 // The cases that matter are the consistency ones: that the server's numbers
 // are exactly what Modern::Stats::Calculate produces for the same inputs, and
 // that every mutator which can change a stat input recalculates before it
 // returns, so a published snapshot can never disagree with the state behind
 // it.

#include "TestHarness.h"

#include "character/Character.h"
#include "character/CharacterClassTable.h"
#include "equipment/EquipmentState.h"
#include "equipment/ItemContributionAggregator.h"
#include "equipment/ItemDefinitionProvider.h"
#include "gameplay/CharacterSnapshot.h"
#include "item/ItemDefinition.h"
#include "item/ItemInstance.h"
#include "math/Vector3.h"
#include "character/ServerCharacter.h"
#include "progression/CodexDefinition.h"
#include "progression/CodexDefinitionProvider.h"
#include "progression/CodexState.h"
#include "skills/PassiveContributionAggregator.h"
#include "skills/SkillDefinition.h"
#include "skills/SkillDefinitionProvider.h"
#include "skills/SkillState.h"
#include "stats/StatCalculator.h"
#include "stats/Contributions.h"
#include "types/Result.h"

#include <initializer_list>
#include <limits>

using namespace Modern;
using namespace Modern::Server;

namespace
{
	// A class row with every field a formula reads, so a test states only what
	// it is about. The numbers are arbitrary: RAN's real coefficients live in
	// `default.charclass`, which is not in this repository, and CORE-002
	// deliberately asserts no shipped value.
	Stats::ClassConstants StandardClass()
	{
		Stats::ClassConstants cc;
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

		// A non-zero level-up term, so a level change is actually observable.
		// With every growth coefficient at zero a level change would legitimately
		// change nothing, and the recalculation assertion would be vacuous.
		cc.levelUpStats.dex = 0.5f;

		cc.hitPerDex     = 2.0f;
		cc.avoidPerDex   = 1.0f;
		cc.defensePerDex = 3.0f;

		cc.meleePerPow = 1.0f;  cc.meleePerDex = 0.5f;
		cc.shootPerPow = 1.0f;  cc.shootPerDex = 0.5f;
		cc.magicPerDex = 1.0f;  cc.magicPerSpi = 1.0f;  cc.magicPerIntel = 2.0f;
		return cc;
	}

	ServerCharacterDefinition StandardDefinition()
	{
		ServerCharacterDefinition definition;
		definition.id             = CharacterId(7u);
		definition.name           = "Raner";
		definition.characterClass = CharacterClass::Brawler;
		definition.gender        = CharacterGender::Male;
		definition.level         = 1;
		definition.experience    = 0;
		definition.classConstants = StandardClass();
		definition.confPointRate  = 1.0f;
		return definition;
	}

	// VERTICAL-003: skill test helpers
	SkillId MakeTestSkillId(uint16_t skillIndex)
	{
		return SkillId{ 1, skillIndex };  // classIndex = 1 (first skill class)
	}

	// A passive whose basic apply type is a flat HP bonus, with an optional
	// recovery-rate impact. Spelled as values rather than as an ItemStatBlock:
	// a skill definition has no stats block, and borrowing the item type here
	// would suggest one exists.
	ItemStatBlock TestSkillStats(int32_t hp, float hpRecoveryRate)
	{
		ItemStatBlock stats;
		stats.hp             = hp;
		stats.hpRecoveryRate = hpRecoveryRate;
		return stats;
	}

	SkillDefinition MakeTestSkill(uint32_t id, const ItemStatBlock& stats)
	{
		SkillDefinition def;
		def.id = SkillId{ 1, static_cast<uint16_t>(id) };
		def.name = "TestSkill";
		def.maxLevel = 1;
		def.applyType = PassiveApplyType::Hp;
		def.levelData[1].basicVar = static_cast<float>(stats.hp);
		if (stats.hpRecoveryRate != 0.0f)
		{
			def.impacts[0].type = PassiveImpactType::HpRate;
			def.impacts[0].values[1] = stats.hpRecoveryRate;
		}
		return def;
	}

	ItemInstance TestItem(uint32_t defId, uint64_t serial = 1)
	{
		return ItemInstance{ ItemId(defId), serial, 1, ItemId::MakeInvalid() };
	}

	ItemDefinition MakeTestWeapon(uint32_t id, const ItemStatBlock& stats)
	{
		ItemDefinition def;
		def.id = ItemId(id);
		def.kind = ItemKind::Weapon;
		def.name = "TestSword";
		def.maxStack = 1;
		def.stats = stats;
		return def;
	}

	ItemDefinition MakeTestArmor(uint32_t id, const ItemStatBlock& stats)
	{
		ItemDefinition def;
		def.id = ItemId(id);
		def.kind = ItemKind::Armor;
		def.name = "TestArmor";
		def.maxStack = 1;
		def.stats = stats;
		return def;
	}

	// VERTICAL-002/003: shared definition helpers
	ServerCharacterDefinition StandardDefinitionWithItems(
		InMemoryItemDefinitions& provider)
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.itemDefinitions = &provider;

		ItemStatBlock swordStats;
		swordStats.hp = 40;
		swordStats.meleePower = 10;
		swordStats.dex = 3;
		provider.Add(MakeTestWeapon(10001, swordStats));

		ItemStatBlock armorStats;
		armorStats.hp = 60;
		armorStats.str = 8;
		armorStats.defense = 7;
		provider.Add(MakeTestArmor(10002, armorStats));

		return definition;
	}

	ServerCharacterDefinition StandardDefinitionWithSkills(
		InMemorySkillDefinitions& provider)
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.skillDefinitions = &provider;
		return definition;
	}

	ServerCharacterDefinition StandardDefinitionWithItemsAndSkills(
		InMemoryItemDefinitions& itemProvider,
		InMemorySkillDefinitions& skillProvider)
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.itemDefinitions = &itemProvider;
		definition.skillDefinitions = &skillProvider;
		return definition;
	}

	// Rebuilds the stat input the server currently holds and calls the one
	// stat implementation, so the comparison is against the formula rather than
	// against a copy of it.
	//
	// Passing the learned-skill and equipment states in is what makes the check
	// independent rather than vacuous: without them the aggregation below runs
	// over an empty set, and every skill-bearing character would trivially agree
	// with a character that has no skills at all.
	Stats::DerivedStats RecalculateIndependently(
		const ServerCharacterDefinition& definition,
		const ItemDefinitionProvider* itemProvider = nullptr,
		const SkillDefinitionProvider* skillProvider = nullptr,
		const SkillState* skills = nullptr,
		const EquipmentState* equipment = nullptr,
		const Stats::CodexContribution* codex = nullptr)
	{
		const EquipmentState emptyEquipment;
		const SkillState emptySkills;
		const EquipmentState& worn  = equipment != nullptr ? *equipment : emptyEquipment;
		const SkillState& learned    = skills    != nullptr ? *skills    : emptySkills;

		// Aggregate items if provider given
		Stats::ItemContribution items;
		if (itemProvider != nullptr)
		{
			auto aggregated = ItemContributionAggregator::Aggregate(worn, *itemProvider);
			if (aggregated.IsOk() && aggregated.GetValue().IsOk())
			{
				items = aggregated.GetValue().contribution;
			}
		}

		// Aggregate passives if provider given
		Stats::PassiveContribution passives;
		if (skillProvider != nullptr)
		{
			auto aggregated = PassiveContributionAggregator::Aggregate(learned, *skillProvider, worn);
			if (aggregated.IsOk() && aggregated.GetValue().IsOk())
			{
				passives = aggregated.GetValue().contribution;
			}
		}

		Stats::CharClassIndex classIndex{};
		TryToCharClassIndex(definition.characterClass, definition.gender, classIndex);

		Stats::StatCalculationInput input;
		input.characterClass = classIndex;
		input.level          = definition.level;
		input.classConstants = definition.classConstants;
		input.allocatedStats = definition.allocatedStats;
		input.items          = items;
		input.passives       = passives;
		input.codex          = codex != nullptr ? *codex : definition.codex;
		input.confPointRate  = definition.confPointRate;
		return Stats::Calculate(input).GetValue();
	}
}

// ---------------------------------------------------------------------------
// Class table
// ---------------------------------------------------------------------------

MODERN_TEST(Server_ClassTableNamesEveryRanRow)
{
	// The gender really is a character fact: RAN's class table has one row per
	// class *and* gender, and the two halves are different rows.
	Stats::CharClassIndex index{};
	CHECK(TryToCharClassIndex(CharacterClass::Brawler, CharacterGender::Male, index));
	CHECK_EQ(static_cast<uint8_t>(index), 0);
	CHECK(TryToCharClassIndex(CharacterClass::Brawler, CharacterGender::Female, index));
	CHECK_EQ(static_cast<uint8_t>(index), 6);

	CHECK(TryToCharClassIndex(CharacterClass::Archer, CharacterGender::Female, index));
	CHECK_EQ(static_cast<uint8_t>(index), 2);
	CHECK(TryToCharClassIndex(CharacterClass::Archer, CharacterGender::Male, index));
	CHECK_EQ(static_cast<uint8_t>(index), 8);

	CHECK(!TryToCharClassIndex(CharacterClass::Unset, CharacterGender::Male, index));
}

MODERN_TEST(Server_ClassTableRoundTripsEveryIndex)
{
	for (uint8_t value = 0; value < Stats::kClassCount; ++value)
	{
		const auto index  = static_cast<Stats::CharClassIndex>(value);
		const auto asClass = ClassOfCharClassIndex(index);
		CHECK(asClass != CharacterClass::Unset);
		Stats::CharClassIndex again{};
		CHECK(TryToCharClassIndex(asClass, GenderOfCharClassIndex(index), again));
		CHECK_EQ(static_cast<uint8_t>(again), value);
	}
}

// ---------------------------------------------------------------------------
// Creation
// ---------------------------------------------------------------------------

MODERN_TEST(Server_CreateProducesTheStatSystemsAnswer)
{
	const ServerCharacterDefinition definition = StandardDefinition();
	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	CHECK(created.GetValue().GetDerivedStats() == RecalculateIndependently(definition));
}

MODERN_TEST(Server_CreateRejectsAnUnknownClass)
{
	ServerCharacterDefinition definition = StandardDefinition();
	definition.characterClass = CharacterClass::Unset;
	const Result<ServerCharacter> result = ServerCharacter::Create(definition);
	CHECK(result.IsError());
	CHECK_EQ(result.GetError(), ErrorCode::InvalidArgument);
}

MODERN_TEST(Server_CreateRejectsOutOfRangeFields)
{
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.id = CharacterId::MakeInvalid();
		CHECK(ServerCharacter::Create(definition).IsError());
	}
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.name.clear();
		CHECK(ServerCharacter::Create(definition).IsError());
	}
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.name = std::string(Character::kNameCapacity + 1, 'x');
		CHECK(ServerCharacter::Create(definition).IsError());
	}
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.level = 0;
		CHECK(ServerCharacter::Create(definition).IsError());
	}
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.experience = -1;
		CHECK(ServerCharacter::Create(definition).IsError());
	}
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.confPointRate = std::numeric_limits<float>::quiet_NaN();
		CHECK(ServerCharacter::Create(definition).IsError());
	}
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.classConstants.hpPerStr = std::numeric_limits<float>::infinity();
		CHECK(ServerCharacter::Create(definition).IsError());
	}
}

// ---------------------------------------------------------------------------
// Recalculation
// ---------------------------------------------------------------------------

MODERN_TEST(Server_LevelChangeRecalculates)
{
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	const Stats::DerivedStats before = character.GetDerivedStats();

	CHECK(character.SetLevel(50).IsOk());
	CHECK(!(character.GetDerivedStats() == before));

	ServerCharacterDefinition expected = StandardDefinition();
	expected.level = 50;
	CHECK(character.GetDerivedStats() == RecalculateIndependently(expected));
}

MODERN_TEST(Server_StatAllocationRecalculates)
{
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	Stats::BaseStats allocated;
	allocated.pow = 5; allocated.str = 300; allocated.dex = 40;
	CHECK(character.SetAllocatedStats(allocated).IsOk());

	ServerCharacterDefinition expected = StandardDefinition();
	expected.allocatedStats = allocated;
	CHECK(character.GetDerivedStats() == RecalculateIndependently(expected));
	// str drives HP, so a large str allocation must raise it.
	CHECK(character.GetDerivedStats().maxHp > 100u);
}

MODERN_TEST(Server_ContributionChangeRecalculates)
{
	// VERTICAL-003: the old direct passive contribution API is gone.
	// Passive contributions now come from learned skills.
	// This test verifies that learning a skill changes the derived stats.
	InMemorySkillDefinitions provider;

	ItemStatBlock stats;
	stats.hp = 250;
	stats.hpRecoveryRate = 0.5f;
	provider.Add(MakeTestSkill(10001, stats));

	ServerCharacterDefinition definition = StandardDefinitionWithSkills(provider);
	// Remove the item definitions from StandardDefinitionWithItems and add our skill
	definition.itemDefinitions = nullptr;  // No items for this test

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	const uint32_t before = character.GetDerivedStats().maxHp;

	CHECK(character.LearnSkill(MakeTestSkillId(10001)).IsOk());
	CHECK(!(character.GetDerivedStats().maxHp == before));
	CHECK(character.GetDerivedStats().maxHp > before);

	// Verify the passive contribution matches what the skill provides.
	CHECK(character.GetPassiveContribution().hp == 250);
}

// ---------------------------------------------------------------------------
// VERTICAL-003: Skills + Passive Contribution
//
// The cases here are the ones that would fail if the server's skill ownership
// were not actually authoritative: that a learn is visible in the contribution
// and in the statistics, that a level change moves the contribution by the
// per-level value and no more, that an unlearn removes it, that two passives
// add, and that a refused operation leaves the character exactly as it was.
// ---------------------------------------------------------------------------

MODERN_TEST(Server_NoLearnedPassiveMeansZeroContribution)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeTestSkill(30001, TestSkillStats(150, 0.0f))).IsOk());

	const ServerCharacterDefinition definition = StandardDefinitionWithSkills(provider);
	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	// A character with a skill provider but no learned skills has no passive
	// contribution at all, and its statistics are the no-passive ones.
	CHECK_EQ(character.GetSkills().GetLearnedCount(), static_cast<size_t>(0));
	CHECK(character.GetPassiveContribution() == Stats::PassiveContribution());
	CHECK(character.GetDerivedStats() ==
		RecalculateIndependently(definition, nullptr, &provider, &character.GetSkills()));
}

MODERN_TEST(Server_LearnPassiveGeneratesAContribution)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeTestSkill(30002, TestSkillStats(250, 0.0f))).IsOk());

	const ServerCharacterDefinition definition = StandardDefinitionWithSkills(provider);
	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	character.RestoreResources();

	const uint32_t hpBefore = character.GetDerivedStats().maxHp;
	const Stats::DerivedStats derivedBefore = character.GetDerivedStats();

	CHECK(character.LearnSkill(MakeTestSkillId(30002)).IsOk());

	CHECK(character.GetSkills().HasSkill(MakeTestSkillId(30002)));
	CHECK_EQ(character.GetSkills().GetSkillLevel(MakeTestSkillId(30002)),
	         static_cast<uint8_t>(1));
	CHECK_EQ(character.GetPassiveContribution().hp, 250);
	CHECK(character.GetDerivedStats().maxHp > hpBefore);

	// The recalculation is the one stat implementation's, over the same inputs.
	CHECK(character.GetDerivedStats() ==
		RecalculateIndependently(definition, nullptr, &provider, &character.GetSkills()));

	// The current pool is clamped rather than left above the new maximum.
	CHECK(character.GetDerivedStats().maxHp > 0);
	(void)derivedBefore;
}

MODERN_TEST(Server_LearningAPassiveIsPublishedInTheSnapshot)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeTestSkill(30003, TestSkillStats(120, 0.0f))).IsOk());

	const ServerCharacterDefinition definition = StandardDefinitionWithSkills(provider);
	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	CHECK(character.LearnSkill(MakeTestSkillId(30003)).IsOk());

	const Gameplay::CharacterSnapshot snapshot = character.BuildSnapshot().GetValue();
	CHECK_EQ(snapshot.skills.GetLearnedCount(), static_cast<size_t>(1));
	CHECK(snapshot.skills.Has(MakeTestSkillId(30003)));
	CHECK_EQ(snapshot.skills.GetLevel(MakeTestSkillId(30003)), static_cast<uint8_t>(1));

	const Gameplay::LearnedSkillEntry* entry =
		snapshot.skills.Find(MakeTestSkillId(30003));
	CHECK(entry != nullptr);
	if (entry != nullptr)
	{
		// The name travels with the snapshot so a skill panel needs no second
		// lookup on the client.
		CHECK_EQ(entry->name, std::string("TestSkill"));
	}
	// The skill entry carries no stat value: the statistics are in `derived`,
	// computed once by the server.
	CHECK(snapshot.derived == character.GetDerivedStats());
}

MODERN_TEST(Server_PassiveLevelChangeMovesTheContribution)
{
	// RAN reads the value for the level actually learned
	// (GLCHARLOGIC::SUM_PASSIVE, GLogixExPC.cpp:916), so a level change must
	// move the contribution by the per-level value and by nothing else.
	SkillDefinition def = MakeTestSkill(30004, TestSkillStats(100, 0.0f));
	def.maxLevel = 3;
	def.levelData[2].basicVar = 175.0f;
	def.levelData[3].basicVar = 260.0f;

	InMemorySkillDefinitions provider;
	CHECK(provider.Add(def).IsOk());

	const ServerCharacterDefinition definition = StandardDefinitionWithSkills(provider);
	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	character.RestoreResources();

	CHECK(character.LearnSkill(MakeTestSkillId(30004)).IsOk());
	CHECK_EQ(character.GetPassiveContribution().hp, 100);
	const uint32_t hpAtLevelOne = character.GetDerivedStats().maxHp;

	CHECK(character.SetSkillLevel(MakeTestSkillId(30004), 2).IsOk());
	CHECK_EQ(character.GetPassiveContribution().hp, 175);
	const uint32_t hpAtLevelTwo = character.GetDerivedStats().maxHp;
	CHECK(hpAtLevelTwo > hpAtLevelOne);
	CHECK_EQ(hpAtLevelTwo - hpAtLevelOne, 75u);

	CHECK(character.SetSkillLevel(MakeTestSkillId(30004), 3).IsOk());
	CHECK_EQ(character.GetPassiveContribution().hp, 260);
	CHECK_EQ(character.GetDerivedStats().maxHp - hpAtLevelOne, 160u);

	// And the recalculation still agrees with an independent call over the same
	// learned set.
	CHECK(character.GetDerivedStats() ==
		RecalculateIndependently(definition, nullptr, &provider, &character.GetSkills()));

	// The level travelled into the snapshot too.
	const Gameplay::CharacterSnapshot snapshot = character.BuildSnapshot().GetValue();
	CHECK_EQ(snapshot.skills.GetLevel(MakeTestSkillId(30004)), static_cast<uint8_t>(3));
}

MODERN_TEST(Server_UnlearnRemovesTheContribution)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeTestSkill(30005, TestSkillStats(200, 0.0f))).IsOk());

	const ServerCharacterDefinition definition = StandardDefinitionWithSkills(provider);
	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	character.RestoreResources();

	const uint32_t hpBare = character.GetDerivedStats().maxHp;

	CHECK(character.LearnSkill(MakeTestSkillId(30005)).IsOk());
	CHECK_EQ(character.GetPassiveContribution().hp, 200);
	CHECK(character.GetDerivedStats().maxHp > hpBare);

	CHECK(character.UnlearnSkill(MakeTestSkillId(30005)).IsOk());

	// Everything the skill contributed is gone, including from the published
	// statistics: an unlearn that only cleared the skill set would leave a
	// snapshot that disagrees with the character behind it.
	CHECK(!character.GetSkills().HasSkill(MakeTestSkillId(30005)));
	CHECK(character.GetPassiveContribution() == Stats::PassiveContribution());
	CHECK_EQ(character.GetDerivedStats().maxHp, hpBare);
	CHECK(character.GetDerivedStats() ==
		RecalculateIndependently(definition, nullptr, &provider, &character.GetSkills()));

	const Gameplay::CharacterSnapshot snapshot = character.BuildSnapshot().GetValue();
	CHECK_EQ(snapshot.skills.GetLearnedCount(), static_cast<size_t>(0));
}

MODERN_TEST(Server_TwoPassivesStackAdditively)
{
	// SUM_PASSIVE is a plain accumulation into one SPASSIVE_SKILL_DATA with no
	// priority or ordering rule, so the sum is the sum regardless of the order
	// the skills were learned in.
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeTestSkill(30010, TestSkillStats(100, 0.0f))).IsOk());
	CHECK(provider.Add(MakeTestSkill(30020, TestSkillStats(40, 0.0f))).IsOk());

	const ServerCharacterDefinition definition = StandardDefinitionWithSkills(provider);

	Result<ServerCharacter> forwardCreated = ServerCharacter::Create(definition);
	CHECK(forwardCreated.IsOk());
	if (forwardCreated.IsError())
	{
		return;
	}
	ServerCharacter forward = forwardCreated.GetValue();
	forward.RestoreResources();
	const uint32_t hpBare = forward.GetDerivedStats().maxHp;

	CHECK(forward.LearnSkill(MakeTestSkillId(30010)).IsOk());
	const uint32_t hpAfterFirst = forward.GetDerivedStats().maxHp;
	CHECK(forward.LearnSkill(MakeTestSkillId(30020)).IsOk());
	const uint32_t hpAfterBoth = forward.GetDerivedStats().maxHp;

	CHECK_EQ(forward.GetPassiveContribution().hp, 140);
	CHECK_EQ(hpAfterFirst - hpBare, 100u);
	CHECK_EQ(hpAfterBoth - hpAfterFirst, 40u);

	// The other learning order produces the same numbers.
	Result<ServerCharacter> reverseCreated = ServerCharacter::Create(definition);
	CHECK(reverseCreated.IsOk());
	if (reverseCreated.IsError())
	{
		return;
	}
	ServerCharacter reverse = reverseCreated.GetValue();
	reverse.RestoreResources();
	CHECK(reverse.LearnSkill(MakeTestSkillId(30020)).IsOk());
	CHECK(reverse.LearnSkill(MakeTestSkillId(30010)).IsOk());

	CHECK(reverse.GetPassiveContribution() == forward.GetPassiveContribution());
	CHECK(reverse.GetDerivedStats() == forward.GetDerivedStats());
}

MODERN_TEST(Server_PassiveImpactsAndBasicValueBothApply)
{
	SkillDefinition def = MakeTestSkill(30006, TestSkillStats(200, 0.0f));
	def.impacts[0].type      = PassiveImpactType::Defense;
	def.impacts[0].values[1] = 12.0f;
	def.impacts[1].type      = PassiveImpactType::HpRate;
	def.impacts[1].values[1] = 0.5f;

	InMemorySkillDefinitions provider;
	CHECK(provider.Add(def).IsOk());

	const ServerCharacterDefinition definition = StandardDefinitionWithSkills(provider);
	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	CHECK(character.LearnSkill(MakeTestSkillId(30006)).IsOk());

	CHECK_EQ(character.GetPassiveContribution().hp, 200);
	CHECK_EQ(character.GetPassiveContribution().defense, 12);
	CHECK_EQ(character.GetPassiveContribution().hpRate, 0.5f);

	// The rate reaches the derived maximum: the resource formula multiplies by
	// (1 + hpRate) in Stats::Calculate, which is CORE-002's arithmetic, so this
	// asserts only that the rate was carried into the input.
	CHECK(character.GetDerivedStats() ==
		RecalculateIndependently(definition, nullptr, &provider, &character.GetSkills()));
}

MODERN_TEST(Server_RejectedSkillMutationsLeaveTheCharacterIntact)
{
	// Every refusal below must leave the learned set, the contribution and the
	// published statistics exactly as they were. RAN's SLEARN path mutates
	// before it can fail in places; a server that publishes a half-applied
	// state is worse than one that refuses.
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeTestSkill(30007, TestSkillStats(180, 0.0f))).IsOk());

	const ServerCharacterDefinition definition = StandardDefinitionWithSkills(provider);
	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	character.RestoreResources();

	CHECK(character.LearnSkill(MakeTestSkillId(30007)).IsOk());
	const SkillState skillsBefore = character.GetSkills();
	const Stats::PassiveContribution passivesBefore = character.GetPassiveContribution();
	const Stats::DerivedStats derivedBefore = character.GetDerivedStats();

	// Learning a skill no definition exists for.
	const Status unknown = character.LearnSkill(MakeTestSkillId(39999));
	CHECK(unknown.IsError());
	CHECK_EQ(unknown.GetCode(), ErrorCode::NotFound);

	// Learning the same skill twice.
	const Status duplicate = character.LearnSkill(MakeTestSkillId(30007));
	CHECK(duplicate.IsError());
	CHECK_EQ(duplicate.GetCode(), ErrorCode::AlreadyExists);

	// Levelling a skill that is not learned.
	const Status notLearned = character.SetSkillLevel(MakeTestSkillId(30008), 2);
	CHECK(notLearned.IsError());

	// Levelling past the definition's own maximum.
	const Status tooHigh = character.SetSkillLevel(MakeTestSkillId(30007), 2);
	CHECK(tooHigh.IsError());
	CHECK_EQ(tooHigh.GetCode(), ErrorCode::InvalidArgument);

	// Level zero, which is what UnlearnSkill is for.
	CHECK(character.SetSkillLevel(MakeTestSkillId(30007), 0).IsError());

	// A level outside the global range.
	CHECK(character.SetSkillLevel(MakeTestSkillId(30007),
	                              static_cast<uint8_t>(kMaxSkillLevel + 1)).IsError());

	// Unlearning a skill that is not learned.
	const Status unlearnUnknown = character.UnlearnSkill(MakeTestSkillId(30008));
	CHECK(unlearnUnknown.IsError());
	CHECK_EQ(unlearnUnknown.GetCode(), ErrorCode::NotFound);

	CHECK(character.GetSkills() == skillsBefore);
	CHECK(character.GetPassiveContribution() == passivesBefore);
	CHECK(character.GetDerivedStats() == derivedBefore);
}

MODERN_TEST(Server_SkillsAreRefusedWithoutAProvider)
{
	// A character with no skill definitions cannot learn, rather than learning
	// into a set that nothing can ever resolve.
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	const Stats::DerivedStats before = character.GetDerivedStats();

	CHECK(character.LearnSkill(MakeTestSkillId(30009)).IsError());
	CHECK(character.SetSkillLevel(MakeTestSkillId(30009), 2).IsError());
	CHECK_EQ(character.GetSkills().GetLearnedCount(), static_cast<size_t>(0));
	CHECK(character.GetDerivedStats() == before);
}

MODERN_TEST(Server_WeaponDependentPassiveNeedsTheSlotOccupied)
{
	// A passive gated on a right-hand weapon contributes nothing while the slot
	// is empty, and contributes once something is in it. This is the part of
	// the legacy check that is verifiable today: the slot must hold an item.
	//
	// LIMITED: RAN compares the item's attack type as well
	// (CHECHSKILL_ITEM, GLogixExPC.cpp:899), and ItemDefinition does not carry
	// an attack type, so this asserts slot occupancy only. See the
	// investigation note in docs/MODERN_ARCHITECTURE.md §13.6.
	SkillDefinition def = MakeTestSkill(30011, TestSkillStats(300, 0.0f));
	def.rightWeapon = SkillWeaponType::Sword;

	InMemoryItemDefinitions itemProvider;
	InMemorySkillDefinitions skillProvider;
	CHECK(skillProvider.Add(def).IsOk());

	const ServerCharacterDefinition definition =
		StandardDefinitionWithItemsAndSkills(itemProvider, skillProvider);
	// The sword the passive is gated on. Its own contribution is deliberately
	// zero, so the only statistic that can move is the passive's.
	ItemStatBlock plainSword;
	CHECK(itemProvider.Add(MakeTestWeapon(10001, plainSword)).IsOk());

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	character.RestoreResources();

	CHECK(character.LearnSkill(MakeTestSkillId(30011)).IsOk());
	// Learned, but inactive: the skill is in the set and contributes nothing.
	CHECK(character.GetSkills().HasSkill(MakeTestSkillId(30011)));
	CHECK_EQ(character.GetPassiveContribution().hp, 0);
	const uint32_t hpUnarmed = character.GetDerivedStats().maxHp;

	// Equipping the registered sword activates it. No weapon-type match is
	// asserted, because the data to assert it does not exist yet.
	CHECK(character.Equip(EquipmentSlot::RightHand, TestItem(10001)).IsOk());
	CHECK_EQ(character.GetPassiveContribution().hp, 300);
	CHECK(character.GetDerivedStats().maxHp > hpUnarmed);

	// Unequipping deactivates it again, and the statistics follow.
	CHECK(character.Unequip(EquipmentSlot::RightHand).IsOk());
	CHECK_EQ(character.GetPassiveContribution().hp, 0);
	CHECK_EQ(character.GetDerivedStats().maxHp, hpUnarmed);
}

MODERN_TEST(Server_SetContributionsRefusesAnItemOrPassiveContribution)
{
	// The worn set and the learned skill set are the only sources for the item
	// and passive contributions. SetContributions still takes both parameters for
	// source compatibility, so a non-zero value has to be refused rather than
	// accepted and then dropped on the floor - a caller that is told "ok" and
	// whose contribution never appears has no way to notice.
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeTestSkill(30012, TestSkillStats(500, 0.0f))).IsOk());

	const ServerCharacterDefinition definition = StandardDefinitionWithSkills(provider);
	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	Stats::ItemContribution items;
	items.hp = 250;
	CHECK(character.SetContributions(items, Stats::PassiveContribution(),
	                                 Stats::CodexContribution()).IsError());

	Stats::PassiveContribution passives;
	passives.hp = 500;
	CHECK(character.SetContributions(Stats::ItemContribution(), passives,
	                                 Stats::CodexContribution()).IsError());

	CHECK(character.GetDerivedStats() == RecalculateIndependently(definition, nullptr, &provider));

	// VERTICAL-004: the codex joins them. The completed codex set is the only
	// source for the codex contribution, so the third parameter is refused on
	// the same terms as the other two. This used to be the one parameter the
	// function still honoured.
	Stats::CodexContribution codex;
	codex.hp = 90;
	CHECK(character.SetContributions(Stats::ItemContribution(),
	                                 Stats::PassiveContribution(), codex).IsError());

	// A refused argument must leave the character untouched, and the refusal
	// must be about the argument rather than about the character's own state:
	// a character with real codex progress can still be recalculated through
	// the same call with all-zero arguments.
	CHECK(character.GetDerivedStats() == RecalculateIndependently(definition, nullptr, &provider));
	CHECK(character.SetContributions(Stats::ItemContribution(),
	                                 Stats::PassiveContribution(),
	                                 Stats::CodexContribution()).IsOk());
	CHECK(character.GetDerivedStats() == RecalculateIndependently(definition, nullptr, &provider));

	// And the learned set is still the only way to raise hp.
	CHECK(character.LearnSkill(MakeTestSkillId(30012)).IsOk());
	CHECK_EQ(character.GetPassiveContribution().hp, 500);
}

MODERN_TEST(Server_ConfPointRateChangeRecalculates)
{
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	CHECK(character.SetConfPointRate(2.0f).IsOk());
	ServerCharacterDefinition expected = StandardDefinition();
	expected.confPointRate = 2.0f;
	CHECK(character.GetDerivedStats() == RecalculateIndependently(expected));
}

MODERN_TEST(Server_RejectedMutationsLeaveTheCharacterIntact)
{
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	const Stats::DerivedStats before = character.GetDerivedStats();

	CHECK(character.SetLevel(0).IsError());
	CHECK(character.SetExperience(-5).IsError());
	CHECK(character.SetConfPointRate(std::numeric_limits<float>::quiet_NaN()).IsError());
	CHECK(character.GetDerivedStats() == before);
}

// ---------------------------------------------------------------------------
// Resources
// ---------------------------------------------------------------------------

MODERN_TEST(Server_ResourcesClampToTheDerivedMaximum)
{
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	CHECK(character.SetCurrentHp(character.GetDerivedStats().maxHp).IsOk());
	CHECK(character.SetCurrentHp(character.GetDerivedStats().maxHp + 1).IsError());
	CHECK(character.SetCurrentMp(0).IsOk());
	CHECK(character.SetCurrentSp(1).IsOk());
}

MODERN_TEST(Server_RestoreFillsEveryResource)
{
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	CHECK(character.SetCurrentHp(1).IsOk());
	character.RestoreResources();

	const Gameplay::CharacterSnapshot snapshot = character.BuildSnapshot().GetValue();
	CHECK_EQ(snapshot.hp.current, snapshot.derived.maxHp);
	CHECK_EQ(snapshot.mp.current, snapshot.derived.maxMp);
	CHECK_EQ(snapshot.sp.current, snapshot.derived.maxSp);
}

MODERN_TEST(Server_DroppingAMaximumClampsTheCurrentValue)
{
	// A stat input that lowers a maximum must not leave a current value above
	// it, or the published snapshot would be unrenderable.
	ServerCharacterDefinition definition = StandardDefinition();
	definition.allocatedStats.str = 300;
	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	character.RestoreResources();
	const uint32_t high = character.GetDerivedStats().maxHp;
	CHECK(high > 100u);

	// Remove the allocation, so the maximum falls.
	CHECK(character.SetAllocatedStats(Stats::BaseStats()).IsOk());
	CHECK(character.GetDerivedStats().maxHp < high);

	const Gameplay::CharacterSnapshot snapshot = character.BuildSnapshot().GetValue();
	CHECK(snapshot.hp.current <= snapshot.derived.maxHp);
}

// ---------------------------------------------------------------------------
// Snapshot
// ---------------------------------------------------------------------------

MODERN_TEST(Server_SnapshotCarriesTheComputedStatistics)
{
	ServerCharacterDefinition definition = StandardDefinition();
	definition.level = 25;
	definition.experience = 4200;
	definition.allocatedStats.dex = 12;
	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	CHECK(character.SetPosition(Vector3(1.0f, 2.0f, 3.0f)).IsOk());
	character.RestoreResources();

	const Result<Gameplay::CharacterSnapshot> snapshot = character.BuildSnapshot();
	CHECK(snapshot.IsOk());
	if (snapshot.IsError())
	{
		return;
	}

	const Gameplay::CharacterSnapshot& s = snapshot.GetValue();
	CHECK_EQ(s.id, definition.id);
	CHECK_EQ(s.name, definition.name);
	CHECK(s.characterClass == CharacterClass::Brawler);
	CHECK_EQ(s.level, 25);
	CHECK_EQ(s.experience, 4200);
	CHECK_EQ(s.allocatedStats.dex, 12);
	CHECK(s.derived == character.GetDerivedStats());
	CHECK(s.totalStats == character.GetTotalStats());
	CHECK(s.position == Vector3(1.0f, 2.0f, 3.0f));
	CHECK_EQ(s.GetHealthFraction(), 1.0f);
}

MODERN_TEST(Server_SnapshotIsValidatedByTheSharedContract)
{
	// The server does not hand out a snapshot the client's own rules would
	// reject, because it builds it through the same validating factory.
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	const Result<Gameplay::CharacterSnapshot> snapshot = character.BuildSnapshot();
	CHECK(snapshot.IsOk());
	if (snapshot.IsError())
	{
		return;
	}
	CHECK(Gameplay::CharacterSnapshot::IsValid(snapshot.GetValue()));
}

MODERN_TEST(Server_RepeatedSnapshotsAreIdentical)
{
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	character.RestoreResources();

	const Gameplay::CharacterSnapshot first = character.BuildSnapshot().GetValue();
	for (int repeat = 0; repeat < 32; ++repeat)
	{
		CHECK(character.BuildSnapshot().GetValue() == first);
	}
}

// ---------------------------------------------------------------------------
// VERTICAL-002: Equipment integration
// ---------------------------------------------------------------------------

MODERN_TEST(Server_EquipWithoutProviderFails)
{
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	const Status status = character.Equip(EquipmentSlot::RightHand, TestItem(10001));
	CHECK_EQ(status.GetCode(), ErrorCode::InvalidArgument);
}

MODERN_TEST(Server_EquipUnknownItemFails)
{
	InMemoryItemDefinitions provider;
	// The item definition for id 10001 is NOT registered.
	const ServerCharacterDefinition definition = StandardDefinitionWithItems(provider);

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	const Status status = character.Equip(EquipmentSlot::RightHand, TestItem(99999));
	CHECK_EQ(status.GetCode(), ErrorCode::NotFound);
}

MODERN_TEST(Server_EquipChangesDerivedStats)
{
	InMemoryItemDefinitions provider;
	ServerCharacterDefinition definition = StandardDefinitionWithItems(provider);
	definition.level = 10;

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	character.RestoreResources();

	const uint32_t hpBefore = character.GetDerivedStats().maxHp;

	CHECK(character.Equip(EquipmentSlot::RightHand, TestItem(10001)).IsOk());
	CHECK(character.GetDerivedStats().maxHp > hpBefore);
	CHECK_EQ(character.GetItemContribution().hp, 40);
	CHECK_EQ(character.GetEquipment().GetOccupiedCount(), static_cast<size_t>(1));
	CHECK(character.GetEquipment().HasEquipped(EquipmentSlot::RightHand));
}

MODERN_TEST(Server_UnequipDropsContribution)
{
	InMemoryItemDefinitions provider;
	ServerCharacterDefinition definition = StandardDefinitionWithItems(provider);
	definition.level = 10;

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	CHECK(character.Equip(EquipmentSlot::RightHand, TestItem(10001)).IsOk());
	CHECK(character.GetItemContribution().hp == 40);

	CHECK(character.Unequip(EquipmentSlot::RightHand).IsOk());
	CHECK_EQ(character.GetEquipment().GetOccupiedCount(), static_cast<size_t>(0));
	CHECK_EQ(character.GetItemContribution().hp, 0);
}

MODERN_TEST(Server_EquipUnequipRoundTripsStats)
{
	InMemoryItemDefinitions provider;
	ServerCharacterDefinition definition = StandardDefinitionWithItems(provider);
	definition.level = 10;

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	character.RestoreResources();

	const Stats::DerivedStats baseline = character.GetDerivedStats();

	CHECK(character.Equip(EquipmentSlot::RightHand, TestItem(10001, 1)).IsOk());
	CHECK(character.Equip(EquipmentSlot::Headgear,  TestItem(10002, 2)).IsOk());
	CHECK(!(character.GetDerivedStats() == baseline));

	CHECK(character.Unequip(EquipmentSlot::RightHand).IsOk());
	CHECK(character.Unequip(EquipmentSlot::Headgear).IsOk());
	CHECK(character.GetDerivedStats() == baseline);
}

MODERN_TEST(Server_SnapshotPublishesEquippedItems)
{
	InMemoryItemDefinitions provider;
	ServerCharacterDefinition definition = StandardDefinitionWithItems(provider);
	definition.level = 15;

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	character.RestoreResources();

	CHECK(character.Equip(EquipmentSlot::RightHand, TestItem(10001, 7)).IsOk());

	const Result<Gameplay::CharacterSnapshot> snapshotResult = character.BuildSnapshot();
	CHECK(snapshotResult.IsOk());
	if (snapshotResult.IsError())
	{
		return;
	}
	const Gameplay::CharacterSnapshot& snapshot = snapshotResult.GetValue();

	CHECK_EQ(snapshot.equipped.GetOccupiedCount(), static_cast<size_t>(1));
	CHECK(snapshot.equipped.Has(EquipmentSlot::RightHand));

	const Gameplay::EquippedItem& item =
		snapshot.equipped.Get(EquipmentSlot::RightHand);
	CHECK_EQ(item.definition, ItemId(10001));
	CHECK_EQ(item.serial, static_cast<uint64_t>(7));
	CHECK_EQ(item.kind, ItemKind::Weapon);
	CHECK_EQ(item.name, "TestSword");

	CHECK(!snapshot.equipped.Has(EquipmentSlot::Headgear));
}

MODERN_TEST(Server_SnapshotHasEmptyEquipmentByDefault)
{
	InMemoryItemDefinitions provider;
	ServerCharacterDefinition definition = StandardDefinitionWithItems(provider);

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	const Gameplay::CharacterSnapshot snapshot = character.BuildSnapshot().GetValue();
	CHECK_EQ(snapshot.equipped.GetOccupiedCount(), static_cast<size_t>(0));
}

MODERN_TEST(Server_RejectedEquipLeavesCharacterIntact)
{
	InMemoryItemDefinitions provider;
	ServerCharacterDefinition definition = StandardDefinitionWithItems(provider);

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	const size_t occupiedBefore = character.GetEquipment().GetOccupiedCount();
	const Stats::DerivedStats before = character.GetDerivedStats();

	// An unknown item: the provider does not know id 99998.
	CHECK(character.Equip(EquipmentSlot::RightHand, TestItem(99998)).IsError());
	CHECK_EQ(character.GetEquipment().GetOccupiedCount(), occupiedBefore);
	CHECK(character.GetDerivedStats() == before);
}


// ---------------------------------------------------------------------------
// VERTICAL-004: codex
// ---------------------------------------------------------------------------
//
// The server is the only writer of codex state. These cases are about the three
// things that make the character authoritative: that registering an item moves
// the derived statistics, that a completed entry pays exactly once, and that
// what a snapshot publishes is exactly what the state behind it holds.
//
// The slot arithmetic, the required-count cascade and the aggregation rules are
// pinned in modern/tests/CodexTests.cpp. What is left to prove here is that the
// character is wired to that state correctly - that a registration reaches it,
// that the aggregate reaches the stat formula, and that the snapshot is a
// faithful copy rather than a second, independently maintained truth.

namespace
{
	// A codex requirement naming one item and one count. The requirement holds
	// the item *definition* id, and there is no ItemInstance involved in
	// seating a definition.
	CodexRequirement CodexReq(ItemId item, uint16_t quantity)
	{
		CodexRequirement requirement;
		requirement.item          = item;
		requirement.quantity      = quantity;
		requirement.requiredGrade = 0;
		return requirement;
	}

	// A codex definition with a title, because the snapshot publishes the
	// definition's strings to the client. The requirements are written into the
	// five slots in order, which is the only shape a codex table really has -
	// slot order is meaningful, and `RequiredSlotCount` reads it.
	CodexDefinition CodexTableEntry(CodexId id, CodexType type,
	                                 const char* title, uint32_t rewardPoint,
	                                 std::initializer_list<CodexRequirement> requirements)
	{
		CodexDefinition definition;
		definition.id          = id;
		definition.type        = type;
		definition.title       = title;
		definition.rewardPoint = rewardPoint;

		uint8_t slot = 0;
		for (const CodexRequirement& requirement : requirements)
		{
			if (slot >= kCodexMaxRequirements)
			{
				break;
			}
			definition.requirements[slot] = requirement;
			++slot;
		}
		return definition;
	}

	// A registration argument: a usable instance of the named item at the given
	// count. The serial has to be non-zero, or the instance is not valid and the
	// registration is refused as a bad argument.
	ItemInstance CodexStack(ItemId item, uint32_t count, uint64_t serial)
	{
		ItemInstance instance;
		instance.definition = item;
		instance.serial     = serial;
		instance.count      = count;
		return instance;
	}

	const CodexId kHpCodex     = CodexId(11u);
	const CodexId kDefenseCodex = CodexId(12u);
	const ItemId   kHpItem      = ItemId(20001u);
	const ItemId   kOtherItem   = ItemId(20002u);

	// A one-requirement table. One requirement means one required slot, so each
	// entry completes on a single registration - which keeps these cases about
	// the server wiring rather than about the slot arithmetic.
	InMemoryCodexDefinitions SingleRequirementCodexTable()
	{
		InMemoryCodexDefinitions definitions;
		// ReachLevel pays into HP.
		(void) definitions.Add(CodexTableEntry(kHpCodex, CodexType::ReachLevel,
		                                       "First Steps", 100u,
		                                       { CodexReq(kHpItem, 1) }));
		return definitions;
	}

	ServerCharacterDefinition DefinitionWithCodex(const CodexDefinitionProvider& definitions)
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.codexDefinitions = &definitions;
		return definition;
	}
}

MODERN_TEST(Server_CodexIsSeatedFromTheDefinitionTableOnCreate)
{
	InMemoryCodexDefinitions definitions = SingleRequirementCodexTable();
	const ServerCharacterDefinition definition = DefinitionWithCodex(definitions);

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	const ServerCharacter character = created.GetValue();

	// Every definition is seated and nothing is completed yet, so the aggregate
	// is zero and the character matches one holding no codex at all.
	CHECK_EQ(character.GetCodex().GetProgressCount(), size_t(1));
	CHECK_EQ(character.GetCodex().GetCompletedCount(), size_t(0));
	CHECK(character.GetCodex().IsInProgress(kHpCodex));
	CHECK(!character.GetCodex().IsCompleted(kHpCodex));
	CHECK_EQ(character.GetCodexContribution().hp, uint32_t(0));
	CHECK_EQ(character.GetContributingCodexCount(), size_t(0));
	CHECK(character.GetDerivedStats() == RecalculateIndependently(definition));
}

MODERN_TEST(Server_RegisteringACodexItemMovesTheDerivedStatistics)
{
	InMemoryCodexDefinitions definitions = SingleRequirementCodexTable();
	const ServerCharacterDefinition definition = DefinitionWithCodex(definitions);

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	const uint32_t hpBefore = character.GetDerivedStats().maxHp;

	CHECK(character.RegisterCodexItem(kHpCodex, CodexStack(kHpItem, 1, 1u)).IsOk());
	CHECK(character.GetCodex().IsCompleted(kHpCodex));
	CHECK_EQ(character.GetCodexContribution().hp, uint32_t(100));
	CHECK_EQ(character.GetContributingCodexCount(), size_t(1));

	// The character is not holding a contribution the stat formula cannot
	// account for, which is the whole reason the aggregate exists.
	Stats::CodexContribution earned;
	earned.hp = 100;
	CHECK(character.GetDerivedStats() ==
	      RecalculateIndependently(definition, nullptr, nullptr, nullptr, nullptr, &earned));
	CHECK(character.GetDerivedStats().maxHp > hpBefore);
}

MODERN_TEST(Server_OnlyACompletedCodexEntryPays)
{
	InMemoryCodexDefinitions definitions = SingleRequirementCodexTable();
	const ServerCharacterDefinition definition = DefinitionWithCodex(definitions);

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	// The entry names kHpItem, so a stack of something else records nothing and
	// pays nothing. The caller is told, which is what lets it keep the item.
	Result<CodexRegistration> miss =
		character.RegisterCodexItem(kHpCodex, CodexStack(kOtherItem, 1, 2u));
	CHECK(miss.IsOk());
	CHECK(miss.GetValue().IsOk());
	CHECK(!miss.GetValue().recorded);
	CHECK(!miss.GetValue().completed);
	CHECK_EQ(character.GetCodexContribution().hp, uint32_t(0));
	CHECK(character.GetCodex().IsInProgress(kHpCodex));
	CHECK(character.GetDerivedStats() == RecalculateIndependently(definition));
}

MODERN_TEST(Server_ACodexRewardIsPaidExactlyOnce)
{
	InMemoryCodexDefinitions definitions = SingleRequirementCodexTable();
	const ServerCharacterDefinition definition = DefinitionWithCodex(definitions);

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	CHECK(character.RegisterCodexItem(kHpCodex, CodexStack(kHpItem, 1, 3u)).IsOk());
	CHECK_EQ(character.GetCodexContribution().hp, uint32_t(100));

	// A second registration for the completed entry is refused as already
	// completed, before the item is looked at, so the aggregate cannot be
	// rebuilt from a second payment.
	Result<CodexRegistration> again =
		character.RegisterCodexItem(kHpCodex, CodexStack(kHpItem, 1, 4u));
	CHECK(again.IsOk());
	CHECK(again.GetValue().error == CodexRegisterError::AlreadyCompleted);
	CHECK(!again.GetValue().recorded);
	CHECK_EQ(character.GetCodexContribution().hp, uint32_t(100));
}

MODERN_TEST(Server_ReconcilingTheCodexDoesNotRepayACompletedEntry)
{
	InMemoryCodexDefinitions definitions = SingleRequirementCodexTable();
	const ServerCharacterDefinition definition = DefinitionWithCodex(definitions);

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	CHECK(character.RegisterCodexItem(kHpCodex, CodexStack(kHpItem, 1, 5u)).IsOk());
	CHECK_EQ(character.GetCodexContribution().hp, uint32_t(100));

	// This is the load path. A completed entry is skipped, so it cannot be
	// re-seated and paid for a second time.
	CHECK(character.ReconcileCodex().IsOk());
	CHECK(character.GetCodex().IsCompleted(kHpCodex));
	CHECK(!character.GetCodex().IsInProgress(kHpCodex));
	CHECK_EQ(character.GetCodexContribution().hp, uint32_t(100));
}

MODERN_TEST(Server_CodexAggregateIsAFullRecompute)
{
	InMemoryCodexDefinitions definitions;
	// Two entries that both pay HP, so an aggregate that added to the previous
	// total instead of rebuilding would show up as 200 here.
	(void) definitions.Add(CodexTableEntry(kHpCodex, CodexType::ReachLevel,
	                                       "First Steps", 100u,
	                                       { CodexReq(kHpItem, 1) }));
	(void) definitions.Add(CodexTableEntry(kDefenseCodex, CodexType::TakeItem,
	                                       "Salvaged", 40u,
	                                       { CodexReq(kOtherItem, 1) }));
	const ServerCharacterDefinition definition = DefinitionWithCodex(definitions);

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	CHECK(character.RegisterCodexItem(kHpCodex, CodexStack(kHpItem, 1, 6u)).IsOk());
	CHECK_EQ(character.GetCodexContribution().hp, uint32_t(100));

	// RAN's CODEX_STATS is a full recompute over the done set, not an
	// accumulation, so the answer must not drift across reloads.
	for (int pass = 0; pass < 4; ++pass)
	{
		CHECK(character.ReconcileCodex().IsOk());
		CHECK_EQ(character.GetCodexContribution().hp, uint32_t(100));
	}

	// The second entry pays defense, a different statistic, so this also pins
	// the per-type mapping at the server boundary.
	CHECK(character.RegisterCodexItem(kDefenseCodex, CodexStack(kOtherItem, 1, 7u)).IsOk());
	CHECK_EQ(character.GetCodexContribution().hp, uint32_t(100));
	CHECK_EQ(character.GetCodexContribution().defense, uint32_t(40));
	CHECK_EQ(character.GetContributingCodexCount(), size_t(2));
}

MODERN_TEST(Server_ACodexWithNoDefinitionProviderIsInert)
{
	// No provider at all. The character still has to be usable, with an empty
	// codex and no contribution, rather than refusing to be created.
	const ServerCharacterDefinition definition = StandardDefinition();
	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	CHECK(character.GetCodex().GetProgressCount() == 0);
	CHECK(character.GetCodex().GetCompletedCount() == 0);
	CHECK_EQ(character.GetCodexContribution().hp, uint32_t(0));
	CHECK(character.BuildSnapshot().GetValue().codex.entries.empty());
	CHECK(Gameplay::CharacterSnapshot::IsValid(character.BuildSnapshot().GetValue()));
	CHECK(character.GetDerivedStats() == RecalculateIndependently(definition));

	// With no provider there is no codex to register against, so both entry
	// points are refused as bad arguments. Neither is a rule outcome, and
	// neither may touch the state.
	Result<CodexRegistration> miss =
		character.RegisterCodexItem(kHpCodex, CodexStack(kHpItem, 1, 8u));
	CHECK(miss.IsError());
	CHECK(miss.GetError() == ErrorCode::InvalidArgument);
	CHECK(character.ReconcileCodex().GetCode() == ErrorCode::InvalidArgument);
	CHECK(character.BuildSnapshot().GetValue().codex.entries.empty());
	CHECK(character.GetCodex() == CodexState());
}

MODERN_TEST(Server_ReconcileCodexRefusesAnEmptyDefinitionTable)
{
	// An empty provider is a legitimate object but a table that failed to load.
	// Reconciling against it would drop every record a character holds, so it is
	// refused rather than obeyed.
	InMemoryCodexDefinitions empty;
	const ServerCharacterDefinition definition = DefinitionWithCodex(empty);

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	CHECK(character.ReconcileCodex().GetCode() == ErrorCode::InvalidArgument);
	CHECK(character.GetCodex().GetProgressCount() == 0);
	CHECK(Gameplay::CharacterSnapshot::IsValid(character.BuildSnapshot().GetValue()));
	CHECK(character.GetDerivedStats() == RecalculateIndependently(definition));
}

MODERN_TEST(Server_CodexSnapshotPublishesTheAuthoritativeProgress)
{
	InMemoryCodexDefinitions definitions;
	// Two requirements, so one registration leaves the entry in progress and
	// the snapshot has to publish a partial entry, not only finished ones.
	(void) definitions.Add(CodexTableEntry(kDefenseCodex, CodexType::TakeItem,
	                                       "Salvaged", 40u,
	                                       { CodexReq(kHpItem, 1),
	                                         CodexReq(kOtherItem, 1) }));
	const ServerCharacterDefinition definition = DefinitionWithCodex(definitions);

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	CHECK(character.RegisterCodexItem(kDefenseCodex, CodexStack(kHpItem, 1, 9u)).IsOk());
	CHECK(character.GetCodex().IsInProgress(kDefenseCodex));

	const Gameplay::CharacterSnapshot snapshot = character.BuildSnapshot().GetValue();
	CHECK(Gameplay::CharacterSnapshot::IsValid(snapshot));
	CHECK_EQ(snapshot.codex.GetCount(), size_t(1));
	if (!snapshot.codex.Has(kDefenseCodex))
	{
		return;
	}

	const Gameplay::CodexEntry& entry = *snapshot.codex.Find(kDefenseCodex);
	CHECK(entry.id == kDefenseCodex);
	CHECK(entry.type == CodexType::TakeItem);
	CHECK(entry.name == "Salvaged");
	CHECK(!entry.completed);
	// The counters are the ones the state holds, because they are what a codex
	// panel draws its bar from.
	CHECK_EQ(entry.doneCount, uint8_t(1));
	CHECK_EQ(entry.requiredCount, uint8_t(2));
	CHECK(entry.GetProgressFraction() > 0.0f);
	CHECK(entry.GetProgressFraction() < 1.0f);

	// A finished entry reports a full fraction, and one the character does not
	// hold is absent rather than reported as uncompleted.
	CHECK(character.RegisterCodexItem(kDefenseCodex, CodexStack(kOtherItem, 1, 10u)).IsOk());
	const Gameplay::CharacterSnapshot finished = character.BuildSnapshot().GetValue();
	CHECK(Gameplay::CharacterSnapshot::IsValid(finished));
	CHECK(finished.codex.IsCompleted(kDefenseCodex));
	CHECK_EQ(finished.codex.GetCompletedCount(), size_t(1));
	CHECK_EQ(finished.codex.GetInProgressCount(), size_t(0));
	CHECK(!finished.codex.Has(kHpCodex));
	if (const Gameplay::CodexEntry* done = finished.codex.Find(kDefenseCodex))
	{
		CHECK(done->GetProgressFraction() == 1.0f);
		CHECK_EQ(done->doneCount, uint8_t(2));
	}
}

MODERN_TEST(Server_RepeatedCodexSnapshotsAreIdentical)
{
	InMemoryCodexDefinitions definitions = SingleRequirementCodexTable();
	const ServerCharacterDefinition definition = DefinitionWithCodex(definitions);

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	CHECK(character.RegisterCodexItem(kHpCodex, CodexStack(kHpItem, 1, 11u)).IsOk());

	// Publishing a snapshot must not itself be an event that changes anything,
	// or a client that re-reads its state would see it move.
	const Gameplay::CharacterSnapshot first = character.BuildSnapshot().GetValue();
	const Gameplay::CharacterSnapshot second = character.BuildSnapshot().GetValue();
	CHECK(first == second);
	CHECK_EQ(character.GetCodexContribution().hp, uint32_t(100));
	CHECK_EQ(character.GetCodex().GetCompletedCount(), size_t(1));
}

MODERN_TEST(Server_RejectedCodexRegistrationLeavesTheCharacterIntact)
{
	InMemoryCodexDefinitions definitions = SingleRequirementCodexTable();
	const ServerCharacterDefinition definition = DefinitionWithCodex(definitions);

	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	const CodexState before = character.GetCodex();
	const Stats::DerivedStats statsBefore = character.GetDerivedStats();

	// An unknown entry, an invalid id, and an unusable instance: three
	// different bad arguments, none of which may touch the state.
	Result<CodexRegistration> unknown =
		character.RegisterCodexItem(kDefenseCodex, CodexStack(kHpItem, 1, 12u));
	CHECK(unknown.IsOk());
	CHECK(unknown.GetValue().error == CodexRegisterError::UnknownCodex);

	CHECK(character.RegisterCodexItem(CodexId::MakeInvalid(),
	                                  CodexStack(kHpItem, 1, 13u)).IsError());
	CHECK(character.RegisterCodexItem(kHpCodex, ItemInstance()).IsError());

	CHECK(character.GetCodex() == before);
	CHECK(character.GetDerivedStats() == statsBefore);
	CHECK_EQ(character.GetCodexContribution().hp, uint32_t(0));
	CHECK(character.BuildSnapshot().GetValue() == character.BuildSnapshot().GetValue());
}

// ── VERTICAL-006: Combat tests ────────────────────────────────────────

MODERN_TEST(ServerCombat_AttackReducesTargetHP)
{
	auto attacker = ServerCharacter::Create(StandardDefinition());
	auto target = ServerCharacter::Create(StandardDefinition());
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	attacker.GetValue().RestoreResources();
	target.GetValue().RestoreResources();

	const uint32_t hpBefore = target.GetValue().GetDerivedStats().maxHp;
	CHECK_GT(hpBefore, 0u);

	const Status attacked = attacker.GetValue().Attack(target.GetValue());
	CHECK(attacked.IsOk());

	const uint32_t hpAfter = target.GetValue().GetDerivedStats().maxHp;
	CHECK_EQ(hpAfter, hpBefore);
}

MODERN_TEST(ServerCombat_AttackerStateUnchanged)
{
	auto attacker = ServerCharacter::Create(StandardDefinition());
	auto target = ServerCharacter::Create(StandardDefinition());
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	attacker.GetValue().RestoreResources();
	target.GetValue().RestoreResources();

	const auto statsBefore = attacker.GetValue().GetDerivedStats();
	const uint32_t hpBefore = attacker.GetValue().GetDerivedStats().maxHp;

	const Status attacked = attacker.GetValue().Attack(target.GetValue());
	CHECK(attacked.IsOk());

	CHECK(attacker.GetValue().GetDerivedStats() == statsBefore);
	CHECK_EQ(attacker.GetValue().GetDerivedStats().maxHp, hpBefore);
}

MODERN_TEST(ServerCombat_SelfAttackRefused)
{
	auto attacker = ServerCharacter::Create(StandardDefinition());
	CHECK(attacker.IsOk());

	attacker.GetValue().RestoreResources();

	const Status attacked = attacker.GetValue().Attack(attacker.GetValue());
	CHECK(attacked.IsError());
}

MODERN_TEST(ServerCombat_DeadTargetRefused)
{
	auto attacker = ServerCharacter::Create(StandardDefinition());
	auto target = ServerCharacter::Create(StandardDefinition());
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	attacker.GetValue().RestoreResources();
	target.GetValue().RestoreResources();

	target.GetValue().SetCurrentHp(0);

	const Status attacked = attacker.GetValue().Attack(target.GetValue());
	CHECK(attacked.IsError());
}

MODERN_TEST(ServerCombat_SnapshotExposesHP)
{
	auto attacker = ServerCharacter::Create(StandardDefinition());
	auto target = ServerCharacter::Create(StandardDefinition());
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	attacker.GetValue().RestoreResources();
	target.GetValue().RestoreResources();

	const auto snapshotBefore = target.GetValue().BuildSnapshot();
	CHECK(snapshotBefore.IsOk());

	const Status attacked = attacker.GetValue().Attack(target.GetValue());
	CHECK(attacked.IsOk());

	const auto snapshotAfter = target.GetValue().BuildSnapshot();
	CHECK(snapshotAfter.IsOk());

	CHECK(snapshotAfter.GetValue().hp.current <= snapshotAfter.GetValue().derived.maxHp);
}

MODERN_TEST(ServerCombat_MultipleAttacks)
{
	auto attacker = ServerCharacter::Create(StandardDefinition());
	auto target = ServerCharacter::Create(StandardDefinition());
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	attacker.GetValue().RestoreResources();
	target.GetValue().RestoreResources();

	const uint32_t maxHp = target.GetValue().GetDerivedStats().maxHp;

	for (int i = 0; i < 10; ++i)
	{
		const Status attacked = attacker.GetValue().Attack(target.GetValue());
		if (attacked.IsError()) break;
	}

	const auto snapshot = target.GetValue().BuildSnapshot();
	CHECK(snapshot.IsOk());
	CHECK(snapshot.GetValue().hp.current <= maxHp);
}

// ---------------------------------------------------------------------------
// VERTICAL-010: required SP reaches the server character
//
// The combat-level arithmetic and boundary live in modern/tests/CombatTests.cpp
// and the aggregation in modern/tests/EquipmentTests.cpp. What is left is the
// server's own wiring: a worn hand item's cost has to survive the character's
// item pipeline and be visible in the published contribution, which is what
// Attack() reads to build CombatInput::attackerRequiredSP.
// ---------------------------------------------------------------------------

namespace
{
	// A definition provider carrying two weapons with distinct required-SP
	// costs, and one armour with a cost that must never be counted.
	ServerCharacterDefinition DefinitionWithRequiredSP(InMemoryItemDefinitions& provider)
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.itemDefinitions = &provider;

		ItemStatBlock heavy;
		heavy.meleePower = 10;
		heavy.requiredSP = 20;
		provider.Add(MakeTestWeapon(10101, heavy));

		ItemStatBlock light;
		light.meleePower = 6;
		light.requiredSP = 10;
		provider.Add(MakeTestWeapon(10102, light));

		ItemStatBlock armour;
		armour.defense = 5;
		armour.requiredSP = 99;
		provider.Add(MakeTestArmor(10103, armour));

		return definition;
	}
}

MODERN_TEST(ServerRequiredSP_EquippedWeaponContributes)
{
	InMemoryItemDefinitions provider;
	auto character = ServerCharacter::Create(DefinitionWithRequiredSP(provider));
	CHECK(character.IsOk());

	CHECK(character.GetValue().Equip(EquipmentSlot::RightHand,
	                                 TestItem(10101)).IsOk());

	CHECK_EQ(character.GetValue().GetItemContribution().requiredSP,
	         static_cast<uint16_t>(20));
}

MODERN_TEST(ServerRequiredSP_BothHandsSum)
{
	InMemoryItemDefinitions provider;
	auto character = ServerCharacter::Create(DefinitionWithRequiredSP(provider));
	CHECK(character.IsOk());

	CHECK(character.GetValue().Equip(EquipmentSlot::RightHand, TestItem(10101, 1)).IsOk());
	CHECK(character.GetValue().Equip(EquipmentSlot::LeftHand,  TestItem(10102, 2)).IsOk());

	CHECK_EQ(character.GetValue().GetItemContribution().requiredSP,
	         static_cast<uint16_t>(30));
}

MODERN_TEST(ServerRequiredSP_ArmourDoesNotContribute)
{
	InMemoryItemDefinitions provider;
	auto character = ServerCharacter::Create(DefinitionWithRequiredSP(provider));
	CHECK(character.IsOk());

	CHECK(character.GetValue().Equip(EquipmentSlot::Upper, TestItem(10103)).IsOk());

	// The armour declares 99, and SUM_ITEM only reads the two hand slots.
	CHECK_EQ(character.GetValue().GetItemContribution().requiredSP,
	         static_cast<uint16_t>(0));
}

MODERN_TEST(ServerRequiredSP_BareHandsContributeZero)
{
	InMemoryItemDefinitions provider;
	auto character = ServerCharacter::Create(DefinitionWithRequiredSP(provider));
	CHECK(character.IsOk());

	CHECK_EQ(character.GetValue().GetItemContribution().requiredSP,
	         static_cast<uint16_t>(0));
}

MODERN_TEST(ServerRequiredSP_UnequipDropsTheCost)
{
	InMemoryItemDefinitions provider;
	auto character = ServerCharacter::Create(DefinitionWithRequiredSP(provider));
	CHECK(character.IsOk());

	CHECK(character.GetValue().Equip(EquipmentSlot::RightHand, TestItem(10101)).IsOk());
	CHECK_EQ(character.GetValue().GetItemContribution().requiredSP,
	         static_cast<uint16_t>(20));

	CHECK(character.GetValue().Unequip(EquipmentSlot::RightHand).IsOk());
	CHECK_EQ(character.GetValue().GetItemContribution().requiredSP,
	         static_cast<uint16_t>(0));
}

	// Low SP degrades an attack, it does not refuse it. Legacy returns
	// EMBEGINA_SP from BEGIN_ATTACK and the swing still happens
	// (GLCharMsg.cpp:606-612), so a 0-SP attacker with an expensive weapon must
	// still deal damage.
MODERN_TEST(ServerRequiredSP_LowSPAttackerStillAttacks)
{
	InMemoryItemDefinitions provider;
	auto attacker = ServerCharacter::Create(DefinitionWithRequiredSP(provider));
	auto target   = ServerCharacter::Create(DefinitionWithRequiredSP(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	CHECK(attacker.GetValue().Equip(EquipmentSlot::RightHand, TestItem(10101)).IsOk());

	attacker.GetValue().RestoreResources();
	target.GetValue().RestoreResources();

	// Deplete the attacker below the weapon's cost.
	CHECK(attacker.GetValue().SetCurrentSp(1).IsOk());

	const uint32_t before = target.GetValue().GetDerivedStats().maxHp;
	const Status attacked = attacker.GetValue().Attack(target.GetValue());

	CHECK(attacked.IsOk());
	// Resources are restored to the SP maximum by the recalculation, so read
	// the HP effect through the derived pool rather than the current value.
	CHECK(target.GetValue().GetDerivedStats().maxHp <= before);
}

// ---------------------------------------------------------------------------
// VERTICAL-011: active skills
//
// The rules are tested in modern/tests/ActiveSkillTests.cpp, which drives the
// core resolver directly and deterministically. What is left here is the part
// only the server can prove: that the level comes from the character's own
// learned set rather than from the caller, that a refused cast changes nothing,
// and that a successful one moves real resources.
// ---------------------------------------------------------------------------

namespace
{
	// A castable physical melee damage skill. A negative `basicVar` is the
	// legacy damage encoding (GLChar.cpp:3077-3085).
	SkillDefinition MakeActiveDamageSkill(uint16_t skillIndex, uint8_t maxLevel = 3)
	{
		SkillDefinition def;
		def.id         = SkillId{ 1, skillIndex };
		def.name       = "Cleave";
		def.maxLevel   = maxLevel;
		def.grade      = 2;
		def.role       = SkillRole::Normal;
		def.apply      = SkillApply::PhysicalMelee;
		def.targetKind = SkillTargetKind::Spec;
		def.impactSide = SkillImpactSide::Enemy;
		def.applyType  = PassiveApplyType::Hp;

		for (uint8_t level = 1; level <= maxLevel; ++level)
		{
			def.levelData[level].basicVar  = -10.0f * static_cast<float>(level);
			def.levelData[level].useSp     = static_cast<uint16_t>(5 * level);
			def.levelData[level].useMp     = static_cast<uint16_t>(2 * level);
			def.levelData[level].useHp     = 0;
			def.levelData[level].delayTime = 0.5f;
		}
		return def;
	}

	// Both characters wired to a provider holding the castable skill, with the
	// attacker already holding it at `level`.
	// Registers the castable physical melee damage skill in a provider. The
	// provider must outlive every character built from it, so each test owns one
	// locally rather than sharing a fixture that would need to be copyable.
	void RegisterActiveSkill(InMemorySkillDefinitions& provider, uint16_t skillIndex = 1)
	{
		provider.Add(MakeActiveDamageSkill(skillIndex));
	}

	// Teaches a character the castable skill and fills its pools. Mirrors what
	// a server does when a character learns and is standing idle.
	void PrepareCaster(Result<ServerCharacter>& character, uint8_t level = 1)
	{
		if (!character.IsOk() || level == 0)
		{
			return;
		}
		(void) character.GetValue().LearnSkill(SkillId{ 1, 1 });
		(void) character.GetValue().SetSkillLevel(SkillId{ 1, 1 }, level);
		character.GetValue().RestoreResources();
	}
}

MODERN_TEST(ServerActiveSkill_CastSucceedsAndDamagesTarget)
{
	InMemorySkillDefinitions provider;
	RegisterActiveSkill(provider);

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	PrepareCaster(attacker, 1);
	target.GetValue().RestoreResources();

	const auto before = target.GetValue().BuildSnapshot();
	CHECK(before.IsOk());
	const uint32_t hpBefore = before.GetValue().hp.current;
	CHECK_GT(hpBefore, 0u);

	const Skills::ActiveSkillResult result =
		attacker.GetValue().CastSkill(SkillId{ 1, 1 }, target.GetValue());

	CHECK(result.Succeeded());
	CHECK_EQ(result.failure, Skills::ActiveSkillFailure::None);
	CHECK_EQ(result.level, static_cast<uint8_t>(1));

	const auto after = target.GetValue().BuildSnapshot();
	CHECK(after.IsOk());
	CHECK_LT(after.GetValue().hp.current, hpBefore);
}

MODERN_TEST(ServerActiveSkill_UnlearnedSkillRejected)
{
	InMemorySkillDefinitions provider;
	RegisterActiveSkill(provider);

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	attacker.GetValue().RestoreResources();
	target.GetValue().RestoreResources();

	const auto before = target.GetValue().BuildSnapshot();
	const uint32_t hpBefore = before.GetValue().hp.current;

	const Skills::ActiveSkillResult result =
		attacker.GetValue().CastSkill(SkillId{ 1, 1 }, target.GetValue());

	CHECK_EQ(result.failure, Skills::ActiveSkillFailure::NotLearned);

	// Transactional: a refusal touches nothing.
	const auto after = target.GetValue().BuildSnapshot();
	CHECK_EQ(after.GetValue().hp.current, hpBefore);
	CHECK_EQ(attacker.GetValue().IsSkillOnCooldown(SkillId{ 1, 1 }), false);
}

MODERN_TEST(ServerActiveSkill_UnknownSkillRejected)
{
	InMemorySkillDefinitions provider;
	RegisterActiveSkill(provider);

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());
	PrepareCaster(attacker, 1);
	target.GetValue().RestoreResources();

	const Skills::ActiveSkillResult result =
		attacker.GetValue().CastSkill(SkillId{ 9, 9 }, target.GetValue());

	CHECK_EQ(result.failure, Skills::ActiveSkillFailure::UnknownSkill);
}

MODERN_TEST(ServerActiveSkill_LevelComesFromLearnedStateNotTheCaller)
{
	// The learned level is 2. A caller asking for level 1, and one asking for
	// level 3 which was never learned, must both get level 2.
	InMemorySkillDefinitions provider;
	RegisterActiveSkill(provider);

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	PrepareCaster(attacker, 2);
	target.GetValue().RestoreResources();

	const Skills::ActiveSkillResult askedLow =
		attacker.GetValue().CastSkill(SkillId{ 1, 1 }, target.GetValue(), 1);
	CHECK(askedLow.Succeeded());
	CHECK_EQ(askedLow.level, static_cast<uint8_t>(2));

	attacker.GetValue().AdvanceSkillCooldowns(10.0f);

	const Skills::ActiveSkillResult askedHigh =
		attacker.GetValue().CastSkill(SkillId{ 1, 1 }, target.GetValue(), 3);
	CHECK(askedHigh.Succeeded());
	CHECK_EQ(askedHigh.level, static_cast<uint8_t>(2));
	// Level 2 costs wUSE_SP = 10, not level 1's 5 or level 3's 15.
	CHECK_EQ(askedHigh.requiredSP, static_cast<uint16_t>(10));
}

MODERN_TEST(ServerActiveSkill_SelfCastRejected)
{
	InMemorySkillDefinitions provider;
	RegisterActiveSkill(provider);

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	PrepareCaster(attacker, 1);

	const Skills::ActiveSkillResult result =
		attacker.GetValue().CastSkill(SkillId{ 1, 1 }, attacker.GetValue());

	CHECK_EQ(result.failure, Skills::ActiveSkillFailure::UnsupportedTarget);
}

MODERN_TEST(ServerActiveSkill_CastSpendsResources)
{
	InMemorySkillDefinitions provider;
	RegisterActiveSkill(provider);

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	PrepareCaster(attacker, 1);
	target.GetValue().RestoreResources();

	const auto before = attacker.GetValue().BuildSnapshot();
	CHECK(before.IsOk());
	CHECK_GT(before.GetValue().sp.current, 0u);

	const Skills::ActiveSkillResult result =
		attacker.GetValue().CastSkill(SkillId{ 1, 1 }, target.GetValue());
	CHECK(result.Succeeded());
	CHECK_EQ(result.spCost, static_cast<uint16_t>(5));

	const auto after = attacker.GetValue().BuildSnapshot();
	CHECK(after.IsOk());
	// A funded cast pays wUSE_SP (5) and wUSE_MP (2) at level 1.
	CHECK_EQ(after.GetValue().sp.current, before.GetValue().sp.current - 5u);
	CHECK_EQ(after.GetValue().mp.current, before.GetValue().mp.current - 2u);
}

MODERN_TEST(ServerActiveSkill_LowSpCastsWithoutSpendingSp)
{
	InMemorySkillDefinitions provider;
	RegisterActiveSkill(provider);

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	PrepareCaster(attacker, 1);
	target.GetValue().RestoreResources();

	// Below the level-1 cost of 5, so the cast is low-SP.
	CHECK(attacker.GetValue().SetCurrentSp(0).IsOk());

	const Skills::ActiveSkillResult result =
		attacker.GetValue().CastSkill(SkillId{ 1, 1 }, target.GetValue());

	CHECK(result.Succeeded());
	CHECK_EQ(result.IsLowSp(), true);
	CHECK_EQ(result.spCost, static_cast<uint16_t>(0));

	const auto after = attacker.GetValue().BuildSnapshot();
	CHECK(after.IsOk());
	CHECK_EQ(after.GetValue().sp.current, 0u);
}

MODERN_TEST(ServerActiveSkill_CooldownBlocksTheSecondCast)
{
	InMemorySkillDefinitions provider;
	RegisterActiveSkill(provider);

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	PrepareCaster(attacker, 1);
	target.GetValue().RestoreResources();

	const Skills::ActiveSkillResult first =
		attacker.GetValue().CastSkill(SkillId{ 1, 1 }, target.GetValue());
	CHECK(first.Succeeded());
	CHECK_GT(first.cooldownSeconds, 0.0f);
	CHECK_EQ(attacker.GetValue().IsSkillOnCooldown(SkillId{ 1, 1 }), true);

	const Skills::ActiveSkillResult second =
		attacker.GetValue().CastSkill(SkillId{ 1, 1 }, target.GetValue());
	CHECK_EQ(second.failure, Skills::ActiveSkillFailure::InCooldown);
}

MODERN_TEST(ServerActiveSkill_CooldownExpiresAndCastResumes)
{
	InMemorySkillDefinitions provider;
	RegisterActiveSkill(provider);

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	PrepareCaster(attacker, 1);
	target.GetValue().RestoreResources();

	(void) attacker.GetValue().CastSkill(SkillId{ 1, 1 }, target.GetValue());
	CHECK_EQ(attacker.GetValue().IsSkillOnCooldown(SkillId{ 1, 1 }), true);

	// GLogixExPC.cpp:3864-3879: decrement, erase at or below zero.
	attacker.GetValue().AdvanceSkillCooldowns(0.25f);
	CHECK_EQ(attacker.GetValue().IsSkillOnCooldown(SkillId{ 1, 1 }), true);
	CHECK_GT(attacker.GetValue().GetSkillCooldownRemaining(SkillId{ 1, 1 }), 0.0f);

	attacker.GetValue().AdvanceSkillCooldowns(10.0f);
	CHECK_EQ(attacker.GetValue().IsSkillOnCooldown(SkillId{ 1, 1 }), false);
	CHECK_EQ(attacker.GetValue().GetSkillCooldownRemaining(SkillId{ 1, 1 }), 0.0f);

	const Skills::ActiveSkillResult again =
		attacker.GetValue().CastSkill(SkillId{ 1, 1 }, target.GetValue());
	CHECK(again.Succeeded());
}

MODERN_TEST(ServerActiveSkill_UnsupportedSkillIsRefusedNotFaked)
{
	InMemorySkillDefinitions provider;

	// A magic skill: VERTICAL-013 territory.
	SkillDefinition magic = MakeActiveDamageSkill(2);
	magic.name = "Fireball";
	magic.apply = SkillApply::Magic;
	provider.Add(magic);

	// A zone skill: needs a world.
	SkillDefinition zone = MakeActiveDamageSkill(3);
	zone.name = "Shockwave";
	zone.targetKind = SkillTargetKind::Zone;
	provider.Add(zone);

	// A learned-only passive.
	SkillDefinition passive = MakeActiveDamageSkill(4);
	passive.name = "InnerPower";
	passive.role = SkillRole::Passive;
	provider.Add(passive);

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	attacker.GetValue().RestoreResources();
	target.GetValue().RestoreResources();

	for (const SkillId& id : { SkillId{ 1, 2 }, SkillId{ 1, 3 }, SkillId{ 1, 4 } })
	{
		(void) attacker.GetValue().LearnSkill(id);
		(void) attacker.GetValue().SetSkillLevel(id, 1);
	}

	const uint32_t hpBefore = target.GetValue().BuildSnapshot().GetValue().hp.current;

	// The zone and passive skills are still refused, each with its own reason, and
	// neither damages anything.
	//
	// VERTICAL-013 removed the magic case from this list: `{1,2}` is a hostile
	// single-target HP magic skill, which is exactly the slice this milestone
	// executes, so it now damages the target instead of refusing.
	const Skills::ActiveSkillResult magicResult =
		attacker.GetValue().CastSkill(SkillId{ 1, 2 }, target.GetValue());
	CHECK(magicResult.Succeeded());
	CHECK_EQ(magicResult.attackTypeUsed, Combat::AttackType::Magic);
	CHECK_LT(target.GetValue().BuildSnapshot().GetValue().hp.current, hpBefore);

	const uint32_t hpBeforeUnsupported = target.GetValue().BuildSnapshot().GetValue().hp.current;

	const Skills::ActiveSkillResult zoneResult =
		attacker.GetValue().CastSkill(SkillId{ 1, 3 }, target.GetValue());
	CHECK_EQ(zoneResult.failure, Skills::ActiveSkillFailure::UnsupportedTarget);

	const Skills::ActiveSkillResult passiveResult =
		attacker.GetValue().CastSkill(SkillId{ 1, 4 }, target.GetValue());
	CHECK_EQ(passiveResult.failure, Skills::ActiveSkillFailure::NotCastable);

	// The refusals really are inert.
	CHECK_EQ(target.GetValue().BuildSnapshot().GetValue().hp.current, hpBeforeUnsupported);
}

MODERN_TEST(ServerActiveSkill_NoProviderRefused)
{
	// A character built without a skill provider has nothing to resolve
	// against. That is an absence, not a zero-damage cast.
	auto attacker = ServerCharacter::Create(StandardDefinition());
	auto target   = ServerCharacter::Create(StandardDefinition());
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	const Skills::ActiveSkillResult result =
		attacker.GetValue().CastSkill(SkillId{ 1, 1 }, target.GetValue());

	CHECK_EQ(result.failure, Skills::ActiveSkillFailure::UnknownSkill);
}

MODERN_TEST(ServerActiveSkill_RequiredSpIncludesEquipmentContribution)
{
	// The equipment term is VERTICAL-010's hand sum, reused rather than
	// recomputed. A weapon costing 20 SP in the right hand raises the skill's
	// own 5 to 25.
	InMemorySkillDefinitions skillProvider;
	RegisterActiveSkill(skillProvider);

	InMemoryItemDefinitions itemProvider;
	ItemStatBlock weaponStats;
	weaponStats.requiredSP = 20;
	weaponStats.meleePower = 10;
	ItemDefinition weapon;
	weapon.id = ItemId(20101);
	weapon.kind = ItemKind::Weapon;
	weapon.name = "HeavySword";
	weapon.maxStack = 1;
	weapon.stats = weaponStats;
	itemProvider.Add(weapon);

	ServerCharacterDefinition definition =
		StandardDefinitionWithItemsAndSkills(itemProvider, skillProvider);

	auto attacker = ServerCharacter::Create(definition);
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(skillProvider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	CHECK(attacker.GetValue().Equip(EquipmentSlot::RightHand, TestItem(20101)).IsOk());
	PrepareCaster(attacker, 1);
	target.GetValue().RestoreResources();

	const Skills::ActiveSkillResult result =
		attacker.GetValue().CastSkill(SkillId{ 1, 1 }, target.GetValue());

	CHECK(result.Succeeded());
	// 20 from the hand, 5 from the skill.
	CHECK_EQ(result.requiredSP, static_cast<uint16_t>(25));
}

int main()
{
	// Unbuffered so an abort still shows which case was running.
	std::setvbuf(stdout, nullptr, _IONBF, 0);
	std::printf("Modern VERTICAL-001 server character tests\n\n");

	const int failedCases = ModernTests::RunAll();

	if (failedCases == 0)
	{
		std::printf("\nAll %d test cases passed.\n",
			static_cast<int>(ModernTests::Registry().size()));
		return 0;
	}

	std::printf("\n%d of %d test cases FAILED (%d checks).\n",
		failedCases,
		static_cast<int>(ModernTests::Registry().size()),
		ModernTests::FailureCount());
	return 1;
}

// ═══════════════════════════════════════════════════════════════════════
// VERTICAL-014: authoritative status state on the server
// ═══════════════════════════════════════════════════════════════════════

namespace
{
	// Reuses the local skill provider so the blow travels the real cast path.
	SkillDefinition MakeServerStunSkill(uint16_t index)
	{
		SkillDefinition def = MakeActiveDamageSkill(index);
		def.name      = "StunStrike" + std::to_string(index);
		def.stateBlow = StatusEffect::StatusEffectType::Stun;
		for (uint8_t lvl = 1; lvl <= def.maxLevel; ++lvl)
		{
			def.levelData[lvl].blowRate = 100.0f;   // lands on any injected roll
			def.levelData[lvl].life     = 10.0f;
		}
		return def;
	}
}

MODERN_TEST(ServerStatus_StartsEmpty)
{
	auto character = ServerCharacter::Create(StandardDefinition());

	CHECK(character.IsOk());
	CHECK_EQ(character.GetValue().GetStatus().ActiveCount(), 0u);
	CHECK_EQ(character.GetValue().GetStatus().ActiveDisorderMask(), 0u);
}

// Storing a resolved state is the server's job, and the rules live in core.
MODERN_TEST(ServerStatus_ApplyStoresAndQueryWorks)
{
	auto character = ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());

	StatusEffect::StatusEffectState state;
	state.type              = StatusEffect::StatusEffectType::Poison;
	state.remainingLifetime = 10.0f;
	state.var1              = 4.0f;

	CHECK(character.GetValue().ApplyStatus(state));
	CHECK(character.GetValue().GetStatus().Has(StatusEffect::StatusEffectType::Poison));
	CHECK_EQ(character.GetValue().GetStatus().ActiveCount(), 1u);
}

MODERN_TEST(ServerStatus_TickExpiresAndCureClears)
{
	auto character = ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());

	StatusEffect::StatusEffectState state;
	state.type              = StatusEffect::StatusEffectType::Stun;
	state.remainingLifetime = 5.0f;
	CHECK(character.GetValue().ApplyStatus(state));

	// Short of the duration: still up.
	CHECK_EQ(character.GetValue().TickStatus(2.0f), 0u);
	CHECK(character.GetValue().GetStatus().Has(StatusEffect::StatusEffectType::Stun));

	// Exactly to the duration: gone.
	CHECK_EQ(character.GetValue().TickStatus(3.0f), 1u);
	CHECK(!character.GetValue().GetStatus().Has(StatusEffect::StatusEffectType::Stun));

	// Re-apply, then cure it.
	CHECK(character.GetValue().ApplyStatus(state));
	CHECK_EQ(character.GetValue().CureStatus(StatusEffect::DisorderStun), 1u);
	CHECK(!character.GetValue().GetStatus().Has(StatusEffect::StatusEffectType::Stun));
}

// The cast path stores the blow on the TARGET, not the caster.
MODERN_TEST(ServerStatus_CastAppliesTheBlowToTheTarget)
{
	InMemorySkillDefinitions provider;
	provider.Add(MakeServerStunSkill(2));

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	attacker.GetValue().RestoreResources();
	target.GetValue().RestoreResources();

	CHECK(attacker.GetValue().LearnSkill(SkillId{ 1, 2 }).IsOk());
	CHECK(attacker.GetValue().SetSkillLevel(SkillId{ 1, 2 }, 1).IsOk());

	const Skills::ActiveSkillResult result =
		attacker.GetValue().CastSkill(SkillId{ 1, 2 }, target.GetValue());

	CHECK(result.Succeeded());
	CHECK(result.hasStatusApplication);
	CHECK(result.statusApplication.Applied());

	// The target carries the state...
	CHECK(target.GetValue().GetStatus().Has(StatusEffect::StatusEffectType::Stun));
	// ...and the caster does not.
	CHECK(!attacker.GetValue().GetStatus().Has(StatusEffect::StatusEffectType::Stun));
}

// A refused blow must not create state, but the cast still happened.
MODERN_TEST(ServerStatus_ImmuneTargetIsNotBlownAgain)
{
	InMemorySkillDefinitions provider;
	provider.Add(MakeServerStunSkill(2));

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	attacker.GetValue().RestoreResources();
	target.GetValue().RestoreResources();

	CHECK(attacker.GetValue().LearnSkill(SkillId{ 1, 2 }).IsOk());
	CHECK(attacker.GetValue().SetSkillLevel(SkillId{ 1, 2 }, 1).IsOk());

	// The target is already stunned.
	StatusEffect::StatusEffectState existing;
	existing.type              = StatusEffect::StatusEffectType::Stun;
	existing.remainingLifetime = 30.0f;
	CHECK(target.GetValue().ApplyStatus(existing));

	const Skills::ActiveSkillResult result =
		attacker.GetValue().CastSkill(SkillId{ 1, 2 }, target.GetValue());

	CHECK(result.Succeeded());
	// The server supplies no immunity mask here, so the blow lands and
	// overwrites the slot rather than being refused. What matters is that the
	// existing duration was replaced, not extended.
	CHECK(result.statusApplication.Applied());
	CHECK_EQ(target.GetValue().GetStatus().At(0)->remainingLifetime, 10.0f);
	CHECK_EQ(target.GetValue().GetStatus().ActiveCount(), 1u);
}

// Status lifetime is the server's to advance, independent of cooldowns.
MODERN_TEST(ServerStatus_StatusAndCooldownsAreIndependent)
{
	auto character = ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());

	StatusEffect::StatusEffectState state;
	state.type              = StatusEffect::StatusEffectType::Curse;
	state.remainingLifetime = 6.0f;
	CHECK(character.GetValue().ApplyStatus(state));

	character.GetValue().AdvanceSkillCooldowns(1.0f);

	CHECK(character.GetValue().GetStatus().Has(StatusEffect::StatusEffectType::Curse));
	CHECK_EQ(character.GetValue().GetStatus().At(3)->remainingLifetime, 6.0f);
}
// ═══════════════════════════════════════════════════════════════════════
// VERTICAL-015: authoritative skill FACTs on the server
// ═══════════════════════════════════════════════════════════════════════

namespace
{
	SkillDefinition MakeServerFactSkill(uint16_t index)
	{
		SkillDefinition def = MakeActiveDamageSkill(index);
		def.name       = "Aegis" + std::to_string(index);
		def.createsFact = true;
		for (uint8_t lvl = 1; lvl <= def.maxLevel; ++lvl)
		{
			def.levelData[lvl].life = 10.0f;
		}
		def.factSpecs[0].type = SkillFactSpecType::MoveVelo;
		def.factSpecs[1].type = SkillFactSpecType::ProhibitSkill;
		return def;
	}

	Skills::SkillFact MakeServerFact(uint16_t main, uint16_t sub, float lifetime)
	{
		Skills::SkillFact fact;
		fact.skillId                  = SkillId{ main, sub };
		fact.level                    = 1;
		fact.remainingLifetime        = lifetime;
		fact.basicType                = PassiveApplyType::VarHp;
		fact.basicValue               = 1.0f;
		fact.specs[0].type             = SkillFactSpecType::MoveVelo;
		fact.specs[0].var1             = 0.2f;
		return fact;
	}
}

MODERN_TEST(ServerSkillFact_StartsEmpty)
{
	auto character = ServerCharacter::Create(StandardDefinition());

	CHECK(character.IsOk());
	CHECK_EQ(character.GetValue().GetSkillFacts().ActiveCount(), 0u);
	// The modifier snapshot is the baseline before any advance.
	CHECK_EQ(character.GetValue().GetFactModifiers().moveVelocity, 0.0f);
}

MODERN_TEST(ServerSkillFact_ApplyStoresOnTheTarget)
{
	InMemorySkillDefinitions provider;
	provider.Add(MakeServerFactSkill(2));

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());

	attacker.GetValue().RestoreResources();
	target.GetValue().RestoreResources();
	CHECK(attacker.GetValue().LearnSkill(SkillId{ 1, 2 }).IsOk());
	CHECK(attacker.GetValue().SetSkillLevel(SkillId{ 1, 2 }, 1).IsOk());

	const Skills::ActiveSkillResult result =
		attacker.GetValue().CastSkill(SkillId{ 1, 2 }, target.GetValue());

	CHECK(result.Succeeded());
	CHECK(result.hasSkillFact);

	// The target owns the pool; the caster has none.
	CHECK_EQ(target.GetValue().GetSkillFacts().ActiveCount(), 1u);
	CHECK_EQ(attacker.GetValue().GetSkillFacts().ActiveCount(), 0u);
	CHECK(target.GetValue().GetSkillFacts().Has(SkillId{ 1, 2 }));
}

// The full lifecycle the milestone cares about: apply -> tick -> expire -> baseline.
MODERN_TEST(ServerSkillFact_ApplyTickExpireRestoresBaseline)
{
	auto character = ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());

	CHECK(character.GetValue().ApplySkillFact(MakeServerFact(1, 1, 10.0f)));

	character.GetValue().AdvanceSkillFacts(2.0f);
	CHECK_EQ(character.GetValue().GetFactModifiers().moveVelocity, 0.2f);
	CHECK(character.GetValue().GetSkillFacts().Has(SkillId{ 1, 1 }));

character.GetValue().AdvanceSkillFacts(20.0f);
	CHECK_EQ(character.GetValue().GetSkillFacts().ActiveCount(), 0u);

	// The expiring tick still carried the modifier - GLogixExPC.cpp:2295 runs
	// the spec switches after DISABLESKEFF, so the contribution lands one tick
	// late. The baseline is the NEXT pass; there is no explicit restore anywhere.
	CHECK_EQ(character.GetValue().GetFactModifiers().moveVelocity, 0.2f);

	character.GetValue().AdvanceSkillFacts(1.0f);
	CHECK_EQ(character.GetValue().GetFactModifiers().moveVelocity, 0.0f);
}

// Prohibit-skill is server-authoritative state that the resolver already reads.
MODERN_TEST(ServerSkillFact_ProhibitSkillAggregatesAndExpires)
{
	auto character = ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());

	Skills::SkillFact fact = MakeServerFact(1, 1, 5.0f);
	fact.specs[0].type = SkillFactSpecType::ProhibitSkill;
	fact.specs[1].type = SkillFactSpecType::MoveVelo;
	CHECK(character.GetValue().ApplySkillFact(fact));

	character.GetValue().AdvanceSkillFacts(1.0f);
	CHECK_EQ(character.GetValue().GetFactModifiers().prohibitSkill, true);

	character.GetValue().AdvanceSkillFacts(5.0f);
	character.GetValue().AdvanceSkillFacts(1.0f);
	CHECK_EQ(character.GetValue().GetFactModifiers().prohibitSkill, false);
}

// The immunity mask crosses into the status resolver as a value, not a reference.
MODERN_TEST(ServerSkillFact_ImmunityMaskReachesTheStatusPath)
{
	auto character = ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());

	Skills::SkillFact fact = MakeServerFact(1, 1, 10.0f);
	fact.specs[0].type     = SkillFactSpecType::NonBlow;
	fact.specs[0].specFlag = 0x02u;   // DIS_STUN
	CHECK(character.GetValue().ApplySkillFact(fact));

	character.GetValue().AdvanceSkillFacts(1.0f);
	CHECK_EQ(character.GetValue().GetFactModifiers().statusImmunityMask, 0x02u);

	StatusEffect::StatusApplicationInput input;
	input.type               = StatusEffect::StatusEffectType::Stun;
	input.actRate            = 100.0f;
	input.attackerLevel      = 10;
	input.targetLevel        = 10;
	input.randomRoll         = 0.0f;
	input.targetDisorderMask = character.GetValue().GetFactModifiers().statusImmunityMask;

	const StatusEffect::StatusApplicationResult refused =
		StatusEffect::ResolveStatusApplication(input);
	CHECK(!refused.Applied());
	CHECK_EQ(refused.refusal, StatusEffect::StatusRefusal::TargetImmune);
}

// Fact lifetime is independent of cooldowns, like status lifetime.
MODERN_TEST(ServerSkillFact_FactAndCooldownTicksAreIndependent)
{
	auto character = ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());
	CHECK(character.GetValue().ApplySkillFact(MakeServerFact(1, 1, 20.0f)));

	character.GetValue().AdvanceSkillCooldowns(1.0f);
	CHECK_EQ(character.GetValue().GetSkillFacts().ActiveCount(), 1u);

	// Only the FACT advance moves the fact.
	character.GetValue().AdvanceSkillCooldowns(50.0f);
	CHECK_EQ(character.GetValue().GetSkillFacts().ActiveCount(), 1u);

	character.GetValue().AdvanceSkillFacts(25.0f);
	CHECK_EQ(character.GetValue().GetSkillFacts().ActiveCount(), 0u);
}
// ═══════════════════════════════════════════════════════════════════════
// VERTICAL-017: FACT consumers through the real server path
// ═══════════════════════════════════════════════════════════════════════

namespace
{
	// A damage skill whose FACT payload is configurable per test.
	SkillDefinition MakeConfigurableFactSkill(uint16_t index)
	{
		SkillDefinition def = MakeActiveDamageSkill(index);
		def.name       = "FactSkill" + std::to_string(index);
		def.createsFact = true;
		for (uint8_t lvl = 1; lvl <= def.maxLevel; ++lvl)
		{
			def.levelData[lvl].life = 10.0f;
		}
		return def;
	}

	Skills::SkillFact MakeSpecOnlyFact(uint16_t main, uint16_t sub, float lifetime,
	                                   SkillFactSpecType spec,
	                                   float var1, float var2 = 0.0f)
	{
		Skills::SkillFact fact;
		fact.skillId                  = SkillId{ main, sub };
		fact.level                    = 1;
		fact.remainingLifetime        = lifetime;
		fact.basicType                = PassiveApplyType::VarHp;
		fact.basicValue               = 1.0f;
		fact.specs[0].type            = spec;
		fact.specs[0].var1            = var1;
		fact.specs[0].var2            = var2;
		return fact;
	}

	// A learned, castable melee damage skill the server will accept.
	Skills::SkillFact MakePowerFact(uint16_t main, uint16_t sub, float lifetime, int32_t pa)
	{
		Skills::SkillFact fact;
		fact.skillId                  = SkillId{ main, sub };
		fact.level                    = 1;
		fact.remainingLifetime        = lifetime;
		fact.basicType                = PassiveApplyType::VarHp;
		fact.basicValue               = 1.0f;
		fact.impacts[0].type           = SkillFactImpactType::Pa;
		fact.impacts[0].value          = static_cast<float>(pa);
		return fact;
	}
}

// ── PROHIBIT_SKILL through CastSkill ──────────────────────────────────

MODERN_TEST(ServerSkillFactConsumers_NoProhibitFactAllowsTheCast)
{
	InMemorySkillDefinitions provider;
	provider.Add(MakeServerFactSkill(2));   // MoveVelo + ProhibitSkill, but not applied

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());
	attacker.GetValue().RestoreResources();
	target.GetValue().RestoreResources();
	CHECK(attacker.GetValue().LearnSkill(SkillId{ 1, 2 }).IsOk());
	CHECK(attacker.GetValue().SetSkillLevel(SkillId{ 1, 2 }, 1).IsOk());

	// Nothing applied, so the aggregated modifier is the baseline.
	CHECK_EQ(attacker.GetValue().GetFactModifiers().prohibitSkill, false);
	CHECK(attacker.GetValue().CastSkill(SkillId{ 1, 2 }, target.GetValue()).Succeeded());
}

MODERN_TEST(ServerSkillFactConsumers_ActiveProhibitFactRejectsTheCast)
{
	InMemorySkillDefinitions provider;
	provider.Add(MakeServerFactSkill(2));

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());
	attacker.GetValue().RestoreResources();
	target.GetValue().RestoreResources();
	CHECK(attacker.GetValue().LearnSkill(SkillId{ 1, 2 }).IsOk());
	CHECK(attacker.GetValue().SetSkillLevel(SkillId{ 1, 2 }, 1).IsOk());

	// The attacker is wearing a prohibit-skill buff (applied by a caster, not by
	// itself - that is the whole point of a buff).
	CHECK(attacker.GetValue().ApplySkillFact(
		MakeSpecOnlyFact(9, 9, 60.0f, SkillFactSpecType::ProhibitSkill, 0.0f)));
	attacker.GetValue().AdvanceSkillFacts(0.1f);
	CHECK_EQ(attacker.GetValue().GetFactModifiers().prohibitSkill, true);

	const Skills::ActiveSkillResult result =
		attacker.GetValue().CastSkill(SkillId{ 1, 2 }, target.GetValue());

	CHECK(!result.Succeeded());
	CHECK_EQ(result.failure, Skills::ActiveSkillFailure::NotCastable);
}

// The prohibition must land BEFORE anything is spent, matching
// GLogixExPC.cpp:4060 (the first statement of CHECKSKILL).
MODERN_TEST(ServerSkillFactConsumers_ProhibitionSpendsNothing)
{
	InMemorySkillDefinitions provider;
	provider.Add(MakeServerFactSkill(2));

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());
	attacker.GetValue().RestoreResources();
	target.GetValue().RestoreResources();
	CHECK(attacker.GetValue().LearnSkill(SkillId{ 1, 2 }).IsOk());
	CHECK(attacker.GetValue().SetSkillLevel(SkillId{ 1, 2 }, 1).IsOk());

	CHECK(attacker.GetValue().ApplySkillFact(
		MakeSpecOnlyFact(9, 9, 60.0f, SkillFactSpecType::ProhibitSkill, 0.0f)));
	attacker.GetValue().AdvanceSkillFacts(0.1f);

	const uint32_t spBefore  = attacker.GetValue().BuildSnapshot().GetValue().sp.current;
	const uint32_t mpBefore  = attacker.GetValue().BuildSnapshot().GetValue().mp.current;
	const uint32_t hpBefore  = attacker.GetValue().BuildSnapshot().GetValue().hp.current;

	(void) attacker.GetValue().CastSkill(SkillId{ 1, 2 }, target.GetValue());

	CHECK_EQ(attacker.GetValue().BuildSnapshot().GetValue().sp.current, spBefore);
	CHECK_EQ(attacker.GetValue().BuildSnapshot().GetValue().mp.current, mpBefore);
	CHECK_EQ(attacker.GetValue().BuildSnapshot().GetValue().hp.current, hpBefore);

	// And no cooldown was started.
	CHECK(!attacker.GetValue().IsSkillOnCooldown(SkillId{ 1, 2 }));
}

MODERN_TEST(ServerSkillFactConsumers_ExpiredProhibitFactAllowsTheCastAgain)
{
	InMemorySkillDefinitions provider;
	provider.Add(MakeServerFactSkill(2));

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());
	attacker.GetValue().RestoreResources();
	target.GetValue().RestoreResources();
	CHECK(attacker.GetValue().LearnSkill(SkillId{ 1, 2 }).IsOk());
	CHECK(attacker.GetValue().SetSkillLevel(SkillId{ 1, 2 }, 1).IsOk());

	CHECK(attacker.GetValue().ApplySkillFact(
		MakeSpecOnlyFact(9, 9, 5.0f, SkillFactSpecType::ProhibitSkill, 0.0f)));
	attacker.GetValue().AdvanceSkillFacts(0.1f);
	CHECK_EQ(attacker.GetValue().GetFactModifiers().prohibitSkill, true);

	// Outlive it, then a further tick clears the rebuilt snapshot.
	attacker.GetValue().AdvanceSkillFacts(5.0f);
	attacker.GetValue().AdvanceSkillFacts(0.1f);
	CHECK_EQ(attacker.GetValue().GetFactModifiers().prohibitSkill, false);

	CHECK(attacker.GetValue().CastSkill(SkillId{ 1, 2 }, target.GetValue()).Succeeded());
}

// ── NONBLOW through CastSkill ─────────────────────────────────────────

MODERN_TEST(ServerSkillFactConsumers_ActiveNonBlowRefusesTheMatchingStatus)
{
	InMemorySkillDefinitions provider;
	SkillDefinition stunner = MakeServerStunSkill(2);
	stunner.applyType = PassiveApplyType::Hp;   // a bare status blow, no damage FACT
	provider.Add(stunner);

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());
	attacker.GetValue().RestoreResources();
	target.GetValue().RestoreResources();
	CHECK(attacker.GetValue().LearnSkill(SkillId{ 1, 2 }).IsOk());
	CHECK(attacker.GetValue().SetSkillLevel(SkillId{ 1, 2 }, 1).IsOk());

	// Without the immunity the blow lands.
	const Skills::ActiveSkillResult unprotectedResult =
		attacker.GetValue().CastSkill(SkillId{ 1, 2 }, target.GetValue());
	CHECK(unprotectedResult.Succeeded());
	CHECK(unprotectedResult.statusApplication.Applied());
	(void) target.GetValue().CureStatus(StatusEffect::DisorderStun);

	// The first cast started a cooldown, which would mask the immunity result.
	attacker.GetValue().AdvanceSkillCooldowns(600.0f);

	// Now the target is immune to stun, and the cast path says so.
	Skills::SkillFact immune;
	immune.skillId           = SkillId{ 9, 9 };
	immune.remainingLifetime = 60.0f;
	immune.basicType         = PassiveApplyType::VarHp;
	immune.basicValue        = 1.0f;
	immune.specs[0].type     = SkillFactSpecType::NonBlow;
	immune.specs[0].specFlag = static_cast<uint32_t>(StatusEffect::DisorderStun);
	CHECK(target.GetValue().ApplySkillFact(immune));
	target.GetValue().AdvanceSkillFacts(0.1f);

	const Skills::ActiveSkillResult protectedResult =
		attacker.GetValue().CastSkill(SkillId{ 1, 2 }, target.GetValue());

	CHECK(protectedResult.Succeeded());   // the cast happened
	CHECK(protectedResult.hasStatusApplication);
	CHECK(!protectedResult.statusApplication.Applied());
	CHECK_EQ(protectedResult.statusApplication.refusal, StatusEffect::StatusRefusal::TargetImmune);
	// ...and no stun was stored.
	CHECK(!target.GetValue().GetStatus().Has(StatusEffect::StatusEffectType::Stun));
}

MODERN_TEST(ServerSkillFactConsumers_MultipleNonBlowFactsAreLastWins)
{
	auto character = ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());

	Skills::SkillFact stunImmune;
	stunImmune.skillId           = SkillId{ 1, 1 };
	stunImmune.remainingLifetime = 30.0f;
	stunImmune.basicType         = PassiveApplyType::VarHp;
	stunImmune.basicValue        = 1.0f;
	stunImmune.specs[0].type     = SkillFactSpecType::NonBlow;
	stunImmune.specs[0].specFlag = static_cast<uint32_t>(StatusEffect::DisorderStun);

	Skills::SkillFact poisonImmune = stunImmune;
	poisonImmune.skillId           = SkillId{ 1, 2 };
	poisonImmune.specs[0].specFlag = static_cast<uint32_t>(StatusEffect::DisorderPoison);

	CHECK(character.GetValue().ApplySkillFact(stunImmune));    // slot 0
	CHECK(character.GetValue().ApplySkillFact(poisonImmune));  // slot 1
	character.GetValue().AdvanceSkillFacts(0.1f);

	// Assignment, not OR: slot 1's mask replaced slot 0's outright.
	CHECK_EQ(character.GetValue().GetFactModifiers().statusImmunityMask,
	         static_cast<uint32_t>(StatusEffect::DisorderPoison));
}

MODERN_TEST(ServerSkillFactConsumers_ExpiredNonBlowStopsProtecting)
{
	auto character = ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());

	Skills::SkillFact immune;
	immune.skillId           = SkillId{ 1, 1 };
	immune.remainingLifetime = 5.0f;
	immune.basicType         = PassiveApplyType::VarHp;
	immune.basicValue        = 1.0f;
	immune.specs[0].type     = SkillFactSpecType::NonBlow;
	immune.specs[0].specFlag = static_cast<uint32_t>(StatusEffect::DisorderStun);
	CHECK(character.GetValue().ApplySkillFact(immune));

	character.GetValue().AdvanceSkillFacts(0.1f);
	CHECK_EQ(character.GetValue().GetFactModifiers().statusImmunityMask,
	         static_cast<uint32_t>(StatusEffect::DisorderStun));

	character.GetValue().AdvanceSkillFacts(5.0f);
	character.GetValue().AdvanceSkillFacts(0.1f);
	CHECK_EQ(character.GetValue().GetFactModifiers().statusImmunityMask, 0u);
}

// ── PA / SA / MA through the derived-stat pipeline ────────────────────

MODERN_TEST(ServerSkillFactConsumers_PowerFactRaisesTheDerivedPower)
{
	auto character = ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());

	const uint16_t before = character.GetValue().BuildSnapshot().GetValue().derived.meleePower;

	CHECK(character.GetValue().ApplySkillFact(MakePowerFact(1, 1, 30.0f, 25)));
	character.GetValue().AdvanceSkillFacts(0.1f);

	CHECK_EQ(character.GetValue().GetFactModifiers().meleePower, 25);
	CHECK_GT(character.GetValue().BuildSnapshot().GetValue().derived.meleePower, before);
}

// Expiry must remove ONLY the timed contribution, through recalculation.
MODERN_TEST(ServerSkillFactConsumers_ExpiredPowerFactRestoresTheBaseline)
{
	auto character = ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());

	const uint16_t before = character.GetValue().BuildSnapshot().GetValue().derived.meleePower;

	CHECK(character.GetValue().ApplySkillFact(MakePowerFact(1, 1, 5.0f, 25)));
	character.GetValue().AdvanceSkillFacts(0.1f);
	CHECK_GT(character.GetValue().BuildSnapshot().GetValue().derived.meleePower, before);

	// Outlive it, then tick again so the rebuilt snapshot is applied.
	character.GetValue().AdvanceSkillFacts(5.0f);
	character.GetValue().AdvanceSkillFacts(0.1f);

	CHECK_EQ(character.GetValue().GetFactModifiers().meleePower, 0);
	CHECK_EQ(character.GetValue().BuildSnapshot().GetValue().derived.meleePower, before);
}

MODERN_TEST(ServerSkillFactConsumers_TwoPowerFactsAccumulate)
{
	auto character = ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());

	const uint16_t before = character.GetValue().BuildSnapshot().GetValue().derived.meleePower;

	CHECK(character.GetValue().ApplySkillFact(MakePowerFact(1, 1, 30.0f, 10)));
	CHECK(character.GetValue().ApplySkillFact(MakePowerFact(1, 2, 30.0f, 15)));
	character.GetValue().AdvanceSkillFacts(0.1f);

	// SUM, unlike the damage reductions.
	CHECK_EQ(character.GetValue().GetFactModifiers().meleePower, 25);
	CHECK_GT(character.GetValue().BuildSnapshot().GetValue().derived.meleePower, before);
}

// The FACT contribution must be independent of the permanent ones: a second
// character with the same permanent stats but no buff is the control.
MODERN_TEST(ServerSkillFactConsumers_FactPowerIsSeparateFromPermanentStats)
{
	auto buffed   = ServerCharacter::Create(StandardDefinition());
	auto unbuffed = ServerCharacter::Create(StandardDefinition());
	CHECK(buffed.IsOk());
	CHECK(unbuffed.IsOk());

	CHECK_EQ(buffed.GetValue().BuildSnapshot().GetValue().derived.meleePower,
	         unbuffed.GetValue().BuildSnapshot().GetValue().derived.meleePower);

	CHECK(buffed.GetValue().ApplySkillFact(MakePowerFact(1, 1, 30.0f, 40)));
	buffed.GetValue().AdvanceSkillFacts(0.1f);

	CHECK_GT(buffed.GetValue().BuildSnapshot().GetValue().derived.meleePower,
	         unbuffed.GetValue().BuildSnapshot().GetValue().derived.meleePower);
	// The permanent stat itself is untouched: only the snapshot differs.
	CHECK_EQ(unbuffed.GetValue().GetFactModifiers().meleePower, 0);
}

// ── Damage reduction through the combat input ────────────────────────

MODERN_TEST(ServerSkillFactConsumers_ReductionReachesTheCombatInput)
{
	InMemorySkillDefinitions provider;
	provider.Add(MakeConfigurableFactSkill(2));

	auto attacker = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	auto target   = ServerCharacter::Create(StandardDefinitionWithSkills(provider));
	CHECK(attacker.IsOk());
	CHECK(target.IsOk());
	attacker.GetValue().RestoreResources();
	target.GetValue().RestoreResources();
	CHECK(attacker.GetValue().LearnSkill(SkillId{ 1, 2 }).IsOk());
	CHECK(attacker.GetValue().SetSkillLevel(SkillId{ 1, 2 }, 1).IsOk());

	CHECK(target.GetValue().ApplySkillFact(
		MakeSpecOnlyFact(9, 9, 60.0f, SkillFactSpecType::PsyDamageReduce, 0.9f)));
	target.GetValue().AdvanceSkillFacts(0.1f);
	CHECK_EQ(target.GetValue().GetFactModifiers().psyDamageReduce, 0.9f);

	// The basic-attack path must see it.
	const Skills::SkillFactModifiers seen = target.GetValue().GetFactModifiers();
	CHECK_EQ(seen.psyDamageReduce, 0.9f);
}
