#pragma once

// CLIENT-RENDER-002: the shared DEFLATE inflater, extracted from the private
// copy that lived inside XMeshDecoder.cpp's translation unit.
//
// Two callers need raw RFC 1951 inflate and neither should carry its own:
//
//   - XMeshDecoder's `bzip` (MSZip) path, which inflates dictionary-chained
//     chunks, and
//   - PngImageDecoder, whose IDAT stream is a zlib (RFC 1950) container that
//     wraps the same RFC 1951 stream.
//
// Keeping one inflater here rather than two is what stops a second, subtly
// different decoder from appearing in the asset layer. Like the original, it is
// written against the RFC rather than pulled in as a dependency: the asset
// layer keeps linking nothing beyond `Modern` and `ModernClientResources`, so
// there is no zlib, no boost and no DirectX behind this include.
//
// It is deliberately strict: a length or distance that would reach outside what
// the stream has actually produced, a corrupt Huffman table, or an output
// longer than the caller allowed is a refusal rather than a truncated read. A
// caller passes its own output ceiling; nothing here is sized by a value the
// stream claims about itself.

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace Modern::Client
{

constexpr size_t kDeflateWindowSize = 32u * 1024u;
constexpr size_t kDeflateWindowMask = kDeflateWindowSize - 1;

inline const uint16_t kLenBase[29] =
{
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
	35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258
};
inline const uint8_t kLenExtra[29] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
	3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
};
inline const uint16_t kDistBase[30] =
{
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
	257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};
inline const uint8_t kDistExtra[30] =
{
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
	7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
};

namespace detail
{

struct DeflateCodeLength
{
	uint16_t count[16];
	uint16_t symbol[288];
};

// Builds the canonical Huffman decode tables of RFC 1951 3.2.2. Returns false
// for an over-subscribed or empty set, which is how a corrupt table is caught
// without decoding anything.
inline bool BuildDeflateHuffman(const uint8_t* lengths, size_t count, DeflateCodeLength& out)
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
		m_history.assign(kDeflateWindowSize, 0);
	}

	// The largest output this inflater may produce. Set from the caller's own
	// bound rather than from anything the stream claims.
	void SetOutputLimit(size_t limit) noexcept { m_limit = limit; }

	// Presets the window with the previous MSZip chunk. Only the last 32 KiB is
	// reachable by a back-reference, which is all DEFLATE can address, so that
	// is all that is kept. PNG does not use this: a zlib stream has no preset
	// dictionary unless the FDICT flag says so, which InflateZlib refuses.
	void SetDictionary(const std::vector<uint8_t>& dictionary)
	{
		const size_t keep = dictionary.size() < kDeflateWindowSize ? dictionary.size() : kDeflateWindowSize;
		const size_t from = dictionary.size() - keep;
		for (size_t i = 0; i < keep; ++i)
		{
			m_history[i] = dictionary[from + i];
		}
		// The window is a ring, so the write position after a full-window
		// dictionary wraps to zero. Leaving it at the window size would make
		// the first Emit write one past the end of m_history.
		m_historyPos = keep & kDeflateWindowMask;
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
	int Decode(const DeflateCodeLength& code) noexcept
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
		m_historyPos = (m_historyPos + 1) & kDeflateWindowMask;
		if (m_historyUsed < kDeflateWindowSize)
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
		const size_t src = (m_historyPos - distance) & kDeflateWindowMask;
		for (uint32_t i = 0; i < length; ++i)
		{
			if (!Emit(m_history[(src + i) & kDeflateWindowMask]))
			{
				return false;
			}
		}
		return true;
	}

	bool Codes(const DeflateCodeLength& lit, const DeflateCodeLength& dist) noexcept
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
			const uint32_t length = kLenBase[si] + Bits(kLenExtra[si]);
			const int dsym = Decode(dist);
			if (dsym < 0 || dsym >= 30)
			{
				return false;
			}
			const uint32_t distance = kDistBase[dsym] + Bits(kDistExtra[dsym]);
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
			BuildDeflateHuffman(lengths, 288, m_fixedLit);

			uint8_t dist[30];
			for (int i = 0; i < 30; ++i) { dist[i] = 5; }
			BuildDeflateHuffman(dist, 30, m_fixedDist);
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
		DeflateCodeLength codeTable;
		if (!BuildDeflateHuffman(codeLengths, 19, codeTable))
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

		DeflateCodeLength lit;
		DeflateCodeLength dist;
		if (!BuildDeflateHuffman(lengths, nlit, lit) ||
		    !BuildDeflateHuffman(lengths + nlit, ndist, dist))
		{
			return false;
		}
		return Codes(lit, dist);
	}

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

	DeflateCodeLength m_fixedLit{};
	DeflateCodeLength m_fixedDist{};
	bool              m_fixedReady = false;
};
} // namespace detail

