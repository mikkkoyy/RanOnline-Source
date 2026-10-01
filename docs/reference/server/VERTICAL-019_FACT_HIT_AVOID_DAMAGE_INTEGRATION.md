# VERTICAL-019 — FACT Hit / Avoid / Damage Integration

## 1. Baseline

| | |
| --- | --- |
| Repository | `mikkkoyy/RanOnline-Source` |
| Branch | `main` (tracking `origin/main`) |
| Baseline commit | `fc02b2aaf680bc2867224256b3229cfa49c6a00f` |
| Starting tree | clean |

Implements the three consumers VERTICAL-018 proved. No new subsystem: each
attached to the owner the investigation identified.

## 2. Legacy Evidence

```
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2327   m_nSUM_HIT   += int(fADDON_VAR);
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2328   m_nSUM_AVOID += int(fADDON_VAR);
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2329   m_gdDAMAGE_SKILL.VAR_PARAM( int(fADDON_VAR) );
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2198   m_nSUM_HIT = m_nHIT;              (reset)
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2199   m_nSUM_AVOID = m_nAVOID;          (reset)
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2196   m_gdDAMAGE_SKILL = m_gdDAMAGE;    (reset)
legacy/Lib_Client/G-Logic/GLogicEx.h:636-637    GETHIT() / GETAVOID()
legacy/Lib_Client/G-Logic/GLChar.h:503          GetAvoid() -> GETAVOID()
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:1322   GLHITRATE(GETHIT(), nAVOID, bFB)
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:1326   return (nHitRate >= (RANDOM_POS*100));
legacy/Lib_Client/G-Logic/GLChar.cpp:2402-2405  magic -> sTargetID.dwID = EMTARGET_NULL
legacy/Lib_Client/G-Logic/GLDefine.h:364-371    VAR_PARAM saturating add, floor 1
```

All three are **SUM with `int()` truncation** — the opposite rule to the damage
reduction specs wired in VERTICAL-017, which take a maximum.

## 3. HITRATE Implementation

```
SkillFactModifiers::hit            (aggregator, SUM + int truncation)
    -> Stats::FactContribution::hit
    -> StatCalculationInput::facts
    -> DerivedStats::hit
    -> CombatInput::attackerHit
    -> HitInput::attackerHit
```

The value joins the same additive run as items, passives and codex
(`StatCalculator.cpp:348-355`) and is inside the percentage scaling, which is
where every other flat hit source sits. No new hit subsystem was created.

## 4. AVOIDRATE Implementation

Identical shape, defender-side:

```
SkillFactModifiers::avoid -> FactContribution::avoid -> DerivedStats::avoid
    -> CombatInput::targetAvoid -> HitInput::targetAvoid
```

Legacy's `m_nSUM_AVOID` reaches the hit formula through
`GetAvoid() -> GETAVOID()`, which was verified hop by hop in VERTICAL-018 rather
than assumed from the names.

## 5. Magic Hit-Check Exclusion

`HitCalculator.h` was **not touched**. It already reproduced legacy, including
the `>=` at the comparison — which is a different rule from the strict `<` used
by `GameRandom::CheckProbability` for status rolls. Confusing the two would have
been an easy and silent regression, so the existing code was verified and left
alone.

The substantive change is the converse: `ServerCharacter::CastSkill` previously
consulted `result.combat.IsHit()` for **every** channel, including magic. That
contradicts `GLChar.cpp:2402-2405`, where a magic skill's target is set to
`EMTARGET_NULL` and the null target skips `CHECKHIT` at `:2414`. Without this,
adding a hit/avoid buff would have let a magic skill miss — something RAN cannot
do.

```cpp
const bool channelRollsForHit = result.attackTypeUsed != Combat::AttackType::Magic;
if (!channelRollsForHit || result.combat.IsHit()) { /* apply damage */ }
```

The hit result is still computed; it is simply not consulted for magic, which is
exactly what legacy leaves behind.

This is proved **behaviourally, not by inspecting a field**:
`ServerFactV019_MagicSkillIgnoresHitAndAvoidBuffs` gives the attacker a hopeless
hit and the target an enormous avoid, then shows the magic cast still damages.
Its complement, `ServerFactV019_PhysicalSkillStillRollsForHit`, shows the same
setup makes a physical skill miss — without it, the magic case would pass even
if the buffs did nothing at all.

## 6. DAMAGE Implementation

```
SkillFactModifiers::damage   (SUM + int truncation)
    -> ActiveSkillInput::factDamage   /   CombatInput::factDamage
    -> PhysicalDamageInput::factDamage / MagicDamageInput::factDamage
    -> ApplyAttackPower on BOTH range ends, BEFORE the attack power
```

One field serves every channel, matching legacy's single `m_gdDAMAGE_SKILL`
(`:1415` for skills, `:2997` for the basic-attack range). No separate
`attackerMagicDamageFact` was introduced.

## 7. Damage Ordering

```
base range
  -> FACT DAMAGE        ApplyAttackPower, both ends, floor 1
  -> weapon item damage (already folded into the incoming range by the stat layer)
  -> attack power       ApplyAttackPower, both ends
  -> ... existing modifiers unchanged
```

