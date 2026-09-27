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
  `core`. `client/application`, `client/input`, `client/rendering` and
  `client/resources` are the client slices built so far (CLIENT-002, CLIENT-003,
  CLIENT-004, CLIENT-005, CLIENT-006); the remaining client systems (ui,
  character, world, audio) and the network/database/server layers are created
  when there is code to put in them.
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

