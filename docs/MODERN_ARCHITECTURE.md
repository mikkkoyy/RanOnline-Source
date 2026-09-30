# Modern Architecture

This document describes how the modern RAN implementation is put together, what
it is allowed to depend on, and what the current state of each area is.

For the two governing rules see the repository [README](../README.md):

1. Legacy RAN source is **reference material**.
2. Modern source is the **authoritative implementation**.

---

## 1. Goals

- A RAN server that can be reasoned about, tested and extended without the
  original codebase's coupling, global state and implicit conventions.
- Deterministic, headless game rules that can run in a test process without a
  renderer, a socket layer, or a database.
- Byte-compatibility with existing RAN clients and data where that is required
  for migration, isolated behind adapters rather than leaking into the core.
- A codebase where any behaviour can be traced back to the legacy
  implementation that defined it.

## 2. Non-goals

- Rewriting the legacy code in place.
- Feature parity with the legacy server on day one.
- Porting the DirectX client renderer, the TIK SDK, or the commercial
  authentication stack into the modern tree.
- Shipping anything from `reference/` in a build.

## 3. Layout

```
modern/
├── core/                    game rules and domain models — legacy-free
│   ├── character/           Character
│   ├── data/                portable text exports of legacy binary tables
│   ├── entity/              Entity
│   ├── item/                ItemDefinition, ItemInstance
│   ├── math/                Vector3
│   └── types/               Ids, Result
├── client/                  new RAN client — consumes core, never legacy
│   ├── application/         Application lifecycle, update loop (CLIENT-002)
│   ├── input/               Input events, system & platform abstraction (CLIENT-003)
│   ├── resources/           Resource boundary, identifiers, providers, cache (CLIENT-005, CLIENT-006)
│   ├── assets/              Typed CPU-side assets, decoders & real DDS decoding (CLIENT-007, CLIENT-008, CLIENT-011, CLIENT-012, CLIENT-013)
│   └── rendering/           Renderer abstraction, null backend & asset upload boundary (CLIENT-004, CLIENT-009)
├── tests/                   headless rule tests (ModernCoreTests)
├── tools/                   offline / research tooling
│   ├── exptable_dump.cpp    reads the packed legacy EXP table -> text
│   └── emulator/            ModernEmulator demonstration harness
└── compatibility/legacy/    the single sanctioned modern -> legacy bridge
    └── CharacterAdapter.*   (sources present, not built — see section 6)
```

`reference/` and `docs/` live outside `modern/` because neither is part of the
shipped implementation.

## 4. Layering

```
        core          <-  network  database  server  client  tools
          |
          X                 no edge from core down into legacy
          |
  compatibility/legacy  -->  legacy/     (adapters only)
```

- **`core`** defines the domain. It depends on the standard library and on
  nothing else. It is the only thing every other modern component may depend on.
- **`network`**, **`database`**, **`server`**, **`client`**, **`tools`** consume
  `core`. `client/application`, `client/input`, `client/rendering`,
  `client/resources` and `client/assets` are the client slices built so far
   (CLIENT-002, CLIENT-003, CLIENT-004, CLIENT-005, CLIENT-006, CLIENT-007,
   CLIENT-008, CLIENT-009, CLIENT-011, CLIENT-012); the remaining client systems (ui, character, world,
   audio) and the network/database/server layers are created when there is code
   to put in them.
- **`compatibility/legacy`** converts between modern types and legacy RAN
  types and formats. Only this layer may see `legacy/`, and nothing in `core`
  may reference it.

A layer is added when there is code for it, not before. A directory that exists
only to hold a placeholder is a directory whose dependency rules nobody has
checked yet.

## 5. Core rules

`modern/core` must stay free of:

- Windows UI, DirectX / Direct3D, GDI+;
- sockets, WinINet, or any transport;
- PostgreSQL, ODBC, or any database client;
- legacy RAN types and headers: `GLChar`, `GLItemMan`, `SITEM`, `GLCONST_CHAR`,
  `ByteCryptDef*.h`, and anything else under `legacy/`;
- global mutable state that makes rules order-dependent.

Rationale: the core is the part that has to be unit-testable and reusable in
every runtime — emulator, server, tools, and future tests. The moment it can
see a legacy global, it can no longer be reasoned about in isolation.

If a change appears to need one of the above, the change belongs in
`compatibility/legacy` or in a future `network` / `database` layer, not in
`core`.

## 6. Legacy compatibility

Legacy code is not modified. It is used as the specification: when the modern
implementation is unsure what the original did, someone reads
`legacy/Lib_Client/G-Logic` and writes the modern equivalent.

Adapters under `modern/compatibility/legacy` are the only sanctioned way for
modern code to see a legacy type. They have one job: translate. They contain no
game rules. When a modern component needs legacy information, it asks the
adapter; it does not include a legacy header itself.

Planned adapters, as the format surface is taken on one at a time:

| Adapter                 | Responsibility                                     |
| ----------------------- | -------------------------------------------------- |
| `LegacyCharacterAdapter`| `GLChar` / `CharacterBase` -> `modern::Character`   |
| `LegacyItemAdapter`     | `SITEM` / `GLItemMan` -> `modern::ItemDefinition`   |
| `LegacyPacketAdapter`   | legacy packet headers -> modern message structs     |
| `LegacyDataImporter`    | legacy binary tables -> `modern/core/data` exports  |

### Status: not built during CORE-001

`CharacterAdapter` exists on disk but is not in the build.

Two reasons, both deliberate:

1. CORE-001 is scoped to `modern/core`. Legacy compatibility is a later phase.
2. The existing adapter maps `GLChar` fields that the clean `Character`
   deliberately does not have — HP/MP/SP pools, the `ActState` bitfield,
   die/revive. It has to be rewritten against the new type, not patched, and a
   half-migrated adapter that compiles against a character model already known
   to be wrong is worse than one that is visibly absent.

For the same reason the root `CMakeLists.txt` no longer adds the legacy
libraries: with the bridge out of the build, nothing in the CMake build needs
`legacy/` at all, and the only remaining modern -> legacy reference is an
include in `modern/tools/exptable_dump.cpp`, which reaches its headers by
relative path and links nothing.

The legacy tree is unaffected. It builds on its own through
`legacy/RanOnline.sln` (section 8).

## 7. Data model

- Modern domain types are plain data owned by `core`, with no legacy members.
- Legacy binary tables are not read at runtime by the core. They are converted
  once, offline, into the portable text exports under `modern/core/data`.
- Exports are committed so that a build is reproducible without the legacy
  binaries.
- The importers that produce those exports live in `reference/` or
  `modern/tools/` and are never linked into the server.

`exptable_dump` is the first such converter: packed/encrypted
`exptable_max.bin` -> `exptable_max.txt`, one `int64` per line, index = level.

## 8. Legacy build

The legacy tree remains buildable so behaviour can be observed.

- Solution: `legacy/RanOnline.sln`
- Toolchain: VS2022, toolset v170, `Win32`, `Release`
- Output: `RanOnline-Build/` at the repository root (the projects use
  `$(SolutionDir)..\RanOnline-Build`, preserved by the move)
- Helpers: `legacy/scripts/build_*.bat`
- The legacy `.vcxproj` files needed no edits: every path they reference is
  either project-relative or a sibling directory that moved with them.

The research `*.ps1` helpers under `legacy/scripts/` still contain absolute
paths from before the move. They are kept as a record of the original audits
and are not expected to run unmodified.

## 9. Testing

- Rule tests are headless: they link `Modern` only, and must not need a
  renderer, a socket, a database, or a legacy library.
- Any test that needs to read a legacy format lives with its adapter in
  `compatibility/legacy`, and is explicitly a compatibility test rather than a
  rules test.
- Data-conversion checks live with the converter tool that owns the format.
- `modern/tests/` holds the first suite: `ModernCoreTests` covers CORE-001.
  It is registered with CTest, so `ctest` in the build directory runs it.

## 10. Tooling

- CMake drives the modern tree; the root `CMakeLists.txt` adds `modern/` only.
- Target names (`Modern`, `ModernEmulator`, `ModernCoreTests`) were kept from
  the pre-move layout to avoid churn.
- `vcpkg.json` pins the modern dependency set (spdlog, fmt, zlib, tbb).
- The legacy tree builds with MSBuild and the Visual Studio solution, not
  CMake.

## 11. Code standards

- Modern code is C++17. MSVC compiles with `/W4 /permissive- /utf-8`.
- Headers own their types; no global headers such as `Lib_Engine/Common`.
- `modern/` uses `modern/`, `modern/core/`, ... for internal includes, relative
  to the modern source root.
- Prefer value semantics in `core`; pointers and handles belong at the edges.
- Format conversions are explicit and named. No implicit narrowing between
  legacy and modern numeric types.

## 12. Definition of done

Work on the modern tree is complete when:

- the feature is implemented in `core` or a modern layer, with no legacy
  dependency outside `compatibility/legacy`;
- it is covered by headless tests that pass without legacy libraries;
- any legacy data it needs has a committed portable export;
- it is verified by running the headless test binary. No client, server or
  other production executable is launched;
- the dependency rules above still hold.

## 13. CORE-001: base core foundation

The first implemented slice of `core`. It is deliberately small, and the
smallness is the point.

### What it contains

| Unit                     | Responsibility                                                     |
| ------------------------ | ------------------------------------------------------------------ |
| `types/Ids.h`            | `EntityId`, `CharacterId`, `ItemId`, `AccountId`, `WorldId`         |
| `types/Result.h`         | `ErrorCode`, `Status`, `Result<T>`                                  |
| `math/Vector3.h`         | Position and direction arithmetic                                   |
| `entity/Entity.h`        | Identity, position, direction, lifecycle state                       |
| `character/Character.h`  | Identity, class, level, experience, lifecycle                       |
| `item/ItemDefinition.h`  | Item type identity                                                  |
| `item/ItemInstance.h`    | Item copy identity                                                  |

### Conventions

**Identifiers are strongly typed.** Each is a distinct type, so an `ItemId`
cannot be passed where a `CharacterId` is expected. Construction is explicit, a
default-constructed id is invalid, invalidity is a single well-known sentinel,
and ordering is defined so ids work as ordered container keys. The invalid
sentinel is the all-ones pattern, matching the legacy `INVALID_*` constants, so
a default-constructed id compares equal to an explicitly invalidated one.

**Failure is a value, not an exception.** The core runs in a fixed-order
simulation loop where an exception thrown from inside a calculation is much
harder to reason about than a returned code. `ErrorCode` covers `None`,
`InvalidArgument`, `NotFound`, `AlreadyExists`, `InvalidState` and
`NotAllowed`. This is an enum and a small wrapper, not a framework.

`InvalidState` and `NotAllowed` are distinct on purpose. `InvalidState` means
the object is in the wrong state for this operation — spawning something already
spawned. `NotAllowed` means the state is coherent but the operation is barred
outright, such as any transition out of a terminal state.

**Entities own their lifecycle.** `Entity<IdType>` holds identity, position,
direction and one `EntityState`. It is templated on the identity type so each
entity carries exactly one id of the right type, rather than a generic
`EntityId` that has to be narrowed at every use.

The state machine is:

```
Uninitialized -> Created -> Spawned <-> Despawned -> Destroyed
      ^                                                    |
      +---------------------- Reset() ---------------------+
```

`Destroyed` is terminal: it releases identity, so the object is safe to return
to a pool. `Reset()` is the only unconditional transition, and after it the
object behaves exactly like a default-constructed one.

`IsActive()` and `IsAlive()` are the same question in two domains — the
simulation asks whether it is active, gameplay asks whether it is alive — and
both mean "currently spawned".

**A character owns facts, not derived state.** Level and experience are facts
about the character. How much experience a level costs is data and belongs to a
future progression system. The following were deliberately left out, and each
one was present in the earlier port:

- HP / MP / SP and recovery rates. In RAN these are recomputed from base stats,
  equipment, passive skills and codex effects. Owning them on the character is
  what forced the core to know about all five.
- Attack, defence, hit, avoid, resistances: calculated combat state.
- Equipment and contribution aggregates: per-system inputs, not character
  state.
- Movement targets, speeds, and the `ActState` bitfield: animation state.

They come back when the systems that own them exist, and not before. A field
that exists only because the old calculation architecture required it is a
field nobody has justified yet.

**CORE-002 gave the first of those systems an owner.** The stat system in
`modern/core/stats` now computes HP / MP / SP, recovery rates, attack and
defence points, the three attack powers, hit, avoid, defence, the physical
damage range and resistances, from a character's own facts plus three
contribution inputs. They are still not *on* the character: the calculation
takes them as an input value and returns them, so `Character` remains a set of
facts and the aggregates stay a per-system input rather than character state.
See CORE-002 at the end of this document.

**Validation happens at the boundary.** NaN and infinity are rejected on the
way in, so one bad value cannot propagate into every downstream rule.
Directions are stored normalised, so no consumer has to ask whether it was
handed a unit vector. Experience saturates rather than wrapping, because a
wrapped total reads as a plausible small number instead of an obviously broken
one. A name at the 32-character limit is accepted and a longer one is rejected
rather than truncated, because silently shortening an identity shows up much
later as two characters sharing a name.

### What was moved out

An earlier work-in-progress port of RAN's derived-stat chain
(`GLOGICEX` → 21 equipment slots, HP/MP/SP pools, `SSUM_ITEM`,
`m_sSUM_PASSIVE`, codex contributions, `ActState`) is preserved in
`reference/legacy-calculation-port/` and is not part of any build. It is a
faithful record of the legacy formulas and the right starting point if those
systems are rebuilt; it is not a reference implementation and is not
maintained. See the README there for the specific defects that stopped it
compiling.

**CORE-002 also partly supersedes it.** The stat system was written from the
legacy source, not from that port, because auditing the port against
`GLogixExPC.cpp` found it had invented behaviour the legacy chain does not
have: a `max(baseHP, stat * factor)` rule that exists nowhere in
`SUM_ADDITION`, `baseHP`/`baseMP`/`baseSP` as class data that
`GLCONST_CHARCLASS` does not carry, and a single final `uint16` cast where RAN
truncates the per-level term per field before adding it. What survived the
audit and was reused is its *vocabulary*: the sixteen `EMCHARINDEX` values and
the six-stat shape, both verified against the legacy headers. The port remains
a research note and is still not in any build.

### Verifying

```
cmake -S . -B build
cmake --build build --config Debug
.\build\Debug\ModernCoreTests.exe
```

`ModernEmulator` prints the same conventions in use. It is a demonstration, not
the authority.


## 13.5. VERTICAL-002: character equipment layer

VERTICAL-002 completes the character + stats vertical slice begun in VERTICAL-001
by adding equipment as the authoritative source of item contributions. The
server now owns the worn set, aggregates item definitions into a stat
contribution, and publishes the result through the shared `CharacterSnapshot`;
the client presents what it receives with no local recomputation.

### What it contains

| Unit                                                  | Responsibility                                                         |
| ----------------------------------------------------- | ---------------------------------------------------------------------- |
| `modern/core/equipment/EquipmentState.h, .cpp`        | 21-slot wearable container (`EMSLOT` legacy values), `Equip`/`Unequip` |
| `modern/core/equipment/ItemDefinitionProvider.h, .cpp`| Read-only interface + `InMemoryItemDefinitions` sorted-vector impl     |
| `modern/core/equipment/ItemContributionAggregator.h, .cpp`| Equipment + definitions -> `Stats::ItemContribution`                |
| `modern/core/item/ItemDefinition.h, .cpp` (extended)  | `ItemStatBlock`, `IsEquipment()`, `IsFinite()`, `IsZero()`            |
| `modern/core/gameplay/CharacterSnapshot.h, .cpp` (ext.)| `EquippedItem`, `EquippedList`, equipment validation in `IsValid()`  |
| `modern/server/character/ServerCharacter.h, .cpp` (ext.)| `Equip`/`Unequip`, aggregation in `Recalculate()`, snapshot publishing |
| `modern/client/gameplay/ClientCharacterState.h, .cpp` (ext.)| Read-only equipment views: `GetEquipment()`, `GetEquippedItem()`    |

### Key conventions

