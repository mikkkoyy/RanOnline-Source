#pragma once

// WORLD-ENTRY-002d: one triangle of the RAN navigation mesh.
//
// Transcribed from legacy/Lib_Engine/NaviMesh/navigationcell.h and
// navigationcell.cpp. Three things about it are worth stating before the code,
// because each is the kind of detail that produces a movement system that looks
// right and walks through walls.
//
// ---------------------------------------------------------------------------
// THE CELL IS A CLOCKWISE TRIANGLE AND EVERY TEST DEPENDS ON IT
// ---------------------------------------------------------------------------
//
// `ComputeCellData` builds the three walls as AB, BC and CA from the vertices in
// the order the file stores them (navigationcell.h:210-212), and RAN's meshes are
// baked clockwise in XZ. `ClassifyPathToCell` therefore treats "right of this
// wall" as "inside" (navigationcell.cpp:57, :83), and `IsPointInCellCollumn`
// counts walls a point is NOT left of and requires all three
// (navigationcell.h:361-369). Both assume the winding; neither checks it.
// Reversing the winding would invert both tests silently, so the loader asserts
// the invariant it can - that `CellID` equals the array index - and leaves the
// winding to the baked data, exactly as legacy does.
//
// ---------------------------------------------------------------------------
// THE ON-DISK CELL IS 188 BYTES AND EVERY FIELD IS BAKED
// ---------------------------------------------------------------------------
//
//     offset   0   m_CellID        DWORD
//     offset   4   m_Vertex[3]     3 x DWORD, indices into the mesh vertex pool
//     offset  16   m_Side[3]       3 x Line2D, 28 bytes each
//     offset 100   m_CellPlane     Plane, 28 bytes
//     offset 128   m_CenterPoint   D3DXVECTOR3
//     offset 140   m_WallMidpoint[3] 3 x D3DXVECTOR3
//     offset 176   m_WallDistance[3] 3 x float
//
// (NavigationSaveLoad.cpp:109-131.) Every one of those except m_Vertex and
// m_CellID is DERIVED data - the centre point, the wall midpoints, the wall
// distances and the sides are all functions of the three vertices. RAN still
// stores them, and still READS them back rather than recomputing
// (NavigationSaveLoad.cpp:121-131: `LoadFile` restores `m_Side`, `m_CellPlane`,
// `m_CenterPoint`, `m_WallMidpoint` and `m_WallDistance` and never calls
// `ComputeCellData`). This type follows that: the loader restores the baked
// fields verbatim, and `ComputeFromVertices` exists only for the case where a
// cell is built from raw vertices.
//
// That is a deliberate compatibility choice, not an optimisation. A recomputed
// wall midpoint differs from the baked one by floating-point rounding, and
// `BuildNavigationPath` puts those midpoints into the waypoint list
// (navigationmesh.cpp:209). A path built from slightly different waypoints walks
// slightly differently for the rest of the character's life.
//
// ---------------------------------------------------------------------------
// A CELL IS IMMUTABLE ONCE LOADED - AND IN LEGACY IT IS NOT
// ---------------------------------------------------------------------------
//
// This is the one place where the modern kernel deliberately does NOT match
// legacy, and the reason is concurrency.
//
// In RAN the A* bookkeeping lives on the cell: `m_SessionID`, `m_ArrivalCost`,
// `m_Heuristic`, `m_Open` and `m_ArrivalWall` are members
// (navigationcell.h:117-121), and `NavigationCell::QueryForPath` and
// `ProcessCell` read and write them in place (navigationcell.cpp:281-397). They
// are reset LAZILY: a cell whose stored `m_SessionID` is older than the mesh's
// current session treats itself as unseen, so a search costs no setup pass over
// the whole mesh. The whole scheme works because a field server runs one
// character at a time.
//
// A modern server does not. The movement service in WORLD-ENTRY-002d is
// documented as running on a worker thread per Field connection, and the
// navigation mesh is loaded once and SHARED between Fields - two characters on
// two Fields in the same map path-find concurrently. With legacy's layout they
// would overwrite each other's arrival costs and produce a path that is neither
// character's.
//
// So the search state moved OUT of the cell and into `NavigationSearchSession`
// (NavigationSearchSession.h), which is a per-search object with parallel arrays
// indexed by `NavigationCell::Index()`. The lazy reset is preserved by keeping
// the session id per cell - just in the session's array rather than in the
// cell. The arithmetic, the unrolled wall costs, the id-vs-pointer discrepancy
// between the two `QueryForPath` branches, and the resulting path are all
// unchanged; only the storage moved.
//
// Consequence, and it is the point: a loaded `NavigationCell` is now read-only.
// `NavigationMesh` can be shared across threads with no lock, because a
// navigation query mutates nothing on it.

