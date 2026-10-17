#include "WorldStaticMesh.h"

#include <cmath>
#include <cstring>

namespace Modern
{
	namespace Legacy
	{
		namespace
		{
			// ---------------------------------------------------------------------------
			// Serialized constants, from the legacy source
			// ---------------------------------------------------------------------------
			//
			// `CSerialFile::WriteFileType` writes m_szFileType (FILETYPESIZE bytes,
			// basestream.h:17) followed by m_FileID (a DWORD). `ReadFileType` reads
			// the same. So every RAN serialized container begins with this header,
			// which is why it is skipped rather than parsed.
			constexpr size_t kSerialFileHeaderBytes = 132;

			// A sentinel status meaning "a record this reader deliberately does not
			// model", carried separately from "a record that is corrupt".
			//
			// `NotFound` is chosen because every other failure in this reader is an
			// `InvalidArgument`, and no real path in a `.wld0` returns `NotFound`: it
			// means "nothing was wrong, but this is not mine to decode", so a stray
			// match cannot be mistaken for corruption.
			const Status kUnsupportedRecord(ErrorCode::NotFound);

			// DxStaticMesh.h:27. `DxStaticMesh::Load` dispatches on exactly this set.
			// Only the versions that read their buckets through
			// `SingleTexMesh_Load_Insert` -- that is, with no extra per-entry flag word
			// -- are decoded below; 0x0104 and 0x0105 route through
			// `SingleTexMesh_Load_Insert_104`, which is a different bucket layout and is
			// refused rather than guessed at.
			constexpr uint32_t kStaticMeshVer100 = 0x0100u;
			constexpr uint32_t kStaticMeshVer101 = 0x0101u;
			constexpr uint32_t kStaticMeshVer102 = 0x0102u;
			constexpr uint32_t kStaticMeshVer103 = 0x0103u;
			constexpr uint32_t kStaticMeshVerWld0 = 0x0200u;  // VERSION_WLD0

			// DxSingleTexMesh versions, from `DxSingleTexMesh::Load`.
			constexpr uint32_t kSingleTexVer100 = 0x0100u;
			constexpr uint32_t kSingleTexVer101 = 0x0101u;
			constexpr uint32_t kSingleTexVer102 = 0x0102u;  // DxSingleTexMesh::VERSION
			constexpr uint32_t kSingleTexVer103 = 0x0103u;
			constexpr uint32_t kSingleTexVer104 = 0x0104u;
			constexpr uint32_t kSingleTexVer105 = 0x0105u;
			constexpr uint32_t kSingleTexVer106 = 0x0106u;

			// NsOCTreeSaveLoad.cpp:718.
			constexpr uint32_t kAabbOctreeVersion = 0x0100u;

			// `OBJOCTree::DynamicLoad` (NsOCTreeSaveLoad.cpp:142) dispatches on one
			// word, and the legacy comment says why it is not simply a version:
			//
			//   dwVer <  0x10000  ->  OctreeLoadOLD, and dwVer *is* the leaf's FVF.
			//                         Those octrees have one fixed vertex format, so
			//                         they carry no version field at all.
			//   dwVer == 0x10000  ->  OctreeLoad_100
			//   dwVer == 0x10001  ->  OctreeLoad (OBJOCTree::m_dwVERSION)
			//
			// Real `.wld0` files contain all three.
			constexpr uint32_t kObjOctreeVersion    = 0x10001u;
			constexpr uint32_t kObjOctreeVersion100 = 0x10000u;
			constexpr uint32_t kObjOctreeOldMax     = 0x0FFFEu;

			// NsOCTree.h:32-34. Note the ordering: DAYNIGHT_ON is *zero*. Only
			// EMLR_DAYNIGHT_ON carries the extra per-vertex DIRECTPOINTCOLOR array after
			// the index buffer (NsOCTreeSaveLoad.cpp:311-314, and again at 481-488 for
			// the 0x10000 layout).
			constexpr uint32_t kLandTypeDayNightOn = 0u;

			constexpr size_t kVector3Bytes = 12u;
			constexpr size_t kDwordBytes = 4u;
			constexpr size_t kIndexBytes = 2u;

			// `DIRECTPOINTCOLOR` (NsOCTree.h:53) is eight WORDs -- alpha, three day
			// channels, three night channels and a temperature -- so it is 16 bytes per
			// vertex, not the 4 a BGRA vertex colour occupies. Both appear in the same
			// reader and confusing them desynchronises every record after it.
			constexpr size_t kDayNightColourBytes = 16u;

			// A vertex's own D3DFVF_DIFFUSE colour, one DWORD.
			constexpr size_t kVertexColourBytes = 4u;

			// `NSOCTREE::LoadOctree` is a pre-order walk over a binary tree and
			// `OBJOCTree::Save` writes one record per node, so a hostile file could
			// otherwise nest nodes without limit. The real trees are far shallower.
			constexpr size_t kMaxTreeDepth = 128u;

			class BoundedCursor
			{
			public:
				BoundedCursor() noexcept = default;

				BoundedCursor(const uint8_t* data, size_t size) noexcept
					: m_data(data), m_size(size)
				{
				}

