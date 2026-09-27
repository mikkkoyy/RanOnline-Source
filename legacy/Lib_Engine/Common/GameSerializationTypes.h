#pragma once

// Portable serialization-compatible type boundary for RAN Online.
//
// This header is intentionally platform-neutral. It must be includable
// from Windows, Android, and iOS projects without pulling in DirectX,
// Windows.h, or MFC headers.
//
// The types defined here mirror the binary layout of existing DirectX
// serialization types used by CByteStream, CSerialFile, and CSerialMemory.
// They are data-only and provide no operators or conversion logic.
//
// Mapping to existing serialized DirectX types:
//
//   D3DVECTOR     (3 x float, 12 bytes)  ->  GameTypes::GameVec3
//   D3DCOLORVALUE (4 x float, 16 bytes)  ->  GameTypes::GameVec4
//   D3DBLEND      (enum, 4 bytes)        ->  GameTypes::GameUInt32
//
// All three existing serialization overloads perform a raw sizeof() byte
// copy. The portable equivalents below preserve the same contiguous
// float/uint32 layout so that future portable serialization code can
// produce identical byte streams.
//
// This header does NOT replace the existing DirectX serialization
// overloads. Those remain active and functional.

#include "GameTypes.h"

namespace GameSerialization
{
    // Serializable 3D position/direction vector.
    // Binary layout matches D3DVECTOR: three contiguous floats (12 bytes).
    using GamePosition = GameTypes::GameVec3;

    // Serializable RGBA color value.
    // Binary layout matches D3DCOLORVALUE: four contiguous floats (16 bytes).
    using GameColor = GameTypes::GameVec4;

    // Serializable blend-mode identifier.
    // Binary layout matches D3DBLEND: a 32-bit unsigned integer (4 bytes).
    using GameBlendMode = GameTypes::GameUInt32;
}

// Convenience aliases in the global namespace.
using GamePosition   = GameSerialization::GamePosition;
using GameColor      = GameSerialization::GameColor;
using GameBlendMode  = GameSerialization::GameBlendMode;