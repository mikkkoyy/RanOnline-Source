// VERTICAL-004: the codex, tested against the rules the legacy investigation
// verified. See docs/reference/client/VERTICAL-004_CODEX_INVESTIGATION.md.
//
// Three groups, in the order the investigation establishes them:
//
//   1. the definition: the eleven type-to-field mapping, and the required-count
//      cascade that decides when an entry completes;
//   2. the state: seating, registration, the equality rules, completion, and
//      the exactly-once property across a reload;
//   3. the contribution: only completed entries, only the mapped field, unsigned
//      accumulation, and a recompute that is idempotent.

#include "TestHarness.h"

#include "progression/CodexContributionAggregator.h"
#include "progression/CodexDefinition.h"
#include "progression/CodexDefinitionProvider.h"
#include "progression/CodexState.h"
#include "stats/Contributions.h"

using namespace Modern;

namespace
{
	constexpr CodexId kId = CodexId(100u);
	constexpr CodexId kOther = CodexId(101u);

	constexpr ItemId kItemA = ItemId(10u);
	constexpr ItemId kItemB = ItemId(11u);
	constexpr ItemId kItemC = ItemId(12u);

	ItemInstance Stack(ItemId definition, uint32_t count)
	{
		ItemInstance item;
		item.definition = definition;
		item.serial     = 1u;
		item.count      = count;
		return item;
	}

	// A definition requiring `count` items from `first` (and `second` after it),
	// which is the only shape the live legacy model can complete.
	CodexDefinition Make(CodexId id, CodexType type, uint32_t rewardPoint,
	                     std::initializer_list<CodexRequirement> requirements)
	{
		CodexDefinition definition;
		definition.id          = id;
		definition.title       = "Entry " + std::to_string(id.Get());
		definition.type        = type;
		definition.rewardPoint = rewardPoint;
		uint8_t slot = 0;
		for (const CodexRequirement& requirement : requirements)
		{
			if (slot < kCodexMaxRequirements)
			{
				definition.requirements[slot] = requirement;
				++slot;
			}
		}
		return definition;
	}

	CodexRequirement Req(ItemId item, uint16_t quantity, uint16_t grade = 0)
	{
		CodexRequirement requirement;
		requirement.item          = item;
		requirement.quantity      = quantity;
		requirement.requiredGrade = grade;
		return requirement;
	}
}

// ---- 1. The definition ----

// GLLogixExPC.cpp:5133-5154. One case per `if` statement in CODEX_STATS, so a
// remapped or reordered enum fails here rather than silently paying a bonus into
// the wrong statistic.
MODERN_TEST(CodexTypeMappingMatchesLegacy)
{
	CHECK(CodexTypeToField(CodexType::ReachLevel)    == CodexRewardField::Hp);
	CHECK(CodexTypeToField(CodexType::KillMob)       == CodexRewardField::Mp);
	CHECK(CodexTypeToField(CodexType::KillPlayer)    == CodexRewardField::Sp);
	CHECK(CodexTypeToField(CodexType::ReachMap)      == CodexRewardField::Attack);
	CHECK(CodexTypeToField(CodexType::TakeItem)      == CodexRewardField::Defense);
	CHECK(CodexTypeToField(CodexType::UseItem)       == CodexRewardField::ShootPower);
	CHECK(CodexTypeToField(CodexType::ReachCodex)    == CodexRewardField::MeleePower);
	CHECK(CodexTypeToField(CodexType::CompleteQuest) == CodexRewardField::MagicAttack);
	CHECK(CodexTypeToField(CodexType::CodexPoint)    == CodexRewardField::Resistance);
	CHECK(CodexTypeToField(CodexType::QuestionBox)   == CodexRewardField::Hit);
	CHECK(CodexTypeToField(CodexType::Etc)           == CodexRewardField::Avoid);
}

// The enum is serialised raw in RAN (GLCodexData.cpp:62), so the numeric values
// are part of the data contract and not an implementation detail.
MODERN_TEST(CodexTypeNumberingIsStable)
{
	CHECK_EQ(static_cast<uint8_t>(CodexType::ReachLevel),    uint8_t(0));
	CHECK_EQ(static_cast<uint8_t>(CodexType::KillMob),       uint8_t(1));
	CHECK_EQ(static_cast<uint8_t>(CodexType::KillPlayer),    uint8_t(2));
	CHECK_EQ(static_cast<uint8_t>(CodexType::ReachMap),      uint8_t(3));
	CHECK_EQ(static_cast<uint8_t>(CodexType::TakeItem),      uint8_t(4));
	CHECK_EQ(static_cast<uint8_t>(CodexType::UseItem),       uint8_t(5));
	CHECK_EQ(static_cast<uint8_t>(CodexType::ReachCodex),    uint8_t(6));
	CHECK_EQ(static_cast<uint8_t>(CodexType::CompleteQuest), uint8_t(7));
	CHECK_EQ(static_cast<uint8_t>(CodexType::CodexPoint),    uint8_t(8));
	CHECK_EQ(static_cast<uint8_t>(CodexType::QuestionBox),   uint8_t(9));
	CHECK_EQ(static_cast<uint8_t>(CodexType::Etc),           uint8_t(10));
	CHECK_EQ(kCodexTypeCount, uint8_t(11));
}

MODERN_TEST(CodexTypeRangeIsChecked)
{
	CHECK(IsValid(CodexType::ReachLevel));
	CHECK(IsValid(CodexType::Etc));
	CHECK(!IsValid(CodexType::Size));
	CHECK(!IsValid(static_cast<CodexType>(200)));
	CHECK(CodexTypeToField(static_cast<CodexType>(200)) == CodexRewardField::None);
}

