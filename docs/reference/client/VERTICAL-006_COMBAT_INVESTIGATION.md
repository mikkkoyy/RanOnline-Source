# VERTICAL-006: Basic Physical Combat Resolution — Investigation

## 1. Legacy Source Locations

| File | Lines | What it contains |
|------|-------|-----------------|
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 1291 | `CHECKHIT` — hit/miss determination |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 1363 | `CALCDAMAGE_20060328` — physical damage calculation |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 1603-1613 | Critical base rate calculation |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 1668 | Random damage range interpolation |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 1684 | State damage application |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 1728 | Damage reduce amount |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 1742 | Damage reflection amount |
| `legacy/Lib_Client/G-Logic/GLogicEx.h` | 270 | `UPDATE_POINT` — recovery (VERTICAL-005) |
| `legacy/Lib_Client/G-Logic/GLogicData.h` | 58 | `GLCONST_CHARCLASS` — class constants |
| `legacy/Lib_Client/G-Logic/GLCharDefine.h` | 373 | `SCHARSTATS` — base stats |
| `legacy/Lib_Engine/Common/GameCharacterCalculations.h` | 68-85 | `HitRate` — hit rate formula |
| `legacy/Lib_Engine/Common/GameCharacterCalculations.h` | 48-54 | `Defense` — defense calculation |
| `legacy/Lib_Engine/Common/GameCharacterCalculations.h` | 68-88 | `CriticalBaseRate` — critical chance |
| `legacy/Lib_Engine/Common/GameCharacterCalculations.h` | 98-107 | `RandomDamageRange` — damage interpolation |
| `legacy/Lib_Engine/Common/GameCharacterCalculations.h` | 116-122 | `ApplyStateDamage` — state damage |
| `legacy/Lib_Engine/Common/GameCharacterCalculations.h` | 149-158 | `DamageReduceAmount` — damage reduction |
| `legacy/Lib_Engine/Common/GameCharacterCalculations.h` | 165-174 | `DamageReflectionAmount` — damage reflection |

## 2. Public Research

- RaGEZONE RAN configuration discussion: combat constants such as low-SP hit reduction, low-SP damage reduction, damage absorption, critical and crushing-blow configuration.
- RaGEZONE discussion of RAN critical/damage randomization and server/client random sources.
- Public RAN source repositories: `tablangdelio/ranOnline`, `yexiuph/RanOnline`.

Public material used as supporting evidence only. The repository's actual legacy implementation has priority.

## 3. Hit Formula

### Legacy (GLogixExPC.cpp:1291 CHECKHIT)

```
hitRate = BASIC(100) + nHit - nAvoid + brightnessModifier
hitRate = clamp(hitRate, MIN_HIT=20, MAX_HIT=99)
if (lowSP):
    hitRate = hitRate * (1 - fLOWSP_HIT_DROP)
hit = (hitRate >= hitRoll * 100)
```

### Modern (HitCalculator.h)

```cpp
int32_t hitRate = constants.basicHitRate + input.attackerHit - input.targetAvoid + brightnessMod;
hitRate = clamp(hitRate, constants.minHitRate, constants.maxHitRate);
if (input.lowSP):
    hitRate = hitRate * (1.0f - constants.lowSPHitDrop);
hit = (hitRate >= hitRoll * 100);
```

### Constants

| Constant | Value | Classification |
|----------|-------|----------------|
| `basicHitRate` | 100 | SOURCE-VERIFIED |
| `maxHitRate` | 99 | SOURCE-VERIFIED |
| `minHitRate` | 20 | SOURCE-VERIFIED |
| `lowSPHitDrop` | 0.25f | SOURCE-VERIFIED |
| `brightnessModDis` | -10 | SOURCE-VERIFIED |
| `brightnessModAver` | 0 | SOURCE-VERIFIED |
| `brightnessModAdv` | 10 | SOURCE-VERIFIED |

### Brightness/Environment

**DEFERRED**: Modern Core does not model world brightness. The `brightnessFB` field is exposed as an explicit input. The server currently defaults to `Aver` (modifier = 0). This is documented as deferred per Section 11 of the VERTICAL-006 specification.

## 4. Damage Formula

### Legacy (GLogixExPC.cpp:1363 CALCDAMAGE_20060328)

