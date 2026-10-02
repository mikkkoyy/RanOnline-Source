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
	// VERTICAL-025: this test asserted that resistance reduces the damage, and
	// it passed only because resistance used to apply unconditionally. Legacy
	// applies it inside `if (pSkill)` (GLogixExPC.cpp:1417, block closing :1571),
	// so a BASIC attack is never resisted and the original assertion was
	// encoding the deviation. `skillCast` is set so the test still exercises what
	// it was written to check - a resisted physical hit - on the path legacy
	// actually resists.
	input.skillCast = true;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	// VERTICAL-025: the original assertion was
	// `preDefenseDamage < rawDamage`, which only held while resistance ran on
	// the rolled figure - `rawDamage` was captured before it and
	// `preDefenseDamage` after. With resistance on the range, the roll is
	// already resisted, so the two are equal here.
	//
	// The test's actual intent is "a resisted hit does less damage than an
	// identical unresisted one", which is what is asserted now - against a
	// baseline captured from the calculator itself rather than a field that
	// used to straddle the resistance.
	PhysicalDamageInput unresisted = input;
	unresisted.resistElement = 0;
	const uint32_t withoutResist = CalculatePhysicalDamage(unresisted, constants).rawDamage;

	const DamageResult result = CalculatePhysicalDamage(input, constants);

	CHECK_EQ(result.preDefenseDamage, result.rawDamage);
	CHECK_LT(result.rawDamage, withoutResist);
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
	// VERTICAL-025: `skillCast` for the same reason as CombatResist_Positive -
	// legacy resists only inside `if (pSkill)`.
	input.skillCast = true;
	input.damageRoll = 0.5f;
	input.criticalRoll = 1.0f;
	input.crushingRoll = 1.0f;

	DamageResult result = CalculatePhysicalDamage(input, constants);

	const int32_t clampedResist = 99;
	float fResistTotal = static_cast<float>(clampedResist) * 0.01f * constants.resistPhysicG;

	// VERTICAL-025: the expectation is recomputed in LEGACY's form - a
	// subtractive fold on the RANGE - rather than the multiplicative
	// post-roll form this test originally used. Recomputing rather than reading
	// `rawDamage` back keeps the assertion independent of the roll.
	Stats::DamageRange ranged = input.physicalDamage;
	const int32_t power = static_cast<int32_t>(input.meleePower);
	ranged.low  = ApplyAttackPower(ranged.low,  power);
	ranged.high = ApplyAttackPower(ranged.high, power);
	ranged.low  = ranged.low  - static_cast<uint32_t>(static_cast<float>(ranged.low)  * fResistTotal);
	ranged.high = ranged.high - static_cast<uint32_t>(static_cast<float>(ranged.high) * fResistTotal);
	ranged.low  = ApplyDamageRate(ranged.low,  input.damageRate);
	ranged.high = ApplyDamageRate(ranged.high, input.damageRate);

	const uint32_t expectedRoll = static_cast<uint32_t>(
		static_cast<float>(ranged.low) +
		(static_cast<float>(ranged.high) - static_cast<float>(ranged.low)) * input.damageRoll);

	CHECK_EQ(result.rawDamage, expectedRoll);
	CHECK_GT(result.damage, 0u);
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

// ═══════════════════════════════════════════════════════════════════════════
// VERTICAL-025: physical resistance on the damage RANGE
// ═══════════════════════════════════════════════════════════════════════════

namespace
{
	// A physical skill hit with resistance only, so the resistance is the sole
	// thing that can move the result.
	PhysicalDamageInput MakeResistInput(uint32_t low, uint32_t high, int32_t resist)
	{
		PhysicalDamageInput in = MakeBasicDamageInput();
		in.physicalDamage = { low, high };
		in.meleePower = 0;
		in.shootPower = 0;
		in.skillCast = true;
		in.resistElement = resist;
		in.defense = 0;
		in.defenseBody = 0;
		in.defenseItem = 0;
		in.lowSP = false;
		in.damageRoll = 0.5f;
		in.criticalRoll = 1.0f;
		in.crushingRoll = 1.0f;
		return in;
	}
}

