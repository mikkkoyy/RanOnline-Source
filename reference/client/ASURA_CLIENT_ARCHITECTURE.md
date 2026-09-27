# ASURA Client Architecture Audit

This document describes the architecture of the existing ASURA client,
inspected from its source (`legacy/`) and its deployed build output
(`D:\FILES\project\RanOnline-Build\ASURA CLIENT`).

The ASURA client is the reference implementation for the planned new
client. This audit records how it currently works so that decisions
about what to reuse, adapt, replace, or isolate can be made before
building the new client.

> The ASURA client is a **reference implementation**, not the
> architecture specification for the new client. See
> `MODERN_CLIENT_MAPPING.md` for the intended mapping.

---

## 1. Application

### Source location

- `legacy\GameClient2\GameClient2.cpp` — application class
- `legacy\GameClient2\GameClient2.h` — application header
- `legacy\GameClient2\GameClient2Wnd.cpp` / `.h` — main window
- `legacy\GameClient2\GameClient2WndD3d.cpp` (139 KB) — D3D environment

### Important classes

| Class | File | Role |
| ----- | ---- | ---- |
| `CGameClient2App` | `GameClient2.cpp` / `.h` | Application, inherits `CWinApp` |
| `CGameClient2Wnd` | `GameClient2Wnd.cpp` / `.h` | Main window, inherits `CWnd` and `CD3DApplication` |
| `DxRendererDX9` / `DxRendererDX11` | `DxRendererDX9.*` / `DxRendererDX11.*` | Renderer implementations |
| `IRenderer` | `IRenderer.h` | Renderer abstraction |
| `DxGlobalStage` | (referenced from `GameClient2Wnd`) | Central game-stage singleton |
| `RANPARAM` | `RANPARAM.h` | Runtime parameters |
| `DXPARAMSET` | `dxparamset.h` | DX parameter initialization |
| `CGameTextMan` | `GameTextMan.*` | Text/locale manager |
| `BugTrap` (`hs_start`/`hs_stop`) | `MinBugTrap.*` | Crash handler |

### Application entry point and main loop

`CGameClient2App` inherits `CWinApp`. `InitInstance()` does the following
in order:

1. Calls `AfxEnableControlContainer()`.
2. Calls `hs_start()` and `hs_start_service()` — initializes BugTrap
   crash handling (title `"MiniA"`).
3. Calls `RANPARAM::LOAD(m_szAppPath)` and `DXPARAMSET::INIT()`.
4. Validates the service-type parameter (region: China, Thailand,
   Malaysia, Philippines, Vietnam, Indonesia, Japan, Korea, Global)
   and requires a launcher-provided account token for some regions.
5. Loads game text via `CGameTextMan::GetInstance().LoadText()` from
   XML/CSV resource files.
6. Creates `CGameClient2Wnd`, calls `Create()`.
7. `Run()` is the Win32 message loop (inherited from `CWinApp`, but
   observed to conditionally call `Render3DEnvironment()` or
   `FrameMove3DEnvironment()` on the inactive path).

Shutdown calls `hs_stop()` and `hs_stop_service()`.

### Window creation

`CGameClient2Wnd::Create()` registers an MFC window class
(`AfxRegisterWndClass`), chooses windowed vs. fullscreen based on
`RANPARAM::bScrWindowed`, sets the size from `RANPARAM::dwScrWidth` /
`dwScrHeight`, then calls `CD3DApplication::Create(m_hWnd, m_hWnd,
AfxGetInstanceHandle())`.

### Message handling

`CGameClient2Wnd` has a standard MFC message map (`ON_WM_KEYDOWN`,
`ON_WM_MOUSEMOVE`, `ON_WM_SIZE`, `ON_WM_ACTIVATEAPP`, `ON_WM_TIMER`,
`ON_WM_GETMINMAXINFO`, `ON_MESSAGE(NET_NOTIFY, OnNetNotify)`, etc.).
`OnNetNotify` forwards to `DxGlobalStage::GetInstance().OnNetNotify`.

### Threading

