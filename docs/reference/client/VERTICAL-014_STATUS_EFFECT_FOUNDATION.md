# VERTICAL-014 — Status effect foundation

## Summary

Status effects ("state blows") are a **four-slot pool with shared occupancy**, not
one slot per ailment. That single fact drives most of this milestone's
behaviour, and it is the opposite of what the `EMSTATE_BLOW` enumeration
suggests at a glance.

Executed scope is the pure domain plus server authority: probability, duration,
slot assignment, expiry, cure, and integration with the existing active-skill
cast. Deliberately excluded: the buff/FACT system, damage-over-time, and any
movement or attack-speed effects, because none of them is traced in this
milestone.

---

## 1. Repository check

`main` at `27d0ade226a775457c50985a8bc0cb93efc806e7`, matching `origin/main`,
working tree clean. Baseline confirmed before any change.

## 2. Public / forum backread

**PUBLIC/FORUM BACKREAD: NEGATIVE**

Searched for `EMSTATE_BLOW`, `EMBLOW_*`, `CHECKSTATEBLOW`, `SSKILLFACT`,
`EMSPECA_*` and `STATE_TO_ELEMENT`. The only RAN-specific result was an empty,
locked YUM RAN Online forum index — thread listings with zero replies and no
content. Everything else was the official marketing feature page and an
unrelated statistical-physics paper.

Nothing was found that adds to the checked-in legacy tree, and nothing was
adopted from it. This is the third consecutive milestone where the public web
supplied nothing usable; the pattern is now recorded rather than rediscovered.

## 3. Legacy sources actually used

| Concern | Location |
| --- | --- |
| Enum, disorder map, element map | `GLCharDefine.h:894-957` |
| Per-skill blow data | `GLSkillApply.h:67-79` (`SSTATE_BLOW`), `:436`+ (`sSTATE_BLOW[MAX_LEVEL]`) |
| Runtime state record | `GLCharData.h:1059-1074` (`SSTATEBLOW`) |
| Slot storage | `GLCharClient.h:106` (`m_sSTATEBLOWS[EMBLOW_MULTI]`) |
| Skill blow resolution | `GLChar.cpp:3355-3397` |
| Weapon blow (basic attack) | `GLChar.cpp:2554-2598` |
| Probability formula | `GameCharacterCalculations.cpp:244-279`, wrapper `GLogicEx.cpp:198-208` |
| Level table | `GLogicData.cpp:309`, `GLogicData.h:508-509` |
| Slot assignment | `GLChar.cpp:6200-6226` |
| Cure | `GLChar.cpp:6228-6243` |
| Ticking / expiry | `GLCharClient.cpp:3772`, `GLFactEffect.cpp:151-160` |
| Immunity mask | `GLogixExPC.cpp:2357` (`EMSPECA_NONBLOW`) |

## 4. State identity — a four-slot pool, not one slot per state

`GLChar.cpp:6204-6207`:

```cpp
int nIndex = 0;
if ( sStateBlow.emBLOW <= EMBLOW_SINGLE )  nIndex = 0;
else                                       nIndex = sStateBlow.emBLOW-EMBLOW_SINGLE;
m_sSTATEBLOWS[nIndex] = sStateBlow;
```

`EMBLOW_SINGLE` is **5** (`GLCharDefine.h:915`) — and it is an *alias of
`EMBLOW_FROZEN`*, not a distinct state. So:

| Slot | States |
| --- | --- |
| 0 | Numb, Stun, Stone, Burn, Frozen — **all five share it** |
| 1 | Mad |
| 2 | Poison |
| 3 | Curse |

`EMBLOW_MULTI = 4` gives the array length.

The consequence is not subtle and is asserted directly: **applying Burn to a
stunned target overwrites and destroys the stun early**, because it is a plain
assignment into a shared slot. `StatusContainer_BurnOverwritesAnExistingStun`
pins it.

## 5. Application probability — `CHECKSTATEBLOW`

`GameCharacterCalculations.cpp:259-279`:

```cpp
int nDXLEVEL = int(wLEVEL - wACTLEVEL);
int nINDEX   = nDXLEVEL + nStateBlowLevelBase;          // BASE = 1
if (nINDEX < 0)                    nINDEX = 0;
if (nINDEX >= nStateBlowLevelSize) nINDEX = nStateBlowLevelSize - 1;
float fThreshold = fACTRATE - fACTRATE * 0.01f * wRESIST * 0.6f
                 + nStateBlowLevel[nINDEX];
return (RANDOM_POS * 100.0f) < fThreshold;
```

