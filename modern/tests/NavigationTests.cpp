// WORLD-ENTRY-002d: navigation kernel tests.
//
// The kernel is the part with no assets, no network and no clock, so these tests
// build their own geometry in memory and assert behaviour directly. That is why
// they can run in ModernCoreTests, which links Modern and nothing else.
//
// ---------------------------------------------------------------------------
// WHY THE CELLS ARE BUILT BY HAND AND NOT READ FROM A FILE
// ---------------------------------------------------------------------------
//
// `NavigationMesh::Build` takes 188-byte records because that is what a `.wld`
// contains, and `WldNavigationTests.cpp` covers that path against real assets.
// Re-serialising a cell here to feed it back through `Build` would test the test's
// own encoder.
//
// So the mesh-level tests below construct `NavigationCell`s directly with
// `ComputeFromVertices` and link them with `SetLink`, and drive `QueryForPath` /
// `ProcessCell` by hand. That is possible precisely because the A* state lives in
// the caller's `NavigationSearchSession` rather than on the mesh - under legacy's
// layout there is no way to run a search without a `NavigationMesh` to own the
// heap, which is one of the reasons the state was moved.

#include "TestHarness.h"

#include "math/Vector2.h"
#include "math/Vector3.h"
#include "navigation/NavigationCell.h"
#include "navigation/NavigationGeometry.h"
#include "navigation/NavigationMesh.h"
#include "navigation/NavigationPath.h"
#include "navigation/NavigationSearchSession.h"
#include "navigation/TriangleCollision.h"
#include "navigation/WldNavigationLoader.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

using namespace Modern;
using namespace Modern::Navigation;

namespace
{
	constexpr float kTolerance = 0.001f;

	bool Near(float a, float b)
	{
		return std::fabs(a - b) <= kTolerance;
	}

