# VERTICAL-021 - Defense-Rate Investigation

Companion to `VERTICAL-021_DEFENSE_RATE_IMPLEMENTATION.md`, which records the
code changes and the reviewer notes. This file records the *investigation*:
where the legacy behaviour was found, what was proven, and what was proven
absent.

## 1. Baseline

| | |
| --- | --- |
| Repository | `mikkkoyy/RanOnline-Source` |
| Branch | `main` (tracking `origin/main`) |
| Baseline commit | `855f5d72d018a2f85f919e2d449b96c1a4aec179` (VERTICAL-020) |
| Starting tree | clean |

## 2. Public / Forum Backread

**PROVENANCE ONLY - NOT BEHAVIORAL EVIDENCE.**

Searches for RAN defence-rate mechanics and `EMIMPACTA_DEFENSE_RATE` returned no
RAN material. Every result belonged to a different game: Kritika, MU Online
derivatives (REX-SA, RaGEZONE, Ragnarok Zero) and Epic 7. One REX-SA stat wiki
states outright that "Attack Rate and Defence Rate are not confirmed", which
describes *that* server's configuration rather than RAN.

**No conclusion in this milestone rests on public material.** The legacy tree in
this repository is the sole authority for every claim below.

## 3. Legacy Source Locations

| What | Where |
| --- | --- |
| Portable consumer | `legacy/Lib_Engine/Common/GameCharacterCalculations.cpp:1141` `ApplyDefenseRate` |
| Declaration | `legacy/Lib_Engine/Common/GameCharacterCalculations.h:906` |
| Call site (PC) | `legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2975-2976` |
| Call site (NPC) | `legacy/Lib_Client/G-Logic/GLogicExNPC.cpp:744` (`m_nSUM_DEFENSE`) |
| Call site (summon) | `legacy/Lib_Client/G-Logic/GLSummon.cpp:893` (`int(m_nSUM_DEFENSE * m_fDefenseRate)`) |
| Accumulator field | `legacy/Lib_Client/G-Logic/GLogicEx.h:418`, seeded `1.0f` at `:541` |
| Field declaration | `legacy/Lib_Client/G-Logic/GLCharData.h:1147` `m_fDEFENSE_RATE` |
| Flat defence fold | `GLogixExPC.cpp:376` |
| Per-tick reset | `GLogixExPC.cpp:2195`, `:2220` |

## 4. The Formula

```cpp
GameInt32 ApplyDefenseRate(GameInt32 defense, float defenseRate)
{
    GameInt32 result = (GameInt32)((float)defense * defenseRate);
    if ( result < 0 )  result = 1;
    return result;
}
```

* **Units.** A **multiplier around 1.0, not a percentage.** The accumulator is
  seeded `1.0f` (`:2220`), so `1.0f` means unchanged and `0.25` is a 25% boost.
* **Truncation.** `int()`, toward zero, applied to the *multiplied* result.
* **Minimum.** `1`, but **only when the result is strictly negative**. A zero
  result stays zero.
* **Maximum.** None.

## 5. Contribution Sources

| Contributor | Legacy | Status |
| --- | --- | --- |
| Seed `1.0f` | `GLogixExPC.cpp:2220` | implemented |
| Permanent passive | `:1046` `m_sSUM_PASSIVE.m_fDEFENSE_RATE += fADDON` | implemented |
| Timed skill FACT | `:2341` `m_fDefenseRate += fADDON_VAR` | implemented |
| Equipment | **no `fDEFENSE_RATE` member on `m_sSUMITEM`** | **not** a contributor |
| Codex | `m_dwDefenseIncrease` folds flat at `:376` | **not** a contributor |
| Pet skill FACT | `:2585`, `:2611`, both `/100.0f` | deferred - no modern pet system |
| Land effect | `:2650` | deferred - no modern land system |
| Item FACT | `:2768` | deferred - no modern item-FACT system |
| System buff | `:2890` | deferred - no modern system-buff system |

Every `m_fDefenseRate` occurrence in the legacy tree was enumerated (15 hits
across `GLogicEx.h`, `GLogicExNPC.cpp`, `GLogixExPC.cpp`, `GLSummon.cpp`, and the
ported calculation file), so the four deferred contributors above are a complete
list rather than a selection.

The two proven absences are load-bearing. Equipment and codex contribute **flat**
defence, which VERTICAL-020 already implemented; verifying that is what makes the
modern axis complete, because it means every source that can reach the
accumulator is either present or belongs to a subsystem that does not exist.

