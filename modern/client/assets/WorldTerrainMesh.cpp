#include "assets/WorldTerrainMesh.h"

#include <cmath>

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