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

    // Minimal data-only 2D vector for portable game-core boundaries.
    struct GameVec2
    {
        float x;
        float y;

        GameVec2() : x(0.0f), y(0.0f) {}
        GameVec2(float _x, float _y) : x(_x), y(_y) {}
    };

    // Minimal data-only 4D vector for portable game-core boundaries.
    struct GameVec4
    {
        float x;
        float y;
        float z;
        float w;

        GameVec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
        GameVec4(float _x, float _y, float _z, float _w)
            : x(_x), y(_y), z(_z), w(_w) {}
    };

    // Minimal data-only quaternion for portable game-core boundaries.
    // Used by animation/skeletal systems that must eventually run on
    // Windows, Android, and iOS without DirectX.
    struct GameQuaternion
    {
        float x;
        float y;
        float z;
        float w;

        GameQuaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
        GameQuaternion(float _x, float _y, float _z, float _w)
            : x(_x), y(_y), z(_z), w(_w) {}
    };

    // Minimal data-only 4x4 matrix for portable game-core boundaries.
    // Row-major fixed-layout representation. No operations are provided
    // here; conversion and multiplication belong in a later portable
    // math implementation task.
    struct GameMatrix
    {
        float m[4][4];

        GameMatrix()
        {
            for (int row = 0; row < 4; ++row)
            {
                for (int col = 0; col < 4; ++col)
                {
                    m[row][col] = (row == col) ? 1.0f : 0.0f;
                }
            }
        }

        GameMatrix(float m00, float m01, float m02, float m03,
                   float m10, float m11, float m12, float m13,
                   float m20, float m21, float m22, float m23,
                   float m30, float m31, float m32, float m33)
        {
            m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
            m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
            m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
            m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
        }
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
using GameVec2   = GameTypes::GameVec2;
using GameVec4   = GameTypes::GameVec4;
using GameQuaternion = GameTypes::GameQuaternion;
using GameMatrix = GameTypes::GameMatrix;