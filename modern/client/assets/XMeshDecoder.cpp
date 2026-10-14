// CLIENT-015: RAN `.X` static mesh decoder.
//
// Three layers, in this order:
//
//   1. header   the 16-byte `xof 0303<enc> 0032` file header
//   2. MSZip    `bzip` is MSZip (raw DEFLATE chunks, each inflated with the
//               previous chunk as a preset dictionary), not bzip2; `bin`
//               skips this layer
//   3. tokens   the DirectX .X tokenized stream, from which the `Mesh`,
//               `MeshNormals` and `MeshTextureCoords` objects are read
//
// Everything else in the stream is walked and discarded, because a token stream
// cannot be stepped over without parsing it. See XMeshDecoder.h for what is
// deliberately deferred and why.

#include "assets/XMeshDecoder.h"

// CLIENT-RENDER-002: the DEFLATE inflater is shared with PngImageDecoder rather
// than duplicated here. The `bzip` (MSZip) path below inflates raw RFC 1951
// chunks with dictionary chaining, which is exactly what `detail::Inflater`
// provides.
#include "assets/Inflate.h"

#include <cmath>
#include <cstring>
#include <string>
#include <vector>

namespace Modern::Client
{
namespace
{

// ---------------------------------------------------------------------------
// Tokens
// ---------------------------------------------------------------------------
//
// Record-bearing tokens are followed by their record; stand-alone tokens are
// grammar punctuation. The values are the published DirectX .X binary
// encoding; the file format is documented rather than invented here.

enum XToken : uint16_t
{
	kTokenName      = 1,
	kTokenString    = 2,
	kTokenInteger   = 3,
	kTokenGuid      = 5,
	kTokenIntList   = 6,
	kTokenFloatList = 7,

	kTokenOBrace    = 10,
	kTokenCBrace    = 11,
	kTokenOParen    = 12,
	kTokenCParen    = 13,
	kTokenOBracket  = 14,
	kTokenCBracket  = 15,
	kTokenOAngle    = 16,
	kTokenCAngle    = 17,
	kTokenDot       = 18,
	kTokenComma     = 19,
	kTokenSemicolon = 20,
	kTokenTemplate  = 31,
	kTokenWord      = 40,
	kTokenDword     = 41,
	kTokenFloat     = 42,
	kTokenDouble    = 43,
	kTokenChar      = 44,
	kTokenUchar     = 45,
	kTokenSword     = 46,
	kTokenSdword    = 47,
	kTokenVoid      = 48,
	kTokenLpstr     = 49,
	kTokenUnicode   = 50,
	kTokenCstring   = 51,
	kTokenArray     = 52,
};

bool IsPrimitiveType(uint16_t token)
{
	return token >= kTokenWord && token <= kTokenCstring;
}

// The three templates whose data becomes geometry. Every other template is
// parsed and dropped; the list is here so the intent is visible rather than
// implied by an absent name.
constexpr const char* kMeshTemplate           = "Mesh";
constexpr const char* kMeshNormalsTemplate    = "MeshNormals";
constexpr const char* kMeshTextureCoordsTempl = "MeshTextureCoords";

// CLIENT-RENDER-002: the three templates that carry material and texture
// information. They were parsed and discarded before; the same grammar walk is
// used, and what it sees is now kept. See XMeshDecoder.h.
constexpr const char* kMaterialTemplate        = "Material";
constexpr const char* kTextureFilenameTemplate = "TextureFilename";
constexpr const char* kMaterialListTemplate    = "MeshMaterialList";

// A name longer than this is refused before it is read. Template and member
// names in the shipped tree are far below the bound; it exists so a corrupt
// length cannot ask for an enormous string.
constexpr size_t kMaxNameBytes = 1024;

// How deeply data objects and frames may nest. Objects nest in the format, so
// the reader recurses, and recursion needs a bound: a stream whose bytes have
// desynchronised can present an object token at almost every position, and
// without a limit that is a stack exhaustion rather than a refusal. The deepest
// real RAN file nests a Mesh inside a Frame, so this is generous.
constexpr int kMaxNestingDepth = 64;

// Undoes one `bzip` payload: a run of "CK"-prefixed raw DEFLATE chunks,
// concatenated into the token stream that belongs behind the 16-byte file
// header. `expectedTotal` is the writer's declared size including that header;
// it bounds the output and reserves it, and is advisory otherwise, because the
// result is validated by the token parse that follows rather than by trusting
// the number.
bool DecodeMsZip(const uint8_t* data, size_t size, uint32_t expectedTotal, std::vector<uint8_t>& out)
{
	if (data == nullptr || size < XMeshDecoder::kMsZipPrefixSize ||
	    expectedTotal < XMeshDecoder::kHeaderSize)
	{
		return false;
	}

	const size_t budget = static_cast<size_t>(expectedTotal) - XMeshDecoder::kHeaderSize;
	if (budget == 0 || budget > XMeshDecoder::kMaxInflatedBytes)
	{
		return false;
	}

	out.clear();
	out.reserve(budget);

	std::vector<uint8_t> previous;
	size_t pos = XMeshDecoder::kMsZipPrefixSize;

	while (pos < size)
	{
		if (pos + 2 > size || data[pos] != 'C' || data[pos + 1] != 'K')
		{
			return false;
		}
		pos += 2;

		detail::Inflater inf(data + pos, size - pos, budget - out.size());
		inf.SetOutputLimit(budget - out.size());
		inf.SetDictionary(previous);
		if (!inf.Run())
		{
			return false;
		}

		previous = inf.Output();
		out.insert(out.end(), previous.begin(), previous.end());
		if (out.size() > XMeshDecoder::kMaxInflatedBytes)
		{
			return false;
		}

		pos += inf.ConsumedInput();
		if (pos >= size)
		{
			break;  // the last chunk has no trailer
		}
		// A 4-byte trailer separates chunks. Its presence is decided by the
		// next chunk's signature, not by a size field, because RAN's writer
		// emits none.
		if (pos + 6 <= size && data[pos + 4] == 'C' && data[pos + 5] == 'K')
		{
			pos += 4;
		}
		else if (pos + 2 > size || data[pos] != 'C' || data[pos + 1] != 'K')
		{
			return false;
		}
	}

	return !out.empty();
}

// ---------------------------------------------------------------------------
// Token reader
// ---------------------------------------------------------------------------

// A bounded reader over the token stream. There is no path that advances the
// cursor without having checked the bytes it is about to consume, so a truncated
// file is refused at the truncation rather than read past.
class XReader
{
public:
	XReader(const uint8_t* data, size_t size, size_t pos) noexcept
		: m_data(data)
		, m_size(size)
		, m_pos(pos)
	{
	}

