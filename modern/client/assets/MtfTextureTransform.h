#pragma once

// CLIENT-012: RAN MTF texture transform boundary.
//
// The RAN client ships textures as `.mtf` files: a 12-byte header
// followed by an obfuscated payload. This transform strips the container
// and decrypts the payload back into plain DDS bytes, which the existing
// DdsImageDecoder then consumes. The transform is deliberately small,
// stateless, and knows nothing about DDS parsing, rendering, or legacy
// systems.
//
// Header layout (12 bytes, little-endian):
//   bytes 0..3   int32 version        (must be 0x100)
//   bytes 4..7   int32 payloadSize    (must equal inputSize - 12)
//   bytes 8..11  int32 fileType       (0 = DDS, supported)
//
// Decryption (for each payload byte):
//   byte += 0x09
//   byte ^= 0x26
//
// This is a transform boundary, not a decoder: it takes ResourceData
// containing MTF bytes and returns ResourceData containing plain DDS
// bytes, which are then fed to DdsImageDecoder.

#include "resources/ResourceData.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Modern::Client
{

class MtfTextureTransform
{
public:
    // Transforms MTF bytes into plain DDS bytes.
    //
    // Validates the MTF container header, decrypts the payload, and
    // verifies the output starts with the DDS magic ('D','D','S',' ').
    // Returns ErrorCode::InvalidArgument for any malformed input.
    static Result<ResourceData> Transform(const ResourceData& mtfData);

private:
    static constexpr int32_t kHeaderSize = 12;
    static constexpr int32_t kVersion    = 0x100;
    static constexpr int32_t kXorData    = 0x26;
    static constexpr int32_t kDiffData   = 0x09;
    static constexpr int32_t kFileTypeDds = 0;

    static constexpr uint32_t kDdsMagic = 0x20534444u; // 'D','D','S',' '
};

} // namespace Modern::Client
