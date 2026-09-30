// CORE-001: base core foundation.
//
// Covers the identities, the result convention, the vector, entity ownership
// and the character lifecycle. Links Modern and nothing else: no renderer, no
// socket, no database, no legacy library.

#include "TestHarness.h"

#include "character/Character.h"
#include "entity/Entity.h"
#include "item/ItemDefinition.h"
#include "item/ItemInstance.h"
#include "math/Vector3.h"
#include "types/Ids.h"
#include "types/Result.h"

#include <limits>

using namespace Modern;

namespace
{
	constexpr uint32_t kEntityValue    = 7;
	constexpr uint32_t kCharacterValue = 42;
}

// ---------------------------------------------------------------------------
// Identifiers
// ---------------------------------------------------------------------------

MODERN_TEST(Ids_DefaultConstructedAreInvalid)
{
	CHECK(!EntityId().IsValid());
	CHECK(!CharacterId().IsValid());
	CHECK(!ItemId().IsValid());
	CHECK(!AccountId().IsValid());
	CHECK(!WorldId().IsValid());
}

MODERN_TEST(Ids_ConstructionIsExplicit)
{
	// CharacterId(1) must not compile without the cast; this documents that
	// the value has to be named deliberately at the call site.
	const CharacterId id(static_cast<uint32_t>(kCharacterValue));
	CHECK(id.IsValid());
	CHECK_EQ(id.Get(), kCharacterValue);
}

MODERN_TEST(Ids_InvalidSentinelMatchesDefault)
{
	CHECK_EQ(CharacterId::InvalidValue, std::numeric_limits<uint32_t>::max());
	CHECK(CharacterId::MakeInvalid() == CharacterId());
	CHECK(!CharacterId::MakeInvalid().IsValid());
}

MODERN_TEST(Ids_EqualityAndOrdering)
{
	const CharacterId a(static_cast<uint32_t>(2));
	const CharacterId b(static_cast<uint32_t>(3));

	CHECK(a == CharacterId(static_cast<uint32_t>(2)));
	CHECK(a != b);
	CHECK(a < b);
	CHECK(b > a);
	CHECK(a <= a);
	CHECK(a >= a);
}

// ---------------------------------------------------------------------------
// Result convention
// ---------------------------------------------------------------------------

MODERN_TEST(Result_StatusDefaultsToOk)
{
	const Status status;
	CHECK(status.IsOk());
	CHECK(!status.IsError());
	CHECK_EQ(status.GetCode(), ErrorCode::None);
	CHECK(std::string(Ok().GetMessage()) == "None");
}

MODERN_TEST(Result_EveryCodeHasAName)
{
	CHECK(std::string(ToString(ErrorCode::None)) == "None");
	CHECK(std::string(ToString(ErrorCode::InvalidArgument)) == "InvalidArgument");
	CHECK(std::string(ToString(ErrorCode::NotFound)) == "NotFound");
	CHECK(std::string(ToString(ErrorCode::AlreadyExists)) == "AlreadyExists");
	CHECK(std::string(ToString(ErrorCode::InvalidState)) == "InvalidState");
	CHECK(std::string(ToString(ErrorCode::NotAllowed)) == "NotAllowed");
}

MODERN_TEST(Result_ValueCarriedOnSuccess)
{
	const Result<int> ok(41 + 1);
	CHECK(ok.IsOk());
	CHECK(!ok.IsError());
	CHECK_EQ(ok.GetValue(), 42);
	CHECK_EQ(ok.GetValueOr(0), 42);
}

MODERN_TEST(Result_NoValueOnFailure)
{
	const Status notFound(ErrorCode::NotFound);
	const Result<int> failed(notFound);
	CHECK(failed.IsError());
	CHECK_EQ(failed.GetError(), ErrorCode::NotFound);
	CHECK(failed.TryGetValue() == nullptr);
	CHECK_EQ(failed.GetValueOr(-1), -1);
}

// ---------------------------------------------------------------------------
// Vector3
// ---------------------------------------------------------------------------

