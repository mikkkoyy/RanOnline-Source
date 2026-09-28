// VERTICAL-001: the client's character state, and the server/client relationship.
//
// Headless. The interesting cases are the ones that would fail if the client
// acquired any authority of its own:
//
//   - that everything the client reports is exactly what the server published,
//     with no recomputation anywhere in between;
//   - that a client holding no snapshot reports defined emptiness rather than a
//     plausible wrong number;
//   - that the client's translation unit contains no call into the stat
//     system, so authority cannot creep back in.

#include "TestHarness.h"

#include "character/Character.h"
#include "character/CharacterClassTable.h"
#include "gameplay/ClientCharacterState.h"
#include "gameplay/CharacterSnapshot.h"
#include "character/ServerCharacter.h"
#include "stats/DerivedStats.h"
#include "stats/StatCalculator.h"
#include "types/Result.h"

#include <fstream>
#include <string>

using namespace Modern;
using namespace Modern::Client::Gameplay;

namespace
{
	Stats::ClassConstants StandardClass()
	{
		Stats::ClassConstants cc;
		cc.beginStats.pow = 10; cc.beginStats.str = 20;
		cc.beginStats.spi = 15; cc.beginStats.dex = 25;
		cc.beginStats.intel = 8; cc.beginStats.sta = 12;
		cc.beginAttackPoint = 10; cc.beginDefensePoint = 5;
		cc.beginMeleePower = 3;   cc.beginShootPower = 4;
		cc.attackPointConversion = 1.0f; cc.defensePointConversion = 1.0f;
		cc.meleePowerConversion = 1.0f;   cc.shootPowerConversion = 1.0f;
		cc.hpPerStr = 5.0f; cc.mpPerSpi = 4.0f; cc.spPerSta = 2.0f;
		// A non-zero level-up term, so a level change is observable.
		cc.levelUpStats.dex = 0.5f;
		cc.hitPerDex = 2.0f; cc.avoidPerDex = 1.0f; cc.defensePerDex = 3.0f;
		cc.meleePerPow = 1.0f; cc.meleePerDex = 0.5f;
		cc.shootPerPow = 1.0f; cc.shootPerDex = 0.5f;
		cc.magicPerDex = 1.0f; cc.magicPerSpi = 1.0f; cc.magicPerIntel = 2.0f;
		return cc;
	}

	Server::ServerCharacterDefinition StandardDefinition()
	{
		Server::ServerCharacterDefinition definition;
		definition.id             = CharacterId(7u);
		definition.name           = "Raner";
		definition.characterClass = CharacterClass::Brawler;
		definition.gender        = CharacterGender::Male;
		definition.level         = 1;
		definition.classConstants = StandardClass();
		return definition;
	}
}

// ---------------------------------------------------------------------------
// The shared contract
// ---------------------------------------------------------------------------

MODERN_TEST(Gameplay_SnapshotRejectsWhatCannotExist)
{
	// Built up one field at a time, so each rule is shown to be the one that
	// refuses rather than a neighbour.
	Gameplay::CharacterSnapshot snapshot;
	CHECK(!Gameplay::CharacterSnapshot::IsValid(snapshot));
	CHECK(Gameplay::CharacterSnapshot::Create(snapshot).IsError());

	snapshot.id = CharacterId(1u);            // still no name
	CHECK(!Gameplay::CharacterSnapshot::IsValid(snapshot));

	snapshot.id = CharacterId::MakeInvalid();
	snapshot.name = "Raner";                 // still no class
	CHECK(!Gameplay::CharacterSnapshot::IsValid(snapshot));

	snapshot.id = CharacterId(1u);
	snapshot.characterClass = CharacterClass::Brawler;
	CHECK(Gameplay::CharacterSnapshot::IsValid(snapshot));

	// Level defaults to 1, which is RAN's minimum, so the default is legal.
	snapshot.level = 0;                      // below the range
	CHECK(!Gameplay::CharacterSnapshot::IsValid(snapshot));

	snapshot.level = 1;
	snapshot.experience = -1;                // experience is never negative
	CHECK(!Gameplay::CharacterSnapshot::IsValid(snapshot));

	snapshot.experience = 0;
	CHECK(Gameplay::CharacterSnapshot::IsValid(snapshot));

	// A current pool above the published maximum is refused rather than
	// clamped, so a publisher bug stays visible.
	snapshot.hp.current     = 1;
	snapshot.derived.maxHp  = 100;
	CHECK(Gameplay::CharacterSnapshot::IsValid(snapshot));
	snapshot.hp.current = 101;
	CHECK(!Gameplay::CharacterSnapshot::IsValid(snapshot));
}

