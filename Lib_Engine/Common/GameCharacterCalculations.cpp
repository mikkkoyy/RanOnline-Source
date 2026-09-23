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

    // ========================================================================
    // Table-driven character calculations (Stage 2B/2C)
    //
    // These are deterministic pure calculations extracted from GLOGICEX
    // (Lib_Client/G-Logic/GLogicEx.cpp) and GLCONST_CHAR
    // (Lib_Client/G-Logic/GLogicData.cpp, GLogicDataLoad.cpp).
    //
    // The table data is loaded at runtime from configuration files by
    // GLCONST_CHAR. The portable module receives const pointers via
    // CharacterCalculationTables and does NOT own or load the data.
    //
    // All formulas, constants, clamping, integer widths, and signedness
    // are preserved byte-for-byte / behavior-for-behavior.
    // ========================================================================

    // ---------------------------------------------------------------------------
    // GetExpRate
    //
    // Original: GLCONST_CHAR::GETEXP_RATE
    //
    // int nDiffLev = nDefenserLev - nAttackerLev;
    // int nResultIndex = nDiffLev + EXPTABLE_RANGE_BASE;
    // if (nResultIndex >= EXPTABLE_RANGE) nResultIndex = EXPTABLE_RANGE-1;
    // else if (nResultIndex < 0) nResultIndex = 0;
    // return fEXP_RATE_TABLE[nResultIndex];
    // ---------------------------------------------------------------------------
    float GetExpRate(
        const CharacterCalculationTables& tables,
        int nAttackerLev,
        int nDefenserLev)
    {
        const int EXPTABLE_RANGE = 61;
        const int EXPTABLE_RANGE_BASE = 30;

        int nDiffLev = nDefenserLev - nAttackerLev;
        int nResultIndex = nDiffLev + EXPTABLE_RANGE_BASE;

        if (nResultIndex >= EXPTABLE_RANGE)
            nResultIndex = EXPTABLE_RANGE - 1;
        else if (nResultIndex < 0)
            nResultIndex = 0;

        return tables.pExpRateTable[nResultIndex];
    }

    // ---------------------------------------------------------------------------
    // AttackExp
    //
    // Original: GLOGICEX::GLATTACKEXP
    //
    // double fRate = static_cast<float>(dwDamage) / static_cast<double>(dwMaxHP);
    // if (fRate > 1.0f) fRate = 1.0f;
    // float fExpRate = GETEXP_RATE(nAttackerLev, nDefenserLev);
    // int nExp = int(dwBonusExp * fExpRate * fRate);
    // return (nExp < 0) ? 0 : nExp;
    //
    // NOTE: The original divides float by double. This is preserved exactly.
    // No divide-by-zero guard exists in the original; if dwMaxHP is 0 the
    // behavior is undefined (infinity/NaN). This is documented, not changed.
    // ---------------------------------------------------------------------------
    int AttackExp(
        const CharacterCalculationTables& tables,
        int nAttackerLev,
        int nDefenserLev,
        GameUInt32 dwDamage,
        GameUInt32 dwMaxHP,
        GameUInt32 dwBonusExp)
    {
        double fRate = static_cast<float>(dwDamage) / static_cast<double>(dwMaxHP);
        if (fRate > 1.0f)
            fRate = 1.0f;

        float fExpRate = GetExpRate(tables, nAttackerLev, nDefenserLev);
        int nExp = static_cast<int>(dwBonusExp * fExpRate * fRate);

        return (nExp < 0) ? 0 : nExp;
    }

    // ---------------------------------------------------------------------------
    // KillExp
    //
    // Original: GLOGICEX::GLKILLEXP
    //
    // float fExpRate = GETEXP_RATE(nAttackerLev, nDefenserLev);
    // int nExp = int(dwBonusExp * fExpRate * fKILL_EXP_RATE);
    // return (nExp < 0) ? 0 : nExp;
    // ---------------------------------------------------------------------------
    int KillExp(
        const CharacterCalculationTables& tables,
        int nAttackerLev,
        int nDefenserLev,
        GameUInt32 dwBonusExp)
    {
        float fExpRate = GetExpRate(tables, nAttackerLev, nDefenserLev);
        int nExp = static_cast<int>(dwBonusExp * fExpRate * tables.fKillExpRate);

        return (nExp < 0) ? 0 : nExp;
    }

    // ---------------------------------------------------------------------------
    // DieDecExp
    //
    // Original: GLOGICEX::GLDIE_DECEXP
    //
    // WORD wSTEP = wACTLEV / 10;
    // if (wSTEP >= DIE_DECEXP_NUM) wSTEP = DIE_DECEXP_NUM-1;
    // return fDIE_DECEXP[wSTEP];
    // ---------------------------------------------------------------------------
    float DieDecExp(
        const CharacterCalculationTables& tables,
        GameUInt16 wACTLEV)
    {
        const int DIE_DECEXP_NUM = 30;

        GameUInt16 wSTEP = static_cast<GameUInt16>(wACTLEV / 10);
        if (wSTEP >= DIE_DECEXP_NUM)
            wSTEP = DIE_DECEXP_NUM - 1;

        return tables.pDieDecExp[wSTEP];
    }

    // ---------------------------------------------------------------------------
    // DieRecoveryExp
    //
    // Original: GLOGICEX::GLDIE_RECOVERYEXP
    // ---------------------------------------------------------------------------
    float DieRecoveryExp(
        const CharacterCalculationTables& tables,
        GameUInt16 wACTLEV)
    {
        const int DIE_DECEXP_NUM = 30;

        GameUInt16 wSTEP = static_cast<GameUInt16>(wACTLEV / 10);
        if (wSTEP >= DIE_DECEXP_NUM)
            wSTEP = DIE_DECEXP_NUM - 1;

        return tables.pDieRecoveryExp[wSTEP];
    }

    // ---------------------------------------------------------------------------
    // DieExpMoney
    //
    // Original: GLOGICEX::GLDIE_EXPMONEY
    // ---------------------------------------------------------------------------
    float DieExpMoney(
        const CharacterCalculationTables& tables,
        GameUInt16 wACTLEV)
    {
        const int DIE_DECEXP_NUM = 30;

        GameUInt16 wSTEP = static_cast<GameUInt16>(wACTLEV / 10);
        if (wSTEP >= DIE_DECEXP_NUM)
            wSTEP = DIE_DECEXP_NUM - 1;

        return tables.pExpRateMoney[wSTEP];
    }

    // ---------------------------------------------------------------------------
    // NeedExp
    //
    // Original: GLOGICEX::GLNEEDEXP
    //
    // if (wLev >= MAX_LEVEL) return 0;
    // return lnEXP_MAX_TABLE[wLev];
    // ---------------------------------------------------------------------------
    GameInt64 NeedExp(
        const CharacterCalculationTables& tables,
        GameUInt16 wLev)
    {
        const int MAX_LEVEL = 300;

        if (wLev >= MAX_LEVEL)
            return 0;

        return tables.pExpMaxTable[wLev];
    }

    // ---------------------------------------------------------------------------
    // NeedExp2
    //
    // Original: GLOGICEX::GLNEEDEXP2
    //
    // if (wLev >= MAX_LEVEL) return 0;
    // return lnEXP_MAX_TABLE_2nd[wLev];
    // ---------------------------------------------------------------------------
    GameInt64 NeedExp2(
        const CharacterCalculationTables& tables,
        GameUInt16 wLev)
    {
        const int MAX_LEVEL = 300;

        if (wLev >= MAX_LEVEL)
            return 0;

        return tables.pExpMaxTable2nd[wLev];
    }
}