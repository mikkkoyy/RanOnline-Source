#include "navigation/NavigationMesh.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace Modern::Navigation
{
	namespace
	{
		constexpr float kFloatMax = 3.402823466e+38f;

		// navagationtree.cpp:61-66 and :110-115: every computed box is widened by
		// this, so a vertex sitting exactly on a plane still tests as inside.
		constexpr float kBoxEpsilon = 0.0001f;

		// navigationmesh.cpp:277: the hard iteration ceiling in ResolveMotionOnMesh.
		// Reaching it means the mesh could not resolve the move, which legacy
		// asserts on.
		constexpr int kMaxResolveIterations = 100;

		void Accumulate(const Vector3& point, Vector3& vMax, Vector3& vMin) noexcept
		{
			vMax.x = std::max(vMax.x, point.x);
			vMax.y = std::max(vMax.y, point.y);
			vMax.z = std::max(vMax.z, point.z);
			vMin.x = std::min(vMin.x, point.x);
			vMin.y = std::min(vMin.y, point.y);
			vMin.z = std::min(vMin.z, point.z);
		}

		void Widen(Vector3& vMax, Vector3& vMin) noexcept
		{
			vMax.x += kBoxEpsilon;
			vMin.x -= kBoxEpsilon;
			vMax.y += kBoxEpsilon;
			vMin.y -= kBoxEpsilon;
			vMax.z += kBoxEpsilon;
			vMin.z -= kBoxEpsilon;
		}

		// navagationtree.cpp:154-201: longest axis first, then the remaining two in
		// descending order.
		//
		// This is a stable TOTAL order and it matters more than it looks: the axis
		// order decides where every split plane goes, the split planes decide the
		// shape of the tree, and the tree decides which cells the broad phase offers
		// - so changing a tie-break here is a compatibility change, not a tidy-up.
		void CalcAxisTable(float sizeX, float sizeY, float sizeZ, int axis[3]) noexcept
		{
			if (sizeX >= sizeY && sizeX >= sizeZ)
			{
				axis[0] = 0;
				if (sizeY >= sizeZ)
				{
					axis[1] = 1;
					axis[2] = 2;
				}
				else
				{
					axis[1] = 2;
					axis[2] = 1;
				}
			}
			else if (sizeY >= sizeX && sizeY >= sizeZ)
			{
				axis[0] = 1;
				if (sizeX >= sizeZ)
				{
					axis[1] = 0;
					axis[2] = 2;
				}
				else
				{
					axis[1] = 2;
					axis[2] = 0;
				}
			}
			else
			{
				axis[0] = 2;
				if (sizeX >= sizeY)
				{
					axis[1] = 0;
					axis[2] = 1;
				}
				else
				{
					axis[1] = 1;
					axis[2] = 0;
				}
			}
		}
	}

	// ---------------------------------------------------------------------------
	// Build
	// ---------------------------------------------------------------------------

	bool NavigationMesh::Build(const std::vector<Vector3>& vertices,
	                           const std::uint8_t* cellData, std::size_t cellCount,
	                           const std::vector<std::uint32_t>& linkIds)
	{
		m_cells.clear();
		m_byId.clear();
		m_vertices.clear();
		m_treeStorage.clear();
		m_treeRoot = nullptr;
		m_built    = false;

		if (cellData == nullptr || cellCount == 0 || linkIds.size() != cellCount * 3)
		{
			return false;
		}

		m_vertices.reserve(vertices.size());
		for (const Vector3& vertex : vertices)
		{
			// The millimetre quantisation of navigationmesh.cpp:894-905. The baked
			// file was produced through it, so applying it again keeps a coordinate
			// meaning the same value here as it did when the mesh was built - and
			// every cell-inside test is an exact comparison against it.
			m_vertices.push_back(
			    {static_cast<float>(static_cast<long>(vertex.x * 1000.f)) / 1000.f,
			     static_cast<float>(static_cast<long>(vertex.y * 1000.f)) / 1000.f,
			     static_cast<float>(static_cast<long>(vertex.z * 1000.f)) / 1000.f});
		}

		m_cells.reserve(cellCount);
		for (std::size_t i = 0; i < cellCount; ++i)
		{
			// The record is a packed mix of floats, DWORDs and 28-byte Line2Ds, so
			// there is no struct to point a reinterpret_cast at. It is copied into an
			// aligned DWORD array first: the caller's buffer comes from a vector of
			// bytes and is not guaranteed to be 4-byte aligned, so casting in place
			// would be undefined behaviour on every record after the first.
			std::uint32_t record[NavigationCell::kCellRecordDwords];
			std::memcpy(record, cellData + i * NavigationCell::kCellRecordBytes,
			            NavigationCell::kCellRecordBytes);

			auto cell = std::make_unique<NavigationCell>();
			cell->RestoreFromRecord(record);
			cell->SetIndex(i);

			// The one invariant the loader can check, and the reason this returns a
			// bool. See the header.
			if (cell->CellId() != i)
			{
				return false;
			}

			for (int v = 0; v < 3; ++v)
			{
				if (cell->Vertex(v) >= m_vertices.size())
				{
					return false;
				}
			}

			m_cells.push_back(std::move(cell));
		}

		// Links second, because they are cell IDs and need every cell to exist.
		m_byId.resize(cellCount, nullptr);
		for (std::size_t i = 0; i < cellCount; ++i)
		{
			m_byId[i] = m_cells[i].get();
		}

		for (std::size_t i = 0; i < cellCount; ++i)
		{
			for (int side = 0; side < 3; ++side)
			{
				const std::uint32_t linkId = linkIds[i * 3 + static_cast<std::size_t>(side)];
				if (linkId == kNoLink)
				{
					m_cells[i]->SetLink(static_cast<NavigationCell::CellSide>(side), nullptr);
					continue;
				}

				const NavigationCell* target = GetCellById(linkId);
				if (target == nullptr)
				{
					return false;
				}
				m_cells[i]->SetLink(static_cast<NavigationCell::CellSide>(side), target);
			}
		}

		MakeAABBTree();
		if (m_treeRoot == nullptr)
		{
			return false;
		}

		m_built = true;
		return true;
	}

	// ---------------------------------------------------------------------------
	// Snapping
	// ---------------------------------------------------------------------------

	Vector3 NavigationMesh::SnapPointToCell(std::uint32_t cellId, const Vector3& point) const
	{
		Vector3 pointOut = point;

		const NavigationCell* cell = GetCellById(cellId);
		if (cell == nullptr)
		{
			return Vector3{};
		}

		if (!cell->IsPointInCellCollumn(pointOut))
		{
			cell->ForcePointToCellCollumn(pointOut);
		}

		cell->MapVectorHeightToCell(pointOut);
		return pointOut;
	}

	Vector3 NavigationMesh::SnapPointToMesh(std::uint32_t* cellOutId,
	                                        const Vector3& point) const
	{
		Vector3 pointOut = point;
		*cellOutId       = FindClosestCell(pointOut);

		return SnapPointToCell(*cellOutId, pointOut);
	}

	std::uint32_t NavigationMesh::FindClosestCell(const Vector3& point) const
	{
		float                closestDistance = 3.4e+38f;
		float                closestHeight   = 3.4e+38f;
		bool                 foundHomeCell   = false;
		const NavigationCell* closestCell     = nullptr;

		for (const auto& cellPtr : m_cells)
		{
			const NavigationCell* cell = cellPtr.get();

			if (cell->IsPointInCellCollumn(point))
			{
				Vector3 newPosition(point);
				cell->MapVectorHeightToCell(newPosition);

				const float thisDistance = std::fabs(newPosition.y - point.y);

				if (foundHomeCell)
				{
					if (thisDistance < closestHeight)
					{
						closestCell   = cell;
						closestHeight = thisDistance;
					}
				}
				else
				{
					closestCell   = cell;
					closestHeight = thisDistance;
					foundHomeCell = true;
				}
			}

			// The fallback runs only while no cell has claimed the point - but note
			// that legacy evaluates it for EVERY cell on the SAME pass, so a cell
			// found later in the array can still overwrite `closestCell` even when an
			// earlier one would have been preferred by the column test. Preserved:
			// the array order is load-bearing here.
			if (!foundHomeCell)
			{
				const Line2D motionPath(
				    Vector2{cell->CenterPoint().x, cell->CenterPoint().z},
				    Vector2{point.x, point.z});

				const NavigationCell* nextCell           = nullptr;
				NavigationCell::CellSide wallHit         = NavigationCell::SideAB;
				Vector2                 pointOfIntersection{};
				const NavigationCell::PathResult result =
				    cell->ClassifyPathToCell(motionPath, &nextCell, wallHit,
				                              &pointOfIntersection);

				if (result == NavigationCell::ExitingCell)
				{
					Vector3 closestPoint3D{pointOfIntersection.x, 0.0f,
					                       pointOfIntersection.y};
					cell->MapVectorHeightToCell(closestPoint3D);

					closestPoint3D -= point;

					const float thisDistance = closestPoint3D.Length();
					if (thisDistance < closestDistance)
					{
						closestDistance = thisDistance;
						closestCell    = cell;
					}
				}
			}
		}

		if (closestCell == nullptr)
		{
			// Legacy asserts here (navigationmesh.cpp:118-123). A server cannot
			// assert on a point that arrived over the wire, so this is the one place
			// that returns cell 0 instead - and the caller snaps into it, which is
			// a defined position rather than a crash.
			return 0;
		}

		return closestCell->CellId();
	}

	// ---------------------------------------------------------------------------
	// Line of sight
	// ---------------------------------------------------------------------------

	bool NavigationMesh::LineOfSightTest(std::uint32_t startId, const Vector3& startPos,
	                                     std::uint32_t endId, const Vector3& endPos) const
	{
		const NavigationCell* startCell = GetCellById(startId);
		const NavigationCell* endCell   = GetCellById(endId);
		if (startCell == nullptr || endCell == nullptr)
		{
			return false;
		}

		const Line2D motionPath(Vector2{startPos.x, startPos.z},
		                        Vector2{endPos.x, endPos.z});

		const NavigationCell* processCell = startCell;

		// RAN's loop is unbounded (`while(1)`, navigationmesh.cpp:364). Every exit
		// is a return or a hop through a link, and a mesh's links are acyclic, so it
		// terminates - but the bound here is one step per cell, so a corrupt link set
		// cannot hang a Field worker.
		const std::size_t maxSteps = m_cells.size() + 1;
		for (std::size_t step = 0; step < maxSteps; ++step)
		{
			const NavigationCell* nextCell       = nullptr;
			NavigationCell::CellSide wallNumber   = NavigationCell::SideAB;
			const NavigationCell::PathResult result =
			    processCell->ClassifyPathToCell(motionPath, &nextCell, wallNumber, nullptr);

			// The pre-2006 code returned true as soon as the path stopped crossing
			// cells. The current version additionally requires the cell it ended in to
			// be the expected one (navigationmesh.cpp:381), so a path that stops in a
			// DIFFERENT cell than asked for is rejected.
			if (result == NavigationCell::ExitingCell)
			{
				if (nextCell == nullptr)
				{
					return false;
				}
				processCell = nextCell;
				continue;
			}

			if (result == NavigationCell::EndingCell &&
			    processCell->CellId() != endCell->CellId())
			{
				return false;
			}
			return true;
		}

		return false;
	}

	// ---------------------------------------------------------------------------
	// Path building
	// ---------------------------------------------------------------------------

	bool NavigationMesh::BuildNavigationPath(NavigationPath& path,
	                                         NavigationSearchSession& session,
	                                         std::uint32_t startId, const Vector3& startPos,
	                                         std::uint32_t endId, const Vector3& endPos,
	                                         float* pathDist) const
	{
		const NavigationCell* startCell = GetCellById(startId);
		const NavigationCell* endCell   = GetCellById(endId);
		if (startCell == nullptr || endCell == nullptr)
		{
			return false;
		}

		bool foundPath = false;

		session.Setup(startPos);

		// A REVERSE search: from EndCell towards StartCell.
		//
		// The comment at navigationmesh.cpp:141-149 explains the null caller: with no
		// previous cell there is no arrival wall to recover, which is why the end
		// cell is pushed first and processed first. The consequence in code is that
		// the end cell is the one that can come off the heap with
		// `ArrivalWall() == -1`, which is the loop's early exit below.
		endCell->QueryForPath(session, nullptr, 0.0f);

		while (session.NotEmpty() && !foundPath)
		{
			NavigationNode thisNode{};
			session.GetTop(thisNode);

			if (thisNode.cell == nullptr)
			{
				return foundPath;
			}

			// navigationmesh.cpp:161-164: popped a cell that was entered through no
			// known side. `ProcessCell` would expand nothing for it, so the search
			// gives up rather than spinning.
			if (session.ArrivalWall(*thisNode.cell) == -1)
			{
				return foundPath;
			}

			if (thisNode.cell == startCell)
			{
				foundPath = true;
			}
			else
			{
				thisNode.cell->ProcessCell(session);
			}
		}

		if (!foundPath)
		{
			return false;
		}

		// Walk the links back from the start cell, turning each cell's arrival-wall
		// midpoint into a waypoint.
		const std::uint32_t resolvedStartId = startCell->CellId();
		const std::uint32_t resolvedEndId   = endCell->CellId();

		path.Setup(this, startPos, resolvedStartId, endPos, resolvedEndId);

		if (pathDist != nullptr && resolvedStartId == resolvedEndId)
		{
			// Same cell: no waypoints at all, and the distance is the straight line.
			// navigationmesh.cpp:193-197.
			*pathDist = (startPos - endPos).Length();
		}
		else
		{
			float                 correctY     = 0.0f;
			const Vector3         prevWayPoint = startPos;
			const NavigationCell* testCell     = startCell;

			while (testCell != nullptr && testCell->CellId() != resolvedEndId)
			{
				const int linkWall = session.ArrivalWall(*testCell);
				Vector3   newWayPoint = testCell->WallMidpoint(linkWall);

				// "Just to be sure" in the original. It matters: the midpoint is
				// baked, and this pulls it back inside the cell that owns it.
				newWayPoint = SnapPointToCell(testCell->CellId(), newWayPoint);

				// A cell whose plane is horizontal or overhanging (`Normal().y <=
				// 0.0001f`) is not a floor, so its midpoint's Y is DISCARDED and the
				// last floor height is reused. navigationmesh.cpp:212-219. The effect
				// is that a path crossing a wall face does not drag the walker's
				// height up to the seam.
				if (testCell->Normal().y <= 0.0001f)
				{
					newWayPoint.y = correctY;
				}
				else
				{
					correctY = newWayPoint.y;
				}

				if (pathDist != nullptr)
				{
					*pathDist += (prevWayPoint - newWayPoint).Length();
				}

				path.AddWayPoint(newWayPoint, testCell->CellId());

				testCell = testCell->Link(static_cast<NavigationCell::CellSide>(linkWall));
			}
		}

		path.EndPath();
		return true;
	}

	// ---------------------------------------------------------------------------
	// Motion resolution
	// ---------------------------------------------------------------------------

	void NavigationMesh::ResolveMotionOnMesh(const Vector3& startPos, std::uint32_t startId,
	                                         Vector3& endPos, std::uint32_t* endId) const
	{
		// Project both endpoints onto XZ. The Y of `endPos` is discarded here and
		// re-solved from the cell plane at the bottom.
		Line2D motionPath(Vector2{startPos.x, startPos.z}, Vector2{endPos.x, endPos.z});

		NavigationCell::PathResult result      = NavigationCell::NoRelationship;
		NavigationCell::CellSide   wallNumber  = NavigationCell::SideAB;
		Vector2                    pointOfIntersection{};
		const NavigationCell*      nextCell    = nullptr;

		const NavigationCell* testCell = GetCellById(startId);
		if (testCell == nullptr)
		{
			return;
		}

		// Progress is measured by TRUNCATED INTEGERS, not by a distance epsilon
		// (navigationmesh.cpp:262-263 and :317-318). Two positions a fraction of a
		// unit apart inside the same integer cell read as "no progress" and end the
		// resolve; the comment beside the original says exactly that.
		bool bDiff = (static_cast<int>(motionPath.EndPointA().x) !=
		              static_cast<int>(motionPath.EndPointB().x)) ||
		             (static_cast<int>(motionPath.EndPointA().y) !=
		              static_cast<int>(motionPath.EndPointB().y));

		// Only the INITIAL no-progress case suppresses the height solve at the end.
		// Losing progress part way through still gets a Y.
		const bool bFirstTimeSame = !bDiff;

		int i = 0;
		for (i = 0; i < kMaxResolveIterations; ++i)
		{
			if (result == NavigationCell::EndingCell)
			{
				break;
			}

			if (!bDiff)
			{
				break;
			}

			result = testCell->ClassifyPathToCell(motionPath, &nextCell, wallNumber,
			                                      &pointOfIntersection);

			if (result == NavigationCell::ExitingCell)
			{
				if (nextCell != nullptr)
				{
					motionPath.SetEndPointA(pointOfIntersection);
					testCell = nextCell;
				}
				else
				{
					// A solid wall: project onto it, then lose 2% of the motion to
					// friction. The comment says 10% (navigationmesh.cpp:304-305) and
					// the code says 0.98f, which is 2%. The code is what RAN does.
					motionPath.SetEndPointA(pointOfIntersection);
					testCell->ProjectPathOnCellWall(wallNumber, motionPath);

					Vector2 direction = motionPath.EndPointB() - motionPath.EndPointA();
					direction *= 0.98f;
					motionPath.SetEndPointB(motionPath.EndPointA() + direction);
				}
			}
			else if (result == NavigationCell::NoRelationship)
			{
				Vector2 newOrigin = motionPath.EndPointA();
				testCell->ForcePointToCellCollumn(newOrigin);
				motionPath.SetEndPointA(newOrigin);
			}

			bDiff = (static_cast<int>(motionPath.EndPointA().x) !=
			         static_cast<int>(motionPath.EndPointB().x)) ||
			        (static_cast<int>(motionPath.EndPointA().y) !=
			         static_cast<int>(motionPath.EndPointB().y));
		}

		*endId = testCell->CellId();

		endPos.x = motionPath.EndPointB().x;
		endPos.z = motionPath.EndPointB().y;

		if (!bFirstTimeSame)
		{
			testCell->MapVectorHeightToCell(endPos);
		}
	}

	// ---------------------------------------------------------------------------
	// Collision
	// ---------------------------------------------------------------------------

	void NavigationMesh::CellTriangle(std::uint32_t cellId, Vector3& t0, Vector3& t1,
	                                  Vector3& t2) const
	{
		const NavigationCell* cell = GetCellById(cellId);
		t0 = m_vertices[cell->Vertex(0)];
		t1 = m_vertices[cell->Vertex(1)];
		t2 = m_vertices[cell->Vertex(2)];
	}

	bool NavigationMesh::IsCollisionOnNode(const AabbNode& node, Vector3& p1, Vector3& p2,
	                                       const Vector3& testStart, Vector3& collision,
	                                       std::uint32_t& collisionId) const
	{
		// `AabbNode::IsCollision` clips its endpoints, so it gets WORKING copies -
		// the caller's endpoints must survive for the sibling branch and for the
		// distance comparison below.
		Vector3 newP1 = p1;
		Vector3 newP2 = p2;

		if (!node.IsCollision(newP1, newP2))
		{
			return false;
		}

		if (node.face != AabbNode::kNoFace)
		{
			Vector3 t0{};
			Vector3 t1{};
			Vector3 t2{};
			CellTriangle(node.face, t0, t1, t2);

			Vector3 newCollision{};
			if (IsLineTriangleCollision(t0, t1, t2, newP1, newP2, newCollision))
			{
				const float newLengthSq = (testStart - newCollision).LengthSq();
				const float oldLengthSq = (testStart - collision).LengthSq();

				if (newLengthSq < oldLengthSq)
				{
					collision   = newCollision;
					collisionId = node.face;
					return true;
				}
			}
			return false;
		}

		// Both children see the SAME clipped segment - which is the entire reason
		// for clipping it - and each keeps whichever hit is nearer.
		bool found = false;
		if (node.left != nullptr)
		{
			found = IsCollisionOnNode(*node.left, newP1, newP2, testStart, collision,
			                          collisionId);
		}
		if (node.right != nullptr)
		{
			// Evaluated unconditionally, then OR-ed - not `found && ...`, or the
			// right subtree would be skipped whenever the left one already hit.
			const bool rightFound =
			    IsCollisionOnNode(*node.right, newP1, newP2, testStart, collision,
			                      collisionId);
			found = rightFound || found;
		}
		return found;
	}

	bool NavigationMesh::IsCollision(const Vector3& point1, const Vector3& point2,
	                                 Vector3& collision, std::uint32_t* collisionId) const
	{
		*collisionId = kNoLink;

		if (m_treeRoot == nullptr)
		{
			return false;
		}

		// The seed: `vCollision = vPoint2`, and the "is this nearer than the current
		// best" test measures from `vPoint1` (navigationmesh.cpp:480-482 and
		// :446-455). Seeding with point2 rather than a point at infinity is why the
		// first triangle found wins ties.
		collision = point2;

		Vector3       workingStart = point1;
		Vector3       workingEnd   = point2;
		std::uint32_t id           = kNoLink;

		if (IsCollisionOnNode(*m_treeRoot, workingStart, workingEnd, point1, collision, id))
		{
			*collisionId = id;
			return true;
		}
		return false;
	}

	// ---------------------------------------------------------------------------
	// AABB tree
	// ---------------------------------------------------------------------------

	bool NavigationMesh::GetSizeNode(const std::vector<std::uint32_t>& cellIndex,
	                                 std::size_t cellIndexCount, Vector3& vMax,
	                                 Vector3& vMin) const
	{
		if (cellIndex.empty() || cellIndexCount < 1)
		{
			return false;
		}

		vMax = Vector3{-kFloatMax, -kFloatMax, -kFloatMax};
		vMin = Vector3{kFloatMax, kFloatMax, kFloatMax};

		for (std::size_t i = 0; i < cellIndexCount; ++i)
		{
			const NavigationCell* cell = GetCellById(cellIndex[i]);
			Accumulate(m_vertices[cell->Vertex(0)], vMax, vMin);
			Accumulate(m_vertices[cell->Vertex(1)], vMax, vMin);
			Accumulate(m_vertices[cell->Vertex(2)], vMax, vMin);
		}

		Widen(vMax, vMin);
		return true;
	}

	void NavigationMesh::GetCenterDistNode(const std::vector<std::uint32_t>& cellIndex,
	                                       std::size_t cellIndexCount, Vector3& vMax,
	                                       Vector3& vMin) const
	{
		vMax = Vector3{-kFloatMax, -kFloatMax, -kFloatMax};
		vMin = Vector3{kFloatMax, kFloatMax, kFloatMax};

		for (std::size_t i = 0; i < cellIndexCount; ++i)
		{
			const NavigationCell* cell = GetCellById(cellIndex[i]);

			Vector3 centre = m_vertices[cell->Vertex(0)];
			centre += m_vertices[cell->Vertex(1)];
			centre += m_vertices[cell->Vertex(2)];
			centre /= 3.0f;

			// LEGACY BUG, REPRODUCED DELIBERATELY.
			//
			// navagationtree.cpp:99-106 tests `vDist` - the LAST transformed vertex
			// - while assigning from `vCenter`, the centroid. Whenever the third
			// vertex of some cell in the set is not the extreme on an axis, that
			// axis is never updated and the "centre bounds" box collapses toward the
			// first extreme that happened to pass.
			//
			// This is NOT fixed here. `GetCenterDistNode` is the second-chance split
			// for a node that could not be divided spatially, so its output chooses
			// the split plane for a whole subtree. Changing it changes the tree, which
			// changes which cells the broad phase offers, which changes which edges
			// become links - i.e. which surfaces are walls. Fixing a bug that is this
			// load-bearing is a compatibility decision with its own evidence, not a
			// cleanup, and it does not belong inside a movement milestone.
			const Vector3 vDist = m_vertices[cell->Vertex(2)];

			if (vDist.x > vMax.x) { vMax.x = centre.x; }
			if (vDist.y > vMax.y) { vMax.y = centre.y; }
			if (vDist.z > vMax.z) { vMax.z = centre.z; }

			if (vDist.x < vMin.x) { vMin.x = centre.x; }
			if (vDist.y < vMin.y) { vMin.y = centre.y; }
			if (vDist.z < vMin.z) { vMin.z = centre.z; }
		}

		Widen(vMax, vMin);
	}

	bool NavigationMesh::IsWithinTriangle(const Vector3& t1, const Vector3& t2,
	                                      const Vector3& t3, float division,
	                                      int axis) noexcept
	{
		Vector3 centre = t1;
		centre += t2;
		centre += t3;
		centre /= 3.0f;

		switch (axis)
		{
		case 0:
			if (centre.x <= division) { return true; }
			break;
		case 1:
			if (centre.y <= division) { return true; }
			break;
		case 2:
			if (centre.z <= division) { return true; }
			break;
		default:
			break;
		}

		return false;
	}

	void NavigationMesh::MakeAABBTree()
	{
		std::vector<std::uint32_t> cellIndex;
		cellIndex.reserve(m_cells.size());
		for (const auto& cell : m_cells)
		{
			cellIndex.push_back(cell->CellId());
		}

		Vector3 vMax{};
		Vector3 vMin{};
		if (!GetSizeNode(cellIndex, cellIndex.size(), vMax, vMin))
		{
			return;
		}

		m_treeStorage.push_back(std::make_unique<AabbNode>());
		AabbNode* root = m_treeStorage.back().get();
		root->vMax     = vMax;
		root->vMin     = vMin;

		if (!MakeAABBNode(root, cellIndex, cellIndex.size(), vMax, vMin))
		{
			return;
		}

		m_treeRoot = root;
	}

	bool NavigationMesh::MakeAABBNode(AabbNode* node, const std::vector<std::uint32_t>& cellIndex,
	                                  std::size_t cellIndexCount, const Vector3& vMax,
	                                  const Vector3& vMin)
	{
		// A single cell IS a leaf, and the box belongs to that face.
		if (cellIndexCount == 1)
		{
			node->vMax = vMax;
			node->vMin = vMin;
			node->face = GetCellById(cellIndex[0])->CellId();
			return true;
		}

		int   axis[3]{};
		float division = 0.0f;
		float sizeX    = vMax.x - vMin.x;
		float sizeY    = vMax.y - vMin.y;
		float sizeZ    = vMax.z - vMin.z;

		CalcAxisTable(sizeX, sizeY, sizeZ, axis);

		std::vector<std::uint32_t> leftIndex;
		std::vector<std::uint32_t> rightIndex;
		leftIndex.reserve(cellIndexCount);
		rightIndex.reserve(cellIndexCount);

		// First attempt: split each triangle by its CENTROID along each axis in
		// turn, taking the first axis that actually divides the set.
		//
		// A triangle straddling the plane goes RIGHT unconditionally
		// (navagationtree.cpp:277) - there is no attempt to balance it, which is
		// why the third fallback below exists.
		for (int i = 0; i < 3; ++i)
		{
			leftIndex.clear();
			rightIndex.clear();

			switch (axis[i])
			{
			case 0: division = vMin.x + sizeX / 2.0f; break;
			case 1: division = vMin.y + sizeY / 2.0f; break;
			default: division = vMin.z + sizeZ / 2.0f; break;
			}

			for (std::size_t j = 0; j < cellIndexCount; ++j)
			{
				const NavigationCell* cell = GetCellById(cellIndex[j]);

				if (IsWithinTriangle(m_vertices[cell->Vertex(0)],
				                     m_vertices[cell->Vertex(1)],
				                     m_vertices[cell->Vertex(2)], division, axis[i]))
				{
					leftIndex.push_back(cellIndex[j]);
				}
				else
				{
					rightIndex.push_back(cellIndex[j]);
				}
			}

			if (!leftIndex.empty() && !rightIndex.empty())
			{
				break;
			}
		}

		// Second attempt: the same, but divided on the centroid BOUNDS rather than
		// the vertex bounds - reached when every triangle's centroid landed on the
		// same side of every vertex-bound midpoint.
		if (leftIndex.empty() || rightIndex.empty())
		{
			Vector3 centreMax{};
			Vector3 centreMin{};
			GetCenterDistNode(cellIndex, cellIndexCount, centreMax, centreMin);

			sizeX = centreMax.x - centreMin.x;
			sizeY = centreMax.y - centreMin.y;
			sizeZ = centreMax.z - centreMin.z;

			CalcAxisTable(sizeX, sizeY, sizeZ, axis);

			for (int i = 0; i < 3; ++i)
			{
				leftIndex.clear();
				rightIndex.clear();

				switch (axis[i])
				{
				case 0: division = centreMin.x + sizeX / 2.0f; break;
				case 1: division = centreMin.y + sizeY / 2.0f; break;
				default: division = centreMin.z + sizeZ / 2.0f; break;
				}

				for (std::size_t j = 0; j < cellIndexCount; ++j)
				{
					const NavigationCell* cell = GetCellById(cellIndex[j]);

					if (IsWithinTriangle(m_vertices[cell->Vertex(0)],
					                     m_vertices[cell->Vertex(1)],
					                     m_vertices[cell->Vertex(2)], division, axis[i]))
					{
						leftIndex.push_back(cellIndex[j]);
					}
					else
					{
						rightIndex.push_back(cellIndex[j]);
					}
				}

				if (!leftIndex.empty() && !rightIndex.empty())
				{
					break;
				}
			}
		}

		// Third attempt: cut the list in half, in order, and give up on splitting
		// spatially at all. navagationtree.cpp:358-374.
		//
		// RAN writes the right-hand start index as `WORD(nCellIndex / 2)`
		// (navagationtree.cpp:369), which TRUNCATES TO 16 BITS. On any set larger
		// than 131070 cells the right half would start in the wrong place, and the
		// tree would then hold some cells twice and omit others - which would
		// silently narrow the collision query's broad phase.
		//
		// RAN's real meshes top out in the low tens of thousands of cells, so the
		// truncation never fires and every `.wld` decodes into a correct tree either
		// way. The widened index is used here regardless: reproducing a latent
		// out-of-range read is not worth the risk, and for every mesh RAN ships the
		// two expressions are identical.
		if (leftIndex.empty() || rightIndex.empty())
		{
			leftIndex.clear();
			rightIndex.clear();

			const std::size_t half = cellIndexCount / 2;
			for (std::size_t j = 0; j < half; ++j)
			{
				leftIndex.push_back(cellIndex[j]);
			}
			for (std::size_t j = half; j < cellIndexCount; ++j)
			{
				rightIndex.push_back(cellIndex[j]);
			}
		}

		if (!leftIndex.empty())
		{
			Vector3 leftMax{};
			Vector3 leftMin{};
			if (!GetSizeNode(leftIndex, leftIndex.size(), leftMax, leftMin))
			{
				return false;
			}

			m_treeStorage.push_back(std::make_unique<AabbNode>());
			AabbNode* child = m_treeStorage.back().get();
			child->vMax     = leftMax;
			child->vMin     = leftMin;
			node->left      = child;

			if (!MakeAABBNode(child, leftIndex, leftIndex.size(), leftMax, leftMin))
			{
				return false;
			}
		}

		if (!rightIndex.empty())
		{
			Vector3 rightMax{};
			Vector3 rightMin{};
			if (!GetSizeNode(rightIndex, rightIndex.size(), rightMax, rightMin))
			{
				return false;
			}

			m_treeStorage.push_back(std::make_unique<AabbNode>());
			AabbNode* child = m_treeStorage.back().get();
			child->vMax     = rightMax;
			child->vMin     = rightMin;
			node->right     = child;

			if (!MakeAABBNode(child, rightIndex, rightIndex.size(), rightMax, rightMin))
			{
				return false;
			}
		}

		return true;
	}

	bool NavigationMesh::GetAABB(Vector3& vMax, Vector3& vMin) const
	{
		if (m_treeRoot == nullptr)
		{
			return false;
		}
		vMax = m_treeRoot->vMax;
		vMin = m_treeRoot->vMin;
		return true;
	}
}