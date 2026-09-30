# VERTICAL-007: Combat Equipment & State Integration — Investigation

## 1. Legacy Source Locations

| File | Lines | What it contains |
|------|-------|-----------------|
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 570-571 | `m_sSUMITEM.fIncR_Critical += sItem.sSuitOp.sVARIATE[svaron].fVariate` — item critical rate from suit variation |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 573-574 | `m_sSUMITEM.fIncR_CrushingBlow += sItem.sSuitOp.sVARIATE[svaron].fVariate` — item crushing blow from suit variation |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 595 | `m_sSUMITEM.fIncR_Critical += sItemCustom.GETOptVALUE(EMR_OPT_BLOW_RATE)` — item critical rate from item option |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 596 | `m_sSUMITEM.fIncR_CrushingBlow += sItemCustom.GETOptVALUE(EMR_OPT_STRIKE_RATE)` — item crushing blow from item option |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 624-625 | `m_sSUMITEM.fInc_Critical += sItem.sSuitOp.sVOLUME.fVolume` — item critical rate from volume |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 627-628 | `m_sSUMITEM.fInc_CrushingBlow += sItem.sSuitOp.sVOLUME.fVolume` — item crushing blow from volume |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 1399 | `nCrushingBlow = (int)(m_sSUMITEM.fIncR_CrushingBlow * 100)` — crushing blow conversion to percentage points |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 1499 | `nCrushingBlow += (int)(sSKILL_SPEC.fVAR2 * 100)` — passive crushing blow from skill spec |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 1620 | `nPercentCri += (int)(m_sSUMITEM.fIncR_Critical * 100)` — critical rate conversion to percentage points |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 1409 | `fDamageReduce = sDamageSpec.m_fPsyDamageReduce` — damage reduction from DAMAGE_SPEC |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 1410-1411 | `fDamageReflection = sDamageSpec.m_fPsyDamageReflection` and `fDamageReflectionRate = sDamageSpec.m_fPsyDamageReflectionRate` — damage reflection from DAMAGE_SPEC |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 372 | `m_nDEFENSE_BODY = int(m_wSUM_DP + m_sSUMSTATS.wDex*cCHARCONST.fDEFENSE_DEX)` — body defense calculation |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 376 | `m_nDEFENSE_SKILL = m_nDEFENSE = int(m_nDEFENSE_BODY + m_sSUMITEM.nDefense + m_sSUM_PASSIVE.m_nDEFENSE + m_dwDefenseIncrease)` — total defense |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 657 | `m_sSUMITEM.nDefense += sItemCustom.GETDEFENSE()` — item defense aggregation |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 1703-1713 | Defense absorption formula using `nDEFAULT_DEFENSE` (body) and `nITEM_DEFENSE` (item) |
| `legacy/Lib_Client/G-Logic/GLChar.cpp` | 3446 | `bLowSP = (float(m_sSP.dwNow) < float(m_wSUM_DisSP)) ? TRUE : FALSE` — low-SP detection |
| `legacy/Lib_Client/G-Logic/GLChar.cpp` | 4811 | `bLowSP = (emCHECK==EMSKILL_NOTSP) ? TRUE : FALSE` — low-SP from skill check |
| `legacy/Lib_Client/G-Logic/GLogicEx.h` | 104 | `SSUM_ITEM` struct — aggregated item combat stats |
| `legacy/Lib_Client/G-Logic/GLogicEx.h` | 134 | `float fIncR_CrushingBlow` — crushing blow rate from items |
| `legacy/Lib_Client/G-Logic/GLogicEx.h` | 139 | `float fInc_CrushingBlow` — crushing blow flat bonus from items |
| `legacy/Lib_Client/G-Logic/GLItemDef.h` | 546 | `EMVAR_CRUSHING_BLOW` — item variation type for crushing blow |

## 2. Public Research

- RaGEZONE RAN configuration discussion: combat constants such as low-SP hit reduction, low-SP damage reduction, damage absorption, critical and crushing-blow configuration.
- RaGEZONE discussion of RAN critical/damage randomization and server/client random sources.
- Public RAN source repositories: `tablangdelio/ranOnline`, `yexiuph/RanOnline`.

Public material used as supporting evidence only. The repository's actual legacy implementation has priority.

## 3. Item Combat Fields

### Critical Rate

| Field | Legacy Source | Legacy Type | Modern Type | Aggregation |
|-------|---------------|-------------|-------------|-------------|
| `criticalRate` | `m_sSUMITEM.fIncR_Critical` | `float` | `float` | Sum of all equipped items |

