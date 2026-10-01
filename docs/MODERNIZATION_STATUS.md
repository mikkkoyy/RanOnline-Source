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
| Vertical gameplay slices 001-009 | complete |
| Build verification (BUILD-001) | complete |
| Ranged / magic combat | not started |
| Required-SP item integration | not started (VERTICAL-010) |

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
| BUILD-001 Tracked build-artifact cleanup | [x] | this commit |
| VERTICAL-010 Required-SP / item integration | [ ] | — |

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

**VERTICAL-010 — Required-SP / item integration.** Not started.

Dependency chain to implement:

```
legacy wReqSP  (SITEM::sSuitOp::wReqSP)
    ↓
ItemStatBlock / ItemDefinition
    ↓
required SP aggregation
    ↓
m_wSUM_DisSP equivalent
    ↓
actual low-SP determination
    ↓
combat
```

This closes the last gap in the low-SP mechanic. VERTICAL-009 established the
correct comparison (`currentSP < requiredSP`, `GLogixExPC.cpp:3497`) and the
correct damage multiplier (`fLOWSP_DAMAGE`), but only `wBASIC_DIS_SP` is
currently modelled — `m_wSUM_DisSP` has no field to live in. See
`docs/reference/client/VERTICAL-009_PHYSICAL_COMBAT_INVESTIGATION.md` §1 and
`docs/MODERN_ARCHITECTURE.md` (VERTICAL-009 limitations).

VERTICAL-010 must begin with a fresh repository check and fresh legacy
investigation.