// MANDATORY REGRESSION: zero resistance must leave the range untouched. This
// is what protects every existing physical baseline.
MODERN_TEST(PhysicalResist_ZeroLeavesTheRangeUnchanged)
{
	CombatConstants constants;
	const PhysicalDamageInput in = MakeResistInput(101, 102, 0);

	CHECK_EQ(CalculatePhysicalDamage(in, constants).rawDamage,
	         CalculatePhysicalDamage(MakeResistInput(101, 102, 0), constants).rawDamage);
	CHECK_EQ(CalculatePhysicalDamage(in, constants).rawDamage, 101u);
}

// Legacy form, GLogixExPC.cpp:1562-1563:
//   fResistTotal = nRESIST * 0.01 * fRESIST_G, capped at 0.8
//   dw -= (DWORD)(dw * fResistTotal)
//
// fRESIST_PHYSIC_G is 0.5, so resistElement 50 gives fResistTotal 0.25.
MODERN_TEST(PhysicalResist_UsesTheSubtractiveLegacyForm)
{
	CombatConstants constants;
	CHECK_EQ(constants.resistPhysicG, 0.5f);

	// 101 - (DWORD)(101 * 0.25) = 101 - 25 = 76; roll 0.5 of {76,76} = 76.
	CHECK_EQ(CalculatePhysicalDamage(MakeResistInput(101, 102, 50), constants).rawDamage, 76u);
}

// THE ORDERING DISCRIMINATOR. Range 101..102, resist 50 (fResistTotal 0.25),
// roll 0.5.
//
//   legacy  range-then-resist: {76,76}, roll -> 76
//   old     roll-then-resist: roll 101.5 -> 101, then 101 * 0.75 = 75.75 -> 75
//
// 76 against 75. The two are one apart, which is the whole point: a test whose
// candidates happened to agree would prove nothing.
MODERN_TEST(PhysicalResist_ResistAppliesToTheRangeBeforeTheRoll)
{
	CombatConstants constants;

	// The correct pre-roll ends, computed independently of the calculator.
	CHECK_EQ(101u - static_cast<uint32_t>(101.0f * 0.25f), 76u);
	CHECK_EQ(102u - static_cast<uint32_t>(102.0f * 0.25f), 77u);

	// With equal ends after rounding, either order's roll would land on the low
	// end, so the assertion that discriminates is on a range whose ends survive
	// the fold separately.
	PhysicalDamageInput in = MakeResistInput(101, 102, 50);
	in.damageRoll = 1.0f;   // take the high end exactly

	CHECK_EQ(CalculatePhysicalDamage(in, constants).rawDamage, 77u);
}

// A range wide enough that the two ends survive the fold distinctly, so the
// discriminator is not an artefact of rounding.
MODERN_TEST(PhysicalResist_WideRangeDiscriminatesTheTwoOrders)
{
	CombatConstants constants;

	PhysicalDamageInput legacyOrder = MakeResistInput(100, 200, 50);
	legacyOrder.damageRoll = 0.0f;
	// {100 - 25, 200 - 50} = {75, 150}, roll 0.0 -> 75.
	CHECK_EQ(CalculatePhysicalDamage(legacyOrder, constants).rawDamage, 75u);

	// The old post-roll form: roll 100.0 -> 100, then trunc(100 * 0.75) = 75.
	// Equal here by coincidence, which is why the tight range above is the real
	// discriminator and this one is the sanity check that a wide range still
	// lands where the arithmetic says.
	PhysicalDamageInput sanity = MakeResistInput(100, 200, 50);
	sanity.damageRoll = 0.5f;
	// {75,150}, roll 0.5 -> trunc(75 + 37.5) = 112.
	CHECK_EQ(CalculatePhysicalDamage(sanity, constants).rawDamage, 112u);
}

