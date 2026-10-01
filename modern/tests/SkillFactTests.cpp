#include "TestHarness.h"

#include "skills/SkillFactTypes.h"
#include "skills/SkillFactContainer.h"
#include "skills/SkillFactAggregator.h"

namespace
{
	using namespace Modern;
	using namespace Modern::Skills;

	SkillId FactSkill(uint16_t main, uint16_t sub) { return SkillId{ main, sub }; }

	// A FACT that "holds something", so Apply accepts it.
	SkillFact MakeFact(const SkillId& id, float lifetime,
	                   SkillFactSpecType specType = SkillFactSpecType::MoveVelo,
	                   float specVar1 = 0.1f)
	{
		SkillFact fact;
		fact.skillId           = id;
		fact.level             = 3;
		fact.remainingLifetime = lifetime;
		fact.basicType         = PassiveApplyType::VarHp;
		fact.basicValue        = 5.0f;
		fact.specs[0].type     = specType;
		fact.specs[0].var1     = specVar1;
		return fact;
	}

	// A pool of `count` distinct facts, all with the same lifetime.
	void FillPool(SkillFactContainer& container, uint32_t count, float lifetime)
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			(void) container.Apply(MakeFact(FactSkill(1, static_cast<uint16_t>(i)), lifetime));
		}
	}
}

// ═══════════════════════════════════════════════════════════════════════
// Constants - the pool size is a shipped value, not a preference
// ═══════════════════════════════════════════════════════════════════════

MODERN_TEST(SkillFact_PoolIsFourteenSlots)
{
	CHECK_EQ(kSkillFactSlotCount, 14u);
	CHECK_EQ(kSkillFactMaxImpacts, 5u);
	CHECK_EQ(kSkillFactMaxSpecs, 5u);
}

// ═══════════════════════════════════════════════════════════════════════
// Container basics
// ═══════════════════════════════════════════════════════════════════════

MODERN_TEST(SkillFact_EmptyPoolHasNoActiveFacts)
{
	SkillFactContainer container;

	CHECK_EQ(container.ActiveCount(), 0u);
	CHECK(!container.Full());
	CHECK(!container.Has(FactSkill(1, 1)));
}

MODERN_TEST(SkillFact_ApplyAndFind)
{
	SkillFactContainer container;
	const SkillId id = FactSkill(1, 1);

	CHECK(container.Apply(MakeFact(id, 10.0f)));

	const SkillFact* stored = container.Find(id);
	CHECK(stored != nullptr);
	CHECK_EQ(stored->skillId, id);
	CHECK_EQ(stored->level, static_cast<uint16_t>(3));
	CHECK_EQ(stored->remainingLifetime, 10.0f);
}

// Legacy's `bHOLD` gate: a record with nothing in it is not stored.
MODERN_TEST(SkillFact_InertFactIsRefused)
{
	SkillFactContainer container;

	SkillFact empty;
	empty.skillId           = FactSkill(1, 1);
	empty.remainingLifetime = 10.0f;

	CHECK(!FactHoldsAnything(empty));
	CHECK(!container.Apply(empty));
	CHECK_EQ(container.ActiveCount(), 0u);
}

MODERN_TEST(SkillFact_RemoveAndClear)
{
	SkillFactContainer container;
	const SkillId a = FactSkill(1, 1);
	const SkillId b = FactSkill(1, 2);
	(void) container.Apply(MakeFact(a, 10.0f));
	(void) container.Apply(MakeFact(b, 10.0f));

	CHECK(container.Remove(a));
	CHECK(!container.Has(a));
	CHECK(container.Has(b));
	CHECK(!container.Remove(a));   // already gone

	container.Clear();
	CHECK_EQ(container.ActiveCount(), 0u);
}

// ═══════════════════════════════════════════════════════════════════════
// Slot selection - GLChar.cpp:6377-6408
// ═══════════════════════════════════════════════════════════════════════

// Rule 1: the same skill reuses ITS slot. It does not consume a second one.
MODERN_TEST(SkillFact_SameSkillReusesItsSlot)
{
	SkillFactContainer container;
	const SkillId id = FactSkill(1, 1);

	CHECK_EQ(container.SelectSlot(id), 0u);
	(void) container.Apply(MakeFact(id, 10.0f));
	CHECK_EQ(container.SelectSlot(id), 0u);

	// A second skill takes the next free slot, not slot 0.
	const SkillId other = FactSkill(1, 2);
	CHECK_EQ(container.SelectSlot(other), 1u);
	(void) container.Apply(MakeFact(other, 10.0f));
	CHECK_EQ(container.ActiveCount(), 2u);
}

// Rule 2: the first empty slot wins, lowest index.
MODERN_TEST(SkillFact_FirstEmptySlotIsUsed)
{
	SkillFactContainer container;
	FillPool(container, 3, 10.0f);

	CHECK_EQ(container.SelectSlot(FactSkill(2, 1)), 3u);
}

// Rule 2 beats rule 3: an empty slot is preferred even when every occupied slot
// has a longer remaining time than the eviction candidate would.
MODERN_TEST(SkillFact_EmptySlotBeatsEviction)
{
	SkillFactContainer container;
	// One long-lived fact.
	(void) container.Apply(MakeFact(FactSkill(1, 1), 100.0f));

	// Slot 1 is free and must be chosen.
	CHECK_EQ(container.SelectSlot(FactSkill(2, 1)), 1u);
}

