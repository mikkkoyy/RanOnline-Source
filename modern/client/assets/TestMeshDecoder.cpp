#include "assets/TestMeshDecoder.h"

#include <cstddef>
#include <cstring>
#include <vector>

namespace Modern::Client
{
namespace
{
	// Little endian, read byte by byte. The container defines the byte order,
	// so the reader must not depend on the host's, and assembling from bytes
	// also means the header needs no alignment.
	uint32_t ReadU32LE(const uint8_t* bytes) noexcept
	{
		return static_cast<uint32_t>(bytes[0]) |
		       (static_cast<uint32_t>(bytes[1]) << 8) |
		       (static_cast<uint32_t>(bytes[2]) << 16) |
		       (static_cast<uint32_t>(bytes[3]) << 24);
	}

	// An IEEE-754 binary32 value assembled from four little endian bytes and
	// copied bit for bit: no rounding, no double round-trip, and the exact
	// bit pattern the container carried - including, if the bytes say so, a
	// NaN or an infinity, which MeshAsset::Create then refuses by name.
	float ReadF32LE(const uint8_t* bytes) noexcept
	{
		const uint32_t bits = ReadU32LE(bytes);
		float value = 0.0f;
		std::memcpy(&value, &bits, sizeof(value));
		return value;
	}

	bool HasMagic(const uint8_t* bytes) noexcept
	{
		return bytes[0] == 'M' && bytes[1] == 'E' && bytes[2] == 'S' && bytes[3] == 'H';
	}
}

Result<MeshAsset> TestMeshDecoder::DecodeMesh(const ResourceData& data)
{
	// Empty data falls out of this too: there is no header to read, so the
	// input is refused rather than treated as a zero-vertex mesh.
	if (data.GetSize() < kHeaderSize)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	const uint8_t* bytes = data.GetData();

	if (!HasMagic(bytes))
	{
		return Status(ErrorCode::InvalidArgument);
	}

	if (bytes[4] != kVersion)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// A format byte is data, not an enumerator, so it may hold any value at
	// all. ComputeMeshByteCount() is what refuses the ones with no layout,
	// which keeps "unsupported format", "zero count" and "excessive
	// geometry" one check apart.
	const MeshVertexFormat format = static_cast<MeshVertexFormat>(bytes[5]);
	const uint32_t         vertexCount = ReadU32LE(bytes + 6);
	const uint32_t         indexCount  = ReadU32LE(bytes + 10);

	const Result<size_t> expected =
		ComputeMeshByteCount(format, vertexCount, indexCount);
	if (expected.IsError())
	{
		return expected.GetStatus();
	}

	// Exact payload size. The header declares the geometry, the geometry
	// declares the byte count, and anything else in the buffer means the
	// input is not the mesh it claims to be: short is truncated, long is a
	// header wrapped around data of the wrong shape.
	const size_t payloadSize = data.GetSize() - kHeaderSize;
	if (payloadSize != expected.GetValue())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// From here the byte counts are proven, so reading the arrays cannot run
	// past the end of the payload, and the reserves cannot ask for more than
	// the ceilings allow.
	const uint8_t* cursor     = bytes + kHeaderSize;
	const size_t   vertexBytes = static_cast<size_t>(vertexCount) * kMeshVertexBytes;

	std::vector<MeshVertex> vertices;
	vertices.reserve(vertexCount);
	for (uint32_t i = 0; i < vertexCount; ++i)
	{
		const uint8_t* raw = cursor + static_cast<size_t>(i) * kMeshVertexBytes;
		vertices.push_back(MeshVertex(
			Vector3(ReadF32LE(raw + 0), ReadF32LE(raw + 4), ReadF32LE(raw + 8)),
			Vector3(ReadF32LE(raw + 12), ReadF32LE(raw + 16), ReadF32LE(raw + 20)),
			ReadF32LE(raw + 24),
			ReadF32LE(raw + 28)));
	}

	const uint8_t* indexRaw = cursor + vertexBytes;
	std::vector<MeshIndex> indices;
	indices.reserve(indexCount);
	for (uint32_t i = 0; i < indexCount; ++i)
	{
		indices.push_back(ReadU32LE(indexRaw + static_cast<size_t>(i) * kMeshIndexBytes));
	}

	// The per-vertex and per-index rules live in one place: MeshAsset::Create
	// re-checks the counts and topology above (harmlessly), and adds the two
	// rules this decoder must not implement a second version of - every
	// component finite, every index in range. A mesh that fails either comes
	// back as InvalidArgument from the same call.
	return MeshAsset::Create(PrimitiveTopology::TriangleList, std::move(vertices), std::move(indices));
}

} // namespace Modern::Client