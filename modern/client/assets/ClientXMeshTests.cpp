// CLIENT-015: RAN `.X` static mesh decoder tests.
//
// The `.X` fixtures here are assembled by hand from the published DirectX .X
// binary token encoding -- the same description XMeshDecoder.h carries -- and
// never by calling the decoder, so a decoder checked against its own writer
// would prove nothing. The two compressed fixtures are DEFLATE streams produced
// by zlib, which is likewise independent of the decoder's own RFC 1951
// implementation.
//
// One case reads a real RAN installation, and only when RAN_ASSET_ROOT names
// one; it reports what it found and distinguishes decoded, unsupported, no
// static geometry and malformed rather than collapsing them into a pass.

#include "TestHarness.h"

#include "assets/AssetTypes.h"
#include "assets/MeshAsset.h"
#include "assets/MeshDecoder.h"
#include "assets/MxfMeshTransform.h"
#include "assets/XMeshDecoder.h"
#include "resources/ResourceData.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>
#include <vector>

using namespace Modern;
using namespace Modern::Client;

namespace
{

// ---------------------------------------------------------------------------
// Token vocabulary
// ---------------------------------------------------------------------------

constexpr uint16_t kName      = 1;
constexpr uint16_t kString    = 2;
constexpr uint16_t kInteger   = 3;
constexpr uint16_t kGuid      = 5;
constexpr uint16_t kIntList   = 6;
constexpr uint16_t kFloatList = 7;
constexpr uint16_t kOBrace    = 10;
constexpr uint16_t kCBrace    = 11;
constexpr uint16_t kOBracket  = 14;
constexpr uint16_t kCBracket  = 15;
constexpr uint16_t kDot       = 18;
constexpr uint16_t kSemicolon = 20;
constexpr uint16_t kTemplate  = 31;
constexpr uint16_t kDword     = 41;
constexpr uint16_t kArray     = 52;

// ---------------------------------------------------------------------------
// Builders
// ---------------------------------------------------------------------------

void PushU16(std::vector<uint8_t>& bytes, uint16_t value)
{
	bytes.push_back(static_cast<uint8_t>(value & 0xFFu));
	bytes.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
}

void PushU32(std::vector<uint8_t>& bytes, uint32_t value)
{
	bytes.push_back(static_cast<uint8_t>(value & 0xFFu));
	bytes.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
	bytes.push_back(static_cast<uint8_t>((value >> 16) & 0xFFu));
	bytes.push_back(static_cast<uint8_t>((value >> 24) & 0xFFu));
}

void PushF32(std::vector<uint8_t>& bytes, float value)
{
	uint32_t bits = 0;
	std::memcpy(&bits, &value, sizeof(bits));
	PushU32(bytes, bits);
}

// Appends raw bytes, one at a time.
//
// Every byte this file appends goes through a loop like this one rather than
// through `bytes.insert(bytes.end(), first, last)`. That is not a style
// preference. With a `const char*` range going into a `std::vector<uint8_t>`,
// overload resolution selects the range form of insert, and on the MSVC 14.44
// STL that form does not terminate: a six-line program consisting of a
// three-byte vector and one `insert(b.end(), "array", "array" + 5)` never
// returns. In the suite that manifested as a non-deterministic debug-heap
// assertion many tests away from the call, which is why it took a standalone
// driver to pin down. A loop has no overload to misresolve and no count to
// reinterpret.
void AppendBytes(std::vector<uint8_t>& bytes, const uint8_t* data, size_t length)
{
	for (size_t i = 0; i < length; ++i)
	{
		bytes.push_back(data[i]);
	}
}

// Appends `length` characters of `text` as bytes. See AppendBytes.
void AppendText(std::vector<uint8_t>& bytes, const char* text, size_t length)
{
	for (size_t i = 0; i < length; ++i)
	{
		bytes.push_back(static_cast<uint8_t>(text[i]));
	}
}

void Append(std::vector<uint8_t>& target, const std::vector<uint8_t>& part)
{
	for (const uint8_t value : part)
	{
		target.push_back(value);
	}
}

void PushName(std::vector<uint8_t>& bytes, const char* text)
{
	const size_t length = std::strlen(text);
	PushU16(bytes, kName);
	PushU32(bytes, static_cast<uint32_t>(length));
	AppendText(bytes, text, length);
}

void PushIntList(std::vector<uint8_t>& bytes, const std::vector<uint32_t>& values)
{
	PushU16(bytes, kIntList);
	PushU32(bytes, static_cast<uint32_t>(values.size()));
	for (const uint32_t value : values)
	{
		PushU32(bytes, value);
	}
}

void PushFloatList(std::vector<uint8_t>& bytes, const std::vector<float>& values)
{
	PushU16(bytes, kFloatList);
	PushU32(bytes, static_cast<uint32_t>(values.size()));
	for (const float value : values)
	{
		PushF32(bytes, value);
	}
}

std::vector<uint8_t> MakeHeader(const char* encoding)
{
	std::vector<uint8_t> bytes;
	const char* fields[4] = { "xof ", "0303", encoding, "0032" };
	for (const char* field : fields)
	{
		AppendText(bytes, field, 4);
	}
	return bytes;
}

// One data object: NAME '{' lists '}'. Integers and floats are separate lists,
// which is how the RAN files carry them, and which is why the reader keeps them
// in separate vectors.
std::vector<uint8_t> MakeObject(
	const char*                 name,
	const std::vector<uint32_t>& ints,
	const std::vector<float>&    floats)
{
	std::vector<uint8_t> bytes;
	PushName(bytes, name);
	PushU16(bytes, kOBrace);
	if (!ints.empty())
	{
		PushIntList(bytes, ints);
	}
	if (!floats.empty())
	{
		PushFloatList(bytes, floats);
	}
	PushU16(bytes, kCBrace);
	return bytes;
}

// The reference triangle every geometry case is built from: three vertices in
// the z = 0 plane, one shared normal, and one UV set.
const std::vector<float> kPositions = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
const std::vector<float> kNormals   = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };
const std::vector<float> kUvs       = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };
// nVertices, nFaces, then per face: corner count then three indices.
const std::vector<uint32_t> kMeshInts   = { 3, 1, 3, 0, 1, 2 };
const std::vector<uint32_t> kNormalInts = { 3, 1, 3, 0, 1, 2 };
const std::vector<uint32_t> kUvInts     = { 3 };

std::vector<uint8_t> MakeTriangleFile(
	bool       withNormals = true,
	bool       withUvs     = true,
	const char* encoding   = "bin ")
{
	std::vector<uint8_t> bytes = MakeHeader(encoding);
	Append(bytes, MakeObject("Mesh", kMeshInts, kPositions));
	if (withNormals)
	{
		Append(bytes, MakeObject("MeshNormals", kNormalInts, kNormals));
	}
	if (withUvs)
	{
		Append(bytes, MakeObject("MeshTextureCoords", kUvInts, kUvs));
	}
	return bytes;
}

// A template definition the decoder must walk past without being confused by.
// This is the shape RAN uses for XSkinMeshHeader: an array of DWORD declared
// with a name reference as its dimension.
std::vector<uint8_t> FixtureSkinHeaderTemplate()
{
	static const uint8_t kGuidBytes[16] =
	{
		0xCE, 0x69, 0xF1, 0x3C, 0x7C, 0xFF, 0xAB, 0x44,
		0x93, 0xC0, 0xF7, 0x8F, 0x62, 0xD1, 0x72, 0xE2
	};

	std::vector<uint8_t> bytes;
	PushU16(bytes, kTemplate);
	PushName(bytes, "XSkinMeshHeader");
	PushU16(bytes, kOBrace);
	PushU16(bytes, kGuid);
	AppendBytes(bytes, kGuidBytes, 16);
	PushU16(bytes, kArray);
	PushU16(bytes, kDword);
	PushName(bytes, "nBones");
	PushU16(bytes, kOBracket);
	PushU16(bytes, kName);
	PushU32(bytes, 5);
	AppendText(bytes, "array", 5);
	PushU16(bytes, kCBracket);
	PushU16(bytes, kSemicolon);
	PushU16(bytes, kCBrace);
	return bytes;
}