The client uses a single-threaded message loop with optional MFC UI
threads. `tbb.dll` / `tbbmalloc.dll` are present in the deployed build,
suggesting Intel TBB is linked somewhere (likely for async asset
loading or the launcher). No evidence of a dedicated render thread.
`m_hMutex` is declared but commented out (a former single-instance guard).

### Timers

`CGameClient2Wnd` declares `m_nGGTimer`, `m_nGGATimer`, `m_nGGA12Timer`
(`UINT_PTR`) and handles `ON_WM_TIMER`. `OnTimer` drives periodic
game updates.

### Dependencies

| Dependency | Evidence |
| ---------- | -------- |
| MFC 7.1 (VS 2003) | `CWinApp`, `AfxEnableControlContainer`, `msvcp71.dll`, `mfc71.dll` |
| DirectX 9.0c | `D3DCLEAR_TARGET`, `LPDIRECT3DDEVICEQ`, `getdxversion` checks `0x00090003` |
| DirectX 11 | `DxRendererDX11` present in source |
| Intel IJL | `ijl15.dll` deployed |
| Intel TBB | `tbb.dll` deployed |
| BugTrap | `hs_start`/`hs_stop`, `BugTrap.dll` |
| Win32 SDK | `CreateMutex`, `AdjustWindowRectEx`, `PeekMessage` |
| Legacy server protocol | `s_NetClient`, `RcvMsgBuffer`, `SendMsgBuffer` |

### Recommendation

**REPLACE.** The application framework (MFC 7.1) is tightly coupled to
Win32 and a specific DirectX version. The message loop, window creation,
and renderer abstraction are portable; MFC and the DirectX device
coupling are not. The `IRenderer` abstraction is the one genuinely
reusable piece.

---

## 2. Rendering

### Source location

- `legacy\GameClient2\DxRendererDX9.*` — DirectX 9 renderer
- `legacy\GameClient2\DxRendererDX11.*` — DirectX 11 renderer
- `legacy\GameClient2\IRenderer.h` — renderer abstraction
- `legacy\GameClient2\RendererTypes.h` — `RendererColor`
- `legacy\GameClient2\GameClient2WndD3d.cpp` (139 KB) — main D3D device
  and environment code
- `legacy\Lib_Engine\DxCommon*` — DirectX helper layers (9 and 8)
- `legacy\Lib_Engine\DxEffect\*` — effect rendering (Char, Single,
  EffAni, EffKeep, EffProj)
- `legacy\Lib_Engine\Meshs` — mesh rendering
- `legacy\Lib_Engine\DxOctree` — occlusion culling
- `legacy\Lib_Engine\TextTexture` — text-to-texture rendering
- `legacy\Lib_Engine\dxframe` — frame/skeleton data
- `legacy\Lib_Engine\GUInterface` — UI rendering queue

### Renderer abstraction

`IRenderer` is a small interface with four pure virtual methods:

```cpp
virtual void Clear ( const RendererColor& Color ) = 0;
virtual bool BeginScene () = 0;
virtual void EndScene () = 0;
virtual bool IsValid () const = 0;
```

Both `DxRendererDX9` and `DxRendererDX11` implement it.

### DirectX versions

- **DirectX 9.0c** is the primary runtime (validated by `getdxversion`
  requiring `0x00090003`). `D3DCREATE`, `LPDIRECT3DDEVICEQ`,
  `D3DX` utilities.
- **DirectX 11** renderer source exists (`DxRendererDX11`) using
  `ID3D11Device`, `ID3D11DeviceContext`, `IDXGISwapChain`,
  `ID3D11RenderTargetView`, `D3D11_VIEWPORT`. It is present in the
  codebase but the observed `CGameClient2Wnd::Create()` instantiates
  `DxRendererDX9`. The DX11 renderer appears to be an alternative or
  future backend.

### Shaders

The source tree contains effect files (`DxEffect`) and shader
references (`D3DXShaderCompile`, `D3DCompile`). Effects are loaded
via `DxEffect` classes for characters, single objects, animation
keepers, and projected effects. Look for `.vsh`/`.psh`/`.fx` assets in
the deployed `textures` folder.

### Texture loading

The Intel IJL library (`ijl15.dll`) is used for JPEG decoding.
Textures are loaded from the deployed `textures` folder (13,934 files).
`UITextureList` manages UI textures; `TextTexture` renders text to
textures.

