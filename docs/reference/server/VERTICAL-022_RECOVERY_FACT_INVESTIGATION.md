# VERTICAL-022 - Recovery / HP-MP-SP-AP FACT Investigation

Companion to the code changes. Every claim below carries a legacy location;
nothing is inferred from a variable name.

## 1. Baseline

| | |
| --- | --- |
| Repository | `mikkkoyy/RanOnline-Source` |
| Branch | `main` (tracking `origin/main`) |
| Baseline commit | `7f3550dcb6253675925bc79862b54851e6327ff2` (VERTICAL-021) |
| Starting tree | clean |

## 2. Public / Forum Backread

**PROVENANCE ONLY - NOT BEHAVIORAL EVIDENCE.**

Searches returned one genuinely useful artefact: a RaGEZONE thread quoting the
RAN `EMIMPACT_ADDON` enum verbatim, which **corroborates the numeric values**
already read from `legacy/Lib_Client/G-Logic/GLCharDefine.h:977-991` (VARHP=5,
VARMP=6, VARSP=7, VARAP=8, HP_RATE=14, MP_RATE=15, SP_RATE=16).

It contributes nothing behavioural. RAN's own site (`ran.com.tw`) and the fandom
wiki discuss recovery *items* and the "60AP" event currency, neither of which is
the skill-FACT axis under investigation.

**No formula, ordering, or consumer claim below rests on public material.** The
legacy tree is the sole authority.

## 3. The decisive finding

**`HP_RATE` / `MP_RATE` / `SP_RATE` do not touch recovery at all**, despite the
names and despite this milestone being framed as "recovery FACT".

They accumulate into `m_fHP_RATE` (`GLogixExPC.cpp:2346`), and that accumulator
is read in exactly one place - `UPDATE_MAX_POINT`:

```cpp
m_sHP.dwMax = DWORD ( m_sHP.dwMax * (1+m_sSUM_PASSIVE.m_fHP_RATE+m_fHP_RATE ) * fCONFT_POINT_RATE );
                                                                   // :2165
```

So they scale the **resource maximum**. The recovery rate is `fINCR_HP`, fed by
`EMIMPACTA_VARHP`, and the two never meet.

**`VARAP` is not an action-point pool.** It adds one value to all three recovery
rates:

```cpp
case EMIMPACTA_VARAP:
    fINCR_HP += sSKEFF.sImpacts[nImpact].fADDON_VAR;
    fINCR_MP += sSKEFF.sImpacts[nImpact].fADDON_VAR;
    fINCR_SP += sSKEFF.sImpacts[nImpact].fADDON_VAR;
    break;                                        // :2334-2338
```

The enum comment agrees: `EMIMPACTA_VARAP = 8,  // HP,MP,SP ???` (`GLCharDefine.h:980`).
RAN's real combat-point resource is `m_sCombatPoint` (`:358`, `:4261`, `:4353`),
and **none** of these seven impacts touches it. The modern tree already models SP
cost (`VERTICAL-010`) but has no CP pool; none was invented, because none is
reachable.

So the seven impacts are **two axes**, not one:

| Axis | Impacts | Consumer |
| --- | --- | --- |
| Recovery rate | VARHP, VARMP, VARSP, VARAP | `fElap * ( dwMax * fINCR_x + ... )` `:3020-3022` |
| Resource maximum | HP_RATE, MP_RATE, SP_RATE | `dwMax * (1 + rate) * conf` `:2165-2177` |

## 4. Impact Table

| Impact | Legacy accumulator | Seed | Operation | Truncation | Clamp | Consumer | Modern owner | Decision |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| VARHP | `fINCR_HP` (local copy of `m_fINCR_HP`) `:2241`, `:2331` | `fHP_INC_PER + m_sSUMITEM.fIncR_HP + m_sSUM_PASSIVE.m_fINCR_HP` `:397` | `+=` float, SUM, raw | none on the rate | none | `dwMax * fINCR_HP` `:3020` | `FactContribution::hpRecoveryRate` -> `DerivedStats::hpRecoveryRate` -> `ResourceState::Recover` | IMPLEMENTED |
| VARMP | `fINCR_MP` `:2332` | same shape `:398` | `+=` float | none | none | `:3021` | `FactContribution::mpRecoveryRate` | IMPLEMENTED |
| VARSP | `fINCR_SP` `:2333` | same shape `:399` | `+=` float | none | none | `:3022` | `FactContribution::spRecoveryRate` | IMPLEMENTED |
| VARAP | all three of the above `:2334-2338` | - | three `+=` | none | none | `:3020-3022` | `FactContribution::hp/mp/spRecoveryRate` | IMPLEMENTED |
| HP_RATE | `m_fHP_RATE` (per-tick) `:2245`, `:2346` | reset to `0` each tick; consumed as `1 + passive + m_fHP_RATE` `:2165` | `+=` float, SUM, raw | `DWORD()` at `:2165` | none on HP; `if (dwMax <= 0) dwMax = 1` at `:2167` (before the codex add) | `m_sHP.dwMax` | `FactContribution::hpRate` -> `SumResourceMax` | IMPLEMENTED |
| MP_RATE | `m_fMP_RATE` `:2246`, `:2347` | same | `+=` float | `DWORD()` `:2172` | none | `m_sMP.dwMax` | `FactContribution::mpRate` | IMPLEMENTED |
| SP_RATE | `m_fSP_RATE` `:2247`, `:2348` | same | `+=` float | `DWORD()` `:2177` | none | `m_sSP.dwMax` | `FactContribution::spRate` | IMPLEMENTED |

