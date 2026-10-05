#include "navigation/TriangleCollision.h"

#include <cmath>

namespace Modern::Navigation
{
	namespace
	{
		// COLLISION::IsfEqual (Collision.h:150-155).
		bool IsfEqual(float a, float b) noexcept
		{
			const float d = a - b;
			return d < 0.001f && d > -0.001f;
		}
	}

	// ---------------------------------------------------------------------------
	// AabbNode::IsCollision
	// ---------------------------------------------------------------------------
	//
	// Collision.cpp:1059-1173, axis by axis. Each axis clips the segment to the
	// slab [fMin, fMax] and returns false when the whole segment is behind it.
	// The mutation of the endpoints is load-bearing: `NavigationMesh::IsCollision`
	// relies on the child receiving the clipped segment rather than re-clipping.
	bool AabbNode::IsCollision(Vector3& p1, Vector3& p2) const noexcept
	{
		float ratio      = 0.0f;
		Vector3 collision{};

		// X, min.  Below fMinX is "front".
		if (p1.x < vMin.x && p2.x < vMin.x)
		{
			return false;
		}
		if (p1.x < vMin.x || p2.x < vMin.x)
		{
			ratio     = (-p1.x + vMin.x) / (-p1.x + p2.x);
			collision = p1 + (p2 - p1) * ratio;
			if (p1.x < vMin.x) { p1 = collision; } else { p2 = collision; }
		}

		// X, max.
		if (p1.x > vMax.x && p2.x > vMax.x)
		{
			return false;
		}
		if (p1.x > vMax.x || p2.x > vMax.x)
		{
			ratio     = (p1.x - vMax.x) / (p1.x - p2.x);
			collision = p1 + (p2 - p1) * ratio;
			if (p1.x > vMax.x) { p1 = collision; } else { p2 = collision; }
		}

		// Y, min.
		if (p1.y < vMin.y && p2.y < vMin.y)
		{
			return false;
		}
		if (p1.y < vMin.y || p2.y < vMin.y)
		{
			ratio     = (-p1.y + vMin.y) / (-p1.y + p2.y);
			collision = p1 + (p2 - p1) * ratio;
			if (p1.y < vMin.y) { p1 = collision; } else { p2 = collision; }
		}

		// Y, max.
		if (p1.y > vMax.y && p2.y > vMax.y)
		{
			return false;
		}
		if (p1.y > vMax.y || p2.y > vMax.y)
		{
			ratio     = (p1.y - vMax.y) / (p1.y - p2.y);
			collision = p1 + (p2 - p1) * ratio;
			if (p1.y > vMax.y) { p1 = collision; } else { p2 = collision; }
		}

		// Z, min.
		if (p1.z < vMin.z && p2.z < vMin.z)
		{
			return false;
		}
		if (p1.z < vMin.z || p2.z < vMin.z)
		{
			ratio     = (-p1.z + vMin.z) / (-p1.z + p2.z);
			collision = p1 + (p2 - p1) * ratio;
			if (p1.z < vMin.z) { p1 = collision; } else { p2 = collision; }
		}

		// Z, max.
		if (p1.z > vMax.z && p2.z > vMax.z)
		{
			return false;
		}
		if (p1.z > vMax.z || p2.z > vMax.z)
		{
			ratio     = (p1.z - vMax.z) / (p1.z - p2.z);
			collision = p1 + (p2 - p1) * ratio;
			if (p1.z > vMax.z) { p1 = collision; } else { p2 = collision; }
		}

		return true;
	}