	size_t Remaining() const noexcept { return m_size - m_pos; }
	bool   AtEnd() const noexcept { return m_pos >= m_size; }

	bool Peek(uint16_t& out) const noexcept
	{
		if (m_pos + 2 > m_size)
		{
			return false;
		}
		out = static_cast<uint16_t>(m_data[m_pos] | (m_data[m_pos + 1] << 8));
		return true;
	}

	bool ReadU16(uint16_t& out) noexcept
	{
		if (!Peek(out))
		{
			return false;
		}
		m_pos += 2;
		return true;
	}

	bool ReadU32(uint32_t& out) noexcept
	{
		if (m_pos + 4 > m_size)
		{
			return false;
		}
		out = static_cast<uint32_t>(m_data[m_pos]) |
		      (static_cast<uint32_t>(m_data[m_pos + 1]) << 8) |
		      (static_cast<uint32_t>(m_data[m_pos + 2]) << 16) |
		      (static_cast<uint32_t>(m_data[m_pos + 3]) << 24);
		m_pos += 4;
		return true;
	}

	bool ReadF32(float& out) noexcept
	{
		uint32_t bits = 0;
		if (!ReadU32(bits))
		{
			return false;
		}
		std::memcpy(&out, &bits, sizeof(out));
		return true;
	}

	// A TOKEN_NAME or TOKEN_STRING record: a count then that many bytes.
	bool ReadName(std::string& out) noexcept
	{
		uint32_t count = 0;
		if (!ReadU32(count))
		{
			return false;
		}
		if (count > kMaxNameBytes || m_pos + count > m_size)
		{
			return false;
		}
		out.assign(reinterpret_cast<const char*>(m_data + m_pos), count);
		m_pos += count;
		return true;
	}

	// Byte count of a list of `count` 32-bit values, or false when that many
	// bytes are not present. The product is formed in 64-bit so a large count
	// cannot wrap into a small, apparently valid one.
	bool ListBytes(uint32_t count, size_t& bytes) const noexcept
	{
		const uint64_t product = static_cast<uint64_t>(count) * 4u;
		if (product > Remaining())
		{
			return false;
		}
		bytes = static_cast<size_t>(product);
		return true;
	}

	bool SkipBytes(size_t count) noexcept
	{
		if (count > Remaining())
		{
			return false;
		}
		m_pos += count;
		return true;
	}

	bool Rewind(size_t count) noexcept
	{
		if (count > m_pos)
		{
			return false;
		}
		m_pos -= count;
		return true;
	}

	bool Expect(uint16_t token) noexcept
	{
		uint16_t got = 0;
		if (!ReadU16(got))
		{
			return false;
		}
		if (got != token)
		{
			return false;
		}
		return true;
	}

private:
	const uint8_t* m_data;
	size_t         m_size;
	size_t         m_pos;
};

// ---------------------------------------------------------------------------
// Grammar
// ---------------------------------------------------------------------------

// The raw contents of one geometry template: every integer and float of every
// list in the object, in member order.
struct MeshChunk
{
	std::vector<uint32_t> ints;
	std::vector<float>    floats;
};

struct XGeometry
{
	std::vector<MeshChunk> meshes;
	std::vector<MeshChunk> normals;
	std::vector<MeshChunk> uvs;

	// CLIENT-RENDER-002: the material table as the file declares it, in
	// declaration order and de-duplicated by instance name.
	std::vector<MeshMaterial> materials;

	// One entry per `MeshMaterialList` object, in document order. Paired with
	// `meshes` by ordinal index -- the same rule this decoder already uses for
	// `MeshNormals` and `MeshTextureCoords` -- and deliberately *not* by
	// adjacency, because the real shipped files disagree on adjacency: RAN's
	// `data/skin/m_luffy_timeskip.mxf` writes `Mesh` before its list, while the
	// static meshes under `data/skeleton` (b_log_in_man.x and 29 others) write
	// `MeshNormals`, then `MeshMaterialList`, then `Mesh`. Positional pairing
	// handles both; "the list that followed this mesh" handles neither.
	struct MaterialList
	{
		uint32_t                 materialCount = 0;
		std::vector<uint32_t>    faceIndices;
		std::vector<std::string> references;  // material names, in slot order
	};
	std::vector<MaterialList> lists;
};

bool IsGeometryTemplate(const std::string& name)
{
	return name == kMeshTemplate || name == kMeshNormalsTemplate ||
	       name == kMeshTextureCoordsTempl;
}

class XParser
{
public:
	explicit XParser(XReader& reader) noexcept
		: m_reader(reader)
	{
	}

