#pragma once

// ASURA-TEXTURE-001: the Targa (TGA) image decoder.
//
// The ASURA `textures/` tree ships 167 `.tga` files (UI flare/element art mostly
// under `textures/effect`). They carry no extension-independence guarantee --
// the bytes are what decides -- so this decoder reads the 18-byte Targa header
// by its contents and refuses anything outside the subset the inventory proves
// is present:
//
//   image type 2   truecolour, uncompressed   (BGR 24-bit / BGRA 32-bit)
//   image type 3   grayscale, uncompressed     (8-bit intensity)
//   image type 10  truecolour, RLE-compressed  (BGR 24-bit / BGRA 32-bit)
//
// Targa has no leading magic. Detection is therefore twofold and exposed as a
// static predicate: the 18-byte `TRUEVISION-XFILE.\0` footer at end-of-file when
// present (every shipped `.tga` carries it), plus a strict header sanity check
// (bounded dimensions, a supported image type and bit depth) so a file that
// predates the footer still resolves. The vertical origin bit (descriptor byte
// 17, bit 5) is honoured: Targa's native order is bottom-up, and the decoded
// `ImageAsset` is always returned top-row-first.
//
// Refused, each as InvalidArgument rather than a guessed read: colour-mapped
// types (1/9/11), 15/16-bit truecolour, empty dimensions, unspecified colour
// maps, a truncated header/palette/pixel body, and an RLE packet that runs past
// the payload. Every size is formed in 64-bit and bounded by the asset layer's
// own ceilings (`ComputeImageByteCount`) before anything is allocated. The
// result is always `R8G8B8A8_UNorm`; 24-bit and grayscale gain opaque alpha,
// 32-bit keeps its own. Like every decoder here it reaches no renderer, GPU,
// Windows or DirectX type.

#include "assets/ImageDecoder.h"

#include <cstddef>
#include <cstdint>

namespace Modern::Client
{

class TgaImageDecoder final : public IImageDecoder
{
public:
	Result<ImageAsset> DecodeImage(const ResourceData& data) override;

	// Whether these bytes claim to be a Targa this decoder can read: the
	// footer signature when present, else a strict header check. Kept beside the
	// decoder so the rule that decides decodability is the same one the loader
	// dispatches on.
	static bool LooksLikeTga(const uint8_t* data, size_t size) noexcept;
};

} // namespace Modern::Client
