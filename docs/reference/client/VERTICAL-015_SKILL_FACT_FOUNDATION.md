# VERTICAL-015 — Skill FACT / buff foundation

## Summary

FACT is RAN's *other* persistent-effect mechanism and is not a status effect.
VERTICAL-014 modelled `EMSTATE_BLOW` — short ailments in four shared slots.
This milestone models `SSKILLFACT` — timed enhancements in fourteen slots, keyed
by skill, carrying impacts and specs. They stay in separate domains because they
differ in storage, lifetime, slot rules and consumers.

The central finding is that **slot selection is a three-rule cascade, and its
third rule evicts the buff nearest to expiry**. It is not a hash, not a scan for
a free slot, and not strongest-wins. With a full pool RAN silently drops the
shortest-lived buff to make room and never refuses the new one.

---

## 1. Repository check

`main` at `24b5f652509e6887ae9b9fbad199d0675c96e4f2`, matching `origin/main`,
working tree clean.

## 2. Public / forum backread — largely NEGATIVE

Searched `SSKILLFACT`, `SKILLFACT_SIZE`, `GLFactData`, `RECEIVE_SKILLFACT`,
`EMSPECA_*`, `EMIMPACTA_*` and `TAR_BUFF`.

One marginally useful result: the RAN Aki wiki skill page for id `11/0` presents
a per-level table with **cooldown, duration, power cost, AoE, target form,
character level and SKP**. That corroborates the source finding that `fLIFE` is
per-level skill data indexed by level (`sDATA_LVL[wlevel].fLIFE`,
`GLChar.cpp:6609`), which this milestone relies on. It contains no
implementation-level detail about FACT slots, aggregation or expiry.

Everything else returned was unrelated: a locked RAN forum index with empty
threads, a Ragnarok 3 fan database, and generic build guides.

**Nothing from the public web is treated as evidence.** All behaviour below
comes from the checked-in tree.

## 3. Legacy files inspected

| Concern | Location |
| --- | --- |
| Record layout | `GLFactData.h:20-36, 50-74` |
| Pool size | `GLCharData.h:200-201` (`SKILLREALFACT_SIZE`, `SKILLFACT_SIZE`, both 14) |
| Impact/spec counts | `GLSkillDefine.h:17-18` (`MAX_IMPACT`, `MAX_SPEC`, both 5) |
| Slot selection | `GLChar.cpp:6377-6408` |
| Creation | `GLChar.cpp:6410-6613` (`RECEIVE_SKILLFACT`) |
| Aggregation / consumption | `GLogixExPC.cpp:2255-2410` |
| Reset of accumulators | `GLogixExPC.cpp:2255-2281` |
| Disable | `GLCharClient.h:241` (`DISABLESKEFF`) |
| Enums | `GLCharDefine.h` (`EMIMPACT_ADDON`, `EMSPEC_ADDON`) |
| Null skill id | `GLDefine.h:129-136`, `ID_NULL = 0xFFFF` |

## 4. Structure

`SSKILLFACT` (`GLFactData.h:50`) holds: skill native id, level, `fAGE`,
`emTYPE`/`fMVAR`, `dwSpecialSkill`, `bRanderSpecialEffect`, up to five impacts,
up to five specs, and the caster crow/id.

Pool size is **14** and is treated as a shipped constant, not a preference.

## 5. Slot selection — the important part

`GLChar::SELECT_SKILLSLOT` (`GLChar.cpp:6377-6408`):

```cpp
for ( i < SKILLREALFACT_SIZE )
    if ( m_sSKILLFACT[i].sNATIVEID == skill_id )  return i;      // 1. refresh

fAGE = FLT_MAX;
for ( i < SKILLREALFACT_SIZE ) {
    if ( m_sSKILLFACT[i].sNATIVEID == SNATIVEID(false) )  return i;   // 2. first empty
    if ( m_sSKILLFACT[i].fAGE < fAGE ) { fAGE = ...; dwSELECT = i; } // 3. min remaining
}
return dwSELECT;
```

Three consequences, each tested:

1. **Re-casting a buff refreshes its own slot.** It does not consume a second
   slot, and it does not stack.
2. **The first empty slot always wins over eviction**, because the `return` sits
   inside the same loop. Eviction is only reachable once all fourteen slots are
   occupied.
3. **A full pool evicts the smallest remaining lifetime.** Not the strongest
   buff, not the longest, and the new buff is never refused.