	bool ParseAll(XGeometry& out)
	{
		m_geometry = &out;
		while (!m_reader.AtEnd())
		{
			uint16_t token = 0;
			if (!m_reader.Peek(token))
			{
				return false;
			}
			if (token == kTokenTemplate)
			{
				if (!ReadTemplate())
				{
					return false;
				}
				continue;
			}
			if (token != kTokenName)
			{
				return false;
			}

			std::string name;
			MeshChunk   chunk;
			if (!ReadObject(name, chunk))
			{
				return false;
			}
		}
		return true;
	}

private:
	// TOKEN_TEMPLATE NAME '{' [GUID] parts '}'
	//
	// The member grammar is walked in full even though the schema is not
	// wanted: a declaration and an array declaration are different token
	// shapes, and misreading either would desynchronise everything after this
	// template. The bzip files also define `Frame` as `[ ... ]` - an optional
	// part containing only an ellipsis - which is the other shape.
	bool ReadTemplate()
	{
		if (!m_reader.Expect(kTokenTemplate))
		{
			return false;
		}
		std::string name;
		if (!m_reader.Expect(kTokenName) || !m_reader.ReadName(name) ||
		    !m_reader.Expect(kTokenOBrace))
		{
			return false;
		}

		uint16_t peek = 0;
		if (m_reader.Peek(peek) && peek == kTokenGuid)
		{
			uint16_t guid = 0;
			if (!m_reader.ReadU16(guid) || !m_reader.SkipBytes(16))
			{
				return false;
			}
		}

		for (;;)
		{
			if (!m_reader.Peek(peek))
			{
				return false;
			}
			if (peek == kTokenCBrace)
			{
				return m_reader.Expect(kTokenCBrace);
			}
			if (peek == kTokenOBracket)
			{
				if (!ReadOptionalPart())
				{
					return false;
				}
				continue;
			}
			if (peek == kTokenDot)
			{
				if (!ReadEllipsis())
				{
					return false;
				}
				continue;
			}
			if (!ReadMember())
			{
				return false;
			}
		}
	}

	// '[' ... ']' - an optional part, either a bare ellipsis or a member list.
	bool ReadOptionalPart()
	{
		if (!m_reader.Expect(kTokenOBracket))
		{
			return false;
		}
		for (;;)
		{
			uint16_t peek = 0;
			if (!m_reader.Peek(peek))
			{
				return false;
			}
			if (peek == kTokenCBracket)
			{
				return m_reader.Expect(kTokenCBracket);
			}
			if (peek == kTokenCBrace)
			{
				return false;  // unterminated optional part
			}
			if (peek == kTokenDot)
			{
				if (!ReadEllipsis())
				{
					return false;
				}
				continue;
			}
			if (!ReadMember())
			{
				return false;
			}
		}
	}

	// TOKEN_DOT TOKEN_DOT TOKEN_DOT
	bool ReadEllipsis()
	{
		return m_reader.Expect(kTokenDot) && m_reader.Expect(kTokenDot) &&
		       m_reader.Expect(kTokenDot);
	}

	// One member declaration. Returns false on any shape the grammar does not
	// allow, which is how a malformed template is refused.
	bool ReadMember()
	{
		uint16_t token = 0;
		if (!m_reader.ReadU16(token))
		{
			return false;
		}

		if (token == kTokenName)  // template reference, optional member name
		{
			std::string name;
			if (!m_reader.ReadName(name))
			{
				return false;
			}
			uint16_t peek = 0;
			if (m_reader.Peek(peek) && peek == kTokenName)
			{
				if (!m_reader.Expect(kTokenName) || !m_reader.ReadName(name))
				{
					return false;
				}
			}
			// RAN's writer emits template references with an inline GUID
			// inside optional parts (e.g. `[Material <GUID>]`); the GUID
			// identifies a specific instance and no semicolon follows it —
			// the closing bracket of the optional part terminates the member.
			if (m_reader.Peek(peek) && peek == kTokenGuid)
			{
				uint16_t guid = 0;
				if (!m_reader.ReadU16(guid) || !m_reader.SkipBytes(16))
				{
					return false;
				}
				return true;
			}
			return m_reader.Expect(kTokenSemicolon);
		}

		if (IsPrimitiveType(token))
		{
			uint16_t peek = 0;
			if (m_reader.Peek(peek) && peek == kTokenName)
			{
				std::string name;
				if (!m_reader.Expect(kTokenName) || !m_reader.ReadName(name))
				{
					return false;
				}
			}
			return m_reader.Expect(kTokenSemicolon);
		}

		if (token != kTokenArray)
		{
			return false;
		}

		uint16_t dataType = 0;
		if (!m_reader.ReadU16(dataType))
		{
			return false;
		}
		if (dataType == kTokenName)
		{
			std::string name;
			if (!m_reader.ReadName(name))
			{
				return false;
			}
		}
		else if (!IsPrimitiveType(dataType))
		{
			return false;
		}

		uint16_t peek = 0;
		if (m_reader.Peek(peek) && peek == kTokenName)
		{
			std::string name;
			if (!m_reader.Expect(kTokenName) || !m_reader.ReadName(name))
			{
				return false;
			}
		}
		if (!m_reader.Expect(kTokenOBracket))
		{
			return false;
		}

		uint16_t dim = 0;
		if (!m_reader.ReadU16(dim))
		{
			return false;
		}
		if (dim == kTokenName)
		{
			std::string name;
			if (!m_reader.ReadName(name))
			{
				return false;
			}
		}
		else if (dim == kTokenInteger)
		{
			uint32_t value = 0;
			if (!m_reader.ReadU32(value))
			{
				return false;
			}
		}
		else
		{
			return false;
		}

		return m_reader.Expect(kTokenCBracket) && m_reader.Expect(kTokenSemicolon);
	}

