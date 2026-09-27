#pragma once

// GameRandom.h
//
// Stage 2D — Character/gameplay RNG boundary.
//
// This header is intentionally platform-neutral. It must be includable
// from Windows, Android, and iOS projects without pulling in DirectX,
// Windows.h, MFC, GLCharacter, GLCharData, GLDefine, GLGaeaClient, or
// GLGaeaServer.
//
// ---------------------------------------------------------------------------
// RNG BOUNDARY CONTRACT
// ---------------------------------------------------------------------------
//
// The portable game core must NOT directly depend on:
//
//   - rand() / srand()
//   - RAND_MAX
//   - GLDefine.h (RANDOM_POS, RANDOM_NUM, RANDOM_GEN macros)
//   - seqrandom namespace
//
// Instead, portable calculations that require randomness must accept a
// normalized random value as an explicit parameter.
//
// The RNG SOURCE (how the random value is produced) belongs to the
// platform/legacy layer, NOT to the portable calculation.
//
// Current RNG sources (documented for reference, NOT pulled in here):
//
//   Source                | Location            | Range        | Notes
//   ----------------------|---------------------|--------------|--------------------------
//   RANDOM_POS            | GLDefine.h:11       | 0.0f ~ 1.0f  | (float)rand()/RAND_MAX
//   RANDOM_NUM            | GLDefine.h:12       | -1.0f ~ 1.0f | (RANDOM_POS*2.f)-1.f
//   RANDOM_GEN            | GLDefine.h:18-21    | bool         | fRate >= seqrandom::getpercent()
//   seqrandom::getpercent | seqrandom.cpp:38-41 | 0.0f ~ 100.0f| sequential RNG, seeded at startup
//
// seqrandom is the existing sequential RNG module at
// Lib_Engine/Common/seqrandom.{h,cpp}. It is already suitable as the
// portable RNG implementation and is NOT rewritten here.
//
// ---------------------------------------------------------------------------
// STAGE 2E CONSUMPTION
// ---------------------------------------------------------------------------
//
// Future character calculations (CHECKSHOCK, CHECKSTATEBLOW) will be
// refactored to accept a random value parameter and delegate to the
// helpers below. Example:
//
//   // Legacy (Stage 2D — unchanged):
//   BOOL CHECKSHOCK(...) { return (CLEANHIT_RATE > (RANDOM_POS*100)); }
//
//   // Stage 2E (target):
//   bool CheckShock(..., float randomValue)
//   {
//       return GameRandom::CheckProbability(CLEANHIT_RATE, randomValue);
//   }
//
// The caller (legacy layer) supplies randomValue from RANDOM_POS,
// seqrandom::getpercent(), or any future RNG source without the
// portable calculation caring which.

#include "GameTypes.h"

namespace GameRandom
{
    // ---------------------------------------------------------------------------
    // CheckProbability
    //
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
    //
    // NOTE: The comparison uses strict less-than to match the original
    // RANDOM_POS-based checks exactly.
    // ---------------------------------------------------------------------------
    inline bool CheckProbability(
        float rate,
        float randomValue)
    {
        return (randomValue * 100.0f) < rate;
    }

    // ---------------------------------------------------------------------------
    // CheckProbabilityRate
    //
    // Pure probability check on a rate scale of 0.0-1.0 (fractional).
    //
    // Parameters:
    //   rate         - Threshold in [0.0, 1.0]. The check passes when the
    //                  random value falls below this threshold.
    //   randomValue  - Caller-supplied normalized random value in [0.0, 1.0].
    //
    // Returns true when randomValue < rate.
    //
    // Legacy equivalent: RANDOM_GEN(rate) when rate is fractional, or
    // (rate >= seqrandom::getpercent() / 100.0f).
    // ---------------------------------------------------------------------------
    inline bool CheckProbabilityRate(
        float rate,
        float randomValue)
    {
        return randomValue < rate;
    }
}