// Rule 3: with a FULL pool, the entry nearest to expiry is evicted. NOT the
// strongest, NOT the oldest, and the new fact is NOT refused.
//
// Note the pool really must be full: rule 2 (first empty slot) is checked in the
// same loop and returns immediately, so eviction is only reachable once every
// one of the fourteen slots is occupied.
MODERN_TEST(SkillFact_FullPoolEvictsTheNearestToExpiry)
{
	SkillFactContainer container;

	// Fourteen facts, lifetime 100 minus the slot index, so slot 13 is the
	// nearest to expiry and slot 0 the furthest.
	for (uint8_t i = 0; i < kSkillFactSlotCount; ++i)
	{
		(void) container.Apply(MakeFact(FactSkill(1, i), 100.0f - static_cast<float>(i)));
	}
	CHECK(container.Full());
	CHECK_EQ(container.ActiveCount(), kSkillFactSlotCount);

	// Slot 13 holds the shortest remaining lifetime.
	CHECK_EQ(container.SelectSlot(FactSkill(9, 9)), 13u);

	(void) container.Apply(MakeFact(FactSkill(9, 9), 1.0f));

	CHECK(container.Has(FactSkill(9, 9)));
	CHECK(!container.Has(FactSkill(1, 13)));   // the nearest-to-expiry was evicted
	CHECK(container.Has(FactSkill(1, 0)));    // the longest survives
	CHECK(container.Has(FactSkill(1, 12)));
	CHECK_EQ(container.ActiveCount(), kSkillFactSlotCount);
}

// A partially filled pool never evicts: the first empty slot wins.
MODERN_TEST(SkillFact_PartiallyFullPoolPrefersTheEmptySlotOverEviction)
{
	SkillFactContainer container;
	// One very short-lived fact in slot 0, everything else free.
	(void) container.Apply(MakeFact(FactSkill(1, 0), 1.0f));

	// Slot 1 is empty, so it is chosen even though slot 0 has the smallest
	// remaining time in the pool.
	CHECK_EQ(container.SelectSlot(FactSkill(2, 2)), 1u);
}

MODERN_TEST(SkillFact_FullPoolDoesNotRefuseTheNewFact)
{
	SkillFactContainer container;
	FillPool(container, kSkillFactSlotCount, 10.0f);
	CHECK(container.Full());

	CHECK(container.Apply(MakeFact(FactSkill(7, 7), 10.0f)));

	CHECK_EQ(container.ActiveCount(), kSkillFactSlotCount);
	CHECK(container.Has(FactSkill(7, 7)));
}

// There is no stacking and no strength comparison: the assignment wins outright.
MODERN_TEST(SkillFact_RecastingRefreshesRatherThanStacking)
{
	SkillFactContainer container;
	const SkillId id = FactSkill(1, 1);

	(void) container.Apply(MakeFact(id, 10.0f));
	container.Tick(6.0f);
	CHECK_EQ(container.At(0)->remainingLifetime, 4.0f);

	(void) container.Apply(MakeFact(id, 10.0f));

	CHECK_EQ(container.ActiveCount(), 1u);
	CHECK_EQ(container.At(0)->remainingLifetime, 10.0f);
}

// ═══════════════════════════════════════════════════════════════════════
// Data preservation
// ═══════════════════════════════════════════════════════════════════════

MODERN_TEST(SkillFact_AllRuntimeFieldsArePreserved)
{
	SkillFactContainer container;

	SkillFact fact;
	fact.skillId           = FactSkill(4, 2);
	fact.level             = 7;
	fact.remainingLifetime = 33.5f;
	fact.basicType         = PassiveApplyType::Ma;
	fact.basicValue        = -12.25f;
	fact.specialSkill      = 0xABCD;
	fact.casterCrow        = 3;
	fact.casterId          = 999;

	fact.impacts[0].type  = SkillFactImpactType::Pa;
	fact.impacts[0].value = 17.0f;

	fact.specs[1].type     = SkillFactSpecType::MagicDamageReflection;
	fact.specs[1].var1     = 0.25f;
	fact.specs[1].var2     = 0.75f;
	fact.specs[1].specFlag = 0xDEADBEEF;
	fact.specs[1].nativeId = 0x12345678;

	CHECK(container.Apply(fact));

	const SkillFact* stored = container.Find(FactSkill(4, 2));
	CHECK(stored != nullptr);
	CHECK_EQ(stored->level, static_cast<uint16_t>(7));
	CHECK_EQ(stored->remainingLifetime, 33.5f);
	CHECK_EQ(stored->basicType, PassiveApplyType::Ma);
	CHECK_EQ(stored->basicValue, -12.25f);
	CHECK_EQ(stored->specialSkill, 0xABCDu);
	CHECK_EQ(stored->casterCrow, static_cast<uint16_t>(3));
	CHECK_EQ(stored->casterId, 999u);
	CHECK_EQ(stored->impacts[0].type, SkillFactImpactType::Pa);
	CHECK_EQ(stored->impacts[0].value, 17.0f);
	CHECK_EQ(stored->specs[1].type, SkillFactSpecType::MagicDamageReflection);
	CHECK_EQ(stored->specs[1].var1, 0.25f);
	CHECK_EQ(stored->specs[1].var2, 0.75f);
	CHECK_EQ(stored->specs[1].specFlag, 0xDEADBEEFu);
	CHECK_EQ(stored->specs[1].nativeId, 0x12345678u);

	// The lookup helpers find it too.
	CHECK(stored->HasSpec(SkillFactSpecType::MagicDamageReflection));
	CHECK(stored->HasImpact(SkillFactImpactType::Pa));
	CHECK(stored->FindSpec(SkillFactSpecType::MagicDamageReflection) != nullptr);
	CHECK(stored->FindImpact(SkillFactImpactType::Pa) != nullptr);
	CHECK(!stored->HasSpec(SkillFactSpecType::MoveVelo));
}

