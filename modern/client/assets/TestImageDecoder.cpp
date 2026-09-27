#include "assets/TestImageDecoder.h"

#include <cstddef>
#include <vector>

namespace Modern::Client
{
namespace
{
	// Little endian, read byte by byte. The container defines the byte order,
	// so the reader must not depend on the host's, and assembling from bytes
	// also means the header needs no alignment.
	uint32_t ReadU16LE(const uint8_t* bytes) noexcept
	{
		return static_cast<uint32_t>(bytes[0]) |
		       (static_cast<uint32_t>(bytes[1]) << 8);
	}

	bool HasMagic(const uint8_t* bytes) noexcept
	{
		return bytes[0] == 'M' && bytes[1] == 'I' && bytes[2] == 'M' && bytes[3] == 'G';
	}
}

Result<ImageAsset> TestImageDecoder::DecodeImage(const ResourceData& data)
{
	// Empty data falls out of this too: there is no header to read, so the
	// input is refused rather than treated as a zero-sized image.
	if (data.GetSize() < kHeaderSize)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	const uint8_t* bytes = data.GetData();

	if (!HasMagic(bytes))
	{
		return Status(ErrorCode::InvalidArgument);
	}

	if (bytes[4] != kVersion)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// A format byte is data, not an enumerator, so it may hold any value at
	// all. ComputeImageByteCount() is what refuses the ones with no layout,
	// which keeps "unknown format" and "zero dimension" one check apart.
	const ImageFormat format = static_cast<ImageFormat>(bytes[5]);
	const uint32_t    width  = ReadU16LE(bytes + 6);
	const uint32_t    height = ReadU16LE(bytes + 8);

	const Result<size_t> expected = ComputeImageByteCount(width, height, format);
	if (expected.IsError())
	{
		return expected.GetStatus();
	}

	// Exact payload size. The header declares the geometry, the geometry
	// declares the byte count, and anything else in the buffer means the input
	// is not the image it claims to be.
	const size_t payloadSize = data.GetSize() - kHeaderSize;
	if (payloadSize != expected.GetValue())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	const uint8_t* pixels = bytes + kHeaderSize;
	return ImageAsset::Create(
		width,
		height,
		format,
		std::vector<uint8_t>(pixels, pixels + payloadSize));
}

} // namespace Modern::Client
