#pragma once

// CLIENT-011: the first real image decoder.
//
// CLIENT-007 proved the byte -> asset boundary with MIMG, a container invented
// for the milestone. This decoder is the opposite: a real, shipped, widely
// documented format. RAN ships 12,732 `.dds` files in its `textures` tree, and
// the ASURA client hands those bytes straight to D3DX. Nothing about DDS needs
// Direct3D to *read* it -- a DDS file is a 128-byte header followed by
// block-compressed or uncompressed texels -- so this file brings the decode
// itself into the modern asset layer and keeps every Direct3D type out of it.
//
// Supported input (chosen from a survey of all 12,732 RAN `.dds` files, not
// from guesswork):
//
//   DXT1 / BC1              6,661 files   -> ImageFormat::R8G8B8A8_UNorm
//   DXT3 / BC2              2,731 files   -> ImageFormat::R8G8B8A8_UNorm
//   DXT5 / BC3              1,921 files   -> ImageFormat::R8G8B8A8_UNorm
//   uncompressed 32-bit RGBA   390 files -> ImageFormat::R8G8B8A8_UNorm
//   uncompressed 32-bit BGRA    53 files -> ImageFormat::B8G8R8A8_UNorm
//
// Refused input, each for a stated reason rather than a blanket failure:
//
//   DXT2, DXT4              premultiplied-alpha variants; RAN has 126 DXT2
//                            files, and un-premultiplying invents precision
//                            the format does not store
//   16-bit (RGB565/RGBA4444), 24-bit RGB, luminance, YUV, paletted
//                            806 files; supporting them means a conversion
//                            this milestone deliberately does not write
//   cubemaps (14 RAN files), volume textures (6)
//                            ImageAsset is one 2D image; a face or a slice is
//                            not the same resource
//   DX10 header extension (0 RAN files)
//                            not used by RAN at all; rejected rather than
//                            half-supported
//   mip chains              78% of RAN files carry 9-11 levels
//   anything malformed      wrong magic, bad header size, zero or
//                            overflowing dimensions, truncated payload,
//                            inconsistent linear size
//
// Mip policy, stated rather than implied: ImageAsset holds exactly one 2D
// image, so this decoder decodes **the top level only** and ignores the
// levels that follow it. It does *not* pretend they are preserved. Because
// most RAN textures are mipped, trailing data after the top level is
// expected and allowed; only a payload too short to hold the top level is
// refused.
//
// This is a decoder and nothing else. It does not know about `.mtf`: the RAN
// obfuscation that wraps a DDS file is a byte transform, it belongs to a
// separate milestone, and a future `.mtf` adapter will emit ordinary DDS
// bytes for this class to decode. No Direct3D, D3DX, MFC, Windows or renderer
// type appears here or in its headers.


#include "assets/ImageDecoder.h"

namespace Modern::Client
{

// The concrete decoder. Stateless, like every decoder in this layer: it holds
// no cache, no device and no configuration, and the same bytes always decode
// to the same pixels.
//
// Every rejection returns Status(ErrorCode::InvalidArgument) and never
// throws, matching the rest of the asset layer: a payload this decoder cannot
// read is not an image, and the only core code that says so without
// inventing new vocabulary is "the input was not acceptable".
class DdsImageDecoder final : public IImageDecoder
{
public:
	Result<ImageAsset> DecodeImage(const ResourceData& data) override;
};

} // namespace Modern::Client
