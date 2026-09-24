// GameCharacterCalculations.cpp
//
// Portable character combat calculation implementations.
//
// These are deterministic pure calculations extracted from GLOGICEX
// (Lib_Client/G-Logic/GLogicEx.cpp). The original formulas are preserved
// byte-for-byte / behavior-for-behavior. No gameplay constants, rounding,
// integer widths, or signedness have been changed.

#include "GameCharacterCalculations.h"
#include "GameRandom.h"

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
    //
    // The brightness modifier is the caller's already-resolved value from
    // the legacy EM_BRIGHT_FB table { -10, 0, 10 }. It is passed as a plain
    // GameInt32 so the portable layer does NOT depend on EM_BRIGHT_FB or
    // GLSPACEGAP. The caller computes the modifier via SpaceGap() and
    // passes the resolved integer.
    // ---------------------------------------------------------------------------
    GameInt32 HitRate(
        GameInt32 nHit,
        GameInt32 nAvoid,
        GameInt32 brightnessModifier)
    {
        const int nBirght[3] = { -10, 0, 10 };
        enum { MAX_HIT = 99, MIN_HIT = 20, BASIC = 100 };

        int hitRate;
        hitRate = BASIC + nHit - nAvoid + nBirght[static_cast<int>(brightnessModifier)];

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
    //
    // The environment factor is the caller's already-resolved value from
    // the legacy EM_BRIGHT_FB table { 0.8f, 1.0f, 1.2f }. It is passed as a
    // plain float so the portable layer does NOT depend on EM_BRIGHT_FB,
    // EMBRIGHT, or GLSPACEGAP. The caller computes the factor via
    // SpaceGap() and passes the resolved float.
    // ---------------------------------------------------------------------------
    GameInt32 Defense(
        GameInt32 defense,
        float environmentFactor)
    {
        return static_cast<GameInt32>(
            static_cast<float>(defense) * environmentFactor);
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
    // Stage 2E — RNG-dependent character calculations.
    //
    // These accept a caller-supplied normalized random value in [0.0, 1.0].
    // The portable layer does NOT generate randomness; the legacy layer
    // supplies it from RANDOM_POS, seqrandom::getpercent(), or any future
    // RNG source. See GameRandom.h for the probability primitives consumed.
    // ========================================================================

    // ---------------------------------------------------------------------------
    // CheckShock
    //
    // Original: GLOGICEX::CHECKSHOCK
    //
    // enum { CLEANHIT_RATE = 1, CRITICALHIT_RATE = 5, MIN_DXLEVEL = 5, MIN_DAMAGE = 6 };
    // int nDXLEV = nDEFLEV - nACTLEV;
    // if ( (-MIN_DXLEVEL) > nDXLEV )   return FALSE;
    // if ( (nACTLEV+MIN_DAMAGE) > nDamage ) return FALSE;
    // if ( bCritical ) return (CRITICALHIT_RATE > (RANDOM_POS*100));
    // return (CLEANHIT_RATE > (RANDOM_POS*100));
    //
    // The probability check (rate > randomValue*100) is delegated to
    // GameRandom::CheckProbability, which is defined as
    // (randomValue * 100.0f) < rate — identical semantics.
    // ---------------------------------------------------------------------------
    bool CheckShock(
        int nACTLEV,
        int nDEFLEV,
        int nDamage,
        bool bCritical,
        float randomValue)
    {
        enum { CLEANHIT_RATE = 1, CRITICALHIT_RATE = 5, MIN_DXLEVEL = 5, MIN_DAMAGE = 6 };

        int nDXLEV = nDEFLEV - nACTLEV;
        if ( (-MIN_DXLEVEL) > nDXLEV )   return false;
        if ( (nACTLEV+MIN_DAMAGE) > nDamage ) return false;

        if ( bCritical )
            return GameRandom::CheckProbability(CRITICALHIT_RATE, randomValue);
        return GameRandom::CheckProbability(CLEANHIT_RATE, randomValue);
    }

    // ---------------------------------------------------------------------------
    // CheckStateBlow
    //
    // Original: GLOGICEX::CHECKSTATEBLOW
    //
    // int nDXLEVEL = int(wLEVEL - wACTLEVEL);
    // int nINDEX = nDXLEVEL + nStateBlowLevelBase;
    // if (nINDEX < 0) nINDEX = 0;
    // if (nINDEX >= nStateBlowLevelSize) nINDEX = nStateBlowLevelSize-1;
    // return (RANDOM_POS*100.0f) < (fACTRATE - fACTRATE * 0.01f * wRESIST * 0.6f
    //                                + nStateBlowLevel[nINDEX]);
    //
    // The probability check (randomValue*100 < threshold) is delegated to
    // GameRandom::CheckProbability, which is defined as
    // (randomValue * 100.0f) < threshold — identical semantics.
    // ---------------------------------------------------------------------------
    bool CheckStateBlow(
        float fACTRATE,
        GameUInt16 wACTLEVEL,
        GameUInt16 wLEVEL,
        GameUInt16 wRESIST,
        const int* pStateBlowLevel,
        int nStateBlowLevelBase,
        int nStateBlowLevelSize,
        float randomValue)
    {
        int nDXLEVEL = static_cast<int>(wLEVEL - wACTLEVEL);

        int nINDEX = nDXLEVEL + nStateBlowLevelBase;
        if ( nINDEX < 0 )                                         nINDEX = 0;
        if ( nINDEX >= nStateBlowLevelSize )                       nINDEX = nStateBlowLevelSize - 1;

        float fThreshold = fACTRATE - fACTRATE * 0.01f * wRESIST * 0.6f
                           + pStateBlowLevel[nINDEX];

        return GameRandom::CheckProbability(fThreshold, randomValue);
    }

    // ---------------------------------------------------------------------------
    // MoveVelocity
    //
    // Legacy: GLCharacter::GetMoveVelo(), GLChar::GetMoveVelo(),
    //         GLCrow::GetMoveVelo()
    //
    // float fDefaultVelo = IsSTATE(EM_ACT_RUN) ? fRUNVELO : fWALKVELO;
    // float fMoveVelo = fDefaultVelo * ( GETMOVEVELO() + GETMOVE_ITEM() );
    // return fMoveVelo;
    //
    // NOTE: GLCrow omits the item-move-velocity term (no equipment), so its
    // itemMoveVelocity is 0.0f. The formula is otherwise identical.
    // ---------------------------------------------------------------------------
    float MoveVelocity(
        float baseVelocity,
        float sumMoveVelocity,
        float itemMoveVelocity,
        bool isRunMode)
    {
        // isRunMode is accepted for symmetry with the legacy callers but is
        // NOT used in the formula — the base velocity is already resolved to
        // run or walk by the caller before calling this function.
        (void)isRunMode;

        return baseVelocity * (sumMoveVelocity + itemMoveVelocity);
    }

    // ---------------------------------------------------------------------------
    // IsLowerHP
    //
    // Legacy: GLCrow::IsLowerHP()
    //
    // float fRate = 0.0f;
    // for( int i = 0; i < m_vecPatternList.size(); i++ )
    // {
    //     fRate = m_vecPatternList[i].m_fPatternDNRate;
    //     if( GETHP() >= (int)( (float)GETMAXHP() * fRate ) / 100.0f )
    //         return i;
    // }
    // return -1;
    //
    // CRITICAL: The (int) cast truncates the product BEFORE division by 100.0f.
    // This is preserved exactly — do NOT move the cast after the division.
    // ---------------------------------------------------------------------------
    int IsLowerHP(
        GameUInt32 currentHP,
        GameUInt32 maxHP,
        const float* patternRates,
        GameSizeT patternCount)
    {
        for (GameSizeT i = 0; i < patternCount; ++i)
        {
            float fRate = patternRates[i];
            if (currentHP >= (int)((float)maxHP * fRate) / 100.0f)
            {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    // ---------------------------------------------------------------------------
    // AttackVelocity
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
    // The portable function does NOT generate randomness. It only performs
    // the deterministic floating-point addition and negative clamp.
    // The caller supplies the item attack velocity rate (0.0f for NPC).
    // ---------------------------------------------------------------------------
    float AttackVelocity(
        float baseAttackVelocity,
        float itemAttackVelocityRate)
    {
        const float result = baseAttackVelocity + itemAttackVelocityRate;
        return result < 0.0f ? 0.0f : result;
    }

    // ========================================================================
    // Stage 2F-3 — Weather power calculations.
    //
    // Minimal portable element/state-blow enums. Numeric values match the
    // legacy EMELEMENT and EMSTATE_BLOW enums exactly (verified from
    // Lib_Client/G-Logic/GLCharDefine.h). The portable layer does NOT
    // import GLCharDefine.h.
    //
    // Weather flag constants (verified from GLPeriod.h:56-68 FGWEATHER):
    //   FGW_RAIN   = 0x0001
    //   FGW_SNOW   = 0x0002
    //   FGW_LEAVES = 0x0004
    // These are re-defined locally so the portable module does NOT need
    // to include GLPeriod.h (which pulls in CTime, D3DXVector2, etc.).
    // ========================================================================

    namespace
    {
        constexpr GameUInt32 kWeatherRain   = 0x0001u;
        constexpr GameUInt32 kWeatherSnow   = 0x0002u;
        constexpr GameUInt32 kWeatherLeaves = 0x0004u;
    }

    // ---------------------------------------------------------------------------
    // StateBlowToElement
    //
    // Legacy: STATE_TO_ELEMENT(emBlow) inline switch in GLCharDefine.h:942-957
    //
    // case EMBLOW_NUMB:   return EMELEMENT_ELECTRIC;
    // case EMBLOW_STUN:   return EMELEMENT_STUN;
    // case EMBLOW_STONE:  return EMELEMENT_STONE;
    // case EMBLOW_BURN:   return EMELEMENT_FIRE;
    // case EMBLOW_FROZEN: return EMELEMENT_ICE;
    // case EMBLOW_MAD:    return EMELEMENT_MAD;
    // case EMBLOW_POISON: return EMELEMENT_POISON;
    // case EMBLOW_CURSE:  return EMELEMENT_CURSE;
    // default:            return EMELEMENT_SPIRIT;
    // ---------------------------------------------------------------------------
    GameElement StateBlowToElement(GameStateBlow blow)
    {
        switch (blow)
        {
        case GameStateBlow::Numb:   return GameElement::Electric;
        case GameStateBlow::Stun:   return GameElement::Stun;
        case GameStateBlow::Stone:  return GameElement::Stone;
        case GameStateBlow::Burn:   return GameElement::Fire;
        case GameStateBlow::Frozen: return GameElement::Ice;
        case GameStateBlow::Mad:    return GameElement::Mad;
        case GameStateBlow::Poison: return GameElement::Poison;
        case GameStateBlow::Curse:  return GameElement::Curse;
        default:                    return GameElement::Spirit;
        }
    }

    // ---------------------------------------------------------------------------
    // WeatherElementPower
    //
    // Legacy: GLOGICEX::WEATHER_ELEMENT_POW
    //
    // float fPOWER = 1.0f;
    // if ( !bWeatherActive ) return fPOWER;
    // switch ( emElement )
    // {
    // case EMELEMENT_FIRE:
    // case EMELEMENT_STONE:
    //     if ( dwWeather&FGW_RAIN )   fPOWER = 0.8f;
    //     if ( dwWeather&FGW_LEAVES ) fPOWER = 1.2f;
    //     break;
    // case EMELEMENT_ICE:
    // case EMELEMENT_MAD:
    //     if ( dwWeather&FGW_LEAVES ) fPOWER = 0.8f;
    //     if ( dwWeather&FGW_SNOW )   fPOWER = 1.2f;
    //     break;
    // case EMELEMENT_ELECTRIC:
    // case EMELEMENT_STUN:
    //     if ( dwWeather&FGW_SNOW ) fPOWER = 0.8f;
    //     if ( dwWeather&FGW_RAIN ) fPOWER = 1.2f;
    //     break;
    // case EMELEMENT_POISON:
    // case EMELEMENT_CURSE:
    //     fPOWER = 1.0f;
    //     break;
    // };
    // return fPOWER;
    //
    // NOTE: The weather flags are passed as a raw GameUInt32 bitmask from
    // the caller. The portable function tests specific bit positions via
    // the locally-defined constants (matching GLPeriod.h FGWEATHER values)
    // and does NOT include GLPeriod.h.
    // ---------------------------------------------------------------------------
    float WeatherElementPower(
        GameElement element,
        GameUInt32 weatherFlags,
        bool weatherActive)
    {
        float fPOWER = 1.0f;
        if ( !weatherActive ) return fPOWER;

        switch ( element )
        {
        case GameElement::Fire:
        case GameElement::Stone:
            if ( weatherFlags & kWeatherRain )   fPOWER = 0.8f;
            if ( weatherFlags & kWeatherLeaves ) fPOWER = 1.2f;
            break;

        case GameElement::Ice:
        case GameElement::Mad:
            if ( weatherFlags & kWeatherLeaves ) fPOWER = 0.8f;
            if ( weatherFlags & kWeatherSnow )   fPOWER = 1.2f;
            break;

        case GameElement::Electric:
        case GameElement::Stun:
            if ( weatherFlags & kWeatherSnow )   fPOWER = 0.8f;
            if ( weatherFlags & kWeatherRain )   fPOWER = 1.2f;
            break;

        case GameElement::Poison:
        case GameElement::Curse:
            fPOWER = 1.0f;
            break;
        };

        return fPOWER;
    }

    // ---------------------------------------------------------------------------
    // WeatherBlowPower
    //
    // Legacy: GLOGICEX::WEATHER_BLOW_POW
    //
    // float fPOWER = 1.0f;
    // if ( !bWeatherActive ) return fPOWER;
    // EMELEMENT emElement = STATE_TO_ELEMENT ( emBlow );
    // return WEATHER_ELEMENT_POW ( emElement, dwWeather, bWeatherActive );
    // ---------------------------------------------------------------------------
    float WeatherBlowPower(
        GameStateBlow blow,
        GameUInt32 weatherFlags,
        bool weatherActive)
    {
        float fPOWER = 1.0f;
        if ( !weatherActive ) return fPOWER;

        GameElement emElement = StateBlowToElement(blow);
        return WeatherElementPower(emElement, weatherFlags, weatherActive);
    }

    // ---------------------------------------------------------------------------
    // DamageReduceAmount
    //
    // Legacy: GLogixExPC.cpp:1728 and GLogicExNPC.cpp:304
    //
    // int nDamageReduce = (int) (
    //     ( (rResultDAMAGE * fDamageReduce) * nLEVEL )
    //     / GLCONST_CHAR::wMAX_LEVEL
    // );
    //
    // CRITICAL: The (int) cast truncates the floating-point result. The
    // multiplication order is preserved exactly: (damage * damageReduce)
    // * level / maxLevel. Do NOT reorder or "simplify".
    // ---------------------------------------------------------------------------
    GameInt32 DamageReduceAmount(
        GameInt32 damage,
        float damageReduce,
        GameInt32 level,
        GameInt32 maxLevel)
    {
        return static_cast<GameInt32>(
            (static_cast<float>(damage) * damageReduce * static_cast<float>(level))
            / static_cast<float>(maxLevel));
    }

    // ---------------------------------------------------------------------------
    // DamageReflectionAmount
    //
    // Legacy: GLogixExPC.cpp:1742 and GLogicExNPC.cpp:318
    //
    // int nDamageReflection = (int) (
    //     ( (rResultDAMAGE * fDamageReflection) * nLEVEL )
    //     / GLCONST_CHAR::wMAX_LEVEL
    // );
    //
    // CRITICAL: The (int) cast truncates the floating-point result. The
    // multiplication order is preserved exactly: (damage * damageReflection)
    // * level / maxLevel. Do NOT reorder or "simplify".
    // ---------------------------------------------------------------------------
    GameInt32 DamageReflectionAmount(
        GameInt32 damage,
        float damageReflection,
        GameInt32 level,
        GameInt32 maxLevel)
    {
        return static_cast<GameInt32>(
            (static_cast<float>(damage) * damageReflection * static_cast<float>(level))
            / static_cast<float>(maxLevel));
    }

    // ---------------------------------------------------------------------------
    // CriticalBaseRate
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
    // All arithmetic is integer-only. No floating point. No RNG.
    // ---------------------------------------------------------------------------
    GameInt32 CriticalBaseRate(
        GameInt32 currentHP,
        GameInt32 maxHP,
        GameInt32 attackerLevel,
        GameInt32 targetLevel)
    {
        GameInt32 levelDifference = targetLevel - attackerLevel;

        if ( levelDifference > 5 )
            levelDifference = 5;

        if ( levelDifference < -5 )
            levelDifference = -5;

        GameInt32 percentHP = (currentHP * 100) / maxHP;

        if ( percentHP <= 10 )
            percentHP = 10;

        return 1000 / percentHP - 10 + levelDifference;
    }

    // ---------------------------------------------------------------------------
    // RandomDamageRange
    //
    // Legacy: GLogixExPC.cpp:1668,1997 and GLogicExNPC.cpp:283
    //
    // int nDAMAGE_NOW = int(gdDamage.dwLow +
    //     (gdDamage.dwHigh - gdDamage.dwLow) * RANDOM_POS);
    //
    // The portable function does NOT generate randomness. It only performs
    // the deterministic floating-point interpolation and integer truncation.
    // The caller supplies the random value (RANDOM_POS, 0.0-1.0).
    // ---------------------------------------------------------------------------
    GameInt32 RandomDamageRange(
        GameUInt32 lowDamage,
        GameUInt32 highDamage,
        float randomValue)
    {
        return static_cast<GameInt32>(
            static_cast<float>(lowDamage) +
            (static_cast<float>(highDamage) -
             static_cast<float>(lowDamage)) * randomValue);
    }

    // ---------------------------------------------------------------------------
    // ApplyStateDamage
    //
    // Legacy: GLogixExPC.cpp:1684,2008 and GLogicExNPC.cpp:294
    //
    // rResultDAMAGE = int(rResultDAMAGE * fSTATE_DAMAGE);
    //
    // The portable function does NOT generate randomness. It only performs
    // the deterministic floating-point multiplication and integer truncation.
    // The caller supplies the state damage multiplier (fSTATE_DAMAGE).
    // ---------------------------------------------------------------------------
    GameInt32 ApplyStateDamage(
        GameInt32 damage,
        float stateDamage)
    {
        return static_cast<GameInt32>(
            static_cast<float>(damage) * stateDamage);
    }

    // ---------------------------------------------------------------------------
    // ApplyDamageRate
    //
    // Legacy: GLogixExPC.cpp:1598-1599,1954-1955,3002-3003 and
    //         GLogicExNPC.cpp:257-258
    //
    // PC:  gdDamage.dwLow  = DWORD ( gdDamage.dwLow  * m_fDamageRate );
    //      gdDamage.dwHigh = DWORD ( gdDamage.dwHigh * m_fDamageRate );
    // NPC: gdDamage.wLow  = int ( gdDamage.wLow  * m_fDamageRate );
    //      gdDamage.wHigh = int ( gdDamage.wHigh * m_fDamageRate );
    //
    // The portable function does NOT generate randomness. It only performs
    // the deterministic floating-point multiplication and integer truncation.
    // The caller supplies the damage rate multiplier (m_fDamageRate).
    //
    // NOTE: The return type is GameUInt32 to safely represent both DWORD-sized
    // (PC) and WORD-sized (NPC) damage values. The NPC caller casts back to
    // WORD via static_cast<decltype(gdDamage.wLow)> to preserve the original
    // destination-field conversion behavior.
    // ---------------------------------------------------------------------------
    GameUInt32 ApplyDamageRate(
        GameUInt32 damage,
        float damageRate)
    {
        return static_cast<GameUInt32>(
            static_cast<float>(damage) * damageRate);
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

    // ---------------------------------------------------------------------------
    // SkillTargetRange
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
    // The portable function does NOT generate randomness. It only performs
    // the deterministic range arithmetic, minimum/clamping, and final
    // GameUInt16 conversion. The caller resolves all skill/object state.
    // ---------------------------------------------------------------------------
    GameUInt16 SkillTargetRange(
        GameInt32 baseTargetRange,
        bool isPhysicalLongRange,
        GameInt32 targetRangeBonus,
        bool affectsEnemy,
        float skillAttackRangeBonus)
    {
        int nRANGE = static_cast<int>(baseTargetRange);

        if ( nRANGE < 20 )
            nRANGE = 20;

        if ( isPhysicalLongRange )
            nRANGE += targetRangeBonus + 5;

        if ( affectsEnemy )
            nRANGE += static_cast<int>(skillAttackRangeBonus);

        if ( nRANGE <= 0 )
            nRANGE = 1;

        return static_cast<GameUInt16>(nRANGE);
    }

    // ---------------------------------------------------------------------------
    // SkillApplyRange
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
    // The portable function does NOT generate randomness. It only performs
    // the deterministic range arithmetic, minimum/clamping, and final
    // GameUInt16 conversion. The caller resolves all skill/object state.
    // ---------------------------------------------------------------------------
    GameUInt16 SkillApplyRange(
        GameInt32 baseApplyRange,
        bool isPhysicalLongRange,
        GameInt32 targetRangeBonus,
        bool affectsEnemy,
        float skillApplyRangeBonus)
    {
        int nRANGE = static_cast<int>(baseApplyRange);

        if ( isPhysicalLongRange )
            nRANGE += targetRangeBonus + 5;

        if ( affectsEnemy )
            nRANGE += static_cast<int>(skillApplyRangeBonus);

        if ( nRANGE <= 0 )
            nRANGE = 1;

        return static_cast<GameUInt16>(nRANGE);
    }
}