				size_t Offset() const noexcept { return m_offset; }
				size_t Size() const noexcept { return m_size; }
				size_t Remaining() const noexcept { return m_size >= m_offset ? m_size - m_offset : 0u; }

				bool ReadDword(uint32_t& value) noexcept
				{
					if (Remaining() < kDwordBytes) { return false; }
					std::memcpy(&value, m_data + m_offset, kDwordBytes);
					m_offset += kDwordBytes;
					return true;
				}

				bool ReadFloat(float& value) noexcept
				{
					if (Remaining() < sizeof(float)) { return false; }
					std::memcpy(&value, m_data + m_offset, sizeof(float));
					m_offset += sizeof(float);
					return true;
				}

				bool ReadVector3(float out[3]) noexcept
				{
					return ReadFloat(out[0]) && ReadFloat(out[1]) && ReadFloat(out[2]);
				}

				// A length-prefixed byte string, as `CSerialFile` writes it.
				bool ReadString(std::string& out)
				{
					uint32_t length = 0;
					if (!ReadDword(length)) { return false; }
					if (length > Remaining()) { return false; }
					out.assign(reinterpret_cast<const char*>(m_data + m_offset), length);
					m_offset += length;
					return true;
				}

				bool ReadBytes(void* destination, size_t count) noexcept
				{
					if (count > Remaining()) { return false; }
					if (count != 0u) { std::memcpy(destination, m_data + m_offset, count); }
					m_offset += count;
					return true;
				}

				bool Skip(size_t count) noexcept
				{
					if (count > Remaining()) { return false; }
					m_offset += count;
					return true;
				}

				// Open the next `declaredBytes` as an independently bounded
				// sub-cursor, and leave this cursor positioned at the far side of it.
				//
				// This is the whole point of a serialized block size.
				// `CSerialFile::EndBlock` writes
				//
				//     dwTotalSize = (endPosition - blockStart) - sizeof(DWORD)
				//
				// so the declared count is exactly the bytes between the size DWORD and
				// the matching EndBlock: it excludes the size field itself, excludes any
				// header written before the block, and excludes anything written *after*
				// EndBlock. Making the block a sub-cursor rather than a pair of offset
				// comparisons means an inner reader physically cannot cross the
				// boundary, whatever it is fed.
				bool OpenBlock(uint32_t declaredBytes, BoundedCursor& block) noexcept
				{
					if (declaredBytes > Remaining()) { return false; }
					block = BoundedCursor(m_data + m_offset, declaredBytes);
					m_offset += declaredBytes;
					return true;
				}

			private:
				const uint8_t* m_data = nullptr;
				size_t         m_size = 0u;
				size_t         m_offset = 0u;
			};

			bool IsFinite(float value) noexcept
			{
				return std::isfinite(value);
			}

			bool IsFiniteVector3(const float values[3]) noexcept
			{
				return IsFinite(values[0]) && IsFinite(values[1]) && IsFinite(values[2]);
			}

			// The shape of one `DxSingleTexMesh` record, per version.
			//
			// `DxSingleTexMesh::Load` (NsOCTreeSaveLoad.cpp:833-1067) is not uniform
			// here, and the detail that matters most is that the texture name comes
			// FIRST for every version except 0x0101:
			//
			//   0x0106  szTexName, m_dwFlag, dwFlags,               dwSize, effects
			//   0x0105  szTexName, m_dwFlag, bCamColl, bDontCastShadowLM, dwSize, effects
			//   0x0104  szTexName, m_dwFlag, b,                     dwSize, effects
			//   0x0103  szTexName, m_dwFlag, b,                     dwSize, effects
			//   0x0102  szTexName, m_dwFlag,                       dwSize, effects  (VERSION)
			//   0x0101  m_dwFlag,   szTexName,                     dwSize, effects  <-- flag first
			//   0x0100  szTexName
			//
			// So the flags split in two, and 0x0100 has no effect count either. Assuming
			// one fixed flag word in front of the name works for 0x0101 alone and
			// misreads every other version -- the string length then comes out of the
			// middle of the name and the whole file desynchronises from that record on.
			struct SingleTexShape
			{
				bool     hasTextureName   = false;
				bool     hasEffectCount   = true;
				uint32_t flagsBeforeName = 0u;
				uint32_t flagsAfterName  = 0u;
			};

			SingleTexShape ShapeForSingleTexVersion(uint32_t version) noexcept
			{
				switch (version)
				{
				case kSingleTexVer100: return SingleTexShape{ true,  false, 0u, 0u };
				case kSingleTexVer101: return SingleTexShape{ true,  true,  1u, 0u };
				case kSingleTexVer102: return SingleTexShape{ true,  true,  0u, 1u };
				case kSingleTexVer103: return SingleTexShape{ true,  true,  0u, 2u };
				case kSingleTexVer104: return SingleTexShape{ true,  true,  0u, 2u };
				case kSingleTexVer105: return SingleTexShape{ true,  true,  0u, 3u };
				case kSingleTexVer106: return SingleTexShape{ true,  true,  0u, 2u };
				default:               return SingleTexShape{ false, false, 0u, 0u };
				}
			}

