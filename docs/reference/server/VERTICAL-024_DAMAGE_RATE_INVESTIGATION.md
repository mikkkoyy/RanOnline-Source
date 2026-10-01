# VERTICAL-024 - DAMAGE_RATE FACT Investigation + Integration

## 1. Baseline

| | |
| --- | --- |
| Repository | `mikkkoyy/RanOnline-Source` |
| Branch | `main` (tracking `origin/main`) |
| Baseline commit | `135c3b5eadc39afc27476dc44248d261d0387110` (VERTICAL-023) |
| Starting tree | clean |

## 2. Public / Forum Backread

**PROVENANCE ONLY - NOT BEHAVIORAL EVIDENCE.**

The same RaGEZONE thread located in VERTICAL-022/023 quotes `EMIMPACT_ADDON` in
full and confirms `EMIMPACTA_DAMAGE_RATE = 9`. Nothing beyond the enum was found:
no RAN source mirror, wiki page or forum thread describing the mechanic. RAN's
public documentation does not cover skill FACT internals.

**No formula, ordering or pipeline claim below rests on public material.**

## 3. Legacy Occurrences

`EMIMPACTA_DAMAGE_RATE` is one of the **live** impacts - unlike VERTICAL-023's
enum 19-23, it appears in every runtime accumulation switch. Complete
enumeration:

| Location | Kind |
| --- | --- |
| `GLCharDefine.h:982` | enum declaration, `= 9` |
| `GLogixExPC.cpp:1041-1043` | **passive skill** accumulator |
| `GLogixExPC.cpp:2340` | **skill FACT** accumulator |
| `GLogixExPC.cpp:2767` | **item FACT** accumulator |
| `GLogixExPC.cpp:2889` | **system buff** accumulator |
| `GLogicExNPC.cpp:537-538` | NPC / crow accumulator |
| `GLSummon.cpp:704-705` | summon accumulator |
| `GLSkillToolTip.cpp:2465` | tooltip (presentation) |
| `SkillInforToolTip.cpp:1715`, `UISkillInfoLoader.cpp` ×4 | tooltip / UI (presentation) |

## 4. Accumulator, Seed, Reset

| Property | Location |
| --- | --- |
| Field | `m_fDamageRate`, `float` - `GLogicEx.h:417` |
| Constructor seed | `1.0f` - `GLogicEx.h:540` |
| Per-tick reset | `m_fDamageRate = 1.0f;` - `GLogixExPC.cpp:120` |
| Permanent seed | `m_fDamageRate = 1.0f + m_sSUM_PASSIVE.m_fDAMAGE_RATE;` - `:2219` |
| Passive source field | `m_sSUM_PASSIVE.m_fDAMAGE_RATE`, `float`, `= 0` - `GLCharData.h:1146`, `:1190` |

So it is **a multiplier around 1.0, not a percentage** - exactly like
`m_fDefenseRate`, and for the same structural reason.

## 5. Contributors

| Contributor | Legacy location | Operation | Order | Modern equivalent |
| --- | --- | --- | --- | --- |
| Permanent passive | `:1042` | `m_sSUM_PASSIVE.m_fDAMAGE_RATE += fADDON` | first (seeded into the per-tick value at `:2219`) | `PassiveContribution::damageRate` |
| Skill FACT | `:2340` | `m_fDamageRate += fADDON_VAR` | after the passive seed | `FactContribution::damageRate` |
| Item FACT | `:2767` | `m_fDamageRate += fADDON_VAR` | after skill FACT | deferred - no item-FACT subsystem |
| System buff | `:2889` | `m_fDamageRate += fADDON_VAR` | after item FACT | deferred - no system-buff subsystem |
| Pet skill FACT | `:2582`, `:2608` | `m_fDamageRate += fMVAR / 100.0f` | - | deferred - no pet subsystem |
| QITEM | `:2550`, `:2558` | `m_fDamageRate += wParam1 / 100.0f` | - | deferred - no QITEM subsystem |
| GM event | `:2565` | `m_fDamageRate += wAttack / 100.0f` | - | deferred - no event subsystem |
| Land effect | `:2647` | `m_fDamageRate += landEffect.fValue` | - | deferred - no land subsystem |
| Equipment | **no field** | - | - | **not a contributor** - `m_sSUMITEM` has no damage-rate member |
| Codex | `:380` folds `m_dwAttackIncrease` flat into `m_gdDAMAGE` | - | - | **not a contributor** |

Two proven absences carry weight. Equipment and codex contribute **flat** damage
only, which V019 already implemented - the same finding V021 reached for defence.

