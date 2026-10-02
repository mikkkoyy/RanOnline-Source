# VERTICAL-025 - Physical Resistance Ordering

Correction milestone. VERTICAL-024 recorded a physical-resistance deviation and
deferred it; this proves the exact legacy behaviour and fixes it.

## 1. Baseline

| | |
| --- | --- |
| Repository | `mikkkoyy/RanOnline-Source` |
| Branch | `main` (tracking `origin/main`) |
| Baseline commit | `b72052e3fc4d6f8df6577b79ace6ea7d67cc66c9` (VERTICAL-024) |
| Starting tree | clean |

## 2. Public / Forum Backread

**NOT FOUND** for behavioural evidence. RAN's public documentation does not
cover elemental resistance ordering, and no source mirror or forum thread
discusses it. The legacy tree in this repository is the sole authority for every
claim below.

## 3. Legacy Physical-Resistance Location

`GLogixExPC.cpp:1556-1563`, inside `CALCDAMAGE_20060328`:

```cpp
{
    float fResistTotal = (float) ((float) nRESIST * 0.01f * fRESIST_G);   // :1558
    fResistTotal = fResistTotal > 0.8f ? 0.8f : fResistTotal;            // :1559

    gdDamage.dwLow  -= (DWORD) ((float) gdDamage.dwLow  * fResistTotal); // :1562
    gdDamage.dwHigh -= (DWORD) ((float) gdDamage.dwHigh * fResistTotal); // :1563
}
```

Inputs, both established earlier in the same function:

| Input | Location |
| --- | --- |
| `nRESIST` | `sRESIST.GetElement(emELMT)` at `:1515`, clamped to `fMAX_RESIST` at `:1516` |
| `fRESIST_G` | `fRESIST_PHYSIC_G` for `EMAPPLY_PHY_SHORT` / `EMAPPLY_PHY_LONG` (`:1455`, `:1467`); `fRESIST_G` for magic (`:1481`) |
| `emELMT` | resolved at `:1504-1513`, including the `EMELEMENT_ARM` weapon lookup |

## 4. Legacy Pipeline

Verified by brace-walking the function, not by reading names. `if ( pSkill )`
at `:1417` **closes at `:1571`**; the `else` at `:1572` runs to `:1597`.

| # | Stage | Location |
| --- | --- | --- |
| 1 | `gdDamage = m_gdDAMAGE_SKILL` | `:1415` |
| 2 | charm-item damage | `:1430-1431` |
| 3 | item damage + `VAR_PARAM(PA/SA/MA)` | `:1448-1477` |
| 4 | skill damage reduction (PC targets) | `:1549-1553` |
| 5 | **physical resistance** | **`:1556-1563`** |
| 6 | clamp ends to >= 0 | `:1567-1570` |
| — | *end of the skill branch* | *`:1571`* |
| 7 | `ApplyDamageRate` | `:1600-1603` |
| 8 | `RandomDamageRange` | `:1672-1673` |
| 9 | low-seed factor + defence subtraction | `:1678-1686` |
| 10 | state damage | `:1688` |
| 11 | defence-rate (`fFinalRate`) | `:1701-1713` |
| 12 | critical / crushing | `:1725-1731` |
| 13 | damage reduction | `:1734-1738` |
| 14 | reflection | `:1746-1763` |
| 15 | floor at 1 | `:1777-1782` |

The basic-attack `else` (`:1572-1597`) contains **item damage, `VAR_PARAM`, and
nothing else** - no resistance.

## 5. Three Deviations, Not One

VERTICAL-024 described this as an ordering problem. It is three, and only
correcting the first would have left the pipeline still wrong.

### 5.1 Position

Resistance ran on the **rolled** figure. Legacy runs it on the **range**, before
the roll - so the two truncations do not commute.

### 5.2 Formula

Modern used `dw * (1.0f - fResistTotal)`. Legacy **subtracts a truncated
product**: `dw -= (DWORD)(dw * fResistTotal)`.

