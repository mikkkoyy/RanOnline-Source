# VERTICAL-020 — FACT Defense / Defense-Rate / Resistance Investigation

## 1. Baseline

| | |
| --- | --- |
| Repository | `mikkkoyy/RanOnline-Source` |
| Branch | `main` (tracking `origin/main`) |
| Baseline commit | `e56b770031654ce5488541acff6e5a0643133190` |
| Starting tree | clean |

Outcome: **two of three implemented, one deferred.** The deferral is evidence-based,
not a shortfall.

## 2. Public / Forum Backread

**PROVENANCE ONLY — NOT BEHAVIORAL EVIDENCE.**

Searches for `EMIMPACTA_DEFENSE`, `EMIMPACTA_DEFENSE_RATE`, `EMIMPACTA_RESIST`
and RAN defence/resistance mechanics returned the same RAN source mirrors found
in VERTICAL-016 (`gitlab.com/ragezone/ran-online/game-sources` and GitHub forks),
which are JS-rendered and exposed no implementation content, plus unrelated
material from other games. Nothing usable was retrieved, and **no conclusion below
rests on it**.

## 3. Legacy Source Locations

| Concern | Location |
| --- | --- |
| Impact accumulation | `GLogixExPC.cpp:2330`, `:2341`, `:2349` |
| Reset / seed block | `GLogixExPC.cpp:2195`, `:2220`, `:2222`, `:2975-2979` |
| Defence accessors | `GLChar.h:493-495`, `GLogicEx.h:642` |
| Resistance accessor | `GLChar.h:478` (`GETRESIST`) |
| `SRESIST` layout and `operator+=` | `GLCharDefine.h:646-778` |
| `ApplyDefenseRate` | `GameCharacterCalculations.cpp` (SUM_ADDITION port) |

## 4. EMIMPACTA_DEFENSE — IMPLEMENTED

### Accumulation

```
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2330
  m_nDEFENSE_SKILL += int(sSKEFF.sImpacts[nImpact].fADDON_VAR);
```

**SUM**, `int()` truncation toward zero. Reset each tick at `:2195`
(`m_nDEFENSE_SKILL = m_nDEFENSE`) and seeded from the permanent total at `:376`,
where `m_nDEFENSE = int(m_nDEFENSE_BODY + m_sSUMITEM.nDefense + m_sSUM_PASSIVE.m_nDEFENSE + m_dwDefenseIncrease)`.

### Ownership — the precise answer

| Quantity | Legacy source | FACT-touched? |
| --- | --- | --- |
| flat total defence (`nDEFENSE`) | `GetDefense()` → `GETDEFENSE()` → `m_nDEFENSE_SKILL` (`GLChar.h:493`) | **YES** |
| body defence (`nDEFAULT_DEFENSE`) | `GetBodyDefense()` → `m_nDEFENSE_BODY` (`GLChar.h:494`) | no |
| item defence (`nITEM_DEFENSE`) | `GetItemDefense()` → `m_sSUMITEM.nDefense` (`GLChar.h:495`) | no |

So the FACT raises the **flat total only**. This is asserted directly in
`ServerFactV020_DefenseFactRaisesTotalDefenseOnly`, which also pins that body
defence is unchanged.

### Consumption

```
GLogicEx.h:642  GETDEFENSE() -> m_nDEFENSE_SKILL
GLChar.h:493    GetDefense()  -> GETDEFENSE()
GLogixExPC.cpp:1383   int nDEFENSE = pActor->GetDefense();
GLogixExPC.cpp:1678   int nNetDAMAGE = int(nDAMAGE_OLD * (1 - fLOW_SEED_DAMAGE) - nDEFENSE);
GLogixExPC.cpp:1684   ResultDamage = nDAMAGE_OLD - nDEFENSE;
```

The magic path zeroes it (`:1474`), unchanged by this milestone.

### Modern integration

