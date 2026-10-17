#pragma once

// WORLD-PLAYABLE-001A: the .wld0 static-mesh terrain reader.
//
// RAN stores a field's geometry in a sibling file, not in the `.wld` itself:
// `DxLandManSaveLoad.cpp` reads `map.wld` for metadata (fog, sky, lights,
// piece *names*), and `DxStaticMesh::Load_File` opens `map.wld0` for the
// geometry. The `.wld` alone is not the terrain.
//
// Every serialized layout below was read out of the legacy readers that produce
// and consume these files, and then confirmed byte-for-byte against real ASURA
// `.wld0` assets:
//
//   legacy/Lib_Engine/Common/SerialFile.cpp        the 132-byte file header
//   legacy/Lib_Engine/Common/SerialFile.cpp        BeginBlock/EndBlock sizing
//   legacy/Lib_Engine/DxEffect/DxStaticMesh.cpp    DxStaticMesh::Load / Load_102
//   legacy/Lib_Engine/DxEffect/DxStaticMesh.cpp    SingleTexMesh_Load_Insert
//   legacy/Lib_Engine/DxCommon/NsOCTreeSaveLoad.cpp DxSingleTexMesh::Save / Load
//   legacy/Lib_Engine/DxCommon/NsOCTreeSaveLoad.cpp DxAABBOctree::Save / Load
//   legacy/Lib_Engine/DxCommon/NsOCTreeSaveLoad.cpp OBJOCTree::Save / Load
//   legacy/Lib_Engine/DxCommon/NsOCTreeSaveLoad.cpp OBJOCTree::DynamicLoad
//   legacy/Lib_Engine/DxCommon/NsOCTreeSaveLoad.cpp OBJOCTree::OctreeLoad
//   legacy/Lib_Engine/DxCommon/Collision.cpp        DxAABBNode::SaveFile/LoadFile
//   legacy/Lib_Engine/DxEffect/DxTexEff*.cpp        the text-effect record shape
//
// THE ONE RULE THAT MATTERS MOST: a declared size is a *block* size, and
// `CSerialFile::EndBlock` defines it as the byte count between the size DWORD
// and the matching `EndBlock` -- nothing else. In particular the enclosing
// header is NOT counted, and for three of the four nested blocks the octree
// that follows is NOT counted:
//
//   DxStaticMesh::Save    block = { vMax, vMin }                      (24 bytes)
//   DxSingleTexMesh::Save block = { flag, texName, effCount, effs }
//   DxAABBOctree::Save    block = { vMax, vMin, exists, NSOCTREE }
//   OBJOCTree::Save       block = { geomExists, geom, collExists, coll }
//
// The AABB octree is therefore bounded *around* the whole node tree, while a
// single-texture mesh's block stops before its octree and an OBJOCTree's block
// stops before its own children. Getting this wrong makes a conforming reader
// either refuse every real file or, worse, silently resynchronise onto
// garbage, so this reader gives each block its own bounded sub-cursor rather
// than treating a size as advisory.
//
// The reader is deliberately independent of Direct3D: it returns CPU-side
// records a later milestone can upload in bounded chunks. The 8 MiB `MeshAsset`
// limit is *not* raised here -- the point of returning per-leaf records is that
// a large field never has to become one asset.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "types/Result.h"

namespace Modern
{
	namespace Legacy
	{
		// One decoded octree leaf. The vertex layout is decoded explicitly rather
		// than reinterpreting the serialized bytes, so an unsupported FVF is a
		// refusal rather than a reinterpretation.
		struct WorldTerrainChunk
		{
			// Serialized object bounds, as read (D3DXVECTOR3 triples).
			float boundsMin[3] = { 0.0f, 0.0f, 0.0f };
			float boundsMax[3] = { 0.0f, 0.0f, 0.0f };

			// The texture this chunk is drawn with. Empty means untextured.
			std::string textureName;

			// Decoded geometry. Sizes are exact: `vertexCount * stride` bytes were
			// consumed, `indices.size()` WORDs. Empty when the leaf carries no
			// geometry or the record was not decoded.
			std::vector<float>    positions;   // 3 per vertex
			std::vector<float>    normals;     // 3 per vertex, may be empty
			std::vector<float>    uvs;         // 2 per vertex, may be empty
			std::vector<uint32_t> colors;      // 1 per vertex, may be empty
			std::vector<uint16_t> indices;     // 3 per triangle

			// Which components the vertex format actually carried, so a caller
			// never has to guess from an empty vector.
			bool hasNormals = false;
			bool hasUvs     = false;
			bool hasColors  = false;

			// The whole FVF value the leaf declared, so diagnostics can report it
			// rather than a lossy description of it.
			uint32_t fvf = 0;

			uint32_t vertexCount() const noexcept { return static_cast<uint32_t>(positions.size() / 3u); }
			uint32_t triangleCount() const noexcept { return static_cast<uint32_t>(indices.size() / 3u); }
		};

		// Named single-texture mesh: one leaf-record group under one texture.
		struct WorldTerrainBucket
		{
			std::string                            name;
			std::vector<WorldTerrainChunk>         chunks;
		};

