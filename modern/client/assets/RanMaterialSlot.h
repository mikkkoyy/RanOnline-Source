#pragma once

// ASURA-CHARACTER-001B: RAN material-name classification.
//
// The RAN body meshes in `data/skin/` carry two different kinds of string in
// the material-texture position, and conflating them produces a confusing
// "texture not found" for something that was never a filename.
//
// 1. **Real texture filenames.** `s_m_hb.X` (the Male Brawler base body, read
//    through `data/skinobject/CUN0001_m.cps`) declares, in its
//    `MeshMaterialList` templates:
//
//        Material -> TextureFilename -> m1_hb_body.dds
//        Material -> TextureFilename -> m2_bs_body.DDS
//        Material -> TextureFilename -> m1_khb_foot.dds
//        Material -> TextureFilename -> m1_hb_leg.dds
//        Material -> TextureFilename -> m1_hb_hand.dds
//
//    All five resolve by bare filename over the ASURA `textures` tree
//    (`textures/char/`), through the existing `TextureLoader`. These are the
//    textures that actually get drawn.
//
// 2. **RAN-internal material slots.** The RAN wearing files also carry names
//    like `IMAGE01`, `IMAGE02`, `IMAGE11`, `IMAGE12`, `IMAGE21`, `IMAGE22`,
//    `BLOOD01`, `STRIKE01` and `ARROW01`. These were read out of the four
//    confirmed `class00` pieces (`CUN0001_m.cps`, `CDN0001_m.cps`,
//    `CFN0034_m.cps`, `BTN0011_m.cps`).
//
//    **No file in the shipped ASURA texture tree is named after any of
//    them** -- a scan of all 13,932 files under `textures/` for a basename of
//    `IMAGE*` returns zero matches. They are RAN's internal slot identifiers
//    for a material *role* ("body diffuse", "blood overlay", "strike
//    effect"), resolved by the legacy engine through its own piece-material
//    tables (`legacy/Lib_Engine/DxCommon/TextureManager.cpp` `EMTT_*` and the
//    `.pie`/`.pie15` tables under `data/piece/`), not by filename lookup.
//
//    So a slot name reaching the texture loader is never a missing file that
//    a different search would have found; it is a name the client is not yet
//    able to resolve. This classifier exists so that outcome is *named*
//    rather than reported as a generic lookup miss.
//
// Deliberately narrow: this classifies a name. It does not invent a texture
// for a slot, it does not fall back to a "similar" file, and it does not
// suppress the diagnostic. A caller that gets `RanMaterialSlot` back must
// still treat the material as unresolved.

#include <cstddef>
#include <string>
#include <string_view>

namespace Modern::Client
{

enum class RanMaterialNameKind
{
    // Empty: the material declares no texture at all. A legal shape.
    None,
    // A bare filename with a known RAN image extension. Resolvable by name.
    TextureFileName,
    // A RAN-internal material slot name (IMAGE##, BLOOD##, STRIKE##, ARROW##).
    // Never a filename in the shipped tree.
    RanMaterialSlot,
    // Anything else: a bare filename with an unrecognised extension, or a
    // name carrying a path separator. Not resolvable by the bare-name loader.
    Unrecognized,
};

const char* ToString(RanMaterialNameKind kind) noexcept;

// Classifies one material-texture name. Pure and total: it never fails and
// never touches the filesystem.
RanMaterialNameKind ClassifyRanMaterialName(std::string_view name);

// True when the name is one of RAN's internal material slots.
bool IsRanMaterialSlotName(std::string_view name);

// True when the name is a bare filename carrying an extension the modern
// image decoders claim (png, dds, mtf, tga, bmp, jpg, jpeg), case-insensitively.
// This is the same extension set `TextureLoader::Decode` dispatches on, listed
// here so classification and dispatch cannot drift apart.
bool IsSupportedImageExtension(std::string_view name);

// A one-line, log-ready explanation of why a name did or did not resolve.
// `resolvedTo` is the path `TextureLoader::Find` returned (empty when it did
// not). Kept here so the client log, the tests and any future tool all say the
// same thing.
std::string DescribeRanMaterialResolution(std::string_view pieceSource,
                                         std::string_view materialName,
                                         std::string_view attemptedResolution);

} // namespace Modern::Client