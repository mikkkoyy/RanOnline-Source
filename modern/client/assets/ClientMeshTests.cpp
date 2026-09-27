// CLIENT-008: typed mesh asset / decoder boundary tests.
//
// The asset layer turns resource bytes into validated CPU-side values. These
// cases pin down the geometry half of that: the value type that carries a mesh,
// and the decoder contract that produces one.
//
// Coverage:
//  1.  MeshVertexFormat names and bytes-per-vertex, including unknown
//      enumerators.
//  2.  PrimitiveTopology names and vertices-per-primitive, including unknown
//      enumerators.
//  3.  ComputeMeshByteCount agrees with the layout it is given.
//  4.  ComputeMeshByteCount refuses unknown layouts and zero counts.
//  5.  ComputeMeshByteCount refuses counts and totals above the ceilings.
//  6.  The serialised vertex and the MeshVertex type agree on layout.
//  7.  A valid triangle list is accepted and reports its geometry.
//  8.  Zero vertices, zero indices and zero both are refused.
//  9.  Index counts that do not make whole triangles are refused.
// 10.  Out-of-range indices are refused; any in-range winding is accepted.
// 11.  Non-finite position, normal, u and v components are refused.
// 12.  An unknown topology is refused, with and without matching geometry.
// 13.  Counts and totals above the ceilings are refused rather than wrapped.
// 14.  MeshAsset is a value type with no invalid state, and owns its geometry.
// 15.  The decoder satisfies IMeshDecoder and works through the interface.
// 16.  Decoding is deterministic across repeats and across instances.
// 17.  Empty data is refused.
// 18.  A truncated header is refused, at every length.
// 19.  A malformed header (magic, version) is refused.
// 20.  An unknown or unsupported vertex format byte is refused.
// 21.  Zero counts in the header are refused.
// 22.  The payload must agree with the header exactly: truncated at every
//      length, and extended with trailing bytes, are both refused.
// 23.  Geometry beyond the ceilings is refused before anything is allocated.
// 24.  Decoded vertices and indices survive the bytes unchanged, endianness
//      included, and the topology is the decoder's explicit choice.
// 25.  NaN and infinity in the payload are refused at construction.
// 26.  An out-of-range index in the payload is refused at construction.
// 27.  The asset layer needs no renderer, no graphics API and no legacy header.
// 28.  The decoder needs no ResourceManager, provider or id.
// 29.  MemoryResourceProvider -> ResourceManager -> ResourceData -> MeshAsset.
// 30.  A decode failure leaves the resource layer untouched.
// 31.  No legacy format is decoded; the decoder knows MESH and nothing else,
//      and the image decoder refuses mesh bytes the same way round.
// 32.  Regression: CLIENT-007's ImageAsset still decodes alongside the mesh
//      boundary in the same binary.
//
// The MMESH container used here is defined in TestMeshDecoder.h. It is a test
// representation invented for this milestone, not a RAN format, and every
// container in these cases is assembled by hand from that description rather
// than through a shared writer, so the reader is checked against an
// independent encoder. CLIENT-007's 31 image cases run unchanged in the same
// CTest run, in ModernClientAssetTests.

#include "TestHarness.h"

#include "assets/AssetTypes.h"
#include "assets/ImageAsset.h"
#include "assets/ImageDecoder.h"
#include "assets/MeshAsset.h"
#include "assets/MeshDecoder.h"
#include "assets/TestImageDecoder.h"
#include "assets/TestMeshDecoder.h"
#include "resources/MemoryResourceProvider.h"
#include "resources/ResourceData.h"
#include "resources/ResourceId.h"
#include "resources/ResourceManager.h"
#include "types/Result.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace Modern;
using namespace Modern::Client;

namespace
{
	ErrorCode DecodeError(IMeshDecoder& decoder, const ResourceData& data)
	{
		return decoder.DecodeMesh(data).GetError();
	}

	// -----------------------------------------------------------------------
	// MMESH assembly
	//
	// Written from the container description in TestMeshDecoder.h. A test
	// calling a writer from the implementation would agree with a broken
	// reader, so the header is assembled field by field here instead, and
	// every multi-byte value is emitted little endian by hand rather than by
	// dumping host memory.
	// -----------------------------------------------------------------------

	void PutU32LE(std::vector<uint8_t>& out, uint32_t value)
	{
		out.push_back(static_cast<uint8_t>(value & 0xFFu));
		out.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
		out.push_back(static_cast<uint8_t>((value >> 16) & 0xFFu));
		out.push_back(static_cast<uint8_t>((value >> 24) & 0xFFu));
	}

	// The bit pattern, not a number reformatted: float to bits to bytes is
	// exact in both directions, so a value the reader produces must be the
	// value the writer was given.
	void PutF32LE(std::vector<uint8_t>& out, float value)
	{
		uint32_t bits = 0;
		std::memcpy(&bits, &value, sizeof(bits));
		PutU32LE(out, bits);
	}

	std::vector<uint8_t> MakeHeader(
		uint32_t         vertexCount,
		uint32_t         indexCount,
		MeshVertexFormat format  = MeshVertexFormat::PositionNormalUvF32,
		uint8_t          version = TestMeshDecoder::kVersion,
		const char*      magic   = "MESH")
	{
		std::vector<uint8_t> header;
		header.reserve(TestMeshDecoder::kHeaderSize);
		header.push_back(static_cast<uint8_t>(magic[0]));
		header.push_back(static_cast<uint8_t>(magic[1]));
		header.push_back(static_cast<uint8_t>(magic[2]));
		header.push_back(static_cast<uint8_t>(magic[3]));
		header.push_back(version);
		header.push_back(static_cast<uint8_t>(format));
		PutU32LE(header, vertexCount);
		PutU32LE(header, indexCount);
		return header;
	}

	void AppendVertex(std::vector<uint8_t>& bytes, const MeshVertex& vertex)
	{
		PutF32LE(bytes, vertex.position.x);
		PutF32LE(bytes, vertex.position.y);
		PutF32LE(bytes, vertex.position.z);
		PutF32LE(bytes, vertex.normal.x);
		PutF32LE(bytes, vertex.normal.y);
		PutF32LE(bytes, vertex.normal.z);
		PutF32LE(bytes, vertex.u);
		PutF32LE(bytes, vertex.v);
	}

	void AppendIndex(std::vector<uint8_t>& bytes, MeshIndex index)
	{
		PutU32LE(bytes, index);
	}

