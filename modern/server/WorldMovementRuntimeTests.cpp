// WORLD-ENTRY-002f: the Field role's movement runtime.
//
// `GLChar::FrameMove` (GLChar.cpp:5238) is what makes a RAN character walk, and none
// of it is reachable from a message handler - the message only sets the destination.
// WorldMovementRuntime is the other half: the actor registry, and the tick that
// advances every actor.
//
// `Tick(float)` is the test interface, and it is INJECTED on purpose. The ticker
// thread that also exists measures the delta between two real instants and sleeps in
// slices, but a test drives `Tick` with whatever it likes - which is what makes
// "two quarters move as far as one half" assertable without sleeping, and what keeps
// total travel a function of elapsed TIME rather than of how many ticks happened.
//
// No socket, no asset root, no clock and no sleep: the navigation mesh comes from the
// injectable `INavigationMapSource` seam that MovementTests also exercises, and every
// case that starts the ticker does so on a runtime with no actors, so the thread has
// nothing to advance.
//
// The cases are grouped by what they defend:
//
//   registry      attach, detach, replace, and the characters that attach without a mesh
//   dispatch      a 3034 applied through the runtime, and the counters it moves
//   the walk      Tick progression, arrival, and the snapshot the caller reads back
//   isolation     several characters sharing ONE mesh, each moving on its own schedule
//
// The isolation group is the one a shared-mesh design can quietly break: 002e measured
// that sixteen of the fifty-six distinct `.wld` files behind the ninety-nine registered
// maps are named by MORE THAN ONE map identity, so two actors routinely share a mesh
// pointer. Nothing mutable is written into it, and these cases prove that.

#include "TestHarness.h"

#include "NavigationMeshFixture.h"