`nSTATEBLOW_LEVEL` (`GLogicData.cpp:309`):

```
+10, +8, +6, +3, 0, -2, -4, -6, -8, -10
```

The difference is `target - attacker`, so a **stronger attacker gets a higher
threshold**, and the table is added in raw percentage points. The comparison is
`roll * 100 < threshold`, strict, with the roll on the left. That direction is
preserved exactly rather than "improved"; the roll is injected and no RNG is
called.

The table is **data** (`GLogicDataLoad.cpp:236-237`), so it is modelled as data
rather than baked in as a rule.

## 6. Resistance — a verified legacy bug, reproduced deliberately

`GLChar.cpp:3369` (and `:2561`, `:2619`, `:4027`, `:4495`):

```cpp
short nBLOWRESIST = pACTOR->GETRESIST().GetElement(...);
if ( nBLOWRESIST > GLCONST_CHAR::fRESIST_G )  nBLOWRESIST = fRESIST_G;
```

The ceiling should be `fMAX_RESIST` (`99.0f`), which is the constant legacy uses
for precisely this purpose a few hundred lines away at `GLogixExPC.cpp:1516`.
Instead it compares against `fRESIST_G`, which is **`0.5f`**
(`GLogicData.cpp:261`). Assigning `0.5f` to a `short` truncates to **0**, so
every resistance of 1 or more is clamped to zero and resistance never affects the
threshold or the duration.

**Decision: reproduced, not corrected.** This migration exists to match RAN, and
silently fixing a clamp would diverge from the client players actually play. The
ceiling is a named parameter (`StatusConstants::resistClampCeiling`, defaulting
to `0.5f`) so adopting the intended `fMAX_RESIST` is a one-line data change
rather than a hunt through the rules.

Both behaviours are tested: `StatusEffect_LegacyResistClampZeroesRealResistance`
pins the shipped behaviour, and
`StatusEffect_WithIntendedClampResistanceReducesThreshold` proves the intended
clamp is reachable and shows the threshold arithmetic it produces.

**This is the one decision in this milestone most worth a second opinion.**

## 7. Duration

`GLChar.cpp:3386-3390`:

```cpp
float fLIFE = sBLOW.fLIFE * fPOWER;
fLIFE = ( fLIFE - (fLIFE*nBLOWRESIST/100.0f*GLCONST_CHAR::fRESIST_G ) );
sSTATEBLOW.fAGE = fLIFE;
```

`fAGE` is named "age" but holds the **remaining lifetime**: it is assigned the
computed duration at creation and then decremented
(`sSTATEBLOW.fAGE -= fElapsedTime`, `GLCharClient.cpp:3772`), expiring at
`<= 0.0f` (`GLFactEffect.cpp:156`). The modern field is named
`remainingLifetime` so the direction is not something a reader has to deduce.

The expiry boundary is tested at all three points: short of it, exactly on it
(where `<= 0` means expired), and beyond it.

## 8. Stacking

There is no merge, no refresh, no "keep the stronger" and no stacking — legacy
does a plain assignment. So:

- Re-applying the same state **resets** its duration.
- Applying a different single-slot state **destroys** the previous one.

Both are asserted. No stacking rule was invented to fill an apparent gap.

## 9. Cure

`GLChar.cpp:6228-6243`: `dwCUREFLAG` is a `EMDISORDER` **bitmask**, and every
slot whose disorder intersects it is cleared. Sources observed:
`DIS_ALL` (death/reset, `GLCharMsg.cpp:2113`) and item drugs
(`GLCharInvenMsg.cpp:5162`). Note legacy clears only `emBLOW`, leaving `fAGE`
and the variables on the dead slot; that is unreachable through the public
surface, so the container zeroes the whole slot.

## 10. Immunity — and a correction of a natural assumption

`GLChar.cpp:3376-3379` gates the probability check on:

```cpp
if ( !(pACTOR->GETHOLDBLOW() & STATE_TO_DISORDER(sBLOW.emTYPE)) )
    bBLOW = CHECKSTATEBLOW(...);
```

