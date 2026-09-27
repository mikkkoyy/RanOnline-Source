#include "assets/ImageAsset.h"

#include <utility>

namespace Modern::Client
{

Result<ImageAsset> ImageAsset::Create(
	uint32_t             width,
	uint32_t             height,
	ImageFormat          format,
	std::vector<uint8_t> pixels)
{
	// Geometry and layout validation lives in one place, ComputeImageByteCount,
	// so an image built here and a payload expected by a decoder can never
	// disagree about what "the right number of bytes" means.
	const Result<size_t> expected = ComputeImageByteCount(width, height, format);
	if (expected.IsError())
	{
		return expected.GetStatus();
	}

	// Exact, not "at least". See the contract in ImageAsset.h: guessing which
	// bytes are pixels is how a malformed container turns into a render bug
	// nobody can trace back to the file that caused it.
	if (pixels.size() != expected.GetValue())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	return ImageAsset(width, height, format, std::move(pixels));
}

} // namespace Modern::Client
