// WORLD-ENTRY-002L-B: the combat-stat provider, with recovered data.
//
// The provider is the seam between recovered class data and `DamageInput`. The
// property that matters most is that it REFUSES rather than answers when it
// has nothing verified - which is now only the unresolved case, because 002L-B
// recovered all sixteen deployed rows. Every case below is deterministic and
// needs no socket.
//
// The cases that produce numbers use the SHIPPED table, so they are the
// deployed RAN values rather than a fixture, and the arithmetic in their
// comments is derived on paper from `class<N>.classconst`.

#include "TestHarness.h"
#include "character/CharacterClassTable.h"
#include "world/CombatStatsProvider.h"
#include "world/WorldCharacter.h"

#include "stats/ClassConstantTable.h"
#include "stats/StatCalculator.h"

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

		// The provider under test. It reads the SHIPPED table, so its answers are
		// the deployed ones.
		ClassConstantCombatStats MakeProvider() { return ClassConstantCombatStats{}; }
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

			// A known class must also have a row in the shipped table - and, now
			// that the rows are recovered, a usable one. The provider refuses for
			// an UNKNOWN class, never for missing data on a known one.
			const ClassConstantRow* row = ClassConstantTable::Verified().Find(index);
			REQUIRE(row != nullptr);
			CHECK_EQ(static_cast<int>(row->source),
			         static_cast<int>(CoefficientSource::Recovered));
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

	MODERN_TEST(CombatStats_TheShippedProviderResolvesEveryLegacyClass)
	{
		// 002L-B recovered all sixteen deployed rows, so the provider answers for
		// every class/gender pair. 002L-A asserted the opposite for every pair;
		// that assertion is what the recovered data changed.
		const ClassConstantCombatStats provider = MakeProvider();

		for (uint32_t raw = 1; raw <= 8; ++raw)
		{
			for (uint8_t gender = 0; gender <= 1; ++gender)
			{
				const WorldCharacter character = MakeCharacter(raw, gender, 1);
				CombatStats out{};
				REQUIRE(provider.TryResolve(character, out));
				CHECK_EQ(static_cast<int>(out.source),
				         static_cast<int>(CoefficientSource::Recovered));
				CHECK(Stats::IsValidClass(out.resolvedIndex));
			}
		}

		// And every one of them came from a recovered row.
		CHECK_EQ(Stats::ClassConstantTable::Verified().RecoveredCount(),
		         Stats::ClassConstantTable::kRowCount);
	}

	MODERN_TEST(CombatStats_AResolvedCharacterCarriesTheDeployedNumbers)
	{
		// Hand arithmetic straight from class8.classconst (ArcherMale) at level 1:
		//
		//   m_sSUMSTATS = (5, 34, 18, 12, 0, 7)
		//   m_wSUM_AP   = (5 + 1.2*0)*0.4  = 2
		//   m_wSUM_DP   = (6 + 0.427*0)*0.57 = 3
		//   m_wPA       = (2 + 0.3*0)*0.6  = 1
		//                 + (int)(5*0.12 + 12*0.08) = 1   -> 2
		//   m_nHIT/AVOID = int(12*0) = 0                 (fHIT_DEX is 0)
		//   m_nDEFENSE_BODY = (int)(3 + 12*0.024) = 3
		//   m_gdDAMAGE_PHYSIC = 2 + VAR_PARAM(2) = (4, 4)
		const ClassConstantCombatStats provider = MakeProvider();
		const WorldCharacter character = MakeCharacter(3u, 0u, 1); // Archer, Male

		CombatStats out{};
		REQUIRE(provider.TryResolve(character, out));
		CHECK_EQ(static_cast<int>(out.resolvedIndex),
		         static_cast<int>(CharClassIndex::ArcherMale));

		CHECK_EQ(out.derived.totalStats.pow, static_cast<uint16_t>(5));
		CHECK_EQ(out.derived.totalStats.str, static_cast<uint16_t>(34));
		CHECK_EQ(out.derived.totalStats.dex, static_cast<uint16_t>(12));
		CHECK_EQ(out.derived.attackPoint, static_cast<uint16_t>(2));
		CHECK_EQ(out.derived.defensePoint, static_cast<uint16_t>(3));
		CHECK_EQ(out.derived.meleePower, static_cast<uint16_t>(2));
		CHECK_EQ(out.derived.hit, 0);
		CHECK_EQ(out.derived.avoid, 0);
		CHECK_EQ(out.derived.defenseBody, 3);
		CHECK_EQ(out.derived.physicalDamage.low, static_cast<uint32_t>(4));
		CHECK_EQ(out.derived.physicalDamage.high, static_cast<uint32_t>(4));
	}

	MODERN_TEST(CombatStats_ARefusalWritesNoUsableStats)
	{
		// A refusal for an UNRESOLVABLE class must not leave a usable answer
		// behind. `out` may carry the reason (source, index) for a log line, but
		// every field a caller would read as a number must be the struct default.
		const ClassConstantCombatStats provider = MakeProvider();

		// Class 0 is Unset and 200 is outside EMCHARCLASS; neither resolves.
		for (uint32_t raw : { 0u, 200u })
		{
			const WorldCharacter character = MakeCharacter(raw, 0u, 1);
			CombatStats out{};
			REQUIRE(!provider.TryResolve(character, out));
			CHECK_EQ(out.derived.hit, 0);
			CHECK_EQ(out.derived.avoid, 0);
			CHECK_EQ(out.derived.defense, 0);
			CHECK_EQ(out.derived.meleePower, static_cast<uint16_t>(0));
			CHECK_EQ(out.derived.physicalDamage.low, static_cast<uint32_t>(0));
		}
	}

	MODERN_TEST(CombatStats_AnInvalidLevelIsRefused)
	{
		// Level 0 is below RAN's `kMinLevel` and 300 is above `kMaxLevel`
		// (255). The calculator refuses both, and the provider passes that on
		// rather than clamping a level the character record should not carry.
		const ClassConstantCombatStats provider = MakeProvider();

		// A valid class, so the refusal can only be the level's.
		// Explicit casts: the field is a WireU16 and 300 is fine, but 0 and 300
		// are narrowed from int literals otherwise.
		for (uint16_t badLevel : { static_cast<uint16_t>(0), static_cast<uint16_t>(300) })
		{
			const WorldCharacter character = MakeCharacter(3u, 0u, badLevel);
			CombatStats out{};
			CHECK(!provider.TryResolve(character, out));
		}

		// And a valid level on the same class still resolves, so the refusals
		// above are about the level and not about the class.
		const WorldCharacter good = MakeCharacter(3u, 0u, 10);
		CombatStats out{};
		CHECK(provider.TryResolve(good, out));
	}

	// ---- the wiring is real ---------------------------------------------------

	MODERN_TEST(CombatStats_ARecoveredRowProducesTheSameNumbersAsTheCalculator)
	{
		// The provider reads the SHIPPED table, which now has recovered rows, so
		// it answers directly. The comparison against `Stats::Calculate` fed the
		// same row proves the provider's inputs and mapping: if either drifted,
		// the two would disagree.
		const ClassConstantCombatStats provider = MakeProvider();
		const WorldCharacter character = MakeCharacter(4u, 0u, 12); // Shaman, Male

		CombatStats out{};
		REQUIRE(provider.TryResolve(character, out));

		Stats::CharClassIndex index{};
		REQUIRE(ClassConstantCombatStats::TryResolveClassIndex(character, index));
		const Stats::ClassConstantRow* row =
		    Stats::ClassConstantTable::Verified().Find(index);
		REQUIRE(row != nullptr);

		Stats::StatCalculationInput input;
		input.characterClass = index;
		input.level          = character.level;
		input.classConstants = row->constants;
		input.items          = Stats::ItemContribution{};
		input.passives       = Stats::PassiveContribution{};
		input.codex          = Stats::CodexContribution{};
		input.facts          = Stats::FactContribution{};
		input.confPointRate  = 1.0f;

		const Modern::Result<Stats::DerivedStats> expected = Stats::Calculate(input);
		REQUIRE(expected.IsOk());
		CHECK(out.derived == expected.GetValue());
	}

	MODERN_TEST(CombatStats_TheProviderInterfaceIsSatisfied)
	{
		// Through the interface, not the concrete type: that is how the runtime
		// holds it, and a provider that only worked through its concrete class
		// would not be a seam.
		const ClassConstantCombatStats concrete = MakeProvider();
		const ICombatStatsProvider& provider = concrete;

		const WorldCharacter character = MakeCharacter(8u, 1u, 1); // Extreme, Female
		CombatStats out{};
		CHECK(provider.TryResolve(character, out));

		// A null provider is handled by the RUNTIME, not by this class: the
		// contract is that `FieldRoleRuntime` checks before calling, so a null
		// provider keeps the prototype constants. That branch is asserted where
		// it lives - in the WorldEntryTcpTests regression cases, which run with
		// no provider installed at all.
	}

} // namespace ModernTests