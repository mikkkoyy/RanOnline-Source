#pragma once

#include "assets/MeshDecoder.h"

#include <cstddef>
#include <cstdint>

namespace Modern::Client
{

// The one concrete mesh decoder CLIENT-008 ships: the MMESH test
// representation.
//
// MMESH (magic 'MESH') is **not** a RAN format and **not** a production format.
// It is a deliberately tiny container, defined here and nowhere else, invented
// so that the byte -> typed geometry boundary can be implemented, tested and
// demonstrated without pretending a real mesh decoder exists:
//
//   offset  size  field
//   0       4     magic 'M','E','S','H'
//   4       1     version, must be kVersion
//   5       1     vertex format, a MeshVertexFormat value; Unknown is refused
//   6       4     vertex count, little endian uint32, 1..kMaxMeshVertices
//   10      4     index count,  little endian uint32, 1..kMaxMeshIndices,
//                 a multiple of 3 (the mesh is a triangle list)
//   14      n     vertices, vertexCount * 32 bytes: per vertex eight little
//                 endian float32 values in the order px, py, pz, nx, ny, nz, u, v
//   14 + n  m     indices, indexCount * 4 bytes: little endian uint32, each
//                 naming a vertex by position
//
// Header and payload are one buffer: no compression, no per-vertex stride
// variation, no index width field, no topology field, no alignment padding and
// no trailing data. Everything the container claims is checked, so a truncated,
// oversized or structurally impossible payload is refused rather than decoded
// into a mesh with a different shape than the header described.
//
// Two deliberate omissions: the container has no topology field because
// CLIENT-008 has exactly one topology (the decoder passes TriangleList
// explicitly, and a second topology would be a new field and a version bump),
// and no index width because indices are always uint32.
//
// This class exists so the boundary is exercised by something other than the
// test binary - `ModernEmulator` decodes MMESH bytes through it - and so the
// shape of a real decoder is already settled. A modern mesh reader, or a reader
// for whatever a future legacy importer writes, implements IMeshDecoder
// alongside this one; nothing above the interface changes when it does, which
// is the entire point of the boundary.
//
// CLIENT-008 does not implement or reverse-engineer RAN mesh formats.
class TestMeshDecoder final : public IMeshDecoder
{
public:
	// The fixed header, in bytes. Public because the format is public: the
	// tests build MMESH payloads by hand from these values, and the emulator
	// assembles its demo sample from the same description.
	static constexpr size_t kHeaderSize = 14;

	// The only version this decoder accepts. Any other value is refused
	// instead of best-effort parsed, because there is no older or newer
	// version of this format to be compatible with.
	static constexpr uint8_t kVersion = 1;

	Result<MeshAsset> DecodeMesh(const ResourceData& data) override;
};

} // namespace Modern::Client