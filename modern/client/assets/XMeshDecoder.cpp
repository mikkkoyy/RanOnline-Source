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

// ---------------------------------------------------------------------------
// Raw DEFLATE (RFC 1951), private to this translation unit
// ---------------------------------------------------------------------------
//
// Written against the RFC rather than pulled in as a dependency, so the asset
// layer keeps linking nothing beyond `Modern` and `ModernClientResources`. It
// is deliberately strict: a length or distance that would reach outside what
// the stream has actually produced, or an output longer than the caller allowed,
// is a refusal rather than a truncated read.

constexpr size_t kWindowSize = 32u * 1024u;
constexpr size_t kWindowMask = kWindowSize - 1;

struct CodeLength
{
	uint16_t count[16];
	uint16_t symbol[288];
};

// Builds the canonical Huffman decode tables of RFC 1951 3.2.2. Returns false
// for an over-subscribed or empty set, which is how a corrupt table is caught
// without decoding anything.
bool BuildHuffman(const uint8_t* lengths, size_t count, CodeLength& out)
{
	for (int i = 0; i < 16; ++i)
	{
		out.count[i] = 0;
	}
	for (size_t i = 0; i < count; ++i)
	{
		if (lengths[i] > 15)
		{
			return false;
		}
		++out.count[lengths[i]];
	}
	if (out.count[0] == count)
	{
		return false;
	}

	int left = 1;
	for (int len = 1; len < 16; ++len)
	{
		left <<= 1;
		left -= out.count[len];
		if (left < 0)
		{
			return false;  // over-subscribed
		}
	}

	uint16_t offs[16];
	offs[0] = 0;
	offs[1] = 0;
	for (int len = 1; len < 15; ++len)
	{
		offs[len + 1] = static_cast<uint16_t>(offs[len] + out.count[len]);
	}
	for (size_t i = 0; i < count; ++i)
	{
		if (lengths[i] != 0)
		{
			out.symbol[offs[lengths[i]]++] = static_cast<uint16_t>(i);
		}
	}
	return true;
}

class Inflater
{
public:
	Inflater(const uint8_t* data, size_t size, size_t outputLimit) noexcept
		: m_in(data)
		, m_inSize(data == nullptr ? 0 : size)
		, m_limit(outputLimit)
	{
		m_out.reserve(outputLimit);
		m_history.assign(kWindowSize, 0);
	}

	// The largest output this inflater may produce. Set from the caller's own
	// bound rather than from anything the stream claims.
	void SetOutputLimit(size_t limit) noexcept { m_limit = limit; }

	// Presets the window with the previous MSZip chunk. Only the last 32 KiB is
	// reachable by a back-reference, which is all DEFLATE can address, so that
	// is all that is kept.
	void SetDictionary(const std::vector<uint8_t>& dictionary)
	{
		const size_t keep = dictionary.size() < kWindowSize ? dictionary.size() : kWindowSize;
		const size_t from = dictionary.size() - keep;
		for (size_t i = 0; i < keep; ++i)
		{
			m_history[i] = dictionary[from + i];
		}
		// The window is a ring, so the write position after a full-window
		// dictionary wraps to zero. Leaving it at kWindowSize would make the
		// first Emit write one past the end of m_history.
		m_historyPos = keep & kWindowMask;
		m_historyUsed = keep;
	}

	bool Run()
	{
		uint32_t last = 0;
		do
		{
			// RFC 1951 3.2.3: every block opens with BFINAL (one bit) and then
			// BTYPE (two bits). Reading them in that order matters: the first
			// bit of a fixed-Huffman block is BFINAL, and treating it as the low
			// bit of BTYPE dispatches on the wrong type and then reads the
			// following bit as a second BFINAL.
			if (!CanRead())
			{
				return false;
			}
			last = Bits(1);
			if (!CanRead())
			{
				return false;
			}
			switch (Bits(2))
			{
			case 0: if (!Stored()) { return false; } break;
			case 1: if (!FixedBlock()) { return false; } break;
			case 2: if (!DynamicBlock()) { return false; } break;
			default: return false;
			}
		} while (last == 0);
		return true;
	}

	const std::vector<uint8_t>& Output() const noexcept { return m_out; }
	size_t ConsumedInput() const noexcept { return m_inPos; }

private:
	// Whether another bit can be read at all. Deliberately not "are N bits
	// already buffered": Bits() refills from the input, so a code longer than
	// the bits currently in the buffer is still decodable. Testing the buffer
	// instead refused the last code of a well-formed stream.
	bool CanRead() const noexcept
	{
		return !m_short && (m_bitCount > 0 || m_inPos < m_inSize);
	}

	// Reads `need` bits LSB-first. On exhaustion the failure flag is set and
	// zeroes are returned; callers check m_short so a truncated tail is refused
	// rather than read as zeroes.
	uint32_t Bits(int need) noexcept
	{
		uint32_t value = m_bitBuf;
		while (m_bitCount < need)
		{
			if (m_inPos >= m_inSize)
			{
				m_short = true;
				return 0;
			}
			value |= static_cast<uint32_t>(m_in[m_inPos++]) << m_bitCount;
			m_bitCount += 8;
		}
		m_bitBuf = value >> need;
		m_bitCount -= need;
		return value & ((1u << need) - 1u);
	}

