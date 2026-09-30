#pragma once

// VERTICAL-006: RNG boundary for combat calculations.
//
// This header is intentionally platform-neutral. It must be includable
// from any platform without pulling in platform-specific dependencies.

#include "GameTypes.h"

namespace Modern::Engine
{
    // Pure probability check. Does NOT generate randomness.
    //
    // Parameters:
    //   rate         - Threshold (percentage scale, 0-100). The check passes
    //                  when the random value falls below this threshold.
    //   randomValue  - Caller-supplied normalized random value in [0.0, 1.0].
    //
    // Returns true when (randomValue * 100.0) < rate.
    //
    // Legacy equivalent: (rate > (RANDOM_POS * 100.0f))
    inline bool CheckProbability(
        float rate,
        float randomValue)
    {
        return (randomValue * 100.0f) < rate;
    }

    // Pure probability check on a rate scale of 0.0-1.0 (fractional).
    inline bool CheckProbabilityRate(
        float rate,
        float randomValue)
    {
        return randomValue < rate;
    }
}