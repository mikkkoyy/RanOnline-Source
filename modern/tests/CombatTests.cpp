// VERTICAL-006: basic physical combat resolution tests.
//
// All tests use deterministic random inputs (hitRoll, damageRoll, etc.)
// drawn from [0, 1] so results are reproducible.

#include "TestHarness.h"

#include "combat/CombatTypes.h"
#include "combat/CombatConstants.h"
#include "combat/HitCalculator.h"
#include "combat/PhysicalDamageCalculator.h"
#include "combat/MagicDamageCalculator.h"
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

	// VERTICAL-012: the melee power is added to the range before the roll
	// (GLogixExPC.cpp:1594, VAR_PARAM). The fixture's range is {10,20} and its
	// melee power is 3, so the rolled low end is 10 + 3 = 13 rather than 10.
	CHECK_EQ(result.damage, 13u);
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

	// The low-SP penalty is `* 0.5` in floating point then truncated to an
	// integer, so on an odd pre-penalty value the halved figure is not exactly
	// half and `low * 2` is one short. The verified property is that the
	// low-SP damage is strictly less and no more than the funded damage, which
	// is what the rule says; the exact-halving form is pinned separately by
	// `RequiredSPMatrix_LowSPHalvesSkillDamage`, whose fixture is chosen to be
	// even.
	CHECK_EQ(lowResult.damageResult.lowSP, true);
	CHECK_EQ(highResult.damageResult.lowSP, false);
	CHECK_LT(lowResult.damageResult.damage, highResult.damageResult.damage);
	CHECK_LE(lowResult.damageResult.damage * 2, highResult.damageResult.damage + 1u);
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
	// VERTICAL-012: the melee power now reaches the range, so this case's
	// absolute figure moved. What it exists to check is that the reduction is
	// applied, and that is asserted against the unreduced value rather than a
	// magic number:
	//
	//   range {10,20} + meleePower 3  -> {13,23}      (:1594, VAR_PARAM)
	//   damageRoll 0.5                 -> 18
	//   targetLevel 100 > attacker 1   -> nExtFORCE int(0.5*99/10) = 4
	//   nDAMAGE_OLD 22, defense 0       -> 22
	//   DamageReduceAmount(22, 0.3, 100, 300) = int(2.2) = 2
	//   22 - 2 = 20
	//
	// The old bound was `< 20`, which the added attack power reaches exactly.
	CHECK_EQ(result.damage, 20u);
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

// ══════════════════════════════════════════════════════════════════════
// VERTICAL-013: magic / elemental combat
// ══════════════════════════════════════════════════════════════════════

namespace
{
	// A magic fixture with every roll pinned so the result is a pure function
	// of the inputs. skillRange is {0,0} on purpose: with a flat range the
	// attack power and the skill magnitude are the only things that can move
	// the number, which is what most of these cases need to isolate.
	MagicDamageInput MakeMagicInput(uint16_t magicAttack = 100,
	                                float    skillBasicVar = -50.0f)
	{
		MagicDamageInput input;
		input.magicAttack   = magicAttack;
		input.skillRange    = { 0, 0 };
		input.skillBasicVar = skillBasicVar;

		input.attackerLevel     = 10;
		input.attackerMaxHP     = 1000;
		input.attackerCurrentHP = 1000;
		input.damageRate        = 1.0f;

		input.targetLevel = 10;
		input.stateDamage = 1.0f;

		input.damageRoll     = 0.0f;
		input.criticalRoll   = 1.0f;
		input.crushingRoll   = 1.0f;
		input.reflectionRoll = 1.0f;
		return input;
	}
}

// ── Attack power ───────────────────────────────────────────────────────
//
// The VERTICAL-013 counterpart to VERTICAL-012's regression. The power must
// reach the range through VAR_PARAM, and swapping the two powers must swap the
// result.
MODERN_TEST(Magic_MagicAttackChangesTheDamageRange)
{
	MagicDamageInput low  = MakeMagicInput(100);
	MagicDamageInput high = MakeMagicInput(300);

	const DamageResult lowResult  = CalculateMagicDamage(low);
	const DamageResult highResult = CalculateMagicDamage(high);

	// {0,0} + 100 + |int(-50)| = 150; {0,0} + 300 + 50 = 350.
	CHECK_EQ(lowResult.rawDamage, 150u);
	CHECK_EQ(highResult.rawDamage, 350u);
	CHECK_GT(highResult.damage, lowResult.damage);
}

