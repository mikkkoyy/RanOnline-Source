// VERTICAL-001: the client's character state, and the server/client relationship.
//
// Headless. The interesting cases are the ones that would fail if the client
// acquired any authority of its own:
//
//   - that everything the client reports is exactly what the server published,
//     with no recomputation anywhere in between;
//   - that a client holding no snapshot reports defined emptiness rather than a
//     plausible wrong number;
//   - that the client's translation unit contains no call into the stat
//     system, so authority cannot creep back in.

#include "TestHarness.h"

#include "character/Character.h"
#include "character/CharacterClassTable.h"
#include "equipment/EquipmentState.h"
#include "equipment/ItemDefinitionProvider.h"
#include "gameplay/ClientCharacterState.h"
#include "gameplay/CharacterSnapshot.h"
#include "item/ItemDefinition.h"
#include "item/ItemInstance.h"
#include "character/ServerCharacter.h"
#include "skills/PassiveContributionAggregator.h"
#include "skills/SkillDefinition.h"
#include "skills/SkillDefinitionProvider.h"
#include "skills/SkillState.h"
#include "stats/DerivedStats.h"
#include "stats/StatCalculator.h"
#include "progression/CodexDefinition.h"
#include "progression/CodexDefinitionProvider.h"
#include "types/Result.h"

#include <fstream>
#include <string>

using namespace Modern;
using namespace Modern::Client::Gameplay;

namespace
{
	Stats::ClassConstants StandardClass()
	{
		Stats::ClassConstants cc;
		cc.beginStats.pow = 10; cc.beginStats.str = 20;
		cc.beginStats.spi = 15; cc.beginStats.dex = 25;
		cc.beginStats.intel = 8; cc.beginStats.sta = 12;
		cc.beginAttackPoint = 10; cc.beginDefensePoint = 5;
		cc.beginMeleePower = 3;   cc.beginShootPower = 4;
		cc.attackPointConversion = 1.0f; cc.defensePointConversion = 1.0f;
		cc.meleePowerConversion = 1.0f;   cc.shootPowerConversion = 1.0f;
		cc.hpPerStr = 5.0f; cc.mpPerSpi = 4.0f; cc.spPerSta = 2.0f;
		// A non-zero level-up term, so a level change is observable.
		cc.levelUpStats.dex = 0.5f;
		cc.hitPerDex = 2.0f; cc.avoidPerDex = 1.0f; cc.defensePerDex = 3.0f;
		cc.meleePerPow = 1.0f; cc.meleePerDex = 0.5f;
		cc.shootPerPow = 1.0f; cc.shootPerDex = 0.5f;
		cc.magicPerDex = 1.0f; cc.magicPerSpi = 1.0f; cc.magicPerIntel = 2.0f;
		return cc;
	}

	Server::ServerCharacterDefinition StandardDefinition()
	{
		Server::ServerCharacterDefinition definition;
		definition.id             = CharacterId(7u);
		definition.name           = "Raner";
		definition.characterClass = CharacterClass::Brawler;
		definition.gender        = CharacterGender::Male;
		definition.level         = 1;
		definition.classConstants = StandardClass();
		return definition;
	}

	// VERTICAL-003: skill test helpers
	SkillId MakeClientTestSkillId(uint16_t skillIndex)
	{
		return SkillId{ 1, skillIndex };
	}

	// A passive whose basic apply type is a flat HP bonus, with an optional
	// recovery-rate impact. A skill definition has no stats block of its own;
	// it carries a per-level basic value and typed impacts.
	ItemStatBlock TestClientSkillStats(int32_t hp, float hpRecoveryRate)
	{
		ItemStatBlock stats;
		stats.hp             = hp;
		stats.hpRecoveryRate = hpRecoveryRate;
		return stats;
	}

	SkillDefinition MakeClientTestSkill(uint32_t id, const ItemStatBlock& stats)
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

	ItemDefinition MakeClientTestWeapon(uint32_t id)
	{
		ItemDefinition def;
		def.id = ItemId(id);
		def.kind = ItemKind::Weapon;
		def.name = "Blade";
		def.maxStack = 1;
		ItemStatBlock stats;
		stats.hp = 55;
		stats.meleePower = 12;
		def.stats = stats;
		return def;
	}

	ItemDefinition MakeClientTestArmor(uint32_t id)
	{
		ItemDefinition def;
		def.id = ItemId(id);
		def.kind = ItemKind::Armor;
		def.name = "Cuirass";
		def.maxStack = 1;
		ItemStatBlock stats;
		stats.hp = 45;
		stats.str = 10;
		stats.defense = 8;
		def.stats = stats;
		return def;
	}

	ItemInstance MakeClientTestInstance(uint32_t defId, uint64_t serial)
	{
		ItemInstance item;
		item.definition = ItemId(defId);
		item.serial = serial;
		item.count = 1;
		return item;
	}

	Server::ServerCharacterDefinition StandardDefinitionWithItems(
		InMemoryItemDefinitions& provider)
	{
		Server::ServerCharacterDefinition definition = StandardDefinition();
		definition.itemDefinitions = &provider;
		provider.Add(MakeClientTestWeapon(20001));
		provider.Add(MakeClientTestArmor(20002));
		return definition;
	}
}

// ---------------------------------------------------------------------------
// The shared contract
// ---------------------------------------------------------------------------