// ═══════════════════════════════════════════════════════════════════════
// Lifetime
// ═══════════════════════════════════════════════════════════════════════

MODERN_TEST(SkillFact_PositiveLifetimeIsActive)
{
	SkillFactContainer container;
	(void) container.Apply(MakeFact(FactSkill(1, 1), 10.0f));

	CHECK_EQ(container.Tick(0.0f), 0u);
	CHECK(container.Has(FactSkill(1, 1)));
}

MODERN_TEST(SkillFact_PartialTickRemainsActive)
{
	SkillFactContainer container;
	(void) container.Apply(MakeFact(FactSkill(1, 1), 10.0f));

	CHECK_EQ(container.Tick(4.0f), 0u);
	CHECK(container.Has(FactSkill(1, 1)));
	CHECK_EQ(container.At(0)->remainingLifetime, 6.0f);
}

MODERN_TEST(SkillFact_ExactZeroExpires)
{
	SkillFactContainer container;
	(void) container.Apply(MakeFact(FactSkill(1, 1), 10.0f));

	CHECK_EQ(container.Tick(10.0f), 1u);
	CHECK(!container.Has(FactSkill(1, 1)));
	CHECK_EQ(container.ActiveCount(), 0u);
}

MODERN_TEST(SkillFact_OverrunExpires)
{
	SkillFactContainer container;
	(void) container.Apply(MakeFact(FactSkill(1, 1), 10.0f));

	CHECK_EQ(container.Tick(25.0f), 1u);
	CHECK(!container.Has(FactSkill(1, 1)));
}

// A FACT created with a non-positive lifetime expires on the first tick.
MODERN_TEST(SkillFact_NegativeLifetimeExpiresImmediately)
{
	SkillFactContainer container;
	(void) container.Apply(MakeFact(FactSkill(1, 1), -1.0f));

	CHECK_EQ(container.Tick(0.0f), 1u);
	CHECK(!container.Has(FactSkill(1, 1)));
}

// ═══════════════════════════════════════════════════════════════════════
// Aggregation - GLogixExPC.cpp:2276-2410
// ═══════════════════════════════════════════════════════════════════════

// No FACT is the baseline: every accumulator at its default.
MODERN_TEST(SkillFact_EmptyPoolAggregatesToBaseline)
{
	SkillFactContainer container;

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.moveVelocity, 0.0f);
	CHECK_EQ(modifiers.attackVelocity, 0.0f);
	CHECK_EQ(modifiers.statusImmunityMask, 0u);
	CHECK_EQ(modifiers.prohibitSkill, false);
	CHECK_EQ(modifiers.prohibitPotion, false);
	CHECK_EQ(modifiers.psyDamageReduce, 0.0f);
	CHECK_EQ(modifiers.magicDamageReduce, 0.0f);
}

// MOVEVELO is ADDITIVE with no sign flip.
MODERN_TEST(SkillFact_MoveVelocityIsAdditive)
{
	SkillFactContainer container;

	SkillFact a = MakeFact(FactSkill(1, 1), 10.0f, SkillFactSpecType::MoveVelo, 0.1f);
	SkillFact b = MakeFact(FactSkill(1, 2), 10.0f, SkillFactSpecType::MoveVelo, 0.25f);
	(void) container.Apply(a);
	(void) container.Apply(b);

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.moveVelocity, 0.35f);
}

// ATTACKVELO is SUBTRACTED: a positive value SLOWS the attacker. This is the
// one that reads backwards, and legacy's own comment at :2363 confirms it.
MODERN_TEST(SkillFact_AttackVelocityIsSignInverted)
{
	SkillFactContainer container;
	(void) container.Apply(MakeFact(FactSkill(1, 1), 10.0f, SkillFactSpecType::AttackVelo, 0.2f));

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	// 0 - 0.2
	CHECK_EQ(modifiers.attackVelocity, -0.2f);
}

// A negative ATTACKVELO value - which is how RAN enters a speed-up - increases it.
MODERN_TEST(SkillFact_NegativeAttackVelocitySpeedsUp)
{
	SkillFactContainer container;
	(void) container.Apply(MakeFact(FactSkill(1, 1), 10.0f, SkillFactSpecType::AttackVelo, -0.1f));

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.attackVelocity, 0.1f);
}

MODERN_TEST(SkillFact_AttackVelocityAccumulatesAcrossFacts)
{
	SkillFactContainer container;
	(void) container.Apply(MakeFact(FactSkill(1, 1), 10.0f, SkillFactSpecType::AttackVelo, 0.1f));
	(void) container.Apply(MakeFact(FactSkill(1, 2), 10.0f, SkillFactSpecType::AttackVelo, 0.05f));

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.attackVelocity, -0.15f);
}

// NONBLOW carries dwSPECFLAG as an immunity bitmask.
MODERN_TEST(SkillFact_NonBlowContributesTheImmunityMask)
{
	SkillFactContainer container;

	SkillFact fact = MakeFact(FactSkill(1, 1), 10.0f, SkillFactSpecType::NonBlow, 0.0f);
	fact.specs[0].specFlag = 0x02u;   // DIS_STUN
	(void) container.Apply(fact);

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.statusImmunityMask, 0x02u);
}