**Legacy usage:**
```
nPercentCri += (int)(m_sSUMITEM.fIncR_Critical * 100);
```
(GLogixExPC.cpp:1620)

The item's critical rate is a float (e.g., 0.05 = 5%). It is converted to percentage points by multiplying by 100 and adding to the base critical rate.

### Crushing Blow

| Field | Legacy Source | Legacy Type | Modern Type | Aggregation |
|-------|---------------|-------------|-------------|-------------|
| `crushingBlow` | `m_sSUMITEM.fIncR_CrushingBlow` | `float` | `float` | Sum of all equipped items |

**Legacy usage:**
```
nCrushingBlow = (int)(m_sSUMITEM.fIncR_CrushingBlow * 100);
```
(GLogixExPC.cpp:1399)

Same pattern as critical rate — float converted to percentage points.

### Damage Reduction

| Field | Legacy Source | Legacy Type | Modern Type | Aggregation |
|-------|---------------|-------------|-------------|-------------|
| `damageReduce` | `sDamageSpec.m_fPsyDamageReduce` | `float` | `float` | Sum of all equipped items |

**Legacy usage:**
```
if (fDamageReduce > 0.0f) {
    int nDamageReduce = GameCharacterCalculations::DamageReduceAmount(
        rResultDAMAGE, fDamageReduce, nLEVEL, GLCONST_CHAR::wMAX_LEVEL);
    rResultDAMAGE -= nDamageReduce;
}
```
(GLogixExPC.cpp:1734-1742)

### Damage Reflection

| Field | Legacy Source | Legacy Type | Modern Type | Aggregation |
|-------|---------------|-------------|-------------|-------------|
| `damageReflection` | `sDamageSpec.m_fPsyDamageReflection` | `float` | `float` | Sum of all equipped items |
| `damageReflectionRate` | `sDamageSpec.m_fPsyDamageReflectionRate` | `float` | `float` | Sum of all equipped items |

**Legacy usage:**
```
if (fDamageReflectionRate > 0.0f) {
    if (fDamageReflectionRate > (RANDOM_POS*1)) {
        int nDamageReflection = GameCharacterCalculations::DamageReflectionAmount(
            rResultDAMAGE, fDamageReflection, nLEVEL, GLCONST_CHAR::wMAX_LEVEL);
        // Apply reflection damage back to attacker
    }
}
```
(GLogixExPC.cpp:1746-1759)

### Item Defense

| Field | Legacy Source | Legacy Type | Modern Type | Aggregation |
|-------|---------------|-------------|-------------|-------------|
| `defense` | `m_sSUMITEM.nDefense` | `int` | `int32_t` | Sum of all equipped items |

**Legacy usage:**
```
m_nDEFENSE = m_nDEFENSE_BODY + m_sSUMITEM.nDefense + m_sSUM_PASSIVE.m_nDEFENSE + m_dwDefenseIncrease;
```
(GLogixExPC.cpp:376)

Item defense is a separate component from body defense. The total defense is used for direct subtraction, while body and item defense are used separately for defense absorption.

## 4. Passive Combat Fields

| Field | Legacy Source | Legacy Type | Modern Type | Aggregation |
|-------|---------------|-------------|-------------|-------------|
| `criticalRate` | `sSKILL_SPEC` critical rate | `float` | `float` | Sum of all learned passives |
| `crushingBlow` | `sSKILL_SPEC.fVAR2` (EMSPECA_CRUSHING_BLOW) | `float` | `float` | Sum of all learned passives |
| `damageReduce` | `sDamageSpec.m_fPsyDamageReduce` | `float` | `float` | Sum of all learned passives |
| `damageReflection` | `sDamageSpec.m_fPsyDamageReflection` | `float` | `float` | Sum of all learned passives |
| `damageReflectionRate` | `sDamageSpec.m_fPsyDamageReflectionRate` | `float` | `float` | Sum of all learned passives |

**Legacy usage for crushing blow from passives:**
```
nCrushingBlow += (int)(sSKILL_SPEC.fVAR2 * 100);
```
(GLogixExPC.cpp:1499)

## 5. Low-SP Condition

### Legacy

```
bLowSP = (float(m_sSP.dwNow) < float(m_wSUM_DisSP)) ? TRUE : FALSE;
```
(GLCharacter.cpp:3446)

