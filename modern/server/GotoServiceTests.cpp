// WORLD-ENTRY-002f: the GOTO rule, `GLChar::MsgGoto` transliterated.
//
// `MsgGoto` (GLCharMsg.cpp:224-324) is a hundred lines of ORDERED checks over four
// inputs: the authoritative state word, the authoritative position, the client's
// claim of where it is, and the client's requested destination. GotoService is that
// function with the socket and the GLChar removed, and this file is where a wrong
// ORDER or a wrong constant is caught.
//
// No socket, and none is needed: the rule is decided before any byte is framed and
// after any byte is read, so testing it here tests the rule rather than a transport.
// The 3034 and 3035 layouts are WorldEntryProtocolTests' and GotoProtocolTests'
// business.
//
// The cases are grouped by the property they defend, and two of those groups are the
// security-relevant ones:
//
//   the 60-unit check   the FULL 3D distance, strictly greater than, measured against
//                       the SERVER's position
//   the ORDER           EM_ACT_DIE beats the run bit, and the run bit beats the
//                       distance check - so a desynchronised client that also asks to
//                       run still gets its run state applied
//   silence             a refusal sends NOTHING, which is why the client pump takes a
//                       COUNT rather than "until the next one"
//   the speed seam      base velocity only, through the recovered 16-row table
//
// The silence group is the one most easily broken by accident. Adding an error packet
// for a refused destination would be the obvious "improvement", and it would break the
// client, whose pump waits for a 3035 that legacy never sent.

#include "TestHarness.h"

#include "NavigationMeshFixture.h"