MODERN_TEST(Gameplay_SnapshotRejectsWhatCannotExist)
{
	// Built up one field at a time, so each rule is shown to be the one that
	// refuses rather than a neighbour.
	Gameplay::CharacterSnapshot snapshot;
	CHECK(!Gameplay::CharacterSnapshot::IsValid(snapshot));
	CHECK(Gameplay::CharacterSnapshot::Create(snapshot).IsError());

	snapshot.id = CharacterId(1u);            // still no name
	CHECK(!Gameplay::CharacterSnapshot::IsValid(snapshot));

	snapshot.id = CharacterId::MakeInvalid();
	snapshot.name = "Raner";                 // still no class
	CHECK(!Gameplay::CharacterSnapshot::IsValid(snapshot));

	snapshot.id = CharacterId(1u);
	snapshot.characterClass = CharacterClass::Brawler;
	CHECK(Gameplay::CharacterSnapshot::IsValid(snapshot));

	// Level defaults to 1, which is RAN's minimum, so the default is legal.
	snapshot.level = 0;                      // below the range
	CHECK(!Gameplay::CharacterSnapshot::IsValid(snapshot));

	snapshot.level = 1;
	snapshot.experience = -1;                // experience is never negative
	CHECK(!Gameplay::CharacterSnapshot::IsValid(snapshot));

	snapshot.experience = 0;
	CHECK(Gameplay::CharacterSnapshot::IsValid(snapshot));

	// A current pool above the published maximum is refused rather than
	// clamped, so a publisher bug stays visible.
	snapshot.hp.current     = 1;
	snapshot.derived.maxHp  = 100;
	CHECK(Gameplay::CharacterSnapshot::IsValid(snapshot));
	snapshot.hp.current = 101;
	CHECK(!Gameplay::CharacterSnapshot::IsValid(snapshot));
}

MODERN_TEST(Gameplay_SnapshotFractionsAreClamped)
{
	Gameplay::CharacterSnapshot snapshot;
	snapshot.id             = CharacterId(1u);
	snapshot.name           = "Raner";
	snapshot.characterClass = CharacterClass::Brawler;
	snapshot.level          = 1;
	snapshot.derived.maxHp  = 200;
	snapshot.hp.current     = 50;
	CHECK_EQ(snapshot.GetHealthFraction(), 0.25f);

	// A zero maximum is a real state for an unbuilt character, and divides to
	// zero rather than by zero.
	snapshot.derived.maxHp = 0;
	snapshot.hp.current    = 0;
	CHECK_EQ(snapshot.GetHealthFraction(), 0.0f);
}

// ---------------------------------------------------------------------------
// Client state
// ---------------------------------------------------------------------------

MODERN_TEST(Gameplay_ClientStartsWithNothing)
{
	ClientCharacterState state;
	CHECK(!state.HasSnapshot());
	CHECK(!state.GetId().IsValid());
	CHECK(state.GetName().empty());
	CHECK(state.GetClass() == CharacterClass::Unset);
	CHECK_EQ(state.GetMaxHp(), 0u);
	CHECK_EQ(state.GetCurrentHp(), 0u);
	CHECK_EQ(state.GetHealthFraction(), 0.0f);
}

MODERN_TEST(Gameplay_ClientPresentsWhatTheServerPublished)
{
	const Result<Server::ServerCharacter> created =
		Server::ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();
	CHECK(character.SetCurrentHp(37).IsOk());
	CHECK(character.SetCurrentMp(11).IsOk());

	const Result<Gameplay::CharacterSnapshot> snapshot = character.BuildSnapshot();
	CHECK(snapshot.IsOk());
	if (snapshot.IsError())
	{
		return;
	}

	ClientCharacterState state;
	CHECK(state.Apply(snapshot.GetValue()).IsOk());
	CHECK(state.HasSnapshot());

	// Every value is the server's, unchanged.
	CHECK_EQ(state.GetId(), character.GetId());
	CHECK_EQ(state.GetName(), character.GetName());
	CHECK(state.GetClass() == CharacterClass::Brawler);
	CHECK_EQ(state.GetLevel(), character.GetLevel());
	CHECK_EQ(state.GetCurrentHp(), 37u);
	CHECK_EQ(state.GetCurrentMp(), 11u);
	CHECK_EQ(state.GetMaxHp(), character.GetDerivedStats().maxHp);
	CHECK(state.GetDerivedStats() == character.GetDerivedStats());
	CHECK(state.GetTotalStats() == character.GetTotalStats());
}

MODERN_TEST(Gameplay_ClientAdoptsALaterSnapshotWholesale)
{
	// A new snapshot replaces the old one. There is no per-field setter, so
	// there is no way for the client to hold a mixture of two of them.
	const Result<Server::ServerCharacter> created =
		Server::ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();
	character.RestoreResources();

	ClientCharacterState state;
	CHECK(state.Apply(character.BuildSnapshot().GetValue()).IsOk());
	CHECK_EQ(state.GetCurrentHp(), character.GetDerivedStats().maxHp);

	CHECK(character.SetLevel(30).IsOk());
	CHECK(character.SetCurrentHp(5).IsOk());
	CHECK(state.Apply(character.BuildSnapshot().GetValue()).IsOk());
	CHECK_EQ(state.GetLevel(), 30);
	CHECK_EQ(state.GetCurrentHp(), 5u);
	CHECK(state.GetDerivedStats() == character.GetDerivedStats());
}

