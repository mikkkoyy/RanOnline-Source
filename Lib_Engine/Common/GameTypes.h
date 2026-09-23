#pragma once

#include <cstdint>
#include <cstddef>

namespace GameTypes
{
    // Fixed-width integer aliases for the portable game core.
    // These are intentionally minimal: they provide exact-width types
    // without pulling in Windows, DirectX, or MFC headers.
    using GameInt8   = std::int8_t;
    using GameUInt8  = std::uint8_t;
    using GameInt16  = std::int16_t;
    using GameUInt16 = std::uint16_t;
    using GameInt32  = std::int32_t;
    using GameUInt32 = std::uint32_t;
    using GameInt64  = std::int64_t;
    using GameUInt64 = std::uint64_t;

    using GameSizeT  = std::size_t;
    using GamePtrDiff = std::ptrdiff_t;

    // Minimal data-only 3D vector for portable game-core boundaries.
    // This is NOT a math library. It carries position/direction data
    // between portable code and platform-specific rendering backends.
    struct GameVec3
    {
        float x;
        float y;
        float z;

        GameVec3() : x(0.0f), y(0.0f), z(0.0f) {}
        GameVec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    };
}

// Convenience aliases in the global namespace for minimal friction
// during incremental adoption. These are forward-declared only;
// platform-specific code should not depend on them.
using GameInt8   = GameTypes::GameInt8;
using GameUInt8  = GameTypes::GameUInt8;
using GameInt16  = GameTypes::GameInt16;
using GameUInt16 = GameTypes::GameUInt16;
using GameInt32  = GameTypes::GameInt32;
using GameUInt32 = GameTypes::GameUInt32;
using GameInt64  = GameTypes::GameInt64;
using GameUInt64 = GameTypes::GameUInt64;
using GameVec3   = GameTypes::GameVec3;