// VERTICAL-001: the server's authoritative character.
//
// Headless. Links ModernServer (and therefore Modern) and nothing else: no
// socket, no database, no renderer, no legacy library, no client.
//
// The cases that matter are the consistency ones: that the server's numbers
// are exactly what Modern::Stats::Calculate produces for the same inputs, and
// that every mutator which can change a stat input recalculates before it
// returns, so a published snapshot can never disagree with the state behind
// it.

#include "TestHarness.h"

#include "character/Character.h"
#include "character/CharacterClassTable.h"
#include "gameplay/CharacterSnapshot.h"
#include "math/Vector3.h"
#include "character/ServerCharacter.h"
#include "stats/StatCalculator.h"
#include "types/Result.h"

#include <limits>

using namespace Modern;
using namespace Modern::Server;

namespace
{
	// A class row with every field a formula reads, so a test states only what
	// it is about. The numbers are arbitrary: RAN's real coefficients live in
	// `default.charclass`, which is not in this repository, and CORE-002
	// deliberately asserts no shipped value.
	Stats::ClassConstants StandardClass()
	{
		Stats::ClassConstants cc;
		cc.beginStats.pow = 10; cc.beginStats.str = 20;
		cc.beginStats.spi = 15; cc.beginStats.dex = 25;
		cc.beginStats.intel = 8; cc.beginStats.sta = 12;

		cc.beginAttackPoint = 10;
		cc.beginDefensePoint = 5;
		cc.beginMeleePower = 3;
		cc.beginShootPower = 4;

		cc.attackPointConversion  = 1.0f;
		cc.defensePointConversion = 1.0f;
		cc.meleePowerConversion   = 1.0f;
		cc.shootPowerConversion   = 1.0f;

		cc.hpPerStr = 5.0f;
		cc.mpPerSpi = 4.0f;
		cc.spPerSta = 2.0f;

		// A non-zero level-up term, so a level change is actually observable.
		// With every growth coefficient at zero a level change would legitimately
		// change nothing, and the recalculation assertion would be vacuous.
		cc.levelUpStats.dex = 0.5f;

		cc.hitPerDex     = 2.0f;
		cc.avoidPerDex   = 1.0f;
		cc.defensePerDex = 3.0f;

		cc.meleePerPow = 1.0f;  cc.meleePerDex = 0.5f;
		cc.shootPerPow = 1.0f;  cc.shootPerDex = 0.5f;
		cc.magicPerDex = 1.0f;  cc.magicPerSpi = 1.0f;  cc.magicPerIntel = 2.0f;
		return cc;
	}

	ServerCharacterDefinition StandardDefinition()
	{
		ServerCharacterDefinition definition;
		definition.id             = CharacterId(7u);
		definition.name           = "Raner";
		definition.characterClass = CharacterClass::Brawler;
		definition.gender        = CharacterGender::Male;
		definition.level         = 1;
		definition.experience    = 0;
		definition.classConstants = StandardClass();
		definition.confPointRate  = 1.0f;
		return definition;
	}

	// Rebuilds the stat input the server currently holds and calls the one
	// stat implementation, so the comparison is against the formula rather than
	// against a copy of it.
	Stats::DerivedStats RecalculateIndependently(const ServerCharacterDefinition& definition)
	{
		Stats::CharClassIndex classIndex{};
		TryToCharClassIndex(definition.characterClass, definition.gender, classIndex);

		Stats::StatCalculationInput input;
		input.characterClass = classIndex;
		input.level          = definition.level;
		input.classConstants = definition.classConstants;
		input.allocatedStats = definition.allocatedStats;
		input.items          = definition.items;
		input.passives       = definition.passives;
		input.codex          = definition.codex;
		input.confPointRate  = definition.confPointRate;
		return Stats::Calculate(input).GetValue();
	}
}

// ---------------------------------------------------------------------------
// Class table
// ---------------------------------------------------------------------------

MODERN_TEST(Server_ClassTableNamesEveryRanRow)
{
	// The gender really is a character fact: RAN's class table has one row per
	// class *and* gender, and the two halves are different rows.
	Stats::CharClassIndex index{};
	CHECK(TryToCharClassIndex(CharacterClass::Brawler, CharacterGender::Male, index));
	CHECK_EQ(static_cast<uint8_t>(index), 0);
	CHECK(TryToCharClassIndex(CharacterClass::Brawler, CharacterGender::Female, index));
	CHECK_EQ(static_cast<uint8_t>(index), 6);

	CHECK(TryToCharClassIndex(CharacterClass::Archer, CharacterGender::Female, index));
	CHECK_EQ(static_cast<uint8_t>(index), 2);
	CHECK(TryToCharClassIndex(CharacterClass::Archer, CharacterGender::Male, index));
	CHECK_EQ(static_cast<uint8_t>(index), 8);

	CHECK(!TryToCharClassIndex(CharacterClass::Unset, CharacterGender::Male, index));
}