// NONBLOW is an ASSIGNMENT, not an OR - the last fact aggregated wins outright.
// Two different masks do not combine.
MODERN_TEST(SkillFact_NonBlowMaskIsAssignedNotCombined)
{
	SkillFactContainer container;

	SkillFact stunImmune = MakeFact(FactSkill(1, 1), 10.0f, SkillFactSpecType::NonBlow, 0.0f);
	stunImmune.specs[0].specFlag = 0x02u;
	SkillFact burnImmune = MakeFact(FactSkill(1, 2), 10.0f, SkillFactSpecType::NonBlow, 0.0f);
	burnImmune.specs[0].specFlag = 0x08u;

	(void) container.Apply(stunImmune);   // slot 0
	(void) container.Apply(burnImmune);   // slot 1

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	// Slot 1 is aggregated last, so its mask replaces the other's entirely.
	CHECK_EQ(modifiers.statusImmunityMask, 0x08u);
}

// Damage reduction is a MAX, not a sum. Two buffs do not add up.
MODERN_TEST(SkillFact_DamageReduceTakesTheStrongestNotTheSum)
{
	SkillFactContainer container;
	(void) container.Apply(MakeFact(FactSkill(1, 1), 10.0f, SkillFactSpecType::PsyDamageReduce, 0.2f));
	(void) container.Apply(MakeFact(FactSkill(1, 2), 10.0f, SkillFactSpecType::PsyDamageReduce, 0.5f));
	(void) container.Apply(MakeFact(FactSkill(1, 3), 10.0f, SkillFactSpecType::PsyDamageReduce, 0.1f));

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.psyDamageReduce, 0.5f);
}

MODERN_TEST(SkillFact_PsyAndMagicReduceAreIndependent)
{
	SkillFactContainer container;
	(void) container.Apply(MakeFact(FactSkill(1, 1), 10.0f, SkillFactSpecType::PsyDamageReduce, 0.4f));
	(void) container.Apply(MakeFact(FactSkill(1, 2), 10.0f, SkillFactSpecType::MagicDamageReduce, 0.9f));

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.psyDamageReduce, 0.4f);
	CHECK_EQ(modifiers.magicDamageReduce, 0.9f);
}

// Reflection amount AND rate come from the one strongest spec, never mixed.
MODERN_TEST(SkillFact_ReflectionPairsAmountAndRateFromOneSpec)
{
	SkillFactContainer container;

	SkillFact weaker = MakeFact(FactSkill(1, 1), 10.0f, SkillFactSpecType::PsyDamageReflection, 0.2f);
	weaker.specs[0].var2 = 0.10f;
	SkillFact stronger = MakeFact(FactSkill(1, 2), 10.0f, SkillFactSpecType::PsyDamageReflection, 0.6f);
	stronger.specs[0].var2 = 0.80f;

	(void) container.Apply(weaker);
	(void) container.Apply(stronger);

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.psyDamageReflection, 0.6f);
	// The rate comes from the stronger spec too, not the last one seen.
	CHECK_EQ(modifiers.psyDamageReflectionRate, 0.80f);
}

MODERN_TEST(SkillFact_MagicReflectionIsIndependentOfPsy)
{
	SkillFactContainer container;

	SkillFact psy = MakeFact(FactSkill(1, 1), 10.0f, SkillFactSpecType::PsyDamageReflection, 0.3f);
	psy.specs[0].var2 = 0.3f;
	SkillFact magic = MakeFact(FactSkill(1, 2), 10.0f, SkillFactSpecType::MagicDamageReflection, 0.7f);
	magic.specs[0].var2 = 0.7f;

	(void) container.Apply(psy);
	(void) container.Apply(magic);

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.psyDamageReflection, 0.3f);
	CHECK_EQ(modifiers.psyDamageReflectionRate, 0.3f);
	CHECK_EQ(modifiers.magicDamageReflection, 0.7f);
	CHECK_EQ(modifiers.magicDamageReflectionRate, 0.7f);
}

MODERN_TEST(SkillFact_ProhibitFlagsAreSet)
{
	SkillFactContainer container;
	(void) container.Apply(MakeFact(FactSkill(1, 1), 10.0f, SkillFactSpecType::ProhibitSkill, 0.0f));
	(void) container.Apply(MakeFact(FactSkill(1, 2), 10.0f, SkillFactSpecType::ProhibitPotion, 0.0f));

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.prohibitSkill, true);
	CHECK_EQ(modifiers.prohibitPotion, true);
}

// ═══════════════════════════════════════════════════════════════════════
// Expiry restores the baseline - because legacy recomputes from zero
// ═══════════════════════════════════════════════════════════════════════

// There is no "restore" step anywhere: the accumulators are rebuilt from their
// defaults on every call, so an expired fact simply stops being counted.
MODERN_TEST(SkillFact_ExpiredFactRestoresTheBaseline)
{
	SkillFactContainer container;
	(void) container.Apply(MakeFact(FactSkill(1, 1), 10.0f, SkillFactSpecType::MoveVelo, 0.4f));

	CHECK_EQ(AdvanceSkillFacts(container, 1.0f).modifiers.moveVelocity, 0.4f);

	const SkillFactAdvanceResult expiring = AdvanceSkillFacts(container, 9.0f);
	CHECK_EQ(expiring.expiredCount, 1u);
	CHECK_EQ(container.ActiveCount(), 0u);

	// Next pass: baseline again, with no restore call anywhere.
	CHECK_EQ(AdvanceSkillFacts(container, 1.0f).modifiers.moveVelocity, 0.0f);
}

