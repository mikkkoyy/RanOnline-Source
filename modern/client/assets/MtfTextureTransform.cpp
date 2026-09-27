#include "assets/MtfTextureTransform.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Modern::Client
{

Result<ResourceData> MtfTextureTransform::Transform(const ResourceData& mtfData)
{
    if (mtfData.GetSize() < kHeaderSize)
    {
        return Status(ErrorCode::InvalidArgument);
    }
    const uint8_t* bytes = mtfData.GetData();
    const int32_t version = static_cast<int32_t>(bytes[0]) |
                            (static_cast<int32_t>(bytes[1]) << 8) |
                            (static_cast<int32_t>(bytes[2]) << 16) |
                            (static_cast<int32_t>(bytes[3]) << 24);
    const int32_t payloadSize = static_cast<int32_t>(bytes[4]) |
                                (static_cast<int32_t>(bytes[5]) << 8) |
                                (static_cast<int32_t>(bytes[6]) << 16) |
                                (static_cast<int32_t>(bytes[7]) << 24);
    const int32_t fileType = static_cast<int32_t>(bytes[8]) |
                             (static_cast<int32_t>(bytes[9]) << 8) |
                             (static_cast<int32_t>(bytes[10]) << 16) |
                             (static_cast<int32_t>(bytes[11]) << 24);
    if (version != kVersion) { return Status(ErrorCode::InvalidArgument); }
    if (payloadSize < 0) { return Status(ErrorCode::InvalidArgument); }
    const size_t expectedTotal = static_cast<size_t>(kHeaderSize) + static_cast<size_t>(payloadSize);
    if (expectedTotal != mtfData.GetSize())
    {
        return Status(ErrorCode::InvalidArgument);
    }
    if (fileType != kFileTypeDds) { return Status(ErrorCode::InvalidArgument); }
    if (payloadSize == 0) { return Status(ErrorCode::InvalidArgument); }
    std::vector<uint8_t> ddsPayload(static_cast<size_t>(payloadSize));
    for (int32_t i = 0; i < payloadSize; ++i)
    {
        const uint8_t raw = bytes[kHeaderSize + static_cast<size_t>(i)];
        ddsPayload[static_cast<size_t>(i)] = static_cast<uint8_t>((raw + kDiffData) ^ kXorData);
    }
    if (ddsPayload.size() < 4 ||
        ddsPayload[0] != 'D' ||
        ddsPayload[1] != 'D' ||
        ddsPayload[2] != 'S' ||
        ddsPayload[3] != ' ')
    {
        return Status(ErrorCode::InvalidArgument);
    }
    return ResourceData(std::move(ddsPayload));
}

} // namespace Modern::Client
