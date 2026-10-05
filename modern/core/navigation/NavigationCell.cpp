#include "navigation/NavigationCell.h"

#include "navigation/NavigationSearchSession.h"

#include <cmath>
#include <cstring>

namespace Modern::Navigation
{
	namespace
	{
		// RAN's float-equality tolerance. COLLISION::IsfEqual (Collision.h:150-155)
		// uses 0.001f, and `NavigationCell::ForcePointToWallInterior` uses the same
		// value as its own epsilon (navigationcell.cpp:154). One named constant for
		// both, so the two cannot drift.
		constexpr float kFloatEpsilon = 0.001f;

		float ReadF32(const std::uint32_t (&record)[NavigationCell::kCellRecordDwords],
		              std::size_t byteOffset) noexcept
		{
			float value = 0.0f;
			const std::uint32_t bits = record[(byteOffset / 4)];
			std::memcpy(&value, &bits, sizeof(value));
			return value;
		}
	}

	// ---------------------------------------------------------------------------
	// Loading
	// ---------------------------------------------------------------------------

	void NavigationCell::RestoreFromRecord(
	    const std::uint32_t (&record)[NavigationCell::kCellRecordDwords])
	{
		m_cellId = record[kOffsetCellId / 4];

		for (int i = 0; i < 3; ++i)
		{
			m_vertex[i] = record[(kOffsetVertex / 4) + static_cast<std::size_t>(i)];
		}

		// Line2D is 28 bytes on disk but only 20 bytes of it are endpoints: the
		// trailing 8 are the CACHED normal and its validity flag, and legacy
		// `ReadBuffer`s all 28 (NavigationSaveLoad.cpp:125). The normal is
		// recomputed on demand from the endpoints, which is why a stale or
		// uninitialised cached normal cannot change a result.
		for (int i = 0; i < 3; ++i)
		{
			const std::size_t base = kOffsetSide + static_cast<std::size_t>(i) * 28;
			const Vector2 a{ReadF32(record, base + 0), ReadF32(record, base + 4)};
			const Vector2 b{ReadF32(record, base + 8), ReadF32(record, base + 12)};
			m_side[i].SetPoints(a, b);
		}

		m_cellPlane.m_normal.x = ReadF32(record, kOffsetPlane + 0);
		m_cellPlane.m_normal.y = ReadF32(record, kOffsetPlane + 4);
		m_cellPlane.m_normal.z = ReadF32(record, kOffsetPlane + 8);
		m_cellPlane.m_point.x  = ReadF32(record, kOffsetPlane + 12);
		m_cellPlane.m_point.y  = ReadF32(record, kOffsetPlane + 16);
		m_cellPlane.m_point.z  = ReadF32(record, kOffsetPlane + 20);
		m_cellPlane.m_distance = ReadF32(record, kOffsetPlane + 24);

		m_centerPoint.x = ReadF32(record, kOffsetCenterPoint + 0);
		m_centerPoint.y = ReadF32(record, kOffsetCenterPoint + 4);
		m_centerPoint.z = ReadF32(record, kOffsetCenterPoint + 8);

		for (int i = 0; i < 3; ++i)
		{
			const std::size_t base = kOffsetWallMidpoint + static_cast<std::size_t>(i) * 12;
			m_wallMidpoint[i].x = ReadF32(record, base + 0);
			m_wallMidpoint[i].y = ReadF32(record, base + 4);
			m_wallMidpoint[i].z = ReadF32(record, base + 8);
		}

		for (int i = 0; i < 3; ++i)
		{
			m_wallDistance[i] = ReadF32(record, kOffsetWallDistance + static_cast<std::size_t>(i) * 4);
		}

		// Links are resolved by the mesh, which owns the cells; a fresh cell has
		// three solid edges.
		for (int i = 0; i < 3; ++i)
		{
			m_link[i] = nullptr;
		}
	}

