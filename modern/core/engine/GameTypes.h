#pragma once

// VERTICAL-006: minimal game types for combat calculations.
//
// This is a clean modern version without legacy complications.

#include <cstdint>
#include <cstddef>

namespace Modern::Engine
{
    // Fixed-width integer aliases for the portable game core.
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

    // Brightness enum for hit calculation
    enum class GameBright : uint8_t
    {
        Light = 0,
        Dark  = 1
    };

    // Brightness feedback enum
    enum class GameBrightFB : uint8_t
    {
        Dis  = 0,
        Aver = 1,
        Adv  = 2
    };

    // Element enum for weather/skill calculations
    enum class GameElement : uint8_t
    {
        Spirit  = 0,
        Fire    = 1,
        Ice     = 2,
        Electric = 3,
        Poison  = 4,
        Stone   = 5,
        Mad     = 6,
        Stun    = 7,
        Curse   = 8
    };

    // State blow enum
    enum class GameStateBlow : uint8_t
    {
        None   = 0,
        Numb   = 1,
        Stun   = 2,
        Stone  = 3,
        Burn   = 4,
        Frozen = 5,
        Mad    = 6,
        Poison = 7,
        Curse  = 8
    };
}

// Convenience aliases in the global namespace
using GameInt8   = Modern::Engine::GameInt8;
using GameUInt8  = Modern::Engine::GameUInt8;
using GameInt16  = Modern::Engine::GameInt16;
using GameUInt16 = Modern::Engine::GameUInt16;
using GameInt32  = Modern::Engine::GameInt32;
using GameUInt32 = Modern::Engine::GameUInt32;
using GameInt64  = Modern::Engine::GameInt64;
using GameUInt64 = Modern::Engine::GameUInt64;
using GameSizeT  = Modern::Engine::GameSizeT;