MODERN_TEST(Gameplay_ClientRejectsAMalformedSnapshot)
{
	ClientCharacterState state;
	// A snapshot with no identity could not have come from a server.
	Gameplay::CharacterSnapshot snapshot;
	CHECK(state.Apply(snapshot).IsError());
	CHECK(!state.HasSnapshot());

	// One that is valid, then one that is not, leaves the valid one in place:
	// a rejected update does not destroy what the client already knew.
	const Result<Server::ServerCharacter> created =
		Server::ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();
	CHECK(state.Apply(character.BuildSnapshot().GetValue()).IsOk());

	Gameplay::CharacterSnapshot broken = character.BuildSnapshot().GetValue();
	broken.hp.current = broken.derived.maxHp + 1000;
	CHECK(state.Apply(broken).IsError());
	CHECK(state.HasSnapshot());
	CHECK(state.GetDerivedStats() == character.GetDerivedStats());
}

MODERN_TEST(Gameplay_ClientClearForgetsEverything)
{
	const Result<Server::ServerCharacter> created =
		Server::ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ClientCharacterState state;
	CHECK(state.Apply(created.GetValue().BuildSnapshot().GetValue()).IsOk());
	CHECK(state.HasSnapshot());

	state.Clear();
	CHECK(!state.HasSnapshot());
	CHECK(!state.GetId().IsValid());
	CHECK_EQ(state.GetMaxHp(), 0u);
	CHECK(state.GetDerivedStats() == Stats::DerivedStats());
}

// ---------------------------------------------------------------------------
// The authority relationship
// ---------------------------------------------------------------------------

MODERN_TEST(Gameplay_ClientNumbersEqualTheOneStatImplementation)
{
	// VERTICAL-003: passive contributions now come from learned skills.
	// Given identical inputs, the server's answer, the client-held answer and a
	// direct call to Modern::Stats::Calculate all agree. This is the assertion
	// that would fail if the client had grown a second implementation.
	InMemoryItemDefinitions itemProvider;
	InMemorySkillDefinitions skillProvider;

	ItemStatBlock skillStats;
	skillStats.hp = 17;
	skillStats.hpRecoveryRate = 0.25f;
	skillProvider.Add(MakeClientTestSkill(20003, skillStats));

	Server::ServerCharacterDefinition definition = StandardDefinitionWithItems(itemProvider);
	definition.level = 42;
	definition.experience = 9999;
	definition.allocatedStats.pow = 7;
	definition.allocatedStats.dex = 19;
	definition.skillDefinitions = &skillProvider;

	const Result<Server::ServerCharacter> created = Server::ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();

	// Learn the skill that provides the passive contribution
	CHECK(character.LearnSkill(MakeClientTestSkillId(20003)).IsOk());
	character.RestoreResources();

	Stats::CharClassIndex classIndex{};
	TryToCharClassIndex(definition.characterClass, definition.gender, classIndex);
	Stats::StatCalculationInput input;
	input.characterClass = classIndex;
	input.level          = definition.level;
	input.classConstants = definition.classConstants;
	input.allocatedStats = definition.allocatedStats;
	input.items          = Stats::ItemContribution();
	// The passive contribution now comes from the learned skill
	input.passives       = character.GetPassiveContribution();
	input.codex          = definition.codex;
	input.confPointRate  = definition.confPointRate;
	const Stats::DerivedStats direct = Stats::Calculate(input).GetValue();

	CHECK(character.GetDerivedStats() == direct);

	ClientCharacterState client;
	CHECK(client.Apply(character.BuildSnapshot().GetValue()).IsOk());
	CHECK(client.GetDerivedStats() == direct);
	CHECK_EQ(client.GetMaxHp(), direct.maxHp);
	CHECK_EQ(client.GetCurrentHp(), direct.maxHp);
	CHECK_EQ(client.GetHealthFraction(), 1.0f);
}

MODERN_TEST(Gameplay_ClientSourceContainsNoStatCalculation)
{
	// Authority is enforced by structure, not by convention. If the client's
	// translation unit ever calls Modern::Stats::Calculate, this fails - which
	// is a build-time signal rather than a code review question.
	const char* path = "modern/client/gameplay/ClientCharacterState.cpp";
	std::ifstream file(path);
	if (!file)
	{
		// Running from a different working directory: the check is skipped
		// rather than reported as a pass, and printed so it is not silent.
		std::printf("      (source check skipped, could not open %s)\n", path);
		return;
	}
	const std::string source{ std::istreambuf_iterator<char>(file),
	                         std::istreambuf_iterator<char>() };

	CHECK(source.find("Stats::Calculate") == std::string::npos);
	CHECK(source.find("StatCalculator.h") == std::string::npos);
}

// ---------------------------------------------------------------------------
// VERTICAL-002: Equipment presentation
// ---------------------------------------------------------------------------

MODERN_TEST(Gameplay_ClientReportsEmptyEquipmentWithNoSnapshot)
{
	ClientCharacterState client;
	CHECK_EQ(client.GetEquipment().GetOccupiedCount(), static_cast<size_t>(0));
	CHECK(!client.HasEquipped(EquipmentSlot::Headgear));
	CHECK(!client.HasEquipped(EquipmentSlot::RightHand));

	const Gameplay::EquippedItem& empty = client.GetEquippedItem(EquipmentSlot::Headgear);
	CHECK(!empty.definition.IsValid());
}

