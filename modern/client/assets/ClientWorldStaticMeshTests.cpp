// WORLD-PLAYABLE-001A: bounded `.wld0` terrain parser tests.
//
// The fixtures are assembled here, byte by byte, from the layout the legacy
// readers publish, so nothing proprietary is committed. Every size field a
// fixture writes is computed with the same rule `CSerialFile::EndBlock` uses --
// the byte count between the size DWORD and the end of the block -- because a
// fixture that inflates a block to cover the octree that follows it does not
// exercise the reader, it misleads it.
//
// The real-asset cases walk a client tree only when RAN_ASSET_ROOT names one,
// and never hardcode a machine-specific absolute path.
//
// Every malformed case asserts an *error*, not a crash, a hang, or an
// allocation driven by an untrusted count.

#include "TestHarness.h"

#include "WorldStaticMesh.h"
#include "WorldTerrainMesh.h"
#include "math/Vector3.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

using namespace Modern;
using namespace Modern::Legacy;

namespace
{
	// ---------------------------------------------------------------------------
	// Serialized constants, mirrored from WorldStaticMesh.cpp
	// ---------------------------------------------------------------------------
	//
	// D3D9 FVF bits. The texture count lives in bits 8-11
	// (`D3DFVF_TEXCOUNT_MASK` = 0x0F00, shift 8); bits 0-3 hold the position
	// form, and `D3DFVF_XYZ` (0x002) is one of them. So an FVF is only ever
	// matched as a whole value -- masking the low nibble as a coordinate count
	// turns 0x152 into "two texture sets" and misreads every vertex.
	constexpr uint32_t kFvfXyzNormalDiffuseTex1 = 0x152u;  // what real ASURA leaves declare
	constexpr uint32_t kFvfXyzNormalTex1        = 0x112u;  // the 32-byte OCTREEVERTEX
	constexpr uint32_t kFvfXyzNormal            = 0x012u;
	constexpr uint32_t kFvfXyzOnly              = 0x002u;
	constexpr uint32_t kFvfTwoTexSets           = 0x162u;  // refused: two coordinate sets
	constexpr uint32_t kFvfUnsupported          = 0x084u;  // refused

	constexpr size_t kStrideXyzNormalDiffuseTex1 = 36u;  // 12 + 12 + 4 + 8
	constexpr size_t kStrideXyzNormalTex1        = 32u;  // 12 + 12 + 8
	constexpr size_t kStrideXyzNormal            = 24u;  // 12 + 12
	constexpr size_t kStrideXyzOnly              = 12u;

	constexpr uint32_t kStaticMeshVer102  = 0x0102u;
	constexpr uint32_t kStaticMeshVerWld0 = 0x0200u;
	constexpr uint32_t kSingleTexVer101   = 0x0101u;
	constexpr uint32_t kSingleTexVer106   = 0x0106u;
	constexpr uint32_t kAabbOctreeVer100  = 0x0100u;
	constexpr uint32_t kObjOctreeVer      = 0x10001u;

	constexpr uint32_t kLandTypeDayNightOn  = 0u;
	constexpr uint32_t kLandTypeDayNightOff = 1u;

	// `DIRECTPOINTCOLOR` (NsOCTree.h:53) is eight WORDs, so the day/night array
	// is 16 bytes per vertex -- not the 4 a vertex D3DFVF_DIFFUSE colour takes.
	constexpr size_t kDayNightColourBytes = 16u;

	constexpr size_t kDwordBytes = 4u;
	constexpr size_t kSerialFileHeaderBytes = 132u;

	void PushU16(std::vector<uint8_t>& bytes, uint16_t value)
	{
		bytes.push_back(static_cast<uint8_t>(value & 0xFFu));
		bytes.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
	}

	void PushU32(std::vector<uint8_t>& bytes, uint32_t value)
	{
		bytes.push_back(static_cast<uint8_t>(value & 0xFFu));
		bytes.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
		bytes.push_back(static_cast<uint8_t>((value >> 16) & 0xFFu));
		bytes.push_back(static_cast<uint8_t>((value >> 24) & 0xFFu));
	}

	void PushF32(std::vector<uint8_t>& bytes, float value)
	{
		// A non-const local: writing the float's bits through a const_cast of a
		// `const` object is undefined, and the compiler is free to treat the
		// value as always 0.
		uint32_t raw = 0;
		std::memcpy(&raw, &value, sizeof(raw));
		PushU32(bytes, raw);
	}

	void PushVector3(std::vector<uint8_t>& bytes, float x, float y, float z)
	{
		PushF32(bytes, x);
		PushF32(bytes, y);
		PushF32(bytes, z);
	}

	void PushString(std::vector<uint8_t>& bytes, const std::string& value)
	{
		PushU32(bytes, static_cast<uint32_t>(value.size()));
		bytes.insert(bytes.end(), value.begin(), value.end());
	}

	// The 132-byte CSerialFile prefix: 128 bytes of file type plus a DWORD id.
	void PushSerialFileHeader(std::vector<uint8_t>& bytes)
	{
		const std::string mark = "default";
		bytes.insert(bytes.end(), mark.begin(), mark.end());
		bytes.resize(bytes.size() + (128 - mark.size()), 0u);
		PushU32(bytes, 0u);
	}

	// One vertex in D3DFVF_XYZ|NORMAL|DIFFUSE|TEX1, the layout every real ASURA
	// terrain leaf declares. 36 bytes: position, normal, BGRA colour, one UV.
	void PushVertex36(std::vector<uint8_t>& bytes,
	                  float px, float py, float pz,
	                  float nx, float ny, float nz,
	                  uint32_t colour,
	                  float u, float v)
	{
		PushF32(bytes, px); PushF32(bytes, py); PushF32(bytes, pz);
		PushF32(bytes, nx); PushF32(bytes, ny); PushF32(bytes, nz);
		PushU32(bytes, colour);
		PushF32(bytes, u);  PushF32(bytes, v);
	}

	// One vertex in D3DFVF_XYZ|NORMAL|TEX1: position, normal, one UV.
	void PushVertex32(std::vector<uint8_t>& bytes,
	                  float px, float py, float pz,
	                  float nx, float ny, float nz,
	                  float u, float v)
	{
		PushF32(bytes, px); PushF32(bytes, py); PushF32(bytes, pz);
		PushF32(bytes, nx); PushF32(bytes, ny); PushF32(bytes, nz);
		PushF32(bytes, u);  PushF32(bytes, v);
	}

	void PushIndex(std::vector<uint8_t>& bytes, uint16_t value)
	{
		PushU16(bytes, value);
	}

	// Overwrite a size field left at zero. The caller supplies the exact byte
	// count `CSerialFile::EndBlock` would have written, which is what makes the
	// fixture an honest fence rather than a hint.
	void PatchSize(std::vector<uint8_t>& bytes, size_t sizeFieldOffset, size_t sizeValue)
	{
		const uint32_t value = static_cast<uint32_t>(sizeValue);
		bytes[sizeFieldOffset + 0u] = static_cast<uint8_t>(value & 0xFFu);
		bytes[sizeFieldOffset + 1u] = static_cast<uint8_t>((value >> 8) & 0xFFu);
		bytes[sizeFieldOffset + 2u] = static_cast<uint8_t>((value >> 16) & 0xFFu);
		bytes[sizeFieldOffset + 3u] = static_cast<uint8_t>((value >> 24) & 0xFFu);
	}

	// The block size `EndBlock` would write for a block whose body starts at
	// `bodyOffset` and ends at the current end of `bytes`.
	size_t BlockSize(const std::vector<uint8_t>& bytes, size_t bodyOffset)
	{
		return bytes.size() - bodyOffset;
	}

	// One OBJOCTree geometry record: the 0x10001 layout `OBJOCTree::OctreeLoad`
	// reads.
	struct GeometryOptions
	{
		uint32_t fvf = kFvfXyzNormalDiffuseTex1;
		// Non-zero makes the geometry dispatch word this FVF instead of a version.
		uint32_t oldFvfWord = 0u;
		uint32_t vertices = 0u;
		uint32_t faces = 0u;
		uint32_t landType = kLandTypeDayNightOff;
		const std::vector<uint8_t>* vertexBytes = nullptr;
		const std::vector<uint16_t>* indices = nullptr;
		bool emitVertexData = true;
		bool emitIndexData = true;
		// Per-vertex DIRECTPOINTCOLOR count, present only for EMLR_DAYNIGHT_ON.
		size_t dayNightColourCount = 0u;
	};