			// 0x0104, 0x0105 and 0x0106 read a light-map type word after the octree
			// exists flag and then RETURN, so those records end there. 0x0100 and 0x0101
			// never read the word at all.
			bool SingleTexHasLightMapType(uint32_t version) noexcept
			{
				return version == kSingleTexVer104 ||
				       version == kSingleTexVer105 ||
				       version == kSingleTexVer106;
			}

			// The decoded form of one supported vertex layout.
			struct VertexLayout
			{
				size_t   stride     = 0u;
				bool     hasNormals = false;
				bool     hasUvs     = false;
				bool     hasColors  = false;
			};

			// The vertex stride a supported FVF produces, or stride 0 when the layout
			// is not modelled.
			//
			// The FVF is matched as a WHOLE value, and each stride below is the one
			// `D3DXGetFVFVertexSize` returns for exactly that value:
			//
			//     2 + (XYZ ? 10 : 0) + (NORMAL ? 12 : 0) + (DIFFUSE ? 4 : 0)
			//       + (SPECULAR ? 4 : 0) + (PSIZE ? 4 : 0)
			//       + 8 * ((FVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT)
			//
			// with D3DFVF_TEXCOUNT_MASK = 0x0F00 and D3DFVF_TEXCOUNT_SHIFT = 8.
			//
			// The texture count lives in bits 8-11, NOT in the low nibble. Bits 0-3 hold
			// the position form -- D3DFVF_XYZ is 0x0002 -- so masking the low nibble as a
			// coordinate count is the classic way to get this wrong: it turns 0x152
			// into "two texture sets", doubles the stride to 44, and misreads every vertex
			// in the file while still looking plausible.
			//
			// The layout real terrain uses was measured from the bytes rather than
			// assumed from the formula:
			//
			//   0x152  XYZ|NORMAL|DIFFUSE|TEX1  12 + 12 + 4 + 8  = 36
			//        Every OBJOCTree leaf in `data/map/*.wld0` declares this. In
			//        `cha2_select.wld0` the leaf's 520-byte block resolves exactly as
			//        24 header + 6 * 36 + 4 * 3 * 2 + a 252-byte collision tree, which is
			//        what fixes the stride at 36 rather than 28 or 44 -- and the vertices
			//        decode as position, normal, BGRA, one UV.
			//   0x112  XYZ|NORMAL|TEX1          12 + 12 + 8     = 32
			//        The encoding that actually matches `OCTREEVERTEX`
			//        (DxCustomTypes.h:10): position, normal, two UVs, 32 bytes.
			//   0x012  XYZ|NORMAL              12 + 12        = 24
			//   0x002  XYZ                      12            = 12
			//
			// Note on the legacy constant: `OCTREEVERTEX::FVF` is *written*
			// `D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1`, which evaluates to 0x012 -- a value
			// carrying no texture-count bits at all, i.e. a 24-byte vertex that
			// contradicts the 32-byte struct it labels. That constant is used for
			// collision maps and effect meshes (NsCollisionMap.h:112,
			// DxCustomTypes.cpp:9), not for terrain, so this reader keys the octree vertex
			// to 0x112 and does not reproduce the mismatch.
			//
			// Every other value, including the RHW forms and any layout with more than
			// one texture set, is refused rather than reinterpreted: the leaf is counted
			// as unsupported and its bytes are stepped over.
			// The complete set of position layouts this reader models: every
			// combination of D3DFVF_NORMAL, D3DFVF_DIFFUSE and exactly one texture
			// coordinate set on a D3DFVF_XYZ position. That is eight whole FVF values,
			// each with the exact `D3DXGetFVFVertexSize` stride and an exact statement of
			// which components the vertex carries.
			//
			// Modelling the family rather than a handful of observed values means a new
			// field that drops the normal (0x142, XYZ|DIFFUSE|TEX1 -- very common in
			// `data/map`) decodes instead of being refused, and it still leaves the
			// genuinely unmodellable cases -- multiple texture sets, RHW, non-XYZ
			// positions -- explicitly refused rather than guessed at.
			VertexLayout LayoutForFvf(uint32_t fvf) noexcept
			{
				switch (fvf)
				{
				// XYZ | NORMAL | DIFFUSE | TEX1 -- the layout most ASURA terrain uses.
				case 0x152u: return VertexLayout{ 36u, true,  true,  true };
				// XYZ | NORMAL | TEX1 -- the 32-byte OCTREEVERTEX.
				case 0x112u: return VertexLayout{ 32u, true,  true,  false };
				// XYZ | DIFFUSE | TEX1.
				case 0x142u: return VertexLayout{ 24u, false, true,  true };
				// XYZ | NORMAL | DIFFUSE.
				case 0x052u: return VertexLayout{ 28u, true,  false, true };
				// XYZ | TEX1.
				case 0x102u: return VertexLayout{ 20u, false, true,  false };
				// XYZ | NORMAL.
				case 0x012u: return VertexLayout{ 24u, true,  false, false };
				// XYZ | DIFFUSE.
				case 0x042u: return VertexLayout{ 16u, false, false, true };
				// XYZ.
				case 0x002u: return VertexLayout{ 12u, false, false, false };
				default:     return VertexLayout{};
				}
			}