`SkillFact_FullPoolEvictsTheNearestToExpiry` fills all fourteen slots with
distinct lifetimes and asserts slot 13 — the shortest — is the one displaced.
`SkillFact_PartiallyFullPoolPrefersTheEmptySlotOverEviction` pins that a pool
with a free slot never evicts, which is the rule most likely to be misread.

## 6. Creation — `RECEIVE_SKILLFACT`

`GLChar.cpp:6519-6613` copies a **whitelisted** subset, and only if `bHOLD`
becomes true:

- basic `emTYPE`/`fMVAR`, but only for 14 specific `EMFOR_*` types (`:6521-6538`)
- every non-`NONE` impact, with `fADDON_VAR[wlevel]` (`:6541-6549`)
- every non-`NULL` spec **from an explicit case list** (`:6551-6601`), copying
  `fVAR1`, `fVAR2`, `dwFLAG` and `dwNativeID`
- then identity, level, and `fAGE = sSKILL_DATA.fLIFE` (`:6607-6609`)

`SkillFactDefinition::createsFact` is the modern stand-in for `bHOLD`, and the
gate is **re-evaluated at cast time** so a definition claiming a FACT but
carrying nothing stores nothing.

Note that legacy whitelists specs by name rather than copying the array
wholesale — so a spec legacy does not list is not stored. The modern
`factSpecs` array is explicit too, which reproduces that.

## 7. Lifetime — and a reproduced off-by-one

`fAGE` is the **remaining** lifetime: assigned `fLIFE` at creation and
decremented per tick. Expiry is `<= 0`.

Two structural facts about the legacy function drive the design:

**Recompute from zero.** The accumulators are reset to their defaults at the top
(`m_bProhibitSkill = false;` at `:2256`, and the lines above it) and rebuilt from
the live pool every call. So there is **no restore step** anywhere — expiry
restores the baseline simply by ceasing to be counted. `AdvanceSkillFacts`
mirrors this.

**An expiring fact still contributes on its final tick.** `:2292-2295` decrements
`fAGE` and calls `DISABLESKEFF`, but the spec switches below still run, because
the loop already passed its `continue` guard and `DISABLESKEFF` only nulls the
skill id. This is reproduced, not tidied, and
`SkillFact_ExpiringFactStillContributesOnItsFinalTick` pins it. It is a
one-tick grace period, which is easy to mistake for a bug in the modern code if
you have not read the legacy.

## 8. Aggregation semantics — where "stacking" mostly is not

From `GLogixExPC.cpp:2353-2408`:

| Spec | Rule | Line |
| --- | --- | --- |
| `MOVEVELO` | `+=` — plain additive | :2360 |
| `ATTACKVELO` | `-=` — **sign-inverted** | :2364 |
| `NONBLOW` | `=` — **assignment, not OR** | :2357 |
| `PROHIBIT_SKILL` | `= true` | :2377 |
| `PROHIBIT_POTION` | `= true` | :2374 |
| `PSY_DAMAGE_REDUCE` | `max` | :2379-2382 |
| `MAGIC_DAMAGE_REDUCE` | `max` | :2384-2387 |
| `PSY_DAMAGE_REFLECTION` | `max`, rate paired from the same spec | :2389-2394 |
| `MAGIC_DAMAGE_REFLECTION` | `max`, rate paired from the same spec | :2396-2401 |

Three things here are genuinely surprising and each has a test:

- **`ATTACKVELO` subtracts.** A positive `fSPECVAR1` makes the attacker
  *slower*. Legacy's own comment at `:2363` records that a `-0.1` value is
  entered to get 10% faster, which only parses with the subtraction.
  `SkillFact_AttackVelocityIsSignInverted` and
  `SkillFact_NegativeAttackVelocitySpeedsUp` pin both directions.
- **Damage reduction is a maximum, not a sum.** Two 0.2 reduction buffs give
  0.2, not 0.4.
- **`NONBLOW` assigns the mask rather than OR-ing it.** Two immunity buffs
  holding different masks do not combine — the last one aggregated replaces the
  earlier one outright. `SkillFact_NonBlowMaskIsAssignedNotCombined` pins it.
  Whether that is intentional is unknowable from the source; it is what the code
  does.

## 9. Impacts — stored, not aggregated