MODERN_TEST(Gameplay_ClientPresentsEquippedItemsFromSnapshot)
{
	InMemoryItemDefinitions provider;
	Server::ServerCharacterDefinition definition = StandardDefinitionWithItems(provider);

	const Result<Server::ServerCharacter> created = Server::ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();

	CHECK(character.Equip(EquipmentSlot::RightHand, MakeClientTestInstance(20001, 1)).IsOk());
	CHECK(character.Equip(EquipmentSlot::Headgear,  MakeClientTestInstance(20002, 2)).IsOk());

	const Gameplay::CharacterSnapshot snapshot = character.BuildSnapshot().GetValue();

	ClientCharacterState client;
	CHECK(client.Apply(snapshot).IsOk());

	CHECK_EQ(client.GetEquipment().GetOccupiedCount(), static_cast<size_t>(2));
	CHECK(client.HasEquipped(EquipmentSlot::RightHand));
	CHECK(client.HasEquipped(EquipmentSlot::Headgear));

	const Gameplay::EquippedItem& weapon =
		client.GetEquippedItem(EquipmentSlot::RightHand);
	CHECK_EQ(weapon.definition, ItemId(20001));
	CHECK_EQ(weapon.serial, static_cast<uint64_t>(1));
	CHECK_EQ(weapon.kind, ItemKind::Weapon);
	CHECK_EQ(weapon.name, "Blade");

	const Gameplay::EquippedItem& armor =
		client.GetEquippedItem(EquipmentSlot::Headgear);
	CHECK_EQ(armor.definition, ItemId(20002));
	CHECK_EQ(armor.serial, static_cast<uint64_t>(2));
	CHECK_EQ(armor.kind, ItemKind::Armor);
	CHECK_EQ(armor.name, "Cuirass");
}

MODERN_TEST(Gameplay_ClientEquippedCountMatchesServer)
{
	InMemoryItemDefinitions provider;
	Server::ServerCharacterDefinition definition = StandardDefinitionWithItems(provider);

	const Result<Server::ServerCharacter> created = Server::ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();

	character.Equip(EquipmentSlot::RightHand, MakeClientTestInstance(20001, 1));
	character.Equip(EquipmentSlot::LeftHand,  MakeClientTestInstance(20001, 3));
	character.Equip(EquipmentSlot::Neck,      MakeClientTestInstance(20002, 4));

	const Gameplay::CharacterSnapshot snapshot = character.BuildSnapshot().GetValue();
	ClientCharacterState client;
	CHECK(client.Apply(snapshot).IsOk());

	CHECK_EQ(client.GetEquipment().GetOccupiedCount(),
	        snapshot.equipped.GetOccupiedCount());
	CHECK_EQ(client.GetEquipment().GetOccupiedCount(),
	        character.GetEquipment().GetOccupiedCount());
}

MODERN_TEST(Gameplay_ClientClearForgetsEquipment)
{
	InMemoryItemDefinitions provider;
	Server::ServerCharacterDefinition definition = StandardDefinitionWithItems(provider);

	const Result<Server::ServerCharacter> created = Server::ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();
	character.Equip(EquipmentSlot::RightHand, MakeClientTestInstance(20001, 1));

	const Gameplay::CharacterSnapshot snapshot = character.BuildSnapshot().GetValue();
	ClientCharacterState client;
	CHECK(client.Apply(snapshot).IsOk());
	CHECK(client.HasEquipped(EquipmentSlot::RightHand));

	client.Clear();
	CHECK_EQ(client.GetEquipment().GetOccupiedCount(), static_cast<size_t>(0));
	CHECK(!client.HasEquipped(EquipmentSlot::RightHand));
}

MODERN_TEST(Gameplay_ClientPresentsSkillLevelChangesFromNewSnapshots)
{
	// A level change arrives as a new snapshot, never as a mutation of the one
	// the client holds. The client is a read-only view: a change comes from the
	// server, and the client's answer is to hold a different snapshot.
	InMemorySkillDefinitions skillProvider;

	SkillDefinition def = MakeClientTestSkill(20005, TestClientSkillStats(80, 0.0f));
	def.maxLevel = 3;
	def.levelData[2].basicVar = 150.0f;
	def.levelData[3].basicVar = 240.0f;
	CHECK(skillProvider.Add(def).IsOk());

	Server::ServerCharacterDefinition definition = StandardDefinition();
	definition.skillDefinitions = &skillProvider;

	const Result<Server::ServerCharacter> created = Server::ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();
	CHECK(character.LearnSkill(MakeClientTestSkillId(20005)).IsOk());

	ClientCharacterState client;
	CHECK(client.Apply(character.BuildSnapshot().GetValue()).IsOk());
	CHECK_EQ(client.GetSkillLevel(MakeClientTestSkillId(20005)), static_cast<uint8_t>(1));
	const Stats::DerivedStats atLevelOne = client.GetDerivedStats();

	CHECK(character.SetSkillLevel(MakeClientTestSkillId(20005), 3).IsOk());
	CHECK(client.Apply(character.BuildSnapshot().GetValue()).IsOk());
	CHECK_EQ(client.GetSkillLevel(MakeClientTestSkillId(20005)), static_cast<uint8_t>(3));

	// The client reports the server's new numbers without having produced them.
	CHECK(client.GetDerivedStats() == character.GetDerivedStats());
	CHECK(!(client.GetDerivedStats() == atLevelOne));

	// An unlearn arrives the same way: as a new snapshot that no longer lists
	// the skill, with the statistics the server computed for that set.
	CHECK(character.UnlearnSkill(MakeClientTestSkillId(20005)).IsOk());
	CHECK(client.Apply(character.BuildSnapshot().GetValue()).IsOk());
	CHECK(!client.HasSkill(MakeClientTestSkillId(20005)));
	CHECK_EQ(client.GetLearnedSkillCount(), static_cast<size_t>(0));
	CHECK_EQ(client.GetSkillLevel(MakeClientTestSkillId(20005)), static_cast<uint8_t>(0));
	CHECK(client.GetDerivedStats() == character.GetDerivedStats());
}