	// ---------------------------------------------------------------------------
	// IsPointInsideTriangle
	// ---------------------------------------------------------------------------
	//
	// Collision.cpp:20-223. Three edge lines as `y = m*x + b`, and the test point
	// must be on the same side of all three as the centroid.
	bool IsPointInsideTriangle(const Vector3& tri0, const Vector3& tri1, const Vector3& tri2,
	                           const Vector3& point, bool testX, bool testY,
	                           bool testZ) noexcept
	{
		// Keeping fewer than two axes degenerates to a line; RAN refuses it and so
		// does this. Collision.cpp:37-41.
		if ((!testX && !testY) || (!testX && !testZ) || (!testY && !testZ) ||
		    (!testX && !testY && !testZ))
		{
			return false;
		}

		Vector2 v1{};
		Vector2 v2{};
		Vector2 v3{};
		Vector2 vp{};

		// Drop Z - keep X and Y.
		if (!testX)
		{
			v1 = Vector2{tri0.y, tri0.z};
			v2 = Vector2{tri1.y, tri1.z};
			v3 = Vector2{tri2.y, tri2.z};
			vp = Vector2{point.y, point.z};
		}
		// Drop Y - keep X and Z.
		else if (!testY)
		{
			v1 = Vector2{tri0.x, tri0.z};
			v2 = Vector2{tri1.x, tri1.z};
			v3 = Vector2{tri2.x, tri2.z};
			vp = Vector2{point.x, point.z};
		}
		// Drop X - keep Y and Z.
		else
		{
			v1 = Vector2{tri0.x, tri0.y};
			v2 = Vector2{tri1.x, tri1.y};
			v3 = Vector2{tri2.x, tri2.y};
			vp = Vector2{point.x, point.y};
		}

		float b12Vertical = false;
		float b23Vertical = false;
		float b31Vertical = false;

		float m1 = 0.0f, m2 = 0.0f, m3 = 0.0f;
		float b1 = 0.0f, b2 = 0.0f, b3 = 0.0f;

		float deltaX = v2.x - v1.x;
		if (deltaX != 0.0f)
		{
			m1 = (v2.y - v1.y) / deltaX;
			b1 = v1.y - (m1 * v1.x);
		}
		else
		{
			b12Vertical = true;
		}

		deltaX = v3.x - v2.x;
		if (deltaX != 0.0f)
		{
			m2 = (v3.y - v2.y) / deltaX;
			b2 = v2.y - (m2 * v2.x);
		}
		else
		{
			b23Vertical = true;
		}

		deltaX = v1.x - v3.x;
		if (deltaX != 0.0f)
		{
			m3 = (v1.y - v3.y) / deltaX;
			b3 = v3.y - (m3 * v3.x);
		}
		else
		{
			b31Vertical = true;
		}

		// All three edges collinear: not a triangle.
		if (IsfEqual(m1, m2) && IsfEqual(m1, m3))
		{
			return false;
		}

		const Vector2 center{(v1.x + v2.x + v3.x) / 3.0f, (v1.y + v2.y + v3.y) / 3.0f};

		const bool up1 = (m1 * center.x + b1) >= center.y;
		const bool up2 = (m2 * center.x + b2) >= center.y;
		const bool up3 = (m3 * center.x + b3) >= center.y;

		int inside = 0;

		if (b12Vertical)
		{
			if ((v1.x < vp.x && v1.x < center.x) || (v1.x > vp.x && v1.x > center.x))
			{
				++inside;
			}
		}
		else if (up1)
		{
			if (vp.y <= (m1 * vp.x) + b1) { ++inside; }
		}
		else if (vp.y >= (m1 * vp.x) + b1) { ++inside; }

		if (b23Vertical)
		{
			if ((v2.x < vp.x && v2.x < center.x) || (v2.x > vp.x && v2.x > center.x))
			{
				++inside;
			}
		}
		else if (up2)
		{
			if (vp.y <= (m2 * vp.x) + b2) { ++inside; }
		}
		else if (vp.y >= (m2 * vp.x) + b2) { ++inside; }

		if (b31Vertical)
		{
			if ((v3.x < vp.x && v3.x < center.x) || (v3.x > vp.x && v3.x > center.x))
			{
				++inside;
			}
		}
		else if (up3)
		{
			if (vp.y <= (m3 * vp.x) + b3) { ++inside; }
		}
		else if (vp.y >= (m3 * vp.x) + b3) { ++inside; }

		return inside == 3;
	}

