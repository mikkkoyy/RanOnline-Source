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
		// VERTICAL-010: not low SP by default. attackerCurrentSP sits at the
		// default 0, so the required SP has to be 0 too for the strict `<` to
		// read as "enough". Cases that want low SP raise the requirement.
		input.attackerRequiredSP = 0;
		input.attackerCurrentSP = 0;

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

MODERN_TEST(Combat_MissWhenAvoidExceedsHit)
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

MODERN_TEST(Combat_HitWhenHitExceedsAvoid)
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

MODERN_TEST(Combat_HitRateClampedToMax)
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

MODERN_TEST(Combat_HitRateClampedToMin)
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

MODERN_TEST(Combat_LowSPReducesHitRate)
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

MODERN_TEST(Combat_DamageRangeMinRoll)
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

MODERN_TEST(Combat_DamageRangeMaxRoll)
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

MODERN_TEST(Combat_DamageRangeMidRoll)
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

MODERN_TEST(Combat_DefenseReducesDamage)
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

MODERN_TEST(Combat_StateDamageMultiplier)
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

MODERN_TEST(Combat_LowSPDamageModifier)
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

MODERN_TEST(Combat_MinimumDamageOneOnHit)
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

MODERN_TEST(Combat_MissProducesZeroDamage)
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

MODERN_TEST(Combat_CriticalHitOccurs)
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

MODERN_TEST(Combat_NonCriticalHit)
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

MODERN_TEST(Combat_CriticalBoundary)
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

MODERN_TEST(Combat_CrushingBlowOccurs)
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

MODERN_TEST(Combat_NonCrushingBlow)
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

MODERN_TEST(Combat_CrushingBlowBoundary)
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

MODERN_TEST(Combat_CriticalAndCrushing)
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

MODERN_TEST(Combat_CriticalWithDefense)
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

MODERN_TEST(Combat_LowSPWithCritical)
{
	CombatConstants constants;
	CombatInput input = MakeBasicInput();
	input.attackerHit = 99;
	input.targetAvoid = 1;
	input.targetDefense = 10;
	input.targetDefenseBody = 0;
	input.targetDefenseItem = 0;
	// VERTICAL-010: low SP is current < required, on the attacker's own pool.
	input.attackerRequiredSP = 31;
	input.attackerCurrentSP = 30;
	input.hitRoll = kRoll05;
	input.damageRoll = kRoll05;
	input.criticalRoll = kRoll0;
	input.crushingRoll = kRoll1;

	CombatResult result = Combat::ResolveCombat(input, constants);

	CHECK_EQ(result.IsHit(), true);
	CHECK_EQ(result.IsCritical(), true);
	CHECK_GT(result.damageResult.damage, 0u);
}

MODERN_TEST(Combat_HighDefenseMinimumDamage)
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

MODERN_TEST(Combat_DeterministicSameInputs)
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

MODERN_TEST(Combat_CombatResultFlags)
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

MODERN_TEST(Combat_CombatResultMissFlags)
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

MODERN_TEST(Combat_CombatResultTargetHP)
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

MODERN_TEST(Combat_CombatResultOverkill)
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

MODERN_TEST(Combat_DamageReductionApplied)
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

MODERN_TEST(Combat_LevelDifferenceBonus)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.targetLevel = 100;
	input.attackerLevel = 1;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_GT(result.damage, 15u);
}

// ── Constants ─────────────────────────────────────────────────────────