Note the deliberate inconsistency, which legacy does not normalise: the pet,
QITEM and event paths divide by 100 while the passive and FACT paths do not. A
"rate" of 0.25 means a 25% boost from a skill but 0.25% from a pet. Modern
preserves the raw addition for the sources it models, exactly as V021 did for
defence.

## 6. Formula

```
gdDamage.dwLow  = ApplyDamageRate(gdDamage.dwLow,  m_fDamageRate);
gdDamage.dwHigh = ApplyDamageRate(gdDamage.dwHigh, m_fDamageRate);
```

`ApplyDamageRate` (`GameCharacterCalculations.cpp:663-669`):

```cpp
return static_cast<GameUInt32>(static_cast<float>(damage) * damageRate);
```

A multiplier around 1.0 applied to **both ends of the range**, truncated toward
zero. Not a flat increase, not a percentage of the final damage, not a roll-time
multiplier.

## 7. Pipeline Position

`CALCDAMAGE` (`GLogixExPC.cpp:1329`) dispatches at compile time on a country
macro to `CALCDAMAGE_20060328` (`:1363`) or `CALCDAMAGE_2004` (`:1799`). **Both
apply the rate at the same structural point**, so the conclusion does not depend
on which is built. From `CALCDAMAGE_20060328`:

| # | Stage | Location |
| --- | --- | --- |
| 1 | `gdDamage = m_gdDAMAGE_SKILL` | `:1415` |
| 2 | charm-item damage added | `:1430-1431` |
| 3 | per-apply: item damage, `VAR_PARAM(PA/SA/MA)` | `:1448-1477` |
| 4 | skill damage reduction | `:1551-1552` |
| 5 | **resistance** (`dw -= DWORD(dw * fResistTotal)`) | `:1562-1563` |
| 6 | clamp ends to >= 0 | `:1567-1570` |
| 7 | **`ApplyDamageRate`** | **`:1600-1603`** |
| 8 | `RandomDamageRange` (the roll) | `:1672-1673` |
| 9 | low-seed factor + defence subtraction | `:1678-1686` |
| 10 | state damage | `:1688` |
| 11 | defence-rate (`fFinalRate`, body x item) | `:1701-1713` |
| 12 | critical / crushing multipliers | `:1725-1731` |
| 13 | damage reduction | `:1734-1738` |
| 14 | reflection | `:1746-1763` |
| 15 | floor at 1 | `:1777-1782` |

**The rate is applied at stage 7 - after the attack power and after
resistance, immediately before the roll.** That is the single most important
fact about this axis.

`CALCDAMAGE_2004` is the same shape: rate at `:1958-1961`, roll at `:2004`.

### What follows from stage 7

* **Critical**: stage 12, so the rate is **pre-critical**. A rate raises the
  rolled figure and the critical multiplier then scales that.
* **Defence**: stage 11, so **pre-defence**. Raising the rate does not change the
  absolute defence subtracted, so it has a larger proportional effect as defence
  rises.
* **Resistance**: stage 5, so **post-resistance** (and the two do not commute on
  magic - see section 10).
* **Low-SP**: stage 9 is the low-seed factor; the `bLowSP` multiplier itself is
  in the defender's `PreStrikeProc` (`GLChar.cpp:2488-2491`) acting on the
  rolled figure. So **pre-low-SP** - and the two never interact arithmetically.
* **Reflection**: stage 14 computes from the post-critical figure, so a rate
  increases what is reflected.

## 8. Clamps, Negative Values, Integer Conversion

**There is no clamp in the live code.** Legacy authored one and then commented
the entire block out:

```cpp
/*if ( m_fDamageRate >= 1.0f ) { ... }
else {
    ... 
    if ( m_fDamageRate <= 0.0f )  { dwLow = 0; dwHigh = 0; }
}*/
                                            // GLogixExPC.cpp:1634-1650
                                            // and again at :1963-1977
```

So the clamp that would have handled a non-positive rate **never runs**.

| Value | Behaviour |
| --- | --- |
| `rate = 1.0` | identity - `trunc(dw * 1.0f) == dw` |
| `rate > 1` | scales up, truncated |
| `0 < rate < 1` | scales down, truncated; `rate = 0` gives 0 and the floor at stage 15 yields damage 1 |
| `rate < 0` | see below |

### The negative case, measured rather than assumed

The first draft of the test asserted that a negative rate "wraps to a huge
unsigned figure". **That was wrong**, and measuring it changed the conclusion:

1. `static_cast<uint32_t>(-100.0f)` on MSVC **does** wrap two's-complement to
   `0xFFFFFF9C` (verified with a standalone probe).