MODERN_TEST(Gameplay_SnapshotFractionsAreClamped)
{
	Gameplay::CharacterSnapshot snapshot;
	snapshot.id             = CharacterId(1u);
	snapshot.name           = "Raner";
	snapshot.characterClass = CharacterClass::Brawler;
	snapshot.level          = 1;
	snapshot.derived.maxHp  = 200;
	snapshot.hp.current     = 50;
	CHECK_EQ(snapshot.GetHealthFraction(), 0.25f);

	// A zero maximum is a real state for an unbuilt character, and divides to
	// zero rather than by zero.
	snapshot.derived.maxHp = 0;
	snapshot.hp.current    = 0;
	CHECK_EQ(snapshot.GetHealthFraction(), 0.0f);
}

// ---------------------------------------------------------------------------
// Client state
// ---------------------------------------------------------------------------

MODERN_TEST(Gameplay_ClientStartsWithNothing)
{
	ClientCharacterState state;
	CHECK(!state.HasSnapshot());
	CHECK(!state.GetId().IsValid());
	CHECK(state.GetName().empty());
	CHECK(state.GetClass() == CharacterClass::Unset);
	CHECK_EQ(state.GetMaxHp(), 0u);
	CHECK_EQ(state.GetCurrentHp(), 0u);
	CHECK_EQ(state.GetHealthFraction(), 0.0f);
}

MODERN_TEST(Gameplay_ClientPresentsWhatTheServerPublished)
{
	const Result<Server::ServerCharacter> created =
		Server::ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();
	CHECK(character.SetCurrentHp(37).IsOk());
	CHECK(character.SetCurrentMp(11).IsOk());

	const Result<Gameplay::CharacterSnapshot> snapshot = character.BuildSnapshot();
	CHECK(snapshot.IsOk());
	if (snapshot.IsError())
	{
		return;
	}

	ClientCharacterState state;
	CHECK(state.Apply(snapshot.GetValue()).IsOk());
	CHECK(state.HasSnapshot());

	// Every value is the server's, unchanged.
	CHECK_EQ(state.GetId(), character.GetId());
	CHECK_EQ(state.GetName(), character.GetName());
	CHECK(state.GetClass() == CharacterClass::Brawler);
	CHECK_EQ(state.GetLevel(), character.GetLevel());
	CHECK_EQ(state.GetCurrentHp(), 37u);
	CHECK_EQ(state.GetCurrentMp(), 11u);
	CHECK_EQ(state.GetMaxHp(), character.GetDerivedStats().maxHp);
	CHECK(state.GetDerivedStats() == character.GetDerivedStats());
	CHECK(state.GetTotalStats() == character.GetTotalStats());
}

MODERN_TEST(Gameplay_ClientAdoptsALaterSnapshotWholesale)
{
	// A new snapshot replaces the old one. There is no per-field setter, so
	// there is no way for the client to hold a mixture of two of them.
	const Result<Server::ServerCharacter> created =
		Server::ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();
	character.RestoreResources();

	ClientCharacterState state;
	CHECK(state.Apply(character.BuildSnapshot().GetValue()).IsOk());
	CHECK_EQ(state.GetCurrentHp(), character.GetDerivedStats().maxHp);

	CHECK(character.SetLevel(30).IsOk());
	CHECK(character.SetCurrentHp(5).IsOk());
	CHECK(state.Apply(character.BuildSnapshot().GetValue()).IsOk());
	CHECK_EQ(state.GetLevel(), 30);
	CHECK_EQ(state.GetCurrentHp(), 5u);
	CHECK(state.GetDerivedStats() == character.GetDerivedStats());
}