MODERN_TEST(Vector3_DefaultIsZeroNotIndeterminate)
{
	const Vector3 v;
	CHECK_EQ(v.x, 0.0f);
	CHECK_EQ(v.y, 0.0f);
	CHECK_EQ(v.z, 0.0f);
	CHECK(v.IsZero());
	CHECK(v == Vector3::Zero);
}

MODERN_TEST(Vector3_NormalizeProducesUnitVector)
{
	const Vector3 unit = Normalize(Vector3(0.0f, 0.0f, 5.0f));
	CHECK(std::fabs(unit.Length() - 1.0f) < 1e-5f);
	CHECK(unit == Vector3::Forward);
}

MODERN_TEST(Vector3_NormalizeRejectsDegenerateInput)
{
	CHECK(Normalize(Vector3::Zero) == Vector3::Zero);

	const float nan = std::numeric_limits<float>::quiet_NaN();
	CHECK(Normalize(Vector3(nan, 0.0f, 0.0f)) == Vector3::Zero);
}

MODERN_TEST(Vector3_IsFiniteRejectsNanAndInfinity)
{
	const float nan = std::numeric_limits<float>::quiet_NaN();
	const float inf = std::numeric_limits<float>::infinity();

	CHECK(Vector3(1.0f, 2.0f, 3.0f).IsFinite());
	CHECK(!Vector3(nan, 0.0f, 0.0f).IsFinite());
	CHECK(!Vector3(0.0f, inf, 0.0f).IsFinite());
}

// ---------------------------------------------------------------------------
// Entity ownership
// ---------------------------------------------------------------------------

MODERN_TEST(Entity_DefaultStateIsUninitialized)
{
	const Character character;
	CHECK_EQ(character.GetState(), EntityState::Uninitialized);
	CHECK(!character.IsActive());
	CHECK(!character.IsAlive());
	CHECK(!character.IsDestroyed());
}

MODERN_TEST(Entity_PositionIsIgnoredBeforeCreation)
{
	Character character;
	character.SetPosition(Vector3(5.0f, 6.0f, 7.0f));
	CHECK(character.GetPosition() == Vector3::Zero);
}

MODERN_TEST(Entity_SpawnedMeansActiveAndAlive)
{
	Character character;
	const Result<Character> created = Character::Create(CharacterId(static_cast<uint32_t>(kCharacterValue)), "Tester");
	CHECK(created.IsOk());

	character = created.GetValue();
	CHECK_EQ(character.GetState(), EntityState::Created);
	CHECK(!character.IsActive());

	CHECK(character.Spawn(Vector3(10.0f, 0.0f, 20.0f), Vector3::Forward).IsOk());
	CHECK_EQ(character.GetState(), EntityState::Spawned);
	CHECK(character.IsActive());
	CHECK(character.IsAlive());
}

// ---------------------------------------------------------------------------
// Character creation
// ---------------------------------------------------------------------------

MODERN_TEST(Character_CreateRejectsInvalidId)
{
	const Result<Character> result = Character::Create(CharacterId(), "Tester");
	CHECK(result.IsError());
	CHECK_EQ(result.GetError(), ErrorCode::InvalidArgument);
}

MODERN_TEST(Character_CreateRejectsEmptyName)
{
	const Result<Character> result =
		Character::Create(CharacterId(static_cast<uint32_t>(kCharacterValue)), "");
	CHECK(result.IsError());
	CHECK_EQ(result.GetError(), ErrorCode::InvalidArgument);
}

MODERN_TEST(Character_NameAtCapacityIsAccepted)
{
	const std::string name(Character::kNameCapacity, 'a');
	const Result<Character> result =
		Character::Create(CharacterId(static_cast<uint32_t>(kCharacterValue)), name);

	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().GetName(), name);
}

MODERN_TEST(Character_NameOverCapacityIsRejected)
{
	const std::string name(Character::kNameCapacity + 1, 'a');
	const Result<Character> result =
		Character::Create(CharacterId(static_cast<uint32_t>(kCharacterValue)), name);

	CHECK(result.IsError());
	CHECK_EQ(result.GetError(), ErrorCode::InvalidArgument);
}