These differ by up to one unit. `101 - (DWORD)(101 * 0.25)` is **76**;
`101 * 0.75` is **75**. The `MagicDamageCalculator` header had already noticed
the asymmetry and called physical's form "the magic path's multiplicative form
on the rolled value" - treating the deviation as a design choice rather than a
bug.

### 5.3 Scope

Modern applied resistance to **every** attack. Legacy applies it **only on the
skill path**, because the whole block sits inside `if (pSkill)`. A basic attack
is never resisted.

This is why three existing tests had to change: they used basic-attack fixtures
and passed only because resistance used to apply unconditionally.

## 6. Which `CALCDAMAGE` Is Authoritative

`CALCDAMAGE` (`:1329`) dispatches at compile time:

```cpp
#if defined(KRT_PARAM) || ... || defined(GS_PARAM) || defined(_RELEASED)
    return CALCDAMAGE_20060328( ... );
#else
    return CALCDAMAGE_2004( ... );
#endif
```

**This is ambiguous and worth stating plainly.** The committed
`Lib_Client.vcxproj` defines only `WIN32;NDEBUG;_LIB` / `WIN32;_DEBUG;_LIB` -
none of the country macros and no `_RELEASED` - so the `#else` branch, and
therefore `CALCDAMAGE_2004`, is what the project file as committed compiles.

The two variants are not interchangeable:

| | `CALCDAMAGE_2004` | `CALCDAMAGE_20060328` |
| --- | --- | --- |
| resistance target | only the skill magnitude `nVAR` (`:1941-1942`) | the **whole range** (`:1562-1563`) |
| 0.8 cap | none | yes (`:1559`) |
| basic attack | no resistance | no resistance |

**The correction here targets `CALCDAMAGE_20060328`, and the reasoning is:**

1. Modern has cited `CALCDAMAGE_20060328` as provenance continuously since
   VERTICAL-006, which is named in the header comment of
   `PhysicalDamageCalculator.h`. V009, V012, V013, V019, V021 and V024 all cite
   its line numbers.
2. The `0.8` reduction cap modern already implements **exists only in
   `CALCDAMAGE_20060328`** (verified by search - one occurrence, `:1559`). So
   modern's existing resistance formula was already bound to that variant.
3. `_RELEASED` is used throughout the codebase for release-only logging
   (`GLAgentServerMsg.cpp:405` and elsewhere) and is defined by a shipping build.
   Its absence from the committed project file reads as a development
   configuration, not as evidence that the shipping variant is `2004`.
4. Switching variants would invalidate six completed milestones, which §27
   protects.

**This is a reasoned choice, not a proven one.** If the shipping RAN build
actually compiles `CALCDAMAGE_2004`, then physical resistance should reduce only
the skill's own magnitude term and leave the base range alone - a materially
different model. That is a real open question and it should be settled against a
shipped build or a release configuration before V025's formula is treated as
final. What is *not* open, and is variant-independent, is that resistance runs
**before** the roll and **not at all** on basic attacks; both are corrected here.

## 7. Before / After

```
BEFORE (modern, wrong on three counts)

range
→ DAMAGE / attack power
→ DAMAGE_RATE
→ roll
→ physical resistance   (multiplicative, wrong form, wrong scope)
→ defence
→ defence rate
→ critical / crushing
→ reduction
→ reflection
→ floor


AFTER (matches CALCDAMAGE_20060328)

range
→ DAMAGE / attack power
→ physical resistance   (subtractive, on the range, skills only)
→ DAMAGE_RATE
→ roll
→ low-seed + defence
→ state damage
→ defence rate
→ critical / crushing
→ reduction
→ reflection
→ floor
```

## 8. Resistance Clamp

| Point | Legacy | Modern |
| --- | --- | --- |
| raw value | `nRESIST` from `SRESIST` | `resistElement` |
| aggregation | SRESIST, floored at 0 per axis by `LIMIT()` (`:2979`) | floored at 0 per axis by the stat calculator |
| pre-fold clamp | `if (nRESIST > fMAX_RESIST) nRESIST = fMAX_RESIST` (`:1516`) | same, `maxResist = 99` |
| fold clamp | `fResistTotal > 0.8f ? 0.8f` (`:1559`) | same, `maxResistReduction = 0.8f` |

