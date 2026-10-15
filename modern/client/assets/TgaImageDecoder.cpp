#include "assets/TgaImageDecoder.h"

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

	// The Targa header is 18 fixed bytes followed by an optional colour map and
	// the image data. Every multi-byte field is little-endian and is assembled
	// by hand, so no unaligned or endian-dependent load happens on bytes that
	// came from a file -- the same rule DdsImageDecoder and PngImageDecoder
	// follow for their little-/big-endian fields.
	constexpr size_t kTgaHeaderBytes = 18;
	constexpr size_t kTgaFooterBytes = 26;  // optional trailing extension

	constexpr size_t kOffIdLength    = 0;  // u8, optional image-ID bytes after the header
	constexpr size_t kOffColorMapType = 1;
	constexpr size_t kOffImageType    = 2;
	constexpr size_t kOffWidth        = 12;  // u16
	constexpr size_t kOffHeight       = 14;  // u16
	constexpr size_t kOffBitsPerPixel = 16;  // u8
	constexpr size_t kOffDescriptor   = 17;  // u8

	// Image types this decoder supports.
	constexpr uint8_t kImageTypeTrueColor = 2;   // uncompressed BGR / BGRA
	constexpr uint8_t kImageTypeGray      = 3;   // uncompressed 8-bit intensity
	constexpr uint8_t kImageTypeRleTrue   = 10;  // RLE truecolour

	// Descriptor byte bit 5 is the vertical origin: set means top-row-first,
	// clear means native bottom-up.
	constexpr uint8_t kDescriptorTopOrigin = 0x20;

	// The optional trailing signature. Its absence is legal (old Targas), so it
	// is only a positive cue, never a requirement.
	const char* const kTgaFooterSignature = "TRUEVISION-XFILE.";

	uint16_t ReadU16Le(const uint8_t* p) noexcept
	{
		return static_cast<uint16_t>(static_cast<uint16_t>(p[0]) |
		                             (static_cast<uint16_t>(p[1]) << 8));
	}

	// Bytes per source pixel for a truecolour depth this decoder reads (24 or
	// 32), or 0 for anything else. 15/16-bit truecolour is refused rather than
	// guessed at.
	uint8_t TrueColorPixelBytes(uint8_t bitsPerPixel) noexcept
	{
		if (bitsPerPixel == 24) { return 3; }
		if (bitsPerPixel == 32) { return 4; }
		return 0;
	}

	bool HasFooterSignature(const uint8_t* data, size_t size) noexcept
	{
		if (size < kTgaFooterBytes)
		{
			return false;
		}
		const uint8_t* footer = data + (size - kTgaFooterBytes);
		for (size_t i = 0; kTgaFooterSignature[i] != '\0'; ++i)
		{
			if (footer[i] != static_cast<uint8_t>(kTgaFooterSignature[i]))
			{
				return false;
			}
		}
		return true;
	}

} // namespace

bool TgaImageDecoder::LooksLikeTga(const uint8_t* data, size_t size) noexcept
{
	if (data == nullptr || size < kTgaHeaderBytes)
	{
		return false;
	}

	// Positive cue first: a file carrying the footer signature is a Targa
	// regardless of how loose its header fields look.
	if (HasFooterSignature(data, size))
	{
		return true;
	}

	// Otherwise a strict header sanity check, so a footer-less Targa still
	// resolves and a signed format (PNG/DDS/BMP) is never mis-attributed here.
	if (data[kOffColorMapType] != 0)
	{
		return false;
	}
	const uint8_t imageType = data[kOffImageType];
	if (imageType != kImageTypeTrueColor && imageType != kImageTypeGray &&
	    imageType != kImageTypeRleTrue)
	{
		return false;
	}
	const uint8_t bpp = data[kOffBitsPerPixel];
	const bool gray = (imageType == kImageTypeGray);
	const uint8_t pixelBytes = gray ? 1u : TrueColorPixelBytes(bpp);
	if (pixelBytes == 0)
	{
		return false;
	}
	const uint16_t width  = ReadU16Le(data + kOffWidth);
	const uint16_t height = ReadU16Le(data + kOffHeight);
	if (width == 0 || height == 0)
	{
		return false;
	}
	// An uncompressed body must be able to hold its pixels within the file.
	if (imageType != kImageTypeRleTrue)
	{
		const uint64_t raw = static_cast<uint64_t>(width) * static_cast<uint64_t>(height) *
		                     static_cast<uint64_t>(pixelBytes);
		if (raw > static_cast<uint64_t>(size))
		{
			return false;
		}
	}
	return true;
}

