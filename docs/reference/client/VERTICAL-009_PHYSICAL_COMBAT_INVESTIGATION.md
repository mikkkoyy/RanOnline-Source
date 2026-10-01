# VERTICAL-009 — Physical Combat Completion: Legacy Investigation

Scope: the physical-combat gaps left open by VERTICAL-006/007/008. Every formula
below was read out of `legacy/` in this repository. Where the source does not prove
a behaviour, the entry says so and no behaviour was invented.

The two authoritative damage functions are:

- `legacy/Lib_Client/G-Logic/GLogixExPC.cpp:1363` `GLCHARLOGIC::CALCDAMAGE_20060328`
  — the modern-equivalent path.
- `legacy/Lib_Client/G-Logic/GLogixExPC.cpp:1363`'s older sibling
  `CALCDAMAGE_2004` (`:1980`) and `legacy/Lib_Client/G-Logic/GLogicExNPC.cpp`
  `GLCROWLOGIC::CALCDAMAGE` (`:220`).

---

## 1. Low SP

### What the legacy actually compares

`bLowSP` is **not** "SP is zero". It is a comparison against the SP the action
costs, multiplied by the number of strikes:

`legacy/Lib_Client/G-Logic/GLogixExPC.cpp:3492-3497` (basic attack)

```cpp
WORD wDisSP = GLCONST_CHAR::wBASIC_DIS_SP;
if ( pRHAND )  wDisSP += pRHAND->sSuitOp.wReqSP;
if ( pLHAND )  wDisSP += pLHAND->sSuitOp.wReqSP;

if ( m_sSP.dwNow < (wDisSP*wStrikeNum) )  return EMBEGINA_SP;
```

`legacy/Lib_Client/G-Logic/GLogixExPC.cpp:4254-4258` (skill) is the same shape
with `sSKILL_DATA.wUSE_SP` in place of `wBASIC_DIS_SP`, returning `EMSKILL_NOTSP`.

The result code is turned into the flag the combat maths reads:

| File | Line | Code |
| --- | --- | --- |
| `legacy/Lib_Client/G-Logic/GLCharMsg.cpp` | 612 | `BOOL bLowSP = (emBeginFB==EMBEGINA_SP) ? TRUE: FALSE;` |
| `legacy/Lib_Client/G-Logic/GLCharSkillMsg.cpp` | 980 | `BOOL bLowSP = (emCHECK==EMSKILL_NOTSP) ? TRUE : FALSE;` |
| `legacy/Lib_Client/G-Logic/GLChar.cpp` | 4811 | `BOOL bLowSP = (emCHECK==EMSKILL_NOTSP) ? TRUE : FALSE;` |

### Where the required SP comes from

`m_wSUM_DisSP` is **not** skill-derived. It is the equipment-imposed SP overhead,
recomputed from the worn hands in `SUM_ITEM`:

`legacy/Lib_Client/G-Logic/GLogixExPC.cpp:428-434`

```cpp
m_wSUM_DisSP = m_wACCEPTP;                          // base from accept points
SITEM* pRHAND = GET_SLOT_ITEMDATA ( emRHand );
SITEM* pLHAND = GET_SLOT_ITEMDATA ( emLHand );
if ( pRHAND )  m_wSUM_DisSP += pRHAND->sSuitOp.wReqSP;
if ( pLHAND )  m_wSUM_DisSP += pLHAND->sSuitOp.wReqSP;
```

Total cost is `m_wSUM_DisSP + <action SP>`, which is what every consumer adds:

| Location | Formula |
| --- | --- |
| `GLChar.cpp:2431` | `wDisSP = m_wSUM_DisSP + GLCONST_CHAR::wBASIC_DIS_SP;` |
| `GLChar.cpp:2454` | `wDisSP = m_wSUM_DisSP + sSKILL_DATA.wUSE_SP;` |
| `GLChar.cpp:2506` | `wDisSP = m_wSUM_DisSP + GLCONST_CHAR::wBASIC_DIS_SP;` |
| `GLChar.cpp:3007` | `wDisSP = m_wSUM_DisSP + sSKILL_DATA.wUSE_SP;` |

