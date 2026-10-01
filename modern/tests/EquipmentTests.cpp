// VERTICAL-002: equipment state, item definitions, and contribution aggregation.
//
// Headless. Links Modern and nothing else: no renderer, no socket, no database,
// no legacy library, no server, no client.

#include "TestHarness.h"

#include "equipment/EquipmentState.h"
#include "equipment/ItemContributionAggregator.h"
#include "equipment/ItemDefinitionProvider.h"
#include "item/ItemDefinition.h"
#include "item/ItemInstance.h"
#include "stats/Contributions.h"

#include <cmath>
#include <limits>

using namespace Modern;

namespace
{
	ItemId Id(uint32_t v) { return ItemId(v); }

	// A provider that returns definitions without going through Add's validation,
	// so the aggregator's defense-in-depth checks can be exercised in isolation.
	class DirectProvider final : public ItemDefinitionProvider
	{
	public:
		ItemDefinition definition;

		const ItemDefinition* Find(ItemId id) const override
		{
			if (id == definition.id)
			{
				return &definition;
			}
			return nullptr;
		}
	};

	ItemInstance MakeInstance(uint32_t defId, uint64_t serial = 1)
	{
		ItemInstance item;
		item.definition = Id(defId);
		item.serial = serial;
		item.count = 1;
		return item;
	}

	ItemDefinition MakeWeapon(uint32_t id, const ItemStatBlock& stats)
	{
		ItemDefinition def;
		def.id = Id(id);
		def.kind = ItemKind::Weapon;
		def.name = "Weapon " + std::to_string(id);
		def.maxStack = 1;
		def.stats = stats;
		return def;
	}

	ItemDefinition MakeArmor(uint32_t id, const ItemStatBlock& stats)
	{
		ItemDefinition def;
		def.id = Id(id);
		def.kind = ItemKind::Armor;
		def.name = "Armor " + std::to_string(id);
		def.maxStack = 1;
		def.stats = stats;
		return def;
	}

	ItemDefinition MakeConsumable(uint32_t id)
	{
		ItemDefinition def;
		def.id = Id(id);
		def.kind = ItemKind::Consumable;
		def.name = "Potion";
		def.maxStack = 10;
		return def;
	}

	ItemDefinition MakeAccessory(uint32_t id, const ItemStatBlock& stats)
	{
		ItemDefinition def;
		def.id = Id(id);
		def.kind = ItemKind::Accessory;
		def.name = "Accessory " + std::to_string(id);
		def.maxStack = 1;
		def.stats = stats;
		return def;
	}
}

// ---------------------------------------------------------------------------
// EquipmentState
// ---------------------------------------------------------------------------

MODERN_TEST(EquipmentState_StartsEmpty)
{
	EquipmentState state;
	CHECK_EQ(state.GetOccupiedCount(), static_cast<size_t>(0));

	for (uint8_t i = 0; i < kEquipmentSlotCount; ++i)
	{
		const EquipmentSlot slot = static_cast<EquipmentSlot>(i);
		CHECK(!state.HasEquipped(slot));
	}
}

MODERN_TEST(EquipmentState_EquipPutsItemInSlot)
{
	EquipmentState state;
	const ItemInstance item = MakeInstance(100, 5);

	CHECK(state.Equip(EquipmentSlot::Headgear, item).IsOk());
	CHECK(state.HasEquipped(EquipmentSlot::Headgear));
	CHECK_EQ(state.GetOccupiedCount(), static_cast<size_t>(1));

	const EquipmentEntry entry = state.Get(EquipmentSlot::Headgear);
	CHECK(entry.item == item);
}

MODERN_TEST(EquipmentState_EquipReplacesExistingItem)
{
	EquipmentState state;
	const ItemInstance first  = MakeInstance(100, 1);
	const ItemInstance second = MakeInstance(200, 2);

	CHECK(state.Equip(EquipmentSlot::Headgear, first).IsOk());
	CHECK(state.Equip(EquipmentSlot::Headgear, second).IsOk());

	CHECK_EQ(state.GetOccupiedCount(), static_cast<size_t>(1));
	const EquipmentEntry entry = state.Get(EquipmentSlot::Headgear);
	CHECK(entry.item == second);
}