	// The body of one OBJOCTree block: geometry, then the collision tree. The
	// caller closes the block.
	void PushObjOctreeBlock(std::vector<uint8_t>& bytes, const GeometryOptions& options)
	{
		PushU32(bytes, 1u);  // geometry exists
		// OBJOCTree::DynamicLoad: a value below 0x10000 here *is* the FVF.
		const bool old = options.oldFvfWord != 0u;
		PushU32(bytes, old ? options.oldFvfWord : kObjOctreeVer);
		if (!old) { PushU32(bytes, options.fvf); }
		PushU32(bytes, options.vertices);
		PushU32(bytes, options.faces);
		// OctreeLoadOLD carries no land type, and therefore no colour array.
		if (!old) { PushU32(bytes, options.landType); }
		if (options.emitVertexData && options.vertexBytes != nullptr)
		{
			bytes.insert(bytes.end(), options.vertexBytes->begin(), options.vertexBytes->end());
		}
		if (options.emitIndexData && options.indices != nullptr)
		{
			for (uint16_t index : *options.indices) { PushU16(bytes, index); }
		}
		for (size_t i = 0; i < options.dayNightColourCount; ++i)
		{
			// `DIRECTPOINTCOLOR` is eight WORDs: alpha, three day channels,
			// three night channels and a temperature.
			for (int w = 0; w < 8; ++w) { PushU16(bytes, 0x2040u); }
		}

		PushU32(bytes, 0u);  // collision tree absent
	}

	// One OBJOCTree *node*: exists flag, bounds, then the block. The two child
	// nodes follow *after* the block, because they are siblings in the
	// enclosing NSOCTREE stream, not part of this node's payload.
	struct NodeOptions
	{
		bool exists = true;
		GeometryOptions geometry;
		bool emitGeometry = true;
	};

	void PushObjOctreeNode(std::vector<uint8_t>& bytes, const NodeOptions& node)
	{
		PushU32(bytes, node.exists ? 1u : 0u);
		if (!node.exists) { return; }

		PushVector3(bytes, 4.0f, 3.0f, 8.0f);    // max
		PushVector3(bytes, -4.0f, 0.0f, -8.0f);  // min
		const size_t sizeFieldOffset = bytes.size();
		PushU32(bytes, 0u);  // block size, patched below
		// The block body starts after the size word, exactly as `EndBlock`
		// measures it.
		const size_t bodyOffset = bytes.size();
		if (node.emitGeometry) { PushObjOctreeBlock(bytes, node.geometry); }
		PatchSize(bytes, sizeFieldOffset, BlockSize(bytes, bodyOffset));
	}

	// One DxSingleTexMesh record.
	struct BucketOptions
	{
		std::string name;
		std::string texture;
		uint32_t singleTexVersion = kSingleTexVer101;
		// `DxSingleTexMesh::Load` reads the texture name FIRST for every version
		// except 0x0101, which reads one flag DWORD first. So the flags split either
		// side of the name, and the two counts are set explicitly per test.
		uint32_t flagsBeforeName = 1u;
		uint32_t flagsAfterName = 0u;
		uint32_t textEffCount = 0u;
		bool octree = true;
		// 0x0104..0x0106 read a light-map type word before the octree. Type 0 is
		// the octree itself; type 1 is the unimplemented light-mapped variant.
		bool emitLightMapType = false;
		uint32_t lightMapType = 0u;
		NodeOptions node;
		bool emitNode = true;
	};

	// One bucket entry: the name, the DxSingleTexMesh block, then the octree
	// *outside* that block.
	void PushBucketEntry(std::vector<uint8_t>& bytes, const BucketOptions& entry)
	{
		PushString(bytes, entry.name);

		const size_t recordSizeOffset = bytes.size() + kDwordBytes;
		PushU32(bytes, entry.singleTexVersion);
		PushU32(bytes, 0u);  // block size, patched below

		const size_t bodyOffset = bytes.size();
		for (uint32_t i = 0; i < entry.flagsBeforeName; ++i) { PushU32(bytes, 0u); }
		PushString(bytes, entry.texture);
		for (uint32_t i = 0; i < entry.flagsAfterName; ++i) { PushU32(bytes, 0u); }

		PushU32(bytes, entry.textEffCount);
		for (uint32_t i = 0; i < entry.textEffCount; ++i)
		{
			PushU32(bytes, 16u);    // type id
			PushU32(bytes, 0x0102u);  // record version
			PushU32(bytes, 8u);    // payload size
			PushU32(bytes, 0u);
			PushU32(bytes, 0u);
		}

		// `DxSingleTexMesh::Save` closes the block here. The octree that follows
		// is deliberately *not* counted -- see the note above the builder.
		PatchSize(bytes, recordSizeOffset, BlockSize(bytes, bodyOffset));

		PushU32(bytes, entry.octree ? 1u : 0u);
		if (!entry.octree) { return; }

		if (entry.emitLightMapType)
		{
			PushU32(bytes, entry.lightMapType);
			if (entry.lightMapType != 0u)
			{
				PushU32(bytes, 0x0100u);  // skipped block version
				PushU32(bytes, 8u);      // skipped block size
				PushU32(bytes, 0u);
				PushU32(bytes, 0u);
				return;
			}
		}

		if (!entry.emitNode) { return; }

		// `DxAABBOctree::Save`. This block *does* enclose the node tree.
		PushU32(bytes, kAabbOctreeVer100);
		const size_t octreeSizeOffset = bytes.size();
		PushU32(bytes, 0u);  // block size, patched below

		const size_t octreeBodyOffset = bytes.size();
		PushVector3(bytes, 10.0f, 5.0f, 20.0f);    // max
		PushVector3(bytes, -10.0f, 0.0f, -20.0f);  // min
		PushU32(bytes, 1u);  // the node tree exists

		PushObjOctreeNode(bytes, entry.node);
		PushObjOctreeNode(bytes, NodeOptions{ false, GeometryOptions{} });  // left child
		PushObjOctreeNode(bytes, NodeOptions{ false, GeometryOptions{} });  // right child

		PatchSize(bytes, octreeSizeOffset, BlockSize(bytes, octreeBodyOffset));
	}

	// A whole `.wld0`.
	std::vector<uint8_t> MakeWld0(const std::vector<std::vector<BucketOptions>>& buckets,
	                              uint32_t staticMeshVersion = kStaticMeshVer102)
	{
		std::vector<uint8_t> bytes;
		PushSerialFileHeader(bytes);

		PushU32(bytes, staticMeshVersion);

		// `DxStaticMesh::Save` closes the block immediately after the bounds, so
		// this size is exactly 24 and the buckets are outside it. Real ASURA files
		// declare 0x18 here.
		const size_t boundsSizeOffset = bytes.size();
		PushU32(bytes, 0u);
		const size_t boundsBodyOffset = bytes.size();
		PushVector3(bytes, 100.0f, 50.0f, 200.0f);    // tile max
		PushVector3(bytes, -100.0f, -1.0f, -200.0f);  // tile min
		PatchSize(bytes, boundsSizeOffset, BlockSize(bytes, boundsBodyOffset));

		// Five buckets, in the order `SingleTexMesh_Save` writes them.
		for (int b = 0; b < 5; ++b)
		{
			const std::vector<BucketOptions>* entries =
			    (b < static_cast<int>(buckets.size())) ? &buckets[b] : nullptr;
			PushU32(bytes, entries != nullptr ? static_cast<uint32_t>(entries->size()) : 0u);
			if (entries == nullptr) { continue; }
			for (const BucketOptions& entry : *entries) { PushBucketEntry(bytes, entry); }
		}
		return bytes;
	}

	Result<WorldStaticMeshAsset> Decode(const std::vector<uint8_t>& bytes)
	{
		return WorldStaticMeshReader().Decode(bytes);
	}


	// Three vertices in the layout real ASURA leaves declare, plus one triangle.
	std::vector<uint8_t> TriangleVertex36()
	{
		std::vector<uint8_t> data;
		PushVertex36(data, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0xFF0000FFu, 0.0f, 0.0f);
		PushVertex36(data, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0xFF00FF00u, 1.0f, 0.0f);
		PushVertex36(data, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0xFFFF0000u, 0.0f, 1.0f);
		return data;
	}

