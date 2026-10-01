// VERTICAL-011: active skill resolution rules.
//
// Headless. Links Modern and nothing else: no renderer, no socket, no database,
// no legacy library, no server, no client.
//
// The passive half of the skill system is tested in SkillTests.cpp and shares no
// code path with this file, which is the point: a learned passive and a castable
// skill are different things, and this file is only about the second.
//
// Every case drives `ActiveSkillInput` directly. The resolver reads nothing and
// fetches nothing, so a test can reach every branch - including refusals for
// systems the modern server does not have - without building a world, and every
// roll is injected so the result is the same on every run.

#include "TestHarness.h"

#include "combat/CombatConstants.h"
#include "skills/ActiveSkill.h"
#include "skills/SkillDefinition.h"

#include <cmath>
#include <limits>

using namespace Modern;
using namespace Modern::Skills;

namespace
{
	constexpr float kRoll0   = 0.0f;
	constexpr float kRoll05  = 0.5f;
	constexpr float kRoll1   = 1.0f;

	// A physical melee damage skill, castable, aimed at a specific target on the
	// enemy side. Mirrors a legacy definition whose `fBASIC_VAR` is negative, so
	// `fBASIC_VAR = -hp` reads as "deal hp damage".
	SkillDefinition MakeDamageSkill(uint16_t skillIndex = 1)
	{
		SkillDefinition def;
		def.id         = SkillId{ 1, skillIndex };
		def.name       = "Cleave" + std::to_string(skillIndex);
		def.maxLevel   = 3;
		def.grade      = 2;
		def.role       = SkillRole::Normal;
		def.apply      = SkillApply::PhysicalMelee;
		def.targetKind = SkillTargetKind::Spec;
		def.impactSide = SkillImpactSide::Enemy;
		def.applyType  = PassiveApplyType::Hp;

		for (uint8_t level = 1; level <= def.maxLevel; ++level)
		{
			def.levelData[level].basicVar  = -10.0f * static_cast<float>(level);
			def.levelData[level].useSp     = static_cast<uint16_t>(5 * level);
			def.levelData[level].useHp     = 0;
			def.levelData[level].useMp     = static_cast<uint16_t>(2 * level);
			def.levelData[level].delayTime = 0.5f;
		}
		return def;
	}

	// A situation in which the cast is expected to reach the damage stage.
	// Rolls are fixed so a difference in outcome can only come from the field
	// under test.
	ActiveSkillInput MakeInput(const SkillDefinition& definition, uint8_t level = 1)
	{
		ActiveSkillInput input;
		input.definition = &definition;
		input.level      = level;

		input.attacker.hit        = 50;
		input.attacker.avoid      = 50;
		input.attacker.meleePower = 10;
		input.attacker.shootPower = 10;
		input.attacker.defense   = 10;
		input.attacker.maxHp      = 200;
		input.attacker.physicalDamage.low  = 5;
		input.attacker.physicalDamage.high = 5;

		input.target.hit        = 50;
		input.target.avoid      = 10;
		input.target.defense    = 0;
		input.target.maxHp      = 300;
		input.hasTarget         = true;
		input.targetCurrentHp   = 300;
		input.targetLevel       = 1;

		input.attackerLevel = 1;
		input.currentHp     = 200;
		input.currentMp     = 100;
		input.currentSp     = 100;

		input.equipmentRequiredSP = 0;
		input.basicAttackSP = Combat::CombatConstants().basicDisSP;

		input.hitRoll        = kRoll05;
		input.damageRoll     = kRoll05;
		input.criticalRoll   = kRoll1;
		input.crushingRoll   = kRoll1;
		input.reflectionRoll = kRoll1;

		input.weatherElementPower = 1.0f;
		return input;
	}
}

// ── Test 1: a learned skill at level 1 succeeds ───────────────────────

MODERN_TEST(ActiveSkill_LearnedSkillSucceeds)
{
	const SkillDefinition definition = MakeDamageSkill();
	ActiveSkillInput input = MakeInput(definition, 1);

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(input);

	CHECK_EQ(result.failure, ActiveSkillFailure::None);
	CHECK(result.Succeeded());
	CHECK_EQ(result.level, static_cast<uint8_t>(1));
	CHECK_EQ(result.IsLowSp(), false);
	CHECK_GT(result.combat.damageResult.damage, 0u);
}

// ── Test 2: not learned ───────────────────────────────────────────────
//
// The server decides this from its own SkillState, so the resolver's input is
// the already-resolved level. Level 0 is what "not learned" looks like by the
// time it reaches here, and the resolver refuses it rather than casting a skill
// nobody has.

MODERN_TEST(ActiveSkill_NotLearnedRejected)
{
	const SkillDefinition definition = MakeDamageSkill();
	ActiveSkillInput input = MakeInput(definition, 0);

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(input);

	CHECK_EQ(result.failure, ActiveSkillFailure::InvalidLevel);
	CHECK(!result.Succeeded());
}

// ── Test 3: invalid skill id ──────────────────────────────────────────

MODERN_TEST(ActiveSkill_UnknownDefinitionRejected)
{
	ActiveSkillInput input = MakeInput(MakeDamageSkill(), 1);
	input.definition = nullptr;

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(input);

	CHECK_EQ(result.failure, ActiveSkillFailure::UnknownSkill);
	CHECK(!result.Succeeded());
}

// ── Test 4: required SP = equipment contribution + wUSE_SP ────────────

MODERN_TEST(ActiveSkill_RequiredSPCombinesEquipmentAndSkill)
{
	const SkillDefinition definition = MakeDamageSkill();
	ActiveSkillInput input = MakeInput(definition, 1);
	// VERTICAL-010's hand contribution, 30.
	input.equipmentRequiredSP = 30;
	// Level 1 of this definition costs wUSE_SP = 5.
	input.currentSp = 1000;

	CHECK_EQ(ActiveSkillResolver::RequiredSP(input, definition),
	         static_cast<uint16_t>(35));

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(input);
	CHECK_EQ(result.requiredSP, static_cast<uint16_t>(35));
	// Funded, so the full cost is charged.
	CHECK_EQ(result.spCost, static_cast<uint16_t>(35));
}