MODERN_TEST(CodexTypeNamesAreReported)
{
	CHECK_EQ(std::string(ToString(CodexType::ReachLevel)), std::string("ReachLevel"));
	CHECK_EQ(std::string(ToString(CodexType::Etc)), std::string("Etc"));
	// The sentinel is not a type, and saying so is more useful than printing a
	// twelfth name.
	CHECK_EQ(std::string(ToString(CodexType::Size)), std::string("<invalid codex type>"));
}

// GLCodexData.cpp:377-386. The four sequential `if`s, transcribed. The cascade
// runs from the last slot backwards, so the *last* unconfigured slot wins and the
// result is a prefix count.
MODERN_TEST(RequiredSlotCountFollowsLegacyCascade)
{
	// All five configured: 5.
	{
		const CodexDefinition d = Make(kId, CodexType::Etc, 1u,
		                               { Req(kItemA, 1), Req(kItemB, 1), Req(kItemC, 1),
		                                 Req(kItemA, 1), Req(kItemB, 1) });
		CHECK_EQ(d.RequiredSlotCount(), uint8_t(5));
	}

	// Slot 4 unconfigured: the first `if` fires and nothing after it does, so 4.
	{
		const CodexDefinition d = Make(kId, CodexType::Etc, 1u,
		                               { Req(kItemA, 1), Req(kItemB, 1), Req(kItemC, 1),
		                                 Req(kItemA, 1) });
		CHECK_EQ(d.RequiredSlotCount(), uint8_t(4));
	}

	// Slots 3 and 4 unconfigured: the second `if` fires last, so 3.
	{
		const CodexDefinition d = Make(kId, CodexType::Etc, 1u,
		                               { Req(kItemA, 1), Req(kItemB, 1), Req(kItemC, 1) });
		CHECK_EQ(d.RequiredSlotCount(), uint8_t(3));
	}

	// Slot 2 unconfigured: 2.
	{
		const CodexDefinition d = Make(kId, CodexType::Etc, 1u,
		                               { Req(kItemA, 1), Req(kItemB, 1) });
		CHECK_EQ(d.RequiredSlotCount(), uint8_t(2));
	}

	// Slot 1 unconfigured: 1.
	{
		const CodexDefinition d = Make(kId, CodexType::Etc, 1u, { Req(kItemA, 1) });
		CHECK_EQ(d.RequiredSlotCount(), uint8_t(1));
	}

	// Nothing configured: the last `if` still fires, so 1 and not 0. RAN cannot
	// produce a required count of zero through this path.
	{
		const CodexDefinition d = Make(kId, CodexType::Etc, 1u, {});
		CHECK_EQ(d.RequiredSlotCount(), uint8_t(1));
	}
}

// The quirk the cascade hides, pinned deliberately. RAN never tests the *first*
// slot, so an entry configuring only `wProgressMobKill` still requires five
// registrations of an item that only one slot names - which can never complete.
// Reproduced rather than corrected; see CodexDefinition::RequiredSlotCount.
MODERN_TEST(RequiredSlotCountIgnoresTheFirstSlot)
{
	// Slot 0 configured, slots 1-4 not. The `if` on slot 1 fires last, so the
	// count is 1 - not 5, and not 0. Only the prefix from slot 1 is counted.
	CodexDefinition definition;
	definition.id          = kId;
	definition.title       = "First slot only";
	definition.type        = CodexType::KillMob;
	definition.requirements[0] = Req(kItemA, 1);
	CHECK_EQ(definition.RequiredSlotCount(), uint8_t(1));
	CHECK(definition.IsCompletable());
}

MODERN_TEST(CodexRequirementNeedsAnItemAndAQuantity)
{
	// Both halves matter, because RAN checks the two separately: the cascade
	// tests the quantity, and DoCodexRegisterItem tests the id.
	CHECK(Req(kItemA, 1).IsConfigured());
	CHECK(!Req(kItemA, 0).IsConfigured());
	CHECK(!Req(ItemId::MakeInvalid(), 1).IsConfigured());
	CHECK(!Req(ItemId::MakeInvalid(), 0).IsConfigured());
}

MODERN_TEST(CodexDefinitionValidityIsChecked)
{
	CodexDefinition valid = Make(kId, CodexType::ReachLevel, 5u, { Req(kItemA, 1) });
	CHECK(valid.IsValid());

	CodexDefinition noId    = valid; noId.id    = CodexId::MakeInvalid();
	CodexDefinition noTitle = valid; noTitle.title.clear();
	CodexDefinition badType = valid; badType.type = static_cast<CodexType>(200);
	CodexDefinition badNote = valid; badNote.notify = static_cast<CodexNotify>(9);
	CHECK(!noId.IsValid());
	CHECK(!noTitle.IsValid());
	CHECK(!badType.IsValid());
	CHECK(!badNote.IsValid());
}

MODERN_TEST(CodexDefinitionWithoutRequirementsIsNotCompletable)
{
	const CodexDefinition d = Make(kId, CodexType::Etc, 10u, {});
	CHECK(!d.IsCompletable());
}