MODERN_TEST(Combat_CombatConstantsSourceVerified)
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
	Modern::ItemDefinition def1 = MakeCombatItem("CritArmor1", 0.25f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
	Modern::ItemDefinition def2 = MakeCombatItem("CritArmor2", 0.25f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
	def1.id = Modern::ItemId(1u);
	def2.id = Modern::ItemId(2u);
	provider.Add(def1);
	provider.Add(def2);

	Modern::EquipmentState equipment;
	Modern::ItemInstance inst1; inst1.definition = def1.id;
	Modern::ItemInstance inst2; inst2.definition = def2.id;
	equipment.Equip(Modern::EquipmentSlot::Upper, inst1);
	equipment.Equip(Modern::EquipmentSlot::Headgear, inst2);

	auto result = Modern::ItemContributionAggregator::Aggregate(equipment, provider);
	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().contribution.criticalRate, 0.5f);
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
	def1.id = Modern::ItemId(1u);
	def2.id = Modern::ItemId(2u);
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
	CHECK(!result.GetValue().IsOk());
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
	CHECK(!result.GetValue().IsOk());
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
	input.targetLevel = 100;

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

// ── VERTICAL-010: required SP and the low-SP rule ──────────────────────
//
// The rule under test, from GLogixExPC.cpp:3492-3497:
//
//   WORD wDisSP = GLCONST_CHAR::wBASIC_DIS_SP;          // 1
//   if ( pRHAND )  wDisSP += pRHAND->sSuitOp.wReqSP;
//   if ( pLHAND )  wDisSP += pLHAND->sSuitOp.wReqSP;
//   if ( m_sSP.dwNow < (wDisSP*wStrikeNum) )  return EMBEGINA_SP;
//
// CombatCalculator resolves the comparison; these cases pin the arithmetic
// and the boundary. Low SP is a property of the attacker, so these vary
// `attackerCurrentSP` and leave the target's SP alone.

namespace
{
	// The required SP an action costs, given the hand-slot wReqSP sum.
	//
	// This mirrors what ServerCharacter::Attack computes from the aggregated
	// contribution; the aggregation itself is covered in EquipmentTests.cpp.
	uint16_t BasicRequiredSP(uint16_t handRequiredSP)
	{
		return static_cast<uint16_t>(handRequiredSP + CombatConstants().basicDisSP);
	}

	// A hit-for-certain melee input whose only variable is the SP state, so a
	// difference in outcome can only come from the low-SP rule.
	//
	// `targetDefense` is 9 on purpose. The rolled damage is 15, so this leaves
	// 6, and 6 halved is exactly 3. With the usual 10 the pre-halving value is
	// 5, `5 * 0.5f` truncates to 2, and every "is exactly half" assertion below
	// would fail on rounding rather than on the rule under test.
	CombatInput MakeSPInput(uint16_t handRequiredSP, uint32_t currentSP)
	{
		CombatInput input = MakeBasicInput();
		input.attackerHit = 99;
		input.targetAvoid = 1;
		input.targetDefense = 9;
		input.targetDefenseBody = 0;
		input.targetDefenseItem = 0;
		input.attackerRequiredSP = BasicRequiredSP(handRequiredSP);
		input.attackerCurrentSP = currentSP;
		input.hitRoll = kRoll05;
		input.damageRoll = kRoll05;
		input.criticalRoll = kRoll1;
		input.crushingRoll = kRoll1;
		return input;
	}
}

// Test 1: no hand requirements, so only the base cost remains.
MODERN_TEST(RequiredSPMatrix_NoHandsRequiredSPIsOne)
{
	CHECK_EQ(BasicRequiredSP(0), static_cast<uint16_t>(1));
}

// Test 2: right hand only.
MODERN_TEST(RequiredSPMatrix_RightHandOnly)
{
	CHECK_EQ(BasicRequiredSP(20), static_cast<uint16_t>(21));
}

// Test 3: left hand only.
MODERN_TEST(RequiredSPMatrix_LeftHandOnly)
{
	CHECK_EQ(BasicRequiredSP(10), static_cast<uint16_t>(11));
}

// Test 4: both hands, the worked example from the milestone brief.
MODERN_TEST(RequiredSPMatrix_BothHands)
{
	CHECK_EQ(BasicRequiredSP(20 + 10), static_cast<uint16_t>(31));
}

// Test 5: exactly the required amount is enough. The comparison is strict.
MODERN_TEST(RequiredSPMatrix_ExactBoundaryIsNotLowSP)
{
	CombatInput input = MakeSPInput(30, 31);

	CHECK_EQ(input.attackerRequiredSP, static_cast<uint16_t>(31));

	// Observable consequence: low SP multiplies damage by 0.5, so a
	// non-low-SP swing must not be halved.
	CombatResult result = Combat::ResolveCombat(input);

	CHECK_EQ(result.IsHit(), true);
	CombatInput notLow = input;
	notLow.attackerCurrentSP = 100;
	CHECK_EQ(Combat::ResolveCombat(notLow).damageResult.damage, result.damageResult.damage);
}

// Test 6: one below the requirement is low SP, and halves the damage.
MODERN_TEST(RequiredSPMatrix_OneBelowBoundaryIsLowSP)
{
	CombatInput low  = MakeSPInput(30, 30);
	CombatInput full = MakeSPInput(30, 31);

	CHECK_EQ(low.attackerRequiredSP, static_cast<uint16_t>(31));

	const CombatResult lowResult  = Combat::ResolveCombat(low);
	const CombatResult fullResult = Combat::ResolveCombat(full);

	CHECK_EQ(lowResult.IsHit(), true);
	CHECK_EQ(fullResult.IsHit(), true);
	// fLOWSP_DAMAGE = 0.50
	CHECK_EQ(lowResult.damageResult.damage, static_cast<uint32_t>(fullResult.damageResult.damage / 2));
}

// Test 7: comfortably above the requirement is not low SP.
MODERN_TEST(RequiredSPMatrix_HighSPIsNotLowSP)
{
	CombatInput low  = MakeSPInput(30, 30);
	CombatInput high = MakeSPInput(30, 500);

	const CombatResult lowResult  = Combat::ResolveCombat(low);
	const CombatResult highResult = Combat::ResolveCombat(high);

	CHECK_EQ(highResult.damageResult.damage, lowResult.damageResult.damage * 2);
}

// Test 8: no SP at all is low SP whenever anything is required.
MODERN_TEST(RequiredSPMatrix_ZeroSPIsLowSP)
{
	CombatInput empty = MakeSPInput(30, 0);
	CombatInput full  = MakeSPInput(30, 31);

	const CombatResult emptyResult = Combat::ResolveCombat(empty);
	const CombatResult fullResult  = Combat::ResolveCombat(full);

	CHECK_EQ(empty.attackerRequiredSP, static_cast<uint16_t>(31));
	CHECK_EQ(emptyResult.damageResult.damage, static_cast<uint32_t>(fullResult.damageResult.damage / 2));
}

// A requirement of zero can never be exceeded, so an empty attacker is not
// low SP. This is why the default `attackerRequiredSP` is 0 rather than
// `basicDisSP`: it keeps a default-constructed input meaning "no constraint".
MODERN_TEST(RequiredSPMatrix_ZeroRequirementIsNeverLowSP)
{
	CombatInput input = MakeSPInput(0, 0);
	input.attackerRequiredSP = 0;
	input.attackerCurrentSP = 0;

	CombatInput reference = MakeSPInput(0, 100);
	reference.attackerRequiredSP = 0;

	CHECK_EQ(Combat::ResolveCombat(input).damageResult.damage,
	         Combat::ResolveCombat(reference).damageResult.damage);
}

// The pool belongs to the attacker. A target with no SP must not make the
// attacker's swing low-SP, and an attacker with no SP must regardless of the
// target's pool.
// The pool belongs to the attacker.
//
// Legacy decides this before the swing, on the character doing the attacking:
// GLCharMsg.cpp:604-612 calls BEGIN_ATTACK (GLogixExPC.cpp:3492-3497), which
// reads that character's own m_sSP.dwNow, and passes the resulting bLowSP to
// that character's PreStrikeProc. The victim is never consulted.
//
// `CombatInput` has no target SP field at all, which is the structural form of
// that rule: there is nothing on the input for a victim's SP to be read from.
MODERN_TEST(RequiredSPMatrix_LowSPFollowsTheAttackerNotTheTarget)
{
	CombatInput attackerEmpty = MakeSPInput(30, 0);
	CombatInput attackerFull  = MakeSPInput(30, 31);

	CHECK_EQ(Combat::ResolveCombat(attackerEmpty).damageResult.damage,
	         static_cast<uint32_t>(Combat::ResolveCombat(attackerFull).damageResult.damage / 2));

	// Holding the attacker exactly at the requirement is not low SP, whatever
	// the target's HP pool happens to be.
	CombatInput atRequirement = MakeSPInput(30, 31);
	atRequirement.targetCurrentHP = 0;
	CHECK_EQ(Combat::ResolveCombat(atRequirement).damageResult.damage,
	         Combat::ResolveCombat(attackerFull).damageResult.damage);
}

// Test 9 / VERTICAL-009 regression: the low-SP damage multiplier is still
// exactly fLOWSP_DAMAGE = 0.50, and the hit-rate drop is still
// fLOWSP_HIT_DROP = 0.25. VERTICAL-010 changed the *input*, not these.
MODERN_TEST(RequiredSPMatrix_LowSPFormulasUnchanged)
{
	CHECK_EQ(CombatConstants().lowSPDamage, 0.50f);
	CHECK_EQ(CombatConstants().lowSPHitDrop, 0.25f);

	CombatInput low  = MakeSPInput(0, 0);
	CombatInput full = MakeSPInput(0, 1);
	low.attackerRequiredSP = 1;
	full.attackerRequiredSP = 1;

	CHECK_EQ(Combat::ResolveCombat(low).damageResult.damage,
	         static_cast<uint32_t>(Combat::ResolveCombat(full).damageResult.damage / 2));

	// Hit rate, via the hit path directly.
	HitInput normal;
	normal.attackerHit = 50;
	normal.targetAvoid = 10;
	normal.brightnessFB = GameBrightFB::Aver;
	normal.lowSP = false;
	normal.hitRoll = 0.5f;

	HitInput depleted = normal;
	depleted.lowSP = true;

	CHECK_EQ(CalculateHit(depleted).hitRate,
	         static_cast<uint32_t>(CalculateHit(normal).hitRate * 0.75f));
}

// The equipment contribution has to actually change the outcome, otherwise a
// regression that dropped the hand term entirely would still pass the
// boundary cases above.
MODERN_TEST(RequiredSPMatrix_EquipmentTermChangesTheOutcome)
{
	// Unarmed: required 1, so 1 SP is enough.
	CombatInput unarmed = MakeSPInput(0, 1);
	unarmed.attackerRequiredSP = BasicRequiredSP(0);

	// Armed with a heavy weapon: required 31, so the same 1 SP is not enough.
	CombatInput armed = MakeSPInput(30, 1);
	armed.attackerRequiredSP = BasicRequiredSP(30);

	CombatInput armedFunded = MakeSPInput(30, 31);
	armedFunded.attackerRequiredSP = BasicRequiredSP(30);

	CHECK_EQ(unarmed.attackerRequiredSP, static_cast<uint16_t>(1));
	CHECK_EQ(armed.attackerRequiredSP, static_cast<uint16_t>(31));

	CHECK_EQ(Combat::ResolveCombat(unarmed).damageResult.damage,
	         Combat::ResolveCombat(armedFunded).damageResult.damage);
	CHECK_EQ(Combat::ResolveCombat(armed).damageResult.damage,
	         static_cast<uint32_t>(Combat::ResolveCombat(armedFunded).damageResult.damage / 2));
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
	input.targetLevel = 100;

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
	input.targetLevel = 100;

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
	input.targetLevel = 100;

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
	input.targetLevel = 100;

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
	input.targetLevel = 100;

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

// ── VERTICAL-009: Low SP ───────────────────────────────────────────────

MODERN_TEST(CombatLowSP_AboveRequired)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.lowSP = false;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.lowSP, false);
	CHECK_GT(result.damage, 0u);
}