// VERIFIED LEGACY OFF-BY-ONE.
//
// GLogixExPC.cpp:2292-2295 decrements fAGE and calls DISABLESKEFF, but the
// spec/impact switches below run regardless, because the loop already passed its
// `continue` guard. A fact therefore still contributes on the tick it expires.
// Reproduced, because it is what RAN does.
MODERN_TEST(SkillFact_ExpiringFactStillContributesOnItsFinalTick)
{
	SkillFactContainer container;
	(void) container.Apply(MakeFact(FactSkill(1, 1), 10.0f, SkillFactSpecType::MoveVelo, 0.4f));

	AdvanceSkillFacts(container, 1.0f);
	CHECK_EQ(container.ActiveCount(), 1u);

	// This tick expires it: remainingLifetime goes to 0.
	const SkillFactAdvanceResult final = AdvanceSkillFacts(container, 9.0f);

	CHECK_EQ(final.expiredCount, 1u);
	// ...and it still contributed.
	CHECK_EQ(final.modifiers.moveVelocity, 0.4f);

	// The NEXT tick is clean.
	CHECK_EQ(AdvanceSkillFacts(container, 1.0f).modifiers.moveVelocity, 0.0f);
}

// The exemption is one tick only, and a fact that expires still frees its slot.
MODERN_TEST(SkillFact_ExpiredFactFreesItsSlotImmediately)
{
	SkillFactContainer container;
	(void) container.Apply(MakeFact(FactSkill(1, 1), 5.0f, SkillFactSpecType::MoveVelo, 0.4f));

	AdvanceSkillFacts(container, 5.0f);

	CHECK_EQ(container.ActiveCount(), 0u);
	// The slot is reusable right away.
	CHECK_EQ(container.SelectSlot(FactSkill(2, 2)), 0u);
}

// Two facts expiring on the same tick both report, and both are counted once.
MODERN_TEST(SkillFact_MultipleExpiriesAreAllCounted)
{
	SkillFactContainer container;
	(void) container.Apply(MakeFact(FactSkill(1, 1), 5.0f, SkillFactSpecType::MoveVelo, 0.1f));
	(void) container.Apply(MakeFact(FactSkill(1, 2), 5.0f, SkillFactSpecType::MoveVelo, 0.2f));

	const SkillFactAdvanceResult result = AdvanceSkillFacts(container, 5.0f);

	CHECK_EQ(result.expiredCount, 2u);
	// Both still contributed on their final tick.
	CHECK_EQ(result.modifiers.moveVelocity, 0.3f);
	CHECK_EQ(container.ActiveCount(), 0u);
}

// ═══════════════════════════════════════════════════════════════════════
// Aggregation over an already-ticked pool
// ═══════════════════════════════════════════════════════════════════════

MODERN_TEST(SkillFact_AggregateAfterTickSkipsExpired)
{
	SkillFactContainer container;
	(void) container.Apply(MakeFact(FactSkill(1, 1), 10.0f, SkillFactSpecType::MoveVelo, 0.4f));

	// Using the lifetime primitive instead, the expiring fact is gone before
	// aggregation, so it contributes nothing. This is the documented difference
	// between the two entry points.
	CHECK_EQ(container.Tick(10.0f), 1u);
	CHECK_EQ(AggregateSkillFacts(container).moveVelocity, 0.0f);
}

// ═══════════════════════════════════════════════════════════════════════
// Determinism
// ═══════════════════════════════════════════════════════════════════════

// Same pool, same elapsed time, same answer - no clock and no RNG anywhere.
MODERN_TEST(SkillFact_AggregationIsDeterministic)
{
	SkillFactContainer a;
	SkillFactContainer b;
	for (uint32_t i = 0; i < 4; ++i)
	{
		const SkillId id = FactSkill(1, static_cast<uint16_t>(i));
		(void) a.Apply(MakeFact(id, 10.0f + static_cast<float>(i),
		                        SkillFactSpecType::MoveVelo, 0.1f * static_cast<float>(i)));
		(void) b.Apply(MakeFact(id, 10.0f + static_cast<float>(i),
		                        SkillFactSpecType::MoveVelo, 0.1f * static_cast<float>(i)));
	}

	const SkillFactModifiers first  = AdvanceSkillFacts(a, 2.0f).modifiers;
	const SkillFactModifiers second = AdvanceSkillFacts(b, 2.0f).modifiers;

	CHECK_EQ(first.moveVelocity, second.moveVelocity);
	CHECK_EQ(first.moveVelocity, 0.0f + 0.1f + 0.2f + 0.3f);
}
// ═══════════════════════════════════════════════════════════════════════
// VERTICAL-017: proven FACT consumers
// ═══════════════════════════════════════════════════════════════════════

namespace
{
	// A FACT carrying one impact and one spec.
	SkillFact MakeImpactFact(const SkillId& id, float lifetime,
	                         SkillFactImpactType impact, float impactValue,
	                         SkillFactSpecType spec, float specVar1, float specVar2 = 0.0f)
	{
		SkillFact fact;
		fact.skillId                  = id;
		fact.level                    = 1;
		fact.remainingLifetime        = lifetime;
		fact.basicType                = PassiveApplyType::VarHp;
		fact.basicValue               = 1.0f;
		fact.impacts[0].type          = impact;
		fact.impacts[0].value         = impactValue;
		fact.specs[0].type            = spec;
		fact.specs[0].var1            = specVar1;
		fact.specs[0].var2            = specVar2;
		return fact;
	}
}

// ── PA / SA / MA ──────────────────────────────────────────────────────
//
// GLogixExPC.cpp:2343-2345 - `nSUM_PA += int(fADDON_VAR)`. SUM with int()
// truncation, so unlike the damage reductions two buffs DO add up.