`wBASIC_DIS_SP = 1` (`GLogicData.cpp:264`); `m_wACCEPTP` is
`legacy/Lib_Client/G-Logic/GLogicEx.h:402`-adjacent `WORD m_wSUM_DisSP`, declared at
`GLogicEx.h:402`, reset at `GLogixExPC.cpp:105`.

### The three modifiers

`legacy/Lib_Client/G-Logic/GLogicData.cpp:266-269`

```cpp
float fLOWSP_MOTION     = 0.20f;
float fLOWSP_DAMAGE     = 0.50f;
float fLOWSP_HIT_DROP   = 0.25f;
float fLOWSP_AVOID_DROP = 0.50f;
```

**Hit** — `legacy/Lib_Client/G-Logic/GLogixExPC.cpp:1322-1324` (`CHECKHIT`):

```cpp
const float fBaseHitRate = float(GLOGICEX::GLHITRATE(GETHIT(), nAVOID, bFB));
const float fMulHitRate = bLowSP ? (1.0f - GLCONST_CHAR::fLOWSP_HIT_DROP) : 1.0f;
const int nHitRate = int(fBaseHitRate * fMulHitRate);
```

**Damage** — `legacy/Lib_Client/G-Logic/GLChar.cpp:2488-2491` (`PreStrikeProc`):

```cpp
float fDAMAGE_RATE(1.0f);
if ( bLowSP )  fDAMAGE_RATE *= (1-GLCONST_CHAR::fLOWSP_DAMAGE);
nDAMAGE = int(nDAMAGE*fDAMAGE_RATE);
```

Note this is a direct damage multiplier applied *after* defence subtraction, not a
defence discount. VERTICAL-006 had modelled low SP as
`defenseUsed = 1.0f - lowSeedDamage`, which is a different quantity entirely
(`lowSeedDamage` is the 0.05 minimum-damage constant, not the low-SP constant) and
is applied at the wrong point in the pipeline. That is corrected here.

**Avoid** — `fLOWSP_AVOID_DROP` is declared and loaded
(`GLogicDataLoad.cpp:199`) but **no call site applies it**. Documented as unused
rather than implemented; implementing it would mean inventing behaviour.

### Also on the low-SP path

- **No SP is consumed.** `GLChar.cpp:2429`, `:2504`, `:3005` all guard the
  deduction with `if ( !bLowSP )`.
- **Skill variable effects are halved.** `GLChar.cpp:3129-3135`:
  `nVAR_HP /= 2; nVAR_MP /= 2; nVAR_SP /= 2;`.
- **A separate, ratio-based low-SP drives animation.**
  `legacy/Lib_Client/G-Logic/GLCharacter.cpp:3446`:
  `bLowSP = ( float(m_sSP.dwNow) / float(m_sSP.dwMax) ) <= GLCONST_CHAR::fLOWSP_MOTION;`
  This is presentation, not combat, and is out of Core's scope.
- **NPCs do not use the low-SP path at all.** `GLogicExNPC.cpp:837-838` is commented
  out.

### Modern implementation

- `CombatInput::attackerRequiredSP` / `PhysicalDamageInput::requiredSP` carry the
  required SP; `CombatConstants::basicDisSP` holds `wBASIC_DIS_SP`.
- `ServerCharacter::Attack()` sets `targetLowSP = (target.m_currentSp < requiredSP)`,
  replacing the `currentSP == 0` proxy.
- `PhysicalDamageCalculator` applies `resultDamage *= (1 - lowSPDamage)` after
  defence subtraction, matching `GLChar.cpp:2489`.
- `HitCalculator` already applied `(1 - lowSPHitDrop)` and is unchanged.

**Limitation.** `m_wSUM_DisSP` is built from `SITEM::sSuitOp::wReqSP`, which the
modern `ItemStatBlock` does not carry. Only `wBASIC_DIS_SP` is modelled, so
`requiredSP` is currently `1`. The comparison operator and the multiplier are
exact; the equipment term is missing pending a `wReqSP` field on the stat block.

---

## 2. Physical resistance

### Formula

`legacy/Lib_Client/G-Logic/GLogixExPC.cpp:1556-1563`