**The V020 "no mid-fold clamp" finding stands.** Both clamps sit *outside* the
subtraction, and no third clamp was introduced.

With `fMAX_RESIST = 99` and `fRESIST_PHYSIC_G = 0.5`, the largest reachable
`fResistTotal` is `99 * 0.01 * 0.5 = 0.495`. **The 0.8 cap is unreachable in the
physical path** and only ever bites on magic (`fRESIST_G` with a higher raw
resistance). `PhysicalResist_HighValuesClampRatherThanWrap` pins that the raw
clamp is the one that bites.

### The `:1567-1570` clamp is dead code

```cpp
if (gdDamage.dwLow  < 0) gdDamage.dwLow  = 0;
if (gdDamage.dwHigh < 0) gdDamage.dwHigh = 0;
```

`gdDamage.dwLow` is a `DWORD`, so `< 0` is never true. The guard can never fire.
It is also unreachable in practice: `fResistTotal` is non-negative (SRESIST is
floored at 0) and capped at 0.8, so the truncated product is always less than the
range end and no unsigned wrap is possible.

**Recorded, not reproduced.** Adding a live clamp would invent behaviour legacy
does not have.

## 9. Integer Conversion

| Boundary | Operation |
| --- | --- |
| resistance fold | `float(dw) * fResistTotal` then `(DWORD)`, truncating toward zero |
| subtraction | unsigned 32-bit |
| roll | `uint32 → float → uint32`, truncating toward the low end |
| `fResistTotal` | `float`, built as `(float)((float) nRESIST * 0.01f * fRESIST_G)` |

The `(DWORD)` cast on the product is what makes the subtractive form differ from
the multiplicative one, and it is preserved exactly.

## 10. Negative Resistance

**Not reachable through any legitimate source.** `nRESIST` comes from
`SRESIST::GetElement`, and every element is floored at zero by
`m_sSUMRESIST_SKILL.LIMIT()` (`:2979`, `GLCharDefine.h:787-795`). Modern's
stat calculator floors every resistance axis at zero too.

There is therefore no legacy behaviour to reproduce, and **no clamp was
invented**. Modern's guard is `input.resistElement > 0`, so a negative is
treated as absent rather than as a damage bonus - pinned by
`PhysicalResist_NegativeResistanceIsNotModelled` and documented there as
recorded behaviour.

## 11. The Ordering Discriminator

The brief asks for values where the two orders differ. Range `101..102`,
resist 50 → `fResistTotal = 0.25`, roll `0.0`:

| Order | Computation | Result |
| --- | --- | --- |
| **legacy** (range → resist) | `{101-25, 102-25}` = `{76, 77}`, roll 0.0 | **76** |
| old modern (roll → resist) | `101` → `trunc(101 × 0.75)` | **75** |

One unit apart. Pinned by `PhysicalResist_ResistAppliesToTheRangeBeforeTheRoll`.

The wide-range case `{100..200}` at resist 50 happens to agree (both give 75 at
roll 0.0), which is exactly why the tight range is the real discriminator.
`PhysicalResist_WideRangeDiscriminatesTheTwoOrders` records that coincidence
rather than pretending the wide case proves anything.

## 12. Interaction Tests