Result<ImageAsset> TgaImageDecoder::DecodeImage(const ResourceData& data)
{
	const size_t   size = data.GetSize();
	const uint8_t* b    = data.GetData();
	if (b == nullptr || size < kTgaHeaderBytes)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	const uint8_t  colorMapType = b[kOffColorMapType];
	const uint8_t  imageType    = b[kOffImageType];
	const uint8_t  bpp          = b[kOffBitsPerPixel];
	const uint8_t  descriptor   = b[kOffDescriptor];
	const uint16_t width        = ReadU16Le(b + kOffWidth);
	const uint16_t height       = ReadU16Le(b + kOffHeight);

	// The types this decoder reads never carry a colour map; a colour map here
	// is the signal of a mapped/RLE-mapped type (1/9/11) that is out of subset.
	if (colorMapType != 0)
	{
		return Status(ErrorCode::InvalidArgument);
	}
	if (imageType != kImageTypeTrueColor && imageType != kImageTypeGray &&
	    imageType != kImageTypeRleTrue)
	{
		return Status(ErrorCode::InvalidArgument);
	}
	if (width == 0 || height == 0)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	const bool    gray       = (imageType == kImageTypeGray);
	const bool    rle        = (imageType == kImageTypeRleTrue);
	const uint8_t pixelBytes = gray ? 1u : TrueColorPixelBytes(bpp);
	const size_t  srcChannels = gray ? 1u : pixelBytes;
	if (pixelBytes == 0)
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

	// The colour map is absent (colorMapType is 0), so the pixel data begins
	// after the 18-byte header and the optional image ID. Skipping the ID keeps
	// files that carry one (a comment or producer tag) correct.
	size_t pixelOffset = kTgaHeaderBytes + b[kOffIdLength];
	if (pixelOffset > size)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	const bool topOrigin = (descriptor & kDescriptorTopOrigin) != 0;

	std::vector<uint8_t> pixels(outBytes.GetValue());
	const size_t rowBytes = static_cast<size_t>(width) * srcChannels;
	std::vector<uint8_t> srcRow(rowBytes);

	for (uint16_t y = 0; y < height; ++y)
	{
		if (rle)
		{
			// RLE truecolour: a packet is one count byte followed either by one
			// repeated pixel (high bit set) or `count` literal pixels. One
			// scanline's worth is decoded per outer iteration, and every advance
			// is bounded against the payload.
			size_t produced = 0;
			for (;;)
			{
				if (pixelOffset >= size)
				{
					return Status(ErrorCode::InvalidArgument);
				}
				const uint8_t  packet = b[pixelOffset++];
				const uint32_t count  = static_cast<uint32_t>(packet & 0x7Fu) + 1u;
				const bool     run    = (packet & 0x80u) != 0;
				if (produced + static_cast<size_t>(count) * pixelBytes > rowBytes)
				{
					return Status(ErrorCode::InvalidArgument);
				}
				if (run)
				{
					if (pixelOffset + pixelBytes > size)
					{
						return Status(ErrorCode::InvalidArgument);
					}
					for (uint32_t i = 0; i < count; ++i)
					{
						for (uint8_t c = 0; c < pixelBytes; ++c)
						{
							srcRow[produced + i * pixelBytes + c] = b[pixelOffset + c];
						}
					}
					pixelOffset += pixelBytes;
				}
				else
				{
					if (pixelOffset + static_cast<size_t>(count) * pixelBytes > size)
					{
						return Status(ErrorCode::InvalidArgument);
					}
					for (uint32_t i = 0; i < count; ++i)
					{
						for (uint8_t c = 0; c < pixelBytes; ++c)
						{
							srcRow[produced + i * pixelBytes + c] =
							    b[pixelOffset + static_cast<size_t>(i) * pixelBytes + c];
						}
					}
					pixelOffset += static_cast<size_t>(count) * pixelBytes;
				}
				produced += static_cast<size_t>(count) * pixelBytes;
				if (produced == rowBytes)
				{
					break;
				}
			}
		}
		else
		{
			if (pixelOffset + rowBytes > size)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			for (size_t i = 0; i < rowBytes; ++i)
			{
				srcRow[i] = b[pixelOffset + i];
			}
			pixelOffset += rowBytes;
		}

		const uint16_t outRow = topOrigin ? y : static_cast<uint16_t>(height - 1 - y);
		uint8_t* dst = pixels.data() + static_cast<size_t>(outRow) * static_cast<size_t>(width) * 4u;
		for (uint16_t x = 0; x < width; ++x)
		{
			const uint8_t* s = srcRow.data() + static_cast<size_t>(x) * srcChannels;
			if (gray)
			{
				dst[x * 4 + 0] = s[0];
				dst[x * 4 + 1] = s[0];
				dst[x * 4 + 2] = s[0];
				dst[x * 4 + 3] = 0xFF;
			}
			else if (srcChannels == 4)  // BGRA
			{
				dst[x * 4 + 0] = s[2];
				dst[x * 4 + 1] = s[1];
				dst[x * 4 + 2] = s[0];
				dst[x * 4 + 3] = s[3];
			}
			else  // BGR
			{
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