Every occurrence in the legacy tree was enumerated - 13 per impact across
`GLogixExPC.cpp` (skill FACT, item FACT, system buff), `GLCharDefine.h`, the
NPC and summon files, and the client tooltip/UI loaders, which are presentation
only.

## 5. Recovery Formulas

Legacy, per resource (`GLogixExPC.cpp:3019-3026`):

```cpp
float fElap = (fElapsedTime/GLCONST_CHAR::fUNIT_TIME);
float fINC_HP = fElap* ( m_sHP.dwMax*fINCR_HP + GLCONST_CHAR::fHP_INC + m_sSUMITEM.fInc_HP );
GLOGICEX::UPDATE_POINT ( m_sHP, m_fIncHP, fINC_HP, 1 );
```

Then `UPDATE_POINT` (`GLogicEx.h:270-284`) accumulates the fraction:

```cpp
fELP_VAR += fVAR;
int nNEWP = int(sPOINT.dwNow) + int(fELP_VAR);
fELP_VAR -= int(fELP_VAR);
```

So the rate is a **fraction of the maximum per unit of time**, added to a flat
term, scaled by elapsed time, then truncated toward zero with the remainder
carried. `ResourceState::Recover` already reproduces this exactly - VERTICAL-005
built it - so this milestone only had to route the new contributions into
`hpRecoveryRate` / `mpRecoveryRate` / `spRecoveryRate`, which already existed.

## 6. Ordering

Three proven orderings, all preserved:

1. **Rate term before flat term.** `dwMax * fINCR_HP + fX_INC + fInc_X` (`:3020`).
   The FACT rate cannot be folded into the flat term.
2. **Permanent before FACT.** `m_fINCR_HP` is seeded from the permanent sum
   (`:397`), copied into a local (`:2241`), and only then increased by the timed
   impact (`:2331`). Modelled as constant + items + passives + facts.
3. **Codex after the rate on the maximum** (`:2165`, codex at `:2168`):

   ```cpp
   dwMax = DWORD( dwMax * (1 + passiveRate + factRate) * conf );
   dwMax += m_dwHPIncrease;     // NOT inside the multiplier
   ```

Pinned by `RecoveryFact_CodexBonusIsAddedAfterTheRateMultiplier`: 100 flat at
rate 0.5 with codex 10 is **160**, not 110.

## 7. Truncation and Clamps

| Step | Operation |
| --- | --- |
| Maximum, after the stat fold | `DWORD(...)` `:2164` |
| Maximum, after the rate | `DWORD(dwMax * (1+rate) * conf)` `:2165` |
| HP maximum floor | `if (dwMax <= 0) dwMax = 1` `:2167` - HP only, before the codex add |
| MP/SP maximum floor | none - `LIMIT()` only |
| Recovery rate | **no cast anywhere**; stays float to the consumer |
| Recovery amount | `int()` toward zero, remainder carried `GLogicEx.h:274-276` |
| Pool current | `if (nNEWP < 0) = 0`, then low limit 1 for HP / 0 for MP+SP, then `LIMIT()` |

Note the HP floor at `:2167` sits **between** the rate multiplication and the
codex addition - a third ordering that must not be reordered. The existing
`SumResourceMax` models the two truncations and the post-rate codex add; the
`dwMax <= 0` floor is a pre-existing property of that helper and is unchanged.

A rate of `-1.0` collapses the maximum to 0 through the second truncation -
pinned by `RecoveryFact_NegativeMaximumRateCanCollapseTheMaximum`.

## 8. Modern Ownership

No new subsystem. Both axes already had correct owners, which is why this
milestone is an extension rather than a design:

| Concept | Modern owner | Status |
| --- | --- | --- |
| Recovery rate | `ItemContribution::hpRecoveryRate`, `PassiveContribution::hpRecoveryRate`, `DerivedStats::hpRecoveryRate`, `ResourceState::Recover` | existed (V005) |
| Recovery flat | `...RecoveryFlat` - **item only** | existed; no FACT contributes, correctly |
| Resource maximum rate | `PassiveContribution::hpRate` -> `SumResourceMax` | existed |
| Timed FACT carrier | `FactContribution` | extended with six floats |
| FACT aggregation | `SkillFactAggregator.h`, both functions | extended |
| Server plumbing | `ServerCharacter::Recalculate`, `RefreshFactStatsIfPowersChanged` | extended |

`DerivedStats` gained **no** new field: `hpRate` and `hpRecoveryRate` are
consumed during the fold and the resolved `maxHp` / `hpRecoveryRate` are what
downstream code reads. This matters - a new derived field would have created a
second statement of the same fact.

The `hpRecoveryFlat` field was left alone deliberately: legacy has no
`EMIMPACTA_*` impact that reaches `fInc_HP`, so a FACT flat contribution would be
invented behaviour.

## 9. FACT Expiry

Both `AdvanceSkillFacts` and `AggregateSkillFacts` rebuild every accumulator
from a default-constructed `SkillFactModifiers` on each call; no save/restore
state was introduced. Pinned by `SkillFactV022_RecoveryExpiryRebuildsFromZero`
and, more importantly, `SkillFactV022_OneExpiryDoesNotDisturbTheSurvivingFact` -
a short fact expires while a long one continues contributing, which a restore
scheme could not satisfy.

The server path's recalc watch list was extended with all six new fields.
Without that, a recovery FACT would sit in `m_factModifiers` and never trigger
`Recalculate`.

## 10. Stacking

**SUM for all six**, floats, added raw. Legacy applies `+=` with no cast and no
comparison on every one of these paths (`:2331-2338`, `:2346-2348`). Pinned by
`SkillFactV022_PositiveAndNegativeFactsCombineBySum`, which uses exactly
representable binary fractions (0.5 and -0.25) so the assertion tests the
stacking rule rather than float subtraction - an earlier revision used
0.01f/0.004f and failed on the rounding rather than on any rule.

## 11. Tests

23 added, all deterministic - no test compares two `CastSkill()` calls, because
each call advances the deterministic RNG sequence.

| File | Count | Covers |
| --- | --- | --- |
| `StatCalculationTests.cpp` | 12 | calculator-level: axis separation, VARAP fan-out, negative, truncation, codex-after-rate, permanent+FACT sum, zero-identity |
| `SkillFactTests.cpp` | 7 | SUM, independent axes, expiry rebuild, partial expiry, no-truncation |
| `ServerCharacterTests.cpp` | 4 | full server path, both axes, independent expiry, VARAP |

The file's oracle was extended to carry the timed FACT block across **all**
fields, and the oracle sweep gained `facts only`, `permanent and timed rates
together`, and `everything` cases. Extending the oracle rather than skipping the
new axes is what makes this a real check: it independently re-derives the same
expressions and compares.

The oracle mismatch report was also upgraded to name each disagreeing field.
That is how a genuine mismatch was found and fixed during this milestone -
it showed `physicalDamage` differing, which surfaced a real distinction
(`EMIMPACTA_DAMAGE` is a combat-boundary `VAR_PARAM`, not a derived-range term,
per V019) that would otherwise have been silently papered over.

## 12. Build

| Suite | Result |
| --- | --- |
| Debug build | 0 errors, 0 warnings |
| Release build | 0 errors, 0 warnings |
| Core tests | 494/494 (was 474) |
| Server tests | 111/111 (was 107) |
| CTest Debug | 14/14 |
| CTest Release | 14/14 |

## 13. Deferred

* Pet skill FACT (`GLogixExPC.cpp:2574`, `:2600`) and land effect (`:2642`) feed
  the same `fINCR_*` accumulators - no modern pet or land subsystem.
* Item FACT (`:2758`) and system buff (`:2880`) recovery rates - no modern
  item-FACT or system-buff subsystem. Their *defence-rate* halves are deferred
  for the same reason, from V021.
* `EMIMPACTA_HP_RECOVERY_VAR` / `MP` / `SP` / `CP_RECOVERY_VAR` /
  `CP_AUTO_VAR` (enum 19-23, `GLCharDefine.h:992-996`) - **not found** in any
  legacy accumulation switch, so there is no proven consumer to implement
  against.
* `EMIMPACTA_CHANGESTATS` (18) - deferred.
* Combat points (`m_sCombatPoint`) - no modern pool, and **no** impact in this
  milestone reaches one. `VARAP` is not one.
