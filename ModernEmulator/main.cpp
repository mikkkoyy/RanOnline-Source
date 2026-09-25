#include "Character/Character.h"
#include "Character/CharacterBaseData.h"
#include "Progression/ProgressionData.h"
#include "Core/Types.h"
#include "Math/Vector3.h"

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

using namespace Modern;

static std::string ActionName(ActionType a)
{
	switch (a)
	{
		case ActionType::Idle:      return "IDLE";
		case ActionType::Move:      return "MOVE";
		case ActionType::Attack:    return "ATTACK";
		case ActionType::Skill:     return "SKILL";
		case ActionType::Shock:     return "SHOCK";
		case ActionType::PushPull:  return "PUSHPULL";
		case ActionType::SkillMove: return "SKILLMOVE";
		case ActionType::Gathering: return "GATHERING";
		case ActionType::Talk:      return "TALK";
		case ActionType::ConftEnd:  return "CONFT_END";
		case ActionType::Falling:   return "FALLING";
		case ActionType::Die:       return "DIE";
		case ActionType::SkillDash: return "SKILLDASH";
		default:                    return "UNKNOWN";
	}
}

static std::string StateName(uint32_t s)
{
	std::string out;
	if (s & ActState::Run)          out += "RUN ";
	if (s & ActState::ContinueMove) out += "CONTMOVE ";
	if (s & ActState::Peaceful)     out += "PEACE ";
	if (s & ActState::Dead)         out += "DEAD ";
	if (out.empty()) out = "(none)";
	return out;
}

static void PrintRow(const char* name, const Character& c, float t)
{
	const CharacterInfo info = c.Inspect();
	std::cout << std::fixed << std::setprecision(2);
	std::cout << "[t=" << t << "] " << name
		<< " pos(" << info.position.x << ", " << info.position.y << ", " << info.position.z << ")"
		<< " action=" << ActionName(info.action)
		<< " moving=" << (c.IsMoving() ? "true" : "false")
		<< " state=" << StateName(info.actState)
		<< " hp=" << info.hpNow << "/" << info.hpMax
		<< " mp=" << info.mpNow << "/" << info.mpMax
		<< " sp=" << info.spNow << "/" << info.spMax
		<< " lvl=" << info.level << " exp=" << info.expNow << "/" << info.expMax
		<< " alive=" << (info.IsAlive() ? "true" : "false")
		<< "\n";
}

static bool ApproxEQ(float a, float b, float e = 1e-3f)
{
	return std::fabs(a - b) < e;
}

static void RunUnitChecks()
{
	std::cout << "== Unit checks ==\n";

	Character c(EntityId(1), "Test");
	assert(c.GetId() == EntityId(1));
	assert(c.GetName() == "Test");
	assert(c.IsAlive());
	assert(!c.IsDead());

	c.Spawn({1.0f, 2.0f, 3.0f}, {0.0f, 0.0f, 1.0f});
	assert(ApproxEQ(c.GetPosition().x, 1.0f));
	assert(c.GetAction() == ActionType::Idle);
	assert(!c.IsMoving());

	c.MoveTo({10.0f, 2.0f, 3.0f});
	assert(c.IsMoving());
	assert(c.GetAction() == ActionType::Move);

	c.Update(100.0f);
	assert(!c.IsMoving());
	assert(c.GetAction() == ActionType::Idle);
	assert(ApproxEQ(c.GetPosition().x, 10.0f));

	c.Run();
	c.MoveTo({20.0f, 2.0f, 3.0f});
	assert((c.GetActState() & ActState::Run) != 0);
	c.Update(100.0f);
	assert(ApproxEQ(c.GetPosition().x, 20.0f));

	c.Die();
	assert(c.IsDead());
	assert(c.GetAction() == ActionType::Die);
	assert((c.GetActState() & ActState::Dead) != 0);

	c.Revive({0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f});
	assert(c.IsAlive());
	assert(c.GetAction() == ActionType::Idle);

	std::cout << "All unit checks passed.\n\n";
}