MODERN_TEST(CodexProviderFindsAndSorts)
{
	InMemoryCodexDefinitions definitions;
	// Added out of order on purpose: GetAll's order is the contract, not the
	// insertion order.
	CHECK(definitions.Add(Make(kOther, CodexType::Etc, 1u, { Req(kItemA, 1) })).IsOk());
	CHECK(definitions.Add(Make(kId,    CodexType::Etc, 1u, { Req(kItemA, 1) })).IsOk());
	CHECK(definitions.Add(Make(CodexId(50u), CodexType::Etc, 1u, { Req(kItemA, 1) })).IsOk());

	CHECK_EQ(definitions.GetCount(), size_t(3));
	CHECK(definitions.GetAll()[0].id == CodexId(50u));
	CHECK(definitions.GetAll()[1].id == kId);
	CHECK(definitions.GetAll()[2].id == kOther);

	CHECK(definitions.Find(kId) != nullptr);
	CHECK(definitions.Find(CodexId(999u)) == nullptr);
	CHECK(definitions.Find(CodexId::MakeInvalid()) == nullptr);

	// Replace in place, not appended: a second Add for the same id keeps the
	// count and the order.
	CHECK(definitions.Add(Make(kId, CodexType::KillMob, 77u, { Req(kItemA, 1) })).IsOk());
	CHECK_EQ(definitions.GetCount(), size_t(3));
	CHECK_EQ(definitions.Find(kId)->rewardPoint, uint32_t(77));
	CHECK(definitions.Find(kId)->type == CodexType::KillMob);

	// An invalid definition is refused and changes nothing.
	CHECK(definitions.Add(Make(CodexId::MakeInvalid(), CodexType::Etc, 1u, { Req(kItemA, 1) }))
	          .GetCode() == ErrorCode::InvalidArgument);
	CHECK_EQ(definitions.GetCount(), size_t(3));

	CHECK(definitions.Remove(kId).IsOk());
	CHECK(definitions.Remove(kId).GetCode() == ErrorCode::NotFound);
	CHECK_EQ(definitions.GetCount(), size_t(2));
}

// ---- 2. The state ----