	std::vector<uint8_t> TriangleVertex32()
	{
		std::vector<uint8_t> data;
		PushVertex32(data, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
		PushVertex32(data, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
		PushVertex32(data, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
		return data;
	}

	std::vector<uint16_t> TriangleIndices()
	{
		return { 0u, 1u, 2u };
	}

	// Give `bucket` one decodable triangle in the layout real terrain declares.
	void FillTriangle(BucketOptions& bucket)
	{
		bucket.node.geometry.fvf      = kFvfXyzNormalDiffuseTex1;
		bucket.node.geometry.vertices = 3u;
		bucket.node.geometry.faces    = 1u;
	}

	// A minimal one-triangle leaf.
	BucketOptions TriangleBucket(const std::string& name, const std::string& texture)
	{
		BucketOptions bucket;
		bucket.name = name;
		bucket.texture = texture;
		FillTriangle(bucket);
		return bucket;
	}

	// A forward-only walker over fixture bytes. Used by the block-size test to
	// assert the fixture declares exactly what `CSerialFile::EndBlock` would
	// have written, offset by offset, without re-deriving it from the builder.
	struct Peek
	{
		const std::vector<uint8_t>& bytes;
		size_t offset = 0;

		uint32_t U32()
		{
			uint32_t value = 0;
			std::memcpy(&value, bytes.data() + offset, kDwordBytes);
			offset += kDwordBytes;
			return value;
		}
		// Reads a length-prefixed string, stepping over its bytes.
		std::string String()
		{
			const uint32_t length = U32();
			std::string value(reinterpret_cast<const char*>(bytes.data() + offset), length);
			offset += length;
			return value;
		}
		// Steps over a D3DXVECTOR3.
		size_t Vector3() { offset += 3u * kDwordBytes; return 3u * kDwordBytes; }
		// Reads a block size, which follows the version word.
		size_t Block() { return U32(); }
	};
} // namespace

// ---------------------------------------------------------------------------
// Valid records
// ---------------------------------------------------------------------------

MODERN_TEST(WorldStaticMesh_MinimalRecordDecodes)
{
	// The layout every real ASURA terrain leaf declares:
	// D3DFVF_XYZ|NORMAL|DIFFUSE|TEX1 (0x152, a 36-byte stride), in a 0x0102
	// container, with a 0x0101 single-texture record. Reading this is the
	// baseline the rest of the suite refines.
	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = TriangleIndices();

	BucketOptions bucket = TriangleBucket("m1_city.tile", "city.dds");
	bucket.node.geometry.vertexBytes = &vertices;
	bucket.node.geometry.indices = &indices;

	const std::vector<uint8_t> file = MakeWld0({ { bucket }, {}, {}, {}, {} });
	const Result<WorldStaticMeshAsset> decoded = Decode(file);
	CHECK(decoded.IsOk());
	if (decoded.IsError()) { return; }

	const WorldStaticMeshAsset& asset = decoded.GetValue();
	CHECK_EQ(asset.totalChunks(), size_t{1});
	CHECK_EQ(asset.totalVertices(), size_t{3});
	CHECK_EQ(asset.totalIndices(), size_t{3});
	CHECK_EQ(asset.unsupportedFvfCount(), size_t{0});
	CHECK_EQ(asset.buckets[0][0].name, std::string("m1_city.tile"));
	CHECK_EQ(asset.buckets[0][0].chunks[0].textureName, std::string("city.dds"));

	const WorldTerrainChunk& chunk = asset.buckets[0][0].chunks[0];
	CHECK(chunk.hasNormals);
	CHECK(chunk.hasUvs);
	CHECK(chunk.hasColors);
	CHECK_EQ(chunk.fvf, kFvfXyzNormalDiffuseTex1);
	CHECK_EQ(chunk.triangleCount(), uint32_t{1});
	CHECK_EQ(chunk.positions[0], 0.0f);
	CHECK_EQ(chunk.positions[3], 1.0f);  // vertex 1's x
	CHECK_EQ(chunk.positions[8], 1.0f);  // vertex 2's z
	CHECK_EQ(chunk.normals[4], 1.0f);
	CHECK_EQ(chunk.uvs[0], 0.0f);
	CHECK_EQ(chunk.uvs[5], 1.0f);
	// The colour really came from the vertex, not from a default fill.
	CHECK_EQ(chunk.colors[0], 0xFF0000FFu);
	CHECK_EQ(chunk.colors[2], 0xFFFF0000u);
	CHECK_EQ(chunk.indices[2], uint16_t{2});
	CHECK_EQ(chunk.boundsMax[0], 4.0f);
	CHECK_EQ(chunk.boundsMin[0], -4.0f);

	// Tile bounds are carried through unchanged.
	CHECK_EQ(asset.tileBoundsMax[0], 100.0f);
	CHECK_EQ(asset.tileBoundsMin[2], -200.0f);
}

MODERN_TEST(WorldStaticMesh_SupportedVertexLayoutOnly)
{
	// Every supported FVF must decode at its own stride, with exactly the
	// components that FVF declares, and no two of them may alias.
	std::vector<uint8_t> vertices32 = TriangleVertex32();
	std::vector<uint16_t> indices = TriangleIndices();

	// XYZ|NORMAL|TEX1, the 32-byte OCTREEVERTEX.
	BucketOptions textured = TriangleBucket("m1_a", "t.dds");
	textured.node.geometry.fvf = kFvfXyzNormalTex1;
	textured.node.geometry.vertexBytes = &vertices32;
	textured.node.geometry.indices = &indices;

	// XYZ|NORMAL, 24 bytes, no texture coordinates.
	std::vector<uint8_t> vertices24;
	PushF32(vertices24, 0.0f); PushF32(vertices24, 0.0f); PushF32(vertices24, 0.0f);
	PushF32(vertices24, 0.0f); PushF32(vertices24, 1.0f); PushF32(vertices24, 0.0f);
	PushF32(vertices24, 1.0f); PushF32(vertices24, 0.0f); PushF32(vertices24, 0.0f);
	PushF32(vertices24, 0.0f); PushF32(vertices24, 1.0f); PushF32(vertices24, 0.0f);
	PushF32(vertices24, 0.0f); PushF32(vertices24, 0.0f); PushF32(vertices24, 1.0f);
	PushF32(vertices24, 0.0f); PushF32(vertices24, 1.0f); PushF32(vertices24, 0.0f);

	BucketOptions normalOnly = TriangleBucket("m1_b", "n.dds");
	normalOnly.node.geometry.fvf = kFvfXyzNormal;
	normalOnly.node.geometry.vertexBytes = &vertices24;
	normalOnly.node.geometry.indices = &indices;

	// XYZ only, 12 bytes, nothing else.
	std::vector<uint8_t> vertices12;
	PushF32(vertices12, 0.0f); PushF32(vertices12, 0.0f); PushF32(vertices12, 0.0f);
	PushF32(vertices12, 1.0f); PushF32(vertices12, 0.0f); PushF32(vertices12, 0.0f);
	PushF32(vertices12, 0.0f); PushF32(vertices12, 0.0f); PushF32(vertices12, 1.0f);

	BucketOptions positionsOnly = TriangleBucket("m1_c", "u.dds");
	positionsOnly.node.geometry.fvf = kFvfXyzOnly;
	positionsOnly.node.geometry.vertexBytes = &vertices12;
	positionsOnly.node.geometry.indices = &indices;

	const Result<WorldStaticMeshAsset> decoded =
	    Decode(MakeWld0({ { textured, normalOnly, positionsOnly }, {}, {}, {}, {} }));
	CHECK(decoded.IsOk());
	if (decoded.IsError()) { return; }

	const WorldStaticMeshAsset& asset = decoded.GetValue();
	CHECK_EQ(asset.unsupportedFvfCount(), size_t{0});
	CHECK_EQ(asset.totalVertices(), size_t{9});

	// 32-byte layout: normals and one UV pair, no diffuse colour.
	const WorldTerrainChunk& a = asset.buckets[0][0].chunks[0];
	CHECK_EQ(a.fvf, kFvfXyzNormalTex1);
	CHECK(a.hasNormals);
	CHECK(a.hasUvs);
	CHECK(!a.hasColors);
	CHECK_EQ(a.positions.size(), size_t{9});
	CHECK_EQ(a.normals.size(), size_t{9});
	CHECK_EQ(a.uvs.size(), size_t{6});
	CHECK_EQ(a.colors.size(), size_t{0});
	CHECK_EQ(a.uvs[0], 0.0f);
	CHECK_EQ(a.uvs[5], 1.0f);

	// 24-byte layout: normals, no UVs, no colour. If this stride were read as
	// 32 the normals and indices would both be garbage.
	const WorldTerrainChunk& b = asset.buckets[0][1].chunks[0];
	CHECK_EQ(b.fvf, kFvfXyzNormal);
	CHECK(b.hasNormals);
	CHECK(!b.hasUvs);
	CHECK(!b.hasColors);
	CHECK_EQ(b.normals.size(), size_t{9});
	CHECK_EQ(b.uvs.size(), size_t{0});
	CHECK_EQ(b.normals[4], 1.0f);
	CHECK_EQ(b.positions[3], 1.0f);  // vertex 1's x
	CHECK_EQ(b.positions[8], 1.0f);  // vertex 2's z

	// 12-byte layout: positions and nothing else.
	const WorldTerrainChunk& c = asset.buckets[0][2].chunks[0];
	CHECK_EQ(c.fvf, kFvfXyzOnly);
	CHECK(!c.hasNormals);
	CHECK(!c.hasUvs);
	CHECK(!c.hasColors);
	CHECK_EQ(c.positions.size(), size_t{9});
	CHECK_EQ(c.normals.size(), size_t{0});
	CHECK_EQ(c.uvs.size(), size_t{0});
	CHECK_EQ(c.colors.size(), size_t{0});
	CHECK_EQ(c.positions[3], 1.0f);  // vertex 1's x
	CHECK_EQ(c.indices[2], uint16_t{2});
}

MODERN_TEST(WorldStaticMesh_InvalidFvfIsNotInterpretedAsTheSupportedLayout)
{
	// An FVF this reader does not model must be counted and refused, never
	// decoded as though it were one of the supported layouts. 0x162 differs
	// from the supported 0x152 only in the texture-count field, so a reader that
	// decomposed the FVF by masking would silently decode it at the wrong
	// stride.
	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = TriangleIndices();

	BucketOptions bucket = TriangleBucket("m1_a", "t.dds");
	bucket.node.geometry.fvf = kFvfTwoTexSets;
	bucket.node.geometry.vertexBytes = &vertices;
	bucket.node.geometry.indices = &indices;

	const Result<WorldStaticMeshAsset> decoded = Decode(MakeWld0({ { bucket }, {}, {}, {}, {} }));
	CHECK(decoded.IsOk());
	if (decoded.IsError()) { return; }

	// The record is recognised but its geometry is not decoded. The mesh keeps
	// its name and loses its chunks: no geometry is invented for it, and the
	// refusal is counted rather than swallowed.
	CHECK_EQ(decoded.GetValue().unsupportedFvfCount(), size_t{1});
	CHECK_EQ(decoded.GetValue().totalVertices(), size_t{0});
	CHECK_EQ(decoded.GetValue().totalChunks(), size_t{0});
	CHECK_EQ(decoded.GetValue().buckets[0].size(), size_t{1});
	CHECK_EQ(decoded.GetValue().buckets[0][0].chunks.size(), size_t{0});
	// And the refusal names the value it refused.
	CHECK_EQ(decoded.GetValue().unsupportedFvfs().size(), size_t{1});
	CHECK_EQ(decoded.GetValue().unsupportedFvfs()[0].fvf, kFvfTwoTexSets);
	CHECK_EQ(decoded.GetValue().unsupportedFvfs()[0].leaves, size_t{1});

	// The same must hold for a value that is not a position form at all.
	BucketOptions rhw = TriangleBucket("m1_b", "r.dds");
	rhw.node.geometry.fvf = kFvfUnsupported;
	rhw.node.geometry.vertexBytes = &vertices;
	rhw.node.geometry.indices = &indices;

	const Result<WorldStaticMeshAsset> rhwDecoded =
	    Decode(MakeWld0({ { rhw }, {}, {}, {}, {} }));
	CHECK(rhwDecoded.IsOk());
	if (rhwDecoded.IsError()) { return; }
	CHECK_EQ(rhwDecoded.GetValue().unsupportedFvfCount(), size_t{1});
	CHECK_EQ(rhwDecoded.GetValue().totalVertices(), size_t{0});
}

MODERN_TEST(WorldStaticMesh_ValidRecordWithMultipleGeometryChunks)
{
	// Two meshes in one bucket proves the reader keeps both, in the order the
	// file lists them, and that each carries its own texture.
	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = TriangleIndices();

	BucketOptions first = TriangleBucket("chunk-a", "a.dds");
	first.node.geometry.vertexBytes = &vertices;
	first.node.geometry.indices = &indices;

	BucketOptions second = TriangleBucket("chunk-b", "b.dds");
	second.node.geometry.vertexBytes = &vertices;
	second.node.geometry.indices = &indices;

	const Result<WorldStaticMeshAsset> decoded =
	    Decode(MakeWld0({ { first, second }, {}, {}, {}, {} }));
	CHECK(decoded.IsOk());
	if (decoded.IsError()) { return; }

	const WorldStaticMeshAsset& asset = decoded.GetValue();
	CHECK_EQ(asset.buckets[0].size(), size_t{2});
	CHECK_EQ(asset.totalMeshes(), size_t{2});
	CHECK_EQ(asset.totalChunks(), size_t{2});
	CHECK_EQ(asset.totalVertices(), size_t{6});
	CHECK_EQ(asset.totalIndices(), size_t{6});

	// File order is preserved, and the two textures are not conflated.
	CHECK_EQ(asset.buckets[0][0].name, std::string("chunk-a"));
	CHECK_EQ(asset.buckets[0][1].name, std::string("chunk-b"));
	CHECK_EQ(asset.buckets[0][0].chunks[0].textureName, std::string("a.dds"));
	CHECK_EQ(asset.buckets[0][1].chunks[0].textureName, std::string("b.dds"));
	CHECK(asset.buckets[0][0].chunks[0].textureName !=
	      asset.buckets[0][1].chunks[0].textureName);
}

MODERN_TEST(WorldStaticMesh_EmptyTopLevelBufferIsRefused)
{
	// A file that is only the header cannot hold a bucket.
	std::vector<uint8_t> bytes;
	PushSerialFileHeader(bytes);
	CHECK(Decode(bytes).IsError());
}

// ---------------------------------------------------------------------------
// The block-size boundary, established from CSerialFile::EndBlock
// ---------------------------------------------------------------------------

MODERN_TEST(WorldStaticMesh_DeclaresTheBlockSizesLegacyWouldWrite)
{
	// One test for all four nested size fields, because they share one rule: a
	// block size is the byte count between the size DWORD and the matching
	// `CSerialFile::EndBlock`, so it covers the body and nothing else. This is
	// the check that catches a fixture padding a block over the octree that
	// follows it -- the mistake that made every conforming record look
	// over-long.
	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = TriangleIndices();

	BucketOptions bucket = TriangleBucket("chunk-a", "city.dds");
	bucket.node.geometry.vertexBytes = &vertices;
	bucket.node.geometry.indices = &indices;

	const std::vector<uint8_t> file = MakeWld0({ { bucket }, {}, {}, {}, {} });
	Peek walk{ file, kSerialFileHeaderBytes };

	// The container block is exactly vMax + vMin, and holds nothing else.
	CHECK_EQ(walk.U32(), kStaticMeshVer102);
	CHECK_EQ(walk.Block(), size_t{ 2u * 3u * kDwordBytes });
	CHECK_EQ(walk.Vector3(), size_t{ 3u * kDwordBytes });
	CHECK_EQ(walk.Vector3(), size_t{ 3u * kDwordBytes });

	// Bucket 0 holds one mesh. Buckets 1-4 are empty and come after it.
	CHECK_EQ(walk.U32(), uint32_t{1});
	CHECK_EQ(walk.String(), std::string("chunk-a"));

	// The single-texture block holds one flag, the texture name and the effect
	// count. Crucially it does NOT hold the octree that follows it.
	(void)walk.U32();                        // version
	const size_t singleTexSizeOffset = walk.offset;
	const size_t singleTexSize = walk.Block();
	CHECK_EQ(singleTexSize, kDwordBytes /* flag */ + kDwordBytes + 8u /* "city.dds" */
	                                 + kDwordBytes /* effect count */);
	(void)walk.U32();                        // flag
	CHECK_EQ(walk.String(), std::string("city.dds"));
	CHECK_EQ(walk.U32(), uint32_t{0});        // effect count
	// The block ends exactly where the octree exists flag begins.
	CHECK_EQ(walk.offset, singleTexSizeOffset + kDwordBytes + singleTexSize);

	CHECK_EQ(walk.U32(), uint32_t{1});        // octree exists, outside the block

	// The AABB octree block does enclose the node tree -- the one block that
	// contains its children.
	const size_t octreeBody = walk.offset + kDwordBytes;
	CHECK_EQ(walk.U32(), kAabbOctreeVer100);
	const size_t octreeSize = walk.Block();
	(void)walk.Vector3();
	(void)walk.Vector3();
	CHECK_EQ(walk.U32(), uint32_t{1});        // node tree exists

	// The OBJOCTree block holds the geometry and the collision flag, and stops
	// before the two absent children.
	CHECK_EQ(walk.U32(), uint32_t{1});        // this node exists
	(void)walk.Vector3();
	(void)walk.Vector3();
	const size_t nodeSizeOffset = walk.offset;
	const size_t nodeSize = walk.Block();
	CHECK_EQ(walk.U32(), uint32_t{1});        // geometry exists
	CHECK_EQ(walk.U32(), kObjOctreeVer);
	CHECK_EQ(walk.U32(), kFvfXyzNormalDiffuseTex1);
	CHECK_EQ(walk.U32(), uint32_t{3});        // vertices
	CHECK_EQ(walk.U32(), uint32_t{1});        // faces
	CHECK_EQ(walk.U32(), kLandTypeDayNightOff);
	walk.offset += 3u * kStrideXyzNormalDiffuseTex1;
	walk.offset += 3u * 2u;                   // three WORD indices
	CHECK_EQ(walk.U32(), uint32_t{0});        // collision tree absent
	CHECK_EQ(walk.offset, nodeSizeOffset + kDwordBytes + nodeSize);

	// The two absent children are siblings *after* that block.
	CHECK_EQ(walk.U32(), uint32_t{0});
	CHECK_EQ(walk.U32(), uint32_t{0});

	// And the AABB block closes exactly here, at the end of this mesh.
	CHECK_EQ(walk.offset, octreeBody + kDwordBytes + octreeSize);

	// The remaining four empty bucket counts close the file.
	for (int b = 1; b < 5; ++b) { CHECK_EQ(walk.U32(), uint32_t{0}); }
	CHECK_EQ(walk.offset, file.size());
}

MODERN_TEST(WorldStaticMesh_DayNightOnLeafCarriesAColourArray)
{
	// `OBJOCTree::OctreeLoad` reads an extra per-vertex DIRECTPOINTCOLOR array
	// after the indices, but only for EMLR_DAYNIGHT_ON -- which is 0, not 1.
	// That array is eight WORDs (16 bytes) per vertex, not the 4 a vertex's own
	// diffuse colour occupies. Getting either detail wrong desynchronises every
	// record after it.
	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = TriangleIndices();

	BucketOptions on = TriangleBucket("chunk-on", "on.dds");
	on.node.geometry.landType = kLandTypeDayNightOn;
	on.node.geometry.vertexBytes = &vertices;
	on.node.geometry.indices = &indices;
	on.node.geometry.dayNightColourCount = 3u;

	// Followed by a second mesh: if the colour array is not stepped over exactly,
	// this one cannot be found.
	BucketOptions off = TriangleBucket("chunk-off", "off.dds");
	off.node.geometry.vertexBytes = &vertices;
	off.node.geometry.indices = &indices;

	const Result<WorldStaticMeshAsset> decoded =
	    Decode(MakeWld0({ { on, off }, {}, {}, {}, {} }));
	CHECK(decoded.IsOk());
	if (decoded.IsError()) { return; }

	const WorldStaticMeshAsset& asset = decoded.GetValue();
	CHECK_EQ(asset.buckets[0].size(), size_t{2});
	CHECK_EQ(asset.buckets[0][0].name, std::string("chunk-on"));
	CHECK_EQ(asset.buckets[0][1].name, std::string("chunk-off"));
	CHECK_EQ(asset.buckets[0][1].chunks[0].textureName, std::string("off.dds"));
	CHECK_EQ(asset.totalVertices(), size_t{6});
	// The vertex's own diffuse colour is untouched by the day/night array.
	CHECK_EQ(asset.buckets[0][0].chunks[0].colors[0], 0xFF0000FFu);
}

MODERN_TEST(WorldStaticMesh_TextEffectRecordsAreBoundedByTheirOwnSizes)
{
	// A text-effect record is `typeId, version, size, payload[size]`. The
	// payload is never interpreted, but it must be stepped over exactly, or the
	// octree that follows the block cannot be found. Two of them, then a real
	// triangle, proves the skip lands exactly.
	BucketOptions bucket = TriangleBucket("chunk-a", "city.dds");
	bucket.textEffCount = 2u;

	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = TriangleIndices();
	bucket.node.geometry.vertexBytes = &vertices;
	bucket.node.geometry.indices = &indices;

	const Result<WorldStaticMeshAsset> decoded =
	    Decode(MakeWld0({ { bucket }, {}, {}, {}, {} }));
	CHECK(decoded.IsOk());
	if (decoded.IsError()) { return; }
	CHECK_EQ(decoded.GetValue().totalVertices(), size_t{3});
	CHECK_EQ(decoded.GetValue().skippedRecordCount(), size_t{2});
}

MODERN_TEST(WorldStaticMesh_SingleTexMeshExtraFlagWordsAreConsumed)
{
	// 0x0106 reads the texture name first and then two flag DWORDs; 0x0101 reads
	// one flag DWORD and then the name. Reading a fixed shape for both lands
	// mid-name on one of them. 0x0106 also reads a light-map type word after the
	// octree exists flag.
	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = TriangleIndices();

	BucketOptions modern = TriangleBucket("chunk-a", "city.dds");
	modern.singleTexVersion = kSingleTexVer106;
	modern.flagsBeforeName = 0u;
	modern.flagsAfterName = 2u;
	modern.emitLightMapType = true;  // read the word, then the octree
	modern.lightMapType = 0u;
	modern.node.geometry.vertexBytes = &vertices;
	modern.node.geometry.indices = &indices;

	const Result<WorldStaticMeshAsset> decoded =
	    Decode(MakeWld0({ { modern }, {}, {}, {}, {} }, kStaticMeshVerWld0));
	CHECK(decoded.IsOk());
	if (decoded.IsError()) { return; }
	CHECK_EQ(decoded.GetValue().buckets[0].size(), size_t{1});
	CHECK_EQ(decoded.GetValue().buckets[0][0].chunks[0].textureName, std::string("city.dds"));
	CHECK_EQ(decoded.GetValue().totalVertices(), size_t{3});
}

MODERN_TEST(WorldStaticMesh_UnimplementedLightMapOctreeIsSkippedNotGuessed)
{
	// Light-map type 1 is the "new DxAABBOctree with light maps" the client has
	// never implemented. It is a self-describing block, so it is skipped and
	// counted rather than decoded -- and it must not be mistaken for a type 0
	// octree.
	BucketOptions bucket = TriangleBucket("chunk-lm", "lm.dds");
	bucket.singleTexVersion = kSingleTexVer106;
	bucket.flagsBeforeName = 0u;
	bucket.flagsAfterName = 2u;
	bucket.emitLightMapType = true;
	bucket.lightMapType = 1u;

	// A real, decodable triangle after it: recovering it is the proof that the
	// skipped block left the stream aligned.
	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = TriangleIndices();
	BucketOptions after = TriangleBucket("chunk-after", "after.dds");
	after.node.geometry.vertexBytes = &vertices;
	after.node.geometry.indices = &indices;

	const Result<WorldStaticMeshAsset> decoded =
	    Decode(MakeWld0({ { bucket, after }, {}, {}, {}, {} }));
	CHECK(decoded.IsOk());
	if (decoded.IsError()) { return; }

	// The light-mapped mesh keeps its name and loses its chunks, and the mesh
	// after it is still found.
	const WorldStaticMeshAsset& asset = decoded.GetValue();
	CHECK_EQ(asset.buckets[0].size(), size_t{2});
	CHECK_EQ(asset.buckets[0][0].name, std::string("chunk-lm"));
	CHECK_EQ(asset.buckets[0][0].chunks.size(), size_t{0});
	CHECK_EQ(asset.buckets[0][1].name, std::string("chunk-after"));
	CHECK(asset.skippedRecordCount() >= size_t{1});
	CHECK_EQ(asset.totalChunks(), size_t{1});
	CHECK_EQ(asset.totalVertices(), size_t{3});
	CHECK_EQ(asset.buckets[0][1].chunks[0].textureName, std::string("after.dds"));
}

// ---------------------------------------------------------------------------
// Malformed input: truncation
// ---------------------------------------------------------------------------

MODERN_TEST(WorldStaticMesh_TruncatedTopLevelHeaderIsRefused)
{
	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = TriangleIndices();

	BucketOptions bucket = TriangleBucket("chunk-a", "t.dds");
	bucket.node.geometry.vertexBytes = &vertices;
	bucket.node.geometry.indices = &indices;

	const std::vector<uint8_t> file = MakeWld0({ { bucket }, {}, {}, {}, {} });

	// Cut at every length below the five bucket counts plus the tile bounds.
	for (size_t cut = 0; cut < 172u; ++cut)
	{
		const std::vector<uint8_t> truncated(file.begin(),
		                                     file.begin() + static_cast<std::ptrdiff_t>(cut));
		CHECK(Decode(truncated).IsError());
	}
}

MODERN_TEST(WorldStaticMesh_TruncatedNestedBufferIsRefused)
{
	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = TriangleIndices();

	BucketOptions bucket = TriangleBucket("chunk-a", "t.dds");
	bucket.node.geometry.vertexBytes = &vertices;
	bucket.node.geometry.indices = &indices;

	const std::vector<uint8_t> file = MakeWld0({ { bucket }, {}, {}, {}, {} });
	// Drop bytes from the end so the geometry record runs past the buffer.
	for (size_t drop = 1u; drop < 40u; ++drop)
	{
		const std::vector<uint8_t> truncated(file.begin(),
		                                     file.end() - static_cast<std::ptrdiff_t>(drop));
		CHECK(Decode(truncated).IsError());
	}
}

MODERN_TEST(WorldStaticMesh_TruncatedVertexBufferIsRefused)
{
	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = TriangleIndices();

	BucketOptions bucket = TriangleBucket("chunk-a", "t.dds");
	bucket.node.geometry.vertexBytes = &vertices;
	bucket.node.geometry.indices = &indices;
	bucket.node.geometry.emitVertexData = false;  // declare vertices, supply none

	CHECK(Decode(MakeWld0({ { bucket }, {}, {}, {}, {} })).IsError());
}

MODERN_TEST(WorldStaticMesh_TruncatedIndexBufferIsRefused)
{
	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = TriangleIndices();

	BucketOptions bucket = TriangleBucket("chunk-a", "t.dds");
	bucket.node.geometry.vertexBytes = &vertices;
	bucket.node.geometry.indices = &indices;
	bucket.node.geometry.emitIndexData = false;

	CHECK(Decode(MakeWld0({ { bucket }, {}, {}, {}, {} })).IsError());
}

// ---------------------------------------------------------------------------
// Malformed input: versions
// ---------------------------------------------------------------------------

MODERN_TEST(WorldStaticMesh_UnsupportedTopLevelVersionIsRefused)
{
	// 0x0104 and 0x0105 read their buckets through SingleTexMesh_Load_Insert_104,
	// which puts an extra flag word in front of every mesh. There is no top-level
	// block that fences the buckets, so an unrecognised top-level version has to
	// be refused rather than guessed at.
	for (uint32_t version : { 0x0104u, 0x0105u, 0x0107u, 0x0000u })
	{
		std::vector<uint8_t> bytes;
		PushSerialFileHeader(bytes);
		PushU32(bytes, version);
		PushU32(bytes, 24u);
		PushVector3(bytes, 1.0f, 1.0f, 1.0f);
		PushVector3(bytes, 0.0f, 0.0f, 0.0f);
		CHECK(Decode(bytes).IsError());
	}
}

MODERN_TEST(WorldStaticMesh_UnsupportedOctreeVersionIsRefused)
{
	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = TriangleIndices();

	BucketOptions bucket = TriangleBucket("chunk-a", "t.dds");
	bucket.node.geometry.vertexBytes = &vertices;
	bucket.node.geometry.indices = &indices;

	std::vector<uint8_t> file = MakeWld0({ { bucket }, {}, {}, {}, {} });

	// `DxAABBOctree::Load` reads the version before the size, so rewriting the
	// version word leaves every block length still consistent.
	bool patched = false;
	for (size_t i = 0; i + kDwordBytes <= file.size() && !patched; ++i)
	{
		if (file[i + 0u] == 0x00u && file[i + 1u] == 0x01u &&
		    file[i + 2u] == 0x00u && file[i + 3u] == 0x00u)
		{
			file[i + 0u] = 0x55u;
			file[i + 1u] = 0x01u;
			file[i + 2u] = 0x00u;
			file[i + 3u] = 0x00u;
			patched = true;
		}
	}
	CHECK(patched);
	CHECK(Decode(file).IsError());
}

MODERN_TEST(WorldStaticMesh_UnsupportedLeafGeometryVersionIsRefused)
{
	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = TriangleIndices();

	BucketOptions bucket = TriangleBucket("chunk-a", "t.dds");
	bucket.node.geometry.vertexBytes = &vertices;
	bucket.node.geometry.indices = &indices;

	std::vector<uint8_t> file = MakeWld0({ { bucket }, {}, {}, {}, {} });

	// `OBJOCTree::DynamicLoad` dispatches on this word: 0x10000 and 0x10001 are
	// the two layouts modelled here, and anything *below* 0x10000 is the leaf's
	// FVF rather than a version at all. A value above 0x10001 is a version the
	// client has never seen, which leaves the record's length unknowable, so it
	// has to be refused.
	const uint8_t pattern[4] = { 0x01u, 0x00u, 0x01u, 0x00u };
	bool patched = false;
	for (size_t i = 0; i + kDwordBytes <= file.size() && !patched; ++i)
	{
		if (std::memcmp(file.data() + i, pattern, 4u) == 0)
		{
			// 0x00010002 -- one past the newest version the client knows.
			file[i + 0u] = 0x02u;
			file[i + 1u] = 0x01u;
			file[i + 2u] = 0x00u;
			file[i + 3u] = 0x00u;
			patched = true;
		}
	}
	CHECK(patched);
	CHECK(Decode(file).IsError());
}

MODERN_TEST(WorldStaticMesh_FixedFvfLeafNeedsNoVersionWord)
{
	// `OBJOCTree::DynamicLoad` sends anything below 0x10000 straight to
	// `OctreeLoadOLD` with the word as the FVF, because those octrees have one
	// fixed vertex format. Plenty of real `.wld0` leaves are shaped this way, and
	// the OLD payload has no land type and therefore no DIRECTPOINTCOLOR array
	// after the indices.
	std::vector<uint8_t> vertices = TriangleVertex32();
	std::vector<uint16_t> indices = TriangleIndices();

	BucketOptions bucket = TriangleBucket("chunk-old", "old.dds");
	// The dispatch word is the FVF, not a version.
	bucket.node.geometry.oldFvfWord = kFvfXyzNormalTex1;
	bucket.node.geometry.fvf = kFvfXyzNormalTex1;
	bucket.node.geometry.vertexBytes = &vertices;
	bucket.node.geometry.indices = &indices;

	// Followed by a second mesh: if the OLD payload were read as though it
	// carried a land type, this one could not be found.
	BucketOptions after = TriangleBucket("chunk-after", "after.dds");
	after.node.geometry.fvf = kFvfXyzNormalTex1;
	after.node.geometry.vertexBytes = &vertices;
	after.node.geometry.indices = &indices;

	const Result<WorldStaticMeshAsset> decoded =
	    Decode(MakeWld0({ { bucket, after }, {}, {}, {}, {} }));
	CHECK(decoded.IsOk());
	if (decoded.IsError()) { return; }

	const WorldStaticMeshAsset& asset = decoded.GetValue();
	CHECK_EQ(asset.buckets[0].size(), size_t{2});
	CHECK_EQ(asset.totalChunks(), size_t{2});
	CHECK_EQ(asset.totalVertices(), size_t{6});
	CHECK_EQ(asset.buckets[0][0].chunks[0].fvf, kFvfXyzNormalTex1);
	CHECK_EQ(asset.buckets[0][1].name, std::string("chunk-after"));
	CHECK_EQ(asset.unsupportedFvfCount(), size_t{0});
}

// ---------------------------------------------------------------------------
// Malformed input: buffer sizes versus their parent buffer
// ---------------------------------------------------------------------------

MODERN_TEST(WorldStaticMesh_DeclaredBufferSizeLargerThanParentIsRefused)
{
	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = TriangleIndices();

	BucketOptions bucket = TriangleBucket("chunk-a", "t.dds");
	bucket.node.geometry.vertexBytes = &vertices;
	bucket.node.geometry.indices = &indices;

	const std::vector<uint8_t> file = MakeWld0({ { bucket }, {}, {}, {}, {} });

	// Inflate the single-texture record's block size. The record then claims far
	// more than the file can hold, which any bounded reader must refuse. The size
	// word is the one after the 0x0101 version.
	std::vector<uint8_t> inflated = file;
	bool patched = false;
	for (size_t i = kSerialFileHeaderBytes; i + kDwordBytes * 2u <= inflated.size(); ++i)
	{
		if (inflated[i + 0u] == 0x01u && inflated[i + 1u] == 0x01u &&
		    inflated[i + 2u] == 0x00u && inflated[i + 3u] == 0x00u)
		{
			const uint32_t huge = 0xFFFFFFFu;
			inflated[i + 4u] = static_cast<uint8_t>(huge & 0xFFu);
			inflated[i + 5u] = static_cast<uint8_t>((huge >> 8) & 0xFFu);
			inflated[i + 6u] = static_cast<uint8_t>((huge >> 16) & 0xFFu);
			inflated[i + 7u] = static_cast<uint8_t>((huge >> 24) & 0xFFu);
			patched = true;
			break;
		}
	}
	CHECK(patched);
	CHECK(Decode(inflated).IsError());
}

MODERN_TEST(WorldStaticMesh_OversizedBoundsBlockIsRefused)
{
	// The container's block holds the tile bounds and nothing else, so a file
	// that declares more inside it is not the layout this reader implements.
	std::vector<uint8_t> bytes;
	PushSerialFileHeader(bytes);
	PushU32(bytes, kStaticMeshVer102);
	PushU32(bytes, 32u);  // claims two vectors' worth
	PushVector3(bytes, 1.0f, 1.0f, 1.0f);
	PushVector3(bytes, 0.0f, 0.0f, 0.0f);
	PushU32(bytes, 0u); PushU32(bytes, 0u);
	PushU32(bytes, 0u); PushU32(bytes, 0u);
	PushU32(bytes, 0u);
	CHECK(Decode(bytes).IsError());
}

// ---------------------------------------------------------------------------
// Malformed input: counts
// ---------------------------------------------------------------------------

MODERN_TEST(WorldStaticMesh_VertexCountOverflowIsRefused)
{
	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = TriangleIndices();

	BucketOptions bucket = TriangleBucket("chunk-a", "t.dds");
	bucket.node.geometry.vertices = 0x7FFFFFFFu;  // absurd
	bucket.node.geometry.vertexBytes = &vertices;
	bucket.node.geometry.indices = &indices;

	CHECK(Decode(MakeWld0({ { bucket }, {}, {}, {}, {} })).IsError());
}

MODERN_TEST(WorldStaticMesh_FaceCountOverflowIsRefused)
{
	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = TriangleIndices();

	BucketOptions bucket = TriangleBucket("chunk-a", "t.dds");
	bucket.node.geometry.faces = 0x7FFFFFFFu;  // absurd
	bucket.node.geometry.vertexBytes = &vertices;
	bucket.node.geometry.indices = &indices;

	CHECK(Decode(MakeWld0({ { bucket }, {}, {}, {}, {} })).IsError());
}

MODERN_TEST(WorldStaticMesh_IndexReferencingNonexistentVertexIsRefused)
{
	std::vector<uint8_t> vertices = TriangleVertex36();
	std::vector<uint16_t> indices = { 0u, 1u, 99u };  // 99 is out of range

	BucketOptions bucket = TriangleBucket("chunk-a", "t.dds");
	bucket.node.geometry.vertexBytes = &vertices;
	bucket.node.geometry.indices = &indices;

	CHECK(Decode(MakeWld0({ { bucket }, {}, {}, {}, {} })).IsError());
}

MODERN_TEST(WorldStaticMesh_NonFiniteVertexCoordinatesAreRefused)
{
	// Built by memcpy, because `0.0f / 0.0f` is a compile-time divide by zero.
	const uint32_t nanBits = 0x7FC00000u;
	float nan = 0.0f;
	std::memcpy(&nan, &nanBits, sizeof(nan));

	std::vector<uint8_t> vertices;
	PushVertex36(vertices, nan, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0xFF0000FFu, 0.0f, 0.0f);
	PushVertex36(vertices, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0xFF00FF00u, 1.0f, 0.0f);
	PushVertex36(vertices, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0xFFFF0000u, 0.0f, 1.0f);
	std::vector<uint16_t> indices = TriangleIndices();

	BucketOptions bucket = TriangleBucket("chunk-a", "t.dds");
	bucket.node.geometry.vertexBytes = &vertices;
	bucket.node.geometry.indices = &indices;

	CHECK(Decode(MakeWld0({ { bucket }, {}, {}, {}, {} })).IsError());
}

MODERN_TEST(WorldStaticMesh_NonFiniteTileBoundsAreRefused)
{
	// Built by memcpy, because `0.0f / 0.0f` is a compile-time divide by zero.
	const uint32_t nanBits = 0x7FC00000u;
	float nan = 0.0f;
	std::memcpy(&nan, &nanBits, sizeof(nan));

	std::vector<uint8_t> bytes;
	PushSerialFileHeader(bytes);
	PushU32(bytes, kStaticMeshVer102);
	PushU32(bytes, 24u);
	PushF32(bytes, nan); PushF32(bytes, 0.0f); PushF32(bytes, 0.0f);  // a NaN tile bound
	PushVector3(bytes, 0.0f, 0.0f, 0.0f);
	PushU32(bytes, 0u); PushU32(bytes, 0u); PushU32(bytes, 0u);
	PushU32(bytes, 0u); PushU32(bytes, 0u);

	CHECK(Decode(bytes).IsError());
}

// ---------------------------------------------------------------------------
// Real ASURA asset validation
// ---------------------------------------------------------------------------

namespace
{
	// The client tree the operator nominated. Kept in one place so the path is
	// never hardcoded into a hermetic test: with no RAN_ASSET_ROOT the real-asset
	// case reports that it was skipped and passes vacuously.
	bool FindMapRoot(std::filesystem::path& mapRoot)
	{
	#if defined(_MSC_VER)
	#pragma warning(push)
	#pragma warning(disable : 4996)
	#endif
		const char* rootEnv = std::getenv("RAN_ASSET_ROOT");
	#if defined(_MSC_VER)
	#pragma warning(pop)
	#endif
		if (rootEnv == nullptr || rootEnv[0] == '\0') { return false; }

		mapRoot = std::filesystem::path(rootEnv) / "data" / "map";
		return std::filesystem::is_directory(mapRoot);
	}

	// How large a `.wld0` this run will parse. The harness gives every test a
	// fixed wall-clock budget, and the largest real terrain files are ~182 MB --
	// far more than fits in one test case. The ceiling keeps a default run
	// bounded and honest; RAN_ASSET_MAX_BYTES raises it deliberately.
	std::size_t MaxAssetBytes()
	{
	#if defined(_MSC_VER)
	#pragma warning(push)
	#pragma warning(disable : 4996)
	#endif
		const char* limitEnv = std::getenv("RAN_ASSET_MAX_BYTES");
	#if defined(_MSC_VER)
	#pragma warning(pop)
	#endif
		if (limitEnv == nullptr || limitEnv[0] == '\0') { return 4u * 1024u * 1024u; }
		return static_cast<std::size_t>(std::strtoull(limitEnv, nullptr, 10));
	}
}

MODERN_TEST(WorldStaticMesh_RealAsuraWld0ParsesWhenAvailable)
{
	std::filesystem::path mapRoot;
	if (!FindMapRoot(mapRoot))
	{
		std::printf("      (real ASURA validation skipped: RAN_ASSET_ROOT not set or has no data/map)\n");
		return;
	}

	std::vector<std::pair<std::filesystem::path, std::size_t>> candidates;
	for (std::filesystem::directory_iterator it(mapRoot);
	     it != std::filesystem::directory_iterator(); ++it)
	{
		if (!it->is_regular_file()) { continue; }
		const std::filesystem::path& path = it->path();
		if (path.extension() != ".wld0") { continue; }
		// A `.wld0` is only meaningful next to the `.wld` that names it.
		if (!std::filesystem::exists(path.parent_path() / (path.stem().string() + ".wld")))
		{
			continue;
		}
		candidates.emplace_back(path, static_cast<std::size_t>(it->file_size()));
	}
	if (candidates.empty())
	{
		std::printf("      (real ASURA validation skipped: no .wld/.wld0 pair under %s)\n", mapRoot.string().c_str());
		return;
	}
	std::sort(candidates.begin(), candidates.end(),
	          [](const auto& a, const auto& b) { return a.second < b.second; });

	const std::size_t maxBytes = MaxAssetBytes();
	std::size_t overBudget = 0;
	for (const auto& candidate : candidates)
	{
		if (candidate.second > maxBytes) { ++overBudget; }
	}
	std::printf("      %zu real .wld0 file(s) under %s; parsing up to %zu bytes (%zu over budget, not parsed)\n", candidates.size(), mapRoot.string().c_str(), maxBytes, overBudget);

	std::size_t parsedOk = 0;
	std::size_t refused = 0;
	std::size_t totalChunks = 0;
	std::size_t totalVertices = 0;
	std::size_t totalIndices = 0;
	std::size_t totalUnsupportedFvf = 0;
	std::size_t totalSkipped = 0;
	std::size_t totalDecodedBytes = 0;
	std::size_t peakDecodedBytes = 0;

	for (const auto& candidate : candidates)
	{
		const std::filesystem::path& path = candidate.first;
		if (candidate.second > maxBytes)
		{
			std::printf("      %-28s %11zu bytes  NOT PARSED (over the %zu-byte budget)\n", path.filename().string().c_str(), candidate.second, maxBytes);
			continue;
		}
		std::ifstream in(path, std::ios::binary);
		if (!in) { continue; }
		const std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

		const Result<WorldStaticMeshAsset> decoded = Decode(bytes);
		if (decoded.IsError())
		{
			++refused;
			std::printf("      %-28s %11zu bytes  REFUSED: %s\n", path.filename().string().c_str(), bytes.size(), decoded.GetMessage());
			continue;
		}

		const WorldStaticMeshAsset& asset = decoded.GetValue();
		++parsedOk;
		totalChunks += asset.totalChunks();
		totalVertices += asset.totalVertices();
		totalIndices += asset.totalIndices();
		totalDecodedBytes += asset.decodedBytes();
		totalUnsupportedFvf += asset.unsupportedFvfCount();
		totalSkipped += asset.skippedRecordCount();
		if (asset.decodedBytes() > peakDecodedBytes) { peakDecodedBytes = asset.decodedBytes(); }
		std::printf("      %-28s %11zu bytes  meshes=%zu chunks=%zu verts=%zu idx=%zu decoded=%.2f MiB\n", path.filename().string().c_str(), bytes.size(), asset.totalMeshes(), asset.totalChunks(), asset.totalVertices(), asset.totalIndices(), static_cast<double>(asset.decodedBytes()) / (1024.0 * 1024.0));
		for (const WorldStaticMeshAsset::UnsupportedFvfEntry& entry : asset.unsupportedFvfs())
		{
			std::printf("        refused FVF 0x%03x on %zu leaf/leaves\n", entry.fvf, entry.leaves);
		}
		if (asset.skippedRecordCount() != 0u)
		{
			std::printf("        %zu record(s) recognised but not decoded\n", asset.skippedRecordCount());
		}
	}

	std::printf("      totals over %zu parsed file(s): %zu chunks, %zu vertices, %zu indices, %.2f MiB decoded (peak %.2f MiB), %zu unsupported-FVF leaves, %zu skipped, %zu refused.\n", parsedOk, totalChunks, totalVertices, totalIndices, static_cast<double>(totalDecodedBytes) / (1024.0 * 1024.0), static_cast<double>(peakDecodedBytes) / (1024.0 * 1024.0), totalUnsupportedFvf, totalSkipped, refused);

	// A run that found files and decoded none of them is a failure, not a skip.
	CHECK(candidates.size() > 0u);
	CHECK(parsedOk > 0u);
	CHECK_EQ(totalUnsupportedFvf, size_t{0});
}

// WORLD-PLAYABLE-001B: terrain mesh conversion tests (BuildTerrainMeshes).
//
// Validates that parsed `.wld0` leaves convert into renderable MeshAsset
// values, that invalid input is rejected safely, and that oversized leaves
// are skipped without failing the whole field.

MODERN_TEST(WorldTerrainMesh_EmptyFieldIsRefused)
{
	WorldStaticMeshAsset empty;
	Result<::Modern::Client::TerrainMeshSet> built = ::Modern::Client::BuildTerrainMeshes(empty);
	CHECK(built.IsError());
}

MODERN_TEST(WorldTerrainMesh_ValidLeafProducesMesh)
{
	WorldStaticMeshAsset asset;
	WorldTerrainBucket bucket;
	bucket.name = "test";

	WorldTerrainChunk chunk;
	chunk.hasNormals = true;
	chunk.hasUvs = true;
	chunk.positions = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
	chunk.normals =   { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };
	chunk.uvs =       { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };
	chunk.indices =   { 0, 1, 2 };
	bucket.chunks.push_back(chunk);
	asset.buckets[0].push_back(bucket);

	Result<::Modern::Client::TerrainMeshSet> built = ::Modern::Client::BuildTerrainMeshes(asset);
	CHECK(built.IsOk());
	const ::Modern::Client::TerrainMeshSet& set = built.GetValue();

	CHECK_EQ(set.meshes.size(), size_t{1});
	CHECK_EQ(set.totalVertices, size_t{3});
	CHECK_EQ(set.totalIndices, size_t{3});
	CHECK_EQ(set.skippedLeaves, size_t{0});
}

MODERN_TEST(WorldTerrainMesh_LeafWithoutNormalsGetsFallback)
{
	WorldStaticMeshAsset asset;
	WorldTerrainBucket bucket;
	bucket.name = "test";

	WorldTerrainChunk chunk;
	chunk.hasNormals = false;
	chunk.hasUvs = false;
	chunk.positions = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
	chunk.indices =   { 0, 1, 2 };
	bucket.chunks.push_back(chunk);
	asset.buckets[0].push_back(bucket);

	Result<::Modern::Client::TerrainMeshSet> built = ::Modern::Client::BuildTerrainMeshes(asset);
	CHECK(built.IsOk());
	const ::Modern::Client::TerrainMeshSet& set = built.GetValue();

	CHECK_EQ(set.meshes.size(), size_t{1});
	CHECK_EQ(set.totalVertices, size_t{3});
	CHECK_EQ(set.skippedLeaves, size_t{0});
}

MODERN_TEST(WorldTerrainMesh_LeafWithoutUvsGetsZeroUv)
{
	WorldStaticMeshAsset asset;
	WorldTerrainBucket bucket;
	bucket.name = "test";

	WorldTerrainChunk chunk;
	chunk.hasNormals = true;
	chunk.hasUvs = false;
	chunk.positions = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
	chunk.normals =   { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };
	chunk.indices =   { 0, 1, 2 };
	bucket.chunks.push_back(chunk);
	asset.buckets[0].push_back(bucket);

	Result<::Modern::Client::TerrainMeshSet> built = ::Modern::Client::BuildTerrainMeshes(asset);
	CHECK(built.IsOk());
	const ::Modern::Client::TerrainMeshSet& set = built.GetValue();

	CHECK_EQ(set.meshes.size(), size_t{1});
	CHECK_EQ(set.totalVertices, size_t{3});
}

MODERN_TEST(WorldTerrainMesh_LeafWithNoGeometryIsSkipped)
{
	WorldStaticMeshAsset asset;
	WorldTerrainBucket bucket;
	bucket.name = "test";

	WorldTerrainChunk emptyChunk;  // no positions, no indices
	bucket.chunks.push_back(emptyChunk);
	asset.buckets[0].push_back(bucket);

	Result<::Modern::Client::TerrainMeshSet> built = ::Modern::Client::BuildTerrainMeshes(asset);
	CHECK(built.IsError());  // no renderable geometry at all
}

MODERN_TEST(WorldTerrainMesh_LeafWithOutOfRangeIndexIsRejected)
{
	WorldStaticMeshAsset asset;
	WorldTerrainBucket bucket;
	bucket.name = "test";

	WorldTerrainChunk chunk;
	chunk.hasNormals = true;
	chunk.hasUvs = true;
	chunk.positions = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
	chunk.normals =   { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };
	chunk.uvs =       { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };
	chunk.indices =   { 0, 1, 9 };  // index 9 references a nonexistent vertex
	bucket.chunks.push_back(chunk);
	asset.buckets[0].push_back(bucket);

	Result<::Modern::Client::TerrainMeshSet> built = ::Modern::Client::BuildTerrainMeshes(asset);
	CHECK(built.IsError());  // MeshAsset validation rejects
}

MODERN_TEST(WorldTerrainMesh_MultipleLeavesAllRendered)
{
	WorldStaticMeshAsset asset;

	for (int b = 0; b < 3; ++b)
	{
		WorldTerrainBucket bucket;
		bucket.name = "bucket" + std::to_string(b);

		WorldTerrainChunk chunk;
		chunk.hasNormals = true;
		chunk.hasUvs = true;
		chunk.positions = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
		chunk.normals =   { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };
		chunk.uvs =       { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };
		chunk.indices =   { 0, 1, 2 };
		bucket.chunks.push_back(chunk);

		asset.buckets[b].push_back(bucket);
	}

	Result<::Modern::Client::TerrainMeshSet> built = ::Modern::Client::BuildTerrainMeshes(asset);
	CHECK(built.IsOk());
	const ::Modern::Client::TerrainMeshSet& set = built.GetValue();

	CHECK_EQ(set.meshes.size(), size_t{3});
	CHECK_EQ(set.totalVertices, size_t{9});
	CHECK_EQ(set.totalIndices, size_t{9});
}

// ---------------------------------------------------------------------------
// Main Test Runner
// ---------------------------------------------------------------------------

int main()
{
    std::printf("Modern WORLD-PLAYABLE-001A ASURA .wld0 terrain parser tests\n\n");

    const int failedCases = ModernTests::RunAll();

    if (failedCases == 0)
    {
        std::printf("\nAll %d test cases passed.\n",
                    static_cast<int>(ModernTests::Registry().size()));
        return 0;
    }

    std::printf("\n%d of %d test cases FAILED (%d checks).\n",
                failedCases,
                static_cast<int>(ModernTests::Registry().size()),
                ModernTests::FailureCount());
    return 1;
}