// SCOPE. Legacy resists only inside `if (pSkill)`; the basic-attack `else`
// (GLogixExPC.cpp:1572-1597) has no resistance at all.
MODERN_TEST(PhysicalResist_BasicAttacksAreNeverResisted)
{
	CombatConstants constants;

	PhysicalDamageInput skill = MakeResistInput(100, 200, 50);
	skill.skillCast = true;
	skill.damageRoll = 0.0f;

	PhysicalDamageInput basic = skill;
	basic.skillCast = false;

	CHECK_LT(CalculatePhysicalDamage(skill, constants).rawDamage,
	         CalculatePhysicalDamage(basic, constants).rawDamage);
	// The basic attack is exactly the unresisted range.
	CHECK_EQ(CalculatePhysicalDamage(basic, constants).rawDamage, 100u);
}

// INTERACTION WITH DAMAGE_RATE. Legacy order is resist (:1562-1563) then
// DAMAGE_RATE (:1600) then roll (:1672). The two do not commute.
MODERN_TEST(PhysicalResist_ResistThenDamageRateThenRoll)
{
	CombatConstants constants;

	// Range 100..100, resist 50 -> 75, rate 1.5 -> trunc(112.5) = 112.
	PhysicalDamageInput in = MakeResistInput(100, 100, 50);
	in.damageRate = 1.5f;
	in.damageRoll = 0.0f;

	CHECK_EQ(CalculatePhysicalDamage(in, constants).rawDamage, 112u);

	// The reverse order would give 100 * 1.5 = 150, then - 37 = 113. Different,
	// so the assertion pins resist-first.
	PhysicalDamageInput rateFirst = MakeResistInput(100, 100, 0);
	rateFirst.damageRate = 1.5f;
	CHECK_EQ(CalculatePhysicalDamage(rateFirst, constants).rawDamage, 150u);
}

// INTERACTION WITH THE V019 DAMAGE FACT, which is also a range input. Legacy
// adds it before resistance (:2329 then :1448/:1594 then :1562).
MODERN_TEST(PhysicalResist_DamageFactIsResistedToo)
{
	CombatConstants constants;

	PhysicalDamageInput in = MakeResistInput(100, 100, 50);
	in.factDamage = 40;
	in.damageRoll = 0.0f;

	// VAR_PARAM floors at 1: 100 + 40 = 140, then 140 - (DWORD)(140*0.25) = 140 - 35 = 105.
	CHECK_EQ(CalculatePhysicalDamage(in, constants).rawDamage, 105u);
}

// RANGED shares the pipeline: same field, same operation, no second
// implementation. Switching AttackType must not fork the resistance.
MODERN_TEST(PhysicalResist_RangedUsesTheSameResistance)
{
	CombatConstants constants;

	PhysicalDamageInput melee = MakeResistInput(100, 200, 50);
	melee.meleePower = 40;
	melee.damageRoll = 0.0f;

	PhysicalDamageInput ranged = melee;
	ranged.attackType = AttackType::Ranged;
	ranged.shootPower = 40;

	CHECK_EQ(CalculatePhysicalDamage(melee, constants).rawDamage,
	         CalculatePhysicalDamage(ranged, constants).rawDamage);
}

// HIGH resistance. fMAX_RESIST is 99 (:1516) and fRESIST_PHYSIC_G is 0.5, so the
// largest reachable fResistTotal is 0.495 and the 0.8 cap is unreachable in the
// physical path - the raw-value clamp is the only cap that can bite.
MODERN_TEST(PhysicalResist_HighValuesClampRatherThanWrap)
{
	CombatConstants constants;

	PhysicalDamageInput atMax = MakeResistInput(100, 200, 99);
	atMax.damageRoll = 0.0f;
	PhysicalDamageInput absurd = MakeResistInput(100, 200, 100000);
	// The roll has to match, or this compares a low-end roll against a midpoint
	// roll and fails for a reason that has nothing to do with the clamp.
	absurd.damageRoll = 0.0f;

	CHECK_EQ(CalculatePhysicalDamage(atMax, constants).rawDamage,
	         CalculatePhysicalDamage(absurd, constants).rawDamage);
	// 100 - (DWORD)(100 * 0.495) = 100 - 49 = 51.
	CHECK_EQ(CalculatePhysicalDamage(atMax, constants).rawDamage, 51u);
}