	// NAME [instance-name]* [STRING] '{' [GUID] parts '}'
	//
	// `chunk` is filled only when `name` is one of the three geometry
	// templates; every other object's numbers are walked past. Objects nest, so
	// this recurses, and the recursion is bounded rather than trusted.
	bool ReadObject(std::string& name, MeshChunk& chunk)
	{
		if (m_depth >= kMaxNestingDepth)
		{
			return false;
		}
		if (!m_reader.Expect(kTokenName) || !m_reader.ReadName(name))
		{
			return false;
		}

		// An object may carry one or more instance names, then an optional
		// string label. Both are optional, and both are followed by '{'.
		// CLIENT-RENDER-002: the first instance name is the object's own name,
		// and it is what a `MeshMaterialList` references a `Material` by, so it
		// is kept rather than discarded.
		std::string firstInstance;
		bool        haveInstance = false;
		uint16_t    peek = 0;
		for (;;)
		{
			if (!m_reader.Peek(peek))
			{
				return false;
			}
			if (peek == kTokenName)
			{
				std::string instance;
				if (!m_reader.Expect(kTokenName) || !m_reader.ReadName(instance))
				{
					return false;
				}
				if (!haveInstance)
				{
					firstInstance  = instance;
					haveInstance   = true;
				}
				continue;
			}
			if (peek == kTokenString)
			{
				std::string label;
				if (!m_reader.Expect(kTokenString) || !m_reader.ReadName(label))
				{
					return false;
				}
			}
			break;
		}

		if (!m_reader.Expect(kTokenOBrace))
		{
			return false;
		}
		if (m_reader.Peek(peek) && peek == kTokenGuid)
		{
			uint16_t guid = 0;
			if (!m_reader.ReadU16(guid) || !m_reader.SkipBytes(16))
			{
				return false;
			}
		}

const bool isMaterial = (name == kMaterialTemplate);
	const bool isTexture  = (name == kTextureFilenameTemplate);
	const bool isList     = (name == kMaterialListTemplate);

		// Capture state is set up around the body and undone after it, so a
		// nested object of the same name cannot leak a capture outwards. The
		// depth is the body's depth, which is what ReadParts compares against.
		const bool     saveIsTexture    = m_inTextureFilename;
		const bool     saveIsMaterial   = m_inMaterial;
		const bool     saveIsList       = m_inMaterialList;
		const int      saveTextureDepth = m_textureDepth;
		const int      saveListDepth    = m_listDepth;
		const std::string saveRef        = m_pendingReference;

		std::string materialName;
		std::string materialTexture;
		if (isMaterial)
		{
			materialName = firstInstance;
			materialTexture.clear();
			m_inMaterial = true;
		}
		if (isTexture)
		{
			m_inTextureFilename = true;
			m_textureDepth = m_depth + 1;
		}
		if (isList)
		{
			m_inMaterialList = true;
			m_listDepth = m_depth + 1;
			// One list at a time: the counts and face indexes collected so far
			// belong to the previous list and must not leak into this one.
			m_materialIntegersSeen = 0;
			m_pendingMaterialCount  = 0;
			m_pendingFaceIndexCount = 0;
			m_pendingFaceIndices.clear();
			m_pendingReferences.clear();
		}
		m_pendingReference.clear();

		MeshChunk* keep = IsGeometryTemplate(name) ? &chunk : nullptr;
		++m_depth;
		bool ok = ReadParts(keep);
		--m_depth;

		// A `TextureFilename` nested in a `Material` names the material's
		// texture. A `TextureFilename` anywhere else is not a character
		// material's, so it is still walked but not recorded.
		if (ok && isTexture && m_inMaterial)
		{
			m_pendingMaterialTexture = m_textureFilenameText;
			m_textureFilenameText.clear();
		}

		if (ok && isMaterial)
		{
			MeshMaterial material;
			material.name            = materialName;
			material.textureFilename = m_pendingMaterialTexture;
			RecordMaterial(material);
			// Cleared here rather than on every exit, because the nested
			// TextureFilename object has just written into it.
			m_pendingMaterialTexture.clear();
		}
if (ok && isList)
	{
		if (!AttachMaterialList())
		{
			ok = false;
		}
	}

		m_inTextureFilename = saveIsTexture;
		m_inMaterial        = saveIsMaterial;
		m_inMaterialList    = saveIsList;
		m_textureDepth      = saveTextureDepth;
		m_listDepth         = saveListDepth;
		m_pendingReference  = saveRef;

		if (ok && keep != nullptr)
		{
			Record(name, chunk);
		}
		return ok;
	}

	// Keeps one geometry object, wherever in the stream it was found. RAN nests
	// `Mesh` inside `Frame`, so recording only top-level objects would drop
	// every real mesh in the shipped tree.
	//
	// CLIENT-RENDER-002: nothing extra is recorded per mesh. Material lists are
	// collected into their own flat list and paired by position in
	// BuildMeshAsset, which is how normals and UV sets are already paired.
	void Record(const std::string& name, const MeshChunk& chunk)
	{
		if (m_geometry == nullptr)
		{
			return;
		}
		if (name == kMeshTemplate)
		{
			m_geometry->meshes.push_back(chunk);
		}
		else if (name == kMeshNormalsTemplate)
		{
			m_geometry->normals.push_back(chunk);
		}
		else if (name == kMeshTextureCoordsTempl)
		{
			m_geometry->uvs.push_back(chunk);
		}
	}

	// CLIENT-RENDER-002: the global material table. De-duplicated by instance
	// name, first declaration wins, because `m_luffy_timeskip.mxf` references
	// the same `Material__773_*` materials from two different sub-meshes and
	// both must resolve to one table entry.
	void RecordMaterial(const MeshMaterial& material)
	{
		if (m_geometry == nullptr)
		{
			return;
		}
		for (const MeshMaterial& existing : m_geometry->materials)
		{
			if (existing.name == material.name && !material.name.empty())
			{
				return;
			}
		}
		m_geometry->materials.push_back(material);
	}

