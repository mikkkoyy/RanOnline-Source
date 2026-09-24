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

    // ========================================================================
    // Stage 2E — RNG-dependent character calculations.
    //
    // These functions accept a caller-supplied normalized random value
    // in [0.0, 1.0]. The portable layer does NOT generate randomness;
    // the legacy layer supplies it from RANDOM_POS, seqrandom::getpercent(),
    // or any future RNG source.
    //
    // See GameRandom.h for the probability primitives these consume.
    // ========================================================================

    // ---------------------------------------------------------------------------
    // CheckShock — shock/stun probability check.
    //
    // Legacy: GLOGICEX::CHECKSHOCK
    //
    // enum { CLEANHIT_RATE = 1, CRITICALHIT_RATE = 5, MIN_DXLEVEL = 5, MIN_DAMAGE = 6 };
    // int nDXLEV = nDEFLEV - nACTLEV;
    // if ( (-MIN_DXLEVEL) > nDXLEV )   return FALSE;
    // if ( (nACTLEV+MIN_DAMAGE) > nDamage ) return FALSE;
    // if ( bCritical ) return (CRITICALHIT_RATE > (RANDOM_POS*100));
    // return (CLEANHIT_RATE > (RANDOM_POS*100));
    //
    // Equivalent to GameRandom::CheckProbability(rate, randomValue).
    // ---------------------------------------------------------------------------
    bool CheckShock(
        int nACTLEV,
        int nDEFLEV,
        int nDamage,
        bool bCritical,
        float randomValue);

    // ---------------------------------------------------------------------------
    // CheckStateBlow — state-blow (ailment) probability check.
    //
    // Legacy: GLOGICEX::CHECKSTATEBLOW
    //
    // int nDXLEVEL = int(wLEVEL - wACTLEVEL);
    // int nINDEX = nDXLEVEL + nStateBlowLevelBase;
    // if (nINDEX < 0) nINDEX = 0;
    // if (nINDEX >= nStateBlowLevelSize) nINDEX = nStateBlowLevelSize-1;
    // return (RANDOM_POS*100.0f) < (fACTRATE - fACTRATE * 0.01f * wRESIST * 0.6f
    //                                + nStateBlowLevel[nINDEX]);
    //
    // Equivalent to GameRandom::CheckProbability(threshold, randomValue).
    // ---------------------------------------------------------------------------
    bool CheckStateBlow(
        float fACTRATE,
        GameUInt16 wACTLEVEL,
        GameUInt16 wLEVEL,
        GameUInt16 wRESIST,
        const int* pStateBlowLevel,
        int nStateBlowLevelBase,
        int nStateBlowLevelSize,
        float randomValue);

    // ========================================================================
    // Table-driven character calculations (Stage 2B/2C)
    //
    // These tables are loaded at runtime from configuration files
    // (default.charclass, exptable_max.bin, exptable_max_2nd.bin) by
    // GLCONST_CHAR. The portable module receives const pointers to this
    // data via CharacterCalculationTables. It does NOT own or load the
    // data itself, and it does NOT depend on GLCONST_CHAR.
    //
    // Compile-time sizes (verified from GLogicData.h):
    //   EXPTABLE_RANGE       = 61   (level differences -30..+30)
    //   EXPTABLE_RANGE_BASE  = 30   (offset into the exp-rate table)
    //   DIE_DECEXP_NUM       = 30   (death tables, indexed by level/10)
    //   MAX_LEVEL            = 300  (level EXP tables)
    // ========================================================================

    struct CharacterCalculationTables
    {
        // EXP rate table indexed by (defenserLev - attackerLev + 30),
        // clamped to [0, 60]. 61 entries.
        const float* pExpRateTable;      // EXPTABLE_RANGE = 61

        // Kill EXP rate scalar.
        float fKillExpRate;

        // Death EXP reduction table, indexed by (level / 10),
        // clamped to [0, 29]. 30 entries.
        const float* pDieDecExp;         // DIE_DECEXP_NUM = 30

        // Death EXP recovery table, indexed by (level / 10),
        // clamped to [0, 29]. 30 entries.
        const float* pDieRecoveryExp;    // DIE_DECEXP_NUM = 30

        // EXP-to-money table, indexed by (level / 10),
        // clamped to [0, 29]. 30 entries.
        const float* pExpRateMoney;      // DIE_DECEXP_NUM = 30

        // Normal level EXP table, indexed by level [0, MAX_LEVEL-1].
        // 300 entries.
        const GameInt64* pExpMaxTable;       // MAX_LEVEL = 300

        // Second level EXP table, indexed by level [0, MAX_LEVEL-1].
        // 300 entries.
        const GameInt64* pExpMaxTable2nd;    // MAX_LEVEL = 300
    };

    // ---------------------------------------------------------------------------
    // EXP rate lookup.
    //
    // Legacy: GLCONST_CHAR::GETEXP_RATE
    //
    // nResultIndex = (nDefenserLev - nAttackerLev) + EXPTABLE_RANGE_BASE
    // clamped to [0, EXPTABLE_RANGE-1]
    // ---------------------------------------------------------------------------
    float GetExpRate(
        const CharacterCalculationTables& tables,
        int nAttackerLev,
        int nDefenserLev);

    // ---------------------------------------------------------------------------
    // Attack EXP calculation.
    //
    // Legacy: GLOGICEX::GLATTACKEXP
    //
    // fRate = dwDamage / dwMaxHP, upper-clamped to 1.0
    // nExp = int(dwBonusExp * GetExpRate * fRate)
    // negative result -> 0
    // ---------------------------------------------------------------------------
    int AttackExp(
        const CharacterCalculationTables& tables,
        int nAttackerLev,
        int nDefenserLev,
        GameUInt32 dwDamage,
        GameUInt32 dwMaxHP,
        GameUInt32 dwBonusExp);

    // ---------------------------------------------------------------------------
    // Kill EXP calculation.
    //
    // Legacy: GLOGICEX::GLKILLEXP
    //
    // nExp = int(dwBonusExp * GetExpRate * fKillExpRate)
    // negative result -> 0
    // ---------------------------------------------------------------------------
    int KillExp(
        const CharacterCalculationTables& tables,
        int nAttackerLev,
        int nDefenserLev,
        GameUInt32 dwBonusExp);

    // ---------------------------------------------------------------------------
    // Death EXP reduction.
    //
    // Legacy: GLOGICEX::GLDIE_DECEXP
    //
    // wSTEP = wACTLEV / 10, upper-clamped to DIE_DECEXP_NUM-1 (29)
    // ---------------------------------------------------------------------------
    float DieDecExp(
        const CharacterCalculationTables& tables,
        GameUInt16 wACTLEV);

    // ---------------------------------------------------------------------------
    // Death EXP recovery.
    //
    // Legacy: GLOGICEX::GLDIE_RECOVERYEXP
    // ---------------------------------------------------------------------------
    float DieRecoveryExp(
        const CharacterCalculationTables& tables,
        GameUInt16 wACTLEV);

    // ---------------------------------------------------------------------------
    // EXP-to-money.
    //
    // Legacy: GLOGICEX::GLDIE_EXPMONEY
    // ---------------------------------------------------------------------------
    float DieExpMoney(
        const CharacterCalculationTables& tables,
        GameUInt16 wACTLEV);

    // ---------------------------------------------------------------------------
    // Level EXP requirement.
    //
    // Legacy: GLOGICEX::GLNEEDEXP
    //
    // wLev >= MAX_LEVEL (300) -> 0, otherwise direct table lookup
    // ---------------------------------------------------------------------------
    GameInt64 NeedExp(
        const CharacterCalculationTables& tables,
        GameUInt16 wLev);

    // ---------------------------------------------------------------------------
    // Second level EXP requirement.
    //
    // Legacy: GLOGICEX::GLNEEDEXP2
    // ---------------------------------------------------------------------------
    GameInt64 NeedExp2(
        const CharacterCalculationTables& tables,
        GameUInt16 wLev);
}

// Convenience aliases in the global namespace for minimal friction
// during incremental adoption.
using GameBright       = GameCharacterCalculations::GameBright;
using GameBrightFB     = GameCharacterCalculations::GameBrightFB;