MODERN_TEST(EquipmentState_EquipRejectsInvalidSlot)
{
	EquipmentState state;
	const ItemInstance item = MakeInstance(100);

	const EquipmentSlot bad = static_cast<EquipmentSlot>(kEquipmentSlotCount);
	CHECK(state.Equip(bad, item).IsError());
	CHECK_EQ(state.GetOccupiedCount(), static_cast<size_t>(0));
}

MODERN_TEST(EquipmentState_EquipRejectsInvalidItem)
{
	EquipmentState state;
	const ItemInstance invalid;

	CHECK(state.Equip(EquipmentSlot::Headgear, invalid).IsError());
	CHECK_EQ(state.GetOccupiedCount(), static_cast<size_t>(0));
}

MODERN_TEST(EquipmentState_UnequipClearsSlot)
{
	EquipmentState state;
	const ItemInstance item = MakeInstance(100);

	CHECK(state.Equip(EquipmentSlot::Neck, item).IsOk());
	CHECK(state.HasEquipped(EquipmentSlot::Neck));

	CHECK(state.Unequip(EquipmentSlot::Neck).IsOk());
	CHECK(!state.HasEquipped(EquipmentSlot::Neck));
	CHECK_EQ(state.GetOccupiedCount(), static_cast<size_t>(0));
}

MODERN_TEST(EquipmentState_UnequipEmptySlotIsSuccess)
{
	EquipmentState state;
	CHECK(state.Unequip(EquipmentSlot::Headgear).IsOk());
}

MODERN_TEST(EquipmentState_UnequipRejectsInvalidSlot)
{
	EquipmentState state;
	const EquipmentSlot bad = static_cast<EquipmentSlot>(kEquipmentSlotCount);
	CHECK(state.Unequip(bad).IsError());
}

MODERN_TEST(EquipmentState_FillAndClearAllSlots)
{
	EquipmentState state;

	for (uint8_t i = 0; i < kEquipmentSlotCount; ++i)
	{
		const EquipmentSlot slot = static_cast<EquipmentSlot>(i);
		CHECK(state.Equip(slot, MakeInstance(1000 + i, i + 1)).IsOk());
	}
	CHECK_EQ(state.GetOccupiedCount(), kEquipmentSlotCount);

	state.Clear();
	CHECK_EQ(state.GetOccupiedCount(), static_cast<size_t>(0));
}

MODERN_TEST(EquipmentState_GetOutOfRangeReturnsEmpty)
{
	EquipmentState state;
	const EquipmentEntry entry = state.Get(static_cast<EquipmentSlot>(99));
	CHECK(!entry.HasItem());
}

// ---------------------------------------------------------------------------
// InMemoryItemDefinitions
// ---------------------------------------------------------------------------

MODERN_TEST(ItemDefinitions_FindReturnsNullForUnknown)
{
	InMemoryItemDefinitions provider;
	CHECK(provider.Find(Id(42)) == nullptr);
	CHECK_EQ(provider.GetCount(), static_cast<size_t>(0));
}

MODERN_TEST(ItemDefinitions_AddRegistersValidDefinition)
{
	InMemoryItemDefinitions provider;

	ItemStatBlock stats;
	stats.hp = 50;
	const ItemDefinition def = MakeWeapon(1001, stats);

	CHECK(provider.Add(def).IsOk());
	CHECK_EQ(provider.GetCount(), static_cast<size_t>(1));

	const ItemDefinition* found = provider.Find(Id(1001));
	CHECK(found != nullptr);
	CHECK_EQ(found->name, "Weapon 1001");
	CHECK_EQ(found->kind, ItemKind::Weapon);
}

