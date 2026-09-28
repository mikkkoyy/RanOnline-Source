#pragma once

#include "assets/MeshAsset.h"
#include "assets/MeshDecoder.h"
#include "resources/ResourceData.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>

namespace Modern::Client
{

// CLIENT-015: the RAN static `.X` mesh decoder.
//
// `xof 0303bin 0032` is DirectX .X version 03.03, binary encoding, 32-bit
// floats. The format is not a header plus a vertex block: it is a *tokenized*
// stream (little-endian WORD tokens, the encoding described by the DirectX .X
// "Binary Encoding" reference), and a mesh is three separate data objects -
// `Mesh` (positions and faces), `MeshNormals` (normals) and
// `MeshTextureCoords` (one UV set) - which are not required to be adjacent.
//
// This class parses that stream itself. No D3DXLoadMesh* call, no IDirect3D*
// object, no MFC, no legacy RAN header, and no renderer. It implements
// `IMeshDecoder` unchanged: `ResourceData` in, one validated `MeshAsset` out,
// failures returned as `ErrorCode::InvalidArgument`, nothing thrown.
//
// -----------------------------------------------------------------------
// What is decoded
// -----------------------------------------------------------------------
//
//   Mesh                nVertices, nFaces, positions, triangle faces
//   MeshNormals         one normal per vertex
//   MeshTextureCoords   one UV pair per vertex
//
// Every other template the RAN files contain is *parsed and skipped* - not
// ignored, because the token stream cannot be stepped over without walking its
// grammar. That includes the ones this milestone deliberately does not
// represent:
//
//   - `MeshMaterialList`, `Material`, `TextureFilename`
//       Parsed and discarded. Materials and textures are intentionally
//       deferred to a future milestone; CLIENT-015 does not expand MeshAsset to
//       carry them and the decoder's only output is geometry.
//   - `XSkinMeshHeader`, `SkinWeights`, `VertexDuplicationIndices`
//       Parsed and discarded. Skinning is deferred. `VertexDuplicationIndices`
//       was checked against the real assets and is *not* required for static
//       geometry: RAN's `Mesh` vertex list is already self-consistent, every
//       one of its indices is in range, and its `MeshNormals.faceNormals` list
//       is a verbatim copy of the face list, so vertex `v`'s normal is
//       `normals[v]`. VDI only relates the duplicated (skinned) vertex list to
//       the original one, which is a question about bones, not about the
//       static triangle soup. It is therefore read past and left unsupported.
//   - `Frame`, `FrameTransformMatrix`, `Matrix4x4`, `AnimationSet`,
//     `AnimationKey`, `Animation`
//       The bone hierarchy and animation are deferred. Frames are walked and
//       their matrices discarded; nothing about a bone reaches MeshAsset, and
//       no bone information is stripped silently - it is simply not part of a
//       static mesh.
//
// -----------------------------------------------------------------------
// How a mesh becomes one MeshAsset
// -----------------------------------------------------------------------
//
// MeshAsset holds a single vertex array, a single index array and
// `PrimitiveTopology::TriangleList`, so a file that carries several `Mesh`
// objects (43 of the shipped assets do, up to 116 sub-meshes in one file) is
// merged in document order: each sub-mesh appends its vertices, and its
// indices are biased by the running vertex base. Nothing is dropped and no
// sub-mesh boundary is invented - the alternative would be submeshes, which
// MeshAsset deliberately does not have.
//
// Refusals, all `InvalidArgument`, and each one a real observation about the
// shipped assets rather than a precaution:
//
//   - a header that is not exactly `xof 0303`, `bin `/`bzip` and `0032`
//     (12 text-encoded `.x` files in the shipped tree are `txt `, and are not
//     decoded by this milestone)
//   - a token stream that is truncated, malformed, or describes a member,
//     array or object the grammar does not allow
//   - a `Mesh` with no `MeshNormals` of the same length, or one whose normal
//     count differs from its vertex count. Normals are never invented: a zero
//     or absent normal is a wrong normal, and MeshAsset has no "no normals"
//     state to fall back to. (One shipped asset, `b_pet_human_ninefox.x`, is
//     refused for exactly this reason.)
//   - a `MeshTextureCoords` set that cannot be matched to its `Mesh`. A file
//     with no `MeshTextureCoords` at all decodes with `u = v = 0`; a file
//     that has some but not one per `Mesh`, or whose UV count differs from the
//     vertex count, is refused rather than paired by position, because RAN
//     emits those UV blocks out of step with the mesh blocks and pairing them
//     by index would attach UVs to the wrong vertices. (Four shipped assets -
//     `b_effet_char.x`, `b_m.x`, `b_m1.x`, `b_w.x` - are refused for this.)
//   - a face that is not a triangle, or an index that is not below
//     `nVertices`. All 13,292 faces in the shipped tree are triangles, so
//     polygon faces are refused instead of being triangulated by an invented
//     fan order.
//   - a non-finite position, normal or UV (NaN or either infinity)
//   - geometry above `kMaxMeshVertices`, `kMaxMeshIndices` or `kMaxMeshBytes`;
//     all size products are formed in 64-bit and refused rather than wrapped
//
// -----------------------------------------------------------------------
// The `bzip` encoding
// -----------------------------------------------------------------------
//
// `xof 0303bzip0032` is **not** bzip2. In DirectX .X it is MSZip, and RAN's
// writer lays it out as:
//
//   bytes 0..15   the same 16-byte file header
//   bytes 16..19  uint32 total uncompressed size, header included
//   bytes 20..23  reserved; not a CRC-32 of the payload and not validated
//   bytes 24..    one or more chunks, each:
//                   "CK"                                  2 bytes
//                   a raw DEFLATE stream (RFC 1951, no zlib wrapper)
//                   a 4-byte trailer, present only between chunks
//
// Every chunk after the first is inflated with the *previous chunk's
// decompressed bytes* as a preset dictionary, which is the defining behaviour
// of MSZip; 85 of the 86 shipped bzip files have one chunk and one
// (`b_aegis_wings.x`) has three. The deflate stream yields the token stream
// *without* the 16-byte header, so the header is prepended to restore a
// complete file.
//
// The RFC 1951 inflater is implemented privately in the decoder's translation
// unit, against the RFC, with no window, symbol or output-size limit taken from
// the host: a length or distance code that would read outside the declared
// output is a refusal, not a truncated read. It adds no dependency to the
// asset layer, which is what keeps this from becoming a zlib or a boost
// linkage in a target that currently has neither.
class XMeshDecoder final : public IMeshDecoder
{
public:
	Result<MeshAsset> DecodeMesh(const ResourceData& data) override;