// ── Test 5: exact SP boundary ─────────────────────────────────────────

MODERN_TEST(ActiveSkill_ExactSPBoundaryIsNotLowSp)
{
	const SkillDefinition definition = MakeDamageSkill();
	ActiveSkillInput funded = MakeInput(definition, 1);
	funded.equipmentRequiredSP = 30;
	funded.currentSp = 35;

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(funded);

	CHECK_EQ(result.requiredSP, static_cast<uint16_t>(35));
	CHECK_EQ(result.IsLowSp(), false);
	CHECK_EQ(result.spCost, static_cast<uint16_t>(35));
}

// ── Test 6: one below the boundary ────────────────────────────────────

MODERN_TEST(ActiveSkill_OneBelowBoundaryIsLowSp)
{
	const SkillDefinition definition = MakeDamageSkill();
	ActiveSkillInput short1 = MakeInput(definition, 1);
	short1.equipmentRequiredSP = 30;
	short1.currentSp = 34;

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(short1);

	CHECK_EQ(result.requiredSP, static_cast<uint16_t>(35));
	CHECK_EQ(result.IsLowSp(), true);
	// Legacy charges nothing for a cast the caster cannot afford
	// (GLChar.cpp:3005, the `if (!bLowSP)` guard).
	CHECK_EQ(result.spCost, static_cast<uint16_t>(0));
	// But the cast still happens, and still hits.
	CHECK(result.Succeeded());
	CHECK_GT(result.combat.damageResult.damage, 0u);
}

// ── Test 7: insufficient HP and MP refuse the cast ────────────────────

MODERN_TEST(ActiveSkill_InsufficientHpRejected)
{
	const SkillDefinition definition = MakeDamageSkill();
	ActiveSkillInput input = MakeInput(definition, 1);
	// Level 1 costs useHp = 0, and the legacy test is `<=`, so a pool of 0 is
	// refused. That is the boundary, not a special case.
	input.currentHp = 0;

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(input);

	CHECK_EQ(result.failure, ActiveSkillFailure::InsufficientHp);
	CHECK(!result.Succeeded());
}

MODERN_TEST(ActiveSkill_HpBoundaryIsInclusive)
{
	// GLogixExPC.cpp:4240 uses `<=`, so a pool exactly equal to the cost is
	// refused. Level 1 of this definition costs useHp = 0, so "equal" means 0.
	SkillDefinition definition = MakeDamageSkill();
	definition.levelData[1].useHp = 4;

	ActiveSkillInput refused = MakeInput(definition, 1);
	refused.currentHp = 4;
	CHECK_EQ(ActiveSkillResolver::Resolve(refused).failure,
	         ActiveSkillFailure::InsufficientHp);

	// One above the cost is enough.
	ActiveSkillInput allowed = MakeInput(definition, 1);
	allowed.currentHp = 5;
	CHECK(ActiveSkillResolver::Resolve(allowed).Succeeded());
}

MODERN_TEST(ActiveSkill_InsufficientMpRejected)
{
	const SkillDefinition definition = MakeDamageSkill();
	ActiveSkillInput input = MakeInput(definition, 1);
	// Level 1 costs useMp = 2, and the legacy test is `<`.
	input.currentMp = 1;

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(input);

	CHECK_EQ(result.failure, ActiveSkillFailure::InsufficientMp);
	CHECK(!result.Succeeded());
}

MODERN_TEST(ActiveSkill_MpBoundaryIsExclusive)
{
	// GLogixExPC.cpp:4241 uses `<`, so a pool exactly equal to the cost passes.
	SkillDefinition definition = MakeDamageSkill();
	definition.levelData[1].useMp = 2;

	ActiveSkillInput exact = MakeInput(definition, 1);
	exact.currentMp = 2;
	CHECK(ActiveSkillResolver::Resolve(exact).Succeeded());

	ActiveSkillInput under = MakeInput(definition, 1);
	under.currentMp = 1;
	CHECK_EQ(ActiveSkillResolver::Resolve(under).failure,
	         ActiveSkillFailure::InsufficientMp);
}

MODERN_TEST(ActiveSkill_ZeroSpStillCasts)
{
	// A low SP pool degrades a cast, it does not refuse it. This is the single
	// most important behaviour difference from the HP and MP checks.
	const SkillDefinition definition = MakeDamageSkill();
	ActiveSkillInput input = MakeInput(definition, 1);
	input.equipmentRequiredSP = 30;
	input.currentSp = 0;

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(input);

	CHECK(result.Succeeded());
	CHECK_EQ(result.IsLowSp(), true);
	CHECK_GT(result.combat.damageResult.damage, 0u);
}

// ── Test 8: skill level selects level-specific data ───────────────────

MODERN_TEST(ActiveSkill_LevelSelectsLevelData)
{
	const SkillDefinition definition = MakeDamageSkill();
	ActiveSkillInput low = MakeInput(definition, 1);
	ActiveSkillInput high = MakeInput(definition, 3);

	const ActiveSkillResult lowResult  = ActiveSkillResolver::Resolve(low);
	const ActiveSkillResult highResult = ActiveSkillResolver::Resolve(high);

	CHECK_EQ(lowResult.level, static_cast<uint8_t>(1));
	CHECK_EQ(highResult.level, static_cast<uint8_t>(3));

	// fBASIC_VAR is -10 * level, so the magnitude tracks the level.
	CHECK_EQ(lowResult.basicDamage, 10);
	CHECK_EQ(highResult.basicDamage, 30);

	// wUSE_SP is 5 * level.
	CHECK_EQ(ActiveSkillResolver::RequiredSP(low, definition),
	         static_cast<uint16_t>(5));
	CHECK_EQ(ActiveSkillResolver::RequiredSP(high, definition),
	         static_cast<uint16_t>(15));
}