MODERN_TEST(ItemDefinitions_AddRejectsInvalidDefinition)
{
	InMemoryItemDefinitions provider;

	ItemDefinition invalid;
	invalid.id = Id(2001);
	invalid.kind = ItemKind::Weapon;
	// name is empty — invalid
	CHECK(provider.Add(invalid).IsError());
	CHECK_EQ(provider.GetCount(), static_cast<size_t>(0));
}

MODERN_TEST(ItemDefinitions_AddReplacesExisting)
{
	InMemoryItemDefinitions provider;

	ItemStatBlock v1;
	v1.hp = 10;
	CHECK(provider.Add(MakeWeapon(3001, v1)).IsOk());

	ItemStatBlock v2;
	v2.hp = 99;
	CHECK(provider.Add(MakeWeapon(3001, v2)).IsOk());

	CHECK_EQ(provider.GetCount(), static_cast<size_t>(1));
	const ItemDefinition* found = provider.Find(Id(3001));
	CHECK(found != nullptr);
	CHECK_EQ(found->stats.hp, 99);
}

MODERN_TEST(ItemDefinitions_AddRejectsNonFiniteStats)
{
	InMemoryItemDefinitions provider;

	ItemStatBlock stats;
	stats.hitPercent = std::numeric_limits<float>::quiet_NaN();
	CHECK(provider.Add(MakeWeapon(6001, stats)).IsError());
	CHECK_EQ(provider.GetCount(), static_cast<size_t>(0));
}

MODERN_TEST(ItemDefinitions_RemoveExistingSucceeds)
{
	InMemoryItemDefinitions provider;

	ItemStatBlock stats;
	stats.str = 5;
	CHECK(provider.Add(MakeArmor(4001, stats)).IsOk());
	CHECK_EQ(provider.GetCount(), static_cast<size_t>(1));

	CHECK(provider.Remove(Id(4001)).IsOk());
	CHECK_EQ(provider.GetCount(), static_cast<size_t>(0));
	CHECK(provider.Find(Id(4001)) == nullptr);
}

MODERN_TEST(ItemDefinitions_RemoveUnknownReturnsNotFound)
{
	InMemoryItemDefinitions provider;
	CHECK_EQ(provider.Remove(Id(9999)).GetCode(), ErrorCode::NotFound);
}

MODERN_TEST(ItemDefinitions_FindInvalidIdReturnsNull)
{
	InMemoryItemDefinitions provider;
	CHECK(provider.Find(ItemId::MakeInvalid()) == nullptr);
}

MODERN_TEST(ItemDefinitions_ClearEmptiesEverything)
{
	InMemoryItemDefinitions provider;

	CHECK(provider.Add(MakeWeapon(5001, ItemStatBlock())).IsOk());
	CHECK(provider.Add(MakeArmor(5002, ItemStatBlock())).IsOk());
	CHECK(provider.Add(MakeAccessory(5003, ItemStatBlock())).IsOk());
	CHECK_EQ(provider.GetCount(), static_cast<size_t>(3));

	provider.Clear();
	CHECK_EQ(provider.GetCount(), static_cast<size_t>(0));
}

// ---------------------------------------------------------------------------
// ItemContributionAggregator
// ---------------------------------------------------------------------------

MODERN_TEST(Aggregator_EmptyEquipmentYieldsZero)
{
	InMemoryItemDefinitions provider;
	EquipmentState          equipment;

	const Result<ItemContributionResult> result =
		ItemContributionAggregator::Aggregate(equipment, provider);

	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().error, ContributionError::None);
	CHECK_EQ(result.GetValue().contributingSlots, static_cast<size_t>(0));

	const Stats::ItemContribution emptyContribution;
	CHECK(result.GetValue().contribution == emptyContribution);
}

