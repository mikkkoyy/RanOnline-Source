# VERTICAL-017 — FACT Combat / CastSkill Integration

## 1. Baseline

| | |
| --- | --- |
| Repository | `mikkkoyy/RanOnline-Source` |
| Branch | `main` (tracking `origin/main`) |
| Baseline commit | `ef4daabc7857d2ea5cc61201c06347b841f00748` |
| Starting tree | clean |

This is an **integration** vertical. Five consumers that VERTICAL-016 proved
were connected to systems that already existed. No new calculation system, no new
FACT domain, and no deferred item was implemented.

## 2. m_sDamageSpec Lifecycle

**RESET PROVEN.**

VERTICAL-016 recorded this as an open question because it began reading the
per-tick reset block at `:2255`. The block actually starts at `:2210`, and the
reset it was looking for is there:

```
legacy/Lib_Client/G-Logic/GLogixExPC.cpp   (per-tick FACT pass)
:2210   m_fSKILL_MOVE      = 0.0f + m_sSUM_PASSIVE.m_fMOVEVELO;
:2217   m_dwHOLDBLOW       = NULL;
:2219   m_fDamageRate      = 1.0f + m_sSUM_PASSIVE.m_fDAMAGE_RATE;
:2222   m_sSUMRESIST_SKILL = m_sSUMRESIST;
:2224   m_sDamageSpec.RESET();
:2228   m_sDamageSpec      = m_sSUM_PASSIVE.m_sDamageSpec;
:2234   m_fATTVELO         = 0.0f;
:2236   m_fSKILLDELAY      = 0.0f;
...      then the FACT accumulation loops, :2280-2410
```

The uniform pattern is **reset to default, seed from the passive total, then
accumulate the FACT contributions**. `m_sDamageSpec` follows it exactly, and the
seed at `:2228` is the decisive evidence that the structure is *rebuilt from
active sources each tick* rather than patched.

So there is no accumulation leak. VERTICAL-016's caution was correct to raise but
unnecessary in the end; the finding was a read-range error, not a legacy defect.

**Modern equivalent.** `ServerCharacter::AdvanceSkillFacts` already rebuilds
`m_factModifiers` from zero on every call (VERTICAL-015), so no save/restore
mechanism was introduced and none was needed.

## 3. Damage Reduction Integration

Proven semantics, `GLogixExPC.cpp:2380-2387`:

```
if ( m_sDamageSpec.m_fPsyDamageReduce   < fSPECVAR1 )  m_sDamageSpec.m_fPsyDamageReduce   = fSPECVAR1;
if ( m_sDamageSpec.m_fMagicDamageReduce < fSPECVAR1 )  m_sDamageSpec.m_fMagicDamageReduce = fSPECVAR1;
```

**MAX, not sum.** Two 0.10 and 0.20 buffs give 0.20.

**Mapping.** No new structure was created. The four values already had authoritative
destinations added by VERTICAL-013:

| Legacy | Modern | Consumer |
| --- | --- | --- |
| `m_fPsyDamageReduce` | `CombatInput::targetDamageReduce` | `PhysicalDamageCalculator` |
| `m_fMagicDamageReduce` | `CombatInput::targetMagicDamageReduce` | `MagicDamageCalculator` |
| `m_fPsyDamageReflection` / `Rate` | `CombatInput::targetDamageReflection{,Rate}` | `PhysicalDamageCalculator` |
| `m_fMagicDamageReflection` / `Rate` | `CombatInput::targetMagicDamageReflection{,Rate}` | `MagicDamageCalculator` |

The combination happens at the two existing boundaries:

- **Basic attack** — `ServerCharacter::Attack` takes the maximum of the target's
  own `DAMAGE_SPEC` value and the FACT value.
- **Skills** — `ActiveSkillResolver` does the same via a `Stronger()` helper,
  because the resolver owns `CombatInput` for the skill path.

The calculators are untouched.

## 4. Damage Reflection Integration

Proven semantics, `GLogixExPC.cpp:2390-2401`:

```
if ( m_sDamageSpec.m_fPsyDamageReflection < fSPECVAR1 ) {
    m_sDamageSpec.m_fPsyDamageReflection     = fSPECVAR1;
    m_sDamageSpec.m_fPsyDamageReflectionRate = fSPECVAR2;     // same spec
}
```

**MAX on the amount, and the rate is taken from that same spec.** Amount and rate
are never mixed between different facts, which is why the modern combination
applies the maximum independently to each field rather than selecting a spec
whole — the aggregator has already paired them, and by the time the values reach
`CombatInput` the pairing is fixed.

`SkillFactConsumers_StrongerReflectionSupersedesThePair` and
`SkillFactConsumers_WeakerReflectionDoesNotOverwriteThePair` pin both directions,
including the case where the weaker fact is applied *second* so slot order alone
would pick it.

## 5. PA/SA/MA FactContribution

