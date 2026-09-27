# Modern Client Mapping

This document maps ASURA client systems to the planned modern
client architecture. The mapping respects the rule that the ASURA
client is a **reference implementation**, not the architecture
specification.

The planned dependency direction is:

```
NEW CLIENT
    ↓
modern/network
    ↓
NEW SERVER
    ↓
modern/core
```

The new client must NOT become dependent on old RAN gameplay rules,
old GLOGICEX calculations, old GLChar, old server assumptions,
old database schema, old packet protocol, old route structure, or
old authoritative/client state design.

---

## Mapping

### Rendering

```
ASURA Renderer
    ↓
modern/client/rendering
```

**Status:** REPLACE

The `IRenderer` abstraction (Clear/BeginScene/EndScene/IsValid) is
retained as a contract. Both the DirectX 9 and DirectX 11 backends
are replaced. The device management (`CD3DApplication`), effect
rendering (`DxEffect\*`), mesh rendering (`Meshs`), octree culling
(`DxOctree`), and text rendering (`TextTexture`) are rebuilt
around a modern graphics API (Vulkan or DX12).

### Application Framework

```
ASURA Application (CGameClient2App, CGameClient2Wnd, MFC)
    ↓
modern/client/application
```

**Status:** REPLACE

The MFC 7.1 application framework, `CWinApp`, `CGameClient2Wnd`,
and `CD3DApplication` are replaced with a platform-agnostic
application framework. The Win32 message-loop pattern is studied
but the MFC and DirectX device coupling are not ported.

### Resource System

```
ASURA Resource System
    ↓
modern/client/resources
```

**Status:** REPLACE with ADAPTER REQUIRED for file infrastructure

The `CryptionRCC`/`FileCrypt`/`MinLzo`/`Hash` file-infrastructure
and the `cFileList.bin` manifest format are wrapped in a
compatibility adapter. The resource management itself (loading,
caching, lifetime) is rebuilt. The `IJL` texture decoding and
`TBB` threading are wrapped or replaced.

### Input

```
ASURA Input (GameClient2Wnd OnKeyDown/OnMouseMove, UIKeyCheck,
Cursor, UIFocusContainer, UIMessageQueue)
    ↓
modern/client/input
```

**Status:** ADAPTER REQUIRED

Raw Win32 input is wrapped in a modern input abstraction. Key
binding tables and cursor state are mapped. The `UIFocusContainer`
focus model informs the new UI's focus system.

### UI

```
ASURA UI (UIMan, UIGroup, UIControl, UIControlEx, UIDockingMan,
UIMessageQueue, UIRenderQueue, InterfaceCfgXml, UITextureList,
GameTextLoader, tinyxml/ticpp)
    ↓
modern/client/ui
```

**Status:** REPLACE

The MFC-based retained-mode UI framework is rebuilt. The
`UIMan`/`UIGroup`/`UIControl` component hierarchy and the
`UIMessageQueue`/`UIRenderQueue` pattern inform the new UI's
architecture, but `UIMan` global state, MFC window messages, and
XML parsing tied to legacy resource paths are not ported. The
XML layout format (from `uioutercfg.xml`) is studied but the
new UI uses a modern layout system.

### Character Presentation

```
ASURA Character Presentation (DxEffect\Char, GLChar, GLItemMan,
SITEM, equipment rendering, animation state, name/title display)
    ↓
modern/client/character
```

**Status:** REBUILD

The character presentation is rebuilt from scratch. The
`GLChar` derived-stat model and `GLItemMan`/`SITEM` equipment
model are replaced. The new client's character communicates
with `modern/core` for identity and lifecycle, and presents
only what the core provides. Equipment appearance and animation
state are driven by the new core's data, not by legacy `GLOGICEX`.

### World

```
ASURA World (GLMap, GLQuest, GLSkill, GLAttack, DxOctree,
Meshs, object/NPC presentation, map transitions)
    ↓
modern/client/world
```

**Status:** REBUILD

The world is rebuilt around the new server's protocol and the
new core's data. `GLMap`, `GLQuest`, `GLSkill`, and `GLAttack`
are not ported. Map loading, terrain rendering, object/NPC
presentation, and map transitions are driven by the new server
and the new core.

### Audio

```
ASURA Audio (DxSound, sounds/)
    ↓
modern/client/audio
```

**Status:** REPLACE

Audio is rebuilt with a modern audio API. The `DxSound` rendering
path and the legacy resource loading are not ported.

### Network

```
ASURA Network (S_NetClient, S_CClientManager, S_CClientSession,
RcvMsgBuffer, SendMsgBuffer, S_CClientMsg*, minTea, des, dhkey,
MinLzo, DaumCrypt, DaumGameAuth, ApexProxy, MyRossoEncrypt)
    ↓
modern/network
```

