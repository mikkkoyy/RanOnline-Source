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
│   ├── assets/              Typed CPU-side assets & decoder boundary (CLIENT-007)
│   └── rendering/           Renderer abstraction & headless null backend (CLIENT-004)
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
  (CLIENT-002, CLIENT-003, CLIENT-004, CLIENT-005, CLIENT-006, CLIENT-007); the
  remaining client systems (ui, character, world, audio) and the
  network/database/server layers are created when there is code to put in them.
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
future renderer adapter  uploads an ImageAsset; does not exist yet
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
future renderer adapter                  GPU upload
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
- A mesh asset waits for the same treatment: a validated CPU-side `MeshAsset`
  next to `ImageAsset`, its own decoder, and the same rule that no backend object
  appears in its header. CLIENT-007 deliberately adds one asset type rather than
  a speculative `Asset` variant, because the second type is what will show
  whether the shared parts were drawn in the right place.
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

