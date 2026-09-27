#pragma once

// CLIENT-007: the vocabulary shared by typed CPU-side assets.
//
// The asset layer sits above the resource layer: a provider delivers bytes,
// ResourceManager caches them, and a decoder turns those bytes into one of the
// validated values defined here. Nothing in this layer knows about rendering,
// GPU upload, DirectX, Vulkan, OpenGL, Windows, MFC, or the legacy RAN formats.

#include "types/Result.h"

#include <cstddef>
#include <cstdint>

namespace Modern::Client
{

// Pixel layout of an image asset, one pixel at a time.
//
// Unknown is never a decodable layout: it is the name a malformed or
// unsupported header is refused under, and BytesPerPixel() answers 0 for it.
// The names describe channel order and component width, not a source format,
// so a format is a property of the bytes rather than of the file they came in.
enum class ImageFormat : uint8_t
{
	Unknown = 0,
	R8_UNorm,          // 1 byte per pixel, single intensity channel
	R8G8B8_UNorm,      // 3 bytes per pixel, red first
	R8G8B8A8_UNorm,    // 4 bytes per pixel, red first
	B8G8R8A8_UNorm,    // 4 bytes per pixel, blue first
};

// Stable name of a pixel layout, for logs and test output. An enumerator that
// is not one of the five above is reported as invalid rather than guessed at.
const char* ToString(ImageFormat format) noexcept;

// Bytes per pixel, or 0 when the layout is Unknown or not a known enumerator.
// Zero is a refusal, not a fallback: nothing in this layer infers a layout it
// was not given, and every caller of this function has to handle the zero.
uint32_t BytesPerPixel(ImageFormat format) noexcept;

// Ceilings applied to every image asset.
//
// These are policy, not a format limit: they keep a single decoded image
// bounded, and they are what makes the refusal of an oversized or
// overflowing image identical on a 32-bit build (this repository builds the
// client 32-bit) and on a 64-bit one. A limit that moved with the host would
// make the same bytes decode on one machine and fail on another.
constexpr uint32_t kMaxImageDimension = 16384;
constexpr size_t   kMaxImageBytes     = 256 * 1024 * 1024;

// Total pixel byte count for a width/height/layout triple.
//
// The product is formed in 64-bit arithmetic and refused rather than wrapped,
// so a header claiming 65535 x 65535 RGBA cannot make a small allocation look
// correct.
//
//   InvalidArgument - the layout is Unknown or not a known enumerator
//                   - width or height is 0
//                   - width or height exceeds kMaxImageDimension
//                   - the product exceeds kMaxImageBytes or cannot be held
//                     by size_t on this build
Result<size_t> ComputeImageByteCount(uint32_t width, uint32_t height, ImageFormat format) noexcept;

} // namespace Modern::Client
