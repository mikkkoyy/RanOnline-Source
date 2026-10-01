# RAN Online Modernization — Project Status

Locked project roadmap for the `modern/` tree. This file records milestone
completion and the verified state of the build at each point. It is the
authoritative answer to "what is done, and was it actually built".

The legacy tree in `legacy/` is reference material and is never modernised in
place. Milestone numbering continues from
`docs/MODERN_ARCHITECTURE.md`, which holds the architecture and the per-milestone
formula provenance.

---

## Status summary

| Area | State |
| ---- | ----- |
| Legacy import | complete |
| Core foundation | complete |
| Vertical gameplay slices 001-015 | complete |
| Build verification (BUILD-001) | complete |
| Impact/addon modifiers, world targeting | not started |

---

## Milestones

| Milestone | State | Commit |
| --------- | ----- | ------ |
| CORE-001 modern core foundation | [x] | `98900f7` |
| CORE-002 RAN stat calculation foundation | [x] | `aecec6e` |
| VERTICAL-001 Character + Stats | [x] | `b16c5f0` |
| VERTICAL-002 Equipment | [x] | see `docs/VERTICAL-002_EQUIPMENT_INVESTIGATION.md` |
| VERTICAL-003 Skills + passive contribution | [x] | `1088675`, `3617963` |
| VERTICAL-004 Codex progress + contribution | [x] | `1bb5c61` |
| VERTICAL-005 Resources HP / MP / SP | [x] | `d135889` |
| VERTICAL-006 Basic physical combat | [x] | `f4b114b` |
| VERTICAL-007 Combat equipment + state | [x] | `1dd6716` |
| VERTICAL-008 Combat events + reflection | [x] | `7ffa8d6` |
| VERTICAL-009 Build verification + physical combat completion | [x] | `ad99138` |
| BUILD-001 Tracked build-artifact cleanup | [x] | `1dd36f9` |
| VERTICAL-010 Required-SP / item integration | [x] | `021be63` |
| VERTICAL-011 Active skill combat | [x] | `df9bf4f` |
| VERTICAL-012 Ranged physical combat | [x] | `7b86e27` |
| VERTICAL-013 Magic / elemental combat | [x] | `27d0ade` |
| VERTICAL-014 Status effect foundation | [x] | `24b5f65` |
| VERTICAL-015 Skill FACT / buff foundation | [x] | `d05cc2d` |
| VERTICAL-016 FACT consumer investigation | [x] | `ef4daab` |
| VERTICAL-017 FACT combat / cast integration | [x] | this commit |

---

## BUILD-001 — Tracked build-artifact cleanup

**State: complete. Verified, not assumed.**

### What was wrong

170 generated files were tracked under `build-debug/` (85) and `build-release/`
(85), committed before `.gitignore` gained a `build-<config>` rule. Git does not
apply ignore rules retroactively, so they stayed tracked and every build dirtied
the working tree.

They were also actively harmful, not merely untidy:

- they were an **x64** configure, so `Release|Win32` failed with `MSB8013`
  ("does not contain the Configuration and Platform combination");
- `ModernCoreTests.vcxproj` referenced **fewer test sources** than the tree
  contained, so restoring them silently dropped test cases from the binary
  (236 -> 167 observed during VERTICAL-009).

The rule intended to prevent this was itself broken. `.gitignore` read:

```
build-*/  <-- added for VERTICAL-005
```

Git has no inline comments, so the whole line was parsed as a single pattern
(`build-*/  <-- added for VERTICAL-005`) which matched nothing. `git check-ignore`
exited 1. The rule now reads as a bare `build-*/` with the provenance moved to
its own comment line.

### What was changed

- `.gitignore`: repaired the `build-*/` pattern. No other rule touched.
- `build-debug/`, `build-release/`: removed from tracking
  (`git rm -r --cached`). Working copies deleted and regenerated from the
  current CMake files.
- No source file was added, removed or modified. `modern/` (147 tracked files)
  and `legacy/` (7534 tracked files) are untouched.

### Verification