	void NavigationCell::ComputeFromVertices(const Vector3* naviVertex, std::uint32_t pointA,
	                                         std::uint32_t pointB, std::uint32_t pointC,
	                                         std::uint32_t cellId)
	{
		m_cellId             = cellId;
		m_vertex[VertA]      = pointA;
		m_vertex[VertB]      = pointB;
		m_vertex[VertC]      = pointC;
		m_link[SideAB]       = nullptr;
		m_link[SideBC]       = nullptr;
		m_link[SideCA]       = nullptr;

		const Vector3& a = naviVertex[m_vertex[VertA]];
		const Vector3& b = naviVertex[m_vertex[VertB]];
		const Vector3& c = naviVertex[m_vertex[VertC]];

		// XZ projection. RAN does the same at navigationcell.h:205-207.
		m_side[SideAB].SetPoints(Vector2{a.x, a.z}, Vector2{b.x, b.z});
		m_side[SideBC].SetPoints(Vector2{b.x, b.z}, Vector2{c.x, c.z});
		m_side[SideCA].SetPoints(Vector2{c.x, c.z}, Vector2{a.x, a.z});

		m_cellPlane.Set(a, b, c);

		m_centerPoint = {(a.x + b.x + c.x) / 3.0f, (a.y + b.y + c.y) / 3.0f,
		                 (a.z + b.z + c.z) / 3.0f};

		// Note the pairing: wall 1 is (C,B) and wall 2 is (C,A), not (B,C) and
		// (A,C) - navigationcell.h:222-232. The side lines run A->B, B->C, C->A but
		// the midpoints are taken from C+B and C+A. Reproducing the sides but
		// "correcting" the midpoints would put waypoints on the wrong walls.
		m_wallMidpoint[0] = {(a.x + b.x) / 2.0f, (a.y + b.y) / 2.0f, (a.z + b.z) / 2.0f};
		m_wallMidpoint[1] = {(c.x + b.x) / 2.0f, (c.y + b.y) / 2.0f, (c.z + b.z) / 2.0f};
		m_wallMidpoint[2] = {(c.x + a.x) / 2.0f, (c.y + a.y) / 2.0f, (c.z + a.z) / 2.0f};

		const Vector3 w01 = m_wallMidpoint[0] - m_wallMidpoint[1];
		m_wallDistance[0] = std::sqrt(w01.x * w01.x + w01.y * w01.y + w01.z * w01.z);
		const Vector3 w12 = m_wallMidpoint[1] - m_wallMidpoint[2];
		m_wallDistance[1] = std::sqrt(w12.x * w12.x + w12.y * w12.y + w12.z * w12.z);
		const Vector3 w20 = m_wallMidpoint[2] - m_wallMidpoint[0];
		m_wallDistance[2] = std::sqrt(w20.x * w20.x + w20.y * w20.y + w20.z * w20.z);
	}

	// ---------------------------------------------------------------------------
	// Queries
	// ---------------------------------------------------------------------------

	NavigationCell::PathResult NavigationCell::ClassifyPathToCell(const Line2D& motionPath,
	                                                               const NavigationCell** nextCell,
	                                                               CellSide& side,
	                                                               Vector2* intersectionPoint) const noexcept
	{
		int interiorCount = 0;

		for (int i = 0; i < 3; ++i)
		{
			if (m_side[i].ClassifyPoint(motionPath.EndPointB()) != Line2D::RightSide)
			{
				if (m_side[i].ClassifyPoint(motionPath.EndPointA()) != Line2D::LeftSide)
				{
					const Line2D::LineClassification result =
					    motionPath.Intersection(m_side[i], intersectionPoint);

					if (result == Line2D::SegmentsIntersect || result == Line2D::ABisectsB)
					{
						// A null link here is a SOLID edge, and the caller treats it
						// as one. That is how a wall becomes a slide rather than a
						// hole in the mesh.
						*nextCell = m_link[i];
						side      = static_cast<CellSide>(i);
						return ExitingCell;
					}
				}
			}
			else
			{
				++interiorCount;
			}
		}

		if (interiorCount == 3)
		{
			return EndingCell;
		}

		return NoRelationship;
	}

	void NavigationCell::ProjectPathOnCellWall(CellSide sideNumber,
	                                           Line2D& motionPath) const noexcept
	{
		// navigationcell.cpp:113-124: normalise the WALL direction, take the
		// motion's component along it, and rebuild the path's end point from that.
		const Vector2 wallNormal =
		    Normalize(m_side[sideNumber].EndPointB() - m_side[sideNumber].EndPointA());
		const Vector2 motionVector = motionPath.EndPointB() - motionPath.EndPointA();
		const float   dotResult   = Dot(motionVector, wallNormal);

		Vector2 projected = wallNormal * dotResult;
		motionPath.SetEndPointB(motionPath.EndPointA() + projected);

		// Keep the START inside the cell.
		Vector2 newPoint = motionPath.EndPointA();
		ForcePointToCellCollumn(newPoint);
		motionPath.SetEndPointA(newPoint);

		// And make sure the END does not re-cross the wall we just slid onto.
		newPoint = motionPath.EndPointB();
		ForcePointToWallInterior(sideNumber, newPoint);
		motionPath.SetEndPointB(newPoint);
	}

