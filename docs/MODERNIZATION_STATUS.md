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
| Vertical gameplay slices 001-010 | complete |
| Build verification (BUILD-001) | complete |
| Ranged / magic combat | not started |
| Active skill combat | not started (VERTICAL-011) |

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
| VERTICAL-010 Required-SP / item integration | [x] | this commit |
| VERTICAL-011 Active skill combat | [ ] | — |

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

**VERTICAL-011 — Active skill combat.** Not started.

VERTICAL-010 closed the equipment half of the required-SP calculation and left
the item model ready for the skill half: a future skill milestone computes
`skill required SP = ItemContribution::requiredSP + SkillDefinition::useSP`
(legacy `GLogixExPC.cpp:4254-4258`) with no further change to the item model.

Still deferred, with reasons in
`docs/reference/client/VERTICAL-010_REQUIRED_SP_INVESTIGATION.md` §10:
`m_wACCEPTP` (needs per-item `sReqStats`/`wReqLevelDW`; excluded from the legacy
low-SP gate as well, so it belongs with SP deduction), the `EMR_OPT_DIS_SP`
refine option, SP consumption, `wStrikeNum`, and skill `wUSE_SP`.

VERTICAL-011 must begin with a fresh repository check and fresh legacy
investigation.

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

### Noted, not fixed

`ItemStatBlock::IsZero()` also omits the three `*RecoveryFlat` fields added in
VERTICAL-005, so an item whose only stats are flat recovery is skipped by the
aggregator. Same class of bug as the `requiredSP` omission this milestone fixed,
but not a required-SP dependency, so it is reported rather than changed.