Or:
```
bLowSP = (emCHECK==EMSKILL_NOTSP) ? TRUE : FALSE;
```
(GLChar.cpp:4811)

Low-SP is triggered when:
1. Current SP is below the required SP for a skill (`m_wSUM_DisSP`)
2. A skill is used without enough SP (`EMSKILL_NOTSP`)

### Modern

**LIMITED**: Without a skill system, we use `currentSP == 0` as a proxy for low-SP state. This is documented as LIMITED per Section 21 of the VERTICAL-007 specification.

The combat engine receives `targetLowSP` as an explicit input from the server, which determines the state from character SP.

## 6. Critical Rate

### Legacy

```
// Base critical rate from character HP and level difference
int nPercentCri = GameCharacterCalculations::CriticalBaseRate(GETHP(), GETMAXHP(), GETLEVEL(), nLEVEL);

// Add item critical rate (converted to percentage points)
nPercentCri += (int)(m_sSUMITEM.fIncR_Critical * 100);

// Clamp to maximum
if (nPercentCri > (int)GLCONST_CHAR::dwCRITICAL_MAX)
    nPercentCri = GLCONST_CHAR::dwCRITICAL_MAX;

// Roll for critical
if (nPercentCri > (RANDOM_POS*100))
    bCritical = true;
```

### Modern

The modern implementation follows the same pattern:
1. Base critical rate from `CriticalBaseRate()`
2. Add item critical rate: `nPercentCri += (int)(criticalRate * 100)`
3. Clamp to `criticalMax`
4. Roll against `criticalRoll * 100`

## 7. Crushing Blow

### Legacy

```
// Item crushing blow (converted to percentage points)
int nCrushingBlow = (int)(m_sSUMITEM.fIncR_CrushingBlow * 100);

// Add passive crushing blow
nCrushingBlow += (int)(sSKILL_SPEC.fVAR2 * 100);

// Clamp to maximum
if (nCrushingBlow > GLCONST_CHAR::dwCRUSHING_BLOW_MAX)
    nCrushingBlow = GLCONST_CHAR::dwCRUSHING_BLOW_MAX;

// Roll for crushing blow
if (nCrushingBlow > (RANDOM_POS*100))
    bCrushingBlow = true;
```

### Modern

The modern implementation follows the same pattern:
1. Item crushing blow: `nCrushingBlow = (int)(crushingBlow * 100)`
2. Clamp to `crushingBlowMax`
3. Roll against `crushingRoll * 100`

## 8. Item Defense

### Legacy

```
// Body defense (from stats)
m_nDEFENSE_BODY = int(m_wSUM_DP + m_sSUMSTATS.wDex*cCHARCONST.fDEFENSE_DEX);

// Total defense (body + items + passives + codex)
m_nDEFENSE_SKILL = m_nDEFENSE = int(m_nDEFENSE_BODY + m_sSUMITEM.nDefense + m_sSUM_PASSIVE.m_nDEFENSE + m_dwDefenseIncrease);

// In combat:
// Direct subtraction uses total defense (m_nDEFENSE)
int nNetDAMAGE = int((float)nDAMAGE_OLD * defenseUsed - (float)nDEFENSE);

// Defense absorption uses body and item defense separately
float fFinalRate = float(nDEFAULT_DEFENSE * nITEM_DEFENSE) * fDecRate;
```

### Modern

- `targetDefense` = total defense (`DerivedStats::defense`) — used for direct subtraction
- `targetDefenseBody` = body defense (`DerivedStats::defenseBody`) — used for absorption
- `targetDefenseItem` = item defense (`ItemContribution::defense`) — used for absorption

## 9. Damage Reduction

### Legacy

```
if (fDamageReduce > 0.0f) {
    int nDamageReduce = GameCharacterCalculations::DamageReduceAmount(
        rResultDAMAGE, fDamageReduce, nLEVEL, GLCONST_CHAR::wMAX_LEVEL);
    rResultDAMAGE -= nDamageReduce;
}
```

Where `DamageReduceAmount` is:
```
int nDamageReduce = (int)(((rResultDAMAGE * fDamageReduce) * nLEVEL) / GLCONST_CHAR::wMAX_LEVEL);
```

### Modern

The modern implementation uses the same formula via `GameCharacterCalculations::DamageReduceAmount()`.

## 10. Damage Reflection

### Legacy