MODERN_TEST(Server_ClassTableRoundTripsEveryIndex)
{
	for (uint8_t value = 0; value < Stats::kClassCount; ++value)
	{
		const auto index  = static_cast<Stats::CharClassIndex>(value);
		const auto asClass = ClassOfCharClassIndex(index);
		CHECK(asClass != CharacterClass::Unset);
		Stats::CharClassIndex again{};
		CHECK(TryToCharClassIndex(asClass, GenderOfCharClassIndex(index), again));
		CHECK_EQ(static_cast<uint8_t>(again), value);
	}
}

// ---------------------------------------------------------------------------
// Creation
// ---------------------------------------------------------------------------

MODERN_TEST(Server_CreateProducesTheStatSystemsAnswer)
{
	const ServerCharacterDefinition definition = StandardDefinition();
	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	CHECK(created.GetValue().GetDerivedStats() == RecalculateIndependently(definition));
}

MODERN_TEST(Server_CreateRejectsAnUnknownClass)
{
	ServerCharacterDefinition definition = StandardDefinition();
	definition.characterClass = CharacterClass::Unset;
	const Result<ServerCharacter> result = ServerCharacter::Create(definition);
	CHECK(result.IsError());
	CHECK_EQ(result.GetError(), ErrorCode::InvalidArgument);
}

MODERN_TEST(Server_CreateRejectsOutOfRangeFields)
{
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.id = CharacterId::MakeInvalid();
		CHECK(ServerCharacter::Create(definition).IsError());
	}
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.name.clear();
		CHECK(ServerCharacter::Create(definition).IsError());
	}
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.name = std::string(Character::kNameCapacity + 1, 'x');
		CHECK(ServerCharacter::Create(definition).IsError());
	}
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.level = 0;
		CHECK(ServerCharacter::Create(definition).IsError());
	}
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.experience = -1;
		CHECK(ServerCharacter::Create(definition).IsError());
	}
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.confPointRate = std::numeric_limits<float>::quiet_NaN();
		CHECK(ServerCharacter::Create(definition).IsError());
	}
	{
		ServerCharacterDefinition definition = StandardDefinition();
		definition.classConstants.hpPerStr = std::numeric_limits<float>::infinity();
		CHECK(ServerCharacter::Create(definition).IsError());
	}
}

// ---------------------------------------------------------------------------
// Recalculation
// ---------------------------------------------------------------------------

MODERN_TEST(Server_LevelChangeRecalculates)
{
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	const Stats::DerivedStats before = character.GetDerivedStats();

	CHECK(character.SetLevel(50).IsOk());
	CHECK(!(character.GetDerivedStats() == before));

	ServerCharacterDefinition expected = StandardDefinition();
	expected.level = 50;
	CHECK(character.GetDerivedStats() == RecalculateIndependently(expected));
}

MODERN_TEST(Server_StatAllocationRecalculates)
{
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	Stats::BaseStats allocated;
	allocated.pow = 5; allocated.str = 300; allocated.dex = 40;
	CHECK(character.SetAllocatedStats(allocated).IsOk());

	ServerCharacterDefinition expected = StandardDefinition();
	expected.allocatedStats = allocated;
	CHECK(character.GetDerivedStats() == RecalculateIndependently(expected));
	// str drives HP, so a large str allocation must raise it.
	CHECK(character.GetDerivedStats().maxHp > 100u);
}

MODERN_TEST(Server_ContributionChangeRecalculates)
{
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	const uint32_t before = character.GetDerivedStats().maxHp;

	Stats::ItemContribution     items;
	Stats::PassiveContribution  passives;
	passives.hp = 250;
	passives.hpRate = 0.5f;
	Stats::CodexContribution    codex;
	codex.hp = 100;
	CHECK(character.SetContributions(items, passives, codex).IsOk());
	CHECK(!(character.GetDerivedStats().maxHp == before));

	ServerCharacterDefinition expected = StandardDefinition();
	expected.items = items;
	expected.passives = passives;
	expected.codex = codex;
	CHECK(character.GetDerivedStats() == RecalculateIndependently(expected));
}

MODERN_TEST(Server_ConfPointRateChangeRecalculates)
{
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	CHECK(character.SetConfPointRate(2.0f).IsOk());
	ServerCharacterDefinition expected = StandardDefinition();
	expected.confPointRate = 2.0f;
	CHECK(character.GetDerivedStats() == RecalculateIndependently(expected));
}

MODERN_TEST(Server_RejectedMutationsLeaveTheCharacterIntact)
{
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	const Stats::DerivedStats before = character.GetDerivedStats();

	CHECK(character.SetLevel(0).IsError());
	CHECK(character.SetExperience(-5).IsError());
	CHECK(character.SetConfPointRate(std::numeric_limits<float>::quiet_NaN()).IsError());
	CHECK(character.GetDerivedStats() == before);
}