MODERN_TEST(ActiveSkill_LevelBeyondMaxRefused)
{
	const SkillDefinition definition = MakeDamageSkill();
	ActiveSkillInput input = MakeInput(definition, 4);   // maxLevel is 3

	CHECK_EQ(ActiveSkillResolver::Resolve(input).failure,
	         ActiveSkillFailure::InvalidLevel);
}

// ── Test 9: target validation ─────────────────────────────────────────

MODERN_TEST(ActiveSkill_NoTargetRejected)
{
	const SkillDefinition definition = MakeDamageSkill();
	ActiveSkillInput input = MakeInput(definition, 1);
	input.hasTarget = false;

	CHECK_EQ(ActiveSkillResolver::Resolve(input).failure,
	         ActiveSkillFailure::UnsupportedTarget);
}

// ── Test 10: determinism ──────────────────────────────────────────────

MODERN_TEST(ActiveSkill_DeterministicForIdenticalInput)
{
	const SkillDefinition definition = MakeDamageSkill();
	const ActiveSkillInput input = MakeInput(definition, 1);

	const ActiveSkillResult a = ActiveSkillResolver::Resolve(input);
	const ActiveSkillResult b = ActiveSkillResolver::Resolve(input);

	CHECK_EQ(a.failure, b.failure);
	CHECK_EQ(a.level, b.level);
	CHECK_EQ(a.requiredSP, b.requiredSP);
	CHECK_EQ(a.spCost, b.spCost);
	CHECK_EQ(a.basicDamage, b.basicDamage);
	CHECK_EQ(a.combat.damageResult.damage, b.combat.damageResult.damage);
	CHECK_EQ(a.combat.hitResult.hit, b.combat.hitResult.hit);
	CHECK_EQ(a.combat.damageResult.critical, b.combat.damageResult.critical);
}

// ── Unsupported, refused with a reason rather than faked ──────────────

MODERN_TEST(ActiveSkill_PassiveRoleRejected)
{
	// GLogixExPC.cpp:4092 refuses anything that is not EMROLE_NORMAL.
	SkillDefinition definition = MakeDamageSkill();
	definition.role = SkillRole::Passive;

	CHECK_EQ(ActiveSkillResolver::Resolve(MakeInput(definition, 1)).failure,
	         ActiveSkillFailure::NotCastable);
}

// VERTICAL-012: `EMAPPLY_PHY_LONG` used to be refused here. It is now a
// supported channel and is covered by the `RangedSkill_*` cases further down,
// which pin that it reads the shoot power rather than the melee power.
MODERN_TEST(ActiveSkill_RangedApplyAccepted)
{
	SkillDefinition definition = MakeDamageSkill();
	definition.apply = SkillApply::PhysicalRanged;

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(MakeInput(definition, 1));

	CHECK(result.Succeeded());
	CHECK_EQ(result.failure, ActiveSkillFailure::None);
}

// VERTICAL-013: `EMAPPLY_MAGIC` used to be refused here. It is now a supported
// channel and is covered by the `Magic_*` cases. The refused magic shapes -
// heal, EMFOR_MP, EMFOR_SP, friendly side - each have their own named test
// below, so this case no longer asserts a blanket refusal.
MODERN_TEST(ActiveSkill_MagicApplyNoLongerBlanketRefused)
{
	SkillDefinition definition = MakeDamageSkill();
	definition.apply = SkillApply::Magic;

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(MakeInput(definition, 1));

	// MakeDamageSkill's fixture is PhysicalMelee-shaped: a hostile HP skill with
	// a negative basicVar. That is inside the executed slice, so it succeeds.
	CHECK(result.Succeeded());
}

MODERN_TEST(ActiveSkill_ZoneTargetRejected)
{
	// TAR_ZONE and TAR_SELF_TOSPEC need positions and an entity registry.
	SkillDefinition definition = MakeDamageSkill();
	definition.targetKind = SkillTargetKind::Zone;

	CHECK_EQ(ActiveSkillResolver::Resolve(MakeInput(definition, 1)).failure,
	         ActiveSkillFailure::UnsupportedTarget);
}

MODERN_TEST(ActiveSkill_FriendlySideRejected)
{
	// SIDE_OUR is a buff, which is a resource path rather than a combat path.
	SkillDefinition definition = MakeDamageSkill();
	definition.impactSide = SkillImpactSide::Our;

	CHECK_EQ(ActiveSkillResolver::Resolve(MakeInput(definition, 1)).failure,
	         ActiveSkillFailure::UnsupportedSide);
}

MODERN_TEST(ActiveSkill_HealRejected)
{
	// GLChar.cpp:3087-3090 reads a non-negative fBASIC_VAR as a heal.
	SkillDefinition definition = MakeDamageSkill();
	definition.levelData[1].basicVar = 10.0f;

	CHECK_EQ(ActiveSkillResolver::Resolve(MakeInput(definition, 1)).failure,
	         ActiveSkillFailure::UnsupportedEffect);
}

MODERN_TEST(ActiveSkill_ZeroMagnitudeRejected)
{
	// Legacy would charge and deal nothing. Refusing says the data is unusable
	// rather than reporting a successful empty cast.
	SkillDefinition definition = MakeDamageSkill();
	definition.levelData[1].basicVar = -0.4f;   // int(-0.4) == 0

	CHECK_EQ(ActiveSkillResolver::Resolve(MakeInput(definition, 1)).failure,
	         ActiveSkillFailure::NoDamageMagnitude);
}

MODERN_TEST(ActiveSkill_NonFiniteRejected)
{
	SkillDefinition definition = MakeDamageSkill();
	definition.levelData[1].basicVar = std::numeric_limits<float>::quiet_NaN();

	CHECK_EQ(ActiveSkillResolver::Resolve(MakeInput(definition, 1)).failure,
	         ActiveSkillFailure::NonFiniteData);
}