### Meshes, materials, animation

- `Meshs` directory — mesh data and rendering.
- `dxframe` — skeleton/frame animation data.
- `DxEffect\Char` (65 files) — character rendering with equipment.
- `DxEffect\Single` (44 files) — single-object rendering.
- `DxEffect\EffAni` (12) — animation effects.
- `DxEffect\EffKeep` (10) — persistent effects.
- `DxEffect\EffProj` (6) — projected effects.

### Culling and visibility

`DxOctree` (29 files) provides octree-based occlusion culling.

### Text rendering

`TextTexture` renders text into textures for HUD/UI display.
`CGameTextMan` manages the text table loading and lookup.

### Camera and viewport

Camera setup is in `CD3DApplication::SetScreen()` and
`CGameClient2Wnd::Create()`/`ReSizeWindow()`. The viewport is set via
`D3DVIEWPORT9` and `D3D11_VIEWPORT`.

### Render loop

`CGameClient2Wnd` provides virtual `FrameMove()` and `Render()`
(overridden in `GameClient2WndD3d.cpp`), exposed as
`FrameMove3DEnvironment()` and `Render3DEnvironment()`. These are
called from the application message loop. `UIRenderQueue` queues UI
rendering commands.

### Recommendation

**REPLACE.** Both DirectX backends are tightly coupled to COM DirectX
interfaces (`LPDIRECT3DDEVICEQ`, `ID3D11Device`). The rendering pipeline
(`FrameMove`, `Render`, effect rendering) is deeply tied to legacy
data formats. The `IRenderer` abstraction is reusable as a contract, but
the implementation must be rebuilt with a modern renderer.

---

## 3. Resource system

### Source location

- `legacy\Lib_Engine\Hash` — hash tables for resource lookup
- `legacy\Lib_Engine\TextTexture` — texture resource manager
- `legacy\Lib_Engine\Meshs` — mesh resource manager
- `legacy\Lib_Engine\DxSound` — sound resources
- `legacy\Lib_Engine\Utils` — utility loaders
- `legacy\Lib_Engine\G-Logic` — game-data loading
- `legacy\Lib_Engine\Common` — common resource helpers
- `legacy\Lib_ZLib` — compression
- `legacy\CryptionRCC` — RC encryption/decryption
- `legacy\FileCrypt` — file encryption
- `legacy\GameClient2\stdafx.h` — precompiled header with include list

### Archive / package formats

The deployed `data` folder (36,026 files) contains extracted assets.
The source has `CryptionRCC` and `FileCrypt` modules that handle
encrypted RC and binary file formats. `MinLzo` provides LZO compression.
`cFileList.bin` (188 KB) is a client file manifest.

### Caching and lifetime

`UITextureList` manages UI texture lifetime. Resource managers appear
to hold references to loaded assets. The exact cache eviction policy
is not clearly isolated in the inspected headers.

### Configuration loading

- `RANPARAM::LOAD(m_szAppPath)` loads runtime parameters from
  `config.ini` / `option.ini` / `param.ini`.
- `DXPARAMSET::INIT()` initializes DX-specific parameters.
- `LauncherConfig.json` (deployed) is the launcher configuration.

### Recommendation

**REPLACE with ADAPTER REQUIRED.** The file-loading and encryption
infrastructure (`CryptionRCC`, `FileCrypt`, `MinLzo`, `Hash`) can be
isolated as a compatibility adapter. The resource management itself
should be rebuilt. The `cFileList.bin` manifest format is a legacy
artifact.

---

## 4. Input

### Source location

- `legacy\GameClient2\GameClient2Wnd.cpp` — `OnKeyDown`, `OnMouseMove`
- `legacy\GameClient2\GameClient2Wnd.h` — message map
- `legacy\Lib_Engine\GUInterface\UIKeyCheck.cpp` / `.h` — key checking
- `legacy\Lib_Engine\GUInterface\Cursor.cpp` / `.h` — cursor management
- `legacy\Lib_Engine\GUInterface\UIFocusContainer.cpp` / `.h` — focus
- `legacy\Lib_Engine\GUInterface\UIControlMessage.cpp` / `.h` — UI messages
- `legacy\Lib_Engine\GUInterface\UIMessageQueue.cpp` / `.h` — input queue
- `legacy\Lib_Engine\G-Logic` — game input processing