**Status:** ADAPTER REQUIRED

The socket layer, packet dispatch, and encryption are wrapped
in a compatibility adapter. The ODBC/database layer and
`DaumGameAuth`/`DaumGameCrypt` authentication are **LEGACY ONLY**
and must not enter the modern client. `NProtect`, `Hackshield`,
and `Tik` are legacy-only anti-cheat and protection layers.

The modern client's network layer (`modern/network`) communicates
with the new server (`NEW SERVER → modern/core`).

### Data Formats

```
ASURA Data (item.isf, textures/, sounds/, data/glogic, cFileList.bin,
config.ini, cVer.bin, .isf packed formats)
    ↓
modern/data
```

**Status:** ADAPTER REQUIRED

Legacy packed formats (`.isf`, encrypted RC files) need a modern
importer/adapter. The CSV/XML exports in `reference/data-formats`
are the starting point. `cFileList.bin`, `cVer.bin`, and `config.ini`
are deployment artifacts, not architectural data.

### Game Logic / GLOGICEX

```
ASURA Game Logic (GLOGICEX, GLChar, GLItemMan, GLMap, GLQuest,
GLSkill, GLAttack, derived stats, contributions)
    ↓
NOT PORTED
```

**Status:** LEGACY ONLY

The entire derived-stat calculation chain, the `GLChar` model,
`GLItemMan`, `GLMap`, `GLQuest`, `GLSkill`, and `GLAttack`
systems are **LEGACY ONLY**. They do not enter the modern client.
The new client's gameplay is driven by `modern/core` and the new
server. See `reference/legacy-calculation-port/README.md` for the
preserved port of these formulas.

### Editors

```
ASURA Editors (Editor_Item, Editor_Text, Editor_Skill, Editor_Level,
Editor_Quest, Editor_MobNpc, Editor_NpcAction, Editor_MapsList,
Editor_Codex, Editor_SkinChar, Editor_SkinPiece, Editor_Taxi,
Editor_Activity, Editor_ItemMix, Editor_Viewer, Editor_GenItem)
    → modern/data-importers
```

**Status:** ADAPTER REQUIRED (data importers)

The editors edit legacy data formats. Modern importers (not
editors) will be needed to convert legacy data into the modern
data formats. The editor binaries and source remain untouched as
reference.

---

## Layered mapping summary

```
ASURA Client
│
├── Application (MFC CWinApp, CGameClient2Wnd)
│   └─→ modern/client/application          REPLACE
│
├── Rendering (DxRendererDX9/DX11, D3DApp, DxEffect, Meshs)
│   └─→ modern/client/rendering             REPLACE
│
├── Resources (CryptionRCC, FileCrypt, MinLzo, Hash, IJL, TBB)
│   └─→ modern/client/resources             REPLACE + ADAPTER
│
├── Input (Win32 OnKeyDown/OnMouseMove, UIKeyCheck, Cursor)
│   └─→ modern/client/input                 ADAPTER
│
├── UI (UIMan, UIGroup, UIControl, InterfaceCfgXml)
│   └─→ modern/client/ui                    REPLACE
│
├── Character Presentation (GLChar, SITEM, DxEffect\Char)
│   └─→ modern/client/character             REBUILD (not ASURA-derived)
│
├── World (GLMap, GLQuest, GLSkill, GLAttack)
│   └─→ modern/client/world                 REBUILD
│
├── Audio (DxSound)
│   └─→ modern/client/audio                 REPLACE
│
├── Network (S_NetClient, packet dispatch, encryption)
│   └─→ modern/network                      ADAPTER
│
├── Data (item.isf, textures, sounds, config)
│   └─→ modern/data                         ADAPTER
│
├── Game Logic (GLOGICEX, GLChar derived stats)
│   └─→ NOT PORTED                          LEGACY ONLY
│
├── Database / Auth (ODBC, DaumGameAuth)
│   └─→ NOT PORTED                          LEGACY ONLY
│
└── Protection (NProtect, Hackshield, Tik)
    └─→ NOT PORTED                          LEGACY ONLY
```

---

## Communication protocol

The modern client communicates with the new server through a
protocol defined by `modern/network`. The ASURA client's packet
structures (`SMsgList`, `s_CClientMsg*`) are studied to understand
what data the server needs to send, but the protocol itself is
designed by the new server and the new client together, based on
`modern/core` data.

```text
NEW CLIENT
    ├── modern/client/application
    ├── modern/client/rendering
    ├── modern/client/resources
    ├── modern/client/input
    ├── modern/client/ui
    ├── modern/client/character
    ├── modern/client/world
    ├── modern/client/audio
    ├── modern/network  ←→  NEW SERVER  ←→  modern/core
    └── modern/data
```