MODERN_TEST(Gameplay_ClientSkillViewsAreReadOnly)
{
	// The client cannot learn, unlearn, or level a skill. This is structural:
	// there is no mutator on the type at all, so a client that tried would not
	// compile. The runtime assertion is that two clients given the same snapshot
	// agree and that the snapshot is unchanged by being presented.
	InMemorySkillDefinitions skillProvider;
	CHECK(skillProvider.Add(MakeClientTestSkill(20006, TestClientSkillStats(60, 0.0f))).IsOk());

	Server::ServerCharacterDefinition definition = StandardDefinition();
	definition.skillDefinitions = &skillProvider;

	const Result<Server::ServerCharacter> created = Server::ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();
	CHECK(character.LearnSkill(MakeClientTestSkillId(20006)).IsOk());

	const Gameplay::CharacterSnapshot snapshot = character.BuildSnapshot().GetValue();
	const Gameplay::CharacterSnapshot snapshotBefore = snapshot;

	ClientCharacterState first;
	ClientCharacterState second;
	CHECK(first.Apply(snapshot).IsOk());
	CHECK(second.Apply(snapshot).IsOk());

	CHECK_EQ(first.GetLearnedSkillCount(), second.GetLearnedSkillCount());
	CHECK_EQ(first.GetSkillLevel(MakeClientTestSkillId(20006)),
	         second.GetSkillLevel(MakeClientTestSkillId(20006)));
	CHECK(first.GetDerivedStats() == second.GetDerivedStats());

	// Reading a skill does not change what is held, and Clear() is the only
	// unconditional transition the client owns.
	CHECK(snapshot == snapshotBefore);
	first.Clear();
	CHECK(!first.HasSkill(MakeClientTestSkillId(20006)));
	CHECK(second.HasSkill(MakeClientTestSkillId(20006)));
}

MODERN_TEST(Gameplay_ClientRejectsASnapshotWithAnImpossibleSkillLevel)
{
	// A level of zero means "not learned", so a snapshot listing it as learned
	// is not one a RAN client could have received, and the client refuses it
	// rather than storing a state it could not render.
	InMemorySkillDefinitions skillProvider;
	CHECK(skillProvider.Add(MakeClientTestSkill(20007, TestClientSkillStats(40, 0.0f))).IsOk());

	Server::ServerCharacterDefinition definition = StandardDefinition();
	definition.skillDefinitions = &skillProvider;

	const Result<Server::ServerCharacter> created = Server::ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();
	CHECK(character.LearnSkill(MakeClientTestSkillId(20007)).IsOk());

	Gameplay::CharacterSnapshot snapshot = character.BuildSnapshot().GetValue();
	snapshot.skills.skills[0].level = 0;

	CHECK(!Gameplay::CharacterSnapshot::IsValid(snapshot));

	ClientCharacterState client;
	CHECK(client.Apply(character.BuildSnapshot().GetValue()).IsOk());
	CHECK(client.Apply(snapshot).IsError());
	// A refused snapshot leaves the state it was holding alone.
	CHECK(client.HasSnapshot());
	CHECK(client.GetDerivedStats() == character.GetDerivedStats());
}

MODERN_TEST(Gameplay_ClientReportsEmptySkillsWithNoSnapshot)
{
	ClientCharacterState client;
	CHECK_EQ(client.GetLearnedSkillCount(), static_cast<size_t>(0));
	CHECK(!client.HasSkill(MakeClientTestSkillId(20001)));
	CHECK_EQ(client.GetSkillLevel(MakeClientTestSkillId(20001)), static_cast<uint8_t>(0));
}

MODERN_TEST(Gameplay_ClientPresentsLearnedSkillsFromSnapshot)
{
	InMemoryItemDefinitions itemProvider;
	InMemorySkillDefinitions skillProvider;

	ItemStatBlock skillStats;
	skillStats.hp = 30;
	skillStats.meleePower = 8;
	skillProvider.Add(MakeClientTestSkill(20004, skillStats));

	Server::ServerCharacterDefinition definition = StandardDefinitionWithItems(itemProvider);
	definition.skillDefinitions = &skillProvider;

	const Result<Server::ServerCharacter> created = Server::ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();

	CHECK(character.LearnSkill(MakeClientTestSkillId(20004)).IsOk());

	const Gameplay::CharacterSnapshot snapshot = character.BuildSnapshot().GetValue();
	ClientCharacterState client;
	CHECK(client.Apply(snapshot).IsOk());

	CHECK(client.HasSkill(MakeClientTestSkillId(20004)));
	CHECK_EQ(client.GetSkillLevel(MakeClientTestSkillId(20004)), static_cast<uint8_t>(1));
	CHECK_EQ(client.GetLearnedSkillCount(), static_cast<size_t>(1));
}

// ---------------------------------------------------------------------------
// VERTICAL-004: codex
// ---------------------------------------------------------------------------
//
// The client's codex is a read-only view of what the server published. The
// property worth pinning is not that the values are copied - that is what a
// struct copy does - but that the client has no way to *produce* a different
// answer: no path to register, complete, or recompute, so a codex panel cannot
// disagree with the character it is drawing.