#include "navigation/NavigationGeometry.h"

#include <cstddef>
#include <cstdint>

namespace Modern::Navigation
{
	class NavigationSearchSession;

	// A navigation cell: one triangle and three walls.
	//
	// Holds a POINTER to its three links rather than indices, because
	// `NavigationMesh::ResolveMotionOnMesh` walks links in a loop
	// (navigationmesh.cpp:291-309) and index arithmetic in that loop is where a
	// subtle off-by-one would hide.
	//
	// After `RestoreFromRecord` and the link pass, only `SetIndex` and `SetLink`
	// are legal writes. See the header note on immutability.
	class NavigationCell
	{
	public:
		enum CellVertex
		{
			VertA = 0,
			VertB,
			VertC,
		};

		enum CellSide
		{
			SideAB = 0,
			SideBC,
			SideCA,
		};

		enum PathResult
		{
			// The path does not cross this cell at all.
			NoRelationship,
			// The path ends inside this cell.
			EndingCell,
			// The path leaves through `side`; `nextCell` is the cell beyond it, or
			// nullptr when that edge is solid.
			ExitingCell,
		};

		// Sentinel for "this edge is solid", used in the link id table the WLD
		// reader produces.
		//
		// A null `Link()` is already how the kernel says "solid", because
		// `ClassifyPathToCell` hands the caller a null next cell and the caller
		// slides along the wall (navigationmesh.cpp:291-309). The sentinel is what
		// the FILE stores: `NavigationSaveLoad.cpp:33-46` writes a `BOOL` presence
		// flag per side and only writes an id when it is set.
		//
		// It lives here rather than in the reader because it is a property of a
		// cell's edge, and because `NavigationMesh::Build` needs the same value -
		// one definition, so the two cannot drift.
		static constexpr std::uint32_t kNoLink = 0xFFFFFFFFu;

		// The vertical window `GLChar::MsgGoto` probes before it will build a path.
		//
		// legacy/Lib_Client/G-Logic/GLCharMsg.cpp:293-297:
		//
		//     m_actorMove.GotoLocation( m_TargetID.vPos + D3DXVECTOR3(0,+10,0),
		//                             m_TargetID.vPos + D3DXVECTOR3(0,-10,0) );
		//
		// so the segment runs from target.y+10 down to target.y-10: a 20-unit span
		// centred on the requested Y. If it hits nothing, `bSucceed` is FALSE, no
		// path is built and NO 3035 goes out (GLCharMsg.cpp:299-319).
		static constexpr float kDestinationProbeHalfHeight = 10.0f;

		// The epsilon `Actor::Create`/`SetPosition` uses to settle a spawn onto the
		// floor (actor.cpp:73-75, :120-122). Kept as a named constant because it is
		// NOT the same epsilon as the GOTO probe.
		static constexpr float kSpawnProbeHalfHeight = 5.0f;