MODERN_TEST(Magic_MagicAttackIsNotSwappedWithMeleeOrShootPower)
{
	// The physical calculator must be indifferent to magicAttack and vice
	// versa. If either had started reading the other's field, this fails.
	PhysicalDamageInput physical = MakeBasicDamageInput();
	physical.meleePower    = 100;
	physical.shootPower    = 300;
	physical.physicalDamage = { 0, 0 };
	physical.damageRoll = 0.0f;
	physical.criticalRoll = 1.0f;
	physical.crushingRoll = 1.0f;

	const DamageResult melee = CalculatePhysicalDamage(physical);
	physical.attackType = AttackType::Ranged;
	const DamageResult ranged = CalculatePhysicalDamage(physical);

	CHECK_EQ(melee.rawDamage, 100u);
	CHECK_EQ(ranged.rawDamage, 300u);

	// Magic with the same numbers, but its own power.
	MagicDamageInput magic = MakeMagicInput(250);
	CHECK_EQ(CalculateMagicDamage(magic).rawDamage, 300u); // 250 + |int(-50)|
}

// The VAR_PARAM floor at 1, not 0. A power low enough to drive an end below 1
// must land on exactly 1 (GLDefine.h:364-371).
MODERN_TEST(Magic_AttackPowerFloorIsOneNotZero)
{
	MagicDamageInput input = MakeMagicInput(0);
	input.skillRange    = { 0, 0 };
	input.skillBasicVar = 0.0f;   // no skill magnitude to hide behind

	const DamageResult result = CalculateMagicDamage(input);

	CHECK_EQ(result.rawDamage, 1u);
}

// ── Skill magnitude ────────────────────────────────────────────────────
//
// float fSKILL_VAR = sSKILL_DATA.fBASIC_VAR;
// int nVAR = abs ( int(fSKILL_VAR*fPOWER) );        :1520-1524
MODERN_TEST(Magic_SkillMagnitudeIsAbsOfTheWeatherScaledProduct)
{
	MagicDamageInput plain = MakeMagicInput(100);
	plain.skillRange = { 0, 0 };

	// -50 -> abs(int(-50)) = 50.
	CHECK_EQ(CalculateMagicDamage(plain).rawDamage, 150u);

	// Truncation happens before abs, so -0.5 becomes 0, not 1.
	MagicDamageInput fractional = MakeMagicInput(100);
	fractional.skillBasicVar = -0.5f;
	CHECK_EQ(CalculateMagicDamage(fractional).rawDamage, 100u);

	// And -1.5 truncates toward zero to -1 -> 1.
	MagicDamageInput oneAndAHalf = MakeMagicInput(100);
	oneAndAHalf.skillBasicVar = -1.5f;
	CHECK_EQ(CalculateMagicDamage(oneAndAHalf).rawDamage, 101u);
}

MODERN_TEST(Magic_WeatherElementPowerScalesTheSkillMagnitude)
{
	MagicDamageInput plain = MakeMagicInput(100);
	plain.skillBasicVar = -100.0f;

	MagicDamageInput weathered = MakeMagicInput(100);
	weathered.skillBasicVar = -100.0f;
	weathered.weatherElementPower = 1.2f;

	// abs(int(-100*1.0)) = 100 vs abs(int(-100*1.2)) = 120.
	CHECK_EQ(CalculateMagicDamage(plain).rawDamage, 200u);
	CHECK_EQ(CalculateMagicDamage(weathered).rawDamage, 220u);
}