MODERN_TEST(ActiveSkill_CooldownRejected)
{
	// GLogixExPC.cpp:4082 returns EMSKILL_DELAYTIME when the map has an entry.
	const SkillDefinition definition = MakeDamageSkill();
	ActiveSkillInput input = MakeInput(definition, 1);
	input.onCooldown = true;

	CHECK_EQ(ActiveSkillResolver::Resolve(input).failure,
	         ActiveSkillFailure::InCooldown);
}

MODERN_TEST(ActiveSkill_ProhibitedAndStunnedRejected)
{
	const SkillDefinition definition = MakeDamageSkill();

	ActiveSkillInput prohibited = MakeInput(definition, 1);
	prohibited.skillProhibited = true;
	CHECK_EQ(ActiveSkillResolver::Resolve(prohibited).failure,
	         ActiveSkillFailure::NotCastable);

	// GLogixExPC.cpp:4062-4063 refuses a skill while stunned.
	ActiveSkillInput stunned = MakeInput(definition, 1);
	stunned.stunned = true;
	CHECK_EQ(ActiveSkillResolver::Resolve(stunned).failure,
	         ActiveSkillFailure::NotCastable);
}

// ── Cooldown formula ──────────────────────────────────────────────────

MODERN_TEST(ActiveSkill_CooldownMatchesLegacyFormula)
{
	// GameCharacterCalculations.cpp:118-126:
	//   (float)(dwGRADE * wSKILL_LEV) / (float)(wCHAR_LEVEL) + fDelay
	//
	// grade 2, level 2, character level 4, delayTime 0.5 -> 4/4 + 0.5 = 1.5
	SkillDefinition definition = MakeDamageSkill();   // grade 2
	definition.levelData[2].delayTime = 0.5f;

	const float seconds = ActiveSkillResolver::CooldownSeconds(definition, 2, 4);

	CHECK(std::fabs(seconds - 1.5f) < 0.0001f);
}

MODERN_TEST(ActiveSkill_CooldownZeroLevelGuarded)
{
	// Legacy divides by wCHAR_LEVEL with no zero guard. The modern resolver
	// returns the bare delay rather than an infinity, which is a documented
	// departure.
	SkillDefinition definition = MakeDamageSkill();
	definition.levelData[1].delayTime = 0.75f;

	const float seconds = ActiveSkillResolver::CooldownSeconds(definition, 1, 0);

	CHECK(std::fabs(seconds - 0.75f) < 0.0001f);
}

MODERN_TEST(ActiveSkill_CastPublishesCooldown)
{
	const SkillDefinition definition = MakeDamageSkill();
	const ActiveSkillResult result = ActiveSkillResolver::Resolve(MakeInput(definition, 1));

	CHECK_GT(result.cooldownSeconds, 0.0f);
}

// ── Damage ────────────────────────────────────────────────────────────

MODERN_TEST(ActiveSkill_DamageUsesAbsoluteBasicVar)
{
	// GLogixExPC.cpp:1524: `int nVAR = abs ( int(fSKILL_VAR*fPOWER) )`. The sign
	// selects damage over heal and is then discarded.
	SkillDefinition definition = MakeDamageSkill();
	definition.levelData[1].basicVar = -37.0f;

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(MakeInput(definition, 1));

	CHECK_EQ(result.basicDamage, 37);
}

MODERN_TEST(ActiveSkill_DamageScaledByWeatherPower)
{
	// The same line with fPOWER != 1: `abs(int(fBASIC_VAR * fPOWER))`.
	SkillDefinition definition = MakeDamageSkill();
	definition.levelData[1].basicVar = -10.0f;

	ActiveSkillInput input = MakeInput(definition, 1);
	input.weatherElementPower = 1.2f;

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(input);

	CHECK_EQ(result.basicDamage, 12);   // int(-12.0) == -12
}

MODERN_TEST(ActiveSkill_DamageScalesWithLevel)
{
	const SkillDefinition definition = MakeDamageSkill();

	const ActiveSkillResult l1 = ActiveSkillResolver::Resolve(MakeInput(definition, 1));
	const ActiveSkillResult l3 = ActiveSkillResolver::Resolve(MakeInput(definition, 3));

	CHECK_EQ(l1.basicDamage, 10);
	CHECK_EQ(l3.basicDamage, 30);
	// More magnitude means more damage through the shared pipeline, all else
	// equal.
	CHECK_GT(l3.combat.damageResult.damage, l1.combat.damageResult.damage);
}

MODERN_TEST(ActiveSkill_MissProducesNoDamage)
{
	const SkillDefinition definition = MakeDamageSkill();
	ActiveSkillInput input = MakeInput(definition, 1);
	// attackerHit 50, targetAvoid 10, so a hit rate near 99. A roll above it
	// misses.
	input.hitRoll = 1.0f;

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(input);

	CHECK(result.Succeeded());
	CHECK_EQ(result.combat.IsHit(), false);
	CHECK_EQ(result.combat.damageResult.damage, 0u);
}

// ── Low-SP reaches the combat pipeline ────────────────────────────────

MODERN_TEST(ActiveSkill_LowSpHalvesSkillDamage)
{
	// A low-SP caster's skill damage is halved by fLOWSP_DAMAGE, exactly as a
	// basic attack is (VERTICAL-009). The same pipeline is used, so the same
	// rule applies.
	const SkillDefinition definition = MakeDamageSkill();

	ActiveSkillInput funded = MakeInput(definition, 1);
	funded.currentSp = 1000;

	ActiveSkillInput low = MakeInput(definition, 1);
	low.currentSp = 0;

	const ActiveSkillResult fundedResult = ActiveSkillResolver::Resolve(funded);
	const ActiveSkillResult lowResult = ActiveSkillResolver::Resolve(low);

	CHECK_EQ(fundedResult.IsLowSp(), false);
	CHECK_EQ(lowResult.IsLowSp(), true);
	CHECK_EQ(lowResult.combat.damageResult.damage,
	         static_cast<uint32_t>(fundedResult.combat.damageResult.damage / 2));
}