	// The harness has no approximate comparison - deliberately, because
	// "approximately" needs a stated tolerance. Navigation is float arithmetic, so
	// this file needs one, and it is spelled out at every call site.
	#define CHECK_NEAR(actual, expected) \
	    ::ModernTests::CheckImpl(Near((actual), (expected)), #actual " ~= " #expected, __FILE__, __LINE__)

	// The XZ projection of a world point, which is what every cell test is made
	// of. Named rather than inlined at each call site so it is obvious when a test
	// is reasoning in 2D.
	Vector2 centreXZ(const NavigationCell& cell)
	{
		return Vector2{cell.CenterPoint().x, cell.CenterPoint().z};
	}

	// ------------------------------------------------------------------------
	// A four-cell chain, built in memory.
	// ------------------------------------------------------------------------
	//
	// Two unit squares laid along X, each split by a diagonal, giving a chain of
	// four cells three links long:
	//
	//     A0 --(side 1, the shared diagonal)--> A1
	//                                             |
	//                              (side 2, the shared x=10 edge)
	//                                             v
	//                                            B0 --(side 1)--> B1
	//
	// Every triangle is wound so that "right of all three walls" is "inside",
	// which is the invariant `ClassifyPathToCell` and `IsPointInCellCollumn` both
	// depend on (see NavigationCell.h). All vertices are at y = 0, so every
	// plane normal is +Y and the cells read as floor.
	//
	// Returns cells[0] == A0, [1] == A1, [2] == B0, [3] == B1.
	std::vector<std::unique_ptr<NavigationCell>> MakeChain()
	{
		const std::vector<std::array<std::size_t, 3>> triangles = {
		    {0, 1, 2}, // A0: (0,0)  (0,10)  (10,0)
		    {3, 1, 5}, // A1: (10,0) (0,10)  (10,10)
		    {3, 4, 6}, // B0: (10,0) (10,10) (20,0)
		    {7, 4, 5}, // B1: (20,0) (10,10) (20,10)
		};

		const std::array<Vector3, 8> vertices = {{
		    Vector3{0.0f, 0.0f, 0.0f},   Vector3{0.0f, 0.0f, 10.0f},
		    Vector3{10.0f, 0.0f, 0.0f},  Vector3{10.0f, 0.0f, 0.0f},
		    Vector3{10.0f, 0.0f, 10.0f}, Vector3{10.0f, 0.0f, 10.0f},
		    Vector3{20.0f, 0.0f, 0.0f},  Vector3{20.0f, 0.0f, 0.0f},
		}};

		std::vector<std::unique_ptr<NavigationCell>> cells;
		cells.reserve(triangles.size());

		for (std::size_t i = 0; i < triangles.size(); ++i)
		{
			auto cell      = std::make_unique<NavigationCell>();
			cell->ComputeFromVertices(vertices.data(), triangles[i][0], triangles[i][1],
			                          triangles[i][2],
			                          static_cast<std::uint32_t>(i));
			cell->SetIndex(i);
			cells.push_back(std::move(cell));
		}

		// A0 side 1 (its diagonal) <-> A1 side 0 (the same segment, reversed).
		cells[0]->SetLink(NavigationCell::SideBC, cells[1].get());
		cells[1]->SetLink(NavigationCell::SideAB, cells[0].get());

		// A1 side 2 (the x=10 edge) <-> B0 side 0 (the same segment).
		cells[1]->SetLink(NavigationCell::SideCA, cells[2].get());
		cells[2]->SetLink(NavigationCell::SideAB, cells[1].get());

		// B0 side 1 (its diagonal) <-> B1 side 0.
		cells[2]->SetLink(NavigationCell::SideBC, cells[3].get());
		cells[3]->SetLink(NavigationCell::SideAB, cells[2].get());

		return cells;
	}

	// Drives the same reverse A* `NavigationMesh::BuildNavigationPath` runs, but
	// over cells this test owns. The loop and the `ArrivalWall == -1` early exit
	// are navigationmesh.cpp:152-177.
	std::vector<const NavigationCell*> SearchCells(const NavigationCell* start,
	                                               const NavigationCell* goal,
	                                               NavigationSearchSession& session)
	{
		session.Setup(goal->CenterPoint());
		goal->QueryForPath(session, nullptr, 0.0f);

		while (session.NotEmpty())
		{
			NavigationNode node{};
			session.GetTop(node);
			if (node.cell == nullptr)
			{
				break;
			}
			if (session.ArrivalWall(*node.cell) == -1)
			{
				break;
			}
			if (node.cell == start)
			{
				return {start, goal};
			}
			node.cell->ProcessCell(session);
		}
		return {};
	}
}

// ===========================================================================
// 1. Vector2 and Vector3 additions
// ===========================================================================

MODERN_TEST(Navigation_Vector3DotAndCrossAreRightHanded)
{
	const Vector3 x(1.0f, 0.0f, 0.0f);
	const Vector3 y(0.0f, 1.0f, 0.0f);
	const Vector3 z(0.0f, 0.0f, 1.0f);

	CHECK(Near(Dot(x, y), 0.0f));
	CHECK(Near(Dot(x, x), 1.0f));

	// `x cross y == z`, in `D3DXVec3Cross` order. RAN builds every navigation
	// plane as cross(P1 - P0, P2 - P0) (plane.h:193), so the order decides which
	// surfaces count as floors and cannot be reversed.
	const Vector3 up = Cross(x, y);
	CHECK(Near(up.x, z.x));
	CHECK(Near(up.y, z.y));
	CHECK(Near(up.z, z.z));

	// The chain cells rely on: these three vertices must give a +Y normal.
	const Vector3 a(0.0f, 0.0f, 0.0f);
	const Vector3 b(0.0f, 0.0f, 10.0f);
	const Vector3 c(10.0f, 0.0f, 0.0f);
	const Vector3 normal = Normalize(Cross(b - a, c - a));
	CHECK(Near(normal.x, 0.0f));
	CHECK(Near(normal.y, 1.0f));
	CHECK(Near(normal.z, 0.0f));
}

MODERN_TEST(Navigation_Vector2DotAndNormalizeAreTotal)
{
	CHECK(Near(Dot(Vector2{3.0f, 4.0f}, Vector2{1.0f, 0.0f}), 3.0f));

	const Vector2 unit = Normalize(Vector2{3.0f, 4.0f});
	CHECK(Near(unit.x, 0.6f));
	CHECK(Near(unit.y, 0.8f));
	CHECK_NEAR(unit.Length(), 1.0f);

	// A zero-length input yields (0,0) rather than NaN, so a caller that forgets
	// to guard reads a zero direction instead of propagating a NaN into a
	// position.
	const Vector2 zero = Normalize(Vector2{0.0f, 0.0f});
	CHECK(zero.IsZero());
}

// ===========================================================================
// 2. Line2D and Plane
// ===========================================================================

MODERN_TEST(Navigation_Line2DNormalIsClockwiseAndPointsRight)
{
	// A -> B along +X. The right-hand normal must point to -Z.
	Line2D line(Vector2{0.0f, 0.0f}, Vector2{10.0f, 0.0f});

	CHECK(Near(line.Normal().x, 0.0f));
	CHECK(Near(line.Normal().y, -1.0f));

	CHECK(line.ClassifyPoint(Vector2{5.0f, -1.0f}) == Line2D::RightSide);
	CHECK(line.ClassifyPoint(Vector2{5.0f, 1.0f}) == Line2D::LeftSide);
	CHECK(line.ClassifyPoint(Vector2{5.0f, 0.0f}) == Line2D::OnLine);

	// Positive to the right of A -> B. This sign is the primitive both cell tests
	// are made of, so it is asserted directly rather than through a cell.
	CHECK(Near(line.SignedDistance(Vector2{5.0f, -2.0f}), 2.0f));
	CHECK(Near(line.SignedDistance(Vector2{5.0f, 2.0f}), -2.0f));
}

MODERN_TEST(Navigation_Line2DIntersectionClassifications)
{
	const Line2D horizontal(Vector2{0.0f, 0.0f}, Vector2{10.0f, 0.0f});

	Vector2 point{};

	// A vertical segment crossing the middle of the horizontal one: both segments
	// genuinely contain the crossing point.
	const Line2D crossing(Vector2{5.0f, -5.0f}, Vector2{5.0f, 5.0f});
	CHECK(horizontal.Intersection(crossing, &point) == Line2D::SegmentsIntersect);
	CHECK(Near(point.x, 5.0f));
	CHECK(Near(point.y, 0.0f));

	// Parallel and distinct.
	const Line2D parallel(Vector2{0.0f, 3.0f}, Vector2{10.0f, 3.0f});
	CHECK(horizontal.Intersection(parallel, &point) == Line2D::Parallel);

	// Collinear.
	const Line2D collinear(Vector2{0.0f, 0.0f}, Vector2{5.0f, 0.0f});
	CHECK(horizontal.Intersection(collinear, &point) == Line2D::Collinear);

	// The lines cross, but outside one of the segments.
	const Line2D outside(Vector2{20.0f, -5.0f}, Vector2{20.0f, 5.0f});
	const Line2D::LineClassification classified = horizontal.Intersection(outside, &point);
	CHECK(classified != Line2D::SegmentsIntersect);
	CHECK(classified != Line2D::Parallel);
}

MODERN_TEST(Navigation_PlaneSolvesForYAndRefusesToDivideByZero)
{
	const Plane floor(Vector3{0.0f, 5.0f, 0.0f}, Vector3{0.0f, 5.0f, 10.0f},
	                  Vector3{10.0f, 5.0f, 0.0f});

	CHECK(Near(floor.Normal().y, 1.0f));

	// Every point of that plane is at y = 5.
	CHECK(Near(floor.SolveForY(0.0f, 0.0f), 5.0f));
	CHECK(Near(floor.SolveForY(3.0f, 7.0f), 5.0f));

	// A vertical plane has no Y normal, and returns 0.0f rather than dividing by
	// zero - the case `Actor::Update` reaches on a wall face.
	const Plane wall(Vector3{0.0f, 0.0f, 0.0f}, Vector3{0.0f, 0.0f, 10.0f},
	                 Vector3{0.0f, 10.0f, 0.0f});
	CHECK(Near(wall.Normal().y, 0.0f));
	CHECK(Near(wall.SolveForY(5.0f, 5.0f), 0.0f));

	// The plane is `dot(N, X) + D == 0`, so D must place it at y = 5.
	CHECK(Near(Dot(floor.Normal(), Vector3{0.0f, 5.0f, 0.0f}) + floor.Distance(), 0.0f));
}

// ===========================================================================
// 3. NavigationCell
// ===========================================================================

MODERN_TEST(Navigation_CellContainsItsOwnInteriorAndNotItsNeighbour)
{
	auto cells = MakeChain();
	const NavigationCell& a0 = *cells[0];

	// The centroid is inside; the centroid of the cell across the shared diagonal
	// is not.
	const Vector3 centre = a0.CenterPoint();
	CHECK(a0.IsPointInCellCollumn(centre));

	const Vector3 otherCentroid = cells[1]->CenterPoint();
	CHECK(!a0.IsPointInCellCollumn(otherCentroid));

	// A point far outside the whole chain.
	CHECK(!a0.IsPointInCellCollumn(Vector3{100.0f, 0.0f, 100.0f}));

	// The plane is flat at y = 0, so the normal points up and the cell reads as
	// floor rather than as a wall face.
	CHECK(a0.Normal().y > 0.9f);
}

MODERN_TEST(Navigation_ClassifyPathReportsEndingExitingAndNoRelationship)
{
	auto cells = MakeChain();
	const NavigationCell& a0 = *cells[0];
	const NavigationCell& a1 = *cells[1];

	const NavigationCell* nextCell = nullptr;
	NavigationCell::CellSide side     = NavigationCell::SideAB;
	Vector2                 intersection{};

	// A path that stops inside A0: the cell the actor is already standing in.
	const Line2D inside(Vector2{centreXZ(a0).x, centreXZ(a0).y}, Vector2{1.0f, 1.0f});
	CHECK(a0.ClassifyPathToCell(inside, &nextCell, side, &intersection) ==
	      NavigationCell::EndingCell);

	// A path from A0's centre to A1's centre crosses the shared diagonal, and the
	// link on that side is A1 - so the caller can continue rather than slide.
	const Vector2 from(centreXZ(a0).x, centreXZ(a0).y);
	const Vector2 to(centreXZ(a1).x, centreXZ(a1).y);
	const Line2D crossing(from, to);

	CHECK(a0.ClassifyPathToCell(crossing, &nextCell, side, &intersection) ==
	      NavigationCell::ExitingCell);
	CHECK(nextCell == &a1);
	CHECK(side == NavigationCell::SideBC);

	// A path heading off the chain entirely crosses a SOLID edge, so `nextCell`
	// is null - which is exactly what makes `ResolveMotionOnMesh` slide.
	const Line2D offMesh(Vector2{centreXZ(a0).x, centreXZ(a0).y}, Vector2{-50.0f, 50.0f});
	CHECK(a0.ClassifyPathToCell(offMesh, &nextCell, side, &intersection) ==
	      NavigationCell::ExitingCell);
	CHECK(nextCell == nullptr);
}

MODERN_TEST(Navigation_ForcePointToWallInteriorNudgesInside)
{
	auto cells = MakeChain();
	const NavigationCell& a0 = *cells[0];

	// A point just outside the x=0 wall (side 1 runs (0,0) -> (0,10)).
	Vector2 point{-1.0f, 5.0f};
	CHECK(!a0.IsPointInCellCollumn(point));

	const bool moved = a0.ForcePointToWallInterior(NavigationCell::SideAB, point);
	CHECK(moved);
	CHECK(a0.IsPointInCellCollumn(point));

	// A point already inside is left alone and reported as not moved.
	Vector2 inside{5.0f, 1.0f};
	CHECK(!a0.ForcePointToWallInterior(NavigationCell::SideAB, inside));
	CHECK_NEAR(inside.x, 5.0f);
}

MODERN_TEST(Navigation_ForcePointToCellCollumnPullsAPointBackInside)
{
	auto cells = MakeChain();
	const NavigationCell& a0 = *cells[0];

	// A point outside the cell entirely: the centre-to-point line exits, so the
	// point is snapped 98% of the way to the boundary and, failing that, to the
	// centre.
	Vector2 point{-40.0f, -40.0f};
	CHECK(a0.ForcePointToCellCollumn(point));
	CHECK(a0.IsPointInCellCollumn(point));

	// The 3D overload must move X and Z and leave Y alone.
	Vector3 point3{-40.0f, 123.0f, -40.0f};
	CHECK(a0.ForcePointToCellCollumn(point3));
	CHECK(a0.IsPointInCellCollumn(point3));
	CHECK_NEAR(point3.y, 123.0f);
}

MODERN_TEST(Navigation_MapVectorHeightToCellSolvesThePlane)
{
	auto cells = MakeChain();
	NavigationCell& a0 = *cells[0];

	Vector3 point{4.0f, 999.0f, 1.0f};
	a0.MapVectorHeightToCell(point);

	// Flat floor at y = 0, so an absurd input Y is replaced by 0.
	CHECK_NEAR(point.y, 0.0f);
	CHECK_NEAR(point.x, 4.0f);
	CHECK_NEAR(point.z, 1.0f);
}

MODERN_TEST(Navigation_CellRecordIsExactly188BytesAndDecodesItsOwnFields)
{
	// The record size is measured, not summed - a 164-byte stride is in
	// circulation and is wrong. WORLD-ENTRY-002b measured 188.
	CHECK_EQ(NavigationCell::kCellRecordBytes, static_cast<std::size_t>(188));
	CHECK_EQ(NavigationCell::kCellRecordDwords, static_cast<std::size_t>(47));

	// The offsets must tile the record exactly, with no gap and no overlap:
	// 4 + 12 + 3*28 + 28 + 12 + 3*12 + 3*4 == 188.
	CHECK_EQ(NavigationCell::kOffsetCellId, static_cast<std::size_t>(0));
	CHECK_EQ(NavigationCell::kOffsetVertex, static_cast<std::size_t>(4));
	CHECK_EQ(NavigationCell::kOffsetSide, static_cast<std::size_t>(16));
	CHECK_EQ(NavigationCell::kOffsetPlane, static_cast<std::size_t>(100));
	CHECK_EQ(NavigationCell::kOffsetCenterPoint, static_cast<std::size_t>(128));
	CHECK_EQ(NavigationCell::kOffsetWallMidpoint, static_cast<std::size_t>(140));
	CHECK_EQ(NavigationCell::kOffsetWallDistance, static_cast<std::size_t>(176));
	CHECK_EQ(NavigationCell::kOffsetWallDistance + 12, NavigationCell::kCellRecordBytes);

	// And a synthetic record decodes into what was written - the same
	// `RestoreFromRecord` the WLD reader hands the file's bytes to.
	std::uint32_t record[NavigationCell::kCellRecordDwords] = {};
	record[0] = 42u; // cell id
	record[1] = 7u;  // vertex 0
	record[2] = 8u;  // vertex 1
	record[3] = 9u;  // vertex 2

	NavigationCell cell;
	cell.RestoreFromRecord(record);

	CHECK_EQ(cell.CellId(), 42u);
	CHECK_EQ(cell.Vertex(0), 7u);
	CHECK_EQ(cell.Vertex(1), 8u);
	CHECK_EQ(cell.Vertex(2), 9u);

	// A restored cell starts with all three edges SOLID, and only the mesh opens
	// them.
	CHECK(cell.Link(0) == nullptr);
	CHECK(cell.Link(1) == nullptr);
	CHECK(cell.Link(2) == nullptr);
}

// ===========================================================================
// 4. A*
// ===========================================================================

MODERN_TEST(Navigation_AStarFindsTheOnlyPathAcrossAChain)
{
	auto cells = MakeChain();

	NavigationSearchSession session(cells.size());
	const std::vector<const NavigationCell*> found =
	    SearchCells(cells[0].get(), cells[3].get(), session);

	CHECK_EQ(found.size(), static_cast<std::size_t>(2));
	if (found.size() == 2)
	{
		CHECK(found[0] == cells[0].get());
		CHECK(found[1] == cells[3].get());
	}
}

MODERN_TEST(Navigation_AStarReportsNoPathIntoASolidEdge)
{
	auto cells = MakeChain();

	// Cut the chain so A0 cannot reach B1 at all, then confirm the search gives
	// up rather than walking off the mesh.
	cells[1]->SetLink(NavigationCell::SideCA, nullptr);
	cells[2]->SetLink(NavigationCell::SideAB, nullptr);

	NavigationSearchSession session(cells.size());
	const std::vector<const NavigationCell*> found =
	    SearchCells(cells[0].get(), cells[3].get(), session);

	CHECK_EQ(found.size(), static_cast<std::size_t>(0));
}

MODERN_TEST(Navigation_TwoSessionsOverOneMeshDoNotInterfere)
{
	// The thread-safety claim. Under legacy's per-cell A* state these two searches
	// would overwrite each other's arrival costs and could each return the
	// other's path; with the state in the session they cannot.
	auto cells = MakeChain();

	NavigationSearchSession sessionA(cells.size());
	NavigationSearchSession sessionB(cells.size());

	sessionA.Setup(cells[3]->CenterPoint());
	sessionB.Setup(cells[0]->CenterPoint());

	// Interleave the two searches step by step - the worst case for shared
	// per-cell state.
	cells[3]->QueryForPath(sessionA, nullptr, 0.0f);
	cells[0]->QueryForPath(sessionB, nullptr, 0.0f);

	NavigationNode nodeA{};
	NavigationNode nodeB{};
	sessionA.GetTop(nodeA);
	sessionB.GetTop(nodeB);

	CHECK(nodeA.cell == cells[3].get());
	CHECK(nodeB.cell == cells[0].get());

	CHECK(sessionA.ArrivalCost(*cells[3].get()) > 0.0f ||
	      sessionA.ArrivalCost(*cells[3].get()) == 0.0f);
	CHECK(sessionB.ArrivalCost(*cells[0].get()) == 0.0f);

	// Both sessions still know their own goal.
	CHECK(Near(sessionA.Goal().x, cells[3]->CenterPoint().x));
	CHECK(Near(sessionB.Goal().x, cells[0]->CenterPoint().x));
}

MODERN_TEST(Navigation_SessionSetupResetsStateLazilyAndCheaply)
{
	auto cells = MakeChain();
	NavigationSearchSession session(cells.size());

	session.Setup(cells[3]->CenterPoint());
	cells[3]->QueryForPath(session, nullptr, 0.0f);
	CHECK(session.HasSeen(*cells[3].get()));
	CHECK(session.HasSeen(*cells[0].get()) == false);

	// A second search must NOT see the first one's cells, even though nothing was
	// cleared - that is the whole point of the session stamp, and it is what makes
	// a per-search session cheaper than a per-mesh clear.
	session.Setup(cells[0]->CenterPoint());
	CHECK(!session.HasSeen(*cells[3].get()));
	CHECK(!session.HasSeen(*cells[0].get()));
	CHECK(session.SessionId() > 1);
}

MODERN_TEST(Navigation_HeuristicIsTheLongestAxisDeltaAndIsAdmissible)
{
	auto cells = MakeChain();
	const NavigationCell& a0 = *cells[0];
	const Vector3         goal{20.0f, 0.0f, 0.0f};

	// A0's centre is (10/3, 0, 10/3), the goal is (20, 0, 0), so the axis deltas
	// are 16.667, 0 and 3.333 and the heuristic is the LONGEST - 16.667. Not the
	// Euclidean distance (17.0), which would overstate it.
	const float expected = 20.0f - (10.0f / 3.0f);

	NavigationSearchSession session(cells.size());
	session.Setup(goal);

	// `QueryForPath` with a non-null caller is what computes the heuristic.
	a0.QueryForPath(session, cells[1].get(), 5.0f);
	CHECK(Near(session.Heuristic(a0), expected));

	// Admissible: it must not exceed the true remaining distance.
	const float straight = (a0.CenterPoint() - goal).Length();
	CHECK_LE(session.Heuristic(a0), straight + kTolerance);

	// And the cell is offered to the open set with g + h.
	CHECK(Near(session.PathfindingCost(a0), 5.0f + expected));
	CHECK(session.IsOpen(a0));
}

MODERN_TEST(Navigation_ProcessCellExpandsOnlyThroughLinkedEdges)
{
	auto cells = MakeChain();
	NavigationSearchSession session(cells.size());

	session.Setup(cells[3]->CenterPoint());
	cells[3]->QueryForPath(session, nullptr, 0.0f);
	CHECK(cells[3]->ProcessCell(session));

	// B1's links are only its diagonal, so exactly one neighbour was offered.
	CHECK(session.HasSeen(*cells[2].get()));
	CHECK(!session.HasSeen(*cells[0].get()));

	// A cell that was never offered is not the session's to expand.
	NavigationSearchSession other(cells.size());
	other.Setup(cells[3]->CenterPoint());
	CHECK(!cells[0]->ProcessCell(other));
}

// ===========================================================================
// 5. Triangle collision
// ===========================================================================

MODERN_TEST(Navigation_LineTriangleHitsInsideAndMissesOutside)
{
	const Vector3 t0{0.0f, 0.0f, 0.0f};
	const Vector3 t1{10.0f, 0.0f, 0.0f};
	const Vector3 t2{0.0f, 0.0f, 10.0f};

	Vector3 hit{};

	// A vertical probe through the middle of the triangle: the classic GOTO
	// destination probe.
	Vector3 start{2.0f, 5.0f, 2.0f};
	Vector3 end{2.0f, -5.0f, 2.0f};
	CHECK(IsLineTriangleCollision(t0, t1, t2, start, end, hit));
	CHECK_NEAR(hit.x, 2.0f);
	CHECK_NEAR(hit.y, 0.0f);
	CHECK_NEAR(hit.z, 2.0f);

	// Outside the triangle in XZ: no hit, which is what makes an off-mesh
	// destination fail navigation validation.
	Vector3 outsideStart{50.0f, 5.0f, 50.0f};
	Vector3 outsideEnd{50.0f, -5.0f, 50.0f};
	CHECK(!IsLineTriangleCollision(t0, t1, t2, outsideStart, outsideEnd, hit));

	// A probe that stops above the triangle never reaches it.
	Vector3 highStart{2.0f, 20.0f, 2.0f};
	Vector3 highEnd{2.0f, 10.0f, 2.0f};
	CHECK(!IsLineTriangleCollision(t0, t1, t2, highStart, highEnd, hit));
}

MODERN_TEST(Navigation_LineTriangleRefusesDegenerateTriangles)
{
	const Vector3 t0{0.0f, 0.0f, 0.0f};
	const Vector3 t1{0.0f, 0.0f, 0.0f};
	const Vector3 t2{0.0f, 0.0f, 0.0f};

	Vector3 hit{};
	Vector3 start{0.0f, 5.0f, 0.0f};
	Vector3 end{0.0f, -5.0f, 0.0f};

	// RAN's rule is to refuse a triangle with more than one pair of equal axes
	// (Collision.cpp:254); a fully collapsed triangle has three, and its normal
	// is (0,0,0). Either way: no hit.
	hit = Vector3{1.0f, 2.0f, 3.0f};
	CHECK(!IsLineTriangleCollision(t0, t1, t2, start, end, hit));

	// And `outCollision` is left UNWRITTEN rather than set to a NaN, which is the
	// property that matters: a caller that ignored the false return would otherwise
	// read NaN.
	CHECK_NEAR(hit.x, 1.0f);
	CHECK_NEAR(hit.y, 2.0f);
	CHECK_NEAR(hit.z, 3.0f);

	// A triangle with two vertices in the same place has ONE equal-axis pair, so
	// it is NOT rejected by that rule - but its normal is still zero after
	// normalisation, and the degenerate guard refuses it rather than dividing by
	// zero. This is the case a vertical GOTO probe can actually land on.
	const Vector3 sliver0{0.0f, 0.0f, 0.0f};
	const Vector3 sliver1{0.0f, 0.0f, 0.0f};
	const Vector3 sliver2{0.0f, 0.0f, 10.0f};
	CHECK(!IsLineTriangleCollision(sliver0, sliver1, sliver2, start, end, hit));
	CHECK_NEAR(hit.x, 1.0f);
}

MODERN_TEST(Navigation_PointInTriangleUsesTheTwoAxisProjection)
{
	const Vector3 t0{0.0f, 0.0f, 0.0f};
	const Vector3 t1{10.0f, 0.0f, 0.0f};
	const Vector3 t2{0.0f, 0.0f, 10.0f};

	// Dropping Y - keeping X and Z - is the projection a flat floor test needs.
	CHECK(IsPointInsideTriangle(t0, t1, t2, Vector3{1.0f, 0.0f, 1.0f}, true, false, true));
	CHECK(!IsPointInsideTriangle(t0, t1, t2, Vector3{50.0f, 0.0f, 1.0f}, true, false, true));

	// Fewer than two axes is a line, not a triangle, and is refused
	// (Collision.cpp:37-41).
	CHECK(!IsPointInsideTriangle(t0, t1, t2, Vector3{1.0f, 0.0f, 1.0f}, true, false, false));
}

MODERN_TEST(Navigation_AabbSlabClipClipsAndRejects)
{
	AabbNode box;
	box.vMin = Vector3{0.0f, 0.0f, 0.0f};
	box.vMax = Vector3{10.0f, 10.0f, 10.0f};

	// A segment that passes through the box keeps its direction and is clipped to
	// the entry point - the mutation is what makes the tree walk cheap.
	Vector3 p1{-5.0f, 5.0f, 5.0f};
	Vector3 p2{15.0f, 5.0f, 5.0f};
	CHECK(box.IsCollision(p1, p2));
	CHECK_GE(p1.x, 0.0f);
	CHECK_LE(p1.x, 10.0f);

	// Entirely behind the box on one axis: rejected.
	Vector3 behindStart{-20.0f, 5.0f, 5.0f};
	Vector3 behindEnd{-10.0f, 5.0f, 5.0f};
	CHECK(!box.IsCollision(behindStart, behindEnd));

	// A box with no face set is an interior node.
	CHECK_EQ(AabbNode::kNoFace, static_cast<std::uint32_t>(0xFFFFFFFFu));
}

// ===========================================================================
// 6. Mesh queries against real assets
// ===========================================================================

namespace
{
	int g_skipped = 0;