	// The counts and per-face indexes of the material list just parsed, plus
	// the material names it references, become one entry of the flat list that
	// BuildMeshAsset pairs against `meshes` by position.
	bool AttachMaterialList()
	{
		if (m_geometry == nullptr)
		{
			return true;
		}
		// The declared shape must be complete: both counts present, at least one
		// material, at least one face index, and exactly as many face indexes as
		// were declared. A list missing its own counts has no face-to-material
		// assignment to preserve.
		if (m_materialIntegersSeen < 2 ||
		    m_pendingMaterialCount == 0 ||
		    m_pendingFaceIndexCount == 0 ||
		    m_pendingFaceIndices.size() != m_pendingFaceIndexCount)
		{
			return false;
		}
		// One material reference per declared slot, in slot order -- but a list
		// may name no materials at all, which is a real RAN shape and means
		// something specific rather than being malformed: D3DX hands such a mesh
		// back with NumMaterials == 0, and the engine's material loop then loads
		// no textures for it. Refusing the file would break every file that does
		// it, so an empty reference list is carried through and the sub-mesh is
		// modelled as one unnamed, untextured material. A *partial* list -- some
		// slots named, some not -- is still refused: that would leave a slot the
		// file declared but never resolved.
		if (!m_pendingReferences.empty() &&
		    m_pendingReferences.size() != m_pendingMaterialCount)
		{
			return false;
		}
		XGeometry::MaterialList list;
		list.materialCount = m_pendingMaterialCount;
		list.faceIndices   = std::move(m_pendingFaceIndices);
		list.references    = std::move(m_pendingReferences);
		m_geometry->lists.push_back(std::move(list));
		m_pendingFaceIndices.clear();
		m_pendingReferences.clear();
		return true;
	}

	// The body of a data object, up to its closing brace.
	bool ReadParts(MeshChunk* keep)
	{
		for (;;)
		{
			uint16_t token = 0;
			if (!m_reader.Peek(token))
			{
				return false;
			}
			if (token == kTokenCBrace)
			{
				return m_reader.Expect(kTokenCBrace);
			}
			if (token == kTokenName)
			{
				// ReadObject consumes the NAME token itself, so this may only
				// look at it. Consuming it here as well steps one token too far
				// and turns every nested object into a parse failure.
				std::string nested;
				MeshChunk   discard;
				if (!ReadObject(nested, discard))
				{
					return false;
				}
				continue;
			}
			if (!m_reader.ReadU16(token))
			{
				return false;
			}
			if (token == kTokenIntList || token == kTokenFloatList)
			{
				uint32_t count = 0;
				size_t   bytes = 0;
				if (!m_reader.ReadU32(count) || !m_reader.ListBytes(count, bytes))
				{
					return false;
				}
				const bool floats = (token == kTokenFloatList);

				// CLIENT-RENDER-002: RAN writes a `MeshMaterialList`'s
				// nMaterials, nFaceIndexes and every face index as ONE integer
				// list rather than as separate TOKEN_INTEGER records. Both
				// shapes are the documented grammar, so both are accepted, and
				// the count is validated against the declared nFaceIndexes
				// before any of it is used.
				const bool materialInts =
					(!floats && keep == nullptr && m_inMaterialList &&
					 m_depth == m_listDepth);

				if (keep != nullptr)
				{
					if (floats)
					{
						for (size_t i = 0; i < bytes / 4u; ++i)
						{
							float value = 0.0f;
							if (!m_reader.ReadF32(value))
							{
								return false;
							}
							keep->floats.push_back(value);
						}
					}
					else
					{
						for (size_t i = 0; i < bytes / 4u; ++i)
						{
							uint32_t value = 0;
							if (!m_reader.ReadU32(value))
							{
								return false;
							}
							keep->ints.push_back(value);
						}
					}
				}
				else if (materialInts)
				{
					for (uint32_t i = 0; i < count; ++i)
					{
						uint32_t element = 0;
						if (!m_reader.ReadU32(element) ||
						    !PushMaterialInteger(element))
						{
							return false;
						}
					}
				}
				else if (!m_reader.SkipBytes(bytes))
				{
					return false;
				}
				continue;
			}
			if (token == kTokenInteger)
			{
				uint32_t value = 0;
				if (!m_reader.ReadU32(value))
				{
					return false;
				}
				if (keep != nullptr)
				{
					keep->ints.push_back(value);
				}
				else if (m_inMaterialList && m_depth == m_listDepth)
				{
					if (!PushMaterialInteger(value))
					{
						return false;
					}
				}
				continue;
			}
			if (token == kTokenString)
			{
				std::string text;
				if (!m_reader.ReadName(text))
				{
					return false;
				}
				// CLIENT-RENDER-002: the string of a `TextureFilename` is the
				// material's texture. Only the direct body of that object is
				// captured, so a string anywhere else in the stream cannot be
				// mistaken for one.
				if (m_inTextureFilename && m_depth == m_textureDepth &&
				    m_textureFilenameText.empty())
				{
					m_textureFilenameText = text;
				}
				// A string list continues with a comma or semicolon; the grammar
				// may also end the list, in which case the token just read
				// belongs to whatever comes next.
				uint16_t separator = 0;
				if (!m_reader.ReadU16(separator))
				{
					return false;
				}
				if (separator != kTokenComma && separator != kTokenSemicolon)
				{
					if (!m_reader.Rewind(2))
					{
						return false;
					}
				}
				continue;
			}
			if (token == kTokenGuid)
			{
				if (!m_reader.SkipBytes(16))
				{
					return false;
				}
				continue;
			}
			if (token == kTokenName)
			{
				std::string nested;
				MeshChunk   discard;
				if (!ReadObject(nested, discard))
				{
					return false;
				}
				continue;
			}
			if (token == kTokenOBrace)  // data reference: '{' NAME [';'] '}'
			{
				uint16_t peek = 0;
				if (m_reader.Peek(peek) && peek == kTokenName)
				{
					std::string referenced;
					if (!m_reader.Expect(kTokenName) || !m_reader.ReadName(referenced))
					{
						return false;
					}
					// CLIENT-RENDER-002: inside a `MeshMaterialList` these are
					// the material slots, in slot order, named by the very
					// `Material` objects the file declared earlier.
					if (m_inMaterialList && m_depth == m_listDepth)
					{
						m_pendingReferences.push_back(referenced);
					}
					if (m_reader.Peek(peek) && peek == kTokenSemicolon)
					{
						if (!m_reader.Expect(kTokenSemicolon))
						{
							return false;
						}
					}
				}
				if (!m_reader.Expect(kTokenCBrace))
				{
					return false;
				}
				continue;
			}
			// A comma or semicolon is the legal separator between two values of
			// one list, and a semicolon may also close a string member, so both
			// are consumed and discarded here rather than treated as unknown.
			if (IsPrimitiveType(token) || token == kTokenOParen || token == kTokenOBracket ||
			    token == kTokenOAngle || token == kTokenDot || token == kTokenArray ||
			    token == kTokenComma || token == kTokenSemicolon)
			{
				continue;
			}
			return false;
		}
	}

