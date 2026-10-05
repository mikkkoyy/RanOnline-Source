#pragma once

// WORLD-ENTRY-002d: the two geometric primitives every RAN navigation query is
// built from.
//
// `Line2D` and `Plane` are both header-only in legacy
// (legacy/Lib_Engine/NaviMesh/line2d.h, plane.h) and both are pure arithmetic,
// so they are header-only here too. Keeping them in one file rather than two is
// deliberate: they are 190 lines between them, they are always used together,
// and splitting them would put a file boundary in the middle of the geometry.
//
// ---------------------------------------------------------------------------
// LINE2D - A SEGMENT ON THE XZ PLANE
// ---------------------------------------------------------------------------
//
// Transcribed from legacy/Lib_Engine/NaviMesh/line2d.h. The normalisation is
// the part that matters and is easy to get wrong:
//
//     GetDirection(m_Normal);            // unit vector A -> B
//     float OldYValue = m_Normal[1];
//     m_Normal[1] = -m_Normal[0];
//     m_Normal[0] = OldYValue;           // rotate by -90 degrees
//
// (line2d.h:348-360). That is a CLOCKWISE rotation of the direction vector,
// and the normal points to the right-hand side of the line when viewed from A
// towards B. `NavigationCell::ClassifyPathToCell` then treats "right of every
// wall" as "inside the cell" (navigationcell.cpp:57, :83), which is only true
// for that rotation direction. Flipping it would make every inside test
// inverted, and the symptom would be movement that never finds its destination
// rather than an obvious crash.
//
// `m_NormalCalculated` is preserved as MUTABLE CACHED state exactly as legacy
// has it (line2d.h:75): the normal is derived, and recomputing it on every
// `SignedDistance` would be both slower and - because `SignedDistance` is
// const and the cache is mutable - behaviourally identical but needlessly
// expensive inside A*.
//
// ---------------------------------------------------------------------------
// PLANE - POINT-NORMAL FORM
// ---------------------------------------------------------------------------
//
// Transcribed from legacy/Lib_Engine/NaviMesh/plane.h. The sign convention is
// the load-bearing part:
//
//     m_Distance = -dot(m_Point, m_Normal)        (plane.h:197)
//
// so the plane is `dot(N, X) + D = 0`, and `SolveForY` is
// `-(Nx*x + Nz*z + D) / Ny` (plane.h:154). RAN only ever solves for Y on a
// navigation cell - that is how `MapVectorHeightToCell` places a walking
// character on the floor (navigationcell.h:342) - but X and Z are here because
// the file is 28 bytes on disk and reading it back means reproducing all three
// members.
//
// A degenerate plane - one whose Y normal is zero, i.e. a vertical wall -
// returns 0.0f from `SolveForY` rather than dividing by zero (plane.h:162-165).
// That is deliberate and it matters: `Actor::Update` calls
// `MapVectorHeightToCell` on every step, and the vertical-wall case is exactly
// the case where the normal test at actor.cpp:361 has already decided to freeze
// Y instead.

#include "math/Vector2.h"
#include "math/Vector3.h"

namespace Modern::Navigation
{
	class Line2D
	{
	public:
		enum PointClassification
		{
			OnLine,
			LeftSide,
			RightSide,
		};

		enum LineClassification
		{
			Collinear,
			LinesIntersect,
			SegmentsIntersect,
			ABisectsB,
			BBisectsA,
			Parallel,
		};

		Line2D() = default;
		Line2D(const Vector2& a, const Vector2& b) noexcept : m_pointA(a), m_pointB(b) {}

		void SetEndPointA(const Vector2& point) noexcept
		{
			m_pointA      = point;
			m_normalValid = false;
		}

		void SetEndPointB(const Vector2& point) noexcept
		{
			m_pointB      = point;
			m_normalValid = false;
		}

		void SetPoints(const Vector2& a, const Vector2& b) noexcept
		{
			m_pointA = a;
			m_pointB = b;
			m_normalValid = false;
		}

		const Vector2& EndPointA() const noexcept { return m_pointA; }
		const Vector2& EndPointB() const noexcept { return m_pointB; }

		// The unit right-hand normal. Computed on first use and cached.
		const Vector2& Normal() const noexcept
		{
			if (!m_normalValid)
			{
				ComputeNormal();
			}
			return m_normal;
		}

		float Length() const noexcept { return (m_pointB - m_pointA).Length(); }

		// Positive when `point` is to the RIGHT of A->B.
		//
		// This is the primitive every cell test is made of: `IsPointInCellCollumn`
		// counts how many walls a point is right of (navigationcell.h:361-369), and
		// `ForcePointToWallInterior` pushes a point along this normal to get it
		// inside (navigationcell.cpp:167).
		float SignedDistance(const Vector2& point) const noexcept
		{
			if (!m_normalValid)
			{
				ComputeNormal();
			}
			return Dot(point - m_pointA, m_normal);
		}

