#include "assets/PngImageDecoder.h"

#include "assets/AssetTypes.h"
#include "assets/ImageAsset.h"
#include "assets/Inflate.h"
#include "resources/ResourceData.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Modern::Client
{
namespace
{

// The eight-byte PNG signature: 137 80 78 71 13 10 26 10. Every PNG file in the
// world opens with it, and the trailing 26 10 also catches a file that has been
// newline-mangled in transit, which is why the whole eight bytes are compared.
const uint8_t kPngSignature[8] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };

// A PNG chunk is a 4-byte big-endian length, a 4-byte type, `length` bytes of
// data and a 4-byte CRC. This bounds one chunk's declared length so a corrupt
// length cannot ask for an enormous allocation; the real files' largest IDAT is
// 8 KiB and IHDR is 13.
constexpr uint32_t kMaxChunkLength = 16u * 1024u * 1024u;

// IHDR is always the first chunk and is exactly 13 bytes:
//   width (4), height (4), bitDepth (1), colourType (1), compression (1),
//   filter (1), interlace (1)   -- all big-endian where multi-byte.
constexpr size_t   kIhdrLength = 13;
constexpr uint8_t  kColourTypeRgb  = 2;  // truecolour, no alpha, 3 bytes/px
constexpr uint8_t  kColourTypeRgba = 6;  // truecolour + alpha, 4 bytes/px
constexpr uint8_t  kBitDepth8      = 8;
constexpr uint8_t  kInterlaceNone  = 0;

// Reads a big-endian uint32 at `offset`. PNG is big-endian throughout, unlike
// the little-endian DDS and RAN containers.
uint32_t ReadBe32(const uint8_t* p) noexcept
{
	return (static_cast<uint32_t>(p[0]) << 24) |
	       (static_cast<uint32_t>(p[1]) << 16) |
	       (static_cast<uint32_t>(p[2]) << 8) |
	        static_cast<uint32_t>(p[3]);
}

bool IsChunkType(const uint8_t* type, char a, char b, char c, char d) noexcept
{
	return type[0] == a && type[1] == b && type[2] == c && type[3] == d;
}

// The CRC-32 (IEEE 802.3, the one PNG uses) of a chunk's type+data. IHDR is the
// one chunk this decoder checks; the stream's integrity is otherwise covered by
// the zlib Adler-32 over the whole IDAT. The table is built once, lazily, so no
// static initialiser order matters and the header stays self-contained.
uint32_t Crc32(const uint8_t* data, size_t length) noexcept
{
	static uint32_t table[256];
	static bool ready = false;
	if (!ready)
	{
		for (uint32_t n = 0; n < 256; ++n)
		{
			uint32_t c = n;
			for (int k = 0; k < 8; ++k)
			{
				c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
			}
			table[n] = c;
		}
		ready = true;
	}
	uint32_t crc = 0xFFFFFFFFu;
	for (size_t i = 0; i < length; ++i)
	{
		crc = table[(crc ^ data[i]) & 0xFFu] ^ (crc >> 8);
	}
	return crc ^ 0xFFFFFFFFu;
}

// The Paeth predictor of PNG specification section 9.2. `a` is left, `b` is
// above, `c` is above-left; all are the already-reconstructed bytes of the
// neighbouring pixels. Returns the prediction of the current byte.
int Paeth(int a, int b, int c) noexcept
{
	const int p = a + b - c;
	const int pa = p > a ? p - a : a - p;
	const int pb = p > b ? p - b : b - p;
	const int pc = p > c ? p - c : c - p;
	if (pa <= pb && pa <= pc) { return a; }
	if (pb <= pc) { return b; }
	return c;
}

} // namespace