	// A container whose header and payload agree, for the geometry the cases
	// use.
	std::vector<uint8_t> MakeMeshBytes(
		const std::vector<MeshVertex>& vertices,
		const std::vector<MeshIndex>&  indices,
		MeshVertexFormat               format = MeshVertexFormat::PositionNormalUvF32)
	{
		std::vector<uint8_t> bytes = MakeHeader(
			static_cast<uint32_t>(vertices.size()),
			static_cast<uint32_t>(indices.size()),
			format);

		for (const MeshVertex& vertex : vertices)
		{
			AppendVertex(bytes, vertex);
		}
		for (const MeshIndex index : indices)
		{
			AppendIndex(bytes, index);
		}
		return bytes;
	}

	// -----------------------------------------------------------------------
	// Test fixtures
	// -----------------------------------------------------------------------

	// A right triangle in the XY plane: distinct positions, distinct UVs and
	// a shared normal, so a decode that swaps components, drops a channel or
	// reorders vertices fails visibly rather than accidentally agreeing.
	std::vector<MeshVertex> TriangleVertices()
	{
		return {
			MeshVertex(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), 0.0f, 0.0f),
			MeshVertex(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), 1.0f, 0.0f),
			MeshVertex(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), 0.0f, 1.0f),
		};
	}

	std::vector<MeshIndex> TriangleIndices()
	{
		return { 0, 1, 2 };
	}

	std::vector<uint8_t> MakeTriangleMesh()
	{
		return MakeMeshBytes(TriangleVertices(), TriangleIndices());
	}
}

// ---------------------------------------------------------------------------
// 1-6: mesh vocabulary
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMesh_VertexFormatNamesAndBytes)
{
	CHECK_EQ(std::string(ToString(MeshVertexFormat::Unknown)), "Unknown");
	CHECK_EQ(std::string(ToString(MeshVertexFormat::PositionNormalUvF32)), "PositionNormalUvF32");

	// A format byte read out of a file is data, not an enumerator: a value
	// with no layout behind it is reported as invalid rather than guessed at.
	CHECK_EQ(std::string(ToString(static_cast<MeshVertexFormat>(200))), "Invalid");
	CHECK_EQ(std::string(ToString(static_cast<MeshVertexFormat>(255))), "Invalid");

	CHECK_EQ(BytesPerVertex(MeshVertexFormat::Unknown), 0u);
	CHECK_EQ(BytesPerVertex(static_cast<MeshVertexFormat>(200)), 0u);
	CHECK_EQ(BytesPerVertex(MeshVertexFormat::PositionNormalUvF32), static_cast<uint32_t>(kMeshVertexBytes));
}

MODERN_TEST(ClientMesh_TopologyNamesAndVerticesPerPrimitive)
{
	CHECK_EQ(std::string(ToString(PrimitiveTopology::Unknown)), "Unknown");
	CHECK_EQ(std::string(ToString(PrimitiveTopology::TriangleList)), "TriangleList");
	CHECK_EQ(std::string(ToString(static_cast<PrimitiveTopology>(200))), "Invalid");

	// Zero is the refusal every caller has to handle before dividing: an
	// unknown topology has no primitive size to group indices by.
	CHECK_EQ(VerticesPerPrimitive(PrimitiveTopology::Unknown), 0u);
	CHECK_EQ(VerticesPerPrimitive(static_cast<PrimitiveTopology>(200)), 0u);
	CHECK_EQ(VerticesPerPrimitive(PrimitiveTopology::TriangleList), 3u);
}

MODERN_TEST(ClientMesh_ComputeMeshByteCountAgreesWithLayout)
{
	// The triangle these tests use: 3 * 32 + 3 * 4 = 108 bytes.
	const Result<size_t> triangle =
		ComputeMeshByteCount(MeshVertexFormat::PositionNormalUvF32, 3, 3);
	CHECK(triangle.IsOk());
	CHECK_EQ(triangle.GetValueOr(size_t(0)), static_cast<size_t>(108));

	const Result<size_t> larger =
		ComputeMeshByteCount(MeshVertexFormat::PositionNormalUvF32, 10, 30);
	CHECK(larger.IsOk());
	CHECK_EQ(larger.GetValueOr(size_t(0)),
		static_cast<size_t>(10 * kMeshVertexBytes + 30 * kMeshIndexBytes));

	// The declared sizes and the types that carry them are the same size, so
	// the arithmetic above and the memory layout cannot disagree.
	CHECK_EQ(kMeshVertexBytes, sizeof(MeshVertex));
	CHECK_EQ(kMeshIndexBytes, sizeof(MeshIndex));
}