| Interaction | Result | Test |
| --- | --- | --- |
| DAMAGE (V019) | applied first, so it **is** resisted: `100 + 40` → `140 - 35` = 105 | `PhysicalResist_DamageFactIsResistedToo` |
| DAMAGE_RATE (V024) | resist **first** then rate: `100 → 75 → trunc(112.5)` = 112; rate-first would give 113 | `PhysicalResist_ResistThenDamageRateThenRoll` |
| Defence (V020/V021) | unchanged - still after the roll | `PhysicalResist_DownstreamStagesAreUnchanged` |
| Critical / crushing | unchanged | `PhysicalResist_DownstreamStagesAreUnchanged` |
| Reflection | unchanged | not re-tested; downstream of the roll either way |
| Low-SP | unchanged | `PhysicalResist_DownstreamStagesAreUnchanged` |
| Ranged | same field, same operation, no second implementation | `PhysicalResist_RangedUsesTheSameResistance` |
| Magic | **untouched** | `PhysicalResist_MagicResistanceIsUnchanged` |

## 13. Existing Tests That Encoded the Deviation

§27 requires these be identified explicitly. Three did:

| Test | What it encoded | Fix |
| --- | --- | --- |
| `CombatResist_Positive` | asserted `preDefenseDamage < rawDamage`, which only held because `rawDamage` was captured **before** a post-roll resistance | `skillCast = true`, and the intent ("a resisted hit does less") is now asserted against an unresisted baseline captured from the calculator |
| `CombatResist_RawValueClampedToMax` | computed its expectation as `rawDamage * (1 - fResistTotal)` - the old multiplicative post-roll form | recomputed in legacy's subtractive pre-roll form, independently of the roll |
| `Combat_...` resistance-ordering test (`:2177`) | asserted flatly that physical `rawDamage` **must not move** when resistance changes, contrasting it with magic | physical half inverted to `rawDamage` **must** move; magic half left untouched |

None was weakened. The third one is the significant case: it documented the
deviation as if it were intended, so it had to be corrected rather than
adjusted.

## 14. Tests Added

12 new, all deterministic, none comparing two `CastSkill()` calls.

`PhysicalResist_ZeroLeavesTheRangeUnchanged` (mandatory regression),
`..._UsesTheSubtractiveLegacyForm`, `..._ResistAppliesToTheRangeBeforeTheRoll`
(the discriminator), `..._WideRangeDiscriminatesTheTwoOrders`,
`..._BasicAttacksAreNeverResisted`, `..._ResistThenDamageRateThenRoll`,
`..._DamageFactIsResistedToo`, `..._RangedUsesTheSameResistance`,
`..._HighValuesClampRatherThanWrap`, `..._NegativeResistanceIsNotModelled`,
`..._DownstreamStagesAreUnchanged`, `..._MagicResistanceIsUnchanged`.

## 15. Changes

| File | Change |
| --- | --- |
| `core/combat/PhysicalDamageCalculator.h` | resistance moved to the range, subtractive form, gated on `skillCast` |
| `core/combat/CombatTypes.h` | `PhysicalDamageInput::skillCast` |
| `core/combat/CombatCalculator.h` | `CombatInput::skillCast`, passed through |
| `core/skills/ActiveSkill.cpp` | `combat.skillCast = true` on the cast path |
| `tests/CombatTests.cpp` | 12 new + 3 corrected |

No `DerivedStats` field was touched; nothing moved out of the combat layer.

## 16. Build

| Suite | Result |
| --- | --- |
| Debug / Release build | 0 errors, 0 warnings |
| Core tests | 526/526 (was 514) |
| Server tests | 114/114 |
| CTest Debug / Release | 14/14 |

The correction is reachable in production: `ActiveSkill.cpp:454` already plumbs
`targetResistElement` on the cast path, and the basic-attack path leaves it at 0
with a pre-existing "not yet modelled" comment, which now happens to match
legacy rather than contradicting it.

## 17. Not Fixed Here

`criticalHitRateBase`, the enhancement damage-rate dead write (`:1630`),
`MOVEVELO`, `ATTACKVELO`, `STUN`, `CONTINUOUS_DAMAGE`, `PROHIBIT_POTION`,
`PIERCE`, `TARRANGE`, `INVISIBLE`, `RECVISIBLE`, `CHANGE_*_RANGE`, and magic
resistance in any form.

## 18. GitHub

See `docs/MODERNIZATION_STATUS.md` for the recorded commit.