	// Walks the canonical code bit by bit, as RFC 1951 3.2.2 describes. The
	// check is on the input, not on the buffer, so a code that ends in the last
	// byte of the stream still decodes.
	int Decode(const CodeLength& code) noexcept
	{
		int codeBits = 0;
		int first = 0;
		int index = 0;
		for (int len = 1; len < 16; ++len)
		{
			codeBits |= static_cast<int>(Bits(1));
			if (m_short)
			{
				return -1;
			}
			const int count = code.count[len];
			if (codeBits - first < count)
			{
				return code.symbol[index + (codeBits - first)];
			}
			index += count;
			first = (first + count) << 1;
			codeBits <<= 1;
		}
		return -1;
	}

	bool Emit(uint8_t value) noexcept
	{
		if (m_out.size() >= m_limit)
		{
			return false;
		}
		m_out.push_back(value);
		m_history[m_historyPos] = value;
		m_historyPos = (m_historyPos + 1) & kWindowMask;
		if (m_historyUsed < kWindowSize)
		{
			++m_historyUsed;
		}
		return true;
	}

	// Copies `length` bytes from `distance` back through the window. A distance
	// of zero, a distance further back than anything produced, and a length
	// that would run past the output limit are all refused.
	bool CopyBack(uint32_t distance, uint32_t length) noexcept
	{
		if (distance == 0 || distance > m_historyUsed)
		{
			return false;
		}
		if (static_cast<size_t>(length) > m_limit - m_out.size())
		{
			return false;
		}
		const size_t src = (m_historyPos - distance) & kWindowMask;
		for (uint32_t i = 0; i < length; ++i)
		{
			if (!Emit(m_history[(src + i) & kWindowMask]))
			{
				return false;
			}
		}
		return true;
	}

	bool Codes(const CodeLength& lit, const CodeLength& dist) noexcept
	{
		for (;;)
		{
			const int symbol = Decode(lit);
			if (symbol < 0)
			{
				return false;
			}
			if (symbol < 256)
			{
				if (!Emit(static_cast<uint8_t>(symbol)))
				{
					return false;
				}
				continue;
			}
			if (symbol == 256)
			{
				return true;
			}
			const int si = symbol - 257;
			if (si >= 29)
			{
				return false;
			}
			const uint32_t length = m_lenBase[si] + Bits(m_lenExtra[si]);
			const int dsym = Decode(dist);
			if (dsym < 0 || dsym >= 30)
			{
				return false;
			}
			const uint32_t distance = m_distBase[dsym] + Bits(m_distExtra[dsym]);
			if (m_short || !CopyBack(distance, length))
			{
				return false;
			}
		}
	}

	bool FixedBlock() noexcept
	{
		if (!m_fixedReady)
		{
			uint8_t lengths[288];
			for (int i = 0; i < 144; ++i) { lengths[i] = 8; }
			for (int i = 144; i < 256; ++i) { lengths[i] = 9; }
			for (int i = 256; i < 280; ++i) { lengths[i] = 7; }
			for (int i = 280; i < 288; ++i) { lengths[i] = 8; }
			BuildHuffman(lengths, 288, m_fixedLit);

			uint8_t dist[30];
			for (int i = 0; i < 30; ++i) { dist[i] = 5; }
			BuildHuffman(dist, 30, m_fixedDist);
			m_fixedReady = true;
		}
		return Codes(m_fixedLit, m_fixedDist);
	}

	bool Stored() noexcept
	{
		m_bitBuf = 0;
		m_bitCount = 0;
		if (m_inPos + 4 > m_inSize)
		{
			return false;
		}
		const uint32_t len = static_cast<uint32_t>(m_in[m_inPos]) |
		                     (static_cast<uint32_t>(m_in[m_inPos + 1]) << 8);
		const uint32_t nlen = static_cast<uint32_t>(m_in[m_inPos + 2]) |
		                     (static_cast<uint32_t>(m_in[m_inPos + 3]) << 8);
		if ((len ^ 0xFFFFu) != nlen)
		{
			return false;
		}
		m_inPos += 4;
		if (m_inPos + len > m_inSize)
		{
			return false;
		}
		if (len > m_limit - m_out.size())
		{
			return false;
		}
		for (uint32_t i = 0; i < len; ++i)
		{
			if (!Emit(m_in[m_inPos + i]))
			{
				return false;
			}
		}
		m_inPos += len;
		return true;
	}