```
nDAMAGE_NOW = gdDamage.dwLow + (gdDamage.dwHigh - gdDamage.dwLow) * RANDOM_POS
if (targetLevel > attackerLevel):
    nExtFORCE = RANDOM_POS * (targetLevel - attackerLevel) / 10
nDAMAGE_OLD = nDAMAGE_NOW + nExtFORCE

defenseUsed = 1.0f
if (lowSP):
    defenseUsed = 1.0f - fLOW_SEED_DAMAGE

nNetDAMAGE = nDAMAGE_OLD * defenseUsed - nDEFENSE
if (nNetDAMAGE < 0): nNetDAMAGE = 0

if (nNetDAMAGE > 0):
    resultDamage = nNetDAMAGE
else:
    resultDamage = nDAMAGE_OLD * fLOW_SEED_DAMAGE * RANDOM_POS

resultDamage = resultDamage * fSTATE_DAMAGE

if (nDEFENSE_BODY > 0 && nDEFENSE_ITEM > 0):
    fDecRate = 1.0f / (fDAMAGE_DEC_RATE * (1.769 * targetLevel / 120.0f))
    fFinalRate = nDEFENSE_BODY * nDEFENSE_ITEM * fDecRate
    fFinalRate = clamp(fFinalRate, 0.0f, 0.6f)
    resultDamage = resultDamage * (1.0f - fFinalRate)
```

### Modern (PhysicalDamageCalculator.h)

The modern implementation follows the same operation order:

1. Damage range interpolation: `nDAMAGE_NOW = low + (high - low) * damageRoll`
2. Level difference bonus: `nExtFORCE = damageRoll * (targetLevel - attackerLevel) / 10` if targetLevel > attackerLevel
3. `nDAMAGE_OLD = nDAMAGE_NOW + nExtFORCE`
4. Low-SP defense multiplier: `defenseUsed = 1.0f - lowSeedDamage` if lowSP
5. `nNetDAMAGE = nDAMAGE_OLD * defenseUsed - defense`, floor at 0
6. If nNetDAMAGE > 0: `resultDamage = nNetDAMAGE`, else: `resultDamage = nDAMAGE_OLD * lowSeedDamage * damageRoll`
7. `resultDamage *= stateDamage`
8. Defense absorption (if defenseBody > 0 && defenseItem > 0): `resultDamage *= (1 - fFinalRate)`

### Constants

| Constant | Value | Classification |
|----------|-------|----------------|
| `lowSeedDamage` | 0.05f | SOURCE-VERIFIED |
| `damageDecayRate` | 40000.0f | SOURCE-VERIFIED |
| `damageGradeK` | 10.0f | SOURCE-VERIFIED |
| `resistPhysicG` | 0.5f | SOURCE-VERIFIED |

## 5. Defense Formula

### Legacy

```
nDEFENSE = m_nDEFENSE (final defense after equipment, passive, codex)
nDEFENSE_BODY = m_nDEFENSE_BODY (defense before equipment)
nDEFENSE_SKILL = m_nDEFENSE_SKILL (skill defense)
```

The basic physical path uses `m_nDEFENSE` (the final defense value) as a direct subtraction from `nDAMAGE_OLD * defenseUsed`. The `m_nDEFENSE_BODY` and item defense are used for the defense absorption multiplier, but only when both are > 0.

### Modern

- `targetDefense` = `DerivedStats::defense` (the final defense)
- `targetDefenseBody` = `DerivedStats::defenseBody` (defense before equipment)
- `targetDefenseItem` = 0 (not yet modeled)

The defense absorption is applied only when both `defenseBody > 0` and `defenseItem > 0`. Since `defenseItem` is 0 in the current server integration, the absorption block is skipped.

## 6. Low-SP Behavior

### Hit

```
if (lowSP):
    hitRate = hitRate * (1 - fLOWSP_HIT_DROP)
```

`fLOWSP_HIT_DROP = 0.25f` (SOURCE-VERIFIED). The hit rate is reduced by 25% when the target is in low-SP state.

### Damage

```
if (lowSP):
    defenseUsed = 1.0f - fLOW_SEED_DAMAGE
```

`fLOW_SEED_DAMAGE = 0.05f` (SOURCE-VERIFIED). The target's defense effectiveness is reduced by 5% when in low-SP state.

### Low-SP Detection

**LIMITED**: The server uses `target.m_currentSp == 0` as a proxy for low-SP state. The exact legacy definition of "low SP" may differ (e.g., SP below a threshold percentage). This is documented as LIMITED per Section 12.

## 7. Critical

### Legacy (GLogixExPC.cpp:1603-1613)