```cpp
{
    float fResistTotal = (float) ((float) nRESIST * 0.01f * fRESIST_G);
    fResistTotal = fResistTotal > 0.8f ? 0.8f : fResistTotal;

    gdDamage.dwLow  -= (DWORD) ((float) gdDamage.dwLow  * fResistTotal);
    gdDamage.dwHigh -= (DWORD) ((float) gdDamage.dwHigh * fResistTotal);
}
```

So physical resistance is a **percentage reduction applied to the damage range
itself, before the random roll and before defence**. It is not a defence-side
modifier and not a post-defence multiplier.

`fRESIST_G` is chosen by attack kind at `GLogixExPC.cpp:1455` (`EMAPPLY_PHY_SHORT`)
and `:1467` (`EMAPPLY_PHY_LONG`), both set to `GLCONST_CHAR::fRESIST_PHYSIC_G`.
Magic attacks use the separate `fRESIST_G` constant instead.

### Constants and caps

| Constant | Value | Location |
| --- | --- | --- |
| `fRESIST_PHYSIC_G` | `0.5f` | `GLogicData.cpp:260` |
| `fMAX_RESIST` | `99.0f` | `GLogicData.cpp:262` |

The raw resistance is clamped **before** the reduction, at
`GLogixExPC.cpp:1516`:

```cpp
if ( nRESIST>GLCONST_CHAR::fMAX_RESIST ) nRESIST = GLCONST_CHAR::fMAX_RESIST;
```

**Consequence worth recording:** with `fMAX_RESIST = 99` and `fRESIST_PHYSIC_G = 0.5`,
the largest reachable `fResistTotal` is `99 * 0.01 * 0.5 = 0.495`. The hardcoded
`0.8f` cap at `:1559` is therefore **unreachable in the physical path**. VERTICAL-006
had recorded `fMAX_RESIST` as `0.8f`, conflating the raw-value clamp with the
unreachable reduction cap; the two are now separate constants (`maxResist = 99.0f`,
`maxResistReduction = 0.8f`).

The older `CALCDAMAGE_2004` (`:1941`) and `GLCROWLOGIC::CALCDAMAGE` (`:250`) apply
the same multiplier but to the skill-variable term only:

```cpp
nVAR = nVAR - (int) ( nVAR*nRESIST*0.01f*fRESIST_G );
```

and neither carries the `0.8f` cap. VERTICAL-009 ports the `CALCDAMAGE_20060328`
form, which is the current path.

### Where resistance is populated

`legacy/Lib_Client/G-Logic/GLogixExPC.cpp:394`

```cpp
m_sSUMRESIST = m_sSUMPASSIVE.m_sSUMRESIST + m_sSUMITEM.sResist + m_dwResistanceIncrease;
```

Item resistances are added per element at `GLogixExPC.cpp:665-669`; skill
(`EMFOR_RESIST`, `:994`) and impact (`EMIMPACTA_RESIST`, `:1073`) buff it; and
`GLogixExPC.cpp:2979` calls `m_sSUMRESIST_SKILL.LIMIT()` to clamp every element to
`>= 0`. The `SRESIST` struct and its `GetElement`/`LIMIT` are at
`legacy/Lib_Client/G-Logic/GLCharDefine.h:676`.

### Modern implementation

`PhysicalDamageCalculator` applies the reduction to `nDAMAGE_OLD` before the
defence subtraction, using `CombatConstants::resistPhysicG` and `maxResist`.
`PhysicalDamageInput::resistElement` already existed and was previously ignored.

---

## 3. PK damage modifier

**`fDAMAGE_DEC_RATE` is not the PK constant.** It is the defence-decay divisor and
has two historical forms:

| Form | Location | Formula |
| --- | --- | --- |
| Current | `GLogixExPC.cpp:1703-1713` | `fDecRate = 1.0f / (fDAMAGE_DEC_RATE * (1.769f * (float)nLEVEL / 120.f))`, then `fFinalRate = body*item*fDecRate` capped at `0.6f` |
| Legacy | `GLogixExPC.cpp:2017`, `GLogicExNPC.cpp:300`, `GLSummon.cpp:471`, `GLSummonSkill.cpp:1667` | `fRATE = 1.0f - nDEFAULT_DEFENSE*nITEM_DEFENSE / fDAMAGE_DEC_RATE` |