	bool NavigationCell::ForcePointToWallInterior(CellSide sideNumber,
	                                               Vector2& testPoint) const noexcept
	{
		float   distance = m_side[sideNumber].SignedDistance(testPoint);
		const float epsilon = kFloatEpsilon;

		if (distance > epsilon)
		{
			return false;
		}

		if (distance <= 0.0f)
		{
			distance -= epsilon;
		}

		distance = std::fabs(distance);
		distance = epsilon > distance ? epsilon : distance;

		const Vector2 normal = m_side[sideNumber].Normal();
		testPoint           += normal * distance;
		return true;
	}

	bool NavigationCell::ForcePointToWallInterior(CellSide sideNumber,
	                                               Vector3& testPoint) const noexcept
	{
		Vector2 point2D{testPoint.x, testPoint.z};
		const bool altered = ForcePointToWallInterior(sideNumber, point2D);
		if (altered)
		{
			testPoint.x = point2D.x;
			testPoint.z = point2D.y;
		}
		return altered;
	}

	bool NavigationCell::ForcePointToCellCollumn(Vector2& testPoint) const noexcept
	{
		// A path from the cell CENTRE to the point: if even that leaves the cell,
		// the point cannot be salvaged by nudging.
		const Line2D testPath(Vector2{m_centerPoint.x, m_centerPoint.z}, testPoint);

		Vector2          pointOfIntersection{};
		CellSide         side     = SideAB;
		const NavigationCell* nextCell = nullptr;
		const PathResult result =
		    ClassifyPathToCell(testPath, &nextCell, side, &pointOfIntersection);

		if (result == ExitingCell)
		{
			// 2% short of the boundary, so the point does not land exactly ON the
			// wall and get classified onto it next time. Same 0.98f as the slide.
			Vector2 pathDirection{pointOfIntersection.x - m_centerPoint.x,
			                      pointOfIntersection.y - m_centerPoint.z};
			pathDirection *= 0.98f;

			testPoint.x = m_centerPoint.x + pathDirection.x;
			testPoint.y = m_centerPoint.z + pathDirection.y;

			if (!IsPointInCellCollumn(testPoint))
			{
				testPoint.x = m_centerPoint.x;
				testPoint.y = m_centerPoint.z;
			}
			return true;
		}

		if (result == NoRelationship)
		{
			testPoint.x = m_centerPoint.x;
			testPoint.y = m_centerPoint.z;
			return true;
		}

		return false;
	}

	bool NavigationCell::ForcePointToCellCollumn(Vector3& testPoint) const noexcept
	{
		Vector2 point2D{testPoint.x, testPoint.z};
		const bool altered = ForcePointToCellCollumn(point2D);
		if (altered)
		{
			testPoint.x = point2D.x;
			testPoint.z = point2D.y;
		}
		return altered;
	}

	// ---------------------------------------------------------------------------
	// A*
	//
	// The state these two write lives in the session, not here - see the header
	// note on immutability. Everything below is otherwise navigationcell.cpp's
	// arithmetic, including the two places where it is internally inconsistent.
	// ---------------------------------------------------------------------------

