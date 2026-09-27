#pragma once

// CLIENT-007, CLIENT-008: the vocabulary shared by typed CPU-side assets.
//
// The asset layer sits above the resource layer: a provider delivers bytes,
// ResourceManager caches them, and a decoder turns those bytes into one of the
// validated values defined here. Nothing in this layer knows about rendering,
// GPU upload, DirectX, Vulkan, OpenGL, Windows, MFC, or the legacy RAN formats.
//
// Two assets are defined: an image (CLIENT-007) and a mesh (CLIENT-008). Each
// has its own layout enum, its own size calculation and its own checked asset
// type; what they share is this file: the enums, the byte-per-element rules and
// the arithmetic that decides whether a payload size is acceptable at all.

#include "types/Result.h"

#include <cstddef>
#include <cstdint>

namespace Modern::Client
{

// Pixel layout of an image asset, one pixel at a time.
//
// Unknown is never a decodable layout: it is the name a malformed or
// unsupported header is refused under, and BytesPerPixel() answers 0 for it.
// The names describe channel order and component width, not a source format,
// so a format is a property of the bytes rather than of the file they came in.
enum class ImageFormat : uint8_t
{
	Unknown = 0,
	R8_UNorm,          // 1 byte per pixel, single intensity channel
	R8G8B8_UNorm,      // 3 bytes per pixel, red first
	R8G8B8A8_UNorm,    // 4 bytes per pixel, red first
	B8G8R8A8_UNorm,    // 4 bytes per pixel, blue first
};

// Stable name of a pixel layout, for logs and test output. An enumerator that
// is not one of the five above is reported as invalid rather than guessed at.
const char* ToString(ImageFormat format) noexcept;

// Bytes per pixel, or 0 when the layout is Unknown or not a known enumerator.
// Zero is a refusal, not a fallback: nothing in this layer infers a layout it
// was not given, and every caller of this function has to handle the zero.
uint32_t BytesPerPixel(ImageFormat format) noexcept;

// Ceilings applied to every image asset.
//
// These are policy, not a format limit: they keep a single decoded image
// bounded, and they are what makes the refusal of an oversized or
// overflowing image identical on a 32-bit build (this repository builds the
// client 32-bit) and on a 64-bit one. A limit that moved with the host would
// make the same bytes decode on one machine and fail on another.
constexpr uint32_t kMaxImageDimension = 16384;
constexpr size_t   kMaxImageBytes     = 256 * 1024 * 1024;

// Total pixel byte count for a width/height/layout triple.
//
// The product is formed in 64-bit arithmetic and refused rather than wrapped,
// so a header claiming 65535 x 65535 RGBA cannot make a small allocation look
// correct.
//
//   InvalidArgument - the layout is Unknown or not a known enumerator
//                   - width or height is 0
//                   - width or height exceeds kMaxImageDimension
//                   - the product exceeds kMaxImageBytes or cannot be held
//                     by size_t on this build
Result<size_t> ComputeImageByteCount(uint32_t width, uint32_t height, ImageFormat format) noexcept;

// ---------------------------------------------------------------------------
// CLIENT-008: mesh vocabulary
// ---------------------------------------------------------------------------

// A mesh index is a 32-bit unsigned vertex reference.
//
// One index width, not two, because a container that could declare 16-bit
// indices would need a width field, a per-width read path and a per-width test
// matrix before anything used the narrow form. Real RAN-era meshes fit in
// 16 bits, but so does the ceiling below in 32, and choosing the wider type now
// costs memory that a single per-mesh byte ceiling already bounds.
using MeshIndex = uint32_t;

// Serialised size of one vertex: eight 32-bit floats (position xyz, normal xyz,
// uv). This is the layout of MeshVertex in MeshAsset.h, which static_asserts
// that it matches this number exactly, so the declared size and the C++ type
// cannot drift apart without a compile error.
constexpr size_t kMeshVertexBytes = 8 * sizeof(float);

// Serialised size of one index.
constexpr size_t kMeshIndexBytes = sizeof(MeshIndex);

// Ceilings applied to every mesh asset.
//
// Policy, not a format limit, and fixed rather than derived from the host, so
// that the same bytes are refused identically by a 32-bit client build and a
// 64-bit one. kMaxMeshBytes is the binding one: a single mesh may not describe
// more than 8 MiB of CPU geometry. The two count ceilings bound each array on
// its own, so a header cannot ask for a million vertices and four indices and
// pass on the total alone.
constexpr size_t   kMaxMeshBytes    = 8 * 1024 * 1024;  // 8 MiB  (256 Ki vertices, or 2 Mi indices)
constexpr uint32_t kMaxMeshVertices = 262144;           // 256 Ki, 8 MiB of vertex data
constexpr uint32_t kMaxMeshIndices  = 1048576;          // 1 Mi,  4 MiB of index data

// Vertex layout of a mesh asset.
//
// Unknown is never a decodable layout: it is the name a malformed or
// unsupported header is refused under, and BytesPerVertex() answers 0 for it.
// The single enumerator is named for what it holds rather than for the
// container it arrived in, so a second layout is a new enumerator rather than
// a new decoder.
enum class MeshVertexFormat : uint8_t
{
	Unknown = 0,
	PositionNormalUvF32,  // px, py, pz, nx, ny, nz, u, v — eight float32 values
};

// Stable name of a vertex layout, for logs and test output. An enumerator that
// is not one of the two above is reported as invalid rather than guessed at.
const char* ToString(MeshVertexFormat format) noexcept;

// Bytes per vertex, or 0 when the layout is Unknown or not a known enumerator.
// Zero is a refusal, not a fallback: nothing in this layer infers a layout it
// was not given.
uint32_t BytesPerVertex(MeshVertexFormat format) noexcept;

// How a mesh's indices are grouped into primitives.
//
// Unknown is not a topology: it is the name an unsupported or unset topology is
// refused under. There is exactly one topology in CLIENT-008, and it is listed
// here rather than assumed, because "the index count is a multiple of three" is
// a property of TriangleList and not of meshes in general.
enum class PrimitiveTopology : uint8_t
{
	Unknown = 0,
	TriangleList,  // index triplets are independent triangles, in order
};

// Stable name of a topology, for logs and test output.
const char* ToString(PrimitiveTopology topology) noexcept;

// Vertices per primitive, or 0 when the topology is Unknown or not a known
// enumerator. Callers must treat 0 as a refusal before dividing by it, which is
// why every caller in this layer checks it first.
uint32_t VerticesPerPrimitive(PrimitiveTopology topology) noexcept;

// Total CPU byte count for a vertex/index pair under a vertex layout.
//
// The product is formed in 64-bit arithmetic and refused rather than wrapped,
// so a header claiming a hundred million vertices cannot make a small
// allocation look correct.
//
//   InvalidArgument - the layout is Unknown or not a known enumerator
//                   - vertex count or index count is 0
//                   - the count exceeds kMaxMeshVertices / kMaxMeshIndices
//                   - the total exceeds kMaxMeshBytes or cannot be held by
//                     size_t on this build
//
// Topology is deliberately not a parameter: this function answers "how big is
// this geometry", and whether an index count describes whole triangles is a
// rule about the mesh, checked where the mesh is built.
Result<size_t> ComputeMeshByteCount(
	MeshVertexFormat format,
	uint32_t         vertexCount,
	uint32_t         indexCount) noexcept;

} // namespace Modern::Client
