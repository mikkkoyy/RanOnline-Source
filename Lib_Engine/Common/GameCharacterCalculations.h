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
    // Formula: BASIC(100) + nHit - nAvoid + brightnessModifier
    // Clamped to [MIN_HIT(20), MAX_HIT(99)].
    //
    // The brightness modifier is the caller's already-resolved value from
    // the legacy EM_BRIGHT_FB table { -10, 0, 10 } (BFB_DIS, BFB_AVER,
    // BFB_ADV). It is passed as a plain GameInt32 so the portable layer
    // does NOT depend on EM_BRIGHT_FB or GLSPACEGAP. The caller computes
    // the modifier via SpaceGap() and passes the resolved integer.
    // ---------------------------------------------------------------------------
    GameInt32 HitRate(
        GameInt32 nHit,
        GameInt32 nAvoid,
        GameInt32 brightnessModifier);

    // ---------------------------------------------------------------------------
    // Defense calculation.
    //
    // Legacy: GLOGICEX::GLDEFENSE
    //
    // Formula: int(nDEFENSE * environmentFactor)
    // where environmentFactor = { 0.8f (Dis), 1.0f (Aver), 1.2f (Adv) }
    //
    // The environment factor is the caller's already-resolved value from
    // the legacy EM_BRIGHT_FB table { 0.8f, 1.0f, 1.2f } (BFB_DIS, BFB_AVER,
    // BFB_ADV). It is passed as a plain float so the portable layer does
    // NOT depend on EM_BRIGHT_FB, EMBRIGHT, or GLSPACEGAP. The caller
    // computes the factor via SpaceGap() and passes the resolved float.
    // ---------------------------------------------------------------------------
    GameInt32 Defense(
        GameInt32 defense,
        float environmentFactor);

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

    // ---------------------------------------------------------------------------
    // MoveVelocity — character movement speed calculation.
    //
    // Legacy: GLOGICEX equivalents in GLCharacter::GetMoveVelo(),
    // GLChar::GetMoveVelo(), GLCrow::GetMoveVelo().
    //
    // Formula: baseVelocity * (sumMoveVelocity + itemMoveVelocity)
    //
    // The caller is responsible for resolving:
    //   - baseVelocity: class/crow base run or walk velocity (selected by
    //     the run/walk state flag)
    //   - sumMoveVelocity: character stat + buff movement velocity sum
    //   - itemMoveVelocity: equipment movement velocity bonus (0.0f for CROW)
    //   - isRunMode: whether the character is in run mode
    //
    // NOTE: isRunMode is accepted for symmetry with the legacy callers but
    // is NOT used in the formula — the base velocity is already resolved
    // to run or walk by the caller before calling this function.
    // ---------------------------------------------------------------------------
    float MoveVelocity(
        float baseVelocity,
        float sumMoveVelocity,
        float itemMoveVelocity,
        bool isRunMode);

    // ---------------------------------------------------------------------------
    // IsLowerHP — crow action-pattern selection based on HP threshold.
    //
    // Legacy: GLCrow::IsLowerHP()
    //
    // for each pattern i:
    //     threshold = (int)((float)maxHP * patternRates[i]) / 100.0f
    //     if (currentHP >= threshold) return i
    // return -1
    //
    // CRITICAL: The (int) cast truncates the product BEFORE division by 100.0f.
    // This is preserved exactly — do NOT move the cast after the division.
    // ---------------------------------------------------------------------------
    int IsLowerHP(
        GameUInt32 currentHP,
        GameUInt32 maxHP,
        const float* patternRates,
        GameSizeT patternCount);

    // ---------------------------------------------------------------------------
    // AttackVelocity — attack animation speed calculation.
    //
    // Legacy: GLCHARLOGIC::GETATTVELO(), GLCharClient::GETATTVELO(),
    //         GLCROWLOGIC::GETATTVELO(), GLSUMMON::GETATTVELO().
    //
    // PC:   float fATTVELO = m_fATTVELO + m_sSUMITEM.fIncR_AtkSpeed;
    //       return fATTVELO < 0.0f ? 0.0f : fATTVELO;
    // NPC:  return m_fATTVELO < 0.0f ? 0.0f : m_fATTVELO;
    // Client: float fATTVELO = m_fATTVELO + m_fITEMATTVELO_R;
    //         return fATTVELO < 0.0f ? 0.0f : fATTVELO;
    //
    // The caller is responsible for:
    //   - obtaining baseAttackVelocity (m_fATTVELO, already resolved)
    //   - obtaining itemAttackVelocityRate (m_sSUMITEM.fIncR_AtkSpeed for
    //     PC, m_fITEMATTVELO_R for client, 0.0f for NPC)
    //   - all surrounding animation/timing pipeline (fSkinAniElap, m_fattTIMER)
    //
    // The portable function does NOT generate randomness. It only performs
    // the deterministic floating-point addition and negative clamp.
    // ---------------------------------------------------------------------------
    float AttackVelocity(
        float baseAttackVelocity,
        float itemAttackVelocityRate);

    // ========================================================================
    // Stage 2F-3 — Weather power calculations.
    //
    // Minimal portable element/state-blow enums. Numeric values match the
    // legacy EMELEMENT and EMSTATE_BLOW enums exactly (verified from
    // Lib_Client/G-Logic/GLCharDefine.h). The portable layer does NOT
    // import GLCharDefine.h.
    //
    //   GameElement: SPIRIT=0, FIRE=1, ICE=2, ELECTRIC=3, POISON=4,
    //                STONE=5, MAD=6, STUN=7, CURSE=8
    //
    //   GameStateBlow: NONE=0, NUMB=1, STUN=2, STONE=3, BURN=4,
    //                  FROZEN=5, MAD=6, POISON=7, CURSE=8
    //
    // STATE_TO_ELEMENT mapping (legacy inline function):
    //   NUMB→ELECTRIC, STUN→STUN, STONE→STONE, BURN→FIRE,
    //   FROZEN→ICE, MAD→MAD, POISON→POISON, CURSE→CURSE,
    //   default→SPIRIT
    // ========================================================================

    enum class GameElement : int
    {
        Spirit = 0,
        Fire   = 1,
        Ice    = 2,
        Electric = 3,
        Poison = 4,
        Stone  = 5,
        Mad    = 6,
        Stun   = 7,
        Curse  = 8
    };

    enum class GameStateBlow : int
    {
        None    = 0,
        Numb    = 1,
        Stun    = 2,
        Stone   = 3,
        Burn    = 4,
        Frozen  = 5,
        Mad     = 6,
        Poison  = 7,
        Curse   = 8
    };

    // ---------------------------------------------------------------------------
    // StateBlowToElement — legacy STATE_TO_ELEMENT mapping.
    //
    // Legacy: STATE_TO_ELEMENT(emBlow) inline switch in GLCharDefine.h:942-957
    // ---------------------------------------------------------------------------
    GameElement StateBlowToElement(GameStateBlow blow);

    // ---------------------------------------------------------------------------
    // WeatherElementPower
    //
    // Legacy: GLOGICEX::WEATHER_ELEMENT_POW
    //
    // Weather inactive -> 1.0f
    // FIRE/STONE:    rain->0.8f, leaves->1.2f
    // ICE/MAD:       leaves->0.8f, snow->1.2f
    // ELECTRIC/STUN: snow->0.8f, rain->1.2f
    // POISON/CURSE:  1.0f
    //
    // The weather flags are passed as a raw DWORD (bitmask) from the caller;
    // the portable function only tests specific bit positions via the
    // caller-supplied flag constants. It does NOT define the flags itself.
    // ---------------------------------------------------------------------------
    float WeatherElementPower(
        GameElement element,
        GameUInt32 weatherFlags,
        bool weatherActive);

    // ---------------------------------------------------------------------------
    // WeatherBlowPower
    //
    // Legacy: GLOGICEX::WEATHER_BLOW_POW
    //
    // Maps the state-blow to an element via StateBlowToElement, then delegates
    // to WeatherElementPower.
    // ---------------------------------------------------------------------------
    float WeatherBlowPower(
        GameStateBlow blow,
        GameUInt32 weatherFlags,
        bool weatherActive);

    // ---------------------------------------------------------------------------
    // DamageReduceAmount — damage reduction amount from level-scaled modifier.
    //
    // Legacy: GLOGIXEXPC.cpp:1728 and GLOGICEXNPC.cpp:304
    //
    // int nDamageReduce = (int) (
    //     ( (rResultDAMAGE * fDamageReduce) * nLEVEL )
    //     / GLCONST_CHAR::wMAX_LEVEL
    // );
    //
    // The caller is responsible for:
    //   - obtaining rResultDAMAGE (post-armor damage)
    //   - obtaining fDamageReduce (reduction modifier, 0.0-1.0)
    //   - obtaining nLEVEL (target level)
    //   - obtaining maxLevel (GLCONST_CHAR::wMAX_LEVEL = 300)
    //   - subtracting the returned reduction from rResultDAMAGE
    //
    // CRITICAL: The (int) cast truncates the floating-point result. The
    // multiplication order is preserved exactly: (damage * damageReduce)
    // * level / maxLevel. Do NOT reorder or "simplify".
    // ---------------------------------------------------------------------------
    GameInt32 DamageReduceAmount(
        GameInt32 damage,
        float damageReduce,
        GameInt32 level,
        GameInt32 maxLevel);

    // ---------------------------------------------------------------------------
    // DamageReflectionAmount — damage reflection amount from level-scaled modifier.
    //
    // Legacy: GLogixExPC.cpp:1742 and GLogicExNPC.cpp:318
    //
    // int nDamageReflection = (int) (
    //     ( (rResultDAMAGE * fDamageReflection) * nLEVEL )
    //     / GLCONST_CHAR::wMAX_LEVEL
    // );
    //
    // The caller is responsible for:
    //   - obtaining rResultDAMAGE (post-armor damage)
    //   - obtaining fDamageReflection (reflection modifier)
    //   - obtaining nLEVEL (target level)
    //   - obtaining maxLevel (GLCONST_CHAR::wMAX_LEVEL = 300)
    //   - deciding whether reflection activates (RANDOM_POS check stays in legacy)
    //   - calling DamageReflectionProc() with the returned amount
    //
    // CRITICAL: The (int) cast truncates the floating-point result. The
    // multiplication order is preserved exactly: (damage * damageReflection)
    // * level / maxLevel. Do NOT reorder or "simplify".
    // ---------------------------------------------------------------------------
    GameInt32 DamageReflectionAmount(
        GameInt32 damage,
        float damageReflection,
        GameInt32 level,
        GameInt32 maxLevel);

    // ---------------------------------------------------------------------------
    // CriticalBaseRate — base critical hit rate from HP ratio and level delta.
    //
    // Legacy: GLogixExPC.cpp:1603-1613 and GLogicExNPC.cpp:262-271
    //
    // int ndxLvl = nLEVEL - GETLEVEL();
    // if ( ndxLvl > 5 )  ndxLvl = 5;
    // if ( ndxLvl < -5 ) ndxLvl = -5;
    // int nPerHP = ((GETHP()*100)/GETMAXHP());
    // if ( nPerHP <= 10 ) nPerHP = 10;
    // int nPercentCri = 1000 / nPerHP - 10 + ndxLvl;
    //
    // The caller is responsible for:
    //   - obtaining currentHP and maxHP (attacker)
    //   - obtaining attackerLevel and targetLevel
    //   - adding item critical bonus (m_sSUMITEM.fIncR_Critical * 100)
    //   - applying GLCONST_CHAR::dwCRITICAL_MAX clamp
    //   - applying the RANDOM_POS proc check
    //
    // All arithmetic is integer-only. No floating point. No RNG.
    // ---------------------------------------------------------------------------
    GameInt32 CriticalBaseRate(
        GameInt32 currentHP,
        GameInt32 maxHP,
        GameInt32 attackerLevel,
        GameInt32 targetLevel);

    // ---------------------------------------------------------------------------
    // RandomDamageRange — random damage interpolation within [low, high].
    //
    // Legacy: GLogixExPC.cpp:1668,1997 and GLogicExNPC.cpp:283
    //
    // int nDAMAGE_NOW = int(gdDamage.dwLow +
    //     (gdDamage.dwHigh - gdDamage.dwLow) * RANDOM_POS);
    //
    // The caller is responsible for:
    //   - obtaining lowDamage and highDamage (precomputed damage range)
    //   - supplying the random value (RANDOM_POS, 0.0-1.0) from the legacy RNG
    //   - all surrounding damage pipeline (nExtFORCE, defense, critical, etc.)
    //
    // The portable function does NOT generate randomness. It only performs
    // the deterministic floating-point interpolation and integer truncation.
    // ---------------------------------------------------------------------------
    GameInt32 RandomDamageRange(
        GameUInt32 lowDamage,
        GameUInt32 highDamage,
        float randomValue);

    // ---------------------------------------------------------------------------
    // ApplyStateDamage — state damage multiplier.
    //
    // Legacy: GLogixExPC.cpp:1684,2008 and GLogicExNPC.cpp:294
    //
    // rResultDAMAGE = int(rResultDAMAGE * fSTATE_DAMAGE);
    //
    // The caller is responsible for:
    //   - obtaining rResultDAMAGE (post-armor damage, pre-state)
    //   - obtaining fSTATE_DAMAGE (state damage multiplier)
    //   - all surrounding damage pipeline (armor, critical, reflection, etc.)
    //
    // The portable function does NOT generate randomness. It only performs
    // the deterministic floating-point multiplication and integer truncation.
    // ---------------------------------------------------------------------------
    GameInt32 ApplyStateDamage(
        GameInt32 damage,
        float stateDamage);

    // ---------------------------------------------------------------------------
    // ApplyDamageRate — damage-rate scaling with integer truncation.
    //
    // Legacy: GLogixExPC.cpp:1598-1599,1954-1955,3002-3003 and
    //         GLogicExNPC.cpp:257-258
    //
    // PC:  gdDamage.dwLow  = DWORD ( gdDamage.dwLow  * m_fDamageRate );
    //      gdDamage.dwHigh = DWORD ( gdDamage.dwHigh * m_fDamageRate );
    // NPC: gdDamage.wLow  = int ( gdDamage.wLow  * m_fDamageRate );
    //      gdDamage.wHigh = int ( gdDamage.wHigh * m_fDamageRate );
    //
    // The caller is responsible for:
    //   - obtaining the pre-rate damage value (low/high)
    //   - obtaining m_fDamageRate (attack power change rate)
    //   - all surrounding damage pipeline (nExtFORCE, critical, armor, etc.)
    //
    // The portable function does NOT generate randomness. It only performs
    // the deterministic floating-point multiplication and integer truncation.
    // The caller supplies the damage rate multiplier.
    //
    // NOTE: The return type is GameUInt32 to safely represent both DWORD-sized
    // (PC) and WORD-sized (NPC) damage values. The NPC caller casts back to
    // WORD via static_cast<decltype(gdDamage.wLow)> to preserve the original
    // destination-field conversion behavior.
    // ---------------------------------------------------------------------------
    GameUInt32 ApplyDamageRate(
        GameUInt32 damage,
        float damageRate);

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

    // ---------------------------------------------------------------------------
    // SkillTargetRange — skill target range arithmetic.
    //
    // Legacy: GLCHARLOGIC::GETSKILLRANGE_TAR (GLogixExPC.cpp:4474)
    //
    // int nRANGE = sSKILL.m_sBASIC.wTARRANGE;
    // if ( nRANGE < 20 ) nRANGE = 20;
    // if ( sSKILL.m_sBASIC.emAPPLY==SKILL::EMAPPLY_PHY_LONG )
    //     nRANGE += (int) GETSUM_TARRANGE() + 5;
    // if ( sSKILL.m_sBASIC.emIMPACT_SIDE == SIDE_ENEMY )
    //     nRANGE += (int) ( m_fSUM_SKILL_ATTACKRANGE
    //                      + m_sSUM_PASSIVE.m_fSUM_SKILL_ATTACKRANGE );
    // if ( nRANGE <= 0 ) nRANGE = 1;
    // return (WORD)nRANGE;
    //
    // The caller is responsible for:
    //   - obtaining baseTargetRange (sSKILL.m_sBASIC.wTARRANGE)
    //   - resolving isPhysicalLongRange (emAPPLY == SKILL::EMAPPLY_PHY_LONG)
    //   - obtaining targetRangeBonus (GETSUM_TARRANGE())
    //   - resolving affectsEnemy (emIMPACT_SIDE == SIDE_ENEMY)
    //   - obtaining skillAttackRangeBonus
    //     (m_fSUM_SKILL_ATTACKRANGE + m_sSUM_PASSIVE.m_fSUM_SKILL_ATTACKRANGE)
    //
    // The portable function does NOT generate randomness. It only performs
    // the deterministic range arithmetic, minimum/clamping, and final
    // GameUInt16 conversion. The caller resolves all skill/object state.
    // ---------------------------------------------------------------------------
    GameUInt16 SkillTargetRange(
        GameInt32 baseTargetRange,
        bool isPhysicalLongRange,
        GameInt32 targetRangeBonus,
        bool affectsEnemy,
        float skillAttackRangeBonus);

    // ---------------------------------------------------------------------------
    // SkillApplyRange — skill apply range arithmetic.
    //
    // Legacy: GLCHARLOGIC::GETSKILLRANGE_APPLY (GLogixExPC.cpp:4503)
    //
    // int nRANGE = sDATA_LVL.wAPPLYRANGE;
    // if ( sSKILL.m_sBASIC.emAPPLY==SKILL::EMAPPLY_PHY_LONG )
    //     nRANGE += (int) GETSUM_TARRANGE() + 5;
    // if ( sSKILL.m_sBASIC.emIMPACT_SIDE == SIDE_ENEMY )
    //     nRANGE += (int) ( m_fSUM_SKILL_APPLYRANGE
    //                      + m_sSUM_PASSIVE.m_fSUM_SKILL_APPLYRANGE );
    // if ( nRANGE <= 0 ) nRANGE = 1;
    // return (WORD)nRANGE;
    //
    // NOTE: The target-range function additionally clamps the base to a
    // minimum of 20. The apply-range function does NOT have that minimum.
    // This is preserved exactly.
    //
    // The caller is responsible for:
    //   - obtaining baseApplyRange (sDATA_LVL.wAPPLYRANGE)
    //   - resolving isPhysicalLongRange (emAPPLY == SKILL::EMAPPLY_PHY_LONG)
    //   - obtaining targetRangeBonus (GETSUM_TARRANGE())
    //   - resolving affectsEnemy (emIMPACT_SIDE == SIDE_ENEMY)
    //   - obtaining skillApplyRangeBonus
    //     (m_fSUM_SKILL_APPLYRANGE + m_sSUM_PASSIVE.m_fSUM_SKILL_APPLYRANGE)
    //
    // The portable function does NOT generate randomness. It only performs
    // the deterministic range arithmetic, minimum/clamping, and final
    // GameUInt16 conversion. The caller resolves all skill/object state.
    // ---------------------------------------------------------------------------
    GameUInt16 SkillApplyRange(
        GameInt32 baseApplyRange,
        bool isPhysicalLongRange,
        GameInt32 targetRangeBonus,
        bool affectsEnemy,
        float skillApplyRangeBonus);
}

// Convenience aliases in the global namespace for minimal friction
// during incremental adoption.
using GameBright       = GameCharacterCalculations::GameBright;
using GameBrightFB     = GameCharacterCalculations::GameBrightFB;