// Inflates a raw RFC 1951 DEFLATE stream. `outputLimit` is the caller's own
// ceiling on the result; a stream that would exceed it is refused. Returns
// false for a corrupt or truncated stream. This is what XMeshDecoder's `bzip`
// (MSZip) path uses, one chunk at a time, feeding the previous chunk's output
// back in as the next chunk's preset dictionary.
inline bool InflateRaw(const uint8_t* data, size_t size, size_t outputLimit,
                       std::vector<uint8_t>& out)
{
	detail::Inflater inflater(data, size, outputLimit);
	if (!inflater.Run())
	{
		return false;
	}
	out = std::move(inflater.Output());
	return true;
}

// Inflates a zlib (RFC 1950) stream: a two-byte header, a raw DEFLATE body and
// a four-byte Adler-32 trailer. This is what PngImageDecoder feeds an IDAT
// stream to.
//
// The FDICT preset-dictionary bit is refused -- PNG only sets it for streams
// this decoder does not need to support, and honouring it here would import
// dictionary handling the asset layer does not otherwise use. The Adler-32 is
// verified so a truncated or corrupted body is not accepted as a valid image.
//
// `outputLimit` is the caller's ceiling on the decompressed size, formed by the
// caller in 64-bit arithmetic from the PNG's own dimensions, so a hostile
// stream cannot size an allocation by its own claim.
inline bool InflateZlib(const uint8_t* data, size_t size, size_t outputLimit,
                        std::vector<uint8_t>& out)
{
	if (data == nullptr || size < 6)
	{
		return false;  // 2-byte header + at least an empty block + 4-byte Adler
	}
	const uint8_t cmf = data[0];
	const uint8_t flg = data[1];
	const unsigned method = cmf & 0x0Fu;
	const unsigned fdict = (flg >> 5) & 0x01u;
	if (method != 8)          // only DEFLATE is defined
	{
		return false;
	}
	if (((static_cast<unsigned>(cmf) << 8) | flg) % 31u != 0u)  // header checksum
	{
		return false;
	}
	if (fdict != 0)           // preset dictionary: refused (see above)
	{
		return false;
	}

	// The DEFLATE body is everything between the 2-byte header and the 4-byte
	// Adler-32 trailer. A one-byte slack is left so a stream that stops exactly
	// at its final byte still decodes: the inflater reads a whole byte at a
	// time and may look one byte past the logical end of the last block.
	detail::Inflater inflater(data + 2, size - 6, outputLimit);
	if (!inflater.Run())
	{
		return false;
	}

	// Adler-32 of the decompressed bytes (RFC 1950 2.2), compared against the
	// trailer the stream carries. A mismatch means the body did not survive
	// intact, which is exactly the corruption PNG's per-chunk CRC is there to
	// catch at a finer grain.
	std::vector<uint8_t> result = std::move(inflater.Output());
	uint32_t a = 1;
	uint32_t b = 0;
	for (const uint8_t byte : result)
	{
		a = (a + byte) % 65521u;
		b = (b + a) % 65521u;
	}
	const uint32_t adler = (b << 16) | a;
	const uint32_t stored = (static_cast<uint32_t>(data[size - 4]) << 24) |
	                        (static_cast<uint32_t>(data[size - 3]) << 16) |
	                        (static_cast<uint32_t>(data[size - 2]) << 8) |
	                         static_cast<uint32_t>(data[size - 1]);
	if (adler != stored)
	{
		return false;
	}

	out = std::move(result);
	return true;
}

} // namespace Modern::Client





