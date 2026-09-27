#pragma once

// CLIENT-013: RAN MXF mesh transform boundary.
//
// The RAN client ships mesh/skin files as `.mxf`: a 12-byte header
// followed by an obfuscated payload. This transform strips the
// container and decrypts the payload back into plain DirectX `.x`
// bytes, which a future X decoder can consume. The transform is
// deliberately small, stateless, and knows nothing about X parsing,
// rendering, or legacy systems.
//
// Header layout (12 bytes, little-endian):
//   bytes 0..3   int32 version        (must be 0x100)
//   bytes 4..7   int32 payloadSize    (must equal inputSize - 12)
//   bytes 8..11  int32 fileType       (0 = skin, supported)
//
// Decryption (for each payload byte):
//   byte += 0xEA
//   byte ^= 0xEB
//
// This is a transform boundary, not a decoder: it takes ResourceData
// containing MXF bytes and returns ResourceData containing plain X
// bytes, which are then fed to a future X decoder.

#include "resources/ResourceData.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Modern::Client
{

class MxfMeshTransform
{
public:
    // Transforms MXF bytes into plain X bytes.
    //
    // Validates the MXF container header, decrypts the payload, and
    // verifies the output starts with the DirectX X magic ("xof ").
    // Returns ErrorCode::InvalidArgument for any malformed input.
    static Result<ResourceData> Transform(const ResourceData& mxfData);

private:
    static constexpr int32_t kHeaderSize = 12;
    static constexpr int32_t kVersion    = 0x100;
    static constexpr int32_t kXorData    = 0xEB;
    static constexpr int32_t kDiffData   = 0xEA;
    static constexpr int32_t kFileTypeSkin = 0;

    static constexpr uint32_t kXMagic = 0x6F6678u; // 'x','o','f'
};

} // namespace Modern::Client