All figures below come from a **clean configure and build performed for this
milestone**, not from any earlier report.

Configuration: Visual Studio 2022 generator, MSVC 14.44.35207, `Win32`,
CMake from `VS2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake`.

| Check | Result |
| ----- | ------ |
| Tracked build artifacts before | 170 (85 + 85) |
| Tracked build artifacts after | 0 |
| Debug build (Core + Server + Client + tests) | PASS — 0 errors, 0 warnings |
| Release build (Core + Server + Client + tests) | PASS — 0 errors, 0 warnings |
| `MSB8013` configuration mismatch | absent |
| CTest Debug | PASS — 14/14 |
| CTest Release | PASS — 14/14 |
| Core tests | 236/236, Debug and Release |
| Server tests | 52/52, Debug and Release |
| Client tests | 12/12 suites, Debug and Release |
| `build-debug/` tracked | no |
| `build-release/` tracked | no |
| Build output ignored | yes, via `.gitignore:42` |

Test sources were cross-checked rather than assumed: the 6 `.cpp` files present
in `modern/tests/` are all referenced by the freshly generated
`ModernCoreTests.vcxproj`, and the resulting executable reports 236 cases. This
directly addresses the stale-project risk recorded in VERTICAL-009.

### Local build procedure

```powershell
cmake -S . -B build-debug   -G "Visual Studio 17 2022" -A Win32
cmake -S . -B build-release -G "Visual Studio 17 2022" -A Win32
cmake --build build-debug   --config Debug
cmake --build build-release --config Release
ctest --test-dir build-debug   -C Debug   --output-on-failure
ctest --test-dir build-release -C Release --output-on-failure
```

`-A Win32` is required: the top-level `CMakeLists.txt` sets
`set(CMAKE_SIZEOF_VOID_P 4)`. `ctest` requires `-C <config>` on this
multi-config generator; without it every test reports `***Not Run` with
`Test not available without configuration`, which is not a test failure.

### Notes

- The build directories remain on disk after a build and are ignored. They are
  not committed.
- `build-*/Testing/Temporary/CTestCostData.txt` and `LastTest.log` are
  CTest-generated run artifacts and are ignored. They are never authored.
- BUILD-001's own commit is the one that introduced this file, so it cannot
  contain its own hash. The commit it was verified against is `ad99138`
  (VERTICAL-009); the BUILD-001 SHA is recorded in the change report.

---

## Next task

**VERTICAL-013 — Magic / elemental combat.** Complete. See
`docs/reference/client/VERTICAL-013_MAGIC_ELEMENTAL_INVESTIGATION.md`.

`SkillApply::Magic` is no longer refused. Magic is a distinct channel, not the
physical formula with a flag: it adds **no** weapon item damage, uses
`m_wSUM_MA`, forces `nDEFENSE`/`nDEFAULT_DEFENSE`/`nITEM_DEFENSE` to zero
(`GLogixExPC.cpp:1474-1476`), applies resistance to the damage **range** before
the roll rather than to the rolled value, and reads the magic halves of
`DAMAGE_SPEC`. So `AttackType::Magic` and `CalculateMagicDamage` were added
rather than a flag.

`DerivedStats::magicAttack` already matched legacy exactly — magic attack has no
class/level term, unlike PA/SA — and was reused. `ApplyAttackPower` moved to
`CombatTypes.h`, because legacy uses the identical `VAR_PARAM` for PA, SA and MA.

Two further defects were closed on the way: `targetResistElement` had been
hardcoded to `0` for every active skill since VERTICAL-011, and magic attack
power would otherwise have reproduced VERTICAL-012's "carried across the
boundary but never used" defect.

Executed scope is `EMAPPLY_MAGIC + EMFOR_HP + fBASIC_VAR < 0 + TAR_SPEC +
SIDE_ENEMY`. Heals, `EMFOR_MP`/`EMFOR_SP`, zone/realm targeting, projectiles, a
weather provider and the item grade term are all explicitly deferred, with
reasons.