Result<ImageAsset> PngImageDecoder::DecodeImage(const ResourceData& data)
{
	const size_t size = data.GetSize();
	const uint8_t* bytes = data.GetData();
	if (bytes == nullptr || size < 8 + kIhdrLength + 4)
	{
		// Too small to even hold the signature and a minimal chunk.
		return Status(ErrorCode::InvalidArgument);
	}
	for (int i = 0; i < 8; ++i)
	{
		if (bytes[i] != kPngSignature[i])
		{
			return Status(ErrorCode::InvalidArgument);
		}
	}

	// Walk the chunks once, collecting the concatenated IDAT payload and the
	// IHDR fields. Every other chunk is skipped by its declared length. The
	// first chunk must be IHDR and there must be exactly one; a second IHDR, or
	// an IHDR that is not first, is refused rather than the last one winning.
	bool     haveIhdr = false;
	bool     sawIend  = false;
	uint32_t width = 0;
	uint32_t height = 0;
	uint8_t  bitDepth = 0;
	uint8_t  colourType = 0;
	uint8_t  interlace = 0;
	std::vector<uint8_t> idat;

	size_t offset = 8;
	while (offset + 8 <= size)
	{
		const uint32_t chunkLength = ReadBe32(bytes + offset);
		const uint8_t* chunkType = bytes + offset + 4;
		const size_t   dataStart = offset + 8;
		// A chunk's data plus its 4-byte CRC must fit in the file. chunkLength
		// is bounded so the size_t arithmetic below cannot overflow.
		if (chunkLength > kMaxChunkLength)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (dataStart + chunkLength + 4 > size)
		{
			return Status(ErrorCode::InvalidArgument);  // truncated chunk
		}
		const uint8_t* chunkData = bytes + dataStart;

		if (IsChunkType(chunkType, 'I', 'H', 'D', 'R'))
		{
			if (haveIhdr || offset != 8 || chunkLength != kIhdrLength)
			{
				// IHDR must be the first chunk, exactly once, exactly 13 bytes.
				return Status(ErrorCode::InvalidArgument);
			}
			// The IHDR CRC is checked here, at the point of reading, so a
			// corrupted header is refused before any of its fields are used to
			// size an allocation.
			const uint32_t crc = ReadBe32(bytes + dataStart + kIhdrLength);
			if (Crc32(chunkType, 4 + kIhdrLength) != crc)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			width      = ReadBe32(chunkData + 0);
			height     = ReadBe32(chunkData + 4);
			bitDepth   = chunkData[8];
			colourType = chunkData[9];
			interlace  = chunkData[12];  // interlace byte, last of IHDR
			haveIhdr = true;
		}
		else if (IsChunkType(chunkType, 'I', 'D', 'A', 'T'))
		{
			// The image data. Concatenated across every IDAT chunk; the set is
			// inflated together as one zlib stream after the walk.
			idat.insert(idat.end(), chunkData, chunkData + chunkLength);
		}
		// Every other chunk type -- IEND, pHYs, gAMA, cHRM, sRGB, tEXt, and any
		// unknown chunk -- is skipped by its length. Its CRC is not checked;
		// ancillary data does not affect the decoded pixels.

		const bool isEnd = IsChunkType(chunkType, 'I', 'E', 'N', 'D');
		offset = dataStart + chunkLength + 4;
		if (isEnd)
		{
			sawIend = true;
			break;
		}
	}

	// IEND is mandatory and last (PNG spec 4.10, 11.2.1). A stream whose bytes
	// stopped after IDAT -- everything but the terminator -- decodes its pixels
	// but is a file that was still arriving, and is refused rather than accepted:
	// the missing IEND is exactly the signal that the file is incomplete.
	if (!haveIhdr || !sawIend || idat.empty())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// The supported subset, stated as refusals with reasons.
	if (bitDepth != kBitDepth8)
	{
		return Status(ErrorCode::InvalidArgument);
	}
	if (colourType != kColourTypeRgb && colourType != kColourTypeRgba)
	{
		return Status(ErrorCode::InvalidArgument);
	}
	if (interlace != kInterlaceNone)
	{
		return Status(ErrorCode::InvalidArgument);
	}


	// Bytes per pixel in the source scanline: RGB is 3, RGBA is 4.
	const size_t channels = (colourType == kColourTypeRgba) ? 4u : 3u;
	// Each filtered scanline is one filter byte followed by width * channels
	// raw bytes. Formed in 64-bit so a header claiming enormous dimensions is
	// refused by ComputeImageByteCount rather than wrapping here.
	const uint64_t stride64 = 1u + static_cast<uint64_t>(width) * channels;
	const uint64_t rawSize64 = stride64 * static_cast<uint64_t>(height);

	// The asset layer's own ceilings, applied before anything is allocated.
	const Result<size_t> decodedBytes =
		ComputeImageByteCount(width, height, ImageFormat::R8G8B8A8_UNorm);
	if (decodedBytes.IsError())
	{
		return decodedBytes.GetStatus();
	}
	// The inflated stream is exactly height scanlines of `stride` bytes. Its
	// size is the caller's ceiling for the inflater, so a stream that would
	// produce more is refused rather than allocated.
	if (rawSize64 == 0 || rawSize64 > decodedBytes.GetValue() + static_cast<uint64_t>(height))
	{
		return Status(ErrorCode::InvalidArgument);
	}

	std::vector<uint8_t> raw;
	if (!InflateZlib(idat.data(), idat.size(), static_cast<size_t>(rawSize64), raw))
	{
		return Status(ErrorCode::InvalidArgument);
	}
	// The inflated stream must be exactly the scanlines the header describes.
	// Short means truncated; long means trailing bytes this decoder does not
	// interpret, and accepting either would be a silent misread.
	if (raw.size() != static_cast<size_t>(rawSize64))
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// Reconstruct the scanlines (undo the per-scanline filter) straight into the
	// RGBA output, expanding RGB to RGBA with opaque alpha as it goes.
	const size_t stride = static_cast<size_t>(stride64);
	std::vector<uint8_t> pixels(decodedBytes.GetValue());

	// `prev` is the reconstructed previous scanline, `cur` the current one, both
	// in raw (source channel) form. Filters reference bytes to the left, above
	// and above-left, so the previous reconstructed row is needed in full.
	std::vector<uint8_t> prev(stride - 1, 0);  // width * channels, no filter byte
	std::vector<uint8_t> cur(stride - 1, 0);

	for (uint32_t y = 0; y < height; ++y)
	{
		const uint8_t* filtered = raw.data() + static_cast<size_t>(y) * stride;
		const uint8_t  filterType = filtered[0];
		const uint8_t* src = filtered + 1;
		const size_t   rowBytes = stride - 1;

		for (size_t i = 0; i < rowBytes; ++i)
		{
			const int left = (i >= channels) ? cur[i - channels] : 0;
			const int up = prev[i];
			const int upLeft = (i >= channels) ? prev[i - channels] : 0;
			int value = 0;
			switch (filterType)
			{
			case 0: value = src[i]; break;                           // None
			case 1: value = src[i] + left; break;                    // Sub
			case 2: value = src[i] + up; break;                      // Up
			case 3: value = src[i] + ((left + up) / 2); break;       // Average
			case 4: value = src[i] + Paeth(left, up, upLeft); break; // Paeth
			default: return Status(ErrorCode::InvalidArgument);      // unknown filter
			}
			cur[i] = static_cast<uint8_t>(value & 0xFF);
		}

		// Expand the reconstructed row into RGBA. RGB gains opaque alpha; RGBA
		// keeps its own, so transparent pixels survive.
		uint8_t* dst = pixels.data() + static_cast<size_t>(y) * width * 4u;
		for (uint32_t x = 0; x < width; ++x)
		{
			dst[x * 4 + 0] = cur[x * channels + 0];
			dst[x * 4 + 1] = cur[x * channels + 1];
			dst[x * 4 + 2] = cur[x * channels + 2];
			dst[x * 4 + 3] = (channels == 4) ? cur[x * channels + 3] : 0xFF;
		}

		std::swap(prev, cur);
	}

	return ImageAsset::Create(width, height, ImageFormat::R8G8B8A8_UNorm, std::move(pixels));
}

} // namespace Modern::Client