MODERN_TEST(Character_StartsAtLevelOneWithoutClass)
{
	const Result<Character> result =
		Character::Create(CharacterId(static_cast<uint32_t>(kCharacterValue)), "Tester");
	const Character& character = result.GetValue();

	CHECK_EQ(character.GetLevel(), Character::kMinLevel);
	CHECK_EQ(character.GetExperience(), static_cast<int64_t>(0));
	CHECK(!character.HasClass());
	CHECK_EQ(character.GetClass(), CharacterClass::Unset);
}

// ---------------------------------------------------------------------------
// Character lifecycle
// ---------------------------------------------------------------------------

namespace
{
	Character MakeCharacter(const char* name = "Tester")
	{
		const Result<Character> result =
			Character::Create(CharacterId(static_cast<uint32_t>(kCharacterValue)), name);
		return result.GetValue();
	}
}

MODERN_TEST(Character_SpawnStoresNormalisedDirection)
{
	Character character = MakeCharacter();
	CHECK(character.Spawn(Vector3(1.0f, 2.0f, 3.0f), Vector3(0.0f, 0.0f, 4.0f)).IsOk());

	CHECK(character.GetPosition() == Vector3(1.0f, 2.0f, 3.0f));
	CHECK(character.GetDirection() == Vector3::Forward);
}

MODERN_TEST(Character_SpawnTwiceIsInvalidState)
{
	Character character = MakeCharacter();
	CHECK(character.Spawn(Vector3::Zero, Vector3::Forward).IsOk());

	const Status again = character.Spawn(Vector3::Zero, Vector3::Forward);
	CHECK_EQ(again.GetCode(), ErrorCode::InvalidState);
}

MODERN_TEST(Character_SpawnRejectsNonFinitePosition)
{
	Character character = MakeCharacter();

	const float nan = std::numeric_limits<float>::quiet_NaN();
	const Status status = character.Spawn(Vector3(nan, 0.0f, 0.0f), Vector3::Forward);
	CHECK_EQ(status.GetCode(), ErrorCode::InvalidArgument);
	CHECK_EQ(character.GetState(), EntityState::Created);
}

MODERN_TEST(Character_SpawnRejectsZeroLengthDirection)
{
	Character character = MakeCharacter();
	const Status status = character.Spawn(Vector3::Zero, Vector3::Zero);
	CHECK_EQ(status.GetCode(), ErrorCode::InvalidArgument);
	CHECK_EQ(character.GetState(), EntityState::Created);
}

MODERN_TEST(Character_SpawnBeforeCreateIsInvalidState)
{
	Character character;
	CHECK_EQ(character.Spawn(Vector3::Zero, Vector3::Forward).GetCode(), ErrorCode::InvalidState);
}

MODERN_TEST(Character_DespawnRequiresSpawned)
{
	Character character = MakeCharacter();
	CHECK_EQ(character.Despawn().GetCode(), ErrorCode::InvalidState);

	CHECK(character.Spawn(Vector3::Zero, Vector3::Forward).IsOk());
	CHECK(character.Despawn().IsOk());
	CHECK_EQ(character.GetState(), EntityState::Despawned);
	CHECK(!character.IsActive());
	CHECK(character.GetId().IsValid());
}

MODERN_TEST(Character_DespawnTwiceIsInvalidState)
{
	Character character = MakeCharacter();
	CHECK(character.Spawn(Vector3::Zero, Vector3::Forward).IsOk());
	CHECK(character.Despawn().IsOk());
	CHECK_EQ(character.Despawn().GetCode(), ErrorCode::InvalidState);
}

MODERN_TEST(Character_DespawnedCharacterCanRespawn)
{
	Character character = MakeCharacter();
	CHECK(character.Spawn(Vector3(1.0f, 1.0f, 1.0f), Vector3::Forward).IsOk());
	CHECK(character.Despawn().IsOk());

	CHECK(character.Spawn(Vector3(9.0f, 9.0f, 9.0f), Vector3::Up).IsOk());
	CHECK_EQ(character.GetState(), EntityState::Spawned);
	CHECK(character.GetPosition() == Vector3(9.0f, 9.0f, 9.0f));
}

