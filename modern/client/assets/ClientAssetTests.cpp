// CLIENT-007: typed asset / decoder boundary tests.
//
// The asset layer turns resource bytes into validated CPU-side values. These
// cases pin down both halves of that: the value types that carry an image, and
// the decoder contract that produces one.
//
// Coverage:
//  1.  ImageFormat names and bytes-per-pixel, including unknown enumerators.
//  2.  ComputeImageByteCount agrees with the layout it is given.
//  3.  ComputeImageByteCount refuses unknown layouts and zero dimensions.
//  4.  ComputeImageByteCount refuses oversized and overflowing geometry.
//  5.  A valid ImageAsset is accepted and reports its geometry.
//  6.  Zero width, zero height and zero both are refused.
//  7.  An unsupported pixel layout is refused, with and without matching bytes.
//  8.  The payload size must be exact: short and long are both refused.
//  9.  Oversized and overflowing images are refused rather than wrapped.
// 10.  Every pixel byte survives construction unchanged.
// 11.  ImageAsset is a value type with no invalid state, and owns its pixels.
// 12.  The decoder satisfies IImageDecoder and works through the interface.
// 13.  Decoding is deterministic across repeats and across instances.
// 14.  Empty data is refused.
// 15.  A truncated header is refused, at every length.
// 16.  A malformed header (magic, version) is refused.
// 17.  An unknown or unsupported format byte is refused.
// 18.  A zero dimension in the header is refused.
// 19.  A truncated payload is refused, at every length below the full one.
// 20.  Trailing bytes are refused.
// 21.  Geometry beyond the ceilings is refused before anything is allocated.
// 22.  Decoded pixels match the container byte for byte.
// 23.  The decoder preserves the declared layout instead of normalising it.
// 24.  The asset layer needs no renderer, no graphics API and no legacy header.
// 25.  The decoder needs no ResourceManager, provider or id.
// 26.  MemoryResourceProvider -> ResourceManager -> ResourceData -> asset.
// 27.  FileSystemResourceProvider -> ResourceManager -> ResourceData -> asset.
// 28.  Clearing the manager's byte cache does not change the decoded asset.
// 29.  A decode failure leaves the resource layer untouched.
// 30.  No RAN installation is required: temporary data only.
// 31.  No legacy format is decoded; the decoder knows MIMG and nothing else.
//
// The MIMG container used here is defined in TestImageDecoder.h. It is a test
// representation invented for this milestone, not a RAN format, and every
// container in these cases is assembled by hand from that description rather
// than through a shared writer, so the reader is checked against an
// independent encoder. Every filesystem case builds its own root under the
// system temporary directory and removes it again.

#include "TestHarness.h"

#include "assets/AssetTypes.h"
#include "assets/ImageAsset.h"
#include "assets/ImageDecoder.h"
#include "assets/TestImageDecoder.h"
#include "resources/FileSystemResourceProvider.h"
#include "resources/MemoryResourceProvider.h"
#include "resources/ResourceData.h"
#include "resources/ResourceId.h"
#include "resources/ResourceManager.h"
#include "resources/ResourceProvider.h"
#include "types/Result.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

using namespace Modern;
using namespace Modern::Client;

namespace
{
	// -----------------------------------------------------------------------
	// MIMG assembly
	//
	// Written from the container description in TestImageDecoder.h. A test
	// calling a writer from the implementation would agree with a broken
	// reader, so the header is assembled field by field here instead.
	// -----------------------------------------------------------------------

	std::vector<uint8_t> MakeHeader(
		uint16_t    width,
		uint16_t    height,
		ImageFormat format,
		uint8_t     version = TestImageDecoder::kVersion,
		const char* magic   = "MIMG")
	{
		std::vector<uint8_t> header(TestImageDecoder::kHeaderSize, 0);
		header[0] = static_cast<uint8_t>(magic[0]);
		header[1] = static_cast<uint8_t>(magic[1]);
		header[2] = static_cast<uint8_t>(magic[2]);
		header[3] = static_cast<uint8_t>(magic[3]);
		header[4] = version;
		header[5] = static_cast<uint8_t>(format);
		header[6] = static_cast<uint8_t>(width & 0xFFu);
		header[7] = static_cast<uint8_t>((width >> 8) & 0xFFu);
		header[8] = static_cast<uint8_t>(height & 0xFFu);
		header[9] = static_cast<uint8_t>((height >> 8) & 0xFFu);
		return header;
	}

	// Deterministic pixel bytes covering every byte value: for a 256-byte
	// payload each value 0x00..0xFF appears exactly once, so a decoder that
	// stops at a zero byte, sign-extends or shifts the payload is caught.
	std::vector<uint8_t> MakePayload(size_t size)
	{
		std::vector<uint8_t> bytes(size);
		for (size_t i = 0; i < size; ++i)
		{
			bytes[i] = static_cast<uint8_t>(i % 256u);
		}
		return bytes;
	}

	std::vector<uint8_t> Concat(
		const std::vector<uint8_t>& header,
		const std::vector<uint8_t>& payload)
	{
		std::vector<uint8_t> bytes = header;
		bytes.insert(bytes.end(), payload.begin(), payload.end());
		return bytes;
	}

	// A container whose geometry and payload agree, for the sizes the cases use.
	std::vector<uint8_t> MakeImage(uint16_t width, uint16_t height, ImageFormat format)
	{
		const size_t payloadSize = static_cast<size_t>(width) * static_cast<size_t>(height) *
		                           static_cast<size_t>(BytesPerPixel(format));
		return Concat(MakeHeader(width, height, format), MakePayload(payloadSize));
	}

	// Rejection cases only read the code, so nothing here touches a value that
	// is not there.
	ErrorCode DecodeError(IImageDecoder& decoder, const ResourceData& data)
	{
		return decoder.DecodeImage(data).GetError();
	}

	// -----------------------------------------------------------------------
	// Test fixtures
	// -----------------------------------------------------------------------

	unsigned NextSequence() noexcept
	{
		static unsigned sequence = 0;
		return ++sequence;
	}

	// A private asset root for one test, removed on destruction.
	//
	// Deliberately smaller than the CLIENT-006 fixture: this suite never aims
	// at a file outside the root, so there is no base/root split and no
	// WriteOutside(). The tests own their root so that they run on any machine
	// and clean up even if one returns early.
	class TempRoot
	{
	public:
		TempRoot()
		{
			std::error_code ec;
			const std::filesystem::path systemTemp = std::filesystem::temp_directory_path(ec);
			if (ec)
			{
				return;
			}

			const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
			m_root = systemTemp /
			         ("modern_client007_" + std::to_string(stamp) + "_" + std::to_string(NextSequence()));

			std::filesystem::remove_all(m_root, ec);
			ec.clear();
			std::filesystem::create_directories(m_root, ec);
			if (ec)
			{
				m_root.clear();
			}
		}

		~TempRoot()
		{
			std::error_code ec;
			std::filesystem::remove_all(m_root, ec);
		}

		TempRoot(const TempRoot&) = delete;
		TempRoot& operator=(const TempRoot&) = delete;

		const std::filesystem::path& Path() const noexcept { return m_root; }
		bool IsUsable() const noexcept { return !m_root.empty(); }

