// WORLD-ENTRY-002f: core movement tests - the speed table, the speed formula, and
// the headless `Actor`.
//
// Three subjects, three sections, and nothing else:
//
//   the TABLE    the sixteen recovered `EMCHARINDEX` rows, value by value, and the
//                proof that the 12/34 constructor defaults are not what is used
//   the FORMULA  `speed = base * (stateMultiplier + moveItem / itemDivisor)`,
//                including the two facts that are easy to get backwards
//   the ACTOR    the ±5 spawn probe, the ±10 destination probe, arrival at 0.01,
//                and `max_distance = maxSpeed * elapsedSeconds` with the elapsed time
//                INJECTED
//
// No socket, no clock, no asset root, no sleep. `Actor::Update` takes the elapsed
// time as an argument precisely so that "two half steps travel as far as one whole
// step" can be asserted without waiting for anything, and that is what makes the
// whole movement rule testable on a machine that has no RAN client installed.
//
// The geometry comes from `NavigationMeshFixture.h`, which builds a flat corridor in
// memory. A skipped movement test is not evidence of anything, so the mesh is
// synthesised rather than read from `RAN_ASSET_ROOT` - see that header for why, and
// for why the 188-byte records are written out by hand.
//
// Every float comparison states its tolerance. The harness has no approximate
// comparison, deliberately, because "approximately" needs a stated number and
// navigation is float arithmetic throughout.

#include "TestHarness.h"

#include "NavigationMeshFixture.h"

#include "map/MapRegistry.h"
#include "math/Vector3.h"
#include "movement/Actor.h"
#include "movement/MovementSpeed.h"
#include "movement/NavigationMapSource.h"
#include "navigation/NavigationCell.h"
#include "navigation/NavigationMesh.h"
#include "navigation/NavigationPath.h"
#include "stats/BaseStats.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>

using namespace Modern;
using namespace Modern::Movement;
using ModernTests::MeshFixture::CellIdFor;
using ModernTests::MeshFixture::kCorridorY;
using ModernTests::MeshFixture::kSegmentCount;
using ModernTests::MeshFixture::kSpanX;

namespace
{
	// One tolerance for the whole file, stated once.
	//
	// Navigation quantises every vertex to a millimetre (`NavigationMesh::Build`) and
	// solves heights on cell planes, so positions are exact to well under this; the
	// slack is for the arithmetic, not for the geometry.
	constexpr float kTolerance = 0.001f;

	bool Near(float actual, float expected)
	{
		return std::fabs(actual - expected) <= kTolerance;
	}