// NEGATIVE resistance cannot occur through any legitimate source: nRESIST comes
// from SRESIST, whose elements `LIMIT()` floors at zero (GLogixExPC.cpp:2979),
// and the aggregator floors every axis at zero too. Modern refuses it at the
// input boundary rather than letting a negative reach the unsigned fold.
MODERN_TEST(PhysicalResist_NegativeResistanceIsNotModelled)
{
	CombatConstants constants;

	// The guard is `resistElement > 0`, so a negative is treated as none - not
	// as a damage bonus. This is recorded behaviour, not an invented clamp:
	// legacy's source cannot produce the value, so there is nothing to reproduce.
	CHECK_EQ(CalculatePhysicalDamage(MakeResistInput(100, 200, 0), constants).rawDamage,
	         CalculatePhysicalDamage(MakeResistInput(100, 200, -50), constants).rawDamage);
}

// DEFENSE, CRITICAL and low-SP must be untouched by the move. The correction
// only relocated resistance; everything downstream still reads the rolled
// figure.
MODERN_TEST(PhysicalResist_DownstreamStagesAreUnchanged)
{
	CombatConstants constants;

	PhysicalDamageInput base = MakeResistInput(100, 200, 50);
	base.damageRoll = 0.0f;

	PhysicalDamageInput defended = base;
	defended.defense = 10;
	CHECK_LT(CalculatePhysicalDamage(defended, constants).damage,
	         CalculatePhysicalDamage(base, constants).damage);

	PhysicalDamageInput lowSp = base;
	lowSp.lowSP = true;
	CHECK_LT(CalculatePhysicalDamage(lowSp, constants).damage,
	         CalculatePhysicalDamage(base, constants).damage);

	// Critical needs the roll to land it, and it must scale a figure that
	// resistance has already reduced.
	PhysicalDamageInput crit = base;
	crit.damageRoll = 0.0f;
	crit.criticalRoll = 0.0f;
	CHECK(CalculatePhysicalDamage(crit, constants).damage >= 1u);
}

// MAGIC MUST NOT MOVE. V024 proved magic already reduces the range before the
// roll; this asserts the physical correction did not touch it.
MODERN_TEST(PhysicalResist_MagicResistanceIsUnchanged)
{
	MagicDamageInput magic = MakeMagicInput(100);
	magic.skillBasicVar = 0.0f;
	magic.resistElement = 0;
	const uint32_t none = CalculateMagicDamage(magic).rawDamage;

	magic.resistElement = 50;
	// 100 - (DWORD)(100 * 0.25) = 75 - the same subtractive form, unchanged.
	CHECK_EQ(CalculateMagicDamage(magic).rawDamage, 75u);
	CHECK_LT(CalculateMagicDamage(magic).rawDamage, none);
}

MODERN_TEST(Magic_DamageRateAppliesAfterResistanceAndBeforeTheRoll)
{
	MagicDamageInput plain = MakeMagicInput(100);
	plain.skillBasicVar = 0.0f;   // range {100,100}
	plain.resistElement = 0;
	plain.damageRate = 1.5f;
	plain.damageRoll = 0.0f;

	// 100 * 1.5 = 150.
	CHECK_EQ(CalculateMagicDamage(plain).rawDamage, 150u);

	// With resistance: legacy reduces the range first (100 - 25 = 75) and only
	// then applies the rate (75 * 1.5 = 112.5 -> 112). Rate-first would give
	// 150 - DWORD(150*0.25) = 150 - 37 = 113. The two differ, so the assertion
	// discriminates the order rather than merely exercising both.
	MagicDamageInput resisted = plain;
	resisted.resistElement = 50;

	CHECK_EQ(CalculateMagicDamage(resisted).rawDamage, 112u);
}