`FactContribution::defense` → `DerivedStats::defense` (folded into the existing
items/passives/codex chain, which already mirrored legacy's `m_nDEFENSE`) →
`CombatInput::targetDefense` → `PhysicalDamageInput::defense`. The consequence is
pinned deterministically by `FactDefense_ReducesFinalDamage`.

One deliberate deviation from the surrounding code: the contribution uses plain
signed `+=` rather than `WrapAdd`, because `WrapAdd`'s bonus parameter is
`uint32_t` and would turn a negative FACT value into a huge positive one. Legacy
accumulates a signed `int` and never clamps here.

## 5. EMIMPACTA_DEFENSE_RATE — DEFERRED

### The consumer IS proven

```
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2220   m_fDefenseRate = 1.0f + m_sSUM_PASSIVE.m_fDEFENSE_RATE;   (seed)
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2341   m_fDefenseRate += sSKEFF.sImpacts[nImpact].fADDON_VAR;       (SUM, float)
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2975-2976
    m_nDEFENSE_SKILL = GameCharacterCalculations::ApplyDefenseRate(m_nDEFENSE_SKILL, m_fDefenseRate);
```

`ApplyDefenseRate` is `m_nDEFENSE_SKILL = int(m_nDEFENSE_SKILL * m_fDefenseRate)`
with a **minimum of 1** when the result is negative. It is a percentage applied
to the *skill* defence accumulator only — not body defence, not item defence.

### Why it is still deferred

1. **Modern has no defence-rate axis at all.** There is no passive, item, pet or
   land-effect rate in `modern/` — only the FACT term would exist. Legacy's
   `m_fDefenseRate` is `1 + passive + FACT + pet + land`; shipping only the FACT
   term would be a partial, asymmetric version of the rule.
2. **The unconditional `min 1` would change existing behaviour.** Legacy applies
   it to every character, so a level-1 character with defence 0 would become 1.
   Introducing that here would alter V006/V009 expectations for characters that
   have no buff at all — a change the milestone brief explicitly forbids.

The correct home is a dedicated slice that introduces the rate axis with its
passive counterpart, where the `min 1` semantic can be introduced deliberately
rather than as a side effect. **Not** "consumer not proven" — the consumer is
proven and recorded above; the blocker is that modern lacks the surrounding rule.

## 6. EMIMPACTA_RESIST — IMPLEMENTED

### Accumulation and the crucial detail

```
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2222   m_sSUMRESIST_SKILL = m_sSUMRESIST;      (reset/seed)
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2349   m_sSUMRESIST_SKILL += int(fADDON_VAR);    (SUM, int truncation)
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2979   m_sSUMRESIST_SKILL.LIMIT();
```

`m_sSUMRESIST_SKILL += int(...)` invokes
`SRESIST::operator+=(int)` (`GLCharDefine.h:765-778`), which writes the **same
value to all five components**:

```cpp
nFire += rvalue;  nIce += rvalue;  nElectric += rvalue;
nPoison += rvalue; nSpirit += rvalue;
```

So it is a **generic all-axes bonus**, not a per-element one. It is **not**
targeted at the attack's element, and the value is stored as integer percentage
points, floored at zero by `LIMIT()`.

### Consumer

This was the surprise of the milestone. `m_sSUMRESIST_SKILL` — not
`m_sSUMRESIST` — is what the game reads:

```
legacy/Lib_Client/G-Logic/GLChar.h:478
  virtual const SRESIST& GETRESIST () const { return m_sSUMRESIST_SKILL; }
```

so the FACT-reduced resistance is used by:

- `GLogixExPC.cpp:1389, 1515-1516` — the damage resistance reduction
- `GLChar.cpp:3368-3369` — the status-blow resistance, feeding the
  `StatusEffectResolver` immunity check of VERTICAL-014

It is **target-side** and affects **both** physical and magic damage, plus status
application.

### Modern integration

`FactContribution::resist` (one scalar) is added to **all five** of
`Stats::Resistances` in `StatCalculator`, matching `operator+=(int)` exactly. The
consumer already existed: `ActiveSkill.cpp:443` resolves the target's resistance
for the skill's element via `ResolveResistance` and feeds
`CombatInput::targetResistElement`.

### A correction made during implementation

My first attempt added `ClampNonNegative()` immediately after the FACT fold,
mirroring a clamp near the end of the existing fold. That was **wrong** and the
stat oracle caught it: legacy floors each element once, at
`m_sSUMRESIST_SKILL.LIMIT()` (`:2979`), *after* every contribution including the
codex one. Clamping mid-fold raised a negative intermediate that a later positive
codex term was meant to offset, producing `3` where the oracle produced `0` on the
ice and spirit axes. The mid-fold clamp was removed; a comment now records why it
must not return.

## 7. Decision Summary

| Impact | Accumulator | Operation | Consumer | Modern owner | Decision |
| --- | --- | --- | --- | --- | --- |
| `EMIMPACTA_DEFENSE` | `m_nDEFENSE_SKILL` | SUM, `int()` | `GetDefense()` → `nDEFENSE` | `FactContribution::defense` → `DerivedStats::defense` | **IMPLEMENTED** |
| `EMIMPACTA_DEFENSE_RATE` | `m_fDefenseRate` | SUM, float | `ApplyDefenseRate` → `m_nDEFENSE_SKILL` | none | **DEFERRED** — no modern rate axis; unconditional `min 1` would change existing defence |
| `EMIMPACTA_RESIST` | `m_sSUMRESIST_SKILL` | SUM, `int()`, all five axes | `GETRESIST()` → damage resist + status resist | `FactContribution::resist` → `Stats::Resistances` | **IMPLEMENTED** |

## 8. Expiry and Stacking

No save/restore state exists. `RefreshFactStatsIfPowersChanged` was extended to
watch `defense` and `resist` so the cached `DerivedStats` snapshot stays honest;
`ServerFactV020_TwoDefenseFactsSumAndExpire`,
`ServerFactV020_TwoResistFactsSumAndExpire` and
`SkillFactV020_DefenseAndResistExpireIndependently` prove it.

Stacking is **SUM** for both, with `int()` truncation, negatives carried, and the
defence/boss-style contrast to the MAX rules of the reduction specs preserved.

## 9. Tests

| Suite | Before | After |
| --- | --- | --- |
| ModernCoreTests | 455 | **461** (+6) |
| ModernServerTests | 101 | **106** (+5) |
| CTest | 14/14 | 14/14 (Debug and Release) |
| Client suites | 12/12 | 12/12 |

Added: aggregation (SUM, truncation, negatives, independent expiry); defence
raising the total but not body; resistance reaching all five axes explicitly;
two-FACT SUM and expiry for each; `FactDefense_ReducesFinalDamage` pinning the
arithmetic consequence; and `FactDefense_BodyAndItemDecayIsSeparateFromFlatDefense`
separating the two axes.

One test was rewritten during implementation. A server-level "defence buff
reduces damage" comparison across two casts proved **unreliable**:
`CastSkill` calls `DeterministicRandom()` per cast and advances the sequence, so
two casts do not share a roll. The deterministic form moved to the calculator
test, and the server test now asserts against a single cast's own
pre-defence figure. This is recorded rather than papered over.

No existing test was weakened; the stat oracle was left untouched and it is what
caught the misplaced clamp.

## 10. Build Result

Debug and Release: 0 errors, 0 warnings. CTest 14/14 in both.
461/461 Core and 106/106 Server in both configurations.

## 11. Deferred

`EMIMPACTA_DEFENSE_RATE` (above), plus everything still open from earlier
milestones: `EMIMPACTA_CHANGESTATS`, `VARHP`/`VARMP`/`VARSP`/`VARAP`,
`HP_RATE`/`MP_RATE`/`SP_RATE`, recovery and CP impacts, `CP_AUTO`,
`MOVEVELO`, `ATTACKVELO`, `PROHIBIT_POTION`, `INVISIBLE`, `RECVISIBLE`, `PIERCE`,
`TARRANGE`, `CHANGE_*_RANGE`, `STUN`, `CONTINUOUS_DAMAGE`, `CURSE`,
`IGNORE_DAMAGE`, `IMMUNE`, `STIGMA`, `ENHANCEMENT`, `DEFENSE_SKILL_ACTIVE`,
`REFDAMAGE`, `TALK_TO_NPC`, `DAMAGE_LOOP`, `TAR_BUFF`, world/entity registry,
networking, client presentation, movement, potion system, status random-roll
redesign, `m_bCaptureTheFlagHoldFlag`, and the physical `DAMAGE_RATE` gap.

## 12. Final Commit

`feat: integrate fact defense and resistance`