MODERN_TEST(CombatLowSP_BelowRequired)
{
	CombatConstants constants;
	PhysicalDamageInput normal = MakeBasicDamageInput();
	normal.defense = 0;
	normal.lowSP = false;
	normal.damageRoll = 0.5f;
	normal.criticalRoll = 1.0f;
	normal.crushingRoll = 1.0f;

	PhysicalDamageInput lowSP = normal;
	lowSP.lowSP = true;

	DamageResult rNormal = CalculatePhysicalDamage(normal, constants);
	DamageResult rLow = CalculatePhysicalDamage(lowSP, constants);

	CHECK_EQ(rLow.lowSP, true);
	CHECK_LT(rLow.damage, rNormal.damage);
}

MODERN_TEST(CombatLowSP_ZeroSP)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.lowSP = true;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.lowSP, true);
	CHECK_GT(result.damage, 0u);
}

// ── VERTICAL-009: Physical Resistance ─────────────────────────────────

MODERN_TEST(CombatResist_Zero)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.resistElement = 0;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_GT(result.damage, 0u);
	CHECK_EQ(result.preDefenseDamage, result.rawDamage);
}

MODERN_TEST(CombatResist_Positive)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.resistElement = 50;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_LT(result.preDefenseDamage, result.rawDamage);
	CHECK_GT(result.damage, 0u);
}

