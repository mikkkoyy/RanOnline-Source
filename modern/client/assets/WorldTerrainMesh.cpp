#include "assets/WorldTerrainMesh.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace Modern::Client
{
	namespace
	{
		// Terrain leaves that declare no normal -- the FVF forms without
		// D3DFVF_NORMAL, e.g. 0x142 -- still need one for lighting. Terrain is
		// overwhelmingly upward-facing, so a fixed up vector is the honest
		// approximation; the alternative, deriving a normal per vertex from
		// neighbouring faces, is a smoothing decision this milestone does not make.
		const Vector3 kFallbackTerrainNormal = Vector3::UnitY;

		// A leaf is usable only if it has a complete vertex and index stream. The
		// parser already refuses non-finite components and out-of-range indices, so
		// this only guards against a leaf that arrived empty.
		struct LeafShape
		{
			size_t vertices = 0;
			size_t indices  = 0;
		};

		bool ReadLeafShape(const Legacy::WorldTerrainChunk& chunk, LeafShape& shape) noexcept
		{
			const size_t positions = chunk.positions.size();
			const size_t indices   = chunk.indices.size();
			if (positions == 0 || indices == 0) { return false; }
			if (positions % 3u != 0u) { return false; }
			if (indices % 3u != 0u) { return false; }

			shape.vertices = positions / 3u;
			shape.indices  = indices;
			return shape.vertices != 0u && shape.indices != 0u;
		}

		// A normal for vertex `v`. Uses the file's own normal when the FVF carried
		// one, and only falls back when it did not -- or when the stored normal is
		// degenerate, which `MeshAsset` would reject as a non-finite-free but
		// useless vector anyway.
		Vector3 NormalFor(const Legacy::WorldTerrainChunk& chunk, size_t vertex) noexcept
		{
			if (chunk.hasNormals && (vertex + 1u) * 3u <= chunk.normals.size())
			{
				const Vector3 stored(
				    chunk.normals[vertex * 3u + 0u],
				    chunk.normals[vertex * 3u + 1u],
				    chunk.normals[vertex * 3u + 2u]);
				if (stored.IsFinite() && !stored.IsZero())
				{
					const Vector3 unit = Normalize(stored);
					if (!unit.IsZero()) { return unit; }
				}
			}
			return kFallbackTerrainNormal;
		}

		void NoteSkip(TerrainMeshSet& set, const char* reason)
		{
			for (TerrainMeshSet::SkipReason& entry : set.skipReasons)
			{
				if (entry.reason == reason) { ++entry.count; return; }
			}
			set.skipReasons.push_back(TerrainMeshSet::SkipReason{ reason, 1u });
		}

		// True when (px, pz) lies inside the triangle's XZ projection, using
		// barycentric coordinates. On the edge counts as inside.
		bool PointInTriangleXZ(float px, float pz,
		                       const Vector3& a, const Vector3& b, const Vector3& c) noexcept
		{
			const float v0x = c.x - a.x, v0z = c.z - a.z;
			const float v1x = b.x - a.x, v1z = b.z - a.z;
			const float v2x = px - a.x,       v2z = pz - a.z;

			const float dot00 = v0x * v0x + v0z * v0z;
			const float dot01 = v0x * v1x + v0z * v1z;
			const float dot02 = v0x * v2x + v0z * v2z;
			const float dot11 = v1x * v1x + v1z * v1z;
			const float dot12 = v1x * v2x + v1z * v2z;

			const float denom = dot00 * dot11 - dot01 * dot01;
			if (denom == 0.0f) { return false; }

			const float invDenom = 1.0f / denom;
			const float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
			const float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

			return u >= 0.0f && v >= 0.0f && (u + v) <= 1.0f;
		}

		// Computes Y at (px, pz) on the triangle's plane, assuming the point is
		// inside the triangle's XZ projection. Returns NaN when the triangle is
		// vertical (no unique Y).
		float TriangleYAt(float px, float pz,
		                  const Vector3& a, const Vector3& b, const Vector3& c) noexcept
		{
			// Plane normal: cross(B-A, C-A)
			const Vector3 ab = b - a;
			const Vector3 ac = c - a;
			const Vector3 n  = Cross(ab, ac);

			// Ray: P(t) = (px, startY, pz) + t * (0, -1, 0)
			// Plane: n · (P - a) = 0
			//   n.x * (px - a.x) + n.y * (startY - a.y) + n.z * (pz - a.z) - t * n.y = 0
			//   t = n · ((px, startY, pz) - a) / n.y
			const float ny = n.y;
			if (ny == 0.0f) { return std::numeric_limits<float>::quiet_NaN(); }

			// Start ray high enough above any terrain; exact startY cancels out
			// because only the difference matters:
			//   intersectionY = startY - n · ((px, startY, pz) - a) / ny
			//   = startY - (n.x*(px-a.x) + n.y*(startY-a.y) + n.z*(pz-a.z)) / ny
			//   = startY - n.x*(px-a.x)/ny - n.y*(startY-a.y)/ny - n.z*(pz-a.z)/ny
			//   = startY - n.x*(px-a.x)/ny - (startY-a.y) - n.z*(pz-a.z)/ny
			//   = a.y - n.x*(px-a.x)/ny - n.z*(pz-a.z)/ny
			const float y = a.y - (n.x * (px - a.x) + n.z * (pz - a.z)) / ny;
			return y;
		}

		Result<MeshAsset> BuildLeafMesh(const Legacy::WorldTerrainChunk& chunk, const LeafShape& shape)
		{
			// Reserved exactly, so a leaf cannot be half-built before the check that
			// rejects it: the parser's ceilings are far above the renderer's, so this
			// is the binding limit for a single leaf.
			std::vector<MeshVertex> vertices;
			vertices.reserve(shape.vertices);

			for (size_t v = 0; v < shape.vertices; ++v)
			{
				const Vector3 position(
				    chunk.positions[v * 3u + 0u],
				    chunk.positions[v * 3u + 1u],
				    chunk.positions[v * 3u + 2u]);

				float u = 0.0f;
				float v2 = 0.0f;
				if (chunk.hasUvs && (v + 1u) * 2u <= chunk.uvs.size())
				{
					u  = chunk.uvs[v * 2u + 0u];
					v2 = chunk.uvs[v * 2u + 1u];
				}

				vertices.push_back(MeshVertex(position, NormalFor(chunk, v), u, v2));
			}

			std::vector<MeshIndex> indices;
			indices.reserve(shape.indices);
			for (const uint16_t index : chunk.indices)
			{
				indices.push_back(static_cast<MeshIndex>(index));
			}

			// `MeshAsset::Create` is the renderer's own validation boundary: it
			// re-checks finiteness, index range and the 8 MiB / vertex / index
			// ceilings, so a leaf that passes it is genuinely uploadable.
			return MeshAsset::Create(PrimitiveTopology::TriangleList,
			                         std::move(vertices),
			                         std::move(indices));
		}
	} // namespace

	// Queries terrain height at (x, z) by casting a ray straight down from
	// above and returning the Y of the closest triangle hit. Returns nullopt
	// when no triangle is below the point.
	std::optional<float> QueryGroundHeight(const TerrainMeshSet& set, float x,
	                                       float z) noexcept
	{
		std::optional<float> bestY;

		for (const GroundMesh& gm : set.groundMeshes)
		{
			// XZ bounds cull: skip this mesh entirely when (x, z) is outside.
			if (x < gm.boundsMin.x || x > gm.boundsMax.x ||
			    z < gm.boundsMin.z || z > gm.boundsMax.z)
			{
				continue;
			}

			for (const GroundTriangle& tri : gm.triangles)
			{
				if (!PointInTriangleXZ(x, z, tri.a, tri.b, tri.c)) { continue; }

				const float y = TriangleYAt(x, z, tri.a, tri.b, tri.c);
				if (!std::isfinite(y)) { continue; }

				// Ground must be below the character's feet: we cast from above,
				// so any hit is below. Keep the highest hit (closest from above).
				if (!bestY.has_value() || y > *bestY) { bestY = y; }
			}
		}

		return bestY;
	}

	Result<TerrainMeshSet> BuildTerrainMeshes(const Legacy::WorldStaticMeshAsset& asset)
	{
		TerrainMeshSet set;

		bool haveBounds = false;
		for (int b = 0; b < 5; ++b)
		{
			for (const Legacy::WorldTerrainBucket& mesh : asset.buckets[b])
			{
				for (const Legacy::WorldTerrainChunk& chunk : mesh.chunks)
				{
					LeafShape shape;
					if (!ReadLeafShape(chunk, shape))
					{
						++set.skippedLeaves;
						NoteSkip(set, "leaf has no usable geometry");
						continue;
					}

					// The renderer's ceilings, checked here so an oversized leaf is
					// *skipped* rather than turned into a whole-field failure.
					if (shape.vertices > kMaxMeshVertices ||
					    shape.indices  > kMaxMeshIndices)
					{
						++set.skippedLeaves;
						NoteSkip(set, "leaf exceeds the renderer vertex or index limit");
						continue;
					}

					Result<MeshAsset> built = BuildLeafMesh(chunk, shape);
					if (built.IsError())
					{
						++set.skippedLeaves;
						NoteSkip(set, "leaf failed MeshAsset validation");
						continue;
					}

					const MeshAsset& mesh = built.GetValue();
					for (const MeshVertex& vertex : mesh.GetVertices())
					{
						const Vector3& p = vertex.position;
						if (!haveBounds)
						{
							set.boundsMin = p;
							set.boundsMax = p;
							haveBounds = true;
						}
						else
						{
							set.boundsMin.x = std::min(set.boundsMin.x, p.x);
							set.boundsMin.y = std::min(set.boundsMin.y, p.y);
							set.boundsMin.z = std::min(set.boundsMin.z, p.z);
							set.boundsMax.x = std::max(set.boundsMax.x, p.x);
							set.boundsMax.y = std::max(set.boundsMax.y, p.y);
							set.boundsMax.z = std::max(set.boundsMax.z, p.z);
						}
					}

					set.totalVertices += mesh.GetVertexCount();
					set.totalIndices  += mesh.GetIndexCount();
					set.meshes.push_back(std::move(built.GetValue()));

					// Build ground-query data for this mesh from the raw chunk
					// data (positions + indices), before it is moved away.
					GroundMesh gm;
					bool gmHaveBounds = false;
					const std::vector<MeshVertex>& gv = set.meshes.back().GetVertices();
					const std::vector<MeshIndex>&  gi = set.meshes.back().GetIndices();
					for (size_t i = 0; i + 2 < gi.size(); i += 3)
					{
						const MeshIndex ia = gi[i];
						const MeshIndex ib = gi[i + 1];
						const MeshIndex ic = gi[i + 2];
						if (ia >= gv.size() || ib >= gv.size() || ic >= gv.size())
						{
							continue;
						}
						GroundTriangle tri;
						tri.a = gv[ia].position;
						tri.b = gv[ib].position;
						tri.c = gv[ic].position;

						if (!gmHaveBounds)
						{
							gm.boundsMin = Vector3(std::min({tri.a.x, tri.b.x, tri.c.x}),
							                       std::min({tri.a.y, tri.b.y, tri.c.y}),
							                       std::min({tri.a.z, tri.b.z, tri.c.z}));
							gm.boundsMax = Vector3(std::max({tri.a.x, tri.b.x, tri.c.x}),
							                       std::max({tri.a.y, tri.b.y, tri.c.y}),
							                       std::max({tri.a.z, tri.b.z, tri.c.z}));
							gmHaveBounds = true;
						}
						else
						{
							gm.boundsMin.x = std::min(gm.boundsMin.x, std::min({tri.a.x, tri.b.x, tri.c.x}));
							gm.boundsMin.y = std::min(gm.boundsMin.y, std::min({tri.a.y, tri.b.y, tri.c.y}));
							gm.boundsMin.z = std::min(gm.boundsMin.z, std::min({tri.a.z, tri.b.z, tri.c.z}));
							gm.boundsMax.x = std::max(gm.boundsMax.x, std::max({tri.a.x, tri.b.x, tri.c.x}));
							gm.boundsMax.y = std::max(gm.boundsMax.y, std::max({tri.a.y, tri.b.y, tri.c.y}));
							gm.boundsMax.z = std::max(gm.boundsMax.z, std::max({tri.a.z, tri.b.z, tri.c.z}));
						}
						gm.triangles.push_back(tri);
					}
					set.groundMeshes.push_back(std::move(gm));
					set.groundMeshes.push_back(std::move(gm));
				}
			}
		}

		if (set.meshes.empty())
		{
			// A field with nothing renderable in it is an error for the caller: it
			// cannot draw anything, and reporting success would hide that.
			return Result<TerrainMeshSet>(Status(ErrorCode::InvalidArgument));
		}

		return set;
	}

} // namespace Modern::Client