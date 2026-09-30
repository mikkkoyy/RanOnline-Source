// VERTICAL-006: basic physical combat resolution tests.
//
// All tests use deterministic random inputs (hitRoll, damageRoll, etc.)
// drawn from [0, 1] so results are reproducible.

#include "TestHarness.h"

#include "combat/CombatTypes.h"
#include "combat/CombatConstants.h"
#include "combat/HitCalculator.h"
#include "combat/PhysicalDamageCalculator.h"
#include "combat/CombatCalculator.h"
#include "engine/GameCharacterCalculations.h"
#include "equipment/EquipmentState.h"
#include "equipment/ItemContributionAggregator.h"
#include "equipment/ItemDefinitionProvider.h"
#include "item/ItemDefinition.h"
#include "item/ItemInstance.h"
#include "types/Ids.h"

#include <limits>

using namespace Modern;
using namespace Modern::Combat;
using namespace Modern::Engine;

namespace
{
	constexpr float kRoll0  = 0.0f;
	constexpr float kRoll05 = 0.5f;
	constexpr float kRoll1  = 1.0f;

	CombatInput MakeBasicInput()
	{
		CombatInput input;
		input.attackerHit = 50;
		input.targetAvoid = 10;
		input.attackerMeleePower = 3;
		input.attackerShootPower = 4;
		input.attackerPhysicalDamage = { 10, 20 };
		input.attackerLevel = 1;
		input.attackerMaxHP = 100;
		input.attackerCurrentHP = 100;
		input.attackerCriticalBonus = 0;
		input.attackerCrushingBonus = 0;
		input.attackType = AttackType::Melee;

		input.targetHit = 50;
		input.targetAvoid = 10;
		input.targetDefense = 10;
		input.targetDefenseBody = 0;
		input.targetDefenseItem = 0;
		input.targetLevel = 1;
		input.targetMaxHP = 100;
		input.targetCurrentHP = 100;
		input.targetStateDamage = 1.0f;
		input.targetDamageReduce = 0.0f;
		input.targetDamageReflection = 0.0f;
		input.targetDamageReflectionRate = 0.0f;
		input.targetResistElement = 0;
		input.targetLowSP = false;

		input.hitRoll = kRoll05;
		input.damageRoll = kRoll05;
		input.criticalRoll = kRoll05;
		input.crushingRoll = kRoll05;
		input.reflectionRoll = kRoll05;
		return input;
	}

	PhysicalDamageInput MakeBasicDamageInput()
	{
		PhysicalDamageInput input;
		input.physicalDamage = { 10, 20 };
		input.meleePower = 3;
		input.shootPower = 4;
		input.attackType = AttackType::Melee;
		input.defense = 10;
		input.defenseBody = 0;
		input.defenseItem = 0;
		input.level = 1;
		input.stateDamage = 1.0f;
		input.damageReduce = 0.0f;
		input.damageReflection = 0.0f;
		input.damageReflectionRate = 0.0f;
		input.resistElement = 0;
		input.lowSP = false;
		input.stateDamageMultiplier = 1.0f;
		input.attackerLevel = 1;
		input.attackerMaxHP = 100;
		input.attackerCurrentHP = 100;
		input.attackerCriticalBonus = 0;
		input.attackerCrushingBonus = 0;
		input.targetLevel = 1;
		input.targetMaxHP = 100;
		input.brightnessFB = GameBrightFB::Aver;
		input.weatherElementPower = 1.0f;
		input.targetResistElement = 0;
		input.fDamageReduce = 0.0f;
		input.fDamageReflection = 0.0f;
		input.fDamageReflectionRate = 0.0f;
		input.hitRoll = 0.5f;
		input.damageRoll = 0.5f;
		input.criticalRoll = 0.5f;
		input.crushingRoll = 0.5f;
		input.reflectionRoll = 0.5f;
		input.lowSPHitRoll = 0.5f;
		input.lowSPDamageRoll = 0.5f;
		return input;
	}
}

// ── Hit / Miss ────────────────────────────────────────────────────────

MODERN_TEST(Combat_ MissWhenAvoidExceedsHit)
{
	CombatConstants constants;
	CombatInput input = MakeBasicInput();
	input.attackerHit = 5;
	input.targetAvoid = 50;
	input.hitRoll = kRoll1;

	CombatResult result = Combat::ResolveCombat(input, constants);

	CHECK_EQ(result.IsHit(), false);
	CHECK_EQ(result.IsMiss(), true);
	CHECK_EQ(result.damageResult.damage, 0u);
	CHECK_EQ(result.targetHPBefore, 100u);
	CHECK_EQ(result.targetHPAfter, 100u);
}