		// Writes binary bytes below the root, creating parent directories.
		bool Write(const std::filesystem::path& relative, const std::vector<uint8_t>& bytes) const
		{
			const std::filesystem::path target = m_root / relative;

			std::error_code ec;
			std::filesystem::create_directories(target.parent_path(), ec);
			if (ec)
			{
				return false;
			}

			std::ofstream out(target, std::ios::binary | std::ios::trunc);
			if (!out)
			{
				return false;
			}

			if (!bytes.empty())
			{
				out.write(reinterpret_cast<const char*>(bytes.data()),
					static_cast<std::streamsize>(bytes.size()));
			}

			return static_cast<bool>(out);
		}

	private:
		std::filesystem::path m_root;
	};

	// Builds a ResourceId, asserting that the identifier layer accepts it: the
	// name rule is CLIENT-005's, so a failure here is a fixture problem rather
	// than a result of this milestone.
	ResourceId MakeId(const char* name)
	{
		const Result<ResourceId> id = ResourceId::Create(name);
		CHECK(id.IsOk());
		return id.GetValueOr(ResourceId());
	}
}

// ---------------------------------------------------------------------------
// 1-4: asset vocabulary
// ---------------------------------------------------------------------------

MODERN_TEST(ClientAssets_FormatNamesAndBytesPerPixel)
{
	CHECK_EQ(std::string(ToString(ImageFormat::Unknown)), "Unknown");
	CHECK_EQ(std::string(ToString(ImageFormat::R8_UNorm)), "R8_UNorm");
	CHECK_EQ(std::string(ToString(ImageFormat::R8G8B8_UNorm)), "R8G8B8_UNorm");
	CHECK_EQ(std::string(ToString(ImageFormat::R8G8B8A8_UNorm)), "R8G8B8A8_UNorm");
	CHECK_EQ(std::string(ToString(ImageFormat::B8G8R8A8_UNorm)), "B8G8R8A8_UNorm");

	// A format byte read out of a file is data, not an enumerator: a value with
	// no layout behind it is reported as invalid rather than guessed at.
	CHECK_EQ(std::string(ToString(static_cast<ImageFormat>(200))), "Invalid");
	CHECK_EQ(std::string(ToString(static_cast<ImageFormat>(255))), "Invalid");

	CHECK_EQ(BytesPerPixel(ImageFormat::Unknown), 0u);
	CHECK_EQ(BytesPerPixel(ImageFormat::R8_UNorm), 1u);
	CHECK_EQ(BytesPerPixel(ImageFormat::R8G8B8_UNorm), 3u);
	CHECK_EQ(BytesPerPixel(ImageFormat::R8G8B8A8_UNorm), 4u);
	CHECK_EQ(BytesPerPixel(ImageFormat::B8G8R8A8_UNorm), 4u);

	// Zero is a refusal, so every caller has to handle it.
	CHECK_EQ(BytesPerPixel(static_cast<ImageFormat>(7)), 0u);
	CHECK_EQ(BytesPerPixel(static_cast<ImageFormat>(200)), 0u);
}

MODERN_TEST(ClientAssets_ComputeImageByteCountMatchesLayout)
{
	const Result<size_t> grey = ComputeImageByteCount(4, 2, ImageFormat::R8_UNorm);
	CHECK(grey.IsOk());
	CHECK_EQ(grey.GetValue(), static_cast<size_t>(8));

	const Result<size_t> rgb = ComputeImageByteCount(4, 2, ImageFormat::R8G8B8_UNorm);
	CHECK(rgb.IsOk());
	CHECK_EQ(rgb.GetValue(), static_cast<size_t>(24));

	const Result<size_t> rgba = ComputeImageByteCount(4, 2, ImageFormat::R8G8B8A8_UNorm);
	CHECK(rgba.IsOk());
	CHECK_EQ(rgba.GetValue(), static_cast<size_t>(32));

	// Two four-byte layouts have the same count but are not the same layout: a
	// byte count is a size, not an identity.
	const Result<size_t> bgra = ComputeImageByteCount(4, 2, ImageFormat::B8G8R8A8_UNorm);
	CHECK(bgra.IsOk());
	CHECK_EQ(bgra.GetValue(), static_cast<size_t>(32));

	// A single pixel of each layout.
	CHECK_EQ(ComputeImageByteCount(1, 1, ImageFormat::R8_UNorm).GetValue(), static_cast<size_t>(1));
	CHECK_EQ(ComputeImageByteCount(1, 1, ImageFormat::R8G8B8_UNorm).GetValue(), static_cast<size_t>(3));
	CHECK_EQ(ComputeImageByteCount(1, 1, ImageFormat::R8G8B8A8_UNorm).GetValue(), static_cast<size_t>(4));
	CHECK_EQ(ComputeImageByteCount(1, 1, ImageFormat::B8G8R8A8_UNorm).GetValue(), static_cast<size_t>(4));

	// A realistic size: 1024 x 1024 RGBA is 4 MiB, nowhere near the ceilings.
	CHECK_EQ(ComputeImageByteCount(1024, 1024, ImageFormat::R8G8B8A8_UNorm).GetValue(),
		static_cast<size_t>(4194304));

	// Non-square geometry is width * height, not the larger dimension squared.
	CHECK_EQ(ComputeImageByteCount(3, 5, ImageFormat::R8_UNorm).GetValue(), static_cast<size_t>(15));
}

MODERN_TEST(ClientAssets_ComputeImageByteCountRejectsUnsupportedAndZero)
{
	CHECK_EQ(ComputeImageByteCount(4, 4, ImageFormat::Unknown).GetError(), ErrorCode::InvalidArgument);
	CHECK_EQ(ComputeImageByteCount(4, 4, static_cast<ImageFormat>(200)).GetError(),
		ErrorCode::InvalidArgument);

	CHECK_EQ(ComputeImageByteCount(0, 4, ImageFormat::R8G8B8A8_UNorm).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ComputeImageByteCount(4, 0, ImageFormat::R8G8B8A8_UNorm).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ComputeImageByteCount(0, 0, ImageFormat::R8G8B8A8_UNorm).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ComputeImageByteCount(0, 0, ImageFormat::R8_UNorm).GetError(), ErrorCode::InvalidArgument);

	// An unsupported layout is refused even when the geometry is enormous:
	// refusing the layout first keeps that refusal independent of the
	// arithmetic that follows it.
	CHECK_EQ(ComputeImageByteCount(65535, 65535, ImageFormat::Unknown).GetError(),
		ErrorCode::InvalidArgument);

	CHECK(ComputeImageByteCount(4, 4, ImageFormat::Unknown).IsError());
}