	// Appends one value of a `MeshMaterialList`'s numeric header, enforcing the
	// shape as it goes: the first two values are the counts, and every one after
	// them is a face index that must be a legal material slot. A file whose
	// declared nFaceIndexes does not match the number of indexes it supplied is
	// refused here rather than producing sections that do not cover the mesh.
	//
	// RAN packs nMaterials, nFaceIndexes and the face indexes into one integer
	// list; the same values arriving one TOKEN_INTEGER at a time is the other
	// documented encoding. Both call this, so the counts are validated
	// identically whichever shape the file used.
bool PushMaterialInteger(uint32_t value)
{
	switch (m_materialIntegersSeen)
		{
		case 0:
			m_pendingMaterialCount = value;
			++m_materialIntegersSeen;
			return true;
		case 1:
			m_pendingFaceIndexCount = value;
			++m_materialIntegersSeen;
			return true;
		default:
			break;
		}

		if (m_pendingFaceIndices.size() >= m_pendingFaceIndexCount)
		{
			// More indexes than the file declared it would write.
			return false;
		}
		if (value >= m_pendingMaterialCount)
		{
			// A face assigned to a slot that does not exist. Refused rather
			// than clamped: clamping would draw the face with a material the
			// file never chose.
			return false;
		}
		m_pendingFaceIndices.push_back(value);
		++m_materialIntegersSeen;
		return true;
	}

	XReader&   m_reader;
	XGeometry* m_geometry = nullptr;
	int        m_depth = 0;

	// CLIENT-RENDER-002 capture state. The three depth-guarded flags are what
	// keep the generic grammar walk from recording a string or a name that
	// happens to appear somewhere unrelated: a value is only taken at the exact
	// nesting depth of the object whose member it is.
	bool     m_inTextureFilename = false;
	bool     m_inMaterial        = false;
	bool     m_inMaterialList    = false;
	int      m_textureDepth      = -1;
	int      m_listDepth         = -1;

	std::string m_textureFilenameText;     // the string of a TextureFilename
	std::string m_pendingMaterialTexture;  // the texture of the Material being read
	std::string m_pendingReference;        // kept only to restore across nesting