`fDAMAGE_DEC_RATE = 40000.0f` (`GLogicData.cpp:272`). VERTICAL-006 already ports the
current form; the four legacy sites differ and are not in the modern path.

`fPK_POINT_DEC_RATE` (`GLogicData.cpp:329`) is unrelated again — it is the
four-hour PK-points decay timer used at `GLChar.cpp:5631`.

The actual PK damage constant is **`fPK_POINT_DEC_PHY = 0.5f`** (`GLogicData.cpp:330`),
applied as a direct multiplier:

`legacy/Lib_Client/G-Logic/GLChar.cpp:2514-2518` (`PreStrikeProc`)

```cpp
if ( m_TargetID.emCrow==CROW_PC )
{
    nDAMAGE = int(nDAMAGE*GLCONST_CHAR::fPK_POINT_DEC_PHY);
    if ( nDAMAGE==0 )	nDAMAGE = 1;
}
```

Also applied to reflected damage (`GLChar.cpp:2697-2701`) and to continuous damage
(`GLLandMan.cpp:2830-2837`).

### Ordering

The PK modifier is **not** inside `CALCDAMAGE_20060328`. It is applied by the
caller *after* the calculation returns:

```
PreStrikeProc:
  CALCDAMAGE_20060328  ->  nDAMAGE
  if (target is PC)  nDAMAGE = int(nDAMAGE * fPK_POINT_DEC_PHY)
  ToDamage(...)
```

`DamageReflectionProc` (`GLChar.cpp:2684-2703`) applies the same multiplier to
reflected damage before `ToDamage`.

### PK state detection

Two independent implementations, both far outside a combat calculator:

- Server: `legacy/Lib_Client/G-Logic/GLChar.cpp:1995` `IsReActionable()` — safe
  zone/time, party membership, `IsConflictTarget()` (`:1961`), PK map + zone,
  hostile flag, `nPK_LIMIT_LEVEL` + `bPK_MODE`, club battle.
- Client: `legacy/Lib_Client/G-Logic/GLCharacter.cpp:2306` `IsPK_TAR()` — the same
  family of checks for presentation.

None of this is a damage formula.

### Modern implementation

`CombatInput::isPK` / `PhysicalDamageInput::isPK` select the multiplier. The
constant is `CombatConstants::pkPointDecPhy = 0.5f`, applied after damage
reduction and before reflection, mirroring the caller-side position in
`PreStrikeProc`. `ServerCharacter::Attack()` currently passes `false`.

**Limitation.** PK *detection* is not implemented; the flag is the seam. A server
without safe zones, parties and PK maps has nothing to evaluate it against, and
inventing that evaluation here would be a behaviour claim the source does not
support in this slice.

---

## 4. Block damage back

`RANPARAM::bFeatureBlockDamageBack` (`RANPARAM.h:329`, default `FALSE` at
`RANPARAM_FEATURE.cpp:68`, loaded at `:157`) is **not a damage formula**. It is a
gating flag that suppresses repeated reflection from the same actor for a window.

`legacy/Lib_Client/G-Logic/GLChar.cpp:2684-2703` (`DamageReflectionProc`)

```cpp
if ( RANPARAM::bFeatureBlockDamageBack && m_pLandMan )
{
    GLACTOR* pactor_target = GLGaeaServer::GetInstance().GetTarget( m_pLandMan, sACTOR );
    if ( pactor_target )
    {
        if ( pactor_target->IsBlockDamageBack() )	return;   // already reflected: skip
        pactor_target->SetBlockDamageBack( true );
    }
}
```

The window closes in `FrameMain` (`GLChar.cpp:5396-5404`) after
`fFeatureBlockDamageBackTimer` (default `1.0f`, `RANPARAM_FEATURE.cpp:69`).
`GLCrow.cpp:772`/`:1297` and `GLSummonField.cpp:1302`/`:662` mirror it;
`GLMaterial.h:163` is a permanent no-op.

Findings:

