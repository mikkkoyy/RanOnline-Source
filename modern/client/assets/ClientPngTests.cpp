// CLIENT-RENDER-002: real PNG decoder and texture loader boundary tests.
//
// The ASURA character's two shipped textures are PNG, so this suite pins the
// decoder that reads them and the loader that finds them. Three things are
// covered: that supported PNGs decode to correct *pixels* (not merely an
// accepted header) across both colour types and all five scanline filters,
// that every malformed shape is refused with InvalidArgument, and that the
// texture loader resolves bare material names the way the legacy
// TextureManager did -- case-blind, .mtf-first -- without a renderer, a
// device or any legacy library.
//
// The fixtures are written here, by hand, from the published format
// descriptions -- PNG spec chapters 3 and 9, RFC 1951 (DEFLATE), RFC 1950
// (zlib), and the IEEE CRC-32 / Adler-32 definitions. The zlib streams are
// stored (uncompressed) DEFLATE blocks assembled byte by byte: nothing in this
// file calls the decoder to produce input for the decoder. That independence
// is the point: a decoder checked against its own output proves nothing.

#include "TestHarness.h"

#include "assets/AssetTypes.h"
#include "assets/DdsImageDecoder.h"
#include "assets/ImageAsset.h"
#include "assets/ImageDecoder.h"
#include "assets/MtfTextureTransform.h"
#include "assets/PngImageDecoder.h"
#include "assets/TextureLoader.h"
#include "resources/ResourceData.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

using namespace Modern;
using namespace Modern::Client;

