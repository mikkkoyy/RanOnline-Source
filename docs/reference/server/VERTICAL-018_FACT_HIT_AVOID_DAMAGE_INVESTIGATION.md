# VERTICAL-018 — FACT Hit / Avoid / Damage Consumer Investigation

**Source-first investigation. No gameplay code changed.** This milestone exists to
prove where `EMIMPACTA_HITRATE`, `EMIMPACTA_AVOIDRATE` and `EMIMPACTA_DAMAGE`
belong in the modern architecture *before* anything is implemented.

---

## 1. Repository / Baseline

| | |
| --- | --- |
| Repository | `mikkkoyy/RanOnline-Source` |
| Branch | `main` (tracking `origin/main`) |
| Baseline commit | `c73aba1344ac1c05f89a4371edaff00b4602c727` |
| Starting tree | clean |

Verified before any edit. No completed system was modified to make the
investigation easier.

## 2. Public / Forum Backread

**PUBLIC/FORUM BACKREAD: NEGATIVE — PROVENANCE ONLY, NOT BEHAVIORAL EVIDENCE.**

| Source | Result |
| --- | --- |
| `https://gitlab.com/ragezone/ran-online/game-sources` | Canonical RAN source repo; directory listings are JS-rendered and returned no content |
| `https://github.com/yexiuph/RanOnline`, `https://github.com/ezrajohnnunez/ran-online-source-code`, `https://gitlab.com/jeremia49/ran-online-source` | RAN mirrors; no FACT content retrievable (`jeremia49` is visibility-limited) |
| `https://www.ran.com.tw/en_gamesite/gameinfo/skills.aspx` | Official skill list. Feature descriptions only ("Increase speed movement but will decrease accuracy and evasion") |
| `https://en.wikipedia.org/wiki/Ran_Online` | Background only |
| `https://fanra.fandom.com/wiki/Skill_Modifiers_and_Combat_Effects` | **EverQuest, not RAN — explicitly excluded** |

Two notes on why this search yields nothing usable:

- The official skill page is a *feature list*. It confirms hit and avoidance are
  player-visible concepts a skill can move, which is weak corroboration, but it
  exposes no arithmetic.
- The EverQuest wiki is a **different game**, and its stacking rules actively
  contradict RAN's. EQ states "Improved Dodge ... Does not stack with other
  Improved Dodge, only the higher one applies" — the exact opposite of RAN's
  `EMIMPACTA_*` accumulation, which is SUM. Treating it as evidence would have
  produced the wrong rule.

No claim below rests on public material. All of it is read from `legacy/`.

## 3. Legacy Source Locations

| Concern | Location |
| --- | --- |
| Impact accumulation (per-tick pass) | `GLogixExPC.cpp:2323-2350` |
| Reset / passive seed block | `GLogixExPC.cpp:2210-2241` |
| Hit check | `GLogixExPC.cpp:1291-1327` (`CHECKHIT`) |
| Hit-rate formula | `GLogicEx.cpp:35` (`GLHITRATE`) |
| Hit-check gating | `GLChar.cpp:2355-2420` (`PreStrikeProc`) |
| Avoid accessor chain | `GLChar.h:503` -> `GLogicEx.h:637` |
| Skill damage range seed | `GLogixExPC.cpp:1415` |
| Physical range build | `GLogixExPC.cpp:2995-3003` |
| Base attack range | `GLogixExPC.cpp:381` |

## 4. HITRATE Tracing

### A. Accumulation

```
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2198   m_nSUM_HIT = m_nHIT;        (reset, per tick)
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2327   m_nSUM_HIT += int(fADDON_VAR);   (skill FACT)
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2309   m_nSUM_HIT += int(fMVAR);        (EMFOR_HITRATE)
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2754   m_nSUM_HIT += int(...)           (item FACT)
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2876   m_nSUM_HIT += int(...)           (system buff)
```

- **Field:** `m_nSUM_HIT`, an `int` member of `GLCHARLOGIC` (`GLogicEx.h:405`).
- **Operation:** **SUM**, with `int()` truncation.
- **Reset:** yes — assigned `m_nHIT` at `:2198` in the per-tick pass.
- **Seeded from passives:** yes — `m_nHIT` is the permanent total, which already
  contains passive and item contributions.
- **Attacker-side or defender-side:** attacker-side. It is the caster's own hit.
- **Negative values possible:** yes in principle; `int()` truncation of a negative
  float is well defined. Nothing clamps it.

### B. Consumption — four stages