	#define CHECK_NEAR(actual, expected) \
	    ::ModernTests::CheckImpl(Near((actual), (expected)), #actual " ~= " #expected, __FILE__, __LINE__)

	// The same comparison with a stated, larger tolerance.
	//
	// Needed where the answer is bounded by a nudge rather than by arithmetic alone -
	// the resolver's ForcePointToCellCollumn leaves a blocked step a fraction of a
	// unit short of a wall, so a millimetre tolerance would be asserting a rounding
	// detail rather than a rule.
	#define CHECK_WITHIN(actual, expected, tolerance) \
	    ::ModernTests::CheckImpl(std::fabs((actual) - (expected)) <= (tolerance), \
	                             #actual " within " #tolerance " of " #expected, __FILE__, __LINE__)

	// The corridor, or a null shared_ptr with the fixture's own reason already
	// reported. Every actor case below needs a mesh, and a helper that returns a
	// usable one keeps each case to the rule it is about.
	std::shared_ptr<Navigation::NavigationMesh> Corridor()
	{
		return ModernTests::MeshFixture::MakeCorridor();
	}

	// A character created on the corridor floor at (5, 0, 3), with its path active
	// toward (25, 0, 3).
	//
	// The destination is the same 3.0 in Z throughout, so every step is purely along
	// X and a distance assertion is a one-dimensional statement.
	Status StartWalking(Actor& actor, const std::shared_ptr<Navigation::NavigationMesh>& mesh,
	                    float maxSpeed)
	{
		const Status created = actor.Create(mesh, Vector3{5.0f, kCorridorY, 3.0f},
		                                    Actor::kNoCell);
		if (created.IsError())
		{
			return created;
		}

		actor.SetMaxSpeed(maxSpeed);

		// The ±10 probe GLCharMsg.cpp:293-297 builds, about the destination.
		if (!actor.GotoDestination(Vector3{25.0f, 25.0f, 3.0f}, Vector3{25.0f, -25.0f, 3.0f}))
		{
			return Status(ErrorCode::NotFound);
		}
		return Ok();
	}

	// =========================================================================
	// 1. The fixture
	// =========================================================================

	// The corridor is asserted once, in the open, rather than trusted. Everything below
	// depends on three properties: the mesh built, a point in the first square is
	// inside a cell's column, and the ±10 vertical probe finds the floor. A fixture
	// whose winding or link table was wrong would otherwise fail every case in this
	// file for the same uninteresting reason.
	MODERN_TEST(Movement_FixtureCorridorIsWalkable)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = Corridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		CHECK(mesh->Built());
		CHECK_EQ(mesh->CellCount(), kSegmentCount * 2);

		// "Right of all three walls is inside" - the winding invariant every cell test
		// depends on, and the one that fails silently if reversed.
		const Navigation::NavigationCell* first = mesh->GetCell(CellIdFor(0, 0));
		CHECK(first != nullptr);
		if (first != nullptr)
		{
			CHECK(first->IsPointInCellCollumn(Vector3{5.0f, kCorridorY, 3.0f}));
			// And it reads as a FLOOR, which is what the sticky height latch tests.
			CHECK(first->Normal().y > 0.0f);
		}

		CHECK_EQ(mesh->FindClosestCell(Vector3{5.0f, kCorridorY, 3.0f}), CellIdFor(0, 0));

		// The ±10 probe: the whole of `GotoDestination`, and the only way a GOTO
		// destination is validated.
		Vector3                   hit{};
		std::uint32_t             hitCell = 0;
		const bool collided = mesh->IsCollision(Vector3{5.0f, 20.0f, 3.0f},
		                                        Vector3{5.0f, -20.0f, 3.0f}, hit, &hitCell);
		CHECK(collided);
		CHECK_NEAR(hit.y, kCorridorY);
		CHECK_EQ(hitCell, CellIdFor(0, 0));

		// Off the corridor there is nothing, which is what makes a "destination
		// unreachable" case constructible without a second fixture.
		CHECK(!mesh->IsCollision(Vector3{500.0f, 20.0f, 3.0f}, Vector3{500.0f, -20.0f, 3.0f},
		                         hit, &hitCell));
	}

	// =========================================================================
	// 2. The class speed table
	// =========================================================================

	// The sixteen recovered rows, value by value, in `EMCHARINDEX` order.
	//
	// Asserted from an independent literal rather than from the production table, which
	// is the only way a transposed row is caught. WORLD-ENTRY-002c §2.3 recovered
	// these by decrypting `default.charclass`; the distinct values are WALK 12-16 and
	// RUN 36/37/39/40/41/42/44.
	MODERN_TEST(Movement_ClassSpeedTableIsTheSixteenMeasuredRows)
	{
		struct Expected
		{
			Stats::CharClassIndex index;
			float                walk;
			float                run;
		};

		const Expected expected[Stats::kClassCount] = {
		    {Stats::CharClassIndex::BrawlerMale, 14.0f, 37.0f},
		    {Stats::CharClassIndex::SwordsmanMale, 12.0f, 36.0f},
		    {Stats::CharClassIndex::ArcherFemale, 16.0f, 42.0f},
		    {Stats::CharClassIndex::ShamanFemale, 13.0f, 40.0f},
		    {Stats::CharClassIndex::ExtremeMale, 14.0f, 39.0f},
		    {Stats::CharClassIndex::ExtremeFemale, 14.0f, 39.0f},
		    {Stats::CharClassIndex::BrawlerFemale, 14.0f, 37.0f},
		    {Stats::CharClassIndex::SwordsmanFemale, 12.0f, 36.0f},
		    {Stats::CharClassIndex::ArcherMale, 16.0f, 42.0f},
		    {Stats::CharClassIndex::ShamanMale, 12.0f, 39.0f},
		    {Stats::CharClassIndex::GunnerMale, 15.0f, 40.0f},
		    {Stats::CharClassIndex::GunnerFemale, 15.0f, 40.0f},
		    {Stats::CharClassIndex::AssassinMale, 12.0f, 44.0f},
		    {Stats::CharClassIndex::AssassinFemale, 12.0f, 44.0f},
		    {Stats::CharClassIndex::TrickerMale, 15.0f, 41.0f},
		    {Stats::CharClassIndex::TrickerFemale, 15.0f, 41.0f},
		};

		// One row per class, so the table and the enum cannot drift apart in size.
		CHECK_EQ(ClassSpeedCount(), static_cast<std::size_t>(Stats::kClassCount));
		CHECK_EQ(ClassSpeedTable() != nullptr, true);

		for (std::size_t i = 0; i < static_cast<std::size_t>(Stats::kClassCount); ++i)
		{
			const ClassMoveSpeed* row =
			    ClassSpeedFor(expected[i].index);
			if (row == nullptr)
			{
				CHECK(false);
				continue;
			}

			CHECK_NEAR(row->walkVelocity, expected[i].walk);
			CHECK_NEAR(row->runVelocity, expected[i].run);

			// The same row through the exposed table, so `ClassSpeedTable` cannot be a
			// second, divergent copy.
			CHECK_NEAR(ClassSpeedTable()[i].walkVelocity, expected[i].walk);
			CHECK_NEAR(ClassSpeedTable()[i].runVelocity, expected[i].run);
		}
	}

	// The wrong legacy constructor defaults are proven NOT to be in use.
	//
	// WORLD-ENTRY-002b assumed `cCONSTCLASS`'s CONSTRUCTOR DEFAULTS (GLogicData.h:118-119)
	// were the class speeds. 002c disproved it: RAN overwrites both fields from the
	// `.classconst` files at startup, so 12/34 never reach a running game - and they are
	// wrong for every one of the sixteen classes.
	//
	// Three independent ways to show it, because "the table looks right" is not the
	// same claim as "the wrong table is not being consulted somewhere":
	//
	//   1. no row is the {12, 34} pair,
	//   2. 34 is not in the run column AT ALL, so no row could have picked it up,
	//   3. the twelve that walk at 12 all run at 36 or 39 or 44 - never at 34.
	MODERN_TEST(Movement_TheTwelveAndThirtyFourConstructorDefaultsAreNotUsed)
	{
		// The constants are still NAMED, so a caller reaching for a fallback has to type
		// the word `Constructor` and see what it is.
		CHECK_NEAR(kLegacyConstructorWalkVelocity, 12.0f);
		CHECK_NEAR(kLegacyConstructorRunVelocity, 34.0f);

		std::size_t walkIsTwelve = 0;

		for (std::size_t i = 0; i < ClassSpeedCount(); ++i)
		{
			const ClassMoveSpeed& row = ClassSpeedTable()[i];

			// (1) The pair itself never appears.
			CHECK(!(Near(row.walkVelocity, kLegacyConstructorWalkVelocity) &&
			         Near(row.runVelocity, kLegacyConstructorRunVelocity)));

			// (2) 34 is not a run velocity in RAN's data either.
			CHECK(!Near(row.runVelocity, kLegacyConstructorRunVelocity));

			// (3) And the four distinct run velocities a twelve-walker actually has.
			if (Near(row.walkVelocity, 12.0f))
			{
				++walkIsTwelve;
				CHECK(Near(row.runVelocity, 36.0f) || Near(row.runVelocity, 39.0f) ||
				      Near(row.runVelocity, 44.0f));
			}

			// Running is always faster than walking, in every row.
			CHECK(row.runVelocity > row.walkVelocity);
		}

		// Twelve-walking classes exist in the data (Swordsman M/F, Shaman M,
		// Assassin M/F), which is what makes (1) and (3) non-vacuous.
		CHECK(walkIsTwelve > 0);

		// And the accessor agrees: a SwordsmanFemale - the row a 12/34 table would have
		// got exactly right on the walk and wrong on the run.
		float run = 0.0f;
		CHECK(TryGetBaseVelocity(Stats::CharClassIndex::SwordsmanFemale, true, run));
		CHECK_NEAR(run, 36.0f);
	}

	// Walk or run is chosen by the AUTHORITATIVE run flag, and an index outside the
	// sixteen has no answer rather than a default one.
	MODERN_TEST(Movement_BaseVelocityChoosesWalkOrRunAndRefusesAnUnknownClass)
	{
		float walk = 0.0f;
		float run  = 0.0f;

		for (std::size_t i = 0; i < static_cast<std::size_t>(Stats::kClassCount); ++i)
		{
			const Stats::CharClassIndex index = static_cast<Stats::CharClassIndex>(i);
			CHECK(TryGetBaseVelocity(index, false, walk));
			CHECK(TryGetBaseVelocity(index, true, run));

			const ClassMoveSpeed* row = ClassSpeedFor(index);
			CHECK(row != nullptr);
			if (row != nullptr)
			{
				CHECK_NEAR(walk, row->walkVelocity);
				CHECK_NEAR(run, row->runVelocity);
			}
			CHECK(run > walk);
		}

		// One past the end, and the widest value the enum's underlying type can carry.
		// A caller must not receive "very slow" for "unknown".
		walk = -1.0f;
		CHECK(!TryGetBaseVelocity(static_cast<Stats::CharClassIndex>(Stats::kClassCount), false,
		                          walk));
		CHECK_NEAR(walk, 0.0f);

		walk = -1.0f;
		CHECK(!TryGetBaseVelocity(static_cast<Stats::CharClassIndex>(0xFFu), true, walk));
		CHECK_NEAR(walk, 0.0f);

		CHECK(ClassSpeedFor(static_cast<Stats::CharClassIndex>(Stats::kClassCount)) == nullptr);
		CHECK(ClassSpeedFor(static_cast<Stats::CharClassIndex>(0xFFu)) == nullptr);
	}

	// =========================================================================
	// 3. The speed formula
	// =========================================================================

	// `GETMOVEVELO()` is a MULTIPLIER that starts at 1.0, not a bonus.
	//
	// So `speed = base * (1.0 + 0.0) == base`, and the default terms are an accurate
	// statement rather than a fabricated zero: 1.0 is `GLLogicExPC`'s own initial
	// `m_fSTATE_MOVE` (GLogixExPC.cpp:73) and 0.0 is RAN's own term for "nothing worn".
	// An implementation that treated the state term as additive-with-a-zero-default
	// would give `base * 0` for an unmodified character - no movement at all - which is
	// why this is asserted rather than assumed.
	MODERN_TEST(Movement_DefaultTermsMakeTheSpeedExactlyTheBaseVelocity)
	{
		const MoveVelocityTerms terms;

		// The defaults, named.
		CHECK_NEAR(terms.stateMultiplier, kDefaultStateMultiplier);
		CHECK_NEAR(terms.stateMultiplier, 1.0f);
		CHECK_NEAR(terms.moveItem, 0.0f);
		CHECK_NEAR(terms.itemDivisor, 0.0f);

		// And what they compute.
		CHECK_NEAR(MoveVelocity(37.0f, terms, false), 37.0f);
		CHECK_NEAR(MoveVelocity(37.0f, terms, true), 37.0f);

		// The run flag changes NOTHING here, because it already happened upstream in the
		// choice of `baseVelocity`. The parameter exists because RAN's takes it
		// (GLChar.cpp:4973), not because anything branches on it.
		CHECK_NEAR(MoveVelocity(16.0f, terms, true), MoveVelocity(16.0f, terms, false));

		// A multiplier is a multiplier: half is half, double is double.
		MoveVelocityTerms halved = terms;
		halved.stateMultiplier   = 0.5f;
		CHECK_NEAR(MoveVelocity(40.0f, halved, true), 20.0f);

		MoveVelocityTerms doubled = terms;
		doubled.stateMultiplier   = 2.0f;
		CHECK_NEAR(MoveVelocity(40.0f, doubled, true), 80.0f);
	}

	// The item term is divided by `fRUNVELO` WHETHER OR NOT the character is running.
	//
	// 002c §3.1 measured this, and it is the easiest thing in the formula to get
	// backwards: a provider that switched divisors with the run flag would disagree with
	// RAN for a WALKING character wearing a speed item, which is exactly the case the
	// two branches look interchangeable in.
	MODERN_TEST(Movement_ItemTermIsDividedByRunVelocityWhicheverWayTheCharacterIsMoving)
	{
		MoveVelocityTerms terms;
		terms.stateMultiplier = kDefaultStateMultiplier;
		terms.moveItem        = 36.0f; // an item worth "one second of running"
		terms.itemDivisor     = 36.0f; // fRUNVELO

		const float walking = MoveVelocity(14.0f, terms, false);
		const float running = MoveVelocity(37.0f, terms, true);

		// 36 / 36 == 1, so the multiplier becomes 2.0 in both cases.
		CHECK_NEAR(walking, 28.0f);
		CHECK_NEAR(running, 74.0f);

		// A half-strength item: 18 / 36 == 0.5.
		terms.moveItem = 18.0f;
		CHECK_NEAR(MoveVelocity(14.0f, terms, false), 21.0f);

		// A NEGATIVE item is arithmetically honoured rather than refused. RAN does no
		// validation on the term either, and a provider that clamped would be making up
		// a rule; what it must not do is silently skip the division.
		terms.moveItem = -36.0f;
		CHECK_NEAR(MoveVelocity(14.0f, terms, false), 0.0f);
	}

	// A zero divisor yields `base * stateMultiplier` rather than an infinity.
	//
	// The DEFAULT terms have a zero divisor on purpose - there is no item term to
	// divide - and dividing anyway would produce an infinity that turns every later
	// multiplication into a NaN one tick into the movement tick. The guarded form makes
	// the no-item case the arithmetic identity it should be.
	MODERN_TEST(Movement_AZeroItemDivisorLeavesTheStateMultiplierAlone)
	{
		MoveVelocityTerms terms;
		terms.moveItem    = 50.0f; // there IS an item
		terms.itemDivisor = 0.0f;  // and nothing to divide it by

		CHECK_NEAR(MoveVelocity(20.0f, terms, true), 20.0f);

		// The non-zero divisor on the same terms, for contrast: the guard is about the
		// divisor and not about the item.
		terms.itemDivisor = 25.0f;
		CHECK_NEAR(MoveVelocity(20.0f, terms, true), 60.0f);

		// And a zero ITEM with a zero divisor is the default case again, which is the
		// configuration every character in this server actually runs.
		terms.moveItem = 0.0f;
		CHECK_NEAR(MoveVelocity(42.0f, terms, false), 42.0f);
	}

	// =========================================================================
	// 4. Actor: creation and the spawn probe
	// =========================================================================

	// A null or unbuilt mesh is refused where it can still be acted on.
	//
	// Legacy accepted a null parent and then reported E_FAIL from every `Update`
	// forever after, so the failure was discovered at the worst possible moment instead
	// of at the call that caused it.
	MODERN_TEST(Movement_CreateRefusesANullOrUnbuiltMesh)
	{
		Actor actor;

		CHECK(actor.Create(nullptr, Vector3{}, Actor::kNoCell).IsError());
		CHECK_EQ(actor.CurrentCellId(), Actor::kNoCell);

		auto unbuilt = std::make_shared<Navigation::NavigationMesh>();
		CHECK(!unbuilt->Built());
		CHECK(actor.Create(unbuilt, Vector3{}, Actor::kNoCell).IsError());
		CHECK_EQ(actor.CurrentCellId(), Actor::kNoCell);
		CHECK(actor.Mesh() == nullptr);

		// An `Update` on an actor with no mesh is the legacy E_FAIL, still reported
		// rather than silently succeeding.
		CHECK(actor.Update(0.1f).IsError());
	}

	// The spawn probe is FIVE units, not the GOTO probe's ten.
	//
	// Using ten here would be a plausible-looking change that moves characters, which is
	// why the two are separate named constants (actor.cpp:74, :121 versus :295-296).
	//
	// Both branches are exercised: a spawn inside the ±5 window adopts the probe's hit
	// point, and a spawn far above the mesh falls back to `FindClosestCell` plus a snap.
	MODERN_TEST(Movement_SpawnSettleUsesTheFiveUnitProbeAndFallsBackToTheNearestCell)
	{
		CHECK_NEAR(kSpawnProbeHalfHeight, 5.0f);
		CHECK_NEAR(kSpawnProbeHalfHeight, Navigation::NavigationCell::kSpawnProbeHalfHeight);

		const std::shared_ptr<Navigation::NavigationMesh> mesh = Corridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		// Four units above the floor: inside the ±5 window, so the probe finds the floor
		// and the actor lands on it rather than staying where it was told to be.
		{
			Actor actor;
			CHECK(actor.Create(mesh, Vector3{5.0f, 4.0f, 3.0f}, Actor::kNoCell).IsOk());
			CHECK_NEAR(actor.Position().y, kCorridorY);
			CHECK_EQ(actor.CurrentCellId(), CellIdFor(0, 0));
		}

		// A hundred units up: the probe spans 105..95 and hits nothing, so the actor is
		// snapped to the nearest cell and its plane.
		{
			Actor actor;
			CHECK(actor.Create(mesh, Vector3{5.0f, 100.0f, 3.0f}, Actor::kNoCell).IsOk());
			CHECK_NEAR(actor.Position().x, 5.0f);
			CHECK_NEAR(actor.Position().y, kCorridorY);
			CHECK_NEAR(actor.Position().z, 3.0f);
		}

		// Off the corridor entirely: still attached, pulled to the nearest walkable
		// point. A character whose database position is off the mesh therefore spawns
		// somewhere walkable, which is what RAN does.
		{
			Actor actor;
			CHECK(actor.Create(mesh, Vector3{-50.0f, 100.0f, -50.0f}, Actor::kNoCell).IsOk());
			CHECK(actor.CurrentCellId() != Actor::kNoCell);
			CHECK(actor.Mesh()->GetCell(actor.CurrentCellId()) != nullptr);
			CHECK(actor.Mesh()->GetCell(actor.CurrentCellId())
			          ->IsPointInCellCollumn(actor.Position()));
		}
	}

	// =========================================================================
	// 5. Actor: the destination probe
	// =========================================================================

	// TEN units, and equal to `NavigationCell`'s own destination probe.
	//
	// The static_assert in Actor.h already ties the two names together; this states the
	// number, so a change to either is visible here as well.
	MODERN_TEST(Movement_TheGotoProbeIsTenUnitsAndIsNotTheSpawnProbe)
	{
		CHECK_NEAR(kGotoProbeHalfHeight, 10.0f);
		CHECK_NEAR(kGotoProbeHalfHeight, Navigation::NavigationCell::kDestinationProbeHalfHeight);

		// The two probes are different numbers, and saying so is the point of naming
		// both: swapping them would move characters.
		CHECK(kGotoProbeHalfHeight > kSpawnProbeHalfHeight);

		// And the unset speed is a named state, not an accident.
		Actor actor;
		CHECK_NEAR(actor.MaxSpeed(), kUnsetMaxSpeed);
		CHECK_NEAR(kUnsetMaxSpeed, 5.0f);
	}

	// `GotoDestination` returns whether the PROBE hit, which is not the same question
	// as "will the actor walk there".
	//
	// A miss is false and nothing happens. A hit is true even when the A* behind it
	// fails, because that is legacy's return value and MsgGoto's `bSucceed` is the
	// probe's - which is why a probe that hits and a pathfind that fails still
	// broadcasts a 3035.
	MODERN_TEST(Movement_GotoDestinationReportsTheProbeAndNotThePathfind)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = Corridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		Actor actor;
		CHECK(actor.Create(mesh, Vector3{5.0f, kCorridorY, 3.0f}, Actor::kNoCell).IsOk());

		// A miss: 500 units along X is off the corridor entirely, so the vertical span
		// finds no surface and legacy's block is an empty `if` - no path, no broadcast.
		CHECK(!actor.GotoDestination(Vector3{500.0f, 10.0f, 3.0f}, Vector3{500.0f, -10.0f, 3.0f}));
		CHECK(!actor.PathIsActive());
		CHECK_EQ(actor.WaypointsRemaining(), static_cast<std::size_t>(0));

		// A hit. The actor walks, so here the probe's answer and the pathfind's agree.
		CHECK(actor.GotoDestination(Vector3{25.0f, 10.0f, 3.0f}, Vector3{25.0f, -10.0f, 3.0f}));
		CHECK(actor.PathIsActive());
		CHECK(actor.WaypointsRemaining() > 0);

		// `TargetPosition` is the LAST waypoint - the goal - and NOT the next one. While
		// a path exists it is a real point, which is what makes it usable as the value
		// a broadcast would carry.
		CHECK_NEAR(actor.TargetPosition().x, 25.0f);
		CHECK(HasNextPosition(actor.TargetPosition()));
	}