Two legacy bugs are recorded rather than "fixed", because correcting them would
move behaviour away from RAN: the damage-reduction flag at
`GLogixExPC.cpp:1740-1741` adds `DAMAGE_TYPE_PSY_REDUCE` in **both** `bPsyDamage`
branches, and the MP/SP resistance arithmetic at `GLChar.cpp:3099` appears
inverted. See sections 9 and 10 of the investigation.

**VERTICAL-014 — Status effect foundation.** Complete. See
`docs/reference/client/VERTICAL-014_STATUS_EFFECT_FOUNDATION.md`.

`SkillDefinition::stateBlow` now reaches a real path. Status effects live in
their own domain, `modern/core/status/`, and `ActiveSkillResolver` holds no
status state: it reports the application verdict and `ServerCharacter` stores it
on the target, mirroring legacy's two-step of deciding `bBLOW` in `SkillProc`
and storing through the target's `STATEBLOW`.

The central finding is that status effects are **a four-slot pool with shared
occupancy**, not one slot per ailment. `EMBLOW_SINGLE` (5) is an alias of
`EMBLOW_FROZEN`, and `GLChar.cpp:6204-6205` routes everything at or below it to
slot 0 — so Numb, Stun, Stone, Burn and Frozen overwrite one another. Applying
Burn to a stunned target **ends the stun early**. There is no refresh, no merge
and no "keep the stronger": re-applying resets the duration.

Also corrected a natural misreading: `GETHOLDBLOW()` is an **immunity mask**
built from `EMSPECA_NONBLOW` specs, not a record of active states, so a target
already stunned can be stunned again.

One verified legacy bug is **reproduced rather than fixed**:
`GLChar.cpp:3369` clamps status resistance against `fRESIST_G` (`0.5f`) instead
of `fMAX_RESIST` (`99.0f`), so assigning it to a `short` truncates to 0 and
resistance never affects the threshold or duration. Correcting it would diverge
from the client players play. The ceiling is a named parameter
(`StatusConstants::resistClampCeiling`) so adopting the intended clamp is a
one-line change, and both behaviours are tested. **This is the decision most
worth a second opinion.**

Damage-over-time, movement/attack-speed effects, the `SSKILLFACT` buff system
(`SKILLFACT_SIZE = 14`), zone/area targeting, projectiles, weather, healing,
`EMFOR_MP`/`EMFOR_SP` and networking are all deferred, with reasons in §12 and §14
of the investigation.

**VERTICAL-015 — Skill FACT / buff foundation.** Complete. See
`docs/reference/client/VERTICAL-015_SKILL_FACT_FOUNDATION.md`.

FACT is RAN's other persistent-effect mechanism and is deliberately kept
separate from VERTICAL-014's status effects: different storage, lifetime, slot
rules and consumers. It lives in `modern/core/skills/SkillFact*.h` and owns a
**14-slot** pool.

The central finding is that slot selection (`GLChar::SELECT_SKILLSLOT`,
`GLChar.cpp:6377-6408`) is a three-rule cascade, and its third rule is the one
that is easy to miss: with a **full** pool, RAN evicts the buff with the
**smallest remaining lifetime**. It does not refuse the new buff and does not
prefer the strongest. Rule 2 (first empty slot) short-circuits inside the same
loop, so a pool with any free slot never evicts at all.

Three aggregation rules are genuinely counterintuitive and each has a test:

- `EMSPECA_ATTACKVELO` **subtracts** — a positive value makes the attacker
  *slower*, and RAN enters `-0.1` to get 10% faster.
- Damage reduction takes a **maximum**, not a sum. Two 0.2 buffs give 0.2.
- `EMSPECA_NONBLOW` **assigns** the immunity mask rather than OR-ing it, so two
  immunity buffs do not combine; the last one aggregated wins.

A legacy off-by-one is reproduced: ticking and aggregation are one pass, and
`DISABLESKEFF` only nulls the skill id, so **a fact still contributes on the tick
it expires**. Expiry needs no restore step at all — legacy rebuilds every
accumulator from zero each call.

