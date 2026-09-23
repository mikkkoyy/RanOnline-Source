#pragma once

// GameCharacterCalculations.h
//
// Portable character combat calculation boundary.
//
// This header is intentionally platform-neutral. It must be includable
// from Windows, Android, and iOS projects without pulling in DirectX,
// Windows.h, MFC, GLCharacter, GLCharData, GLDefine, GLGaeaClient, or
// GLGaeaServer.
//
// The functions here are deterministic pure calculations extracted from
// GLOGICEX (GLogicEx.cpp). They preserve the original RAN Online formulas
// byte-for-byte / behavior-for-behavior.
//
// Legacy enum mapping (verified from GLCharDefine.h):
//
//   EMBRIGHT:
//       BRIGHT_LIGHT = 0
//       BRIGHT_DARK  = 1
//
//   EM_BRIGHT_FB:
//       BFB_DIS  = 0   // disadvantage
//       BFB_AVER = 1   // average
//       BFB_ADV = 2    // advantage
//       BFB_SIZE = 3
//
// These are re-defined here as scoped enums so the portable module does
// not need to include GLCharDefine.h.

#include <cstdlib>   // std::abs
#include <climits>   // USHRT_MAX

#include "GameTypes.h"

namespace GameCharacterCalculations
{
    // Brightness / light-dark state.
    enum class GameBright : GameUInt8
    {
        Light = 0,   // BRIGHT_LIGHT
        Dark  = 1    // BRIGHT_DARK
    };

    // Brightness factor used by hit-rate and defense calculations.
    enum class GameBrightFB : GameUInt8
    {
        Dis  = 0,    // BFB_DIS
        Aver = 1,    // BFB_AVER
        Adv = 2,     // BFB_ADV
        Size = 3     // BFB_SIZE
    };

    // ---------------------------------------------------------------------------
    // Space gap (actor/receiver/environment brightness relationship).
    //
    // Legacy: GLOGICEX::GLSPACEGAP
    //
    // Returns GameBrightFB indicating advantage/average/disadvantage based
    // on the brightness relationship between actor, receiver, and space.
    // ---------------------------------------------------------------------------
    GameBrightFB SpaceGap(
        GameBright emACTOR,
        GameBright emRECEP,
        GameBright emSPACE);

    // ---------------------------------------------------------------------------
    // Hit rate calculation.
    //
    // Legacy: GLOGICEX::GLHITRATE
    //
    // Formula: BASIC(100) + nHit - nAvoid + nBirght[bFB]
    // Clamped to [MIN_HIT(20), MAX_HIT(99)].
    // ---------------------------------------------------------------------------
    int HitRate(
        int nHit,
        int nAvoid,
        GameBrightFB bFB);

    // ---------------------------------------------------------------------------
    // Defense calculation.
    //
    // Legacy: GLOGICEX::GLDEFENSE
    //
    // Formula: int(nDEFENSE * fFactor[bFB])
    // where fFactor = { 0.8f (Dis), 1.0f (Aver), 1.2f (Adv) }
    // ---------------------------------------------------------------------------
    int Defense(
        int nDEFENSE,
        GameBright emACTOR,
        GameBright emRecep,
        GameBright emSPACE);

    // ---------------------------------------------------------------------------
    // Skill delay calculation.
    //
    // Legacy: GLOGICEX::SKILLDELAY
    //
    // Formula: float(dwSKILL_GRADE * wSKILL_LEV) / float(wCHAR_LEVEL) + fDelay
    // Fully standalone; no skill-system dependencies.
    // ---------------------------------------------------------------------------
    float SkillDelay(
        GameUInt32 dwSKILL_GRADE,
        GameUInt16 wSKILL_LEV,
        GameUInt16 wCHAR_LEVEL,
        float fDelay);

    // ---------------------------------------------------------------------------
    // VARIATION (WORD overload).
    //
    // Legacy: GLOGICEX::VARIATION(WORD, WORD, int)
    //
    // Applies nValue to wNow using int arithmetic, clamps to [0, USHRT_MAX],
    // then clamps to wMax. Returns the absolute delta.
    // ---------------------------------------------------------------------------
    GameUInt16 Variation(
        GameUInt16& wNow,
        GameUInt16 wMax,
        int nValue);

    // ---------------------------------------------------------------------------
    // VARIATION (DWORD overload).
    //
    // Legacy: GLOGICEX::VARIATION(DWORD, DWORD, int)
    //
    // Applies nValue to dwNow using int arithmetic, clamps to [0, ...],
    // then clamps to dwMax. Returns the absolute delta.
    //
    // NOTE: The original does NOT clamp the upper bound to a DWORD max;
    // it only checks nNew < 0. This is preserved exactly.
    // ---------------------------------------------------------------------------
    GameUInt32 Variation(
        GameUInt32& dwNow,
        GameUInt32 dwMax,
        int nValue);
}

// Convenience aliases in the global namespace for minimal friction
// during incremental adoption.
using GameBright       = GameCharacterCalculations::GameBright;
using GameBrightFB     = GameCharacterCalculations::GameBrightFB;