MODERN_TEST(Character_DestroyReleasesIdentity)
{
	Character character = MakeCharacter("Doomed");
	CHECK(character.Spawn(Vector3::Zero, Vector3::Forward).IsOk());
	CHECK(character.Destroy().IsOk());

	CHECK_EQ(character.GetState(), EntityState::Destroyed);
	CHECK(!character.GetId().IsValid());
	CHECK(character.GetName().empty());
	CHECK(character.GetPosition() == Vector3::Zero);
	CHECK_EQ(character.GetLevel(), Character::kMinLevel);
}

MODERN_TEST(Character_DestroyTwiceIsNotAllowed)
{
	Character character = MakeCharacter();
	CHECK(character.Destroy().IsOk());
	CHECK_EQ(character.Destroy().GetCode(), ErrorCode::NotAllowed);
}

MODERN_TEST(Character_DestroyBeforeCreateIsInvalidState)
{
	Character character;
	CHECK_EQ(character.Destroy().GetCode(), ErrorCode::InvalidState);
}

MODERN_TEST(Character_DestroyedCharacterRefusesSpawn)
{
	Character character = MakeCharacter();
	CHECK(character.Destroy().IsOk());
	CHECK_EQ(character.Spawn(Vector3::Zero, Vector3::Forward).GetCode(), ErrorCode::NotAllowed);
}

MODERN_TEST(Character_DestroyedCharacterRefusesSetters)
{
	Character character = MakeCharacter();
	CHECK(character.Destroy().IsOk());

	CHECK_EQ(character.SetName("New").GetCode(), ErrorCode::NotAllowed);
	CHECK_EQ(character.SetClass(CharacterClass::Archer).GetCode(), ErrorCode::NotAllowed);
	CHECK_EQ(character.SetLevel(10).GetCode(), ErrorCode::NotAllowed);
	CHECK_EQ(character.SetExperience(10).GetCode(), ErrorCode::NotAllowed);
	CHECK_EQ(character.AddExperience(10).GetCode(), ErrorCode::NotAllowed);
}

MODERN_TEST(Character_DefaultConstructedRefusesSetters)
{
	Character character;
	CHECK_EQ(character.SetName("New").GetCode(), ErrorCode::InvalidState);
	CHECK_EQ(character.SetLevel(10).GetCode(), ErrorCode::InvalidState);
}

MODERN_TEST(Character_ResetRecoversDestroyedCharacterForReuse)
{
	Character character = MakeCharacter("Pooled");
	CHECK(character.Destroy().IsOk());

	character.Reset();
	CHECK_EQ(character.GetState(), EntityState::Uninitialized);
	CHECK(!character.GetId().IsValid());
	CHECK(character.GetName().empty());

	// A reset character behaves exactly like a default-constructed one, which
	// is what makes it safe to hand back to a pool. It must be created again
	// before it can enter the world.
	CHECK_EQ(character.Spawn(Vector3::Zero, Vector3::Forward).GetCode(), ErrorCode::InvalidState);

	character = MakeCharacter("Reused");
	CHECK(character.Spawn(Vector3::Zero, Vector3::Forward).IsOk());
	CHECK_EQ(character.GetState(), EntityState::Spawned);
}

// ---------------------------------------------------------------------------
// Character state
// ---------------------------------------------------------------------------

MODERN_TEST(Character_LevelIsBounded)
{
	Character character = MakeCharacter();

	CHECK(character.SetLevel(Character::kMaxLevel).IsOk());
	CHECK_EQ(character.GetLevel(), Character::kMaxLevel);

	CHECK_EQ(character.SetLevel(0).GetCode(), ErrorCode::InvalidArgument);
	CHECK_EQ(character.GetLevel(), Character::kMaxLevel);
}

