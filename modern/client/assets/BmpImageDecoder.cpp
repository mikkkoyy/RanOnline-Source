#include "assets/BmpImageDecoder.h"

#include "assets/AssetTypes.h"
#include "assets/ImageAsset.h"
#include "resources/ResourceData.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace Modern::Client
{
namespace
{

	// The 14-byte BITMAPFILEHEADER plus the BITMAPINFOHEADER. Field offsets are
	// from the first byte of the file. Every multi-byte field is little-endian
	// and assembled by hand, so no unaligned or endian-dependent load happens on
	// bytes that came from a file.
	constexpr size_t kMaxFieldsOffset = 0x2E;  // biClrUsed, past every field read

	constexpr size_t kOffPixelOffset  = 0x0A;  // u32
	constexpr size_t kOffHeaderSize   = 0x0E;  // u32
	constexpr size_t kOffWidth        = 0x12;  // i32
	constexpr size_t kOffHeight       = 0x16;  // i32 (negative => top-down)
	constexpr size_t kOffBitsPerPixel = 0x1C;  // u16
	constexpr size_t kOffCompression  = 0x1E;  // u32
	constexpr size_t kOffColorUsed    = 0x2E;  // u32

	constexpr uint32_t kCompressionRgb = 0u;

	constexpr uint8_t  kMagic0 = 'B';
	constexpr uint8_t  kMagic1 = 'M';

	uint16_t ReadU16Le(const uint8_t* p) noexcept
	{
		return static_cast<uint16_t>(static_cast<uint16_t>(p[0]) |
		                             (static_cast<uint16_t>(p[1]) << 8));
	}

	uint32_t ReadU32Le(const uint8_t* p) noexcept
	{
		return static_cast<uint32_t>(p[0]) |
		       (static_cast<uint32_t>(p[1]) << 8) |
		       (static_cast<uint32_t>(p[2]) << 16) |
		       (static_cast<uint32_t>(p[3]) << 24);
	}

	int32_t ReadI32Le(const uint8_t* p) noexcept
	{
		return static_cast<int32_t>(ReadU32Le(p));
	}

} // namespace

bool BmpImageDecoder::LooksLikeBmp(const uint8_t* data, size_t size) noexcept
{
	if (data == nullptr || size <= kMaxFieldsOffset)
	{
		return false;
	}
	if (data[0] != kMagic0 || data[1] != kMagic1)
	{
		return false;
	}
	// A header size of at least 40 names the fields this reader reads.
	const uint32_t headerSize = ReadU32Le(data + kOffHeaderSize);
	return headerSize >= 40u;
}

Result<ImageAsset> BmpImageDecoder::DecodeImage(const ResourceData& data)
{
	const size_t   size = data.GetSize();
	const uint8_t* b    = data.GetData();
	if (b == nullptr || size <= kMaxFieldsOffset)
	{
		return Status(ErrorCode::InvalidArgument);
	}
	if (b[0] != kMagic0 || b[1] != kMagic1)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	const uint32_t pixelOffsetRaw = ReadU32Le(b + kOffPixelOffset);
	const uint32_t headerSize     = ReadU32Le(b + kOffHeaderSize);
	const int32_t  widthSigned    = ReadI32Le(b + kOffWidth);
	const int32_t  heightSigned   = ReadI32Le(b + kOffHeight);
	const uint16_t bitsPerPixel   = ReadU16Le(b + kOffBitsPerPixel);
	const uint32_t compression     = ReadU32Le(b + kOffCompression);
	const uint32_t colorUsed       = ReadU32Le(b + kOffColorUsed);

	if (headerSize < 40u)
	{
		// The only header this reader understands is BITMAPINFOHEADER or larger.
		return Status(ErrorCode::InvalidArgument);
	}
	if (compression != kCompressionRgb)
	{
		// RLE4/RLE8/BITFIELDS/JPEG/PNG are refused rather than guessed at.
		return Status(ErrorCode::InvalidArgument);
	}
	if (bitsPerPixel != 8 && bitsPerPixel != 24 && bitsPerPixel != 32)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// A negative height marks a top-down image; the magnitude is the height.
	const bool topDown = (heightSigned < 0);
	const uint32_t width  = (widthSigned > 0) ? static_cast<uint32_t>(widthSigned)
	                                          : static_cast<uint32_t>(-static_cast<int64_t>(widthSigned));
	const uint32_t height = (heightSigned > 0) ? static_cast<uint32_t>(heightSigned)
	                                           : static_cast<uint32_t>(-static_cast<int64_t>(heightSigned));
	if (width == 0 || height == 0)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// The asset layer's own ceilings, in 64-bit, before anything is allocated.
	const Result<size_t> outBytes =
	    ComputeImageByteCount(width, height, ImageFormat::R8G8B8A8_UNorm);
	if (outBytes.IsError())
	{
		return outBytes.GetStatus();
	}

	const uint32_t bytesPerPixel = bitsPerPixel / 8u;
	// A scanline is padded up to a 4-byte boundary.
	const uint64_t rowStride64 =
	    (static_cast<uint64_t>(width) * bytesPerPixel + 3u) & ~static_cast<uint64_t>(3u);

	// Palette (bpp <= 8 only), immediately after the 14-byte file header and the
	// DIB header (RGBQUAD entries: B, G, R, reserved).
	size_t paletteBytes = 0;
	if (bitsPerPixel == 8)
	{
		const uint32_t entries = (colorUsed != 0u) ? colorUsed : 256u;
		paletteBytes = static_cast<size_t>(entries) * 4u;
	}
	const size_t paletteStart = kOffHeaderSize + headerSize;  // 14 (file header) + DIB header size
	if (paletteStart > size || paletteBytes > size - paletteStart)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// Pixel data offset: trusts bfOffBits, but never below the end of the
	// palette (a bogus small offset would otherwise read header bytes as pixels).
	const size_t pixelStart = (pixelOffsetRaw > paletteStart + paletteBytes)
	                              ? static_cast<size_t>(pixelOffsetRaw)
	                              : paletteStart + paletteBytes;

	std::vector<uint8_t> pixels(outBytes.GetValue());
	std::vector<uint8_t> row(rowStride64 == 0 ? 1 : static_cast<size_t>(rowStride64));

	for (uint32_t y = 0; y < height; ++y)
	{
		const uint64_t rowPos = pixelStart + static_cast<uint64_t>(y) * rowStride64;
		if (rowPos + rowStride64 > static_cast<uint64_t>(size))
		{
			return Status(ErrorCode::InvalidArgument);
		}
		const uint8_t* src = b + rowPos;
		const uint32_t outRow = topDown ? y : (height - 1 - y);
		uint8_t* dst = pixels.data() + static_cast<size_t>(outRow) * static_cast<size_t>(width) * 4u;
		for (uint32_t x = 0; x < width; ++x)
		{
			if (bitsPerPixel == 8)
			{
				const uint8_t index = src[x];
				const uint8_t* entry = b + paletteStart + static_cast<size_t>(index) * 4u;
				dst[x * 4 + 0] = entry[2];  // R
				dst[x * 4 + 1] = entry[1];  // G
				dst[x * 4 + 2] = entry[0];  // B
				dst[x * 4 + 3] = 0xFF;
			}
			else if (bitsPerPixel == 24)
			{
				const uint8_t* s = src + static_cast<size_t>(x) * 3u;
				dst[x * 4 + 0] = s[2];
				dst[x * 4 + 1] = s[1];
				dst[x * 4 + 2] = s[0];
				dst[x * 4 + 3] = 0xFF;
			}
			else  // 32-bit BI_RGB: BGRX, the fourth byte is unused -> opaque
			{
				const uint8_t* s = src + static_cast<size_t>(x) * 4u;
				dst[x * 4 + 0] = s[2];
				dst[x * 4 + 1] = s[1];
				dst[x * 4 + 2] = s[0];
				dst[x * 4 + 3] = 0xFF;
			}
		}
	}

	return ImageAsset::Create(width, height, ImageFormat::R8G8B8A8_UNorm, std::move(pixels));
}

} // namespace Modern::Client