		// ---- the on-disk record ----------------------------------------------
		//
		// `NavigationCell::SaveFile` writes exactly this many bytes
		// (NavigationSaveLoad.cpp:109-119): 4 + 12 + 3*28 + 28 + 12 + 3*12 + 3*4.
		//
		// MEASURED, not summed: WORLD-ENTRY-002b compiled a reconstruction of
		// `Line2D` and found it is 28 bytes, not the 20 that a naive sum of three
		// D3DXVECTOR2 gives. A 164-byte stride is also in circulation and is WRONG -
		// it makes 423 of 424 cell ids mismatch their index on a real map.
		//
		// Declared BEFORE the methods that name it, because a member's own type in
		// a function PARAMETER is not a complete-class context.
		static constexpr std::size_t kCellRecordBytes = 188;
		static constexpr std::size_t kCellRecordDwords = kCellRecordBytes / 4;

		// Field offsets inside the record, for the loader and for the tests that
		// assert the loader reads the right bytes.
		static constexpr std::size_t kOffsetCellId       = 0;
		static constexpr std::size_t kOffsetVertex       = 4;   // 3 x DWORD
		static constexpr std::size_t kOffsetSide         = 16;  // 3 x Line2D (28 each)
		static constexpr std::size_t kOffsetPlane        = 100; // Plane (28)
		static constexpr std::size_t kOffsetCenterPoint  = 128;
		static constexpr std::size_t kOffsetWallMidpoint = 140; // 3 x Vector3
		static constexpr std::size_t kOffsetWallDistance = 176; // 3 x float

		NavigationCell() = default;

		// Restores a cell from the 188 bytes `NavigationCell::LoadFile` reads.
		//
		// `links` is filled afterwards by NavigationMesh, because a link is a
		// pointer into the mesh and the mesh owns the cells.
		//
		// Leaves all three links SOLID (nullptr). That is correct for a fresh
		// cell: `BuildNavigationLinks` opens only the edges it can find a shared
		// vertex pair across, and everything else is a wall.
		void RestoreFromRecord(const std::uint32_t (&record)[kCellRecordDwords]);

		// Recomputes the derived fields from the mesh's vertex pool.
		//
		// Used only when a cell is built from raw vertices. NOT used by the loader -
		// see the header note on baked data.
		void ComputeFromVertices(const Vector3* naviVertex, std::uint32_t pointA,
		                        std::uint32_t pointB, std::uint32_t pointC,
		                        std::uint32_t cellId);

		// ---- queries ---------------------------------------------------------

		// Classifies a 2D path against this cell.
		//
		// `nextCell` is written on ExitingCell and is nullptr for a solid edge.
		// `intersectionPoint` is written whenever legacy writes it, which is when
		// the two segments actually intersect.
		PathResult ClassifyPathToCell(const Line2D& motionPath,
		                               const NavigationCell** nextCell, CellSide& side,
		                               Vector2* intersectionPoint) const noexcept;

		// Turns a path that hit a wall into one that runs ALONG it.
		//
		// navigationcell.cpp:110-143. This is the `ResolveMotionOnMesh` slide, and
		// it is a projection onto the wall direction, not a reflection - the motion
		// keeps its component along the wall and loses the component into it.
		void ProjectPathOnCellWall(CellSide sideNumber, Line2D& motionPath) const noexcept;

		// Solves Y for this cell's plane at the point's X and Z.
		void MapVectorHeightToCell(Vector3& motionPoint) const noexcept
		{
			motionPoint.y = m_cellPlane.SolveForY(motionPoint.x, motionPoint.z);
		}

		bool IsPointInCellCollumn(const Vector3& testPoint) const noexcept
		{
			return IsPointInCellCollumn(Vector2{testPoint.x, testPoint.z});
		}

		bool IsPointInCellCollumn(const Vector2& testPoint) const noexcept
		{
			// "In" means right of, or exactly on, ALL THREE walls.
			int interiorCount = 0;
			for (int i = 0; i < 3; ++i)
			{
				if (m_side[i].ClassifyPoint(testPoint) != Line2D::LeftSide)
				{
					++interiorCount;
				}
			}
			return interiorCount == 3;
		}