static void RunResourceChecks()
{
	std::cout << "== Unit checks: HP/MP/SP ==\n";

	Character c(EntityId(2), "Guard");
	c.SetMaxHP(200);
	c.SetMaxMP(100);
	c.SetMaxSP(50);
	c.Spawn({0, 0, 0}, {0, 0, 1});
	assert(c.GetHP() == 200);
	assert(c.GetMP() == 0);
	assert(c.GetSP() == 0);

	c.SetHP(120);
	c.SetMP(40);
	c.SetSP(20);
	assert(c.GetHP() == 120);
	assert(c.GetMP() == 40);
	assert(c.GetSP() == 20);

	c.DamageHP(30);
	assert(c.GetHP() == 90);
	c.HealHP(50);
	assert(c.GetHP() == 140);
	c.DamageHP(200);
	assert(c.IsDead());
	assert(c.GetHP() == 0);

	c.Revive({5, 5, 5}, {0, 0, 1});
	assert(c.IsAlive());
	assert(c.GetHP() == 200);
	assert(c.GetMP() == 100);
	assert(c.GetSP() == 50);

	c.ConsumeMP(250);
	assert(c.GetMP() == 0);
	c.ConsumeSP(10);
	assert(c.GetSP() == 40);

	std::cout << "All resource checks passed.\n\n";
}

static void RunRecoveryChecks()
{
	std::cout << "== Unit checks: recovery ==\n";

	Character c(EntityId(3), "Recov");
	c.Spawn({0, 0, 0}, {0, 0, 1});
	c.SetMaxHP(100);
	c.SetMaxMP(100);
	c.SetMaxSP(100);
	c.SetHP(40);
	c.SetMP(0);
	c.SetSP(0);
	c.SetRecoveryRates(0.5f, 0.1f, 0.2f);

	c.Update(1.0f);
	assert(c.GetHP() == 90);  // 40 + 100*0.5
	assert(c.GetMP() == 10);  // 0 + 100*0.1
	assert(c.GetSP() == 20);  // 0 + 100*0.2
	assert(c.GetHP() <= c.GetMaxHP());

	std::cout << "All recovery checks passed.\n\n";
}

static void RunLevelChecks()
{
	std::cout << "== Unit checks: EXP & level-up (TestProgressionData) ==\n";

	TestProgressionData testProgression(10);  // max level 10 for faster test
	Character c(EntityId(4), "Seeker");
	c.SetProgressionData(&testProgression);
	assert(c.GetLevel() == 1);
	assert(c.GetExpMax() == 100);  // level 1 -> 2 needs 100

	c.SetMaxHP(100);
	c.SetMaxMP(60);
	c.SetMaxSP(40);
	c.Spawn({0, 0, 0}, {0, 0, 1});
	c.SetHP(10);
	c.SetMP(10);
	c.SetSP(10);

	c.AddExperience(100);
	assert(c.GetLevel() == 2);
	assert(c.GetExp() == 0);
	assert(c.GetExpMax() == 200);  // level 2 -> 3 needs 200
	assert(c.GetHP() == 100);  // full restore on level-up
	assert(c.GetMP() == 60);
	assert(c.GetSP() == 40);

	c.AddExperience(500);
	assert(c.GetLevel() == 4);
	assert(c.GetExp() == 0);
	assert(c.GetExpMax() == 400);  // level 4 -> 5 needs 400

	c.SetMaxLevel(5);
	assert(c.GetMaxLevel() == 5);
	c.AddExperience(1000);
	assert(c.GetLevel() == 5);
	int64_t expAtCap = c.GetExp();
	c.AddExperience(5000);
	assert(c.GetLevel() == 5);
	assert(c.GetExp() == expAtCap + 5000);

	std::cout << "All level checks passed.\n\n";
}

