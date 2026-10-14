#pragma once

#include "assets/AssetTypes.h"
#include "math/Vector3.h"
#include "types/Result.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
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

// CLIENT-RENDER-002: one material of a mesh, as far as the CPU side needs it.
//
// A material here is *a name and a texture filename*. Nothing more, because the
// ASURA character proves nothing more is needed to render it: the real
// `m_luffy_timeskip.mxf` declares 21 `Material` objects whose entire varying
// content is a `TextureFilename` string, and the 17 materials its six
// sub-meshes actually reference all name a `.png` file -- just two distinct
// ones, `blackcatyb_luffy_timeskip.png` (sixteen materials) and
// `blackcatyb_luffy_timeskip_eye.png` (the eye). The ambient/power/diffuse
// float members RAN also writes are all zero and are deliberately not carried:
// a PBR or lighting material system is a different milestone, and inventing
// fields the asset does not have would be the opposite of modelling what the
// data says.
//
// Both strings are *data from the file*, not paths the asset layer resolved.
// `textureFilename` is exactly the text of a `.X` `TextureFilename` record,
// which for RAN is a bare filename with no directory component; turning that
// name into something on disk is the resource layer's job, not this one's.
// Neither string is used as a path anywhere below this header.
struct MeshMaterial
{
	// The material's instance name as written in the `.X` file, e.g.
	// "Material__757_Model001_Material002Sub0". A `MeshMaterialList` refers to
	// materials by this name, and it is what ties a section to its material
	// when a file declares the materials globally and references them later.
	// Empty when the source named the object only (a material with no instance
	// name), which is legal and is not an error.
	std::string name;

	// The `TextureFilename` string. Empty when the material carries no
	// texture reference, or carries a `TextureFilename` with no string in it:
	// both mean "this material has no texture", which a caller must handle
	// explicitly rather than treat as a file named "".
	std::string textureFilename;

	bool operator==(const MeshMaterial& other) const noexcept
	{
		return name == other.name && textureFilename == other.textureFilename;
	}

	bool operator!=(const MeshMaterial& other) const noexcept { return !(*this == other); }
};

// CLIENT-RENDER-002: a contiguous run of indices drawn with one material.
//
// DirectX `.X` describes material assignment per *face*, so the geometry of a
// character is only correct if each face is drawn with the material its
// `MeshMaterialList` named. `indexOffset`/`indexCount` address a run of
// `MeshAsset::GetIndices()` and `materialIndex` names the material for it.
//
// A range, rather than a per-face lookup table, because that is what the real
// asset needs and no more: in `m_luffy_timeskip.mxf` the per-face material
// indexes of all six sub-meshes are already grouped into contiguous runs
// (3, 3, 4, 5, 1 and 6 runs), so 22 ranges describe the whole character with
// no triangle reordering at all. A file whose face indexes interleave would
// produce one range per run there as well -- more ranges, same information,
// still no reordering. Reordering indices to sort them by material would have
// been cheaper and would have silently changed the geometry order the decoder
// has always preserved, so it is not done.
//
// The ranges are validated at construction: in order, non-overlapping, each a
// whole number of triangles, together covering every index exactly once. There
// is no partial coverage and no overlap to be discovered by a renderer later.
struct MeshSection
{
	uint32_t indexOffset = 0;   // first index in MeshAsset::GetIndices()
	uint32_t indexCount  = 0;   // multiple of 3
	uint32_t materialIndex = 0; // into MeshAsset::GetMaterials()

	bool operator==(const MeshSection& other) const noexcept
	{
		return indexOffset == other.indexOffset && indexCount == other.indexCount &&
		       materialIndex == other.materialIndex;
	}

	bool operator!=(const MeshSection& other) const noexcept { return !(*this == other); }
};

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

	// CLIENT-RENDER-002: builds a mesh that also carries its materials and the
	// index ranges drawn with each of them.
	//
	// The three-argument Create() above is exactly this with no materials and no
	// sections, and is kept as the whole of the untextured contract rather than
	// folded into an overload: an untextured mesh has exactly one section
	// covering every index and no materials, and saying that implicitly would
	// hand every existing caller a section list it never asked for.
	//
	// On top of every rule Create() enforces, this adds, all `InvalidArgument`:
	//
	//   - an empty materials list with a non-empty sections list, or a
	//     non-empty materials list with an empty sections list: the two are a
	//     statement about each other, so neither half alone is a mesh
	//   - a section whose materialIndex does not name a declared material
	//   - a section whose indexCount is zero or not a whole number of triangles
	//   - a section whose range runs past the end of the index array
	//   - sections that are not in ascending, non-overlapping order, or that
	//     leave a gap between them
	//   - sections that do not cover the index array exactly
	//
	// The last two together are what "cover exactly" means: the first section
	// must start at index 0, each must begin where the previous ended, and the
	// last must end at GetIndexCount(). A renderer can therefore draw every
	// section in order and have drawn the whole mesh, with no index left
	// silently untextured and no index drawn twice.
	static Result<MeshAsset> CreateWithSections(
		PrimitiveTopology          topology,
		std::vector<MeshVertex>    vertices,
		std::vector<MeshIndex>     indices,
		std::vector<MeshMaterial>  materials,
		std::vector<MeshSection>   sections);

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

	// CLIENT-RENDER-002: the material table and the index ranges. Both are
	// empty for a mesh built by the three-argument Create(), which is the
	// untextured case. Both are non-empty together or empty together.
	size_t GetMaterialCount() const noexcept { return m_materials.size(); }
	size_t GetSectionCount() const noexcept { return m_sections.size(); }

	const std::vector<MeshMaterial>& GetMaterials() const noexcept { return m_materials; }
	const std::vector<MeshSection>& GetSections() const noexcept { return m_sections; }

	// True when the mesh carries material information a caller can draw with.
	// Convenient rather than clever: it is `!m_sections.empty()`, spelled once
	// so no caller has to know that materials without sections cannot exist.
	bool HasSections() const noexcept { return !m_sections.empty(); }

private:
	MeshAsset(
		PrimitiveTopology       topology,
		std::vector<MeshVertex> vertices,
		std::vector<MeshIndex>  indices,
		std::vector<MeshMaterial> materials,
		std::vector<MeshSection>  sections)
		: m_topology(topology)
		, m_vertices(std::move(vertices))
		, m_indices(std::move(indices))
		, m_materials(std::move(materials))
		, m_sections(std::move(sections))
	{
	}

	PrimitiveTopology          m_topology = PrimitiveTopology::Unknown;
	std::vector<MeshVertex>    m_vertices;
	std::vector<MeshIndex>     m_indices;
	std::vector<MeshMaterial>  m_materials;
	std::vector<MeshSection>   m_sections;
};

} // namespace Modern::Client