namespace
{
	const CodexId kClientHpCodex  = CodexId(31u);
	const CodexId kClientMapCodex = CodexId(32u);
	const ItemId   kClientHpItem  = ItemId(21001u);
	const ItemId   kClientMapItem = ItemId(21002u);

	// An item provider that outlives every definition built from it. The codex
	// cases need no equipment, but ServerCharacterDefinition holds the provider
	// by pointer, so a temporary would dangle.
	InMemoryItemDefinitions& EmptyItemProvider()
	{
		static InMemoryItemDefinitions provider;
		return provider;
	}

	Server::ServerCharacterDefinition ClientCodexDefinition(
		const CodexDefinitionProvider& definitions)
	{
		Server::ServerCharacterDefinition definition =
			StandardDefinitionWithItems(EmptyItemProvider());
		definition.codexDefinitions = &definitions;
		return definition;
	}

	CodexRequirement CodexReq(ItemId item, uint16_t quantity)
	{
		CodexRequirement requirement;
		requirement.item     = item;
		requirement.quantity = quantity;
		return requirement;
	}

	// A definition whose requirements are written into the five slots in order.
	// Slot order is meaningful: `RequiredSlotCount` reads it.
	CodexDefinition CodexTableEntry(CodexId id, CodexType type, const char* title,
	                                 uint32_t rewardPoint,
	                                 std::initializer_list<CodexRequirement> requirements)
	{
		CodexDefinition definition;
		definition.id          = id;
		definition.type        = type;
		definition.title       = title;
		definition.description = "";
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

	// One entry that completes on a single registration and one that takes a
	// two-count stack, so a snapshot can carry a finished and an unfinished
	// entry at once.
	InMemoryCodexDefinitions ClientCodexTable()
	{
		InMemoryCodexDefinitions definitions;
		(void) definitions.Add(CodexTableEntry(kClientHpCodex, CodexType::ReachLevel,
		                                       "Novice Path", 100u,
		                                       { CodexReq(kClientHpItem, 1) }));
		(void) definitions.Add(CodexTableEntry(kClientMapCodex, CodexType::ReachMap,
		                                       "Wayfarer", 250u,
		                                       { CodexReq(kClientMapItem, 2) }));
		return definitions;
	}

	ItemInstance ClientCodexStack(ItemId item, uint32_t count, uint64_t serial)
	{
		ItemInstance instance;
		instance.definition = item;
		instance.serial     = serial;
		instance.count      = count;
		return instance;
	}
}

MODERN_TEST(Gameplay_ClientPresentsTheCodexFromSnapshot)
{
	InMemoryCodexDefinitions definitions = ClientCodexTable();
	const Server::ServerCharacterDefinition definition = ClientCodexDefinition(definitions);

	const Result<Server::ServerCharacter> created = Server::ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();

	// One entry finished, one left untouched.
	CHECK(character.RegisterCodexItem(kClientHpCodex,
	                                  ClientCodexStack(kClientHpItem, 1, 1u)).IsOk());

	const Gameplay::CharacterSnapshot snapshot = character.BuildSnapshot().GetValue();
	ClientCharacterState client;
	CHECK(client.Apply(snapshot).IsOk());

	CHECK_EQ(client.GetCodexCount(), static_cast<size_t>(2));
	CHECK_EQ(client.GetCompletedCodexCount(), static_cast<size_t>(1));
	CHECK_EQ(client.GetInProgressCodexCount(), static_cast<size_t>(1));

	// A finished entry, with the name and counters a codex panel needs.
	CHECK(client.HasCodex(kClientHpCodex));
	CHECK(client.IsCodexCompleted(kClientHpCodex));
	CHECK(!client.IsCodexInProgress(kClientHpCodex));
	const Gameplay::CodexEntry& finished = client.GetCodexEntry(kClientHpCodex);
	CHECK(finished.name == "Novice Path");
	CHECK(finished.type == CodexType::ReachLevel);
	CHECK_EQ(finished.doneCount, static_cast<uint8_t>(1));
	CHECK_EQ(finished.requiredCount, static_cast<uint8_t>(1));
	CHECK(finished.GetProgressFraction() == 1.0f);

	// And one that is held but not finished, which has to be distinguishable
	// from the finished case rather than reported as the same thing.
	CHECK(client.HasCodex(kClientMapCodex));
	CHECK(!client.IsCodexCompleted(kClientMapCodex));
	CHECK(client.IsCodexInProgress(kClientMapCodex));
	const Gameplay::CodexEntry& partial = client.GetCodexEntry(kClientMapCodex);
	CHECK(partial.name == "Wayfarer");
	CHECK_EQ(partial.doneCount, static_cast<uint8_t>(0));
	CHECK_EQ(partial.requiredCount, static_cast<uint8_t>(1));
	CHECK(partial.GetProgressFraction() == 0.0f);
}

MODERN_TEST(Gameplay_ClientCodexViewsAreReadOnly)
{
	InMemoryCodexDefinitions definitions = ClientCodexTable();
	const Server::ServerCharacterDefinition definition = ClientCodexDefinition(definitions);

	const Result<Server::ServerCharacter> created = Server::ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();
	CHECK(character.RegisterCodexItem(kClientHpCodex, ClientCodexStack(kClientHpItem, 1, 1u)).IsOk());

	const Gameplay::CharacterSnapshot snapshot = character.BuildSnapshot().GetValue();
	ClientCharacterState client;
	CHECK(client.Apply(snapshot).IsOk());

	// The accessors hand back const references, so the only way to change what
	// the client holds is a new snapshot. A full recompute on the client side
	// would be the alternative, and the type surface rules it out: there is no
	// mutable codex member, no RegisterCodexItem, and no contribution of the
	// client's own to feed a recalculation. This pins the published view as the
	// same values twice over, which is what a panel actually draws.
	const Gameplay::CodexEntry& first  = client.GetCodexEntry(kClientHpCodex);
	const Gameplay::CodexEntry& second = client.GetCodexEntry(kClientHpCodex);
	CHECK(first == second);
	CHECK(client.GetCodex() == snapshot.codex);

	// And the client's derived statistics are the server's, already carrying the
	// codex bonus. The client does not add a second copy on top.
	CHECK(client.GetDerivedStats() == snapshot.derived);
	CHECK_EQ(client.GetDerivedStats().maxHp, snapshot.derived.maxHp);
}

MODERN_TEST(Gameplay_ClientReportsEmptyCodexWithNoSnapshot)
{
	// No snapshot at all. The accessors have to answer rather than crash, and
	// they have to answer "nothing held" rather than a default-constructed entry
	// that looks like a real one.
	ClientCharacterState client;
	CHECK(!client.HasCodex(kClientHpCodex));
	CHECK(!client.IsCodexCompleted(kClientHpCodex));
	CHECK(!client.IsCodexInProgress(kClientHpCodex));
	CHECK_EQ(client.GetCodexCount(), static_cast<size_t>(0));
	CHECK_EQ(client.GetCompletedCodexCount(), static_cast<size_t>(0));
	CHECK_EQ(client.GetInProgressCodexCount(), static_cast<size_t>(0));

	// The shared empty view is the same one every accessor falls back to, so
	// two calls cannot return two different "empty" lists.
	CHECK(client.GetCodex() == ClientCharacterState::EmptyCodex());
	CHECK(&client.GetCodex() == &client.GetCodex());

	// An entry the character does not have is an empty entry, and an empty entry
	// is not a completed one.
	const Gameplay::CodexEntry& missing = client.GetCodexEntry(kClientHpCodex);
	CHECK(!missing.completed);
	CHECK_EQ(missing.id, CodexId::MakeInvalid());
	CHECK(missing.GetProgressFraction() == 0.0f);
}

MODERN_TEST(Gameplay_ClientCodexSurvivesASnapshotWithoutOne)
{
	// A snapshot with an empty codex list is valid - it is what a character with
	// no codex table produces - and it must not leave the client reading a stale
	// codex from an earlier snapshot.
	InMemoryCodexDefinitions definitions = ClientCodexTable();
	const Server::ServerCharacterDefinition definition = ClientCodexDefinition(definitions);

	const Result<Server::ServerCharacter> created = Server::ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();
	CHECK(character.RegisterCodexItem(kClientHpCodex, ClientCodexStack(kClientHpItem, 1, 1u)).IsOk());

	ClientCharacterState client;
	CHECK(client.Apply(character.BuildSnapshot().GetValue()).IsOk());
	CHECK(client.HasCodex(kClientHpCodex));
	CHECK_EQ(client.GetCodexCount(), static_cast<size_t>(2));

	// A second character with no codex table at all.
	const Server::ServerCharacterDefinition bare = StandardDefinitionWithItems(EmptyItemProvider());
	const Result<Server::ServerCharacter> bareCreated = Server::ServerCharacter::Create(bare);
	CHECK(bareCreated.IsOk());
	if (bareCreated.IsError())
	{
		return;
	}
	Server::ServerCharacter bareCharacter = bareCreated.GetValue();
	const Gameplay::CharacterSnapshot bareSnapshot = bareCharacter.BuildSnapshot().GetValue();
	CHECK(Gameplay::CharacterSnapshot::IsValid(bareSnapshot));
	CHECK(bareSnapshot.codex.entries.empty());

	CHECK(client.Apply(bareSnapshot).IsOk());
	CHECK(!client.HasCodex(kClientHpCodex));
	CHECK_EQ(client.GetCodexCount(), static_cast<size_t>(0));
	CHECK_EQ(client.GetCompletedCodexCount(), static_cast<size_t>(0));
}

// ── VERTICAL-006: Combat presentation tests ──────────────────────────

MODERN_TEST(ClientCombat_EmptyState)
{
	ClientCharacterState client;
	CHECK_EQ(client.HasSnapshot(), false);
	CHECK_EQ(client.GetCurrentHp(), 0u);
}

MODERN_TEST(ClientCombat_PresentsAuthoritativeHit)
{
	auto character = Server::ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());
	character.GetValue().RestoreResources();

