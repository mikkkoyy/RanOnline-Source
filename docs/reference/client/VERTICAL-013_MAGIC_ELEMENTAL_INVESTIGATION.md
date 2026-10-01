# VERTICAL-013 — Magic / elemental combat

## Summary

Magic is a **distinct damage channel**, not the physical formula with a flag.
The two agree from the damage roll onward and disagree completely before it, so
this milestone adds `AttackType::Magic` and `CalculateMagicDamage` rather than a
`bool isMagic` on the existing calculator. It also closes two defects found on
the way: attack power reaching the combat boundary without being used (the exact
VERTICAL-012 bug, still latent for magic), and `targetResistElement` being
hardcoded to `0` for every active skill since VERTICAL-011.

Executed scope is deliberately narrow:

```
EMAPPLY_MAGIC + EMFOR_HP + fBASIC_VAR < 0 + TAR_SPEC + SIDE_ENEMY
```

Single-target hostile HP-damaging magic. Every other shape in the magic switch
is an explicit refusal with a named reason.

---

## 1. Repository check

Started from `main` at `7b86e275469952a8d441c5440a299d34022a3831`, matching
`origin/main`, working tree clean.

## 2. Public / forum backread — negative

Searched for RAN magic internals (`m_wSUM_MA`, `EMAPPLY_MAGIC`, `fRESIST_G`,
magic damage formulas). Every result was a different game: MapleStory, RuneScape,
Disgaea, Elder Scrolls Online, Heroes of Might and Magic III, Ragnarok 3,
Iruna Online. Nothing RAN-specific, nothing implementation-level.

This is the second consecutive milestone where the public web yielded nothing
usable. The conclusion worth acting on is that **future migrations should plan
on the checked-in legacy tree**, not on external sources. Nothing from this
backread is cited below as evidence.

## 3. Magic entry point and `EMAPPLY_MAGIC`

`GLCHARLOGIC::CALCDAMAGE_20060328`, `GLogixExPC.cpp:1473-1487`:

```cpp
case SKILL::EMAPPLY_MAGIC:
    nDEFENSE          = 0;
    nDEFAULT_DEFENSE  = 0;
    nITEM_DEFENSE     = 0;
    gdDamage.VAR_PARAM ( m_wSUM_MA );
    fRESIST_G          = GLCONST_CHAR::fRESIST_G;
    fDamageReduce      = sDamageSpec.m_fMagicDamageReduce;
    fDamageReflection  = sDamageSpec.m_fMagicDamageReflection;
    fDamageReflectionRate = sDamageSpec.m_fMagicDamageReflectionRate;
    bPsyDamage = false;
```

Six differences from physical in eight lines. Everything after this block is
shared code.

## 4. What makes magic a separate function

| Stage | Physical | Magic |
| --- | --- | --- |
| Weapon item damage added to range | Yes, `:1448` / `:1460` | **No** |
| Attack power | `m_wSUM_PA` / `m_wSUM_SA` | `m_wSUM_MA`, `:1477` |
| Resistance applied | to the **rolled** value | to the **range**, `:1562-1563` |
| Resistance form | `x * (1 - f)` | `x -= (DWORD)(x * f)` |
| Resistance factor | `fRESIST_PHYSIC_G` | `fRESIST_G`, `:1481` |
| `nDEFENSE` / body / item | all applied | all forced to `0`, `:1474-1476` |
| Damage reduce | `m_fPsyDamageReduce` | `m_fMagicDamageReduce`, `:1482` |
| Reflection | psy values; suppressed when ranged | magic values, `:1483-1484` |
| Reflection flag | `DAMAGE_TYPE_PSY_REFLECTION` | `DAMAGE_TYPE_MAGIC_REFLECTION`, `:1756` |

Forcing these through one function with flags would have hidden exactly the
distinctions this milestone exists to establish.

### The resistance ordering is not a detail

Legacy applies magic resistance to `gdDamage.dwLow` / `dwHigh` **before** the
roll, subtractively:

```cpp
gdDamage.dwLow  -= (DWORD) ((float) gdDamage.dwLow  * fResistTotal);
gdDamage.dwHigh -= (DWORD) ((float) gdDamage.dwHigh * fResistTotal);
```

Physical applies it to the already-rolled `nDAMAGE_OLD` multiplicatively. On a
flat range the two agree numerically, so this is easy to get wrong and hard to
notice. `Magic_ResistanceRunsBeforeTheRollAndPhysicalResistanceDoesNot` pins it
by asserting that `rawDamage` moves with resistance for magic and does **not**
move for physical.

## 5. `m_wSUM_MA`

`GLogixExPC.cpp:319`, `:331-332`, `:2972`, `:3015`:

```cpp
m_wMA = WORD ( m_sSUMSTATS.wDex * fMA_DEX
             + m_sSUMSTATS.wSpi * fMA_SPI
             + m_sSUMSTATS.wInt * fMA_INT );

int nSUM_MA = m_sSUMITEM.nMA + m_sSUM_PASSIVE.m_nMA + m_dwEnergyIncrease;
GLOGICEX::VARIATION ( m_wMA, USHRT_MAX, nSUM_MA );
m_wSUM_MA = m_wMA + nSUM_MA;
if ( m_wSUM_MA >= 50000 )  m_wSUM_MA = 1;
```