MODERN_TEST(Combat_ HitWhenHitExceedsAvoid)
{
	CombatConstants constants;
	CombatInput input = MakeBasicInput();
	input.attackerHit = 99;
	input.targetAvoid = 1;
	input.hitRoll = kRoll0;

	CombatResult result = Combat::ResolveCombat(input, constants);

	CHECK_EQ(result.IsHit(), true);
	CHECK_EQ(result.IsMiss(), false);
	CHECK_GT(result.damageResult.damage, 0u);
}

MODERN_TEST(Combat_ HitRateClampedToMax)
{
	CombatConstants constants;
	HitInput hitInput;
	hitInput.attackerHit = 9999;
	hitInput.targetAvoid = 0;
	hitInput.brightnessFB = GameBrightFB::Aver;
	hitInput.lowSP = false;
	hitInput.hitRoll = 0.99f;

	HitResult result = CalculateHit(hitInput, constants);

	CHECK_EQ(result.hit, true);
	CHECK_EQ(result.hitRate, 99u);
}

MODERN_TEST(Combat_ HitRateClampedToMin)
{
	CombatConstants constants;
	HitInput hitInput;
	hitInput.attackerHit = 0;
	hitInput.targetAvoid = 9999;
	hitInput.brightnessFB = GameBrightFB::Aver;
	hitInput.lowSP = false;
	hitInput.hitRoll = 0.01f;

	HitResult result = CalculateHit(hitInput, constants);

	CHECK_EQ(result.hit, true);
	CHECK_EQ(result.hitRate, 20u);
}

MODERN_TEST(Combat_ LowSPReducesHitRate)
{
	CombatConstants constants;
	HitInput normal;
	normal.attackerHit = 50;
	normal.targetAvoid = 10;
	normal.brightnessFB = GameBrightFB::Aver;
	normal.lowSP = false;
	normal.hitRoll = 0.5f;

	HitInput lowSP = normal;
	lowSP.lowSP = true;

	HitResult rNormal = CalculateHit(normal, constants);
	HitResult rLow = CalculateHit(lowSP, constants);

	CHECK_GT(rNormal.hitRate, rLow.hitRate);
}

// ── Damage ────────────────────────────────────────────────────────────

MODERN_TEST(Combat_ DamageRangeMinRoll)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageRoll = 0.0f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_GT(result.damage, 0u);
	CHECK_LE(result.damage, 10u);
}

MODERN_TEST(Combat_ DamageRangeMaxRoll)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageRoll = 1.0f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_GT(result.damage, 15u);
}

MODERN_TEST(Combat_ DamageRangeMidRoll)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_GE(result.damage, 10u);
	CHECK_LE(result.damage, 20u);
}

MODERN_TEST(Combat_ DefenseReducesDamage)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 5;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_GT(result.damage, 0u);
	CHECK_LT(result.damage, 20u);
}

MODERN_TEST(Combat_ StateDamageMultiplier)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.stateDamage = 2.0f;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_GT(result.damage, 20u);
}

MODERN_TEST(Combat_ LowSPDamageModifier)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 10;
	input.lowSP = true;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_GT(result.damage, 0u);
}

MODERN_TEST(Combat_ MinimumDamageOneOnHit)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.physicalDamage = { 1, 1 };
	input.defense = 999;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_GE(result.damage, 1u);
}

MODERN_TEST(Combat_ MissProducesZeroDamage)
{
	CombatConstants constants;
	CombatInput input = MakeBasicInput();
	input.attackerHit = 5;
	input.targetAvoid = 50;
	input.hitRoll = kRoll1;

	CombatResult result = Combat::ResolveCombat(input, constants);

	CHECK_EQ(result.IsHit(), false);
	CHECK_EQ(result.damageResult.damage, 0u);
}

// ── Critical ──────────────────────────────────────────────────────────

MODERN_TEST(Combat_ CriticalHitOccurs)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageRoll = 0.5f;
	input.criticalRoll = 0.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.critical, true);
	CHECK_GT(result.damage, 15u);
}

MODERN_TEST(Combat_ NonCriticalHit)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.critical, false);
	CHECK_GE(result.damage, 10u);
	CHECK_LE(result.damage, 20u);
}

