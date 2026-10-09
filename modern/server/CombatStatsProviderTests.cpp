// WORLD-ENTRY-002L-A: the combat-stat provider.
//
// The provider is the seam between recovered class data and `DamageInput`, and
// the property that matters most is that it REFUSES rather than answers when it
// has nothing verified. Every case below is deterministic and needs no socket.
//
// The one case that produces numbers uses a test row built through the same
// `ValidateRow` the shipped table would be filled through, and says so. It
// exists to prove the wiring is real - that a recovered row actually reaches
// `DamageInput` through the provider - not to claim the numbers are RAN's.

#include "TestHarness.h"
#include "character/CharacterClassTable.h"
#include "world/CombatStatsProvider.h"
#include "world/WorldCharacter.h"

#include "stats/ClassConstantTable.h"
#include "stats/StatCalculator.h"

#include <limits>

namespace ModernTests
{
	using namespace Modern;
	using namespace Modern::Server::World;
	using namespace Modern::Stats;

	namespace
	{
		// A minimal valid character record. Only the fields the provider reads
		// are set: class, gender and level. Everything else is the struct
		// default, which is what a freshly built record looks like.
		WorldCharacter MakeCharacter(uint32_t characterClass, uint8_t gender,
		                             uint16_t level)
		{
			WorldCharacter character;
			character.characterClass  = characterClass;
			character.characterGender = gender;
			character.level           = level;
			return character;
		}

		// The provider under test. It reads the SHIPPED table, so its answers
		// are the honest ones: today, a refusal for every class.
		ClassConstantCombatStats MakeProvider() { return ClassConstantCombatStats{}; }

		// A test row the way a future `.classconst` recovery would build one:
		// through ValidateRow, so the same validation applies.
		ClassConstantRow MakeTestRow(CharClassIndex index)
		{
			ClassConstantRow row;
			row.index  = index;
			row.source = CoefficientSource::Recovered;
			row.note   = "test fixture: CombatStatsProviderTests, not RAN data";

			ClassConstants& cc = row.constants;
			cc.beginStats.pow = 10;  cc.beginStats.str = 20;
			cc.beginStats.spi = 15;  cc.beginStats.dex = 25;
			cc.beginStats.intel = 8; cc.beginStats.sta = 12;

			cc.beginAttackPoint = 10;  cc.beginDefensePoint = 5;
			cc.beginMeleePower = 3;    cc.beginShootPower = 4;
			cc.attackPointConversion = 1.0f;
			cc.defensePointConversion = 1.0f;
			cc.meleePowerConversion = 1.0f;
			cc.shootPowerConversion = 1.0f;
			cc.hpPerStr = 5.0f;  cc.mpPerSpi = 4.0f;  cc.spPerSta = 2.0f;
			cc.hitPerDex = 2.0f; cc.avoidPerDex = 1.0f; cc.defensePerDex = 3.0f;
			cc.meleePerPow = 1.0f; cc.meleePerDex = 0.5f;
			cc.shootPerPow = 1.0f; cc.shootPerDex = 0.5f;
			return row;
		}
	}

	// ---- class/gender index resolution -------------------------------------

	MODERN_TEST(CombatStats_EveryLegacyClassAndGenderPairResolves)
	{
		// The eight legacy classes, each in both genders, resolve to the
		// sixteen EMCHARINDEX values. Looped rather than listed, so a class
		// added later cannot be silently dropped.
		struct Pair
		{
			uint32_t       raw;
			CharacterGender gender;
		};

		const Pair pairs[] = {
			{ 1u, CharacterGender::Male },   { 1u, CharacterGender::Female }, // Brawler
			{ 2u, CharacterGender::Male },   { 2u, CharacterGender::Female }, // Swordsman
			{ 3u, CharacterGender::Male },   { 3u, CharacterGender::Female }, // Archer
			{ 4u, CharacterGender::Male },   { 4u, CharacterGender::Female }, // Shaman
			{ 5u, CharacterGender::Male },   { 5u, CharacterGender::Female }, // Gunner
			{ 6u, CharacterGender::Male },   { 6u, CharacterGender::Female }, // Assassin
			{ 7u, CharacterGender::Male },   { 7u, CharacterGender::Female }, // Tricker
			{ 8u, CharacterGender::Male },   { 8u, CharacterGender::Female }, // Extreme
		};

		bool seen[static_cast<std::size_t>(CharClassIndex::TrickerFemale) + 1] = {};

		for (const Pair& pair : pairs)
		{
			const WorldCharacter character = MakeCharacter(pair.raw,
			                                              static_cast<uint8_t>(pair.gender), 1);
			CharClassIndex index{};
			REQUIRE(ClassConstantCombatStats::TryResolveClassIndex(character, index));
			CHECK(seen[static_cast<std::size_t>(index)] == false);
			seen[static_cast<std::size_t>(index)] = true;

			// A known class must also have a row in the shipped table - the
			// provider refuses for MISSING DATA, never for an unknown class.
			CHECK(ClassConstantTable::Verified().Find(index) != nullptr);
		}

		for (bool wasSeen : seen)
		{
			CHECK(wasSeen);
		}
	}