```
if (fDamageReflectionRate > 0.0f) {
    if (fDamageReflectionRate > (RANDOM_POS*1)) {
        int nDamageReflection = GameCharacterCalculations::DamageReflectionAmount(
            rResultDAMAGE, fDamageReflection, nLEVEL, GLCONST_CHAR::wMAX_LEVEL);
        // Apply reflection damage back to attacker
    }
}
```

### Modern

**LIMITED**: The modern implementation has the reflection fields in `CombatInput` but does not implement the reflection damage application. This requires a combat event system that is deferred.

## 11. Modern Implementation Mapping

| Legacy | Modern |
|--------|--------|
| `m_sSUMITEM.fIncR_Critical` | `ItemContribution::criticalRate` |
| `m_sSUMITEM.fIncR_CrushingBlow` | `ItemContribution::crushingBlow` |
| `sDamageSpec.m_fPsyDamageReduce` | `ItemContribution::damageReduce` |
| `sDamageSpec.m_fPsyDamageReflection` | `ItemContribution::damageReflection` |
| `sDamageSpec.m_fPsyDamageReflectionRate` | `ItemContribution::damageReflectionRate` |
| `m_sSUMITEM.nDefense` | `ItemContribution::defense` |
| `m_nDEFENSE_BODY` | `DerivedStats::defenseBody` |
| `m_nDEFENSE` | `DerivedStats::defense` |
| `bLowSP` | `CombatInput::targetLowSP` (from server) |

## 12. Aggregation Rules

### Item Contribution

All combat fields are summed across all equipped items in slot order:
- `criticalRate`: sum of all items' `criticalRate`
- `crushingBlow`: sum of all items' `crushingBlow`
- `damageReduce`: sum of all items' `damageReduce`
- `damageReflection`: sum of all items' `damageReflection`
- `damageReflectionRate`: sum of all items' `damageReflectionRate`
- `defense`: sum of all items' `defense`

### Passive Contribution

All combat fields are summed across all learned passive skills:
- `criticalRate`: sum of all passives' `criticalRate`
- `crushingBlow`: sum of all passives' `crushingBlow`
- `damageReduce`: sum of all passives' `damageReduce`
- `damageReflection`: sum of all passives' `damageReflection`
- `damageReflectionRate`: sum of all passives' `damageReflectionRate`

### Derived Stats

The final combat state is the sum of item and passive contributions:
- `criticalRate = items.criticalRate + passives.criticalRate`
- `crushingBlow = items.crushingBlow + passives.crushingBlow`
- `damageReduce = items.damageReduce + passives.damageReduce`
- `damageReflection = items.damageReflection + passives.damageReflection`
- `damageReflectionRate = items.damageReflectionRate + passives.damageReflectionRate`

## 13. Test Coverage

### Core Tests (CombatTests.cpp)

- No combat item bonus
- Single critical-rate item
- Multiple critical-rate items
- Crushing blow item
- Multiple crushing items
- Item defense
- Body defense vs item defense
- Damage reduction contribution
- Damage reflection contribution
- Equipment removal
- Equipment replacement
- Empty equipment
- Missing item definition
- Non-finite item values

### Server Tests (ServerCharacterTests.cpp)

- Equip item → attack → combat uses item modifier
- Unequip item → attack → modifier disappears
- Equipment change → derived/combat state recalculated → next attack uses new state

### Client Tests (ClientGameplayTests.cpp)

- Client receives authoritative results after equipment changes
- Client does not calculate the bonus itself

## 14. Build and Test

### Build

```powershell
cmake --build build-debug --config Debug
cmake --build build-release --config Release
```

### Test

```powershell
ctest --test-dir build-debug --output-on-failure
ctest --test-dir build-release --output-on-failure
```

## 15. Runtime

- Emulator: NOT REQUIRED
- MiniA: NOT USED
- Production client: NOT USED
- Production server: NOT USED

## 16. Limitations

| Behavior | Status | Notes |
|----------|--------|-------|
| Low-SP detection | LIMITED | Uses `currentSP == 0` as proxy; legacy uses SP < required SP for skill |
| Damage reflection application | LIMITED | Fields exist but reflection damage requires combat event system |
| Elemental resistance | LIMITED | Not connected to physical combat (legacy uses `fRESIST_PHYSIC_G` for physical) |
| State damage | LIMITED | Set to 1.0f; no state damage yet |
| Brightness/environment | DEFERRED | Hardcoded to Aver |
| Magic combat | DEFERRED | Not part of VERTICAL-007 |
| Ranged combat | DEFERRED | Melee only |
