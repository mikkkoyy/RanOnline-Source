# RanOnline-Source

A RAN (RanOnline) private-server codebase in the middle of a long-term
modernisation effort.

This repository keeps the original code and the new code **separately**. That
separation is the whole point: the new implementation must be able to replace
the old one without being dragged down by it.

| Directory    | Role                                                                          |
| ------------ | ----------------------------------------------------------------------------- |
| `legacy/`    | The original RAN implementation. Reference material only.                    |
| `modern/`    | The new, authoritative RAN implementation. This is what we build.              |
| `reference/` | Research notes, data-format exports and mapping tables. Not built.            |
| `docs/`      | Architecture and process documentation.                                       |

Two rules govern everything in this repository:

1. **Legacy RAN source is reference material.**
   It stays in the repository, unmodernised and buildable, so behaviour can be
   observed and copied. We do not add features to it and we do not fix it.

2. **Modern source is the authoritative implementation.**
   Anything that ships comes from `modern/`. `modern/core/` never includes,
   links, or otherwise depends on legacy code.

The dependency rules, the exception list and the reasoning behind them live in
[docs/MODERN_ARCHITECTURE.md](docs/MODERN_ARCHITECTURE.md).

## Layout

```
.
├── legacy/                    original RAN implementation (reference only)
│   ├── RanOnline.sln          Visual Studio solution (VS2022, v170, Win32)
│   ├── Lib_*/                 shared libraries
│   ├── GameClient2/           client
│   ├── GameEmulator/          headless game logic host
│   ├── Server{Field,Login,Session,Agent}/
│   ├── Editor*/  GMTool/      content and item editors
│   ├── Database/              schema and data
│   ├── CFG/                   configuration
│   ├── res/                   icons and shared resources
│   ├── Tik/                   third-party SDK (boost, tbb, lua, ogg, lzo, ...)
│   └── scripts/               legacy build + research helpers
├── modern/                    new authoritative implementation
│   ├── core/                  game rules and domain models (legacy-free)
│   ├── tools/                 modern CLI/offline tooling
│   └── compatibility/legacy/  the one sanctioned modern -> legacy bridge
├── reference/                 research notes and data-format exports
│   ├── data-formats/          CSV exports and native round-trip validators
│   ├── legacy-mapping/        legacy concept -> modern type mapping
│   ├── protocol/              packet and wire-format notes
│   └── notes/                 audits, dependency notes, one-off scripts
└── docs/                      architecture documentation
```

## Building

### Modern (CMake)

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Targets:

| Target                | What it is                                        |
| --------------------- | ------------------------------------------------- |
| `Modern`              | static lib, `modern/core` — no legacy dependency   |
| `ModernEmulator`      | headless harness linked against `Modern`           |
| `ModernLegacyAdapter` | static lib, `modern/compatibility/legacy`          |
| `exptable_dump`       | research tool that reads the legacy EXP table     |

`exptable_dump` is the single known exception to the no-legacy rule; see
[reference/notes/modern-legacy-dependencies.md](reference/notes/modern-legacy-dependencies.md).

### Legacy (Visual Studio)

Open `legacy/RanOnline.sln`, or use the helpers in `legacy/scripts/`
(`build_libs.bat`, `build_lib_engine.bat`, `build_editoritem_*.bat`).

Toolchain: Visual Studio 2022, platform toolset v170, `Win32`, `Release`.

Legacy output is written to `RanOnline-Build/` at the repository root, because
the projects use `$(SolutionDir)..\RanOnline-Build` and that relationship was
preserved by the move.

## Runtime rule

Only **`Emulator.exe`** may be launched. `MiniA.exe`, the real client, and every
production server process are never started from this working tree.

## Ground rules for new work

- New code goes in `modern/`. Legacy is read, not extended.
- `modern/core/` stays free of Windows UI, DirectX, sockets, PostgreSQL and
  legacy RAN types (`GLChar`, `GLItemMan`, `SITEM`, ...).
- Anything that must understand a legacy format goes behind an adapter in
  `modern/compatibility/legacy/`.
- Research output goes in `reference/`, not in the build.
- Do not commit local scratch data. `ItemTest/` (generated `.isf` fixtures and
  tool logs, ~72 MB) is ignored.
