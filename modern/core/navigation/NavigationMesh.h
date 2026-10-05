#pragma once

// WORLD-ENTRY-002d: the loaded navigation mesh and every query navigation makes.
//
// Transcribed from legacy/Lib_Engine/NaviMesh/navigationmesh.h,
// navigationmesh.cpp and navagationtree.cpp.
//
// ---------------------------------------------------------------------------
// IMMUTABLE ONCE BUILT, WHICH IS THE POINT OF THE WHOLE ARRANGEMENT
// ---------------------------------------------------------------------------
//
// A built `NavigationMesh` is read-only. Every query below is `const` and
// allocates nothing that outlives the call except a path.
//
// That is not how RAN's is. RAN keeps `m_PathSession` and a `NavigationHeap`
// member on the mesh and resets them per search (navigationmesh.h:95-96), and
// keeps the per-cell A* state on the cells; both are unsafe the moment two
// searches overlap. Here the session is owned by the CALLER
// (`NavigationSearchSession`) and the per-cell state lives in it, so:
//
//   * one `NavigationMesh` is loaded once and shared by every Field in a map,
//   * each Field's movement runs on its own thread with its own session,
//   * no lock appears anywhere in this file.
//
// ---------------------------------------------------------------------------
// WHAT IS NOT HERE, AND WHY
// ---------------------------------------------------------------------------
//
// Three legacy members are absent, all for the same reason - nothing in a
// headless server reaches them:
//
//   * `CreateNaviMesh` / `AddCell` / `SetNaviVertexIndexData` (navigationmesh.cpp:626-965)
//     build a mesh by walking a D3DX `ID3DXMesh` out of a `DxFrame` and
//     transforming its vertices by `matCombined`. A server has no D3DX device
//     and no frame graph. The navigation arrives already baked in the `.wld`
//     file instead, so `Build` reads that.
//
//     One behaviour from that path IS carried over, because the baked file
//     depends on it: every vertex is quantised to a millimetre before use -
//     `long(v * 1000) / 1000` (navigationmesh.cpp:894-905). `Build` applies the
//     same rounding to the file's vertices, so a coordinate means the same thing
//     here as it did when the mesh was baked.
//
//   * `LinkCells` / `CheckLink` / `RequestLink` (navigationmesh.cpp:393-457,
//     :920-972) discover links by comparing cells pairwise. The `.wld` file
//     stores the links explicitly (NavigationSaveLoad.cpp:28-47), and
//     `LoadFile` reads them rather than rebuilding them
//     (NavigationSaveLoad.cpp:76-96). Reading them is what we do; recomputing
//     them would be slower and could disagree with the file.
//
//   * `CheckIntegrity`, `GotoErrorPosition`, `CreateVBIB`, `Render`, `Update`
//     write a debug log file, jump the debug camera, and draw wireframe
//     triangles. All rendering or debugging.
//
// What IS here that is worth noticing:
//
//   * `MakeAABBTree` and its helpers, including the two places legacy's version
//     compares the wrong variable (see the .cpp) - because the tree decides
//     which cells the link pass and the collision test ever look at, so a
//     "cleaned up" tree would produce different walls.
//
//   * `IsCollision`, which is what finds the floor under a GOTO destination.
//
//   * `ResolveMotionOnMesh`, `BuildNavigationPath`, `LineOfSightTest`,
//     `FindClosestCell` and the two snap functions.