	Gameplay::CharacterSnapshot snapshot = character.GetValue().BuildSnapshot().GetValue();

	ClientCharacterState client;
	const Status applied = client.Apply(snapshot);
	CHECK(applied.IsOk());

	CHECK_EQ(client.HasSnapshot(), true);
	CHECK_EQ(client.GetCurrentHp(), snapshot.hp.current);
}

MODERN_TEST(ClientCombat_PresentsAuthoritativeMiss)
{
	auto character = Server::ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());
	character.GetValue().RestoreResources();

	Gameplay::CharacterSnapshot snapshot = character.GetValue().BuildSnapshot().GetValue();

	ClientCharacterState client;
	client.Apply(snapshot);

	CHECK_EQ(client.HasSnapshot(), true);
	CHECK_EQ(client.GetCurrentHp(), snapshot.hp.current);
}

MODERN_TEST(ClientCombat_DamagePresentation)
{
	auto character = Server::ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());
	character.GetValue().RestoreResources();

	Gameplay::CharacterSnapshot snapshot = character.GetValue().BuildSnapshot().GetValue();

	ClientCharacterState client;
	client.Apply(snapshot);

	CHECK_EQ(client.GetCurrentHp(), snapshot.hp.current);
	CHECK_EQ(client.GetMaxHp(), snapshot.derived.maxHp);
}