Proven semantics, `GLogixExPC.cpp:2343-2345`:

```
case EMIMPACTA_PA:  nSUM_PA += int(fADDON_VAR);  break;
case EMIMPACTA_SA:  nSUM_SA += int(fADDON_VAR);  break;
case EMIMPACTA_MA:  nSUM_MA += int(fADDON_VAR);  break;
```

**SUM, `int()`-truncated** — the opposite rule to the damage reductions, and
that contrast is deliberate and tested.

Legacy sums these separately from the passive total and adds them only at the
point of use (`:2970-2972`), so modern gained a **fourth** contribution struct:

```cpp
// modern/core/stats/Contributions.h
struct FactContribution { int32_t meleePower; int32_t shootPower; int32_t magicAttack; };
```

It is deliberately **not** folded into `PassiveContribution`. Doing so would make
a buff that expires on a timer indistinguishable from a skill that was learned
permanently, and would let it survive expiry. It is wired into
`StatCalculationInput::facts` as a fourth source beside `items`, `passives` and
`codex`, and `SumAttackPowers` folds it in via the existing `VariationClamped`
path, so the 16-bit clamp stays in the stat layer exactly as for every other
source.

Only the three proven values are carried. The other twenty `EMIMPACTA_*` values
stay untouched in the FACT record rather than being added speculatively.

**Recalculation.** `DerivedStats` is a cached snapshot, so the buff would not be
visible until something else triggered a recalculation. `ServerCharacter` tracks
the power values currently folded into the snapshot and recalculates when — and
only when — one of them moves (`RefreshFactStatsIfPowersChanged`). This is the one
piece of machinery this milestone added that has no direct legacy counterpart,
because legacy reads the accumulators live each tick; it exists only to keep the
modern cached snapshot honest.

## 6. PROHIBIT_SKILL CastSkill Integration

Proven call path, `GLogixExPC.cpp:4056-4083` `CHECHSKILL`:

```
:4060   if ( m_bProhibitSkill || m_bCaptureTheFlagHoldFlag )  return EMSKILL_PROHIBIT;
:4063   if ( m_bSTATE_STUN )                                   return EMSKILL_PROHIBIT;
:4077   ... return EMSKILL_NOTLEARN
:4082   ... return EMSKILL_DELAYTIME
```

It is the **first statement** of the function, so it precedes the learned check,
the cooldown check, target validation and any resource deduction.

Modern already implemented the refusal (`ActiveSkillResolver:145`, returning
`NotCastable`); only the server-side input was missing. `ServerCharacter::CastSkill`
now sets `input.skillProhibited = m_factModifiers.prohibitSkill`. **No second
check was added** — the resolver still owns the decision.

`ServerSkillFactConsumers_ProhibitionSpendsNothing` verifies the ordering
consequence: SP, MP and HP are all unchanged and no cooldown is started.

**Not carried across:** `m_bCaptureTheFlagHoldFlag`, a PvP flag with no modern
equivalent. Recorded as DEFERRED — NOT PROVEN, and it must not be dropped silently.

## 7. NONBLOW CastSkill Integration

Proven semantics, `GLogixExPC.cpp:2357` — an **assignment**, not an OR — applied
as a gate at `GLChar.cpp:3376-3379`:

```cpp
if ( !(pACTOR->GETHOLDBLOW() & STATE_TO_DISORDER(sBLOW.emTYPE)) )
    bBLOW = CHECKSTATEBLOW(...);
```

`ServerCharacter::CastSkill` now sets
`input.targetDisorderMask = target.m_factModifiers.statusImmunityMask`.

The architecture is unchanged and the domains stay separate — a number crosses a
boundary:

```
SkillFactContainer -> SkillFactModifiers::statusImmunityMask
                   -> ActiveSkillInput::targetDisorderMask
                   -> StatusEffectResolver -> StatusEffectContainer
```

`StatusEffectContainer` never inspects FACT records. `StatusEffectResolver`
already refuses with `TargetImmune` and skips the probability roll entirely, which
is legacy's behaviour.

The status random roll is left at its default here. Supplying it needs a
deterministic per-cast source, and inventing one would change which blows land;
that part is DEFERRED rather than guessed.

## 8. Expiry Behavior

No save/restore anywhere. Every value disappears because the aggregator rebuilds
from zero and the consumers re-read the rebuilt snapshot.

| Modifier | After expiry |
| --- | --- |
| Damage reductions / reflections | next tick aggregates to 0; combat input takes the target's own value again |
| PA/SA/MA | `meleePower` returns to 0, `RefreshFactStatsIfPowersChanged` recalculates, snapshot returns to baseline |
| `PROHIBIT_SKILL` | rebuilt to `false`; casts resume |
| `NONBLOW` mask | rebuilt to 0; the matching status becomes applicable again |