			// `DxAABBNode::LoadFile` (Collision.cpp:1280): six bounds floats, a face
			// count, then a left and a right child, each preceded by its own exists flag.
			//
			// No finiteness test is applied to the bounds. `DxAABBNode` initialises
			// `dwFace` to AABB_NONINDEX (0xFFFFFFFF, Collision.h:17) and leaves the bounds
			// of an unindexed node unset, so real files carry NaN in these six floats.
			// This reader does not model collision data at all -- it only has to walk the
			// tree -- so imposing a rule the format does not have would refuse valid
			// terrain. The enclosing OBJOCTree block is what actually bounds this walk.
			Status ReadCollisionNode(BoundedCursor& cursor, size_t depth)
			{
				if (depth > kMaxTreeDepth) { return Status(ErrorCode::InvalidArgument); }

				float bounds[6];
				for (int i = 0; i < 6; ++i)
				{
					if (!cursor.ReadFloat(bounds[i])) { return Status(ErrorCode::InvalidArgument); }
				}

				uint32_t faceCount = 0;
				if (!cursor.ReadDword(faceCount)) { return Status(ErrorCode::InvalidArgument); }

				for (int child = 0; child < 2; ++child)
				{
					uint32_t exists = 0;
					if (!cursor.ReadDword(exists)) { return Status(ErrorCode::InvalidArgument); }
					if (exists == 0u) { continue; }
					const Status nested = ReadCollisionNode(cursor, depth + 1u);
					if (nested.IsError()) { return nested; }
				}
				return Status();
			}

			// Decodes one OBJOCTree leaf payload. `withLandType` distinguishes the three
			// layouts `OBJOCTree::DynamicLoad` can dispatch to:
			//
			//   OctreeLoad (0x10001)  FVF, vert, face, landType, VB, IB, [colours]
			//   OctreeLoad_100        FVF, vert, face, landType, VB, IB, [colours]
			//   OctreeLoadOLD         FVF, vert, face, VB, IB
			//
			// 0x10000 and 0x10001 serialise identically: both read four DWORDs then a
			// vertex buffer at `D3DXGetFVFVertexSize(m_dwFVF)`, an index buffer of
			// `face * 3` WORDs, and -- for EMLR_DAYNIGHT_ON only -- the DIRECTPOINTCOLOR
			// array. They differ only in how the vertices are re-interpreted in RAM, which
			// is a rendering concern rather than a serialization one. The OLD layout has no
			// land type, so it carries no colour array either.
			Status DecodeLeafGeometry(BoundedCursor& cursor,
			                          uint32_t fvf,
			                          bool withLandType,
			                          WorldTerrainChunk& chunk,
			                          WorldStaticMeshAsset& asset)
			{
				uint32_t vertexCount = 0;
				uint32_t faceCount   = 0;
				uint32_t landType    = 0;
				if (!cursor.ReadDword(vertexCount)) { return Status(ErrorCode::InvalidArgument); }
				if (!cursor.ReadDword(faceCount))   { return Status(ErrorCode::InvalidArgument); }
				if (withLandType)
				{
					if (!cursor.ReadDword(landType)) { return Status(ErrorCode::InvalidArgument); }
				}

				const VertexLayout layout = LayoutForFvf(fvf);
				if (layout.stride == 0u)
				{
					// An FVF this reader does not model is counted and refused. The caller
					// steps over the rest of the block, so the stream stays in sync
					// without the stride ever being guessed at.
					asset.NoteUnsupportedFvf(fvf);
					return kUnsupportedRecord;
				}

				// A leaf with no vertices or no faces is recognised but carries no
				// geometry; it is counted rather than decoded into empty vectors that
				// would look like a successful empty chunk.
				if (vertexCount == 0u || faceCount == 0u) { return kUnsupportedRecord; }

				// Counts are checked against the reader ceilings before any allocation is
				// attempted.
				if (vertexCount > WorldStaticMeshReader::kMaxLeafVertices ||
				    faceCount > WorldStaticMeshReader::kMaxLeafIndices / 3u)
				{
					return Status(ErrorCode::InvalidArgument);
				}

				chunk.fvf        = fvf;
				chunk.hasNormals = layout.hasNormals;
				chunk.hasUvs     = layout.hasUvs;
				chunk.hasColors  = layout.hasColors;

				const size_t total = static_cast<size_t>(vertexCount);
				chunk.positions.assign(total * 3u, 0.0f);
				if (layout.hasNormals) { chunk.normals.assign(total * 3u, 0.0f); }
				if (layout.hasUvs)     { chunk.uvs.assign(total * 2u, 0.0f); }
				if (layout.hasColors)  { chunk.colors.assign(total, 0u); }

				// The vertex buffer is a single contiguous run of `stride` bytes per
				// vertex, read through the enclosing block cursor, so it can never exceed
				// what the block left.
				std::vector<uint8_t> raw(total * layout.stride);
				if (!cursor.ReadBytes(raw.data(), raw.size()))
				{
					return Status(ErrorCode::InvalidArgument);
				}

				for (size_t v = 0; v < total; ++v)
				{
					const uint8_t* vertex = raw.data() + v * layout.stride;

					float position[3];
					std::memcpy(position, vertex + 0u, kVector3Bytes);
					if (!IsFiniteVector3(position)) { return Status(ErrorCode::InvalidArgument); }
					chunk.positions[v * 3u + 0u] = position[0];
					chunk.positions[v * 3u + 1u] = position[1];
					chunk.positions[v * 3u + 2u] = position[2];

					size_t offset = kVector3Bytes;
					if (layout.hasNormals)
					{
						float normal[3];
						std::memcpy(normal, vertex + offset, kVector3Bytes);
						if (!IsFiniteVector3(normal)) { return Status(ErrorCode::InvalidArgument); }
						chunk.normals[v * 3u + 0u] = normal[0];
						chunk.normals[v * 3u + 1u] = normal[1];
						chunk.normals[v * 3u + 2u] = normal[2];
						offset += kVector3Bytes;
					}
					if (layout.hasColors)
					{
						uint32_t colour = 0;
						std::memcpy(&colour, vertex + offset, kVertexColourBytes);
						chunk.colors[v] = colour;
						offset += kVertexColourBytes;
					}
					if (layout.hasUvs)
					{
						float uv[2];
						std::memcpy(uv, vertex + offset, sizeof(uv));
						if (!IsFinite(uv[0]) || !IsFinite(uv[1]))
						{
							return Status(ErrorCode::InvalidArgument);
						}
						chunk.uvs[v * 2u + 0u] = uv[0];
						chunk.uvs[v * 2u + 1u] = uv[1];
						offset += sizeof(uv);
					}
				}

				// The index buffer is 16-bit WORDs, three per triangle.
				const size_t indexTotal = static_cast<size_t>(faceCount) * 3u;
				chunk.indices.assign(indexTotal, 0u);
				std::vector<uint8_t> indices(indexTotal * kIndexBytes);
				if (!cursor.ReadBytes(indices.data(), indices.size()))
				{
					return Status(ErrorCode::InvalidArgument);
				}
				for (size_t i = 0; i < indexTotal; ++i)
				{
					uint16_t index = 0;
					std::memcpy(&index, indices.data() + i * kIndexBytes, kIndexBytes);
					// An index past the vertex list would read memory the buffer does not
					// own once this is uploaded.
					if (index >= vertexCount) { return Status(ErrorCode::InvalidArgument); }
					chunk.indices[i] = index;
				}

				// Only EMLR_DAYNIGHT_ON carries the extra per-vertex colour array after
				// the indices. EMLR_DAYNIGHT_OFF and EMLR_PIECE do not, and treating them
				// as if they did would desynchronise the stream.
				if (withLandType && landType == kLandTypeDayNightOn)
				{
					if (!cursor.Skip(total * kDayNightColourBytes))
					{
						return Status(ErrorCode::InvalidArgument);
					}
				}

				return Status();
			}