MODERN_TEST(Aggregator_AggregatesASingleItem)
{
	InMemoryItemDefinitions provider;

	ItemStatBlock stats;
	stats.hp = 100;
	stats.str = 15;
	stats.hitPercent = 10.0f;
	provider.Add(MakeWeapon(7001, stats));

	EquipmentState equipment;
	equipment.Equip(EquipmentSlot::RightHand, MakeInstance(7001, 1));

	const Result<ItemContributionResult> result =
		ItemContributionAggregator::Aggregate(equipment, provider);

	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().error, ContributionError::None);
	CHECK_EQ(result.GetValue().contributingSlots, static_cast<size_t>(1));

	const Stats::ItemContribution& c = result.GetValue().contribution;
	CHECK_EQ(c.hp, 100);
	CHECK_EQ(c.stats.str, static_cast<uint16_t>(15));
	CHECK_EQ(c.hitRatePercent, 10.0f);
}

MODERN_TEST(Aggregator_SumsMultipleItems)
{
	InMemoryItemDefinitions provider;

	ItemStatBlock swordStats;
	swordStats.hp = 50;
	swordStats.meleePower = 8;
	provider.Add(MakeWeapon(8001, swordStats));

	ItemStatBlock armorStats;
	armorStats.str = 20;
	armorStats.hp = 30;
	armorStats.defense = 12;
	provider.Add(MakeArmor(8002, armorStats));

	ItemStatBlock ringStats;
	ringStats.dex = 5;
	ringStats.hitPercent = 3.0f;
	provider.Add(MakeAccessory(8003, ringStats));

	EquipmentState equipment;
	equipment.Equip(EquipmentSlot::RightHand,    MakeInstance(8001, 1));
	equipment.Equip(EquipmentSlot::Headgear,     MakeInstance(8002, 2));
	equipment.Equip(EquipmentSlot::RightFinger,   MakeInstance(8003, 3));

	const Result<ItemContributionResult> result =
		ItemContributionAggregator::Aggregate(equipment, provider);

	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().error, ContributionError::None);
	CHECK_EQ(result.GetValue().contributingSlots, static_cast<size_t>(3));

	const Stats::ItemContribution& c = result.GetValue().contribution;
	CHECK_EQ(c.hp, 80);            // 50 + 30
	CHECK_EQ(c.stats.str, static_cast<uint16_t>(20));
	CHECK_EQ(c.stats.dex, static_cast<uint16_t>(5));
	CHECK_EQ(c.meleePower, 8);
	CHECK_EQ(c.defense, 12);
	CHECK_EQ(c.hitRatePercent, 3.0f);
}

MODERN_TEST(Aggregator_MissingDefinitionIsError)
{
	InMemoryItemDefinitions provider;
	// Note: the item definition for id 9001 is NOT registered.

	EquipmentState equipment;
	equipment.Equip(EquipmentSlot::Headgear, MakeInstance(9001, 1));

	const Result<ItemContributionResult> result =
		ItemContributionAggregator::Aggregate(equipment, provider);

	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().error, ContributionError::MissingDefinition);
}

MODERN_TEST(Aggregator_NonFiniteStatBlockIsError)
{
	// InMemoryItemDefinitions rejects non-finite definitions at Add time, so a
	// DirectProvider bypasses that gate to exercise the aggregator's own
	// defense-in-depth check.
	DirectProvider provider;
	provider.definition.id = Id(9101);
	provider.definition.kind = ItemKind::Weapon;
	provider.definition.name = "BadWeapon";
	provider.definition.maxStack = 1;
	provider.definition.stats.hitPercent = std::numeric_limits<float>::quiet_NaN();

	EquipmentState equipment;
	equipment.Equip(EquipmentSlot::RightHand, MakeInstance(9101, 1));

	const Result<ItemContributionResult> result =
		ItemContributionAggregator::Aggregate(equipment, provider);

	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().error, ContributionError::NonFinite);
}

MODERN_TEST(Aggregator_ConsumableItemIsSkipped)
{
	InMemoryItemDefinitions provider;
	// A consumable is not equipment, so it contributes nothing and is not an error.
	provider.Add(MakeConsumable(9201));

	EquipmentState equipment;
	equipment.Equip(EquipmentSlot::Headgear, MakeInstance(9201, 1));

	const Result<ItemContributionResult> result =
		ItemContributionAggregator::Aggregate(equipment, provider);

	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().error, ContributionError::None);
	CHECK_EQ(result.GetValue().contributingSlots, static_cast<size_t>(0));
	CHECK_EQ(result.GetValue().contribution.hp, 0);
}

