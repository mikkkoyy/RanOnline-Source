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

MODERN_TEST(ActiveSkill_RangedApplyRejected)
{
	// EMAPPLY_PHY_LONG is VERTICAL-012.
	SkillDefinition definition = MakeDamageSkill();
	definition.apply = SkillApply::PhysicalRanged;

	CHECK_EQ(ActiveSkillResolver::Resolve(MakeInput(definition, 1)).failure,
	         ActiveSkillFailure::UnsupportedApply);
}

MODERN_TEST(ActiveSkill_MagicApplyRejected)
{
	// EMAPPLY_MAGIC needs the elemental pipeline of VERTICAL-013.
	SkillDefinition definition = MakeDamageSkill();
	definition.apply = SkillApply::Magic;

	CHECK_EQ(ActiveSkillResolver::Resolve(MakeInput(definition, 1)).failure,
	         ActiveSkillFailure::UnsupportedApply);
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
