// ASURA-CHARACTER-001B: RAN material-name classification.
//
// See RanMaterialSlot.h for what each kind means and why it matters.

#include "assets/RanMaterialSlot.h"

#include <algorithm>
#include <array>
#include <cctype>

namespace Modern::Client
{

namespace
{
    char LowerAscii(char c)
    {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    bool EqualsIgnoreCase(std::string_view lhs, std::string_view rhs)
    {
        if (lhs.size() != rhs.size()) { return false; }
        for (size_t i = 0; i < lhs.size(); ++i)
        {
            if (LowerAscii(lhs[i]) != LowerAscii(rhs[i])) { return false; }
        }
        return true;
    }

    // "IMAGE01" -> "IMAGE" plus "01". True when the name is a RAN slot:
    // a known uppercase prefix followed by at least one digit and nothing
    // else. The prefix must be uppercase: RAN writes them uppercase, and
    // accepting "image01" would let an ordinary lowercase texture filename
    // be misreported as a slot.
    bool IsSlotWithPrefix(std::string_view name, std::string_view prefix)
    {
        if (name.size() <= prefix.size()) { return false; }
        if (name.compare(0, prefix.size(), prefix) != 0) { return false; }
        const std::string_view digits = name.substr(prefix.size());
        for (const char c : digits)
        {
            if (c < '0' || c > '9') { return false; }
        }
        return true;
    }

    // The RAN slot prefixes, as observed in the confirmed class00 pieces.
    constexpr std::array<std::string_view, 4> kSlotPrefixes = {
        "IMAGE", "BLOOD", "STRIKE", "ARROW"
    };

    // The extensions the modern image decoders claim. Kept in sync with
    // TextureLoader::Decode's content dispatch by construction: the list is
    // written out here and the header says so, and ClientMaterialSlotTests
    // checks a representative name for each.
    constexpr std::array<std::string_view, 7> kImageExtensions = {
        ".png", ".dds", ".mtf", ".tga", ".bmp", ".jpg", ".jpeg"
    };
} // namespace

const char* ToString(RanMaterialNameKind kind) noexcept
{
    switch (kind)
    {
    case RanMaterialNameKind::None:            return "none";
    case RanMaterialNameKind::TextureFileName: return "texture-filename";
    case RanMaterialNameKind::RanMaterialSlot: return "ran-material-slot";
    case RanMaterialNameKind::Unrecognized:    return "unrecognized";
    }
    return "unrecognized";
}

bool IsSupportedImageExtension(std::string_view name)
{
    const size_t dot = name.rfind('.');
    if (dot == std::string_view::npos || dot + 1 == name.size()) { return false; }
    const std::string_view extension = name.substr(dot);
    for (const std::string_view candidate : kImageExtensions)
    {
        if (EqualsIgnoreCase(extension, candidate)) { return true; }
    }
    return false;
}

bool IsRanMaterialSlotName(std::string_view name)
{
    for (const std::string_view prefix : kSlotPrefixes)
    {
        if (IsSlotWithPrefix(name, prefix)) { return true; }
    }
    return false;
}

RanMaterialNameKind ClassifyRanMaterialName(std::string_view name)
{
    if (name.empty()) { return RanMaterialNameKind::None; }
    if (IsRanMaterialSlotName(name)) { return RanMaterialNameKind::RanMaterialSlot; }

    // A path separator means this is not a bare name, and the bare-name
    // texture loader refuses one on purpose.
    if (name.find('/') != std::string_view::npos ||
        name.find('\\') != std::string_view::npos)
    {
        return RanMaterialNameKind::Unrecognized;
    }

    return IsSupportedImageExtension(name) ? RanMaterialNameKind::TextureFileName
                                          : RanMaterialNameKind::Unrecognized;
}

std::string DescribeRanMaterialResolution(std::string_view pieceSource,
                                         std::string_view materialName,
                                         std::string_view attemptedResolution)
{
    std::string text;
    text.reserve(160);

    if (!pieceSource.empty())
    {
        text.append("piece '").append(pieceSource).append("': ");
    }
    text.append("material '").append(materialName).append("' -> ");

    switch (ClassifyRanMaterialName(materialName))
    {
    case RanMaterialNameKind::None:
        text.append("declares no texture");
        return text;

    case RanMaterialNameKind::RanMaterialSlot:
        text.append("RAN-internal material slot (no shipped texture carries this name; "
                    "the RAN engine resolves it through its piece-material tables, "
                    "which the modern client does not read yet) UNRESOLVED");
        return text;

    case RanMaterialNameKind::TextureFileName:
        if (attemptedResolution.empty())
        {
            text.append("texture filename, not present in the texture tree UNRESOLVED");
        }
        else
        {
            text.append("texture filename resolved to '").append(attemptedResolution).append("'");
        }
        return text;

    case RanMaterialNameKind::Unrecognized:
        break;
    }

    text.append("unrecognized name (not a bare image filename, not a RAN slot) UNRESOLVED");
    return text;
}

} // namespace Modern::Client