```
1. accumulator   GLogixExPC.cpp:2327   m_nSUM_HIT += int(fADDON_VAR)
2. accessor      GLogicEx.h:636        int GETHIT() const { return m_nSUM_HIT; }
3. formula       GLogixExPC.cpp:1322   float fBaseHitRate = float(GLHITRATE(GETHIT(), nAVOID, bFB));
4. comparison    GLogixExPC.cpp:1326   return (nHitRate >= (RANDOM_POS*100));
```

with the low-SP multiplier applied in between at `:1323-1324`:

```cpp
const float fMulHitRate = bLowSP ? (1.0f - GLCONST_CHAR::fLOWSP_HIT_DROP) : 1.0f;
const int   nHitRate    = int(fBaseHitRate * fMulHitRate);
```

`GLHITRATE` itself is `GLogicEx.cpp:35`.

### C. Timing — the non-obvious part

`CHECKHIT` is called from exactly **one** place on the player path,
`GLChar.cpp:2416`, inside `GLChar::PreStrikeProc(BOOL bSkill, BOOL bLowSP)`
(`GLChar.cpp:2355`). The gate is at `:2370-2406`:

```
bSkill == FALSE (basic attack)                      -> :2378  bCheckHit = TRUE;
bSkill == TRUE  and emAPPLY != EMAPPLY_MAGIC        -> :2400  bCheckHit = TRUE;
bSkill == TRUE  and emAPPLY == EMAPPLY_MAGIC        -> :2404  sTargetID.dwID = EMTARGET_NULL;
```

and `:2414` skips the check when the target is null, which is exactly what the
magic branch sets. `PreStrikeProc(TRUE, ...)` is reached from the skill cast path
at `GLCharSkillMsg.cpp:981`.

| Path | Hit check |
| --- | --- |
| Basic attack | **YES** |
| Active skill, physical / ranged (`!= EMAPPLY_MAGIC`) | **YES** |
| Active skill, **magic** (`== EMAPPLY_MAGIC`) | **NO** |

**Magic skills never roll for a hit.** This is proven by `:2402-2405` and is not
inferable from the field name.

## 5. AVOIDRATE Tracing

### A. Accumulation

```
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2199   m_nSUM_AVOID = m_nAVOID;
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2328   m_nSUM_AVOID += int(fADDON_VAR);
```

Identical shape to HITRATE: **SUM**, `int()`-truncated, reset per tick, seeded
from the permanent total.

### B. Consumption

The hit formula does **not** call `GETAVOID()`. It calls a different accessor:

```
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:1320   const int nAVOID = pActor->GetAvoid();
```

The chain resolves:

```
legacy/Lib_Client/G-Logic/GLChar.h:503        virtual int GetAvoid() const { return GETAVOID(); }
legacy/Lib_Client/G-Logic/GLogicEx.h:637      int GETAVOID() const { return m_nSUM_AVOID; }
```

So the defender's avoid **is** `m_nSUM_AVOID` and the AVOIDRATE impact does reach
the formula. This mattered: `GETAVOID()` and `GetAvoid()` look like synonyms
and a reader could easily assume one of them bypasses the accumulator. Both
hops were checked; neither does.

### C. Timing

Identical to HITRATE — the same `CHECKHIT` call, so the same basic-attack /
physical-skill / no-magic table applies.

## 6. DAMAGE Tracing

### A. Accumulation

```
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2196   m_gdDAMAGE_SKILL = m_gdDAMAGE;                  (reset)
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2329   m_gdDAMAGE_SKILL.VAR_PARAM( int(fADDON_VAR) );  (skill FACT)
```

- **Field:** `m_gdDAMAGE_SKILL`, a `GLDWDATA` range (low/high).
- **Operation:** `VAR_PARAM` — a **saturating add to BOTH ends of the range**,
  each floored at `1` (`GLDefine.h:364-371`). This is the same operation the
  attack power uses, not an addition to the final number.
- **Reset:** yes, from `m_gdDAMAGE` at `:2196` per tick.
- **Base:** `m_gdDAMAGE` is `int(m_wSUM_AP + m_sSUM_PASSIVE.m_nDAMAGE + m_dwAttackIncrease)`
  (`:381`) — the permanent attack-damage range.

### B. Consumption — and why the scope is wider than expected

```
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:1415   GLDWDATA gdDamage = m_gdDAMAGE_SKILL;   (CALCDAMAGE_20060328)
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2997   m_gdDAMAGE_PHYSIC = m_gdDAMAGE_SKILL;    (basic attack)
```