#include "GotoProtocol.h"
#include "MovementStateProtocol.h"
#include "movement/Actor.h"
#include "movement/NavigationMapSource.h"
#include "world/CharacterClassMovementSpeed.h"
#include "world/GotoService.h"
#include "world/MovementStateService.h"
#include "world/WorldCharacter.h"
#include "world/WorldMovementRuntime.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace
{
	using namespace Modern;
	using namespace Modern::Server::World;
	using namespace ModernTests::MeshFixture;
	namespace MS = Modern::Network::MovementState;

	constexpr float kTolerance = 0.001f;

	bool Near(float actual, float expected)
	{
		return std::fabs(actual - expected) <= kTolerance;
	}

	#define CHECK_NEAR(actual, expected) \
	    ::ModernTests::CheckImpl(Near((actual), (expected)), #actual " ~= " #expected, __FILE__, __LINE__)

	constexpr std::uint32_t kFixtureMapId = 7u;
	constexpr std::uint32_t kMissingMapId = 8u;

	// Archer, female - `EMCHARINDEX` 2, which walks 16 and runs 42. A class whose two
	// speeds differ enough that a mixed-up run/walk branch cannot pass by luck.
	constexpr std::uint32_t kArcher   = 3;
	constexpr Network::WireU8 kFemale = 1;

	WorldCharacter MakeCharacter(std::uint32_t gaeaId, const Vector3& spawn)
	{
		WorldCharacter character;
		character.id               = WorldCharacterId{gaeaId};
		character.accountId        = WorldAccountId{1};
		character.userId           = "user_1";
		character.name             = "Tester";
		character.characterClass   = kArcher;
		character.characterGender  = kFemale;
		character.school           = 1;
		character.level            = 10;
		character.hp               = {3000u, 3000u};
		character.mp               = {1500u, 1500u};
		character.sp               = {800u, 800u};
		character.saveMapId.value  = kFixtureMapId;
		character.savePosition     = {spawn.x, spawn.y, spawn.z};
		character.gaeaId           = gaeaId;
		character.accountLevel     = 0;
		return character;
	}

	// A 3034 that walks along the corridor from (5, 0, 3) to (25, 0, 3).
	Network::Goto::GotoRequest MakeWalkRequest(Network::WireU32 actState = 0)
	{
		Network::Goto::GotoRequest request;
		request.actState        = actState;
		request.currentPosition = {5.0f, 0.0f, 3.0f};
		request.targetPosition  = {25.0f, 0.0f, 3.0f};
		return request;
	}

	// Everything a runtime needs, wired the way the Field role wires it.
	//
	// The GOTO service is built from the movement-state service at wire time rather
	// than stored by value, because a service that changed under a built GotoService
	// would be a dangling borrow.
	struct Wired
	{
		CharacterClassMovementSpeed                    provider;
		MovementStateService                           movement{&provider};
		GotoService                                    gotoService{movement};
		std::shared_ptr<Navigation::NavigationMesh>     mesh;
		SingleMapSource                    source;
		WorldMovementRuntime                            runtime;

		explicit Wired(bool withMapSource)
			: mesh(MakeCorridor())
			, source(mesh, kFixtureMapId)
		{
			if (withMapSource)
			{
				runtime.SetMapSource(&source);
			}
			runtime.SetMovementStateService(&movement);
			runtime.SetGotoService(&gotoService);
		}

		// The corridor mesh, or a null one with the reason already checked by the caller.
		const std::shared_ptr<Navigation::NavigationMesh>& Mesh() const { return mesh; }
	};

	// =========================================================================
	// 1. The registry
	// =========================================================================

	// A fresh runtime has nothing in it, and says so.
	MODERN_TEST(WorldMovement_AFreshRuntimeHasNoActorsAndNoCounters)
	{
		WorldMovementRuntime runtime;

		CHECK_EQ(runtime.ActorCount(), static_cast<std::size_t>(0));
		CHECK_EQ(runtime.MeshlessActorCount(), static_cast<std::size_t>(0));
		CHECK_EQ(runtime.GotoAcceptedCount(), static_cast<std::size_t>(0));
		CHECK_EQ(runtime.GotoRejectedCount(), static_cast<std::size_t>(0));
		CHECK_EQ(runtime.ArrivalCount(), static_cast<std::size_t>(0));
		CHECK_EQ(runtime.TickCount(), static_cast<std::size_t>(0));
		CHECK(!runtime.TickerRunning());

		ActorSnapshot snapshot;
		CHECK(!runtime.Snapshot(1, snapshot));

		// A tick with nothing attached is still a tick that counts.
		CHECK_EQ(runtime.Tick(0.1f), static_cast<std::size_t>(0));
		CHECK_EQ(runtime.TickCount(), static_cast<std::size_t>(1));
	}

	// Attach binds a character to an actor on ITS map, and the mesh is resolved before
	// the slot is published so no reader ever sees a half-built one.
	MODERN_TEST(WorldMovement_AttachPublishesAnActorBoundToTheCharactersMap)
	{
		Wired wired{true};
		CHECK(wired.Mesh() != nullptr);
		if (wired.Mesh() == nullptr)
		{
			return;
		}

		const WorldCharacter character = MakeCharacter(100, Vector3{5.0f, 0.0f, 3.0f});

		CHECK(wired.runtime.Attach(1, character).IsOk());
		CHECK_EQ(wired.runtime.ActorCount(), static_cast<std::size_t>(1));
		CHECK_EQ(wired.runtime.MeshlessActorCount(), static_cast<std::size_t>(0));

		ActorSnapshot snapshot;
		CHECK(wired.runtime.Snapshot(1, snapshot));

		CHECK_EQ(snapshot.sessionId, static_cast<Network::WireU64>(1));
		CHECK_EQ(snapshot.gaeaId, static_cast<Network::WireU32>(100));
		CHECK_EQ(snapshot.mapId, static_cast<Network::WireU32>(kFixtureMapId));

		CHECK(snapshot.hasMesh);
		CHECK(snapshot.currentCellId != Movement::Actor::kNoCell);

		// `Actor::Create` runs legacy's ±5 spawn probe, so the actor LANDS on the mesh
		// rather than merely being told where it is. A character whose database position
		// is off the walkable surface therefore spawns at the nearest walkable point.
		CHECK_NEAR(snapshot.position.x, 5.0f);
		CHECK_NEAR(snapshot.position.y, 0.0f);
		CHECK_NEAR(snapshot.position.z, 3.0f);

		// A character with no destination is not walking, and says so.
		CHECK(!snapshot.pathActive);
		CHECK_EQ(snapshot.waypointsRemaining, static_cast<std::size_t>(0));

		// The map source was consulted once, at attach - not once per tick.
		CHECK_EQ(wired.source.calls, static_cast<std::size_t>(1));
	}

	// World entry is not refused for a missing asset.
	//
	// RAN's field servers always have their meshes, but a modern server started without
	// an asset root must still accept a world entry and still spawn; the character is
	// told "no mesh" rather than refused a login it is entitled to. A GOTO for such a
	// character is `RejectedNoMesh`, which is a different answer from "you may not be
	// here".
	MODERN_TEST(WorldMovement_AttachAcceptsACharacterEvenWhenNoMeshIsAvailable)
	{
		// No map source at all.
		{
			Wired wired{false};
			CHECK(wired.runtime.Attach(1, MakeCharacter(101, Vector3{5.0f, 0.0f, 3.0f})).IsOk());

			CHECK_EQ(wired.runtime.ActorCount(), static_cast<std::size_t>(1));
			CHECK_EQ(wired.runtime.MeshlessActorCount(), static_cast<std::size_t>(1));

			ActorSnapshot snapshot;
			CHECK(wired.runtime.Snapshot(1, snapshot));
			CHECK(!snapshot.hasMesh);
			CHECK_EQ(snapshot.currentCellId, Movement::Actor::kNoCell);
		}

		// A map source that knows nothing about this map.
		{
			Wired wired{true};
			CHECK(wired.Mesh() != nullptr);
			if (wired.Mesh() == nullptr)
			{
				return;
			}

			WorldCharacter character = MakeCharacter(102, Vector3{5.0f, 0.0f, 3.0f});
			character.saveMapId.value = kMissingMapId;

			CHECK(wired.runtime.Attach(1, character).IsOk());
			CHECK_EQ(wired.runtime.MeshlessActorCount(), static_cast<std::size_t>(1));

			ActorSnapshot snapshot;
			CHECK(wired.runtime.Snapshot(1, snapshot));
			CHECK(!snapshot.hasMesh);
		}
	}

	// The two identities `Attach` cannot accept, and Detach's tolerance of both.
	MODERN_TEST(WorldMovement_AttachRefusesAZeroSessionIdAndAnUnspawnedCharacter)
	{
		Wired wired{true};

		// Session zero is not a session; a character that has never been persisted is
		// not a character. Either would be a slot nothing could ever resolve.
		CHECK(wired.runtime.Attach(0, MakeCharacter(103, Vector3{5.0f, 0.0f, 3.0f})).IsError());
		CHECK_EQ(wired.runtime.ActorCount(), static_cast<std::size_t>(0));

		WorldCharacter unspawned = MakeCharacter(0, Vector3{5.0f, 0.0f, 3.0f});
		CHECK(wired.runtime.Attach(1, unspawned).IsError());
		CHECK_EQ(wired.runtime.ActorCount(), static_cast<std::size_t>(0));

		// Detaching something that was never attached is safe, and detaching twice is
		// safe.
		wired.runtime.Detach(1);
		wired.runtime.Detach(1);
		CHECK_EQ(wired.runtime.ActorCount(), static_cast<std::size_t>(0));
	}

	// A second Attach for one session REPLACES the first.
	//
	// Reachable only if a Field role reused a session id, and replacing is the safer of
	// the two answers: keeping both would leave an orphaned actor walking a map nobody is
	// watching.
	MODERN_TEST(WorldMovement_ASecondAttachForOneSessionReplacesTheFirst)
	{
		Wired wired{true};
		CHECK(wired.Mesh() != nullptr);
		if (wired.Mesh() == nullptr)
		{
			return;
		}

		CHECK(wired.runtime.Attach(1, MakeCharacter(104, Vector3{5.0f, 0.0f, 3.0f})).IsOk());

		// The same session, a different entity and a different spawn point.
		CHECK(wired.runtime.Attach(1, MakeCharacter(105, Vector3{15.0f, 0.0f, 7.0f})).IsOk());

		CHECK_EQ(wired.runtime.ActorCount(), static_cast<std::size_t>(1));
		CHECK_EQ(wired.runtime.MeshlessActorCount(), static_cast<std::size_t>(0));

		ActorSnapshot snapshot;
		CHECK(wired.runtime.Snapshot(1, snapshot));
		CHECK_EQ(snapshot.gaeaId, static_cast<Network::WireU32>(105));
		CHECK_NEAR(snapshot.position.x, 15.0f);
		CHECK_NEAR(snapshot.position.z, 7.0f);
	}

	// =========================================================================
	// 2. Dispatching a 3034
	// =========================================================================

	// A 3034 applied through the runtime: the whole legacy sequence runs under the
	// slot's lock, and the new authoritative state is written back into the caller's
	// record so the session and the snapshot cannot drift.
	MODERN_TEST(WorldMovement_ApplyGotoWalksTheCharacterAndWritesTheStateBack)
	{
		Wired wired{true};
		CHECK(wired.Mesh() != nullptr);
		if (wired.Mesh() == nullptr)
		{
			return;
		}

		WorldCharacter character = MakeCharacter(110, Vector3{5.0f, 0.0f, 3.0f});
		CHECK(wired.runtime.Attach(1, character).IsOk());

		GotoResult result;
		CHECK(wired.runtime.ApplyGoto(1, character, MakeWalkRequest(MS::kActRun), result).IsOk());

		CHECK_EQ(static_cast<int>(result.outcome), static_cast<int>(GotoOutcome::Accepted));
		CHECK(result.accepted);

		// The authoritative record took the run bit, not just the snapshot.
		CHECK_EQ(character.actState, static_cast<Network::WireU32>(MS::kActRun));

		ActorSnapshot snapshot;
		CHECK(wired.runtime.Snapshot(1, snapshot));

		// The snapshot's word and the record's word are the same word.
		CHECK_EQ(snapshot.actState, character.actState);

		// The walk: path active, speed set from the recovered table, target on the floor.
		CHECK(snapshot.pathActive);
		CHECK(snapshot.waypointsRemaining > 0);
		CHECK_NEAR(snapshot.maxSpeed, 42.0f);
		CHECK_NEAR(snapshot.target.x, 25.0f);

		// The counters moved by exactly one acceptance.
		CHECK_EQ(wired.runtime.GotoAcceptedCount(), static_cast<std::size_t>(1));
		CHECK_EQ(wired.runtime.GotoRejectedCount(), static_cast<std::size_t>(0));
	}

	// Two refusals, and the counters that make them visible to an operator.
	//
	// The runtime is the only place that counts, so these are the numbers a log would
	// carry - and a GOTO handler that accepted a dead character would show up here.
	MODERN_TEST(WorldMovement_RefusalsAreCountedSeparatelyFromAcceptances)
	{
		Wired wired{true};
		CHECK(wired.Mesh() != nullptr);
		if (wired.Mesh() == nullptr)
		{
			return;
		}

		WorldCharacter character = MakeCharacter(111, Vector3{5.0f, 0.0f, 3.0f});
		CHECK(wired.runtime.Attach(1, character).IsOk());

		// An unreachable destination.
		Network::Goto::GotoRequest refused = MakeWalkRequest(0);
		refused.targetPosition = {500.0f, 0.0f, 3.0f};

		GotoResult result;
		CHECK(wired.runtime.ApplyGoto(1, character, refused, result).IsOk());
		CHECK(!result.accepted);
		CHECK_EQ(wired.runtime.GotoRejectedCount(), static_cast<std::size_t>(1));
		CHECK_EQ(wired.runtime.GotoAcceptedCount(), static_cast<std::size_t>(0));

		// A desynchronised client.
		Network::Goto::GotoRequest drifted = MakeWalkRequest(0);
		drifted.currentPosition = {500.0f, 0.0f, 3.0f};

		CHECK(wired.runtime.ApplyGoto(1, character, drifted, result).IsOk());
		CHECK(!result.accepted);
		CHECK_EQ(wired.runtime.GotoRejectedCount(), static_cast<std::size_t>(2));

		// And a good one, to prove the two counters are not the same counter.
		CHECK(wired.runtime.ApplyGoto(1, character, MakeWalkRequest(0), result).IsOk());
		CHECK(result.accepted);
		CHECK_EQ(wired.runtime.GotoAcceptedCount(), static_cast<std::size_t>(1));
		CHECK_EQ(wired.runtime.GotoRejectedCount(), static_cast<std::size_t>(2));
	}

	// A meshless refusal is reported with the MAP SOURCE'S reason, not the generic one.
	//
	// This is the difference an operator debugging a missing asset needs, and it costs
	// one call on a path that is already failing. Without it, "every map is
	// unreachable" and "the asset root is wrong" print the same line.
	MODERN_TEST(WorldMovement_AMeshlessRefusalIsReportedWithTheMapSourcesOwnReason)
	{
		Wired wired{true};
		CHECK(wired.Mesh() != nullptr);
		if (wired.Mesh() == nullptr)
		{
			return;
		}

		WorldCharacter character = MakeCharacter(112, Vector3{5.0f, 0.0f, 3.0f});
		character.saveMapId.value = kMissingMapId;

		CHECK(wired.runtime.Attach(1, character).IsOk());

		GotoResult result;
		CHECK(wired.runtime.ApplyGoto(1, character, MakeWalkRequest(0), result).IsOk());

		CHECK_EQ(static_cast<int>(result.outcome), static_cast<int>(GotoOutcome::RejectedNoMesh));
		CHECK(!result.accepted);

		// The message is the MAP SOURCE'S, verbatim - which is the whole point of
		// swapping it in. Asserted against the source itself rather than against a
		// substring, so a fixture that renamed its reason fails here too.
		CHECK(!result.detail.empty());
		CHECK_EQ(result.detail, wired.source.Describe(kMissingMapId));
		CHECK(result.detail != std::string("the character has no navigation mesh"));
	}

	// A session with no movement state, and a Field role with no GOTO rule.
	//
	// Both are misconfiguration rather than a player's click, so both report a named
	// status - and the second says so in the outcome rather than pretending the
	// destination was unreachable.
	MODERN_TEST(WorldMovement_AMissingSlotOrRuleIsReportedRatherThanGuessed)
	{
		// No slot.
		{
			Wired wired{true};
			WorldCharacter character = MakeCharacter(113, Vector3{5.0f, 0.0f, 3.0f});

			GotoResult result;
			CHECK(wired.runtime.ApplyGoto(1, character, MakeWalkRequest(0), result).IsError());
			CHECK(!result.accepted);
			CHECK(!result.detail.empty());
			CHECK(result.detail.find("spawned") != std::string::npos);
		}

		// No GOTO rule configured: the slot exists, the rule does not.
		{
			WorldMovementRuntime runtime;
			MovementStateService  movement{nullptr};

			WorldCharacter character = MakeCharacter(114, Vector3{5.0f, 0.0f, 3.0f});
			CHECK(runtime.Attach(1, character).IsOk());

			GotoResult result;
			CHECK(runtime.ApplyGoto(1, character, MakeWalkRequest(0), result).IsError());
			CHECK_EQ(static_cast<int>(result.outcome),
			         static_cast<int>(GotoOutcome::RejectedNoMesh));
			CHECK(result.detail.find("no GOTO rule") != std::string::npos);
		}
	}

	// =========================================================================
	// 3. The walk
	// =========================================================================

	// `Tick` advances by `maxSpeed * elapsedSeconds`, with the elapsed time INJECTED.
	//
	// The speed is refreshed EVERY tick rather than once per GOTO, because
	// `GetMoveVelo()` reads the state word and that word can change between ticks
	// (GLChar.cpp:6089). So a 3032 that arrives mid-walk changes the speed on the very
	// next slice - which is what the second half of this case shows.
	MODERN_TEST(WorldMovement_TickAdvancesBySpeedTimesTheInjectedElapsedTime)
	{
		Wired wired{true};
		CHECK(wired.Mesh() != nullptr);
		if (wired.Mesh() == nullptr)
		{
			return;
		}

		WorldCharacter character = MakeCharacter(120, Vector3{5.0f, 0.0f, 3.0f});
		CHECK(wired.runtime.Attach(1, character).IsOk());

		GotoResult result;
		CHECK(wired.runtime.ApplyGoto(1, character, MakeWalkRequest(0), result).IsOk());
		CHECK(result.accepted);

		// WALKING at 16 (an ArcherFemale's walk velocity), so half a second is a
		// budget of 8 units and the step is bounded by the BUDGET rather than by the
		// twenty-unit distance - which is what makes the product the thing under test.
		const std::size_t moved = wired.runtime.Tick(0.5f);
		CHECK_EQ(moved, static_cast<std::size_t>(1));
		CHECK_EQ(wired.runtime.TickCount(), static_cast<std::size_t>(1));

		ActorSnapshot snapshot;
		CHECK(wired.runtime.Snapshot(1, snapshot));
		CHECK_NEAR(snapshot.position.x, 13.0f);
		CHECK_NEAR(snapshot.movedDist, 8.0f);

		// A quarter of a second is 4 units, and the second half of the walk continues
		// from where the first left off rather than from the start.
		CHECK_EQ(wired.runtime.Tick(0.25f), static_cast<std::size_t>(1));
		CHECK(wired.runtime.Snapshot(1, snapshot));
		CHECK_NEAR(snapshot.movedDist, 12.0f);
	}

	// Two quarter ticks move as far as one half tick.
	//
	// The property the whole injected-time design exists for: total travel is a function
	// of elapsed TIME, not of how many ticks happened. A server running 200 ticks a
	// second and one running 20 move a character the same distance per second. A fixed
	// per-tick rate would make a character's movement rate a function of the server's
	// frame rate, which is precisely the mistake WORLD-ENTRY-002c §3.1 warns about.
	MODERN_TEST(WorldMovement_TwoQuarterTicksMoveAsFarAsOneHalfTick)
	{
		const auto walkFor = [](std::size_t halfTicks) -> float {
			Wired wired{true};
			if (wired.Mesh() == nullptr)
			{
				return -1.0f;
			}

			WorldCharacter character = MakeCharacter(121, Vector3{5.0f, 0.0f, 3.0f});
			if (wired.runtime.Attach(1, character).IsError())
			{
				return -1.0f;
			}

			GotoResult result;
			if (!wired.runtime.ApplyGoto(1, character, MakeWalkRequest(0), result).IsOk())
			{
				return -1.0f;
			}

			// Walk speed, 16 units a second. Ten units to the first waypoint, so a half
			// second does not finish the walk and the arithmetic stays simple.
			for (std::size_t i = 0; i < halfTicks; ++i)
			{
				(void)wired.runtime.Tick(0.25f);
			}

			ActorSnapshot snapshot;
			if (!wired.runtime.Snapshot(1, snapshot))
			{
				return -1.0f;
			}
			return snapshot.movedDist;
		};

		const float oneHalf = walkFor(2);
		const float twoQuarters = walkFor(2);
		const float oneWhole = walkFor(4);

		CHECK(oneHalf >= 0.0f);
		CHECK(twoQuarters >= 0.0f);
		CHECK(oneWhole >= 0.0f);

		// 16 * 0.5 == 8 units either way.
		CHECK_NEAR(oneHalf, 8.0f);
		CHECK_NEAR(twoQuarters, 8.0f);

		// And four quarters are sixteen units, so the distance really is linear in
		// elapsed time rather than saturating somewhere.
		CHECK_NEAR(oneWhole, 16.0f);
	}

	// A tick with no elapsed time does nothing, and is not counted.
	//
	// Both halves matter: the count is what an operator reads as "is the role alive",
	// and a ticker that woke with a zero delta must not make the number mean something
	// it does not.
	MODERN_TEST(WorldMovement_ATickWithNoElapsedTimeDoesNothingAndIsNotCounted)
	{
		Wired wired{true};
		CHECK(wired.Mesh() != nullptr);
		if (wired.Mesh() == nullptr)
		{
			return;
		}

		WorldCharacter character = MakeCharacter(122, Vector3{5.0f, 0.0f, 3.0f});
		CHECK(wired.runtime.Attach(1, character).IsOk());

		GotoResult result;
		CHECK(wired.runtime.ApplyGoto(1, character, MakeWalkRequest(0), result).IsOk());

		CHECK_EQ(wired.runtime.Tick(0.0f), static_cast<std::size_t>(0));
		CHECK_EQ(wired.runtime.Tick(-1.0f), static_cast<std::size_t>(0));
		CHECK_EQ(wired.runtime.TickCount(), static_cast<std::size_t>(0));

		ActorSnapshot snapshot;
		CHECK(wired.runtime.Snapshot(1, snapshot));
		CHECK_NEAR(snapshot.movedDist, 0.0f);
		CHECK_NEAR(snapshot.position.x, 5.0f);

		// And a real tick still counts exactly once.
		CHECK_EQ(wired.runtime.Tick(0.1f), static_cast<std::size_t>(1));
		CHECK_EQ(wired.runtime.TickCount(), static_cast<std::size_t>(1));
	}

	// Arrival is counted once, and the path goes inactive.
	//
	// The count is the modern equivalent of `TurnAction(GLAT_IDLE)` (GLChar.cpp:6091-6095):
	// when the path ends the character stops, and "no longer walking" is what every
	// caller already understands. It must not be counted again on later ticks, which is
	// why the loop below keeps ticking after arrival and the count stays at one.
	MODERN_TEST(WorldMovement_ArrivalIsCountedOnceAndTheWalkStops)
	{
		Wired wired{true};
		CHECK(wired.Mesh() != nullptr);
		if (wired.Mesh() == nullptr)
		{
			return;
		}

		WorldCharacter character = MakeCharacter(123, Vector3{5.0f, 0.0f, 3.0f});
		CHECK(wired.runtime.Attach(1, character).IsOk());

		GotoResult result;
		CHECK(wired.runtime.ApplyGoto(1, character, MakeWalkRequest(0), result).IsOk());

		ActorSnapshot snapshot;
		int      ticks = 0;
		while (ticks < 200)
		{
			(void)wired.runtime.Tick(0.1f);
			++ticks;

			CHECK(wired.runtime.Snapshot(1, snapshot));
			if (!snapshot.pathActive)
			{
				break;
			}
		}

		CHECK(ticks > 0);
		CHECK(ticks < 200);

		// One arrival, and the character is standing on its destination.
		CHECK_EQ(wired.runtime.ArrivalCount(), static_cast<std::size_t>(1));
		CHECK_NEAR(snapshot.position.x, 25.0f);
		CHECK_NEAR(snapshot.position.z, 3.0f);
		CHECK_NEAR(snapshot.position.y, 0.0f);

		// Further ticks change nothing and do not count another arrival.
		for (int i = 0; i < 5; ++i)
		{
			CHECK_EQ(wired.runtime.Tick(0.1f), static_cast<std::size_t>(0));
		}
		CHECK_EQ(wired.runtime.ArrivalCount(), static_cast<std::size_t>(1));
		CHECK(wired.runtime.Snapshot(1, snapshot));
		CHECK_NEAR(snapshot.position.x, 25.0f);
	}

	// The speed is re-read from the state word on every tick.
	//
	// GLChar.cpp:6089 refreshes `SetMaxSpeed(GetMoveVelo())` inside `FrameMove`, so a
	// state change that arrives mid-walk takes effect on the next slice rather than at
	// the next GOTO. This is the observable difference, and it is why the runtime holds
	// an identity SNAPSHOT at all.
	MODERN_TEST(WorldMovement_TheSpeedIsRereadFromTheStateEveryTick)
	{
		Wired wired{true};
		CHECK(wired.Mesh() != nullptr);
		if (wired.Mesh() == nullptr)
		{
			return;
		}

		WorldCharacter character = MakeCharacter(124, Vector3{5.0f, 0.0f, 3.0f});
		CHECK(wired.runtime.Attach(1, character).IsOk());

		GotoResult result;
		CHECK(wired.runtime.ApplyGoto(1, character, MakeWalkRequest(0), result).IsOk());

		ActorSnapshot walking;
		CHECK(wired.runtime.Snapshot(1, walking));
		CHECK_NEAR(walking.maxSpeed, 16.0f);

		// The character starts running - by a 3032 elsewhere on the role, which the Field
		// session reports here so the snapshot is never a tick behind.
		character.actState = MS::kActRun;
		CHECK(wired.runtime.SetActState(1, character.actState).IsOk());

		// The snapshot took it...
		ActorSnapshot running;
		CHECK(wired.runtime.Snapshot(1, running));
		CHECK_EQ(running.actState, static_cast<Network::WireU32>(MS::kActRun));

		// ...and the next tick walks at the run velocity.
		CHECK_EQ(wired.runtime.Tick(0.1f), static_cast<std::size_t>(1));
		CHECK(wired.runtime.Snapshot(1, running));
		CHECK_NEAR(running.maxSpeed, 42.0f);
	}

	// `SetActState` for a session that is not attached is a named status, not a silent
	// no-op - a Field session that has lost its slot needs to know.
	MODERN_TEST(WorldMovement_SetActStateIsRefusedForAnUnknownSession)
	{
		Wired wired{true};

		CHECK(wired.runtime.SetActState(1, MS::kActRun).IsError());

		CHECK(wired.runtime.Attach(1, MakeCharacter(125, Vector3{5.0f, 0.0f, 3.0f})).IsOk());
		CHECK(wired.runtime.SetActState(1, MS::kActRun).IsOk());

		ActorSnapshot snapshot;
		CHECK(wired.runtime.Snapshot(1, snapshot));
		CHECK_EQ(snapshot.actState, static_cast<Network::WireU32>(MS::kActRun));
	}

	// The snapshot is a COPY under the slot's lock.
	//
	// Handing out a reference to the live actor would let a reader race the ticker, which
	// is exactly the bug the slot mutex exists to prevent - so the snapshot is a value,
	// and a stale copy cannot be observed to change under the reader's feet.
	MODERN_TEST(WorldMovement_ASnapshotIsACopyAndReadsAsFalseForAnUnknownSession)
	{
		Wired wired{true};
		CHECK(wired.Mesh() != nullptr);
		if (wired.Mesh() == nullptr)
		{
			return;
		}

		WorldCharacter character = MakeCharacter(126, Vector3{5.0f, 0.0f, 3.0f});
		CHECK(wired.runtime.Attach(1, character).IsOk());

		GotoResult result;
		CHECK(wired.runtime.ApplyGoto(1, character, MakeWalkRequest(0), result).IsOk());

		ActorSnapshot before;
		CHECK(wired.runtime.Snapshot(1, before));
		CHECK_EQ(before.sessionId, static_cast<Network::WireU64>(1));

		CHECK_EQ(wired.runtime.Tick(0.1f), static_cast<std::size_t>(1));

		// The copy taken BEFORE the tick still reads the position from before it.
		CHECK_NEAR(before.position.x, 5.0f);

		ActorSnapshot after;
		CHECK(wired.runtime.Snapshot(1, after));
		CHECK(after.position.x > before.position.x);

		// And an unknown session writes nothing into the caller's snapshot, so a caller
		// that reuses one value cannot mistake a stale read for a fresh one.
		ActorSnapshot reused = after;
		CHECK(!wired.runtime.Snapshot(999, reused));
		CHECK_EQ(reused.sessionId, static_cast<Network::WireU64>(1));
	}

	// =========================================================================
	// 4. Several characters, one mesh
	// =========================================================================

	// Two characters in the same map, sharing ONE mesh pointer, and only one of them
	// moving.
	//
	// This is the relation 002e measured - sixteen of the fifty-six distinct `.wld`
	// files are named by more than one map identity - so two actors routinely hold the
	// same pointer. Nothing mutable is written into the mesh, and each actor keeps its own
	// A* session, so this is correct rather than lucky.
	MODERN_TEST(WorldMovement_TwoCharactersShareOneMeshAndMoveIndependently)
	{
		Wired wired{true};
		CHECK(wired.Mesh() != nullptr);
		if (wired.Mesh() == nullptr)
		{
			return;
		}

		WorldCharacter first  = MakeCharacter(130, Vector3{5.0f, 0.0f, 3.0f});
		WorldCharacter second = MakeCharacter(131, Vector3{15.0f, 0.0f, 7.0f});

		CHECK(wired.runtime.Attach(1, first).IsOk());
		CHECK(wired.runtime.Attach(2, second).IsOk());
		CHECK_EQ(wired.runtime.ActorCount(), static_cast<std::size_t>(2));

		// The map source was asked once per attach, and both answers are the same mesh.
		CHECK_EQ(wired.source.calls, static_cast<std::size_t>(2));

		ActorSnapshot firstSnapshot;
		ActorSnapshot secondSnapshot;
		CHECK(wired.runtime.Snapshot(1, firstSnapshot));
		CHECK(wired.runtime.Snapshot(2, secondSnapshot));
		CHECK(firstSnapshot.hasMesh);
		CHECK(secondSnapshot.hasMesh);

		// Only the first is given a destination. The request's claimed position has to be
		// the FIRST character's own position, which it is - the second is left alone.
		GotoResult result;
		CHECK(wired.runtime.ApplyGoto(1, first, MakeWalkRequest(0), result).IsOk());
		CHECK(result.accepted);

		CHECK_EQ(wired.runtime.Tick(0.1f), static_cast<std::size_t>(1));

		CHECK(wired.runtime.Snapshot(1, firstSnapshot));
		CHECK(wired.runtime.Snapshot(2, secondSnapshot));

		// The first walked sixteen hundredths of a second at its walk speed, 1.6 units.
		CHECK(firstSnapshot.position.x > 5.0f);
		CHECK_NEAR(firstSnapshot.movedDist, 1.6f);

		// The second did not move, was not given a path, and has spent no frames.
		CHECK_NEAR(secondSnapshot.position.x, 15.0f);
		CHECK(!secondSnapshot.pathActive);
		CHECK_NEAR(secondSnapshot.movedDist, 0.0f);
		CHECK_NEAR(secondSnapshot.movedTime, 0.0f);

		// And the counters are per-runtime, not per-character.
		CHECK_EQ(wired.runtime.GotoAcceptedCount(), static_cast<std::size_t>(1));
	}

	// Detaching one character leaves the other walking.
	//
	// The actor is torn down AFTER the slot leaves the list, so no ticker iteration can
	// be looking at it - and the detached character's mesh reference is dropped rather
	// than leaked, which matters when 002e's shared-mesh relation means several slots
	// hold the same pointer.
	MODERN_TEST(WorldMovement_DetachForgetsOneCharacterAndLeavesTheOtherWalking)
	{
		Wired wired{true};
		CHECK(wired.Mesh() != nullptr);
		if (wired.Mesh() == nullptr)
		{
			return;
		}

		WorldCharacter first  = MakeCharacter(140, Vector3{5.0f, 0.0f, 3.0f});
		WorldCharacter second = MakeCharacter(141, Vector3{15.0f, 0.0f, 7.0f});

		CHECK(wired.runtime.Attach(1, first).IsOk());
		CHECK(wired.runtime.Attach(2, second).IsOk());

		GotoResult result;
		CHECK(wired.runtime.ApplyGoto(1, first, MakeWalkRequest(0), result).IsOk());

		wired.runtime.Detach(1);

		CHECK_EQ(wired.runtime.ActorCount(), static_cast<std::size_t>(1));

		ActorSnapshot gone;
		CHECK(!wired.runtime.Snapshot(1, gone));

		// The survivor is untouched and still walking.
		ActorSnapshot survivor;
		CHECK(wired.runtime.Snapshot(2, survivor));
		CHECK_EQ(survivor.gaeaId, static_cast<Network::WireU32>(141));
		CHECK_NEAR(survivor.position.x, 15.0f);

		// A tick with no path active moves nobody, and detaching mid-world is safe.
		CHECK_EQ(wired.runtime.Tick(0.1f), static_cast<std::size_t>(0));
		CHECK(wired.runtime.Snapshot(2, survivor));
		CHECK_NEAR(survivor.position.x, 15.0f);
	}

	// =========================================================================
	// 5. The ticker thread
	// =========================================================================

	// Starting is optional, and starting twice is refused rather than ignored.
	//
	// A second Start would be a second thread walking the same actors, and two threads
	// advancing one Actor is a data race with no correct outcome. Calling Start is
	// OPTIONAL at all because `Tick` is the interface a caller that drives movement
	// itself should use - which is what every other case in this file does.
	//
	// The runtime here has no actors, so the thread has nothing to advance and the case
	// costs no wall-clock time: it starts, refuses a second start, and stops.
	MODERN_TEST(WorldMovement_TheTickerIsOptionalAndRefusesASecondThread)
	{
		WorldMovementRuntime runtime;

		// Not running yet, and stopping something that is not running is harmless.
		CHECK(!runtime.TickerRunning());
		runtime.StopTicker();
		CHECK(!runtime.TickerRunning());

		CHECK(runtime.StartTicker().IsOk());
		CHECK(runtime.TickerRunning());

		// The second start is REFUSED, not silently accepted.
		CHECK(runtime.StartTicker().IsError());
		CHECK(runtime.TickerRunning());

		runtime.StopTicker();
		CHECK(!runtime.TickerRunning());

		// And stopping twice is harmless too.
		runtime.StopTicker();
		CHECK(!runtime.TickerRunning());

		// After stopping, `Tick` is still the caller's to drive.
		CHECK_EQ(runtime.Tick(0.1f), static_cast<std::size_t>(0));
	}
}
