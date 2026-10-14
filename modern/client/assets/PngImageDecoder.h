#pragma once

// CLIENT-RENDER-002: the PNG image decoder.
//
// The ASURA character's actual textures are PNG, not the MTF-wrapped DDS the
// rest of the pipeline speaks: `m_luffy_timeskip.mxf` references
// `blackcatyb_luffy_timeskip.png` and `blackcatyb_luffy_timeskip_eye.png`, both
// of which exist on disk under `textures/item/` and both of which are genuine
// PNG files. `DdsImageDecoder` cannot read them and `MtfTextureTransform` only
// unwraps MTF, so rendering the character's real texture needs a PNG path.
//
// This decoder is the smallest sound one that decodes both of those files,
// built on the same `IImageDecoder` contract as `DdsImageDecoder`: `ResourceData`
// in, one validated `ImageAsset` out, `InvalidArgument` on anything it cannot
// read, nothing thrown. It sits in the asset layer and reaches nothing above it
// -- no renderer, no GPU object, no DirectX/Vulkan/Windows type. Decompression
// is the shared `Inflate.h` (RFC 1950 wrapper over the RFC 1951 inflater the
// `bzip`/MSZip path already uses), so the asset layer gains no zlib dependency.
//
// The supported subset is exactly what the real files need, and it is stated
// here rather than implied:
//
//   - signature            the eight-byte PNG magic
//   - IHDR                 colour type 2 (truecolour RGB) and 6 (truecolour +
//                          alpha RGBA), bit depth 8, no interlace
//   - IDAT                 one or more chunks, concatenated, inflated as one
//                          zlib stream; the Adler-32 is verified
//   - filtering            all five scanline filters (None, Sub, Up, Average,
//                          Paeth), reconstructed per the specification
//   - ancillary chunks      every other chunk type is skipped by its length, so
//                          pHYs / gAMA / cHRM / sRGB / tEXt and the like do not
//                          stop the decode
//
// Everything else is refused with a stated reason rather than guessed at:
// interlaced (Adam7) images, palette (colour type 3), greyscale (0), bit depths
// other than 8, a missing or duplicated IHDR, a truncated chunk, a CRC mismatch
// on IHDR, and an inflated scanline count that disagrees with the header. The
// result is always an `R8G8B8A8_UNorm` image: colour-type 6 keeps its alpha,
// colour-type 2 is given opaque alpha, so transparent pixels survive and the
// uploader already carries an explicit RGBA conversion.

#include "assets/ImageDecoder.h"

namespace Modern::Client
{

class PngImageDecoder final : public IImageDecoder
{
public:
	Result<ImageAsset> DecodeImage(const ResourceData& data) override;
};

} // namespace Modern::Client