	bool NavigationCell::ProcessCell(NavigationSearchSession& session) const noexcept
	{
		// A cell that was never offered to this session is not ours to expand.
		// navigationcell.cpp:283-286.
		if (!session.HasSeen(*this))
		{
			return false;
		}

		// Processed cells are closed.
		session.SetOpen(*this, false);

		const int arrivalWall = session.ArrivalWall(*this);
		const float arrivalCost = session.ArrivalCost(*this);

		// navigationcell.cpp:290-307, unrolled because the three branches use
		// three DIFFERENT wall-distance indices. Folding them into
		// `abs(i - m_ArrivalWall)` would reproduce the formula RAN commented out.
		switch (arrivalWall)
		{
		case 0:
			if (m_link[0]) { m_link[0]->QueryForPath(session, this, 0.0f); }
			if (m_link[1]) { m_link[1]->QueryForPath(session, this, arrivalCost + m_wallDistance[0]); }
			if (m_link[2]) { m_link[2]->QueryForPath(session, this, arrivalCost + m_wallDistance[2]); }
			break;
		case 1:
			if (m_link[1]) { m_link[1]->QueryForPath(session, this, 0.0f); }
			if (m_link[2]) { m_link[2]->QueryForPath(session, this, arrivalCost + m_wallDistance[1]); }
			if (m_link[0]) { m_link[0]->QueryForPath(session, this, arrivalCost + m_wallDistance[0]); }
			break;
		case 2:
			if (m_link[2]) { m_link[2]->QueryForPath(session, this, 0.0f); }
			if (m_link[0]) { m_link[0]->QueryForPath(session, this, arrivalCost + m_wallDistance[2]); }
			if (m_link[1]) { m_link[1]->QueryForPath(session, this, arrivalCost + m_wallDistance[1]); }
			break;
		default:
			// arrivalWall == -1: this cell was entered through no known side, so
			// RAN has no cost for any edge and expands nothing.
			break;
		}

		return true;
	}

	bool NavigationCell::QueryForPath(NavigationSearchSession& session,
	                                  const NavigationCell* caller,
	                                  float arrivalCost) const noexcept
	{
		// First time this cell has been seen in this session.
		if (!session.HasSeen(*this))
		{
			session.MarkUnseen(*this);

			if (caller != nullptr)
			{
				session.SetOpen(*this, true);
				session.SetHeuristic(*this, ComputeHeuristic(session.Goal()));
				session.SetArrivalCost(*this, arrivalCost);

				// Remember which side the caller is entering through, by CELL ID.
				// navigationcell.cpp:339-354 compares `Link(i)->CellID()` to the
				// caller's, not the pointers - which matters, because two different
				// cell pointers can carry the same id in a mesh that has been rebuilt.
				if (m_link[0] && m_link[0]->CellId() == caller->CellId())
				{
					session.SetArrivalWall(*this, 0);
				}
				else if (m_link[1] && m_link[1]->CellId() == caller->CellId())
				{
					session.SetArrivalWall(*this, 1);
				}
				else if (m_link[2] && m_link[2]->CellId() == caller->CellId())
				{
					session.SetArrivalWall(*this, 2);
				}
				else
				{
					session.SetArrivalWall(*this, -1);
				}
			}
			else
			{
				// The goal cell: the search starts here and never re-enters it.
				session.SetOpen(*this, false);
				session.SetArrivalCost(*this, 0.0f);
				session.SetHeuristic(*this, 0.0f);
				session.SetArrivalWall(*this, 0);
			}

			session.AddCell(this);
			return true;
		}

		// Already open: a better route may have appeared.
		if (session.IsOpen(*this))
		{
			const float heuristic = session.Heuristic(*this);

			// The comparison keeps the heuristic on both sides, so it reduces to
			// "is this arrival cost lower" - reproduced verbatim rather than
			// simplified, because simplifying it changes nothing today and would
			// change what happens if the heuristic ever became non-constant.
			if ((arrivalCost + heuristic) < (session.ArrivalCost(*this) + heuristic))
			{
				session.SetArrivalCost(*this, arrivalCost);

				// Here legacy compares POINTERS (navigationcell.cpp:387-397), unlike
				// the branch above, which compares ids. Reproduced as written; the
				// two branches disagree in legacy and choosing between them would be
				// a silent behaviour change.
				if (caller == m_link[0])
				{
					session.SetArrivalWall(*this, 0);
				}
				else if (caller == m_link[1])
				{
					session.SetArrivalWall(*this, 1);
				}
				else if (caller == m_link[2])
				{
					session.SetArrivalWall(*this, 2);
				}

				session.AdjustCell(this);
				return true;
			}
		}

		return false;
	}

	float NavigationCell::ComputeHeuristic(const Vector3& goal) const noexcept
	{
		// The LONGEST axis delta, not the Euclidean distance
		// (navigationcell.cpp:419-427). Admissible, which is what keeps A* optimal.
		const float xDelta = std::fabs(goal.x - m_centerPoint.x);
		const float yDelta = std::fabs(goal.y - m_centerPoint.y);
		const float zDelta = std::fabs(goal.z - m_centerPoint.z);

		return (xDelta > yDelta) ? ((xDelta > zDelta) ? xDelta : zDelta)
		                        : ((yDelta > zDelta) ? yDelta : zDelta);
	}
}