MODERN_TEST(ClientAssets_ComputeImageByteCountRejectsExcessiveDimensions)
{
	// One past the dimension ceiling, in both orders. Either byte count would
	// fit in a size_t, so it is the ceiling doing the refusing.
	CHECK_EQ(ComputeImageByteCount(kMaxImageDimension + 1, 1, ImageFormat::R8_UNorm).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ComputeImageByteCount(1, kMaxImageDimension + 1, ImageFormat::R8_UNorm).GetError(),
		ErrorCode::InvalidArgument);

	// At the ceiling with one byte per pixel the byte count is exactly the byte
	// ceiling: the limits are inclusive, which is what makes them checkable.
	const Result<size_t> atBothCeilings =
		ComputeImageByteCount(kMaxImageDimension, kMaxImageDimension, ImageFormat::R8_UNorm);
	CHECK(atBothCeilings.IsOk());
	CHECK_EQ(atBothCeilings.GetValue(), kMaxImageBytes);

	// One byte per pixel more exceeds the byte ceiling and is refused rather
	// than wrapped or truncated.
	CHECK_EQ(ComputeImageByteCount(kMaxImageDimension, kMaxImageDimension, ImageFormat::R8G8B8_UNorm).GetError(),
		ErrorCode::InvalidArgument);

	// A header claiming 65535 x 65535 would be about 17 GB of RGBA. The answer
	// is the same code on a 32-bit and a 64-bit build, because the policy
	// ceilings are reached before size_t is.
	CHECK_EQ(ComputeImageByteCount(65535, 65535, ImageFormat::R8G8B8A8_UNorm).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ComputeImageByteCount(65535, 65535, ImageFormat::R8_UNorm).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ComputeImageByteCount(65535, 65535, ImageFormat::R8G8B8_UNorm).GetError(),
		ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// 5-11: ImageAsset invariants
// ---------------------------------------------------------------------------

MODERN_TEST(ClientAssets_ImageAssetAcceptsValidGeometry)
{
	const std::vector<uint8_t> pixels = MakePayload(24);
	const Result<ImageAsset> created = ImageAsset::Create(3, 2, ImageFormat::R8G8B8A8_UNorm, pixels);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}

	const ImageAsset& image = created.GetValue();
	CHECK_EQ(image.GetWidth(), 3u);
	CHECK_EQ(image.GetHeight(), 2u);
	CHECK_EQ(std::string(ToString(image.GetFormat())), "R8G8B8A8_UNorm");
	CHECK_EQ(image.GetPixelCount(), static_cast<size_t>(6));
	CHECK_EQ(image.GetPixelByteCount(), static_cast<size_t>(24));
	CHECK(image.GetPixels() == pixels);

	// A one-pixel image is legal: nothing here requires even dimensions or a
	// power-of-two size, because a texture does not have to be either.
	const Result<ImageAsset> minimal = ImageAsset::Create(1, 1, ImageFormat::R8_UNorm, MakePayload(1));
	CHECK(minimal.IsOk());
	if (minimal.IsError())
	{
		return;
	}
	CHECK_EQ(minimal.GetValue().GetPixelCount(), static_cast<size_t>(1));
	CHECK_EQ(minimal.GetValue().GetPixelByteCount(), static_cast<size_t>(1));
}

MODERN_TEST(ClientAssets_ImageAssetRejectsZeroDimensions)
{
	const std::vector<uint8_t> empty;

	CHECK_EQ(ImageAsset::Create(0, 2, ImageFormat::R8G8B8A8_UNorm, empty).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ImageAsset::Create(2, 0, ImageFormat::R8G8B8A8_UNorm, empty).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ImageAsset::Create(0, 0, ImageFormat::R8G8B8A8_UNorm, empty).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ImageAsset::Create(0, 0, ImageFormat::R8_UNorm, empty).GetError(),
		ErrorCode::InvalidArgument);

	// A zero dimension is refused even when bytes are present: the geometry is
	// the thing that is wrong, so no payload size can rescue it.
	CHECK_EQ(ImageAsset::Create(0, 2, ImageFormat::R8G8B8A8_UNorm, MakePayload(8)).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ImageAsset::Create(2, 0, ImageFormat::R8G8B8A8_UNorm, MakePayload(8)).GetError(),
		ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientAssets_ImageAssetRejectsUnsupportedFormat)
{
	CHECK_EQ(ImageAsset::Create(1, 1, ImageFormat::Unknown, MakePayload(4)).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ImageAsset::Create(1, 1, static_cast<ImageFormat>(200), MakePayload(4)).GetError(),
		ErrorCode::InvalidArgument);

	// The geometry that would be right for some other layout does not turn an
	// unsupported layout into a supported one: no layout is inferred.
	CHECK_EQ(ImageAsset::Create(2, 2, ImageFormat::Unknown, MakePayload(16)).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ImageAsset::Create(4, 4, static_cast<ImageFormat>(255), MakePayload(64)).GetError(),
		ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientAssets_ImageAssetRequiresExactPayloadSize)
{
	// 2 x 2 RGBA is 16 bytes.
	CHECK(ImageAsset::Create(2, 2, ImageFormat::R8G8B8A8_UNorm, MakePayload(16)).IsOk());
	CHECK_EQ(ImageAsset::Create(2, 2, ImageFormat::R8G8B8A8_UNorm, MakePayload(15)).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ImageAsset::Create(2, 2, ImageFormat::R8G8B8A8_UNorm, MakePayload(17)).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ImageAsset::Create(2, 2, ImageFormat::R8G8B8A8_UNorm, MakePayload(0)).GetError(),
		ErrorCode::InvalidArgument);

	// Row padding is not part of the layout: 3 x 1 RGB is 9 bytes, so a padded
	// 12-byte row is a size mismatch and not a wider stride.
	CHECK(ImageAsset::Create(3, 1, ImageFormat::R8G8B8_UNorm, MakePayload(9)).IsOk());
	CHECK_EQ(ImageAsset::Create(3, 1, ImageFormat::R8G8B8_UNorm, MakePayload(10)).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ImageAsset::Create(3, 1, ImageFormat::R8G8B8_UNorm, MakePayload(12)).GetError(),
		ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientAssets_ImageAssetRejectsExcessiveOrOverflowingImages)
{
	// Past the dimension ceiling. The byte count for the second case would fit
	// in a size_t, which is why the ceiling has to exist on its own.
	CHECK_EQ(ImageAsset::Create(kMaxImageDimension + 1, 1, ImageFormat::R8_UNorm, MakePayload(1)).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ImageAsset::Create(1, kMaxImageDimension + 1, ImageFormat::R8_UNorm, MakePayload(1)).GetError(),
		ErrorCode::InvalidArgument);

	// 65535 x 65535 RGBA claims about 17 GB. The refusal happens on the
	// geometry and the payload is never consulted, so a small buffer cannot
	// make an enormous image look correct.
	CHECK_EQ(ImageAsset::Create(65535, 65535, ImageFormat::R8G8B8A8_UNorm, MakePayload(16)).GetError(),
		ErrorCode::InvalidArgument);

	// 16384 x 16384 RGBA is 1 GiB: under the dimension ceiling, over the byte
	// ceiling, and refused rather than wrapped into a smaller allocation.
	CHECK_EQ(ImageAsset::Create(kMaxImageDimension, kMaxImageDimension, ImageFormat::R8G8B8A8_UNorm,
		            MakePayload(16)).GetError(),
		ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientAssets_ImageAssetPreservesPixelsExactly)
{
	// 8 x 8 RGBA is 256 bytes, so every byte value appears exactly once. A
	// layer that drops, reorders, sign-extends or stops at a byte is caught
	// here rather than three systems later.
	const std::vector<uint8_t> pixels = MakePayload(256);
	const Result<ImageAsset> created = ImageAsset::Create(8, 8, ImageFormat::R8G8B8A8_UNorm, pixels);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}

	const std::vector<uint8_t>& stored = created.GetValue().GetPixels();
	CHECK_EQ(stored.size(), static_cast<size_t>(256));
	CHECK(stored == pixels);

	size_t mismatches = 0;
	for (size_t i = 0; i < stored.size(); ++i)
	{
		if (stored[i] != static_cast<uint8_t>(i))
		{
			++mismatches;
		}
	}
	CHECK_EQ(mismatches, static_cast<size_t>(0));

	// The extremes specifically: a zero byte is data, not a terminator, and the
	// last byte is read rather than being one byte short.
	CHECK_EQ(stored.front(), static_cast<uint8_t>(0));
	CHECK_EQ(stored.back(), static_cast<uint8_t>(255));
}

MODERN_TEST(ClientAssets_ImageAssetIsAValueWithNoInvalidState)
{
	// There is deliberately no default constructor, so an image that has not
	// passed validation cannot be named, held or passed around. That is why no
	// case in this file tests for an invalid ImageAsset: the state does not
	// exist to be tested.
	static_assert(!std::is_default_constructible<ImageAsset>::value,
		"ImageAsset must not have an invalid default state");
	static_assert(std::is_copy_constructible<ImageAsset>::value,
		"ImageAsset must be a copyable value");
	static_assert(std::is_move_constructible<ImageAsset>::value,
		"ImageAsset must be movable");
	static_assert(std::is_same<decltype(std::declval<const ImageAsset&>().GetPixels()),
		const std::vector<uint8_t>&>::value,
		"ImageAsset must hand out CPU bytes, not a backend handle");
	static_assert(std::is_same<decltype(std::declval<const ImageAsset&>().GetPixelByteCount()),
		size_t>::value,
		"ImageAsset must report its byte count as a size");

	// The asset owns its pixels: the vector it was built from can be changed
	// afterwards without changing the image, and a copy is a second value.
	std::vector<uint8_t> source = MakePayload(4);
	const Result<ImageAsset> created = ImageAsset::Create(1, 1, ImageFormat::R8G8B8A8_UNorm, source);
	CHECK(created.IsOk());
	if (created.IsError())
	{
		return;
	}

	source[0] = 0xFE;
	source[1] = 0xFE;
	CHECK(created.GetValue().GetPixels() != source);
	CHECK_EQ(created.GetValue().GetPixels()[0], static_cast<uint8_t>(0));

	const ImageAsset copy = created.GetValue();
	CHECK(copy.GetPixels() == created.GetValue().GetPixels());
	CHECK_EQ(copy.GetWidth(), created.GetValue().GetWidth());
	CHECK_EQ(copy.GetPixelByteCount(), created.GetValue().GetPixelByteCount());
}

// ---------------------------------------------------------------------------
// 12-23: decoder contract
// ---------------------------------------------------------------------------

MODERN_TEST(ClientAssets_TestDecoderSatisfiesImageDecoderContract)
{
	// The interface is abstract, the implementation is a leaf, and the signature
	// is the one that was designed: bytes in, Result<ImageAsset> out.
	static_assert(std::is_abstract<IImageDecoder>::value,
		"IImageDecoder must be a pure interface");
	static_assert(std::is_base_of<IImageDecoder, TestImageDecoder>::value,
		"TestImageDecoder must satisfy IImageDecoder");
	static_assert(std::is_same<decltype(std::declval<IImageDecoder&>().DecodeImage(
		std::declval<const ResourceData&>())), Result<ImageAsset>>::value,
		"DecodeImage must return Result<ImageAsset>");

	CHECK_EQ(TestImageDecoder::kHeaderSize, static_cast<size_t>(10));
	CHECK_EQ(TestImageDecoder::kVersion, static_cast<uint8_t>(1));

	// Used entirely through the interface, which is the only way the layers
	// above ever see a decoder.
	TestImageDecoder concrete;
	IImageDecoder&   decoder = concrete;

	const std::vector<uint8_t> bytes = MakeImage(2, 2, ImageFormat::R8G8B8_UNorm);
	const Result<ImageAsset> decoded = decoder.DecodeImage(ResourceData(bytes));
	CHECK(decoded.IsOk());
	if (decoded.IsError())
	{
		return;
	}

	CHECK_EQ(decoded.GetValue().GetWidth(), 2u);
	CHECK_EQ(decoded.GetValue().GetHeight(), 2u);
	CHECK_EQ(std::string(ToString(decoded.GetValue().GetFormat())), "R8G8B8_UNorm");
	CHECK_EQ(decoded.GetValue().GetPixelByteCount(), static_cast<size_t>(12));
}

MODERN_TEST(ClientAssets_DecodeIsDeterministic)
{
	TestImageDecoder decoder;

	const std::vector<uint8_t> bytes = MakeImage(4, 3, ImageFormat::B8G8R8A8_UNorm);
	const ResourceData data(bytes);

	const Result<ImageAsset> first = decoder.DecodeImage(data);
	const Result<ImageAsset> second = decoder.DecodeImage(data);
	CHECK(first.IsOk());
	CHECK(second.IsOk());
	if (first.IsError() || second.IsError())
	{
		return;
	}

	CHECK_EQ(second.GetValue().GetWidth(), first.GetValue().GetWidth());
	CHECK_EQ(second.GetValue().GetHeight(), first.GetValue().GetHeight());
	CHECK_EQ(second.GetValue().GetPixelByteCount(), first.GetValue().GetPixelByteCount());
	CHECK_EQ(std::string(ToString(second.GetValue().GetFormat())),
		std::string(ToString(first.GetValue().GetFormat())));
	CHECK(second.GetValue().GetPixels() == first.GetValue().GetPixels());

	// A second decoder over the same bytes produces the same image: there is no
	// hidden state for a fresh decoder to disagree about, which is what
	// "stateless" has to mean to be worth claiming.
	TestImageDecoder other;
	const Result<ImageAsset> third = other.DecodeImage(data);
	CHECK(third.IsOk());
	if (third.IsError())
	{
		return;
	}
	CHECK(third.GetValue().GetPixels() == first.GetValue().GetPixels());

	// A failure is equally deterministic: the same malformed bytes are refused
	// the same way on every attempt, so no caller can see one input succeed and
	// fail on alternate calls.
	const std::vector<uint8_t> wrongVersion =
		Concat(MakeHeader(4, 3, ImageFormat::R8G8B8A8_UNorm, static_cast<uint8_t>(9)), MakePayload(48));
	CHECK_EQ(decoder.DecodeImage(ResourceData(wrongVersion)).GetError(), ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientAssets_EmptyDataIsRejected)
{
	TestImageDecoder decoder;

	// No bytes at all: there is no header to read, so this is refused rather
	// than treated as a zero-sized image.
	CHECK(ResourceData().IsEmpty());
	CHECK_EQ(DecodeError(decoder, ResourceData()), ErrorCode::InvalidArgument);
	CHECK_EQ(DecodeError(decoder, ResourceData(std::vector<uint8_t>())), ErrorCode::InvalidArgument);
	CHECK_EQ(DecodeError(decoder, ResourceData(std::string_view())), ErrorCode::InvalidArgument);

	// A well-formed header with no payload is a different failure - the
	// geometry expects pixels that are not there - and is refused too.
	CHECK_EQ(DecodeError(decoder, ResourceData(MakeHeader(1, 1, ImageFormat::R8G8B8A8_UNorm))),
		ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientAssets_TruncatedHeaderIsRejected)
{
	TestImageDecoder decoder;

	// Every length below the ten-byte header, including zero. None of these has
	// enough bytes to describe an image.
	for (size_t size = 0; size < TestImageDecoder::kHeaderSize; ++size)
	{
		const ResourceData data(MakePayload(size));
		CHECK_EQ(DecodeError(decoder, data), ErrorCode::InvalidArgument);
	}

	// A real header with its last byte cut off is still short: the fields that
	// did arrive do not make up for the one that did not.
	const std::vector<uint8_t> header = MakeHeader(2, 2, ImageFormat::R8_UNorm);
	const std::vector<uint8_t> shortHeader(
		header.begin(),
		header.begin() + static_cast<std::ptrdiff_t>(TestImageDecoder::kHeaderSize - 1));
	CHECK_EQ(DecodeError(decoder, ResourceData(shortHeader)), ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientAssets_MalformedHeaderIsRejected)
{
	TestImageDecoder decoder;

	// Each of the four magic bytes, one at a time: a reader comparing only the
	// first letter, or the first two, is caught rather than passing.
	for (size_t index = 0; index < 4; ++index)
	{
		std::vector<uint8_t> bytes = MakeImage(2, 2, ImageFormat::R8_UNorm);
		bytes[index] = static_cast<uint8_t>(bytes[index] ^ 0x20u);
		CHECK_EQ(DecodeError(decoder, ResourceData(bytes)), ErrorCode::InvalidArgument);
	}

	// Unsupported versions: zero, one past the only supported value, and the
	// largest a byte can hold. A version is refused rather than best-effort
	// parsed, because there is no other version of this format to support.
	const uint8_t badVersions[] = { 0u, 2u, 255u };
	for (size_t i = 0; i < 3; ++i)
	{
		const std::vector<uint8_t> bytes =
			Concat(MakeHeader(2, 2, ImageFormat::R8_UNorm, badVersions[i]), MakePayload(4));
		CHECK_EQ(DecodeError(decoder, ResourceData(bytes)), ErrorCode::InvalidArgument);
	}

	// Bad magic and bad version together are still one refusal, not a cascade
	// of partial parses with a partial result at the end.
	std::vector<uint8_t> both = MakeImage(2, 2, ImageFormat::R8_UNorm);
	both[0] = static_cast<uint8_t>('X');
	both[4] = static_cast<uint8_t>(7);
	CHECK_EQ(DecodeError(decoder, ResourceData(both)), ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientAssets_UnknownFormatIsRejected)
{
	TestImageDecoder decoder;

	// Format byte 0 is Unknown, and every other value with no layout behind it
	// is refused the same way. A payload of the right size for some other
	// layout does not rescue the file, because no layout is ever inferred.
	//
	// The names differ and that is the point: 0 is the enumerator that means
	// "no layout", so it has a name, while an unassigned byte is not an
	// enumerator at all and is reported as invalid.
	CHECK_EQ(std::string(ToString(static_cast<ImageFormat>(0))), "Unknown");
	CHECK_EQ(std::string(ToString(static_cast<ImageFormat>(5))), "Invalid");
	CHECK_EQ(std::string(ToString(static_cast<ImageFormat>(200))), "Invalid");

	const uint8_t unsupported[] = { 0u, 5u, 200u, 255u };
	for (size_t i = 0; i < 4; ++i)
	{
		const ImageFormat format = static_cast<ImageFormat>(unsupported[i]);
		CHECK_EQ(BytesPerPixel(format), 0u);

		const std::vector<uint8_t> bytes = Concat(MakeHeader(2, 2, format), MakePayload(4));
		CHECK_EQ(DecodeError(decoder, ResourceData(bytes)), ErrorCode::InvalidArgument);
	}

	// Unknown with a payload that would suit a 2 x 2 RGBA image: still refused.
	const std::vector<uint8_t> plausible =
		Concat(MakeHeader(2, 2, ImageFormat::Unknown), MakePayload(16));
	CHECK_EQ(DecodeError(decoder, ResourceData(plausible)), ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientAssets_ZeroDimensionHeaderIsRejected)
{
	TestImageDecoder decoder;

	// A header may say zero; an image may not be zero. The refusal is the same
	// code as any other undecodable input, with no payload present at all.
	CHECK_EQ(DecodeError(decoder, ResourceData(Concat(MakeHeader(0, 4, ImageFormat::R8G8B8A8_UNorm),
		            MakePayload(0)))),
		ErrorCode::InvalidArgument);
	CHECK_EQ(DecodeError(decoder, ResourceData(Concat(MakeHeader(4, 0, ImageFormat::R8G8B8A8_UNorm),
		            MakePayload(0)))),
		ErrorCode::InvalidArgument);
	CHECK_EQ(DecodeError(decoder, ResourceData(Concat(MakeHeader(0, 0, ImageFormat::R8_UNorm),
		            MakePayload(0)))),
		ErrorCode::InvalidArgument);

	// Zero in one dimension with bytes present is still refused: the header
	// describes no pixels, so those bytes cannot be its pixels.
	CHECK_EQ(DecodeError(decoder, ResourceData(Concat(MakeHeader(0, 4, ImageFormat::R8_UNorm),
		            MakePayload(4)))),
		ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientAssets_TruncatedPayloadIsRejected)
{
	TestImageDecoder decoder;

	// A 4 x 3 RGBA image is 48 bytes. Every payload length short of that is
	// refused, including one byte short: a reader that takes what it can get
	// and returns a plausible image would pass the one-byte case and fail here.
	const std::vector<uint8_t> header = MakeHeader(4, 3, ImageFormat::R8G8B8A8_UNorm);
	const std::vector<uint8_t> full   = MakePayload(48);

	for (size_t size = 0; size < full.size(); ++size)
	{
		const std::vector<uint8_t> payload(
			full.begin(),
			full.begin() + static_cast<std::ptrdiff_t>(size));
		CHECK_EQ(DecodeError(decoder, ResourceData(Concat(header, payload))),
			ErrorCode::InvalidArgument);
	}

	// The complete payload decodes, which is what makes the loop above a
	// statement about truncation rather than about decoding being broken.
	const Result<ImageAsset> decoded = decoder.DecodeImage(ResourceData(Concat(header, full)));
	CHECK(decoded.IsOk());
	if (decoded.IsError())
	{
		return;
	}
	CHECK_EQ(decoded.GetValue().GetPixelByteCount(), static_cast<size_t>(48));
	CHECK(decoded.GetValue().GetPixels() == full);
}

MODERN_TEST(ClientAssets_TrailingBytesAreRejected)
{
	TestImageDecoder decoder;

	// A 2 x 2 RGB image is 12 bytes. One extra byte is enough to be refused:
	// the container declares its own length through its geometry, so anything
	// beyond that is not part of the image.
	const std::vector<uint8_t> header = MakeHeader(2, 2, ImageFormat::R8G8B8_UNorm);
	CHECK_EQ(DecodeError(decoder, ResourceData(Concat(header, MakePayload(13)))),
		ErrorCode::InvalidArgument);

	// A whole extra row, and a whole extra image's worth: refused too.
	CHECK_EQ(DecodeError(decoder, ResourceData(Concat(header, MakePayload(24)))),
		ErrorCode::InvalidArgument);

	// The exact payload is accepted, so the refusals above are about the extra
	// bytes rather than about the size check being unreachable.
	CHECK(decoder.DecodeImage(ResourceData(Concat(header, MakePayload(12)))).IsOk());
}

MODERN_TEST(ClientAssets_ExcessiveImageDimensionsAreRejected)
{
	TestImageDecoder decoder;

	// The header can describe 65535 x 65535 RGBA; decoding it is refused on the
	// geometry, before anything computes or allocates a 17 GB buffer.
	CHECK_EQ(DecodeError(decoder, ResourceData(Concat(MakeHeader(65535, 65535,
		            ImageFormat::R8G8B8A8_UNorm), MakePayload(16)))),
		ErrorCode::InvalidArgument);

	// One pixel past the dimension ceiling, in both orders, and one dimension
	// past while the other stays legal: all refused.
	const uint16_t tooWide = static_cast<uint16_t>(kMaxImageDimension + 1);
	CHECK_EQ(DecodeError(decoder, ResourceData(Concat(MakeHeader(tooWide, 1, ImageFormat::R8_UNorm),
		            MakePayload(1)))),
		ErrorCode::InvalidArgument);
	CHECK_EQ(DecodeError(decoder, ResourceData(Concat(MakeHeader(1, tooWide, ImageFormat::R8_UNorm),
		            MakePayload(1)))),
		ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientAssets_DecodedPixelsMatchTheContainerExactly)
{
	TestImageDecoder decoder;

	// 8 x 8 RGBA is 256 bytes of payload, so every byte value appears. The
	// expected pixels are built here from the same description the container
	// was, independently of the decoder.
	const std::vector<uint8_t> bytes    = MakeImage(8, 8, ImageFormat::R8G8B8A8_UNorm);
	const std::vector<uint8_t> expected = MakePayload(256);

	const Result<ImageAsset> decoded = decoder.DecodeImage(ResourceData(bytes));
	CHECK(decoded.IsOk());
	if (decoded.IsError())
	{
		return;
	}

	const ImageAsset& image = decoded.GetValue();
	CHECK_EQ(image.GetWidth(), 8u);
	CHECK_EQ(image.GetHeight(), 8u);
	CHECK_EQ(image.GetPixelCount(), static_cast<size_t>(64));
	CHECK_EQ(image.GetPixelByteCount(), static_cast<size_t>(256));
	CHECK(image.GetPixels() == expected);

	// The first and last pixels are distinguishable in the first and last
	// bytes, so a reader that skips or shifts the payload shows up here.
	CHECK_EQ(image.GetPixels().front(), static_cast<uint8_t>(0));
	CHECK_EQ(image.GetPixels()[1], static_cast<uint8_t>(1));
	CHECK_EQ(image.GetPixels().back(), static_cast<uint8_t>(255));

	// The input is not modified by decoding, and the image is the only thing
	// the decoder returns.
	CHECK(bytes == MakeImage(8, 8, ImageFormat::R8G8B8A8_UNorm));
}

MODERN_TEST(ClientAssets_DecoderPreservesPixelLayout)
{
	TestImageDecoder decoder;

	// Four layouts, four different byte counts for the same 2 x 2 geometry. A
	// decoder that normalised one layout into another - RGBA into BGRA, or
	// greyscale into RGB - would report the wrong layout and the wrong count.
	struct Layout
	{
		ImageFormat format;
		size_t      bytesPerPixel;
	};

	const Layout layouts[] = {
		{ ImageFormat::R8_UNorm, 1u },
		{ ImageFormat::R8G8B8_UNorm, 3u },
		{ ImageFormat::R8G8B8A8_UNorm, 4u },
		{ ImageFormat::B8G8R8A8_UNorm, 4u },
	};

	for (const Layout& layout : layouts)
	{
		const std::vector<uint8_t> bytes = MakeImage(2, 2, layout.format);
		CHECK_EQ(bytes.size(), TestImageDecoder::kHeaderSize + (4u * layout.bytesPerPixel));

		const Result<ImageAsset> decoded = decoder.DecodeImage(ResourceData(bytes));
		CHECK(decoded.IsOk());
		if (decoded.IsError())
		{
			return;
		}

		// The layout survives as declared, by name and by size.
		CHECK_EQ(std::string(ToString(decoded.GetValue().GetFormat())),
			std::string(ToString(layout.format)));
		CHECK_EQ(decoded.GetValue().GetPixelByteCount(), 4u * layout.bytesPerPixel);
		CHECK(decoded.GetValue().GetPixels() == MakePayload(4u * layout.bytesPerPixel));
	}

	// The two four-byte layouts produce the same number of bytes but stay
	// distinct values: BGRA is not silently relabelled as RGBA.
	const Result<ImageAsset> rgba =
		decoder.DecodeImage(ResourceData(MakeImage(1, 1, ImageFormat::R8G8B8A8_UNorm)));
	const Result<ImageAsset> bgra =
		decoder.DecodeImage(ResourceData(MakeImage(1, 1, ImageFormat::B8G8R8A8_UNorm)));
	CHECK(rgba.IsOk());
	CHECK(bgra.IsOk());
	if (rgba.IsError() || bgra.IsError())
	{
		return;
	}
	CHECK(rgba.GetValue().GetPixels() == bgra.GetValue().GetPixels());
	CHECK(rgba.GetValue().GetFormat() != bgra.GetValue().GetFormat());
}

// ---------------------------------------------------------------------------
// 24-25: separation from rendering, resources and legacy
// ---------------------------------------------------------------------------

MODERN_TEST(ClientAssets_AssetLayerDoesNotRequireRendererOrLegacy)
{
	// The guarantee is structural: this translation unit includes TestHarness.h,
	// the asset and resource headers and standard headers only, and links
	// ModernClientAssets, ModernClientResources and Modern. It links no renderer,
	// no DirectX, no D3DX, no Vulkan, no OpenGL, no MFC, no Windows header and
	// no legacy library, so a dependency in any of those directions would fail
	// to compile or fail to link rather than pass quietly.
	//
	// What can be asserted here is the shape that makes those absences hold:
	// the decoder is a pure interface, the asset is a self-contained value, and
	// decoding happens in a console process with no window and no device.
	static_assert(std::is_abstract<IImageDecoder>::value,
		"a decoder must be an interface, not a renderer");
	static_assert(std::is_base_of<IImageDecoder, TestImageDecoder>::value,
		"the concrete decoder must be a leaf behind the interface");
	static_assert(!std::is_default_constructible<ImageAsset>::value,
		"an asset must not be constructible without validation");

	// No member of ImageAsset is a backend object or a resource: the interface
	// is geometry, a layout name and bytes.
	static_assert(std::is_same<decltype(std::declval<const ImageAsset&>().GetPixels()),
		const std::vector<uint8_t>&>::value,
		"pixels must be CPU bytes");
	static_assert(std::is_same<decltype(std::declval<const ImageAsset&>().GetFormat()),
		ImageFormat>::value,
		"the layout must be one of this layer's own enumerators");

	TestImageDecoder decoder;
	const Result<ImageAsset> decoded =
		decoder.DecodeImage(ResourceData(MakeImage(2, 2, ImageFormat::R8G8B8A8_UNorm)));
	CHECK(decoded.IsOk());
	if (decoded.IsError())
	{
		return;
	}
	CHECK_EQ(decoded.GetValue().GetPixelByteCount(), static_cast<size_t>(16));
}

MODERN_TEST(ClientAssets_DecoderDoesNotDependOnResourceManager)
{
	// A decoder needs bytes and nothing else: no manager, no provider, no id.
	// The ResourceData here is built by hand and never loaded, which is the
	// strongest statement available that decoding is independent of where the
	// bytes came from. The integration cases below show the other direction:
	// the same decoder over bytes a provider and a manager produced.
	const std::vector<uint8_t> bytes = MakeImage(3, 1, ImageFormat::R8_UNorm);
	const ResourceData byHand(bytes);

	TestImageDecoder decoder;
	const Result<ImageAsset> first = decoder.DecodeImage(byHand);
	CHECK(first.IsOk());

	// Wrapping the same bytes differently - a copy of the vector, a view of
	// the same memory - decodes to the same image, because the decoder reads
	// ResourceData and not the object identity behind it.
	const std::vector<uint8_t> copy(bytes);
	CHECK_EQ(DecodeError(decoder, ResourceData(copy)), ErrorCode::None);
	if (first.IsError())
	{
		return;
	}
	CHECK_EQ(decoder.DecodeImage(ResourceData(copy)).GetValue().GetPixelByteCount(),
		first.GetValue().GetPixelByteCount());
	CHECK(decoder.DecodeImage(ResourceData(copy)).GetValue().GetPixels() == first.GetValue().GetPixels());
}

// ---------------------------------------------------------------------------
// 26-29: provider -> ResourceManager -> ResourceData -> decoder -> asset
// ---------------------------------------------------------------------------

MODERN_TEST(ClientAssets_MemoryProviderThroughManagerToImageAsset)
{
	TestImageDecoder decoder;
	const std::vector<uint8_t> bytes = MakeImage(4, 4, ImageFormat::R8G8B8A8_UNorm);

	MemoryResourceProvider provider;
	ResourceManager        manager;
	manager.SetProvider(&provider);
	CHECK(manager.Initialize().IsOk());

	const ResourceId id = MakeId("ui/test_image.mimg");
	CHECK(provider.RegisterResource(id, ResourceData(bytes)).IsOk());

	// provider -> ResourceManager -> ResourceData
	const Result<ResourceData> loaded = manager.Load(id);
	CHECK(loaded.IsOk());
	if (loaded.IsError())
	{
		return;
	}

	// The manager hands back exactly the bytes the provider stored. Nothing
	// above the provider interprets them, which is what leaves decoding to the
	// decoder rather than to the cache.
	CHECK(loaded.GetValue() == ResourceData(bytes));
	CHECK_EQ(loaded.GetValue().GetSize(), bytes.size());
	CHECK(manager.IsCached(id));

	// ResourceData -> decoder -> ImageAsset
	const Result<ImageAsset> decoded = decoder.DecodeImage(loaded.GetValue());
	CHECK(decoded.IsOk());
	if (decoded.IsError())
	{
		return;
	}

	CHECK_EQ(decoded.GetValue().GetWidth(), 4u);
	CHECK_EQ(decoded.GetValue().GetHeight(), 4u);
	CHECK_EQ(std::string(ToString(decoded.GetValue().GetFormat())), "R8G8B8A8_UNorm");
	CHECK_EQ(decoded.GetValue().GetPixelByteCount(), static_cast<size_t>(64));
	CHECK(decoded.GetValue().GetPixels() == MakePayload(64));

	// Decoding is not caching: the manager still holds bytes, and the asset is
	// a separate value produced from them. Decoding the cached bytes again
	// yields an equal image rather than the same object.
	const Result<ImageAsset> again = decoder.DecodeImage(manager.Load(id).GetValue());
	CHECK(again.IsOk());
	if (again.IsError())
	{
		return;
	}
	CHECK(again.GetValue().GetPixels() == decoded.GetValue().GetPixels());
	CHECK_EQ(again.GetValue().GetPixelByteCount(), decoded.GetValue().GetPixelByteCount());
	CHECK_EQ(again.GetValue().GetWidth(), decoded.GetValue().GetWidth());

	// What the manager caches is still the container bytes and not the image:
	// the layer above the provider stays untyped, and the decoder stays the
	// only thing that knows about pixels.
	CHECK(manager.IsCached(id));
	CHECK_EQ(manager.GetCachedCount(), static_cast<size_t>(1));
	CHECK(manager.Load(id).GetValue() == ResourceData(bytes));

	CHECK(manager.Shutdown().IsOk());
}

MODERN_TEST(ClientAssets_FileSystemProviderThroughManagerToImageAsset)
{
	TempRoot temp;
	CHECK(temp.IsUsable());

	TestImageDecoder decoder;
	const std::vector<uint8_t> bytes = MakeImage(3, 2, ImageFormat::B8G8R8A8_UNorm);
	CHECK(temp.Write("textures/menu/test_image.mimg", bytes));

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	ResourceManager manager;
	manager.SetProvider(&provider);
	CHECK(manager.Initialize().IsOk());

	const ResourceId id = MakeId("textures/menu/test_image.mimg");
	const Result<ResourceData> loaded = manager.Load(id);
	CHECK(loaded.IsOk());
	if (loaded.IsError())
	{
		return;
	}

	CHECK(manager.IsCached(id));
	CHECK_EQ(loaded.GetValue().GetSize(), bytes.size());
	CHECK(loaded.GetValue() == ResourceData(bytes));

	const Result<ImageAsset> decoded = decoder.DecodeImage(loaded.GetValue());
	CHECK(decoded.IsOk());
	if (decoded.IsError())
	{
		return;
	}

	CHECK_EQ(decoded.GetValue().GetWidth(), 3u);
	CHECK_EQ(decoded.GetValue().GetHeight(), 2u);
	CHECK_EQ(std::string(ToString(decoded.GetValue().GetFormat())), "B8G8R8A8_UNorm");
	CHECK_EQ(decoded.GetValue().GetPixelByteCount(), static_cast<size_t>(24));
	CHECK(decoded.GetValue().GetPixels() == MakePayload(24));

	// Bytes that travelled through a provider, a cache and a file decode to the
	// same image as bytes handed to the decoder directly: the decoder cannot
	// tell where they came from, which is the whole reason it takes
	// ResourceData rather than a path.
	const Result<ImageAsset> direct = decoder.DecodeImage(provider.Load(id).GetValue());
	CHECK(direct.IsOk());
	if (direct.IsError())
	{
		return;
	}
	CHECK(direct.GetValue().GetPixels() == decoded.GetValue().GetPixels());
	CHECK_EQ(std::string(ToString(direct.GetValue().GetFormat())),
		std::string(ToString(decoded.GetValue().GetFormat())));

	// A resource that is not there fails to load, and therefore never reaches a
	// decoder: the two failure modes stay distinguishable.
	CHECK_EQ(manager.Load(MakeId("textures/menu/absent.mimg")).GetError(), ErrorCode::NotFound);

	CHECK(manager.Shutdown().IsOk());
}

MODERN_TEST(ClientAssets_ManagerCacheDoesNotChangeDecoding)
{
	TestImageDecoder decoder;
	const std::vector<uint8_t> bytes = MakeImage(2, 3, ImageFormat::R8G8B8_UNorm);

	MemoryResourceProvider provider;
	ResourceManager        manager;
	manager.SetProvider(&provider);
	CHECK(manager.Initialize().IsOk());

	const ResourceId id = MakeId("ui/cached.mimg");
	CHECK(provider.RegisterResource(id, ResourceData(bytes)).IsOk());

	// Decoded from a cached load.
	const Result<ImageAsset> before = decoder.DecodeImage(manager.Load(id).GetValue());
	CHECK(before.IsOk());
	CHECK(manager.IsCached(id));

	// Clearing the byte cache changes which bytes were served, not what they
	// decode to. Decoding is a pure function of the payload, so an asset is
	// unaffected by the manager's cache policy - which is what allows that
	// policy to change later without touching this layer.
	CHECK(manager.ClearCache().IsOk());
	CHECK(!manager.IsCached(id));
	const Result<ImageAsset> after = decoder.DecodeImage(manager.Load(id).GetValue());
	CHECK(after.IsOk());

	if (before.IsError() || after.IsError())
	{
		return;
	}

	CHECK(after.GetValue().GetPixels() == before.GetValue().GetPixels());
	CHECK_EQ(after.GetValue().GetWidth(), before.GetValue().GetWidth());
	CHECK_EQ(after.GetValue().GetHeight(), before.GetValue().GetHeight());
	CHECK_EQ(after.GetValue().GetPixelByteCount(), before.GetValue().GetPixelByteCount());
	CHECK_EQ(std::string(ToString(after.GetValue().GetFormat())),
		std::string(ToString(before.GetValue().GetFormat())));
	CHECK(manager.IsCached(id));

	CHECK(manager.Shutdown().IsOk());
}

MODERN_TEST(ClientAssets_DecodeFailureLeavesManagerIntact)
{
	TestImageDecoder decoder;

	MemoryResourceProvider provider;
	ResourceManager        manager;
	manager.SetProvider(&provider);
	CHECK(manager.Initialize().IsOk());

	// A resource that exists and loads perfectly, and is still not an image.
	// The failure belongs to the decoder: it has no state to leave behind, so
	// nothing about the resource layer changes.
	const ResourceId garbage = MakeId("ui/not_an_image.bin");
	CHECK(provider.RegisterResource(garbage, ResourceData("NOT_AN_IMAGE_AT_ALL")).IsOk());

	const Result<ResourceData> loaded = manager.Load(garbage);
	CHECK(loaded.IsOk());
	if (loaded.IsError())
	{
		return;
	}

	CHECK(manager.IsCached(garbage));
	CHECK_EQ(loaded.GetValue().GetSize(), static_cast<size_t>(19));
	CHECK_EQ(decoder.DecodeImage(loaded.GetValue()).GetError(), ErrorCode::InvalidArgument);

	// The manager still serves it, unmodified: a decode failure is not a load
	// failure and does not invalidate the cache entry.
	CHECK(manager.IsCached(garbage));
	CHECK(manager.Load(garbage).IsOk());
	CHECK(manager.Load(garbage).GetValue() == ResourceData("NOT_AN_IMAGE_AT_ALL"));

	// A valid image alongside it still decodes: one undecodable resource does
	// not poison the decoder for the next one, because there is nothing to
	// poison.
	const ResourceId good = MakeId("ui/real_image.mimg");
	CHECK(provider.RegisterResource(good, ResourceData(MakeImage(2, 2, ImageFormat::R8_UNorm))).IsOk());

	const Result<ImageAsset> decoded = decoder.DecodeImage(manager.Load(good).GetValue());
	CHECK(decoded.IsOk());
	if (decoded.IsError())
	{
		return;
	}
	CHECK_EQ(decoded.GetValue().GetPixelByteCount(), static_cast<size_t>(4));
	CHECK_EQ(manager.GetCachedCount(), static_cast<size_t>(2));
	CHECK_EQ(manager.GetState(), ResourceManagerState::Ready);

	CHECK(manager.Shutdown().IsOk());
}

// ---------------------------------------------------------------------------
// 30-31: no RAN installation, no legacy format
// ---------------------------------------------------------------------------

MODERN_TEST(ClientAssets_NoRanInstallationRequired)
{
	// Everything the integration cases touch is created under the system
	// temporary directory and removed again. There is no asset directory in the
	// repository, no installation path in any header of this layer, and nothing
	// here reads an archive or a legacy file.
	TempRoot temp;
	CHECK(temp.IsUsable());

	std::error_code ec;
	const std::filesystem::path systemTemp = std::filesystem::temp_directory_path(ec);
	CHECK(!ec);
	CHECK(temp.Path().string().rfind(systemTemp.string(), 0) == 0);

	TestImageDecoder decoder;
	const std::vector<uint8_t> bytes = MakeImage(2, 2, ImageFormat::R8G8B8A8_UNorm);
	CHECK(temp.Write("generated/test_image.mimg", bytes));

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	const ResourceId id = MakeId("generated/test_image.mimg");
	CHECK(provider.HasResource(id));

	const Result<ResourceData> loaded = provider.Load(id);
	CHECK(loaded.IsOk());
	if (loaded.IsError())
	{
		return;
	}

	// The root is deleted when this case returns, which is also the proof that
	// nothing was cached outside the temporary directory in the meantime.
	CHECK_EQ(loaded.GetValue().GetSize(), bytes.size());

	const Result<ImageAsset> decoded = decoder.DecodeImage(loaded.GetValue());
	CHECK(decoded.IsOk());
	if (decoded.IsError())
	{
		return;
	}
	CHECK_EQ(decoded.GetValue().GetPixelByteCount(), static_cast<size_t>(16));
	CHECK(decoded.GetValue().GetPixels() == MakePayload(16));
}

MODERN_TEST(ClientAssets_NoLegacyFormatIsDecoded)
{
	// The one decoder in the build recognises the documented MIMG container and
	// nothing else. Inputs that look like other formats are refused rather than
	// guessed at, which is what keeps "CLIENT-007 does not decode RAN assets"
	// a testable statement instead of a promise.
	TestImageDecoder decoder;

	// A DDS header ('D','D','S',' '), the modern-era texture container most
	// likely to be mistaken for something this layer handles.
	const std::vector<uint8_t> dds = {
		'D', 'D', 'S', ' ', 0x7Cu, 0x00u, 0x00u, 0x00u, 0x0Fu, 0x00u, 0x00u, 0x00u
	};
	CHECK_EQ(DecodeError(decoder, ResourceData(dds)), ErrorCode::InvalidArgument);

	// A legacy-looking binary blob, and plain text: neither is this container.
	const uint8_t legacyBlob[] = { 0x00u, 0x00u, 0x02u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x12u, 0x34u };
	CHECK_EQ(DecodeError(decoder, ResourceData(legacyBlob, sizeof(legacyBlob))),
		ErrorCode::InvalidArgument);
	CHECK_EQ(DecodeError(decoder, ResourceData("textures/ui/login.dds")),
		ErrorCode::InvalidArgument);

	// Four bytes of the magic with nothing behind them is still not an image:
	// the container is longer than its signature.
	CHECK_EQ(DecodeError(decoder, ResourceData("MIMG")), ErrorCode::InvalidArgument);

	// A well-formed header whose payload is a legacy-shaped blob is refused on
	// size, so a valid header cannot be used as a wrapper around undecodable
	// data of the wrong length.
	CHECK_EQ(DecodeError(decoder, ResourceData(
		            Concat(MakeHeader(4, 4, ImageFormat::R8G8B8A8_UNorm), MakePayload(12)))),
		ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// Main Test Runner
// ---------------------------------------------------------------------------

int main()
{
	std::printf("Modern client typed asset / decoder boundary tests (CLIENT-007)\n\n");

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