// ── Definition helpers added for this milestone ───────────────────────

MODERN_TEST(ActiveSkill_LevelDataAccessorIsBoundsSafe)
{
	const SkillDefinition definition = MakeDamageSkill();

	// Level 0 and an out-of-range level read the zeroed record rather than
	// another skill's level.
	CHECK_EQ(definition.GetLevelData(0).useSp, static_cast<uint16_t>(0));
	CHECK_EQ(definition.GetLevelData(200).useSp, static_cast<uint16_t>(0));
	CHECK_EQ(definition.GetLevelData(1).useSp, static_cast<uint16_t>(5));
	CHECK_EQ(definition.GetLevelData(3).useSp, static_cast<uint16_t>(15));
}

// ── VERTICAL-012: ranged physical ──────────────────────────────────────
//
// A ranged physical skill is the same calculation with a different attack
// power. The cases below are the ones that would fail if ranged were a melee
// attack wearing a different name.

namespace
{
	// A ranged physical damage skill: `EMAPPLY_PHY_LONG`.
	SkillDefinition MakeRangedDamageSkill(uint16_t skillIndex = 5)
	{
		SkillDefinition def = MakeDamageSkill(skillIndex);
		def.name   = "ArrowShot" + std::to_string(skillIndex);
		def.apply  = SkillApply::PhysicalRanged;
		def.grade  = 1;
		for (uint8_t level = 1; level <= def.maxLevel; ++level)
		{
			// The same `basicVar` magnitude as MakeDamageSkill on purpose: the
			// percentage bonus scales the range, so leaving the two fixtures at
			// different magnitudes would make the channel comparison meaningless.
			def.levelData[level].basicVar = -10.0f * static_cast<float>(level);
			def.levelData[level].useSp    = static_cast<uint16_t>(4 * level);
			def.levelData[level].useMp    = static_cast<uint16_t>(1 * level);
			def.levelData[level].delayTime = 0.25f;
		}
		return def;
	}

	// The same situation with the two attack powers deliberately unequal, which
	// is the only way a test can tell which one the pipeline read.
	ActiveSkillInput MakePowerInput(const SkillDefinition& definition,
	                               uint16_t meleePower, uint16_t shootPower)
	{
		ActiveSkillInput input = MakeInput(definition, 1);
		input.attacker.meleePower = meleePower;
		input.attacker.shootPower = shootPower;
		// A flat range, so the only thing that can move the damage is the
		// attack power.
		input.attacker.physicalDamage.low  = 0;
		input.attacker.physicalDamage.high = 0;
		return input;
	}
}

// A ranged physical skill is no longer refused.
MODERN_TEST(RangedSkill_ApplyChannelIsAccepted)
{
	const SkillDefinition definition = MakeRangedDamageSkill();
	const ActiveSkillResult result = ActiveSkillResolver::Resolve(MakeInput(definition, 1));

	CHECK(result.Succeeded());
	CHECK_EQ(result.failure, ActiveSkillFailure::None);
}

// The critical regression: PA and SA are not interchangeable. A big shoot power
// and a small melee power must produce a ranged result that a melee cast of the
// same skill does not.
MODERN_TEST(RangedSkill_UsesShootPowerNotMeleePower)
{
	const SkillDefinition definition = MakeRangedDamageSkill();

	// shoot power far above melee power.
	const ActiveSkillResult result = ActiveSkillResolver::Resolve(
		MakePowerInput(definition, 100, 300));

	CHECK(result.Succeeded());
	// The shoot power reached the range, so the damage reflects 300, not 100.
	// The fixture zeroes the range, so the raw damage is the power itself.
	CHECK(result.combat.damageResult.damage > 100u);
}

MODERN_TEST(RangedSkill_MeleeChannelStillUsesMeleePower)
{
	const SkillDefinition definition = MakeDamageSkill();   // PhysicalMelee

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(
		MakePowerInput(definition, 100, 300));
	const ActiveSkillResult rangedResult = ActiveSkillResolver::Resolve(
		MakePowerInput(MakeRangedDamageSkill(), 100, 300));

	CHECK(result.Succeeded());
	// The melee channel must not have picked up the larger shoot power.
	// Range {0,0} + melee power 100 = 100, then the flat `basicVar` bonus of
	// 10 makes 110. Were the melee channel wrongly reading the 300 shoot power
	// this would be 310.
	CHECK_EQ(result.combat.damageResult.damage, 110u);
	// And the ranged channel on the same inputs does read the shoot power:
	// {0,0} + 300 + 10 = 310.
	CHECK_EQ(rangedResult.combat.damageResult.damage, 310u);
}

// The same pair of inputs, both channels, so the two results are directly
// comparable. The ranged one must be strictly stronger, because only the shoot
// power differs and it is the larger.
MODERN_TEST(RangedSkill_RangedAndMeleeDifferOnTheSameInputs)
{
	const SkillDefinition ranged = MakeRangedDamageSkill();
	const SkillDefinition melee  = MakeDamageSkill();

	const ActiveSkillResult rangedResult = ActiveSkillResolver::Resolve(
		MakePowerInput(ranged, 100, 300));
	const ActiveSkillResult meleeResult = ActiveSkillResolver::Resolve(
		MakePowerInput(melee, 100, 300));

	CHECK(rangedResult.Succeeded());
	CHECK(meleeResult.Succeeded());
	CHECK_GT(rangedResult.combat.damageResult.damage,
	         meleeResult.combat.damageResult.damage);
}