### Keyboard and mouse

`CGameClient2Wnd::OnKeyDown` handles `VK_ESCAPE` and forwards other
keys. `OnMouseMove` updates cursor position and delegates to the
cursor system (`CCursor::GetInstance().SetCursorNow()`). The window is
created with `WS_POPUP` (fullscreen) or `WS_OVERLAPPED` (windowed).

### Key bindings

Key binding tables are referenced through `RANPARAM` parameters and
`UIKeyCheck`. The exact binding table structure lives in
`Lib_Engine\G-Logic`. Bindings appear configurable via the deployed
`config.ini`.

### Camera and movement controls

Camera and movement controls are handled in `GameClient2WndD3d.cpp`
(139 KB) and the `G-Logic` layer. Movement interpolation and camera
follow are in the game stage code (`DxGlobalStage`).

### UI input routing

`UIFocusContainer` manages which UI element has focus. `UIMessageQueue`
queues input messages. `UIControlMessage` defines UI-specific message
types. The UI has a focus chain separate from the game-input path.

### Cursor

`CCursor` manages the cursor. When `RANPARAM::bGameCursor` is set, a
custom cursor is displayed; otherwise the system cursor is used.

### Recommendation

**ADAPTER REQUIRED.** Raw Win32 input can be wrapped in a modern
input abstraction. Key binding tables and cursor state are legacy data
structures that need mapping.

---

## 5. UI

### Source location

- `legacy\Lib_ClientUI` (1,020 files) — the main UI layer
- `legacy\Lib_Engine\GUInterface` (67 files) — low-level UI primitives
- `legacy\Lib_Engine\TextTexture` — text-to-texture rendering
- `legacy\Lib_Engine\GUInterface\GameTextLoader.cpp` — text table loading
- Deployed: `Backup_ModernLogin\Data\GUI\uioutercfg.xml`, `xmllist.ini`

### UI framework

The UI is a **retained-mode retained-window framework** built on top
of MFC:

| Class | File | Role |
| ----- | ---- | ---- |
| `UIMan` | `UIMan.cpp` / `.h` | UI manager (singleton) |
| `UIGroup` / `UIGroupBasic` | `UIGroup*.cpp` / `.h` | UI group containers |
| `UIControl` / `UIControlEx` | `UIControl*.cpp` / `.h` | UI controls |
| `UIControlContainer` | `UIControlContainer.cpp` / `.h` | Control container |
| `UIFocusContainer` | `UIFocusContainer.cpp` / `.h` | Focus management |
| `UIMessageQueue` | `UIMessageQueue.cpp` / `.h` | Input message queue |
| `UIRenderQueue` | `UIRenderQueue.cpp` / `.h` | Render command queue |
| `UIDockingMan` | `UIDockingMan.cpp` / `.h` | Docking manager |
| `UICursor` | `Cursor.cpp` / `.h` | Cursor management |
| `InterfaceCfg` / `InterfaceCfgXml` | `InterfaceCfg*.cpp` / `.h` | UI configuration |
| `UIConfigMgr` | `UIConfigMgr.cpp` / `.h` | UI config manager |
| `GameTextLoader` | `GameTextLoader.cpp` / `.h` | Text table loading |
| `PositionKeeper` | `PositionKeeper.cpp` / `.h` | UI position keeper |
| `NSGUI` | `NSGUI.cpp` / `.h` | NS (NetScreen?) GUI |

### XML and skinning

The UI uses XML for layout and skinning:

- `tinyxml`, `tinystr` (TinyXML) and `ticpp` are vendored XML parsers.
- `RanXML` / `RanXMLParser` provide a thin wrapper.
- `InterfaceCfgXml` parses UI configuration from XML.
- `uioutercfg.xml` (deployed) is the UI outer configuration.
- `xmllist.ini` lists XML resource files.

### Texture atlas

`UITextureList` manages UI textures (texture atlas / batching).
`UITextUtil` handles text rendering into UI textures.