#include "math/Vector3.h"
#include "navigation/NavigationCell.h"
#include "navigation/NavigationPath.h"
#include "navigation/NavigationSearchSession.h"
#include "navigation/TriangleCollision.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace Modern::Navigation
{
	class NavigationMesh
	{
	public:
		// Sentinel for "this edge is solid", used in the link id array.
		//
		// RAN stores the same information as a `BOOL` per side and only writes
		// the id when it is true (NavigationSaveLoad.cpp:33-46); the reader
		// collapses that into this one value.
		static constexpr std::uint32_t kNoLink = 0xFFFFFFFFu;

		NavigationMesh() = default;

		// Builds from the navigation block of a `.wld` file.
		//
		// `cellData` is `cellCount` records of `NavigationCell::kCellRecordBytes`
		// each, in file order. `linkIds` is `cellCount * 3` cell ids, three per
		// cell in SIDE_AB, SIDE_BC, SIDE_CA order, with `kNoLink` for a solid
		// edge - exactly the information `LoadFile` reconstructs from the
		// `bExist`/id pairs.
		//
		// Returns false and leaves the mesh empty if a record's baked `m_CellID`
		// does not equal its index. That check is not decoration: it is the only
		// invariant that makes the id-keyed comparisons inside `QueryForPath`
		// (navigationcell.cpp:339-354) sound, and WORLD-ENTRY-002b measured a
		// wrong record stride producing exactly that mismatch on 423 of 424 cells.
		bool Build(const std::vector<Vector3>& vertices, const std::uint8_t* cellData,
		           std::size_t cellCount, const std::vector<std::uint32_t>& linkIds);

		bool  Built() const noexcept { return m_built; }
		std::size_t CellCount() const noexcept { return m_cells.size(); }
		std::size_t VertexCount() const noexcept { return m_vertices.size(); }

		// nullptr when out of range.
		//
		// RAN's version tests `size() < index` and then calls `.at(index)`
		// (navigationmesh.h:141-144), so an index equal to the size throws
		// instead of returning null. That is a latent crash on a bad cell id
		// arriving off the wire, so this one bounds-checks properly. Nothing
		// depends on the old behaviour: every caller in legacy already null-checks.
		const NavigationCell* GetCell(std::size_t index) const noexcept
		{
			return index < m_cells.size() ? m_cells[index].get() : nullptr;
		}

		// Same lookup by the baked id rather than the array index.
		const NavigationCell* GetCellById(std::uint32_t cellId) const noexcept
		{
			return cellId < m_byId.size() ? m_byId[cellId] : nullptr;
		}

		// ---- snapping ---------------------------------------------------------

		// Pulls a point into `cellId` if it is outside, then solves its Y on that
		// cell's plane. navigationmesh.cpp:29-42.
		Vector3 SnapPointToCell(std::uint32_t cellId, const Vector3& point) const;

		// Finds the nearest cell, then snaps. navigationmesh.cpp:44-50.
		Vector3 SnapPointToMesh(std::uint32_t* cellOutId, const Vector3& point) const;

		// The cell whose column contains the point and whose floor is nearest to
		// it; failing that, the cell whose wall the centre-to-point line crosses
		// closest. navigationmesh.cpp:52-125.
		//
		// A LINEAR SCAN over every cell. It is O(cells) per call and RAN calls it
		// on every GOTO and every spawn, which is affordable there and is also
		// affordable here; the AABB tree is used for the two queries where a
		// wrong answer would be visible. Replacing this with a tree query is a
		// possible optimisation and is NOT attempted, because "nearest" here is
		// RAN's particular tie-broken answer and a different candidate set would
		// change which cell a spawn lands in.
		std::uint32_t FindClosestCell(const Vector3& point) const;

		// ---- queries ----------------------------------------------------------

		// Can two points on the mesh see each other in a straight line?
		// navigationmesh.cpp:350-385.
		bool LineOfSightTest(std::uint32_t startId, const Vector3& startPos,
		                     std::uint32_t endId, const Vector3& endPos) const;

		// Fills `path` with the A* waypoint list from start to end.
		//
		// `session` is the caller's; see the header on immutability. `pathDist`,
		// when given, receives the walked length - or, when start and end are the
		// same cell, the straight-line distance and no waypoints at all
		// (navigationmesh.cpp:193-197).
		bool BuildNavigationPath(NavigationPath& path, NavigationSearchSession& session,
		                         std::uint32_t startId, const Vector3& startPos,
		                         std::uint32_t endId, const Vector3& endPos,
		                         float* pathDist = nullptr) const;

		// Slides a requested move across the mesh, stopping at walls.
		//
		// Writes the corrected position and the cell it ended in.
		// navigationmesh.cpp:241-341.
		void ResolveMotionOnMesh(const Vector3& startPos, std::uint32_t startId,
		                         Vector3& endPos, std::uint32_t* endId) const;

		// The nearest triangle hit by the segment `point1`..`point2`.
		//
		// This is the GOTO destination probe. `collisionId` receives the cell that
		// was hit and is meaningful only when this returns true. navigationmesh.cpp:471-486,
		// :428-469.
		bool IsCollision(const Vector3& point1, const Vector3& point2, Vector3& collision,
		                 std::uint32_t* collisionId) const;

		// The mesh's bounding box, or false when the tree is absent.
		bool GetAABB(Vector3& vMax, Vector3& vMin) const;

	private:
		// ---- the AABB tree ----------------------------------------------------
		//
		// `AabbNode` holds raw non-owning links to its children, so the nodes must
		// have stable addresses. They do: each is separately allocated and owned by
		// `m_treeStorage`, which is a vector of unique_ptr rather than a vector of
		// nodes - growing a vector of nodes would reallocate and dangle every child
		// pointer already stored in a parent.
		void MakeAABBTree();
		bool MakeAABBNode(AabbNode* node, const std::vector<std::uint32_t>& cellIndex,
		                  std::size_t cellIndexCount, const Vector3& vMax, const Vector3& vMin);
		bool GetSizeNode(const std::vector<std::uint32_t>& cellIndex, std::size_t cellIndexCount,
		                 Vector3& vMax, Vector3& vMin) const;
		// Reproduces navagationtree.cpp:71-118, legacy's `vDist`-vs-`vCenter`
		// comparison bug included. See the .cpp for why it cannot be fixed here.
		void GetCenterDistNode(const std::vector<std::uint32_t>& cellIndex,
		                       std::size_t cellIndexCount, Vector3& vMax, Vector3& vMin) const;
		static bool IsWithinTriangle(const Vector3& t1, const Vector3& t2, const Vector3& t3,
		                             float division, int axis) noexcept;

		// The tree walk behind `IsCollision`. Recursive to mirror legacy
		// (navagationtree.cpp:428-469); the tree is log-depth so the stack cost is
		// a dozen frames, not a concern.
		bool IsCollisionOnNode(const AabbNode& node, Vector3& p1, Vector3& p2,
		                       const Vector3& testStart, Vector3& collision,
		                       std::uint32_t& collisionId) const;

		// The three vertices of `cellId`, in file order.
		void CellTriangle(std::uint32_t cellId, Vector3& t0, Vector3& t1,
		                  Vector3& t2) const;

		std::vector<std::unique_ptr<NavigationCell>> m_cells{};
		std::vector<const NavigationCell*>            m_byId{};
		std::vector<Vector3>                          m_vertices{};

		// One entry per tree node, allocated up front so `MakeAABBNode` can pass
		// `AabbNode&` around instead of a pointer it has to keep allocating.
		std::vector<std::unique_ptr<AabbNode>> m_treeStorage{};
		const AabbNode*         m_treeRoot = nullptr;

		bool m_built = false;
	};
}