#include "assets/MeshAsset.h"

namespace Modern::Client
{

Result<MeshAsset> MeshAsset::Create(
	PrimitiveTopology       topology,
	std::vector<MeshVertex> vertices,
	std::vector<MeshIndex>  indices)
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

	return MeshAsset(topology, std::move(vertices), std::move(indices));
}

} // namespace Modern::Client