		PointClassification ClassifyPoint(const Vector2& point,
		                                  float epsilon = 0.0f) const noexcept
		{
			const float distance = SignedDistance(point);
			if (distance > epsilon)
			{
				return RightSide;
			}
			if (distance < -epsilon)
			{
				return LeftSide;
			}
			return OnLine;
		}

		// Where this segment meets `other`, if it does.
		//
		// Transcribed from line2d.h:254-314 including the epsilon-free "posting on
		// a vertex" case that legacy deliberately commented out (line2d.h:282-290):
		// no nudge, so a path that lands exactly on a shared vertex reports the
		// classification the raw arithmetic gives. `ResolveMotionOnMesh`'s
		// `int(x) != int(x)` progress test (navigationmesh.cpp:262) is written
		// against exactly that behaviour.
		LineClassification Intersection(const Line2D& other,
		                                Vector2* intersectionPoint = nullptr) const noexcept
		{
			const float ayMinusCy = m_pointA.y - other.m_pointA.y;
			const float dxMinusCx = other.m_pointB.x - other.m_pointA.x;
			const float axMinusCx = m_pointA.x - other.m_pointA.x;
			const float dyMinusCy = other.m_pointB.y - other.m_pointA.y;
			const float bxMinusAx = m_pointB.x - m_pointA.x;
			const float byMinusAy = m_pointB.y - m_pointA.y;

			const float numerator   = (ayMinusCy * dxMinusCx) - (axMinusCx * dyMinusCy);
			const float denominator = (bxMinusAx * dyMinusCy) - (axMinusCx * dxMinusCx);

			if (denominator == 0.0f)
			{
				return numerator == 0.0f ? Collinear : Parallel;
			}

			const float factorAB = numerator / denominator;
			const float factorCD =
			    ((ayMinusCy * bxMinusAx) - (axMinusCx * byMinusAy)) / denominator;

			if (intersectionPoint != nullptr)
			{
				intersectionPoint->x = m_pointA.x + (factorAB * bxMinusAx);
				intersectionPoint->y = m_pointA.y + (factorAB * byMinusAy);
			}

			if (factorAB >= 0.0f && factorAB <= 1.0f && factorCD >= 0.0f && factorCD <= 1.0f)
			{
				return SegmentsIntersect;
			}
			if (factorCD >= 0.0f && factorCD <= 1.0f)
			{
				return ABisectsB;
			}
			if (factorAB >= 0.0f && factorAB <= 1.0f)
			{
				return BBisectsA;
			}
			return LinesIntersect;
		}

	private:
		// See the header: a CLOCKWISE rotation of the direction vector.
		void ComputeNormal() const noexcept
		{
			m_normal     = Normalize(m_pointB - m_pointA);
			const float oldY = m_normal.y;
			m_normal.y   = -m_normal.x;
			m_normal.x   = oldY;
			m_normalValid = true;
		}

		Vector2 m_pointA{};
		Vector2 m_pointB{};

		mutable Vector2 m_normal{};
		mutable bool    m_normalValid = false;
	};

	// A plane in point-normal form: dot(N, X) + D = 0.
	//
	// The three members are exactly the 28 bytes `NavigationCell::SaveFile`
	// writes (NavigationSaveLoad.cpp:114), so this type is also the on-disk
	// shape. The loader memcpy-equivalents it rather than recomputing it, because
	// the file's values are authoritative - a recomputed plane could differ in the
	// last bit and `Normal().y <= 0.0001f` would then classify a cell differently.
	class Plane
	{
	public:
		Vector3 m_normal{};
		Vector3 m_point{};
		float   m_distance = 0.0f;

		Plane() = default;

		Plane(const Vector3& p0, const Vector3& p1, const Vector3& p2) noexcept
		{
			Set(p0, p1, p2);
		}

		void Set(const Vector3& p0, const Vector3& p1, const Vector3& p2) noexcept
		{
			const Vector3 v1 = p1 - p0;
			const Vector3 v2 = p2 - p0;
			m_normal   = Normalize(Cross(v1, v2));
			m_point    = p0;
			m_distance = -Dot(m_point, m_normal);
		}

		const Vector3& Normal() const noexcept { return m_normal; }
		const Vector3& Point() const noexcept { return m_point; }
		float Distance() const noexcept { return m_distance; }

		// 0.0f for a degenerate axis rather than a division by zero - see the
		// header note on vertical walls.
		float SolveForX(float y, float z) const noexcept
		{
			return m_normal.x != 0.0f ? -(m_normal.y * y + m_normal.z * z + m_distance) /
			                                m_normal.x
			                          : 0.0f;
		}

		float SolveForY(float x, float z) const noexcept
		{
			return m_normal.y != 0.0f ? -(m_normal.x * x + m_normal.z * z + m_distance) /
			                                m_normal.y
			                          : 0.0f;
		}

		float SolveForZ(float x, float y) const noexcept
		{
			return m_normal.z != 0.0f ? -(m_normal.x * x + m_normal.y * y + m_distance) /
			                                m_normal.z
			                          : 0.0f;
		}

		friend bool operator==(const Plane& a, const Plane& b)
		{
			return a.m_normal == b.m_normal && a.m_point == b.m_point;
		}
	};
}