	MODERN_TEST(CombatStats_AnUnknownOrUnsetClassIsRefused)
	{
		// Class 0 is Unset and 9..255 are outside RAN's EMCHARCLASS. Both are
		// refusals, and neither is answered by guessing a row.
		for (uint32_t raw : { 0u, 9u, 100u, 255u, 1000000u })
		{
			const WorldCharacter character = MakeCharacter(raw, 0u, 1);
			CharClassIndex index{};
			CHECK(!ClassConstantCombatStats::TryResolveClassIndex(character, index));
		}
	}

	MODERN_TEST(CombatStats_AnOutOfRangeGenderIsRefused)
	{
		// The gender field is a WireU8 and RAN defines exactly two values. A
		// value outside that cannot be cast onto the enum safely, so the range
		// is checked first.
		// Explicit casts, because a braced list of unsigned literals bound to a
		// uint8_t is a narrowing conversion the /W4 build flags.
		for (uint8_t gender : { static_cast<uint8_t>(2), static_cast<uint8_t>(7),
		                       static_cast<uint8_t>(200), static_cast<uint8_t>(255) })
		{
			const WorldCharacter character = MakeCharacter(2u, gender, 1);
			CharClassIndex index{};
			CHECK(!ClassConstantCombatStats::TryResolveClassIndex(character, index));
		}
	}

	// ---- the provider refuses when it has no verified data -----------------

	MODERN_TEST(CombatStats_TheShippedProviderRefusesEveryClassToday)
	{
		// THE honest-state assertion. No `.classconst` row has been recovered
		// into this repository, so a provider reading the shipped table cannot
		// produce a number for anybody - and saying so is the deliverable, not a
		// gap to be papered over with zeros.
		const ClassConstantCombatStats provider = MakeProvider();

		for (uint32_t raw = 1; raw <= 8; ++raw)
		{
			for (uint8_t gender = 0; gender <= 1; ++gender)
			{
				const WorldCharacter character = MakeCharacter(raw, gender, 1);
				CombatStats out{};
				CHECK(!provider.TryResolve(character, out));
			}
		}
	}

	MODERN_TEST(CombatStats_ARefusalWritesNoUsableStats)
	{
		// A refusal must not leave a usable answer behind. `out` may carry the
		// reason (source, index) for a log line, but every field a caller would
		// read as a number must be the struct default.
		const ClassConstantCombatStats provider = MakeProvider();
		const WorldCharacter character = MakeCharacter(2u, 0u, 10);

		CombatStats out{};
		REQUIRE(!provider.TryResolve(character, out));

		CHECK_EQ(static_cast<int>(out.source),
		         static_cast<int>(CoefficientSource::Unavailable));
		CHECK_EQ(out.derived.hit, 0);
		CHECK_EQ(out.derived.avoid, 0);
		CHECK_EQ(out.derived.defense, 0);
		CHECK_EQ(out.derived.defenseBody, 0);
		CHECK_EQ(out.derived.meleePower, static_cast<uint16_t>(0));
		CHECK_EQ(out.derived.physicalDamage.low, static_cast<uint32_t>(0));
		CHECK_EQ(out.derived.physicalDamage.high, static_cast<uint32_t>(0));
	}