**Magic attack has no class/level term.** PA and SA both have
`(wBEGIN_PA + fLVLUP_PA*ZBLEVEL) * fCONV_PA`; MA has only the three stat terms.
Its codex field is `m_dwEnergyIncrease`, not `m_dwMeleeIncrease` /
`m_dwShootingIncrease`.

Modern `DerivedStats::magicAttack` **already matched this exactly**
(`StatCalculator.cpp:192-205`), including the absence of a level term. Reused
as-is; no new representation of MA was created. The comment there claiming "no
VARIATION clamp" is wrong — legacy does call `VARIATION`, and the code correctly
does too. Only the comment was corrected.

`ApplyAttackPower` moved from a private helper in `PhysicalDamageCalculator.h`
to `CombatTypes.h`, because legacy uses the identical `VAR_PARAM` for PA, SA and
MA. One implementation, four legacy call sites.

## 6. Skill `fBASIC_VAR` and weather

`GLogixExPC.cpp:1520-1531`:

```cpp
float fSKILL_VAR = sSKILL_DATA.fBASIC_VAR;
int nVAR = abs ( int(fSKILL_VAR*fPOWER) );
float fGrade = (float) wGRADE / GLCONST_CHAR::fDAMAGE_GRADE_K;
gdDamage.dwLow  += DWORD (nVAR + ((float) gdDamage.dwLow  * fGrade));
gdDamage.dwHigh += DWORD (nVAR + ((float) gdDamage.dwHigh * fGrade));
```

Truncation happens on the float product **before** `abs`, so `-0.5` yields 0 and
not 1. Pinned by `Magic_SkillMagnitudeIsAbsOfTheWeatherScaledProduct`.

### Weather

`GLOGICEX::WEATHER_ELEMENT_POW` returns exactly `1.0f` when weather is inactive
(`GameCharacterCalculations.cpp:461-462`), which confirms VERTICAL-011's
injected `1.0f` default. The world/weather provider does not exist, so the value
stays an injected deterministic input. **No weather subsystem was created.**

## 7. Element selection and `EMELEMENT_ARM`

`GLogixExPC.cpp:1504-1513`:

```cpp
EMELEMENT emELMT(EMELEMENT_SPIRIT);
if ( pSkill->m_sAPPLY.emELEMENT == EMELEMENT_ARM ) {
    SITEM *pITEM = GET_ELMT_ITEM ();
    if ( pITEM )  emELMT = STATE_TO_ELEMENT(pITEM->sSuitOp.sBLOW.emTYPE);
} else {
    emELMT = pSkill->m_sAPPLY.emELEMENT;
}
```

Two behaviours that matter:

1. `EMELEMENT_ARM` is a **selector**, not an element — it inherits the
   caster's weapon's blow element.
2. The default is Spirit and `ARM` only overrides it when a weapon is actually
   found, so a missing weapon yields Spirit rather than a refusal.

Core cannot read legacy item types, so the weapon element is an input boundary
(`ActiveSkillInput::weaponElement`) and resolution lives in the resolver.

Modern `SkillElement` is a new enum. `Stats::Resistances` carries only five axes
(fire, ice, electric, poison, spirit); stone, mad, curse and zen have no field
and read `0` rather than borrowing another element's number. Widening that
struct is a data-model change, and inventing four balance values would be
fabrication.

## 8. Critical, crushing, low-SP

**Critical** — no magic-specific rule. `CriticalBaseRate` and
`dwCRITICAL_DAMAGE` are shared (`:1615-1620`, `:1729`). The `criticalHitRateBase`
addition is VERTICAL-009's documented deviation and is inherited.

**Crushing** — `EMSPECA_CRUSHING_BLOW` is added in a loop at `:1494-1501` that
sits **outside** the apply switch, so magic skills can carry it. Implemented as
`MagicDamageInput::skillCrushingBonus`.

**Low-SP** — this needed care, because VERTICAL-012 warned about double-halving
and the risk is real here.

`CALCDAMAGE` applies **no** low-SP damage penalty. `fLOWSP_DAMAGE` appears
nowhere in `GLogixExPC.cpp`. Instead it is applied exactly once, in one of two
places depending on entry point:

- basic attacks: `GLChar::DamageProc`, `GLChar.cpp:2489`
- skills: `SkillProc`, `GLChar.cpp:3131`, *after* `CALCDAMAGE` returns

Modern models that single 0.5 factor inside the calculator for physical
(VERTICAL-009). Magic follows the same choice so both channels agree.
`Magic_LowSpIsAppliedOnceNotTwice` exists specifically to catch a future
double-halving.

## 9. `bPsyDamage` — a legacy bug, recorded not "fixed"

`GLogixExPC.cpp:1740-1741`:

```cpp
if ( bPsyDamage ) dwDamageFlag += DAMAGE_TYPE_PSY_REDUCE;
else              dwDamageFlag += DAMAGE_TYPE_PSY_REDUCE;
```

