#pragma once

// ASURA-TEXTURE-001: the Windows BMP image decoder.
//
// The ASURA tree ships 31 `.bmp` files. BMP opens with a two-byte `BM` magic,
// so it is the one unsigned image format in the inventory that sniffs as
// reliably as PNG or DDS. This decoder reads the little-endian BITMAPINFOHEADER
// by hand and supports the uncompressed layouts the inventory proves are present
// and that a game-texture pipeline needs:
//
//   DIB header size >= 40 (BITMAPINFOHEADER and the V4/V5 supersets)
//   compression BI_RGB (0) only -- RLE / BITFIELDS / JPEG / PNG are refused
//   bit depth 8 (palette, up to 256 RGBQUAD entries), 24 (BGR), or 32 (BGRX)
//
// A 32-bit BI_RGB image stores an *unused* fourth byte; it is treated as opaque
// (alpha 255), because reading it as the alpha channel would render such an
// image fully transparent. Real alpha arrives as 32-bit BITFIELDS or PNG, not
// BI_RGB. Rows are bottom-up by default and flipped; a negative height marks a
// top-down image and is honoured. Output is always `R8G8B8A8_UNorm`.
//
// Every field is bounded: dimensions nonzero and within the asset ceiling
// (`ComputeImageByteCount`), a truncated header/palette/body refused, and a
// `pixelOffset`/row-stride that would read past the payload rejected. Like
// every decoder here it reaches no renderer, GPU, Windows or DirectX type.

#include "assets/ImageDecoder.h"

#include <cstddef>
#include <cstdint>

namespace Modern::Client
{

class BmpImageDecoder final : public IImageDecoder
{
public:
	Result<ImageAsset> DecodeImage(const ResourceData& data) override;

	// Whether these bytes claim to be a BMP: the `BM` magic plus a header large
	// enough to name its fields.
	static bool LooksLikeBmp(const uint8_t* data, size_t size) noexcept;
};

} // namespace Modern::Client