	// ---------------------------------------------------------------------------
	// IsLineTriangleCollision
	// ---------------------------------------------------------------------------
	//
	// Collision.cpp:228-359. The segment is intersected with the triangle's plane
	// and the hit is then confirmed to be inside the triangle.
	bool IsLineTriangleCollision(const Vector3& tri0, const Vector3& tri1, const Vector3& tri2,
	                             Vector3& point1, Vector3& point2, Vector3& outCollision,
	                             Vector3* outNormal, bool frontColl) noexcept
	{
		// RAN counts how many axes all three vertices agree on, and refuses the
		// triangle outright if more than one - a sliver seen edge-on.
		int  equalCount = 0;
		bool equalX = false, equalY = false, equalZ = false;

		if (IsfEqual(tri0.x, tri1.x) && IsfEqual(tri0.x, tri2.x))
		{
			equalX = true;
			++equalCount;
		}
		if (IsfEqual(tri0.y, tri1.y) && IsfEqual(tri0.y, tri2.y))
		{
			equalY = true;
			++equalCount;
		}
		if (IsfEqual(tri0.z, tri1.z) && IsfEqual(tri0.z, tri2.z))
		{
			equalZ = true;
			++equalCount;
		}

		if (equalCount > 1)
		{
			return false;
		}

		const Vector3 v1       = tri1 - tri0;
		const Vector3 v2       = tri2 - tri0;
		Vector3       vNormal  = Normalize(Cross(v1, v2));

		// A degenerate triangle normalises to (0,0,0); RAN would carry that through
		// and produce t = 0/0. Refusing it here is the same answer without the NaN.
		if (vNormal.IsZero())
		{
			return false;
		}

		// Front-face only: the segment must be heading into the front of the face.
		if (frontColl)
		{
			const Vector3 dir   = point2 - point1;
			const float   fDot  = Dot(dir, vNormal);
			if (fDot > 0.0f)
			{
				return false;
			}
		}

		const Vector3 vPoint = point1;

		// Where the segment crosses the plane.
		const Vector3 dxP1Tri0 = vPoint - tri0;
		const float   distance = Dot(dxP1Tri0, vNormal);
		const Vector3 dxP1P2   = vPoint - point2;
		const float   ratio    = distance / Dot(dxP1P2, vNormal);

		const Vector3 vCollision = vPoint + (point2 - vPoint) * ratio;

		// A vertical segment that starts and ends at the same XZ has no meaningful
		// plane crossing; RAN refuses it (Collision.cpp:291-298). A vertical GOTO
		// probe is exactly this case, and refusing it is why the probe looks for a
		// triangle whose plane the segment actually crosses.
		if ((vPoint.x == point2.x) && (vPoint.z == point2.z))
		{
			if (!(((vPoint.y <= vCollision.y) && (vCollision.y <= point2.y)) ||
			      ((point2.y <= vCollision.y) && (vCollision.y <= vPoint.y))))
			{
				return false;
			}
		}

		// The plane in Ax + By + Cz + D = 0 form, D from the FIRST vertex.
		const float planeA = vNormal.x;
		const float planeB = vNormal.y;
		const float planeC = vNormal.z;
		const float planeD = -(vNormal.x * tri0.x + vNormal.y * tri0.y + vNormal.z * tri0.z);

		// RAN's segment parameter t, written out longhand at Collision.cpp:309-315.
		// `i` is the plane equation evaluated at point2; `t` is rebuilt so that the
		// ratio is measured from point1 to point2 rather than point2 to point1.
		const float i = planeA * point2.x + planeB * point2.y + planeC * point2.z + planeD;
		float       t = -(planeA * point2.x + planeB * point2.y + planeC * point2.z);
		t += planeA * vPoint.x + planeB * vPoint.y + planeC * vPoint.z;
		t = (-i) / t;

		if (!(t >= 0.0f && t <= 1.0f))
		{
			return false;
		}

		// Is the hit inside the triangle?
		bool inside = false;

		if (equalX || equalY || equalZ)
		{
			if (equalX)
			{
				inside = IsPointInsideTriangle(tri0, tri1, tri2, vCollision, false, true, true);
			}
			else if (equalY)
			{
				inside = IsPointInsideTriangle(tri0, tri1, tri2, vCollision, true, false, true);
			}
			else
			{
				inside = IsPointInsideTriangle(tri0, tri1, tri2, vCollision, true, true, false);
			}
		}
		else
		{
			// RAN runs all three dominant-axis tests without an else chain
			// (Collision.cpp:339-349), so the LAST one that matches wins. Reproduced
			// as written: the three are mutually exclusive except on exact ties, and
			// on a tie the Z branch's answer is the one RAN keeps.
			if ((std::fabs(planeA) >= std::fabs(planeB)) && (std::fabs(planeA) >= std::fabs(planeC)))
			{
				inside = IsPointInsideTriangle(tri0, tri1, tri2, vCollision, false, true, true);
			}
			if ((std::fabs(planeB) >= std::fabs(planeA)) && (std::fabs(planeB) >= std::fabs(planeC)))
			{
				inside = IsPointInsideTriangle(tri0, tri1, tri2, vCollision, true, false, true);
			}
			if ((std::fabs(planeC) >= std::fabs(planeA)) && (std::fabs(planeC) >= std::fabs(planeB)))
			{
				inside = IsPointInsideTriangle(tri0, tri1, tri2, vCollision, true, true, false);
			}
		}

		if (!inside)
		{
			return false;
		}

		outCollision = vCollision;
		if (outNormal != nullptr)
		{
			*outNormal = vNormal;
		}
		return true;
	}
}