			// `OBJOCTree::Load` (NsOCTreeSaveLoad.cpp:98) followed by the node walk of
			// `NSOCTREE::LoadOctree`.
			//
			// Per `OBJOCTree::Save`, this record is:
			//
			//     vec3 vMax; vec3 vMin;
			//     BLOCK { geomExists, [geometry], collExists, [collision] }
			//     <left child>  <right child>          <-- OUTSIDE the block
			//
			// so the block holds one node's payload and the children are siblings in the
			// enclosing tree stream. Nesting the children inside the block would make every
			// real leaf look over-long.
			//
			// `sawUnsupported` is an out-parameter rather than a sentinel status on
			// purpose: a leaf this reader cannot decode still has two child records sitting
			// on the stream right after it, so the walk must go on and consume them.
			// Returning early would leave the caller desynchronised.
			Status ReadObjOctree(BoundedCursor& cursor,
			                     WorldTerrainChunk& chunk,
			                     WorldStaticMeshAsset& asset,
			                     bool& sawUnsupported,
			                     size_t depth)
			{
				if (depth > kMaxTreeDepth) { return Status(ErrorCode::InvalidArgument); }

				// `NSOCTREE::LoadOctree` writes the node's exists flag first, then the node
				// record, then the left and right subtrees.
				uint32_t exists = 0;
				if (!cursor.ReadDword(exists)) { return Status(ErrorCode::InvalidArgument); }
				if (exists == 0u) { return Status(); }

				if (!cursor.ReadVector3(chunk.boundsMax) || !cursor.ReadVector3(chunk.boundsMin))
				{
					return Status(ErrorCode::InvalidArgument);
				}
				if (!IsFiniteVector3(chunk.boundsMax) || !IsFiniteVector3(chunk.boundsMin))
				{
					return Status(ErrorCode::InvalidArgument);
				}

				uint32_t bufferSize = 0;
				if (!cursor.ReadDword(bufferSize)) { return Status(ErrorCode::InvalidArgument); }

				// `OpenBlock` leaves `cursor` at the far side of the block, so the children
				// below are read from the right place whether or not this node's payload
				// turned out to be decodable.
				BoundedCursor block;
				if (!cursor.OpenBlock(bufferSize, block)) { return Status(ErrorCode::InvalidArgument); }

				// `OBJOCTree::DynamicLoad`: geometry, then the collision tree.
				bool     geometryDecoded = true;
				uint32_t geometryExists = 0;
				if (!block.ReadDword(geometryExists)) { return Status(ErrorCode::InvalidArgument); }

				if (geometryExists != 0u)
				{
					uint32_t version = 0;
					if (!block.ReadDword(version)) { return Status(ErrorCode::InvalidArgument); }

					// The dispatch word is a version for 0x10000 and 0x10001 and an FVF for
					// anything below 0x10000.
					bool     withLandType = false;
					uint32_t fvf = version;
					if (version == kObjOctreeVersion || version == kObjOctreeVersion100)
					{
						withLandType = true;
						if (!block.ReadDword(fvf)) { return Status(ErrorCode::InvalidArgument); }
					}
					else if (version > kObjOctreeOldMax)
					{
						// `OctreeLoad_100` is handled above because it serialises the same
						// bytes, so anything left here is a version the client has never
						// seen. `OBJOCTree::DynamicLoad` reports it and abandons the node;
						// refusing is the honest equivalent, since where the record ends
						// inside the block is unknowable.
						return Status(ErrorCode::InvalidArgument);
					}

					const Status geometry = DecodeLeafGeometry(block, fvf, withLandType, chunk, asset);
					// `kUnsupportedRecord` is a distinguished *error* code, so it has to be
					// recognised before the generic failure check: a leaf this reader simply
					// does not model must not abort the whole file.
					if (geometry == kUnsupportedRecord) { geometryDecoded = false; }
					else if (geometry.IsError())          { return geometry; }

					if (!geometryDecoded)
					{
						// The collision word cannot be located: an unmodelled FVF means the
						// vertex and index buffers are an unknown length. Stepping to the end
						// of the block is the only safe move, and the children below still
						// follow, so the stream stays aligned.
						sawUnsupported = true;
						const Status child =
						    ReadObjOctree(cursor, chunk, asset, sawUnsupported, depth + 1u);
						if (child.IsError()) { return child; }
						return ReadObjOctree(cursor, chunk, asset, sawUnsupported, depth + 1u);
					}
				}

				uint32_t collisionExists = 0;
				if (!block.ReadDword(collisionExists)) { return Status(ErrorCode::InvalidArgument); }
				if (collisionExists != 0u)
				{
					const Status collision = ReadCollisionNode(block, depth);
					if (collision.IsError()) { return collision; }
				}

				// Whatever is left inside the block belongs to a layout this reader does not
				// model. The block cursor is discarded here, which steps over it.
				// Pre-order: this node, then its left subtree, then its right.
				const Status child = ReadObjOctree(cursor, chunk, asset, sawUnsupported, depth + 1u);
				if (child.IsError()) { return child; }
				return ReadObjOctree(cursor, chunk, asset, sawUnsupported, depth + 1u);
			}