MODERN_TEST(ClientMesh_ComputeMeshByteCountRefusesUnknownAndEmpty)
{
	// Unknown layout, and a format byte with no layout behind it.
	CHECK_EQ(ComputeMeshByteCount(MeshVertexFormat::Unknown, 3, 3).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ComputeMeshByteCount(static_cast<MeshVertexFormat>(200), 3, 3).GetError(),
		ErrorCode::InvalidArgument);

	// Zero of either count: two different "empty" cases are both refused so
	// that no consumer has to invent a meaning for them.
	CHECK_EQ(ComputeMeshByteCount(MeshVertexFormat::PositionNormalUvF32, 0, 3).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ComputeMeshByteCount(MeshVertexFormat::PositionNormalUvF32, 3, 0).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ComputeMeshByteCount(MeshVertexFormat::PositionNormalUvF32, 0, 0).GetError(),
		ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientMesh_ComputeMeshByteCountRefusesCeilingViolations)
{
	// One vertex above the count ceiling.
	CHECK_EQ(ComputeMeshByteCount(MeshVertexFormat::PositionNormalUvF32,
		kMaxMeshVertices + 1, 3).GetError(), ErrorCode::InvalidArgument);

	// One index above the count ceiling, at a whole-triangle multiple so the
	// refusal cannot be attributed to grouping (this function has no
	// topology; the caller does).
	CHECK_EQ(ComputeMeshByteCount(MeshVertexFormat::PositionNormalUvF32,
		3, kMaxMeshIndices + 2).GetError(), ErrorCode::InvalidArgument);

	// Counts no container could ever hold: refused on the ceiling rather than
	// wrapped into something small enough to pass.
	CHECK_EQ(ComputeMeshByteCount(MeshVertexFormat::PositionNormalUvF32,
		0xFFFFFFFFu, 3).GetError(), ErrorCode::InvalidArgument);
	CHECK_EQ(ComputeMeshByteCount(MeshVertexFormat::PositionNormalUvF32,
		3, 0xFFFFFFFFu).GetError(), ErrorCode::InvalidArgument);

	// The binding ceiling: kMaxMeshVertices * 32 bytes is exactly
	// kMaxMeshBytes, so three indices push the total over. Refused rather
	// than truncated to a total that fits.
	CHECK_EQ(ComputeMeshByteCount(MeshVertexFormat::PositionNormalUvF32,
		kMaxMeshVertices, 3).GetError(), ErrorCode::InvalidArgument);

	// Just under the line: one vertex fewer, and the same three indices are
	// a legal mesh's worth of bytes.
	const Result<size_t> edge = ComputeMeshByteCount(MeshVertexFormat::PositionNormalUvF32,
		kMaxMeshVertices - 1, 3);
	CHECK(edge.IsOk());
	CHECK(edge.GetValueOr(size_t(0)) <= kMaxMeshBytes);
}

MODERN_TEST(ClientMesh_MeshVertexIsTheDeclaredLayout)
{
	// The container and the type describe the same 32 bytes. The
	// static_assert in MeshAsset.h makes this a compile error if they drift;
	// here the same claim is visible in the suite's output.
	CHECK_EQ(kMeshVertexBytes, sizeof(MeshVertex));
	CHECK_EQ(kMeshVertexBytes, static_cast<size_t>(32));
	CHECK_EQ(kMeshIndexBytes, sizeof(MeshIndex));

	// A default-constructed vertex is a real, finite vertex, not indeterminate
	// memory: the zeroed state is what a caller gets when it wants to fill in
	// components itself.
	const MeshVertex fresh;
	CHECK(fresh.IsFinite());
	CHECK(fresh.position == Vector3::Zero);
	CHECK(fresh.normal == Vector3::Zero);
	CHECK_EQ(fresh.u, 0.0f);
	CHECK_EQ(fresh.v, 0.0f);
}

// ---------------------------------------------------------------------------
// 7-14: MeshAsset
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMesh_MeshAssetAcceptsATriangle)
{
	const Result<MeshAsset> mesh = MeshAsset::Create(
		PrimitiveTopology::TriangleList, TriangleVertices(), TriangleIndices());
	CHECK(mesh.IsOk());
	if (mesh.IsError())
	{
		return;
	}

	const MeshAsset& value = mesh.GetValue();
	CHECK_EQ(std::string(ToString(value.GetTopology())), "TriangleList");
	CHECK_EQ(value.GetVertexCount(), static_cast<size_t>(3));
	CHECK_EQ(value.GetIndexCount(), static_cast<size_t>(3));
	CHECK_EQ(value.GetTriangleCount(), static_cast<size_t>(1));
	CHECK_EQ(value.GetVertexByteCount(), static_cast<size_t>(96));
	CHECK_EQ(value.GetIndexByteCount(), static_cast<size_t>(12));
	CHECK_EQ(value.GetTotalByteCount(), static_cast<size_t>(108));

	// Every component survives construction, in order: positions, normals and
	// texture coordinates come back exactly as they went in.
	const std::vector<MeshVertex>& vertices = value.GetVertices();
	CHECK_EQ(vertices.size(), static_cast<size_t>(3));
	if (vertices.size() == 3)
	{
		CHECK(vertices[0].position == Vector3(0.0f, 0.0f, 0.0f));
		CHECK(vertices[1].position == Vector3(1.0f, 0.0f, 0.0f));
		CHECK(vertices[2].normal == Vector3(0.0f, 0.0f, 1.0f));
		CHECK_EQ(vertices[1].u, 1.0f);
		CHECK_EQ(vertices[2].v, 1.0f);
	}

	const std::vector<MeshIndex>& indices = value.GetIndices();
	CHECK_EQ(indices.size(), static_cast<size_t>(3));
	if (indices.size() == 3)
	{
		CHECK_EQ(indices[0], 0u);
		CHECK_EQ(indices[1], 1u);
		CHECK_EQ(indices[2], 2u);
	}
}

MODERN_TEST(ClientMesh_MeshAssetRejectsEmptyGeometry)
{
	// Zero of either array is not a mesh, and both empties are refused by the
	// same rule rather than becoming a special "empty mesh" value above.
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList, {},
		TriangleIndices()).GetError(), ErrorCode::InvalidArgument);
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList, TriangleVertices(),
		{}).GetError(), ErrorCode::InvalidArgument);
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList, {}, {}).GetError(),
		ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientMesh_MeshAssetRejectsNonTriangleIndexCounts)
{
	// Four indices are a triangle plus a stray reference. Whole primitives
	// only: a partial triangle is not a smaller mesh, it is a malformed one,
	// and every consumer would otherwise decide for itself how to draw it.
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList, TriangleVertices(),
		{ 0, 1, 2, 0 }).GetError(), ErrorCode::InvalidArgument);
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList, TriangleVertices(),
		{ 0, 1, 2, 0, 1 }).GetError(), ErrorCode::InvalidArgument);
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList, TriangleVertices(),
		{ 0, 1 }).GetError(), ErrorCode::InvalidArgument);
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList, TriangleVertices(),
		{ 0 }).GetError(), ErrorCode::InvalidArgument);

	// Six indices over three vertices are two whole triangles and are
	// accepted: the rule is about grouping, not about how much geometry.
	CHECK(MeshAsset::Create(PrimitiveTopology::TriangleList, TriangleVertices(),
		{ 0, 1, 2, 0, 1, 2 }).IsOk());
}

MODERN_TEST(ClientMesh_MeshAssetRejectsOutOfRangeIndices)
{
	// One past the end: three vertices are addressed as 0, 1 and 2, and 3 is
	// not one of them.
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList, TriangleVertices(),
		{ 0, 1, 3 }).GetError(), ErrorCode::InvalidArgument);

	// The largest value an index can hold, so the comparison is proven against
	// the whole domain rather than only against small mistakes.
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList, TriangleVertices(),
		{ 0, 1, 0xFFFFFFFFu }).GetError(), ErrorCode::InvalidArgument);

	// Winding is not a rule here: any in-range ordering is a valid triangle,
	// and normals are not required to agree with it either.
	CHECK(MeshAsset::Create(PrimitiveTopology::TriangleList, TriangleVertices(),
		{ 2, 1, 0 }).IsOk());
	CHECK(MeshAsset::Create(PrimitiveTopology::TriangleList, TriangleVertices(),
		{ 1, 1, 1 }).IsOk());
}

MODERN_TEST(ClientMesh_MeshAssetRejectsNonFiniteComponents)
{
	const float nanValue = std::numeric_limits<float>::quiet_NaN();
	const float infinity = std::numeric_limits<float>::infinity();

	// Each component in turn. Any one of them is enough to refuse the mesh:
	// one non-finite value poisons every downstream rule that touches it,
	// without ever looking wrong in the mesh itself.
	std::vector<MeshVertex> vertices = TriangleVertices();
	vertices[0].position.x = nanValue;
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList,
		std::move(vertices), TriangleIndices()).GetError(), ErrorCode::InvalidArgument);

	vertices = TriangleVertices();
	vertices[1].normal.y = infinity;
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList,
		std::move(vertices), TriangleIndices()).GetError(), ErrorCode::InvalidArgument);

	vertices = TriangleVertices();
	vertices[2].u = -infinity;
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList,
		std::move(vertices), TriangleIndices()).GetError(), ErrorCode::InvalidArgument);

	vertices = TriangleVertices();
	vertices[2].v = nanValue;
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList,
		std::move(vertices), TriangleIndices()).GetError(), ErrorCode::InvalidArgument);

	// And the all-finite fixture still builds, so the refusals above are
	// about the components and not about the fixture.
	CHECK(MeshAsset::Create(PrimitiveTopology::TriangleList,
		TriangleVertices(), TriangleIndices()).IsOk());
}