Every `EMIMPACTA_*` consumer at `:2327-2349` feeds a different subsystem: PA/SA/MA
into the derived-stat attack powers, `DAMAGE` into the skill damage range,
`HITRATE`/`AVOIDRATE` into the hit calculator, the `VAR*` family into resource
pools, and the `RATE` family into resource maxima.

Wiring any of them into this aggregator would either duplicate an existing
authoritative calculation (violating §12) or require subsystems that do not
exist. All impacts are therefore **stored faithfully and left unaggregated**,
recorded as DEFERRED rather than guessed at. The values survive in the record,
so a later slice needs no re-derivation.

## 10. Modern mapping

```
modern/core/skills/
  SkillFactTypes.h       runtime record, slot constants, the EMFOR whitelist
  SkillFactContainer.h   the 14 slots and the three-rule slot selection
  SkillFactAggregator.h  the single-pass advance + aggregation
```

The impact/spec **enums** live in `SkillDefinition.h` rather than
`SkillFactTypes.h`, because a `SkillDefinition` has to carry them and
`SkillFactTypes.h` depends on that header; declaring them in both directions
would be a circular include.

Namespace is `Modern::Skills`. `Modern::Status` was avoided — it is already the
project's error wrapper, and `Modern::StatusEffect` is VERTICAL-014's domain.

### A correctness fix that the FACT pool forced

`SkillId{}` default-constructs to `{0,0}`, which **is a valid skill id** — only
`0xFFFF` is not. A default-constructed FACT therefore looked occupied, and an
empty pool reported fourteen active facts. Legacy avoids this because its empty
marker is `SNATIVEID(false)` = `{0xFFFF,0xFFFF}` (`GLDefine.h:129-136`).

`SkillId::Invalid()` was added to express that marker explicitly. This was a real
latent trap in the shared type, not something specific to FACT.

## 11. Integration

`ActiveSkillResolver` returns `hasSkillFact` plus the built `skillFact` — data,
never persistent state. `ServerCharacter` owns the pool and stores the record on
the **target**, mirroring legacy's two-step. The immunity mask crosses into
VERTICAL-014's `StatusEffectResolver` as a plain value; the domains never reach
into each other.

`ActiveSkillResolver::skillProhibited` was **not** re-plumbed to FACT yet. The
flag exists and the aggregator produces `prohibitSkill`, but wiring it through
`CastSkill` would change existing refusal behaviour without a proven
server-side call site in this slice, so it is recorded as a documented
follow-up rather than done speculatively.

## 12. Deferred, with reasons

Impacts (all — §9), `TAR_BUFF` targeting, zone/area targeting, world and entity
registry, networking, client presentation, invisibility, forced animation,
projectiles, weather, continuous damage, curse damage, land effects, item
effects, item food FACTs, system buffs, stigma, skill-release systems, skill
illusion, skill-delay additions, and `prohibitSkill` plumbing into `CastSkill`.

Two enum values are **provisional**: `EMSPECA_PROHIBIT_POTION` and
`EMSPECA_PROHIBIT_SKILL` were assigned 30 and 31 because legacy's enum runs past
the block inspected. The aggregation treats them as flags, so the numbering has
no behavioural effect, but it is recorded rather than passed off as transcribed.

## 13. Verification

| Suite | Result |
| --- | --- |
| ModernCoreTests | 425 (was 383; +42) |
| ModernServerTests | 82 (was 76; +6) |
| CTest | 14/14 Debug and Release |
| Client suites | 12/12 |
| Builds | Debug and Release, 0 errors, 0 warnings |

No existing test was modified, weakened or deleted. VERTICAL-001 through
VERTICAL-014 expectations are untouched; the FACT domain is additive and shares
no code path with the combat formulas or the status pool.

---

## References

- `legacy/Lib_Client/G-Logic/GLFactData.h:20-36, 50-74`
- `legacy/Lib_Client/G-Logic/GLChar.cpp:6377-6408, 6410-6613`
- `legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2255-2410`
- `legacy/Lib_Client/G-Logic/GLCharData.h:200-201`
- `legacy/Lib_Client/G-Logic/GLCharDefine.h` (`EMIMPACT_ADDON`, `EMSPEC_ADDON`)
- `legacy/Lib_Client/G-Logic/GLCharClient.h:241`
- `legacy/Lib_Engine/G-Logic/GLDefine.h:129-136`
- `modern/core/skills/SkillFactTypes.h`
- `modern/core/skills/SkillFactContainer.h`
- `modern/core/skills/SkillFactAggregator.h`