namespace
{

// ---------------------------------------------------------------------------
// Checksums, written from their definitions
// ---------------------------------------------------------------------------

// CRC-32 with the IEEE 802.3 reflected polynomial 0xEDB88320 -- the one PNG
// uses for every chunk. The table is built on first use.
uint32_t Crc32(const uint8_t* data, size_t length)
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

// Adler-32 (RFC 1950 2.2): two running sums modulo 65521, combined into one
// big-endian word. This is the trailer InflateZlib verifies.
uint32_t Adler32(const std::vector<uint8_t>& data)
{
	uint32_t a = 1;
	uint32_t b = 0;
	for (const uint8_t byte : data)
	{
		a = (a + byte) % 65521u;
		b = (b + a) % 65521u;
	}
	return (b << 16) | a;
}

// ---------------------------------------------------------------------------
// Byte assembly helpers
// ---------------------------------------------------------------------------

void PushU32Le(std::vector<uint8_t>& bytes, uint32_t value)
{
	bytes.push_back(static_cast<uint8_t>(value & 0xFFu));
	bytes.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
	bytes.push_back(static_cast<uint8_t>((value >> 16) & 0xFFu));
	bytes.push_back(static_cast<uint8_t>((value >> 24) & 0xFFu));
}

void PushU32Be(std::vector<uint8_t>& bytes, uint32_t value)
{
	bytes.push_back(static_cast<uint8_t>((value >> 24) & 0xFFu));
	bytes.push_back(static_cast<uint8_t>((value >> 16) & 0xFFu));
	bytes.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
	bytes.push_back(static_cast<uint8_t>(value & 0xFFu));
}

// A zlib stream around `deflateBody`-shaped data: the 2-byte header
// (CM=8 DEFLATE, CINFO=7 32 KiB window, FCHECK making the header word
// divisible by 31, FLEVEL=0), then the DEFLATE body, then the big-endian
// Adler-32 of the *uncompressed* bytes. `deflateBody` is raw RFC 1951; the
// helper only wraps it.
std::vector<uint8_t> WrapZlib(const std::vector<uint8_t>& deflateBody,
                              const std::vector<uint8_t>& uncompressed)
{
	std::vector<uint8_t> stream;
	stream.push_back(0x78);  // CM=8, CINFO=7
	stream.push_back(0x01);  // FLEVEL=0, FDICT=0, FCHECK=1 (0x7801 % 31 == 0)
	stream.insert(stream.end(), deflateBody.begin(), deflateBody.end());
	PushU32Be(stream, Adler32(uncompressed));
	return stream;
}

// One stored (BTYPE=00) final DEFLATE block around rawBytes: the single
// header byte 0x01 (BFINAL=1, BTYPE=00, then the spec's pad-to-byte-boundary),
// the little-endian LEN, its ones-complement NLEN, and the bytes themselves.
// RFC 1951 3.2.4. Stored blocks are what make a fixture independent of any
// compressor: the bytes on the wire are the bytes in this function.
std::vector<uint8_t> DeflateStored(const std::vector<uint8_t>& rawBytes)
{
	std::vector<uint8_t> block;
	block.push_back(0x01);
	const uint16_t len = static_cast<uint16_t>(rawBytes.size());
	const uint16_t nlen = static_cast<uint16_t>(~len);
	PushU32Le(block, static_cast<uint32_t>(len) | (static_cast<uint32_t>(nlen) << 16));
	block.insert(block.end(), rawBytes.begin(), rawBytes.end());
	return block;
}

// One PNG chunk: big-endian length, the four type bytes, the data, and the
// CRC-32 of type+data (the CRC covers the type too -- PNG spec 5.2).
void AppendChunk(std::vector<uint8_t>& png, const char* type,
                 const std::vector<uint8_t>& data)
{
	PushU32Be(png, static_cast<uint32_t>(data.size()));
	const size_t crcStart = png.size();
	png.insert(png.end(), type, type + 4);
	png.insert(png.end(), data.begin(), data.end());
	PushU32Be(png, Crc32(png.data() + crcStart, 4 + data.size()));
}

const std::vector<uint8_t>& PngSignature()
{
	static const std::vector<uint8_t> signature =
		{ 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
	return signature;
}

// IHDR payload, all configurable: width/height big-endian, then bit depth,
// colour type, compression (always 0), filter (always 0) and interlace.
std::vector<uint8_t> MakeIhdr(uint32_t width, uint32_t height,
                              uint8_t colourType = 2, uint8_t bitDepth = 8,
                              uint8_t interlace = 0)
{
	std::vector<uint8_t> ihdr;
	PushU32Be(ihdr, width);
	PushU32Be(ihdr, height);
	ihdr.push_back(bitDepth);
	ihdr.push_back(colourType);
	ihdr.push_back(0);  // compression method: only 0 is defined
	ihdr.push_back(0);  // filter method: only 0 is defined
	ihdr.push_back(interlace);
	return ihdr;
}

// The Paeth predictor of PNG specification section 9.2, restated here so the
// *fixtures* do not call the decoder they are testing: `a` is left, `b` is
// above, `c` is above-left. A fixture encoder that shares code with the
// decoder under test proves nothing.
int PaethPredict(int a, int b, int c)
{
	const int p = a + b - c;
	const int pa = p > a ? p - a : a - p;
	const int pb = p > b ? p - b : b - p;
	const int pc = p > c ? p - c : c - p;
	if (pa <= pb && pa <= pc) { return a; }
	if (pb <= pc) { return b; }
	return c;
}

// Encodes one scanline's raw bytes with the given filter, against the previous
// raw scanline (all zeros for the first row). The mirror of the decoder's
// reconstruction loop, written independently from the specification.
std::vector<uint8_t> EncodeScanline(const std::vector<uint8_t>& raw,
                                    const std::vector<uint8_t>& prev,
                                    unsigned channels, uint8_t filter)
{
	const size_t rowBytes = raw.size();
	std::vector<uint8_t> out;
	out.reserve(rowBytes + 1);
	out.push_back(filter);
	for (size_t i = 0; i < rowBytes; ++i)
	{
		const int left = (i >= channels) ? raw[i - channels] : 0;
		const int up = static_cast<int>(prev[i]);
		const int upLeft = (i >= channels) ? prev[i - channels] : 0;
		int predicted = 0;
		switch (filter)
		{
		case 0: predicted = 0; break;
		case 1: predicted = left; break;
		case 2: predicted = up; break;
		case 3: predicted = (left + up) / 2; break;
		case 4: predicted = PaethPredict(left, up, upLeft); break;
		default: predicted = 0; break;
		}
		out.push_back(static_cast<uint8_t>((raw[i] - predicted) & 0xFF));
	}
	return out;
}

// Assembles a complete PNG: signature, IHDR, the IDAT payload split across
// `idatSplit` chunks (1 = one chunk; the decoder must concatenate), and IEND.
// An optional tEXt chunk is inserted before IDAT when `withText` is true, so a
// case can prove ancillary chunks are skipped rather than choking the decode.
// The IDAT bytes are the *zlib stream*, already wrapped by the caller.
std::vector<uint8_t> MakePng(const std::vector<uint8_t>& ihdr,
                             const std::vector<uint8_t>& zlibStream,
                             size_t idatSplit = 1, bool withText = false)
{
	std::vector<uint8_t> png = PngSignature();
	AppendChunk(png, "IHDR", ihdr);
	if (withText)
	{
		const std::vector<uint8_t> text = { 't', 'e', 's', 't', 0, 'v', 'a', 'l' };
		AppendChunk(png, "tEXt", text);
	}
	size_t offset = 0;
	for (size_t part = 0; part < idatSplit; ++part)
	{
		const size_t remaining = zlibStream.size() - offset;
		const size_t partsLeft = idatSplit - part;
		const size_t take = remaining / partsLeft;
		const std::vector<uint8_t> piece(zlibStream.begin() + static_cast<std::ptrdiff_t>(offset),
		                                 zlibStream.begin() + static_cast<std::ptrdiff_t>(offset + take));
		AppendChunk(png, "IDAT", piece);
		offset += take;
	}
	const std::vector<uint8_t> empty;
	AppendChunk(png, "IEND", empty);
	return png;
}

// Compares one RGBA pixel of a decoded image. The layout is the one
// ImageAsset.h states: row-major, tightly packed, top row first.
void CheckPixel(const ImageAsset& image, uint32_t x, uint32_t y,
                uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	const size_t at = (static_cast<size_t>(y) * image.GetWidth() + x) * 4u;
	const std::vector<uint8_t>& px = image.GetPixels();
	CHECK_EQ(px[at + 0], r);
	CHECK_EQ(px[at + 1], g);
	CHECK_EQ(px[at + 2], b);
	CHECK_EQ(px[at + 3], a);
}

} // namespace

// ---------------------------------------------------------------------------
// Positive: truecolour decodes to stated pixels
// ---------------------------------------------------------------------------

MODERN_TEST(ClientPng_RgbDecodesToStatedPixels)
{
	// 2x2 RGB, one stored block, filter None on both rows. The values are
	// chosen so every channel differs per pixel: a swapped channel shows.
	const std::vector<uint8_t> row0 = { 10, 20, 30, 40, 50, 60 };
	const std::vector<uint8_t> row1 = { 70, 80, 90, 100, 110, 120 };
	const std::vector<uint8_t> zeros(6, 0);
	std::vector<uint8_t> scanlines = EncodeScanline(row0, zeros, 3, 0);
	const std::vector<uint8_t> row1Enc = EncodeScanline(row1, row0, 3, 0);
	scanlines.insert(scanlines.end(), row1Enc.begin(), row1Enc.end());

	const std::vector<uint8_t> png =
	    MakePng(MakeIhdr(2, 2, 2), WrapZlib(DeflateStored(scanlines), scanlines));

	PngImageDecoder decoder;
	const Result<ImageAsset> decoded = decoder.DecodeImage(ResourceData(png));
	REQUIRE(decoded.IsOk());
	const ImageAsset& image = decoded.GetValue();
	CHECK_EQ(image.GetWidth(), 2u);
	CHECK_EQ(image.GetHeight(), 2u);
	// The PNG result is always RGBA: RGB gains opaque alpha.
	CHECK(image.GetFormat() == ImageFormat::R8G8B8A8_UNorm);
	CHECK_EQ(image.GetPixelByteCount(), 16u);
	CheckPixel(image, 0, 0, 10, 20, 30, 255);
	CheckPixel(image, 1, 0, 40, 50, 60, 255);
	CheckPixel(image, 0, 1, 70, 80, 90, 255);
	CheckPixel(image, 1, 1, 100, 110, 120, 255);
}

MODERN_TEST(ClientPng_RgbaKeepsAlpha)
{
	// 2x1 RGBA with a transparent and a semi-transparent pixel: the decoder
	// must keep the file's alpha, not force it opaque.
	const std::vector<uint8_t> row =
	    { 200, 100, 50, 0, 10, 20, 30, 128 };
	const std::vector<uint8_t> zeros(8, 0);
	const std::vector<uint8_t> scanlines = EncodeScanline(row, zeros, 4, 0);

	const std::vector<uint8_t> png =
	    MakePng(MakeIhdr(2, 1, 6), WrapZlib(DeflateStored(scanlines), scanlines));

	PngImageDecoder decoder;
	const Result<ImageAsset> decoded = decoder.DecodeImage(ResourceData(png));
	REQUIRE(decoded.IsOk());
	const ImageAsset& image = decoded.GetValue();
	CHECK_EQ(image.GetWidth(), 2u);
	CHECK_EQ(image.GetHeight(), 1u);
	CheckPixel(image, 0, 0, 200, 100, 50, 0);
	CheckPixel(image, 1, 0, 10, 20, 30, 128);
}

// P1A
// All five filters
MODERN_TEST(ClientPng_AllFiveFiltersDecodeToSamePixels)
{
	const std::vector<uint8_t> raw = { 90, 15, 200, 91, 16, 201, 92, 17, 202 };
	const std::vector<uint8_t> zeros(raw.size(), 0);

	PngImageDecoder decoder;
	for (uint8_t filter = 0; filter <= 4; ++filter)
	{
		const std::vector<uint8_t> scanlines = EncodeScanline(raw, zeros, 3, filter);
		const std::vector<uint8_t> png =
		    MakePng(MakeIhdr(3, 1, 2), WrapZlib(DeflateStored(scanlines), scanlines));
		const Result<ImageAsset> decoded = decoder.DecodeImage(ResourceData(png));
		REQUIRE(decoded.IsOk());
		const ImageAsset& image = decoded.GetValue();
		CheckPixel(image, 0, 0, 90, 15, 200, 255);
		CheckPixel(image, 1, 0, 91, 16, 201, 255);
		CheckPixel(image, 2, 0, 92, 17, 202, 255);
	}
}

MODERN_TEST(ClientPng_MultiRowFiltersUsePreviousRow)
{
	const std::vector<uint8_t> row0 = { 50, 60, 70, 80, 90, 100 };
	const std::vector<uint8_t> row1 = { 51, 61, 71, 81, 91, 101 };
	const std::vector<uint8_t> zeros(row0.size(), 0);

	PngImageDecoder decoder;
	for (uint8_t filter = 0; filter <= 4; ++filter)
	{
		std::vector<uint8_t> scanlines = EncodeScanline(row0, zeros, 3, filter);
		const std::vector<uint8_t> row1Enc = EncodeScanline(row1, row0, 3, filter);
		scanlines.insert(scanlines.end(), row1Enc.begin(), row1Enc.end());
		const std::vector<uint8_t> png =
		    MakePng(MakeIhdr(2, 2, 2), WrapZlib(DeflateStored(scanlines), scanlines));
		const Result<ImageAsset> decoded = decoder.DecodeImage(ResourceData(png));
		REQUIRE(decoded.IsOk());
		const ImageAsset& image = decoded.GetValue();
		CheckPixel(image, 0, 0, 50, 60, 70, 255);
		CheckPixel(image, 1, 0, 80, 90, 100, 255);
		CheckPixel(image, 0, 1, 51, 61, 71, 255);
		CheckPixel(image, 1, 1, 81, 91, 101, 255);
	}
}

MODERN_TEST(ClientPng_SplitIdatConcatenates)
{
	const std::vector<uint8_t> row = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	const std::vector<uint8_t> zeros(row.size(), 0);
	const std::vector<uint8_t> scanlines = EncodeScanline(row, zeros, 3, 0);
	const std::vector<uint8_t> zlib = WrapZlib(DeflateStored(scanlines), scanlines);

	PngImageDecoder decoder;
	const Result<ImageAsset> a =
	    decoder.DecodeImage(ResourceData(MakePng(MakeIhdr(3, 1, 2), zlib, 1)));
	const Result<ImageAsset> b =
	    decoder.DecodeImage(ResourceData(MakePng(MakeIhdr(3, 1, 2), zlib, 3)));
	REQUIRE(a.IsOk());
	REQUIRE(b.IsOk());
	CHECK(a.GetValue().GetPixels() == b.GetValue().GetPixels());
}

MODERN_TEST(ClientPng_AncillaryChunkIsSkipped)
{
	const std::vector<uint8_t> row = { 5, 6, 7 };
	const std::vector<uint8_t> zeros(3, 0);
	const std::vector<uint8_t> scanlines = EncodeScanline(row, zeros, 3, 0);
	const std::vector<uint8_t> zlib = WrapZlib(DeflateStored(scanlines), scanlines);

	PngImageDecoder decoder;
	const Result<ImageAsset> decoded =
	    decoder.DecodeImage(ResourceData(MakePng(MakeIhdr(1, 1, 2), zlib, 1, true)));
	REQUIRE(decoded.IsOk());
	CheckPixel(decoded.GetValue(), 0, 0, 5, 6, 7, 255);
}

// P1B
// Negative: structural damage
MODERN_TEST(ClientPng_GarbageSignatureIsRefused)
{
	PngImageDecoder decoder;
	CHECK_EQ(decoder.DecodeImage(ResourceData(std::vector<uint8_t>(64, 0))).GetError(),
	         ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientPng_TruncationIsRefused)
{
	const std::vector<uint8_t> row = { 9, 8, 7, 6, 5, 4 };
	const std::vector<uint8_t> zeros(row.size(), 0);
	const std::vector<uint8_t> scanlines = EncodeScanline(row, zeros, 3, 0);
	const std::vector<uint8_t> png =
	    MakePng(MakeIhdr(2, 1, 2), WrapZlib(DeflateStored(scanlines), scanlines));

	PngImageDecoder decoder;
	REQUIRE(decoder.DecodeImage(ResourceData(png)).IsOk());
	// Every strict prefix is a file that stopped arriving: refused, all.
	for (size_t cut = 8; cut < png.size(); ++cut)
	{
		const std::vector<uint8_t> prefix(png.begin(), png.begin() + static_cast<std::ptrdiff_t>(cut));
		if (decoder.DecodeImage(ResourceData(prefix)).IsOk())
		{
			CHECK_EQ(cut, png.size());
			break;
		}
	}
}

// P1C
// Negative: header semantics
MODERN_TEST(ClientPng_UnsupportedHeadersAreRefused)
{
	PngImageDecoder decoder;
	const std::vector<uint8_t> row = { 1, 2, 3 };
	const std::vector<uint8_t> zeros(3, 0);
	const std::vector<uint8_t> scanlines = EncodeScanline(row, zeros, 3, 0);
	const std::vector<uint8_t> zlib = WrapZlib(DeflateStored(scanlines), scanlines);

	// palette, greyscale, interlaced, non-8-bit: each refused for its own
	// stated reason in the decoder.
	const std::vector<uint8_t> badColour[] = {
		MakeIhdr(1, 1, 3), MakeIhdr(1, 1, 0), MakeIhdr(1, 1, 2, 8, 1), MakeIhdr(1, 1, 2, 4)
	};
	for (const std::vector<uint8_t>& ihdr : badColour)
	{
		CHECK_EQ(decoder.DecodeImage(ResourceData(MakePng(ihdr, zlib))).GetError(),
		         ErrorCode::InvalidArgument);
	}
	// Zero width, and a width the asset ceiling refuses.
	CHECK_EQ(decoder.DecodeImage(
	    ResourceData(MakePng(MakeIhdr(0, 1, 2), zlib))).GetError(), ErrorCode::InvalidArgument);
	CHECK_EQ(decoder.DecodeImage(
	    ResourceData(MakePng(MakeIhdr(16385, 1, 2), zlib))).GetError(), ErrorCode::InvalidArgument);
}

// P1D
// The remaining fixtures come from the DDS and MTF descriptions, so the
// dispatch can be proved end to end without a renderer: a minimal uncompressed
// 32-bit DDS, and that same DDS wrapped in an MTF container (version 0x100,
// payload `(plain ^ 0x26) - 0x09`, fileType DDS -- the inverse of
// MtfTextureTransform's `(raw + 0x09) ^ 0x26`). Neither is produced by calling
// the code under test.
namespace
{

// A 2x2 uncompressed 32-bit DDS with R/G/B/A masks naming the channels in order
// (the layout DdsImageDecoder reports as R8G8B8A8_UNorm), plus four texels.
std::vector<uint8_t> MakeUncompressedDds()
{
	std::vector<uint8_t> dds;
	auto U32 = [&dds](uint32_t v) {
		dds.push_back(static_cast<uint8_t>(v & 0xFFu));
		dds.push_back(static_cast<uint8_t>((v >> 8) & 0xFFu));
		dds.push_back(static_cast<uint8_t>((v >> 16) & 0xFFu));
		dds.push_back(static_cast<uint8_t>((v >> 24) & 0xFFu));
	};

	dds.push_back('D'); dds.push_back('D'); dds.push_back('S'); dds.push_back(' ');
	U32(124u);            // dwSize
	U32(0x00001007u);     // dwFlags: caps | height | width | pixelformat
	U32(2u);              // height
	U32(2u);              // width
	U32(16u);             // dwPitchOrLinearSize
	U32(0u);              // dwDepth
	U32(0u);              // dwMipMapCount
	for (int i = 0; i < 11; ++i) { U32(0u); }  // dwReserved1[11]

	U32(32u);             // ddspf.dwSize
	U32(0x40u);           // ddspf.dwFlags: RGB (no FourCC)
	U32(0u);              // ddspf.dwFourCC
	U32(32u);             // ddspf.dwRGBBitCount
	U32(0x00ff0000u);     // R mask
	U32(0x0000ff00u);     // G mask
	U32(0x000000ffu);     // B mask
	U32(0xff000000u);     // A mask
	U32(0x00001000u);     // dwCaps: texture
	U32(0u); U32(0u); U32(0u); U32(0u);

	for (uint32_t texel = 0; texel < 4; ++texel)
	{
		dds.push_back(static_cast<uint8_t>(10 + texel));
		dds.push_back(static_cast<uint8_t>(20 + texel));
		dds.push_back(static_cast<uint8_t>(30 + texel));
		dds.push_back(0xFF);
	}
	return dds;
}

// Wraps a plain DDS in the MTF container the RAN client ships, encrypted the
// way MtfTextureTransform decrypts it: the header version 0x100, the payload
// size, fileType 0 (DDS), then each byte `(plain ^ 0x26) - 0x09`.
std::vector<uint8_t> MakeMtfFile(const std::vector<uint8_t>& plainDds)
{
	std::vector<uint8_t> mtf;
	auto U32 = [&mtf](uint32_t v) {
		mtf.push_back(static_cast<uint8_t>(v & 0xFFu));
		mtf.push_back(static_cast<uint8_t>((v >> 8) & 0xFFu));
		mtf.push_back(static_cast<uint8_t>((v >> 16) & 0xFFu));
		mtf.push_back(static_cast<uint8_t>((v >> 24) & 0xFFu));
	};
	U32(0x100u);
	U32(static_cast<uint32_t>(plainDds.size()));
	U32(0u);  // fileType: DDS
	for (uint8_t b : plainDds)
	{
		mtf.push_back(static_cast<uint8_t>((b ^ 0x26u) - 0x09u));
	}
	return mtf;
}

void WriteFileBytes(const std::filesystem::path& path, const std::vector<uint8_t>& bytes)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream file(path, std::ios::binary | std::ios::trunc);
	file.write(reinterpret_cast<const char*>(bytes.data()),
	           static_cast<std::streamsize>(bytes.size()));
}

// A fresh, empty textures tree under the OS temp directory, so the loader's
// name resolution is exercised against real files with no shipped asset.
std::filesystem::path MakeTempTexturesTree()
{
	static int counter = 0;
	const std::filesystem::path root =
	    std::filesystem::temp_directory_path() /
	    ("ran_client_png_" + std::to_string(counter++));
	std::error_code ec;
	std::filesystem::remove_all(root, ec);
	std::filesystem::create_directories(root / "item", ec);
	return root;
}

} // namespace

// P1E
// Negative: corrupted integrity and malformed compression, each refused.
MODERN_TEST(ClientPng_IhdrCrcMismatchIsRefused)
{
	const std::vector<uint8_t> row = { 1, 2, 3 };
	const std::vector<uint8_t> zeros(3, 0);
	const std::vector<uint8_t> scanlines = EncodeScanline(row, zeros, 3, 0);
	std::vector<uint8_t> png = MakePng(MakeIhdr(1, 1, 2), WrapZlib(DeflateStored(scanlines), scanlines));

	// The IHDR data begins at offset 16 (signature + length + type); flipping a
	// byte leaves the stored CRC stale, which must be refused before any field
	// is used to size an allocation.
	png[16] ^= 0xFF;

	PngImageDecoder decoder;
	CHECK_EQ(decoder.DecodeImage(ResourceData(png)).GetError(), ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientPng_AdlerMismatchIsRefused)
{
	const std::vector<uint8_t> row = { 9, 8, 7 };
	const std::vector<uint8_t> zeros(3, 0);
	const std::vector<uint8_t> scanlines = EncodeScanline(row, zeros, 3, 0);

	// Corrupt the zlib Adler-32 trailer (last byte of the stream). AppendChunk
	// still computes a valid IDAT CRC over the corrupted stream, so the only
	// defence that can catch this is the zlib Adler check inside the inflate.
	std::vector<uint8_t> zlib = WrapZlib(DeflateStored(scanlines), scanlines);
	zlib[zlib.size() - 1] ^= 0xFF;

	PngImageDecoder decoder;
	CHECK_EQ(decoder.DecodeImage(ResourceData(MakePng(MakeIhdr(1, 1, 2), zlib))).GetError(),
	         ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientPng_MalformedDeflateIsRefused)
{
	// A zlib stream whose single DEFLATE block declares a reserved BTYPE (0x07
	// is BFINAL=1, BTYPE=11). The inflate fails before any pixel is produced.
	PngImageDecoder decoder;
	CHECK_EQ(decoder.DecodeImage(ResourceData(MakePng(MakeIhdr(1, 1, 2), WrapZlib({ 0x07 }, { 0 }))))
	             .GetError(),
	         ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientPng_DeclaredChunkLengthBeyondFileIsRefused)
{
	const std::vector<uint8_t> row = { 1, 2, 3 };
	const std::vector<uint8_t> zeros(3, 0);
	const std::vector<uint8_t> scanlines = EncodeScanline(row, zeros, 3, 0);
	std::vector<uint8_t> png = MakePng(MakeIhdr(1, 1, 2), WrapZlib(DeflateStored(scanlines), scanlines));

	// The first chunk's length field (offset 8) set past the 16 MiB ceiling:
	// a corrupt length that would demand an enormous allocation is refused
	// before anything is allocated.
	png[8] = 0xFF; png[9] = 0xFF; png[10] = 0xFF; png[11] = 0xFF;

	PngImageDecoder decoder;
	CHECK_EQ(decoder.DecodeImage(ResourceData(png)).GetError(), ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientPng_ScanlineCountDisagreeingWithHeaderIsRefused)
{
	const std::vector<uint8_t> row0 = { 10, 20, 30, 40, 50, 60 };
	const std::vector<uint8_t> zeros(6, 0);
	// One scanline of data, but a header that claims two rows.
	const std::vector<uint8_t> scanlines = EncodeScanline(row0, zeros, 3, 0);

	PngImageDecoder decoder;
	CHECK_EQ(decoder.DecodeImage(
	             ResourceData(MakePng(MakeIhdr(2, 2, 2), WrapZlib(DeflateStored(scanlines), scanlines))))
	             .GetError(),
	         ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// Texture loader: content-based dispatch (PNG -> PNG, DDS -> DDS, MTF -> DDS)
// ---------------------------------------------------------------------------

MODERN_TEST(ClientPng_DecodeDispatchesPngToPngDecoder)
{
	const std::vector<uint8_t> row = { 200, 100, 50, 10, 20, 30 };
	const std::vector<uint8_t> zeros(6, 0);
	const std::vector<uint8_t> scanlines = EncodeScanline(row, zeros, 3, 0);
	const std::vector<uint8_t> png =
	    MakePng(MakeIhdr(2, 1, 2), WrapZlib(DeflateStored(scanlines), scanlines));

	const Result<ImageAsset> decoded = TextureLoader::Decode(ResourceData(png));
	REQUIRE(decoded.IsOk());
	// Only the PNG channel yields opaque-alpha RGBA from an RGB truecolour file.
	CHECK_EQ(decoded.GetValue().GetFormat(), ImageFormat::R8G8B8A8_UNorm);
	CheckPixel(decoded.GetValue(), 0, 0, 200, 100, 50, 255);
	CheckPixel(decoded.GetValue(), 1, 0, 10, 20, 30, 255);
}

MODERN_TEST(ClientPng_DecodeDispatchesDdsToDdsDecoder)
{
	const Result<ImageAsset> decoded = TextureLoader::Decode(ResourceData(MakeUncompressedDds()));
	REQUIRE(decoded.IsOk());
	CHECK_EQ(decoded.GetValue().GetFormat(), ImageFormat::R8G8B8A8_UNorm);
	CHECK_EQ(decoded.GetValue().GetWidth(), 2u);
	CHECK_EQ(decoded.GetValue().GetHeight(), 2u);
}

MODERN_TEST(ClientPng_DecodeDispatchesMtfThroughToDds)
{
	// The MTF container hides a DDS; the loader must unwrap it and DDS-decode
	// the result, producing the same image the plain DDS produced.
	const std::vector<uint8_t> plain = MakeUncompressedDds();
	const Result<ImageAsset> fromMtf =
	    TextureLoader::Decode(ResourceData(MakeMtfFile(plain)));
	const Result<ImageAsset> fromDds = TextureLoader::Decode(ResourceData(plain));
	REQUIRE(fromMtf.IsOk());
	REQUIRE(fromDds.IsOk());
	CHECK(fromMtf.GetValue().GetPixels() == fromDds.GetValue().GetPixels());
	CHECK_EQ(fromMtf.GetValue().GetWidth(), fromDds.GetValue().GetWidth());
}

MODERN_TEST(ClientPng_DecodeRefusesUnknownMagic)
{
	// Neither a PNG signature, nor DDS magic, nor the MTF version: no decoder
	// claims it, and the loader refuses rather than guessing.
	CHECK_EQ(TextureLoader::Decode(ResourceData(std::vector<uint8_t>(32, 0x42))).GetError(),
	         ErrorCode::InvalidArgument);
	CHECK_EQ(TextureLoader::Decode(ResourceData(std::vector<uint8_t>(3, 0x01))).GetError(),
	         ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// Texture loader: name resolution over a real tree
// ---------------------------------------------------------------------------

MODERN_TEST(ClientLoader_FindsBareNameCaseInsensitively)
{
	const std::filesystem::path root = MakeTempTexturesTree();
	WriteFileBytes(root / "item" / "Blackcatyb_Luffy.png", PngSignature());

	TextureLoader loader;
	REQUIRE(loader.Initialize(root).IsOk());
	const std::filesystem::path found = loader.Find("blackcatyb_luffy.png");
	CHECK(!found.empty());
	CHECK_EQ(found.filename().string(), std::string("Blackcatyb_Luffy.png"));

	std::error_code ec;
	std::filesystem::remove_all(root, ec);
}

MODERN_TEST(ClientLoader_PrefersMtfSiblingOverPlainFile)
{
	const std::filesystem::path root = MakeTempTexturesTree();
	WriteFileBytes(root / "item" / "foo.png", PngSignature());
	WriteFileBytes(root / "item" / "foo.mtf", PngSignature());

	TextureLoader loader;
	REQUIRE(loader.Initialize(root).IsOk());
	// Legacy order: the encrypted same-stem `.mtf` wins over the plain name.
	CHECK_EQ(loader.Find("foo.png").filename().string(), std::string("foo.mtf"));

	std::error_code ec;
	std::filesystem::remove_all(root, ec);
}

MODERN_TEST(ClientLoader_RefusesPathSeparatorAndEmptyName)
{
	const std::filesystem::path root = MakeTempTexturesTree();
	WriteFileBytes(root / "item" / "foo.png", PngSignature());

	TextureLoader loader;
	REQUIRE(loader.Initialize(root).IsOk());
	// A directory separator is not an address here: only bare filenames are.
	CHECK(loader.Find("item/foo.png").empty());
	CHECK(loader.Find("..\\foo.png").empty());
	CHECK(loader.Find("").empty());
	// But the bare name still resolves.
	CHECK(!loader.Find("foo.png").empty());

	std::error_code ec;
	std::filesystem::remove_all(root, ec);
}

MODERN_TEST(ClientLoader_LoadMissingNameIsNotFound)
{
	const std::filesystem::path root = MakeTempTexturesTree();
	TextureLoader loader;
	REQUIRE(loader.Initialize(root).IsOk());
	CHECK_EQ(loader.Load("not_there.png").GetError(), ErrorCode::NotFound);

	std::error_code ec;
	std::filesystem::remove_all(root, ec);
}

MODERN_TEST(ClientLoader_LoadUnrecognizedBytesAreRefused)
{
	const std::filesystem::path root = MakeTempTexturesTree();
	WriteFileBytes(root / "junk.dat", std::vector<uint8_t>(64, 0x7Fu));

	TextureLoader loader;
	REQUIRE(loader.Initialize(root).IsOk());
	// The name resolves, but the bytes are not a PNG/DDS/MTF texture.
	CHECK_EQ(loader.Load("junk.dat").GetError(), ErrorCode::InvalidArgument);

	std::error_code ec;
	std::filesystem::remove_all(root, ec);
}

// ---------------------------------------------------------------------------
// Real RAN textures, when one is available
// ---------------------------------------------------------------------------

// Decodes both of the character's shipped PNGs -- a 1024x1024 RGB body texture
// and a 256x256 RGBA eye texture -- straight through PngImageDecoder, only when
// RAN_ASSET_ROOT names a client tree. Fixtures prove the format; this proves
// the exact files the character references actually decode.
MODERN_TEST(ClientPng_RealRanTexturesDecodeWhenAvailable)
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
		std::printf("      (real RAN texture validation skipped: RAN_ASSET_ROOT not set)\n");
		return;
	}

	const std::filesystem::path itemDir = std::filesystem::path(root) / "textures" / "item";
	if (!std::filesystem::is_directory(itemDir))
	{
		std::printf("      (real RAN texture validation skipped: no textures/item directory)\n");
		return;
	}

	PngImageDecoder decoder;
	const char* bodies[] = { "blackcatyb_luffy_timeskip.png" };
	const char* eyes[]   = { "blackcatyb_luffy_timeskip_eye.png" };
	auto decode = [&](const std::filesystem::path& path, uint32_t expectW, uint32_t expectH) {
		std::ifstream file(path, std::ios::binary);
		if (!file)
		{
			return false;
		}
		const std::vector<uint8_t> bytes(
			(std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		const Result<ImageAsset> image = decoder.DecodeImage(ResourceData(bytes));
		if (image.IsError())
		{
			return false;
		}
		if (image.GetValue().GetWidth() != expectW || image.GetValue().GetHeight() != expectH ||
		    image.GetValue().GetFormat() != ImageFormat::R8G8B8A8_UNorm ||
		    image.GetValue().GetPixelByteCount() !=
		        static_cast<size_t>(expectW) * static_cast<size_t>(expectH) * 4u)
		{
			return false;
		}
		// A real character texture has visible content: not one flat colour, and
		// in particular not all black (a decoder whose pixels collapsed to zero
		// would still satisfy every shape check above but render a black mesh).
		const std::vector<uint8_t>& px = image.GetValue().GetPixels();
		uint64_t lum = 0;
		size_t   distinct = 0;
		int      prev = -1;
		for (size_t i = 0; i < px.size(); ++i)
		{
			lum += px[i];
		}
		const auto mean = static_cast<double>(lum) / static_cast<double>(px.size());
		for (size_t i = 0; i + 4 <= px.size(); i += 4)
		{
			const int level = (px[i] + px[i + 1] + px[i + 2]) / 3;
			if (level != prev)
			{
				++distinct;
				prev = level;
			}
		}
		std::printf("      %s: mean-channel=%.1f, distinct-channel-run-levels=%zu\n",
			path.filename().string().c_str(), mean, distinct);
		return mean > 5.0 && distinct > 4u;
	};

	for (const char* name : bodies) { CHECK(decode(itemDir / name, 1024, 1024)); }
	for (const char* name : eyes)   { CHECK(decode(itemDir / name, 256, 256)); }
}

int main()
{
	std::printf("Modern CLIENT-RENDER-002 PNG decoder and texture loader tests\n\n");

	const int failedCases = ModernTests::RunAll();

	if (failedCases == 0)
	{
		std::printf("\nAll %d test cases passed.\n", static_cast<int>(ModernTests::Registry().size()));
		return 0;
	}

	std::printf("\n%d of %d test cases FAILED (%d checks).\n",
		failedCases,
		static_cast<int>(ModernTests::Registry().size()),
		ModernTests::FailureCount());
	return 1;
}