// A template that uses the optional-parts shape RAN's bzip files carry:
// `template Frame { [ ... ] }`.
std::vector<uint8_t> FixtureOptionalFrameTemplate()
{
	static const uint8_t kGuidBytes[16] =
	{
		0x46, 0xAB, 0x82, 0x3D, 0xDA, 0x62, 0xCF, 0x11,
		0xAB, 0x39, 0x00, 0x20, 0xAF, 0x71, 0xE4, 0x33
	};

	std::vector<uint8_t> bytes;
	PushU16(bytes, kTemplate);
	PushName(bytes, "Frame");
	PushU16(bytes, kOBrace);
	PushU16(bytes, kGuid);
	AppendBytes(bytes, kGuidBytes, 16);
	PushU16(bytes, kOBracket);
	PushU16(bytes, kDot);
	PushU16(bytes, kDot);
	PushU16(bytes, kDot);
	PushU16(bytes, kCBracket);
	PushU16(bytes, kCBrace);
	return bytes;
}

std::vector<uint8_t> FixtureVertexDuplicationIndices()
{
	return MakeObject("VertexDuplicationIndices", { 3, 2, 0, 0, 1, 0, 2, 1 }, {});
}

std::vector<uint8_t> FixtureSkinWeights()
{
	std::vector<uint8_t> bytes;
	PushName(bytes, "SkinWeights");
	PushU16(bytes, kOBrace);
	PushU16(bytes, kString);
	PushU32(bytes, 7);
	AppendText(bytes, "Bone001", 7);
	PushU16(bytes, kSemicolon);
	PushIntList(bytes, { 3, 0, 0, 1, 0, 2 });
	PushFloatList(bytes, { 1.0f, 0.0f, 0.0f });
	PushU16(bytes, kCBrace);
	return bytes;
}

std::vector<uint8_t> FixtureMaterial()
{
	std::vector<uint8_t> bytes;
	PushName(bytes, "Material");
	PushU16(bytes, kOBrace);
	PushU16(bytes, kString);
	PushU32(bytes, 6);
	AppendText(bytes, "ep3_bo", 6);
	PushU16(bytes, kSemicolon);
	PushU16(bytes, kFloatList);
	PushU32(bytes, 4);
	for (int i = 0; i < 4; ++i)
	{
		PushF32(bytes, 0.0f);
	}
	PushU16(bytes, kSemicolon);
	PushU16(bytes, kCBrace);
	return bytes;
}

std::vector<uint8_t> FixtureTextureFilename()
{
	std::vector<uint8_t> bytes;
	PushName(bytes, "TextureFilename");
	PushU16(bytes, kOBrace);
	PushU16(bytes, kString);
	PushU32(bytes, 6);
	AppendText(bytes, "ep3_bo", 6);
	PushU16(bytes, kSemicolon);
	PushU16(bytes, kCBrace);
	return bytes;
}

std::vector<uint8_t> FixtureFrameHierarchy()
{
	std::vector<uint8_t> bytes;
	PushName(bytes, "Frame");
	PushName(bytes, "Scene_Root");
	PushU16(bytes, kOBrace);
	PushName(bytes, "FrameTransformMatrix");
	PushU16(bytes, kOBrace);
	std::vector<float> identity(16, 0.0f);
	identity[0] = 1.0f;
	identity[5] = 1.0f;
	identity[10] = 1.0f;
	identity[15] = 1.0f;
	PushFloatList(bytes, identity);
	PushU16(bytes, kCBrace);
	PushU16(bytes, kCBrace);
	return bytes;
}

// A file carrying the given extra parts ahead of the reference triangle.
std::vector<uint8_t> FileWithParts(const std::vector<std::vector<uint8_t>>& parts)
{
	std::vector<uint8_t> bytes = MakeHeader("bin ");
	for (const std::vector<uint8_t>& part : parts)
	{
		Append(bytes, part);
	}
	Append(bytes, MakeObject("Mesh", kMeshInts, kPositions));
	Append(bytes, MakeObject("MeshNormals", kNormalInts, kNormals));
	Append(bytes, MakeObject("MeshTextureCoords", kUvInts, kUvs));
	return bytes;
}

ErrorCode DecodeError(XMeshDecoder& decoder, const std::vector<uint8_t>& bytes)
{
	const Result<MeshAsset> result = decoder.DecodeMesh(ResourceData(bytes));
	return result.IsError() ? result.GetError() : ErrorCode::None;
}

bool Decodes(XMeshDecoder& decoder, const std::vector<uint8_t>& bytes)
{
	return !decoder.DecodeMesh(ResourceData(bytes)).IsError();
}

void CheckStillDecodes(const char* what, const std::vector<std::vector<uint8_t>>& parts)
{
	XMeshDecoder decoder;
	const Result<MeshAsset> result = decoder.DecodeMesh(ResourceData(FileWithParts(parts)));
	if (result.IsError())
	{
		std::printf("    (%s refused: %s)\n", what, Modern::ToString(result.GetError()));
		++ModernTests::FailureCount();
		return;
	}
	CHECK_EQ(result.GetValue().GetVertexCount(), static_cast<size_t>(3));
	CHECK_EQ(result.GetValue().GetIndexCount(), static_cast<size_t>(3));
}

// Builds a file with one value replaced, so each numeric case states exactly
// the bytes it is about. `which`: 0 positions, 1 normals, 2 UVs.
std::vector<uint8_t> MakeFileWithValue(int which, int index, float value)
{
	std::vector<float> positions = kPositions;
	std::vector<float> normals   = kNormals;
	std::vector<float> uvs       = kUvs;

	if (which == 0) { positions[static_cast<size_t>(index)] = value; }
	if (which == 1) { normals[static_cast<size_t>(index)] = value; }
	if (which == 2) { uvs[static_cast<size_t>(index)] = value; }

	std::vector<uint8_t> bytes = MakeHeader("bin ");
	Append(bytes, MakeObject("Mesh", kMeshInts, positions));
	Append(bytes, MakeObject("MeshNormals", kNormalInts, normals));
	Append(bytes, MakeObject("MeshTextureCoords", kUvInts, uvs));
	return bytes;
}

// ---------------------------------------------------------------------------
// MSZip fixtures
// ---------------------------------------------------------------------------
//
// The compressed payloads below are raw DEFLATE streams emitted by zlib for the
// 246-byte token stream of the reference triangle, so the decoder's own
// RFC 1951 implementation is checked against an independent encoder. The second
// pair is the same stream cut in half, with the second half compressed using the
// first as a preset dictionary -- which is what MSZip does between chunks, and
// the only way that path is exercised deterministically.

const uint8_t kDeflateSingle[] =
{
	0x63, 0x64, 0x60, 0x61, 0x60, 0x60, 0xF0, 0x4D, 0x2D, 0xCE, 0xE0, 0x62,
	0x60, 0x03, 0x42, 0x06, 0x06, 0x66, 0x20, 0x66, 0x84, 0xD2, 0x0C, 0x50,
	0x36, 0x13, 0x10, 0xB3, 0x33, 0x70, 0x32, 0xA0, 0x83, 0x06, 0x7B, 0x6C,
	0x7C, 0x6E, 0xA0, 0x1E, 0x6E, 0xA8, 0xA9, 0x7E, 0xF9, 0x45, 0xB9, 0x89,
	0x39, 0xC5, 0xA4, 0x1A, 0x8E, 0x6C, 0x30, 0x2A, 0x1B, 0x64, 0xB8, 0x20,
	0xD4, 0xF0, 0x90, 0xD4, 0x8A, 0x92, 0xD2, 0xA2, 0x54, 0xE7, 0xFC, 0xFC,
	0xA2, 0x14, 0xB0, 0x15, 0x30, 0xA3, 0xD9, 0xC1, 0x96, 0xE1, 0x36, 0x02,
	0x00,
};

const uint8_t kDeflateChunkA[] =
{
	0x63, 0x64, 0x60, 0x61, 0x60, 0x60, 0xF0, 0x4D, 0x2D, 0xCE, 0xE0, 0x62,
	0x60, 0x03, 0x42, 0x06, 0x06, 0x66, 0x20, 0x66, 0x84, 0xD2, 0x0C, 0x50,
	0x36, 0x13, 0x10, 0xB3, 0x33, 0x70, 0x32, 0xA0, 0x83, 0x06, 0x7B, 0x6C,
	0x7C, 0x6E, 0xA0, 0x1E, 0x6E, 0xA8, 0xA9, 0x7E, 0xF9, 0x45, 0xB9, 0x89,
	0x39, 0xC5, 0xD8, 0x0C, 0x07, 0x00,
};