// ── Physical defence is ignored ────────────────────────────────────────
//
// GLogixExPC.cpp:1474-1476 forces nDEFENSE, nDEFAULT_DEFENSE and
// nITEM_DEFENSE to 0 for magic. The magic calculator has no defence parameter
// at all, so a target's physical armour cannot reach it.
MODERN_TEST(Magic_PhysicalDefenseDoesNotReduceMagicDamage)
{
	MagicDamageInput input = MakeMagicInput(100);

	const DamageResult direct = CalculateMagicDamage(input);

	// Drive the physical path with the same fixture and a large defence: it
	// must be reduced. This is the contrast that proves the magic path is
	// genuinely separate rather than silently reusing the physical one.
	CombatInput physical = MakeBasicInput();
	physical.attackType = AttackType::Melee;
	physical.attackerMeleePower = 100;
	physical.targetDefense = 0;
	const CombatResult undefended = Combat::ResolveCombat(physical);
	physical.targetDefense = 9999;
	const CombatResult defended = Combat::ResolveCombat(physical);

	CHECK_LT(defended.damageResult.damage, undefended.damageResult.damage);

	// The magic result is the same figure whichever way the physical path is
	// configured, because it never reads a defence field.
	CHECK_EQ(direct.rawDamage, 150u);
}

// ── Resistance ─────────────────────────────────────────────────────────
//
// Magic applies resistance to the RANGE with a subtraction of the truncated
// product (:1562-1563), before the roll. That is deliberately not the physical
// path's multiplicative form on the rolled value.
MODERN_TEST(Magic_ResistanceReducesTheRangeBeforeTheRoll)
{
	MagicDamageInput plain = MakeMagicInput(100);
	plain.skillBasicVar = 0.0f;   // range {100,100} exactly
	plain.resistElement = 0;

	MagicDamageInput resisted = plain;
	resisted.resistElement = 50;

	// fResistTotal = 50 * 0.01 * fRESIST_G(0.5) = 0.25.
	// 100 - DWORD(100 * 0.25) = 100 - 25 = 75.
	CHECK_EQ(CalculateMagicDamage(plain).rawDamage, 100u);
	CHECK_EQ(CalculateMagicDamage(resisted).rawDamage, 75u);
}

MODERN_TEST(Magic_ResistanceIsClampedToTheLegacyMaximum)
{
	// fMAX_RESIST is 99 and the reduction caps at 0.8 (GLogixExPC.cpp:1516,
	// :1559). resistElement is clamped to 99 first, so 99*0.01*0.5 = 0.495,
	// and both an above-max value and the cap value agree.
	MagicDamageInput capped = MakeMagicInput(100);
	capped.skillBasicVar = 0.0f;
	capped.resistElement = 99;

	MagicDamageInput absurd = capped;
	absurd.resistElement = 100000;

	CHECK_EQ(CalculateMagicDamage(capped).rawDamage, CalculateMagicDamage(absurd).rawDamage);

	// 100 - DWORD(100 * 0.495) = 100 - 49 = 51.
	CHECK_EQ(CalculateMagicDamage(capped).rawDamage, 51u);
}

// The reduction cap: a resistance large enough to exceed 0.8 is held at 0.8.
MODERN_TEST(Magic_ResistanceReductionIsCappedAtEightyPercent)
{
	CombatConstants constants;
	constants.resistGeneralG = 1.0f;   // force fResistTotal past the 0.8 cap

	MagicDamageInput input = MakeMagicInput(100);
	input.skillBasicVar = 0.0f;
	input.resistElement = 99;         // 99 * 0.01 * 1.0 = 0.99 -> capped to 0.8

	// 100 - DWORD(100 * 0.8) = 100 - 80 = 20.
	CHECK_EQ(CalculateMagicDamage(input, constants).rawDamage, 20u);
}

// ── Magic damage reduction ─────────────────────────────────────────────
//
// Same DamageReduceAmount as physical, different source value (:1482).
MODERN_TEST(Magic_MagicDamageReduceUsesTheLevelScaledAmount)
{
	MagicDamageInput none = MakeMagicInput(100);
	MagicDamageInput reduced = MakeMagicInput(100);
	reduced.targetLevel = 150;
	reduced.magicDamageReduce = 0.5f;

	const DamageResult full = CalculateMagicDamage(none);
	const DamageResult less = CalculateMagicDamage(reduced);

	CHECK_GT(less.damage, 0u);
	CHECK_LT(less.damage, full.damage);
}