MODERN_TEST(Combat_ CriticalBoundary)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageRoll = 0.5f;
	input.crushingRoll = 1.0f;

	PhysicalDamageInput atBoundary = input;
	atBoundary.criticalRoll = 0.04f;
	DamageResult rBoundary = CalculatePhysicalDamage(atBoundary, constants);

	PhysicalDamageInput aboveBoundary = input;
	aboveBoundary.criticalRoll = 0.05f;
	DamageResult rAbove = CalculatePhysicalDamage(aboveBoundary, constants);

	CHECK_EQ(rBoundary.critical, true);
	CHECK_EQ(rAbove.critical, false);
}

// ── Crushing Blow ─────────────────────────────────────────────────────

MODERN_TEST(Combat_ CrushingBlowOccurs)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 0.0f;
	input.attackerCrushingBonus = 150;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.crushing, true);
	CHECK_GT(result.damage, 20u);
}

MODERN_TEST(Combat_ NonCrushingBlow)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;
	input.attackerCrushingBonus = 0;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.crushing, false);
}

MODERN_TEST(Combat_ CrushingBlowBoundary)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.attackerCrushingBonus = 20;

	PhysicalDamageInput atBoundary = input;
	atBoundary.crushingRoll = 0.19f;
	DamageResult rBoundary = CalculatePhysicalDamage(atBoundary, constants);

	PhysicalDamageInput aboveBoundary = input;
	aboveBoundary.crushingRoll = 0.21f;
	DamageResult rAbove = CalculatePhysicalDamage(aboveBoundary, constants);

	CHECK_EQ(rBoundary.crushing, true);
	CHECK_EQ(rAbove.crushing, false);
}

// ── Combined ──────────────────────────────────────────────────────────

MODERN_TEST(Combat_ CriticalAndCrushing)
{
	CombatConstants constants;
	CombatInput input = MakeBasicInput();
	input.attackerHit = 99;
	input.targetAvoid = 1;
	input.targetDefense = 10;
	input.targetDefenseBody = 0;
	input.targetDefenseItem = 0;
	input.attackerCrushingBonus = 150;
	input.hitRoll = kRoll05;
	input.damageRoll = kRoll05;
	input.criticalRoll = kRoll0;
	input.crushingRoll = kRoll0;

	CombatResult result = Combat::ResolveCombat(input, constants);

	CHECK_EQ(result.IsHit(), true);
	CHECK_EQ(result.IsCritical(), true);
	CHECK_EQ(result.IsCrushing(), true);
	CHECK_GT(result.damageResult.damage, 20u);
}

MODERN_TEST(Combat_ CriticalWithDefense)
{
	CombatConstants constants;
	CombatInput input = MakeBasicInput();
	input.attackerHit = 99;
	input.targetAvoid = 1;
	input.targetDefense = 10;
	input.targetDefenseBody = 0;
	input.targetDefenseItem = 0;
	input.hitRoll = kRoll05;
	input.damageRoll = kRoll05;
	input.criticalRoll = kRoll0;
	input.crushingRoll = kRoll1;

	CombatResult result = Combat::ResolveCombat(input, constants);

	CHECK_EQ(result.IsHit(), true);
	CHECK_EQ(result.IsCritical(), true);
	CHECK_GT(result.damageResult.damage, 0u);
}

MODERN_TEST(Combat_ LowSPWithCritical)
{
	CombatConstants constants;
	CombatInput input = MakeBasicInput();
	input.attackerHit = 99;
	input.targetAvoid = 1;
	input.targetDefense = 10;
	input.targetDefenseBody = 0;
	input.targetDefenseItem = 0;
	input.targetLowSP = true;
	input.hitRoll = kRoll05;
	input.damageRoll = kRoll05;
	input.criticalRoll = kRoll0;
	input.crushingRoll = kRoll1;

	CombatResult result = Combat::ResolveCombat(input, constants);

	CHECK_EQ(result.IsHit(), true);
	CHECK_EQ(result.IsCritical(), true);
	CHECK_GT(result.damageResult.damage, 0u);
}