MODERN_TEST(Aggregator_ZeroStatEquipmentIsSkipped)
{
	InMemoryItemDefinitions provider;
	// An armor piece with all-zero stats: valid equipment but contributes nothing.
	provider.Add(MakeArmor(9301, ItemStatBlock()));

	EquipmentState equipment;
	equipment.Equip(EquipmentSlot::Headgear, MakeInstance(9301, 1));

	const Result<ItemContributionResult> result =
		ItemContributionAggregator::Aggregate(equipment, provider);

	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().error, ContributionError::None);
	CHECK_EQ(result.GetValue().contributingSlots, static_cast<size_t>(0));
	CHECK_EQ(result.GetValue().contribution.hp, 0);
}

MODERN_TEST(Aggregator_DeterministicSlotOrder)
{
	InMemoryItemDefinitions provider;

	ItemStatBlock stats;
	stats.hp = 10;
	provider.Add(MakeWeapon(9401, stats));

	EquipmentState equipment;

	// Equip the same item id in two different slots, then check that the result
	// is the same regardless of which was equipped first.
	EquipmentState orderA;
	orderA.Equip(EquipmentSlot::RightHand, MakeInstance(9401, 1));
	orderA.Equip(EquipmentSlot::LeftHand,  MakeInstance(9401, 2));

	EquipmentState orderB;
	orderB.Equip(EquipmentSlot::LeftHand,  MakeInstance(9401, 2));
	orderB.Equip(EquipmentSlot::RightHand, MakeInstance(9401, 1));

	const auto a = ItemContributionAggregator::Aggregate(orderA, provider).GetValue();
	const auto b = ItemContributionAggregator::Aggregate(orderB, provider).GetValue();

	CHECK(a.contribution == b.contribution);
	CHECK_EQ(a.contributingSlots, static_cast<size_t>(2));
}

// ---------------------------------------------------------------------------
// VERTICAL-010: required SP aggregation
//
// Legacy: GLogixExPC.cpp:430-434
//     SITEM* pRHAND = GET_SLOT_ITEMDATA ( emRHand );
//     SITEM* pLHAND = GET_SLOT_ITEMDATA ( emLHand );
//     if ( pRHAND )  m_wSUM_DisSP += pRHAND->sSuitOp.wReqSP;
//     if ( pLHAND )  m_wSUM_DisSP += pLHAND->sSuitOp.wReqSP;
//
// The two guards are the rule. Every test here is about which slots are read,
// because summing all 21 would be wrong and would not reproduce RAN.
// ---------------------------------------------------------------------------

namespace
{
	// A definition whose only non-zero stat is the required-SP cost. That is
	// the case that fails if `IsZero()` forgets the new field: the aggregator
	// would skip it as a non-contributor before ever reading the slot.
	ItemDefinition MakeRequiredSPWeapon(uint32_t id, uint16_t requiredSP)
	{
		ItemStatBlock stats;
		stats.requiredSP = requiredSP;
		return MakeWeapon(id, stats);
	}
}

MODERN_TEST(RequiredSP_NoHandsContributesZero)
{
	InMemoryItemDefinitions provider;
	provider.Add(MakeRequiredSPWeapon(9501, 20));
	provider.Add(MakeRequiredSPWeapon(9502, 10));

	EquipmentState equipment;

	const auto result = ItemContributionAggregator::Aggregate(equipment, provider);

	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().error, ContributionError::None);
	CHECK_EQ(result.GetValue().contribution.requiredSP, static_cast<uint16_t>(0));
}