**Both branches add the psy flag.** The `bPsyDamage` test has no effect, so a
magic hit that is reduced is tagged `DAMAGE_TYPE_PSY_REDUCE` rather than a magic
flag. This is reproduced faithfully rather than corrected, because correcting it
would change observable behaviour away from RAN. Flagged here and in the code.

Reflection at `:1755-1756` *does* branch correctly and magic gets
`DAMAGE_TYPE_MAGIC_REFLECTION`.

## 10. Deliberate departures

1. **Item grade omitted.** The `wGRADE / fDAMAGE_GRADE_K` term (`:1527-1531`) is
   absent because modern `ItemDefinition` has no grade. The skill magnitude
   itself *is* included; only the grade-weighted part is missing. Inventing a
   grade would be fabrication.
2. **`fLOW_SEED_DAMAGE` in the magic low-seed branch.** Magic computes
   `nNetDAMAGE = int(nDAMAGE_OLD * (1 - fLOW_SEED_DAMAGE) - nDEFENSE)`
   (`:1678`), faithfully. VERTICAL-009's physical path omits the `0.95` factor
   and computes `nDAMAGE_OLD - nDEFENSE`. Magic therefore does **not** inherit
   that deviation. Fixing physical would regress V009–V012 expectations, so it
   is recorded here rather than changed silently.
3. **Legacy's MP/SP arithmetic appears wrong.** `GLChar.cpp:3099`:
   `nVAR_MP -= (int)( nVAR - (nVAR*nRESIST/100.0f*fRESIST_G) )` algebraically
   reduces to `nVAR - nVAR + int(nVAR*resist/100*fRESIST_G)`, i.e. the *resisted
   fraction* rather than the damage — resistance appears inverted. Not
   implemented, so not reproduced; recorded so whoever implements MP/SP damage
   does not copy it blindly.

## 11. Explicitly deferred

| Area | Reason |
| --- | --- |
| HP heal (`fBASIC_VAR > 0`) | `UnsupportedEffect`. Needs the resource-effect path; a heal is not negative damage. |
| `EMFOR_MP` / `EMFOR_SP` | `UnsupportedEffect`. Separate arithmetic, never reaches `CALCDAMAGE`. |
| Stone/mad/curse/zen resistance | No field in `Stats::Resistances`. |
| Weather provider | No world subsystem; injected value retained. |
| `TAR_ZONE`, `TAR_SELF_TOSPEC`, realm | Need positions and an entity registry. |
| Projectiles, travel time | Client presentation. Damage resolves at the combat boundary. |
| Strike division (`dwDivCount`) | Animation data (`m_wDivCount`). |
| Shock (`CHECKSHOCK`) | Out of scope for this channel. |

Cost and effect are kept distinct: `wUSE_MP` is the caster paying to cast and is
charged through VERTICAL-011's authority; `EMFOR_MP` is the target losing MP and
is refused. `MagicSkill_MpCostIsChargedButMpEffectIsRefused` proves both halves
at once.

## 12. Verification

| Suite | Result |
| --- | --- |
| ModernCoreTests | 337 (was 303; +34) |
| ModernServerTests | 70 (unchanged count, one case rewritten) |
| CTest | 14/14 Debug and Release |
| Client suites | 12/12 |
| Builds | Debug and Release, 0 errors, 0 warnings |

Test-count increases: 21 magic cases in `CombatTests.cpp` and 13 resolver cases
in `ActiveSkillTests.cpp`.

`ActiveSkill_MagicApplyRejected` was retired because it asserted the deferral
this milestone removes; `ServerActiveSkill_UnsupportedSkillIsRefusedNotFaked` was
rewritten because its magic skill now legitimately executes.

---

## References

- `legacy/Lib_Client/G-Logic/GLogixExPC.cpp:319, 331-332, 1399-1411, 1442-1492, 1494-1501, 1504-1517, 1520-1531, 1545-1553, 1556-1570, 1600-1608, 1615-1620, 1652-1660, 1672-1689, 1701-1713, 1725-1742, 1746-1763, 1777-1793, 2489, 2972, 3015`
- `legacy/Lib_Client/G-Logic/GLChar.cpp:2484-2491, 3070-3123, 3129-3134`
- `legacy/Lib_Engine/Common/GameCharacterCalculations.cpp:456-491, 554-563`
- `legacy/Lib_Engine/G-Logic/GLDefine.h:364-371`
- `legacy/Lib_Client/G-Logic/GLogicData.cpp:242, 260-262`
- `legacy/Lib_Client/G-Logic/GLCharData.h:262-273`
- `modern/core/combat/MagicDamageCalculator.h`
- `modern/core/combat/CombatTypes.h`
- `modern/core/stats/StatCalculator.cpp:168-206`
- `docs/reference/client/VERTICAL-009_PHYSICAL_COMBAT_INVESTIGATION.md`
- `docs/reference/client/VERTICAL-012_RANGED_PHYSICAL_INVESTIGATION.md`