MODERN_TEST(Combat_ HighDefenseMinimumDamage)
{
	CombatConstants constants;
	CombatInput input = MakeBasicInput();
	input.attackerHit = 99;
	input.targetAvoid = 1;
	input.attackerPhysicalDamage = { 10, 20 };
	input.targetDefense = 999;
	input.targetDefenseBody = 0;
	input.targetDefenseItem = 0;
	input.hitRoll = kRoll05;
	input.damageRoll = kRoll05;
	input.criticalRoll = kRoll1;
	input.crushingRoll = kRoll1;

	CombatResult result = Combat::ResolveCombat(input, constants);

	CHECK_EQ(result.IsHit(), true);
	CHECK_GE(result.damageResult.damage, 1u);
	CHECK_LE(result.damageResult.damage, 5u);
}

// ── Determinism ───────────────────────────────────────────────────────

MODERN_TEST(Combat_ DeterministicSameInputs)
{
	CombatConstants constants;
	CombatInput input = MakeBasicInput();

	CombatResult r1 = Combat::ResolveCombat(input, constants);
	CombatResult r2 = Combat::ResolveCombat(input, constants);

	CHECK_EQ(r1.hitResult.hit, r2.hitResult.hit);
	CHECK_EQ(r1.damageResult.damage, r2.damageResult.damage);
	CHECK_EQ(r1.damageResult.critical, r2.damageResult.critical);
	CHECK_EQ(r1.damageResult.crushing, r2.damageResult.crushing);
}

// ── Combat Result ─────────────────────────────────────────────────────

MODERN_TEST(Combat_ CombatResultFlags)
{
	CombatConstants constants;
	CombatInput input = MakeBasicInput();
	input.attackerHit = 99;
	input.targetAvoid = 1;
	input.targetDefense = 10;
	input.targetDefenseBody = 0;
	input.targetDefenseItem = 0;
	input.attackerCrushingBonus = 150;
	input.hitRoll = kRoll05;
	input.damageRoll = kRoll05;
	input.criticalRoll = kRoll0;
	input.crushingRoll = kRoll0;

	CombatResult result = Combat::ResolveCombat(input, constants);

	CHECK_EQ(result.IsHit(), true);
	CHECK_EQ(result.IsCritical(), true);
	CHECK_EQ(result.IsCrushing(), true);
	CHECK_NE(result.damageFlag & DAMAGE_TYPE_CRITICAL, 0u);
	CHECK_NE(result.damageFlag & DAMAGE_TYPE_CRUSHING_BLOW, 0u);
}

MODERN_TEST(Combat_ CombatResultMissFlags)
{
	CombatConstants constants;
	CombatInput input = MakeBasicInput();
	input.attackerHit = 5;
	input.targetAvoid = 50;
	input.hitRoll = kRoll1;

	CombatResult result = Combat::ResolveCombat(input, constants);

	CHECK_EQ(result.IsHit(), false);
	CHECK_EQ(result.IsMiss(), true);
	CHECK_EQ(result.IsCritical(), false);
	CHECK_EQ(result.IsCrushing(), false);
	CHECK_EQ(result.damageFlag, 0u);
}

MODERN_TEST(Combat_ CombatResultTargetHP)
{
	CombatConstants constants;
	CombatInput input = MakeBasicInput();
	input.attackerHit = 99;
	input.targetAvoid = 1;
	input.targetDefense = 10;
	input.targetDefenseBody = 0;
	input.targetDefenseItem = 0;
	input.targetCurrentHP = 50;
	input.hitRoll = kRoll05;
	input.damageRoll = kRoll05;
	input.criticalRoll = kRoll1;
	input.crushingRoll = kRoll1;

	CombatResult result = Combat::ResolveCombat(input, constants);

	CHECK_EQ(result.IsHit(), true);
	CHECK_EQ(result.targetHPBefore, 50u);
	CHECK_LT(result.targetHPAfter, 50u);
	CHECK_GE(result.targetHPAfter, 0u);
}

MODERN_TEST(Combat_ CombatResultOverkill)
{
	CombatConstants constants;
	CombatInput input = MakeBasicInput();
	input.attackerHit = 99;
	input.targetAvoid = 1;
	input.targetDefense = 0;
	input.targetDefenseBody = 0;
	input.targetDefenseItem = 0;
	input.targetCurrentHP = 5;
	input.hitRoll = kRoll05;
	input.damageRoll = kRoll05;
	input.criticalRoll = kRoll1;
	input.crushingRoll = kRoll1;

	CombatResult result = Combat::ResolveCombat(input, constants);

	CHECK_EQ(result.IsHit(), true);
	CHECK_EQ(result.targetHPBefore, 5u);
	CHECK_EQ(result.targetHPAfter, 0u);
}