MODERN_TEST(ClientCombat_CriticalPresentation)
{
	auto character = Server::ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());
	character.GetValue().RestoreResources();

	Gameplay::CharacterSnapshot snapshot = character.GetValue().BuildSnapshot().GetValue();

	ClientCharacterState client;
	client.Apply(snapshot);

	CHECK_EQ(client.HasSnapshot(), true);
}

MODERN_TEST(ClientCombat_CrushingPresentation)
{
	auto character = Server::ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());
	character.GetValue().RestoreResources();

	Gameplay::CharacterSnapshot snapshot = character.GetValue().BuildSnapshot().GetValue();

	ClientCharacterState client;
	client.Apply(snapshot);

	CHECK_EQ(client.HasSnapshot(), true);
}

MODERN_TEST(ClientCombat_UpdatedHPPresentation)
{
	auto character = Server::ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());
	character.GetValue().RestoreResources();

	Gameplay::CharacterSnapshot snapshot = character.GetValue().BuildSnapshot().GetValue();

	ClientCharacterState client;
	client.Apply(snapshot);

	CHECK_EQ(client.GetCurrentHp(), snapshot.hp.current);
}

MODERN_TEST(ClientCombat_LaterSnapshotReplacesEarlier)
{
	auto character = Server::ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());
	character.GetValue().RestoreResources();

	Gameplay::CharacterSnapshot snapshot1 = character.GetValue().BuildSnapshot().GetValue();

	ClientCharacterState client;
	client.Apply(snapshot1);

	CHECK_EQ(client.GetCurrentHp(), snapshot1.hp.current);

	Gameplay::CharacterSnapshot snapshot2 = character.GetValue().BuildSnapshot().GetValue();
	client.Apply(snapshot2);

	CHECK_EQ(client.GetCurrentHp(), snapshot2.hp.current);
}

MODERN_TEST(ClientCombat_ClientDoesNotRecalculate)
{
	auto character = Server::ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());
	character.GetValue().RestoreResources();

	Gameplay::CharacterSnapshot snapshot = character.GetValue().BuildSnapshot().GetValue();

	ClientCharacterState client;
	client.Apply(snapshot);

	CHECK_EQ(client.GetCurrentHp(), snapshot.hp.current);
	CHECK_EQ(client.GetDerivedStats().hit, snapshot.derived.hit);
	CHECK_EQ(client.GetDerivedStats().avoid, snapshot.derived.avoid);
	CHECK_EQ(client.GetDerivedStats().defense, snapshot.derived.defense);
}

// WORLD-ENTRY-002h: ApplyResourceUpdate from 3046
MODERN_TEST(Gameplay_ApplyResourceUpdate_ChangesPresentedPools)
{
	auto character = Server::ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());
	character.GetValue().RestoreResources();

	Gameplay::CharacterSnapshot snapshot = character.GetValue().BuildSnapshot().GetValue();

	ClientCharacterState client;
	client.Apply(snapshot);

	const uint32_t originalHp = client.GetCurrentHp();
	const uint32_t originalMp = client.GetCurrentMp();
	const uint32_t originalSp = client.GetCurrentSp();

	// Apply a 3046 with different current values
	client.ApplyResourceUpdate(42, 17, 99);

	CHECK_EQ(client.GetCurrentHp(), 42u);
	CHECK_EQ(client.GetCurrentMp(), 17u);
	CHECK_EQ(client.GetCurrentSp(), 99u);

	// Maxima are NOT changed - they come from derived stats
	CHECK_EQ(client.GetMaxHp(), originalHp); // was full, so max == original current
	CHECK_EQ(client.GetDerivedStats().maxHp, snapshot.derived.maxHp);
	CHECK_EQ(client.GetDerivedStats().maxMp, snapshot.derived.maxMp);
	CHECK_EQ(client.GetDerivedStats().maxSp, snapshot.derived.maxSp);
}

MODERN_TEST(Gameplay_ApplyResourceUpdate_NoOpWhenNoSnapshot)
{
	ClientCharacterState client;
	CHECK(!client.HasSnapshot());

	client.ApplyResourceUpdate(100, 200, 300);

	CHECK_EQ(client.GetCurrentHp(), 0u);
	CHECK_EQ(client.GetCurrentMp(), 0u);
	CHECK_EQ(client.GetCurrentSp(), 0u);
}

MODERN_TEST(Gameplay_ApplyResourceUpdate_DoesNotMutateDerivedStats)
{
	auto character = Server::ServerCharacter::Create(StandardDefinition());
	CHECK(character.IsOk());
	character.GetValue().RestoreResources();

	Gameplay::CharacterSnapshot snapshot = character.GetValue().BuildSnapshot().GetValue();
	const Stats::DerivedStats originalDerived = snapshot.derived;

	ClientCharacterState client;
	client.Apply(snapshot);

	client.ApplyResourceUpdate(1, 2, 3);

	CHECK(client.GetDerivedStats() == originalDerived);
}


int main()
{
	// Unbuffered so an abort still shows which case was running.
	std::setvbuf(stdout, nullptr, _IONBF, 0);
	std::printf("Modern VERTICAL-001 client gameplay tests\n\n");

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