- Block does **not** change primary damage.
- It returns no damage of its own; it only permits or suppresses a reflection.
- It cannot recurse, by construction — the second attempt in the window returns
  before `ToDamage`.

The modern reflection path already never recurses: `PhysicalDamageCalculator` calls
`DamageReflectionAmount` and returns a number, and the caller applies it once. So
the mechanism VERTICAL-009 was asked to investigate is **already satisfied** at the
damage-calculation level. What is missing is only the per-actor cooldown, which is
authoritative character state and belongs with a real reflection/actor system.

**Deferred** — the cooldown flag and timer, as a server-side actor state. The
feature is also `FALSE` by default, so no current behaviour depends on it.

---

## 5. State damage

`legacy/Lib_Engine/Common/GameCharacterCalculations.cpp:635-641`

```cpp
GameInt32 ApplyStateDamage( GameInt32 damage, float stateDamage )
{
    return static_cast<GameInt32>( static_cast<float>(damage) * stateDamage );
}
```

Default `1.0f` at `GLogixExPC.cpp:2209` (`RESET_DATA`) and
`GLogicExNPC.cpp:30`; `GLMaterial.h:152` returns `0.0f`.

Exactly one state blow changes it, and additively:
`GLogixExPC.cpp:2521` (`EMBLOW_FROZEN`) and `GLogicExNPC.cpp:718`

```cpp
m_fSTATE_DAMAGE += sSTATEBLOW.fSTATE_VAR2;
```

STUN, STONE, BURN, MAD, POISON, CURSE and NUMB do not touch it.

**Basic physical attacks do go through it.** `CALCDAMAGE_20060328` has one shared
`ApplyStateDamage` at `GLogixExPC.cpp:1688-1689` covering both the skill branch
(`:1415-1553`) and the basic branch (`:1572-1597`, selected by
`ISLONGRANGE_ARMS()`). The neutral default means a basic attack is unaffected in
practice.

Modern `PhysicalDamageCalculator` already multiplies by `input.stateDamage` at the
same pipeline position (after defence, before the defence-decay stage), and
`ServerCharacter` passes `1.0f`. **Verified correct, no change needed.**

---

## 6. Brightness / environment

`legacy/Lib_Client/G-Logic/GLCharDefine.h:841-848`

```cpp
enum EM_BRIGHT_FB { BFB_DIS = 0, BFB_AVER = 1, BFB_ADV = 2, BFB_SIZE = 3 };
```

Resolved by `SpaceGap` (`legacy/Lib_Engine/Common/GameCharacterCalculations.cpp:26-50`):
equal actor and receiver brightness is always `Aver`; otherwise the *space*
brightness decides who is advantaged. Obtained in
`GLogixExPC.cpp:1318-1319` and `GLogicExNPC.cpp:167-168` from
`pLandMan->GETBRIGHT()` plus both actors' `GETBRIGHT()`.

**Hit** — `legacy/Lib_Engine/Common/GameCharacterCalculations.cpp:68-85`, table
`{-10, 0, +10}` over the `BASIC(100) + nHit - nAvoid + nBirght[bFB]` expression,
clamped to `[20, 99]`. This is the only place brightness is live in the damage path.

**Damage** — `Defense()` (`GameCharacterCalculations.cpp:102-108`) takes an
environment factor `{0.8f, 1.0f, 1.2f}` and multiplies defence by it, but **no
pipeline calls it**: `CALCDAMAGE_20060328` and `GLCROWLOGIC::CALCDAMAGE` both use
`nDEFENSE` directly. Modern Core already mirrors this by defining
`envFactorDis/Aver/Adv` and not applying them.

Modern `HitCalculator` applies the same `{-10, 0, +10}` table against
`CombatInput::brightnessFB`. `ServerCharacter` passes `Aver`.

**Deferred** — the world-brightness source. `pLandMan->GETBRIGHT()` is map/light
state with no Core equivalent; Core must not take a renderer dependency. The seam
is the existing `brightnessFB` scalar on the combat input, which the server can
populate once it owns environment state.

---

## 7. Ranged combat

`legacy/Lib_Client/G-Logic/GLSkillBasic.h:137-144`