	uint32_t m_pendingMaterialCount    = 0;
	uint32_t m_pendingFaceIndexCount   = 0;
	uint32_t m_materialIntegersSeen    = 0;
	std::vector<uint32_t>    m_pendingFaceIndices;
	std::vector<std::string> m_pendingReferences;
};

// ---------------------------------------------------------------------------
// Geometry assembly
// ---------------------------------------------------------------------------

bool IsFiniteTriple(const float* values)
{
	return std::isfinite(values[0]) && std::isfinite(values[1]) && std::isfinite(values[2]);
}

// Turns the collected `Mesh` / `MeshNormals` / `MeshTextureCoords` objects into
// one mesh. See XMeshDecoder.h for each rule and the asset it is based on.
Result<MeshAsset> BuildMeshAsset(XGeometry& geometry)
{
	if (geometry.meshes.empty())
	{
		return Status(ErrorCode::InvalidArgument);
	}
	// Normals are never invented, so a mesh without a matching normal set has
	// no representation in MeshAsset and is refused.
	if (geometry.normals.size() != geometry.meshes.size())
	{
		return Status(ErrorCode::InvalidArgument);
	}
// UVs are different: a file with none at all is untextured, which MeshAsset
	// represents as (0,0). A file with *some* is claiming a correspondence to
	// the meshes, so an unmatchable set is refused rather than paired by
	// position.
	if (!geometry.uvs.empty() && geometry.uvs.size() != geometry.meshes.size())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// CLIENT-RENDER-002: material lists follow the same rule as normals. A file
	// with no `MeshMaterialList` at all is an untextured mesh, which MeshAsset
	// represents with no materials and no sections. A file with *some* is
	// claiming a correspondence to the meshes, and sections must cover every
	// index, so a count that does not match is refused rather than paired by
	// guesswork.
	const bool hasMaterialLists = !geometry.lists.empty();
	if (hasMaterialLists && geometry.lists.size() != geometry.meshes.size())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// CLIENT-RENDER-002: resolve each sub-mesh's material slot references to
	// indices in the global material table. Done after the whole stream is
	// parsed, because a list may reference a `Material` declared before *or*
	// after it; RAN declares them all in the file header, but the format does
	// not require that and the cost of not assuming it is one extra pass.
	// A reference to a name that was never declared is refused: the file asks
	// for a material whose texture cannot be known, and drawing that slot with
	// some other material would be a guess.
	//
	// The table that reaches MeshAsset holds only the materials some sub-mesh
	// actually draws with, in first-referenced order. `m_luffy_timeskip.mxf`
	// declares 21 materials and references 17 of them; the other four
	// (`Material__0`..`Material__3`, pointing at `M_sc_bsk_*.dds`) belong to
	// equipment the character is not wearing. Keeping declarations no section
	// names would add table entries with no meaning, and would break the rule
	// that a mesh's materials and its sections are both present or both absent.
	std::vector<std::vector<uint32_t>> slotTable;  // [sub-mesh][slot] -> material
	std::vector<MeshMaterial> materials;          // referenced materials, in order
	if (hasMaterialLists)
	{
		slotTable.clear();
		for (const XGeometry::MaterialList& list : geometry.lists)
		{
			std::vector<uint32_t> slots;
			slots.reserve(list.references.size());

			// A material list that named nothing becomes one shared unnamed,
			// untextured material. One shared entry rather than one per
			// sub-mesh, so several untextured sub-meshes do not present as
			// several distinct materials to whatever loads the textures.
			//
			// The slot table is indexed by the *face's material index*, which
			// the parser already proved is below this list's materialCount. So
			// every declared slot -- materialCount of them, not one -- must map
			// to the shared unnamed material; otherwise a face whose material
			// index is 1 or higher indexes past a one-entry table and reads a
			// neighbouring vector's memory. b_3boss_01.x is exactly that shape:
			// a list with materialCount > 1 and no named references.
			if (list.references.empty())
			{
				uint32_t unnamed = static_cast<uint32_t>(materials.size());
				for (size_t k = 0; k < materials.size(); ++k)
				{
					if (materials[k].name.empty() && materials[k].textureFilename.empty())
					{
						unnamed = static_cast<uint32_t>(k);
						break;
					}
				}
				if (unnamed == materials.size())
				{
					materials.push_back(MeshMaterial());
				}
				// materialCount is at least 1 (AttachMaterialList refuses 0),
				// so this loop runs at least once.
				for (uint32_t s = 0; s < list.materialCount; ++s)
				{
					slots.push_back(unnamed);
				}
				slotTable.push_back(std::move(slots));
				continue;
			}

			for (const std::string& name : list.references)
			{
				size_t found = geometry.materials.size();
				for (size_t m = 0; m < geometry.materials.size(); ++m)
				{
					if (geometry.materials[m].name == name)
					{
						found = m;
						break;
					}
				}
				if (found == geometry.materials.size())
				{
					return Status(ErrorCode::InvalidArgument);
				}

				// Map into the referenced-only table, reusing the entry when
				// this material was already reached: the real character uses the
				// same five `Material__773_*` materials from two sub-meshes, and
				// they must land on one entry so one texture upload serves both.
				uint32_t target = static_cast<uint32_t>(materials.size());
				for (size_t k = 0; k < materials.size(); ++k)
				{
					if (materials[k] == geometry.materials[found])
					{
						target = static_cast<uint32_t>(k);
						break;
					}
				}
				if (target == materials.size())
				{
					materials.push_back(geometry.materials[found]);
				}
				slots.push_back(target);
			}
			slotTable.push_back(std::move(slots));
		}
	}

	std::vector<MeshVertex> vertices;
	std::vector<MeshIndex>  indices;
	std::vector<MeshSection> sections;

	for (size_t m = 0; m < geometry.meshes.size(); ++m)
	{
		const MeshChunk& mesh    = geometry.meshes[m];
		const MeshChunk& normals = geometry.normals[m];

		if (mesh.ints.size() < 2 || normals.ints.empty())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const uint32_t vertexCount = mesh.ints[0];
		const uint32_t faceCount   = mesh.ints[1];

		if (vertexCount == 0 || vertexCount > kMaxMeshVertices)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// The position array is exactly three floats per vertex, and the
		// faceNormals list is a verbatim copy of the face list. The normal
		// array may be SHORTER than the vertex count: RAN skin files omit a
		// handful of normals on some sub-meshes (m_luffy_timeskip.mxf is
		// short by 3, 3 and 1 on three of its six sub-meshes), so requiring
		// equality refused the real character. More normals than vertices is
		// still refused -- that is a genuine mismatch, not a shortfall.
		if (mesh.floats.size() != static_cast<size_t>(vertexCount) * 3u ||
		    normals.ints[0] > vertexCount ||
		    normals.floats.size() != static_cast<size_t>(normals.ints[0]) * 3u)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		bool hasUv = false;
		if (!geometry.uvs.empty())
		{
			const MeshChunk& uvs = geometry.uvs[m];
			if (uvs.ints.empty() || uvs.ints[0] != vertexCount ||
			    uvs.floats.size() != static_cast<size_t>(vertexCount) * 2u)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			hasUv = true;
		}

		if (faceCount == 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// Form the totals in 64-bit and refuse rather than wrap.
		const uint64_t newVertices = static_cast<uint64_t>(vertices.size()) + vertexCount;
		if (newVertices > kMaxMeshVertices)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// Walk the face list to confirm its exact shape before reserving for
		// it: nFaces groups of (count, count indices), every count exactly 3.
		uint64_t triangleTotal = 0;
		{
			size_t k = 2;
			for (uint32_t f = 0; f < faceCount; ++f)
			{
				if (k >= mesh.ints.size())
				{
					return Status(ErrorCode::InvalidArgument);
				}
				const uint32_t cornerCount = mesh.ints[k++];
				if (cornerCount != 3)
				{
					// Every face in the shipped tree is a triangle, so a
					// polygon is not RAN behaviour to be accommodated; it
					// would need a fan order this format does not state.
					return Status(ErrorCode::InvalidArgument);
				}
				if (k + 3 > mesh.ints.size())
				{
					return Status(ErrorCode::InvalidArgument);
				}
				for (uint32_t c = 0; c < 3; ++c)
				{
					if (mesh.ints[k + c] >= vertexCount)
					{
						return Status(ErrorCode::InvalidArgument);
					}
				}
				k += 3;
				triangleTotal += 3u;
			}
			if (k != mesh.ints.size())
			{
				return Status(ErrorCode::InvalidArgument);  // trailing integers
			}
		}

		const uint64_t newIndices = static_cast<uint64_t>(indices.size()) + triangleTotal;
		if (newIndices > kMaxMeshIndices)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const MeshIndex base = static_cast<MeshIndex>(vertices.size());
		// CLIENT-RENDER-002: where this sub-mesh's triangles begin in the merged
		// index array. Recorded before any of them are appended, because a
		// section's indexOffset is absolute.
		const uint32_t sectionBase = static_cast<uint32_t>(indices.size());
		vertices.reserve(static_cast<size_t>(newVertices));
		for (uint32_t v = 0; v < vertexCount; ++v)
		{
			const float* p = &mesh.floats[static_cast<size_t>(v) * 3u];
			// RAN skin files may provide fewer normals than vertices. The
			// uncovered vertices still appear in the face list, so their
			// normal is genuinely absent rather than unused; zero is the
			// default D3DXLoaderX leaves for them, and it is finite, so it
			// passes MeshAsset's validation without inventing a direction.
			bool hasNormal = (v < normals.ints[0]);
			const float* n = hasNormal ? &normals.floats[static_cast<size_t>(v) * 3u] : nullptr;
			if (!IsFiniteTriple(p) || (n != nullptr && !IsFiniteTriple(n)))
			{
				return Status(ErrorCode::InvalidArgument);
			}
			float u = 0.0f;
			float w = 0.0f;
			if (hasUv)
			{
				const float* uv = &geometry.uvs[m].floats[static_cast<size_t>(v) * 2u];
				if (!std::isfinite(uv[0]) || !std::isfinite(uv[1]))
				{
					return Status(ErrorCode::InvalidArgument);
				}
				u = uv[0];
				w = uv[1];
			}
			vertices.emplace_back(
				Vector3(p[0], p[1], p[2]),
				n != nullptr ? Vector3(n[0], n[1], n[2]) : Vector3::Zero,
				u, w);
		}

		indices.reserve(static_cast<size_t>(newIndices));
		size_t k = 2;
		for (uint32_t f = 0; f < faceCount; ++f)
		{
			++k;  // the corner count, already checked to be 3
			for (uint32_t c = 0; c < 3; ++c)
			{
				indices.push_back(base + mesh.ints[k + c]);
			}
			k += 3;
		}

		// CLIENT-RENDER-002: one section per contiguous run of faces sharing a
		// material slot. Verified above that the list declares exactly faceCount
		// indexes, so this cannot run short of or past the faces just appended.
		if (hasMaterialLists)
		{
			const XGeometry::MaterialList& list = geometry.lists[m];
			if (list.faceIndices.size() != faceCount)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			uint32_t runStart = 0;
			for (uint32_t f = 1; f <= faceCount; ++f)
			{
				const bool runEnds =
					(f == faceCount) || (list.faceIndices[f] != list.faceIndices[runStart]);
				if (!runEnds)
				{
					continue;
				}
				MeshSection section;
				section.indexOffset = sectionBase + runStart * 3u;
				section.indexCount  = (f - runStart) * 3u;
				section.materialIndex = slotTable[m][list.faceIndices[runStart]];
				sections.push_back(section);
				runStart = f;
			}
		}
	}

	// CLIENT-RENDER-002: the materials are the ones some sub-mesh draws with, and
	// the sections reference them by index. CreateWithSections re-checks every
	// section against the index array, so this only has to hand over what it
	// read.
	return MeshAsset::CreateWithSections(PrimitiveTopology::TriangleList,
	                                      std::move(vertices),
	                                      std::move(indices),
	                                      std::move(materials),
	                                      std::move(sections));
}

} // namespace

// ---------------------------------------------------------------------------
// XMeshDecoder
// ---------------------------------------------------------------------------

Result<MeshAsset> XMeshDecoder::DecodeMesh(const ResourceData& data)
{
	const size_t size = data.GetSize();
	if (size < kHeaderSize)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	const uint8_t* bytes = data.GetData();
	if (bytes == nullptr)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// The header is four four-byte fields. Everything is compared as written,
	// so a header that is merely close is refused rather than interpreted.
	const bool magicOk   = std::memcmp(bytes + 0, kMagic, 4) == 0;
	const bool versionOk = std::memcmp(bytes + 4, kVersion, 4) == 0;
	const bool floatOk   = std::memcmp(bytes + 12, kFloatSize32, 4) == 0;
	const bool isBinary  = std::memcmp(bytes + 8, kBinaryTag, 4) == 0;
	const bool isBzip    = std::memcmp(bytes + 8, kBzipTag, 4) == 0;

	if (!magicOk || !versionOk || !floatOk || (!isBinary && !isBzip))
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// The token stream. For `bin` that is the rest of the file; for `bzip` it is
	// the inflated MSZip payload, which RAN writes without the file header, so
	// the header is put back in front of it.
	std::vector<uint8_t> inflated;
	const uint8_t* tokens = bytes + kHeaderSize;
	size_t         tokenCount = size - kHeaderSize;

	if (isBzip)
	{
		uint32_t expectedTotal = 0;
		for (int i = 0; i < 4; ++i)
		{
			expectedTotal |= static_cast<uint32_t>(bytes[16 + i]) << (8 * i);
		}
		if (!DecodeMsZip(bytes, size, expectedTotal, inflated))
		{
			return Status(ErrorCode::InvalidArgument);
		}
		inflated.insert(inflated.begin(), bytes, bytes + kHeaderSize);
		// The 16-byte header now sits at the front of `inflated`, so the token
		// stream starts after it. Pointing at the buffer start would hand the
		// reader the ASCII "xof 0303bzip0032" and fail on the first token.
		tokens = inflated.data() + kHeaderSize;
		tokenCount = inflated.size() - kHeaderSize;
	}

	if (tokenCount == 0)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	XReader reader(tokens, tokenCount, 0);
	XParser parser(reader);

	XGeometry geometry;
	if (!parser.ParseAll(geometry))
	{
		return Status(ErrorCode::InvalidArgument);
	}

	return BuildMeshAsset(geometry);
}

} // namespace Modern::Client
