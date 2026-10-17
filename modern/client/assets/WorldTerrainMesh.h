#pragma once

// WORLD-PLAYABLE-001B: turn parsed ASURA terrain into renderable CPU-side meshes.
//
// This is the seam between the WORLD-PLAYABLE-001A format reader and the
// renderer's upload boundary. It is deliberately the *only* place that knows
// both: the parser stays CPU-side and Direct3D-free, and the renderer never sees
// a legacy type. Nothing here is ASURA-specific apart from the input struct, and
// nothing here is Direct3D-specific apart from the output struct -- both of
// which are already the project's existing asset types.
//
// WINDING. Indices are passed through in file order, deliberately. Measured on
// the real assets: for `data/map/bambooforest.wld0` (103,351,702 bytes, 160,095
// triangles) the right-handed face normal `cross(B-A, C-A)` agrees with the
// stored per-vertex normal in 53,357 of 53,358 non-degenerate triangles and
// disagrees in exactly one. Across the 23 smaller fields the same holds, with the
// disagreement count in the single digits. The file's index order is therefore
// already the order that produces an outward (up-facing) geometric normal, so
// flipping it would invert the lighting instead of fixing it. The renderer
// leaves back-face culling at the project-wide default it already uses for
// characters, which is why winding does not affect visibility today; that default
// is a cross-backend convention question, not something to change while adding
// terrain.

#include "WorldStaticMesh.h"
#include "assets/MeshAsset.h"
#include "math/Vector3.h"
#include "types/Result.h"

#include <cstddef>
#include <vector>

namespace Modern::Client
{
	// Every renderable leaf of one field, plus the scene bounds and the counts a
	// caller needs to log what it got and to release what it uploaded.
	struct TerrainMeshSet
	{
		// One mesh per decoded leaf that fitted the renderer's limits, in file order.
		std::vector<MeshAsset> meshes;

		// Axis-aligned bounds of the geometry actually kept, which is what the
		// camera should frame. Zero-sized when `meshes` is empty.
		Vector3 boundsMin = Vector3::Zero;
		Vector3 boundsMax = Vector3::Zero;

		size_t totalVertices = 0;
		size_t totalIndices  = 0;

		// Leaves that were recognised but could not become a mesh: no geometry at
		// all, or a leaf over `kMaxMeshVertices` / `kMaxMeshIndices` / the 8 MiB mesh
		// byte ceiling. Reported rather than silently dropped, and never fatal --
		// one oversized leaf must not cost a whole field.
		size_t skippedLeaves = 0;

		// Why they were skipped, once each, for the diagnostic log.
		struct SkipReason
		{
			const char* reason = "";
			size_t      count  = 0;
		};
		std::vector<SkipReason> skipReasons;
	};

	// Build renderable meshes from a parsed field.
	//
	// Returns an error only when the input cannot be used at all (an empty field,
	// or a field whose every leaf is unusable). Per-leaf limits are enforced by
	// skipping that leaf and counting it, so a field with one bad leaf still
	// renders the rest.
	Result<TerrainMeshSet> BuildTerrainMeshes(const Legacy::WorldStaticMeshAsset& asset);

} // namespace Modern::Client