// ---------------------------------------------------------------------------
// Resources
// ---------------------------------------------------------------------------

MODERN_TEST(Server_ResourcesClampToTheDerivedMaximum)
{
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	CHECK(character.SetCurrentHp(character.GetDerivedStats().maxHp).IsOk());
	CHECK(character.SetCurrentHp(character.GetDerivedStats().maxHp + 1).IsError());
	CHECK(character.SetCurrentMp(0).IsOk());
	CHECK(character.SetCurrentSp(1).IsOk());
}

MODERN_TEST(Server_RestoreFillsEveryResource)
{
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();

	CHECK(character.SetCurrentHp(1).IsOk());
	character.RestoreResources();

	const Gameplay::CharacterSnapshot snapshot = character.BuildSnapshot().GetValue();
	CHECK_EQ(snapshot.hp.current, snapshot.derived.maxHp);
	CHECK_EQ(snapshot.mp.current, snapshot.derived.maxMp);
	CHECK_EQ(snapshot.sp.current, snapshot.derived.maxSp);
}

MODERN_TEST(Server_DroppingAMaximumClampsTheCurrentValue)
{
	// A stat input that lowers a maximum must not leave a current value above
	// it, or the published snapshot would be unrenderable.
	ServerCharacterDefinition definition = StandardDefinition();
	definition.allocatedStats.str = 300;
	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	character.RestoreResources();
	const uint32_t high = character.GetDerivedStats().maxHp;
	CHECK(high > 100u);

	// Remove the allocation, so the maximum falls.
	CHECK(character.SetAllocatedStats(Stats::BaseStats()).IsOk());
	CHECK(character.GetDerivedStats().maxHp < high);

	const Gameplay::CharacterSnapshot snapshot = character.BuildSnapshot().GetValue();
	CHECK(snapshot.hp.current <= snapshot.derived.maxHp);
}

// ---------------------------------------------------------------------------
// Snapshot
// ---------------------------------------------------------------------------

MODERN_TEST(Server_SnapshotCarriesTheComputedStatistics)
{
	ServerCharacterDefinition definition = StandardDefinition();
	definition.level = 25;
	definition.experience = 4200;
	definition.allocatedStats.dex = 12;
	const Result<ServerCharacter> created = ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	CHECK(character.SetPosition(Vector3(1.0f, 2.0f, 3.0f)).IsOk());
	character.RestoreResources();

	const Result<Gameplay::CharacterSnapshot> snapshot = character.BuildSnapshot();
	CHECK(snapshot.IsOk());
	if (snapshot.IsError())
	{
		return;
	}

	const Gameplay::CharacterSnapshot& s = snapshot.GetValue();
	CHECK_EQ(s.id, definition.id);
	CHECK_EQ(s.name, definition.name);
	CHECK(s.characterClass == CharacterClass::Brawler);
	CHECK_EQ(s.level, 25);
	CHECK_EQ(s.experience, 4200);
	CHECK_EQ(s.allocatedStats.dex, 12);
	CHECK(s.derived == character.GetDerivedStats());
	CHECK(s.totalStats == character.GetTotalStats());
	CHECK(s.position == Vector3(1.0f, 2.0f, 3.0f));
	CHECK_EQ(s.GetHealthFraction(), 1.0f);
}

MODERN_TEST(Server_SnapshotIsValidatedByTheSharedContract)
{
	// The server does not hand out a snapshot the client's own rules would
	// reject, because it builds it through the same validating factory.
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	const Result<Gameplay::CharacterSnapshot> snapshot = character.BuildSnapshot();
	CHECK(snapshot.IsOk());
	if (snapshot.IsError())
	{
		return;
	}
	CHECK(Gameplay::CharacterSnapshot::IsValid(snapshot.GetValue()));
}

MODERN_TEST(Server_RepeatedSnapshotsAreIdentical)
{
	const Result<ServerCharacter> created = ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ServerCharacter character = created.GetValue();
	character.RestoreResources();

	const Gameplay::CharacterSnapshot first = character.BuildSnapshot().GetValue();
	for (int repeat = 0; repeat < 32; ++repeat)
	{
		CHECK(character.BuildSnapshot().GetValue() == first);
	}
}

int main()
{
	// Unbuffered so an abort still shows which case was running.
	std::setvbuf(stdout, nullptr, _IONBF, 0);
	std::printf("Modern VERTICAL-001 server character tests\n\n");

	const int failedCases = ModernTests::RunAll();

	if (failedCases == 0)
	{
		std::printf("\nAll %d test cases passed.\n",
			static_cast<int>(ModernTests::Registry().size()));
		return 0;
	}

	std::printf("\n%d of %d test cases FAILED (%d checks).\n",
		failedCases,
		static_cast<int>(ModernTests::Registry().size()),
		ModernTests::FailureCount());
	return 1;
}