`m_dwHOLDBLOW` is **not** a record of active states. It is an immunity mask built
from `EMSPECA_NONBLOW` specs (`GLogixExPC.cpp:2357`). The name suggests
otherwise, which is worth stating because reading it as "already holds this"
would produce the wrong rule: a target already stunned *can* be stunned again,
and the new application overwrites.

Because that mask requires passive/buff/item spec aggregation, which is a
separate system, it is supplied as an already-computed value on the input rather
than being fetched inside the resolver.

## 11. `fVAR1` / `fVAR2` — carried, not interpreted

Legacy carries `fVAR1`/`fVAR2` per blow and consumes them in scattered places
(`GLChar.cpp:2560-2600` for weapon blows, effect-specific logic elsewhere). The
per-state meaning is data, not a rule this milestone can verify end to end, so
the values are **stored and returned exactly** and nothing interprets them.
Inventing a meaning per ailment would be exactly the guessing the workflow
forbids.

## 12. FACT / buff classification

| Legacy system | Size | Verdict |
| --- | --- | --- |
| State blow (`SSTATEBLOW`, `EMBLOW_MULTI`) | 4 slots | **Implemented** |
| Skill FACT (`SSKILLFACT`, `SKILLFACT_SIZE`) | 14 (`GLCharData.h:201`) | Deferred — separate vertical |
| Special skill effect (`SINCREASEEFF`, `SSPEC`) | per-skill | Deferred; only `EMSPECA_NONBLOW` consumed, as an input |
| Land effect (`SLANDEFFECT`) | — | Deferred, needs a world |
| Item / question effect | — | Deferred, needs items |

No FACT effect is a dependency of status calculation, so nothing from that
system was implemented.

## 13. Modern mapping

```
modern/core/status/
  StatusEffectTypes.h      modern EMSTATE_BLOW / EMDISORDER / STATE_TO_ELEMENT
  StatusEffectResolver.h   pure probability + duration, roll injected
  StatusEffectContainer.h  the 4 slots: apply, tick, cure, query
```

Namespace is `Modern::StatusEffect`, **not** `Modern::Status` — `Modern::Status`
is already the project's error-wrapper type (`types/Result.h`), and shadowing it
does not compile.

`ActiveSkillResolver` gained a verdict, not state:
`ActiveSkillResult::hasStatusApplication` plus
`statusApplication`. `ServerCharacter` owns the container and stores the verdict
on the **target**, mirroring legacy's two-step of deciding `bBLOW` in
`SkillProc` and storing via the target's `STATEBLOW`.

`SkillDefinition` gained `stateBlow`; `SkillLevelData` gained `blowRate`,
`blowVar1`, `blowVar2` and `life`.

## 14. Deferred

Damage-over-time (BURN/POISON), movement and attack-speed effects, the FACT/buff
system, zone and area targeting, projectiles, a world/entity registry, weather,
healing, `EMFOR_MP`/`EMFOR_SP`, networking and client status presentation.

Stun's "forced idle action" (`GLChar.cpp:6210-6225`) is a presentation/animation
behaviour and is deferred with the rest.

## 15. Verification

| Suite | Result |
| --- | --- |
| ModernCoreTests | 383 (was 337; +46) |
| ModernServerTests | 76 (was 70; +6) |
| CTest | 14/14 Debug and Release |
| Client suites | 12/12 |
| Builds | Debug and Release, 0 errors, 0 warnings |

No existing test was modified, weakened or deleted. VERTICAL-009 through
VERTICAL-013 combat expectations are untouched — the status domain is additive
and shares no code path with them.

---

## References

- `legacy/Lib_Client/G-Logic/GLCharDefine.h:894-957`
- `legacy/Lib_Client/G-Logic/GLSkillApply.h:67-79`
- `legacy/Lib_Client/G-Logic/GLCharData.h:201, 1059-1074`
- `legacy/Lib_Client/G-Logic/GLCharClient.h:106, 242`
- `legacy/Lib_Client/G-Logic/GLChar.cpp:2554-2598, 3355-3397, 6200-6243`
- `legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2357`
- `legacy/Lib_Client/G-Logic/GLFactEffect.cpp:151-160`
- `legacy/Lib_Client/G-Logic/GLogicData.cpp:261, 309`
- `legacy/Lib_Engine/Common/GameCharacterCalculations.cpp:244-279`
- `modern/core/status/StatusEffectTypes.h`
- `modern/core/status/StatusEffectResolver.h`
- `modern/core/status/StatusEffectContainer.h`