// ── Damage Reduction ──────────────────────────────────────────────────

MODERN_TEST(Combat_ DamageReductionApplied)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageReduce = 0.5f;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_LT(result.damage, 20u);
}

// ── Level Difference ──────────────────────────────────────────────────

MODERN_TEST(Combat_ LevelDifferenceBonus)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.level = 10;
	input.attackerLevel = 1;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_GT(result.damage, 15u);
}

// ── Constants ─────────────────────────────────────────────────────────

MODERN_TEST(Combat_ CombatConstantsSourceVerified)
{
	CombatConstants constants;

	CHECK_EQ(constants.lowSPHitDrop, 0.25f);
	CHECK_EQ(constants.lowSPDamage, 0.50f);
	CHECK_EQ(constants.damageGradeK, 10.0f);
	CHECK_EQ(constants.damageDecayRate, 40000.0f);
	CHECK_EQ(constants.resistPhysicG, 0.5f);
	CHECK_EQ(constants.criticalDamage, 120u);
	CHECK_EQ(constants.criticalMax, 40u);
	CHECK_EQ(constants.crushingBlowDamage, 150u);
	CHECK_EQ(constants.crushingBlowMax, 20u);
	CHECK_EQ(constants.crushingBlowRange, 10.0f);
	CHECK_EQ(constants.lowSeedDamage, 0.05f);
	CHECK_EQ(constants.maxHitRate, 99u);
	CHECK_EQ(constants.minHitRate, 20u);
	CHECK_EQ(constants.basicHitRate, 100u);
}

// ── VERTICAL-007: Equipment combat integration tests ──────────────────

namespace
{
	Modern::ItemDefinition MakeCombatItem(const std::string& name,
	                                      float criticalRate,
	                                      float crushingBlow,
	                                      float damageReduce,
	                                      float damageReflection,
	                                      float damageReflectionRate,
	                                      int32_t defense)
	{
		Modern::ItemDefinition def;
		def.id = Modern::ItemId(1u);
		def.name = name;
		def.kind = Modern::ItemKind::Armor;
		def.stats.criticalRate = criticalRate;
		def.stats.crushingBlow = crushingBlow;
		def.stats.damageReduce = damageReduce;
		def.stats.damageReflection = damageReflection;
		def.stats.damageReflectionRate = damageReflectionRate;
		def.stats.defense = defense;
		return def;
	}
}

MODERN_TEST(CombatEquip_NoCombatBonus)
{
	Modern::InMemoryItemDefinitions provider;
	Modern::ItemDefinition def = MakeCombatItem("PlainArmor", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
	provider.Add(def);

	Modern::EquipmentState equipment;
	Modern::ItemInstance instance;
	instance.definition = def.id;
	equipment.Equip(Modern::EquipmentSlot::Upper, instance);

	auto result = Modern::ItemContributionAggregator::Aggregate(equipment, provider);
	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.criticalRate, 0.0f);
	CHECK_EQ(result.GetValue().contribution.crushingBlow, 0.0f);
	CHECK_EQ(result.GetValue().contribution.damageReduce, 0.0f);
	CHECK_EQ(result.GetValue().contribution.damageReflection, 0.0f);
	CHECK_EQ(result.GetValue().contribution.damageReflectionRate, 0.0f);
}

MODERN_TEST(CombatEquip_CriticalRateItem)
{
	Modern::InMemoryItemDefinitions provider;
	Modern::ItemDefinition def = MakeCombatItem("CritArmor", 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
	provider.Add(def);

	Modern::EquipmentState equipment;
	Modern::ItemInstance instance;
	instance.definition = def.id;
	equipment.Equip(Modern::EquipmentSlot::Upper, instance);

	auto result = Modern::ItemContributionAggregator::Aggregate(equipment, provider);
	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.criticalRate, 0.05f);
}