const uint8_t kDeflateChunkB[] =
{
	0xC3, 0x67, 0x38, 0xB2, 0xC1, 0xA8, 0x6C, 0x90, 0xE1, 0x82, 0x50, 0xC3,
	0x43, 0x52, 0x2B, 0x4A, 0x4A, 0x8B, 0x52, 0x9D, 0xF3, 0xF3, 0x8B, 0x52,
	0xC0, 0x56, 0xC0, 0x8C, 0x66, 0x07, 0x5B, 0x86, 0xDB, 0x08, 0x00,
};

constexpr size_t kTokenStreamBytes = 246;

std::vector<uint8_t> MakeMsZipFile(
	const std::vector<std::vector<uint8_t>>& chunks,
	bool withTrailerBetweenChunks)
{
	std::vector<uint8_t> bytes = MakeHeader("bzip");
	PushU32(bytes, static_cast<uint32_t>(XMeshDecoder::kHeaderSize + kTokenStreamBytes));
	PushU32(bytes, 0);  // reserved, not validated

	for (size_t i = 0; i < chunks.size(); ++i)
	{
		bytes.push_back('C');
		bytes.push_back('K');
		Append(bytes, chunks[i]);
		if (withTrailerBetweenChunks && i + 1 < chunks.size())
		{
			PushU32(bytes, 0xDEADBEEFu);
		}
	}
	return bytes;
}

std::vector<std::vector<uint8_t>> AsVectors(const uint8_t* data, size_t size)
{
	return { std::vector<uint8_t>(data, data + size) };
}

// ---------------------------------------------------------------------------
// MXF wrapper
// ---------------------------------------------------------------------------

// CLIENT-013's transform, reimplemented here so this case tests the integration
// rather than agreeing with whatever Transform() happens to do. The encoding is
// the inverse of the transform's `byte += 0xEA; byte ^= 0xEB`, which is
// `encoded = (plain ^ 0xEB) - 0xEA`. Applying the transform's own formula to
// encode does not round-trip, because adding and XORing is not an involution.
std::vector<uint8_t> MakeMxfFromX(const std::vector<uint8_t>& plainX)
{
	std::vector<uint8_t> bytes;
	PushU32(bytes, 0x100u);
	PushU32(bytes, static_cast<uint32_t>(plainX.size()));
	PushU32(bytes, 0u);  // fileType = skin
	for (const uint8_t raw : plainX)
	{
		const uint8_t encoded = static_cast<uint8_t>((raw ^ 0xEBu) - 0xEAu);
		bytes.push_back(encoded);
	}
	return bytes;
}

} // namespace

// ===========================================================================
// Header
// ===========================================================================

// 1. A valid binary header is accepted and the file decodes.
MODERN_TEST(ClientX_ValidBinaryHeaderDecodes)
{
	XMeshDecoder decoder;
	CHECK(Decodes(decoder, MakeTriangleFile()));
}

// 2. Every truncation of the header is refused, including no bytes at all.
MODERN_TEST(ClientX_TruncatedHeaderIsRefused)
{
	XMeshDecoder decoder;
	for (size_t size = 0; size < XMeshDecoder::kHeaderSize; ++size)
	{
		const std::vector<uint8_t> full(XMeshDecoder::kHeaderSize, 0xFF);
		CHECK_EQ(DecodeError(decoder, std::vector<uint8_t>(full.begin(), full.begin() + size)),
		         ErrorCode::InvalidArgument);
	}
}