The VERTICAL-015 off-by-one still applies and is still reproduced: a FACT
contributes on the tick it expires, because `GLogixExPC.cpp:2295` disables it
after the loop's `continue` guard at `:2280` has already passed.

## 9. Tests

| Suite | Before | After |
| --- | --- | --- |
| ModernCoreTests | 425 | **441** (+16) |
| ModernServerTests | 82 | **94** (+12) |
| CTest | 14/14 | 14/14 (Debug and Release) |
| Client suites | 12/12 | 12/12 |

Added to `modern/tests/SkillFactTests.cpp`:

- powers: baseline, one fact, two facts summing, `int()` truncation, an impact and
  a spec on the same fact
- reduction: baseline, one, two (MAX not sum), weaker-second, magic independent of
  physical, expired
- reflection: paired amount+rate, stronger supersedes, weaker does not overwrite,
  magic independent of physical, expired

Added to `modern/server/ServerCharacterTests.cpp`:

- `PROHIBIT_SKILL`: absent / active / expired, plus a case proving no SP, MP or HP
  is spent and no cooldown starts
- `NONBLOW`: an actual cast refused with `TargetImmune`, last-wins across two
  facts, expiry
- PA/SA/MA: raises the derived power, two facts accumulate, expiry restores the
  exact baseline, and a two-character control proving the permanent stat of an
  unbuffed character is untouched

No existing test was modified, weakened or deleted. VERTICAL-001 through
VERTICAL-016 expectations are unchanged.

## 10. Deferred Items

Unchanged from VERTICAL-016, and still not implemented:

`EMIMPACTA_CHANGESTATS`, the recovery/CP impacts, `CP_AUTO`, `DEFENSE`,
`DEFENSE_RATE`, `RESIST`, `DAMAGE_RATE` into the physical pipeline (the
pre-existing `PhysicalDamageInput` gap — **not** fixed here, per §9),
`EMIMPACTA_DAMAGE`, `HITRATE`, `AVOIDRATE`, `VARHP/VARMP/VARSP/VARAP`,
`HP/MP/SP_RATE`, `MOVEVELO`, `ATTACKVELO`, `PROHIBIT_POTION`, `INVISIBLE`,
`RECVISIBLE`, `PIERCE`, `TARRANGE`, `CHANGE_*_RANGE`, `STUN`, `CONTINUOUS_DAMAGE`,
`CURSE`, `IGNORE_DAMAGE`, `IMMUNE`, `STIGMA`, `ENHANCEMENT`,
`DEFENSE_SKILL_ACTIVE`, `REFDAMAGE`, `TALK_TO_NPC`, `DAMAGE_LOOP`, `TAR_BUFF`,
the world/entity registry, networking, client presentation, movement simulation
and any potion system.

Also newly deferred this milestone:

- `m_bCaptureTheFlagHoldFlag` — DEFERRED — NOT PROVEN (no modern PvP equivalent)
- the status random roll for the `CastSkill` path — DEFERRED, needs a proven
  deterministic per-cast source

## 11. Exact Legacy Evidence

```
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2210-2241  per-tick reset block
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2224       m_sDamageSpec.RESET()
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2228       m_sDamageSpec = m_sSUM_PASSIVE.m_sDamageSpec
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2292-2295  tick + expire, off-by-one
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2343-2345  EMIMPACTA_PA/SA/MA, SUM + int()
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2357       EMSPECA_NONBLOW, assignment
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2374,2377  PROHIBIT_POTION / PROHIBIT_SKILL
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2379-2401  damage reduce / reflection MAX
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2970-2972  passive and FACT powers summed separately
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:4056-4083  CHECHSKILL; prohibition at :4060
legacy/Lib_Client/G-Logic/GLChar.cpp:3376-3379      GETHOLDBLOW immunity gate
legacy/Lib_Client/G-Logic/GLFactData.h:50-74        SSKILLFACT
```

Modern files changed:

```
modern/core/skills/SkillFactAggregator.h   PA/SA/MA aggregation
modern/core/skills/ActiveSkill.h           six fact* combat inputs
modern/core/skills/ActiveSkill.cpp         Stronger() + combination into CombatInput
modern/core/stats/Contributions.h          FactContribution
modern/core/stats/StatCalculator.h         facts on StatCalculationInput
modern/core/stats/StatCalculator.cpp       fourth attack-power source
modern/server/character/ServerCharacter.h  ApplySkillFact refresh, member
modern/server/character/ServerCharacter.cpp CastSkill inputs, stat input, recalc
modern/tests/SkillFactTests.cpp            +16 consumer tests
modern/server/ServerCharacterTests.cpp     +12 integration tests
```

Modern files read but deliberately **not** changed: `PhysicalDamageCalculator.h`
and `MagicDamageCalculator.h` (their consumers were already correct),
`SkillFactContainer.h`, `SkillFactTypes.h`, `StatusEffectResolver.h`,
`StatusEffectContainer.h`, and all of `modern/core/status/`.