MODERN_TEST(RequiredSP_RightHandOnly)
{
	InMemoryItemDefinitions provider;
	provider.Add(MakeRequiredSPWeapon(9501, 20));
	provider.Add(MakeRequiredSPWeapon(9502, 10));

	EquipmentState equipment;
	equipment.Equip(EquipmentSlot::RightHand, MakeInstance(9501, 1));

	const auto result = ItemContributionAggregator::Aggregate(equipment, provider);

	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.requiredSP, static_cast<uint16_t>(20));
}

MODERN_TEST(RequiredSP_LeftHandOnly)
{
	InMemoryItemDefinitions provider;
	provider.Add(MakeRequiredSPWeapon(9501, 20));
	provider.Add(MakeRequiredSPWeapon(9502, 10));

	EquipmentState equipment;
	equipment.Equip(EquipmentSlot::LeftHand, MakeInstance(9502, 1));

	const auto result = ItemContributionAggregator::Aggregate(equipment, provider);

	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.requiredSP, static_cast<uint16_t>(10));
}

MODERN_TEST(RequiredSP_BothHandsSum)
{
	InMemoryItemDefinitions provider;
	provider.Add(MakeRequiredSPWeapon(9501, 20));
	provider.Add(MakeRequiredSPWeapon(9502, 10));

	EquipmentState equipment;
	equipment.Equip(EquipmentSlot::RightHand, MakeInstance(9501, 1));
	equipment.Equip(EquipmentSlot::LeftHand,  MakeInstance(9502, 2));

	const auto result = ItemContributionAggregator::Aggregate(equipment, provider);

	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.requiredSP, static_cast<uint16_t>(30));
}

// The point of the two hand guards: every other worn slot must be ignored even
// when its definition carries a required-SP cost.
MODERN_TEST(RequiredSP_NonHandSlotsDoNotContribute)
{
	InMemoryItemDefinitions provider;

	// One definition, equipped in slots that are not hands. Because
	// `MakeRequiredSPWeapon` yields a single id, the same cost is carried by
	// every slot here.
	provider.Add(MakeRequiredSPWeapon(9501, 25));

	EquipmentState equipment;
	equipment.Equip(EquipmentSlot::Headgear, MakeInstance(9501, 1));
	equipment.Equip(EquipmentSlot::Upper,    MakeInstance(9501, 2));
	equipment.Equip(EquipmentSlot::Lower,    MakeInstance(9501, 3));
	equipment.Equip(EquipmentSlot::Hand,     MakeInstance(9501, 4));
	equipment.Equip(EquipmentSlot::Foot,     MakeInstance(9501, 5));
	equipment.Equip(EquipmentSlot::Neck,     MakeInstance(9501, 6));
	equipment.Equip(EquipmentSlot::Wrist,    MakeInstance(9501, 7));
	equipment.Equip(EquipmentSlot::Ornament, MakeInstance(9501, 8));
	equipment.Equip(EquipmentSlot::Misc,     MakeInstance(9501, 9));

	const auto result = ItemContributionAggregator::Aggregate(equipment, provider);

	CHECK(result.IsOk());
	// The items are still worn and still counted as contributing slots, they
	// simply carry nothing into the required-SP total.
	CHECK_EQ(result.GetValue().contribution.requiredSP, static_cast<uint16_t>(0));
	CHECK_EQ(result.GetValue().contributingSlots, static_cast<size_t>(9));
}

MODERN_TEST(RequiredSP_HandPlusOtherSlotsStillOnlyHands)
{
	InMemoryItemDefinitions provider;
	provider.Add(MakeRequiredSPWeapon(9501, 20));
	provider.Add(MakeRequiredSPWeapon(9502, 30));

	EquipmentState equipment;
	equipment.Equip(EquipmentSlot::RightHand, MakeInstance(9501, 1));
	equipment.Equip(EquipmentSlot::Upper,      MakeInstance(9502, 2));

	const auto result = ItemContributionAggregator::Aggregate(equipment, provider);

	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.requiredSP, static_cast<uint16_t>(20));
}

