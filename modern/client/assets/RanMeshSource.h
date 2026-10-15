#pragma once

// ASURA-CHARACTER-001B: RAN mesh source normalisation.
//
// RAN ships the same DirectX `.x` content in two containers:
//
//   `.mxf`  a 12-byte header plus an obfuscated payload
//           (`MxfMeshTransform`, `MxfMeshTransform.cpp`)
//   `.X`    the plain `.x` bytes, starting with the DirectX magic "xof "
//
// The Male Brawler base body is the second kind: `data/skin/s_m_hb.X` begins
// `xof 0303bin 0032` with no container at all. Feeding it to
// `MxfMeshTransform` fails on the version check, and skipping the transform
// unconditionally would break the `.mxf` regression asset
// (`data/skin/m_luffy_timeskip.mxf`).
//
// So the choice is made from the bytes, not from the filename: a payload that
// already starts with the DirectX X magic is passed through unchanged, and
// anything else goes through `MxfMeshTransform`. This is deliberately a
// *routing* decision, not a second container parser -- `MxfMeshTransform`
// remains the only thing that knows the `.mxf` layout, and `XMeshDecoder`
// remains the only thing that knows the `.x` layout. Neither is modified.
//
// The reason to route rather than sniff-and-report separately is that a
// caller that has to remember which transform applies to which extension will
// eventually apply the wrong one. The cost is one four-byte comparison on a
// path that already reads the whole file.

#include "resources/ResourceData.h"
#include "types/Result.h"

#include <cstdint>

namespace Modern::Client
{

enum class RanMeshContainer
{
    // Already plain DirectX `.x` bytes ("xof " magic at offset 0).
    PlainX,
    // An `.mxf` container; the payload must be transformed.
    Mxf,
};

const char* ToString(RanMeshContainer container) noexcept;

class RanMeshSource final
{
public:
    // The DirectX X file magic, as it appears at offset 0 of a plain `.x`.
    static constexpr size_t      kXMagicSize = 4;
    static constexpr std::uint32_t kXMagic    = 0x6F6678u; // 'x','o','f' in the low three bytes

    // Identifies which container `data` is in, from its first four bytes.
    //
    //   InvalidArgument - the input is shorter than the magic, so it is
    //                     neither a plain `.x` nor a readable `.mxf`
    static Result<RanMeshContainer> Classify(const ResourceData& data);

    // True when `data` already begins with the DirectX X magic. Total and
    // side-effect free: any input shorter than the magic is simply not one.
    static bool IsPlainX(const ResourceData& data) noexcept;

    // Returns plain `.x` bytes either way: unchanged for a plain `.x`, and
    // the `MxfMeshTransform` output for an `.mxf`.
    //
    //   InvalidArgument - input shorter than the X magic, or an `.mxf`
    //                     container that fails `MxfMeshTransform`'s own
    //                     validation
    static Result<ResourceData> ToPlainX(const ResourceData& data);
};

} // namespace Modern::Client