MODERN_TEST(SkillFactConsumers_NoFactMeansNoPowerBonus)
{
	SkillFactContainer container;

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.meleePower, 0);
	CHECK_EQ(modifiers.shootPower, 0);
	CHECK_EQ(modifiers.magicAttack, 0);
}

MODERN_TEST(SkillFactConsumers_OnePowerFactContributes)
{
	SkillFactContainer container;
	(void) container.Apply(MakeImpactFact(FactSkill(1, 1), 10.0f,
	                                      SkillFactImpactType::Pa, 7.0f,
	                                      SkillFactSpecType::None, 0.0f));

	CHECK_EQ(AdvanceSkillFacts(container, 1.0f).modifiers.meleePower, 7);
}

MODERN_TEST(SkillFactConsumers_PowersSumAcrossFacts)
{
	SkillFactContainer container;
	(void) container.Apply(MakeImpactFact(FactSkill(1, 1), 10.0f,
	                                      SkillFactImpactType::Pa, 7.0f,
	                                      SkillFactSpecType::None, 0.0f));
	(void) container.Apply(MakeImpactFact(FactSkill(1, 2), 10.0f,
	                                      SkillFactImpactType::Pa, 5.0f,
	                                      SkillFactSpecType::None, 0.0f));
	(void) container.Apply(MakeImpactFact(FactSkill(1, 3), 10.0f,
	                                      SkillFactImpactType::Ma, 11.0f,
	                                      SkillFactSpecType::None, 0.0f));

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	// SUM, so 7 + 5 - not the strongest one.
	CHECK_EQ(modifiers.meleePower, 12);
	CHECK_EQ(modifiers.magicAttack, 11);
}

// Legacy truncates with `int(...)`, so a fractional impact loses its fraction
// on the way into the accumulator.
MODERN_TEST(SkillFactConsumers_PowerImpactIsIntTruncated)
{
	SkillFactContainer container;
	(void) container.Apply(MakeImpactFact(FactSkill(1, 1), 10.0f,
	                                      SkillFactImpactType::Sa, 7.9f,
	                                      SkillFactSpecType::None, 0.0f));

	CHECK_EQ(AdvanceSkillFacts(container, 1.0f).modifiers.shootPower, 7);
}

// ── Damage reduction ──────────────────────────────────────────────────
//
// MAX, not SUM. Legacy `if ( m_sDamageSpec.m_fPsyDamageReduce < v ) m_... = v;`
// (GLogixExPC.cpp:2380-2381). The example the milestone asks about: 0.10 + 0.20
// must be 0.20, not 0.30.

MODERN_TEST(SkillFactConsumers_NoFactMeansNoReduction)
{
	SkillFactContainer container;

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.psyDamageReduce, 0.0f);
	CHECK_EQ(modifiers.magicDamageReduce, 0.0f);
}

MODERN_TEST(SkillFactConsumers_OneReductionFact)
{
	SkillFactContainer container;
	(void) container.Apply(MakeImpactFact(FactSkill(1, 1), 10.0f,
	                                      SkillFactImpactType::None, 0.0f,
	                                      SkillFactSpecType::PsyDamageReduce, 0.2f));

	CHECK_EQ(AdvanceSkillFacts(container, 1.0f).modifiers.psyDamageReduce, 0.2f);
}

MODERN_TEST(SkillFactConsumers_TwoReductionsTakeTheStrongestNotTheSum)
{
	SkillFactContainer container;
	(void) container.Apply(MakeImpactFact(FactSkill(1, 1), 10.0f,
	                                      SkillFactImpactType::None, 0.0f,
	                                      SkillFactSpecType::PsyDamageReduce, 0.10f));
	(void) container.Apply(MakeImpactFact(FactSkill(1, 2), 10.0f,
	                                      SkillFactImpactType::None, 0.0f,
	                                      SkillFactSpecType::PsyDamageReduce, 0.20f));

	// 0.10 + 0.20 must be 0.20.
	CHECK_EQ(AdvanceSkillFacts(container, 1.0f).modifiers.psyDamageReduce, 0.20f);
}

MODERN_TEST(SkillFactConsumers_WeakerSecondReductionDoesNotLowerTheValue)
{
	SkillFactContainer container;
	(void) container.Apply(MakeImpactFact(FactSkill(1, 1), 10.0f,
	                                      SkillFactImpactType::None, 0.0f,
	                                      SkillFactSpecType::PsyDamageReduce, 0.5f));
	(void) container.Apply(MakeImpactFact(FactSkill(1, 2), 10.0f,
	                                      SkillFactImpactType::None, 0.0f,
	                                      SkillFactSpecType::PsyDamageReduce, 0.1f));

	CHECK_EQ(AdvanceSkillFacts(container, 1.0f).modifiers.psyDamageReduce, 0.5f);
}

MODERN_TEST(SkillFactConsumers_MagicReductionIsIndependentOfPhysical)
{
	SkillFactContainer container;
	(void) container.Apply(MakeImpactFact(FactSkill(1, 1), 10.0f,
	                                      SkillFactImpactType::None, 0.0f,
	                                      SkillFactSpecType::PsyDamageReduce, 0.3f));
	(void) container.Apply(MakeImpactFact(FactSkill(1, 2), 10.0f,
	                                      SkillFactImpactType::None, 0.0f,
	                                      SkillFactSpecType::MagicDamageReduce, 0.8f));

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.psyDamageReduce, 0.3f);
	CHECK_EQ(modifiers.magicDamageReduce, 0.8f);
}

