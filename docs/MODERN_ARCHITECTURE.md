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
  `core`. They are created when there is code to put in them; the directories do
  not exist yet.
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

### Verifying

```
cmake -S . -B build
cmake --build build --config Debug
.\build\Debug\ModernCoreTests.exe
```

`ModernEmulator` prints the same conventions in use. It is a demonstration, not
the authority.