### Specific UI screens

The source tree has editors for each UI-adjacent system:

| Editor | Purpose |
| ------ | ------- |
| `Editor_Item.exe` | Item data editor |
| `Editor_Text.exe` | Text/string table editor |
| `Editor_Skill.exe` | Skill UI/data editor |
| `Editor_Level.exe` | Level/exp editor |
| `Editor_Quest.exe` | Quest editor |
| `Editor_MobNpc.exe` | Monster/NPC editor |
| `Editor_Codex.exe` | Codex/encyclopedia editor |
| `Editor_SkinChar.exe` | Character skin editor |
| `Editor_SkinPiece.exe` | Skin piece editor |
| `Editor_Taxi.exe` | Taxi/route editor |
| `Editor_MapsList.exe` | Map list editor |
| `Editor_Activity.exe` | Activity/event editor |
| `Editor_ItemMix.exe` | Item mixing editor |
| `Editor_NpcAction.exe` | NPC action editor |
| `Editor_Viewer.exe` | Asset viewer |

Specific UI screens (login, character select, inventory, skill, chat,
minimap, settings, loading) are implemented in `Lib_ClientUI` and
reference `UIMan`, `UIGroup`, and `UIControl`.

### Text and localization

`CGameTextMan` loads text from XML/CSV resource files via
`GameTextLoader`. Text tables are managed by `Editor_Text.exe`. The
`textures` folder contains localized text textures.

### Recommendation

**REPLACE.** The UI is a custom MFC-based retained-mode framework with
heavy legacy coupling (MFC window messages, `UIMan` global state,
XML parsing tied to legacy resource paths). The `UIMan`/`UIGroup`/
`UIControl` hierarchy can inform the new UI's component model, but the
implementation must be rebuilt.

---

## 6. World / Character presentation

### Source location

- `legacy\Lib_Client` — gameplay layer (client-side logic)
- `legacy\Lib_Engine\G-Logic` — game logic (`GLChar`, `GLItemMan`,
  `GLMap`, `GLQuest`, `GLSkill`, `GLAttack`, etc.)
- `legacy\Lib_Engine\DxEffect\Char` (65 files) — character rendering
  with equipment
- `legacy\Lib_Engine\DxEffect\Single` (44 files) — single-object
  rendering
- `legacy\Lib_Engine\Meshs` — mesh rendering
- `legacy\Lib_Engine\DxOctree` — occlusion culling
- `legacy\Lib_Engine\dxframe` — skeleton/frame animation
- `legacy\Lib_Network\s_NetClient` — client-side network messages

### Character model creation and equipment

`DxEffect\Char` renders characters with equipment skins. `GLChar` is
the legacy character model, carrying all derived-stat data
(HP/MP/SP, combat stats, contribution aggregates). Equipment appearance
is handled through `GLItemMan` and `SITEM` structures.

### Animation state and movement presentation

`GLChar` stores animation state, movement interpolation targets, and
action state (`ActState` bitfield). Movement interpolation and camera
follow are in `DxGlobalStage`.

### Combat animation and effects

`DxEffect\EffAni` handles animation effects. `DxEffect\EffKeep`
handles persistent effects. `DxEffect\EffProj` handles projected
effects. Combat animations are driven by `GLAttack` and `GLSkill`.

### Name/title display and overhead UI

Name and title display is part of `DxEffect\Char` and the UI layer
(`UIMan`). Overhead UI (health/mana/SP bars, player names) is rendered
through `TextTexture` and the UI render queue.

### Map loading, terrain, objects, NPCs, spawn presentation

`GLMap` handles map loading. Terrain rendering is in `Meshs` and
`DxEffect\Single`. NPCs, monsters, and object presentation are
managed by `GLMap` and the rendering layer. Spawn presentation is
handled by the client-side network messages (`s_NetClientMsg`).

### Visibility

`DxOctree` provides octree-based occlusion culling.

### World streaming and map transitions

Map transitions are handled by the client-side network protocol
(`s_CClientField`). World streaming (if present) is embedded in the
map-loading and object-presentation code.

### Recommendation