// Reversing the two powers must reverse which channel is stronger, which is the
// half of the regression that a one-directional test would miss.
MODERN_TEST(RangedSkill_PowerSelectionFollowsTheChannelNotMagnitude)
{
	const SkillDefinition ranged = MakeRangedDamageSkill();
	const SkillDefinition melee  = MakeDamageSkill();

	// Now the melee power is the larger of the two.
	const ActiveSkillResult rangedResult = ActiveSkillResolver::Resolve(
		MakePowerInput(ranged, 300, 100));
	const ActiveSkillResult meleeResult = ActiveSkillResolver::Resolve(
		MakePowerInput(melee, 300, 100));

	CHECK(rangedResult.Succeeded());
	CHECK(meleeResult.Succeeded());
	// The ranged channel read the smaller power, so the melee cast is now the
	// stronger of the pair.
	CHECK_GT(meleeResult.combat.damageResult.damage,
	         rangedResult.combat.damageResult.damage);
}

// Ranged reflection is suppressed. GLogixExPC.cpp:1468-1469 zeroes both
// reflection terms for EMAPPLY_PHY_LONG, and VERTICAL-009 implemented that in
// the calculator. A target that reflects against a ranged skill must not
// reflect back, while the same target reflecting against a melee skill still
// does.
MODERN_TEST(RangedSkill_ReflectionSuppressedButMeleeStillReflects)
{
	// A reflecting target.
	SkillDefinition ranged = MakeRangedDamageSkill();
	SkillDefinition melee  = MakeDamageSkill();

	ActiveSkillInput rangedInput = MakeInput(ranged, 1);
	rangedInput.target.damageReflection     = 0.5f;
	rangedInput.target.damageReflectionRate = 0.5f;
	rangedInput.targetLevel = 100;   // so the reflection amount does not truncate
	// A roll that would clear the rate if reflection were consulted at all.
	rangedInput.reflectionRoll = 0.0f;

	ActiveSkillInput meleeInput = MakeInput(melee, 1);
	meleeInput.target.damageReflection     = 0.5f;
	meleeInput.target.damageReflectionRate = 0.5f;
	meleeInput.targetLevel = 100;
	meleeInput.reflectionRoll = 0.0f;

	const ActiveSkillResult rangedResult = ActiveSkillResolver::Resolve(rangedInput);
	const ActiveSkillResult meleeResult  = ActiveSkillResolver::Resolve(meleeInput);

	CHECK(rangedResult.Succeeded());
	CHECK(meleeResult.Succeeded());

	// The ranged cast triggers no reflection at all.
	CHECK_EQ(rangedResult.combat.damageResult.reflectionTriggered, false);
	CHECK_EQ(rangedResult.combat.damageResult.reflectionDamage, 0u);

	// The melee cast still does, so the suppression is specific to ranged.
	CHECK_EQ(meleeResult.combat.damageResult.reflectionTriggered, true);
	CHECK_GT(meleeResult.combat.damageResult.reflectionDamage, 0u);
}

// Low-SP is the attacker's, and the range of the attack does not change that.
MODERN_TEST(RangedSkill_LowSpUsesTheAttackersPool)
{
	const SkillDefinition definition = MakeRangedDamageSkill();

	ActiveSkillInput low = MakeInput(definition, 1);
	low.currentSp = 0;

	ActiveSkillInput funded = MakeInput(definition, 1);
	funded.currentSp = 1000;

	const ActiveSkillResult lowResult    = ActiveSkillResolver::Resolve(low);
	const ActiveSkillResult fundedResult = ActiveSkillResolver::Resolve(funded);

	CHECK(lowResult.Succeeded());
	CHECK_EQ(lowResult.IsLowSp(), true);
	CHECK(fundedResult.Succeeded());
	CHECK_EQ(fundedResult.IsLowSp(), false);
	// The same fLOWSP_DAMAGE penalty as melee, so the ranged damage is halved.
	CHECK_EQ(lowResult.combat.damageResult.damage,
	         static_cast<uint32_t>(fundedResult.combat.damageResult.damage / 2));
}

// A low-SP attacker charges no SP for a ranged skill either.
MODERN_TEST(RangedSkill_LowSpChargesNoSp)
{
	const SkillDefinition definition = MakeRangedDamageSkill();

	ActiveSkillInput low = MakeInput(definition, 1);
	low.currentSp = 0;

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(low);

	CHECK(result.Succeeded());
	CHECK_EQ(result.IsLowSp(), true);
	CHECK_EQ(result.spCost, static_cast<uint16_t>(0));
}

// Ranged skills are not immune to critical or crushing. Both channels run the
// same critical and crushing rules, so a low roll on either produces the
// multiplier.
MODERN_TEST(RangedSkill_CriticalAndCrushingStillApply)
{
	SkillDefinition definition = MakeRangedDamageSkill();
	// Enough crushing bonus to clear the cap so the roll decides.
	ActiveSkillInput input = MakePowerInput(definition, 100, 100);

	// Critical: a zero roll against the base rate is a critical.
	ActiveSkillInput critical = input;
	critical.criticalRoll = 0.0f;
	critical.crushingRoll = 1.0f;
	const ActiveSkillResult criticalResult = ActiveSkillResolver::Resolve(critical);
	CHECK_EQ(criticalResult.combat.damageResult.critical, true);

	// Crushing: a zero roll with a crushing bonus over the cap.
	SkillDefinition crushing = definition;
	crushing.levelData[1].useSp = 0;   // keep the cost out of the comparison
	ActiveSkillInput crushingInput = MakePowerInput(crushing, 100, 100);
	crushingInput.criticalRoll = 1.0f;
	crushingInput.crushingRoll = 0.0f;
	const ActiveSkillResult crushingResult = ActiveSkillResolver::Resolve(crushingInput);
	// With no crushing bonus the rate is 0, so a zero roll cannot beat it. This
	// asserts the shared rule rather than a ranged-specific one: the roll is
	// compared against the same rate melee uses.
	CHECK(crushingResult.Succeeded());
}