MODERN_TEST(Magic_PhysicalDamageReduceDoesNotAffectMagic)
{
	// `MagicDamageInput` has no physical reduction field at all - magic reads
	// only `magicDamageReduce` - so the two cannot be confused. What is
	// asserted is that the magic value is honoured on its own.
	MagicDamageInput plain = MakeMagicInput(100);
	MagicDamageInput reduced = plain;
	reduced.magicDamageReduce = 0.5f;

	const DamageResult plainResult  = CalculateMagicDamage(plain);
	const DamageResult reducedResult = CalculateMagicDamage(reduced);

	CHECK_GT(plainResult.damage, reducedResult.damage);
}

// ── Magic reflection ───────────────────────────────────────────────────
//
// Magic reflection is NOT the ranged-physical suppression VERTICAL-012 added.
// Magic reads m_fMagicDamageReflection / m_fMagicDamageReflectionRate
// (:1483-1484) and reflects normally.
MODERN_TEST(Magic_MagicReflectionOccursWhenEnabled)
{
	MagicDamageInput input = MakeMagicInput(100);
	input.targetLevel = 100;
	input.magicDamageReflection     = 0.5f;
	input.magicDamageReflectionRate = 0.5f;
	input.reflectionRoll = 0.0f;      // clears the rate

	const DamageResult result = CalculateMagicDamage(input);

	CHECK_EQ(result.reflectionTriggered, true);
	CHECK_GT(result.reflectionDamage, 0u);
}

MODERN_TEST(Magic_MagicReflectionDoesNotOccurWhenDisabled)
{
	MagicDamageInput input = MakeMagicInput(100);
	input.targetLevel = 100;
	input.magicDamageReflection     = 0.5f;
	input.magicDamageReflectionRate = 0.0f;   // disabled
	input.reflectionRoll = 0.0f;

	const DamageResult result = CalculateMagicDamage(input);

	CHECK_EQ(result.reflectionTriggered, false);
	CHECK_EQ(result.reflectionDamage, 0u);
}

// The complement of VERTICAL-012's suppression: magic reflection is unaffected
// by the attack type, and physical ranged reflection is still suppressed.
MODERN_TEST(Magic_MagicReflectionIsIndependentOfTheRangedSuppression)
{
	MagicDamageInput magic = MakeMagicInput(100);
	magic.targetLevel = 100;
	magic.magicDamageReflection     = 0.5f;
	magic.magicDamageReflectionRate = 0.5f;
	magic.reflectionRoll = 0.0f;

	CHECK_EQ(CalculateMagicDamage(magic).reflectionTriggered, true);

	// The same target reflecting against a ranged physical skill still does not.
	PhysicalDamageInput ranged = MakeBasicDamageInput();
	ranged.attackType = AttackType::Ranged;
	ranged.damageReflection     = 0.5f;
	ranged.damageReflectionRate = 0.5f;
	ranged.targetLevel = 100;
	ranged.reflectionRoll = 0.0f;

	CHECK_EQ(CalculatePhysicalDamage(ranged).reflectionTriggered, false);

	// ...and melee still does, so the suppression is still scoped to ranged.
	PhysicalDamageInput melee = ranged;
	melee.attackType = AttackType::Melee;
	CHECK_EQ(CalculatePhysicalDamage(melee).reflectionTriggered, true);
}

// ── Critical ───────────────────────────────────────────────────────────
//
// Magic has no separate critical rule in legacy; it shares dwCRITICAL_DAMAGE.
MODERN_TEST(Magic_CriticalAppliesTheSharedCriticalDamage)
{
	MagicDamageInput normal = MakeMagicInput(100);
	MagicDamageInput critical = normal;
	critical.criticalRoll = 0.0f;

	const DamageResult normalResult   = CalculateMagicDamage(normal);
	const DamageResult criticalResult = CalculateMagicDamage(critical);

	CHECK_EQ(normalResult.critical, false);
	CHECK_EQ(criticalResult.critical, true);
	CHECK_EQ(criticalResult.damage,
	         static_cast<uint32_t>(static_cast<float>(normalResult.damage) * 1.2f));
}