MODERN_TEST(CombatResist_High)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.resistElement = 200;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_GT(result.damage, 0u);
}

MODERN_TEST(CombatResist_RawValueClampedToMax)
{
	// Legacy: GLogixExPC.cpp:1516 clamps nRESIST to fMAX_RESIST (99) before the
	// reduction is computed, and :1559 caps fResistTotal at 0.8.
	//
	// Because fMAX_RESIST is 99 and fRESIST_PHYSIC_G is 0.5, the largest
	// reachable fResistTotal is 99*0.01*0.5 = 0.495. The 0.8 cap is therefore
	// unreachable in the physical path, so the raw-value clamp is the only cap
	// that can actually bite. This test pins the clamp, not the unreachable cap.
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.resistElement = 1000;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	const int32_t clampedResist = 99;
	float fResistTotal = static_cast<float>(clampedResist) * 0.01f * constants.resistPhysicG;
	uint32_t expectedDamage = static_cast<uint32_t>(
		static_cast<float>(result.rawDamage) * (1.0f - fResistTotal));
	if (expectedDamage == 0)
		expectedDamage = 1;

	CHECK_EQ(result.damage, expectedDamage);
}

// ── VERTICAL-009: PK Damage Modifier ───────────────────────────────────

MODERN_TEST(CombatPK_NonPK)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.isPK = false;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_GT(result.damage, 0u);
}