MODERN_TEST(CombatEquip_MultipleCriticalItems)
{
	Modern::InMemoryItemDefinitions provider;
	Modern::ItemDefinition def1 = MakeCombatItem("CritArmor1", 0.03f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
	Modern::ItemDefinition def2 = MakeCombatItem("CritArmor2", 0.02f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
	provider.Add(def1);
	provider.Add(def2);

	Modern::EquipmentState equipment;
	Modern::ItemInstance inst1; inst1.definition = def1.id;
	Modern::ItemInstance inst2; inst2.definition = def2.id;
	equipment.Equip(Modern::EquipmentSlot::Upper, inst1);
	equipment.Equip(Modern::EquipmentSlot::Headgear, inst2);

	auto result = Modern::ItemContributionAggregator::Aggregate(equipment, provider);
	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.criticalRate, 0.05f);
}

MODERN_TEST(CombatEquip_CrushingBlowItem)
{
	Modern::InMemoryItemDefinitions provider;
	Modern::ItemDefinition def = MakeCombatItem("CrushArmor", 0.0f, 0.03f, 0.0f, 0.0f, 0.0f, 0);
	provider.Add(def);

	Modern::EquipmentState equipment;
	Modern::ItemInstance instance;
	instance.definition = def.id;
	equipment.Equip(Modern::EquipmentSlot::Upper, instance);

	auto result = Modern::ItemContributionAggregator::Aggregate(equipment, provider);
	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.crushingBlow, 0.03f);
}

MODERN_TEST(CombatEquip_DamageReduceItem)
{
	Modern::InMemoryItemDefinitions provider;
	Modern::ItemDefinition def = MakeCombatItem("ReduceArmor", 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0);
	provider.Add(def);

	Modern::EquipmentState equipment;
	Modern::ItemInstance instance;
	instance.definition = def.id;
	equipment.Equip(Modern::EquipmentSlot::Upper, instance);

	auto result = Modern::ItemContributionAggregator::Aggregate(equipment, provider);
	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.damageReduce, 0.1f);
}

MODERN_TEST(CombatEquip_DamageReflectionItem)
{
	Modern::InMemoryItemDefinitions provider;
	Modern::ItemDefinition def = MakeCombatItem("ReflectArmor", 0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0);
	provider.Add(def);

	Modern::EquipmentState equipment;
	Modern::ItemInstance instance;
	instance.definition = def.id;
	equipment.Equip(Modern::EquipmentSlot::Upper, instance);

	auto result = Modern::ItemContributionAggregator::Aggregate(equipment, provider);
	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.damageReflection, 0.2f);
	CHECK_EQ(result.GetValue().contribution.damageReflectionRate, 0.5f);
}

MODERN_TEST(CombatEquip_ItemDefense)
{
	Modern::InMemoryItemDefinitions provider;
	Modern::ItemDefinition def = MakeCombatItem("DefArmor", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 50);
	provider.Add(def);

	Modern::EquipmentState equipment;
	Modern::ItemInstance instance;
	instance.definition = def.id;
	equipment.Equip(Modern::EquipmentSlot::Upper, instance);

	auto result = Modern::ItemContributionAggregator::Aggregate(equipment, provider);
	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.defense, 50);
}