			// `DxAABBOctree::Load` (NsOCTreeSaveLoad.cpp:737):
			//
			//     u32 version; BLOCK { vMax, vMin, exists, NSOCTREE }
			//
			// This is the one block that *does* enclose the node tree.
			Status ReadAabbOctree(BoundedCursor& cursor,
			                      WorldTerrainChunk& chunk,
			                      WorldStaticMeshAsset& asset,
			                      bool& sawUnsupported,
			                      size_t depth)
			{
				if (depth > kMaxTreeDepth) { return Status(ErrorCode::InvalidArgument); }

				uint32_t version = 0;
				if (!cursor.ReadDword(version)) { return Status(ErrorCode::InvalidArgument); }
				if (version != kAabbOctreeVersion)
				{
					// `DxAABBOctree::Load` seeks past the block instead, which is safe
					// because the size is there. Refusing is stricter than the legacy reader
					// and fails closed on a version whose contents cannot be interpreted at
					// all.
					return Status(ErrorCode::InvalidArgument);
				}

				uint32_t bufferSize = 0;
				if (!cursor.ReadDword(bufferSize)) { return Status(ErrorCode::InvalidArgument); }

				BoundedCursor block;
				if (!cursor.OpenBlock(bufferSize, block)) { return Status(ErrorCode::InvalidArgument); }

				if (!block.ReadVector3(chunk.boundsMax) || !block.ReadVector3(chunk.boundsMin))
				{
					return Status(ErrorCode::InvalidArgument);
				}
				if (!IsFiniteVector3(chunk.boundsMax) || !IsFiniteVector3(chunk.boundsMin))
				{
					return Status(ErrorCode::InvalidArgument);
				}

				uint32_t exists = 0;
				if (!block.ReadDword(exists)) { return Status(ErrorCode::InvalidArgument); }
				if (exists == 0u) { return Status(); }

				return ReadObjOctree(block, chunk, asset, sawUnsupported, depth + 1u);
			}