		// Pulls a point back inside the cell, 2% short of the boundary.
		//
		// navigationcell.cpp:201-241. The 0.98f here is the SAME constant that makes
		// a blocked step shrink in `ResolveMotionOnMesh`; RAN uses it in both places
		// and it is 0.98f in both, despite the nearby comment in navigationmesh.cpp:305
		// claiming 10%.
		bool ForcePointToCellCollumn(Vector2& testPoint) const noexcept;
		bool ForcePointToCellCollumn(Vector3& testPoint) const noexcept;

		// Nudges a point to the interior side of one wall.
		//
		// navigationcell.cpp:151-172.
		bool ForcePointToWallInterior(CellSide sideNumber, Vector2& testPoint) const noexcept;
		bool ForcePointToWallInterior(CellSide sideNumber, Vector3& testPoint) const noexcept;

		// ---- A*, with the state living in the session -----------------------
		//
		// Both are const and both write into `session`. See the header note on why
		// the A* bookkeeping is not a cell member.

		// Expands this cell's neighbours into the session's open set.
		//
		// The per-side costs are hand-unrolled from navigationcell.cpp:290-307
		// rather than folded into a loop, because the three cases use three
		// DIFFERENT `m_WallDistance` indices. The `abs(i - m_ArrivalWall)` formula
		// that RAN had here is commented out in legacy (navigationcell.cpp:281-288)
		// and replaced by these, so reproducing the loop instead of the unrolling
		// would reproduce the wrong costs.
		bool ProcessCell(NavigationSearchSession& session) const noexcept;

		// Offers this cell to the session's open set, given the cell we would be
		// entering from.
		bool QueryForPath(NavigationSearchSession& session, const NavigationCell* caller,
		                  float arrivalCost) const noexcept;

		// ---- accessors --------------------------------------------------------

		std::uint32_t CellId() const noexcept { return m_cellId; }
		std::uint32_t Vertex(int index) const noexcept { return m_vertex[index]; }
		const Vector3& CenterPoint() const noexcept { return m_centerPoint; }
		const Vector3& Normal() const noexcept { return m_cellPlane.Normal(); }
		const Plane& CellPlane() const noexcept { return m_cellPlane; }
		Vector3 WallMidpoint(int side) const noexcept { return m_wallMidpoint[side]; }
		float WallDistance(int side) const noexcept { return m_wallDistance[side]; }
		const NavigationCell* Link(int side) const noexcept { return m_link[side]; }

		// The cell's slot in the mesh's arrays, and its slot in a search
		// session's per-cell arrays. Assigned once by the loader.
		std::size_t Index() const noexcept { return m_index; }
		void        SetIndex(std::size_t index) noexcept { m_index = index; }

		// Link assignment is a loader/builder step, not a query-time one.
		void SetLink(CellSide side, const NavigationCell* cell) noexcept
		{
			m_link[side] = cell;
		}

	private:
		// The A* estimate: the LONGEST axis delta to the goal, not the Euclidean
		// distance (navigationcell.cpp:419-427). An admissible heuristic matters
		// here - a non-admissible one makes A* return a path that is not shortest,
		// and the path becomes the character's waypoint list for the rest of the
		// walk.
		float ComputeHeuristic(const Vector3& goal) const noexcept;

		Plane   m_cellPlane{};
		Vector3 m_centerPoint{};
		Line2D  m_side[3]{};
		std::uint32_t m_vertex[3]{};
		std::uint32_t m_cellId  = 0;
		std::size_t   m_index   = 0;

		// A null link denotes a SOLID edge - the wall an actor slides along.
		//
		// Const pointers: a cell can only ever point at another cell, never be
		// modified through the link.
		const NavigationCell* m_link[3]{};

		Vector3 m_wallMidpoint[3]{};
		float   m_wallDistance[3]{};
	};

	static_assert(sizeof(float) == 4, "the 188-byte cell record assumes 32-bit floats");
}