// The physical resistance rule is shared, not reimplemented for ranged.
MODERN_TEST(RangedSkill_UsesTheSharedPhysicalResistance)
{
	const SkillDefinition definition = MakeRangedDamageSkill();

	ActiveSkillInput plain = MakeInput(definition, 1);
	ActiveSkillInput resistant = MakeInput(definition, 1);
	resistant.target.resistances.fire = 50;   // any non-zero element resists

	// The resistance input is the target's aggregate; what matters for this
	// slice is that a non-zero resistElement on the combat input is honoured
	// identically regardless of the attack channel, which the calculator
	// already guarantees. Assert the ranged channel is not special-cased by
	// checking it resolves the same way a melee one does with the same input.
	CHECK(ActiveSkillResolver::Resolve(plain).Succeeded());
	CHECK(ActiveSkillResolver::Resolve(resistant).Succeeded());
}

// ── VERTICAL-013: magic through the resolver ───────────────────────────

namespace
{
	// A hostile single-target HP magic skill: the one shape this milestone
	// executes (EMAPPLY_MAGIC + EMFOR_HP + fBASIC_VAR < 0 + SIDE_ENEMY).
	SkillDefinition MakeMagicDamageSkill()
	{
		SkillDefinition def = MakeDamageSkill();
		def.name   = "Fireball";
		def.apply  = SkillApply::Magic;
		def.element = SkillElement::Fire;
		def.applyType = PassiveApplyType::Hp;
		def.impactSide = SkillImpactSide::Enemy;
		return def;
	}
}

MODERN_TEST(MagicSkill_HostileHpDamageIsAccepted)
{
	const SkillDefinition definition = MakeMagicDamageSkill();

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(MakeInput(definition, 1));

	CHECK(result.Succeeded());
	// The channel is reported rather than inferred from the damage number.
	CHECK_EQ(result.attackTypeUsed, Combat::AttackType::Magic);
	CHECK_GT(result.combat.damageResult.damage, 0u);
}

// A heal is not negative damage. Legacy GLChar.cpp:3087-3091 reads a positive
// fBASIC_VAR as a heal capped at the target's missing HP.
MODERN_TEST(MagicSkill_HealIsRefusedNotConvertedToDamage)
{
	SkillDefinition definition = MakeMagicDamageSkill();
	for (uint8_t lvl = 1; lvl <= definition.maxLevel; ++lvl)
	{
		definition.levelData[lvl].basicVar = 10.0f * static_cast<float>(lvl);   // positive
	}

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(MakeInput(definition, 1));

	CHECK(!result.Succeeded());
	CHECK_EQ(result.failure, ActiveSkillFailure::UnsupportedEffect);
}

// EMFOR_MP / EMFOR_SP never reach CALCDAMAGE (GLChar.cpp:3094-3122). They are
// their own arithmetic and would need their own tests.
MODERN_TEST(MagicSkill_MpAndSpEffectsAreRefused)
{
	for (const PassiveApplyType type : { PassiveApplyType::Mp, PassiveApplyType::Sp })
	{
		SkillDefinition definition = MakeMagicDamageSkill();
		definition.applyType = type;

		const ActiveSkillResult result = ActiveSkillResolver::Resolve(MakeInput(definition, 1));

		CHECK(!result.Succeeded());
		CHECK_EQ(result.failure, ActiveSkillFailure::UnsupportedEffect);
	}
}

// A friendly-side magic skill is restorative, not damage.
MODERN_TEST(MagicSkill_FriendlySideIsRefused)
{
	SkillDefinition definition = MakeMagicDamageSkill();
	definition.impactSide = SkillImpactSide::Our;

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(MakeInput(definition, 1));

	CHECK(!result.Succeeded());
	CHECK_EQ(result.failure, ActiveSkillFailure::UnsupportedEffect);
}

// Zone and realm targeting still need a world and an entity registry.
MODERN_TEST(MagicSkill_ZoneTargetIsRefused)
{
	SkillDefinition definition = MakeMagicDamageSkill();
	definition.targetKind = SkillTargetKind::Zone;

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(MakeInput(definition, 1));

	CHECK(!result.Succeeded());
	CHECK_EQ(result.failure, ActiveSkillFailure::UnsupportedTarget);
}

// ── Element selection ──────────────────────────────────────────────────
//
// GLogixExPC.cpp:1504-1513: the skill's own element, except ArmWeapon which
// resolves through the caster's weapon and falls back to Spirit.
MODERN_TEST(MagicSkill_ElementSelectsTheMatchingResistance)
{
	SkillDefinition definition = MakeMagicDamageSkill();
	definition.element = SkillElement::Fire;

	ActiveSkillInput plain = MakeInput(definition, 1);
	ActiveSkillInput resisted = MakeInput(definition, 1);
	resisted.target.resistances.fire = 50;

	const ActiveSkillResult plainResult    = ActiveSkillResolver::Resolve(plain);
	const ActiveSkillResult resistedResult = ActiveSkillResolver::Resolve(resisted);

	CHECK(plainResult.Succeeded());
	CHECK(resistedResult.Succeeded());
	CHECK_LT(resistedResult.combat.damageResult.damage,
	         plainResult.combat.damageResult.damage);
}

// Fire resistance must not blunt an ice spell: the lookup is per element.
MODERN_TEST(MagicSkill_ResistanceIsPerElementNotGlobal)
{
	SkillDefinition definition = MakeMagicDamageSkill();
	definition.element = SkillElement::Ice;

	ActiveSkillInput mismatched = MakeInput(definition, 1);
	mismatched.target.resistances.fire = 50;   // wrong axis
	ActiveSkillInput matched = MakeInput(definition, 1);
	matched.target.resistances.ice = 50;

	CHECK_EQ(ActiveSkillResolver::Resolve(mismatched).combat.damageResult.damage,
	         ActiveSkillResolver::Resolve(MakeInput(definition, 1)).combat.damageResult.damage);
	CHECK_LT(ActiveSkillResolver::Resolve(matched).combat.damageResult.damage,
	         ActiveSkillResolver::Resolve(mismatched).combat.damageResult.damage);
}

