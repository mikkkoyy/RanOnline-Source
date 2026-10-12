// SKILL-018: headless rule tests for the world skill runtime.

#include "TestHarness.h"
#include "world/SkillRuntime.h"
#include "skills/SkillDefinition.h"
#include "skills/SkillDefinitionProvider.h"

using Modern::Server::World::SkillRuntime;
using Modern::SkillDefinition;
using Modern::SkillId;
using Modern::InMemorySkillDefinitions;
using Modern::PassiveApplyType;

namespace
{
	SkillId Id(uint16_t main, uint16_t sub)
	{
		return SkillId{ main, sub };
	}

	SkillDefinition MakeDef(uint16_t main, uint16_t sub, uint8_t maxLevel)
	{
		SkillDefinition def;
		def.id = Id(main, sub);
		def.name = "TestSkill" + std::to_string(main) + "_" + std::to_string(sub);
		def.maxLevel = maxLevel;
		def.applyType = PassiveApplyType::Hp;
		def.levelData[1].basicVar = 100.0f;
		def.levelData[1].applyRange = 50;
		def.targetRange = 30;
		return def;
	}
}

MODERN_TEST(SkillRuntime_StartsInDeterministicState)
{
	SkillRuntime rt(nullptr);
	CHECK_EQ(rt.GetSkillLevel(Id(1, 1)), 0);
	CHECK(!rt.HasSkill(Id(1, 1)));
	CHECK(!rt.IsSkillOnCooldown(Id(1, 1)));
	CHECK_EQ(rt.GetSkillCooldownRemaining(Id(1, 1)), 0.0f);
}

MODERN_TEST(SkillRuntime_NullProvider_RefusesLearn)
{
	SkillRuntime rt(nullptr);
	CHECK(rt.LearnSkill(Id(1, 1)).IsError());
}

MODERN_TEST(SkillRuntime_LearnSucceedsWithKnownDefinition)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeDef(1, 1, 5)).IsOk());
	SkillRuntime rt(&provider);
	CHECK(rt.LearnSkill(Id(1, 1)).IsOk());
	CHECK(rt.HasSkill(Id(1, 1)));
	CHECK_EQ(rt.GetSkillLevel(Id(1, 1)), 1);
}

MODERN_TEST(SkillRuntime_LearnRefusesUnknownDefinition)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeDef(1, 1, 5)).IsOk());
	SkillRuntime rt(&provider);
	CHECK(rt.LearnSkill(Id(9, 9)).IsError());
}

MODERN_TEST(SkillRuntime_LearnRefusesDuplicate)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeDef(1, 1, 5)).IsOk());
	SkillRuntime rt(&provider);
	CHECK(rt.LearnSkill(Id(1, 1)).IsOk());
	CHECK(rt.LearnSkill(Id(1, 1)).IsError());
}

MODERN_TEST(SkillRuntime_SetSkillLevel_RespectsDefinitionMax)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeDef(1, 1, 3)).IsOk());
	SkillRuntime rt(&provider);
	CHECK(rt.LearnSkill(Id(1, 1)).IsOk());
	CHECK(rt.SetSkillLevel(Id(1, 1), 3).IsOk());
	CHECK_EQ(rt.GetSkillLevel(Id(1, 1)), 3);
	CHECK(rt.SetSkillLevel(Id(1, 1), 4).IsError());
	CHECK_EQ(rt.GetSkillLevel(Id(1, 1)), 3);
}

MODERN_TEST(SkillRuntime_UnlearnRemovesSkill)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeDef(1, 1, 5)).IsOk());
	SkillRuntime rt(&provider);
	CHECK(rt.LearnSkill(Id(1, 1)).IsOk());
	CHECK(rt.UnlearnSkill(Id(1, 1)).IsOk());
	CHECK(!rt.HasSkill(Id(1, 1)));
	CHECK_EQ(rt.GetSkillLevel(Id(1, 1)), 0);
}

MODERN_TEST(SkillRuntime_TwoInstances_DoNotLeakState)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeDef(1, 1, 5)).IsOk());
	CHECK(provider.Add(MakeDef(2, 2, 5)).IsOk());
	SkillRuntime a(&provider);
	SkillRuntime b(&provider);
	CHECK(a.LearnSkill(Id(1, 1)).IsOk());
	CHECK(b.LearnSkill(Id(2, 2)).IsOk());
	CHECK(a.HasSkill(Id(1, 1)));
	CHECK(!a.HasSkill(Id(2, 2)));
	CHECK(b.HasSkill(Id(2, 2)));
	CHECK(!b.HasSkill(Id(1, 1)));
}

MODERN_TEST(SkillRuntime_Cooldown_StartQueryAndAdvance)
{
	SkillRuntime rt(nullptr);
	CHECK(!rt.IsSkillOnCooldown(Id(1, 1)));
	rt.StartCooldown(Id(1, 1), 10.0f);
	CHECK(rt.IsSkillOnCooldown(Id(1, 1)));
	CHECK_EQ(rt.GetSkillCooldownRemaining(Id(1, 1)), 10.0f);
	rt.AdvanceCooldowns(4.0f);
	CHECK(rt.IsSkillOnCooldown(Id(1, 1)));
	CHECK_EQ(rt.GetSkillCooldownRemaining(Id(1, 1)), 6.0f);
	rt.AdvanceCooldowns(6.0f);
	CHECK(!rt.IsSkillOnCooldown(Id(1, 1)));
	CHECK_EQ(rt.GetSkillCooldownRemaining(Id(1, 1)), 0.0f);
}

MODERN_TEST(SkillRuntime_Cooldown_ZeroDurationIsNoOp)
{
	SkillRuntime rt(nullptr);
	rt.StartCooldown(Id(1, 1), 0.0f);
	CHECK(!rt.IsSkillOnCooldown(Id(1, 1)));
}

MODERN_TEST(SkillRuntime_Cooldown_InvalidInputDoesNotCorruptState)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeDef(1, 1, 5)).IsOk());
	SkillRuntime rt(&provider);
	CHECK(rt.LearnSkill(Id(1, 1)).IsOk());
	CHECK(rt.SetSkillLevel(Id(1, 1), 0).IsError());
	CHECK_EQ(rt.GetSkillLevel(Id(1, 1)), 1);
	CHECK(rt.SetSkillLevel(Id(1, 1), 99).IsError());
	CHECK_EQ(rt.GetSkillLevel(Id(1, 1)), 1);
	CHECK(rt.LearnSkill(Id(0, 0)).IsError());
	CHECK_EQ(rt.GetSkillLevel(Id(1, 1)), 1);
}