			// The text-effect records inside a single-texture mesh's block.
			//
			// Every `DxTexEff*::LoadPSF` begins identically -- a version DWORD, a size
			// DWORD, then that many bytes of payload -- and each ends by seeking to
			// `current + size` when it does not recognise the version. So one shape covers
			// every effect type without this reader needing to know what any of them mean:
			//
			//     u32 typeId; u32 version; BLOCK { payload }
			Status SkipTextEffects(BoundedCursor& cursor, WorldStaticMeshAsset& asset)
			{
				uint32_t count = 0;
				if (!cursor.ReadDword(count))
				{
					return Status(ErrorCode::InvalidArgument);
				}

				for (uint32_t i = 0; i < count; ++i)
				{
					uint32_t typeId = 0;
					uint32_t effectVersion = 0;
					uint32_t payloadSize = 0;
					if (!cursor.ReadDword(typeId))      { return Status(ErrorCode::InvalidArgument); }
					if (!cursor.ReadDword(effectVersion)) { return Status(ErrorCode::InvalidArgument); }
					if (!cursor.ReadDword(payloadSize))  { return Status(ErrorCode::InvalidArgument); }

					BoundedCursor payload;
					if (!cursor.OpenBlock(payloadSize, payload))
					{
						return Status(ErrorCode::InvalidArgument);
					}
					++asset.skippedRecord;
				}
				return Status();
			}

			// `DxSingleTexMesh::Save` / `Load` (NsOCTreeSaveLoad.cpp:788, 823):
			//
			//     u32 version;
			//     BLOCK { flag[, extra flags], texName, [effCount, effects] }
			//     u32 octreeExists;            <-- OUTSIDE the block
			//     [lightmapType] [octree]
			//
			// The block ends before the octree. Fencing the octree as if it were inside is
			// the mistake that made every conforming record look over-long.
			Status ReadSingleTexMesh(BoundedCursor& cursor,
			                         WorldTerrainBucket& out,
			                         WorldStaticMeshAsset& asset,
			                         size_t depth)
			{
				if (depth > kMaxTreeDepth) { return Status(ErrorCode::InvalidArgument); }

				uint32_t version = 0;
				if (!cursor.ReadDword(version)) { return Status(ErrorCode::InvalidArgument); }
				uint32_t bufferSize = 0;
				if (!cursor.ReadDword(bufferSize)) { return Status(ErrorCode::InvalidArgument); }

				BoundedCursor block;
				// A declared size the file cannot back is a corrupt record, not a large one.
				if (!cursor.OpenBlock(bufferSize, block)) { return Status(ErrorCode::InvalidArgument); }

				const SingleTexShape shape = ShapeForSingleTexVersion(version);
				if (!shape.hasTextureName)
				{
					// `DxSingleTexMesh::Load`'s fallback: seek past the block, then carry on
					// to the octree, which is what every supported version does after its
					// own block.
					uint32_t octreeExists = 0;
					if (!cursor.ReadDword(octreeExists)) { return Status(ErrorCode::InvalidArgument); }
					if (octreeExists != 0u)
					{
						WorldTerrainChunk chunk;
						bool sawUnsupported = false;
						const Status octree =
						    ReadAabbOctree(cursor, chunk, asset, sawUnsupported, depth + 1u);
						if (octree.IsError()) { return octree; }
						if (sawUnsupported) { ++asset.skippedRecord; return Status(); }
						out.chunks.push_back(std::move(chunk));
					}
					++asset.skippedRecord;
					return Status();
				}

				for (uint32_t i = 0; i < shape.flagsBeforeName; ++i)
				{
					uint32_t flag = 0;
					if (!block.ReadDword(flag)) { return Status(ErrorCode::InvalidArgument); }
				}

				std::string textureName;
				if (!block.ReadString(textureName)) { return Status(ErrorCode::InvalidArgument); }

				for (uint32_t i = 0; i < shape.flagsAfterName; ++i)
				{
					uint32_t flag = 0;
					if (!block.ReadDword(flag)) { return Status(ErrorCode::InvalidArgument); }
				}

				if (shape.hasEffectCount)
				{
					const Status effs = SkipTextEffects(block, asset);
					if (effs.IsError()) { return effs; }
				}

				// Past the block: the octree, and for 0x0104+ a light-map type.
				uint32_t octreeExists = 0;
				if (!cursor.ReadDword(octreeExists)) { return Status(ErrorCode::InvalidArgument); }

				WorldTerrainChunk chunk;
				chunk.textureName = textureName;
				bool sawUnsupported = false;

				if (octreeExists != 0u)
				{
					if (SingleTexHasLightMapType(version))
					{
						uint32_t lightMapType = 0;
						if (!cursor.ReadDword(lightMapType))
						{
							return Status(ErrorCode::InvalidArgument);
						}
						if (lightMapType != 0u)
						{
							// Type 1 is the "new DxAABBOctree with light maps", which the
							// client has never implemented: a version and a size, then a seek
							// past it. Nothing is decoded, and the record is counted as
							// skipped.
							uint32_t lmVersion = 0;
							uint32_t lmSize = 0;
							if (!cursor.ReadDword(lmVersion)) { return Status(ErrorCode::InvalidArgument); }
							if (!cursor.ReadDword(lmSize))     { return Status(ErrorCode::InvalidArgument); }
							if (!cursor.Skip(lmSize))          { return Status(ErrorCode::InvalidArgument); }
							++asset.skippedRecord;
							return Status();
						}
					}

					const Status octree =
					    ReadAabbOctree(cursor, chunk, asset, sawUnsupported, depth + 1u);
					if (octree.IsError()) { return octree; }
				}

				if (sawUnsupported)
				{
					// The octree was walked to its end so the stream stays aligned, but at
					// least one leaf in it is not a layout this reader models. The record
					// is reported as skipped rather than half-decoded.
					++asset.skippedRecord;
					return Status();
				}

				out.chunks.push_back(std::move(chunk));
				return Status();
			}