```
ndxLvl = nLEVEL - GETLEVEL()
ndxLvl = clamp(ndxLvl, -5, 5)
nPerHP = (GETHP() * 100) / GETMAXHP()
if (nPerHP <= 10): nPerHP = 10
nPercentCri = 1000 / nPerHP - 10 + ndxLvl
nPercentCri = clamp(nPercentCri, 0, dwCRITICAL_MAX)
bCritical = (nPercentCri > criticalRoll * 100)
```

### Modern (PhysicalDamageCalculator.h)

```cpp
int32_t nPercentCri = CriticalBaseRate(attackerCurrentHP, attackerMaxHP, attackerLevel, targetLevel);
nPercentCri = clamp(nPercentCri, 0, constants.criticalMax);
bool bCritical = (nPercentCri > criticalRoll * 100);
```

### Critical Damage

```
if (bCritical):
    resultDamage = nDAMAGE_OLD * dwCRITICAL_DAMAGE / 100
```

`dwCRITICAL_DAMAGE = 120` (SOURCE-VERIFIED). Critical hits deal 120% of the original damage (20% bonus).

### Constants

| Constant | Value | Classification |
|----------|-------|----------------|
| `criticalDamage` | 120 | SOURCE-VERIFIED |
| `criticalMax` | 40 | SOURCE-VERIFIED |

## 8. Crushing Blow

### Legacy

```
nCrushingBlow = attackerCrushingBonus (from items)
nCrushingBlow = clamp(nCrushingBlow, 0, dwCRUSHING_BLOW_MAX)
bCrushingBlow = (nCrushingBlow > crushingRoll * 100)
```

### Modern (PhysicalDamageCalculator.h)

```cpp
int32_t nCrushingBlow = input.attackerCrushingBonus;
nCrushingBlow = clamp(nCrushingBlow, 0, constants.crushingBlowMax);
bool bCrushingBlow = (nCrushingBlow > crushingRoll * 100);
```

### Crushing Blow Damage

```
if (bCritical && bCrushingBlow):
    resultDamage = nDAMAGE_OLD * dwCRUSHING_BLOW_DAMAGE / 100
else if (bCritical):
    resultDamage = nDAMAGE_OLD * dwCRITICAL_DAMAGE / 100
else if (bCrushingBlow):
    resultDamage = nDAMAGE_OLD * dwCRUSHING_BLOW_DAMAGE / 100
```

When both critical and crushing blow apply, the crushing blow damage formula is used (150%).

### Constants

| Constant | Value | Classification |
|----------|-------|----------------|
| `crushingBlowDamage` | 150 | SOURCE-VERIFIED |
| `crushingBlowMax` | 20 | SOURCE-VERIFIED |
| `crushingBlowRange` | 10.0f | SOURCE-VERIFIED |

### Crushing Blow Trigger

**LIMITED**: The crushing blow trigger uses `attackerCrushingBonus` from items. In the current server integration, this is 0 (not yet modeled). The `fCRUSH_BLOW_RANGE` constant is available but not used in the basic physical path.

## 9. Minimum Damage

### Legacy

```
if (resultDamage == 0):
    resultDamage = 1
```

A successful hit always deals at least 1 damage. A miss deals 0 damage.

### Modern (PhysicalDamageCalculator.h:124-125)

```cpp
if (result.damage == 0)
    result.damage = 1;
```

## 10. Random Input Model

All random values are supplied by the caller as deterministic inputs:

| Input | Range | Used for |
|-------|-------|----------|
| `hitRoll` | [0.0, 1.0] | Hit/miss determination |
| `damageRoll` | [0.0, 1.0] | Damage range interpolation |
| `criticalRoll` | [0.0, 1.0] | Critical hit determination |
| `crushingRoll` | [0.0, 1.0] | Crushing blow determination |
| `reflectionRoll` | [0.0, 1.0] | Damage reflection (deferred) |

No `rand()` or `std::rand()` is called inside combat rules. The server uses a deterministic RNG (`std::mt19937` with fixed seed) for testing.

## 11. Missing Constants