	// The span the CALLER passes is what is probed, which is why the ±10 lives in
	// `NavigationCell` and is applied by the GOTO rule rather than by the actor.
	//
	// A destination nine and a half units above the floor is inside the window and a
	// destination ten and a half is not - the distinction is the whole of the probe, and
	// it is only observable through the span.
	MODERN_TEST(Movement_TheProbedSpanDecidesWhetherADestinationExists)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = Corridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		// 9.5 above the floor: the span 19.5 .. -0.5 crosses it.
		{
			Actor actor;
			CHECK(actor.Create(mesh, Vector3{5.0f, kCorridorY, 5.0f}, Actor::kNoCell).IsOk());
			CHECK(actor.GotoDestination(Vector3{22.0f, 19.5f, 5.0f}, Vector3{22.0f, -0.5f, 5.0f}));
			// The path goes to the PROBED point on the floor, not to the requested one.
			// GLCharMsg.cpp:315 broadcasts the raw request anyway; see GotoServiceTests.
			CHECK_NEAR(actor.TargetPosition().y, kCorridorY);
			CHECK_NEAR(actor.TargetPosition().x, 22.0f);
		}

		// 10.5 above: the span 20.5 .. 0.5 never reaches the floor, so nothing exists.
		{
			Actor actor;
			CHECK(actor.Create(mesh, Vector3{5.0f, kCorridorY, 5.0f}, Actor::kNoCell).IsOk());
			CHECK(!actor.GotoDestination(Vector3{22.0f, 20.5f, 5.0f}, Vector3{22.0f, 0.5f, 5.0f}));
			CHECK(!actor.PathIsActive());
		}