2. But the roll then converts the range end back to float
   (`static_cast<float>(damage.low)`), and `0xFFFFFF9C` = 4294967196 is **not
   representable** - floats near 2^32 are spaced 512 apart, so it rounds to
   `2^32` exactly.
3. `static_cast<uint32_t>(2^32)` is out of range, and MSVC yields **0**.

End to end a negative rate therefore collapses the range to zero and the damage
floors at 1 - the same outcome as `rate = 0`. Reproduced as measured, and pinned
by `DamageRate_NegativeRateWrapsThenCollapsesToZeroThroughTheRoll`, which
asserts *both* conversions so the rounding that decides the outcome is visible.

Conversions, in order:

| Boundary | Operation |
| --- | --- |
| rate application | `float -> uint32` via `static_cast`, truncation toward zero |
| roll | `uint32 -> float -> uint32`, truncating toward the low end |
| defence subtraction | `int` |
| critical / crushing | `int(r * pct / 100)` |
| reduction / reflection | `int` |

## 9. Stacking and Expiry

**SUM**, float, added raw - `+=` with no cast and no comparison at every site
(`:1042`, `:2340`, `:2767`, `:2889`). This is unlike the V019 impacts, which are
`int()`-truncated, and unlike the V017 spec reductions, which are last-wins.
A negative value is legal and is carried faithfully; nothing sanitises it.

Permanent and timed sources share one accumulator, the per-tick `m_fDamageRate`
seeded at `1.0f + m_sSUM_PASSIVE.m_fDAMAGE_RATE` and then increased. Expiry
rebuilds from zero, which is the architecture V015-V023 already established.

## 10. Physical, Ranged and Magic

**Physical and ranged share the pipeline.** V012/V019 established that melee and
ranged differ only in which power `VAR_PARAM` receives, and both branches of
`CALCDAMAGE_20060328` converge on the single rate application at `:1600-1603`.
So one field, one operation, one position covers both - confirmed by
`DefenseRate_...`-style tests on `attackType` switching.

**Magic uses the same accumulator and the same stage**, reached through the
`EMAPPLY_MAGIC` branch at `:1473-1487`. This is not an assumption: the switch at
`:1444-1492` only selects the attack power and the resistance constant, and the
rate application at `:1600` sits after the branch closes.

Magic needs separate test coverage because its **resistance is applied to the
range before the roll** (`:1562-1563`) while physical's is applied to the rolled
value. The two therefore do not commute on magic, and the order is
**resist-then-rate**. Pinned by
`Magic_DamageRateAppliesAfterResistanceAndBeforeTheRoll`: with range 100,
resist 50 and rate 1.5, the correct answer is 112 and the reverse order gives
113.

## 11. A Dead Legacy Write

`pIncreaseEff->GetIncreaseDamageRate()` is added to `m_fDamageRate` at `:1630` -
**after** the range was already rated at `:1600`, and the only other rate
application in that function (`:1634-1650`) is commented out. So the
enhancement system's damage-rate bonus has **no effect** in
`CALCDAMAGE_20060328`. Recorded, not fixed: `ENHANCEMENT` is out of scope for
this milestone, and legacy is reference material.

## 12. Modern Ownership

**No `DerivedStats` field was added**, and this is the load-bearing decision.
Legacy reads `m_fDamageRate` only inside `CALCDAMAGE`; the stat pipeline never
sees it. Adding a derived statistic would have created a second, divergent
statement of a combat-boundary value - precisely the trap §23 of the brief warns
about, and precisely what V019 documented for `EMIMPACTA_DAMAGE`.

| Concern | Owner | Change |
| --- | --- | --- |
| Permanent source | `PassiveContribution::damageRate` | **new field** |
| Timed source | `FactContribution::damageRate` | **new field** |
| Accumulation | `SkillFactModifiers::damageRate`, both aggregators | **new field + 2 cases** |
| Passive aggregation | `PassiveContributionAggregator.cpp:198` | **replaced a `// Not in PassiveContribution` no-op** |
| Combat boundary | `CombatInput::attackerDamageRate` | already existed (V013, magic-only) |
| Physical consumer | `PhysicalDamageInput::damageRate` | **new field + application** |
| Magic consumer | `MagicDamageInput::damageRate` | already existed and already correct |
| Skill path | `ActiveSkillInput::attackerDamageRate` | **new field + wiring** |
| Server | `ServerCharacter::Attack` and `::CastSkill` | populate both |