**REPLACE.** The entire world and character presentation is tied to
legacy `GLChar`, `GLItemMan`, `GLMap`, `GLQuest`, `GLSkill`, `GLAttack`
structures and derived-stat calculations. These belong to a future
system, not the new client's core. The rendering of characters, effects,
and terrain can inform the new renderer's data requirements, but the
presentation logic must be rebuilt.

---

## 7. Audio

### Source location

- `legacy\Lib_Engine\DxSound` (18 files) — sound rendering
- `deployed ASURA CLIENT\sounds` (864 files in deployed build) — sound assets
- `legacy\Lib_Engine\Common` — common audio helpers

### Music, sound effects, 3D audio

`DxSound` handles music and sound effects. The deployed `sounds`
folder contains 864 sound assets. 3D audio positioning is likely part
of `DxSound` but the exact API is not fully isolated in the inspected
headers. Voice chat is not clearly present as a separate subsystem.

### Audio resource management

Audio resources are loaded through `DxSound` and managed as part of
the engine resource system. The exact lifetime/caching model is not
clearly separated from the general resource system.

### Recommendation

**REPLACE.** Audio is tightly coupled to the legacy DirectX audio API
and the legacy resource loading infrastructure.

---

## 8. Network

### Source location

- `legacy\Lib_Network` (183 files) — network layer
- Client-side: `s_CClientManager`, `s_CClientAgent`, `s_CClientLogin`,
  `s_CClientField`, `s_CClientSession`, `s_CClientLock`, `s_CNetUser`,
  `s_NetClient`, `s_NetClientMsg`, `RcvMsgBuffer`, `SendMsgBuffer`,
  `s_COverlapped`
- Server-side: `s_CServer`, `s_CLoginServer`, `s_CFieldServer`,
  `s_CAgentServer`, `s_CSessionServer`
- Crypto: `DaumGameCrypt`, `DaumGameAuth`, `minTea`, `des`, `dhkey`,
  `MinLzo`, `ApexProxy`, `ApexClient`, `MyRossoEncrypt`
- Anti-cheat: `s_CClientNProtect`, `s_CAgentServerNPROTECT`,
  `s_CNetUserNProtect`, `NSPCID`, `s_CWhiteRock`, `s_CWhiteRockXML`
- Authentication: `DaumGameAuth`, `s_CWeb`, `s_CDbActionWeb`
- ODBC: `s_COdbcGame`, `s_COdbcUser`, `s_COdbcManager`, etc.

### Client-side network

`S_CClientManager` manages the client connection state.
`S_CClientSession` handles the game session. `S_NetClient` is the
client socket layer. `RcvMsgBuffer` / `SendMsgBuffer` are the send/receive
buffers. Messages are dispatched through `s_NetClientMsg*`.

### Packet structures and dispatch

Packets are structured through `SMsgList` (`s_CSMsgList`) and dispatched
via `s_CClientMsg*` / `s_CFieldServerMsg*` / `s_CAgentServerMsg*`.
`ServerControllerMsgDefine.h` defines message IDs. Serialization uses
custom buffer operators.

### Connection manager, login, game connection, session

`S_CClientLogin` handles the login connection. `S_CClientField` handles
the game connection. `S_CClientSession` manages the session. Login
validation uses `DaumGameAuth` and web-based account verification.

### Encryption, compression, reconnect

- `minTea` — TEA encryption
- `des` — DES encryption
- `dhkey` — Diffie-Hellman key exchange
- `MinLzo` — LZO compression
- `DaumGameCrypt` / `DaumGameAuth` — game-specific encryption/
  authentication
- `ApexProxy` / `MyRossoEncrypt` — additional encryption layers
- `MyRossoEncrypt.dll` / `DaumCrypt.DLL` / `EGameEncrypt.dll` —
  deployed DLLs for runtime encryption
- `NProtect` (`NSPCID`, `s_CClientNProtect`) — third-party anti-cheat
  (NProtect GameGuard)

### Reconnect logic

Reconnect logic is in `s_CClientManager` and `s_CClientSession`.
`S_CPatch` handles patching and reconnection.

### ODBC and database

