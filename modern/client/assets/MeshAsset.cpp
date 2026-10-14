#include "assets/MeshAsset.h"

namespace Modern::Client
{
namespace
{

// The structural and per-element rules of a mesh, shared by both constructors.
// Returns the Status to propagate: Ok() means the vertices and indices are a
// valid triangle list and may be stored.
Status ValidateGeometry(
	PrimitiveTopology         topology,
	const std::vector<MeshVertex>& vertices,
	const std::vector<MeshIndex>&  indices)
{
	// Topology first: it decides what the rest of the rules mean. An unknown
	// or unassigned topology is refused here, before its primitive size is
	// used to divide, so no caller reaches a zero divisor.
	const uint32_t verticesPerPrimitive = VerticesPerPrimitive(topology);
	if (verticesPerPrimitive == 0)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// Zero of either array is not a mesh. Refusing it here is what keeps a
	// caller from building a MeshAsset and discovering later that it has no
	// geometry to draw.
	if (vertices.empty() || indices.empty())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// Whole primitives only: indexCount % 3 == 0 for TriangleList. A partial
	// triangle is not a smaller mesh, it is a malformed one, and every
	// consumer would have to decide independently how to draw it.
	if (indices.size() % verticesPerPrimitive != 0)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// Count ceilings, checked while the counts are still size_t and before
	// either is narrowed to uint32_t for the byte calculation below.
	if (vertices.size() > kMaxMeshVertices || indices.size() > kMaxMeshIndices)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// The byte total is the binding policy check: 8 MiB of CPU geometry per
	// mesh, and no product that this build's size_t cannot hold. The value it
	// returns is not needed here - the arrays already carry their own sizes -
	// but the decision it makes is.
	const Result<size_t> expected = ComputeMeshByteCount(
		MeshVertexFormat::PositionNormalUvF32,
		static_cast<uint32_t>(vertices.size()),
		static_cast<uint32_t>(indices.size()));
	if (expected.IsError())
	{
		return expected.GetStatus();
	}

	// Per-element rules, after the structural ones so a mesh that cannot be
	// valid is never walked. Finiteness first: NaN and infinity are refused
	// wherever they appear - position, normal or texture coordinate.
	for (const MeshVertex& vertex : vertices)
	{
		if (!vertex.IsFinite())
		{
			return Status(ErrorCode::InvalidArgument);
		}
	}

	// Then index bounds: every index must name a vertex that exists. Checked
	// against the vertex count rather than the largest legal index, so a mesh
	// with three vertices rejects 3 just as it rejects 4294967295.
	const size_t vertexCount = vertices.size();
	for (const MeshIndex index : indices)
	{
		if (static_cast<size_t>(index) >= vertexCount)
		{
			return Status(ErrorCode::InvalidArgument);
		}
	}

	return Ok();
}

} // namespace

Result<MeshAsset> MeshAsset::Create(
	PrimitiveTopology       topology,
	std::vector<MeshVertex> vertices,
	std::vector<MeshIndex>  indices)
{
	const Status valid = ValidateGeometry(topology, vertices, indices);
	if (valid.IsError())
	{
		return valid;
	}
	return MeshAsset(topology, std::move(vertices), std::move(indices), {}, {});
}

Result<MeshAsset> MeshAsset::CreateWithSections(
	PrimitiveTopology         topology,
	std::vector<MeshVertex>    vertices,
	std::vector<MeshIndex>     indices,
	std::vector<MeshMaterial>  materials,
	std::vector<MeshSection>   sections)
{
	// Geometry first, and by exactly the rules Create() applies, so a mesh with
	// materials is never held to a weaker standard than one without.
	const Status geometry = ValidateGeometry(topology, vertices, indices);
	if (geometry.IsError())
	{
		return geometry;
	}

	// Materials and sections describe each other: a section names a material,
	// so one without the other is a half-statement that could only be resolved
	// by a convention, and a convention is exactly what is being refused here.
	const bool haveMaterials = !materials.empty();
	const bool haveSections  = !sections.empty();
	if (haveMaterials != haveSections)
	{
		return Status(ErrorCode::InvalidArgument);
	}
	if (!haveSections)
	{
		return MeshAsset(topology, std::move(vertices), std::move(indices), {}, {});
	}

	// The index count is the unit every range is checked against, and it is
	// already known to be a whole number of triangles.
	const uint32_t verticesPerPrimitive = VerticesPerPrimitive(topology);
	const uint32_t totalIndices = static_cast<uint32_t>(indices.size());

	// The walk enforces order, non-overlap, whole triangles and exact coverage
	// in one pass, because "exactly" only has meaning if the ranges are in
	// order: `expectedOffset` is where this range must begin, and it starts at
	// zero and advances by each range's own length.
	uint32_t expectedOffset = 0;
	for (const MeshSection& section : sections)
	{
		if (section.materialIndex >= materials.size())
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (section.indexCount == 0 || section.indexCount % verticesPerPrimitive != 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (section.indexOffset != expectedOffset)
		{
			// Catches an overlap, a gap and an out-of-order list in one
			// comparison, which is why no separate ordering pass exists.
			return Status(ErrorCode::InvalidArgument);
		}
		// Formed in 64-bit so a range that would run past the end of a large
		// index array is refused rather than wrapping to a small value that
		// looks like a valid offset.
		const uint64_t end = static_cast<uint64_t>(section.indexOffset) +
		                     static_cast<uint64_t>(section.indexCount);
		if (end > totalIndices)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		expectedOffset = static_cast<uint32_t>(end);
	}

	// Coverage, not just contiguity: the last range has to end where the index
	// array ends, or the tail of the mesh would belong to no material.
	if (expectedOffset != totalIndices)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	return MeshAsset(topology, std::move(vertices), std::move(indices),
	                 std::move(materials), std::move(sections));
}

} // namespace Modern::Client