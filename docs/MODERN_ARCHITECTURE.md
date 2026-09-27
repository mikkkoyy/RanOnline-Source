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
│   ├── character/           Character, CharacterBaseData, CombatStats,
│   │                        CodexContribution, PassiveSkillData
│   ├── data/                portable text exports of legacy binary tables
│   ├── entity/              Entity
│   ├── item/                ItemData, InstanceCustomContribution
│   ├── math/                Vector3
│   ├── progression/         ProgressionData
│   └── types/               shared primitive types
├── tools/                   offline / research tooling
│   ├── exptable_dump.cpp    reads the packed legacy EXP table -> text
│   └── emulator/            headless ModernEmulator harness
└── compatibility/legacy/    the single sanctioned modern -> legacy bridge
    └── CharacterAdapter.*
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
| `LegacyItemAdapter`     | `SITEM` / `GLItemMan` -> `modern::ItemData`         |
| `LegacyPacketAdapter`   | legacy packet headers -> modern message structs     |
| `LegacyDataImporter`    | legacy binary tables -> `modern/core/data` exports  |

`CharacterAdapter` is implemented; the rest are planned.

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
- `modern/tests/` is created when the first suite lands; it does not exist yet.

## 10. Tooling

- CMake drives the modern tree; the root `CMakeLists.txt` adds `modern/` and
  the legacy libraries needed by the one adapter.
- Target names (`Modern`, `ModernEmulator`, `ModernLegacyAdapter`) were kept
  from the pre-move layout to avoid churn.
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
- it runs under `Emulator.exe` — the only executable allowed to be launched;
- the dependency rules above still hold.