static void RunBaseDataChecks()
{
	std::cout << "== Unit checks: Character Base Data (TestCharacterBaseDataProvider) ==\n";

	TestCharacterBaseDataProvider baseProvider;
	TestProgressionData testProgression(10);

	// Test Swordsman Male (index 1)
	{
		Character c(EntityId(10), "SwordsmanTest");
		c.SetBaseDataProvider(&baseProvider);
		c.SetProgressionData(&testProgression);
		c.SetClass(static_cast<uint32_t>(CharIndex::SwordsmanM), Gender::Male);
		c.SetSchool(0);
		c.Spawn({0, 0, 0}, {0, 0, 1});

		// Verify base data loaded
		assert(c.GetClassId() == static_cast<uint32_t>(CharIndex::SwordsmanM));
		assert(c.GetSchool() == 0);
		assert(c.GetMaxHP() > 0);
		assert(c.GetMaxMP() > 0);
		assert(c.GetMaxSP() > 0);
		assert(c.GetHP() == c.GetMaxHP()); // Spawn should set to max
		assert(c.GetMP() == 0); // MP starts at 0 (no base MP in test data)
		assert(c.GetSP() == 0); // SP starts at 0

		// Check class-specific values from test provider
		const auto* baseData = baseProvider.GetBaseData(static_cast<uint32_t>(CharIndex::SwordsmanM), 0);
		assert(baseData != nullptr);
		assert(baseData->className == "Swordsman");
		assert(baseData->hpStrFactor == 5.0f);
		assert(baseData->baseStats.str == 15);

		// Level up should increase stats and resources
		uint32_t hpBefore = c.GetMaxHP();
		c.AddExperience(100); // level 1 -> 2
		assert(c.GetLevel() == 2);
		assert(c.GetMaxHP() >= hpBefore); // HP should increase with level
		assert(c.GetHP() == c.GetMaxHP()); // Full restore on level up
	}

	// Test Swordsman Female (index 7)
	{
		Character c(EntityId(11), "SwordsmanFTest");
		c.SetBaseDataProvider(&baseProvider);
		c.SetProgressionData(&testProgression);
		c.SetClass(static_cast<uint32_t>(CharIndex::SwordsmanW), Gender::Female);
		c.SetSchool(0);
		c.Spawn({0, 0, 0}, {0, 0, 1});

		assert(c.GetMaxHP() > 0);
		assert(c.GetMaxMP() > 0);
		assert(c.GetMaxSP() > 0);

		// Female swordsman has different base stats
		const auto* baseData = baseProvider.GetBaseData(static_cast<uint32_t>(CharIndex::SwordsmanW), 0);
		assert(baseData != nullptr);
		assert(baseData->baseStats.str == 13); // Different from male
		assert(baseData->hpStrFactor == 4.5f); // Different factor
	}

	// Test Brawler Male (index 0)
	{
		Character c(EntityId(12), "BrawlerTest");
		c.SetBaseDataProvider(&baseProvider);
		c.SetProgressionData(&testProgression);
		c.SetClass(static_cast<uint32_t>(CharIndex::BrawlerM), Gender::Male);
		c.SetSchool(0);
		c.Spawn({0, 0, 0}, {0, 0, 1});

		assert(c.GetMaxHP() > 0);
		assert(c.GetMaxMP() > 0);
		assert(c.GetMaxSP() > 0);

		// Brawler has higher HP factor
		const auto* baseData = baseProvider.GetBaseData(static_cast<uint32_t>(CharIndex::BrawlerM), 0);
		assert(baseData != nullptr);
		assert(baseData->hpStrFactor == 6.0f);
		assert(baseData->baseStats.str == 18);
	}

	// Test school/class combination lookup
	{
		assert(baseProvider.HasClass(static_cast<uint32_t>(CharIndex::SwordsmanM), 0));
		assert(baseProvider.HasClass(static_cast<uint32_t>(CharIndex::SwordsmanW), 0));
		assert(baseProvider.HasClass(static_cast<uint32_t>(CharIndex::BrawlerM), 0));
	}

	std::cout << "All base data checks passed.\n\n";
}