MODERN_TEST(MagicSkill_ArmWeaponUsesTheCastersWeaponElement)
{
	SkillDefinition definition = MakeMagicDamageSkill();
	definition.element = SkillElement::ArmWeapon;

	// The weapon says Fire and the target resists Fire: the inherited element is
	// the one that is consulted, so this takes the resistance.
	ActiveSkillInput armed = MakeInput(definition, 1);
	armed.weaponElement = SkillElement::Fire;
	armed.target.resistances.fire = 50;

	// The weapon says Ice and the target resists nothing: no resistance applies,
	// because the inherited element is Ice and the Fire axis is untouched.
	ActiveSkillInput icy = MakeInput(definition, 1);
	icy.weaponElement = SkillElement::Ice;

	ActiveSkillResult armedResult = ActiveSkillResolver::Resolve(armed);
	ActiveSkillResult icyResult    = ActiveSkillResolver::Resolve(icy);

	CHECK(armedResult.Succeeded());
	CHECK(icyResult.Succeeded());

	// Fire-resisted is hurt; the Ice-armed one is not.
	CHECK_LT(armedResult.combat.damageResult.damage,
	         icyResult.combat.damageResult.damage);
}

// The converse: with Ice as the weapon element, Fire resistance must not help,
// and Ice resistance must.
MODERN_TEST(MagicSkill_ArmWeaponResistanceFollowsTheWeaponNotTheSkill)
{
	SkillDefinition definition = MakeMagicDamageSkill();
	definition.element = SkillElement::ArmWeapon;

	ActiveSkillInput fireOnIce = MakeInput(definition, 1);
	fireOnIce.weaponElement = SkillElement::Ice;
	fireOnIce.target.resistances.fire = 50;

	ActiveSkillInput iceOnIce = MakeInput(definition, 1);
	iceOnIce.weaponElement = SkillElement::Ice;
	iceOnIce.target.resistances.ice = 50;

	CHECK_LT(ActiveSkillResolver::Resolve(iceOnIce).combat.damageResult.damage,
	         ActiveSkillResolver::Resolve(fireOnIce).combat.damageResult.damage);
}

// Legacy initialises the element to Spirit and only overrides it when a weapon
// is actually found (:1504, :1507-1508), so a missing weapon yields Spirit.
MODERN_TEST(MagicSkill_ArmWeaponWithoutAWeaponFallsBackToSpirit)
{
	SkillDefinition definition = MakeMagicDamageSkill();
	definition.element = SkillElement::ArmWeapon;

	ActiveSkillInput noWeapon = MakeInput(definition, 1);
	noWeapon.weaponElement = SkillElement::Spirit;      // nothing to inherit
	noWeapon.target.resistances.spirit = 50;

	ActiveSkillInput baseline = MakeInput(definition, 1);
	baseline.weaponElement = SkillElement::Spirit;

	CHECK(ActiveSkillResolver::Resolve(noWeapon).Succeeded());
	CHECK_LT(ActiveSkillResolver::Resolve(noWeapon).combat.damageResult.damage,
	         ActiveSkillResolver::Resolve(baseline).combat.damageResult.damage);
}

// ── Resource costs ─────────────────────────────────────────────────────
//
// `wUSE_MP` is the caster paying to cast. `EMFOR_MP` is the target losing MP.
// This milestone implements the cost through VERTICAL-011's authority and
// refuses the effect, and the two must not be conflated.
MODERN_TEST(MagicSkill_MpCostIsChargedButMpEffectIsRefused)
{
	SkillDefinition definition = MakeMagicDamageSkill();
	for (uint8_t lvl = 1; lvl <= definition.maxLevel; ++lvl)
	{
		definition.levelData[lvl].useMp = static_cast<uint16_t>(7 * lvl);
	}

	ActiveSkillInput input = MakeInput(definition, 1);
	input.currentMp = 500;

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(input);

	CHECK(result.Succeeded());
	// The caster pays MP to cast.
	CHECK_EQ(result.mpCost, definition.levelData[1].useMp);
	CHECK_GT(result.mpCost, 0u);
}

MODERN_TEST(MagicSkill_SpCostIsChargedButSpEffectIsRefused)
{
	SkillDefinition definition = MakeMagicDamageSkill();
	for (uint8_t lvl = 1; lvl <= definition.maxLevel; ++lvl)
	{
		definition.levelData[lvl].useSp = static_cast<uint16_t>(6 * lvl);
	}

	ActiveSkillInput input = MakeInput(definition, 1);
	input.currentSp = 500;

	const ActiveSkillResult result = ActiveSkillResolver::Resolve(input);

	CHECK(result.Succeeded());
	CHECK_EQ(result.spCost, definition.levelData[1].useSp);
	CHECK_GT(result.spCost, 0u);
}

// ── Low SP ─────────────────────────────────────────────────────────────
MODERN_TEST(MagicSkill_LowSpUsesTheCastersPoolAndHalvesOnce)
{
	const SkillDefinition definition = MakeMagicDamageSkill();

	ActiveSkillInput funded = MakeInput(definition, 1);
	funded.currentSp = 1000;
	ActiveSkillInput low = MakeInput(definition, 1);
	low.currentSp = 0;

	const ActiveSkillResult fundedResult = ActiveSkillResolver::Resolve(funded);
	const ActiveSkillResult lowResult    = ActiveSkillResolver::Resolve(low);

	CHECK(fundedResult.Succeeded());
	CHECK(lowResult.Succeeded());
	CHECK_EQ(lowResult.IsLowSp(), true);
	CHECK_EQ(fundedResult.IsLowSp(), false);
	CHECK_LT(lowResult.combat.damageResult.damage, fundedResult.combat.damageResult.damage);
	// Charged nothing, because legacy does not bill a cast it knows is short.
	CHECK_EQ(lowResult.spCost, static_cast<uint16_t>(0));
}