This milestone also fixed a latent trap in a shared type: `SkillId{}` is a
*valid* id (only `0xFFFF` is not), so a default-constructed FACT looked occupied
and an empty pool reported fourteen active facts. `SkillId::Invalid()` now
expresses legacy's `SNATIVEID(false)` explicitly.

Impacts are stored faithfully but **not aggregated**: every `EMIMPACTA_*` feeds a
different subsystem (derived stats, hit calculation, resource pools), so wiring
them here would duplicate an existing authoritative calculation. Deferred with
reasons in §9 and §12, along with `TAR_BUFF`, world targeting, networking and
`prohibitSkill` plumbing into `CastSkill`.

**VERTICAL-016 — FACT consumer / impact integration investigation.** Complete,
documentation only. See
`docs/reference/server/VERTICAL-016_FACT_CONSUMER_INVESTIGATION.md`.

No production code changed. The milestone exists to establish where RAN actually
*consumes* FACT data after creation, so the next vertical can attach to systems
that already exist rather than inventing new ones.

Every `EMIMPACTA_*` consumer was traced with its exact operation. Three findings
matter most:

- **PA/SA/MA must not be folded into `PassiveContribution`.** Legacy keeps
  `nSUM_MA` separate from `m_sSUM_PASSIVE.m_nMA` and sums them only at the point
  of use (`:2970-2972`), and modern already mirrors that with three distinct
  contribution structs. A timed buff needs a **fourth** input; putting it in the
  passive bucket would make it survive expiry.
- **Damage reduction and reflection already have owners.** VERTICAL-013 added the
  four psy/magic pairs to `CombatInput`, and the FACT aggregator already produces
  the values. No new combat formula is needed.
- **A pre-existing gap surfaced:** `PhysicalDamageInput` has no `damageRate`
  field, although legacy applies `ApplyDamageRate` to the range for every channel
  (`:1600-1603`). Only the magic calculator has it. Recorded, not fixed, since
  changing it would alter V009 physical expectations.

Two hand-offs that VERTICAL-015 proved in unit tests were found **not to be wired
into the real cast path**: `CastSkill` populates neither `skillProhibited` nor
`targetDisorderMask`, so today a `PROHIBIT_SKILL` or `NONBLOW` FACT has no effect
on an actual cast. Both are pure wiring gaps and are the first two items of the
recommended next vertical.

One open question is recorded rather than guessed: the reduction and reflection
specs max-accumulate into the member struct `m_sDamageSpec`, and no reset of
those fields is visible in the per-tick reset block. The next vertical must prove
that reset before wiring those four fields.

`MOVEVELO` is aggregated on the server but consumed **only by the client**;
`ATTACKVELO` has a single server use (an attack-interval timer) whose formula
could not be read. `PROHIBIT_POTION` is read only by inventory and storage
paths, and `TAR_BUFF` needs an entity registry. All DEFERRED.

**VERTICAL-017 — FACT combat / cast integration.** Complete. See
`docs/reference/server/VERTICAL-017_FACT_COMBAT_CAST_INTEGRATION.md`.

Five VERTICAL-016-proven consumers are now connected to systems that already
existed. No new calculation system, no new FACT domain, and no deferred item.

**`m_sDamageSpec` — RESET PROVEN.** The blocker VERTICAL-016 flagged is resolved:
`GLogixExPC.cpp:2224` resets it and `:2228` seeds it from the passive total, in
the same per-tick block (`:2210-2241`) that resets every other accumulator. The
unified pattern is *reset to default, seed from passives, then accumulate FACTs*,
so the structure is rebuilt rather than patched. VERTICAL-016 had simply started
reading at `:2255`, above the reset. No accumulation leak, and no save/restore
mechanism was needed.

Implemented:

- **Damage reduction and reflection** into the four `CombatInput` pairs VERTICAL-013
  already owned. MAX, not sum, applied at the two existing boundaries
  (`ServerCharacter::Attack` and `ActiveSkillResolver`). The calculators were not
  touched.
