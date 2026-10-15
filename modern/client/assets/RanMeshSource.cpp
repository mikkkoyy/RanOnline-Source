// ASURA-CHARACTER-001B: RAN mesh source normalisation.
//
// See RanMeshSource.h. Two details worth stating next to the code:
//
//   - `IsPlainX` compares the three magic bytes 'x','o','f' explicitly rather
//     than masking the fourth byte. A plain RAN `.x` always has a space there,
//     but requiring it would reject a hypothetical four-character magic for no
//     benefit; matching the three bytes that identify the format is the whole
//     test, and anything that does not match goes to the `.mxf` path where
//     `MxfMeshTransform` applies its own, stricter, validation.
//
//   - `ToPlainX` returns the input *unchanged* for a plain `.x`. It does not
//     copy-and-rewrite, and it does not re-wrap anything.

#include "assets/RanMeshSource.h"

#include "assets/MxfMeshTransform.h"

namespace Modern::Client
{

const char* ToString(RanMeshContainer container) noexcept
{
    switch (container)
    {
    case RanMeshContainer::PlainX: return "plain .X";
    case RanMeshContainer::Mxf:    return ".mxf container";
    }
    return "unknown";
}

bool RanMeshSource::IsPlainX(const ResourceData& data) noexcept
{
    if (data.GetSize() < kXMagicSize) { return false; }
    const uint8_t* bytes = data.GetData();
    return bytes[0] == 'x' && bytes[1] == 'o' && bytes[2] == 'f';
}

Result<RanMeshContainer> RanMeshSource::Classify(const ResourceData& data)
{
    if (data.GetSize() < kXMagicSize) { return Status(ErrorCode::InvalidArgument); }
    return Result<RanMeshContainer>(IsPlainX(data) ? RanMeshContainer::PlainX
                                                  : RanMeshContainer::Mxf);
}

Result<ResourceData> RanMeshSource::ToPlainX(const ResourceData& data)
{
    if (data.GetSize() < kXMagicSize) { return Status(ErrorCode::InvalidArgument); }

    if (IsPlainX(data))
    {
        // Already plain DirectX `.x` bytes: hand them back untouched.
        return Result<ResourceData>(data);
    }

    // Not a plain `.x`, so it must be the container. MxfMeshTransform owns
    // that validation and that transform; this function only routes.
    return MxfMeshTransform::Transform(data);
}

} // namespace Modern::Client