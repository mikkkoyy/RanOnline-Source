#include "assets/DdsImageDecoder.h"

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
	// ---------------------------------------------------------------------------
	// The DDS container, as bytes
	// ---------------------------------------------------------------------------
	//
	// A DDS file is a four-byte magic followed by a 124-byte DDS_HEADER, which
	// embeds a 32-byte DDS_PIXELFORMAT: 128 bytes in total, then the surface
	// data. Every field below is read as four little-endian bytes assembled by
	// hand rather than reinterpreted, so a payload on any host alignment and
	// any byte order still reads the same way, and no unaligned load is
	// performed on bytes that came from a file.
	//
	// magic 'D','D','S',' ' is the little-endian uint32 0x20534444.
	constexpr size_t   kDdsFileHeaderBytes = 128;
	constexpr uint32_t kDdsMagic           = 0x20534444u;
	constexpr uint32_t kHeaderSize         = 124u;
	constexpr uint32_t kPixelFormatSize    = 32u;

	// Field offsets within the file, from the magic at 0.
	constexpr size_t kOffHeaderSize     = 4;
	constexpr size_t kOffFlags          = 8;
	constexpr size_t kOffHeight         = 12;
	constexpr size_t kOffWidth          = 16;
	constexpr size_t kOffPitchOrLinear  = 20;
	constexpr size_t kOffDepth          = 24;
	constexpr size_t kOffMipMapCount    = 28;
	constexpr size_t kOffPfSize         = 76;
	constexpr size_t kOffPfFlags        = 80;
	constexpr size_t kOffPfFourCC       = 84;
	constexpr size_t kOffPfRgbBitCount  = 88;
	constexpr size_t kOffPfRMask        = 92;
	constexpr size_t kOffPfGMask        = 96;
	constexpr size_t kOffPfBMask        = 100;
	constexpr size_t kOffPfAMask        = 104;
	constexpr size_t kOffCaps2          = 112;

	// DDS_PIXELFORMAT.dwFlags
	constexpr uint32_t kPfFourCC     = 0x00000004u;
	constexpr uint32_t kPfRgb        = 0x00000040u;
	constexpr uint32_t kPfLuminance  = 0x00020000u;

	// DDS_HEADER.dwCaps2
	constexpr uint32_t kCaps2CubeMap  = 0x00000200u;
	constexpr uint32_t kCaps2Volume   = 0x00200000u;

	// The four-character codes this decoder knows. A FourCC is read as four
	// bytes into one little-endian uint32, so each code below is its ASCII
	// bytes reversed relative to how it is written in the file.
	constexpr uint32_t kFourCcDxt1 = 0x31545844u;  // 'D','X','T','1'
	constexpr uint32_t kFourCcDxt3 = 0x33545844u;  // 'D','X','T','3'
	constexpr uint32_t kFourCcDxt5 = 0x35545844u;  // 'D','X','T','5'

	// A DDS file may carry the DX10 extension header, signalled both by this
	// dwFlags bit and by a 'DX10' FourCC. No RAN asset uses it, and a partial
	// reading of one would silently misinterpret the layout, so it is refused
	// rather than tolerated.
	constexpr uint32_t kFlagDx10 = 0x80000000u;
	constexpr uint32_t kFourCcDx10 = 0x30315844u;  // 'D','X','1','0'

	// Uncompressed layouts this decoder copies through unchanged. Both are
	// named by their channel masks rather than by a FourCC, because DDS
	// describes them that way.
	constexpr uint32_t kMaskRgba32R = 0x00ff0000u;
	constexpr uint32_t kMaskRgba32G = 0x0000ff00u;
	constexpr uint32_t kMaskRgba32B = 0x000000ffu;
	constexpr uint32_t kMaskRgba32A = 0xff000000u;
	constexpr uint32_t kMaskBgra32R = 0x000000ffu;
	constexpr uint32_t kMaskBgra32B = 0x00ff0000u;

	// The texel block of each supported compressed format: 4x4 texels held in
	// 8 bytes for DXT1 and 16 for DXT3/DXT5.
	constexpr uint32_t kBlockSide     = 4u;
	constexpr size_t   kDxt1BlockBytes = 8u;
	constexpr size_t   kDxt3BlockBytes = 16u;
	constexpr size_t   kDxt5BlockBytes = 16u;

	// Reads four little-endian bytes. Called only at offsets the header layout
	// guarantees are inside the 128-byte prefix, which DecodeImage has already
	// checked in full.
	uint32_t ReadU32(const uint8_t* bytes) noexcept
	{
		return static_cast<uint32_t>(bytes[0]) |
		       (static_cast<uint32_t>(bytes[1]) << 8) |
		       (static_cast<uint32_t>(bytes[2]) << 16) |
		       (static_cast<uint32_t>(bytes[3]) << 24);
	}

	// How the surface data of this file is stored. Decided once, from the
	// header, before a single payload byte is read.
	enum class DdsLayout : uint8_t
	{
		Unknown = 0,
		Dxt1,
		Dxt3,
		Dxt5,
		Rgba32,
		Bgra32,
	};

	// A parsed, still-untrusted description of one DDS file.
	struct DdsHeader
	{
		uint32_t width            = 0;
		uint32_t height           = 0;
		uint32_t pitchOrLinearSize = 0;
		DdsLayout layout          = DdsLayout::Unknown;
	};

	// Maps the FourCC onto a layout. Anything else -- DXT2 and DXT4 among
	// them, plus ATI2, RGTC, DX10 and every unknown code -- stays Unknown and
	// is refused by the caller, so adding support later is a new enumerator
	// here and nowhere else.
	DdsLayout LayoutFromFourCC(uint32_t fourCC) noexcept
	{
		switch (fourCC)
		{
		case kFourCcDxt1: return DdsLayout::Dxt1;
		case kFourCcDxt3: return DdsLayout::Dxt3;
		case kFourCcDxt5: return DdsLayout::Dxt5;
		default:          return DdsLayout::Unknown;
		}
	}

	// Maps the uncompressed 32-bit channel masks onto a layout. A file with
	// no FourCC and no recognised mask pair -- 16-bit RGB565, 24-bit RGB,
	// luminance, YUV, paletted -- stays Unknown and is refused. Nothing here
	// guesses a layout it was not given, which is the same rule the rest of
	// the asset layer follows.
	DdsLayout LayoutFromMasks(uint32_t flags, uint32_t bitCount, uint32_t r, uint32_t g, uint32_t b, uint32_t a) noexcept
	{
		// Only true colour. A luminance or alpha-only file describes something
		// other than an RGBA image and is not silently widened into one.
		if ((flags & (kPfRgb | kPfLuminance)) != kPfRgb)
		{
			return DdsLayout::Unknown;
		}

		if (bitCount != 32)
		{
			return DdsLayout::Unknown;
		}

		if (r == kMaskRgba32R && g == kMaskRgba32G && b == kMaskRgba32B && a == kMaskRgba32A)
		{
			return DdsLayout::Rgba32;
		}

		if (r == kMaskBgra32R && g == kMaskRgba32G && b == kMaskBgra32B && a == kMaskRgba32A)
		{
			return DdsLayout::Bgra32;
		}

		return DdsLayout::Unknown;
	}

	// Bytes one texel block occupies in the file.
	size_t BlockBytes(DdsLayout layout) noexcept
	{
		switch (layout)
		{
		case DdsLayout::Dxt1: return kDxt1BlockBytes;
		case DdsLayout::Dxt3: return kDxt3BlockBytes;
		case DdsLayout::Dxt5: return kDxt5BlockBytes;
		default:              return 0;
		}
	}

	// Exact byte count of the top level, the only level this decoder reads.
	//
	// Formed in 64-bit arithmetic and refused rather than wrapped: a header
	// claiming 0xFFFFFFFF x 0xFFFFFFFF would overflow a 32-bit product and
	// then ask for a small allocation. The dimension ceilings in
	// ComputeImageByteCount are applied by the caller as well, because this
	// count is the *file's* claim and that one is the *asset's* limit.
	Result<size_t> TopLevelByteCount(uint32_t width, uint32_t height, DdsLayout layout) noexcept
	{
		if (width == 0 || height == 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		if (layout == DdsLayout::Rgba32 || layout == DdsLayout::Bgra32)
		{
			const uint64_t total = static_cast<uint64_t>(width) * static_cast<uint64_t>(height) * 4ull;
			return static_cast<size_t>(total);
		}

		const size_t blockBytes = BlockBytes(layout);
		if (blockBytes == 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// Blocks round up: a 3x3 image still occupies one whole 4x4 block.
		const uint64_t blocksX = (static_cast<uint64_t>(width) + kBlockSide - 1ull) / kBlockSide;
		const uint64_t blocksY = (static_cast<uint64_t>(height) + kBlockSide - 1ull) / kBlockSide;
		const uint64_t total    = blocksX * blocksY * static_cast<uint64_t>(blockBytes);

		return static_cast<size_t>(total);
	}

	// Parses and validates the 128-byte prefix. Every offset read here is
	// inside the prefix, which the caller has already checked exists in full.
	//
	// Order matters and is deliberate: identity (magic, then the two declared
	// structure sizes) before shape (dimensions, caps), then layout (pixel
	// format), then size. A file is refused for the first thing wrong with
	// it, so the reason is always the outermost one.
	Result<DdsHeader> ParseHeader(const ResourceData& data) noexcept
	{
		if (data.GetSize() < kDdsFileHeaderBytes)
		{
			// Covers the empty payload and every truncation shorter than a
			// header, which is the majority of malformed input.
			return Status(ErrorCode::InvalidArgument);
		}

		const uint8_t* bytes = data.GetData();

		if (ReadU32(bytes) != kDdsMagic)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		if (ReadU32(bytes + kOffHeaderSize) != kHeaderSize)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		if (ReadU32(bytes + kOffPfSize) != kPixelFormatSize)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const uint32_t caps2 = ReadU32(bytes + kOffCaps2);

		// ImageAsset is one 2D image. A cubemap is six faces and a volume is a
		// stack of slices; neither is that, so neither is silently reduced to
		// one face or one slice.
		if ((caps2 & kCaps2CubeMap) != 0 || (caps2 & kCaps2Volume) != 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const uint32_t flags = ReadU32(bytes + kOffFlags);

		// The DX10 extension replaces the interpretation of everything that
		// follows with a DXGI format and a dimension enum. Refusing it whole
		// is the only way to be sure none of those bytes are read as if they
		// were a 2D block layout.
		if ((flags & kFlagDx10) != 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		DdsHeader header;
		header.width             = ReadU32(bytes + kOffWidth);
		header.height            = ReadU32(bytes + kOffHeight);
		header.pitchOrLinearSize = ReadU32(bytes + kOffPitchOrLinear);

		// Shape before layout: a zero dimension makes every later calculation
		// meaningless, and it is the more fundamental fault when both are wrong.
		if (header.width == 0 || header.height == 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const uint32_t pfFlags  = ReadU32(bytes + kOffPfFlags);
		const uint32_t pfFourCC = ReadU32(bytes + kOffPfFourCC);
		const uint32_t bitCount = ReadU32(bytes + kOffPfRgbBitCount);

		if ((pfFlags & kPfFourCC) != 0)
		{
			if (pfFourCC == kFourCcDx10)
			{
				return Status(ErrorCode::InvalidArgument);
			}

			header.layout = LayoutFromFourCC(pfFourCC);
		}
		else
		{
			header.layout = LayoutFromMasks(
				pfFlags,
				bitCount,
				ReadU32(bytes + kOffPfRMask),
				ReadU32(bytes + kOffPfGMask),
				ReadU32(bytes + kOffPfBMask),
				ReadU32(bytes + kOffPfAMask));
		}

		if (header.layout == DdsLayout::Unknown)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		return header;
	}

	// ---------------------------------------------------------------------------
	// Block decoding
	// ---------------------------------------------------------------------------
	//
	// The three compressed formats share a colour block and differ in how
	// alpha travels. The decoders below work a block at a time into a local
	// 4x4 RGBA texel array, and the caller crops that array to the image,
	// which is what makes a non-multiple-of-four width correct rather than
	// approximately correct.

	uint16_t ReadU16(const uint8_t* bytes) noexcept
	{
		return static_cast<uint16_t>(
			static_cast<uint16_t>(bytes[0]) |
			(static_cast<uint16_t>(bytes[1]) << 8));
	}

	// Endpoint expansion for the 5:6:5 colour format, by bit replication:
	// 31 maps to 255 and 63 maps to 255 exactly, with no rounding table.
	void Expand565(uint16_t color, uint8_t* rgb) noexcept
	{
		const uint8_t r = static_cast<uint8_t>((color >> 11) & 0x1Fu);
		const uint8_t g = static_cast<uint8_t>((color >> 5) & 0x3Fu);
		const uint8_t b = static_cast<uint8_t>(color & 0x1Fu);

		rgb[0] = static_cast<uint8_t>((r << 3) | (r >> 2));
		rgb[1] = static_cast<uint8_t>((g << 2) | (g >> 4));
		rgb[2] = static_cast<uint8_t>((b << 3) | (b >> 2));
	}

	// The shared colour block: endpoints c0 and c1 in 5:6:5, then sixteen
	// 2-bit indices, least significant pair first.
	//
	// DXT1 decides its palette by comparing the endpoints as unsigned
	// integers, and the two cases differ in more than one texel: when c0 > c1
	// the block is four opaque colours, and when c0 <= c1 the third colour is
	// the halfway point and the fourth is transparent black. DXT3 and DXT5
	// carry alpha elsewhere, so they always use the four-colour form; that is
	// why dxt1Alpha selects the rule and the others do not.
	//
	// The alpha channel is written only when this block is the whole answer,
	// which is DXT1 and nothing else. DXT3 and DXT5 write their alpha first and
	// then call this with dxt1Alpha false, so the colour block must leave the
	// fourth byte of each texel exactly as it found it -- overwriting it with
	// an opaque 255 is how a transparent RAN texture silently turns solid.
	void DecodeColorBlock(const uint8_t* block, uint8_t* texels, bool dxt1Alpha) noexcept
	{
		const uint16_t c0 = ReadU16(block);
		const uint16_t c1 = ReadU16(block + 2);
		const uint32_t indices = ReadU32(block + 4);

		uint8_t palette[4][4];
		Expand565(c0, palette[0]);
		palette[0][3] = 0xFF;
		Expand565(c1, palette[1]);
		palette[1][3] = 0xFF;

		if (dxt1Alpha && c0 <= c1)
		{
			// Three colours plus transparency: 2 = halfway, 3 = fully clear.
			const uint16_t mid = static_cast<uint16_t>((c0 + c1) / 2u);
			Expand565(mid, palette[2]);
			palette[2][3] = 0xFF;
			palette[3][0] = 0;
			palette[3][1] = 0;
			palette[3][2] = 0;
			palette[3][3] = 0x00;
		}
		else
		{
			// Four opaque colours at one and two thirds of the way between the
			// endpoints. The arithmetic is on the 5:6:5 values, as the format
			// specifies, and the result is expanded afterwards.
			const uint16_t third0 = static_cast<uint16_t>((2u * c0 + c1) / 3u);
			const uint16_t third1 = static_cast<uint16_t>((c0 + 2u * c1) / 3u);
			Expand565(third0, palette[2]);
			palette[2][3] = 0xFF;
			Expand565(third1, palette[3]);
			palette[3][3] = 0xFF;
		}

		for (uint32_t texel = 0; texel < 16u; ++texel)
		{
			const uint32_t index = (indices >> (texel * 2u)) & 0x3u;
			texels[texel * 4u + 0u] = palette[index][0];
			texels[texel * 4u + 1u] = palette[index][1];
			texels[texel * 4u + 2u] = palette[index][2];

			if (dxt1Alpha)
			{
				texels[texel * 4u + 3u] = palette[index][3];
			}
		}
	}

	// DXT3 carries sixteen 4-bit alpha values explicitly, two per byte, low
	// nibble first. A nibble n is the byte n * 17, so 0xF is 255. That is eight
	// bytes of alpha, which with the eight-byte colour block makes the
	// format's sixteen.
	void DecodeExplicitAlpha(const uint8_t* block, uint8_t* texels) noexcept
	{
		for (uint32_t texel = 0; texel < 16u; ++texel)
		{
			const uint8_t packed = block[texel / 2u];
			const uint8_t nibble = (texel % 2u == 0u)
				? static_cast<uint8_t>(packed & 0x0Fu)
				: static_cast<uint8_t>((packed >> 4) & 0x0Fu);
			texels[texel * 4u + 3u] = static_cast<uint8_t>(nibble * 17u);
		}
	}

	// DXT5 carries two 8-bit alpha endpoints and sixteen 3-bit indices packed
	// into 48 bits, least significant group first.
	//
	// As in DXT1, the endpoints decide the table: a0 > a1 gives eight
	// interpolated values, and a0 <= a1 gives six, with index 7 meaning fully
	// transparent.
	void DecodeInterpolatedAlpha(const uint8_t* block, uint8_t* texels) noexcept
	{
		const uint8_t a0 = block[0];
		const uint8_t a1 = block[1];

		// Six index bytes as one 48-bit little-endian value. Built by
		// accumulation rather than a wider read, so no alignment is assumed.
		uint64_t indexBits = 0;
		for (uint32_t byte = 0; byte < 6u; ++byte)
		{
			indexBits |= static_cast<uint64_t>(block[2u + byte]) << (8u * byte);
		}

		uint8_t table[8];
		table[0] = a0;
		table[1] = a1;

		if (a0 > a1)
		{
			for (uint32_t i = 2u; i < 8u; ++i)
			{
				table[i] = static_cast<uint8_t>(((8u - i) * a0 + i * a1) / 8u);
			}
		}
		else
		{
			for (uint32_t i = 2u; i < 7u; ++i)
			{
				table[i] = static_cast<uint8_t>(((6u - i) * a0 + i * a1) / 6u);
			}
			table[7] = 0;
		}

		for (uint32_t texel = 0; texel < 16u; ++texel)
		{
			const uint32_t index = static_cast<uint32_t>((indexBits >> (3u * texel)) & 0x7u);
			texels[texel * 4u + 3u] = table[index];
		}
	}

	// Writes the surface data of one file into a row-major, top-row-first RGBA
	// or BGRA buffer sized for the image exactly.
	//
	// Every read is bounded by the top-level byte count the caller already
	// verified the file contains, and every write is bounded by the decoded
	// byte count, so a block that runs past the right or bottom edge of a
	// non-multiple-of-four image is cropped rather than written past.
	void DecodeCompressed(
		const uint8_t*       source,
		const DdsHeader&     header,
		std::vector<uint8_t>& pixels) noexcept
	{
		const size_t blockBytes = BlockBytes(header.layout);
		const uint32_t blocksX = (header.width + kBlockSide - 1u) / kBlockSide;
		const uint32_t blocksY = (header.height + kBlockSide - 1u) / kBlockSide;

		for (uint32_t blockY = 0; blockY < blocksY; ++blockY)
		{
			for (uint32_t blockX = 0; blockX < blocksX; ++blockX)
			{
				const size_t blockIndex = static_cast<size_t>(blockY) * blocksX + blockX;
				const uint8_t* block = source + blockIndex * blockBytes;

				uint8_t texels[16 * 4] = {};

				if (header.layout == DdsLayout::Dxt1)
				{
					DecodeColorBlock(block, texels, true);
				}
				else if (header.layout == DdsLayout::Dxt3)
				{
					// Eight bytes of explicit alpha, then the colour block.
					DecodeExplicitAlpha(block, texels);
					DecodeColorBlock(block + 8, texels, false);
				}
				else
				{
					// DXT5: six bytes of interpolated alpha, then the colour
					// block, for the same sixteen-byte layout.
					DecodeInterpolatedAlpha(block, texels);
					DecodeColorBlock(block + 8, texels, false);
				}

				for (uint32_t row = 0; row < kBlockSide; ++row)
				{
					const uint32_t y = blockY * kBlockSide + row;
					if (y >= header.height)
					{
						continue;
					}

					for (uint32_t column = 0; column < kBlockSide; ++column)
					{
						const uint32_t x = blockX * kBlockSide + column;
						if (x >= header.width)
						{
							continue;
						}

						const size_t texelIndex = static_cast<size_t>(row) * kBlockSide + column;
						uint8_t* destination = pixels.data() +
							(static_cast<size_t>(y) * header.width + x) * 4u;

						destination[0] = texels[texelIndex * 4u + 0u];
						destination[1] = texels[texelIndex * 4u + 1u];
						destination[2] = texels[texelIndex * 4u + 2u];
						destination[3] = texels[texelIndex * 4u + 3u];
					}
				}
			}
		}
	}

	// Uncompressed 32-bit surfaces are already texels, so this is a straight
	// copy in both supported cases. The RGBA/BGRA distinction is not a
	// per-texel operation: it was already decided when the buffer's
	// ImageFormat was chosen, and B8G8R8A8_UNorm preserves the file's channel
	// order rather than re-ordering it to match an RGBA file.
	void DecodeUncompressed(
		const uint8_t*       source,
		std::vector<uint8_t>& pixels) noexcept
	{
		const size_t texelCount = pixels.size() / 4u;

		for (size_t texel = 0; texel < texelCount; ++texel)
		{
			const uint8_t* in = source + texel * 4u;
			uint8_t* out = pixels.data() + texel * 4u;

			out[0] = in[0];
			out[1] = in[1];
			out[2] = in[2];
			out[3] = in[3];
		}
	}

} // namespace

Result<ImageAsset> DdsImageDecoder::DecodeImage(const ResourceData& data)
{
	const Result<DdsHeader> parsed = ParseHeader(data);
	if (parsed.IsError())
	{
		return parsed.GetStatus();
	}

	const DdsHeader header = parsed.GetValue();

	// The format this file decodes to. A compressed surface and an RGBA one
	// both become R8G8B8A8_UNorm; a BGRA one keeps its own channel order,
	// which is exactly what ImageFormat::B8G8R8A8_UNorm exists to express.
	const ImageFormat format = (header.layout == DdsLayout::Bgra32)
		? ImageFormat::B8G8R8A8_UNorm
		: ImageFormat::R8G8B8A8_UNorm;

	// The asset's own limits, applied before anything is allocated: 16384 per
	// side and 256 MiB of decoded pixels. A header that survives the DDS checks
	// and fails here is simply too large to be an image this layer accepts.
	const Result<size_t> decodedByteCount = ComputeImageByteCount(header.width, header.height, format);
	if (decodedByteCount.IsError())
	{
		return decodedByteCount.GetStatus();
	}

	const Result<size_t> topLevelByteCount = TopLevelByteCount(header.width, header.height, header.layout);
	if (topLevelByteCount.IsError())
	{
		return topLevelByteCount.GetStatus();
	}

	const size_t levelBytes = topLevelByteCount.GetValue();

	// dwPitchOrLinearSize is the writer's own claim about the top level. Zero
	// means "not stated" and is accepted; any stated value that disagrees with
	// the geometry is a header contradicting itself, and a header that lies
	// about its own size is not worth decoding.
	if (header.pitchOrLinearSize != 0 && static_cast<size_t>(header.pitchOrLinearSize) != levelBytes)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// The file must physically contain the top level. Bytes after it are the
	// mip chain, which is expected -- most RAN textures carry nine to eleven
	// levels -- and are ignored rather than refused. A file that ends before
	// the top level does is refused, and this subtraction cannot underflow
	// because ParseHeader already proved the header is present in full.
	if (data.GetSize() - kDdsFileHeaderBytes < levelBytes)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	std::vector<uint8_t> pixels(decodedByteCount.GetValue());
	const uint8_t* source = data.GetData() + kDdsFileHeaderBytes;

	if (header.layout == DdsLayout::Rgba32 || header.layout == DdsLayout::Bgra32)
	{
		DecodeUncompressed(source, pixels);
	}
	else
	{
		DecodeCompressed(source, header, pixels);
	}

	// Built through Create, so the same ceilings and the same exact-size rule
	// that guard every other image in the client guard this one too.
	return ImageAsset::Create(header.width, header.height, format, std::move(pixels));
}

} // namespace Modern::Client

