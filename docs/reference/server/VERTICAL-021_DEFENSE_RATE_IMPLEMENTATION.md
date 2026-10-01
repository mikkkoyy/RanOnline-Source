# VERTICAL-021 - Defense-Rate Implementation

## 1. Baseline

| | |
| --- | --- |
| Repository | `mikkkoyy/RanOnline-Source` |
| Branch | `main` (tracking `origin/main`) |
| Baseline commit | `855f5d72d018a2f85f919e2d449b96c1a4aec179` (V020) |
| Starting tree | clean |

Outcome: **implemented in full for every contributor modern actually models.**
Four legacy contributors are deferred with their subsystems (section 5).

## 2. Public / Forum Backread

**PROVENANCE ONLY - NOT BEHAVIORAL EVIDENCE.**

A search for RAN defence-rate mechanics and `EMIMPACTA_DEFENSE_RATE` returned no
RAN material. Every hit was a different game: Kritika, MU Online derivatives
(REX-SA, RaGEZONE, Ragnarok Zero) and Epic 7. One of those - a REX-SA stat wiki -
states outright that "Attack Rate and Defence Rate are not confirmed", which is a
statement about *that* server's configuration, not about RAN.

Nothing usable was retrieved, and **no conclusion below rests on it.** The legacy
source remains the sole authority.

## 3. Correction to VERTICAL-020

VERTICAL-020 deferred this axis partly on a misreading of the clamp, recorded as
"legacy applies a minimum of 1 unconditionally, so a level-1 character with
defence 0 would become 1."

The actual body is:

```cpp
GameInt32 ApplyDefenseRate(GameInt32 defense, float defenseRate)
{
    GameInt32 result = (GameInt32)((float)defense * defenseRate);
    if ( result < 0 )  result = 1;
    return result;
}
```

The guard is `result < 0`, **not** `<= 0`. A zero result therefore **stays zero**.

This matters, because it dissolves the blocker. The V020 fear was that applying the
axis would move a level-1 character's defence from 0 to 1 and regress the V006/V009
no-buff baseline. It does not: at rate 1.0 the function is the identity for every
non-negative defence, and zero survives the clamp. The baseline is pinned by
`DefenseRate_NoBuffBaselineIsUnchanged`.

The V020 statement is wrong and should not be relied on.

## 4. Units

`m_fDefenseRate` is a **multiplier around 1.0, not a percentage.** The accumulator
is seeded at `1.0f`, so `1.0f` means unchanged and a source value of `0.25` is a
25% boost.

Sources are added **raw**. Legacy does not normalise them, and it cannot, because
they do not share a convention: the pet path adds `fMVAR / 100.0f`
(`GLogixExPC.cpp:2585`, `:2611`) while the passive and FACT paths add the raw
value. Modern preserves the raw addition rather than inventing a normalisation
legacy does not have.

## 5. Contributors

| Contributor | Legacy | Status |
| --- | --- | --- |
| Seed `1.0f` | `:2220` | implemented |
| Permanent passive | `:1046` `m_sSUM_PASSIVE.m_fDEFENSE_RATE += fADDON` | implemented |
| Timed skill FACT | `:2341` `m_fDefenseRate += fADDON_VAR` | implemented |
| Equipment | **no `fDEFENSE_RATE` in `m_sSUMITEM`** | not a contributor |
| Codex | `m_dwDefenseIncrease` is folded flat at `:376` | not a contributor |
| Pet skill FACT | `:2585`, `:2611` (`/100.0f`) | deferred - no modern pet system |
| Land effect | `:2650` | deferred - no modern land system |
| Item FACT | `:2768` | deferred - no modern item-FACT system |
| System buff | `:2890` | deferred - no modern system-buff system |

The two absences are load-bearing and were verified rather than assumed: equipment
and codex contribute **flat** defence, which V020 already implemented. Confirming
this is what makes the modern axis complete - every source that can reach it is
present, and the rest belong to subsystems that do not exist yet.

## 6. Application Point and Ordering

Legacy applies the rate **once**, at the end of the defence fold:

```cpp
m_nDEFENSE_SKILL = GameCharacterCalculations::ApplyDefenseRate(m_nDEFENSE_SKILL, m_fDefenseRate);  // :2975-2976
```

The rate therefore multiplies the **already-summed** flat defence, *including* the
FACT flat contribution. It is not applied to the base before the bonuses, and the
FACT rate is not a second independent multiplier. Both sources fold into one
multiplier first:

```
defenseRate = 1.0f + passives.defenseRate + facts.defenseRate
defense     = ApplyDefenseRate(flatDefenseSum, defenseRate)
```

Pinned by `DefenseRate_RateAppliesAfterFlatBonusesIncludingTheFact`: base 50 + FACT
10 at rate 1.5 is 90, not the 85 an apply-then-add ordering would give.

Truncation is `int()`, i.e. toward zero, and is applied to the multiplied result -
pinned by `DefenseRate_TruncationIsTowardZero`.

## 7. Changes

| File | Change |
| --- | --- |
| `core/engine/GameCharacterCalculations.h` | added `ApplyDefenseRate` |
| `core/stats/Contributions.h` | `PassiveContribution::defenseRate`, `FactContribution::defenseRate` |
| `core/stats/DerivedStats.h` | `DerivedStats::defenseRate` (default `1.0f`) |
| `core/stats/StatCalculator.cpp` | rate fold + single application |
| `core/skills/PassiveContributionAggregator.cpp` | `DefenseRate` case (was a no-op) |
| `core/skills/SkillFactAggregator.h` | `DefenseRate` case, both aggregators |
| `server/character/ServerCharacter.cpp` | populate `input.facts.defenseRate`; extend the recalc guard |

The `PassiveImpactType::DefenseRate` enum and its `SkillDefinition` string mapping
already existed from V007, but the aggregator discarded the value with a
`// Not in PassiveContribution.` no-op. That comment is now resolved.

## 8. Notes for Reviewers

- **`AdvanceSkillFacts` reports a fact on the call that crosses its expiry
  boundary.** Expiry assertions therefore read state on the *following* advance.
  This is a pre-existing API quirk, not something V021 introduced, and the V020
  expiry tests use the same shape.
- **Damage floors at 1, not 0.** `DefenseRate_BoostedDefenseBeyondDamageFloorsDamage`
  pins this; it is easy to assume 0 and be wrong.
- The rate folded into `DerivedStats::defense`, so combat is untouched. Magic is
  likewise untouched - the rate never reaches the magic path, and the flat
  V020 defence remains excluded from magic.

## 9. Verification

| Suite | Baseline | Result |
| --- | --- | --- |
| Core (Debug) | 461 | 474 passed |
| Core (Release) | 461 | 474 passed |
| Server (Debug/Release) | 106 | 107 passed |
| CTest | 14/14 | 14/14 Debug and Release |
| Compiler warnings | none | none |

13 new tests: 8 stat-arithmetic, 3 FACT stacking/expiry, 2 combat/damage.