		// The whole field.
		//
		// `DxStaticMesh::Load_200` reads five buckets in this order:
		//
		//   0 opaque   1 soft-alpha   2 soft-alpha 0   3 soft-alpha 1   4 alpha
		//
		// (`SingleTexMesh_Save` writes m_mapMesh, SOFTALPHA, SOFTALPHA01,
		// SOFTALPHA02, ALPHA -- note that alpha comes *last*, not first.)
		struct WorldStaticMeshAsset
		{
			// Tile bounds straight from the file header.
			float tileBoundsMin[3] = { 0.0f, 0.0f, 0.0f };
			float tileBoundsMax[3] = { 0.0f, 0.0f, 0.0f };

			std::vector<WorldTerrainBucket> buckets[5];

			// The number of decoded leaf records. Not the number of bucket
			// entries: a mesh whose octree this reader could not decode still
			// appears in `buckets` but contributes no chunk.
			size_t totalChunks() const noexcept
			{
				size_t total = 0;
				for (const std::vector<WorldTerrainBucket>& bucket : buckets)
				{
					for (const WorldTerrainBucket& mesh : bucket) { total += mesh.chunks.size(); }
				}
				return total;
			}

			// The number of named meshes the file listed, decoded or not.
			size_t totalMeshes() const noexcept
			{
				size_t total = 0;
				for (const std::vector<WorldTerrainBucket>& bucket : buckets) { total += bucket.size(); }
				return total;
			}

			size_t totalVertices() const noexcept
			{
				size_t total = 0;
				for (const std::vector<WorldTerrainBucket>& bucket : buckets)
				{
					for (const WorldTerrainBucket& mesh : bucket)
					{
						for (const WorldTerrainChunk& chunk : mesh.chunks) { total += chunk.vertexCount(); }
					}
				}
				return total;
			}

			size_t totalIndices() const noexcept
			{
				size_t total = 0;
				for (const std::vector<WorldTerrainBucket>& bucket : buckets)
				{
					for (const WorldTerrainBucket& mesh : bucket)
					{
						for (const WorldTerrainChunk& chunk : mesh.chunks) { total += chunk.indices.size(); }
					}
				}
				return total;
			}

			// Bytes of decoded CPU-side geometry held by the returned records. This
			// is the figure that decides whether a field can be uploaded at all,
			// so it is reported rather than left to the caller to recompute.
			size_t decodedBytes() const noexcept
			{
				size_t total = 0;
				for (const std::vector<WorldTerrainBucket>& bucket : buckets)
				{
					for (const WorldTerrainBucket& mesh : bucket)
					{
						for (const WorldTerrainChunk& chunk : mesh.chunks)
						{
							total += chunk.positions.size() * sizeof(float);
							total += chunk.normals.size()   * sizeof(float);
							total += chunk.uvs.size()       * sizeof(float);
							total += chunk.colors.size()    * sizeof(uint32_t);
							total += chunk.indices.size()   * sizeof(uint16_t);
							total += chunk.textureName.size();
						}
					}
				}
				return total;
			}

			// Diagnostics reported by the real-asset validation cases. These are
			// counts of records that were recognised but not decoded, so a caller
			// can tell "no geometry" from "geometry this reader does not model".
			size_t unsupportedFvfCount() const noexcept { return unsupportedFvf; }
			size_t skippedRecordCount() const noexcept { return skippedRecord; }

			// Every FVF this reader refused, in file order, with how many leaves
			// declared it. Empty for a field this reader fully understands.
			struct UnsupportedFvfEntry
			{
				uint32_t fvf    = 0;
				size_t   leaves = 0;
			};
			const std::vector<UnsupportedFvfEntry>& unsupportedFvfs() const noexcept { return rejectedFvfs; }

			size_t unsupportedFvf = 0;
			size_t skippedRecord = 0;

			std::vector<UnsupportedFvfEntry> rejectedFvfs;

			// Called by the reader as it walks the file. Not part of the parsed
			// content -- only the parse itself records a refusal.
			void NoteUnsupportedFvf(uint32_t fvf)
			{
				++unsupportedFvf;
				for (UnsupportedFvfEntry& entry : rejectedFvfs)
				{
					if (entry.fvf == fvf) { ++entry.leaves; return; }
				}
				rejectedFvfs.push_back(UnsupportedFvfEntry{ fvf, 1u });
			}
		};

		// Bounded reader for the ASURA `.wld0` static-mesh container.
		//
		// Every nested block is read through its own bounded sub-cursor, so a
		// record can never read past the size it declared no matter what the
		// counts inside it claim. Allocations are bounded by the input size, so
		// a corrupt count cannot ask for memory the file cannot back.
		class WorldStaticMeshReader
		{
		public:
			// A single mesh may not exceed this many vertices or indices. The real
			// ASURA leaves are orders of magnitude below; the ceiling exists so a
			// corrupt count cannot force a huge allocation through a plausible
			// file size.
			static constexpr size_t kMaxLeafVertices = 4u * 1024u * 1024u;
			static constexpr size_t kMaxLeafIndices  = 8u * 1024u * 1024u;

			// The largest input this reader will touch. Chosen above the real
			// ASURA terrain files (the largest is ~182 MB) but low enough that a
			// nonsense size is refused before a real allocation.
			static constexpr size_t kMaxInputBytes = 512u * 1024u * 1024u;

			Result<WorldStaticMeshAsset> Decode(const std::vector<uint8_t>& bytes) const;
		};
	}
}