Extensive ODBC usage (`s_COdbcGame`, `s_COdbcUser`, `s_COdbcManager`,
`s_COdbcShop`, `s_COdbcLog`, etc.) connects to a Microsoft SQL Server
database for account, character, inventory, shop, and log data.
`S_CDbAction*` classes implement database actions.

### Recommendation

**ADAPTER REQUIRED (network) + LEGACY ONLY (database/auth).** The
socket layer, packet dispatch, and encryption can be wrapped in a
compatibility adapter. The ODBC/database layer and the NProtect
anti-cheat integration are legacy-only and must not enter the modern
client. Authentication via `DaumGameAuth` and the web-layer is
legacy-only.

---

## 9. Data formats

### Item data

- `D:\FILES\project\RanOnline-Build\ASURA CLIENT\data\glogic\item.isf`
  — item data (ISM/ISF packed format)
- `Editor_Item.exe` — item data editor
- `Editor_ItemMix.exe` — item mixing editor
- `Editor_GenItem.exe` — item generation editor
- `legacy\MinimalItemExporter` / `legacy\EditGenItem` — item export
  tools
- `reference\data-formats\Item.csv`, `RandomOpt.csv` — item data exports

### Text and localization

- `data/glogic` — text/string tables
- `Editor_Text.exe` — text editor
- `LauncherConfig.json` — launcher configuration
- `config.ini`, `option.ini`, `param.ini` — runtime configuration
- `cVer.bin` — client version
- `cFileList.bin` — client file manifest (188 KB)

### Maps and terrain

- `data/glogic` — map data
- `Editor_Level.exe`, `Editor_MapsList.exe` — map editors
- `legacy\Lib_Engine\G-Logic` — map loading code

### Models, textures, animations, effects, sounds

- `textures` (13,934 files) — texture assets
- `sounds` (864 files) — sound assets
- `data/glogic` — model/animation/effect data
- `data/glogic` — skill/quest/npc data

### UI resources

- `Backup_ModernLogin\Data\GUI\uioutercfg.xml` — UI outer configuration
- `Backup_ModernLogin\Data\GUI\xmllist.ini` — XML resource list
- `data/glogic` — UI layout resources

### Recommendation

**ADAPTER REQUIRED.** The `.isf` and other legacy packed formats need
a modern importer/adapter. The CSV/XML exports in `reference/data-formats`
are the right starting point. Configuration files (`config.ini`, etc.)
are legacy artifacts. `cFileList.bin` and `cVer.bin` are deployment
artifacts, not architectural data.

---

## 10. Legacy dependencies

### Critical legacy dependencies

| Legacy concept | Evidence | Impact |
| -------------- | -------- | ------ |
| `GLChar` | `Lib_Engine\G-Logic`, `DxEffect\Char` | Character model carries all derived state |
| `GLItemMan` | `Lib_Engine\G-Logic`, item editors | Item management |
| `SITEM` / `SITEMCUSTOM` | `Lib_Client`, item data | Item structures |
| `GLMap` / `GLQuest` / `GLSkill` / `GLAttack` | `Lib_Engine\G-Logic` | World/quest/skill/attack systems |
| `GLOGICEX` | `Lib_Engine\G-Logic`, `GLogic.h` | Derived-stat calculation engine |
| `ByteCryptDef*.h` | `tools\exptable_dump.cpp` (documented exception) | EXP table decryption |
| `LIB_CLIENT` / `LIB_ENGINE` | `legacy/Lib_Client`, `legacy/Lib_Engine` | Client/engine libraries |
| `Lib_ClientUI` | `legacy/Lib_ClientUI` | UI library |
| ODBC / PostgreSQL | `s_COdbc*`, `s_CDbAction*` | Database access |
| `NProtect` | `s_CClientNProtect`, `NSPCID`, `s_CWhiteRock` | Anti-cheat integration |
| `DaumGameAuth` / `DaumGameCrypt` | `Lib_Network`, deployed DLLs | Authentication/encryption |
| `BugTrap` | `hs_start`/`hs_stop`, `BugTrap.dll` | Crash handling |
| `Hackshield` | `legacy\Hackshield`, deployed folder | Anti-cheat |
| `Tik` SDK | `legacy\Tik` (4,014 files) | Launcher/protection |

### GLOGICEX