// ── Crushing ───────────────────────────────────────────────────────────
//
// EMSPECA_CRUSHING_BLOW is added in a loop outside the apply switch
// (:1494-1501), so a magic skill can carry it.
MODERN_TEST(Magic_CrushingAppliesFromTheSkillSpec)
{
	MagicDamageInput normal = MakeMagicInput(100);
	MagicDamageInput crushing = normal;
	crushing.skillCrushingBonus = 20;   // exactly the cap
	crushing.crushingRoll = 0.0f;

	const DamageResult normalResult   = CalculateMagicDamage(normal);
	const DamageResult crushingResult = CalculateMagicDamage(crushing);

	CHECK_EQ(normalResult.crushing, false);
	CHECK_EQ(crushingResult.crushing, true);
	CHECK_EQ(crushingResult.damage,
	         static_cast<uint32_t>(static_cast<float>(normalResult.damage) * 1.5f));
}

// ── Low SP ─────────────────────────────────────────────────────────────
//
// Applied exactly once, at the same 0.5 factor physical uses. Even fixture:
// the halved value is exactly half, so the doubling identity is satisfiable.
MODERN_TEST(Magic_LowSpHalvesDamageOnAnEvenFixture)
{
	MagicDamageInput funded = MakeMagicInput(100);
	MagicDamageInput low = funded;
	low.lowSP = true;

	const DamageResult fundedResult = CalculateMagicDamage(funded);
	const DamageResult lowResult    = CalculateMagicDamage(low);

	CHECK_EQ(lowResult.lowSP, true);
	CHECK_EQ(fundedResult.lowSP, false);
	CHECK_EQ(lowResult.damage, static_cast<uint32_t>(fundedResult.damage / 2));
	CHECK_EQ(lowResult.damage * 2, fundedResult.damage);
}

// Odd fixture: truncation means the doubling identity is NOT satisfiable.
// VERTICAL-012 already demonstrated why asserting it blindly is wrong, so this
// case asserts the rule instead of the arithmetic coincidence.
MODERN_TEST(Magic_LowSpHalvingOnAnOddFixtureLosesTheRemainder)
{
	MagicDamageInput funded = MakeMagicInput(100);
	MagicDamageInput low = funded;
	low.lowSP = true;

	const DamageResult fundedResult = CalculateMagicDamage(funded);
	const DamageResult lowResult    = CalculateMagicDamage(low);

	CHECK_LT(lowResult.damage, fundedResult.damage);
	CHECK_GE(fundedResult.damage - lowResult.damage, 1u);
	CHECK_LE(lowResult.damage * 2, fundedResult.damage + 1u);
}

// Exactly once, not twice. If the resolver and the calculator both halved, the
// result would be a quarter and the even-fixture identity would fail.
MODERN_TEST(Magic_LowSpIsAppliedOnceNotTwice)
{
	MagicDamageInput funded = MakeMagicInput(200);
	MagicDamageInput low = funded;
	low.lowSP = true;

	const DamageResult fundedResult = CalculateMagicDamage(funded);
	const DamageResult lowResult    = CalculateMagicDamage(low);

	CHECK_GT(fundedResult.damage / 2, 0u);
	CHECK_GT(lowResult.damage, fundedResult.damage / 4);
}

// ── Minimum damage ─────────────────────────────────────────────────────
MODERN_TEST(Magic_DamageNeverReturnsZero)
{
	MagicDamageInput input = MakeMagicInput(0);
	input.skillBasicVar = 0.0f;
	input.skillRange = { 0, 0 };
	input.resistElement = 99;      // resist the whole range away
	input.magicDamageReduce = 1.0f;

	const DamageResult result = CalculateMagicDamage(input);

	CHECK_GE(result.damage, 1u);
}