| Constant | Status | Notes |
|----------|--------|-------|
| `fLOWSP_HIT_DROP` | SOURCE-VERIFIED | 0.25f |
| `fLOWSP_DAMAGE` | SOURCE-VERIFIED | 0.50f (used as `lowSPDamage`) |
| `fLOW_SEED_DAMAGE` | SOURCE-VERIFIED | 0.05f |
| `fDAMAGE_DEC_RATE` | SOURCE-VERIFIED | 40000.0f |
| `fDAMAGE_GRADE_K` | SOURCE-VERIFIED | 10.0f |
| `fRESIST_PHYSIC_G` | SOURCE-VERIFIED | 0.5f |
| `dwCRITICAL_DAMAGE` | SOURCE-VERIFIED | 120 |
| `dwCRITICAL_MAX` | SOURCE-VERIFIED | 40 |
| `dwCRUSHING_BLOW_DAMAGE` | SOURCE-VERIFIED | 150 |
| `dwCRUSHING_BLOW_MAX` | SOURCE-VERIFIED | 20 |
| `fCRUSH_BLOW_RANGE` | SOURCE-VERIFIED | 10.0f |

## 12. Limited Behavior

| Behavior | Status | Notes |
|----------|--------|-------|
| Brightness/environment | DEFERRED | Hardcoded to Aver; modern Core does not model world brightness |
| Low-SP detection | LIMITED | Uses `currentSP == 0` as proxy; exact legacy definition may differ |
| Item critical/crushing bonuses | LIMITED | Set to 0; not yet modeled |
| Item defense | LIMITED | Set to 0; not yet modeled |
| Damage reduction | LIMITED | Set to 0; not yet modeled |
| Damage reflection | LIMITED | Set to 0; not yet modeled |
| Element resistance | LIMITED | Set to 0; not yet modeled |
| State damage | LIMITED | Set to 1.0f; no state damage yet |

## 13. Deferred Behavior

| Behavior | Status | Notes |
|----------|--------|-------|
| Magic combat | DEFERRED | Not part of VERTICAL-006 |
| Skill-specific damage | DEFERRED | Not part of VERTICAL-006 |
| PvP systems | DEFERRED | Not part of VERTICAL-006 |
| Monster AI | DEFERRED | Not part of VERTICAL-006 |
| Ranged combat | DEFERRED | Melee only; ranged requires additional state |
| Full skill effects | DEFERRED | Not part of VERTICAL-006 |

## 14. Modern Implementation Mapping

| Legacy | Modern |
|--------|--------|
| `GLHITRATE` | `HitCalculator::CalculateHitRate` |
| `CHECKHIT` | `HitCalculator::CheckHit` |
| `CALCDAMAGE_20060328` | `PhysicalDamageCalculator::CalculatePhysicalDamage` |
| `GLDEFENSE` | `GameCharacterCalculations::Defense` |
| `CriticalBaseRate` | `GameCharacterCalculations::CriticalBaseRate` |
| `RandomDamageRange` | `GameCharacterCalculations::RandomDamageRange` |
| `ApplyStateDamage` | `GameCharacterCalculations::ApplyStateDamage` |
| `DamageReduceAmount` | `GameCharacterCalculations::DamageReduceAmount` |
| `DamageReflectionAmount` | `GameCharacterCalculations::DamageReflectionAmount` |
| `GLCHARLOGIC::RECEIVE_DAMAGE` | `ResourceState::ApplyDamage` |

## 15. Test Coverage

### Core Tests (CombatTests.cpp)

- Hit/miss boundaries
- Hit rate clamping (min/max)
- Low-SP hit modifier
- Damage range (min/mid/max rolls)
- Defense reduction
- State damage multiplier
- Low-SP damage modifier
- Minimum damage = 1
- Miss = 0 damage
- Critical hit (occurs, non-occurs, boundary)
- Crushing blow (occurs, non-occurs, boundary)
- Combined critical + crushing
- Critical + defense
- Low-SP + critical
- High defense + minimum damage
- Deterministic behavior
- Combat result flags
- Combat result target HP
- Overkill behavior
- Damage reduction
- Level difference bonus
- Combat constants verification

### Server Tests (ServerCharacterTests.cpp)

- Attack reduces target HP
- Attacker state unchanged
- Self-attack refused
- Dead target refused
- Snapshot exposes HP
- Multiple attacks

### Client Tests (ClientGameplayTests.cpp)

- Empty state
- Authoritative hit presentation
- Authoritative miss presentation
- Damage presentation
- Critical presentation
- Crushing presentation
- Updated HP presentation
- Later snapshot replaces earlier
- Client does not recalculate

## 16. Build and Test

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

## 17. Runtime

- Emulator: NOT REQUIRED
- MiniA: NOT USED
- Production client: NOT USED
- Production server: NOT USED