`GLOGICEX` is the legacy derived-stat calculation engine. It is the
core of the old RAN character calculation chain. All derived stats
(HP/MP/SP, combat stats, resistances) flow through it. See
`reference/legacy-calculation-port/README.md` for the preserved port.

### Recommendation

**LEGACY ONLY** for the database/ODBC layer, anti-cheat integrations,
and `GLOGICEX`. **ADAPTER REQUIRED** for `GLChar`, `GLItemMan`,
`GLMap`, `GLQuest`, `GLSkill`, `GLAttack`, and the packet/encryption
layers.

---

## 11. External dependencies (deployed build)

| DLL | Purpose | Status |
| ----- | ------- | ------ |
| `msvcp71.dll` / `msvcr71.dll` / `mfc71.dll` | MFC 7.1 (VS 2003) | Must be replaced |
| `D3dx9_26.dll`, `D3dx9_35.dll`, `D3dx9_43.dll` | DirectX 9 D3DX | Must be replaced |
| `D3DCompiler_43.dll` | DirectX 9 shader compiler | Must be replaced |
| `D3dx9d_43.dll` | DirectX 9 debug D3DX | Must be replaced |
| `ijl15.dll` | Intel JPEG Library | Wrap or replace |
| `tbb.dll`, `tbbmalloc.dll`, `tbbmalloc_proxy.dll` | Intel TBB | Wrap or replace |
| `BugTrap.dll` | Crash handler | Wrap or replace |
| `DaumCrypt.DLL`, `EGameEncrypt.dll`, `MyRossoEncrypt.dll` | Runtime encryption | Legacy only |
| `minLzo.dll` (if present) | LZO compression | Wrap or replace |
| `nprotect` files | Anti-cheat | Legacy only |
| `BugTrap.dll` | Crash dump | Wrap or replace |

---

## 12. Summary of classification

### REUSABLE

- `IRenderer` abstraction (4-method interface)
- Win32 message-loop pattern (portable)
- Window creation pattern (portable)
- XML parsing (TinyXML/Ticpp vendored — can be wrapped)
- `UIMan`/`UIGroup`/`UIControl` hierarchy (informs component model)
- `UIMessageQueue`/`UIRenderQueue` pattern
- `RANPARAM` parameter structure (informs config model)
- `CGameTextMan` text-management pattern
- `DxOctree` octree culling concept
- `CryptionRCC`/`FileCrypt`/`MinLzo`/`Hash` file-infrastructure
- `cFileList.bin` manifest format (informs patch model)

### REPLACE

- MFC 7.1 application framework
- DirectX 9 and DirectX 11 device coupling
- `GLChar`, `GLItemMan`, `GLMap`, `GLQuest`, `GLSkill`, `GLAttack`
- `GLOGICEX` derived-stat engine
- `UIMan` global-state UI manager
- `DxSound` audio rendering
- ODBC/database layer
- NProtect anti-cheat integration
- `DaumGameAuth`/`DaumGameCrypt` authentication
- `BugTrap` crash handling
- `Hackshield` anti-cheat
- `Tik` SDK
- All texture/asset loading paths
- `D3DApp`/`CD3DApplication` device management

### ADAPTER REQUIRED

- Socket layer (`S_NetClient`, `RcvMsgBuffer`, `SendMsgBuffer`)
- Packet dispatch (`SMsgList`, `s_CClientMsg*`)
- `minTea`/`des`/`dhkey` encryption
- `IJL` texture decoding
- `TBB` threading
- `.isf` and other packed data formats
- `GLChar` field access (via compatibility adapter)
- `RANPARAM` config loading (wrapped)
- `CGameTextMan` text loading (wrapped)

### LEGACY ONLY

- ODBC / PostgreSQL database access
- `GLOGICEX` calculation engine
- `NProtect` / `Hackshield` / `Tik` anti-cheat and protection
- `DaumGameAuth` / `DaumGameCrypt` authentication
- `BugTrap` crash reporting
- `ByteCryptDef*.h` EXP table decryption
- `cFileList.bin` / `cVer.bin` / `config.ini` deployment artifacts
- `Editor_*.exe` tools (they edit legacy formats)
- `Tik` SDK launcher/protection