- **`Stats::FactContribution`**, a fourth stat source beside items, passives and
  codex. Deliberately *not* folded into `PassiveContribution`: legacy keeps
  `nSUM_MA` separate and adds it only at the point of use (`:2970-2972`), and
  merging them would let a timed buff survive expiry.
- **`PROHIBIT_SKILL`** into `CastSkill`. The resolver already owned the refusal;
  only the server-side input was missing, so no second check was added. A test
  proves no SP/MP/HP is spent and no cooldown starts.
- **`NONBLOW`** immunity mask into `CastSkill`, crossing as a value. FACT and
  `StatusEffect` remain separate domains.

Two of the three previously-invisible hand-offs now work in a real cast; the third
(PA/SA/MA) needed the derived-stat snapshot to be refreshed when a power impact
moves, which legacy gets for free by reading its accumulators live.

**Next:** the remaining deferred FACT consumers, each needing its own subsystem
(resource recovery, hit/avoid, damage range, world targeting). The number is
deliberately left open rather than invented.

---

## VERTICAL-011 — Active skill combat

**State: complete. Verified, not assumed.**

Full derivation:
`docs/reference/client/VERTICAL-011_ACTIVE_SKILL_INVESTIGATION.md`.

### Active and passive are separate systems

VERTICAL-003's `PassiveContributionAggregator` reads a definition when a skill
is *learned*. Nothing there is cast, costs anything, hits anything, or has a
cooldown, and pushing active execution through it would conflate the two. RAN
keeps them apart as well - `SUM_PASSIVE` (`GLogixExPC.cpp:918-1002`) versus
`CHECHSKILL` -> `ACCOUNTSKILL` -> `SkillProc`, with no shared function.

So VERTICAL-011 adds a third thing:

| Piece | Where |
| --- | --- |
| Active-skill data on the definition | `ItemStatBlock`-parallel: `SkillDefinition` / `SkillLevelData` |
| `ActiveSkillResolver` - the rules | `modern/core/skills/ActiveSkill.h/.cpp` |
| `ServerCharacter::CastSkill` - authority | `modern/server/character/ServerCharacter.cpp` |

The resolver is pure: it takes a definition, a resolved level and a description
of the situation, and returns a verdict plus the costs. It fetches nothing, so
a test reaches every branch - including refusals for systems the server does not
have - without building a world, and it has no clock.

### The server owns the level

`CastSkill(id, target, requestedLevel)` accepts a level and ignores it. The
level comes from the character's own `SkillState`, as in RAN
(`GLogixExPC.cpp:4075`). A caller that could set the cast level could cast a
skill above what it has learned.

### Low SP degrades, it does not refuse

RAN's server-side first check rejects `EMSKILL_NOTSP`
(`GLCharSkillMsg.cpp:358`, the tolerance commented out) while the running-cast
re-check tolerates it (`GLChar.cpp:4798`). The tolerated path is the one that
produces the cast, so that is the one followed: a short SP pool halves the
damage and charges nothing (`GLChar.cpp:3005`), exactly as a basic attack does.

The pool is the caster's, per VERTICAL-010's locked correction.

### Verification

| Check | Result |
| ----- | ------ |
| Debug build | PASS - 0 errors, 0 warnings |
| Release build | PASS - 0 errors, 0 warnings |
| CTest Debug | PASS - 14/14 |
| CTest Release | PASS - 14/14 |
| Core tests | 293/293 (was 259), Debug and Release |
| Server tests | 70/70 (was 58), Debug and Release |
| Client tests | 12/12 suites, Debug and Release |

### Departures from legacy, all deliberate