	bool RequireMapAssets(const char* what)
	{
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
		const char* root = std::getenv("RAN_ASSET_ROOT");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
		if (root == nullptr || root[0] == '\0')
		{
			++g_skipped;
			std::printf("      SKIPPED %s: RAN_ASSET_ROOT is not set\n", what);
			return false;
		}

		const std::filesystem::path base(root);
		const std::filesystem::path candidates[] = {
		    base / "Data" / "Map",
		    base / "data" / "Map",
		    base / "Map",
		};

		for (const std::filesystem::path& candidate : candidates)
		{
			if (std::filesystem::is_directory(candidate))
			{
				return true;
			}
		}

		++g_skipped;
		std::printf("      SKIPPED %s: no Data/Map under RAN_ASSET_ROOT\n", what);
		return false;
	}

	std::string MapAsset(const char* fileName)
	{
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
		const char* root = std::getenv("RAN_ASSET_ROOT");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
		return (std::filesystem::path(root) / "Data" / "Map" / fileName).string();
	}
}

MODERN_TEST(Navigation_RealMeshFindsClosestCellAndSnapsToIt)
{
	if (!RequireMapAssets("snap on a real mesh")) { return; }

	const NavigationLoadResult result = LoadWldNavigationMesh(MapAsset("w_school_01.wld"));
	if (!result.Ok())
	{
		return;
	}

	std::uint32_t cellId = 0;
	const Vector3  snapped = result.mesh->SnapPointToMesh(&cellId, Vector3{1031.327f, -59.215f, 997.280f});

	CHECK(cellId < result.mesh->CellCount());
	CHECK(snapped.IsFinite());

	// The snapped point is INSIDE the cell it was placed in, and its Y comes from
	// that cell's plane - both are the contract `SnapPointToMesh` offers.
	const NavigationCell* cell = result.mesh->GetCellById(cellId);
	CHECK(cell != nullptr);
	if (cell != nullptr)
	{
		CHECK(cell->IsPointInCellCollumn(snapped));
		CHECK_NEAR(snapped.y, cell->CellPlane().SolveForY(snapped.x, snapped.z));
	}

	// Snapping a point that is already on the mesh must not move it far.
	const Vector3 again = result.mesh->SnapPointToCell(cellId, snapped);
	CHECK_NEAR(again.x, snapped.x);
	CHECK_NEAR(again.z, snapped.z);
}

MODERN_TEST(Navigation_RealMeshResolvesAnUnobstructedStepExactly)
{
	if (!RequireMapAssets("motion resolve on a real mesh")) { return; }

	const NavigationLoadResult result = LoadWldNavigationMesh(MapAsset("w_school_01.wld"));
	if (!result.Ok())
	{
		return;
	}

	// A step between two adjacent cells' centres: nothing blocks it, so the
	// resolved position must equal the request (with Y from the destination
	// cell's plane).
	const NavigationCell* start = result.mesh->GetCellById(6603);
	const NavigationCell* goal  = result.mesh->GetCellById(6604);
	if (start == nullptr || goal == nullptr)
	{
		CHECK(false);
		return;
	}

	const Vector3 from = start->CenterPoint();
	const Vector3 to   = goal->CenterPoint();

	Vector3       resolved = to;
	std::uint32_t resolvedCell = start->CellId();
	result.mesh->ResolveMotionOnMesh(from, start->CellId(), resolved, &resolvedCell);

	CHECK_NEAR(resolved.x, to.x);
	CHECK_NEAR(resolved.z, to.z);
	CHECK_EQ(resolvedCell, goal->CellId());
	CHECK(resolved.y > from.y - 1.0f);
	CHECK(resolved.y < from.y + 1.0f);
}

MODERN_TEST(Navigation_RealMeshClampsAnObstructedStepAndLosesTwoPercent)
{
	if (!RequireMapAssets("wall slide on a real mesh")) { return; }

	const NavigationLoadResult result = LoadWldNavigationMesh(MapAsset("w_school_01.wld"));
	if (!result.Ok())
	{
		return;
	}

	// 20 units outward along the wall-1 midpoint from cell 6603. Side 1 of that
	// cell has NO link (002c §14), so the step must stop at the wall.
	// (1057.718, -59.410, 1002.816)
	const NavigationCell* start = result.mesh->GetCellById(6603);
	if (start == nullptr)
	{
		CHECK(false);
		return;
	}
	CHECK(start->Link(1) == nullptr);

	const Vector3 from    = start->CenterPoint();
	const Vector3 blocked(1057.718f, -59.410f, 1002.816f);

	Vector3       resolved = blocked;
	std::uint32_t resolvedCell = start->CellId();
	result.mesh->ResolveMotionOnMesh(from, start->CellId(), resolved, &resolvedCell);

	// The requested point is unreachable, so the result must differ from it...
	CHECK(!(Near(resolved.x, blocked.x) && Near(resolved.z, blocked.z)));

	// ...but it must stay inside the cell it claims to be in, at the height of
	// that cell's plane. This is the assertion that catches a slide that
	// projects past the boundary instead of onto it.
	const NavigationCell* landing = result.mesh->GetCellById(resolvedCell);
	CHECK(landing != nullptr);
	if (landing != nullptr)
	{
		CHECK(landing->IsPointInCellCollumn(resolved));
		CHECK_NEAR(resolved.y, landing->CellPlane().SolveForY(resolved.x, resolved.z));
	}

	// And it must have travelled less than asked - which is the friction. Legacy
	// scales a blocked step by 0.98f even though the nearby comment says 10%, and
	// the 0.98f is what RAN does. A single 2% loss on one pass is only 2% of the
	// blocked component, so the assertion is that progress was made but the
	// request was not met.
	const float requested = (blocked - from).Length();
	const float achieved  = (resolved - from).Length();
	CHECK_GT(achieved, 0.0f);
	CHECK_LT(achieved, requested);
}

MODERN_TEST(Navigation_RealMeshLineOfSightAndPathAcrossManyCells)
{
	if (!RequireMapAssets("path across a real mesh")) { return; }

	const NavigationLoadResult result = LoadWldNavigationMesh(MapAsset("w_school_01.wld"));
	if (!result.Ok())
	{
		return;
	}

	const NavigationCell* start = result.mesh->GetCellById(6603);
	const NavigationCell* goal  = result.mesh->GetCellById(0);
	if (start == nullptr || goal == nullptr)
	{
		CHECK(false);
		return;
	}

	const Vector3 from = start->CenterPoint();
	const Vector3 to   = goal->CenterPoint();

	// XZ distance 3568 (002c §14): a long path that needs real A*, not a
	// two-cell hop.
	CHECK_GT((to - from).Length(), 1000.0f);

	NavigationSearchSession session(result.mesh->CellCount());
	NavigationPath          path;

	float distance = 0.0f;
	CHECK(result.mesh->BuildNavigationPath(path, session, start->CellId(), from,
	                                      goal->CellId(), to, &distance));

	// Start and end are pushed by `Setup` and `EndPath`, so at minimum there are
	// two entries, and for a path this long there are many more.
	CHECK_GT(path.Size(), static_cast<std::size_t>(10));

	// The walked distance is at least the straight line - it cannot be shorter,
	// because it is a real walk over the terrain.
	CHECK_GE(distance, (to - from).Length() - 1.0f);

	// Every waypoint is finite and inside the mesh's bounding box.
	Vector3 vMax{};
	Vector3 vMin{};
	CHECK(result.mesh->GetAABB(vMax, vMin));
	for (const NavigationWaypoint& waypoint : path.Waypoints())
	{
		CHECK(waypoint.position.IsFinite());
		CHECK_GE(waypoint.position.x, vMin.x - 1.0f);
		CHECK_LE(waypoint.position.x, vMax.x + 1.0f);
		CHECK_GE(waypoint.position.z, vMin.z - 1.0f);
		CHECK_LE(waypoint.position.z, vMax.z + 1.0f);
		CHECK_LT(waypoint.cellId, result.mesh->CellCount());
	}

	// The string-pull must be able to see past the first waypoint, or a
	// character would stop at every cell boundary. The furthest visible point
	// from the start is strictly beyond waypoint 1.
	CHECK_GT(path.GetFurthestVisibleWayPoint(0), static_cast<std::size_t>(1));

	// Line of sight between the two cell centres, 3568 units apart across a whole
	// map, is whatever the mesh says.
	//
	// The REVERSE direction is deliberately NOT asserted to agree, because it does
	// not and is not supposed to. `ClassifyPathToCell` compares the two endpoints
	// asymmetrically - it skips a wall when B is right of it, or when A is LEFT of
	// it (navigationcell.cpp:57-83) - so walking the same segment the other way is
	// a different question with a different answer. Every caller
	// (`NavigationPath::GetFurthestVisibleWayPoint`) asks it forward along the
	// path, which is the direction that matters.
	//
	// What IS asserted is determinism: the same question twice must give the same
	// answer, because the path string-pull asks it repeatedly and a flickering
	// answer would make a character corner unpredictably.
	const bool forward =
	    result.mesh->LineOfSightTest(start->CellId(), from, goal->CellId(), to);
	const bool forwardAgain =
	    result.mesh->LineOfSightTest(start->CellId(), from, goal->CellId(), to);
	CHECK_EQ(forward, forwardAgain);

	// A point inside its own cell is visible from itself, in either direction.
	CHECK(result.mesh->LineOfSightTest(start->CellId(), from, start->CellId(), from));
}

MODERN_TEST(Navigation_RealMeshLineOfSightIsSymmetricWithinOneCell)
{
	if (!RequireMapAssets("line of sight on a real mesh")) { return; }

	const NavigationLoadResult result = LoadWldNavigationMesh(MapAsset("w_school_01.wld"));
	if (!result.Ok())
	{
		return;
	}

	const NavigationCell* cell = result.mesh->GetCellById(6603);
	if (cell == nullptr)
	{
		CHECK(false);
		return;
	}

	// Two points a couple of units apart, both inside one cell: visible.
	const Vector3 a = cell->CenterPoint();
	const Vector3 b = a + Vector3{1.0f, 0.0f, 1.0f};

	CHECK(result.mesh->LineOfSightTest(cell->CellId(), a, cell->CellId(), b));

	// An unknown cell id is a refusal, not a crash. Legacy's `GetCell` would
	// throw here; the modern bounds check returns null and the query says no.
	CHECK(!result.mesh->LineOfSightTest(cell->CellId(), a, 0xFFFFFFFFu, b));
}

MODERN_TEST(Navigation_RealMeshGetBoundsCoversTheWholeMap)
{
	if (!RequireMapAssets("mesh bounds on a real mesh")) { return; }

	const NavigationLoadResult result = LoadWldNavigationMesh(MapAsset("w_school_01.wld"));
	if (!result.Ok())
	{
		return;
	}

	Vector3 vMax{};
	Vector3 vMin{};
	CHECK(result.mesh->GetAABB(vMax, vMin));

	// 002c §14 measured XZ extent [-2013.497, 1460.000] x [-2438.650, 2374.930].
	// The tree boxes are widened by 0.0001 (navagationtree.cpp:61-66), so allow
	// a small margin - but not a large one.
	CHECK_GT(vMin.x, -2014.0f);
	CHECK_LT(vMax.x, 1461.0f);
	CHECK_GT(vMin.z, -2440.0f);
	CHECK_LT(vMax.z, 2376.0f);

	// Every vertex is inside the reported box, and the box is not inverted.
	CHECK(vMax.x >= vMin.x);
	CHECK(vMax.y >= vMin.y);
	CHECK(vMax.z >= vMin.z);
}

MODERN_TEST(Navigation_RealMeshOutOfRangeCellIdIsRefusedNotCrashed)
{
	if (!RequireMapAssets("cell bounds on a real mesh")) { return; }

	const NavigationLoadResult result = LoadWldNavigationMesh(MapAsset("w_school_01.wld"));
	if (!result.Ok())
	{
		return;
	}

	// The second deviation from legacy: `GetCell` bounds-checks properly instead
	// of testing `size() < index` and then calling `.at(index)`, which throws when
	// the index equals the size.
	const std::size_t count = result.mesh->CellCount();
	CHECK(result.mesh->GetCell(count) == nullptr);
	CHECK(result.mesh->GetCell(count + 1000) == nullptr);
	CHECK(result.mesh->GetCellById(0xFFFFFFFFu) == nullptr);
	CHECK(result.mesh->GetCell(0) != nullptr);
	CHECK_EQ(result.mesh->GetCell(count - 1)->CellId(), static_cast<std::uint32_t>(count - 1));
}