	// The 16-byte file header, in bytes. Public because the format is public:
	// the tests build fixtures from these values instead of writing them out
	// longhand, and a reader comparing a decoder against its own writer proves
	// nothing.
	static constexpr size_t kHeaderSize = 16;

	// The only version, encoding pair and float width this decoder accepts.
	// The shipped tree contains exactly these three values across all 608
	// `.x` files, so anything else is refused rather than guessed at.
	static constexpr const char* kMagic       = "xof ";
	static constexpr const char* kVersion     = "0303";
	static constexpr const char* kBinaryTag   = "bin ";
	static constexpr const char* kBzipTag     = "bzip";
	static constexpr const char* kFloatSize32 = "0032";

	// Bytes that precede the first MSZip chunk in a `bzip` file: the 16-byte
	// header, a uint32 total size, and a reserved uint32.
	static constexpr size_t kMsZipPrefixSize = 24;

	// Ceiling on the inflated token stream of a `bzip` file, in bytes. Policy,
	// not a format limit: it is the same shape of bound as the mesh ceilings
	// and keeps a hostile or corrupt stream from being sized by its own
	// declared total. Far above the largest shipped asset (79,586 bytes).
	static constexpr size_t kMaxInflatedBytes = 64u * 1024u * 1024u;
};

} // namespace Modern::Client