MODERN_TEST(Gameplay_ClientRejectsAMalformedSnapshot)
{
	ClientCharacterState state;
	// A snapshot with no identity could not have come from a server.
	Gameplay::CharacterSnapshot snapshot;
	CHECK(state.Apply(snapshot).IsError());
	CHECK(!state.HasSnapshot());

	// One that is valid, then one that is not, leaves the valid one in place:
	// a rejected update does not destroy what the client already knew.
	const Result<Server::ServerCharacter> created =
		Server::ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();
	CHECK(state.Apply(character.BuildSnapshot().GetValue()).IsOk());

	Gameplay::CharacterSnapshot broken = character.BuildSnapshot().GetValue();
	broken.hp.current = broken.derived.maxHp + 1000;
	CHECK(state.Apply(broken).IsError());
	CHECK(state.HasSnapshot());
	CHECK(state.GetDerivedStats() == character.GetDerivedStats());
}

MODERN_TEST(Gameplay_ClientClearForgetsEverything)
{
	const Result<Server::ServerCharacter> created =
		Server::ServerCharacter::Create(StandardDefinition());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	ClientCharacterState state;
	CHECK(state.Apply(created.GetValue().BuildSnapshot().GetValue()).IsOk());
	CHECK(state.HasSnapshot());

	state.Clear();
	CHECK(!state.HasSnapshot());
	CHECK(!state.GetId().IsValid());
	CHECK_EQ(state.GetMaxHp(), 0u);
	CHECK(state.GetDerivedStats() == Stats::DerivedStats());
}

// ---------------------------------------------------------------------------
// The authority relationship
// ---------------------------------------------------------------------------

MODERN_TEST(Gameplay_ClientNumbersEqualTheOneStatImplementation)
{
	// Given identical inputs, the server's answer, the client-held answer and a
	// direct call to Modern::Stats::Calculate all agree. This is the assertion
	// that would fail if the client had grown a second implementation.
	Server::ServerCharacterDefinition definition = StandardDefinition();
	definition.level = 42;
	definition.experience = 9999;
	definition.allocatedStats.pow = 7;
	definition.allocatedStats.dex = 19;
	Stats::ItemContribution items;
	items.hp = 31;
	items.hitRatePercent = 12.0f;
	definition.items = items;
	Stats::PassiveContribution passives;
	passives.hp = 17;
	passives.hpRate = 0.25f;
	definition.passives = passives;
	Stats::CodexContribution codex;
	codex.hp = 500;
	definition.codex = codex;

	const Result<Server::ServerCharacter> created = Server::ServerCharacter::Create(definition);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}
	Server::ServerCharacter character = created.GetValue();
	character.RestoreResources();

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
	const Stats::DerivedStats direct = Stats::Calculate(input).GetValue();

	CHECK(character.GetDerivedStats() == direct);

	ClientCharacterState client;
	CHECK(client.Apply(character.BuildSnapshot().GetValue()).IsOk());
	CHECK(client.GetDerivedStats() == direct);
	CHECK_EQ(client.GetMaxHp(), direct.maxHp);
	CHECK_EQ(client.GetCurrentHp(), direct.maxHp);
	CHECK_EQ(client.GetHealthFraction(), 1.0f);
}

MODERN_TEST(Gameplay_ClientSourceContainsNoStatCalculation)
{
	// Authority is enforced by structure, not by convention. If the client's
	// translation unit ever calls Modern::Stats::Calculate, this fails - which
	// is a build-time signal rather than a code review question.
	const char* path = "modern/client/gameplay/ClientCharacterState.cpp";
	std::ifstream file(path);
	if (!file)
	{
		// Running from a different working directory: the check is skipped
		// rather than reported as a pass, and printed so it is not silent.
		std::printf("      (source check skipped, could not open %s)\n", path);
		return;
	}
	const std::string source{ std::istreambuf_iterator<char>(file),
	                         std::istreambuf_iterator<char>() };

	CHECK(source.find("Stats::Calculate") == std::string::npos);
	CHECK(source.find("StatCalculator.h") == std::string::npos);
}

int main()
{
	// Unbuffered so an abort still shows which case was running.
	std::setvbuf(stdout, nullptr, _IONBF, 0);
	std::printf("Modern VERTICAL-001 client gameplay tests\n\n");

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