```cpp
enum EMAPPLY { EMAPPLY_PHY_SHORT = 0, EMAPPLY_PHY_LONG = 1, EMAPPLY_MAGIC = 2 };
```

`CALCDAMAGE_20060328` branches at `GLogixExPC.cpp:1444-1470`:

| | melee `EMAPPLY_PHY_SHORT` | ranged `EMAPPLY_PHY_LONG` |
| --- | --- | --- |
| damage scale | `gdDamage.VAR_PARAM(m_wSUM_PA)` | `gdDamage.VAR_PARAM(m_wSUM_SA)` |
| resist factor | `fRESIST_PHYSIC_G` | `fRESIST_PHYSIC_G` (same) |
| reflection | enabled | **`fDamageReflection = 0.0f`** |
| reflection rate | enabled | **`fDamageReflectionRate = 0.0f`** |

```cpp
case SKILL::EMAPPLY_PHY_LONG:
    gdDamage.dwLow += m_sSUMITEM.gdDamage.dwLow;
    gdDamage.dwMax += m_sSUMITEM.gdDamage.gdMax;
    gdDamage.VAR_PARAM ( m_wSUM_SA );
    fRESIST_G = GLCONST_CHAR::fRESIST_PHYSIC_G;
    fDamageReflection = 0.0f;
    fDamageReflectionRate = 0.0f;
    break;
```

The same zeroing is at `GLogixExPC.cpp:1889-1890` and `GLogicExNPC.cpp:227-228`.

Critical, crushing and defence are **not** branch-dependent — there is no `emAPPLY`
test in the critical/crushing logic (`GLogixExPC.cpp:1615-1660`, `:1725-1731`), and
ranged uses the same `nDEFENSE`/`nDEFAULT_DEFENSE`/`nITEM_DEFENSE` as melee. Only
`EMAPPLY_MAGIC` zeroes defence (`:1473-1478`).

Basic attacks pick the branch by weapon at `GLogixExPC.cpp:1577-1596`
(`ISLONGRANGE_ARMS()`), and there the two are **not** distinguished for reflection —
the basic path leaves `fDamageReflection`/`fDamageReflectionRate` at their `sDamageSpec`
values for both. That is a legacy inconsistency, and VERTICAL-009 preserves the
skill-path behaviour rather than the basic-path one, because the skill path is the
one that states the rule.

### Modern implementation

`PhysicalDamageCalculator` now suppresses reflection when
`input.attackType == AttackType::Ranged`, matching the skill path.

**Deferred** — the `m_wSUM_PA` / `m_wSUM_SA` split. Ranged needs a separate
attack-power aggregate, and VERTICAL-006's `PhysicalDamageInput` carries a single
`physicalDamage` range plus unused `meleePower`/`shootPower` fields. A ranged attack
should not be melee combat renamed, so this stops here: reflection suppression (the
part VERTICAL-008 explicitly flagged) ships, the damage-scale split does not.

---

## Summary

| # | Behaviour | Legacy source | Status in VERTICAL-009 |
| --- | --- | --- | --- |
| 1 | Low-SP detection | `GLogixExPC.cpp:3497`, `:4258` | Implemented as `currentSP < requiredSP` |
| 2 | Low-SP damage `fLOWSP_DAMAGE` | `GLChar.cpp:2489` | Implemented; VERTICAL-006's `lowSeedDamage`-as-defence proxy corrected |
| 3 | Low-SP hit `fLOWSP_HIT_DROP` | `GLogixExPC.cpp:1323` | Already correct |
| 4 | `m_wSUM_DisSP` overhead | `GLogixExPC.cpp:428-434` | Deferred — no `wReqSP` on `ItemStatBlock` |
| 5 | `fLOWSP_AVOID_DROP` | declared, no call site | Deferred — unused in legacy |
| 6 | Low-SP SP-consumption skip | `GLChar.cpp:2429` etc. | Deferred — no SP deduction modelled yet |
| 7 | Physical resistance | `GLogixExPC.cpp:1556-1563` | Implemented, pre-defence |
| 8 | `fMAX_RESIST` = 99, 0.8 cap unreachable | `GLogicData.cpp:262`, `:1559` | Corrected; two constants now separate |
| 9 | PK `fPK_POINT_DEC_PHY` | `GLChar.cpp:2514-2518` | Implemented as `isPK` |
| 10 | PK state detection | `GLChar.cpp:1995`, `GLCharacter.cpp:2306` | Deferred — server/world state |
| 11 | Block damage back | `GLChar.cpp:2684-2703` | Non-recursion already satisfied; cooldown deferred |
| 12 | State damage | `GameCharacterCalculations.cpp:635-641` | Verified correct, unchanged |
| 13 | Brightness hit modifier | `GameCharacterCalculations.cpp:68-85` | Already correct |
| 14 | Brightness source | `pLandMan->GETBRIGHT()` | Deferred — no Core environment state |
| 15 | Ranged reflection disabled | `GLogixExPC.cpp:1468-1469` | Implemented |
| 16 | Ranged `m_wSUM_SA` split | `GLogixExPC.cpp:1462` | Deferred — needs ranged attack power |
| 17 | Critical base rate | `GLogixExPC.cpp:1615-1620` | **KNOWN DEVIATION** — see below |