MODERN_TEST(Character_ClassMustBeReal)
{
	Character character = MakeCharacter();

	CHECK_EQ(character.SetClass(CharacterClass::Unset).GetCode(), ErrorCode::InvalidArgument);
	CHECK(!character.HasClass());

	CHECK(character.SetClass(CharacterClass::Shaman).IsOk());
	CHECK(character.HasClass());
	CHECK_EQ(character.GetClass(), CharacterClass::Shaman);
	CHECK(std::string(ToString(character.GetClass())) == "Shaman");
}

MODERN_TEST(Character_ExperienceIsNonNegativeAndSaturating)
{
	Character character = MakeCharacter();

	CHECK(character.AddExperience(100).IsOk());
	CHECK_EQ(character.GetExperience(), static_cast<int64_t>(100));

	CHECK_EQ(character.AddExperience(-1).GetCode(), ErrorCode::InvalidArgument);
	CHECK_EQ(character.GetExperience(), static_cast<int64_t>(100));

	CHECK_EQ(character.SetExperience(-1).GetCode(), ErrorCode::InvalidArgument);
	CHECK_EQ(character.GetExperience(), static_cast<int64_t>(100));

	// Overflow must clamp, not wrap: a wrapped total would read as a small
	// plausible number instead of an obviously broken one.
	CHECK(character.SetExperience(std::numeric_limits<int64_t>::max()).IsOk());
	CHECK(character.AddExperience(1).IsOk());
	CHECK_EQ(character.GetExperience(), std::numeric_limits<int64_t>::max());
}

MODERN_TEST(Character_LifecycleIsDeterministic)
{
	// The same call sequence must produce the same state every time, because
	// the simulation replays in fixed order.
	const auto run = []()
	{
		Character character = MakeCharacter("Deterministic");
		character.SetClass(CharacterClass::Swordsman);
		character.SetLevel(12);
		character.AddExperience(5000);
		character.Spawn(Vector3(100.0f, 0.0f, 250.0f), Vector3::Right);
		character.Despawn();
		character.Destroy();
		character.Reset();
		return character.GetState();
	};

	CHECK_EQ(run(), run());
}

// ---------------------------------------------------------------------------
// Items
// ---------------------------------------------------------------------------

MODERN_TEST(Item_DefinitionValidity)
{
	ItemDefinition definition;
	CHECK(!definition.IsValid());

	definition.id    = ItemId(static_cast<uint32_t>(1000));
	definition.kind  = ItemKind::Weapon;
	definition.name  = "Broadsword";
	definition.maxStack = 1;

	CHECK(definition.IsValid());
	CHECK(!definition.CanStack());

	definition.maxStack = 10;
	CHECK(definition.CanStack());
}

MODERN_TEST(Item_InstanceIdentityIsDefinitionPlusSerial)
{
	ItemInstance a;
	a.definition = ItemId(static_cast<uint32_t>(1000));
	a.serial     = 1;
	a.count      = 3;

	ItemInstance b = a;
	CHECK(a.IsValid());
	CHECK(a.IsStacked());
	CHECK(a.IsFree());
	CHECK(a == b);

	b.serial = 2;
	CHECK(a != b);

	ItemInstance unbound = a;
	unbound.boundTo = ItemId(static_cast<uint32_t>(2000));
	CHECK(!unbound.IsFree());
}

MODERN_TEST(Item_InstanceWithoutDefinitionIsInvalid)
{
	ItemInstance instance;
	instance.count = 1;
	CHECK(!instance.IsValid());

	instance.definition = ItemId(static_cast<uint32_t>(1000));
	instance.count      = 0;
	CHECK(!instance.IsValid());
}

// ---------------------------------------------------------------------------

int main()
{
	std::printf("Modern core tests (CORE-001 + VERTICAL-002 + VERTICAL-003 + VERTICAL-004)\n\n");

	const int failedCases = ModernTests::RunAll();

	if (failedCases == 0)
	{
		std::printf("\nAll %d test cases passed.\n", static_cast<int>(ModernTests::Registry().size()));
		return 0;
	}

	std::printf("\n%d of %d test cases FAILED (%d checks).\n",
		failedCases,
		static_cast<int>(ModernTests::Registry().size()),
		ModernTests::FailureCount());
	return 1;
}