- **Equipment is the sole source of item contributions.** `ServerCharacterDefinition.items` is zeroed in `Create()` and never read again; `SetContributions()` refuses non-zero items. All `ItemContribution` comes from the worn set.
- **The server owns the worn set.** `Equip`/`Unequip` are the only mutators; each stages the change, validates against the item provider, commits atomically, then recalculates before returning. A failed call leaves the character untouched.
- **The client receives, never computes.** `ClientCharacterState` exposes `GetEquipment()`, `GetEquippedItem()`, `HasEquipped()`, `GetOccupiedSlotCount()` — all read-only views of the published snapshot. The client translation unit contains no call to `Modern::Stats::Calculate`.
- **Definitions are the shared truth.** `ItemDefinition.stats` carries the base values every copy of the item contributes. Per-copy random options, refine state, and custom bonuses are deferred; the investigation report (§7) records why they are absent.
- **Aggregation is deterministic.** Slots are visited in order (0..20), so the same equipment always produces the same contribution regardless of build order. The six base stats accumulate as 16-bit unsigned values (wrapping like RAN's `SSUM_ITEM`); the wrap happens in `Calculate`, not in the aggregator.
- **Defense in depth.** `InMemoryItemDefinitions::Add` rejects non-finite stat blocks and invalid definitions. The aggregator additionally checks `IsFinite()` on every definition it reads and returns `ContributionError::NonFinite` if one slips through. Missing definitions yield `ContributionError::MissingDefinition`. An instance bound to a different definition is refused so one copy cannot be counted twice.

### Legacy provenance

| Modern element                    | Legacy origin                                                                 |
| --------------------------------- | ----------------------------------------------------------------------------- |
| `EquipmentSlot` enum (21 values)  | `EMSLOT` in `GLItemDef.h:207-245`, `SLOT_NSIZE_S_2 = 21`                     |
| `ItemContribution` fields         | `SSUM_ITEM` / `SUM_ITEM` in `GLogixExPC.cpp:441`                              |
| `ItemStatBlock` fields            | `SSUIT` in `GLItemSuit.h:87`, `SITEM` / `SITEMCUSTOM` in `GLItem.h`          |
| `EMADD_*` typed add-ons           | `GLItemDef.h:508-535`                                                         |
| Slot iteration order              | `SUM_ITEM` walks `i in [0, SLOT_NSIZE_S_2)` in `GLogixExPC.cpp:446`          |

The full investigation is in `docs/reference/client/VERTICAL-002_EQUIPMENT_INVESTIGATION.md`.

### Verifying

```powershell
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

The headless test suite covers:
- `ModernCoreTests` (96 cases): `EquipmentState`, `InMemoryItemDefinitions`, `ItemContributionAggregator`
- `ModernServerTests` (24 cases): server equipment integration, snapshot publishing, rollback on failure
- `ModernClientGameplayTests` (13 cases): client presentation of equipped items, empty state, clear

All tests pass without legacy libraries, DirectX, sockets, or a database.


## 13.6. VERTICAL-003: Skills + Passive Contribution

VERTICAL-003 completes the character + stats vertical slice by adding passive
skills as an authoritative source of stat contributions. The server owns the
learned skill set, aggregates passive skill definitions into a
`Stats::PassiveContribution`, and publishes the result through the shared
`CharacterSnapshot`; the client presents what it receives with no local
recomputation.

### What it contains

| Unit                                                  | Responsibility                                                         |
| ----------------------------------------------------- | ---------------------------------------------------------------------- |
| `modern/core/skills/SkillDefinition.h, .cpp`          | Passive skill identity, weapon requirements, per-level values, impacts |
| `modern/core/skills/SkillState.h, .cpp`               | Character's learned skill set (ordered map, deterministic iteration)   |
| `modern/core/skills/SkillDefinitionProvider.h, .cpp`  | Read-only interface + `InMemorySkillDefinitions` sorted-vector impl    |
| `modern/core/skills/PassiveContributionAggregator.h, .cpp` | Learned skills + definitions -> `Stats::PassiveContribution`       |
| `modern/server/character/ServerCharacter.h, .cpp` (ext.) | `LearnSkill`/`UnlearnSkill`/`SetSkillLevel`, passive aggregation in `Recalculate()`, snapshot publication |
| `modern/client/gameplay/ClientCharacterState.h, .cpp` (ext.) | Read-only skill views: `GetSkills()`, `HasSkill()`, `GetSkillLevel()`, `GetLearnedSkillCount()` |
| `modern/core/gameplay/CharacterSnapshot.h, .cpp` (ext.) | `SkillList`, `LearnedSkillEntry`, skill validation in `IsValid()`      |
| `modern/core/stats/Contributions.h`, `StatCalculator.cpp` (ext.) | `IsZero()` for the two contributions, so a second source can be refused without naming every field |
| `modern/tests/SkillTests.cpp`               | the transcription tests, in core: enum-to-field mapping, per-level selection, refusals, the weapon gate |

### Key conventions

- **Passive skills are the sole source of passive contributions.** `ServerCharacterDefinition.passives` is zeroed in `Create()` and never read again; `SetContributions()` refuses a non-zero item *or* passive contribution with `NotAllowed`, so a caller cannot install a second source. All `PassiveContribution` comes from the learned skill set.
- **The server owns the learned skill set.** `LearnSkill`/`UnlearnSkill`/`SetSkillLevel` are the only mutators; each stages the change on a copy, validates against the skill definition provider, commits, then recalculates before returning, and restores the previous state if the recalculation fails. A failed call leaves the character exactly as it was.
- **`SkillId` is a `(classIndex, skillIndex)` pair, and validity is a sentinel.** `IsValid()` is false only when a half is `0xFFFF` (`SNATIVEID::ID_NULL`), so a default-constructed `SkillId{0, 0}` names a real skill and is *valid*. There is no `MakeInvalid()`, unlike `ItemId`. What keeps a zeroed id out of a character is `LearnSkill` resolving it against the provider first and refusing `NotFound` — the `SkillState` value test is the weaker of the two defences, not the only one.
- **The client receives, never computes.** `ClientCharacterState` exposes `GetSkills()`, `HasSkill()`, `GetSkillLevel()`, `GetLearnedSkillCount()` — all read-only views of the published snapshot. The client translation unit contains no call to `Modern::Stats::Calculate`.
- **Definitions are the shared truth.** `SkillDefinition` carries the per-level basic values and impacts every copy of the skill contributes. Prerequisites, class restrictions, and SP costs are deferred; the investigation report records why they are absent from the stat pipeline.
- **Aggregation is deterministic.** Skills are visited in `SkillId` order (map order), so the same skill set always produces the same contribution regardless of learning order. The six base stats accumulate as 16-bit unsigned values (wrapping like RAN's `SSUM_ITEM`); the wrap happens in `Calculate`, not in the aggregator.
- **Defense in depth.** `InMemorySkillDefinitions::Add` rejects invalid definitions. The aggregator additionally checks `IsFinite()` on every definition it reads and returns `PassiveAggregationError::NonFinite` if one slips through. Missing definitions yield `PassiveAggregationError::MissingDefinition` — a divergence from RAN, which skips a skill it cannot resolve (see the investigation report, §2).
- **Equipment-dependent passives — LIMITED.** If a passive requires a weapon type in a hand slot, the slot must hold an item; if it does not, the skill contributes nothing and the call still succeeds. **What is not checked is the weapon type.** RAN compares `SITEM::sSuitOp.emAttack` through `CHECHSKILL_ITEM`; `ItemDefinition` carries no attack type, so the check is slot occupancy only. A dagger in the right hand will therefore activate a sword-gated passive. `Server_WeaponDependentPassiveNeedsTheSlotOccupied` asserts as far as the data allows and says so at the assertion.

### Known divergences from RAN

| Divergence | Legacy | Modern | Why |
| --- | --- | --- | --- |
| Passive accumulator width | `SPASSIVE_SKILL_DATA` integer fields are `short` | `int32_t` / `float` | a stat total that wraps at 32767 is a bug, not behaviour to preserve |
| A learned skill with no definition | `continue` — silently no contribution | `MissingDefinition`, recalculation refused | a silent zero is indistinguishable from a data fault |
| Weapon gate | weapon-type match, plus a hidden-fist case | slot occupancy only | no attack type on `ItemDefinition`; see LIMITED above |
| `EMSPECA_*` specs | summed, except four arms that take the maximum | not modelled | CORE-002 has no destination; a straight `+=` would be wrong for four arms |
| Vehicle exclusion | a character in a vehicle gets no passives at all | not modelled | no vehicle system exists |

The full trace, including the four max-not-sum spec arms and the
`SRESIST` all-five-elements rule, is in
`docs/reference/client/VERTICAL-003_SKILL_INVESTIGATION.md`.

### Legacy provenance

| Modern element                          | Legacy origin                                                                 |
| --------------------------------------- | ----------------------------------------------------------------------------- |
| `SkillId` (classIndex + skillIndex)     | `SNATIVEID` (wMainID, wSubID) in `GLCharData.h` / `GLSkill.h`                |
| `SkillState` (learned skill map)        | `SCHARSKILL` + `SKILL_MAP` in `GLCharData.h:233, 889`                        |
| `PassiveApplyType`                      | `SKILL::EMTYPES` in `GLSkillApply.h`                                          |
| `PassiveImpactType`                     | `SKILL::EMIMPACT_ADDON` in `GLSkillApply.h`                                   |
| `PassiveSpecType`                       | `SKILL::EMSPEC_ADDON` in `GLSkillApply.h` (not in stat pipeline)             |
| `SkillWeaponType` / `SkillWeaponSlot`   | `SKILL::GLSKILL_ATT` in `GLSkillBasic.h`                                      |
| Passive aggregation loop                | `GLCHARLOGIC::SUM_PASSIVE` in `GLogixExPC.cpp:863`                           |
| Passive skill data structure            | `SPASSIVE_SKILL_DATA` in `GLCharData.h:1123`                                |
| Per-level basic values                  | `SKILL::CDATA_LVL::fBASIC_VAR` in `GLSkillApply.h:252`                     |
| Prerequisite skill system               | `SLEARN` in `GLSkillLearn.h:75` (deferred)                                 |
| Weapon-type check                        | `CHECHSKILL_ITEM` in `GLogicEx.h:1274` (LIMITED, see above)                |
| Level / impact / spec limits             | `SKILL::MAX_LEVEL` / `MAX_IMPACT` / `MAX_SPEC` in `GLSkillDefine.h:16-18`  |
| Role filter                              | `SKILL::EMROLE_PASSIVE` in `GLSkillBasic.h:133`                            |

The full investigation is in `docs/reference/client/VERTICAL-003_SKILL_INVESTIGATION.md`.

### Verifying

```powershell
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

Release is built and run the same way with `--config Release`. Both
configurations pass all 14 suites.

The headless test suite covers:

| Suite | Cases | What it covers |
| ----- | ----: | -------------- |
| `ModernCoreTests` | 132 | `SkillId`, `SkillDefinition`, `SkillState`, `SkillDefinitionProvider`, `PassiveContributionAggregator` — every `EMTYPES` / `EMIMPACTA_*` arm landing in the contribution field RAN's switch names, per-level value selection, additive stacking, order independence, refusals, the weapon-slot gate, input immutability |
| `ModernServerTests` | 35 | learn / level / unlearn, contribution and derived-stat movement, snapshot publication, order-independent stacking, refused mutations leaving the character byte-identical, the weapon gate, `SetContributions` refusing a second source |
| `ModernClientGameplayTests` | 18 | read-only skill views, level and unlearn arriving as new snapshots, agreement with the one stat implementation, refusing a malformed skill level, and the source-level no-calculation check |

All tests pass without legacy libraries, DirectX, sockets, or a database. The
`RecalculateIndependently` helper in both server and client suites rebuilds the
stat input from the character's *own* learned set and worn set and calls
`Stats::Calculate` directly, so "the server's numbers are the formula's numbers"
is checked against the inputs rather than against a stored expectation.


## 13.7. VERTICAL-004: Codex Progress + Contribution

VERTICAL-004 adds the codex as a third authoritative source of stat
contributions. The server owns the two codex maps, decides what is completable,
aggregates the completed set into a `Stats::CodexContribution`, and publishes the
result and the progress through the shared `CharacterSnapshot`; the client
presents what it receives with no local recomputation.

The shape of this milestone is set by one legacy finding: **of RAN's eleven
codex progress types, exactly one has a working implementation.** The per-type
`switch` in `SCODEX_CHAR_DATA::Assign` (`GLCodexData.cpp:415-480`) and in
`Correction` (`:508-585`) are both inside block comments. Registering an item is
the only thing that advances a codex entry in RAN, so it is the only thing that
does in the modern tree.

### What it contains

| Unit                                                  | Responsibility                                                         |
| ----------------------------------------------------- | ---------------------------------------------------------------------- |
| `modern/core/types/Ids.h` (ext.)                       | `CodexId`, distinct from `ItemId` and `SkillId`                        |
| `modern/core/progression/CodexDefinition.h, .cpp`      | `CodexType`, `CodexNotify`, `CodexRequirement`, `CodexDefinition`, the required-count cascade |
| `modern/core/progression/CodexDefinitionProvider.h, .cpp` | Read-only `Find`/`GetAll` + `InMemoryCodexDefinitions` sorted-vector impl |
| `modern/core/progression/CodexState.h, .cpp`           | Two ordered maps, `Reconcile`, `RegisterItem`, `CodexRegistration`     |
| `modern/core/progression/CodexContributionAggregator.h, .cpp` | Completed set + definitions -> `Stats::CodexContribution`        |
| `modern/server/character/ServerCharacter.h, .cpp` (ext.) | `codexDefinitions` on the definition, `RegisterCodexItem`, `ReconcileCodex`, aggregation in `Recalculate()`, snapshot publication |
| `modern/client/gameplay/ClientCharacterState.h, .cpp` (ext.) | Read-only codex views: `GetCodex()`, `GetCodexEntry()`, `HasCodex()`, `IsCodexCompleted()`, `IsCodexInProgress()`, three counts |
| `modern/core/gameplay/CharacterSnapshot.h, .cpp` (ext.) | `CodexList`, `CodexEntry`, codex validation in `IsValid()`             |
| `modern/tests/CodexTests.cpp`                          | the transcription tests, in core: the cascade, the match rules, reconciliation, aggregation |

### Key conventions

- **The completed codex set is the sole source of the codex contribution.**
  `ServerCharacterDefinition.codex` is not read as a source, and
  `SetContributions()` refuses a non-zero codex contribution with `NotAllowed`,
  on the same terms as the item and passive arguments it already refused. The
  check is on the *argument*, not on the derived member: testing the member would
  accept a caller's value and drop it while refusing a zeroed one.
- **The server owns both maps.** `RegisterCodexItem` is the only mutator. It
  stages on a copy of `CodexState`, commits, recalculates, and restores the
  previous state if the recalculation fails, so a failed call leaves the codex
  set, the contribution and the statistics exactly as they were. A refused
  registration does not recalculate, because it changed nothing.
- **Two maps, not one record with a flag.** RAN holds `m_mapCodexProg` and
  `m_mapCodexDone` keyed by codex id, so an id is in exactly one. Completion is a
  two-phase move: the counter is clamped, then a second pass inserts into the
  done map and erases from the progress map. This is why a single registration
  can complete at most the entries it names.
- **The required-count cascade is reproduced, quirks included.**
  `CodexDefinition::RequiredSlotCount` is four sequential assignments with the
  last match winning, and slot 0 is never tested. See "Known divergences" below.
- **A done flag is a claim about a requirement, not about a record.** So
  `Reconcile` carries progress across a refresh that changed nothing and drops it
  when any requirement or the required count actually changed. Reconciliation
  runs on every load, so a version that always reset would silently destroy every
  character's progress.
- **The client receives, never computes.** `ClientCharacterState` exposes the
  published `CodexList` through const references and has no codex contribution
  of its own. RAN's client keeps a private mirror of both maps and calls its own
  `CODEX_STATS` on completion (`GLCharacterMsg.cpp:5206-5214`), so the formula
  runs on both sides; that is not reproduced.
- **The snapshot carries no reward values.** `CodexEntry` publishes id, type,
  name, description, badge and the counters. The bonus is already folded into
  `derived` by the server, so a client cannot compute a second one even if it
  wanted to.

### Known divergences from RAN

| Divergence | Legacy | Modern | Why |
| --- | --- | --- | --- |
| Per-type progress rules | both `switch`es commented out | not implemented | no live legacy rule to transcribe; not deferred behind a stub |
| A completed entry with no definition row | `CodexComplete` returns early, and the caller still erases the progress record — the entry vanishes from both maps | skipped, reason reported as `CodexContributionError::MissingDefinition` | a silent loss is indistinguishable from a data fault |
| An entry naming no item in its counted window | seated, never completable | seated (matching RAN), registration refused `NotCompletable` | the seating matches; the refusal makes a broken table row visible |
| Grade match | exact, against the registered instance's grinding grade | field kept, **not enforced** — LIMITED | `ItemInstance` carries no grade, upgrade or option state, by design |
| Quantity match | `==` | `==` | relaxed to `>=` would let one stack satisfy a five-count requirement |
| Item spend | deleted unconditionally, even when nothing matched | not spent; `CodexRegistration::recorded` is returned | reproduces RAN's data loss otherwise |
| Recompute | full over the done map, both parameters unused | full, no parameters | a partial recompute would be a second authority |
| Reconciliation refresh | the five item ids only; quantities, grades and `dwProgressMax` go stale | the whole requirement and the required count | a retune must not leave a character holding a stale completion condition |
| Badge grant | never, within the codex system | never, and named `rewardBadge` to say so | the grant lives in the separate Activity system |
| Client recompute | private mirror plus its own `CODEX_STATS` | snapshot only | one authority, one answer |

The full trace, including the required-count cascade, the eleven-type reward
mapping and the per-slot match rule, is in
`docs/reference/client/VERTICAL-004_CODEX_INVESTIGATION.md`.

### Legacy provenance

| Modern element                          | Legacy origin                                                                 |
| --------------------------------------- | ----------------------------------------------------------------------------- |
| `CodexId`                               | `DWORD dwCodexID` in `GLCodexData.h:26`                                        |
| `CodexDefinition`                       | `SCODEX_FILE_DATA` in `GLCodexData.h:26`                                       |
| `CodexRequirement` (item, quantity, grade) | `sidProgressItem1..5` / `wQuantity1..5` / `wItemGrade1..5` in `GLCodexData.cpp:388-407` |
| `CodexState`'s two maps                 | `m_mapCodexProg` / `m_mapCodexDone` in `GLCharData.h`                         |
| Required-count cascade                  | `dwProgressMax` in `SCODEX_CHAR_DATA::Assign`, `GLCodexData.cpp:377-386`       |
| `CodexState::Reconcile`                 | `GLCharDataCodex.cpp:73-132` (load reconciliation and orphan removal)         |
| `CodexState::RegisterItem`              | `GLChar::DoCodexRegisterItem`, `GLCharCodex.cpp:60-179`                        |
| `GLChar::CodexComplete` → the done-map move | `GLCharCodex.cpp:14-19` and the second pass at `:161-173`                  |
| Request validation and the item spend    | the item-registration handler, `GLCharInvenMsg.cpp:9080-9213`                  |
| `CodexContributionAggregator`           | `GLCHARLOGIC::CODEX_STATS`, `GLogixExPC.cpp:5103-5173`                         |
| `CodexEntry` in the snapshot             | the two client inserts in `DxGameStage.cpp:931`, `:950`                       |
| `Stats::CodexContribution`              | the eleven `m_dw*Increase` accumulators, `GLogixExPC.cpp:5106-5168` (CORE-002) |

### Verifying

```powershell
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

Release is built and run the same way with `--config Release`. Both
configurations pass all 14 suites.

| Suite | Cases | What it covers |
| ----- | ----: | -------------- |
| `ModernCoreTests` | 167 | the cascade at every prefix length, the untested slot 0, the empty entry, id/quantity equality, one stack satisfying two slots, a satisfied slot not recorded twice, a slot outside the required count, the clamp and the two-phase move, the four distinguishable refusals, reconciliation (seating, the completed skip, the type reset, the refresh, progress preserved / dropped, orphan removal), aggregation (completed only, per-field, idempotent, order-independent, skip reported, unmapped type reported) |
| `ModernServerTests` | 46 | seating on create, a registration moving the derived statistics and matching an independent `Stats::Calculate`, only-a-completed-entry-pays, the reward paid exactly once, reconciliation not repaying, the aggregate not drifting across four reloads, per-type mapping at the server boundary, the published snapshot carrying the authoritative counters, repeated snapshots identical, no provider being inert, an empty table refused, `SetContributions` refusing a third source, a rejected registration leaving the character intact |
| `ModernClientGameplayTests` | 22 | presentation from a snapshot, finished and unfinished being distinguishable, the views being const-refs over the published list, the client's derived statistics being the server's, the empty fallbacks with no snapshot, and a later snapshot replacing an earlier codex rather than merging with it |

All tests pass without legacy libraries, DirectX, sockets, or a database.


## 14. CLIENT-002: modern client application foundation

Establishes modern/client/application, providing an isolated, deterministic
application lifecycle and frame loop without legacy Direct3D or MFC dependencies.

### What it contains

| Unit                                    | Responsibility                                                |
| --------------------------------------- | ------------------------------------------------------------- |
| modern/client/application/Application | Lifecycle state machine, frame rate throttle, update pipeline |

### Key conventions

- **Strict isolation from legacy engine loops**: CD3DApplication and CGameClient2Wnd
  are not referenced; the application owns startup, shutdown, and update ticks cleanly.
- **Headless testability**: IPlatformEvents and IInputSource default to headless
  stubs (NullPlatformEvents, NullInputSource) allowing deterministic simulation
  under unit tests and CLI tools without creating an OS window.
- **Explicit frame callbacks**: Simulation logic hooks into SetUpdateCallback with
  monotonic frame numbering and clean lifecycle state transitions (Uninitialized ->
  Initialized -> Running -> Stopping -> Stopped).


## 15. CLIENT-003: modern client input boundary

Establishes modern/client/input, providing a platform-independent input model,
instantaneous per-frame event queue, persistent key/mouse state tracking, and
seamless integration into the client application loop.

### What it contains

| Unit                                               | Responsibility                                                    |
| -------------------------------------------------- | ----------------------------------------------------------------- |
| modern/client/input/InputEvents.h, .cpp        | Portable KeyCode, MouseButton, InputEvent tagged variant    |
| modern/client/input/InputSystem.h, .cpp        | FIFO per-frame queue, persistent held state, frame boundary clear |
| modern/client/input/FakeInputSource.h, .cpp    | Scripted deterministic input feeds for headless test harnesses    |
| modern/client/input/platform/Win32InputAdapter.* | Isolated Win32 virtual key and mouse sample translation seam      |

### Key conventions

- **Platform isolation**: Public modern interfaces expose zero platform headers
  (<Windows.h>, DirectInput, MFC) and no raw handle types (HWND, WPARAM, LPARAM).
  Windows key codes (VK_*) are translated to modern KeyCode enums strictly inside
  Win32InputAdapter.cpp.
- **Event vs. state separation**: Instantaneous per-frame events (KeyEvent,
  MouseButtonEvent, MouseMoveEvent, MouseWheelEvent, WindowCloseEvent,
  WindowResizeEvent) reside in a FIFO queue cleared deterministically each frame via
  EndFrame(). Persistent querying state (IsKeyDown(), IsMouseButtonDown(),
  GetMouseX(), GetMouseY(), GetMouseWheelDeltaY()) remains stable across frame
  boundaries, preserving legacy semantic behavior while eliminating hidden globals.
- **Single application input pipeline**: Application polls IInputSource, routes
  events into InputSystem, broadcasts to subscribers, runs the update callback, and
  calls EndFrame() at frame boundaries in strict, reproducible order.


## 16. CLIENT-004: modern client rendering / RHI boundary

Establishes `modern/client/rendering`, providing a platform-independent renderer
abstraction (`IRenderer`), rendering configuration model (`RendererConfig`),
deterministic headless backend (`NullRenderer`), and seamless integration into
the client application pipeline without introducing any GPU API, Win32 handle, or
legacy DirectX code dependencies.

### Architecture

```text
Modern Core
    ↑
Modern Client
    ├── Application
    │     ├── InputSystem (CLIENT-003)
    │     └── IRenderer (CLIENT-004)
    │           └── NullRenderer
    ├── Input
    └── Rendering
```

Application frame semantics:

```text
Platform / Input Source
      ↓
InputSystem (FIFO events queued)
      ↓
Window Resize Check -> Application updates IRenderer::Resize
      ↓
Input Subscribers
      ↓
Application Update Callback (game state)
      ↓
IRenderer::BeginFrame()
      ↓
Application Render Callback (render passes)
      ↓
IRenderer::EndFrame()
      ↓
InputSystem::EndFrame()
```

### What it contains

| Unit                                                    | Responsibility                                                     |
| ------------------------------------------------------- | ------------------------------------------------------------------ |
| `modern/client/rendering/RenderingTypes.h, .cpp`        | Portable `RendererConfig`, `RenderColor`, `RendererState`, enums    |
| `modern/client/rendering/Renderer.h, .cpp`              | Clean, pure abstract `IRenderer` interface contract                |
| `modern/client/rendering/NullRenderer.h, .cpp`          | Deterministic headless renderer implementation tracking state/stats|
| `modern/client/rendering/ClientRenderingTests.cpp`      | 16 targeted unit tests covering lifecycle, resize, clear, frame    |

### Key conventions

- **Strict API isolation**: Public modern client rendering headers never include
  `<Windows.h>`, Direct3D (`d3d9.h`, `d3d11.h`), DXGI, Vulkan, OpenGL, or MFC.
  No `HWND`, `HINSTANCE`, `ID3D11Device`, or `VkInstance` leaks past the boundary.
- **Renderer lifecycle state machine**:
  ```text
  Uninitialized -> Initialized <-> InFrame -> Shutdown
  ```
  Invalid state transitions return standard `Status(ErrorCode::InvalidState)` or
  `Status(ErrorCode::NotAllowed)` rather than throwing exceptions or crashing.
- **Decoupled input and rendering**: `InputSystem` has zero knowledge of `IRenderer`.
  The `Application` orchestrates resize event translation: when a `WindowResized`
  input event arrives, `Application` calls `m_renderer->Resize(width, height)`.
- **Headless testability**: `NullRenderer` provides a complete working implementation
  enabling CI, automated testing, and headless server/emulator execution without
  requiring an active GPU, display adapter, or OS window.
- **Future backend strategy**: DirectX 12, Vulkan, or other modern graphics APIs
  will be implemented as concrete backends implementing `IRenderer` under
  `modern/client/rendering/backends/` without altering the modern application or core.
- **Asset upload boundary (CLIENT-009, section 21)**: `IAssetUploader` sits
  beside this renderer boundary as its own target (`ModernClientAssetUpload`)
  and consumes CPU assets through opaque handles. `IRenderer` itself still
  knows nothing about assets, storage or upload.

## 17. CLIENT-005: modern client resource / asset boundary

Establishes `modern/client/resources`, providing a platform-independent resource
boundary consisting of a strongly-typed identifier (`ResourceId`), an immutable
binary data representation (`ResourceData`), a provider abstraction (`IResourceProvider`),
an in-memory test provider (`MemoryResourceProvider`), and a deterministic caching
coordinator (`ResourceManager`).

### Architecture

```text
Modern Core
    ↑
Modern Client
    ├── Application
    │     ├── InputSystem (CLIENT-003)
    │     ├── IRenderer (CLIENT-004)
    │     └── ResourceManager (CLIENT-005)
    ├── Input
    ├── Rendering
    └── Resources
```

### What it contains

| Unit                                                    | Responsibility                                                     |
| ------------------------------------------------------- | ------------------------------------------------------------------ |
| `modern/client/resources/ResourceId.h, .cpp`            | Strongly validated logical resource identifier (canonical forward slashes) |
| `modern/client/resources/ResourceData.h`                | Immutable/value-oriented binary payload holding loaded bytes       |
| `modern/client/resources/ResourceProvider.h`            | Pure abstract contract resolving `ResourceId` -> `ResourceData`    |
| `modern/client/resources/MemoryResourceProvider.h, .cpp`| Headless test provider storing in-memory registered byte payloads  |
| `modern/client/resources/ResourceManager.h, .cpp`       | Cache-coordinating manager with deterministic lifecycle            |
| `modern/client/resources/ClientResourceTests.cpp`       | 10 CLIENT-005 headless tests (extended by CLIENT-006)              |

### Key conventions

- **Strict isolation & zero GPU/OS coupling**: Modern client resource headers never include
  `<Windows.h>`, Direct3D (`d3d9.h`, `d3d11.h`), Vulkan, OpenGL, MFC, or legacy RAN headers.
  The resource subsystem does NOT depend on `modern/client/rendering` or vice-versa.
  The resource layer delivers raw bytes; decoding into GPU resources (textures, vertex buffers,
  shaders) is the responsibility of future asset decoders and render resource factories.
- **ResourceId semantics**:
  `ResourceId` represents a logical, portable identity (e.g. `textures/ui/login_background`,
  `models/character/body`), never a raw Windows path or backslash-laden filesystem path.
  Construction via `ResourceId::Create(...)` rejects empty names, whitespace, backslashes,
  and leading/trailing slashes, returning `ErrorCode::InvalidArgument`.
- **ResourceData semantics**:
  A safe, immutable byte container (`std::vector<uint8_t>`) that can be viewed as bytes or
  string views without exposing OS file descriptors or raw pointer ownership.
- **Provider abstraction**:
  `IResourceProvider` defines `HasResource(id)` and `Load(id)`. Several providers exist
  behind it: `MemoryResourceProvider` for headless testing (CLIENT-005) and
  `FileSystemResourceProvider` for loose files under a configured root (CLIENT-006).
  Future archive, patch and remote providers implement the same two methods.
- **ResourceManager lifecycle**:
  ```text
  Uninitialized -> Ready -> Shutdown
  ```
  `Initialize()` requires an injected `IResourceProvider`. `Load()` looks up the internal
  cache first, falling back to the provider, caching successful results deterministically.
- **Legacy RAN formats inspection & isolation**:
  Legacy RAN assets use diverse formats and access systems:
  - `CryptionRCC` / `CCrypt`: proprietary block encryption (`0x100` version, header offset 12)
    applied to loose files and RCC archives.
  - `FileCrypt` / `IMethod`: blowfish/block encryption for client assets.
  - `SFileSystem`: legacy archive packaging format (`RANPACKAGEFILESYSTEM` header).
  - Legacy asset types: `.isf` (items/skills), `.ssf` (effects), `.cps` (characters), `.x` (DirectX meshes).
  - Legacy manager coupling: `TextureManager` and `DxMeshTexMan` were tightly coupled to
    `LPDIRECT3DDEVICEQ`, D3DX9 texture loaders (`D3DXCreateTextureFromFileInMemoryEx`), and MFC globals.
  These legacy systems are intentionally NOT ported into `modern/client/resources`. Any future
  support for legacy RCC archives or binary formats will be implemented via dedicated, isolated
  adapters under `modern/compatibility/` or future data importer tools.

## 18. CLIENT-006: filesystem asset provider

Gives the CLIENT-005 resource boundary one real storage backend.
`FileSystemResourceProvider` resolves a logical `ResourceId` against a single
configured root directory and returns the bytes of the file it finds there.
Nothing above it changed: `ResourceManager`, `IResourceProvider`, `ResourceId`
and `ResourceData` are exactly as CLIENT-005 left them.

### Architecture

```text
ResourceManager
       ↓
IResourceProvider
   ┌───┴───────────────────────┐
   ↓                           ↓
MemoryResourceProvider   FileSystemResourceProvider
                               ↓
                          local files
```

The interface is all the manager knows: it holds a pointer, calls
`HasResource()` and `Load()`, and caches what comes back. Which storage answered
is invisible above `IResourceProvider`, which is what lets a future provider
replace the filesystem without the manager noticing.

### What it contains

| Unit                                                           | Responsibility                                                          |
| -------------------------------------------------------------- | ----------------------------------------------------------------------- |
| `modern/client/resources/FileSystemResourceProvider.h, .cpp`   | Resolves `ResourceId` to a file under a configured root and returns its bytes |
| `modern/client/resources/ClientResourceTests.cpp` (extended)   | +21 CLIENT-006 cases: roots, loading, traversal, identifiers, manager integration |

### Root configuration

- The root is a constructor argument: `FileSystemResourceProvider provider(root)`.
  There is no default, no environment variable, no registry key and no
  installation directory compiled in. A provider that has to be told where its
  assets live is a provider that can be pointed at a test directory.
- The constructor stores the path and performs no I/O. `Initialize()` validates it
  and resolves it once to a canonical absolute path, kept as the resolved root.
  Nothing before that point touches the filesystem, so a bad root is a returned
  code rather than a throw out of a constructor.
- `Initialize()` never creates the root. A missing root is `NotFound`; a root that
  exists but is not a directory, or an empty path, is `InvalidArgument`; a second
  `Initialize()` is `InvalidState`.

### ResourceId to file resolution

- An identifier is a forward-slash logical namespace joined to the root as path
  components: `ui/textures/login_background` resolves under
  `<root>/ui/textures/login_background`.
- No extension is appended and no name is rewritten. A provider that silently
  tried `login_background.dds` would make the asset-to-file mapping unguessable,
  and guessability is what the identifier exists to remove.
- The identifier is preserved exactly; only the path handed to the platform is
  translated. `ResourceId::Create()` still owns the logical form.

### Path safety

`ResourceId` is a logical name; whether it can escape the root is a property of
the code that turns it into a path, so the checks live in the provider.

- **Lexical rules** (`IsPortableLogicalName`, applied before any path is built):
  printable ASCII only (no confusion between a solidus and a lookalike), no `:`
  (drive letters, NTFS alternate data streams), no `\` (Windows separators, UNC
  and device paths), no empty components, no leading or trailing separator, no
  `.` or `..` component, and no component ending in `.` or ` ` (Windows strips
  those, so `a` and `a.` would otherwise name one file while comparing as two
  identifiers).
- **Semantic containment**: the joined path goes through `weakly_canonical()`,
  which resolves the components that exist - symlinks included - and normalises
  the rest, and the result must be strictly below the resolved root, compared
  component by component so that `assets-extra` is not mistaken for `assets`.
  `weakly_canonical()` rather than `canonical()` because the target need not
  exist: `HasResource()` has to be able to answer no.
- Every refusal is `InvalidArgument`, from `HasResource()` as `false` and from
  `Load()` as an error. An identifier that escapes the root is a bad argument,
  not a missing file.
- The lexical half is the primary guard because it is deterministic and needs no
  I/O; the containment check covers what naming rules cannot see, such as a
  symlink inside the root pointing out of it.
- `ResourceId::Create()` already refuses leading, trailing and repeated
  separators and backslashes, so several of these cases cannot be built through
  the public identifier API. The provider re-checks them anyway - defense in
  depth - and the reachable cases (`../x`, `a/../b`, `C:/x`, `file:stream`) are
  covered by tests.

### Loading and error mapping

`Load()` validates the identifier, resolves the path, confirms a regular file,
opens it binary and read-only, sizes it, reads it fully and returns the bytes as
`ResourceData`. `HasResource()` stops after the metadata check: it never opens a
stream, so it cannot read a file in order to answer a question about existence.

| Situation                             | `Load`            | `HasResource` |
| ------------------------------------- | ----------------- | ------------- |
| `Initialize()` has not been called    | `InvalidState`    | `false`       |
| invalid or unsafe identifier          | `InvalidArgument` | `false`       |
| no file at the resolved path          | `NotFound`        | `false`       |
| the resolved path is a directory      | `NotFound`        | `false`       |
| regular file, zero bytes              | success, 0 bytes  | `true`        |
| regular file, bytes returned          | success           | `true`        |
| file opened but not read to the end   | `NotFound`        | `true`        |

- No new error codes were introduced. A read that fails after a successful open
  reuses `NotFound`, because the core vocabulary has no I/O code by design and
  adding one is a Core-level decision; either way the provider cannot produce the
  resource.
- A file whose size no `size_t` can hold is refused rather than truncated, which
  matters because this repository builds a 32-bit client.
- Nothing throws for a resource failure, filesystem calls included: every one of
  them uses the `std::error_code` overload.

### What the provider deliberately is not

- **Not a cache.** `ResourceManager` owns caching, and a second cache here would
  make "was this read from disk?" unanswerable. A test asserts the split: the
  manager serves its cached copy while the provider, asked directly, re-reads the
  changed file.
- **Not a decoder.** It returns bytes. No `IRenderer`, no DirectX/Vulkan/OpenGL,
  no texture or mesh type, no GPU upload, and no dependency between
  `modern/client/resources` and `modern/client/rendering` in either direction.
- **Not concurrent.** One call, one synchronous read: no worker threads, no async
  API, no mutex. Threaded loading remains a future architecture decision.
- **Not an archive reader.** `CryptionRCC`/`CCrypt`, `FileCrypt`/`IMethod`,
  `SFileSystem`, the `.isf`/`.ssf`/`.cps`/`.x` formats and the legacy
  `TextureManager`/`DxMeshTexMan` coupling stay outside this layer. The path is a
  future `ArchiveResourceProvider` behind `IResourceProvider`, not a dependency of
  `ResourceManager`:

  ```text
  legacy archive
        ↓
  future ArchiveResourceProvider
        ↓
  IResourceProvider
        ↓
  ResourceManager
  ```

  `ArchiveResourceProvider`, `RemoteResourceProvider` and `PatchResourceProvider`
  can each implement the same two methods, which is the test of whether this
  boundary is drawn in the right place.

### Tests and verification

`ClientResourceTests.cpp` carries 31 cases: the ten CLIENT-005 cases plus
twenty-one CLIENT-006 cases covering valid, missing and invalid roots; discovery
and loading; empty files; directories; nested paths; forward-slash identifiers;
parent and nested traversal; absolute, drive-letter and alternate-stream
identifiers; files outside the root; metadata-only `HasResource()`; and
`ResourceManager` integration, caching and rendering independence.

Every filesystem case builds its own root under the system temporary directory,
writes deterministic bytes into it and removes it on destruction, so no test
reads a RAN installation and none needs one. `ModernEmulator` shows the same on a
temporary root it creates and deletes itself.

```powershell
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```


## 19. CLIENT-007: typed asset / decoder boundary

Places a typed CPU-side asset layer above `ResourceData`. CLIENT-005 and
CLIENT-006 move bytes; CLIENT-007 is where those bytes can become something the
rest of the client can name. Nothing below it changed: `IResourceProvider`,
`MemoryResourceProvider`, `FileSystemResourceProvider`, `ResourceId`,
`ResourceData` and `ResourceManager` are exactly as CLIENT-005 and CLIENT-006
left them.

### Architecture

```text
IResourceProvider
        |
        v
ResourceManager          bytes: cached, untyped
        |
        v
ResourceData
        |
        v
IImageDecoder            DecodeImage(ResourceData) -> Result<ImageAsset>
        |
        v
ImageAsset               validated CPU-side image
        |
        v
asset upload boundary     IAssetUploader -> opaque handle (CLIENT-009)
```

### Why typed assets exist

A `ResourceData` is a bag of bytes that has been checked for exactly one thing:
that something could produce it. It has no width, no height and no layout, so
every consumer that needs one would have to ask "is this really an image?"
separately — and each would answer slightly differently, one silently accepting
a short payload, another trusting a header field, a third reading past the end.

CLIENT-007 answers that question once, at a boundary, and hands every later
consumer the same answer as a value that cannot be in an invalid state. Three
consequences follow, and they are the reason the layer exists:

- **A decode failure is visible where it happens.** The caller gets
  `InvalidArgument` from `DecodeImage()`, at the call that read the resource,
  rather than a suspicious texture three systems later.
- **Consumers do not repeat the checks.** A renderer adapter, a future UI atlas
  builder and a test all read the same validated `ImageAsset`; none of them
  re-derives what "24 bytes for a 3 x 2 RGBA image" means.
- **The renderer is not on the critical path of correctness.** Whether an image
  is valid is decided without a device, a window or a swap chain, so the rule is
  testable in a console process.

### What it contains

| Unit                                            | Responsibility                                                                  |
| ----------------------------------------------- | ------------------------------------------------------------------------------- |
| `modern/client/assets/AssetTypes.h, .cpp`        | `ImageFormat`, its name, bytes per pixel, and the checked byte-count calculation |
| `modern/client/assets/ImageAsset.h, .cpp`        | Validated, immutable CPU-side image: geometry, layout, pixel bytes                |
| `modern/client/assets/ImageDecoder.h`            | `IImageDecoder`: the `ResourceData` -> `ImageAsset` contract                     |
| `modern/client/assets/TestImageDecoder.h, .cpp`  | The one concrete decoder: the documented MIMG test container                     |
| `modern/client/assets/ClientAssetTests.cpp`      | 31 headless cases: vocabulary, image invariants, decoder, integration             |

`ModernClientAssets` links `Modern` and `ModernClientResources`. It links **no**
renderer, no graphics API and no legacy library — see the banner in
`modern/client/assets/CMakeLists.txt`, which states the direction and the reason
rather than leaving it implied by an include path.

### ImageAsset design

```cpp
class ImageAsset
{
public:
    static Result<ImageAsset> Create(uint32_t width, uint32_t height,
                                     ImageFormat format, std::vector<uint8_t> pixels);

    uint32_t GetWidth() const noexcept;
    uint32_t GetHeight() const noexcept;
    ImageFormat GetFormat() const noexcept;
    size_t GetPixelCount() const noexcept;
    size_t GetPixelByteCount() const noexcept;
    const std::vector<uint8_t>& GetPixels() const noexcept;
};
```

- **No default constructor.** There is no "empty but valid" image and no
  half-built one, so an `ImageAsset` is either absent or already checked. That is
  why no case in the suite tests for an invalid state: the state cannot be named.
- **Immutable by interface.** Pixels leave through a const reference and the
  factory is the only way in. Changing pixels means building a new value, which
  re-runs every check.
- **Pixels are a copy this layer owns.** `Create()` takes the vector by value, so
  the source can be reused or destroyed immediately. Pixels are row-major,
  tightly packed, top row first, with no row padding: the layout has to be stated
  once, and it is stated here.
- **CPU-side only.** No `IDirect3DTexture9`, `ID3D11Texture2D`, `VkImage`,
  `GLuint`, `HWND` or device pointer appears in the header, which includes no
  Windows header at all. An `ImageAsset` never learns which API eventually reads
  it.

Validation, all of it returning `ErrorCode::InvalidArgument` and none of it
throwing:

| Input                                                             | Result            |
| ----------------------------------------------------------------- | ----------------- |
| unknown or unassigned pixel layout                                 | `InvalidArgument` |
| zero width or zero height                                          | `InvalidArgument` |
| a dimension above `kMaxImageDimension` (16384)                     | `InvalidArgument` |
| pixel count above `kMaxImageBytes` (256 MiB)                       | `InvalidArgument` |
| pixel count no `size_t` on this build can hold                     | `InvalidArgument` |
| payload shorter *or longer* than width x height x bytes per pixel  | `InvalidArgument` |
| exact geometry and exact payload                                   | success           |

The size check is exact in both directions on purpose. A short payload is a
truncated decode; a long one means the bytes handed over are not the pixels that
were described. Accepting "at least" would turn a mismatched header into a
plausible-looking corrupted image instead of an error.

The two ceilings are **policy, not format limits**, and they are fixed rather
than derived from `SIZE_MAX`: this repository builds the client 32-bit, and a
limit that moved with the host would make the same bytes decode on one machine
and fail on another. They are also what makes the refusal of a 65535 x 65535 RGBA
header — about 17 GB — identical on a 32-bit and a 64-bit build. The product is
formed in 64-bit arithmetic and checked twice before it is narrowed, so it cannot
wrap into a value small enough to look sane.

### Decoder contract

```cpp
class IImageDecoder
{
public:
    virtual ~IImageDecoder() = default;

    virtual Result<ImageAsset> DecodeImage(const ResourceData& data) = 0;
};
```

- **Bytes in, one typed value out.** The decoder is handed `ResourceData`, not a
  `ResourceId`, not a path, and not a manager. It therefore behaves identically
  whether those bytes came from a test literal, `MemoryResourceProvider`, a loose
  file under a `FileSystemResourceProvider` root, or a future archive provider.
- **Stateless by construction.** No `Initialize()`, no `Shutdown()`, no
  configuration object, no state machine. A decoder acquires nothing — no device,
  no thread, no cache — so there is no lifetime to get wrong and no partial state
  for a failure to leave behind. A decoder that later needs configuration takes it
  as a constructor argument, and one that needs scratch space owns it as a member;
  neither turns into a lifecycle.
- **Error behaviour.** `ErrorCode::InvalidArgument` for input that cannot be
  decoded: empty, truncated, malformed, unsupported version, or geometry that does
  not match the payload. Nothing is thrown, no new error code was introduced, and
  a failed decode leaves the decoder usable — the next call is unaffected because
  there was never any state to damage. A resource that could not be *found* still
  fails as `NotFound` in the layer below, so the two failure modes stay
  distinguishable.
- **The call is not `const`,** so a decoder that keeps scratch space is not forced
  to declare it `mutable`. The decoder shipped here holds no state at all.

### What the decoder layer deliberately is not

- **Not a cache.** `ResourceManager` caches bytes, and that is the only cache in
  the client resource/asset stack. The manager keeps serving container bytes that
  the decoder has already turned into an image, and a test asserts it: clearing
  the byte cache changes which bytes were served, not what they decode to.
- **Not a provider.** It reads no file, opens no archive and knows no root. The
  same decoder works over any provider, present or future.
- **Not a renderer.** It creates no GPU object, uploads nothing, and calls no
  `IRenderer`. There is no `#include "rendering/..."` in this layer, and no
  `#include "assets/..."` in the rendering layer.
- **Not concurrent.** One call, one synchronous decode: no worker threads, no
  async API, no mutex. Threaded decoding remains a future architecture decision.
- **Not a format registry.** There is no map of extensions to decoders, no
  by-name lookup and no plugin table. One decoder exists; when a second one
  arrives, whoever needs both chooses between them at the call site. Adding a
  registry before there are two entries is inventing a design for a requirement
  nobody has stated.

### The one decoder: MIMG

`TestImageDecoder` implements the container described in its own header — a
ten-byte header (`MIMG` magic, version 1, an `ImageFormat` byte, little-endian
width and height) followed by exactly `width * height * bytes per pixel` of
pixels, with no compression, no palette, no mip chain and no trailing data. Every
field the container claims is checked, and a header describing an image that
cannot be represented is refused before its payload is looked at.

It is named and documented as a **test** decoder because that is what it is: it
was invented for this milestone so the boundary could be implemented, tested and
demonstrated honestly. It decodes no production image format. The suite builds
its containers by hand from that description rather than through a writer from
the implementation, so the reader is checked against an independent encoder.

### Rendering independence

The dependency direction is one-way, and the asset layer is the lower end of it:

```text
Modern Core
     ^
     |
Resources (ResourceData)
     ^
     |
Assets / Decoders (ImageAsset, IImageDecoder)
     ^
     |
future rendering integration
```

- `ModernClientAssets` does not link `ModernClientRendering`, and no header in
  `modern/client/assets` includes one from `modern/client/rendering`.
- `IRenderer` knows nothing about decoders, `ImageAsset` or `IImageDecoder`; the
  only thing that will connect the two is a future adapter that *reads* an
  `ImageAsset` and uploads it, and that adapter belongs to rendering, not here.
- The practical consequence is testable in both directions: every case in this
  milestone runs in a console process with no window, no device and no
  `IRenderer`, and a linker error is what a mistaken dependency produces rather
  than a passing test.
- The separation is also why a decode failure is cheap. Reading and validating a
  resource needs no GPU, so it can happen before the renderer exists, in a tool,
  in a test, or in the background of a loading screen.

### Legacy format exclusion

> CLIENT-007 does not implement or reverse-engineer RAN legacy asset formats.

Explicitly **not** implemented, imported, linked or parsed by this milestone:

| Not done                                              | Why it is out of scope here                                    |
| ----------------------------------------------------- | -------------------------------------------------------------- |
| `.isf`, `.ssf`, `.mnsf`, `.cps` legacy tables          | game data, not images; they belong to future importers          |
| `.x` DirectX meshes                                    | a mesh importer needs a mesh asset type, which does not exist yet |
| `.dds` and every other production texture container    | no decoder was written, and no third-party graphics dependency was added for one |
| `glogic.rcc`, `CryptionRCC`, `CCrypt`, `FileCrypt`, `SFileSystem` | archive and crypt layers; a future `ArchiveResourceProvider`, not a decoder |
| D3DX, `IDirect3DDevice9`, `TextureManager`, `DxMeshTexMan` | renderer-era coupling; excluded by the whole point of the boundary |
| MFC, `<Windows.h>`, Win32 types in any public header    | the layer builds and tests headless                             |

The MIMG decoder refuses anything that is not MIMG, and a case in the suite
feeds it a DDS-shaped header, a legacy-looking binary blob and plain text to
assert exactly that. "We do not decode RAN assets yet" is therefore a property
the build checks, not a promise in a document.

### Future relationship to actual RAN assets

CLIENT-007 establishes the middle and lower part of the pipeline. The upper part
stays out of the modern build entirely:

```text
legacy archive (RCC / .isf / .x / encrypted loose files)
      |
      v
future isolated importer/converter      reads legacy, writes modern bytes
      |                                 (modern/compatibility or modern/tools)
      v
modern resource files (loose, or an archive a provider can read)
      |
      v
IResourceProvider                        Memory / FileSystem / future Archive
      |
      v
ResourceManager
      |
      v
ResourceData                             untyped bytes, cached
      |
      v
IImageDecoder                            DecodeImage()
      |
      v
ImageAsset                               validated CPU-side image
      |
      v
asset upload boundary                    opaque handle (CLIENT-009)
```

What that means in practice:

- A real texture decode is added by writing another `IImageDecoder` — DDS, the
  output of a future legacy importer, anything — and nothing above the interface
  changes. The MIMG decoder stays as the deterministic fixture the boundary is
  tested with.
- The legacy container is never opened by this layer. Conversion happens once,
  offline, in an isolated tool or adapter that is allowed to see `legacy/`; what
  reaches a decoder is already modern bytes. That keeps the exclusion above a
  structural fact rather than a discipline.
- A mesh asset received the same treatment as CLIENT-008 (section 20): a
  validated CPU-side `MeshAsset` next to `ImageAsset`, its own decoder, and the
  same rule that no backend object appears in its header. CLIENT-007 deliberately
  added one asset type rather than a speculative `Asset` variant, because the
  second type is what would show whether the shared parts were drawn in the right
  place.
- `ResourceManager` keeps its single responsibility throughout: it caches and
  serves bytes. Decoding stays next to the caller that needs a typed value, which
  is also where a future cache-eviction or threading policy can be decided
  without the decoder participating.

### Tests and verification

`ClientAssetTests.cpp` carries 31 cases in one new `ModernClientAssetTests`
binary, registered with CTest:

- **Vocabulary:** format names and bytes per pixel, including unassigned byte
  values; the byte-count calculation for every layout, the inclusive ceilings, and
  the refusal of unknown layouts, zero dimensions, oversized geometry and
  arithmetic that would otherwise overflow.
- **Image invariants:** valid geometry accepted; zero width, zero height and both
  refused; unsupported layouts refused; payload size exact in both directions;
  oversized and overflowing images refused; all 256 byte values preserved;
  ownership, copies and the absence of an invalid default state.
- **Decoder:** usable only through `IImageDecoder`; deterministic across repeats
  and across instances; empty data, every truncated header length, damaged magic,
  unsupported versions, unknown format bytes, zero dimensions, every truncated
  payload length and trailing bytes all refused, each with `InvalidArgument`; and
  the pixel bytes of a decoded image compared against an independently built
  container.
- **Integration:** `MemoryResourceProvider` and `FileSystemResourceProvider` each
  driven through `ResourceManager` into a decoded `ImageAsset`; the byte cache
  cleared between decodes; a decode failure leaving the resource layer untouched;
  no RAN installation and no legacy format.

Every filesystem case builds its own root under the system temporary directory
and removes it on destruction, so no case reads a RAN installation and none needs
one. `ModernEmulator` decodes an in-memory sample through the same decoder and
prints the resulting size, layout and byte count; it creates no asset directory
and loads no real asset file.

```powershell
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
build\Debug\ModernClientAssetTests.exe
```

## 20. CLIENT-008: typed mesh asset / decoder boundary

CLIENT-007 closed by asking what a second asset type would show about where the
shared parts were drawn. CLIENT-008 is that experiment: the geometry half of the
asset layer, parallel in shape to the image half rather than folded into it.
Nothing below it changed — `IResourceProvider`, `MemoryResourceProvider`,
`FileSystemResourceProvider`, `ResourceId`, `ResourceData` and `ResourceManager`
are exactly as CLIENT-005 through CLIENT-007 left them — and CLIENT-007's own
files changed only by gaining mesh vocabulary beside the image vocabulary they
already carried.

### Architecture

```text
IResourceProvider
        |
        v
ResourceManager          bytes: cached, untyped
        |
        v
ResourceData
        |
        v
IMeshDecoder             DecodeMesh(ResourceData) -> Result<MeshAsset>
        |
        v
MeshAsset                validated CPU-side mesh: topology, vertices, indices
        |
        v
asset upload boundary     returns an opaque MeshResourceHandle (CLIENT-009); GPU buffers do not exist yet
```

### Why a second type, not a general one

The question CLIENT-007 left open was empirical: one asset type proves nothing
about the boundary, because every decision could have been tailored to images.
A second type, written independently against the same rules, is what shows which
decisions were really shared:

- **Shared, and confirmed shared:** `Result<T>` with `ErrorCode::InvalidArgument`
  as the only failure code; a private constructor behind a
  `static Result<T> Create(...)` factory; a byte-count calculation in
  `AssetTypes` formed in 64-bit arithmetic and refused rather than wrapped; a
  stateless interface over `ResourceData`; a hand-built test container whose
  encoder lives in the tests, not with the reader.
- **Not shared, and confirmed separate:** the layout enums (`ImageFormat` vs
  `MeshVertexFormat`), the geometry rules (payload-length equality vs
  index grouping and index range), the topology question images do not have, and
  the containers themselves.

So the asset layer stays two independently typed siblings. There is no `Asset`
union, no `variant`, no base class and no downcast: `ImageAsset` and `MeshAsset`
share the vocabulary file and the conventions, and nothing forces a consumer
that wants one to know the other exists. A generic `Asset` would have answered
the question in the wrong direction — by making everything know about
everything.

### What it contains

| Unit                                            | Responsibility                                                                     |
| ----------------------------------------------- | ---------------------------------------------------------------------------------- |
| `modern/client/assets/AssetTypes.h, .cpp`       | now also `MeshIndex`, vertex/index byte sizes, mesh ceilings, `MeshVertexFormat`, `PrimitiveTopology`, their names, and the checked mesh byte-count calculation |
| `modern/client/assets/MeshAsset.h, .cpp`        | Validated, immutable CPU-side mesh: topology, vertices, indices                      |
| `modern/client/assets/MeshDecoder.h`            | `IMeshDecoder`: the `ResourceData` -> `MeshAsset` contract                           |
| `modern/client/assets/TestMeshDecoder.h, .cpp`  | The one concrete decoder: the documented MMESH test container                        |
| `modern/client/assets/ClientMeshTests.cpp`      | 32 headless cases: vocabulary, mesh invariants, decoder, integration, image regression |

`ModernClientAssets` gained the two mesh sources and still links `Modern` and
`ModernClientResources` only. `ModernClientMeshAssetTests` is a **second** test
executable — the image suite owns its own `main()` — registered with CTest beside
`ModernClientAssetTests`, with the same no-renderer linkage rules.

### MeshAsset design

```cpp
struct MeshVertex
{
    Vector3 position;   // where it is
    Vector3 normal;     // which way it faces (stored as given, not renormalised)
    float   u, v;       // texture coordinates
    // sizeof == kMeshVertexBytes (32), static_asserted in MeshAsset.h
};

class MeshAsset
{
public:
    static Result<MeshAsset> Create(PrimitiveTopology topology,
                                    std::vector<MeshVertex> vertices,
                                    std::vector<MeshIndex>  indices);

    PrimitiveTopology           GetTopology()    const noexcept;
    size_t                      GetVertexCount() const noexcept;
    size_t                      GetIndexCount()  const noexcept;
    size_t                      GetTriangleCount() const noexcept;
    const std::vector<MeshVertex>& GetVertices() const noexcept;
    const std::vector<MeshIndex>&  GetIndices()  const noexcept;
};
```

- **The layout is declared once.** `kMeshVertexBytes` is 32 (eight float32
  values), and a `static_assert` ties it to `sizeof(MeshVertex)`, so the number
  the container is counted with and the type that reads it cannot drift apart
  without a compile error. Position and normal reuse core's `Vector3` rather
  than growing a second vector type; UV is two floats because CLIENT-008 does
  not extend core — a `Vector2` belongs in core, for everyone, when someone
  needs it there.
- **Normals are stored as given.** Not required to be unit length or to agree
  with the winding: whether a normal needs renormalising is the consumer's
  decision (importer, renderer, tool), and enforcing it here would reject data
  that is valid for every other use. Only finiteness is enforced.
- **No default constructor.** There is no "empty but valid" mesh and no
  half-built one, so a `MeshAsset` is either absent or already checked — which
  is why no case in the suite tests for an invalid state: the state cannot be
  named.
- **Immutable by interface.** Vertices and indices leave through const
  references; to change geometry you build a new `MeshAsset`, which re-runs
  every check.
- **CPU-side only.** No `ID3D11Buffer`, `IDirect3DVertexBuffer9`, `VkBuffer`,
  `GLuint` or device pointer appears in the header, which includes no Windows
  header at all. Whether indices become 16-bit or 32-bit in a GPU buffer, how
  the vertex declaration is laid out and which primitive restart is used are
  questions for a future renderer adapter, and `MeshAsset` never learns that
  any of them were asked.

Validation, all of it returning `ErrorCode::InvalidArgument` and none of it
throwing, in the order it runs:

| Input                                                          | Result            |
| -------------------------------------------------------------- | ----------------- |
| unknown or unassigned topology                                 | `InvalidArgument` |
| zero vertices or zero indices                                  | `InvalidArgument` |
| index count not a whole number of primitives (not `% 3`)       | `InvalidArgument` |
| count above `kMaxMeshVertices` (262144) / `kMaxMeshIndices` (1048576) | `InvalidArgument` |
| total above `kMaxMeshBytes` (8 MiB) or no `size_t` can hold it | `InvalidArgument` |
| any position, normal, u or v component that is NaN or infinite | `InvalidArgument` |
| any index outside `[0, vertexCount)`                           | `InvalidArgument` |
| everything above consistent                                   | success           |

Cheap structural rules run first, per-element rules second, so a mesh that
cannot be valid is never walked — an out-of-range index is checked against the
vertex count *after* the count is known to be sane, and a million-element
finiteness loop never runs on a mesh that already failed on its shape. The
count ceilings are checked while the counts are still `size_t`, before either
is narrowed for the byte calculation, so the arithmetic never sees a size it
could misrepresent. The byte ceiling is the binding one: `kMaxMeshVertices`
vertices alone are exactly 8 MiB, so no index data can fit at the vertex
ceiling — the suite asserts both the refusal at the line and the acceptance one
vertex under it.

### Decoder contract

```cpp
class IMeshDecoder
{
public:
    virtual ~IMeshDecoder() = default;

    virtual Result<MeshAsset> DecodeMesh(const ResourceData& data) = 0;
};
```

Deliberately the same shape as `IImageDecoder`, down to the parts that cost
nothing to keep parallel and that would have been annoying to retrofit later:
bytes in, one typed value out; no manager, no provider, no id; stateless; the
call not `const`; `InvalidArgument` for anything undecodable and `NotFound`
stayed in the layer below, so "these bytes are not a mesh" and "this resource
does not exist" remain distinguishable. The mesh decoder enforces nothing about
images and the image decoder enforces nothing about meshes — each accepts its
own container and refuses the other's, which the suite asserts in both
directions.

### The one decoder: MMESH

`TestMeshDecoder` implements the container described in its own header — a
fourteen-byte header (`MESH` magic, version 1, a `MeshVertexFormat` byte,
little-endian vertex and index counts) followed by exactly
`vertexCount * 32 + indexCount * 4` bytes: vertices as eight little-endian
float32 values each (px, py, pz, nx, ny, nz, u, v), then indices as
little-endian uint32. No compression, no per-vertex stride variation, no index
width field, no topology field, no padding and no trailing data — everything
the header claims is checked, so a truncated, oversized or structurally
impossible payload is refused rather than decoded into a mesh of a different
shape than the header described.

Two deliberate omissions: the container has no topology field because
CLIENT-008 has exactly one topology — the decoder passes
`PrimitiveTopology::TriangleList` explicitly, and a second topology would be a
new field and a version bump — and no index width, because indices are always
uint32. Like MIMG, it is named and documented as a **test** container: invented
for this milestone, not a RAN format, built by hand in the tests from its
description so the reader is checked against an independent encoder.

### Rendering independence

The dependency direction is one-way, and the asset layer is still the lower end
of it — it now carries both asset types:

```text
Modern Core
     ^
     |
Resources (ResourceData)
     ^
     |
Assets / Decoders (ImageAsset, IImageDecoder, MeshAsset, IMeshDecoder)
     ^
     |
future rendering integration
```

- `ModernClientAssets` does not link `ModernClientRendering`, and no header in
  `modern/client/assets` includes one from `modern/client/rendering` — the mesh
  headers added no exception.
- `IRenderer` still knows nothing about decoders, `MeshAsset` or `IMeshDecoder`.
  Buffer creation, index format choice and vertex declaration setup are exactly
  the decisions the future adapter makes *from* a `MeshAsset`; the asset never
  learns which API made them.
- A decode failure therefore stays cheap and testable: validating geometry needs
  no device, so it can happen before the renderer exists, in a tool, in a test
  or in the background of a loading screen — every case in this milestone runs
  in a console process, and a mistaken dependency would produce a compile or
  link error rather than a passing test.

### Legacy format exclusion

> CLIENT-008 does not implement or reverse-engineer RAN legacy mesh formats.

The rule is the one CLIENT-007 set, now with evidence it holds for a second
type: a legacy model is converted once, offline, by an isolated tool or adapter
that is allowed to see `legacy/`, and what reaches a decoder is already modern
bytes. A real mesh format — whatever a future importer writes, or a reader for
a modern container — is added by implementing `IMeshDecoder` beside
`TestMeshDecoder`, and nothing above the interface changes when it arrives. The
MMESH decoder stays what it was declared to be: the deterministic fixture the
boundary is tested with.
#### CLIENT-015: a real RAN decoder at that boundary

The rule above is no longer hypothetical. `modern/client/assets/XMeshDecoder.h`
and `.cpp` implement `IMeshDecoder` for the RAN `xof 0303` `.X` mesh format, and
that is the whole of the new production surface:

| Unit | Responsibility |
| ----------------------------------------------- | ---------------------------------------------------------------------------------- |
| `modern/client/assets/XMeshDecoder.h, .cpp`     | `XMeshDecoder`: RAN `xof 0303` `.X` -> `MeshAsset`                             |
| `modern/client/assets/ClientXMeshTests.cpp`     | 64 headless cases, registered as `ModernClientXMeshTests`                        |

What it adds and what it deliberately does not:

- **Still one boundary.** `ResourceData` in, one validated `MeshAsset` out, via
  `IMeshDecoder`. The interface, `MeshAsset` and `MeshAsset::Create` are
  unchanged, and nothing above the interface moved.
- **No new dependency.** The asset layer still links `Modern` and
  `ModernClientResources` only. The MSZip/raw-DEFLATE reader the `bzip` encoding
  needs is implemented privately in the decoder's translation unit, against
  RFC 1951; the asset layer gains no zlib, no boost and no DirectX.
- **Still not a renderer.** No GPU object, no `IAssetUploader` change, no edge to
  `modern/client/rendering`. CLIENT-015 ends at `MeshAsset`.
- **Still not a general `.X` library.** It reads the three geometry templates it
  needs and walks past the rest. Materials, textures, skinning, bones, morph
  targets, submeshes and animation remain deferred, and `MeshAsset` is still one
  vertex array, one index array, `TriangleList`.
- **The RAN transforms stay separate and in front of it.** `.mxf` becomes plain
  `.X` bytes in `MxfMeshTransform` (CLIENT-013) before the decoder sees them, so
  the decoder never reads a legacy container. The composition is exercised end
  to end on 414 real shipped `.mxf` files.
- **The RAN-specific format knowledge lives in the decoder, not in core.** The
  token grammar, the MSZip chunk layout and the decision to merge multi-`Mesh`
  files are all documented in `XMeshDecoder.h` and in the verification addendum
  in `docs/reference/client/RAN_X_FORMAT_INVESTIGATION.md`, so `MeshAsset` stays
  a format-agnostic value.

### Tests and verification

`ClientMeshTests.cpp` carries 32 cases in the new `ModernClientMeshAssetTests`
binary, registered with CTest beside the image suite:

- **Vocabulary:** `MeshVertexFormat` and `PrimitiveTopology` names, including
  unassigned byte values; bytes per vertex; vertices per primitive as a refusal
  at zero; the byte-count calculation for valid pairs, unknown layouts, zero
  counts, both count ceilings, the binding byte ceiling at the line and one
  vertex under it.
- **Mesh invariants:** a triangle accepted with its geometry compared
  component by component; empty geometry refused; non-triangle index counts
  refused (whole primitives only); out-of-range indices refused across the
  whole `uint32` domain while any in-range winding is accepted; NaN and
  ±infinity refused in each vertex component; unknown topology refused before
  the rules it would have decided; counts and totals above every ceiling
  refused; copies and moves preserving the value.
- **Decoder:** usable only through `IMeshDecoder`; deterministic across repeats
  and across instances; empty data, every truncated header length, each damaged
  magic position, unsupported versions, unknown format bytes, zero counts,
  every truncated payload length, trailing bytes and a lying header all refused;
  ceiling violations refused from a header far too small for its claim, before
  anything is allocated; vertices and indices of a decoded mesh compared against
  an independently built container, byte for byte in value; the topology the
  decoder states rather than one it reads; NaN, infinity and an out-of-range
  index smuggled into an otherwise perfect payload refused at construction.
- **Integration and regression:** the decoder over hand-built bytes with no
  manager anywhere; `MemoryResourceProvider` driven through `ResourceManager`
  into a decoded `MeshAsset`; a decode failure leaving the cache byte-identical;
  no legacy format accepted, with the image decoder refusing mesh bytes in the
  same case; and an `ImageAsset` decoded from a hand-built MIMG container inside
  this suite, while the unchanged 31-case image suite still passes beside it.

`ModernEmulator` gained a "Client typed mesh decoder" section that mirrors the
image one: it assembles a quad in MMESH bytes, decodes it through the interface,
prints topology, triangle count and byte totals, shows a malformed container and
an out-of-range index each refused with a code, and then walks
provider -> `ResourceManager` -> `ResourceData` -> decoder -> `MeshAsset` over
the same manager the image section used. It creates no asset directory and
loads no real asset file.

```powershell
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
build\Debug\ModernClientMeshAssetTests.exe
```


## 22. CLIENT-012: RAN MTF texture transform

The RAN client ships `.mtf` files: a 12-byte header (`version`,
`payloadSize`, `fileType`) followed by an obfuscated payload. This
milestone adds a small transform boundary that strips the container
and decrypts the payload back into plain DDS bytes, which the existing
`DdsImageDecoder` then consumes.

### Why a separate transform

The obfuscation is a byte-level operation, not a decoder: it has no
pixels, no geometry, and no image format awareness. Folding it into
`DdsImageDecoder` would make the decoder understand `.mtf`, which
violates the layered boundary. The transform produces plain DDS bytes
that are indistinguishable from shipped `.dds` files, so the decoder
is unchanged and the transform is independently testable.

### Architecture

```text
ResourceData (MTF bytes)
        |
        v
RAN MTF Transform          validate header, decrypt payload, verify DDS magic
        |
        v
ResourceData (plain DDS)
        |
        v
DdsImageDecoder            unchanged, pure DDS decoder
        |
        v
ImageAsset
        |
        v
IAssetUploader -> ImageResourceHandle
```

### Header format

```text
bytes 0..3   int32 version        (must be 0x100)
bytes 4..7   int32 payloadSize    (must equal inputSize - 12)
bytes 8..11  int32 fileType       (0 = DDS, the only supported type)
bytes 12..   payload bytes        (encrypted with XOR 0x26 and +0x09)
```

### Decryption

For each payload byte:
```cpp
byte += 0x09;
byte ^= 0x26;
```

This is the inverse of the legacy `EncryptTexture()` operation. The
transform uses unsigned byte arithmetic to avoid undefined behaviour.

### Validation

The transform rejects any input that is not a valid MTF DDS container:

- Truncated header (less than 12 bytes)
- Invalid version (anything other than 0x100)
- Payload size mismatch (total size != 12 + payloadSize)
- Unsupported file type (only fileType == 0 is supported; TGA and
  BMP are explicitly rejected)
- Zero-length payload
- Bad decrypted DDS magic (output must start with 'D','D','S',' ')
- Integer overflow in size calculation

The transform is stateless and has no graphics dependencies. It takes
`ResourceData` and returns `ResourceData`, and is usable in a console
process without any renderer, device, or legacy library.

### What is in scope

Only MTF -> DDS is implemented for CLIENT-012. MXF (which maps to
`.x` mesh files) is explicitly out of scope for this milestone.

### Tests

`ClientMtfTests.cpp` registers with CTest as `ModernClientMtfTests`:

- **Valid synthetic MTF -> exact DDS byte recovery**: round-trip
  encrypts a known DDS file into MTF format, transforms it back, and
  compares the recovered bytes against the original.
- **Truncated header**: every length below 12 bytes is refused.
- **Invalid version**: versions 0, 1, and 0xFFFF are all refused.
- **Payload size mismatch**: too small and too large are refused.
- **Truncated payload**: partial payload is refused.
- **Extra trailing bytes**: one extra byte is refused.
- **Zero payload**: refused.
- **Unsupported file types**: TGA (1), BMP (2), and unknown (255)
  are refused.
- **Bad decrypted DDS magic**: corrupting the DDS magic after
  decryption is refused.
- **Valid MTF -> DdsImageDecoder -> ImageAsset**: the full pipeline
  works end to end.
- **Invalid MTF must not reach DDS decoding**: a TGB MTF file is
  refused before it can produce any data.
- **Real RAN assets**: when `RAN_ASSET_ROOT` names a client tree,
  the transform is tested against real `.mtf` files.

```powershell
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
build\Debug\ModernClientMtfTests.exe
```


## 21. CLIENT-009: renderer asset upload boundary

CLIENT-004 drew the renderer side (`IRenderer`, `NullRenderer`) and CLIENT-007 /
CLIENT-008 drew the asset side (`ImageAsset`, `MeshAsset`, the decoder
boundary). Both ends were complete and neither could see the other:
`ImageAsset.h` and `MeshAsset.h` say in their own comments that uploading
belongs to a future renderer adapter, and `IRenderer` says nothing at all
about assets. CLIENT-009 is that adapter's *contract* -- the seam where a
validated CPU asset becomes a renderer resource -- built so the crossing can
be specified, tested and demonstrated without a device.

Nothing below the seam changed. `IRenderer`, `NullRenderer`,
`RenderingTypes.h`, `ImageAsset`, `MeshAsset`, `AssetTypes.h`, both decoders
and the whole resource layer are exactly as their milestones left them. The
asset target's CMake block was not edited at all, and no asset header gained
an include.

### Architecture

```text
ResourceData
      |
      v
Decoder                            IImageDecoder / IMeshDecoder (CLIENT-007, CLIENT-008)
      |
      v
CPU Asset                          ImageAsset / MeshAsset: validated, immutable
      |
      v
Renderer Asset Boundary            IAssetUploader  (CLIENT-009, this section)
      |
      v
Opaque Resource Handle             ImageResourceHandle / MeshResourceHandle
      |
      v
future GPU backend                 builds the real texture / buffer; not this milestone
```

Two independently typed assets enter the same boundary, and each produces its
own handle type. As in section 20, there is no `Asset` union, no `variant` and
no base class: a consumer that wants an image handle is not required to know
that mesh handles exist, and the two id spaces cannot be mixed even by
accident (the types are unrelated and non-convertible, which is
`static_assert`ed in `AssetUpload.h`).

The dependency direction is the whole point of the design:

```text
                     Modern Core
                          ^
              +-----------+-----------+
              |                       |
          Client Assets           Rendering
              |                       |
              +-----------+-----------+
                          |
                   ModernClientAssetUpload
```

`ModernClientAssetUpload` is a separate target precisely so that "rendering
may consume assets" is one link edge in a small adapter instead of a mutual
dependency between two slices. `ModernClientAssets` still links only `Modern`
and `ModernClientResources`; `ModernClientRendering` still links only `Modern`;
neither links the other, and neither links the adapter. The edge points one
way, so a cycle is not merely avoided by convention -- it is unrepresentable.

### Why the renderer receives assets, not ResourceIds

The tempting design is to hand the renderer what it already has: a
`ResourceId`, or the `ResourceData` the manager cached. CLIENT-009 refuses
that, for four reasons that each hold on their own:

- **It would make the renderer a resource loader.** The moment a renderer
  takes a `ResourceId` it needs a `ResourceManager`, a provider and a
  filesystem or archive to answer it. The renderer would grow the loading
  half of the client while the resource layer grew the policy half, and every
  backend would repeat that. The renderer consumes *content*; how bytes
  became that content is a layer below it.
- **The renderer cannot do the work anyway.** An `ImageAsset` is what
  CLIENT-007 made validation produce: width, height, layout and pixels already
  checked. Re-deriving those from `ResourceData` inside a backend would undo
  the boundary and re-implement the decoder's refusal rules per API.
- **It keeps decoding testable without a renderer.** `ImageAsset` /
  `MeshAsset` construction needs no device, so assets can be built and
  verified in a tool, in a test or on a loading thread with no GPU anywhere in
  the process. The upload boundary is the first point that needs a renderer,
  and it needs only a live one.
- **It matches how the layers are already split.** The resource layer owns
  raw `ResourceData` caching, the decoder converts bytes to a CPU asset, and
  the renderer boundary converts a CPU asset to a renderer resource. Three
  responsibilities, three places, no overlap.

Consequently no `ResourceId`, `ResourceData`, `ResourceManager` or
`IResourceProvider` appears anywhere in `AssetUpload.h` or
`NullAssetUploader.h`, and the suite pins the signatures as exact types so
adding one would not compile.

### Handle design

```cpp
using ImageResourceHandle = detail::StrongId<struct ImageResourceHandleTag, uint64_t>;
using MeshResourceHandle  = detail::StrongId<struct MeshResourceHandleTag,  uint64_t>;
```

Both handles reuse the core's existing strongly typed id
(`modern/core/types/Ids.h`) rather than inventing a new scheme: one
`Underlying` member, default-constructed to the all-ones invalid sentinel,
with `IsValid()`, `Get()`, `operator==`, `operator!=` and ordering. That reuse
buys the properties the milestone asks for, and each is asserted in the header
and again in the suite:

- **Cheap to copy** -- 8 bytes, trivially copyable, `static_assert`ed
  against `sizeof(uint64_t)`. Copy and move preserve identity; moving one
  leaves the source still holding its number.
- **Explicit invalid state** -- the default-constructed value *is* the invalid
  state, and `IsValid()` is the only question a caller has to ask. There is no
  "maybe valid" handle.
- **No pointer to a GPU object** -- there is no pointer at all, so no
  `IDirect3DTexture9`, `ID3D11Texture2D`, `ID3D11Buffer`, `VkImage`,
  `VkBuffer`, `GLuint`, `HWND` or device pointer can leak through it. The
  header asserts the types are not convertible to `const void*`, and the
  public headers include no `<Windows.h>` at all.
- **Deterministic identity** -- an uploader assigns ids `1, 2, 3, ...` per
  asset kind, in upload order, from a fresh instance. The same sequence of
  uploads in a new uploader yields the same handles in every run, which is
  what makes an emulator printout comparable to a test assertion. Ids are
  never recycled after a release, so a stale handle can never alias a newer
  resource.
- **Meaningful only to the issuing uploader** -- a fabricated id names nothing
  and is refused, and the invalid sentinel is refused more specifically, so
  "you passed nonsense" and "that resource is not here" stay distinguishable.

The header also states what the handles deliberately are not: there is no
generic `Asset` handle and no public object combining an image and a mesh.
Two handle types, because two resource kinds, is the honest answer.

### The null / headless implementation

`NullAssetUploader` is the "null backend" of uploading, the counterpart of
`NullRenderer`. `UploadImage` reads the asset's metadata and returns a handle;
`UploadMesh` does the same for geometry. It then retains that metadata so a
test can prove the values survived the crossing:

| Kind   | Retained                                                                            |
| ------ | ----------------------------------------------------------------------------------- |
| Image  | `width`, `height`, `format`, `byteCount`                                             |
| Mesh   | `vertexCount`, `indexCount`, `triangleCount`, `topology`, `byteCount`                |

What it does *not* do matters as much:

- It allocates no GPU resource, no device, no context and no OS object. The
  handle names bookkeeping, and a future backend gives it a real object
  behind the same number.
- It does not retain asset bytes. The pixels, the vertices and the indices are
  read during the call and dropped; the registry holds the numbers above and
  nothing else. A second byte cache would be a second `ResourceManager`, and
  the milestone explicitly refuses that.
- It does not load anything. No `ResourceId`, provider or filesystem appears
  in its sources or its headers.
- It does not know which `IRenderer` implementation it observes.

### Lifetime and error conventions

`IAssetUploader` has no `Initialize()` and no `Shutdown()`. A second lifecycle
would be a second state machine that could disagree with the renderer's, so
instead the implementation observes a **borrowed** `IRenderer` (the same
borrowing convention `Application` uses for its renderer) and answers with the
codes `IRenderer` already documents. Every method decides in the same order:

| Situation                                              | Result          |
| ------------------------------------------------------ | --------------- |
| renderer `Uninitialized`, or no renderer attached       | `InvalidState`  |
| renderer `ShutdownState` (terminal)                    | `NotAllowed`    |
| the invalid handle sentinel passed as an argument      | `InvalidArgument` |
| an id that names nothing live (unknown or released)    | `NotFound`      |
| a live handle, release, or a valid upload              | `Ok`            |

Those are the existing core codes, unchanged: `InvalidState` for a live-but-
wrong state, `NotAllowed` from the terminal state, `InvalidArgument` for a
caller-supplied nonsense id ("an invalid id" in the core's own words),
`NotFound` for a lookup that found nothing -- exactly the split
`ResourceManager::Load` uses. No new error code was invented for this
milestone, and none is needed.

Two consequences are worth naming because they are policies rather than
accidents:

- **Lifecycle precedes handle.** A shut-down renderer refuses *everything*
  uniformly with `NotAllowed`, including handles that were live a moment
  earlier. There is no ordering in which a caller can observe a
  post-shutdown `NotFound` and wonder whether the resource still exists.
- **Uploading while `InFrame` is legal.** `IRenderer::IsInitialized()` covers
  `InFrame`, so a texture can be uploaded mid-frame and the frame loop never
  has to end a frame to load one. A resource's lifetime is not tied to a
  frame's.

### Ownership and shutdown policy

Ownership is deliberately dull, because the alternative is a lifetime nobody
has to reason about:

- The **uploader owns only its registry entries**: a handle and the metadata
  recorded for it. Not the asset, not the bytes, not the renderer.
- The **caller keeps owning the asset**. `UploadImage` / `UploadMesh` take a
  `const&`, read what they need and return; the caller may destroy the asset
  immediately afterwards and the handle stays valid.
- The **renderer is borrowed, never owned**. The uploader never initializes it
  and never shuts it down, exactly as `Application` treats its renderer.
- There are **no singletons, globals, threads, locks or async work** anywhere
  in the boundary. Two uploaders are two independent registries, which is why
  the determinism case compares two of them directly.

**Shutdown with handles outstanding** is a defined state, not a leak waiting
to happen, and the policy is stated in the header rather than discovered:

1. The instant the renderer reports `ShutdownState`, **every handle becomes
   invalid**. `IsValidImage` / `IsValidMesh` and the live counts answer from
   the renderer state, so they read `false` and `0` without anything being
   asked of them.
2. The **retained metadata is released at the first mutating call afterwards**,
   which reports `NotAllowed` and drops the entries it held; or, failing that,
   when the uploader is destroyed. Because `RendererState::ShutdownState` is
   terminal under the `IRenderer` contract, there is no state to return to, so
   no id can ever come back.
3. **Destructor is not the contract.** A caller who forgets to release a
   handle before shutdown still gets the right answers on every subsequent
   query, which is what makes teardown order forgiving rather than
   load-bearing.

Double destruction is deterministic rather than harmful: the first release
returns `Ok`, and every later release of the same handle returns `NotFound`.

### Separation from the rest of the client

| Concern                                    | Owner                                    | Must never do                                     |
| ------------------------------------------ | ---------------------------------------- | -------------------------------------------------- |
| raw `ResourceData` caching, providers, ids | `ResourceManager` (CLIENT-005/006)        | know about resource handles, or about assets       |
| bytes -> validated CPU asset               | `IImageDecoder` / `IMeshDecoder`         | allocate, upload, or know a renderer exists         |
| CPU asset -> renderer resource             | `IAssetUploader` (CLIENT-009)             | load, cache bytes, or name a `ResourceId`           |
| renderer lifecycle, frames, clear, resize  | `IRenderer` / `NullRenderer` (CLIENT-004)| know about assets, decoders or handles             |
| actual GPU objects                         | future backend (not this milestone)       | change the contract above it                        |

Both directions of leakage are prevented structurally, not by discipline:

- **Assets do not depend on rendering.** `ImageAsset.h`, `MeshAsset.h`,
  `AssetTypes.h`, `ImageDecoder.h` and `MeshDecoder.h` contain no rendering
  include, and `ModernClientAssets` still links only `Modern` and
  `ModernClientResources`. The asset target's CMake block was not modified by
  this milestone at all.
- **The renderer does not depend on storage.** `IRenderer` and
  `NullRenderer` are unchanged and know nothing about `ResourceManager`,
  `IResourceProvider`, `ResourceId` or `ResourceData`; the uploader's API
  takes assets, never storage types, and the test suite pins the signatures as
  exact types so a storage parameter would not compile.

### What is excluded

- **No graphics backend.** No DirectX 9 / 11 / 12, D3DX, Vulkan or OpenGL; no
  device, swapchain, texture, vertex buffer, index buffer, shader, command
  queue, descriptor heap or pipeline state. The public headers include no
  `<Windows.h>`, no `d3d*.h`, no `vulkan.h` and no `GL/gl.h`, and the test
  binary `#error`s if any of those macros appear on the boundary's own include
  chain (`_WINDOWS_`, `VK_VERSION_1_0`, `__gl_h_`).
- **No legacy RAN import.** No `.isf`, `.ssf`, `.mnsf`, `.cps`, `.x` or
  `.rcc`, no `CryptionRCC`, `CCrypt`, `FileCrypt` or `SFileSystem`, and no
  reference to `legacy/`, `TextureManager` or `DxMeshTexMan`. Nothing from
  `legacy/` is included or linked.
- **No MFC, no Win32, no third-party graphics dependency**, no global mutable
  state, no singleton, no async, no threading, no mutexes.

> CLIENT-009 establishes the renderer asset contract only. It does not
> implement a real graphics backend.

### Where a real backend will land

A GPU backend implements `IAssetUploader` in the same directory as this
milestone put the contract (and, for the renderer itself, under the
`modern/client/rendering/backends/` location section 16 reserved). It keeps
the same handles and the same error table, and only the body of `UploadImage`
/ `UploadMesh` changes: the asset's pixels or vertices are handed to a device,
and the handle is the id of the created texture or buffer. Because the
contract is already fixed and tested headlessly, that backend arrives without
editing any asset, decoder or resource file -- which is precisely what the
failure to compile above is meant to guarantee.

### Tests and verification

`ClientAssetUploadTests.cpp` carries **26 cases** in the new
`ModernClientAssetUploadTests` binary, registered with CTest beside the
existing seven suites:

- **Handles:** the default-constructed handle is the explicit invalid state
  and equals `MakeInvalid()`; equality, inequality and ordering are
  deterministic; the type is trivially copyable, 8 bytes, standard layout and
  never a pointer; copy and move preserve identity; two independent uploaders
  assign the same ids for the same uploads, and ids run `1, 2, 3, ...` per
  kind; multiple uploads receive distinct handles and the live counts agree.
- **Image resource:** metadata preserved (width, height, format, byte count)
  and compared against the source asset; upload before initialization and
  upload with no renderer attached both return `InvalidState` with nothing
  half-created; `SetRenderer` round-trip; upload allowed while `InFrame` and
  still live after `EndFrame`; release works and takes its metadata with it;
  the sentinel is `InvalidArgument` and an unknown id is `NotFound`; double
  release is `Ok`, then `NotFound`, then `NotFound`; shutdown invalidates
  handles immediately, empties the counts, answers `NotAllowed` everywhere and
  drops the registry on the first mutating call; released ids are never
  recycled.
- **Mesh resource:** vertex count, index count, triangle count, topology and
  byte total preserved and checked against the asset (4 / 6 / 2 /
  `TriangleList` / 152); upload before initialization returns `InvalidState`;
  three meshes receive distinct handles and releasing one leaves the others
  live; release, sentinel, unknown and double-release all deterministic;
  shutdown deterministic.
- **Pipelines:** `MemoryResourceProvider` -> `ResourceManager` ->
  `ResourceData` -> `TestImageDecoder` -> `ImageAsset` -> `IAssetUploader` ->
  `ImageResourceHandle`, and the same path through `TestMeshDecoder` and
  `MeshAsset` -> `MeshResourceHandle`. Each also proves the layers are
  independent: shutting the manager down leaves the uploaded resource live,
  and releasing the handle never touched the byte cache.
- **Separation:** the upload signatures are asserted as exact types (asset in,
  handle out), the boundary is an abstract interface with a headless leaf
  implementation, an upload works with no resource-layer object constructed at
  all, and `Get*Info` refusals follow the documented decision order
  (lifecycle, then `InvalidArgument`, then `NotFound`).
- **"An invalid asset cannot be uploaded"** is enforced one step earlier than
  the uploader and stated that way in the suite: `ImageAsset` and `MeshAsset`
  have no default constructor (`static_assert`), so no unchecked value can
  exist, and every `Create()` refusal -- zero dimensions, unknown layout,
  short and long payloads, unknown topology, empty geometry, non-triangle
  index counts, out-of-range indices, a NaN vertex -- is exercised with its
  `InvalidArgument`.

`ModernEmulator` gained a "Client renderer asset boundary" section: it decodes
the same hand-built MIMG and MMESH samples the earlier sections use, refuses an
upload before the renderer exists, initializes the renderer, uploads both
assets, prints the deterministic handles and the retained metadata, releases
both, then shows a handle outstanding across shutdown reading `invalid` and the
next upload returning `NotAllowed`. No GPU, no asset directory, no real asset
file, no permanent file of any kind.

```text
Client renderer asset boundary
  upload before init     InvalidState
  renderer init          None
  image uploaded         handle=1
  image metadata         4x2 R8G8B8A8_UNorm bytes=32
  mesh uploaded          handle=1
  mesh metadata          vertices=4 indices=6 triangles=2 topology=TriangleList
  image released         handle=1 live=0
  mesh released          handle=1 live=0
  upload after shutdown  NotAllowed
  handle after shutdown  invalid
```

```powershell
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
build\Debug\ModernClientAssetUploadTests.exe

## 23. CLIENT-013: RAN MXF mesh transform

The RAN client ships mesh/skin files as `.mxf`: a 12-byte header
(`version`, `payloadSize`, `fileType`) followed by an obfuscated
payload. This milestone adds a small transform boundary that strips
the container and decrypts the payload back into plain DirectX `.x`
bytes, which a future X decoder can consume.

### Why a separate transform

The obfuscation is a byte-level operation, not a decoder: it has no
vertices, no indices, and no mesh format awareness. Folding it into
a future X decoder would make the decoder understand `.mxf`, which
violates the layered boundary. The transform produces plain `.x`
bytes that are indistinguishable from shipped `.x` files, so the
decoder stays unchanged and the transform is independently testable.

### Architecture

```text
ResourceData (MXF bytes)
        |
        v
RAN MXF Transform          validate header, decrypt payload, verify X magic
        |
        v
ResourceData (plain .X)
        |
        v
future X decoder           NOT YET IMPLEMENTED
        |
        v
MeshAsset
        |
        v
IAssetUploader -> MeshResourceHandle
```

### Header format

```text
bytes 0..3   int32 version        (must be 0x100)
bytes 4..7   int32 payloadSize    (must equal inputSize - 12)
bytes 8..11  int32 fileType       (0 = skin, the only supported type)
bytes 12..   payload bytes        (encrypted with XOR 0xEB and +0xEA)
```

### Decryption

For each payload byte:
```cpp
byte += 0xEA;
byte ^= 0xEB;
```

This is the inverse of the legacy `EncryptSkin()` operation.
The transform uses unsigned byte arithmetic to avoid undefined
behaviour.

### Validation

- Truncated header (less than 12 bytes): refused
- Invalid version (not 0x100): refused
- Payload size mismatch (input != 12 + payloadSize): refused
- Unsupported file type (not 0): refused
- Zero payload: refused
- Decrypted output does not start with `xof ` magic: refused
- The transform does not mutate the caller's input buffer

### What is NOT in scope

- `.X` format parsing and decoding belongs to a future milestone
- `D3DXLoadMeshFromX` and all DirectX dependencies remain excluded
- No `DirectX`, `D3DX`, `MFC`, `COM`, or renderer headers are used
- The transform does not construct `MeshAsset` or call `IMeshDecoder`

### Tests

`ClientMxfTests.cpp` registers with CTest as `ModernClientMxfTests`:

- **Valid synthetic MXF -> exact .X byte recovery**: round-trip
  through encryption and decryption produces identical bytes
- **Truncated header, invalid version, payload mismatch, zero
  payload**: all refused with `InvalidArgument`
- **Unsupported file type**: refused
- **Bad decrypted .X magic**: refused
- **Correct byte transformation**: `+0xEA` then `^0xEB` verified
- **Output independence**: mutating the input buffer does not
  affect the transform output
- **Deterministic repeated calls**: stateless and repeatable
- **Real-asset validation**: validates every `.mxf` file in the
  `RAN_ASSET_ROOT` skeleton directory (52 real files), confirming
  each decrypts to valid `xof ` bytes

### Integration

The transform is integrated into the existing asset layer:
`ModernClientAssets` links `MxfMeshTransform.cpp` alongside
`MtfTextureTransform.cpp`. The emulator demonstrates the full
`MXF → X bytes` path. No existing `MeshAsset`, `IMeshDecoder`,
`IAssetUploader`, or renderer code is modified.

### Legacy source of truth

The behavior is derived from `legacy/Lib_Engine/DxCommon/MemoryXFile.cpp`:
- `CMemoryXFile::DecryptSkin()` validates the header and decrypts
  the payload
- `CMemoryXFile::DecryptSkinToTile()` writes the result as `.x`
- The constants `SKIN_VERSION = 0x100`, `SKIN_XOR_DATA = 0x92617EB`,
  and `SKIN_DIFF_DATA = 0x99701EA` define the transform


## 22. CLIENT-011: real DDS image decoder boundary

CLIENT-007 proved the byte-to-asset boundary with MIMG, a container invented
for the milestone. CLIENT-011 replaces the proof with the real thing: RAN
ships **12,732 `.dds` files** in its `textures` tree, the ASURA client hands
those bytes straight to D3DX, and none of that needs Direct3D to *read* it. A
DDS file is a 128-byte header followed by block-compressed or uncompressed
texels, so this milestone moves the decode itself into the modern asset layer
and leaves every Direct3D type outside it.

```text
ResourceData
    ↓
DdsImageDecoder          DecodeImage(ResourceData) -> Result<ImageAsset>
    ↓
ImageAsset               validated CPU-side image
    ↓
IAssetUploader           CLIENT-009, unchanged
    ↓
ImageResourceHandle
```

The decoder is a leaf behind the existing `IImageDecoder`. `IImageDecoder`,
`ImageAsset`, `AssetTypes.h` and `TestImageDecoder` were not modified, and the
asset target still links only `Modern` and `ModernClientResources`.

### Supported formats, chosen from the shipped assets

Scope was decided by reading the headers of all 12,732 shipped `.dds` files,
not by assumption:

| Layout                   | RAN files | Decoded to         | Why it is in                  |
| ------------------------ | --------- | ------------------ | ----------------------------- |
| DXT1 / BC1               | 6,661     | `R8G8B8A8_UNorm`   | 89% of all shipped textures   |
| DXT3 / BC2               | 2,731     | `R8G8B8A8_UNorm`   | explicit 4-bit alpha          |
| DXT5 / BC3               | 1,921     | `R8G8B8A8_UNorm`   | interpolated alpha            |
| uncompressed 32-bit RGBA | 390       | `R8G8B8A8_UNorm`   | a copy, no conversion invented |
| uncompressed 32-bit BGRA | 53        | `B8G8R8A8_UNorm`   | keeps the file's channel order |

That is 92.7% of the shipped set decoded by code that exists, and the rest
refused for stated reasons rather than decoded approximately.

### Refused, and why

| Refused                               | RAN files | Reason                                                      |
| ------------------------------------- | --------- | ----------------------------------------------------------- |
| DXT2, DXT4                            | 126       | premultiplied alpha; un-premultiplying invents precision the format does not store |
| 16-bit RGB565 / 24-bit RGB / RGBA4444  | 806       | widening them is a conversion this milestone does not write  |
| cubemaps                              | 14        | six faces; `ImageAsset` is one 2D image, not the first of six |
| volume textures                       | 6         | a stack of slices, which is not one image                    |
| DX10 extension header                 | 0         | unused by RAN; half-supporting it would misread the layout   |
| anything malformed                    | --        | wrong magic, bad header size, zero or overflowing dimensions, truncated payload, contradictory linear size |

Every refusal is `ErrorCode::InvalidArgument`, returned by value, never thrown
-- the same code and convention the MIMG decoder established.

### Validation order and arithmetic

Identity (magic, `dwSize`, `ddspf.dwSize`) before shape (dimensions, caps2),
before layout (pixel format), before size. Every size is formed in 64-bit
arithmetic and refused rather than wrapped, a header claiming
`0xFFFFFFFF x 0xFFFFFFFF` cannot make a small allocation look correct, and
the existing 16384-per-side and 256 MiB ceilings are applied through
`ComputeImageByteCount` before anything is allocated. `dwPitchOrLinearSize` is
treated as the writer's own claim: zero means "not stated" and is accepted, a
stated value that disagrees with the geometry is refused.

Output is a plain `ImageAsset`: row-major, top row first, four bytes per
texel, and nothing DDS-specific escapes the decoder.

### What this milestone does not do

`.mtf -> DDS` is **not** here. The RAN obfuscation that wraps a DDS file is a
byte transform, not a format, and CLIENT-010 identified that seam and
deferred it deliberately. CLIENT-012 will add the transform; this decoder
consumes ordinary DDS bytes and is what that adapter will feed. No `.mxf`, no
`.x`, no GPU backend, no renderer change, and no Direct3D, D3DX, MFC, Windows
or legacy dependency anywhere in the asset layer.

### Tests and verification

`ClientDdsTests.cpp` carries **22 cases** in a new `ModernClientDdsImageTests`
binary. Every fixture is written by hand from the published format
description and never by calling the decoder, so a decoder checked against
its own output would prove nothing. Compressed formats are verified on
**actual decoded pixels** -- endpoint colours, all four DXT1 palette entries,
the DXT1 three-colour transparent branch, the DXT3 nibble table, and both DXT5
alpha tables -- not merely on an accepted header. The negative suite covers
empty data, every truncation below 128 bytes, wrong magic (including a real
PNG header), bad header and pixel-format sizes, zero and overflowing
dimensions, payload truncation at every length, unsupported FourCCs and
uncompressed layouts, cubemaps, volumes and the DX10 flag.

**Real RAN validation, performed:** with `RAN_ASSET_ROOT` pointing at the
shipped client, the suite read **200 real `.dds` files** -- **178 decoded**
(33,862,704 texels) and **22 refused**, and every refusal was accounted for:
19 files that are PNG under a `.dds` name, 2 cubemaps, 1 truncated file. No
file in a supported layout was refused. That case is env-gated so the suite
stays hermetic in CI, and no proprietary asset is committed here.

`ModernEmulator` gained a "Client real DDS decoder" section that assembles a
4x4 DXT1 file in code, decodes it (first texel `(255, 0, 0, 255)` -- the pure
red endpoint every index points at), uploads the result through the
CLIENT-009 boundary, reads back the retained metadata, releases the handle,
and shows a truncated file refused with `InvalidArgument`.

```powershell
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
build\Debug\ModernClientDdsImageTests.exe
```


**A real finding, not a hypothetical one:** 40 of the 12,732 files are PNG
data under a `.dds` name. The magic check is what refuses them.

### Mip policy

`ImageAsset` holds exactly one 2D image, so this decoder decodes **the top
level only** and ignores the levels after it. This matters because 78% of
shipped textures carry a 9-11 level chain, so trailing data is the normal
case: a file is accepted when it contains the top level and refused only when
it ends before it. The chain is not preserved, and nothing pretends otherwise.

```

---

# CORE-002: RAN Stat System

The first system that computes anything rather than describing it. It
reconstructs RAN's character-stat pipeline and reproduces its arithmetic,
including the truncations, because the truncations are the behaviour.

## Verified source files

| Legacy file | What was taken from it |
| ----------------------------------------------------------------- | ---------------------------------------------------------------------------------- |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp:286`                     | `SUM_ADDITION`, the whole derived-stat pass                               |
| `legacy/Lib_Client/G-Logic/GLogixExPC.cpp:863`                     | `SUM_PASSIVE`, the passive contribution shape                             |
| `legacy/Lib_Client/G-Logic/GLCharDefine.h:373` / `:328`           | `SCHARSTATS` (six `WORD`) and `FCHARSTATS` (the same six as `float`)       |
| `legacy/Lib_Client/G-Logic/GLCharDefine.h:497` / `:452`           | the `SCHARSTATS` arithmetic, and therefore the truncation and wrapping     |
| `legacy/Lib_Client/G-Logic/GLCharDefine.h:235`                     | `EMCHARINDEX`, the sixteen class values                                    |
| `legacy/Lib_Client/G-Logic/GLCharDefine.h:788`                     | `SRESIST::LIMIT`, the resistance floor                                     |
| `legacy/Lib_Client/G-Logic/GLogicData.h:58`                        | `GLCONST_CHARCLASS`, the per-class coefficient table                        |
| `legacy/Lib_Client/G-Logic/GLogicDataLoad.cpp:1169`               | the exact field list parsed out of `default.charclass`                      |
| `legacy/Lib_Client/G-Logic/GLogicData.cpp:252-254`                | the three recovery-rate constants                                           |
| `legacy/Lib_Client/G-Logic/GLogicEx.h:104`                        | `SSUM_ITEM`, the equipment contribution                                     |
| `legacy/Lib_Client/G-Logic/GLCharData.h:1123`                     | `SPASSIVE_SKILL_DATA`, the passive contribution                             |
| `legacy/Lib_Client/G-Logic/GLCharData.h:721-731`                  | the eleven `m_dw*Increase` codex bonuses                                    |
| `legacy/Lib_Engine/G-Logic/GLDefine.h:400`                        | `GLDWDATA`, the pool and the `VAR_PARAM` damage floor                        |
| `legacy/Lib_Engine/Common/GameCharacterCalculations.h` / `.cpp`   | the `VARIATION` clamp, which RAN uses to bound the attack powers            |

`reference/legacy-calculation-port/` was audited against these first. It is
preserved as a research note and is still in no build; see the note above
CORE-002 for what was reused and what was found defective.

## Dependency graph

```
Character facts          ClassConstants (per-class row, loaded from default.charclass)
  class, level     \
  allocated stats    >-- StatCalculationInput
                      |
  ItemContribution -+   (equipment, not implemented: the shape only)
  PassiveContribution -+ (passive skills, not implemented: the shape only)
  CodexContribution  -+ (codex, not implemented: the shape only)
                      |
                      v
              Calculate(input)  -- stateless, no globals
                      |
                      v
                DerivedStats
```

The order is RAN's, not a design choice. `SUM_ADDITION` consumes, in this
sequence: `SUM_PASSIVE` has already been folded into `m_sSUM_PASSIVE`,
`SUM_ITEM` into `m_sSUMITEM`, and the class row is read on demand. The three
contributions are added *at the point of use*, not accumulated into one total
first, which is why there is no stacking rule to reproduce: nothing is summed
twice and nothing needs a priority.

## Base stats

RAN's six, in RAN's own vocabulary. They are not the conventional six, and no
other MMORPG's stat list was consulted:

| Field | RAN | Type | Feeds |
| ---------- | ------ | ------ | ---------------------------------------------------------- |
| `pow`  | íž˜   | `uint16` | melee power, shoot power, attack point |
| `str`  | ì²´ë ¥ | `uint16` | HP |
| `spi`  | ì •ì‹  | `uint16` | MP, magic attack |
| `dex`  | ë¯¼ì²© | `uint16` | hit, avoid, defence, all three attack powers, magic attack |
| `int`  | ì§€ë ¥ | `uint16` | magic attack |
| `sta`  | ê·¼ë ¥ | `uint16` | SP |

All six are 16-bit unsigned, and the stat *sum* is 16-bit unsigned arithmetic
that wraps. That is reproduced rather than widened. A character whose sum
passes 65535 wraps in RAN, so a saturating modern build would silently
disagree with a shipped client on exactly the characters most likely to roll
over.

The per-level growth is `FCHARSTATS`, the same six as `float`, and it is
floating point in the legacy source. The float is part of the arithmetic, not
incidental: RAN multiplies it by `ZBLEVEL` and truncates the product per field,
so a growth rate of `0.5` at level 4 contributes `1`, not `1.5`, and at level
2 contributes `0`.

## Derived stats, and where each comes from

Every field is an output of a verified line in `SUM_ADDITION`. Nothing is
present because it is common in an MMORPG.

| `DerivedStats` field | Legacy source |
| ------------------------------------- | ------------------------------------------------------- |
| `totalStats`                    | `m_sSUMSTATS` (line 306)                     |
| `attackPoint`, `defensePoint`   | `m_wSUM_AP`, `m_wSUM_DP` (309-310)            |
| `meleePower`, `shootPower`, `magicAttack` | `m_wPA`, `m_wSA`, `m_wMA` (313-332), after `VARIATION` |
| `maxHp`, `maxMp`, `maxSp`       | `m_sHP.dwMax` and siblings (342-355)          |
| `hpRecoveryRate` and siblings  | `m_fINCR_HP` and siblings (397-399)          |
| `hit`, `avoid`                  | `m_nHIT`, `m_nAVOID` (365-371)                |
| `defenseBody`, `defense`        | `m_nDEFENSE_BODY`, `m_nDEFENSE` (372, 376)    |
| `physicalDamage`                | `m_gdDAMAGE_PHYSIC` (380-389), after `VAR_PARAM` |
| `resistances`                   | `m_sSUMRESIST` (394), after `SRESIST::LIMIT`  |

Deliberately absent, and why: combat point is a fixed constant with no
contribution source, so there is nothing to calculate; movement speed, attack
speed, critical, pierce, skill ranges, `m_wACCEPTP` and `m_wSUM_DisSP` are
equipment, weapon or animation state, or belong to a future movement or
progression system.

## Modifier sources

Three, because the verified chain has exactly three independent summations
plus the class table. No stacking, priority or ordering rule was found between
them, so none is implemented.

| Source | Legacy | Consumed by |
| ------------------------------------- | ---------------------- | ---------------------------- |
| `ItemContribution`      | `SSUM_ITEM`      | stat sum, all three resources, attack powers, hit, avoid, defence, damage range, resistances, recovery rates |
| `PassiveContribution`   | `m_sSUM_PASSIVE` | all three resources and their rates, attack powers, hit, avoid, defence, damage, resistances, recovery rates |
| `CodexContribution`     | `m_dw*Increase`  | the three resources, attack, the three attack powers, defence, hit, avoid, resistances |
| `ClassConstants`        | `GLCONST_CHARCLASS` | every base value and every coefficient |

The class row is a *value*, not a lookup. RAN's coefficients live in
`default.charclass`, which is a data file and is not in this repository, so
nothing here is hard-coded to RAN's shipped numbers. The arithmetic is what is
reproduced.

## Calculation ordering and rounding

The rules that change answers, each verified against the legacy source and each
pinned by a test:

1. **The level term is `level - 1`**, RAN's `ZBLEVEL`. Zero-based, so level 1
   contributes no growth.
2. **The per-level growth truncates per field.** `WORD(lvlup * ZBLEVEL)` is
   added to a `WORD`, per stat, before the class and character terms are
   combined. Rounding it, or carrying the float, changes results for every
   fractional growth rate.
3. **The stat sum is 16-bit and wraps.** Two `WORD + WORD` additions follow the
   truncated level term. `60000 + 6000` is `464`, not `66000`.
4. **A resource maximum truncates to 32 bits twice.** Once on
   `stat * coefficient + item + passive`, then again after
   `* (1 + rate) * confPointRate`. Folding them into one is wrong: for
   `stat 205, coefficient 0.35, rate 0.5` the answer is `106`, not `107`.
5. **The codex bonus is added last**, after both truncations, as a flat
   `uint32`. It is never diluted by a rate and never truncated away.
6. **Hit and avoid scale by `int(value * (100 + percent) * 0.01f)`.** The
   multiply by `(100 + percent)` happens before the scale, and the grouping is
   preserved.
7. **Attack powers are clamped, not wrapped.** `VARIATION` adds in `int` then
   clamps to `[0, 65535]`, so a large equipment bonus saturates instead of
   rolling the stat over.
8. **The damage range has a floor of 1.** `VAR_PARAM` adds the attack power but
   never lets the result fall below one.
9. **Resistances floor at zero.** `SRESIST::LIMIT` clamps each element.

Where RAN performs a C cast whose value is out of the destination range it is
undefined behaviour, and it does so in several places. Every such cast here
saturates, so a hostile or corrupt input yields a bounded result rather than an
arbitrary one. For every in-range input the result is identical to RAN's.

## The modern API boundary

```cpp
namespace Modern::Stats
{
    Result<DerivedStats> Calculate(const StatCalculationInput& input) noexcept;
}
```

One function. It is stateless, allocates nothing beyond its return value, has
no globals, no I/O, no clock, no randomness, and no legacy header, and it never
throws. The same input always produces the same output. It reports failure by
value, like the rest of the core: `InvalidArgument` for a class index outside
the sixteen, a level outside 1..255, or a non-finite coefficient or
contribution.

`Character` still does not own any of this. It holds the class and the level;
the stat system takes them as an input value and returns the derived set. That
is the line between a character being facts and a character being a `GLChar`.

`modern/core` gained no dependency. It still links nothing, and the four new
files include no Windows header, no DirectX, no MFC, no socket, no database
and no legacy type. The legacy names that appear in them appear only in
comments, as provenance for each rule.

## Tests

`modern/tests/StatCalculationTests.cpp`, registered in the existing headless
`ModernCoreTests` target. It links `Modern` and nothing else, and does not
launch a client, an emulator or a server.

Expected values come from two separate sources:

- **An oracle inside the test file** that re-derives each result from the
  documented RAN expression, sharing no code with the calculator, compared
  across a 255-level sweep, all sixteen classes, every contribution
  combination, a configuration-point-rate sweep and extreme magnitudes. This is
  what catches structural drift.
- **Literal numbers**, for cases whose arithmetic is exact and readable by
  hand. These are the regression fixtures, and two of them exist specifically
  to discriminate the truncations: a fractional level-up rate that truncates to
  zero over several levels, and a resource maximum where the two truncations
  disagree (`106` against `107`).

## Known compatibility limitations

- **No RAN class data ships here.** `default.charclass` is not in the
  repository, so a caller that has loaded the row supplies it. No RAN-shipped
  coefficient is asserted anywhere in the tests, because asserting one would
  require inventing it.
- **The damage branch is the melee one.** RAN picks melee or shoot power from
  the equipped weapon's range; with no equipment the melee branch is used, and
  the ranged branch differs only in that substitution.
- **A rollback-free build is not claimed.** `SCHARSTATS::operator+` in RAN
  returns a reference to a function-local `static`, so its result is shared and
  order-dependent. This implementation returns values, which is well defined
  and produces the same numbers for the chain as RAN writes it, but the legacy
  static is not reproduced and must not be depended on.
- **Saturation replaces undefined behaviour.** Where RAN is undefined, this
  differs by construction. Every in-range input agrees.

## What is deferred, by design

Equipment, item database, passive skills, the codex, combat resolution, damage
calculation, PvP, monsters, quests, network packets, server processes, the
database, rendering, UI and the client status window. The three contribution
types are the *shapes* those systems will fill, not the systems.
---

# VERTICAL-001: Character + Stats Vertical Slice

The first slice that crosses the client/server boundary. It connects CORE-001's
`Character` and CORE-002's `Stats` into one gameplay architecture rather than
two implementations that later have to be reconciled.

    modern/core            shared gameplay contracts (no transport, no I/O)
        |
        +-- modern/server      authority: decides, publishes
        |
        +-- modern/client      presentation: receives, displays

Everything below distinguishes three things explicitly: **verified legacy
behaviour**, **modern design decision**, and **not yet implemented**. Where
modern parts company with RAN, that is said rather than glossed.

## Verified legacy client findings

Read from `legacy/`, not assumed.

**One character class, compiled into both sides.** RAN does not have a client
character and a server character; it has one shared logic class,
`GLCHARLOGIC` (`Lib_Client/G-Logic/GLogicEx.h`, `GLogixExPC.cpp`), with
different derived classes per side: `GLCharacter : GLCHARLOGIC : GLCOPY :
GLGaeaClient` and `GLCharClient : GLCOPY` for the client, `GLChar :
GLCHARLOGIC : GLACTOR` for the server actor, and `GLCharAG : GLCHARAG_DATA` for
the agent.

**Both sides run the stat calculation.** `SUM_ADDITION` and `SUM_PASSIVE` are
members of the shared class, so the client computes maxima locally exactly as
the server does.

**Recalculation is event-driven, not per-frame.** The entry point is
`GLCHARLOGIC::INIT_DATA(bNEW, bReGen, fCONFT_POINT_RATE, bInitNowExp)`
(GLogixExPC.cpp:1233): it resolves the class row from the character's index,
resolves equipped items, then calls `SUM_ADDITION(fCONFT_POINT_RATE)` at line
1254. For a new character it sets `m_bServerStorage = TRUE` and fills
HP/MP/SP (lines 1256-1262). The server reaches it through
`GLChar::SetData` after `SCHARDATA2::Assign` (GLChar.cpp:618-622). The client
reaches it from `GLCharacterMsg.cpp` on equipment changes - slot release
(:714), `NET_MSG_GCTRL_PUTON_UPDATE` (:766), `NET_MSG_GCTRL_PUTON_CHANGE` (:796)
- and on stat resets.

**The point rate travels with the message.** On a put-on change the client
recalculates with `pNetMsg->fCONFT_HP_RATE` (GLCharacterMsg.cpp:796). That
value, plus the fact that both sides run the same formula, is the entire reason
RAN's two calculations agree.

**The client is sent current values, never maxima.** HP, MP and SP arrive as
`m_sHP.dwNow`, `m_sMP.dwNow`, `m_sSP.dwNow` from a skill-consume feedback
(GLCharacterMsg.cpp:1022-1024), and SP separately at :402. No message carries
a maximum. Stats points, skill points, bright and experience arrive as deltas
(:457, :427, :442, :370), and a level-up feedback carries the new level plus
the stat and skill point counts (:839-841).

## Verified legacy server findings

**The persisted record is `SCHARDATA` / `SCHARDATA2`** (GLCharData.h:566 and
:888). The character facts are `m_dwCharID`, `m_szName`, `m_emClass`,
`m_wSchool`, `m_wSex`, `m_wLevel` (:593-606), the allocated `SCHARSTATS
m_sStats` (:621), `m_wStatsPoint` (:622), `GLLLDATA m_sExperience` (:631) and
`m_dwSkillPoint` (:634). `SCHARDATA2` adds equipment, skills, inventory, quests
and storage.

**The server is where the character is created, loaded and saved**, and it is
what sends the client the level, the points and the current pools.

**The derived maxima are not persisted.** They are recomputed by
`INIT_DATA` on load from the persisted facts plus the loaded contributions.
That is the same rule CORE-002 implements, so a stored maximum is never a
source of truth.

## The ownership decision

**Modern design decision, and a deliberate divergence from RAN.**

RAN duplicates the stat calculation: the client runs `SUM_ADDITION` on every
equipment change and displays the result, with the point rate shipped in the
message purely to keep the two runs in step. That works, but it means two
implementations must agree for the client to show anything at all, and the
client's numbers are only as correct as the last message that triggered them.

Modern does not reproduce that. The rules for this milestone are that there is
exactly one authoritative stat-calculation implementation and no independent
client and server versions of RAN's gameplay rules. So:

- the **server** calls `Modern::Stats::Calculate` and owns the result;
- the **client** receives the result in a `CharacterSnapshot` and presents it.

`ClientCharacterState` cannot compute a derived value. It has no stat input, no
class table and no calculator, and the only way to change what it holds is
`Apply(snapshot)`. A test asserts the client's translation unit contains no
reference to `Stats::Calculate`, so authority cannot creep back in without a
test failing.

The one thing lost is the ability to predict a new maximum on the client
between messages. Nothing in this milestone needs that, and a HUD must not
predict one anyway.

## Shared gameplay contract

`modern/core/gameplay/CharacterSnapshot.h` is a value, not a protocol: identity,
class, gender, level, experience, the allocated and summed base stats, the
`DerivedStats` result, the three current resource pools, and the position.

It carries no socket, no buffer, no serialisation format, no database handle,
no renderer object and no Windows type, so core stays free of transport.
Deliberately absent: the class-table row (server data, resolved before the
snapshot is taken), the stat inputs the client has no business seeing
(equipment, passive skills, codex), and anything RAN keeps server-only.

It has a validating factory. A snapshot whose current pool exceeds the
published maximum is refused rather than clamped, so a publisher bug stays
visible instead of rendering as a full bar.

## Character and stats integration

One gap had to be closed. CORE-001 modelled eight classes and deliberately
dropped gender, because nothing in the core needed it. CORE-002 kept RAN's
sixteen-value class table, which pairs class *and* gender, because
`default.charclass` has one row per `EMCHARINDEX`. A character therefore
cannot select its row without a gender, and `m_wSex` is a character fact after
all.

`modern/core/character/CharacterClassTable.h` introduces `CharacterGender` and
maps `(class, gender)` to `Stats::CharClassIndex`, transcribed one-for-one
from the legacy enum. It lives in core because it is a fact about a character,
and because both sides need it: the server to pick a row, and a future save
layer to read a stored index back.

The dependency stays one-way. `Character` owns identity, class and level;
`ServerCharacter` turns those into a `StatCalculationInput` and calls
`Calculate`. Neither `Character` nor `ClientCharacterState` knows about the
other, and the stat system knows about neither.

## Server authority

`modern/server/character/ServerCharacter.h` is the only place in the modern
tree that decides what a character's derived statistics are. It owns the class
and gender, level, experience, allocated stats, the three contribution sets as
values, the configuration point rate, and the current pools.

Every mutator that can change a stat input recalculates before it returns, so
there is no "remember to recalculate" step to forget and a published snapshot
can never disagree with the state behind it. A rejected mutation leaves the
character untouched.

This is the **only** production call to `Modern::Stats::Calculate` in the tree.
The formulas are not restated anywhere.

## Client state

`modern/client/gameplay/ClientCharacterState.h` holds the last authoritative
snapshot. It has no mutators for individual fields, so it cannot hold a mixture
of two snapshots, and a rejected update does not destroy what it already knew.

With no snapshot it reports defined emptiness - an invalid id, an empty name,
`CharacterClass::Unset`, zero maxima, a zero health fraction - rather than a
plausible wrong number. The `Get*Fraction` helpers exist so a future HUD has
something honest to bind to; they divide published values and invent nothing.

## Legacy compatibility boundary

Nothing in `legacy/` was modified, and no modern file includes a legacy header.
`modern/core` gained no dependency: it still links nothing, and the core
targets contain no Windows, DirectX, MFC, socket, database or legacy include.

## Tests

| Suite | Cases | What it covers |
| ------------------------------ | ----- | ------------------------------------------------------------------ |
| `ModernCoreTests`              | 69    | CORE-001 and CORE-002, unchanged                                 |
| `ModernServerTests`            | 16    | authority: creation, every recalculating mutator, snapshot validity, determinism |
| `ModernClientGameplayTests`    | 9     | client presentation, rejection, and the server/client relationship |

The load-bearing case is `Gameplay_ClientNumbersEqualTheOneStatImplementation`:
given identical inputs, the server's numbers, the client-held numbers and a
direct call to `Modern::Stats::Calculate` are all equal. That is the assertion
that fails if a second implementation ever appears.

## Not yet implemented

Transport and serialisation, a session, a login or agent process, persistence,
equipment, items, inventory, skills, passives, the codex, combat, PvP, monsters,
NPCs, quests, maps, movement networking, a HUD, rendering, animation. Each
would feed the boundary that now exists rather than requiring it to be
rebuilt.