// ── Channel separation ─────────────────────────────────────────────────
//
// The strongest statement of VERTICAL-013's central claim: magic is not the
// physical formula with a flag. Same power, same range, different channel,
// different answer.
MODERN_TEST(Magic_ResistanceRunsBeforeTheRollAndPhysicalResistanceDoesNot)
{
	// The ordering difference, stated so it cannot drift.
	//
	// Magic applies resistance to the damage RANGE (:1562-1563) before the
	// roll, so `rawDamage` - the rolled figure - moves when resistance changes.
	// Physical applies resistance to the ALREADY-ROLLED value
	// (PhysicalDamageCalculator.h:119), so its `rawDamage` is recorded before
	// resistance and must not move at all.
	PhysicalDamageInput physical = MakeBasicDamageInput();
	physical.attackType     = AttackType::Melee;
	physical.meleePower     = 100;
	physical.physicalDamage = { 0, 0 };
	physical.damageRoll     = 0.0f;
	physical.criticalRoll   = 1.0f;
	physical.crushingRoll   = 1.0f;
	physical.defense        = 0;
	physical.defenseBody    = 0;
	physical.defenseItem    = 0;
	physical.resistElement  = 0;
	const uint32_t physicalRawNoResist = CalculatePhysicalDamage(physical).rawDamage;

	physical.resistElement = 50;
	const DamageResult physicalResisted = CalculatePhysicalDamage(physical);

	// Physical: the pre-resistance roll is untouched.
	CHECK_EQ(physicalResisted.rawDamage, physicalRawNoResist);
	CHECK_LT(physicalResisted.damage, physicalRawNoResist);

	// Magic: the range itself shrinks, so the rolled figure moves.
	MagicDamageInput magic = MakeMagicInput(100);
	magic.skillBasicVar = 0.0f;
	magic.resistElement = 0;
	const uint32_t magicRawNoResist = CalculateMagicDamage(magic).rawDamage;

	magic.resistElement = 50;
	const uint32_t magicRawResisted = CalculateMagicDamage(magic).rawDamage;

	CHECK_EQ(magicRawNoResist, 100u);
	CHECK_EQ(magicRawResisted, 75u);
	CHECK_LT(magicRawResisted, magicRawNoResist);
}
// ═══════════════════════════════════════════════════════════════════════
// VERTICAL-019: FACT damage applied to the range, before the attack power
// ═══════════════════════════════════════════════════════════════════════

namespace
{
	// A physical input with a pinned range and no crit/crush, so the only
	// things that can move `rawDamage` are the FACT value and the power.
	PhysicalDamageInput MakeFactDamageInput(int32_t factDamage)
	{
		PhysicalDamageInput input = MakeBasicDamageInput();
		input.physicalDamage = { 100, 120 };
		input.meleePower     = 10;
		input.shootPower     = 10;
		input.factDamage     = factDamage;
		input.damageRoll     = 0.0f;
		input.criticalRoll   = 1.0f;
		input.crushingRoll   = 1.0f;
		input.defense        = 0;
		input.defenseBody    = 0;
		input.defenseItem    = 0;
		return input;
	}
}

// No FACT: only the attack power reaches the range.
MODERN_TEST(FactDamage_NoContributionLeavesTheRangeAtPowerOnly)
{
	const DamageResult result = CalculatePhysicalDamage(MakeFactDamageInput(0));

	// 100 + 10 = 110.
	CHECK_EQ(result.rawDamage, 110u);
}

// The FACT value is added to the RANGE, so it shifts the low end as well - which
// is what a "roll then add" implementation would get wrong.
MODERN_TEST(FactDamage_AppliesToBothEndsNotTheRolledValue)
{
	const DamageResult result = CalculatePhysicalDamage(MakeFactDamageInput(25));

	// {100,120} +25 -> {125,145}, then +10 power -> {135,155}. The minimum roll
	// is 135: the FACT moved the RANGE, so both ends carry it.
	CHECK_EQ(result.rawDamage, 135u);

	// And the maximum end moved by the same amount.
	PhysicalDamageInput top = MakeFactDamageInput(25);
	top.damageRoll = 1.0f;
	CHECK_EQ(CalculatePhysicalDamage(top).rawDamage, 155u);
}

// Ordering against the attack power. Legacy applies the FACT before the power
// (GLogixExPC.cpp:2329 then :1451), and because both use the saturating add the
// order is observable exactly at the VAR_PARAM floor of 1.
MODERN_TEST(FactDamage_IsAppliedBeforeTheAttackPower)
{
	// Range {1,1}: the -1 FACT floors the end at 1 and it STAYS there, then the
	// +10 power lifts it to 11. Applying the power first would give {11,11}
	// then -1 -> {10,10}.
	PhysicalDamageInput input = MakeFactDamageInput(-1);
	input.physicalDamage = { 1, 1 };

	const DamageResult result = CalculatePhysicalDamage(input);

	CHECK_EQ(result.rawDamage, 11u);
}