Five are listed in the investigation. The two that matter most: the SP charge
omits `m_wACCEPTP` (so gate and charge agree, where legacy's do not), and the
skill damage omits the item grade term (modern items have no grade field).

### Noted, not fixed

`Resources::ResourceState` was used by nothing before this milestone -
`ServerCharacter` holds raw `m_currentHp/Mp/Mp` and `Attack` subtracts from them
directly. `CastSkill` routes its own costs through `ResourceState` for the
saturating `GLDWDATA::DECREASE` semantics, but migrating the whole character
onto it is a separate change that would touch VERTICAL-005 through 009 and
their tests.


---

## VERTICAL-010 — Required-SP / item integration

**State: complete. Verified, not assumed.**

Full derivation: `docs/reference/client/VERTICAL-010_REQUIRED_SP_INVESTIGATION.md`.

### What changed

The required-SP value now has somewhere to live and reaches combat.

| Piece | Where |
| --- | --- |
| `ItemStatBlock::requiredSP` (`uint16_t`) | `modern/core/item/ItemDefinition.h` |
| `ItemStatBlock::IsZero()` learns the field | `modern/core/item/ItemDefinition.cpp` |
| `ItemContribution::requiredSP` | `modern/core/stats/Contributions.h` |
| Hand-only accumulation | `modern/core/equipment/ItemContributionAggregator.cpp` |
| `CombatInput::attackerCurrentSP`, core-side low-SP rule | `modern/core/combat/CombatCalculator.h` |
| Server supplies the real value | `modern/server/character/ServerCharacter.cpp` |

Legacy `SUM_ITEM` reads `wReqSP` from `emRHand` and `emLHand` only
(`GLogixExPC.cpp:430-434`), so the aggregator applies the term under a slot
test rather than summing all 21 slots. The loop became indexed for that reason.

`CombatInput::attackerRequiredSP` is VERTICAL-009's field and keeps its name; it
now receives `m_items.requiredSP + basicDisSP` instead of the bare constant.

### One correction to VERTICAL-009

VERTICAL-009 fed its low-SP flag from the **target's** SP. Legacy decides this
on the **attacker**: `GLCharMsg.cpp:604-612` calls `BEGIN_ATTACK`, which reads
that character's own `m_sSP.dwNow`, and passes the result to that character's
`PreStrikeProc`. The victim is never consulted.

With a real required-SP value the old wiring could not behave correctly — an
attacker needing 31 SP would never be low-SP against a target holding 100 SP.
`targetLowSP` is gone; `ResolveCombat` evaluates
`attackerCurrentSP < attackerRequiredSP` in one place, and `CombatInput` no
longer carries a target SP field at all.

The VERTICAL-009 formulas were not touched: `fLOWSP_HIT_DROP = 0.25` and
`fLOWSP_DAMAGE = 0.50` are pinned by
`RequiredSPMatrix_LowSPFormulasUnchanged`.

### Verification

| Check | Result |
| ----- | ------ |
| Debug build | PASS — 0 errors, 0 warnings |
| Release build | PASS — 0 errors, 0 warnings |
| CTest Debug | PASS — 14/14 |
| CTest Release | PASS — 14/14 |
| Core tests | 259/259 (was 236), Debug and Release |
| Server tests | 58/58 (was 52), Debug and Release |
| Client tests | 12/12 suites, Debug and Release |

`m_wACCEPTP` is deliberately **not** implemented. It is a stat-deficit penalty
needing per-item `sReqStats` and `wReqLevelDW`, which the modern item model has
no field for. It is also absent from the legacy low-SP gate
(`GLogixExPC.cpp:3492-3494` rebuilds `wDisSP` from `wBASIC_DIS_SP` and the two
hands without reading it), so excluding it is faithful rather than a shortcut.
It belongs to whichever milestone implements SP deduction.

VERTICAL-011 implemented the skill SP charge and is that milestone for the
skill path. The term is still absent from it, for the same reason; the charge
and the gate are equal there, where legacy's are not. See
`docs/reference/client/VERTICAL-011_ACTIVE_SKILL_INVESTIGATION.md` §13.

### Noted, not fixed

`ItemStatBlock::IsZero()` also omits the three `*RecoveryFlat` fields added in
VERTICAL-005, so an item whose only stats are flat recovery is skipped by the
aggregator. Same class of bug as the `requiredSP` omission this milestone fixed,
but not a required-SP dependency, so it is reported rather than changed.

