// GameTime.h
//
// Portable game-time contract for the RAN Online modernization.
//
// This header is intentionally platform-neutral. It must be includable
// from Windows, Android, and iOS projects without pulling in Windows.h,
// MFC, or any DirectX headers.
//
// The interface represents MONOTONIC ELAPSED TIME only (duration
// measurement, cooldown timing, timeouts). It does NOT represent
// calendar/wall-clock time. Calendar time belongs in a separate boundary.
//
// The Windows-specific implementation lives in
// Lib_Engine/DxCommon/DxWindowsTime and is the ONLY place that may
// reference timeGetTime, QueryPerformanceCounter, or other Win32 timing APIs.

#pragma once

#include "GameTypes.h"

namespace GameTime
{
    // ---------------------------------------------------------------------------
    // Portable monotonic time provider contract.
    //
    // Implementations own ALL platform-specific details. Consumers call only
    // portable methods with portable types.
    // ---------------------------------------------------------------------------
    class ITimeProvider
    {
    public:
        ~ITimeProvider() = default;

        // Returns the current monotonic time in milliseconds.
        //
        // The unit is milliseconds, matching the legacy timeGetTime() semantics.
        // The width is 32-bit (GameUInt32), matching the legacy DWORD width
        // used by CPROFILE and other callers. This preserves 32-bit wraparound
        // behavior for existing unsigned arithmetic on the returned value.
        virtual GameUInt32 GetMonotonicMilliseconds() const = 0;
    };
}