// 3. Invalid magic, in any of the four header fields, is refused.
MODERN_TEST(ClientX_InvalidMagicIsRefused)
{
	XMeshDecoder decoder;
	for (size_t field = 0; field < 4; ++field)
	{
		std::vector<uint8_t> bytes = MakeTriangleFile();
		bytes[field * 4 + 1] = 'X';
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
}

// 4. An unsupported version is refused. 03.03 is the only one RAN ships.
MODERN_TEST(ClientX_UnsupportedVersionIsRefused)
{
	XMeshDecoder decoder;
	for (const char* version : { "0302", "0304", "0403", "0203", "xxxx" })
	{
		std::vector<uint8_t> bytes = MakeTriangleFile();
		bytes[4] = version[0];
		bytes[5] = version[1];
		bytes[6] = version[2];
		bytes[7] = version[3];
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
}

// 5. An unsupported encoding is refused. `txt ` is what the twelve text-encoded
//    shipped files use, and `tzip` is the other MSZip-family tag RAN never
//    writes; neither is decoded by this milestone.
MODERN_TEST(ClientX_UnsupportedEncodingIsRefused)
{
	XMeshDecoder decoder;
	for (const char* encoding : { "txt ", "tzip", "BIN ", "zzzz" })
	{
		std::vector<uint8_t> bytes = MakeTriangleFile();
		bytes[8] = encoding[0];
		bytes[9] = encoding[1];
		bytes[10] = encoding[2];
		bytes[11] = encoding[3];
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
}

// 6. Empty input is refused.
MODERN_TEST(ClientX_EmptyInputIsRefused)
{
	XMeshDecoder decoder;
	CHECK_EQ(DecodeError(decoder, std::vector<uint8_t>()), ErrorCode::InvalidArgument);
}

// 7. A 64-bit-float header is refused rather than misread as 32-bit.
MODERN_TEST(ClientX_UnsupportedFloatWidthIsRefused)
{
	XMeshDecoder decoder;
	std::vector<uint8_t> bytes = MakeTriangleFile();
	bytes[12] = '0';
	bytes[13] = '0';
	bytes[14] = '6';
	bytes[15] = '4';
	CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
}

// 8. A valid header followed by nothing is refused: there is no mesh in it.
MODERN_TEST(ClientX_HeaderOnlyIsRefused)
{
	XMeshDecoder decoder;
	CHECK_EQ(DecodeError(decoder, MakeHeader("bin ")), ErrorCode::InvalidArgument);
}

// ===========================================================================
// Geometry
// ===========================================================================

// 9. The minimal valid triangle decodes to three vertices and three indices.
MODERN_TEST(ClientX_MinimalValidTriangle)
{
	XMeshDecoder decoder;
	const Result<MeshAsset> result = decoder.DecodeMesh(ResourceData(MakeTriangleFile()));
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	CHECK_EQ(result.GetValue().GetVertexCount(), static_cast<size_t>(3));
	CHECK_EQ(result.GetValue().GetIndexCount(), static_cast<size_t>(3));
	CHECK_EQ(result.GetValue().GetTriangleCount(), static_cast<size_t>(1));
}

// 10. Vertex positions come through exactly, bit for bit.
MODERN_TEST(ClientX_CorrectVertexPositions)
{
	XMeshDecoder decoder;
	const Result<MeshAsset> result = decoder.DecodeMesh(ResourceData(MakeTriangleFile()));
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	const std::vector<MeshVertex>& vertices = result.GetValue().GetVertices();
	CHECK_EQ(vertices.size(), static_cast<size_t>(3));
	CHECK(vertices[0].position == Vector3(0.0f, 0.0f, 0.0f));
	CHECK(vertices[1].position == Vector3(1.0f, 0.0f, 0.0f));
	CHECK(vertices[2].position == Vector3(0.0f, 1.0f, 0.0f));
}

// 11. Normals come through as the file states them, not derived from positions.
MODERN_TEST(ClientX_CorrectNormals)
{
	XMeshDecoder decoder;
	const Result<MeshAsset> result = decoder.DecodeMesh(ResourceData(MakeTriangleFile()));
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	for (const MeshVertex& vertex : result.GetValue().GetVertices())
	{
		CHECK(vertex.normal == Vector3(0.0f, 0.0f, 1.0f));
	}
}

// 12. UVs map u -> u and v -> v, in the order the file lists them.
MODERN_TEST(ClientX_CorrectUvs)
{
	XMeshDecoder decoder;
	const Result<MeshAsset> result = decoder.DecodeMesh(ResourceData(MakeTriangleFile()));
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	const std::vector<MeshVertex>& vertices = result.GetValue().GetVertices();
	CHECK_EQ(vertices[0].u, 0.0f);
	CHECK_EQ(vertices[0].v, 0.0f);
	CHECK_EQ(vertices[1].u, 1.0f);
	CHECK_EQ(vertices[1].v, 0.0f);
	CHECK_EQ(vertices[2].u, 0.0f);
	CHECK_EQ(vertices[2].v, 1.0f);
}

// 13. Indices are the face's three corners, in the file's winding order.
MODERN_TEST(ClientX_CorrectTriangleIndices)
{
	XMeshDecoder decoder;
	const Result<MeshAsset> result = decoder.DecodeMesh(ResourceData(MakeTriangleFile()));
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	const std::vector<MeshIndex>& indices = result.GetValue().GetIndices();
	CHECK_EQ(indices.size(), static_cast<size_t>(3));
	CHECK_EQ(indices[0], static_cast<MeshIndex>(0));
	CHECK_EQ(indices[1], static_cast<MeshIndex>(1));
	CHECK_EQ(indices[2], static_cast<MeshIndex>(2));
}

// 14. The topology is the only one MeshAsset has, and it is stated.
MODERN_TEST(ClientX_TopologyIsTriangleList)
{
	XMeshDecoder decoder;
	const Result<MeshAsset> result = decoder.DecodeMesh(ResourceData(MakeTriangleFile()));
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	CHECK(result.GetValue().GetTopology() == PrimitiveTopology::TriangleList);
}

// 15. An index that does not name a vertex is refused, not clamped.
MODERN_TEST(ClientX_IndexOutOfRangeIsRefused)
{
	XMeshDecoder decoder;

	// One past the last vertex, and far past it. The boundary itself, index 2 of
	// 3 vertices, is legal and is asserted at the end.
	const std::vector<uint32_t> atLimit = { 3, 1, 3, 0, 1, 2 };
	const std::vector<uint32_t> past    = { 3, 1, 3, 0, 1, 3 };
	const std::vector<uint32_t> huge    = { 3, 1, 3, 0, 1, 0xFFFFFFFFu };

	for (const std::vector<uint32_t>* ints : { &past, &huge })
	{
		std::vector<uint8_t> bytes = MakeHeader("bin ");
		Append(bytes, MakeObject("Mesh", *ints, kPositions));
		Append(bytes, MakeObject("MeshNormals", kNormalInts, kNormals));
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}

	std::vector<uint8_t> ok = MakeHeader("bin ");
	Append(ok, MakeObject("Mesh", atLimit, kPositions));
	Append(ok, MakeObject("MeshNormals", kNormalInts, kNormals));
	CHECK(Decodes(decoder, ok));
}

// 16. A malformed vertex count is refused: zero, a count the float list cannot
//     support, and counts past the asset ceiling.
MODERN_TEST(ClientX_MalformedVertexCountIsRefused)
{
	XMeshDecoder decoder;

	{  // zero vertices
		std::vector<uint8_t> bytes = MakeHeader("bin ");
		Append(bytes, MakeObject("Mesh", { 0, 0 }, {}));
		Append(bytes, MakeObject("MeshNormals", { 0 }, {}));
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
	{  // claims four vertices, supplies three
		std::vector<uint8_t> bytes = MakeHeader("bin ");
		Append(bytes, MakeObject("Mesh", { 4, 1, 3, 0, 1, 2 }, kPositions));
		Append(bytes, MakeObject("MeshNormals", { 4, 1, 3, 0, 1, 2 }, kNormals));
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
	{  // a count no float list could satisfy
		std::vector<uint8_t> bytes = MakeHeader("bin ");
		Append(bytes, MakeObject("Mesh", { 0xFFFFFFFFu, 1, 3, 0, 1, 2 }, kPositions));
		Append(bytes, MakeObject("MeshNormals", { 0xFFFFFFFFu, 1, 3, 0, 1, 2 }, kNormals));
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
	{  // past kMaxMeshVertices
		std::vector<uint8_t> bytes = MakeHeader("bin ");
		Append(bytes, MakeObject("Mesh", { kMaxMeshVertices + 1u, 1, 3, 0, 1, 2 }, kPositions));
		Append(bytes, MakeObject("MeshNormals", kNormalInts, kNormals));
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
}

// 17. A malformed face count is refused: zero, more faces than the integers
//     hold, trailing integers, and a face claiming too many corners.
MODERN_TEST(ClientX_MalformedFaceCountIsRefused)
{
	XMeshDecoder decoder;

	{  // no faces at all
		std::vector<uint8_t> bytes = MakeHeader("bin ");
		Append(bytes, MakeObject("Mesh", { 3, 0 }, kPositions));
		Append(bytes, MakeObject("MeshNormals", { 3, 0 }, kNormals));
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
	{  // claims two faces, supplies one
		std::vector<uint8_t> bytes = MakeHeader("bin ");
		Append(bytes, MakeObject("Mesh", { 3, 2, 3, 0, 1, 2 }, kPositions));
		Append(bytes, MakeObject("MeshNormals", kNormalInts, kNormals));
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
	{  // trailing integer after a complete face list
		std::vector<uint8_t> bytes = MakeHeader("bin ");
		Append(bytes, MakeObject("Mesh", { 3, 1, 3, 0, 1, 2, 7 }, kPositions));
		Append(bytes, MakeObject("MeshNormals", kNormalInts, kNormals));
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
	{  // a face claiming more corners than the list holds
		std::vector<uint8_t> bytes = MakeHeader("bin ");
		Append(bytes, MakeObject("Mesh", { 3, 1, 4, 0, 1, 2 }, kPositions));
		Append(bytes, MakeObject("MeshNormals", kNormalInts, kNormals));
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
}

// 18. A non-triangle face is refused rather than triangulated by a fan order
//     this format does not state. All 13,292 faces in the shipped tree are
//     triangles, so a polygon is not RAN behaviour to accommodate.
MODERN_TEST(ClientX_NonTriangleFaceIsRefused)
{
	XMeshDecoder decoder;

	const std::vector<float> quadPositions =
	{
		0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f
	};
	std::vector<float> quadNormals(12, 0.0f);
	for (size_t i = 2; i < quadNormals.size(); i += 3)
	{
		quadNormals[i] = 1.0f;
	}

	std::vector<uint8_t> bytes = MakeHeader("bin ");
	Append(bytes, MakeObject("Mesh", { 4, 1, 4, 0, 1, 2, 3 }, quadPositions));
	Append(bytes, MakeObject("MeshNormals", { 4, 1, 4, 0, 1, 2, 3 }, quadNormals));
	CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
}

// 19. Truncated vertex data is refused: every prefix of the Mesh object, and
//     every prefix of the normals object after it.
MODERN_TEST(ClientX_TruncatedVertexDataIsRefused)
{
	XMeshDecoder decoder;
	const std::vector<uint8_t> meshObject = MakeObject("Mesh", kMeshInts, kPositions);
	const std::vector<uint8_t> normalObject = MakeObject("MeshNormals", kNormalInts, kNormals);

	for (size_t length = 1; length < meshObject.size(); ++length)
	{
		std::vector<uint8_t> bytes = MakeHeader("bin ");
		for (size_t i = 0; i < length; ++i)
		{
			bytes.push_back(meshObject[i]);
		}
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}

	for (size_t length = 1; length < normalObject.size(); ++length)
	{
		std::vector<uint8_t> bytes = MakeHeader("bin ");
		Append(bytes, meshObject);
		for (size_t i = 0; i < length; ++i)
		{
			bytes.push_back(normalObject[i]);
		}
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
}

// 20. Truncated index data is refused, as a short file and as a list whose
//     count exceeds the bytes that follow.
MODERN_TEST(ClientX_TruncatedIndexDataIsRefused)
{
	XMeshDecoder decoder;

	{  // the file stops one byte into the face list
		std::vector<uint8_t> bytes = MakeHeader("bin ");
		Append(bytes, MakeObject("Mesh", kMeshInts, kPositions));
		bytes.pop_back();
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
	{  // an integer list whose count exceeds the file
		std::vector<uint8_t> bytes = MakeHeader("bin ");
		PushName(bytes, "Mesh");
		PushU16(bytes, kOBrace);
		PushU16(bytes, kIntList);
		PushU32(bytes, 4096u);
		PushU32(bytes, 3);
		PushU32(bytes, 1);
		PushFloatList(bytes, kPositions);
		PushU16(bytes, kCBrace);
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
	{  // a float list whose count exceeds the file
		std::vector<uint8_t> bytes = MakeHeader("bin ");
		PushName(bytes, "Mesh");
		PushU16(bytes, kOBrace);
		PushIntList(bytes, kMeshInts);
		PushU16(bytes, kFloatList);
		PushU32(bytes, 4096u);
		PushFloatList(bytes, kPositions);
		PushU16(bytes, kCBrace);
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
}

// ===========================================================================
// Numeric validation
// ===========================================================================

// 21. NaN in a position is refused.
MODERN_TEST(ClientX_NanPositionIsRefused)
{
	XMeshDecoder decoder;
	const float nan = std::numeric_limits<float>::quiet_NaN();
	for (int i = 0; i < 9; ++i)
	{
		CHECK_EQ(DecodeError(decoder, MakeFileWithValue(0, i, nan)), ErrorCode::InvalidArgument);
	}
}

// 22. Positive or negative infinity in a position is refused.
MODERN_TEST(ClientX_InfinitePositionIsRefused)
{
	XMeshDecoder decoder;
	const float inf = std::numeric_limits<float>::infinity();
	for (int i = 0; i < 9; ++i)
	{
		CHECK_EQ(DecodeError(decoder, MakeFileWithValue(0, i, inf)), ErrorCode::InvalidArgument);
		CHECK_EQ(DecodeError(decoder, MakeFileWithValue(0, i, -inf)), ErrorCode::InvalidArgument);
	}
}

// 23. NaN in a normal is refused. Normals are never invented, so a normal that
//     is not a number cannot become a zero vector.
MODERN_TEST(ClientX_NanNormalIsRefused)
{
	XMeshDecoder decoder;
	const float nan = std::numeric_limits<float>::quiet_NaN();
	for (int i = 0; i < 9; ++i)
	{
		CHECK_EQ(DecodeError(decoder, MakeFileWithValue(1, i, nan)), ErrorCode::InvalidArgument);
	}
}

// 24. Infinity in a normal is refused.
MODERN_TEST(ClientX_InfiniteNormalIsRefused)
{
	XMeshDecoder decoder;
	const float inf = std::numeric_limits<float>::infinity();
	for (int i = 0; i < 9; ++i)
	{
		CHECK_EQ(DecodeError(decoder, MakeFileWithValue(1, i, inf)), ErrorCode::InvalidArgument);
		CHECK_EQ(DecodeError(decoder, MakeFileWithValue(1, i, -inf)), ErrorCode::InvalidArgument);
	}
}

// 25. A non-finite UV is refused, which is what MeshAsset's finiteness rule
//     requires of the coordinate it does have.
MODERN_TEST(ClientX_InvalidUvIsRefused)
{
	XMeshDecoder decoder;
	const float nan = std::numeric_limits<float>::quiet_NaN();
	const float inf = std::numeric_limits<float>::infinity();
	for (int i = 0; i < 6; ++i)
	{
		CHECK_EQ(DecodeError(decoder, MakeFileWithValue(2, i, nan)), ErrorCode::InvalidArgument);
		CHECK_EQ(DecodeError(decoder, MakeFileWithValue(2, i, inf)), ErrorCode::InvalidArgument);
		CHECK_EQ(DecodeError(decoder, MakeFileWithValue(2, i, -inf)), ErrorCode::InvalidArgument);
	}
}

// ===========================================================================
// Fixtures outside the decoder's scope
// ===========================================================================

// 26-33. One fixture per case, built with no decoding anywhere. These are the
//        cases that exposed the const char* insert defect, so each is checked on
//        its own: a case that aborts names its own builder.
MODERN_TEST(ClientX_Fixture_Header)
{
	CHECK_EQ(MakeHeader("bin ").size(), static_cast<size_t>(16));
}

MODERN_TEST(ClientX_Fixture_SkinHeaderTemplate)
{
	CHECK(FixtureSkinHeaderTemplate().size() > 0u);
}

MODERN_TEST(ClientX_Fixture_OptionalFrameTemplate)
{
	CHECK(FixtureOptionalFrameTemplate().size() > 0u);
}

MODERN_TEST(ClientX_Fixture_VertexDuplicationIndices)
{
	CHECK(FixtureVertexDuplicationIndices().size() > 0u);
}

MODERN_TEST(ClientX_Fixture_SkinWeights)
{
	CHECK(FixtureSkinWeights().size() > 0u);
}

MODERN_TEST(ClientX_Fixture_Material)
{
	CHECK(FixtureMaterial().size() > 0u);
}

MODERN_TEST(ClientX_Fixture_TextureFilename)
{
	CHECK(FixtureTextureFilename().size() > 0u);
}

MODERN_TEST(ClientX_Fixture_FrameHierarchy)
{
	CHECK(FixtureFrameHierarchy().size() > 0u);
}

// 34. No extra part: the reference triangle alone still decodes.
MODERN_TEST(ClientX_Deferred_0_Baseline)
{
	CheckStillDecodes("baseline", {});
}

// 35-41. One extra part at a time, so a failure names its own fixture.
MODERN_TEST(ClientX_Deferred_1_SkinHeaderTemplate)
{
	CheckStillDecodes("skinHeaderTemplate", { FixtureSkinHeaderTemplate() });
}

MODERN_TEST(ClientX_Deferred_2_OptionalFrameTemplate)
{
	CheckStillDecodes("optionalFrameTemplate", { FixtureOptionalFrameTemplate() });
}

MODERN_TEST(ClientX_Deferred_3_VertexDuplicationIndices)
{
	CheckStillDecodes("vertexDuplicationIndices", { FixtureVertexDuplicationIndices() });
}

MODERN_TEST(ClientX_Deferred_4_SkinWeights)
{
	CheckStillDecodes("skinWeights", { FixtureSkinWeights() });
}

MODERN_TEST(ClientX_Deferred_5_Material)
{
	CheckStillDecodes("material", { FixtureMaterial() });
}

MODERN_TEST(ClientX_Deferred_6_TextureFilename)
{
	CheckStillDecodes("textureFilename", { FixtureTextureFilename() });
}

MODERN_TEST(ClientX_Deferred_7_FrameHierarchy)
{
	CheckStillDecodes("frameHierarchy", { FixtureFrameHierarchy() });
}

// 42-44. Accumulated, so the smallest combination that misbehaves is the one
//        that stops.
MODERN_TEST(ClientX_Deferred_8_FirstTwo)
{
	CheckStillDecodes("firstTwo", { FixtureSkinHeaderTemplate(), FixtureOptionalFrameTemplate() });
}

MODERN_TEST(ClientX_Deferred_9_AllButFrame)
{
	CheckStillDecodes("allButFrame",
	{
		FixtureSkinHeaderTemplate(), FixtureOptionalFrameTemplate(),
		FixtureVertexDuplicationIndices(), FixtureSkinWeights(),
		FixtureMaterial(), FixtureTextureFilename(),
	});
}

// 45. The whole set: skinning, material and frame data must all be walked and
//     dropped without changing the geometry or leaking into the mesh.
MODERN_TEST(ClientX_DeferredTemplatesAreParsedAndDropped)
{
	CheckStillDecodes("all",
	{
		FixtureSkinHeaderTemplate(), FixtureOptionalFrameTemplate(),
		FixtureVertexDuplicationIndices(), FixtureSkinWeights(),
		FixtureMaterial(), FixtureTextureFilename(), FixtureFrameHierarchy(),
	});
}

// 46. A mesh nested inside a Frame is found, because the geometry templates are
//     not required to sit at the top level. RAN puts every real mesh there.
MODERN_TEST(ClientX_MeshNestedInsideFrameIsDecoded)
{
	XMeshDecoder decoder;

	std::vector<uint8_t> bytes = MakeHeader("bin ");
	PushName(bytes, "Frame");
	PushName(bytes, "Root");
	PushU16(bytes, kOBrace);
	Append(bytes, MakeObject("Mesh", kMeshInts, kPositions));
	Append(bytes, MakeObject("MeshNormals", kNormalInts, kNormals));
	Append(bytes, MakeObject("MeshTextureCoords", kUvInts, kUvs));
	PushU16(bytes, kCBrace);

	const Result<MeshAsset> result = decoder.DecodeMesh(ResourceData(bytes));
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	CHECK_EQ(result.GetValue().GetVertexCount(), static_cast<size_t>(3));
}

// 47. Several Mesh objects are merged in document order, each sub-mesh's indices
//     biased by the running vertex base. 43 shipped assets are multi-part, up
//     to 116 sub-meshes in one file.
MODERN_TEST(ClientX_MultipleMeshesAreMergedInOrder)
{
	XMeshDecoder decoder;

	const std::vector<float> secondPositions =
	{
		2.0f, 2.0f, 0.0f, 3.0f, 2.0f, 0.0f, 2.0f, 3.0f, 0.0f
	};

	std::vector<uint8_t> bytes = MakeHeader("bin ");
	Append(bytes, MakeObject("Mesh", kMeshInts, kPositions));
	Append(bytes, MakeObject("MeshNormals", kNormalInts, kNormals));
	Append(bytes, MakeObject("MeshTextureCoords", kUvInts, kUvs));
	Append(bytes, MakeObject("Mesh", kMeshInts, secondPositions));
	Append(bytes, MakeObject("MeshNormals", kNormalInts, kNormals));
	Append(bytes, MakeObject("MeshTextureCoords", kUvInts, kUvs));

	const Result<MeshAsset> result = decoder.DecodeMesh(ResourceData(bytes));
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}

	const MeshAsset& mesh = result.GetValue();
	CHECK_EQ(mesh.GetVertexCount(), static_cast<size_t>(6));
	CHECK_EQ(mesh.GetIndexCount(), static_cast<size_t>(6));
	CHECK_EQ(mesh.GetTriangleCount(), static_cast<size_t>(2));
	CHECK(mesh.GetVertices()[3].position == Vector3(2.0f, 2.0f, 0.0f));
	CHECK_EQ(mesh.GetIndices()[3], static_cast<MeshIndex>(3));
	CHECK_EQ(mesh.GetIndices()[4], static_cast<MeshIndex>(4));
	CHECK_EQ(mesh.GetIndices()[5], static_cast<MeshIndex>(5));
}

// 48. A mesh with no MeshNormals is refused: normals are never invented. One
//     shipped asset is refused for this reason (b_pet_human_ninefox.x).
MODERN_TEST(ClientX_MissingNormalsAreRefused)
{
	XMeshDecoder decoder;
	CHECK_EQ(DecodeError(decoder, MakeTriangleFile(false, true)), ErrorCode::InvalidArgument);
	CHECK_EQ(DecodeError(decoder, MakeTriangleFile(false, false)), ErrorCode::InvalidArgument);
}

// 49. A MeshNormals whose count differs from its mesh's vertex count is refused.
MODERN_TEST(ClientX_MismatchedNormalCountIsRefused)
{
	XMeshDecoder decoder;

	std::vector<uint8_t> bytes = MakeHeader("bin ");
	Append(bytes, MakeObject("Mesh", kMeshInts, kPositions));
	Append(bytes, MakeObject("MeshNormals", { 4, 1, 3, 0, 1, 2 }, kNormals));
	CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
}

// 50. A file with no MeshTextureCoords decodes as untextured, u = v = 0. That
//     is the one absence MeshAsset has a value for, and it is stated rather
//     than inferred. Ten shipped assets have no UVs.
MODERN_TEST(ClientX_MissingUvsDecodeAsZero)
{
	XMeshDecoder decoder;
	const Result<MeshAsset> result =
		decoder.DecodeMesh(ResourceData(MakeTriangleFile(true, false)));
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	for (const MeshVertex& vertex : result.GetValue().GetVertices())
	{
		CHECK_EQ(vertex.u, 0.0f);
		CHECK_EQ(vertex.v, 0.0f);
	}
}

// 51. A file that has some UV sets but not one per mesh is refused rather than
//     paired by position: RAN emits those blocks out of step in four shipped
//     assets (b_effet_char.x, b_m.x, b_m1.x, b_w.x), and pairing them by index
//     would attach UVs to wrong vertices.
MODERN_TEST(ClientX_UnmatchableUvsAreRefused)
{
	XMeshDecoder decoder;

	{  // two meshes, one UV set
		std::vector<uint8_t> bytes = MakeHeader("bin ");
		Append(bytes, MakeObject("Mesh", kMeshInts, kPositions));
		Append(bytes, MakeObject("Mesh", kMeshInts, kPositions));
		Append(bytes, MakeObject("MeshNormals", kNormalInts, kNormals));
		Append(bytes, MakeObject("MeshNormals", kNormalInts, kNormals));
		Append(bytes, MakeObject("MeshTextureCoords", kUvInts, kUvs));
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
	{  // a UV set whose count is not the vertex count
		std::vector<uint8_t> bytes = MakeHeader("bin ");
		Append(bytes, MakeObject("Mesh", kMeshInts, kPositions));
		Append(bytes, MakeObject("MeshNormals", kNormalInts, kNormals));
		Append(bytes, MakeObject("MeshTextureCoords", { 2 }, { 0.0f, 0.0f, 1.0f, 0.0f }));
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
}

// 52. A file with no mesh at all is refused: a skeleton-only `.x` is not a
//     static mesh, and an empty asset would be a different answer. Most of the
//     shipped tree, and all but one of the bzip files, is in this class.
MODERN_TEST(ClientX_FileWithoutMeshIsRefused)
{
	XMeshDecoder decoder;

	std::vector<uint8_t> bytes = MakeHeader("bin ");
	Append(bytes, FixtureSkinHeaderTemplate());
	PushName(bytes, "Frame");
	PushName(bytes, "Scene_Root");
	PushU16(bytes, kOBrace);
	PushU16(bytes, kCBrace);
	CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
}

// 53. A Mesh with zero vertices is refused, which is what the eight shipped
//     assets that carry an empty Mesh object hit.
MODERN_TEST(ClientX_EmptyMeshIsRefused)
{
	XMeshDecoder decoder;
	std::vector<uint8_t> bytes = MakeHeader("bin ");
	Append(bytes, MakeObject("Mesh", { 0, 0 }, {}));
	Append(bytes, MakeObject("MeshNormals", { 0 }, {}));
	CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
}

// ===========================================================================
// bzip / MSZip
// ===========================================================================

// 54. A single-chunk MSZip file decodes to the same mesh as the binary form.
//     The payload is a zlib-produced DEFLATE stream, so this also covers the
//     decoder's RFC 1951 implementation.
MODERN_TEST(ClientX_BzipSingleChunkDecodes)
{
	XMeshDecoder decoder;
	const std::vector<std::vector<uint8_t>> chunks =
		AsVectors(kDeflateSingle, sizeof(kDeflateSingle));

	const Result<MeshAsset> result =
		decoder.DecodeMesh(ResourceData(MakeMsZipFile(chunks, false)));
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	CHECK_EQ(result.GetValue().GetVertexCount(), static_cast<size_t>(3));
	CHECK_EQ(result.GetValue().GetIndexCount(), static_cast<size_t>(3));
	CHECK(result.GetValue().GetVertices()[2].position == Vector3(0.0f, 1.0f, 0.0f));
}

// 55. Two chunks decode, which is only possible when each chunk is inflated with
//     the previous chunk's output as a preset dictionary. 85 of the 86 shipped
//     bzip files have one chunk and one has three.
MODERN_TEST(ClientX_BzipMultipleChunksDecode)
{
	XMeshDecoder decoder;
	const std::vector<std::vector<uint8_t>> chunks =
	{
		std::vector<uint8_t>(kDeflateChunkA, kDeflateChunkA + sizeof(kDeflateChunkA)),
		std::vector<uint8_t>(kDeflateChunkB, kDeflateChunkB + sizeof(kDeflateChunkB)),
	};

	const Result<MeshAsset> result =
		decoder.DecodeMesh(ResourceData(MakeMsZipFile(chunks, true)));
	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	CHECK_EQ(result.GetValue().GetVertexCount(), static_cast<size_t>(3));
	CHECK_EQ(result.GetValue().GetIndexCount(), static_cast<size_t>(3));
}

// 56. A bzip file whose chunk signature is wrong is refused.
MODERN_TEST(ClientX_BzipWithBadChunkSignatureIsRefused)
{
	XMeshDecoder decoder;
	std::vector<uint8_t> bytes =
		MakeMsZipFile(AsVectors(kDeflateSingle, sizeof(kDeflateSingle)), false);
	bytes[XMeshDecoder::kMsZipPrefixSize] = 'X';
	CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
}

// 57. A bzip file cut short is refused, at every truncation of the payload.
MODERN_TEST(ClientX_BzipTruncatedPayloadIsRefused)
{
	XMeshDecoder decoder;
	const std::vector<uint8_t> full =
		MakeMsZipFile(AsVectors(kDeflateSingle, sizeof(kDeflateSingle)), false);

	for (size_t size = XMeshDecoder::kMsZipPrefixSize; size < full.size(); ++size)
	{
		std::vector<uint8_t> bytes;
		for (size_t i = 0; i < size; ++i)
		{
			bytes.push_back(full[i]);
		}
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
}

// 58. A bzip file whose declared total is impossible is refused rather than
//     sized by the number the file states.
MODERN_TEST(ClientX_BzipWithImpossibleDeclaredSizeIsRefused)
{
	XMeshDecoder decoder;
	const std::vector<std::vector<uint8_t>> chunks =
		AsVectors(kDeflateSingle, sizeof(kDeflateSingle));

	{  // zero
		std::vector<uint8_t> bytes = MakeMsZipFile(chunks, false);
		bytes[16] = 0; bytes[17] = 0; bytes[18] = 0; bytes[19] = 0;
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
	{  // below the header it claims to include
		std::vector<uint8_t> bytes = MakeMsZipFile(chunks, false);
		bytes[16] = 4;
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
	{  // past the decoder's inflated ceiling
		std::vector<uint8_t> bytes = MakeMsZipFile(chunks, false);
		bytes[16] = 0xFF; bytes[17] = 0xFF; bytes[18] = 0xFF; bytes[19] = 0xFF;
		CHECK_EQ(DecodeError(decoder, bytes), ErrorCode::InvalidArgument);
	}
}

// 59. Corrupt DEFLATE bytes are refused rather than decoded to something that
//     merely looks like geometry.
MODERN_TEST(ClientX_BzipCorruptDeflateIsRefused)
{
	XMeshDecoder decoder;

	for (size_t index = 0; index < sizeof(kDeflateSingle); index += 7)
	{
		std::vector<std::vector<uint8_t>> chunks =
			AsVectors(kDeflateSingle, sizeof(kDeflateSingle));
		chunks[0][index] = static_cast<uint8_t>(chunks[0][index] ^ 0xFFu);
		const Result<MeshAsset> result =
			decoder.DecodeMesh(ResourceData(MakeMsZipFile(chunks, false)));
		if (!result.IsError())
		{
			CHECK(result.GetValue().GetVertexCount() != 3);
		}
	}
}

// ===========================================================================
// Integration
// ===========================================================================

// 60. ResourceData -> XMeshDecoder -> MeshAsset, through the interface, with
//     counts, topology and representative values all checked.
MODERN_TEST(ClientX_ResourceDataToMeshAssetIntegration)
{
	XMeshDecoder concrete;
	IMeshDecoder& decoder = concrete;

	const std::vector<uint8_t> bytes = MakeTriangleFile();
	const Result<MeshAsset> result = decoder.DecodeMesh(ResourceData(bytes));

	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}

	const MeshAsset& mesh = result.GetValue();
	CHECK(mesh.GetTopology() == PrimitiveTopology::TriangleList);
	CHECK_EQ(mesh.GetVertexCount(), static_cast<size_t>(3));
	CHECK_EQ(mesh.GetIndexCount(), static_cast<size_t>(3));
	CHECK_EQ(mesh.GetVertexByteCount(), 3u * kMeshVertexBytes);
	CHECK_EQ(mesh.GetIndexByteCount(), 3u * kMeshIndexBytes);
	CHECK_EQ(mesh.GetTotalByteCount(), 3u * kMeshVertexBytes + 3u * kMeshIndexBytes);

	const std::vector<MeshVertex>& vertices = mesh.GetVertices();
	CHECK(vertices[1].position == Vector3(1.0f, 0.0f, 0.0f));
	CHECK(vertices[1].normal == Vector3(0.0f, 0.0f, 1.0f));
	CHECK_EQ(vertices[2].u, 0.0f);
	CHECK_EQ(vertices[2].v, 1.0f);
	for (const MeshVertex& vertex : vertices)
	{
		CHECK(vertex.IsFinite());
	}
}

// 61. The decoder is stateless: a failure leaves the next call unaffected, and
//     separate instances agree.
MODERN_TEST(ClientX_DecoderIsDeterministicAndStateless)
{
	const std::vector<uint8_t> bytes = MakeTriangleFile();
	const std::vector<uint8_t> broken = MakeHeader("bin ");

	XMeshDecoder decoder;
	const Result<MeshAsset> first = decoder.DecodeMesh(ResourceData(bytes));
	CHECK(first.IsOk());

	CHECK(decoder.DecodeMesh(ResourceData(broken)).IsError());
	const Result<MeshAsset> second = decoder.DecodeMesh(ResourceData(bytes));
	CHECK(second.IsOk());

	if (first.IsError() || second.IsError())
	{
		return;
	}
	CHECK_EQ(first.GetValue().GetVertexCount(), second.GetValue().GetVertexCount());
	CHECK(first.GetValue().GetVertices() == second.GetValue().GetVertices());
	CHECK(first.GetValue().GetIndices() == second.GetValue().GetIndices());

	XMeshDecoder other;
	const Result<MeshAsset> elsewhere = other.DecodeMesh(ResourceData(bytes));
	CHECK(elsewhere.IsOk());
	if (elsewhere.IsError())
	{
		return;
	}
	CHECK(elsewhere.GetValue().GetVertices() == first.GetValue().GetVertices());
}

// 62. The bytes handed over are not retained: the caller's buffer may be
//     destroyed immediately afterwards.
MODERN_TEST(ClientX_OutputIsIndependentOfInput)
{
	std::vector<uint8_t> bytes = MakeTriangleFile();

	XMeshDecoder decoder;
	Result<MeshAsset> result = decoder.DecodeMesh(ResourceData(bytes));
	bytes.assign(bytes.size(), 0xFF);  // clobber the caller's buffer

	CHECK(result.IsOk());
	if (result.IsError())
	{
		return;
	}
	CHECK(result.GetValue().GetVertices()[1].position == Vector3(1.0f, 0.0f, 0.0f));
}

// 63. .mxf -> MxfMeshTransform -> XMeshDecoder -> MeshAsset. CLIENT-013 owns the
//     transform's own tests; this one only proves the boundaries compose.
MODERN_TEST(ClientX_MxfToMeshAssetIntegration)
{
	const std::vector<uint8_t> x = MakeTriangleFile();
	const std::vector<uint8_t> mxf = MakeMxfFromX(x);

	MxfMeshTransform transform;
	const Result<ResourceData> plain = transform.Transform(ResourceData(mxf));
	CHECK(plain.IsOk());
	if (plain.IsError())
	{
		return;
	}
	CHECK(plain.GetValue() == ResourceData(x));

	XMeshDecoder decoder;
	const Result<MeshAsset> mesh = decoder.DecodeMesh(plain.GetValue());
	CHECK(mesh.IsOk());
	if (mesh.IsError())
	{
		return;
	}
	CHECK_EQ(mesh.GetValue().GetVertexCount(), static_cast<size_t>(3));
	CHECK_EQ(mesh.GetValue().GetIndexCount(), static_cast<size_t>(3));
	CHECK(mesh.GetValue().GetVertices()[1].position == Vector3(1.0f, 0.0f, 0.0f));
}

// 64. The same holds for a compressed .mxf: transform first, decode second.
MODERN_TEST(ClientX_MxfBzipToMeshAssetIntegration)
{
	const std::vector<std::vector<uint8_t>> chunks =
		AsVectors(kDeflateSingle, sizeof(kDeflateSingle));
	const std::vector<uint8_t> x = MakeMsZipFile(chunks, false);

	MxfMeshTransform transform;
	const Result<ResourceData> plain =
		transform.Transform(ResourceData(MakeMxfFromX(x)));
	CHECK(plain.IsOk());
	if (plain.IsError())
	{
		return;
	}

	XMeshDecoder decoder;
	const Result<MeshAsset> mesh = decoder.DecodeMesh(plain.GetValue());
	CHECK(mesh.IsOk());
	if (mesh.IsError())
	{
		return;
	}
	CHECK_EQ(mesh.GetValue().GetVertexCount(), static_cast<size_t>(3));
}

// ===========================================================================
// Real RAN assets
// ===========================================================================

namespace
{

// The outcome of one real asset. The categories are kept apart on purpose: a
// file the decoder cannot do anything with is not a file it failed on, and an
// unsupported format is not a pass.
enum class Verdict
{
	Decoded,      // a MeshAsset came out
	Unsupported,  // it carries geometry this decoder documents as refused
	Malformed,    // not a decodable .x at all
	NoGeometry,   // valid .x, but no static Mesh in it: skeleton or animation
	NotEncoding,  // not a binary encoding: the txt files
};

// Whether the bytes carry a `Mesh` object at all, matched as its token
// encoding: TOKEN_NAME, count 4, "Mesh". A classification hint for the report,
// never a decode decision. It cannot see inside a compressed `bzip` payload, so
// for those the decoder's own verdict stands on its own.
bool HasMeshTemplate(const std::vector<uint8_t>& bytes)
{
	static const uint8_t kMeshToken[] =
	{
		0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x4D, 0x65, 0x73, 0x68
	};
	if (bytes.size() < sizeof(kMeshToken))
	{
		return false;
	}
	for (size_t i = 0; i + sizeof(kMeshToken) <= bytes.size(); ++i)
	{
		if (std::memcmp(bytes.data() + i, kMeshToken, sizeof(kMeshToken)) == 0)
		{
			return true;
		}
	}
	return false;
}

Verdict Classify(const std::vector<uint8_t>& bytes)
{
	if (bytes.size() < XMeshDecoder::kHeaderSize)
	{
		return Verdict::Malformed;
	}
	if (std::memcmp(bytes.data(), XMeshDecoder::kMagic, 4) != 0 ||
	    std::memcmp(bytes.data() + 4, XMeshDecoder::kVersion, 4) != 0)
	{
		return Verdict::Malformed;
	}
	const bool isBinary = std::memcmp(bytes.data() + 8, XMeshDecoder::kBinaryTag, 4) == 0;
	const bool isBzip   = std::memcmp(bytes.data() + 8, XMeshDecoder::kBzipTag, 4) == 0;
	if (!isBinary && !isBzip)
	{
		return Verdict::NotEncoding;
	}

	XMeshDecoder decoder;
	const Result<MeshAsset> result = decoder.DecodeMesh(ResourceData(bytes));
	if (result.IsError())
	{
		return HasMeshTemplate(bytes) ? Verdict::Unsupported : Verdict::NoGeometry;
	}
	if (result.GetValue().GetVertexCount() == 0 || result.GetValue().GetIndexCount() == 0)
	{
		return Verdict::Malformed;
	}
	return Verdict::Decoded;
}

} // namespace

// 65. The real RAN client tree, when RAN_ASSET_ROOT names one. Every `.x` under
//     data/skeleton is attempted and reported by category and encoding; nothing
//     is capped silently, every refused file is named, and the counts are
//     printed whether they pass or not.
MODERN_TEST(ClientX_RealRanAssetsWhenAvailable)
{
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
	const char* root = std::getenv("RAN_ASSET_ROOT");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
	if (root == nullptr || root[0] == '\0')
	{
		std::printf("      (real RAN validation skipped: RAN_ASSET_ROOT not set)\n");
		return;
	}

	const std::filesystem::path skeleton =
		std::filesystem::path(root) / "data" / "skeleton";
	if (!std::filesystem::is_directory(skeleton))
	{
		std::printf("      (skipped: no data/skeleton under the root)\n");
		return;
	}

	// The tree holds 604 .x files and 4 with an uppercase .X, so the
	// extension is compared case-insensitively. Comparing it exactly would
	// quietly cap the sweep at 604 of 608.
	std::vector<std::filesystem::path> candidates;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(skeleton))
	{
		if (!entry.is_regular_file())
		{
			continue;
		}
		std::string extension = entry.path().extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		if (extension == ".x")
		{
			candidates.push_back(entry.path());
		}
	}
	std::sort(candidates.begin(), candidates.end());

	size_t total = 0;
	size_t binary = 0, binaryDecoded = 0, binaryUnsupported = 0, binaryMalformed = 0, binaryNoGeom = 0;
	size_t bzip = 0, bzipDecoded = 0, bzipUnsupported = 0, bzipMalformed = 0, bzipNoGeom = 0;
	size_t txt = 0;
	size_t totalVertices = 0, totalTriangles = 0;
	size_t samples = 0;
	std::vector<std::string> refused;

	for (const auto& path : candidates)
	{
		std::ifstream file(path, std::ios::binary);
		if (!file)
		{
			continue;
		}
		const std::vector<uint8_t> bytes{
			std::istreambuf_iterator<char>(file),
			std::istreambuf_iterator<char>()};
		++total;

		const Verdict verdict = Classify(bytes);
		const bool isBzip = bytes.size() >= 12 &&
			std::memcmp(bytes.data() + 8, XMeshDecoder::kBzipTag, 4) == 0;
		const bool isBinary = bytes.size() >= 12 &&
			std::memcmp(bytes.data() + 8, XMeshDecoder::kBinaryTag, 4) == 0;

		if (isBzip)        { ++bzip; }
		else if (isBinary) { ++binary; }
		else               { ++txt; }

		switch (verdict)
		{
		case Verdict::Decoded:
			if (isBzip)        { ++bzipDecoded; }
			else if (isBinary) { ++binaryDecoded; }
			{
				XMeshDecoder decoder;
				const Result<MeshAsset> mesh = decoder.DecodeMesh(ResourceData(bytes));
				if (!mesh.IsError())
				{
					totalVertices += mesh.GetValue().GetVertexCount();
					totalTriangles += mesh.GetValue().GetTriangleCount();
					if (samples < 6)
					{
						++samples;
						std::printf("      sample: %s (%zu v, %zu t)\n",
							path.filename().string().c_str(),
							mesh.GetValue().GetVertexCount(),
							mesh.GetValue().GetTriangleCount());
					}
				}
			}
			break;
		case Verdict::Unsupported:
			if (isBzip) { ++bzipUnsupported; } else { ++binaryUnsupported; }
			refused.push_back(path.filename().string());
			break;
		case Verdict::Malformed:
			if (isBzip) { ++bzipMalformed; } else { ++binaryMalformed; }
			break;
		case Verdict::NoGeometry:
			if (isBzip) { ++bzipNoGeom; } else { ++binaryNoGeom; }
			break;
		case Verdict::NotEncoding:
			break;
		}
	}

	std::printf("      total .x: %zu\n", total);
	std::printf("      bin  : %zu candidates, %zu decoded, %zu unsupported, %zu malformed, %zu no static geometry\n",
		binary, binaryDecoded, binaryUnsupported, binaryMalformed, binaryNoGeom);
	std::printf("      bzip : %zu candidates, %zu decoded, %zu unsupported, %zu malformed, %zu no static geometry\n",
		bzip, bzipDecoded, bzipUnsupported, bzipMalformed, bzipNoGeom);
	std::printf("      txt  : %zu candidates (text-encoded, not in scope)\n", txt);
	std::printf("      geometry: %zu vertices, %zu triangles\n", totalVertices, totalTriangles);
	if (!refused.empty())
	{
		std::printf("      refused (%zu):", refused.size());
		for (const std::string& name : refused)
		{
			std::printf(" %s", name.c_str());
		}
		std::printf("\n");
	}

	CHECK(total > 0u);
	CHECK(binary + bzip > 0u);
	CHECK(binaryDecoded + bzipDecoded > 0u);
	// Every candidate lands in exactly one accounted category.
	CHECK(binaryDecoded + binaryUnsupported + binaryMalformed + binaryNoGeom == binary);
	CHECK(bzipDecoded + bzipUnsupported + bzipMalformed + bzipNoGeom == bzip);
	// The real geometry never approaches the asset ceilings.
	CHECK(totalVertices < 4000000u);
	CHECK(totalTriangles < 8000000u);
}

int main()
{
	// Unbuffered so an abort still shows which case was running.
	std::setvbuf(stdout, nullptr, _IONBF, 0);
	std::printf("Modern CLIENT-015 RAN .X static mesh decoder tests\n\n");

	const int failedCases = ModernTests::RunAll();

	if (failedCases == 0)
	{
		std::printf("\nAll %d test cases passed.\n",
			static_cast<int>(ModernTests::Registry().size()));
		return 0;
	}

	std::printf("\n%d of %d test cases FAILED (%d checks).\n",
		failedCases,
		static_cast<int>(ModernTests::Registry().size()),
		ModernTests::FailureCount());
	return 1;
}
