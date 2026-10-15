#pragma once

// ASURA-CHARACTER-001B: the RAN `.cps` SkinPiece container.
//
// A RAN character's geometry is not in its class configuration. The class
// file (`data/glogic/classXX.charset`) records which items the character
// starts wearing; `item.csv` turns each item into a "wearing file"; and the
// wearing file -- a `.cps` under `data/skinobject/` -- names the mesh, the
// skeleton and the mesh container that the piece actually draws.
//
// For the Male Brawler's starter outfit (from `class00.charset`):
//
//   [UPPER] [50,1] -> CUN0001_m.cps -> s_m_hb.X / b_m.X / m2_hb_body[Mesh]
//   [LOWER] [53,1] -> CDN0001_m.cps -> s_m_hb.X / b_m.X / m2_hb_leg[Mesh]
//   [FOOT]  [56,1] -> CFN0034_m.cps -> s_m_hb.X / b_m.X / m2_hb_foot[Mesh]
//   [RHAND] [23,1] -> BTN0011_m.cps -> s_m_gongyong.X / b_m.X / BTN0011[Mesh]
//
// Container header (every `.cps` in the ASURA tree starts this way, from
// `legacy/Lib_Engine/Common/SerialFile.cpp:50` `ReadFileType` and
// `legacy/Lib_Engine/Common/basestream.h:17`):
//
//   bytes 0..127   char[128] fileType  ("default" + NUL padding)
//   bytes 128..131 uint32   fileID
//   byte  132...            payload, little-endian
//
// Payload layout for version 0x0106
// (`legacy/Lib_Engine/Meshs/DxSkinPieceSaveLoad.cpp:658` `LoadPiece_0106`):
//
//   uint32   version                (0x0106)
//   uint32   reference
//   string   meshFilename           length-prefixed, includes its NUL
//   string   skeletonFilename       length-prefixed
//   uint32   pieceType              EMPIECECHAR (1=UPPER 2=LOWER 4=RHAND 6=FOOT)
//   uint32   weaponWhereBack
//   string   meshNodeName           length-prefixed
//   uint32   materialSlotCount
//   ...      materialSlotCount material records, then traces, effects, flag
//
// "String" means uint32 byte count (including the trailing NUL) followed by
// exactly that many bytes; the value is the NUL-terminated text.
//
// Scope, stated plainly:
//
//   Only version 0x0106 is decoded, because that is the version the four
//   confirmed class00 fixture pieces carry, and that is the only version
//   whose field boundaries this reader has verified against real bytes.
//   Anything else is refused as an unsupported version rather than
//   guessed at.
//
//   The material record array, the trace array, the effect array and the
//   trailing flag are NOT decoded. Their per-version layouts were not
//   confirmed against the real fixtures, and a reader that half-parses them
//   would be reporting invented values. Everything from the end of
//   `materialSlotCount` to end-of-file is therefore surfaced as
//   `GetOpaqueTail()`: preserved byte-for-byte, never reinterpreted.
//
//   This costs the caller nothing for this milestone. The `.X` mesh carries
//   its own real texture filenames in its `MeshMaterialList` template, which
//   `XMeshDecoder` already extracts, so the material records in the `.cps`
//   are not the source of the textures that get drawn. See
//   `RanMaterialSlot.h` for what the RAN-internal slot names in that opaque
//   region mean and why they are not filenames.
//
// Safety: every field is read through a bounds-checked cursor. A length that
// runs past the end of the buffer, a string longer than the buffer, a
// material count larger than the remaining bytes, or a file smaller than the
// fixed header are all refused with ErrorCode::InvalidArgument. Nothing is
// searched for by scanning for printable text.

#include "resources/ResourceData.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Modern::Client
{

// The RAN EMPIECECHAR values this reader has observed, named so a log line
// says something useful. Unknown values are preserved, not refused.
enum class SkinPieceType : uint32_t
{
    Unknown     = 0,
    Upper       = 1,
    Lower       = 2,
    Hand        = 4,
    Foot        = 6,
};

const char* ToString(SkinPieceType type) noexcept;

// A decoded RAN `.cps` wearing-file.
struct SkinPiece
{
    uint32_t    version           = 0;
    uint32_t    reference         = 0;
    uint32_t    fileId            = 0;
    std::string fileType;              // the 128-byte container tag, NUL-trimmed

    // Bare filenames, as written in the file. `meshFilename` resolves under
    // `data/skin/`, `skeletonFilename` under `data/skeleton/`.
    std::string meshFilename;
    std::string skeletonFilename;

    uint32_t    pieceTypeRaw      = 0;
    uint32_t    weaponWhereBack   = 0;

    // The mesh container inside `meshFilename` this piece draws, e.g.
    // "m2_hb_body[Mesh]". `s_m_hb.X` holds several (body/leg/foot/hand) and
    // each wearing file selects one.
    std::string meshNodeName;

    uint32_t    materialSlotCount = 0;

    // Everything from the end of the materialSlotCount field to end-of-file.
    // Preserved, never interpreted. See the scope note in the header.
    std::vector<uint8_t> opaqueTail;

    // Byte offset at which `opaqueTail` begins, for diagnostics.
    size_t opaqueTailOffset = 0;

    SkinPieceType GetPieceType() const noexcept
    {
        switch (pieceTypeRaw)
        {
        case static_cast<uint32_t>(SkinPieceType::Upper): return SkinPieceType::Upper;
        case static_cast<uint32_t>(SkinPieceType::Lower): return SkinPieceType::Lower;
        case static_cast<uint32_t>(SkinPieceType::Hand):  return SkinPieceType::Hand;
        case static_cast<uint32_t>(SkinPieceType::Foot):  return SkinPieceType::Foot;
        default: return SkinPieceType::Unknown;
        }
    }

    size_t GetOpaqueTailSize() const noexcept { return opaqueTail.size(); }
};

class SkinPieceContainer final
{
public:
    // The only container version this reader decodes.
    static constexpr uint32_t kSupportedVersion = 0x0106u;

    // The fixed container header that precedes every `.cps` payload.
    static constexpr size_t kFileTypeSize = 128;  // char[128]
    static constexpr size_t kFileIdSize   = 4;    // uint32
    static constexpr size_t kHeaderSize   = kFileTypeSize + kFileIdSize;

    static bool IsSupportedVersion(uint32_t version) noexcept
    {
        return version == kSupportedVersion;
    }

    // The smallest input this reader will look at: the container header plus
    // the version word.
    static constexpr size_t kMinimumSize = kHeaderSize + 4;

    //   InvalidArgument - input shorter than kMinimumSize, an unsupported
    //                     container version, a string length that runs past
    //                     the end of the buffer, a materialSlotCount larger
    //                     than the bytes that remain, or an empty
    //                     mesh/skeleton/node filename
    static Result<SkinPiece> Decode(const ResourceData& data);
};

} // namespace Modern::Client