MODERN_TEST(CombatEquip_UnequipRemovesBonus)
{
	Modern::InMemoryItemDefinitions provider;
	Modern::ItemDefinition def = MakeCombatItem("CritArmor", 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
	provider.Add(def);

	Modern::EquipmentState equipment;
	Modern::ItemInstance instance;
	instance.definition = def.id;
	equipment.Equip(Modern::EquipmentSlot::Upper, instance);

	auto result1 = Modern::ItemContributionAggregator::Aggregate(equipment, provider);
	CHECK(result1.IsOk());
	CHECK_EQ(result1.GetValue().contribution.criticalRate, 0.05f);

	equipment.Unequip(Modern::EquipmentSlot::Upper);

	auto result2 = Modern::ItemContributionAggregator::Aggregate(equipment, provider);
	CHECK(result2.IsOk());
	CHECK_EQ(result2.GetValue().contribution.criticalRate, 0.0f);
}

MODERN_TEST(CombatEquip_ReplaceChangesBonus)
{
	Modern::InMemoryItemDefinitions provider;
	Modern::ItemDefinition def1 = MakeCombatItem("CritArmor1", 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
	Modern::ItemDefinition def2 = MakeCombatItem("CritArmor2", 0.10f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
	provider.Add(def1);
	provider.Add(def2);

	Modern::EquipmentState equipment;
	Modern::ItemInstance inst1; inst1.definition = def1.id;
	equipment.Equip(Modern::EquipmentSlot::Upper, inst1);

	auto result1 = Modern::ItemContributionAggregator::Aggregate(equipment, provider);
	CHECK(result1.IsOk());
	CHECK_EQ(result1.GetValue().contribution.criticalRate, 0.05f);

	Modern::ItemInstance inst2; inst2.definition = def2.id;
	equipment.Equip(Modern::EquipmentSlot::Upper, inst2);

	auto result2 = Modern::ItemContributionAggregator::Aggregate(equipment, provider);
	CHECK(result2.IsOk());
	CHECK_EQ(result2.GetValue().contribution.criticalRate, 0.10f);
}

MODERN_TEST(CombatEquip_EmptyEquipment)
{
	Modern::InMemoryItemDefinitions provider;
	Modern::EquipmentState equipment;

	auto result = Modern::ItemContributionAggregator::Aggregate(equipment, provider);
	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.criticalRate, 0.0f);
	CHECK_EQ(result.GetValue().contribution.crushingBlow, 0.0f);
	CHECK_EQ(result.GetValue().contribution.damageReduce, 0.0f);
	CHECK_EQ(result.GetValue().contribution.damageReflection, 0.0f);
	CHECK_EQ(result.GetValue().contribution.damageReflectionRate, 0.0f);
}

MODERN_TEST(CombatEquip_MissingDefinition)
{
	Modern::InMemoryItemDefinitions provider;
	Modern::EquipmentState equipment;
	Modern::ItemInstance instance;
	instance.definition = Modern::ItemId(999u);
	equipment.Equip(Modern::EquipmentSlot::Upper, instance);

	auto result = Modern::ItemContributionAggregator::Aggregate(equipment, provider);
	CHECK(!result.IsOk());
}

MODERN_TEST(CombatEquip_NonFiniteValues)
{
	Modern::InMemoryItemDefinitions provider;
	Modern::ItemDefinition def;
	def.id = Modern::ItemId(1u);
	def.name = "BadArmor";
	def.kind = Modern::ItemKind::Armor;
	def.stats.criticalRate = std::numeric_limits<float>::quiet_NaN();
	provider.Add(def);

	Modern::EquipmentState equipment;
	Modern::ItemInstance instance;
	instance.definition = def.id;
	equipment.Equip(Modern::EquipmentSlot::Upper, instance);

	auto result = Modern::ItemContributionAggregator::Aggregate(equipment, provider);
	CHECK(!result.IsOk());
}

// ── VERTICAL-008: Reflection tests ────────────────────────────────────

MODERN_TEST(CombatReflection_Disabled)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageReflection = 0.0f;
	input.damageReflectionRate = 0.0f;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;
	input.reflectionRoll = 0.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.reflectionTriggered, false);
	CHECK_EQ(result.reflectionDamage, 0u);
}

MODERN_TEST(CombatReflection_RateZero)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageReflection = 0.5f;
	input.damageReflectionRate = 0.0f;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;
	input.reflectionRoll = 0.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.reflectionTriggered, false);
	CHECK_EQ(result.reflectionDamage, 0u);
}

MODERN_TEST(CombatReflection_ThresholdBelowRoll)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageReflection = 0.5f;
	input.damageReflectionRate = 0.3f;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;
	input.reflectionRoll = 0.2f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.reflectionTriggered, true);
	CHECK_GT(result.reflectionDamage, 0u);
}

MODERN_TEST(CombatReflection_ThresholdAboveRoll)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageReflection = 0.5f;
	input.damageReflectionRate = 0.3f;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;
	input.reflectionRoll = 0.4f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.reflectionTriggered, false);
	CHECK_EQ(result.reflectionDamage, 0u);
}

MODERN_TEST(CombatReflection_ExactBoundary)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageReflection = 0.5f;
	input.damageReflectionRate = 0.3f;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;
	input.reflectionRoll = 0.3f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.reflectionTriggered, false);
}

MODERN_TEST(CombatReflection_AmountZero)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageReflection = 0.0f;
	input.damageReflectionRate = 0.5f;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;
	input.reflectionRoll = 0.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.reflectionTriggered, true);
	CHECK_EQ(result.reflectionDamage, 0u);
}

MODERN_TEST(CombatReflection_LevelScaling)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageReflection = 0.5f;
	input.damageReflectionRate = 0.5f;
	input.targetLevel = 100;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;
	input.reflectionRoll = 0.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.reflectionTriggered, true);
	CHECK_GT(result.reflectionDamage, 0u);
}