Two things were already half-built and are the reason this was a wiring job
rather than a design: V013 had created `CombatInput::attackerDamageRate` and
assigned it to the magic input (`:192`), and `MagicDamageInput::damageRate`
already applied it at the correct legacy position. Nothing ever *supplied* a
value, so the whole axis was inert. The physical path had no field at all.

## 13. Implementation Decision

**IMPLEMENTED in full for every contributor modern models.**

| | |
| --- | --- |
| Legacy consumer | **PROVEN** - `CALCDAMAGE`, `:1600-1603` / `:1958-1961` |
| Formula | **PROVEN** - multiplier on both range ends, `trunc` toward zero |
| Pipeline position | **PROVEN** - stage 7, pre-roll, post-resistance, post-attack-power |
| Stacking | **PROVEN** - SUM, float, raw |
| Expiry | **PROVEN** - rebuild from zero |
| Modern owner | **PROVEN** - combat boundary, no derived field |
| Deterministic tests | **YES** - 14 added |

Deferred with their subsystems: item FACT, system buff, pet skill FACT, QITEM,
GM event, land effect.

## 14. Tests

14 added, all deterministic. No test compares two `CastSkill()` calls, because
each call advances the deterministic RNG sequence and the two would not share a
damage roll.

| Test | What it pins |
| --- | --- |
| `DamageRate_RateOneIsAnIdentityOnBothEnds` | the no-buff baseline is unmoved |
| `DamageRate_AppliesToTheRangeBeforeTheRoll` | **the ordering discriminator** - range 101..102 at rate 1.5 with roll 0.5 gives 152 pre-roll and 151 post-roll |
| `DamageRate_AppliesAfterTheAttackPower` | `(100+3)*1.5 = 154`, not `100*1.5+3 = 153` |
| `DamageRate_TruncatesTowardZero` | `101 * 1.333 = 134` |
| `DamageRate_BelowOneReducesWithoutAClamp` | 0.5 halves; 0.0 floors at 1 |
| `DamageRate_NegativeRateWrapsThenCollapsesToZeroThroughTheRoll` | both conversions in section 8 |
| `DamageRate_IsSeparateFromTheFactDamageImpact` | DAMAGE_RATE is not DAMAGE |
| `DamageRate_AndLowSpAreIndependent` | rate is upstream of the low-SP multiplier |
| `Magic_DamageRateAppliesAfterResistanceAndBeforeTheRoll` | 112 vs 113 - the magic-only ordering |
| `Magic_DamageRateOneIsAnIdentity` | magic baselines unmoved |
| `SkillFactV024_DamageRateStacksBySum` | SUM |
| `SkillFactV024_NegativeDamageRateIsCarriedFaithfully` | no sanitising at the aggregator |
| `SkillFactV024_FractionalDamageRateIsNotTruncated` | 2^-6 survives; no `int()` |
| `SkillFactV024_DamageRateAndDefenseRateAreIndependent` | sibling axes stay separate |
| `SkillFactV024_DamageRateIsRemovedOnExpiry` / `OneExpiryDoesNotDisturbTheSurvivingFact` / `BothAggregatorsAgreeOnDamageRate` | expiry and aggregator parity |
| `ServerFactV024_*` (3) | the full server path, and that no derived stat moves |

The oracle was **not** extended, and deliberately so: the rate never enters the
stat calculator, so there is nothing for the oracle to re-derive. Adding it
would have meant asserting that a stat which by design does not exist does not
exist - a test that passes by construction.

## 15. Known Pre-existing Deviations (Not Introduced Here)

Recorded, not touched, because §28 forbids changing completed behaviour without
a proven V024 dependency:

1. **Physical resistance position.** `PhysicalDamageCalculator` applies
   resistance to the *rolled* figure; legacy applies it to the *range* before the
   roll (`:1562`). This is a VERTICAL-009 characteristic. It does not affect
   DAMAGE_RATE's own position, which is correctly pre-roll in both, but it means
   physical resist/rate ordering differs from legacy. Fixing it would move V006
   and V009 baselines, so it needs its own milestone with its own evidence.
2. The `criticalHitRateBase` deviation is already documented at the call site in
   `PhysicalDamageCalculator.h` and is unrelated to this axis.

## 16. Build

| Suite | Result |
| --- | --- |
| Debug build | 0 errors, 0 warnings |
| Release build | 0 errors, 0 warnings |
| Core tests | 514/514 (was 497) |
| Server tests | 114/114 (was 111) |
| CTest Debug | 14/14 |
| CTest Release | 14/14 |

## 17. GitHub

See `docs/MODERNIZATION_STATUS.md` for the recorded commit.