`m_gdDAMAGE_SKILL` seeds **both** the skill damage range and the basic-attack
physical range. So an `EMIMPACTA_DAMAGE` buff raises damage on **both** paths,
not only on skills.

Ordering, on each path:

| Path | Order |
| --- | --- |
| Skill (`CALCDAMAGE_20060328`) | `m_gdDAMAGE_SKILL` (incl. FACT) -> charm damage `:1430-1432` -> item damage + `VAR_PARAM(PA/SA/MA)` `:1447-1477` -> roll `:1672` |
| Basic attack (`:2995-3003`) | `m_gdDAMAGE_SKILL` (incl. FACT) -> `+= m_sSUMITEM.gdDamage` -> `VAR_PARAM(PA/SA)` -> `ApplyDamageRate` `:3005-3008` |

The FACT value is therefore applied **before** the weapon's item damage and
**before** the attack power, and long before the roll, resistance, defence,
critical and reduction stages.

### C. Answers to the specific questions

| Question | Answer |
| --- | --- |
| Modifies min, max, or both? | **Both** — `VAR_PARAM` touches both ends |
| Applied before or after the random range selection? | **Before** |
| Physical / ranged / magic? | **All three.** The skill range is consumed by `CALCDAMAGE_20060328` regardless of `emAPPLY`, and the basic path is physical |
| Applies to active skills? | **Yes**, and to basic attacks as well |
| Integer truncation? | Yes — `int(fADDON_VAR)`, then `VAR_PARAM` |
| Can it be negative? | Yes in principle. `VAR_PARAM` adds a signed value, so a negative impact reduces both ends, floored at 1 |
| Final damage? | **No.** It never reaches the post-defence figure |

## 7. Modern Ownership Analysis

### HITRATE and AVOIDRATE

Both owners already exist and are already wired at **both** boundaries.

| Legacy | Modern owner | Wired at |
| --- | --- | --- |
| `GETHIT()` -> `HitInput::attackerHit` | `Combat::HitInput::attackerHit` | `CombatCalculator.h:155`; fed from `DerivedStats::hit` by `ServerCharacter.cpp:717` and `ActiveSkill.cpp:373` |
| `GetAvoid()` -> `HitInput::targetAvoid` | `Combat::HitInput::targetAvoid` | `CombatCalculator.h:156`; fed from `DerivedStats::avoid` by `ServerCharacter.cpp:732` and `ActiveSkill.cpp:401` |

`HitCalculator.h` already reproduces the legacy formula exactly, including the
detail that matters: `outResult.hit = (hitRate >= hitRoll)` matches legacy's
`:1326 nHitRate >= (RANDOM_POS*100)`. (`GameRandom::CheckProbability`'s strict
`<` is used for the *status* checks, not the hit check — no discrepancy.)

`DerivedStats::hit` / `::avoid` are already built as
`dex-derived base + items + passives + codex`, then a percentage multiplier
(`StatCalculator.cpp:350-362`). Adding `FactContribution::hit` / `::avoid` into
the same additive run is the direct analogue of VERTICAL-017's PA/SA/MA, and
keeps the separation the milestone brief requires:

```
Base + Equipment + Passive + Codex + Timed FACT = Derived result
```

### DAMAGE

| Legacy | Modern owner | Status |
| --- | --- | --- |
| `m_gdDAMAGE_SKILL` -> skill range | `CombatInput::attackerPhysicalDamage` -> `MagicDamageInput::skillRange` (`CombatCalculator.h:184`) | exists |
| `m_gdDAMAGE_SKILL` -> basic range | `CombatInput::attackerPhysicalDamage` -> `PhysicalDamageInput::physicalDamage` (`CombatCalculator.h:216`) | exists |

Modern already uses **one** range field for both paths, which matches legacy's
single `m_gdDAMAGE_SKILL` seeding both. `ActiveSkill.cpp:368-377` builds that
range and already adds the skill magnitude to it:

```cpp
Stats::DamageRange range = input.attacker.physicalDamage;
range.low  += result.basicDamage;
range.high += result.basicDamage;
combat.attackerPhysicalDamage = range;
```

That is the exact structural analogue of legacy's `nVAR` term at `:1530-1531`. A
FACT damage scalar belongs beside it, added to both ends before the calculator
applies the attack power.

## 8. Aggregation Rules

| Impact | Rule | Truncation | Order-dependent? |
| --- | --- | --- | --- |
| `HITRATE` | SUM | `int()` | No |
| `AVOIDRATE` | SUM | `int()` | No |
| `DAMAGE` | SUM via `VAR_PARAM` on both range ends | `int()`, then saturating add floored at 1 | No |