MODERN_TEST(SkillFactConsumers_ExpiredReductionReturnsToBaseline)
{
	SkillFactContainer container;
	(void) container.Apply(MakeImpactFact(FactSkill(1, 1), 5.0f,
	                                      SkillFactImpactType::None, 0.0f,
	                                      SkillFactSpecType::PsyDamageReduce, 0.4f));

	CHECK_EQ(AdvanceSkillFacts(container, 1.0f).modifiers.psyDamageReduce, 0.4f);

	AdvanceSkillFacts(container, 5.0f);
	// The rebuild is from zero, so there is nothing to restore.
	CHECK_EQ(AdvanceSkillFacts(container, 1.0f).modifiers.psyDamageReduce, 0.0f);
}

// ── Reflection ────────────────────────────────────────────────────────
//
// MAX on the amount, and the rate is taken from that SAME spec
// (GLogixExPC.cpp:2390-2394). Amount and rate are never mixed.

MODERN_TEST(SkillFactConsumers_ReflectionTakesAmountAndRateFromOneSpec)
{
	SkillFactContainer container;
	(void) container.Apply(MakeImpactFact(FactSkill(1, 1), 10.0f,
	                                      SkillFactImpactType::None, 0.0f,
	                                      SkillFactSpecType::PsyDamageReflection, 0.3f, 0.9f));

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.psyDamageReflection, 0.3f);
	CHECK_EQ(modifiers.psyDamageReflectionRate, 0.9f);
}

MODERN_TEST(SkillFactConsumers_StrongerReflectionSupersedesThePair)
{
	SkillFactContainer container;
	(void) container.Apply(MakeImpactFact(FactSkill(1, 1), 10.0f,
	                                      SkillFactImpactType::None, 0.0f,
	                                      SkillFactSpecType::PsyDamageReflection, 0.2f, 0.10f));
	(void) container.Apply(MakeImpactFact(FactSkill(1, 2), 10.0f,
	                                      SkillFactImpactType::None, 0.0f,
	                                      SkillFactSpecType::PsyDamageReflection, 0.6f, 0.80f));

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.psyDamageReflection, 0.6f);
	// The rate comes from the winning spec, NOT from whichever was seen last.
	CHECK_EQ(modifiers.psyDamageReflectionRate, 0.80f);
}

MODERN_TEST(SkillFactConsumers_WeakerReflectionDoesNotOverwriteThePair)
{
	SkillFactContainer container;
	// The stronger fact is applied second, so slot order alone would pick it if
	// the rule were "last wins" rather than "strongest wins".
	(void) container.Apply(MakeImpactFact(FactSkill(1, 1), 10.0f,
	                                      SkillFactImpactType::None, 0.0f,
	                                      SkillFactSpecType::PsyDamageReflection, 0.9f, 0.90f));
	(void) container.Apply(MakeImpactFact(FactSkill(1, 2), 10.0f,
	                                      SkillFactImpactType::None, 0.0f,
	                                      SkillFactSpecType::PsyDamageReflection, 0.1f, 0.10f));

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.psyDamageReflection, 0.9f);
	CHECK_EQ(modifiers.psyDamageReflectionRate, 0.90f);
}

MODERN_TEST(SkillFactConsumers_MagicReflectionIsIndependentOfPhysical)
{
	SkillFactContainer container;
	(void) container.Apply(MakeImpactFact(FactSkill(1, 1), 10.0f,
	                                      SkillFactImpactType::None, 0.0f,
	                                      SkillFactSpecType::PsyDamageReflection, 0.3f, 0.3f));
	(void) container.Apply(MakeImpactFact(FactSkill(1, 2), 10.0f,
	                                      SkillFactImpactType::None, 0.0f,
	                                      SkillFactSpecType::MagicDamageReflection, 0.7f, 0.7f));

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.psyDamageReflection, 0.3f);
	CHECK_EQ(modifiers.magicDamageReflection, 0.7f);
}

MODERN_TEST(SkillFactConsumers_ExpiredReflectionReturnsToBaseline)
{
	SkillFactContainer container;
	(void) container.Apply(MakeImpactFact(FactSkill(1, 1), 5.0f,
	                                      SkillFactImpactType::None, 0.0f,
	                                      SkillFactSpecType::MagicDamageReflection, 0.5f, 0.5f));

	AdvanceSkillFacts(container, 1.0f);

	AdvanceSkillFacts(container, 5.0f);
	const SkillFactModifiers after = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(after.magicDamageReflection, 0.0f);
	CHECK_EQ(after.magicDamageReflectionRate, 0.0f);
}

// ── Power impacts and reduction coexist on one fact ───────────────────

MODERN_TEST(SkillFactConsumers_ImpactAndSpecCombineOnOneFact)
{
	SkillFactContainer container;
	(void) container.Apply(MakeImpactFact(FactSkill(1, 1), 10.0f,
	                                      SkillFactImpactType::Ma, 20.0f,
	                                      SkillFactSpecType::MagicDamageReduce, 0.5f));

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.magicAttack, 20);
	CHECK_EQ(modifiers.magicDamageReduce, 0.5f);
}
// ═══════════════════════════════════════════════════════════════════════
// VERTICAL-019: hit / avoid / damage aggregation
// ═══════════════════════════════════════════════════════════════════════

namespace
{
	SkillFact MakeHitAvoidDamageFact(const SkillId& id, float lifetime,
	                                  SkillFactImpactType impact, float value)
	{
		SkillFact fact;
		fact.skillId           = id;
		fact.level             = 1;
		fact.remainingLifetime = lifetime;
		fact.basicType         = PassiveApplyType::VarHp;
		fact.basicValue        = 1.0f;
		fact.impacts[0].type    = impact;
		fact.impacts[0].value   = value;
		return fact;
	}
}

