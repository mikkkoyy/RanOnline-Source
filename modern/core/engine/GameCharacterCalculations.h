#pragma once

// VERTICAL-006: minimal combat calculations for basic physical combat.
//
// This is a clean modern version focused on what VERTICAL-006 needs:
// Hit rate, defense, critical base rate, random damage range, state damage,
// damage rate, and damage reduction/reflection.

#include "GameTypes.h"
#include <cmath>

namespace Modern::Engine
{
    // ---------------------------------------------------------------------------
    // Hit rate calculation.
    //
    // Legacy: GLOGICEX::GLHITRATE
    //
    // Formula: BASIC(100) + nHit - nAvoid + brightnessModifier
    // Clamped to [MIN_HIT(20), MAX_HIT(99)].
    // ---------------------------------------------------------------------------
    inline GameInt32 HitRate(
        GameInt32 nHit,
        GameInt32 nAvoid,
        GameInt32 brightnessModifier)
    {
        const GameInt32 BASIC = 100;
        const GameInt32 MAX_HIT = 99;
        const GameInt32 MIN_HIT = 20;

        GameInt32 hitRate = BASIC + nHit - nAvoid + brightnessModifier;

        if (hitRate > MAX_HIT)
            hitRate = MAX_HIT;
        else if (hitRate < MIN_HIT)
            hitRate = MIN_HIT;

        return hitRate;
    }

    // ---------------------------------------------------------------------------
    // Defense calculation.
    //
    // Legacy: GLOGICEX::GLDEFENSE
    //
    // Formula: int(nDEFENSE * environmentFactor)
    // ---------------------------------------------------------------------------
    inline GameInt32 Defense(
        GameInt32 defense,
        float environmentFactor)
    {
        return static_cast<GameInt32>(
            static_cast<float>(defense) * environmentFactor);
    }

    // ---------------------------------------------------------------------------
    // Critical base rate.
    //
    // Legacy: GLogixExPC.cpp:1603-1613
    //
    // int ndxLvl = nLEVEL - GETLEVEL();
    // if ( ndxLvl > 5 )  ndxLvl = 5;
    // if ( ndxLvl < -5 ) ndxLvl = -5;
    // int nPerHP = ((GETHP()*100)/GETMAXHP());
    // if ( nPerHP <= 10 ) nPerHP = 10;
    // int nPercentCri = 1000 / nPerHP - 10 + ndxLvl;
    // ---------------------------------------------------------------------------
    inline GameInt32 CriticalBaseRate(
        GameInt32 currentHP,
        GameInt32 maxHP,
        GameInt32 attackerLevel,
        GameInt32 targetLevel)
    {
        GameInt32 levelDifference = targetLevel - attackerLevel;

        if (levelDifference > 5)
            levelDifference = 5;

        if (levelDifference < -5)
            levelDifference = -5;

        GameInt32 percentHP = (currentHP * 100) / maxHP;

        if (percentHP <= 10)
            percentHP = 10;

        return 1000 / percentHP - 10 + levelDifference;
    }

    // ---------------------------------------------------------------------------
    // Random damage range.
    //
    // Legacy: GLogixExPC.cpp:1668
    //
    // int nDAMAGE_NOW = int(gdDamage.dwLow +
    //     (gdDamage.dwHigh - gdDamage.dwLow) * RANDOM_POS);
    // ---------------------------------------------------------------------------
    inline GameInt32 RandomDamageRange(
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
    // Apply state damage.
    //
    // Legacy: GLogixExPC.cpp:1684
    //
    // rResultDAMAGE = int(rResultDAMAGE * fSTATE_DAMAGE);
    // ---------------------------------------------------------------------------
    inline GameInt32 ApplyStateDamage(
        GameInt32 damage,
        float stateDamage)
    {
        return static_cast<GameInt32>(
            static_cast<float>(damage) * stateDamage);
    }

    // ---------------------------------------------------------------------------
    // Apply damage rate.
    //
    // Legacy: GLogixExPC.cpp:1598-1599
    //
    // PC: gdDamage.dwLow = DWORD(gdDamage.dwLow * m_fDamageRate);
    // ---------------------------------------------------------------------------
    inline GameUInt32 ApplyDamageRate(
        GameUInt32 damage,
        float damageRate)
    {
        return static_cast<GameUInt32>(
            static_cast<float>(damage) * damageRate);
    }

    // ---------------------------------------------------------------------------
    // Damage reduce amount.
    //
    // Legacy: GLogixExPC.cpp:1728
    //
    // int nDamageReduce = (int) (
    //     ( (rResultDAMAGE * fDamageReduce) * nLEVEL )
    //     / GLCONST_CHAR::wMAX_LEVEL
    // );
    // ---------------------------------------------------------------------------
    inline GameInt32 DamageReduceAmount(
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
    // Damage reflection amount.
    //
    // Legacy: GLogixExPC.cpp:1742
    // ---------------------------------------------------------------------------
    inline GameInt32 DamageReflectionAmount(
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
    // Check shock (stun) probability.
    //
    // Legacy: GLOGICEX::CHECKSHOCK
    // ---------------------------------------------------------------------------
    inline bool CheckShock(
        int nACTLEV,
        int nDEFLEV,
        int nDamage,
        bool bCritical,
        float randomValue)
    {
        const int CLEANHIT_RATE = 1;
        const int CRITICALHIT_RATE = 5;
        const int MIN_DXLEVEL = 5;
        const int MIN_DAMAGE = 6;

        int nDXLEV = nDEFLEV - nACTLEV;
        if ((-MIN_DXLEVEL) > nDXLEV)
            return false;
        if ((nACTLEV + MIN_DAMAGE) > nDamage)
            return false;

        if (bCritical)
            return (randomValue * 100.0f) < CRITICALHIT_RATE;
        return (randomValue * 100.0f) < CLEANHIT_RATE;
    }

    // ---------------------------------------------------------------------------
    // Space gap (brightness relationship).
    //
    // Legacy: GLOGICEX::GLSPACEGAP
    // ---------------------------------------------------------------------------
    inline GameBrightFB SpaceGap(
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
}