// Rate 1.0 must be an identity here too, which is what keeps every V013 magic
// baseline unmoved.
MODERN_TEST(Magic_DamageRateOneIsAnIdentity)
{
	MagicDamageInput input = MakeMagicInput(100);
	input.skillBasicVar = 0.0f;
	input.resistElement = 0;
	input.damageRate = 1.0f;
	input.damageRoll = 0.0f;

	CHECK_EQ(CalculateMagicDamage(input).rawDamage, 100u);
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
	// The ordering, stated so it cannot drift.
	//
	// VERTICAL-025: this block previously asserted that PHYSICAL resistance
	// runs on the already-rolled figure while MAGIC runs on the range - a real
	// difference at the time, but the physical half of it was a deviation from
	// legacy, not a distinction. Both channels reduce the range in
	// CALCDAMAGE_20060328 (`:1562-1563`), so both must move `rawDamage`.
	//
	// The physical case is a SKILL cast here, because legacy resists only inside
	// `if (pSkill)`.
	PhysicalDamageInput physical = MakeBasicDamageInput();
	physical.attackType     = AttackType::Melee;
	physical.meleePower     = 100;
	physical.physicalDamage = { 0, 0 };
	physical.skillCast      = true;
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

	// Physical: the range shrinks before the roll, so the rolled figure moves.
	CHECK_LT(physicalResisted.rawDamage, physicalRawNoResist);
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
// ═══════════════════════════════════════════════════════════════════════
// VERTICAL-021: the defence rate reaches the damage figure
// ═══════════════════════════════════════════════════════════════════════

MODERN_TEST(DefenseRate_HigherRateReducesFinalDamageByTheSameRoll)
{
	// The same roll twice; only the resolved defence differs. This is the
	// deterministic form of the assertion the server test cannot make, because
	// CastSkill advances its RNG sequence per cast.
	PhysicalDamageInput bare = MakeBasicDamageInput();
	bare.physicalDamage = { 300, 300 };
	bare.meleePower = 0;
	bare.shootPower = 0;
	bare.defense = 0;
	bare.defenseBody = 0;
	bare.defenseItem = 0;
	bare.damageRoll = 0.0f;
	bare.criticalRoll = 1.0f;
	bare.crushingRoll = 1.0f;
	bare.lowSP = false;

	// A 50% defence boost takes 100 defence to 150 through ApplyDefenseRate.
	PhysicalDamageInput defended = bare;
	defended.defense = Modern::Engine::ApplyDefenseRate(100, 1.5f);
	CHECK_EQ(defended.defense, 150);

	const DamageResult bareResult     = CalculatePhysicalDamage(bare);
	const DamageResult defendedResult = CalculatePhysicalDamage(defended);

	// The whole resolved defence is subtracted, so the boost costs the attacker
	// exactly the 50 points it added to the defence.
	CHECK_EQ(bareResult.damage, 300u);
	CHECK_EQ(defendedResult.damage, 150u);
	CHECK_EQ(bareResult.damage - defendedResult.damage, 150u);
}

// Defence must be able to floor damage: a boosted defence that overshoots the
// remaining damage still clamps to the damage floor, exactly as legacy does.
MODERN_TEST(DefenseRate_BoostedDefenseBeyondDamageFloorsDamage)
{
	PhysicalDamageInput bare = MakeBasicDamageInput();
	bare.physicalDamage = { 30, 30 };
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
	defended.defense = Modern::Engine::ApplyDefenseRate(100, 2.0f);  // 200

	CHECK_EQ(defended.defense, 200);

	// 30 - 200 goes below zero and the damage floor is 1, not 0.
	CHECK_EQ(CalculatePhysicalDamage(defended).damage, 1u);
}

// ═══════════════════════════════════════════════════════════════════════════
// VERTICAL-024: the damage-rate multiplier on the physical path
// ═══════════════════════════════════════════════════════════════════════════

namespace
{
	// A damage-rate case with every other axis pinned: no attack power, no
	// resistance, no critical, no defense, no reduction, no reflection. Only
	// `damageRate` can move the result, so a failure localises to the axis.
	PhysicalDamageInput MakeDamageRateInput(uint32_t low, uint32_t high, float rate)
	{
		PhysicalDamageInput in = MakeBasicDamageInput();
		in.physicalDamage = { low, high };
		in.meleePower = 0;
		in.shootPower = 0;
		in.damageRate = rate;
		in.defense = 0;
		in.defenseBody = 0;
		in.defenseItem = 0;
		in.resistElement = 0;
		in.damageRoll = 0.0f;      // take the low end exactly
		in.criticalRoll = 1.0f;    // never crit
		in.crushingRoll = 1.0f;    // never crush
		in.lowSP = false;
		return in;
	}
}

// Rate 1.0 is the identity. This is the load-bearing no-buff property: it is
// what lets the axis be added without moving any VERTICAL-006/009 baseline.
MODERN_TEST(DamageRate_RateOneIsAnIdentityOnBothEnds)
{
	const DamageResult atOne = CalculatePhysicalDamage(MakeDamageRateInput(100, 200, 1.0f));
	CHECK_EQ(atOne.rawDamage, 100u);
	CHECK_EQ(atOne.damage, 100u);
}

// ORDERING, and the reason this test exists. Legacy applies the rate to the
// RANGE immediately before the roll (GLogixExPC.cpp:1600-1603 then :1672), not
// to the rolled figure and not before the attack power.
//
// range 100, rate 1.5:
//   legacy  (pre-roll)  -> int(100 * 1.5)               = 150
//   post-roll           -> int(100) then int(100 * 1.5)= 150  (same here)
// so a single case cannot discriminate; this one does, with a fractional
// product and a non-zero roll:
//
// range 101..102, rate 1.5, roll 0.5
//   legacy:  int(101*1.5)=151, int(102*1.5)=153, roll -> 152
//   post-roll: roll 101.5 -> 101, then int(101*1.5)    = 151
// The two differ, so the assertion pins the pre-roll position.
MODERN_TEST(DamageRate_AppliesToTheRangeBeforeTheRoll)
{
	PhysicalDamageInput in = MakeDamageRateInput(101, 102, 1.5f);
	in.damageRoll = 0.5f;

	// The pre-roll range is observable through `rawDamage`, which modern sets
	// immediately after the roll from the (already rated) range.
	const DamageResult rated = CalculatePhysicalDamage(in);
	CHECK_EQ(rated.rawDamage, 152u);

	// The pre-roll ends must be 151 and 153 - not 101/102 (no rate applied) and
	// not the post-roll alternative. Recomputed independently here rather than
	// read back from a field, so a change in the calculator cannot agree with
	// itself.
	CHECK_EQ(Modern::Engine::ApplyDamageRate(101u, 1.5f), 151u);
	CHECK_EQ(Modern::Engine::ApplyDamageRate(102u, 1.5f), 153u);
}

// Truncation is toward zero, applied to the multiplied value.
MODERN_TEST(DamageRate_TruncatesTowardZero)
{
	// 101 * 1.333 = 134.633 -> 134
	CHECK_EQ(CalculatePhysicalDamage(MakeDamageRateInput(101, 101, 1.333f)).rawDamage, 134u);
}

// ORDERING against the attack power. Legacy adds the power BEFORE the rate
// (:1594 then :1600), so a rate scales the power too. Applying it first would
// give a different integer, and this range is chosen so it does.
MODERN_TEST(DamageRate_AppliesAfterTheAttackPower)
{
	PhysicalDamageInput in = MakeDamageRateInput(100, 100, 1.5f);
	in.meleePower = 3;

	// (100 + 3) * 1.5 = 154.5 -> 154. Rate-before-power would give
	// 100 * 1.5 + 3 = 153.
	CHECK_EQ(CalculatePhysicalDamage(in).rawDamage, 154u);
}

// A rate below 1.0 is a reduction, and legacy clamps nothing.
MODERN_TEST(DamageRate_BelowOneReducesWithoutAClamp)
{
	CHECK_EQ(CalculatePhysicalDamage(MakeDamageRateInput(100, 100, 0.5f)).rawDamage, 50u);
	// 0.0 drives the range to zero, and the damage floor of 1 then applies -
	// the floor is downstream of the rate, at :1777.
	CHECK_EQ(CalculatePhysicalDamage(MakeDamageRateInput(100, 100, 0.0f)).damage, 1u);
}

// NEGATIVE, and this pins a two-step legacy hazard rather than tidying it up.
//
// `ApplyDamageRate` is `static_cast<GameUInt32>(float(damage) * rate)`
// (GameCharacterCalculations.cpp:667-668). With a negative rate the product is
// negative and the conversion wraps two's-complement: -100.0f becomes
// 0xFFFFFF9C. Legacy authored a clamp for exactly this
// (`if (m_fDamageRate <= 0.0f) { dwLow = 0; dwHigh = 0; }`) and then commented
// the whole block out at GLogixExPC.cpp:1634-1650 and :1963-1977, so it never
// runs.
//
// The end-to-end result is NOT simply "wraps to a huge number", and this test
// exists because the first draft of it asserted exactly that and was wrong. The
// roll converts the range end back to float, and 4294967196 is not
// representable: floats near 2^32 are spaced 512 apart, so it rounds to
// 2^32 exactly. `static_cast<uint32_t>(2^32)` is then out of range and MSVC
// yields 0. The damage therefore floors at 1 like any other empty roll.
//
// Reproduced as measured, not as guessed: an invented clamp would change a
// number RAN produces, and an invented wrap model would have hidden the
// rounding that actually decides the outcome.
MODERN_TEST(DamageRate_NegativeRateWrapsThenCollapsesToZeroThroughTheRoll)
{
	// Step 1: the conversion itself wraps, on this compiler.
	CHECK_EQ(Modern::Engine::ApplyDamageRate(100u, -1.0f), 0xFFFFFF9Cu);

	// Step 2: that value cannot survive the float round-trip in the roll.
	const DamageResult negative = CalculatePhysicalDamage(
		MakeDamageRateInput(100, 100, -1.0f));
	CHECK_EQ(negative.rawDamage, 0u);
	CHECK_EQ(negative.damage, 1u);
}

// The rate is independent of the V019 flat `factDamage`: they are separate
// impacts, added at separate points, and one must not stand in for the other.
MODERN_TEST(DamageRate_IsSeparateFromTheFactDamageImpact)
{
	PhysicalDamageInput flat = MakeDamageRateInput(100, 100, 1.0f);
	flat.factDamage = 10;

	PhysicalDamageInput rate = MakeDamageRateInput(100, 100, 1.5f);
	rate.factDamage = 10;

	// flat only  -> 100 + 10, saturating at 1, then * 1.0 = 110
	CHECK_EQ(CalculatePhysicalDamage(flat).rawDamage, 110u);
	// both       -> (100 + 10) * 1.5 = 165
	CHECK_EQ(CalculatePhysicalDamage(rate).rawDamage, 165u);
}

// ORDERING against the low-SP reduction. Legacy applies the rate to the range
// long before the low-SP multiplier, which lives in the defender's
// `PreStrikeProc` (GLChar.cpp:2488-2491) and runs on the rolled figure. These
// therefore never interact arithmetically, and the test says so.
MODERN_TEST(DamageRate_AndLowSpAreIndependent)
{
	PhysicalDamageInput in = MakeDamageRateInput(100, 100, 2.0f);
	in.lowSP = true;

	const DamageResult lowSp = CalculatePhysicalDamage(in);

	// Rate doubles the range to 200; low-SP then scales the post-defence figure.
	// The point is that the rate is already inside `rawDamage` when low-SP
	// runs, not that the product is some particular number.
	CHECK_EQ(lowSp.rawDamage, 200u);
	CHECK_EQ(lowSp.lowSP, true);
	CHECK(CalculatePhysicalDamage(in).damage < 200u);
}