Legacy order (`:2329`, then `:2997-3002` / `:1451`):

```
m_gdDAMAGE -> +FACT (VAR_PARAM) -> +m_sSUMITEM.gdDamage -> VAR_PARAM(PA/SA/MA)
```

Plain addition commutes, so the only observable ordering is against the attack
power, and that is preserved exactly. Ordering is pinned by
`FactDamage_IsAppliedBeforeTheAttackPower`, which uses the `VAR_PARAM` floor to
make the two orders distinguishable: on range `{1,1}` with a `-1` FACT and `+10`
power, FACT-first gives **11** while power-first would give **10**.

## 8. Integer / Truncation Behaviour

`int(fADDON_VAR)` truncates toward zero at the accumulator
(`SkillFactV019_TruncationIsTowardZero`: `7.9 -> 7`, `-2.7 -> -2`). The range
application is then the saturating `VAR_PARAM`, which floors each end at `1`
rather than wrapping (`FactDamage_VarParamFloorIsOne`).

Negative values are carried unchanged, as legacy does — nothing clamps them at
the accumulator, and `VAR_PARAM` only intervenes at the floor.

## 9. Expiry Behaviour

No `previousHit` / `previousAvoid` / `previousDamage` state exists anywhere.
Every value disappears through the existing `reset -> seed -> accumulate`
rebuild, and `RefreshFactStatsIfPowersChanged` was extended to watch hit and
avoid alongside the powers so the cached `DerivedStats` snapshot stays honest.

The VERTICAL-015 off-by-one still applies and is still reproduced: a FACT
contributes on the tick it expires.

## 10. Tests

| Suite | Before | After |
| --- | --- | --- |
| ModernCoreTests | 441 | **455** (+14) |
| ModernServerTests | 94 | **101** (+7) |
| CTest | 14/14 | 14/14 (Debug and Release) |
| Client suites | 12/12 | 12/12 |

**Aggregation** (`SkillFactTests.cpp`): baseline; hit 5+7+3=15; avoid 9+4=13;
damage 10+6=16; truncation toward zero; negative values; expiry of all three.

**Damage ordering** (`CombatTests.cpp`): no FACT; both ends move; ordering
against the power at the floor; `VAR_PARAM` floor of 1; ranged uses the same
value; magic uses the same field and operation; expiry.

**Server integration** (`ServerCharacterTests.cpp`): hit raises the derived
value; two facts SUM; avoid raises; expiry restores the exact baseline; a damage
buff raises skill damage; magic ignores hopeless hit/avoid; physical still rolls.

No existing test was modified, weakened or deleted.

## 11. Build Results

Debug and Release: 0 errors, 0 warnings. CTest 14/14 in both.
`ModernCoreTests` 455/455 and `ModernServerTests` 101/101 in both configurations.

## 12. Deferred Scope

Untouched, as instructed: the physical `DAMAGE_RATE` gap, `EMIMPACTA_CHANGESTATS`,
recovery/CP impacts, `CP_AUTO`, `DEFENSE`, `DEFENSE_RATE`, `RESIST`, `VARHP`,
`VARMP`, `VARSP`, `VARAP`, `HP_RATE`, `MP_RATE`, `SP_RATE`, `MOVEVELO`,
`ATTACKVELO`, `PROHIBIT_POTION`, `INVISIBLE`, `RECVISIBLE`, `PIERCE`,
`TARRANGE`, `CHANGE_*_RANGE`, `STUN`, `CONTINUOUS_DAMAGE`, `CURSE`,
`IGNORE_DAMAGE`, `IMMUNE`, `STIGMA`, `ENHANCEMENT`, `DEFENSE_SKILL_ACTIVE`,
`REFDAMAGE`, `TALK_TO_NPC`, `DAMAGE_LOOP`, `TAR_BUFF`, world/entity registry,
networking, client presentation, movement, potion system, status random-roll
redesign, `m_bCaptureTheFlagHoldFlag`.

Carried forward from earlier milestones: `HitCalculator` was verified and left
alone; the status random roll for the `CastSkill` path remains DEFERRED.

## 13. Exact Modern Evidence

```
modern/core/skills/SkillFactAggregator.h   hit/avoid/damage aggregation
modern/core/skills/ActiveSkill.h/.cpp      factDamage -> CombatInput
modern/core/stats/Contributions.h          FactContribution::hit / ::avoid
modern/core/stats/StatCalculator.h/.cpp    facts folded into hitRaw / avoidRaw
modern/core/combat/CombatTypes.h           PhysicalDamageInput::factDamage
modern/core/combat/CombatCalculator.h      CombatInput::factDamage -> both channels
modern/core/combat/PhysicalDamageCalculator.h  VAR_PARAM before the power
modern/core/combat/MagicDamageCalculator.h      VAR_PARAM before the power
modern/server/character/ServerCharacter.cpp stat input, basic attack, cast, magic gate
```

Deliberately unchanged: `HitCalculator.h`, `StatusEffectResolver.h`,
`StatusEffectContainer.h`, `SkillFactContainer.h`, `SkillFactTypes.h`, and
`GLCONST_CHAR`-equivalent constants.