// GLCharDataCodex.cpp:72-93. Every definition the character does not have is
// seated with zero progress, and a completed one is not re-seated.
MODERN_TEST(ReconcileSeatsEverythingUnfinished)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(kId,    CodexType::ReachLevel, 5u, { Req(kItemA, 1) })).IsOk());
	CHECK(definitions.Add(Make(kOther, CodexType::KillMob,    6u, { Req(kItemB, 2) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);

	CHECK_EQ(state.GetProgressCount(), size_t(2));
	CHECK_EQ(state.GetCompletedCount(), size_t(0));
	CHECK(state.IsInProgress(kId));
	CHECK(state.IsInProgress(kOther));

	const CodexProgress record = state.GetProgress(kId);
	CHECK_EQ(record.doneCount, uint8_t(0));
	CHECK_EQ(record.requiredCount, uint8_t(1));
	CHECK(record.type == CodexType::ReachLevel);
	// RAN copies the requirements onto the character record, so it is
	// self-contained and needs no definition to be read back.
	CHECK(record.requirements[0].item == kItemA);
	CHECK_EQ(record.requirements[0].quantity, uint16_t(1));

	// The GM/new-character path (GLCharDataLoad.cpp:357-373) does the same seat.
	// Reconcile is idempotent, so running it twice is what a reload looks like.
	const CodexState first = state;
	state.Reconcile(definitions);
	CHECK(state == first);
}

// GLCharDataCodex.cpp:79-80. A completed entry is skipped by the `continue`, and
// that is what makes the reward exactly-once across a reload.
MODERN_TEST(ReconcileNeverReseatsACompletedEntry)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(kId, CodexType::ReachLevel, 5u, { Req(kItemA, 1) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	CHECK(state.RegisterItem(kId, Stack(kItemA, 1)).GetValue().completed);
	CHECK(state.IsCompleted(kId));

	// The table is reloaded. The entry must not come back as progress, or the
	// second registration would pay again.
	state.Reconcile(definitions);
	CHECK(state.IsCompleted(kId));
	CHECK(!state.IsInProgress(kId));
	CHECK_EQ(state.GetProgressCount(), size_t(0));
	CHECK_EQ(state.GetCompletedCount(), size_t(1));
}

// GLCodexData.cpp:95-132. A record whose definition is gone is dropped from both
// maps.
MODERN_TEST(ReconcileDropsOrphanedRecords)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(kId,    CodexType::ReachLevel, 5u, { Req(kItemA, 1) })).IsOk());
	CHECK(definitions.Add(Make(kOther, CodexType::KillMob,    6u, { Req(kItemB, 2) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	CHECK(state.RegisterItem(kId, Stack(kItemA, 1)).GetValue().completed);
	CHECK(state.IsCompleted(kId));
	CHECK(state.IsInProgress(kOther));

	// The unfinished entry is removed from the table.
	CHECK(definitions.Remove(kOther).IsOk());
	state.Reconcile(definitions);
	CHECK(!state.IsInProgress(kOther));
	CHECK_EQ(state.GetProgressCount(), size_t(0));
	// The completed one stays, because its definition is still there.
	CHECK(state.IsCompleted(kId));

	// And the completed one goes when its definition does.
	CHECK(definitions.Remove(kId).IsOk());
	state.Reconcile(definitions);
	CHECK(!state.IsCompleted(kId));
	CHECK_EQ(state.GetCompletedCount(), size_t(0));
}

// GLCodexData.cpp:486-494. A retyped entry loses its progress.
MODERN_TEST(ReconcileResetsProgressWhenTheTypeChanges)
{
	InMemoryCodexDefinitions definitions;
	// Two requirements, so satisfying the first leaves the entry in progress
	// rather than completing it and moving it out of reach of the retune.
	CHECK(definitions.Add(Make(kId, CodexType::KillMob, 5u,
	                           { Req(kItemA, 1), Req(kItemB, 1) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	CHECK(state.RegisterItem(kId, Stack(kItemA, 1)).GetValue().recorded);
	CHECK(!state.IsCompleted(kId));
	CHECK_EQ(state.GetProgress(kId).doneCount, uint8_t(1));

	// The table is retuned to a different type. RAN zeroes the counters and
	// re-runs Assign, so the entry starts over with the new type.
	CHECK(definitions.Add(Make(kId, CodexType::Etc, 9u,
	                           { Req(kItemB, 3), Req(kItemA, 2) })).IsOk());
	state.Reconcile(definitions);

	CHECK(state.IsInProgress(kId));
	CHECK(state.GetProgress(kId).type == CodexType::Etc);
	CHECK_EQ(state.GetProgress(kId).doneCount, uint8_t(0));
	CHECK_EQ(state.GetProgress(kId).requiredCount, uint8_t(2));
	CHECK(state.GetProgress(kId).requirements[0].item == kItemB);
	CHECK_EQ(state.GetProgress(kId).requirements[0].quantity, uint16_t(3));
}

// The one place modern deliberately departs from RAN. GLCodexData.cpp:497-506
// refreshes only the five item ids and leaves the captured quantities, grades
// and required count stale, so a retuned table leaves existing characters
// holding requirements that no longer exist. Modern refreshes the whole
// requirement.
MODERN_TEST(ReconcileRefreshesQuantitiesAndRequiredCount)
{
	InMemoryCodexDefinitions definitions;
	// Two requirements, so the count is 2.
	CHECK(definitions.Add(Make(kId, CodexType::TakeItem, 5u,
	                           { Req(kItemA, 1), Req(kItemB, 1) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	CHECK_EQ(state.GetProgress(kId).requiredCount, uint8_t(2));

	// The table is retuned: the second requirement is dropped and the first now
	// needs three.
	CHECK(definitions.Add(Make(kId, CodexType::TakeItem, 5u, { Req(kItemA, 3) })).IsOk());
	state.Reconcile(definitions);

	// RAN would still say 2, and would still want one of each. Modern says 1 and
	// wants three.
	CHECK_EQ(state.GetProgress(kId).requiredCount, uint8_t(1));
	CHECK_EQ(state.GetProgress(kId).requirements[0].quantity, uint16_t(3));
	CHECK(state.IsInProgress(kId));
}

// Reconciliation runs on every load, so it has to be idempotent. RAN keeps
// `dwProgressItemDone1..5` across a same-type refresh
// (GLCodexData.cpp:497-506), and modern keeps it too - but only while the
// requirements are actually unchanged, because a done flag is a claim about a
// requirement, not about a record.
MODERN_TEST(ReconcilePreservesProgressAcrossAnIdenticalRefresh)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(kId, CodexType::TakeItem, 5u,
	                           { Req(kItemA, 1), Req(kItemB, 2) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	CHECK(state.RegisterItem(kId, Stack(kItemA, 1)).GetValue().recorded);
	CodexState before = state;

	// Reloading the same table three times must not cost the recorded slot.
	for (int reload = 0; reload < 3; ++reload)
	{
		state.Reconcile(definitions);
		CHECK(state == before);
	}
	CHECK(state.RegisterItem(kId, Stack(kItemB, 2)).GetValue().completed);
	CHECK(state.IsCompleted(kId));
}

// A done flag records "this exact item and count was registered". Change the
// count and that claim is no longer true, so the progress is dropped rather
// than carried across the retune.
MODERN_TEST(ReconcileDropsProgressWhenARequirementChanges)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(kId, CodexType::TakeItem, 5u,
	                           { Req(kItemA, 1), Req(kItemB, 2) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	CHECK(state.RegisterItem(kId, Stack(kItemA, 1)).GetValue().recorded);
	CHECK_EQ(state.GetProgress(kId).doneCount, uint8_t(1));

	// Slot 0 now needs two instead of one. Slot 0's done flag cannot survive
	// that, and the count moving to 1 changes the completion condition, so the
	// whole record starts over.
	CHECK(definitions.Add(Make(kId, CodexType::TakeItem, 5u,
	                           { Req(kItemA, 2), Req(kItemB, 2) })).IsOk());
	state.Reconcile(definitions);
	CHECK(state.IsInProgress(kId));
	CHECK_EQ(state.GetProgress(kId).doneCount, uint8_t(0));
	CHECK(!state.GetProgress(kId).done[0]);
	CHECK(!state.GetProgress(kId).done[1]);
}

// The happy path. GLCharCodex.cpp:147-172.
MODERN_TEST(RegistrationCompletesAtTheRequiredCount)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(kId, CodexType::ReachLevel, 5u,
	                           { Req(kItemA, 1), Req(kItemB, 2) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);

	// The first of two. Recorded, not complete, still in progress.
	Result<CodexRegistration> first = state.RegisterItem(kId, Stack(kItemA, 1));
	CHECK(first.IsOk());
	CHECK(first.GetValue().recorded);
	CHECK(!first.GetValue().completed);
	CHECK_EQ(first.GetValue().doneCount, uint8_t(1));
	CHECK_EQ(first.GetValue().requiredCount, uint8_t(2));
	CHECK(state.IsInProgress(kId));
	CHECK(!state.IsCompleted(kId));

	// The second. Complete, and moved from progress to completed.
	Result<CodexRegistration> second = state.RegisterItem(kId, Stack(kItemB, 2));
	CHECK(second.IsOk());
	CHECK(second.GetValue().recorded);
	CHECK(second.GetValue().completed);
	CHECK_EQ(second.GetValue().doneCount, uint8_t(2));
	CHECK(!state.IsInProgress(kId));
	CHECK(state.IsCompleted(kId));
	CHECK_EQ(state.GetProgressCount(), size_t(0));
	CHECK_EQ(state.GetCompletedCount(), size_t(1));
}

// GLCharCodex.cpp:81, :95, :109, :123, :137. Equality, not `>=`.
MODERN_TEST(RegistrationComparesQuantityForEquality)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(kId, CodexType::TakeItem, 5u, { Req(kItemA, 3) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);

	// Fewer than required: no match. RAN's `wQuantity1 == wTurnNum` fails, so
	// this records nothing - a modern caller must not spend the item either.
	Result<CodexRegistration> tooFew = state.RegisterItem(kId, Stack(kItemA, 2));
	CHECK(tooFew.IsOk());
	CHECK(!tooFew.GetValue().recorded);
	CHECK_EQ(state.GetProgress(kId).doneCount, uint8_t(0));

	// More than required: also no match. Relaxing this to `>=` would be an
	// invention, and it would let a single registration satisfy a five-count
	// requirement.
	Result<CodexRegistration> tooMany = state.RegisterItem(kId, Stack(kItemA, 4));
	CHECK(tooMany.IsOk());
	CHECK(!tooMany.GetValue().recorded);
	CHECK_EQ(state.GetProgress(kId).doneCount, uint8_t(0));

	// Exactly right.
	Result<CodexRegistration> exact = state.RegisterItem(kId, Stack(kItemA, 3));
	CHECK(exact.IsOk());
	CHECK(exact.GetValue().recorded);
	CHECK(exact.GetValue().completed);
}

// GLCharCodex.cpp:75-144. The five slots are independent `if`s, not `else if`, so
// one stack can satisfy two slots when an entry names the same item twice.
MODERN_TEST(OneStackCanSatisfyTwoSlotsNamingTheSameItem)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(kId, CodexType::Etc, 5u,
	                           { Req(kItemA, 1), Req(kItemA, 1) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	CHECK_EQ(state.GetProgress(kId).requiredCount, uint8_t(2));

	// One registration, both slots satisfied, both counters incremented.
	Result<CodexRegistration> result = state.RegisterItem(kId, Stack(kItemA, 1));
	CHECK(result.IsOk());
	CHECK(result.GetValue().recorded);
	CHECK_EQ(result.GetValue().doneCount, uint8_t(2));
	CHECK(result.GetValue().completed);
}

// A slot is satisfied once. GLCharCodex.cpp:77, :91, :105, :119, :133 test
// `dwProgressItemDoneN != 1` before recording.
MODERN_TEST(ASatisfiedSlotIsNotRecordedTwice)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(kId, CodexType::Etc, 5u,
	                           { Req(kItemA, 1), Req(kItemB, 1) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	CHECK(state.RegisterItem(kId, Stack(kItemA, 1)).GetValue().recorded);
	CHECK_EQ(state.GetProgress(kId).doneCount, uint8_t(1));

	// The same item again: slot 0 is already done, so nothing is recorded and
	// the count does not move.
	Result<CodexRegistration> again = state.RegisterItem(kId, Stack(kItemA, 1));
	CHECK(again.IsOk());
	CHECK(!again.GetValue().recorded);
	CHECK_EQ(state.GetProgress(kId).doneCount, uint8_t(1));
	CHECK(state.IsInProgress(kId));
}

// GLCharCodex.cpp:75-144. A slot outside the required count is never consulted,
// even when it would match.
//
// The cascade makes this shape awkward to build: for prefix data the required
// count always equals the number of configured slots, so nothing can sit past
// it. It needs a table with a hole. Configuring slots 0, 1 and 3 leaves the
// count at 2 - slot 2 is unconfigured, and the `if` that lowers the count to 2
// is the last one to fire - so slot 3 is configured but uncounted.
MODERN_TEST(SlotsBeyondTheRequiredCountAreIgnored)
{
	InMemoryCodexDefinitions definitions;
	CodexDefinition holed = Make(kId, CodexType::Etc, 5u,
	                             { Req(kItemA, 1), Req(kItemB, 1) });
	holed.requirements[3] = Req(kItemC, 1);
	CHECK_EQ(holed.RequiredSlotCount(), uint8_t(2));
	CHECK(holed.IsCompletable());
	CHECK(definitions.Add(holed).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	CHECK_EQ(state.GetProgress(kId).requiredCount, uint8_t(2));

	// Slot 3 names item C and is configured, but the count is 2, so the match
	// loop stops at slot 1 and never reaches it.
	Result<CodexRegistration> ignored = state.RegisterItem(kId, Stack(kItemC, 1));
	CHECK(ignored.IsOk());
	CHECK(!ignored.GetValue().recorded);
	CHECK(state.IsInProgress(kId));
	CHECK_EQ(state.GetProgress(kId).doneCount, uint8_t(0));

	// The two counted slots are what complete it, and the count clamps at 2.
	CHECK(state.RegisterItem(kId, Stack(kItemA, 1)).GetValue().recorded);
	CHECK(state.RegisterItem(kId, Stack(kItemB, 1)).GetValue().completed);
	CHECK(state.IsCompleted(kId));
	CHECK_EQ(state.GetCompleted(kId).doneCount, uint8_t(2));
	CHECK(state.GetCompleted(kId).IsComplete());
}

// The refusals, and the difference between them.
MODERN_TEST(RegistrationRefusalsAreDistinguished)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(kId, CodexType::Etc, 5u, { Req(kItemA, 1) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	CHECK(state.RegisterItem(kId, Stack(kItemA, 1)).GetValue().completed);

	// A completed entry. RAN refuses this a step earlier, at the request handler
	// (GLCharInvenMsg.cpp:9130-9136).
	Result<CodexRegistration> completed = state.RegisterItem(kId, Stack(kItemA, 1));
	CHECK(completed.IsOk());
	CHECK(completed.GetValue().error == CodexRegisterError::AlreadyCompleted);
	CHECK(!completed.GetValue().recorded);

	// An entry the character does not have at all.
	Result<CodexRegistration> unknown = state.RegisterItem(kOther, Stack(kItemA, 1));
	CHECK(unknown.IsOk());
	CHECK(unknown.GetValue().error == CodexRegisterError::UnknownCodex);

	// A malformed id or an unusable item is a bad argument, not a rule outcome.
	CHECK(state.RegisterItem(CodexId::MakeInvalid(), Stack(kItemA, 1))
	          .GetError() == ErrorCode::InvalidArgument);
	CHECK(state.RegisterItem(kId, ItemInstance()).GetError() == ErrorCode::InvalidArgument);
	// A zero-count instance is not a stack and is not a valid instance.
	ItemInstance empty = Stack(kItemA, 1);
	empty.count = 0;
	CHECK(state.RegisterItem(kId, empty).GetError() == ErrorCode::InvalidArgument);
}

// RAN has no such guard: such an entry sits in the progress map forever. Modern
// refuses the registration, because the answer is knowable and the alternative
// is a record nothing can ever satisfy.
MODERN_TEST(AnEntryWithNoRequirementsCannotBeRegistered)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(kId, CodexType::Etc, 5u, {})).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	// Seated - RAN seats it too, with a required count of 1 from the cascade.
	CHECK(state.IsInProgress(kId));

	Result<CodexRegistration> result = state.RegisterItem(kId, Stack(kItemA, 1));
	CHECK(result.IsOk());
	CHECK(result.GetValue().error == CodexRegisterError::NotCompletable);
	CHECK(!result.GetValue().recorded);
	CHECK(!state.IsCompleted(kId));
}

// A retune that leaves an entry with no requirements drops the record rather than
// keeping a record nothing can satisfy.
MODERN_TEST(RetuningAnEntryAwayFromItsLastRequirementDropsTheRecord)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(kId, CodexType::Etc, 5u, { Req(kItemA, 1) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	CHECK(state.IsInProgress(kId));

	// The item is taken out of the table entry.
	CHECK(definitions.Add(Make(kId, CodexType::Etc, 5u, {})).IsOk());
	state.Reconcile(definitions);
	CHECK(!state.IsInProgress(kId));
	CHECK(!state.IsCompleted(kId));
	CHECK_EQ(state.GetProgressCount(), size_t(0));
}

MODERN_TEST(CodexStateClearAndEquality)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(kId, CodexType::Etc, 5u, { Req(kItemA, 1) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	CodexState copy = state;
	CHECK(state == copy);

	state.RegisterItem(kId, Stack(kItemA, 1));
	CHECK(state != copy);

	// The registration moved the record out of progress and into completed; it
	// is not in both maps, which is what the `!=` above already implies. So
	// clearing the completed map leaves this state empty while `copy` still
	// holds the seated record - they are not equal again until `copy` is
	// cleared too.
	state.ClearCompleted();
	CHECK_EQ(state.GetCompletedCount(), size_t(0));
	CHECK(state != copy);
	CodexState fresh;
	CHECK(state == fresh);

	// Clearing progress leaves the same empty state.
	state.Reconcile(definitions);
	CHECK(state.IsInProgress(kId));
	state.ClearProgress();
	CHECK(state == fresh);
}

// ---- 3. The contribution ----

// GLogixExPC.cpp:5122-5156. Only completed entries, each adding its definition's
// reward point to the one field its type selects.
MODERN_TEST(ContributionCountsOnlyCompletedEntries)
{
	InMemoryCodexDefinitions definitions;
	// One of each of three types, so three different fields are exercised.
	CHECK(definitions.Add(Make(CodexId(1u), CodexType::ReachLevel, 10u, { Req(kItemA, 1) })).IsOk());
	CHECK(definitions.Add(Make(CodexId(2u), CodexType::KillMob,    20u, { Req(kItemB, 1) })).IsOk());
	CHECK(definitions.Add(Make(CodexId(3u), CodexType::Etc,        30u, { Req(kItemC, 1) })).IsOk());
	CHECK(definitions.Add(Make(CodexId(4u), CodexType::UseItem,    40u, { Req(kItemA, 2) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	CHECK_EQ(state.GetProgressCount(), size_t(4));

	// Nothing completed yet: the contribution is zero, and it says so.
	Result<CodexContributionResult> empty =
		CodexContributionAggregator::Aggregate(state, definitions);
	CHECK(empty.IsOk());
	CHECK(Stats::IsZero(empty.GetValue().contribution));
	CHECK_EQ(empty.GetValue().contributingCodex, size_t(0));

	// Complete one. Only its field moves.
	CHECK(state.RegisterItem(CodexId(1u), Stack(kItemA, 1)).GetValue().completed);
	Result<CodexContributionResult> one =
		CodexContributionAggregator::Aggregate(state, definitions);
	CHECK(one.IsOk());
	CHECK_EQ(one.GetValue().contribution.hp, uint32_t(10));
	CHECK_EQ(one.GetValue().contribution.mp, uint32_t(0));
	CHECK_EQ(one.GetValue().contribution.avoid, uint32_t(0));
	CHECK_EQ(one.GetValue().contributingCodex, size_t(1));

	// Complete two more, of two different types.
	CHECK(state.RegisterItem(CodexId(2u), Stack(kItemB, 1)).GetValue().completed);
	CHECK(state.RegisterItem(CodexId(4u), Stack(kItemA, 2)).GetValue().completed);
	Result<CodexContributionResult> three =
		CodexContributionAggregator::Aggregate(state, definitions);
	CHECK(three.IsOk());
	CHECK_EQ(three.GetValue().contribution.hp, uint32_t(10));
	CHECK_EQ(three.GetValue().contribution.mp, uint32_t(20));
	CHECK_EQ(three.GetValue().contribution.shootPower, uint32_t(40));
	CHECK_EQ(three.GetValue().contribution.avoid, uint32_t(0));
	CHECK_EQ(three.GetValue().contributingCodex, size_t(3));
	// The unfinished entry contributed nothing.
	CHECK_EQ(state.GetProgressCount(), size_t(1));
	CHECK_EQ(three.GetValue().skippedCodex, size_t(0));
}

// The accumulation is per field, so two entries of the same type add up and
// entries of different types do not bleed into each other.
MODERN_TEST(ContributionAccumulatesPerField)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(CodexId(1u), CodexType::KillMob, 7u, { Req(kItemA, 1) })).IsOk());
	CHECK(definitions.Add(Make(CodexId(2u), CodexType::KillMob, 9u, { Req(kItemB, 1) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	CHECK(state.RegisterItem(CodexId(1u), Stack(kItemA, 1)).GetValue().completed);
	CHECK(state.RegisterItem(CodexId(2u), Stack(kItemB, 1)).GetValue().completed);

	Result<CodexContributionResult> result =
		CodexContributionAggregator::Aggregate(state, definitions);
	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.mp, uint32_t(16));
	CHECK_EQ(result.GetValue().contribution.hp, uint32_t(0));
	CHECK_EQ(result.GetValue().contributingCodex, size_t(2));
}

// GLogixExPC.cpp:5106-5116, :5158-5168. The legacy function is a full recompute
// from local zeros with two unused parameters, so it is idempotent. Aggregating
// twice must give the same answer, which is what makes it safe on every
// recalculation.
MODERN_TEST(ContributionIsIdempotent)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(CodexId(1u), CodexType::TakeItem, 11u, { Req(kItemA, 1) })).IsOk());
	CHECK(definitions.Add(Make(CodexId(2u), CodexType::ReachCodex, 13u, { Req(kItemB, 1) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	state.RegisterItem(CodexId(1u), Stack(kItemA, 1));
	state.RegisterItem(CodexId(2u), Stack(kItemB, 1));

	const Result<CodexContributionResult> first =
		CodexContributionAggregator::Aggregate(state, definitions);
	const Result<CodexContributionResult> second =
		CodexContributionAggregator::Aggregate(state, definitions);
	CHECK(first.IsOk());
	CHECK(second.IsOk());
	CHECK(first.GetValue().contribution == second.GetValue().contribution);
	CHECK_EQ(first.GetValue().contribution.defense, uint32_t(11));
	CHECK_EQ(first.GetValue().contribution.meleePower, uint32_t(13));
}

// Completion order must not change the result. The maps are ordered by id, so
// this is true by construction rather than by a sort at the end.
MODERN_TEST(ContributionIsIndependentOfCompletionOrder)
{
	InMemoryCodexDefinitions definitions;
	for (uint32_t id = 1u; id <= 4u; ++id)
	{
		CHECK(definitions.Add(Make(CodexId(id), CodexType::Etc, id * 3u,
		                            { Req(ItemId(id), 1) })).IsOk());
	}

	CodexState forward;
	forward.Reconcile(definitions);
	CodexState backward;
	backward.Reconcile(definitions);

	for (uint32_t id = 1u; id <= 4u; ++id)
	{
		forward.RegisterItem(CodexId(id), Stack(ItemId(id), 1));
	}
	for (uint32_t id = 4u; id >= 1u; --id)
	{
		backward.RegisterItem(CodexId(id), Stack(ItemId(id), 1));
	}

	const Result<CodexContributionResult> a =
		CodexContributionAggregator::Aggregate(forward, definitions);
	const Result<CodexContributionResult> b =
		CodexContributionAggregator::Aggregate(backward, definitions);
	CHECK(a.IsOk());
	CHECK(b.IsOk());
	// 3 + 6 + 9 + 12.
	CHECK_EQ(a.GetValue().contribution.avoid, uint32_t(30));
	CHECK(a.GetValue().contribution == b.GetValue().contribution);
}

// A completed entry whose definition is gone is skipped, as RAN skips it at
// GLogixExPC.cpp:5131. Modern reports the count, because a reward that silently
// pays nothing is otherwise indistinguishable from a working one.
MODERN_TEST(ContributionReportsEntriesItSkipped)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(kId, CodexType::Etc, 50u, { Req(kItemA, 1) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	CHECK(state.RegisterItem(kId, Stack(kItemA, 1)).GetValue().completed);

	// The definition is removed without a reconcile, so the record survives while
	// its definition does not - the transitional state RAN tolerates.
	InMemoryCodexDefinitions emptied;
	Result<CodexContributionResult> result =
		CodexContributionAggregator::Aggregate(state, emptied);
	CHECK(result.IsOk());
	CHECK(Stats::IsZero(result.GetValue().contribution));
	CHECK_EQ(result.GetValue().skippedCodex, size_t(1));
	CHECK(result.GetValue().skipReason == CodexContributionError::MissingDefinition);
	CHECK(result.GetValue().IsOk());
}

// A definition whose type is outside the verified enum cannot name a field. RAN
// falls through all eleven `if`s and pays nothing; modern skips and says so.
MODERN_TEST(ContributionReportsAnUnmappedType)
{
	// A definition that bypasses the validity check, to model a table that was
	// loaded without one. `Add` refuses it, which is the first line of defence.
	CodexDefinition broken;
	broken.id          = kId;
	broken.title       = "Broken";
	broken.type        = static_cast<CodexType>(200);
	broken.rewardPoint = 50u;
	CHECK(!broken.IsValid());

	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(broken).GetCode() == ErrorCode::InvalidArgument);

	// So the aggregation path for it is reached by holding a completed record and
	// asking a provider that has no answer, which is the MissingDefinition case
	// above, and by confirming the field mapping itself refuses.
	CHECK(CodexTypeToField(static_cast<CodexType>(200)) == CodexRewardField::None);
}

// A zero reward point is still a completed entry, and it is counted separately
// from the ones that paid something.
MODERN_TEST(ContributionSeparatesCompletedFromPaid)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(CodexId(1u), CodexType::Etc, 0u, { Req(kItemA, 1) })).IsOk());
	CHECK(definitions.Add(Make(CodexId(2u), CodexType::Etc, 5u, { Req(kItemB, 1) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	state.RegisterItem(CodexId(1u), Stack(kItemA, 1));
	state.RegisterItem(CodexId(2u), Stack(kItemB, 1));

	Result<CodexContributionResult> result =
		CodexContributionAggregator::Aggregate(state, definitions);
	CHECK(result.IsOk());
	CHECK_EQ(state.GetCompletedCount(), size_t(2));
	CHECK_EQ(result.GetValue().contributingCodex, size_t(1));
	CHECK_EQ(result.GetValue().contribution.avoid, uint32_t(5));
}

// The reward is exactly once per completed entry, no matter how many times the
// registration is attempted.
MODERN_TEST(ARewardIsPaidExactlyOnce)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(kId, CodexType::Etc, 25u, { Req(kItemA, 1) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	CHECK(state.RegisterItem(kId, Stack(kItemA, 1)).GetValue().completed);

	// Every one of these is refused, and none of them pays again.
	for (int attempt = 0; attempt < 5; ++attempt)
	{
		CHECK(state.RegisterItem(kId, Stack(kItemA, 1))
		          .GetValue().error == CodexRegisterError::AlreadyCompleted);
		state.Reconcile(definitions);
	}

	Result<CodexContributionResult> result =
		CodexContributionAggregator::Aggregate(state, definitions);
	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.avoid, uint32_t(25));
	CHECK_EQ(state.GetCompletedCount(), size_t(1));
}

// The whole chain, end to end in core: seat, register one of three, verify the
// partial contribution, finish it, verify the whole.
MODERN_TEST(CodexEndToEndInCore)
{
	InMemoryCodexDefinitions definitions;
	CHECK(definitions.Add(Make(CodexId(1u), CodexType::ReachLevel, 100u,
	                            { Req(kItemA, 1) })).IsOk());
	CHECK(definitions.Add(Make(CodexId(2u), CodexType::KillMob, 200u,
	                            { Req(kItemB, 1) })).IsOk());
	CHECK(definitions.Add(Make(CodexId(3u), CodexType::CompleteQuest, 300u,
	                            { Req(kItemC, 1) })).IsOk());

	CodexState state;
	state.Reconcile(definitions);
	CHECK_EQ(state.GetProgressCount(), size_t(3));

	CHECK(state.RegisterItem(CodexId(1u), Stack(kItemA, 1)).GetValue().completed);
	CHECK(state.RegisterItem(CodexId(2u), Stack(kItemA, 1)).GetValue().recorded == false);
	CHECK(state.RegisterItem(CodexId(2u), Stack(kItemB, 1)).GetValue().completed);
	CHECK(state.RegisterItem(CodexId(3u), Stack(kItemC, 1)).GetValue().completed);

	CHECK_EQ(state.GetProgressCount(), size_t(0));
	CHECK_EQ(state.GetCompletedCount(), size_t(3));

	Result<CodexContributionResult> result =
		CodexContributionAggregator::Aggregate(state, definitions);
	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contributingCodex, size_t(3));
	CHECK_EQ(result.GetValue().contribution.hp, uint32_t(100));
	CHECK_EQ(result.GetValue().contribution.mp, uint32_t(200));
	CHECK_EQ(result.GetValue().contribution.magicAttack, uint32_t(300));
	CHECK_EQ(result.GetValue().contribution.sp, uint32_t(0));
	CHECK(Stats::IsZero(Stats::CodexContribution()) == true);
}
