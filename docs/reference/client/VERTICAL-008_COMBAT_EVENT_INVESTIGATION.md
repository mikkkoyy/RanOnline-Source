# VERTICAL-008: Combat Event Resolution & Reflection — Investigation

## 1. Legacy Source Locations

| File | Lines | What it contains |
|------|-------|-----------------|
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 1745-1763 | Reflection calculation in CALCDAMAGE_20060328 |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp` | 2041-2058 | Reflection calculation in CALCDAMAGE_2004 |
| `legacy/Lib_Client/G-Logic/GLogicExNPC.cpp` | 319-338 | Reflection calculation in NPC combat |
| `legacy/Lib_Client/G-Logic/GLChar.cpp` | 2684-2703 | `DamageReflectionProc` — applies reflection damage to attacker |
| `legacy/Lib_Client/G-Logic/GLChar.cpp` | 2342-2353 | `ToDamage` — calls `ReceiveDamage` directly (no recursion) |
| `legacy/Lib_Client/G-Logic/GLChar.cpp` | 2093-2099 | `RECEIVE_DAMAGE` — direct HP decrease |
| `legacy/Lib_Client/G-Logic/GLCharData.h` | 266-273 | `DAMAGE_SPEC` struct with reflection fields |
| `legacy/Lib_Engine/Common/GameCharacterCalculations.cpp` | 554-563 | `DamageReflectionAmount` formula |
| `legacy/Lib_Engine/G-Logic/GLDefine.h` | 772-785 | `DAMAGE_TYPE_PSY_REFLECTION` flag |

## 2. Public Research

- RaGEZONE RAN configuration discussion: combat constants such as low-SP hit reduction, low-SP damage reduction, damage absorption, critical and crushing-blow configuration.
- RaGEZONE discussion of RAN critical/damage randomization and server/client random sources.
- Public RAN source repositories: `tablangdelio/ranOnline`, `yexiuph/RanOnline`.

Public material used as supporting evidence only. The repository's actual legacy implementation has priority.

## 3. Reflection Formula

### Legacy

```cpp
// GLogixExPC.cpp:1745-1763
if ( fDamageReflectionRate > 0.0f )
{
    if ( fDamageReflectionRate > (RANDOM_POS*1) )
    {
        int nDamageReflection = GameCharacterCalculations::DamageReflectionAmount(
            rResultDAMAGE, fDamageReflection, nLEVEL, GLCONST_CHAR::wMAX_LEVEL);

        if ( nDamageReflection > 0 )
        {
            if ( bPsyDamage ) dwDamageFlag += DAMAGE_TYPE_PSY_REFLECTION;
            else dwDamageFlag += DAMAGE_TYPE_MAGIC_REFLECTION;

            // The wounded reflects damage back to the attacker.
            STARGETID sActor(CROW_PC,dwGaeaID);
            pActor->DamageReflectionProc( nDamageReflection, sActor );
        }
    }
}
```

### Formula

```
nDamageReflection = (int)(((rResultDAMAGE * fDamageReflection) * nLEVEL) / GLCONST_CHAR::wMAX_LEVEL)
```

Where:
- `rResultDAMAGE` = post-reduction, post-critical final damage
- `fDamageReflection` = reflection modifier (percentage as decimal, e.g., 0.1 = 10%)
- `nLEVEL` = target's level (the one reflecting)
- `wMAX_LEVEL` = 300 (constant)

### Modern

```cpp
// PhysicalDamageCalculator.h
if (input.damageReflectionRate > 0.0f && result.damage > 0)
{
    uint32_t reflectionRoll = static_cast<uint32_t>(input.reflectionRoll * 100.0f);
    result.reflectionRoll = reflectionRoll;

    if (static_cast<uint32_t>(input.damageReflectionRate * 100.0f) > reflectionRoll)
    {
        result.reflectionTriggered = true;

        int32_t nDamageReflection = Modern::Engine::DamageReflectionAmount(
            static_cast<int32_t>(result.damage),
            input.damageReflection,
            static_cast<int32_t>(input.targetLevel),
            static_cast<int32_t>(constants.maxLevel));

        if (nDamageReflection > 0)
        {
            result.reflectionDamage = static_cast<uint32_t>(nDamageReflection);
        }
    }
}
```

## 4. Reflection Ordering

### Legacy

Reflection is checked **AFTER** final damage is computed — specifically after critical/crushing damage multiplication AND after damage reduction. It is the last damage calculation step before the defense skill check.

### Exact Ordering

1. **Raw damage** (base damage range + skill VAR + item damage)
2. **Defense subtraction** (`nDEFENSE` subtracted from raw damage)
3. **Critical/Crushing blow multiplication** (applied to post-defense damage)
4. **Damage reduction** (subtracted from post-critical damage)
5. **Final primary damage** (`rResultDAMAGE` after all above steps)
6. **Reflection** (calculated from `rResultDAMAGE`, applied to attacker)

### Modern

The modern implementation follows the same ordering:
1. Raw damage interpolation
2. Defense subtraction
3. Critical/crushing multiplication
4. Damage reduction
5. Reflection calculation

## 5. Reflection Recursion

### Legacy

**NO** — reflection cannot recursively trigger.

`DamageReflectionProc` calls `ToDamage` directly, NOT `CALCDAMAGE`. The `ToDamage` function calls `ReceiveDamage` which directly decreases HP without going through the damage calculation pipeline.

### Call Chain

```
DamageReflectionProc → ToDamage → ReceiveDamage → RECEIVE_DAMAGE (direct HP decrease)
```

No recursion possible.

### Modern

The modern implementation prevents recursion by:
1. Reflection is calculated in `PhysicalDamageCalculator` as a pure function
2. The server applies reflection damage directly to attacker HP
3. No combat calculation is triggered by reflection damage

## 6. Reflection on Critical/Crushing Hits

### Legacy

**YES** — reflection applies on critical/crushing hits.

Reflection is checked AFTER critical/crushing damage is applied to `rResultDAMAGE`. The reflection amount is calculated from the post-critical damage value.

### Modern

The modern implementation calculates reflection after critical/crushing damage is applied, using the final damage value.

## 7. Reflection Damage Flag

### Legacy

**YES** — reflection has its own damage flags:

```cpp
DAMAGE_TYPE_PSY_REFLECTION = 0x0020    // Physical reflection
DAMAGE_TYPE_MAGIC_REFLECTION = 0x0040  // Magic reflection
```

### Modern

The modern implementation uses `DAMAGE_TYPE_PSY_REFLECTION` when reflection is triggered.

## 8. Reflection Can Kill Attacker

### Legacy

**YES** — reflection can kill the attacker.

`DamageReflectionProc` → `ToDamage` → `ReceiveDamage` → `RECEASE_DAMAGE` directly decreases HP via `DECREASE(m_dwNowHP, dwDamage)`. If HP reaches 0, the target dies. There is no minimum HP floor for reflection damage.

### Modern

The modern implementation applies reflection damage to the attacker via direct HP subtraction. If the attacker's HP reaches 0, they die.

## 9. Reflection Random Roll

### Legacy

```cpp
if ( fDamageReflectionRate > (RANDOM_POS*1) )
```

`RANDOM_POS` generates a random float in range [0.0, 1.0]. The rate is compared: if `fDamageReflectionRate > RANDOM_POS`, reflection triggers.

Example: if `fDamageReflectionRate = 0.3` (30%), then reflection triggers when `RANDOM_POS < 0.3`, which is 30% of the time.

### Modern

The modern implementation uses the same pattern:
```cpp
if (static_cast<uint32_t>(input.damageReflectionRate * 100.0f) > reflectionRoll)
```

Where `reflectionRoll` is the caller-supplied random value scaled to [0, 100].

## 10. Dead Targets Cannot Reflect

### Legacy

**NO** — dead targets cannot reflect.

The `ToDamage` function checks `IsValidBody()` before calling `ReceiveDamage`. If the target is dead (HP = 0), `IsValidBody()` returns false and no damage is applied.

### Modern

The modern implementation checks that the target is alive before applying damage. If the target dies from primary damage, reflection is not applied (the target cannot reflect).

## 11. Low-SP Condition

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

**LIMITED**: Without a skill system, we use `currentSP == 0` as a proxy for low-SP state. This is documented as LIMITED per Section 22 of the VERTICAL-008 specification.

The combat engine receives `targetLowSP` as an explicit input from the server, which determines the state from character SP.

## 12. Modern Implementation Mapping

| Legacy | Modern |
|--------|--------|
| `fDamageReflectionRate` | `CombatInput::targetDamageReflectionRate` |
| `fDamageReflection` | `CombatInput::targetDamageReflection` |
| `DamageReflectionAmount()` | `GameCharacterCalculations::DamageReflectionAmount()` |
| `DamageReflectionProc()` | `ServerCharacter::Attack()` (reflection application) |
| `DAMAGE_TYPE_PSY_REFLECTION` | `Combat::DAMAGE_TYPE_PSY_REFLECTION` |
| `RANDOM_POS` | `CombatInput::reflectionRoll` |

## 13. Test Coverage

### Core Tests (CombatTests.cpp)

- Reflection disabled
- Reflection rate zero
- Reflection threshold below roll
- Reflection threshold above roll
- Exact reflection boundary
- Reflection amount zero
- Reflection truncation
- Level scaling
- Maximum-level scaling
- Critical + reflection
- Crushing + reflection
- Damage reduction + reflection
- Minimum primary damage + reflection
- Large primary damage
- No recursion
- Attacker HP tracked
- Miss produces no reflection

### Server Tests (ServerCharacterTests.cpp)

- Normal attack → target HP decreases
- Reflection enabled → target HP decreases → attacker HP decreases
- Reflection disabled → attacker HP unchanged
- Target dies → correct reflection behavior
- Attacker dies from reflection → target state remains correct
- Transactional failure → neither character changes

### Client Tests (ClientGameplayTests.cpp)

- Server resolves primary damage → client receives authoritative result
- Server resolves reflection → client receives authoritative reflection result
- Client does not independently calculate reflection

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
| State damage | LIMITED | Set to 1.0f; no state damage yet |
| Brightness/environment | DEFERRED | Hardcoded to Aver |
| Elemental resistance | LIMITED | Not connected to physical combat |
| Magic combat | DEFERRED | Not part of VERTICAL-008 |
| Ranged combat | DEFERRED | Melee only; reflection disabled for ranged in legacy |
| PK damage penalty | DEFERRED | Legacy has `fPK_POINT_DEC_PHY` for PC reflection |
| Block damage back | DEFERRED | Legacy has `RANPARAM::bFeatureBlockDamageBack` |
