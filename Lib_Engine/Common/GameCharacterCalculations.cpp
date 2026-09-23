// GameCharacterCalculations.cpp
//
// Portable character combat calculation implementations.
//
// These are deterministic pure calculations extracted from GLOGICEX
// (Lib_Client/G-Logic/GLogicEx.cpp). The original formulas are preserved
// byte-for-byte / behavior-for-behavior. No gameplay constants, rounding,
// integer widths, or signedness have been changed.

#include "GameCharacterCalculations.h"

namespace GameCharacterCalculations
{
    // ---------------------------------------------------------------------------
    // SpaceGap
    //
    // Original: GLOGICEX::GLSPACEGAP
    //
    // Verified enum values from GLCharDefine.h:
    //   BRIGHT_LIGHT = 0, BRIGHT_DARK = 1
    //   BFB_DIS = 0, BFB_AVER = 1, BFB_ADV = 2
    // ---------------------------------------------------------------------------
    GameBrightFB SpaceGap(
        GameBright emACTOR,
        GameBright emRECEP,
        GameBright emSPACE)
    {
        if (emACTOR == emRECEP)
            return GameBrightFB::Aver;

        if (emSPACE == GameBright::Light)
        {
            if (emACTOR == GameBright::Light && emRECEP == GameBright::Dark)
                return GameBrightFB::Adv;
            if (emACTOR == GameBright::Dark && emRECEP == GameBright::Light)
                return GameBrightFB::Dis;
        }
        else
        {
            if (emACTOR == GameBright::Light && emRECEP == GameBright::Dark)
                return GameBrightFB::Dis;
            if (emACTOR == GameBright::Dark && emRECEP == GameBright::Light)
                return GameBrightFB::Adv;
        }

        return GameBrightFB::Aver;
    }

    // ---------------------------------------------------------------------------
    // HitRate
    //
    // Original: GLOGICEX::GLHITRATE
    //
    // const int nBirght[3] = { -10, 0, 10 };
    // enum { MAX_HIT = 99, MIN_HIT = 20, BASIC = 100 };
    // hitRate = BASIC + nHit - nAvoid + nBirght[bFB];
    // clamped to [MIN_HIT, MAX_HIT].
    // ---------------------------------------------------------------------------
    int HitRate(
        int nHit,
        int nAvoid,
        GameBrightFB bFB)
    {
        const int nBirght[3] = { -10, 0, 10 };
        enum { MAX_HIT = 99, MIN_HIT = 20, BASIC = 100 };

        int hitRate;
        hitRate = BASIC + nHit - nAvoid + nBirght[static_cast<int>(bFB)];

        if (hitRate > MAX_HIT)
            hitRate = MAX_HIT;
        else if (hitRate < MIN_HIT)
            hitRate = MIN_HIT;

        return hitRate;
    }

    // ---------------------------------------------------------------------------
    // Defense
    //
    // Original: GLOGICEX::GLDEFENSE
    //
    // float fFactor[BFB_SIZE] = { 0.8f, 1.0f, 1.2f };
    // EM_BRIGHT_FB brightFB = GLSPACEGAP(emACTOR, emRecep, emSPACE);
    // return int(nDEFENSE * fFactor[brightFB]);
    // ---------------------------------------------------------------------------
    int Defense(
        int nDEFENSE,
        GameBright emACTOR,
        GameBright emRecep,
        GameBright emSPACE)
    {
        float fFactor[3] = { 0.8f, 1.0f, 1.2f };
        GameBrightFB brightFB = SpaceGap(emACTOR, emRecep, emSPACE);

        return static_cast<int>(nDEFENSE * fFactor[static_cast<int>(brightFB)]);
    }

    // ---------------------------------------------------------------------------
    // SkillDelay
    //
    // Original: GLOGICEX::SKILLDELAY
    //
    // return static_cast<float>(dwSKILL_GRADE * wSKILL_LEV)
    //      / static_cast<float>(wCHAR_LEVEL) + fDelay;
    // ---------------------------------------------------------------------------
    float SkillDelay(
        GameUInt32 dwSKILL_GRADE,
        GameUInt16 wSKILL_LEV,
        GameUInt16 wCHAR_LEVEL,
        float fDelay)
    {
        return static_cast<float>(dwSKILL_GRADE * wSKILL_LEV)
             / static_cast<float>(wCHAR_LEVEL) + fDelay;
    }

    // ---------------------------------------------------------------------------
    // Variation (WORD overload)
    //
    // Original: GLOGICEX::VARIATION(WORD&, WORD, int)
    //
    // int nOld = int(wNow);
    // int nNew = nOld + nValue;
    // if (nNew < 0) nNew = 0;
    // if (nNew > USHRT_MAX) nNew = USHRT_MAX;
    // wNow = nNew;
    // if (wNow >= wMax) wNow = wMax;
    // WORD wDX = (WORD) abs(nOld - int(wNow));
    // return wDX;
    // ---------------------------------------------------------------------------
    GameUInt16 Variation(
        GameUInt16& wNow,
        GameUInt16 wMax,
        int nValue)
    {
        int nOld = static_cast<int>(wNow);

        int nNew = nOld + nValue;

        if (nNew < 0)
            nNew = 0;
        if (nNew > USHRT_MAX)
            nNew = USHRT_MAX;

        wNow = static_cast<GameUInt16>(nNew);
        if (wNow >= wMax)
            wNow = wMax;

        GameUInt16 wDX = static_cast<GameUInt16>(std::abs(nOld - static_cast<int>(wNow)));
        return wDX;
    }

    // ---------------------------------------------------------------------------
    // Variation (DWORD overload)
    //
    // Original: GLOGICEX::VARIATION(DWORD&, DWORD, int)
    //
    // int nOld = int(dwNow);
    // int nNew = nOld + nValue;
    // if (nNew < 0) nNew = 0;
    // dwNow = nNew;
    // if (dwNow >= dwMax) dwNow = dwMax;
    // DWORD dwDX = (DWORD) abs(nOld - int(dwNow));
    // return dwDX;
    //
    // NOTE: The original does NOT clamp the upper bound to a DWORD max;
    // it only checks nNew < 0. This is preserved exactly.
    // ---------------------------------------------------------------------------
    GameUInt32 Variation(
        GameUInt32& dwNow,
        GameUInt32 dwMax,
        int nValue)
    {
        int nOld = static_cast<int>(dwNow);

        int nNew = nOld + nValue;

        if (nNew < 0)
            nNew = 0;

        dwNow = static_cast<GameUInt32>(nNew);
        if (dwNow >= dwMax)
            dwNow = dwMax;

        GameUInt32 dwDX = static_cast<GameUInt32>(std::abs(nOld - static_cast<int>(dwNow)));
        return dwDX;
    }
}