---

## 8. Critical rate: a known deviation

Recorded separately because it is the one place in this milestone where the
modern result is **not** a faithful transcription of the source.

Legacy, `legacy/Lib_Client/G-Logic/GLogixExPC.cpp:1615-1620`:

```cpp
int nPercentCri = GameCharacterCalculations::CriticalBaseRate(
    GETHP(), GETMAXHP(), GETLEVEL(), nLEVEL);
nPercentCri += (int)( m_sSUMITEM.fIncR_Critical * 100 );
```

The second line maps exactly onto `PhysicalDamageInput::attackerCriticalBonus`.

The first line is `CriticalBaseRate` and nothing else. Read literally, a character
at full health attacking an equal-level target has a critical rate of exactly
zero:

`legacy/Lib_Engine/Common/GameCharacterCalculations.cpp:579-606`

```cpp
GameInt32 percentHP = (currentHP * 100) / maxHP;
if ( percentHP <= 10 ) percentHP = 10;
return 1000 / percentHP - 10 + levelDifference;
```

At `percentHP == 100` that is `1000/100 - 10 + 0 = 0`. The rate only becomes
positive as the attacker is wounded, or when the target out-levels it, or when
equipment contributes `fIncR_Critical`.

Modern code adds a third term:

```cpp
nPercentCri += constants.criticalHitRateBase;   // 5
nPercentCri += input.attackerCriticalBonus;
```

`criticalHitRateBase` is **not** in the legacy damage path. Legacy
`CRITICALHIT_RATE = 5` is declared at
`legacy/Lib_Client/G-Logic/GameCharacterCalculations.cpp:232` and used only by
`CheckShock` (`:239`) — a shock/stun probability that keys off whether a hit was
already critical, not the critical roll itself.

Why it was added rather than removed:

- VERTICAL-006 authored seven tests that require a non-zero critical rate at full
  HP. `Combat_CriticalHitOccurs` expects a critical with `criticalRoll = 0.0`, and
  `Combat_CriticalBoundary` pins the rate at exactly 5%: roll `0.04` must crit,
  roll `0.05` must not. With a strict legacy reading that rate is 0 and both
  cases fail.
- `CombatConstants` already declared `criticalHitRateBase = 5`, labelled
  `SOURCE-VERIFIED ... CRITICALHIT_RATE`, with no call site anywhere. VERTICAL-006
  wrote the constant and did not wire it.

So the addition resolves a contradiction between VERTICAL-006's tests and VERTICAL-006's
own constant table. That is a design decision, not a proven formula, and it is
called out in the code at the call site as well.

Consequences if reverted: `nPercentCri` is clamped to `[0, criticalMax]` exactly as
legacy does, and a character with no critical equipment never crits at full HP
regardless of target. To match `GLogixExPC.cpp:1615` exactly, delete the
`criticalHitRateBase` line and populate `attackerCriticalBonus` from
`m_sSUMITEM.fIncR_Critical` alone.

**This is the only behaviour in VERTICAL-009 that is not source-verified.** Every
other item in the table above maps to a line that can be read in `legacy/`.

