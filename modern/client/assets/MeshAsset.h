#pragma once

#include "assets/AssetTypes.h"
#include "math/Vector3.h"
#include "types/Result.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

// The serialised vertex is eight float32 values, and MeshVertex is what reads
// them: if anyone adds a member, changes a float, or the compiler inserts
// padding, the declared layout and the C++ type stop agreeing and the build
// stops instead of the container drifting out of step with the asset.
static_assert(sizeof(float) == 4, "the mesh vertex layout is defined in 32-bit floats");
static_assert(sizeof(Modern::Vector3) == 3 * sizeof(float), "Vector3 must be three packed floats");
static_assert(sizeof(Modern::Client::MeshIndex) == 4, "a mesh index is 32 bits");

namespace Modern::Client
{

// One vertex of CPU-side geometry: where it is, which way it faces, and where
// its texture coordinates sit.
//
// Position and normal reuse Modern::Vector3, the core's existing three-float
// vector, rather than growing a second vector type or a math library for this
// milestone. UV is two floats because core math has no two-component type and
// CLIENT-008 does not extend core; a future Vector2 belongs there, in core, for
// everyone — not inside an asset header.
//
// Normal is stored as given. It is not required to be unit length, non-zero or
// normalised: whether a normal needs renormalising is a decision for whoever
// consumes the mesh (an importer, a renderer, a tool), and enforcing it here
// would reject data that is perfectly valid for every other use. What is
// enforced is that it is finite.
struct MeshVertex
{
	Vector3 position = Vector3::Zero;
	Vector3 normal   = Vector3::Zero;
	float   u        = 0.0f;
	float   v        = 0.0f;

	MeshVertex() = default;
	MeshVertex(const Vector3& position_, const Vector3& normal_, float u_, float v_) noexcept
		: position(position_)
		, normal(normal_)
		, u(u_)
		, v(v_)
	{
	}

	// NaN and infinity are refused at the boundary, exactly as Vector3 does
	// for positions: one non-finite vertex poisons every downstream rule that
	// touches it — a bounding sphere, a tangent basis, a cull test — without
	// ever looking wrong in the mesh itself.
	bool IsFinite() const
	{
		return position.IsFinite() && normal.IsFinite() &&
		       std::isfinite(u) && std::isfinite(v);
	}
};

// Componentwise equality. Two vertices are equal when every component is: no
// epsilon, because equality here means "the same values were decoded", which
// is a statement about the bytes rather than about distance in space.
inline bool operator==(const MeshVertex& a, const MeshVertex& b) noexcept
{
	return a.position == b.position && a.normal == b.normal && a.u == b.u && a.v == b.v;
}

inline bool operator!=(const MeshVertex& a, const MeshVertex& b) noexcept
{
	return !(a == b);
}

static_assert(sizeof(MeshVertex) == kMeshVertexBytes, "MeshVertex must match the serialised vertex size");

// A validated CPU-side mesh: its topology, its vertices and its indices.
//
// A MeshAsset is the geometry half of the asset layer, the counterpart of
// ImageAsset (CLIENT-007). It exists so that everything above it can work with
// "a triangle list" rather than with "bytes that are probably a mesh".
//
// What it deliberately is not:
//
//   - **Not a GPU resource.** There is no `ID3D11Buffer`,
//     `IDirect3DVertexBuffer9`, `VkBuffer`, `GLuint` or device pointer here,
//     and no `<Windows.h>` in this header. Buffer creation, vertex declaration
//     setup and index format choice belong to a future renderer adapter that
//     reads a MeshAsset; the asset never learns which API did so.
//   - **Not an animation, a material or a skeleton.** Positions, normals, UVs
//     and triangle indices only. Skinning weights, materials, shaders and
//     animation clips are separate future assets, each with its own type — not
//     optional fields on this one.
//   - **Not a file, a stream or a decoder.** It holds no path, no handle and
//     no reference to the bytes it was decoded from.
//   - **Not a mutable buffer.** Vertices and indices leave through const
//     references, and `Create()` is the only way to obtain a value: to change
//     geometry, build a new MeshAsset, which re-runs every check.
//
// There is no default constructor, so there is no invalid MeshAsset state to
// inspect: a value that exists has already passed validation.
class MeshAsset
{
public:
	// Builds a mesh after checking every invariant it claims to satisfy. The
	// checks run in this order, cheap structural rules before per-element
	// ones, so a malformed mesh is refused before anything walks it:
	//
	//   InvalidArgument - topology is Unknown or not a known enumerator
	//                   - zero vertices or zero indices
	//                   - an index count that does not divide into whole
	//                     primitives (indexCount % 3 == 0 for TriangleList)
	//                   - vertex or index count above its ceiling
	//                     (kMaxMeshVertices / kMaxMeshIndices)
	//                   - total geometry above kMaxMeshBytes or not
	//                     representable by size_t on this build
	//                   - any vertex component that is not finite
	//                     (NaN, positive infinity or negative infinity)
	//                   - any index that does not reference an existing
	//                     vertex
	//
	// The count ceilings are checked before the counts are narrowed to
	// uint32_t for ComputeMeshByteCount, so the byte calculation never sees a
	// size it could misrepresent.
	static Result<MeshAsset> Create(
		PrimitiveTopology       topology,
		std::vector<MeshVertex> vertices,
		std::vector<MeshIndex>  indices);

	PrimitiveTopology GetTopology() const noexcept { return m_topology; }

	// Counts and derived triangle count. Neither array is empty for a value
	// that exists, because zero of either is refused at construction.
	size_t GetVertexCount() const noexcept { return m_vertices.size(); }
	size_t GetIndexCount() const noexcept { return m_indices.size(); }
	size_t GetTriangleCount() const noexcept { return m_indices.size() / 3; }

	// Serialised byte counts, using the declared layout sizes rather than
	// sizeof, so these answer the same question the decoders answer.
	size_t GetVertexByteCount() const noexcept { return m_vertices.size() * kMeshVertexBytes; }
	size_t GetIndexByteCount() const noexcept { return m_indices.size() * kMeshIndexBytes; }
	size_t GetTotalByteCount() const noexcept { return GetVertexByteCount() + GetIndexByteCount(); }

	// The geometry, read-only. Vertices in declaration order; indices as
	// triplets of MeshIndex, each referencing a vertex by position, wound in
	// whatever order the mesh was authored with.
	const std::vector<MeshVertex>& GetVertices() const noexcept { return m_vertices; }
	const std::vector<MeshIndex>& GetIndices() const noexcept { return m_indices; }

private:
	MeshAsset(
		PrimitiveTopology       topology,
		std::vector<MeshVertex> vertices,
		std::vector<MeshIndex>  indices)
		: m_topology(topology)
		, m_vertices(std::move(vertices))
		, m_indices(std::move(indices))
	{
	}

	PrimitiveTopology       m_topology = PrimitiveTopology::Unknown;
	std::vector<MeshVertex> m_vertices;
	std::vector<MeshIndex>  m_indices;
};

} // namespace Modern::Client