		// The same window works downward: 9.5 BELOW the floor still finds it.
		{
			Actor actor;
			CHECK(actor.Create(mesh, Vector3{5.0f, kCorridorY, 5.0f}, Actor::kNoCell).IsOk());
			CHECK(actor.GotoDestination(Vector3{22.0f, 0.5f, 5.0f}, Vector3{22.0f, -19.5f, 5.0f}));
			CHECK(actor.PathIsActive());
		}
	}

	// =========================================================================
	// 6. Actor: the step
	// =========================================================================

	// `max_distance = maxSpeed * elapsedSeconds`, with the elapsed time INJECTED.
	//
	// There is no fixed rate anywhere in the movement layer, and 002c §9 measured that
	// RAN has none either: `GLChar::FrameMove` receives `fElapsedTime` and the `0.020f`
	// compare at :294 is followed by a COMMENTED-OUT `return S_FALSE`.
	//
	// So total travel is a function of elapsed TIME, not of how many ticks happened.
	// Two quarters move exactly as far as one half - which is the whole property, and
	// the reason a unit test can prove it without sleeping.
	MODERN_TEST(Movement_StepDistanceIsMaxSpeedTimesInjectedElapsedTime)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = Corridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		constexpr float kSpeed = 10.0f;

		Actor whole;
		CHECK(StartWalking(whole, mesh, kSpeed).IsOk());
		CHECK(whole.Update(0.5f).IsOk());

		Actor halves;
		CHECK(StartWalking(halves, mesh, kSpeed).IsOk());
		CHECK(halves.Update(0.25f).IsOk());
		CHECK(halves.Update(0.25f).IsOk());

		// 10 * 0.5 == 5 units, and the start was x = 5.
		CHECK_NEAR(whole.Position().x, 10.0f);
		CHECK_NEAR(whole.MovedDist(), 5.0f);

		// Same distance, and the same place, from two ticks.
		CHECK_NEAR(halves.Position().x, 10.0f);
		CHECK_NEAR(halves.MovedDist(), 5.0f);

		// The distance really is the product, not a constant: four times the elapsed time
		// is four times the distance, from the same starting point.
		Actor quarter;
		CHECK(StartWalking(quarter, mesh, kSpeed).IsOk());
		CHECK(quarter.Update(0.125f).IsOk());
		CHECK_NEAR(quarter.MovedDist(), 1.25f);

		// And the speed is the other half of the product. A zero speed is a character
		// that cannot move, which is a real state (no configured provider) and not a
		// division by zero.
		Actor stopped;
		CHECK(StartWalking(stopped, mesh, 0.0f).IsOk());
		CHECK(stopped.Update(0.5f).IsOk());
		CHECK_NEAR(stopped.Position().x, 5.0f);
		CHECK_NEAR(stopped.MovedDist(), 0.0f);
		CHECK(stopped.PathIsActive());
	}

	// Arrival is at 0.01 units, and below it the waypoint is TAKEN rather than walked
	// to.
	//
	// The observable that distinguishes the two branches is `PathIsActive`, not the
	// final position: a destination five thousandths of a unit away is REACHED in one
	// tick, and a destination twenty thousandths away is not.
	MODERN_TEST(Movement_ArrivalThresholdIsAHundredthOfAUnit)
	{
		CHECK_NEAR(kArrivalThreshold, 0.01f);

		const std::shared_ptr<Navigation::NavigationMesh> mesh = Corridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		// 0.005 away: inside the threshold, so the actor is PLACED there and the path is
		// finished.
		{
			Actor actor;
			CHECK(actor.Create(mesh, Vector3{5.0f, kCorridorY, 3.0f}, Actor::kNoCell).IsOk());
			CHECK(actor.GotoCell(Vector3{5.005f, kCorridorY, 3.0f}, CellIdFor(0, 0)));
			actor.SetMaxSpeed(100.0f);

			CHECK(actor.Update(1.0f).IsOk());
			CHECK_NEAR(actor.Position().x, 5.005f);
			CHECK(!actor.PathIsActive());
			CHECK_EQ(actor.WaypointsRemaining(), static_cast<std::size_t>(0));
		}

		// 0.02 away: outside the threshold, so the actor WALKS it - and the same tick that
		// walks it is the tick that runs out of waypoints to consume, so the walk is not
		// over yet.
		{
			Actor actor;
			CHECK(actor.Create(mesh, Vector3{5.0f, kCorridorY, 3.0f}, Actor::kNoCell).IsOk());
			CHECK(actor.GotoCell(Vector3{5.02f, kCorridorY, 3.0f}, CellIdFor(0, 0)));
			actor.SetMaxSpeed(100.0f);

			CHECK(actor.Update(1.0f).IsOk());
			CHECK_NEAR(actor.Position().x, 5.02f);
			CHECK(actor.PathIsActive());
		}

		// `MovedTime` is the documented frame counter, not seconds: legacy's arrival
		// branch re-enters `Update` and each re-entry adds the elapsed time again
		// (actor.cpp:308). Both cases above consumed two logical frames in one tick,
		// which is why the counter reads two elapsed times rather than one.
	}


	// The arrival branch's OTHER observable is the frame counter.
	//
	// MOVEDTIME IS NOT SECONDS. Legacy's arrival branch re-enters Update, and each
	// re-entry adds the elapsed time again (actor.cpp:308), so one tick that consumes two
	// waypoints accumulates twice the elapsed time. This implementation reproduces that as
	// an explicit loop over logical frames rather than as recursion - identical
	// arithmetic - but the counter is still frames visited.
	MODERN_TEST(Movement_MovedTimeCountsLogicalFramesNotSeconds)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = Corridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		Actor actor;
		CHECK(StartWalking(actor, mesh, 10.0f).IsOk());

		// The path's first waypoint is where the actor already stands, so the first
		// logical frame is an ARRIVAL (distance zero) and the second is the move: two
		// frames, therefore two elapsed times, from one Update(0.25f) call.
		CHECK(actor.Update(0.25f).IsOk());
		CHECK_NEAR(actor.MovedTime(), 0.5f);
		CHECK_NEAR(actor.Position().x, 7.5f);

		// A second tick consumes ONE frame: the arrival already happened, so there is
		// nothing left to re-enter for. The counter therefore grows by one elapsed
		// time, not two - which is the frame count showing through.
		CHECK(actor.Update(0.25f).IsOk());
		CHECK_NEAR(actor.MovedTime(), 0.75f);

		// A character that never moved has visited no frames at all.
		Actor idle;
		CHECK(idle.Create(mesh, Vector3{5.0f, kCorridorY, 3.0f}, Actor::kNoCell).IsOk());
		CHECK(idle.Update(0.25f).IsOk());
		CHECK_NEAR(idle.MovedTime(), 0.0f);
		CHECK_NEAR(idle.MovedDist(), 0.0f);
	}

	// A full walk arrives, stops, and stops for the reason the caller can see.
	//
	// The corridor is twenty units long and the speed is ten a second, so twenty-one
	// tenths is enough with room to spare - and the loop is bounded so that a broken
	// arrival rule fails the case instead of hanging it.
	MODERN_TEST(Movement_AWalkArrivesAndClearsThePath)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = Corridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		Actor actor;
		CHECK(StartWalking(actor, mesh, 10.0f).IsOk());

		int ticks = 0;
		while (actor.PathIsActive() && ticks < 100)
		{
			CHECK(actor.Update(0.1f).IsOk());
			++ticks;
		}

		CHECK(!actor.PathIsActive());
		CHECK(ticks > 0);
		CHECK(ticks <= 100);

		// It arrived at the destination, on the floor, having travelled at least the
		// twenty units between the two points.
		CHECK_NEAR(actor.Position().x, 25.0f);
		CHECK_NEAR(actor.Position().z, 3.0f);
		CHECK_NEAR(actor.Position().y, kCorridorY);
		CHECK(actor.MovedDist() >= 20.0f);

		// The path is consumed, and the sentinel is back: `GLChar.cpp:6104` compares it
		// component-wise before trusting it.
		CHECK_EQ(actor.WaypointsRemaining(), static_cast<std::size_t>(0));
		CHECK(!HasNextPosition(actor.NextPosition()));
	}

	// =========================================================================
	// 7. Actor: the blocked step, and the 0.98 friction
	// =========================================================================

	// A step into a wall stops AT the wall, and the along-wall motion is scaled by
	// 0.98f.
	//
	// The nearby comment in legacy says 10%; the code says 2%, and 002d kept the code.
	// The friction is only observable on a motion with a component ALONG the wall -
	// perpendicular motion projects to nothing and the factor has nothing to scale - so
	// this is asserted by DIFFERENCE: two blocked motions whose requested along-wall
	// components differ by exactly one unit end up 0.98 apart. The point where each one
	// touches the wall cancels, because it is the same point.
	//
	// This is the resolver `Actor::Update` calls on every step (Actor.cpp:358),
	// exercised directly because a path built by A* or by line-of-sight never asks an
	// actor to walk THROUGH a wall: every waypoint it produces is reachable in a
	// straight line. The blocked case is therefore only constructible here.
	MODERN_TEST(Movement_BlockedMotionStopsAtTheWallAndLosesTwoPercent)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = Corridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		// Two five-hundred-unit motions along X from the same point, differing only in
		// their Z component - so they ask for the same wall, one unit apart along it.
		Vector3       low{500.0f, kCorridorY, 6.0f};
		std::uint32_t lowCell = 0;
		mesh->ResolveMotionOnMesh(Vector3{5.0f, kCorridorY, 5.0f}, CellIdFor(0, 0), low, &lowCell);

		Vector3       high{500.0f, kCorridorY, 7.0f};
		std::uint32_t highCell = 0;
		mesh->ResolveMotionOnMesh(Vector3{5.0f, kCorridorY, 5.0f}, CellIdFor(0, 0), high, &highCell);

		// Both stop at the corridor's far wall rather than travelling the five hundred
		// units they asked for. Twenty-nine and a bit, not five hundred.
		CHECK(low.x < kSpanX);
		CHECK(high.x < kSpanX);
		CHECK_WITHIN(low.x, kSpanX, 0.01f);
		CHECK_WITHIN(high.x, kSpanX, 0.01f);

		// The along-wall difference is 0.98 of the requested difference of 1.0. This is
		// the friction, isolated from everything else the resolver does.
		CHECK_WITHIN(high.z - low.z, 0.98f, 0.005f);
	}

	// An UNBLOCKED step is not reduced at all, which is the contrast that makes the
	// 0.98 above mean something: friction is a property of hitting a wall, not a tax on
	// every step.
	MODERN_TEST(Movement_AnUnblockedStepTravelsTheWholeRequestedDistance)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = Corridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		Vector3       end{25.0f, kCorridorY, 3.0f};
		std::uint32_t endCell = 0;
		mesh->ResolveMotionOnMesh(Vector3{5.0f, kCorridorY, 3.0f}, CellIdFor(0, 0), end,
		                          &endCell);

		CHECK_NEAR(end.x, 25.0f);
		CHECK_NEAR(end.z, 3.0f);
		CHECK_NEAR(end.y, kCorridorY);
	}

	// =========================================================================
	// 8. Actor: state, sentinels and teardown
	// =========================================================================

	// The "no valid next position" sentinel is FLT_MAX in all three components, and
	// `HasNextPosition` is the component-wise test `GLChar.cpp:6104` performs.
	//
	// `Vector3` has no `operator>`, which is why a caller cannot compare the sentinel
	// as a vector - and why the helper exists rather than being inlined at each site.
	MODERN_TEST(Movement_TheNoNextPositionSentinelIsFloatMaxInEveryComponent)
	{
		const Vector3 sentinel = NoNextPosition();

		CHECK(sentinel.x == std::numeric_limits<float>::max());
		CHECK(sentinel.y == std::numeric_limits<float>::max());
		CHECK(sentinel.z == std::numeric_limits<float>::max());

		CHECK(!HasNextPosition(sentinel));

		// Anything with a real coordinate is a next position, including the origin - the
		// test is per component, so a partly-sentinel value is a real one.
		CHECK(HasNextPosition(Vector3{}));
		CHECK(!HasNextPosition(Vector3{1.0f, std::numeric_limits<float>::max(), 2.0f}));
	}

	// A fresh actor holds the sentinel, `Stop` does not clear it, and `Detach` does.
	//
	// Legacy's `Stop` leaves `m_NextPosition` alone (actor.cpp:36-49 is `Release`, not
	// `Stop`), and that is preserved - a stale next position feeds the facing
	// computation at GLChar.cpp:6104, so a function whose only effect was to clear it
	// would be carrying the hazard rather than removing it.
	MODERN_TEST(Movement_StopClearsThePathButDetachClearsEverything)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = Corridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		Actor actor;
		CHECK(StartWalking(actor, mesh, 10.0f).IsOk());
		CHECK(actor.PathIsActive());
		CHECK(actor.Path().Size() > 0);

		actor.Stop();
		CHECK(!actor.PathIsActive());
		CHECK_EQ(actor.Path().Size(), static_cast<std::size_t>(0));
		CHECK_EQ(actor.WaypointsRemaining(), static_cast<std::size_t>(0));

		// The empty path has no last waypoint, so `TargetPosition` is the sentinel.
		CHECK(!HasNextPosition(actor.TargetPosition()));

		// The mesh is still bound: `Stop` is about the walk, not the world.
		CHECK(actor.Mesh() != nullptr);
		CHECK(actor.CurrentCellId() != Actor::kNoCell);

		actor.Detach();
		CHECK(actor.Mesh() == nullptr);
		CHECK_EQ(actor.CurrentCellId(), Actor::kNoCell);
		CHECK(actor.Position() == Vector3{});
		CHECK(!HasNextPosition(actor.NextPosition()));
	}

	// An actor with no path reports success from `Update`, because legacy does.
	//
	// It is the overwhelmingly common case - most characters are standing still - and
	// turning it into a failure would make every caller special-case it.
	MODERN_TEST(Movement_UpdateWithNoPathIsSuccessNotFailure)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = Corridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		Actor actor;
		CHECK(actor.Create(mesh, Vector3{5.0f, kCorridorY, 3.0f}, Actor::kNoCell).IsOk());
		CHECK(!actor.PathIsActive());

		for (int i = 0; i < 4; ++i)
		{
			CHECK(actor.Update(0.1f).IsOk());
		}
		CHECK_NEAR(actor.Position().x, 5.0f);
	}

	// One mesh, several actors.
	//
	// 002e measured that sixteen of the fifty-six distinct `.wld` files behind the
	// ninety-nine registered maps are named by MORE THAN ONE map identity, so "one mesh
	// per character" would build ninety-nine copies of fifty-six objects. Sharing is the
	// correct shape, and nothing mutable is written into the mesh.
	MODERN_TEST(Movement_OneSharedMeshServesManyActorsIndependently)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = Corridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		Actor first;
		Actor second;
		Actor third;

		CHECK(first.Create(mesh, Vector3{5.0f, kCorridorY, 3.0f}, Actor::kNoCell).IsOk());
		CHECK(second.Create(mesh, Vector3{15.0f, kCorridorY, 7.0f}, Actor::kNoCell).IsOk());
		CHECK(third.Create(mesh, Vector3{25.0f, kCorridorY, 3.0f}, Actor::kNoCell).IsOk());

		// The same pointer, not three copies of one mesh.
		CHECK(first.Mesh().get() == mesh.get());
		CHECK(second.Mesh().get() == mesh.get());
		CHECK(third.Mesh().get() == mesh.get());

		// Only the first is given a destination, and only the first moves.
		CHECK(StartWalking(first, mesh, 10.0f).IsOk());
		second.SetMaxSpeed(10.0f);
		third.SetMaxSpeed(10.0f);

		CHECK(first.Update(0.1f).IsOk());
		CHECK(second.Update(0.1f).IsOk());
		CHECK(third.Update(0.1f).IsOk());

		CHECK(first.Position().x > 5.0f);
		CHECK_NEAR(second.Position().x, 15.0f);
		CHECK_NEAR(third.Position().x, 25.0f);

		// Detaching one leaves the others bound and walking.
		first.Detach();
		CHECK(first.Mesh() == nullptr);
		CHECK(second.Mesh().get() == mesh.get());
		CHECK(second.Update(0.1f).IsOk());
	}

	// =========================================================================
	// 9. Navigation through the injectable map source
	// =========================================================================

	// The seam, used for what it exists for.
	//
	// A movement test that required a deployed ASURA client could not run on a machine
	// without one, which would leave the 60-unit rule, the vertical probe, arrival and
	// the wall slide untested in CI. `INavigationMapSource` is the one place that knows
	// how a map becomes a mesh, and a fixture that reads one map out of a table is
	// exactly what the interface was declared for.
	MODERN_TEST(Movement_NavigationArrivesThroughTheInjectedMapSource)
	{
		const std::shared_ptr<Navigation::NavigationMesh> mesh = Corridor();
		CHECK(mesh != nullptr);
		if (mesh == nullptr)
		{
			return;
		}

		ModernTests::MeshFixture::SingleMapSource source(mesh, 7u);

		// The map the fixture holds.
		const std::shared_ptr<const Navigation::NavigationMesh> found =
		    source.MeshForPackedMapId(7u);
		CHECK(found != nullptr);
		if (found != nullptr)
		{
			// SHARED, not copied - and immutable, so the pointer outlives the call.
			CHECK(found.get() == mesh.get());
		}

		// A map the fixture does not hold is a normal null answer, not an error, and
		// `Describe` says why - which is what lets an operator tell "this map has no
		// navigation" from "every map is unreachable".
		CHECK(source.MeshForPackedMapId(8u) == nullptr);
		CHECK(!source.Describe(7u).empty());
		CHECK(source.Describe(8u) != source.Describe(7u));

		// And it was consulted, so a caller can prove it is on the path at all.
		CHECK_EQ(source.calls, static_cast<std::size_t>(2));
	}

	// The production source over a registry that never loaded.
	//
	// This is the case that matters operationally: a source over a registry that failed
	// answers "no mesh" for EVERY map, which is indistinguishable from "every map is
	// unreachable" unless the load state is exposed. It is, and asserted here.
	MODERN_TEST(Movement_MapRegistrySourceNamesWhyItHasNoMesh)
	{
		Map::MapRegistry registry{std::string()};
		CHECK(!registry.Loaded());

		const MapRegistryMeshSource source{registry};

		CHECK(!source.RegistryLoaded());
		CHECK(source.MeshForPackedMapId(7u) == nullptr);

		// The reason names the stage. An empty string would leave an operator with a
		// bare "no".
		const std::string reason = source.Describe(7u);
		CHECK(!reason.empty());
		CHECK(reason.find("did not load") != std::string::npos);
	}
}