// All three are SUM - the opposite of the MAX used by the reduction specs. A
// reader carrying over "buffs take the strongest value" would get all three
// wrong, so each direction is pinned.

MODERN_TEST(SkillFactV019_NoFactMeansNoHitAvoidDamage)
{
	SkillFactContainer container;

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.hit, 0);
	CHECK_EQ(modifiers.avoid, 0);
	CHECK_EQ(modifiers.damage, 0);
}

MODERN_TEST(SkillFactV019_HitRateIsAdditive)
{
	SkillFactContainer container;
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 1), 10.0f,
	                                              SkillFactImpactType::HitRate, 5.0f));
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 2), 10.0f,
	                                              SkillFactImpactType::HitRate, 7.0f));
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 3), 10.0f,
	                                              SkillFactImpactType::HitRate, 3.0f));

	// 5 + 7 + 3 = 15.
	CHECK_EQ(AdvanceSkillFacts(container, 1.0f).modifiers.hit, 15);
}

MODERN_TEST(SkillFactV019_AvoidRateIsAdditive)
{
	SkillFactContainer container;
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 1), 10.0f,
	                                              SkillFactImpactType::AvoidRate, 9.0f));
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 2), 10.0f,
	                                              SkillFactImpactType::AvoidRate, 4.0f));

	CHECK_EQ(AdvanceSkillFacts(container, 1.0f).modifiers.avoid, 13);
}

MODERN_TEST(SkillFactV019_DamageIsAdditive)
{
	SkillFactContainer container;
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 1), 10.0f,
	                                              SkillFactImpactType::Damage, 10.0f));
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 2), 10.0f,
	                                              SkillFactImpactType::Damage, 6.0f));

	CHECK_EQ(AdvanceSkillFacts(container, 1.0f).modifiers.damage, 16);
}

// `int(fADDON_VAR)` truncates toward zero on every one of them.
MODERN_TEST(SkillFactV019_TruncationIsTowardZero)
{
	SkillFactContainer container;
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 1), 10.0f,
	                                              SkillFactImpactType::Damage, 7.9f));
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 2), 10.0f,
	                                              SkillFactImpactType::HitRate, -2.7f));

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.damage, 7);
	CHECK_EQ(modifiers.hit, -2);
}

// Legacy allows a signed value: `int()` of a negative float is well defined and
// nothing clamps it at the accumulator.
MODERN_TEST(SkillFactV019_NegativeValuesAreCarried)
{
	SkillFactContainer container;
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 1), 10.0f,
	                                              SkillFactImpactType::Damage, -5.0f));
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 2), 10.0f,
	                                              SkillFactImpactType::AvoidRate, -8.0f));

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.damage, -5);
	CHECK_EQ(modifiers.avoid, -8);
}

MODERN_TEST(SkillFactV019_ExpiryRemovesAllThree)
{
	SkillFactContainer container;
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 1), 5.0f,
	                                              SkillFactImpactType::HitRate, 11.0f));
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 2), 5.0f,
	                                              SkillFactImpactType::AvoidRate, 12.0f));
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 3), 5.0f,
	                                              SkillFactImpactType::Damage, 13.0f));

	CHECK_EQ(AdvanceSkillFacts(container, 1.0f).modifiers.hit, 11);

	AdvanceSkillFacts(container, 5.0f);
	const SkillFactModifiers after = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(after.hit, 0);
	CHECK_EQ(after.avoid, 0);
	CHECK_EQ(after.damage, 0);
}
// ═══════════════════════════════════════════════════════════════════════
// VERTICAL-020: defense and resistance
// ═══════════════════════════════════════════════════════════════════════

// GLogixExPC.cpp:2330 / :2349 - both SUM with int() truncation.
MODERN_TEST(SkillFactV020_DefenseIsAdditive)
{
	SkillFactContainer container;
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 1), 10.0f,
	                                              SkillFactImpactType::Defense, 8.0f));
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 2), 10.0f,
	                                              SkillFactImpactType::Defense, 5.0f));

	CHECK_EQ(AdvanceSkillFacts(container, 1.0f).modifiers.defense, 13);
}

MODERN_TEST(SkillFactV020_DefenseTruncatesAndAllowsNegative)
{
	SkillFactContainer container;
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 1), 10.0f,
	                                              SkillFactImpactType::Defense, 6.9f));
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 2), 10.0f,
	                                              SkillFactImpactType::Defense, -2.7f));

	const SkillFactModifiers modifiers = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(modifiers.defense, 4);   // 6 + (-2)
}

MODERN_TEST(SkillFactV020_ResistIsAdditive)
{
	SkillFactContainer container;
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 1), 10.0f,
	                                              SkillFactImpactType::Resist, 7.0f));
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 2), 10.0f,
	                                              SkillFactImpactType::Resist, 4.0f));

	CHECK_EQ(AdvanceSkillFacts(container, 1.0f).modifiers.resist, 11);
}

MODERN_TEST(SkillFactV020_DefenseAndResistExpireIndependently)
{
	SkillFactContainer container;
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 1), 5.0f,
	                                              SkillFactImpactType::Defense, 10.0f));
	(void) container.Apply(MakeHitAvoidDamageFact(FactSkill(1, 2), 50.0f,
	                                              SkillFactImpactType::Resist, 3.0f));

	AdvanceSkillFacts(container, 1.0f);
	AdvanceSkillFacts(container, 5.0f);   // defence expires, resist does not

	const SkillFactModifiers after = AdvanceSkillFacts(container, 1.0f).modifiers;

	CHECK_EQ(after.defense, 0);
	CHECK_EQ(after.resist, 3);
}