	bool DynamicBlock() noexcept
	{
		static const uint8_t kOrder[19] =
			{ 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

		const uint32_t nlit  = Bits(5) + 257;
		const uint32_t ndist = Bits(5) + 1;
		const uint32_t ncode = Bits(4) + 4;
		if (m_short || nlit > 286 || ndist > 30)
		{
			return false;
		}

		uint8_t codeLengths[19];
		std::memset(codeLengths, 0, sizeof(codeLengths));
		for (uint32_t i = 0; i < ncode; ++i)
		{
			codeLengths[kOrder[i]] = static_cast<uint8_t>(Bits(3));
		}
		CodeLength codeTable;
		if (!BuildHuffman(codeLengths, 19, codeTable))
		{
			return false;
		}

		uint8_t lengths[288 + 30];
		std::memset(lengths, 0, sizeof(lengths));
		uint32_t index = 0;
		while (index < nlit + ndist)
		{
			const int symbol = Decode(codeTable);
			if (symbol < 0)
			{
				return false;
			}
			if (symbol < 16)
			{
				lengths[index++] = static_cast<uint8_t>(symbol);
			}
			else
			{
				uint8_t fill = 0;
				uint32_t repeat = 0;
				if (symbol == 16)
				{
					if (index == 0)
					{
						return false;
					}
					fill = lengths[index - 1];
					repeat = 3 + Bits(2);
				}
				else if (symbol == 17)
				{
					repeat = 3 + Bits(3);
				}
				else
				{
					repeat = 11 + Bits(7);
				}
				if (m_short || index + repeat > nlit + ndist)
				{
					return false;
				}
				while (repeat-- > 0)
				{
					lengths[index++] = fill;
				}
			}
		}
		if (lengths[256] == 0)
		{
			return false;  // no end-of-block code
		}

		CodeLength lit;
		CodeLength dist;
		if (!BuildHuffman(lengths, nlit, lit) ||
		    !BuildHuffman(lengths + nlit, ndist, dist))
		{
			return false;
		}
		return Codes(lit, dist);
	}

	static const uint16_t m_lenBase[29];
	static const uint8_t  m_lenExtra[29];
	static const uint16_t m_distBase[30];
	static const uint8_t  m_distExtra[30];

	const uint8_t* m_in;
	size_t         m_inSize;
	size_t         m_inPos    = 0;
	uint32_t       m_bitBuf   = 0;
	int            m_bitCount = 0;
	bool           m_short    = false;
	size_t         m_limit    = 0;

	std::vector<uint8_t> m_out;
	std::vector<uint8_t> m_history;
	size_t               m_historyPos  = 0;
	size_t               m_historyUsed = 0;

	CodeLength m_fixedLit{};
	CodeLength m_fixedDist{};
	bool       m_fixedReady = false;
};

const uint16_t Inflater::m_lenBase[29] =
{
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
	35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258
};
const uint8_t Inflater::m_lenExtra[29] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
	3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
};
const uint16_t Inflater::m_distBase[30] =
{
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
	257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};
const uint8_t Inflater::m_distExtra[30] =
{
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
	7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
};

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

		Inflater inf(data + pos, size - pos, budget - out.size());
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
		uint16_t peek = 0;
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

		MeshChunk* keep = IsGeometryTemplate(name) ? &chunk : nullptr;
		++m_depth;
		const bool ok = ReadParts(keep);
		--m_depth;
		if (ok && keep != nullptr)
		{
			Record(name, chunk);
		}
		return ok;
	}

	// Keeps one geometry object, wherever in the stream it was found. RAN nests
	// `Mesh` inside `Frame`, so recording only top-level objects would drop
	// every real mesh in the shipped tree.
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
				continue;
			}
			if (token == kTokenString)
			{
				std::string text;
				if (!m_reader.ReadName(text))
				{
					return false;
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

	XReader&   m_reader;
	XGeometry* m_geometry = nullptr;
	int        m_depth = 0;
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
Result<MeshAsset> BuildMeshAsset(const XGeometry& geometry)
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
	// the meshes, and RAN emits those blocks out of step in some files, so an
	// unmatchable set is refused rather than paired by position.
	if (!geometry.uvs.empty() && geometry.uvs.size() != geometry.meshes.size())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	std::vector<MeshVertex> vertices;
	std::vector<MeshIndex>  indices;

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

		// The position array is exactly three floats per vertex, and the normal
		// array the same, and RAN's faceNormals list is a copy of the face list,
		// so vertex v's normal is normals[v].
		if (mesh.floats.size() != static_cast<size_t>(vertexCount) * 3u ||
		    normals.ints[0] != vertexCount ||
		    normals.floats.size() != static_cast<size_t>(vertexCount) * 3u)
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
		vertices.reserve(static_cast<size_t>(newVertices));
		for (uint32_t v = 0; v < vertexCount; ++v)
		{
			const float* p = &mesh.floats[static_cast<size_t>(v) * 3u];
			const float* n = &normals.floats[static_cast<size_t>(v) * 3u];
			if (!IsFiniteTriple(p) || !IsFiniteTriple(n))
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
				Vector3(n[0], n[1], n[2]),
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
	}

	return MeshAsset::Create(PrimitiveTopology::TriangleList,
	                         std::move(vertices),
	                         std::move(indices));
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
