#pragma once

// WORLD-ENTRY-002d: line/triangle intersection, and the AABB-node slab test.
//
// ---------------------------------------------------------------------------
// WHY THESE TWO ARE HERE AND NOT IN A COLLISION LIBRARY
// ---------------------------------------------------------------------------
//
// `NavigationMesh::IsCollision` needs exactly two things from legacy's
// `DxCommon/Collision.h`: `DxAABBNode::IsCollision` (a segment-versus-box slab
// clip, Collision.cpp:1059-1173) and `COLLISION::IsLineTriangleCollision`
// (Collision.cpp:228-359). Together they are how the GOTO vertical probe finds
// the floor.
//
// Collision.h declares 30-odd functions covering spheres, view volumes, object
// trees and mesh bounds; none of the rest is reachable from navigation. Pulling
// in the header to get these two would drag `DxFrameMesh`, `DxLandMan`,
// `DxCustomTypes` and a `CLIPVOLUME` definition in with it, so the two
// transliterations live here instead and nothing else came along.
//
// The translation is faithful down to the quirks, because the quirks are the
// behaviour:
//
//   * `IsLineTriangleCollision` rejects a triangle with two equal axes outright
//     (Collision.cpp:254) - a degenerate sliver is treated as no collision.
//   * its degenerate "same X and same Y" branch (Collision.cpp:327-335) and its
//     dominant-axis branch (:339-349) both end at `IsPoint_Inside_Triangle`,
//     whose two-point-slope test is reproduced rather than replaced by a barycentric
//     one, because the two disagree on edge cases and those edge cases are
//     exactly what a floor probe lands on.
//   * `DxAABBNode::IsCollision` MUTATES its two endpoints, clipping them to the
//     box as it goes, so a caller walking a tree passes COPIES. The public
//     signature takes the endpoints by reference for that reason and is
//     documented as destructive.
//
// ---------------------------------------------------------------------------
// NO GLOBALS
// ---------------------------------------------------------------------------
//
// RAN stores the probe segment in two file-scope globals,
// `COLLISION::vColTestStart` and `vColTestEnd` (Collision.h:133-134), plus a
// `static BOOL s_bCollision` in navagationtree.cpp:427. That is safe only because
// a field server is single-threaded. Here the segment is a parameter, so the
// kernel is reentrant - which a modern server with a worker thread per Field
// connection requires.

#include "math/Vector2.h"
#include "math/Vector2.h"
#include "math/Vector3.h"

#include <cstdint>

#include <cstdint>

namespace Modern::Navigation
{
	// A node of the navigation mesh's bounding-volume tree.
	//
	// `kNoFace` marks an interior node. RAN spells the same value
	// `AABB_NONINDEX = 0xFFFFFFFF` (Collision.h:17).
	struct AabbNode
	{
		Vector3 vMax{-3.402823466e+38f, -3.402823466e+38f, -3.402823466e+38f};
		Vector3 vMin{3.402823466e+38f, 3.402823466e+38f, 3.402823466e+38f};

		std::uint32_t face = kNoFace;
		AabbNode*     left  = nullptr;
		AabbNode*     right = nullptr;

		static constexpr std::uint32_t kNoFace = 0xFFFFFFFFu;

		// Clips `p1`..`p2` against this box, MUTATING both endpoints.
		//
		// Returns false when the segment is entirely behind the box on some axis.
		// Returns true when the segment still may intersect it. The caller relies on
		// the mutation: the child nodes receive the already-clipped segment, which
		// is what makes the tree traversal cheap.
		bool IsCollision(Vector3& p1, Vector3& p2) const noexcept;
	};

	// Is `point` inside the triangle, projected away from one axis?
	//
	// `testX`/`testY`/`testZ` name the axes to KEEP. At least two must be true -
	// keeping fewer than two degenerates to a line, which the caller detects and
	// refuses (Collision.cpp:37-41).
	bool IsPointInsideTriangle(const Vector3& tri0, const Vector3& tri1, const Vector3& tri2,
	                           const Vector3& point, bool testX, bool testY,
	                           bool testZ) noexcept;

	// Does the segment `point1`..`point2` hit the triangle?
	//
	// `outCollision` receives the hit point when this returns true. `frontColl`
	// restricts the hit to the front face; navigation never sets it, and it is
	// here because the parameter changes the arithmetic
	// (Collision.cpp:265-270).
	bool IsLineTriangleCollision(const Vector3& tri0, const Vector3& tri1, const Vector3& tri2,
	                             Vector3& point1, Vector3& point2, Vector3& outCollision,
	                             Vector3* outNormal = nullptr,
	                             bool frontColl = false) noexcept;
}