static void RunRealDataTest()
{
	std::cout << "== Real RAN data loading test ==\n";

	// Try to load real RAN progression data from text export.
	// The text export would be produced by the legacy pack loader from exptable_max.bin.
	// Expected format: one int64 per line, level-indexed (index = level).
	RANProgressionData ranProgression;
	const char* expTablePath = "../../Modern/Data/exptable_max.txt";
	if (ranProgression.LoadFromTextFile(expTablePath))
	{
		std::cout << "Loaded RAN progression from: " << expTablePath << "\n";
		std::cout << "  Levels: " << ranProgression.GetLevelCount()
			<< ", Max level: " << ranProgression.GetMaxLevel() << "\n";

		// Verify several known levels (index 1..5 should be >0)
		bool ok = true;
		for (uint16_t lvl = 1; lvl < 6 && lvl < ranProgression.GetMaxLevel(); ++lvl)
		{
			int64_t need = ranProgression.GetRequiredExperience(lvl);
			if (need <= 0)
			{
				std::cout << "  WARNING: level " << lvl << " has zero/negative need\n";
				ok = false;
			}
			else
			{
				std::cout << "  Level " << lvl << " -> " << lvl + 1 << " needs " << need << " EXP\n";
			}
		}
		if (ok && ranProgression.GetMaxLevel() >= 255)
		{
			std::cout << "  Real RAN data loading: PASS\n";
		}
		else
		{
			std::cout << "  Real RAN data loading: INCOMPLETE (table may be partial)\n";
		}

		// Compatibility check: use with Character
		Character c(EntityId(99), "RanTest");
		c.SetProgressionData(&ranProgression);
		c.SetMaxHP(100); c.SetMaxMP(50); c.SetMaxSP(30);
		c.Spawn({0,0,0}, {0,0,1});
		c.SetLevel(1);
		int64_t need1 = ranProgression.GetRequiredExperience(1);
		c.AddExperience(need1);
		assert(c.GetLevel() == 2);
		assert(c.GetExp() == 0);
		std::cout << "  Character level-up with real data: PASS\n";
	}
	else
	{
		std::cout << "Real progression file not found at: " << expTablePath << "\n";
		std::cout << "  (Generate from legacy exptable_max.bin via legacy pack loader)\n";
		std::cout << "  Real RAN data loading: SKIPPED\n";
	}
	std::cout << "\n";
}

static void RunScenario()
{
	std::cout << "== Scenario: spawn, walk, run, death, revive ==\n";

	TestProgressionData progression(10);
	TestCharacterBaseDataProvider baseProvider;

	Character hero(EntityId(1), "Hero");
	hero.SetBaseDataProvider(&baseProvider);
	hero.SetProgressionData(&progression);
	hero.SetClass(static_cast<uint32_t>(CharIndex::SwordsmanM), Gender::Male);
	hero.SetSchool(0);
	hero.Spawn({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});

	Character mob(EntityId(2), "ForestMob");
	mob.SetBaseDataProvider(&baseProvider);
	mob.SetProgressionData(&progression);
	mob.SetClass(static_cast<uint32_t>(CharIndex::BrawlerM), Gender::Female);
	mob.SetSchool(0);
	mob.Spawn({30.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f});

	float t = 0.0f;
	const float dt = 0.5f;

	PrintRow("Hero", hero, t);
	PrintRow("Mob ", mob, t);

	hero.Walk();
	hero.MoveTo(mob.GetPosition());
	std::cout << "\n[Hero walks toward Mob]\n";

	for (int i = 0; i < 12; ++i)
	{
		t += dt;
		hero.Update(dt);
		PrintRow("Hero", hero, t);
		if (!hero.IsMoving())
		{
			std::cout << "Hero arrived.\n";
			break;
		}
	}

	hero.Run();
	hero.MoveTo({60.0f, 0.0f, 0.0f});
	std::cout << "\n[Hero runs away]\n";
	t += dt;
	hero.Update(dt);
	PrintRow("Hero", hero, t);

	std::cout << "\n[Hero is struck and dies]\n";
	hero.DamageHP(1000);
	t += dt;
	hero.Update(dt);
	PrintRow("Hero", hero, t);
	assert(hero.IsDead());
	assert(hero.GetHP() == 0);

	std::cout << "\n[Hero is revived at origin]\n";
	hero.Revive({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
	t += dt;
	hero.Update(dt);
	PrintRow("Hero", hero, t);
	assert(hero.IsAlive());
	assert(hero.GetHP() == hero.GetMaxHP());

	std::cout << "\nInspect:\n";
	const CharacterInfo info = hero.Inspect();
	std::cout << "id=" << info.id.value << " name=" << info.name
		<< " class=0x" << std::hex << info.classId << std::dec
		<< " level=" << info.level
		<< " exp=" << info.expNow << "/" << info.expMax
		<< " school=" << info.school << "\n";

	std::cout << "\n== Scenario complete ==\n";
}

int main()
{
	std::cout << "Progression source: TEST DATA (deterministic)\n";
	std::cout << "Base data source: TEST DATA (deterministic)\n\n";

	RunUnitChecks();
	RunResourceChecks();
	RunRecoveryChecks();
	RunBaseDataChecks();
	RunLevelChecks();
	RunRealDataTest();
	RunScenario();

	std::cout << "ALL SCENARIOS PASSED\n";
	return 0;
}