MODERN_TEST(ClientMesh_MeshAssetRejectsUnknownTopology)
{
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::Unknown,
		TriangleVertices(), TriangleIndices()).GetError(), ErrorCode::InvalidArgument);

	// An enumerator with no rule behind it is refused rather than treated as
	// a triangle list with an unknown primitive size.
	CHECK_EQ(MeshAsset::Create(static_cast<PrimitiveTopology>(200),
		TriangleVertices(), TriangleIndices()).GetError(), ErrorCode::InvalidArgument);

	// Topology is checked first: even empty geometry with an unknown topology
	// fails the same way, so the order of the rules does not change the code
	// a caller sees.
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::Unknown, {}, {}).GetError(),
		ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientMesh_MeshAssetRejectsCeilingViolations)
{
	// One vertex above the count ceiling: refused on the count alone, before
	// the byte total is considered.
	std::vector<MeshVertex> tooManyVertices(static_cast<size_t>(kMaxMeshVertices) + 1);
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList,
		std::move(tooManyVertices), TriangleIndices()).GetError(), ErrorCode::InvalidArgument);

	// One index above the count ceiling, at a whole-triangle multiple, so the
	// refusal is the ceiling rather than the grouping rule.
	std::vector<MeshIndex> tooManyIndices(static_cast<size_t>(kMaxMeshIndices) + 2, 0);
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList,
		TriangleVertices(), std::move(tooManyIndices)).GetError(), ErrorCode::InvalidArgument);

	// Inside both count ceilings but over the byte ceiling: the vertex count
	// alone is exactly kMaxMeshBytes, and three indices cannot fit beside it.
	std::vector<MeshVertex> atVertexCeiling(kMaxMeshVertices);
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList,
		std::move(atVertexCeiling), TriangleIndices()).GetError(), ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientMesh_MeshAssetIsAValueWithNoInvalidState)
{
	const Result<MeshAsset> created = MeshAsset::Create(
		PrimitiveTopology::TriangleList, TriangleVertices(), TriangleIndices());
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}

	// A copy carries its own geometry: equal in content, owned separately, and
	// independent of the value it came from.
	MeshAsset copy = created.GetValue();
	CHECK_EQ(copy.GetVertexCount(), created.GetValue().GetVertexCount());
	CHECK_EQ(copy.GetTriangleCount(), created.GetValue().GetTriangleCount());
	CHECK(copy.GetVertices() == created.GetValue().GetVertices());
	CHECK(copy.GetIndices() == created.GetValue().GetIndices());
	CHECK(copy.GetTopology() == created.GetValue().GetTopology());

	// Moving takes the geometry with it, and the result is the same mesh.
	MeshAsset moved = std::move(copy);
	CHECK_EQ(moved.GetVertexCount(), static_cast<size_t>(3));
	CHECK_EQ(moved.GetIndexCount(), static_cast<size_t>(3));
	CHECK(moved.GetVertices() == created.GetValue().GetVertices());
	CHECK(moved.GetTopology() == PrimitiveTopology::TriangleList);

	// Nothing here observed a mesh that failed validation: there is no default
	// constructor, no setter and no non-const reference to the arrays, so a
	// MeshAsset that exists has passed every check (asserted as a type property
	// in the renderer-independence case below).
}

// ---------------------------------------------------------------------------
// 15-21: the decoder contract
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMesh_DecoderWorksThroughTheInterface)
{
	// The contract in use: a TestMeshDecoder behind IMeshDecoder produces a
	// MeshAsset from nothing but bytes, with no manager, provider or renderer
	// anywhere in the call.
	TestMeshDecoder        decoder;
	IMeshDecoder&          interface = decoder;
	const ResourceData     data(MakeTriangleMesh());

	const Result<MeshAsset> decoded = interface.DecodeMesh(data);
	CHECK(decoded.IsOk());
	if (decoded.IsError())
	{
		return;
	}

	const MeshAsset& mesh = decoded.GetValue();
	CHECK_EQ(mesh.GetVertexCount(), static_cast<size_t>(3));
	CHECK_EQ(mesh.GetIndexCount(), static_cast<size_t>(3));
	CHECK_EQ(mesh.GetTriangleCount(), static_cast<size_t>(1));
	CHECK(mesh.GetVertices() == TriangleVertices());
	CHECK(mesh.GetIndices() == TriangleIndices());

	// The interface itself has no state to configure: constructing another
	// instance and decoding the same bytes gives the same mesh.
	TestMeshDecoder other;
	CHECK(other.DecodeMesh(data).IsOk());
}

MODERN_TEST(ClientMesh_DecodingIsDeterministic)
{
	TestMeshDecoder decoder;
	const ResourceData data(MakeTriangleMesh());

	const Result<MeshAsset> first  = decoder.DecodeMesh(data);
	const Result<MeshAsset> second = decoder.DecodeMesh(data);
	CHECK(first.IsOk());
	CHECK(second.IsOk());
	if (first.IsError() || second.IsError())
	{
		return;
	}

	CHECK(first.GetValue().GetVertices() == second.GetValue().GetVertices());
	CHECK(first.GetValue().GetIndices() == second.GetValue().GetIndices());
	CHECK_EQ(first.GetValue().GetTotalByteCount(), second.GetValue().GetTotalByteCount());

	// A different instance of the same stateless decoder agrees with the
	// first, because there is no scratch state for them to disagree about.
	TestMeshDecoder fresh;
	const Result<MeshAsset> third = fresh.DecodeMesh(data);
	CHECK(third.IsOk());
	if (third.IsError())
	{
		return;
	}
	CHECK(third.GetValue().GetVertices() == first.GetValue().GetVertices());
	CHECK(third.GetValue().GetIndices() == first.GetValue().GetIndices());
}

