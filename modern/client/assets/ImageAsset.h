#pragma once

#include "assets/AssetTypes.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace Modern::Client
{

// A validated CPU-side image: its geometry, its pixel layout, and its pixels.
//
// An ImageAsset is the end of the line for the asset layer. It exists so that
// everything above it can work with "an image" rather than with "bytes that
// are probably an image", and so that a renderer adapter has one obvious thing
// to read.
//
// What it deliberately is not:
//
//   - **Not a GPU resource.** There is no `IDirect3DTexture9`, `ID3D11Texture2D`,
//     `VkImage`, `GLuint`, `HWND` or device pointer here, and no `<Windows.h>`
//     in this header. Uploading belongs to a future renderer adapter that reads
//     an ImageAsset; the asset never learns which API did so.
//   - **Not a file, a stream or a decoder.** It holds no path, no handle and no
//     reference to the bytes it was decoded from.
//   - **Not a mutable buffer.** Pixels leave through a const reference, and
//     `Create()` is the only way to obtain a value: to change pixels, build a
//     new ImageAsset, which re-runs every check.
//
// There is no default constructor, so there is no invalid ImageAsset state to
// inspect: a value that exists has already passed validation.
class ImageAsset
{
public:
	// Builds an image after checking every invariant it claims to satisfy:
	//
	//   InvalidArgument - unknown or unsupported pixel layout
	//                   - zero width or height
	//                   - a dimension above kMaxImageDimension
	//                   - a pixel count that cannot be represented by this
	//                     build or is above kMaxImageBytes
	//                   - a payload whose size is not exactly
	//                     width * height * bytes per pixel
	//
	// The size check is exact in both directions. A short payload is a
	// truncated decode; a long one means the bytes handed over are not the
	// pixels that were described, which is how a mismatched header becomes a
	// corrupted image instead of an error.
	static Result<ImageAsset> Create(
		uint32_t             width,
		uint32_t             height,
		ImageFormat          format,
		std::vector<uint8_t> pixels);

	uint32_t GetWidth() const noexcept { return m_width; }
	uint32_t GetHeight() const noexcept { return m_height; }
	ImageFormat GetFormat() const noexcept { return m_format; }

	// Pixel count and packed byte count. Neither is zero for a value that
	// exists, because a zero dimension is refused at construction.
	size_t GetPixelCount() const noexcept
	{
		return static_cast<size_t>(m_width) * static_cast<size_t>(m_height);
	}

	size_t GetPixelByteCount() const noexcept { return m_pixels.size(); }

	// The pixels: row-major, tightly packed, top row first, no row padding.
	// The layout has to be stated somewhere, and this is where it is stated.
	const std::vector<uint8_t>& GetPixels() const noexcept { return m_pixels; }

private:
	ImageAsset(
		uint32_t             width,
		uint32_t             height,
		ImageFormat          format,
		std::vector<uint8_t> pixels)
		: m_width(width)
		, m_height(height)
		, m_format(format)
		, m_pixels(std::move(pixels))
	{
	}

	uint32_t             m_width  = 0;
	uint32_t             m_height = 0;
	ImageFormat          m_format = ImageFormat::Unknown;
	std::vector<uint8_t> m_pixels;
};

} // namespace Modern::Client
