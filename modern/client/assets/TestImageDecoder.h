#pragma once

#include "assets/ImageDecoder.h"

#include <cstddef>
#include <cstdint>

namespace Modern::Client
{

// The one concrete decoder CLIENT-007 ships: the MIMG test representation.
//
// MIMG is **not** a RAN format and **not** a production format. It is a
// deliberately tiny container, defined here and nowhere else, invented so that
// the byte -> typed asset boundary can be implemented, tested and demonstrated
// without pretending a real decoder exists:
//
//   offset  size  field
//   0       4     magic 'M','I','M','G'
//   4       1     version, must be kVersion
//   5       1     format, an ImageFormat value; Unknown is refused
//   6       2     width,  little endian, 1..kMaxImageDimension
//   8       2     height, little endian, 1..kMaxImageDimension
//   10      n     pixels, exactly width * height * bytes per pixel
//
// Header and payload are one buffer with no compression, no palette, no mip
// chain, no alignment padding and no trailing data. Everything the container
// claims is checked, so a truncated or oversized payload is refused rather
// than decoded into a smaller or differently shaped image than the header
// described.
//
// This class exists so the boundary is exercised by something other than the
// test binary — `ModernEmulator` decodes MIMG bytes through it — and so the
// shape of a real decoder is already settled. A modern DDS reader, or a reader
// for whatever a future legacy importer writes, implements IImageDecoder
// alongside this one; nothing above the interface changes when it does, which
// is the entire point of the boundary.
class TestImageDecoder final : public IImageDecoder
{
public:
	// The fixed header, in bytes. Public because the format is public: the
	// tests build MIMG payloads by hand from these values, and the emulator
	// assembles its demo sample from the same description.
	static constexpr size_t kHeaderSize = 10;

	// The only version this decoder accepts. Any other value is refused
	// instead of best-effort parsed, because there is no older or newer
	// version of this format to be compatible with.
	static constexpr uint8_t kVersion = 1;

	Result<ImageAsset> DecodeImage(const ResourceData& data) override;
};

} // namespace Modern::Client