MODERN_TEST(ClientMesh_DecoderRejectsEmptyData)
{
	// Empty is not a zero-vertex mesh: there is not even a header to read, so
	// the input is refused rather than interpreted.
	TestMeshDecoder decoder;
	CHECK_EQ(DecodeError(decoder, ResourceData(std::vector<uint8_t>())),
		ErrorCode::InvalidArgument);
	CHECK_EQ(DecodeError(decoder, ResourceData("")), ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientMesh_DecoderRejectsTruncatedHeader)
{
	// Every length below the header is refused: no prefix of the container is
	// a smaller valid container, so there is no partial read that succeeds.
	TestMeshDecoder decoder;
	const std::vector<uint8_t> full = MakeTriangleMesh();

	for (size_t length = 0; length < TestMeshDecoder::kHeaderSize; ++length)
	{
		const std::vector<uint8_t> prefix(full.begin(), full.begin() + static_cast<ptrdiff_t>(length));
		CHECK_EQ(DecodeError(decoder, ResourceData(prefix)), ErrorCode::InvalidArgument);
	}

	// One byte short of the full container is refused too: the truncation rule
	// covers the payload as well as the header (every payload length is
	// checked in the mismatch case below).
	const std::vector<uint8_t> almost(full.begin(), full.end() - 1);
	CHECK_EQ(DecodeError(decoder, ResourceData(almost)), ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientMesh_DecoderRejectsMalformedHeader)
{
	TestMeshDecoder decoder;
	const std::vector<uint8_t> valid = MakeTriangleMesh();

	// Each magic position, flipped individually: the signature is four bytes
	// and all four are checked, not just the first.
	for (size_t position = 0; position < 4; ++position)
	{
		std::vector<uint8_t> broken = valid;
		broken[position] = static_cast<uint8_t>(broken[position] ^ 0xFFu);
		CHECK_EQ(DecodeError(decoder, ResourceData(broken)), ErrorCode::InvalidArgument);
	}

	// Versions this format does not have, instead of best-effort parsing: the
	// only accepted value is the one the header declares.
	std::vector<uint8_t> wrongVersion = valid;
	wrongVersion[4] = 0;
	CHECK_EQ(DecodeError(decoder, ResourceData(wrongVersion)), ErrorCode::InvalidArgument);
	wrongVersion[4] = 2;
	CHECK_EQ(DecodeError(decoder, ResourceData(wrongVersion)), ErrorCode::InvalidArgument);
	wrongVersion[4] = 0xFF;
	CHECK_EQ(DecodeError(decoder, ResourceData(wrongVersion)), ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientMesh_DecoderRejectsUnknownFormatByte)
{
	// A format byte is data, not an enumerator, so it may hold any value at
	// all: Unknown, a value with no enumerator behind it, and the extreme.
	TestMeshDecoder decoder;
	const std::vector<uint8_t> valid = MakeTriangleMesh();

	const uint8_t refusals[] = { 0x00u, 0x02u, 0x63u, 0xFFu };
	for (const uint8_t format : refusals)
	{
		std::vector<uint8_t> broken = valid;
		broken[5] = format;
		CHECK_EQ(DecodeError(decoder, ResourceData(broken)), ErrorCode::InvalidArgument);
	}

	// The known format byte still decodes, so the loop above is refusing the
	// values and not the byte.
	std::vector<uint8_t> known = valid;
	known[5] = static_cast<uint8_t>(MeshVertexFormat::PositionNormalUvF32);
	CHECK_EQ(DecodeError(decoder, ResourceData(known)), ErrorCode::None);
}

MODERN_TEST(ClientMesh_DecoderRejectsZeroCounts)
{
	TestMeshDecoder decoder;

	// Zero vertices: the header says so before the payload is examined, and
	// the payload behind it (three indices, no vertices) never gets read.
	const std::vector<uint8_t> zeroVertices = MakeHeader(0, 3);
	std::vector<uint8_t> indicesOnly;
	for (const MeshIndex index : TriangleIndices())
	{
		AppendIndex(indicesOnly, index);
	}
	std::vector<uint8_t> withZeroVertices = zeroVertices;
	withZeroVertices.insert(withZeroVertices.end(), indicesOnly.begin(), indicesOnly.end());
	CHECK_EQ(DecodeError(decoder, ResourceData(withZeroVertices)), ErrorCode::InvalidArgument);

	// Zero indices: three vertices with nothing to draw them.
	std::vector<uint8_t> withZeroIndices = MakeHeader(3, 0);
	for (const MeshVertex& vertex : TriangleVertices())
	{
		AppendVertex(withZeroIndices, vertex);
	}
	CHECK_EQ(DecodeError(decoder, ResourceData(withZeroIndices)), ErrorCode::InvalidArgument);

	// Zero of both: refused on the counts, not on the empty payload.
	CHECK_EQ(DecodeError(decoder, ResourceData(MakeHeader(0, 0))),
		ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientMesh_DecoderRejectsPayloadThatDisagreesWithHeader)
{
	TestMeshDecoder decoder;
	const std::vector<uint8_t> full = MakeTriangleMesh();
	const size_t expectedPayload = 3 * kMeshVertexBytes + 3 * kMeshIndexBytes;

	// Every truncation of the payload: the header still claims a full mesh,
	// so a short buffer is refused instead of decoded as a smaller one. The
	// loop starts at the header size; lengths below it are covered by the
	// truncated-header case.
	for (size_t length = TestMeshDecoder::kHeaderSize;
	     length < TestMeshDecoder::kHeaderSize + expectedPayload;
	     ++length)
	{
		const std::vector<uint8_t> shortened(full.begin(), full.begin() + static_cast<ptrdiff_t>(length));
		CHECK_EQ(DecodeError(decoder, ResourceData(shortened)), ErrorCode::InvalidArgument);
	}

	// Trailing bytes in both directions: a long buffer is a header wrapped
	// around data of the wrong shape, not a mesh with a spare tail.
	std::vector<uint8_t> trailing = full;
	trailing.push_back(0x00u);
	CHECK_EQ(DecodeError(decoder, ResourceData(trailing)), ErrorCode::InvalidArgument);

	std::vector<uint8_t> trailingSeven = full;
	trailingSeven.insert(trailingSeven.end(), 7, 0xABu);
	CHECK_EQ(DecodeError(decoder, ResourceData(trailingSeven)), ErrorCode::InvalidArgument);

	// A declared count that matches neither the payload nor a legal size: the
	// header's counts and the geometry's bytes must be the same statement.
	std::vector<uint8_t> lyingHeader = MakeHeader(4, 3);
	lyingHeader.insert(lyingHeader.end(),
		full.begin() + static_cast<ptrdiff_t>(TestMeshDecoder::kHeaderSize), full.end());
	CHECK_EQ(DecodeError(decoder, ResourceData(lyingHeader)), ErrorCode::InvalidArgument);

	// And the intact container still decodes, so the loop refused lengths.
	CHECK_EQ(DecodeError(decoder, ResourceData(full)), ErrorCode::None);
}

MODERN_TEST(ClientMesh_DecoderRejectsCeilingViolationsBeforeAllocating)
{
	// The header claims geometry no host could hold: four billion vertices and
	// the same again in indices, in a buffer of a few hundred bytes. The claim
	// is refused from the header alone - on the ceiling, before the payload
	// size is compared and long before anything is reserved - which is why the
	// input here is far too small for what it declares and the test still
	// passes without pretending to hold eight gigabytes.
	TestMeshDecoder decoder;
	const std::vector<uint8_t> absurd = MakeHeader(0xFFFFFFFFu, 0xFFFFFFFFu);
	CHECK_EQ(DecodeError(decoder, ResourceData(absurd)), ErrorCode::InvalidArgument);

	// One vertex above the ceiling, at a plausible payload size: refused on
	// the count rather than accepted because the buffer happens to match.
	const std::vector<uint8_t> overVertices = MakeHeader(kMaxMeshVertices + 1, 3);
	CHECK_EQ(DecodeError(decoder, ResourceData(overVertices)), ErrorCode::InvalidArgument);

	// One index group above the ceiling, at a whole-triangle multiple.
	const std::vector<uint8_t> overIndices = MakeHeader(3, kMaxMeshIndices + 2);
	CHECK_EQ(DecodeError(decoder, ResourceData(overIndices)), ErrorCode::InvalidArgument);

	// At both count ceilings the byte ceiling still binds: 256 Ki vertices
	// alone are the full 8 MiB, so three indices cannot fit beside them.
	const std::vector<uint8_t> overBytes = MakeHeader(kMaxMeshVertices, 3);
	CHECK_EQ(DecodeError(decoder, ResourceData(overBytes)), ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// 24: bytes survive the round trip
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMesh_DecodedGeometrySurvivesTheBytes)
{
	// Distinct, exactly representable values across every component and both
	// arrays: if the reader shifted a byte, swapped a component, sign-extended
	// an index or read the host's byte order instead of the container's, these
	// comparisons fail. Nothing here depends on rounding.
	std::vector<MeshVertex> vertices = {
		MeshVertex(Vector3(1.5f, -2.25f, 1024.0f), Vector3(0.0f, 1.0f, 0.0f), 0.125f, 0.875f),
		MeshVertex(Vector3(-0.5f, 3.0f, -512.0f), Vector3(1.0f, 0.0f, 0.0f), 0.5f, 0.25f),
		MeshVertex(Vector3(7.75f, 0.0625f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), 1.0f, 0.0f),
	};
	const std::vector<MeshIndex> indices = { 2, 0, 1 };

	TestMeshDecoder decoder;
	const Result<MeshAsset> decoded = decoder.DecodeMesh(
		ResourceData(MakeMeshBytes(vertices, indices)));
	CHECK(decoded.IsOk());
	if (decoded.IsError())
	{
		return;
	}

	CHECK(decoded.GetValue().GetVertices() == vertices);
	CHECK(decoded.GetValue().GetIndices() == indices);

	// The container has no topology field, so the decoder states one: every
	// decoded mesh is this layer's single topology, chosen in code rather than
	// inherited from a byte that might say otherwise.
	CHECK(decoded.GetValue().GetTopology() == PrimitiveTopology::TriangleList);

	// And the total is exactly what the container carried, counted with the
	// declared layout sizes rather than with sizeof.
	CHECK_EQ(decoded.GetValue().GetTotalByteCount(),
		3 * kMeshVertexBytes + 3 * kMeshIndexBytes);
}

// ---------------------------------------------------------------------------
// 27-32: boundary and regression
// ---------------------------------------------------------------------------

namespace
{
	// Builds a ResourceId, asserting that the identifier layer accepts it: the
	// name rule is CLIENT-005's, so a failure here is a fixture problem rather
	// than a result of this milestone.
	ResourceId MakeId(const char* name)
	{
		const Result<ResourceId> id = ResourceId::Create(name);
		CHECK(id.IsOk());
		return id.GetValueOr(ResourceId());
	}
}

MODERN_TEST(ClientMesh_MeshLayerDoesNotRequireRendererOrLegacy)
{
	// The guarantee is structural, exactly as CLIENT-007's equivalent case:
	// this translation unit includes TestHarness.h, the asset and resource
	// headers and standard headers only, and links ModernClientAssets,
	// ModernClientResources and Modern. It links no renderer, no DirectX, no
	// D3DX, no Vulkan, no OpenGL, no MFC, no Windows header and no legacy
	// library, so a dependency in any of those directions would fail to
	// compile or fail to link rather than pass quietly.
	//
	// What can be asserted here is the shape that makes those absences hold:
	// the decoder is a pure interface, the asset is a self-contained value,
	// and decoding happens in a console process with no window and no device.
	static_assert(std::is_abstract<IMeshDecoder>::value,
		"a decoder must be an interface, not a renderer");
	static_assert(std::is_base_of<IMeshDecoder, TestMeshDecoder>::value,
		"the concrete decoder must be a leaf behind the interface");
	static_assert(!std::is_default_constructible<MeshAsset>::value,
		"an asset must not be constructible without validation");

	// No member of MeshAsset is a backend object or a resource: the interface
	// is topology, CPU-side vertices and CPU-side index integers.
	static_assert(std::is_same<decltype(std::declval<const MeshAsset&>().GetVertices()),
		const std::vector<MeshVertex>&>::value,
		"vertices must be CPU-side values");
	static_assert(std::is_same<decltype(std::declval<const MeshAsset&>().GetIndices()),
		const std::vector<MeshIndex>&>::value,
		"indices must be CPU-side integers");
	static_assert(std::is_same<decltype(std::declval<const MeshAsset&>().GetTopology()),
		PrimitiveTopology>::value,
		"the topology must be one of this layer's own enumerators");

	TestMeshDecoder decoder;
	const Result<MeshAsset> decoded = decoder.DecodeMesh(ResourceData(MakeTriangleMesh()));
	CHECK(decoded.IsOk());
	if (decoded.IsError())
	{
		return;
	}
	CHECK_EQ(decoded.GetValue().GetTotalByteCount(), static_cast<size_t>(108));
}

MODERN_TEST(ClientMesh_DecoderDoesNotDependOnResourceManager)
{
	// A decoder needs bytes and nothing else: no manager, no provider, no id.
	// The ResourceData here is built by hand and never loaded, which is the
	// strongest statement available that decoding is independent of where the
	// bytes came from. The integration case below shows the other direction:
	// the same decoder over bytes a provider and a manager produced.
	const std::vector<uint8_t> bytes = MakeTriangleMesh();
	const ResourceData byHand(bytes);

	TestMeshDecoder decoder;
	const Result<MeshAsset> first = decoder.DecodeMesh(byHand);
	CHECK(first.IsOk());
	if (first.IsError())
	{
		return;
	}

	// Wrapping the same bytes differently - a copy of the vector - decodes to
	// the same mesh, because the decoder reads ResourceData and not the object
	// identity behind it.
	const std::vector<uint8_t> copy(bytes);
	const Result<MeshAsset> second = decoder.DecodeMesh(ResourceData(copy));
	CHECK(second.IsOk());
	if (second.IsError())
	{
		return;
	}
	CHECK(second.GetValue().GetVertices() == first.GetValue().GetVertices());
	CHECK(second.GetValue().GetIndices() == first.GetValue().GetIndices());
	CHECK_EQ(second.GetValue().GetTotalByteCount(), first.GetValue().GetTotalByteCount());
}

MODERN_TEST(ClientMesh_MemoryProviderThroughManagerToMeshAsset)
{
	TestMeshDecoder decoder;
	const std::vector<uint8_t> bytes = MakeTriangleMesh();

	MemoryResourceProvider provider;
	ResourceManager        manager;
	manager.SetProvider(&provider);
	CHECK(manager.Initialize().IsOk());

	const ResourceId id = MakeId("meshes/test_mesh.mmsh");
	CHECK(provider.RegisterResource(id, ResourceData(bytes)).IsOk());

	// provider -> ResourceManager -> ResourceData
	const Result<ResourceData> loaded = manager.Load(id);
	CHECK(loaded.IsOk());
	if (loaded.IsError())
	{
		return;
	}

	// The manager hands back exactly the bytes the provider stored. Nothing
	// above the provider interprets them, which is what leaves decoding to the
	// decoder rather than to the cache.
	CHECK(loaded.GetValue() == ResourceData(bytes));
	CHECK_EQ(loaded.GetValue().GetSize(), bytes.size());
	CHECK(manager.IsCached(id));

	// ResourceData -> decoder -> MeshAsset
	const Result<MeshAsset> decoded = decoder.DecodeMesh(loaded.GetValue());
	CHECK(decoded.IsOk());
	if (decoded.IsError())
	{
		return;
	}
	CHECK_EQ(decoded.GetValue().GetVertexCount(), static_cast<size_t>(3));
	CHECK(decoded.GetValue().GetVertices() == TriangleVertices());
	CHECK(decoded.GetValue().GetIndices() == TriangleIndices());
	CHECK(decoded.GetValue().GetTopology() == PrimitiveTopology::TriangleList);

	// Decoding is not caching: the manager still holds bytes, and the asset is
	// a separate value produced from them. Decoding the cached bytes again
	// yields an equal mesh rather than the same object.
	const Result<MeshAsset> again = decoder.DecodeMesh(manager.Load(id).GetValue());
	CHECK(again.IsOk());
	if (again.IsError())
	{
		return;
	}
	CHECK(again.GetValue().GetVertices() == decoded.GetValue().GetVertices());
	CHECK(again.GetValue().GetIndices() == decoded.GetValue().GetIndices());

	// What the manager caches is still the container bytes and not the mesh:
	// the layer above the provider stays untyped, and the decoder stays the
	// only thing that knows about vertices.
	CHECK(manager.IsCached(id));
	CHECK_EQ(manager.GetCachedCount(), static_cast<size_t>(1));
	CHECK(manager.Load(id).GetValue() == ResourceData(bytes));

	CHECK(manager.Shutdown().IsOk());
}

MODERN_TEST(ClientMesh_DecodeFailureLeavesManagerIntact)
{
	// A decode failure is a statement about bytes, not about the resource
	// layer: garbage loads and caches like any other resource, and it is the
	// decoder that refuses it. Nothing is evicted, rewritten or confused by
	// the refusal, and the caller can still tell "these bytes are not a mesh"
	// from "this resource does not exist" because the codes stay separate.
	TestMeshDecoder decoder;
	const std::vector<uint8_t> valid   = MakeTriangleMesh();
	const std::vector<uint8_t> garbage = {
		'n', 'o', 't', ' ', 'a', ' ', 'm', 'e', 's', 'h', '!', 0x00u, 0x01u, 0x02u, 0x03u
	};

	MemoryResourceProvider provider;
	ResourceManager        manager;
	manager.SetProvider(&provider);
	CHECK(manager.Initialize().IsOk());

	const ResourceId goodId = MakeId("meshes/good.mmsh");
	const ResourceId badId  = MakeId("meshes/bad.mmsh");
	CHECK(provider.RegisterResource(goodId, ResourceData(valid)).IsOk());
	CHECK(provider.RegisterResource(badId, ResourceData(garbage)).IsOk());

	CHECK(manager.Load(goodId).IsOk());
	CHECK(manager.Load(badId).IsOk());
	CHECK(manager.IsCached(goodId));
	CHECK(manager.IsCached(badId));
	const size_t cachedBefore = manager.GetCachedCount();

	// The bytes load fine - that is the resource layer's job - and decoding is
	// what refuses them: the failure code comes from the decoder alone.
	const Result<ResourceData> loaded = manager.Load(badId);
	CHECK(loaded.IsOk());
	if (loaded.IsError())
	{
		return;
	}
	CHECK_EQ(DecodeError(decoder, loaded.GetValue()), ErrorCode::InvalidArgument);

	// The valid mesh still decodes in the same run, so the failure above is
	// about those bytes and not about the decoder being left in a bad state.
	CHECK_EQ(DecodeError(decoder, ResourceData(valid)), ErrorCode::None);

	// And the resource layer is untouched: same entries, same bytes.
	CHECK(manager.IsCached(goodId));
	CHECK(manager.IsCached(badId));
	CHECK_EQ(manager.GetCachedCount(), cachedBefore);
	CHECK(manager.Load(goodId).GetValue() == ResourceData(valid));
	CHECK(manager.Load(badId).GetValue() == ResourceData(garbage));

	CHECK(manager.Shutdown().IsOk());
}

MODERN_TEST(ClientMesh_NoLegacyFormatIsDecoded)
{
	// The one mesh decoder in the build recognises the documented MMESH
	// container and nothing else. Inputs that look like other formats are
	// refused rather than guessed at, which is what keeps "CLIENT-008 does
	// not decode RAN assets" a testable statement instead of a promise.
	TestMeshDecoder decoder;

	// An X file header ('xof '), the modern-era mesh container most likely to
	// be mistaken for something this layer handles.
	const std::vector<uint8_t> xFile = {
		'x', 'o', 'f', ' ', '0', '3', '0', '0', 't', 'x', 't', ' ', '0', '0', '3', '2'
	};
	CHECK_EQ(DecodeError(decoder, ResourceData(xFile)), ErrorCode::InvalidArgument);

	// A legacy-looking binary blob, and plain text: neither is this container.
	const uint8_t legacyBlob[] = {
		0x00u, 0x00u, 0x02u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
		0x12u, 0x34u, 0x56u, 0x78u, 0x9Au, 0xBCu
	};
	CHECK_EQ(DecodeError(decoder, ResourceData(legacyBlob, sizeof(legacyBlob))),
		ErrorCode::InvalidArgument);
	CHECK_EQ(DecodeError(decoder, ResourceData("meshes/npc/butcha.rmx")),
		ErrorCode::InvalidArgument);

	// Four bytes of the magic with nothing behind them is still not a mesh:
	// the container is longer than its signature.
	CHECK_EQ(DecodeError(decoder, ResourceData("MESH")), ErrorCode::InvalidArgument);

	// And the image decoder refuses mesh bytes: each boundary accepts only its
	// own container, so "which decoder does this need" has exactly one answer
	// instead of two decoders both saying yes.
	TestImageDecoder imageDecoder;
	CHECK_EQ(imageDecoder.DecodeImage(ResourceData(MakeTriangleMesh())).GetError(),
		ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientMesh_ImageAssetStillDecodesAlongside)
{
	// Regression: CLIENT-007 and CLIENT-008 share AssetTypes.h, the asset
	// library and this binary's link. The image boundary still turns MIMG
	// bytes into pixels with the mesh vocabulary compiled in beside it, and a
	// mesh decode failure does not disturb it either.
	TestImageDecoder imageDecoder;

	// A hand-assembled MIMG container, per TestImageDecoder.h: magic, version,
	// format byte, little endian width and height, then the pixel bytes.
	std::vector<uint8_t> mimg;
	mimg.push_back('M');
	mimg.push_back('I');
	mimg.push_back('M');
	mimg.push_back('G');
	mimg.push_back(TestImageDecoder::kVersion);
	mimg.push_back(static_cast<uint8_t>(ImageFormat::R8G8B8A8_UNorm));
	mimg.push_back(2);
	mimg.push_back(0);
	mimg.push_back(2);
	mimg.push_back(0);
	mimg.insert(mimg.end(), 16, 0xABu);

	const Result<ImageAsset> image = imageDecoder.DecodeImage(ResourceData(mimg));
	CHECK(image.IsOk());
	if (image.IsError())
	{
		return;
	}
	CHECK_EQ(image.GetValue().GetWidth(), 2u);
	CHECK_EQ(image.GetValue().GetHeight(), 2u);
	CHECK_EQ(std::string(ToString(image.GetValue().GetFormat())), "R8G8B8A8_UNorm");
	CHECK_EQ(image.GetValue().GetPixelByteCount(), static_cast<size_t>(16));

	// The mesh decoder refuses those same image bytes, so the two boundaries
	// have not drifted toward each other while the mesh half was added.
	TestMeshDecoder meshDecoder;
	CHECK_EQ(DecodeError(meshDecoder, ResourceData(mimg)), ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// Main Test Runner
// ---------------------------------------------------------------------------

int main()
{
	std::printf("Modern client typed mesh asset / decoder boundary tests (CLIENT-008)\n\n");

	const int failedCases = ModernTests::RunAll();

	if (failedCases == 0)
	{
		std::printf("\nAll %d test cases passed.\n", static_cast<int>(ModernTests::Registry().size()));
		return 0;
	}

	std::printf("\n%d of %d test cases FAILED (%d checks).\n",
		failedCases,
		static_cast<int>(ModernTests::Registry().size()),
		ModernTests::FailureCount());
	return 1;
}




// ---------------------------------------------------------------------------
// 25-26: per-element payload rules
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMesh_DecoderRejectsNonFiniteFloatsInPayload)
{
	// The header, the counts and the payload size are all correct; the only
	// thing wrong is the value of one float. The container carries the bit
	// pattern of a NaN or an infinity where a position, a normal or a texture
	// coordinate belongs, and the decode is refused at construction - the
	// same rule the caller reaches by building the vertex by hand.
	TestMeshDecoder decoder;

	auto withBitsAt = [](size_t payloadOffset, uint32_t bits)
	{
		std::vector<uint8_t> bytes = MakeTriangleMesh();
		const size_t offset = TestMeshDecoder::kHeaderSize + payloadOffset;
		bytes[offset + 0] = static_cast<uint8_t>(bits & 0xFFu);
		bytes[offset + 1] = static_cast<uint8_t>((bits >> 8) & 0xFFu);
		bytes[offset + 2] = static_cast<uint8_t>((bits >> 16) & 0xFFu);
		bytes[offset + 3] = static_cast<uint8_t>((bits >> 24) & 0xFFu);
		return bytes;
	};

	// Vertex 0, position x: a quiet NaN.
	CHECK_EQ(DecodeError(decoder, ResourceData(withBitsAt(0, 0x7FC00000u))),
		ErrorCode::InvalidArgument);

	// Vertex 1, normal y (vertex stride then component offset): +infinity.
	CHECK_EQ(DecodeError(decoder, ResourceData(withBitsAt(kMeshVertexBytes + 16, 0x7F800000u))),
		ErrorCode::InvalidArgument);

	// Vertex 2, u: -infinity.
	CHECK_EQ(DecodeError(decoder, ResourceData(withBitsAt(2 * kMeshVertexBytes + 24, 0xFF800000u))),
		ErrorCode::InvalidArgument);

	// And the untouched container decodes, so the refusals are about the
	// values and not about the way the bytes are read.
	CHECK_EQ(DecodeError(decoder, ResourceData(MakeTriangleMesh())), ErrorCode::None);
}

MODERN_TEST(ClientMesh_DecoderRejectsOutOfRangeIndexInPayload)
{
	// Header, counts and payload size all agree, and the last index simply
	// names a vertex that does not exist. No size rule can see this; it is
	// caught where every index is checked against the vertex count.
	TestMeshDecoder decoder;
	const size_t lastIndexOffset = TestMeshDecoder::kHeaderSize +
		3 * kMeshVertexBytes + 2 * kMeshIndexBytes;

	std::vector<uint8_t> justPastTheEnd = MakeTriangleMesh();
	justPastTheEnd[lastIndexOffset + 0] = 3;  // index 3, one past the last vertex
	justPastTheEnd[lastIndexOffset + 1] = 0;
	justPastTheEnd[lastIndexOffset + 2] = 0;
	justPastTheEnd[lastIndexOffset + 3] = 0;
	CHECK_EQ(DecodeError(decoder, ResourceData(justPastTheEnd)), ErrorCode::InvalidArgument);

	// The largest value an index can hold, refused the same way.
	std::vector<uint8_t> largest = MakeTriangleMesh();
	largest[lastIndexOffset + 0] = 0xFFu;
	largest[lastIndexOffset + 1] = 0xFFu;
	largest[lastIndexOffset + 2] = 0xFFu;
	largest[lastIndexOffset + 3] = 0xFFu;
	CHECK_EQ(DecodeError(decoder, ResourceData(largest)), ErrorCode::InvalidArgument);

	// The original index (2) still decodes, so the refusals are about range.
	CHECK_EQ(DecodeError(decoder, ResourceData(MakeTriangleMesh())), ErrorCode::None);
}