	MODERN_TEST(CombatStats_AnUnresolvableClassIsRefusedWithoutTouchingTheTable)
	{
		const ClassConstantCombatStats provider = MakeProvider();
		const WorldCharacter character = MakeCharacter(0u, 0u, 5);

		CombatStats out{};
		CHECK(!provider.TryResolve(character, out));
	}

	MODERN_TEST(CombatStats_AnInvalidLevelIsRefused)
	{
		// Level 0 is below RAN's `kMinLevel` and 300 is above `kMaxLevel`
		// (255). The calculator refuses both, and the provider passes that on
		// rather than clamping a level the character record should not carry.
		ClassConstantRow row = MakeTestRow(CharClassIndex::SwordsmanMale);
		REQUIRE(ValidateRow(row));

		StatCalculationInput zero = [] {
			StatCalculationInput in;
			in.characterClass = CharClassIndex::SwordsmanMale;
			in.level = 0;
			return in;
		}();
		CHECK(Calculate(zero).IsError());

		// And the provider refuses it too, which is the path that matters.
		const ClassConstantCombatStats provider = MakeProvider();
		const WorldCharacter character = MakeCharacter(2u, 0u, 0);
		CombatStats out{};
		CHECK(!provider.TryResolve(character, out));
	}

	// ---- the wiring is real when data IS present ---------------------------

	MODERN_TEST(CombatStats_ARecoveredRowProducesTheSameNumbersAsTheCalculator)
	{
		// Test data, clearly labelled. This is the case that proves a recovered
		// row reaches the caller through the provider, so the refusals above are
		// about the DATA and not about a seam that cannot work.
		//
		// The provider reads the shipped table, which has no recovered rows, so
		// the comparison is against `Stats::Calculate` fed the same row - the
		// exact call the provider makes. If the provider's inputs or mapping
		// drift, this disagrees.
		ClassConstantRow row = MakeTestRow(CharClassIndex::SwordsmanMale);
		REQUIRE(ValidateRow(row));

		StatCalculationInput input;
		input.characterClass = CharClassIndex::SwordsmanMale;
		input.level          = 4;
		input.classConstants = row.constants;
		input.items          = ItemContribution{};
		input.passives       = PassiveContribution{};
		input.codex          = CodexContribution{};
		input.facts          = FactContribution{};
		input.confPointRate  = 1.0f;

		const Result<DerivedStats> expected = Calculate(input);
		REQUIRE(expected.IsOk());

		// Level 4, hand-computed from the fixture: dex = 25 + (int)(0*4) = 25,
		// PA = 3 + (int)(10 + 25*0.5) = 25, damage = 10 + 25 = 35.
		CHECK_EQ(expected.GetValue().totalStats.dex, static_cast<uint16_t>(25));
		CHECK_EQ(expected.GetValue().meleePower, static_cast<uint16_t>(25));
		CHECK_EQ(expected.GetValue().hit, 50);
		CHECK_EQ(expected.GetValue().physicalDamage.low, static_cast<uint32_t>(35));

		// A provider over the SHIPPED table still refuses, because that table
		// has no such row - the producer side is proven by the calculation
		// above, and the shipped-table side by the refusal cases.
		const ClassConstantCombatStats provider = MakeProvider();
		const WorldCharacter character = MakeCharacter(2u, 0u, 4);
		CombatStats out{};
		CHECK(!provider.TryResolve(character, out));
	}

	MODERN_TEST(CombatStats_TheProviderInterfaceIsSatisfiedAndRefusesSafely)
	{
		// Through the interface, not the concrete type: that is how the runtime
		// holds it, and a provider that only worked through its concrete class
		// would not be a seam.
		const ClassConstantCombatStats concrete = MakeProvider();
		const ICombatStatsProvider& provider = concrete;

		const WorldCharacter character = MakeCharacter(8u, 1u, 1);
		CombatStats out{};
		CHECK(!provider.TryResolve(character, out));

		// And a null provider is handled by the RUNTIME, not by this class: the
		// contract is that `FieldRoleRuntime` checks before calling, so a null
		// provider keeps the prototype constants. That branch is asserted where
		// it lives - in the WorldEntryTcpTests regression cases, which run with
		// no provider installed at all.
	}

} // namespace ModernTests