MODERN_TEST(CombatReflection_CriticalPlusReflection)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageReflection = 0.5f;
	input.damageReflectionRate = 0.5f;
	input.damageRoll = 0.5f;
	input.criticalRoll = 0.0f;
	input.crushingRoll = 1.0f;
	input.reflectionRoll = 0.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.critical, true);
	CHECK_EQ(result.reflectionTriggered, true);
	CHECK_GT(result.reflectionDamage, 0u);
}

MODERN_TEST(CombatReflection_CrushingPlusReflection)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageReflection = 0.5f;
	input.damageReflectionRate = 0.5f;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 0.0f;
	input.attackerCrushingBonus = 150;
	input.reflectionRoll = 0.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.crushing, true);
	CHECK_EQ(result.reflectionTriggered, true);
	CHECK_GT(result.reflectionDamage, 0u);
}

MODERN_TEST(CombatReflection_DamageReducePlusReflection)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.damageReflection = 0.5f;
	input.damageReflectionRate = 0.5f;
	input.damageReduce = 0.3f;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;
	input.reflectionRoll = 0.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.reflectionTriggered, true);
	CHECK_GT(result.reflectionDamage, 0u);
	CHECK_LT(result.damage, 20u);
}

MODERN_TEST(CombatReflection_MinimumDamagePlusReflection)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.physicalDamage = { 1, 1 };
	input.defense = 999;
	input.damageReflection = 0.5f;
	input.damageReflectionRate = 0.5f;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;
	input.reflectionRoll = 0.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_GE(result.damage, 1u);
	CHECK_EQ(result.reflectionTriggered, true);
	CHECK_EQ(result.reflectionDamage, 0u);
}

MODERN_TEST(CombatReflection_NoRecursion)
{
	CombatConstants constants;
	CombatInput input = MakeBasicInput();
	input.attackerHit = 99;
	input.targetAvoid = 1;
	input.targetDefense = 10;
	input.targetDefenseBody = 0;
	input.targetDefenseItem = 0;
	input.targetDamageReflection = 0.5f;
	input.targetDamageReflectionRate = 0.5f;
	input.hitRoll = kRoll05;
	input.damageRoll = kRoll05;
	input.criticalRoll = kRoll1;
	input.crushingRoll = kRoll1;
	input.reflectionRoll = kRoll0;

	CombatResult result = Combat::ResolveCombat(input, constants);

	CHECK_EQ(result.IsHit(), true);
	CHECK_EQ(result.IsReflection(), true);
	CHECK_GT(result.damageResult.reflectionDamage, 0u);
	CHECK_EQ(result.attackerHPAfter, result.attackerHPBefore - result.damageResult.reflectionDamage);
}

MODERN_TEST(CombatReflection_AttackerHPTracked)
{
	CombatConstants constants;
	CombatInput input = MakeBasicInput();
	input.attackerHit = 99;
	input.targetAvoid = 1;
	input.targetDefense = 10;
	input.targetDefenseBody = 0;
	input.targetDefenseItem = 0;
	input.targetDamageReflection = 0.5f;
	input.targetDamageReflectionRate = 0.5f;
	input.attackerCurrentHP = 100;
	input.targetCurrentHP = 100;
	input.hitRoll = kRoll05;
	input.damageRoll = kRoll05;
	input.criticalRoll = kRoll1;
	input.crushingRoll = kRoll1;
	input.reflectionRoll = kRoll0;

	CombatResult result = Combat::ResolveCombat(input, constants);

	CHECK_EQ(result.IsHit(), true);
	CHECK_EQ(result.IsReflection(), true);
	CHECK_EQ(result.attackerHPBefore, 100u);
	CHECK_LT(result.attackerHPAfter, 100u);
}

MODERN_TEST(CombatReflection_MissNoReflection)
{
	CombatConstants constants;
	CombatInput input = MakeBasicInput();
	input.attackerHit = 5;
	input.targetAvoid = 50;
	input.targetDamageReflection = 0.5f;
	input.targetDamageReflectionRate = 0.5f;
	input.hitRoll = kRoll1;
	input.damageRoll = kRoll05;
	input.criticalRoll = kRoll1;
	input.crushingRoll = kRoll1;
	input.reflectionRoll = kRoll0;

	CombatResult result = Combat::ResolveCombat(input, constants);

	CHECK_EQ(result.IsHit(), false);
	CHECK_EQ(result.IsReflection(), false);
	CHECK_EQ(result.damageResult.reflectionDamage, 0u);
}
