#include "assets/AssetTypes.h"

#include <limits>

namespace Modern::Client
{

const char* ToString(ImageFormat format) noexcept
{
	switch (format)
	{
	case ImageFormat::Unknown:
		return "Unknown";
	case ImageFormat::R8_UNorm:
		return "R8_UNorm";
	case ImageFormat::R8G8B8_UNorm:
		return "R8G8B8_UNorm";
	case ImageFormat::R8G8B8A8_UNorm:
		return "R8G8B8A8_UNorm";
	case ImageFormat::B8G8R8A8_UNorm:
		return "B8G8R8A8_UNorm";
	}

	// Only reached for a value that is not an enumerator, which is exactly
	// what a decoder reading a format byte out of a hostile header produces.
	return "Invalid";
}

uint32_t BytesPerPixel(ImageFormat format) noexcept
{
	switch (format)
	{
	case ImageFormat::R8_UNorm:
		return 1;
	case ImageFormat::R8G8B8_UNorm:
		return 3;
	case ImageFormat::R8G8B8A8_UNorm:
		return 4;
	case ImageFormat::B8G8R8A8_UNorm:
		return 4;
	case ImageFormat::Unknown:
		break;
	}

	return 0;
}

Result<size_t> ComputeImageByteCount(uint32_t width, uint32_t height, ImageFormat format) noexcept
{
	const uint32_t bytesPerPixel = BytesPerPixel(format);
	if (bytesPerPixel == 0)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	if (width == 0 || height == 0)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	if (width > kMaxImageDimension || height > kMaxImageDimension)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// uint64_t so the intermediate cannot wrap for any width/height a uint32_t
	// caller can pass; the answer is checked against two limits before it is
	// narrowed, and the cast below is only reached when both checks passed.
	const uint64_t total = static_cast<uint64_t>(width) * static_cast<uint64_t>(height) *
	                       static_cast<uint64_t>(bytesPerPixel);

	if (total > static_cast<uint64_t>(kMaxImageBytes))
	{
		return Status(ErrorCode::InvalidArgument);
	}

	if (total > static_cast<uint64_t>(std::numeric_limits<size_t>::max()))
	{
		return Status(ErrorCode::InvalidArgument);
	}

	return static_cast<size_t>(total);
}

} // namespace Modern::Client