MODERN_TEST(CombatPK_PKCombat)
{
	CombatConstants constants;
	PhysicalDamageInput normal = MakeBasicDamageInput();
	normal.defense = 0;
	normal.isPK = false;
	normal.damageRoll = 0.5f;
	normal.criticalRoll = 1.0f;
	normal.crushingRoll = 1.0f;

	PhysicalDamageInput pk = normal;
	pk.isPK = true;

	DamageResult rNormal = CalculatePhysicalDamage(normal, constants);
	DamageResult rPK = CalculatePhysicalDamage(pk, constants);

	CHECK_LT(rPK.damage, rNormal.damage);
}

MODERN_TEST(CombatPK_PKModifierValue)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.isPK = true;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	uint32_t expectedDamage = static_cast<uint32_t>(
		static_cast<float>(result.rawDamage) * constants.pkPointDecPhy);
	if (expectedDamage == 0)
		expectedDamage = 1;

	CHECK_EQ(result.damage, expectedDamage);
}

// ── VERTICAL-009: Ranged Reflection Disable ────────────────────────────

MODERN_TEST(CombatRanged_NoReflection)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.attackType = AttackType::Ranged;
	input.damageReflection = 0.5f;
	input.damageReflectionRate = 0.5f;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;
	input.reflectionRoll = 0.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.reflectionTriggered, false);
	CHECK_EQ(result.reflectionDamage, 0u);
}

MODERN_TEST(CombatRanged_MeleeStillReflects)
{
	CombatConstants constants;
	PhysicalDamageInput input = MakeBasicDamageInput();
	input.defense = 0;
	input.attackType = AttackType::Melee;
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