// A required-SP-only weapon still aggregates its cost. If `IsZero()` ignored
// the field this would read 0, because the aggregator skips a block it
// considers empty before it reaches the hand-slot test.
MODERN_TEST(RequiredSP_OnlyStatStillAggregates)
{
	InMemoryItemDefinitions provider;

	ItemStatBlock stats;
	stats.requiredSP = 7;
	// Nothing else set.
	provider.Add(MakeWeapon(9503, stats));

	EquipmentState equipment;
	equipment.Equip(EquipmentSlot::RightHand, MakeInstance(9503, 1));

	const auto result = ItemContributionAggregator::Aggregate(equipment, provider);

	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.requiredSP, static_cast<uint16_t>(7));
	CHECK_EQ(result.GetValue().contributingSlots, static_cast<size_t>(1));
}

// A required-SP cost is a property of equipment. A consumable in a hand slot
// is skipped exactly as any other non-equipment item is.
MODERN_TEST(RequiredSP_NonEquipmentSlotIgnored)
{
	InMemoryItemDefinitions provider;

	ItemDefinition potion;
	potion.id = Id(9504);
	potion.kind = ItemKind::Consumable;
	potion.name = "Potion";
	potion.maxStack = 10;
	potion.stats.requiredSP = 40;
	provider.Add(potion);

	EquipmentState equipment;
	equipment.Equip(EquipmentSlot::RightHand, MakeInstance(9504, 1));

	const auto result = ItemContributionAggregator::Aggregate(equipment, provider);

	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.requiredSP, static_cast<uint16_t>(0));
}

// 16-bit wrap, because legacy sums into a WORD and so does this. 60000 + 60000
// is 120000, which is 4464 past the wrap.
MODERN_TEST(RequiredSP_SumsWithWordWrap)
{
	InMemoryItemDefinitions provider;
	provider.Add(MakeRequiredSPWeapon(9505, 60000));
	provider.Add(MakeRequiredSPWeapon(9506, 60000));

	EquipmentState equipment;
	equipment.Equip(EquipmentSlot::RightHand, MakeInstance(9505, 1));
	equipment.Equip(EquipmentSlot::LeftHand,  MakeInstance(9506, 2));

	const auto result = ItemContributionAggregator::Aggregate(equipment, provider);

	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.requiredSP, static_cast<uint16_t>(120000 - 65536));
}

MODERN_TEST(RequiredSP_UnequipRemovesCost)
{
	InMemoryItemDefinitions provider;
	provider.Add(MakeRequiredSPWeapon(9501, 20));
	provider.Add(MakeRequiredSPWeapon(9502, 10));

	EquipmentState equipment;
	equipment.Equip(EquipmentSlot::RightHand, MakeInstance(9501, 1));
	equipment.Equip(EquipmentSlot::LeftHand,  MakeInstance(9502, 2));

	const auto both = ItemContributionAggregator::Aggregate(equipment, provider);
	CHECK_EQ(both.GetValue().contribution.requiredSP, static_cast<uint16_t>(30));

	equipment.Unequip(EquipmentSlot::RightHand);

	const auto remaining = ItemContributionAggregator::Aggregate(equipment, provider);
	CHECK_EQ(remaining.GetValue().contribution.requiredSP, static_cast<uint16_t>(10));
}

// Required SP and the ordinary stat sum are independent: the hand guard applies
// only to the required-SP term.
MODERN_TEST(RequiredSP_DoesNotDisturbOtherStats)
{
	InMemoryItemDefinitions provider;

	ItemStatBlock stats;
	stats.requiredSP = 20;
	stats.hp = 50;
	provider.Add(MakeWeapon(9507, stats));

	EquipmentState equipment;
	equipment.Equip(EquipmentSlot::RightHand, MakeInstance(9507, 1));

	const auto result = ItemContributionAggregator::Aggregate(equipment, provider);

	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.requiredSP, static_cast<uint16_t>(20));
	CHECK_EQ(result.GetValue().contribution.hp, 50);
}