Sources are added **raw**. Legacy cannot normalise them, because they do not
share a convention - the pet path divides by 100 (`:2585`) while the passive and
FACT paths do not. Modern preserves the raw addition.

## 6. Ordering

Legacy applies the rate **once**, at the end of the defence fold:

```cpp
m_nDEFENSE_SKILL = GameCharacterCalculations::ApplyDefenseRate(m_nDEFENSE_SKILL, m_fDefenseRate);  // :2975-2976
```

Both rates fold into a single multiplier first, then the multiplier is applied to
the **already-summed** flat defence:

```
defenseRate = 1.0f + passives.defenseRate + facts.defenseRate
defense     = ApplyDefenseRate(flatDefenseSum, defenseRate)
```

The `flatDefenseSum` **includes** the FACT flat contribution. Proof that ordering
matters: base 50 + FACT 10 at rate 1.5 yields **90**, whereas apply-then-add
yields 85. Pinned by `DefenseRate_RateAppliesAfterFlatBonusesIncludingTheFact`.

## 7. Level-1 / Zero-Defence Behaviour

VERTICAL-020 deferred this axis on the belief that legacy forces a minimum of 1
unconditionally. **That reading was wrong** - the guard is `result < 0`, not
`<= 0`.

| Base defence | Rate | Result | Reached by |
| --- | --- | --- | --- |
| 0 | 0 | **0** | `int(0 * 1.0) = 0`, not `< 0` |
| 0 | + | **0** | `int(0 * 1.5) = 0`, not `< 0` |
| 0 | - | **0** | `int(0 * 0.5) = 0`, not `< 0` |
| >0 | 0 | unchanged | `int(d * 1.0)` is identity for `d >= 0` |
| >0 | + | raised | `int(d * r)`, truncated toward zero |
| >0 | - | lowered, floored at 1 | negative result becomes 1 |

The minimum of 1 belongs to the **final defence transformation**, not to the rate.
It is unreachable from a zero base defence at any rate, which is precisely why
applying this axis does **not** move the VERTICAL-006/009 no-buff baseline. That
baseline is pinned by `DefenseRate_NoBuffBaselineIsUnchanged`, and the
zero-base / negative-rate cell above is pinned by
`DefenseRate_ZeroDefenceStaysZeroEvenWithAPositiveRate`.

## 8. FACT Stacking

**SUM**, of floats, added raw - `m_fDefenseRate += fADDON_VAR` is a plain
accumulation with no max/min/last-wins behaviour. Permanent and FACT rates share
one accumulator, so both participate in the same sum before a single application.
Pinned by `DefenseRate_PermanentAndFactRatesSum` and
`SkillFactV021_DefenseRateStacksAcrossSources`.

## 9. FACT Expiry

Timed FACT contributions rebuild from zero on every tick; no save/restore state
was introduced. Removing the last defence-rate FACT returns the accumulator to
exactly the permanent baseline. Pinned by
`SkillFactV021_DefenseRateIsRemovedOnExpiry` and
`ServerFactV021_DefenseRateFactReachesDerivedStatsAndExpires`.

## 10. Consumer

`DerivedStats::defenseRate` folds into `DerivedStats::defense`, so combat reads
the resolved value and the combat calculators needed no change. Proven end to
end by `DefenseRate_HigherRateReducesFinalDamageByTheSameRoll`, which holds the
roll fixed and varies only the resolved defence, and by
`DefenseRate_BoostedDefenseBeyondDamageFloorsDamage`, which pins the damage floor
at 1 rather than 0.

Magic is untouched: the rate never reaches the magic path, and the VERTICAL-013
physical/magic split and the VERTICAL-020 flat-defence exclusion both stand.

## 11. Tests

13 added by this milestone: 8 calculator-level stat arithmetic, 3 FACT
stacking/expiry, 2 combat/damage. All deterministic - no test compares two
independent casts, because `CastSkill` advances its RNG sequence per call.

## 12. Build

Debug and Release both build with **0 errors and 0 warnings**. CTest is 14/14 in
both configurations. Core 474/474 and Server 107/107.

## 13. Deferred

Pet skill FACT, land effect, item FACT and system buff defence rates - each
waiting on a subsystem that does not exist yet. Unrelated FACT impacts
(`CHANGESTATS`, recovery impacts, velocity, status, range) remain deferred and
were not touched.