#include "MovementStateProtocol.h"
#include "character/CharacterClassTable.h"
#include "movement/Actor.h"
#include "movement/MovementSpeed.h"
#include "stats/BaseStats.h"
#include "world/CharacterClassMovementSpeed.h"
#include "world/GotoService.h"
#include "world/MovementStateService.h"
#include "world/WorldCharacter.h"

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

	// One tolerance for the whole file, as in MovementTests.cpp: navigation is float
	// arithmetic and "approximately" needs a stated number.
	constexpr float kTolerance = 0.001f;

	bool Near(float actual, float expected)
	{
		return std::fabs(actual - expected) <= kTolerance;
	}

	#define CHECK_NEAR(actual, expected) \
	    ::ModernTests::CheckImpl(Near((actual), (expected)), #actual " ~= " #expected, __FILE__, __LINE__)

	// The packed map id the fixture mesh answers for.
	constexpr std::uint32_t kFixtureMapId = 7u;

	// A character on the corridor, on the map the fixture holds.
	//
	// `classValue` and `gender` are the RAW wire numbers - RAN's `EMCHARCLASS` is 1..8
	// and `CharacterGender` is 0/1 - because the pair is what `CharacterClassMovementSpeed`
	// resolves and what a stored record actually carries.
	WorldCharacter MakeCharacter(std::uint32_t gaeaId, std::uint32_t classValue,
	                             Network::WireU8 gender, const Vector3& spawn)
	{
		WorldCharacter character;
		character.id              = WorldCharacterId{gaeaId};
		character.accountId       = WorldAccountId{1};
		character.userId          = "user_1";
		character.name            = "Tester";
		character.characterClass  = classValue;
		character.characterGender = gender;
		character.school          = 1;
		character.level           = 10;

		character.hp = {3000u, 3000u};
		character.mp = {1500u, 1500u};
		character.sp = {800u, 800u};

		character.saveMapId.value = kFixtureMapId;
		character.savePosition   = {spawn.x, spawn.y, spawn.z};
		character.gaeaId         = gaeaId;
		// accountLevel stays 0: an ordinary account, below USER_GM3.
		return character;
	}

	// Archer, female - `EMCHARINDEX` 2, the row that walks 16 and runs 42. Chosen
	// because walk and run differ enough that a mixed-up branch cannot pass by luck.
	constexpr std::uint32_t kArcher    = 3;
	constexpr Network::WireU8 kFemale  = 1;

	// Swordsman, male - `EMCHARINDEX` 1, walk 12 run 36.
	constexpr std::uint32_t kSwordsman = 2;
	constexpr Network::WireU8 kMale    = 0;

	// A 3034 that walks from (5, 0, 3) to (25, 0, 3), claiming to be exactly where the
	// server thinks it is.
	GotoRequest MakeWalkRequest(Network::WireU32 actState = 0)
	{
		GotoRequest request;
		request.requestedActState = actState;
		request.claimedCurrent    = Vector3{5.0f, 0.0f, 3.0f};
		request.requestedTarget   = Vector3{25.0f, 0.0f, 3.0f};
		return request;
	}

	// =========================================================================
	// 1. The accepted path
	// =========================================================================

	// The whole accepted sequence in one case: state applied, distance accepted,
	// destination probed, speed set, and everything the 3035 would carry filled in.
	MODERN_TEST(Goto_AnAcceptedRequestWalksSetsTheSpeedAndFillsTheBroadcastFields)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = MakeCorridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		CharacterClassMovementSpeed provider;
		const MovementStateService movement{&provider};
		const GotoService          service{movement};

		Actor actor;
		CHECK(actor.Create(mesh, Vector3{5.0f, 0.0f, 3.0f}, Actor::kNoCell).IsOk());

		WorldCharacter character = MakeCharacter(100, kArcher, kFemale, Vector3{5.0f, 0.0f, 3.0f});

		// Ask to RUN. The bit is in the request, and it is the request that applies it.
		const GotoResult result = service.Apply(actor, character, MakeWalkRequest(MS::kActRun));

		CHECK_EQ(static_cast<int>(result.outcome), static_cast<int>(GotoOutcome::Accepted));
		CHECK(result.accepted);
		CHECK(result.detail.empty());

		// State: the server's WHOLE word after the run bit, never the request.
		CHECK(result.actStateChanged);
		CHECK_EQ(result.previousActState, static_cast<Network::WireU32>(0));
		CHECK_EQ(result.actState, static_cast<Network::WireU32>(MS::kActRun));
		CHECK_EQ(character.actState, static_cast<Network::WireU32>(MS::kActRun));

		// Distance: the client's claim, measured against the server's position.
		CHECK_NEAR(result.claimedDistance, 0.0f);

		// The 3035 payload. `vCurPos` is the SERVER's position at the moment of the
		// decision, which is how a client that has drifted learns where the server thinks
		// it is.
		CHECK_NEAR(result.authoritativeCurrent.x, 5.0f);
		CHECK_NEAR(result.authoritativeCurrent.y, 0.0f);
		CHECK_NEAR(result.authoritativeCurrent.z, 3.0f);

		// `vTarPos` is the RAW requested target (GLCharMsg.cpp:290 and :315), and the
		// actor's own speed became the recovered RUN velocity for this class row.
		CHECK_NEAR(result.authoritativeTarget.x, 25.0f);
		CHECK_NEAR(result.maxSpeed, 42.0f);
		CHECK_NEAR(actor.MaxSpeed(), 42.0f);
		CHECK(result.pathActive);
	}

	// The speed is the class's WALK velocity when the character is not running, and the
	// run velocity when it is - through the SAME provider the 3032 path uses, so a 3034
	// and a 3032 cannot disagree about how fast a character walks.
	MODERN_TEST(Goto_TheSpeedIsWalkOrRunAccordingToTheAuthoritativeRunFlag)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = MakeCorridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		CharacterClassMovementSpeed provider;
		const MovementStateService movement{&provider};
		const GotoService          service{movement};

		// Already running, and asking to keep running: the run velocity, and no state
		// change because the word did not move.
		{
			Actor actor;
			CHECK(actor.Create(mesh, Vector3{5.0f, 0.0f, 3.0f}, Actor::kNoCell).IsOk());
			WorldCharacter character =
			    MakeCharacter(101, kSwordsman, kMale, Vector3{5.0f, 0.0f, 3.0f});
			character.actState = MS::kActRun;

			const GotoResult result =
			    service.Apply(actor, character, MakeWalkRequest(MS::kActRun));

			CHECK(result.accepted);
			CHECK(!result.actStateChanged);
			CHECK_NEAR(result.maxSpeed, 36.0f);
		}

		// Walking, and asking to walk: the walk velocity.
		{
			Actor actor;
			CHECK(actor.Create(mesh, Vector3{5.0f, 0.0f, 3.0f}, Actor::kNoCell).IsOk());
			WorldCharacter character =
			    MakeCharacter(102, kSwordsman, kMale, Vector3{5.0f, 0.0f, 3.0f});

			const GotoResult result = service.Apply(actor, character, MakeWalkRequest(0));

			CHECK(result.accepted);
			CHECK(!result.actStateChanged);
			CHECK_NEAR(result.maxSpeed, 12.0f);
		}

		// Walking, and asking to RUN: the run velocity, from the SAME request.
		//
		// GLCharMsg.cpp:306 computes `GetMoveVelo()` AFTER :258-262 applied the bit, so
		// a character starts running on the tick its GOTO arrives rather than one tick
		// later. That ordering is reproduced, and it is the reason the provider reads the
		// CHARACTER rather than the request.
		{
			Actor actor;
			CHECK(actor.Create(mesh, Vector3{5.0f, 0.0f, 3.0f}, Actor::kNoCell).IsOk());
			WorldCharacter character =
			    MakeCharacter(103, kSwordsman, kMale, Vector3{5.0f, 0.0f, 3.0f});

			const GotoResult result =
			    service.Apply(actor, character, MakeWalkRequest(MS::kActRun));

			CHECK(result.accepted);
			CHECK(result.actStateChanged);
			CHECK_NEAR(result.maxSpeed, 36.0f);
		}
	}

	// The RAW requested target is broadcast, even though the actor paths to the point
	// the ±10 probe actually found.
	//
	// GLCharMsg.cpp:290 stores the client's value verbatim and :315 broadcasts
	// `m_TargetID.vPos` - not the probe's resolution. So peers are told where the mover
	// ASKED to go while the server paths to a point that can be ten units away in Y.
	// A real client re-runs its own probe against the target it receives, which is why
	// this reproduces rather than "fixes".
	MODERN_TEST(Goto_TheRawTargetIsBroadcastWhileTheActorPathsToTheProbedPoint)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = MakeCorridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		CharacterClassMovementSpeed provider;
		const MovementStateService movement{&provider};
		const GotoService          service{movement};

		Actor actor;
		CHECK(actor.Create(mesh, Vector3{5.0f, 0.0f, 5.0f}, Actor::kNoCell).IsOk());

		WorldCharacter character = MakeCharacter(104, kArcher, kFemale, Vector3{5.0f, 0.0f, 5.0f});

		// Four units above the floor, so the ±10 probe finds the floor beneath it.
		GotoRequest request          = MakeWalkRequest(0);
		request.claimedCurrent       = Vector3{5.0f, 0.0f, 5.0f};
		request.requestedTarget      = Vector3{22.0f, 4.0f, 5.0f};

		const GotoResult result = service.Apply(actor, character, request);

		CHECK(result.accepted);

		// The broadcast carries what was asked for...
		CHECK_NEAR(result.authoritativeTarget.x, 22.0f);
		CHECK_NEAR(result.authoritativeTarget.y, 4.0f);

		// ...and the actor walks to the floor, which is four units lower.
		CHECK_NEAR(actor.TargetPosition().y, 0.0f);
		CHECK(result.pathActive);
	}

	// =========================================================================
	// 2. The 60-unit desynchronisation check
	// =========================================================================

	// RAN's number, measured over the FULL 3D distance, and STRICTLY greater than.
	//
	// A modern tolerance chosen to be generous would let a client claim a position
	// further from the server's own than RAN ever permitted, so the constant is
	// asserted rather than treated as a policy knob.
	MODERN_TEST(Goto_TheSixtyUnitToleranceIsStrictlyGreaterAndFullThreeDimensional)
	{
		CHECK_NEAR(kGotoPositionTolerance, 60.0f);

		const std::shared_ptr<Navigation::NavigationMesh> mesh = MakeCorridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		CharacterClassMovementSpeed provider;
		const MovementStateService movement{&provider};
		const GotoService          service{movement};

		struct Case
		{
			const char* what;
			Vector3     claimed;
			bool        accepted;
		};

		// The four claims that decide what the rule measures: exactly 60 passes, one
		// hundredth over fails, a pure vertical offset counts, and an XZ offset counts.
		const Case cases[4] = {
		    {"exactly sixty units away", Vector3{65.0f, 0.0f, 3.0f}, true},
		    {"one hundredth of a unit over", Vector3{65.01f, 0.0f, 3.0f}, false},
		    {"forty units below", Vector3{5.0f, -40.0f, 3.0f}, true},
		    {"seventy point seven units away in XZ", Vector3{5.0f + 50.0f, 0.0f, 3.0f - 50.0f},
		     false},
		};

		for (const Case& testCase : cases)
		{
			Actor actor;
			CHECK(actor.Create(mesh, Vector3{5.0f, 0.0f, 3.0f}, Actor::kNoCell).IsOk());

			WorldCharacter character =
			    MakeCharacter(105, kArcher, kFemale, Vector3{5.0f, 0.0f, 3.0f});

			GotoRequest request     = MakeWalkRequest(0);
			request.claimedCurrent = testCase.claimed;

			const GotoResult result = service.Apply(actor, character, request);

			// The distance is reported even when the request is refused, because an
			// operator reading a refusal needs the number that caused it.
			CHECK_NEAR(result.claimedDistance, (actor.Position() - testCase.claimed).Length());

			if (testCase.accepted)
			{
				CHECK_EQ(static_cast<int>(result.outcome),
				         static_cast<int>(GotoOutcome::Accepted));
			}
			else
			{
				CHECK_EQ(static_cast<int>(result.outcome),
				         static_cast<int>(GotoOutcome::RejectedPositionDesync));
			}
		}

		// The three-dimensional claim, stated on its own because it is the one an
		// XZ-only implementation would get wrong: a client forty units BELOW its server
		// position is forty units out.
		{
			Actor actor;
			CHECK(actor.Create(mesh, Vector3{5.0f, 0.0f, 3.0f}, Actor::kNoCell).IsOk());
			WorldCharacter character =
			    MakeCharacter(106, kArcher, kFemale, Vector3{5.0f, 0.0f, 3.0f});

			GotoRequest request     = MakeWalkRequest(0);
			request.claimedCurrent = Vector3{5.0f, -61.0f, 3.0f};

			const GotoResult result = service.Apply(actor, character, request);

			CHECK_NEAR(result.claimedDistance, 61.0f);
			CHECK_EQ(static_cast<int>(result.outcome),
			         static_cast<int>(GotoOutcome::RejectedPositionDesync));
		}
	}

	// A desynchronised client's GOTO is refused AND THE ACTOR STOPS.
	//
	// The stop is the part of the branch that affects the world. The two correction
	// packets legacy sends - 3830 to the moving client and 3064 to everyone around it -
	// are NOT implemented, which is a named deferral with a stated consequence: a
	// drifted client is refused and is not told to correct its position. The server's
	// position is still authoritative and still unchanged.
	MODERN_TEST(Goto_ADesynchronisedClientIsRefusedAndTheActorStops)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = MakeCorridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		CharacterClassMovementSpeed provider;
		const MovementStateService movement{&provider};
		const GotoService          service{movement};

		Actor actor;
		CHECK(actor.Create(mesh, Vector3{5.0f, 0.0f, 3.0f}, Actor::kNoCell).IsOk());

		WorldCharacter character = MakeCharacter(107, kArcher, kFemale, Vector3{5.0f, 0.0f, 3.0f});

		// Give it a path first, so "the actor stops" is observable.
		CHECK(service.Apply(actor, character, MakeWalkRequest(0)).accepted);
		CHECK(actor.PathIsActive());

		const Vector3 before = actor.Position();

		// Now a request that claims to be 100 units away, with a destination that would
		// otherwise be perfectly walkable.
		GotoRequest refused     = MakeWalkRequest(0);
		refused.claimedCurrent = Vector3{105.0f, 0.0f, 3.0f};
		refused.requestedTarget = Vector3{15.0f, 0.0f, 3.0f};

		const GotoResult result = service.Apply(actor, character, refused);

		CHECK_EQ(static_cast<int>(result.outcome),
		         static_cast<int>(GotoOutcome::RejectedPositionDesync));
		CHECK(!result.accepted);
		CHECK(!result.pathActive);

		// Stopped, and not moved by the refused request.
		CHECK(!actor.PathIsActive());
		CHECK_NEAR(result.claimedDistance, 100.0f);
		CHECK(actor.Position() == before);

		// The reason names the number, because an operator reading a refusal needs it.
		CHECK(!result.detail.empty());
		CHECK(result.detail.find("60-unit") != std::string::npos);

		// And no speed was set: legacy calls SetMaxSpeed only on the success path
		// (GLCharMsg.cpp:306).
		CHECK_NEAR(result.maxSpeed, 0.0f);
	}

	// =========================================================================
	// 3. The ORDER - the part that is easy to get backwards
	// =========================================================================

	// The run bit is applied BEFORE the 60-unit check, and that is the point.
	//
	// GLCharMsg.cpp:258-262 runs before :266. So a client that sends a desynchronised
	// GOTO with a changed run flag still gets its run flag applied: the check governs
	// MOVEMENT, not STATE. A server that checked the distance first would have refused
	// the movement and silently discarded the state change - so a player who drifted
	// could not stop running until they stopped drifting.
	MODERN_TEST(Goto_TheRunFlagIsAppliedBeforeTheDesyncCheck)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = MakeCorridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		CharacterClassMovementSpeed provider;
		const MovementStateService movement{&provider};
		const GotoService          service{movement};

		Actor actor;
		CHECK(actor.Create(mesh, Vector3{5.0f, 0.0f, 3.0f}, Actor::kNoCell).IsOk());

		WorldCharacter character = MakeCharacter(108, kArcher, kFemale, Vector3{5.0f, 0.0f, 3.0f});

		GotoRequest request     = MakeWalkRequest(MS::kActRun);
		request.claimedCurrent = Vector3{500.0f, 0.0f, 3.0f};

		const GotoResult result = service.Apply(actor, character, request);

		// The movement was refused...
		CHECK_EQ(static_cast<int>(result.outcome),
		         static_cast<int>(GotoOutcome::RejectedPositionDesync));
		CHECK(!result.accepted);

		// ...and the state was NOT. The run bit is in the authoritative word, on both
		// the record and the result.
		CHECK(result.actStateChanged);
		CHECK_EQ(result.actState, static_cast<Network::WireU32>(MS::kActRun));
		CHECK_EQ(character.actState, static_cast<Network::WireU32>(MS::kActRun));
	}

	// EM_ACT_DIE beats the run bit: a dying character refuses the GOTO WITHOUT its run
	// state changing.
	//
	// The other half of the ordering (GLCharMsg.cpp:238 runs before :258). Die, then
	// run-flag, then distance - and both asymmetries are reproduced exactly.
	MODERN_TEST(Goto_ADeadCharacterRefusesWithoutItsRunStateChanging)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = MakeCorridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		CharacterClassMovementSpeed provider;
		const MovementStateService movement{&provider};
		const GotoService          service{movement};

		Actor actor;
		CHECK(actor.Create(mesh, Vector3{5.0f, 0.0f, 3.0f}, Actor::kNoCell).IsOk());

		WorldCharacter character = MakeCharacter(109, kArcher, kFemale, Vector3{5.0f, 0.0f, 3.0f});
		character.actState       = MS::kActDie;

		// A request that also asks to run, and claims to be exactly where it is, so the
		// distance check would have passed.
		const GotoResult result =
		    service.Apply(actor, character, MakeWalkRequest(MS::kActRun));

		CHECK_EQ(static_cast<int>(result.outcome), static_cast<int>(GotoOutcome::RejectedDead));
		CHECK(!result.accepted);

		// Nothing about the state moved.
		CHECK(!result.actStateChanged);
		CHECK_EQ(character.actState, static_cast<Network::WireU32>(MS::kActDie));
		CHECK_EQ(result.actState & MS::kActRun, static_cast<Network::WireU32>(0));

		// And nothing about the world either.
		CHECK(!actor.PathIsActive());
		CHECK(!result.pathActive);
	}

	// A 3034 applies ONE bit, and only that one.
	//
	// This is deliberately NOT `MovementStateService::ApplyMoveState`, which is 3032's
	// rule and touches FOUR bits behind the USER_GM3 gate. MsgGoto never reads
	// EM_ACT_PEACEMODE and never reads either visibility bit, so routing a 3034 through
	// the 3032 rule would grant a GOTO authority it does not have - including the right
	// to set EM_ACT_DIE, which is the bit the whole world trusts.
	MODERN_TEST(Goto_ARequestAppliesExactlyOneBitAndNeverTheDeathBit)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = MakeCorridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		CharacterClassMovementSpeed provider;
		const MovementStateService movement{&provider};
		const GotoService          service{movement};

		Actor actor;
		CHECK(actor.Create(mesh, Vector3{5.0f, 0.0f, 3.0f}, Actor::kNoCell).IsOk());

		// A request with every bit set: EM_ACT_RUN plus EM_ACT_DIE plus
		// EM_ACT_PEACEMODE. A routing mistake through the 3032 rule would set all three.
		WorldCharacter character = MakeCharacter(110, kArcher, kFemale, Vector3{5.0f, 0.0f, 3.0f});

		const GotoResult result = service.Apply(
		    actor, character, MakeWalkRequest(MS::kActRun | MS::kActDie | MS::kActPeaceMode));

		CHECK(result.accepted);

		// Exactly one bit came out the other side.
		CHECK_EQ(character.actState, static_cast<Network::WireU32>(MS::kActRun));
		CHECK_EQ(character.actState & MS::kActDie, static_cast<Network::WireU32>(0));
		CHECK_EQ(character.actState & MS::kActPeaceMode, static_cast<Network::WireU32>(0));
		CHECK_EQ(result.actState, character.actState);
	}

	// =========================================================================
	// 4. Refusals, and why they are SILENT
	// =========================================================================

	// A refusal is silent: no 3035, no movement, and the client is told nothing.
	//
	// Legacy's block for an unreachable destination is an empty `if` with a commented-out
	// log - no state change, no early return, S_OK - and because `bSucceed` is FALSE no
	// 3035 goes out. That is why the client pump takes a COUNT (`PumpUntilGotoCount`)
	// rather than "until the next one": there is nothing to wait for.
	//
	// So the property to assert is that `accepted` is false and `maxSpeed` is zero for
	// EVERY refusal. `accepted` is the one field that decides whether a 3035 exists, and
	// it must never be true for anything but `Accepted`.
	MODERN_TEST(Goto_EveryRefusalIsSilentAndNothingWalks)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = MakeCorridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		CharacterClassMovementSpeed provider;
		const MovementStateService movement{&provider};
		const GotoService          service{movement};

		// (a) The destination has no navigation surface within ten units of it. 500
		// units along X is off the corridor entirely.
		{
			Actor actor;
			CHECK(actor.Create(mesh, Vector3{5.0f, 0.0f, 3.0f}, Actor::kNoCell).IsOk());
			WorldCharacter character =
			    MakeCharacter(111, kArcher, kFemale, Vector3{5.0f, 0.0f, 3.0f});

			GotoRequest request = MakeWalkRequest(0);
			request.requestedTarget = Vector3{500.0f, 0.0f, 3.0f};

			const GotoResult result = service.Apply(actor, character, request);

			CHECK_EQ(static_cast<int>(result.outcome),
			         static_cast<int>(GotoOutcome::RejectedDestination));
			CHECK(!result.accepted);
			CHECK(!result.pathActive);
			CHECK(!actor.PathIsActive());
			CHECK_NEAR(result.maxSpeed, 0.0f);
			CHECK(!result.detail.empty());
		}

		// (b) The character is dead.
		{
			Actor actor;
			CHECK(actor.Create(mesh, Vector3{5.0f, 0.0f, 3.0f}, Actor::kNoCell).IsOk());
			WorldCharacter character =
			    MakeCharacter(112, kArcher, kFemale, Vector3{5.0f, 0.0f, 3.0f});
			character.actState = MS::kActDie;

			const GotoResult result = service.Apply(actor, character, MakeWalkRequest(0));

			CHECK_EQ(static_cast<int>(result.outcome), static_cast<int>(GotoOutcome::RejectedDead));
			CHECK(!result.accepted);
			CHECK(!result.pathActive);
			CHECK_NEAR(result.maxSpeed, 0.0f);
		}

		// (c) The character's map has no navigation mesh.
		//
		// RAN cannot reach this - a field server always has its meshes - so a modern
		// server started without an asset root can. It is its OWN outcome rather than a
		// flavour of "destination unreachable", because the two have different fixes:
		// one is a deployment problem and the other is a player's click.
		{
			Actor          actor; // never created, so no mesh
			WorldCharacter character =
			    MakeCharacter(113, kArcher, kFemale, Vector3{5.0f, 0.0f, 3.0f});

			const GotoResult result = service.Apply(actor, character, MakeWalkRequest(0));

			CHECK_EQ(static_cast<int>(result.outcome),
			         static_cast<int>(GotoOutcome::RejectedNoMesh));
			CHECK(!result.accepted);
			CHECK(!result.pathActive);
			CHECK_NEAR(result.maxSpeed, 0.0f);
			CHECK(result.detail.find("no navigation mesh") != std::string::npos);
		}

		// (d) The desynchronisation check, restated here so that the four refusals sit
		// in one list and the invariant is checked over all of them at once.
		{
			Actor actor;
			CHECK(actor.Create(mesh, Vector3{5.0f, 0.0f, 3.0f}, Actor::kNoCell).IsOk());
			WorldCharacter character =
			    MakeCharacter(114, kArcher, kFemale, Vector3{5.0f, 0.0f, 3.0f});

			GotoRequest request     = MakeWalkRequest(0);
			request.claimedCurrent = Vector3{500.0f, 0.0f, 3.0f};

			const GotoResult result = service.Apply(actor, character, request);

			CHECK_EQ(static_cast<int>(result.outcome),
			         static_cast<int>(GotoOutcome::RejectedPositionDesync));
			CHECK(!result.accepted);
			CHECK_NEAR(result.maxSpeed, 0.0f);
		}
	}

	// A refused GOTO leaves the actor's speed alone.
	//
	// Legacy calls `SetMaxSpeed(GetMoveVelo())` only on the success path
	// (GLCharMsg.cpp:306), so a character that is refused a destination keeps walking at
	// whatever it was walking at. Overwriting it on refusal would be a plausible-looking
	// "cleanup" that changes what a mid-walk click does.
	MODERN_TEST(Goto_ARefusedRequestLeavesTheActorsSpeedAlone)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = MakeCorridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		CharacterClassMovementSpeed provider;
		const MovementStateService movement{&provider};
		const GotoService          service{movement};

		Actor actor;
		CHECK(actor.Create(mesh, Vector3{5.0f, 0.0f, 3.0f}, Actor::kNoCell).IsOk());

		// An accepted request sets the class speed.
		WorldCharacter character = MakeCharacter(115, kArcher, kFemale, Vector3{5.0f, 0.0f, 3.0f});
		CHECK(service.Apply(actor, character, MakeWalkRequest(0)).accepted);
		CHECK_NEAR(actor.MaxSpeed(), 16.0f);

		// A refused one does not.
		GotoRequest refused = MakeWalkRequest(0);
		refused.requestedTarget = Vector3{500.0f, 0.0f, 3.0f};
		CHECK(!service.Apply(actor, character, refused).accepted);
		CHECK_NEAR(actor.MaxSpeed(), 16.0f);
	}

	// Every outcome has a name, and the names are distinct.
	//
	// A log line that printed "Unrecognised" for a real outcome would be the only clue
	// an operator has, so the mapping is asserted rather than trusted.
	MODERN_TEST(Goto_EveryOutcomeHasADistinctName)
	{
		const GotoOutcome outcomes[5] = {
		    GotoOutcome::Accepted,
		    GotoOutcome::RejectedDead,
		    GotoOutcome::RejectedPositionDesync,
		    GotoOutcome::RejectedNoMesh,
		    GotoOutcome::RejectedDestination,
		};

		for (std::size_t i = 0; i < 5; ++i)
		{
			const std::string name = ToString(outcomes[i]);
			CHECK(!name.empty());
			CHECK(name != "Unrecognised");

			for (std::size_t j = i + 1; j < 5; ++j)
			{
				CHECK(name != ToString(outcomes[j]));
			}
		}
	}

	// =========================================================================
	// 5. The speed provider
	// =========================================================================

	// Class ALONE is not enough, which is the only reason gender exists on the record.
	//
	// RAN's speed table is indexed by `EMCHARINDEX`, sixteen entries - eight classes
	// times two genders - while `WorldCharacter::characterClass` is eight wide. The
	// resolved index is not transmitted and not stored: it is recomputed from the pair
	// every time a speed is needed, so there is exactly one answer to "which row".
	MODERN_TEST(Goto_TheClassIndexIsResolvedFromTheClassAndGenderPair)
	{
		const CharacterClassMovementSpeed provider;

		struct Case
		{
			std::uint32_t  classValue;
			Network::WireU8 gender;
			Stats::CharClassIndex expected;
			float                walk;
			float                run;
		};

		// All sixteen, through the provider, against the table MovementTests asserts.
		const Case cases[16] = {
		    {1, 0, Stats::CharClassIndex::BrawlerMale, 14.0f, 37.0f},
		    {2, 0, Stats::CharClassIndex::SwordsmanMale, 12.0f, 36.0f},
		    {3, 1, Stats::CharClassIndex::ArcherFemale, 16.0f, 42.0f},
		    {4, 1, Stats::CharClassIndex::ShamanFemale, 13.0f, 40.0f},
		    {5, 0, Stats::CharClassIndex::GunnerMale, 15.0f, 40.0f},
		    {5, 1, Stats::CharClassIndex::GunnerFemale, 15.0f, 40.0f},
		    {1, 1, Stats::CharClassIndex::BrawlerFemale, 14.0f, 37.0f},
		    {2, 1, Stats::CharClassIndex::SwordsmanFemale, 12.0f, 36.0f},
		    {3, 0, Stats::CharClassIndex::ArcherMale, 16.0f, 42.0f},
		    {4, 0, Stats::CharClassIndex::ShamanMale, 12.0f, 39.0f},
		    {8, 0, Stats::CharClassIndex::ExtremeMale, 14.0f, 39.0f},
		    {8, 1, Stats::CharClassIndex::ExtremeFemale, 14.0f, 39.0f},
		    {6, 0, Stats::CharClassIndex::AssassinMale, 12.0f, 44.0f},
		    {6, 1, Stats::CharClassIndex::AssassinFemale, 12.0f, 44.0f},
		    {7, 0, Stats::CharClassIndex::TrickerMale, 15.0f, 41.0f},
		    {7, 1, Stats::CharClassIndex::TrickerFemale, 15.0f, 41.0f},
		};

		for (const Case& testCase : cases)
		{
			WorldCharacter character =
			    MakeCharacter(120, testCase.classValue, testCase.gender, Vector3{});

			Stats::CharClassIndex index{};
			CHECK(provider.TryResolveClassIndex(character, index));
			CHECK_EQ(static_cast<int>(index), static_cast<int>(testCase.expected));

			// The same row, reached through the speed.
			float walk = 0.0f;
			CHECK(CharacterClassMovementSpeed::TryBaseVelocity(index, false, walk));
			CHECK_NEAR(walk, testCase.walk);

			float run = 0.0f;
			CHECK(CharacterClassMovementSpeed::TryBaseVelocity(index, true, run));
			CHECK_NEAR(run, testCase.run);
		}
	}

	// A class/gender pair with no row yields NO speed - not a default.
	//
	// `CharacterClass::Unset` is 0, which RAN never puts on a character, and the
	// underlying enum's width means a stored record could hold anything. A provider that
	// fell back would put a number on the authoritative server that RAN never produces;
	// `TryMaxSpeed` exists so that "no speed" cannot be confused with "very slow".
	MODERN_TEST(Goto_AClassWithNoTableRowYieldsNoSpeedRatherThanAFallback)
	{
		const CharacterClassMovementSpeed provider;

		// Unset.
		{
			WorldCharacter character = MakeCharacter(121, 0, 0, Vector3{});

			Stats::CharClassIndex index{};
			CHECK(!provider.TryResolveClassIndex(character, index));

			float speed = -1.0f;
			CHECK(!provider.TryMaxSpeed(character, speed));
			CHECK_NEAR(speed, 0.0f);
			CHECK_NEAR(provider.MaxSpeedFor(character), 0.0f);
		}

		// Past the last class RAN defines.
		{
			WorldCharacter character = MakeCharacter(122, 9, 0, Vector3{});
			Stats::CharClassIndex index{};
			CHECK(!provider.TryResolveClassIndex(character, index));
		}

		// A gender value RAN does not define. The range is checked before the cast
		// rather than after it, because the field is wider than the enum.
		{
			WorldCharacter character = MakeCharacter(123, 1, 2, Vector3{});
			Stats::CharClassIndex index{};
			CHECK(!provider.TryResolveClassIndex(character, index));

			character.characterGender = 0xFFu;
			CHECK(!provider.TryResolveClassIndex(character, index));
		}
	}

	// The provider reads the AUTHORITATIVE word, never the request.
	//
	// GLChar.cpp:4968 reads `IsSTATE(EM_ACT_RUN)` - the server's own state. A provider
	// that took a `bRunning` argument from the message handler would let a 3034 choose
	// its own speed, which is the authority the 3034 must not have.
	MODERN_TEST(Goto_TheProviderReadsTheAuthoritativeStateNotTheRequest)
	{
		const CharacterClassMovementSpeed provider;

		WorldCharacter character = MakeCharacter(124, kArcher, kFemale, Vector3{});
		character.actState       = 0;

		float speed = 0.0f;
		CHECK(provider.TryMaxSpeed(character, speed));
		CHECK_NEAR(speed, 16.0f);

		// Setting the bit on the record is the ONLY thing that changes the answer.
		character.actState = MS::kActRun;
		CHECK(provider.TryMaxSpeed(character, speed));
		CHECK_NEAR(speed, 42.0f);

		// And a bit the provider does not read - EM_ACT_PEACEMODE - changes nothing.
		character.actState = MS::kActPeaceMode;
		CHECK(provider.TryMaxSpeed(character, speed));
		CHECK_NEAR(speed, 16.0f);
	}

	// With no provider configured there is NO speed, and `GotoService` says so rather
	// than inventing one.
	//
	// This is a supported configuration in 002a and it is still supported here: the
	// alternative would be fabricating a number RAN does not produce.
	MODERN_TEST(Goto_WithNoSpeedProviderTheAcceptedRequestStillWalksAtNoSpeed)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = MakeCorridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		const MovementStateService movement{nullptr};
		CHECK(!movement.HasSpeedProvider());

		float speed = -1.0f;
		CHECK(!movement.TrySpeedFor(MakeCharacter(125, kArcher, kFemale, Vector3{}), speed));
		CHECK_NEAR(speed, 0.0f);

		// The GOTO itself is still ACCEPTED - a character with no speed is a deployment
		// condition, not a bad click - and the rule reports no speed.
		const GotoService service{movement};

		Actor actor;
		CHECK(actor.Create(mesh, Vector3{5.0f, 0.0f, 3.0f}, Actor::kNoCell).IsOk());
		WorldCharacter character = MakeCharacter(126, kArcher, kFemale, Vector3{5.0f, 0.0f, 3.0f});

		const GotoResult result = service.Apply(actor, character, MakeWalkRequest(MS::kActRun));

		CHECK(result.accepted);
		CHECK(result.actStateChanged);
		CHECK_NEAR(result.maxSpeed, 0.0f);
		// The ACTOR's speed is untouched, though, and that is worth stating: with no
		// provider the rule never calls SetMaxSpeed at all, so the actor keeps whatever
		// it had - here the constructor's named "unset" value. Zeroing it would
		// have meant inventing a number RAN never produces.
		CHECK_NEAR(actor.MaxSpeed(), Movement::kUnsetMaxSpeed);
	}
}