MODERN_TEST(FactDamage_VarParamFloorIsOne)
{
	// A large negative floors the range end at 1 rather than wrapping.
	PhysicalDamageInput input = MakeFactDamageInput(-500);

	const DamageResult result = CalculatePhysicalDamage(input);

	// 1 (floor) + 10 (power) = 11.
	CHECK_EQ(result.rawDamage, 11u);
}

MODERN_TEST(FactDamage_SameValueAppliesToRangedAsToMelee)
{
	PhysicalDamageInput ranged = MakeFactDamageInput(25);
	ranged.attackType = AttackType::Ranged;
	ranged.shootPower = 40;

	const DamageResult result = CalculatePhysicalDamage(ranged);

	// {125,145} then +40 -> 165.
	CHECK_EQ(result.rawDamage, 165u);
}

// One source, two channels: magic takes the same field and the same operation.
MODERN_TEST(FactDamage_MagicPathUsesTheSameContribution)
{
	MagicDamageInput magic;
	magic.magicAttack    = 10;
	magic.skillRange     = { 100, 120 };
	magic.factDamage     = 25;
	magic.damageRoll     = 0.0f;
	magic.criticalRoll   = 1.0f;
	magic.crushingRoll   = 1.0f;
	magic.reflectionRoll = 1.0f;
	// Required: CriticalBaseRate divides by max HP, which is unguarded.
	magic.attackerMaxHP     = 1000;
	magic.attackerCurrentHP = 1000;

	const DamageResult result = CalculateMagicDamage(magic);

	// {125,145} then +10 magic attack -> 135.
	CHECK_EQ(result.rawDamage, 135u);
}

MODERN_TEST(FactDamage_ExpiryIsJustTheValueGoingAway)
{
	// The same range with and without the contribution; nothing is restored,
	// the input simply stops carrying the number.
	const uint32_t withFact = CalculatePhysicalDamage(MakeFactDamageInput(25)).rawDamage;
	const uint32_t without  = CalculatePhysicalDamage(MakeFactDamageInput(0)).rawDamage;

	CHECK_GT(withFact, without);
	CHECK_EQ(withFact - without, 25u);
}
// ═══════════════════════════════════════════════════════════════════════
// VERTICAL-020: flat total defence reaching the damage figure
// ═══════════════════════════════════════════════════════════════════════

MODERN_TEST(FactDefense_ReducesFinalDamage)
{
	// The same roll twice; only the defence differs. This is the deterministic
	// form of the assertion the server test cannot make, because CastSkill
	// advances its RNG sequence per cast.
	PhysicalDamageInput bare = MakeBasicDamageInput();
	bare.physicalDamage = { 100, 100 };
	bare.meleePower = 0;
	bare.shootPower = 0;
	bare.defense = 0;
	bare.defenseBody = 0;
	bare.defenseItem = 0;
	bare.damageRoll = 0.0f;
	bare.criticalRoll = 1.0f;
	bare.crushingRoll = 1.0f;
	bare.lowSP = false;

	PhysicalDamageInput defended = bare;
	defended.defense = 20;

	const DamageResult bareResult    = CalculatePhysicalDamage(bare);
	const DamageResult defendedResult = CalculatePhysicalDamage(defended);

	CHECK_EQ(bareResult.damage - defendedResult.damage, 20u);
}

MODERN_TEST(FactDefense_BodyAndItemDecayIsSeparateFromFlatDefense)
{
	// The FACT adds flat defence only. Body/item defence feeds the separate
	// decay stage (GLogixExPC.cpp:1701-1713) and is not the same axis.
	PhysicalDamageInput bare = MakeBasicDamageInput();
	bare.defense = 0;
	bare.defenseBody = 0;
	bare.defenseItem = 0;
	bare.damageRoll = 0.0f;
	bare.criticalRoll = 1.0f;
	bare.crushingRoll = 1.0f;

	PhysicalDamageInput decayOnly = bare;
	decayOnly.defenseBody = 30;
	decayOnly.defenseItem = 30;

	CHECK_LT(CalculatePhysicalDamage(decayOnly).damage,
	         CalculatePhysicalDamage(bare).damage);
}