All three are **additive**, which is the opposite of the damage-reduction and
reflection specs wired in VERTICAL-017 (MAX). A reader assuming "buffs take the
strongest" would get all three wrong.

## 9. Expiry Behavior

The `reset -> seed -> accumulate` model (VERTICAL-017 §2) still applies unchanged:

- `m_nSUM_HIT = m_nHIT` (`:2198`), `m_nSUM_AVOID = m_nAVOID` (`:2199`),
  `m_gdDAMAGE_SKILL = m_gdDAMAGE` (`:2196`) are all reassigned each tick.

So an expired FACT contributes nothing on the next pass, with **no save/restore**
anywhere. The VERTICAL-015 off-by-one still applies: a FACT contributes on the
tick it expires, because `DISABLESKEFF` (`:2295`) runs after the loop's
`continue` guard (`:2280`) has already passed.

For DAMAGE the modern equivalent needs care, because the derived stats are a
cached snapshot: `RefreshFactStatsIfPowersChanged` (added in VERTICAL-017)
currently watches only the three power values. A DAMAGE buff would either need
that guard extended or the value supplied at the combat-input boundary. Legacy
reads `m_gdDAMAGE_SKILL` live each tick, so it has no such issue; this is a
consequence of modern's caching and is recorded rather than assumed away.

## 10. Implementation Decision

**DOCUMENTATION ONLY.**

All three impacts are **PROVEN**, and each has a conclusively identified existing
modern owner. That satisfies the brief's bar. Implementation was still withheld,
for reasons of sequencing rather than doubt:

- All three are accumulated in the **same switch** (`:2327`, `:2328`, `:2329`) and
  all three belong to the **same `FactContribution`** that VERTICAL-017 created.
  Landing two of them now and the third in a later milestone would split one
  subsystem across two commits for no benefit.
- `DAMAGE` needs a decision VERTICAL-017 deliberately avoided: it requires adding
  a field to **both** damage calculators and fixing its position relative to the
  attack-power `VAR_PARAM`. That is a real change to VERTICAL-006 and
  VERTICAL-013 code, and it deserves its own reviewed commit rather than riding
  along with two one-line stat additions.
- `HITRATE` and `AVOIDRATE` individually qualify as "trivial, already-owned
  handoffs". They are held back so the three arrive together.

The exact handoff points are recorded in §7 so no further investigation is
needed.

## 11. Deferred Items

Unchanged and not touched: `EMIMPACTA_CHANGESTATS`, recovery/CP impacts,
`CP_AUTO`, `DEFENSE`, `DEFENSE_RATE`, `RESIST`, `VARHP`, `VARMP`, `VARSP`,
`VARAP`, `HP_RATE`, `MP_RATE`, `SP_RATE`, the physical `DAMAGE_RATE` gap,
`MOVEVELO`, `ATTACKVELO`, `PROHIBIT_POTION`, `INVISIBLE`, `RECVISIBLE`, `PIERCE`,
`TARRANGE`, `CHANGE_*_RANGE`, `STUN`, `CONTINUOUS_DAMAGE`, `CURSE`,
`IGNORE_DAMAGE`, `IMMUNE`, `STIGMA`, `ENHANCEMENT`, `DEFENSE_SKILL_ACTIVE`,
`REFDAMAGE`, `TALK_TO_NPC`, `DAMAGE_LOOP`, `TAR_BUFF`, world/entity registry,
networking, client presentation, movement, potion system, status random-roll
redesign.

**The physical `DAMAGE_RATE` gap was deliberately not fixed**, per the brief.

Newly identified, recorded only:

- `m_bCaptureTheFlagHoldFlag` — DEFERRED — NOT PROVEN (VERTICAL-017)
- the status random roll for the `CastSkill` path — DEFERRED
- the cached-snapshot interaction for a DAMAGE buff (§9)

## 12. Tests

No tests were added, because no code changed. Existing suites were run as
regression validation and are unchanged:

| Suite | Result |
| --- | --- |
| ModernCoreTests | 441 |
| ModernServerTests | 94 |
| Client suites | 12/12 |
| CTest Debug | 14/14 |
| CTest Release | 14/14 |
| Debug build | 0 errors, 0 warnings |
| Release build | 0 errors, 0 warnings |

No speculative coverage was added for unimplemented consumers.

## 13. Final Git State

Documentation only. Commit message
`docs: investigate FACT hit avoid and damage consumers`, pushed to `origin/main`,
`HEAD == origin/main`, working tree clean.