			// `DxStaticMesh::SingleTexMesh_Load_Insert` (DxStaticMesh.cpp:970): a count,
			// then that many (name, DxSingleTexMesh) pairs. There is no block size here,
			// so the only bound is the enclosing file.
			Status ReadBucket(BoundedCursor& cursor,
			                  std::vector<WorldTerrainBucket>& out,
			                  WorldStaticMeshAsset& asset)
			{
				uint32_t count = 0;
				if (!cursor.ReadDword(count)) { return Status(ErrorCode::InvalidArgument); }
				if (count > 1024u * 1024u) { return Status(ErrorCode::InvalidArgument); }

				for (uint32_t i = 0; i < count; ++i)
				{
					WorldTerrainBucket bucket;
					if (!cursor.ReadString(bucket.name)) { return Status(ErrorCode::InvalidArgument); }

					const Status mesh = ReadSingleTexMesh(cursor, bucket, asset, 0u);
					// A record this reader cannot model is skipped, not fatal: the file may
					// hold many supported ones after it. The skip is recorded on `asset`, and
					// the stream has already been advanced past the whole record by the time
					// we get here.
					if (mesh.IsError()) { return mesh; }
					out.push_back(std::move(bucket));
				}
				return Status();
			}

		} // namespace

		Result<WorldStaticMeshAsset> WorldStaticMeshReader::Decode(
		    const std::vector<uint8_t>& bytes) const
		{
			if (bytes.empty() || bytes.size() > kMaxInputBytes)
			{
				return Status(ErrorCode::InvalidArgument);
			}

			BoundedCursor cursor(bytes.data(), bytes.size());
			WorldStaticMeshAsset asset;

			if (!cursor.Skip(kSerialFileHeaderBytes))
			{
				return Status(ErrorCode::InvalidArgument);
			}

			// `DxStaticMesh::Load`: a version, a block size, then the tile bounds. The
			// block covers the bounds and nothing else -- `DxStaticMesh::Save` closes it
			// immediately after vMax/vMin and writes the five buckets afterwards -- so the
			// buckets are read against the file, not the block. Real `.wld0` files declare
			// exactly 24 here.
			uint32_t version = 0;
			if (!cursor.ReadDword(version)) { return Status(ErrorCode::InvalidArgument); }
			if (version != kStaticMeshVer100 && version != kStaticMeshVer101 &&
			    version != kStaticMeshVer102 && version != kStaticMeshVer103 &&
			    version != kStaticMeshVerWld0)
			{
				// 0x0104 and 0x0105 dispatch to `Load_104`, whose buckets carry an extra
				// per-entry flag word this reader does not model. Refusing is deliberate:
				// their top-level block does not fence the buckets, so there is no way to
				// seek past an unknown tail.
				return Status(ErrorCode::InvalidArgument);
			}

			uint32_t bufferSize = 0;
			if (!cursor.ReadDword(bufferSize)) { return Status(ErrorCode::InvalidArgument); }

			BoundedCursor boundsBlock;
			if (!cursor.OpenBlock(bufferSize, boundsBlock)) { return Status(ErrorCode::InvalidArgument); }

			if (!boundsBlock.ReadVector3(asset.tileBoundsMax) ||
			    !boundsBlock.ReadVector3(asset.tileBoundsMin))
			{
				return Status(ErrorCode::InvalidArgument);
			}
			if (!IsFiniteVector3(asset.tileBoundsMax) || !IsFiniteVector3(asset.tileBoundsMin))
			{
				return Status(ErrorCode::InvalidArgument);
			}
			// The bounds block holds the bounds and nothing else. A file that declares more
			// is not the format this reader implements.
			if (boundsBlock.Remaining() != 0u) { return Status(ErrorCode::InvalidArgument); }

			// `DxStaticMesh::Load_200`: five buckets, in the order `SingleTexMesh_Save`
			// writes them -- opaque, soft-alpha, soft-alpha 0, soft-alpha 1, alpha.
			for (int bucket = 0; bucket < 5; ++bucket)
			{
				const Status result = ReadBucket(cursor, asset.buckets[bucket], asset);
				if (result.IsError()) { return result; }
			}

			return asset;
		}
	}
}