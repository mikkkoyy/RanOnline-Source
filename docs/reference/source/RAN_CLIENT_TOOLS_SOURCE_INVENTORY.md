# RAN Client Tools, Source Projects & Executable Inventory (VERTICAL-030-A)

Status: **investigation only.** No production code changed. Run retrospectively against
`6972871` ("V030 modern NET_COMPRESS LZO protocol layer") after the brief's expected
baseline `6b6a72a` was found to be one commit behind reality.

Purpose: answer, from evidence, **where the RAN networking and LZO implementation
actually lives** — and therefore whether V030's LZO sourcing decision was correct.

---

## 0. Why this ran retrospectively

V030-A's brief specified baseline `6b6a72a` and forbade vendoring minilzo or choosing a
vendor strategy before the audit. V030 had already committed both (`6972871`). At the
baseline gate the tree was:

```text
HEAD        6972871  V030 modern NET_COMPRESS LZO protocol layer
             6b6a72a  V029 ASURA client login authority investigation   <- brief's baseline
origin/main 6972871 (in sync, clean)
```

`6b6a72a` is an ancestor of `HEAD`, so this is one commit ahead, not a divergence. The
audit was run with the user's explicit approval as **retrospective validation** of a
shipped decision. Its conclusions are reported without qualification, including where
they contradict V030.

---

## 1. Headline answer

> **Is there already a real LZO1X implementation in the source tree?**

**NO — only headers plus a prebuilt binary library. No implementation source exists.**

| Component | Present? | Detail |
| --- | --- | --- |
| LZO public headers | **YES** | `legacy/Tik/Include/` — full LZO **2.02** header set (Oct 17 2005) |
| LZO implementation source (`src/*.c`) | **NO** | absent from the entire repository |
| `minilzo.h` / `minilzo.c` | **NO** | absent from the entire repository |
| `lzo2.lib` (prebuilt) | **YES** | `legacy/Tik/Library/lzo2.lib`, 186 KB, x86 COFF, complete LZO 2.x, **74 object files built from `src/*.c`** |

The decisive detail: `legacy/Lib_Network/MinLzo.cpp:2` does `#include "minlzo.h"`, and
**no file named `minilzo.h` exists anywhere in the repository**. As committed,
`Lib_Network` cannot compile its own LZO wrapper. This is not unique to this snapshot —
see §11.

### Consequence for V030's decision

V030 rejected `lzo2.lib` on **build-toolchain** grounds (VC7.1 CRT, no source, legacy
dependency). That reasoning holds. This audit adds a stronger, independent fact: **there
was no alternative source in the tree to use instead.** Vendoring LZO source was the only
option, and vendoring the *canonical* miniLZO (verified SHA-1, same `lzo1x_1_compress` /
`lzo1x_decompress_safe` entry points) remains the correct choice.

Two corrections to V030's documentation are required — see §12.

---

## 2. Project inventory

34 projects total: **27 executables, 6 static libraries, 1 DLL**. One solution file,
`legacy/RanOnline.sln`. Project files: 30 original `.vcproj` (VS2003, `Version="7.10"`)
and 31 converted `.vcxproj`; 29 exist in both. The `.vcproj` files were treated as
authority for build settings, per the project's standing rule.

Four projects exist **only** as `.vcxproj` — they postdate the VS2003 project set:

| Project | Type | Output (Debug / Release) |
| --- | --- | --- |
| `BugTrap` | **DynamicLibrary** | `BugTrapD.dll` / `BugTrap.dll` |
| `Lib_Helper` | StaticLibrary | `MfcExD.lib` / `Lib_Helper.lib` |
| `Lib_ZLib` | StaticLibrary | `ZLibD.lib` / `Lib_ZLib.lib` |
| `MinimalItemExporter` | Application | `MinimalItemExporter.exe` |

`Lib_Helper`'s Debug target name `MfcExD` is a rename artifact — the project was
originally `MfcEx`.

### Output EXE names are NOT always the project name

Two projects override `$(ProjectName).exe`, both verified from the project file rather
than assumed:

| Project | Configuration | `OutputFile` | Actual EXE |
| --- | --- | --- | --- |
| **GameClient2** | Debug **and** Release | `$(SolutionDir)_Bin\$(ConfigurationName)\MiniA.exe` | **`MiniA.exe`** |
| **EditGenItem** | Debug | `$(SolutionDir)/_BinD/[Edit]ABF.exe` | **`[Edit]ABF.exe`** |
| **EditGenItem** | Release | `$(SolutionDir)/_Bin/EditGenItem.exe` | `EditGenItem.exe` |

`EditGenItem` produces **two different executable names depending on configuration**, into
two different trees (`_BinD`/`_DBuildData` vs `_Bin`/`_RBuildData`). Any inventory that
assumes one EXE per project is wrong here.

Every other executable project uses `OutputFile="$(ProjectName).exe"`.

---

## 3. Master inventory table

| Application/Tool | Source project | Project file | Output EXE | Main libraries | Network | LZO (linked) | ASURA binary |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Game client | `GameClient2` | `GameClient2/GameClient2.vcproj` | **`MiniA.exe`** | Lib_ClientUI, Lib_Client, Lib_Engine, Lib_Network | **YES** | YES | `MiniA.exe` |
| Game emulator | `GameEmulator` | `GameEmulator/GameEmulator.vcproj` | `GameEmulator.exe` | Lib_Engine, Lib_Network | **no** (injects messages) | YES | `Emulator.exe` |
| 3D viewer | `GameViewer` | `GameViewer/GameViewer.vcproj` | `GameViewer.exe` | Lib_Engine, Lib_Network | **no** | YES | `Editor_Viewer.exe` |
| GM admin | `GMTool` | `GMTool/GMTool.vcproj` | `GMTool.exe` | Lib_Engine, Lib_Network | **no** (ODBC only) | YES | `GM_Tool.exe` |
| Item editor | `EditorItem` | `EditorItem/EditorItem.vcproj` | `EditorItem.exe` | Lib_Engine, Lib_Network | no | YES | `Editor_Item.exe` |
| Text editor | `EditorText` | `EditorText/EditorText.vcproj` | `EditorText.exe` | none of Lib_* | no | listed, unused | `Editor_Text.exe` |
| File crypt | `FileCrypt` | `FileCrypt/FileCrypt.vcproj` | `FileCrypt.exe` | none of Lib_* | no | listed, unused | `Editor_Crypt.exe` |
| RCC cryptor | `CryptionRCC` | `CryptionRCC/CryptionRCC.vcproj` | `CryptionRCC.exe` | none of Lib_* | no | listed, unused | `Editor_RCC.exe` |
| Item gen. tool | `EditGenItem` | `EditGenItem/EditGenItem.vcproj` | **`[Edit]ABF.exe`** (Dbg) / `EditGenItem.exe` (Rel) | none of Lib_* | no | listed, unused | `Editor_GenItem.exe` |
| Item exporter | `MinimalItemExporter` | `MinimalItemExporter/MinimalItemExporter.vcxproj` | `MinimalItemExporter.exe` | — | no | no | **absent** |
| 12 further editors | `EditorActivity`…`EditorTaxi` | respective `.vcproj` | `$(ProjectName).exe` | Lib_Engine, Lib_Network | no | YES | `Editor_*.exe` ×12 |
| Login server | `ServerLogin` | `ServerLogin/ServerLogin.vcproj` | `ServerLogin.exe` | Lib_Network, Lib_Engine | YES (server) | YES | `[4]ServerLogin.exe` |
| Session server | `ServerSession` | `ServerSession/ServerSession.vcproj` | `ServerSession.exe` | Lib_Network, Lib_Engine | YES (server) | YES | `[1]ServerSession.exe` |
| Field server | `ServerField` | `ServerField/ServerField.vcproj` | `ServerField.exe` | Lib_Network, Lib_Engine | YES (server) | YES | `[2]ServerField.exe` |
| Agent server | `ServerAgent` | `ServerAgent/ServerAgent.vcproj` | `ServerAgent.exe` | Lib_Network, Lib_Engine | YES (server) | YES | `[3]ServerAgent.exe` |
| Crash handler | `BugTrap` | `BugTrap/BugTrap.vcxproj` | `BugTrap.dll` | — | no | no | `BugTrap.dll` |
| Launcher | **none** | — | — | — | no | no | `Ran Online Launcher.exe` |

Full editor list (14 `Editor*` projects): Activity, Codex, Item, ItemMix, Level,
MapsList, MobNpc, NpcAction, Quest, Skill, SkinChar, SkinPiece, Taxi, Text.

ASURA ships **18** editor-named binaries; the 4 that do not correspond to an `Editor*`
project name map as: `Editor_Crypt` = FileCrypt, `Editor_GenItem` = EditGenItem,
`Editor_RCC` = CryptionRCC, `Editor_Viewer` = GameViewer.

---

## 4. Library inventory

| Library project | Type | Output | Owns | Consumers | Network? | LZO? |
| --- | --- | --- | --- | --- | --- | --- |
| `Lib_Network` | StaticLibrary | `Lib_Network.lib` | **all sockets, CNetClient, message framing, LZO wrapper** | all 25 EXEs | **YES — exclusively** | wrapper only |
| `Lib_ClientUI` | StaticLibrary | `Lib_ClientUI.lib` | client UI incl. **login page** | GameClient2, editors | calls CNetClient (179 refs) | no |
| `Lib_Client` | StaticLibrary | `Lib_Client.lib` | game logic (`GLCharacter` etc.) | GameClient2, GameEmulator | 9 refs, mostly commented | no |
| `Lib_Engine` | StaticLibrary | `Lib_Engine.lib` | D3D engine, meshes, IO, UI framework | most EXEs | no | no |
| `Lib_Helper` | StaticLibrary | `Lib_Helper.lib` / `MfcExD.lib` | MFC helpers | (bridge, currently disabled) | no | no |
| `Lib_ZLib` | StaticLibrary | `Lib_ZLib.lib` / `ZLibD.lib` | **zlib + minizip source** | (bridge, currently disabled) | no | **no** |
| `BugTrap` | DynamicLibrary | `BugTrap.dll` | crash reporting | GameClient2 | no | no |

### Lib_ZLib (§23) — checked, and it is not LZO

`Lib_ZLib` contains **complete zlib and minizip C source**: `adler32.c`, `compress.c`,
`crc32.c`, `deflate.c`, `gzio.c`, `infback.c`, `inffast.c`, `inflate.c`, `inftrees.c`,
`ioapi.c`, `iowin32.c`, `mztools.c`, `trees.c`, `uncompr.c`, `unzip.c`, `zip.c`,
`zutil.c`, plus 17 headers. **No LZO whatsoever.** The project name was accurate.

Two findings of independent interest:

1. `legacy/Lib_ZLib/CMakeLists.txt` already exists and defines the target properly
   (MSVC `/W0` in Release, `/MP`, `MultiThreaded$<$<CONFIG:Debug>:Debug>` runtime) —
   someone previously prepared zlib for the modern build. **It is currently disabled**:
   the root `CMakeLists.txt:46-56` has all `add_subdirectory(legacy/...)` calls commented
   out behind "To re-enable the bridge, restore the add_subdirectory() calls below".
2. This is **existing repo precedent for vendoring a third-party compression library as
   source**. V030's miniLZO placement follows a pattern already established here, which
   strengthens that decision.

---

## 5. LZO findings

### 5.1 Whole-tree search (§18)

3,580 files scanned across `.c .cpp .h .inl .cxx .cc .vcproj .vcxproj .sln .props .targets .def`.

LZO appears in ~70 files, in exactly two categories:

- **Project files (25 of them)** — all reference `lzo2.lib` in `AdditionalDependencies`
  for both Debug and Release.
- **`legacy/Lib_Network/`** (7 files) — the RAN wrapper: `MinLzo.h`, `MinLzo.cpp`,
  `RcvMsgBuffer.*`, `SendMsgBuffer.*`, `s_CServer.cpp`, `s_NetClient.cpp`, `s_NetGlobal.h`.
- **`legacy/Tik/Include/`** (13 headers) — the LZO 2.02 header set, plus `portab.h`,
  `portab_a.h`, `miniacc.h`.

**No LZO implementation source exists anywhere.** No `.c` file in the tree implements any
`lzo*` function.

### 5.2 lzo1x.h (§20)

`legacy/Tik/Include/lzo1x.h` is the genuine upstream public interface — copyright
Oberhumer 1996-2005, `GNU General Public License, version 2` (note: **GPL-2 exactly**,
not "or later"). Version from `lzoconf.h`:

```c
#define LZO_VERSION        0x2020
#define LZO_VERSION_STRING "2.02"
#define LZO_VERSION_DATE   "Oct 17 2005"
```

The header set is the **full** LZO 2.02 public interface (lzo1.h, lzo1a-f, lzo1x.h,
lzo1y/z.h, lzo2a.h, lzo_asm.h, lzoutil.h) — not a mini subset. As V030 noted, a header
does not prove the implementation is present. It is not.

### 5.3 lzo2.lib (§21)

| Property | Finding |
| --- | --- |
| Format | COFF archive, **x86 (`machine 14C`)** on every member |
| Members | **74 `.obj` files** — `lzo1.obj`, `lzo1x_1.obj`, `lzo1a_*`, `lzo1b_*`, `lzo1c_*`, `lzo1f_*`, `lzo1x_1k/1l/1o`, `lzo1y_*`, `lzo1z_*`, `lzo2a_*`, `lzo_init.obj`, `lzo_crc`, `lzo_ptr`, `lzo_str`, `lzo_util` |
| Embedded source paths | `src/lzo1x_1.c`, `src/lzo_init.c`, … (74 of them) — built from a full LZO source tree |
| Public symbols | `_lzo1x_1_compress`, `_lzo1x_decompress_safe`, `___lzo_init_v2`, `___lzo_init_done` all present |
| Size | 186 KB (identical copies at `Tik\Library\` and `Tik\=Library\`, SHA-256 `292CD066…`) |
| Compiler era | **VC7.1** — proven by V030's link failure (`LNK2019 __except_handler4_common` without `/NODEFAULTLIB:MSVCRT`) and independently corroborated by ASURA shipping `msvcp71.dll`/`msvcr71.dll`/`mfc71.dll` |

This is the **complete LZO 2.x library**, not a mini subset — considerably larger than
the miniLZO now vendored in `modern/network/third_party/minilzo/`, and for the two entry
points RAN actually calls, functionally equivalent.

### 5.4 Consumers (§22, §24) — 25 projects, and it is transitive

Every executable project links `lzo2.lib` directly:

```text
GameClient2, GameEmulator, GameViewer, GMTool,
EditorActivity, EditorCodex, EditorItem, EditorItemMix, EditorLevel,
EditorMapsList, EditorMobNpc, EditorNpcAction, EditorQuest, EditorSkill,
EditorSkinChar, EditorSkinPiece, EditorTaxi, EditorText,
FileCrypt, CryptionRCC, EditGenItem,
ServerLogin, ServerSession, ServerField, ServerAgent
```

**`Lib_Network` itself does NOT link `lzo2.lib`.** It *compiles* `MinLzo.cpp` into
`Lib_Network.lib`, leaving `lzo1x_*` unresolved in the archive. The dependency is
resolved at final link, by whichever EXE consumes the library:

```text
MinLzo.cpp  --compiles into-->  Lib_Network.lib  (lzo1x_* undefined)
                                        |
      +----------------+-----------------+-----------------+
      |                |                                   |
GameClient2.exe   ServerAgent.exe ... 25 EXEs        (each links lzo2.lib)
```

This is a **transitive dependency of essentially the whole RAN tree on `lzo2.lib`**, and
it corrects V030's framing of `lzo2.lib` as a client-only concern.

### 5.5 Listed ≠ used — confirmed against ASURA binaries (§27, §29)

Every ASURA RAN binary was scanned for the TEA key, the `GARBAGE_DATA` token `K9IHANA`
(both live in `Lib_Network` object files), and LZO symbols:

| Binaries | TEA key | GARBAGE_DATA | LZO symbols | Count |
| --- | --- | --- | --- | ---: |
| 4 servers, MiniA, Emulator, GM_Tool, Editor_Viewer, 13 large editors | yes | yes | yes | 22 |
| `Editor_Crypt`, `Editor_GenItem`, `Editor_RCC`, `Editor_Text`, `Ran Online Launcher` | **no** | **no** | **no** | 5 |

The five negatives all link `Lib_Network.lib` and `lzo2.lib` on the command line, yet
contain **none** of the network object code. That is ordinary static-link semantics: a
linker only pulls an archive member if a symbol in it is referenced. `Editor_Text.exe`
is 168 KB against ~9.6 MB for its peers — it simply links almost nothing.

**This is why `Network` and `LZO` columns in §3 are distinguished** ("YES" vs "listed,
unused"), and it is the evidence that no tool except the client and the servers
participates in production networking.

LZO symbols being visible inside the EXEs (rather than only as imports) also confirms
LZO is **statically linked** into RAN binaries — absence of an import would not have
proved its absence, as §27 requires.

---

## 6. Network ownership map (§30)

Every `?` replaced with evidence. All figures are counts of matching references in each
project's own source files.

```text
GameClient2.exe (MiniA.exe)
    └── Lib_ClientUI        179 CNetClient/NetClient-accessor refs
          └── LoginPage.cpp:258  pNetClient->SndLogin(...)   <- the login call site
          └── BasicChatRightBodyEx.cpp:151  pNetClient->SndChatNormal(...)
    └── Lib_Client            9 refs (GLCharacter.cpp; most commented out)
    └── Lib_Network         174 source files — ALL sockets, CNetClient, framing
          ├── CNetClient         7 files
          ├── s_NetClientMsgLogin.cpp   CNetClient::SndLogin + 8 other variants
          ├── MinLzo.cpp / SendMsgBuffer.cpp / RcvMsgBuffer.cpp
          └── (lzo1x_* unresolved here; resolved by the EXE's lzo2.lib)

GameEmulator.exe (Emulator.exe)
    └── Lib_Engine, Lib_Network   0 socket refs, 0 CNetClient refs
    └── GameEmulatorView3D.cpp:46,65,72,82
          casts buffers to NET_MSG_GENERIC* and calls
          DxGlobalStage::GetInstance().MsgProcess(...)
          => INJECTS messages; never opens a socket

GameViewer.exe (Editor_Viewer.exe)
    └── Lib_Engine, Lib_Network   0 network refs — a viewer only

GMTool.exe (GM_Tool.exe)
    └── Lib_Engine, Lib_Network   0 socket refs, 0 CNetClient refs
    └── 6 files include "s_NetGlobal.h" only (GMToolConfig.h, GMToolData.h,
          GMToolGlobal.h, GMToolOdbcBase.h, GMToolOdbcConn.h, DlgLogin.cpp)
          => reuses the header for constants/structs; its "login" is an
             ODBC database login, NOT the game protocol

Editor_Item, Editor_Level, Editor_Quest, Editor_Skill, ... (14 editors)
    └── Lib_Engine, Lib_Network   0 network refs in own sources

Editor_Text, Editor_Crypt, Editor_GenItem, Editor_RCC
    └── no Lib_* linkage that pulls network code; binaries confirm absence

Servers (Login/Session/Field/Agent)
    └── Lib_Network (23 socket-bearing files) — same library, server roles
          1 Login, 2 Session, 3 Field, 4 Agent  (s_NetGlobal.h:82-86)
```

**`Lib_Network` is the sole owner of networking in the RAN tree** — 23 files with socket
references and 7 with `CNetClient`, against zero for every other project.

### One correction to V029

V029 established `CNetClient::SndLogin` at `legacy/Lib_Network/s_NetClientMsgLogin.cpp`
and implied that was the whole login path. That file is the **implementation**. The
**call site** is `legacy/Lib_ClientUI/Interface/LoginPage.cpp:258` — `Lib_ClientUI`, a
different library. A modern client must therefore reproduce both: the `Lib_ClientUI` UI
trigger and the `Lib_Network` wire encoding. WORLD-001 should not assume the two are the
same component.

---

## 7. GameEmulator vs ASURA authority (§13)

`GameEmulator` is a **message-injection harness**: it hands buffers to
`DxGlobalStage::MsgProcess()` and never opens a socket. It is this repository's
**gameplay testing authority** by the standing project rule, and that is exactly what it
is. It is **not** evidence about ASURA's protocol. Its binary carries the same TEA key
and GARBAGE_DATA strings as `MiniA.exe` purely because both link `Lib_Network.lib` — a
linkage fact, not a behavioural one.

The two must stay separate: *Emulator gameplay testing* ≠ *ASURA client protocol
authority*. The latter is V028/V029, derived from `MiniA.exe` and legacy source.

---

## 8. GameViewer (§14)

`GameViewer` links `Lib_Network.lib` but has **zero** network references in its own
sources. It is a rendering viewer (`EditorBar.cpp`, `GameViewer.cpp`, .rc resources) and
does not participate in production networking. Recorded as such.

---

## 9. GMTool (§16)

`GMTool` has **no game network client**. It includes `s_NetGlobal.h` in 6 files purely to
reuse protocol constants and structs. Its `DlgLogin.cpp` implements an **ODBC database
login**, not `NET_MSG_LOGIN`. It contains no `CNetClient`, no sockets, and no
`NET_COMPRESS` logic of its own.

**GMTool's protocol is not the GameClient2 protocol**, and treating its "login" as
evidence about the game login would be a mistake.

---

## 10. FileCrypt / CryptionRCC / EditGenItem (§17)

| Project | Purpose | Lib_* | LZO code in own sources? | ASURA EXE |
| --- | --- | --- | --- | --- |
| `FileCrypt` | file encryption GUI (`FileCryptDlgCrypt.cpp`) | none | **no** | `Editor_Crypt.exe` |
| `CryptionRCC` | RCC asset encryptor (`CryptionRCCDlg.cpp`) | none | **no** | `Editor_RCC.exe` |
| `EditGenItem` | item/ABF generator (`EditGenItemDlg.cpp`) | none | **no** | `Editor_GenItem.exe` |

None contains LZO code. All three list `lzo2.lib` and `Lib_Network.lib` as dependencies
but pull neither into their binaries (§5.5).

---

## 11. Public/forum backread (§33)

Corroboration only; repository source remained authority.

**The missing-LZO problem is a known RAN source defect, not specific to this snapshot.**
An independently distributed RAN EP11 source reports the identical failure:

```text
e:\Source\netclientLib\MinLzo.h(3): fatal error C1083:
    Cannot open include file: 'lzoconf.h': No such file or directory
e:\Source\enginelib\Common\Unzipper.h(20): fatal error C1083:
    Cannot open include file: '../=zlib/unzip.h': No such file or directory
```

(RaGEZONE, "Share EP11 Source Code w/ Client".) `MinLzo.h` cannot find `lzoconf.h`
there, for the same underlying reason it cannot find `minilzo.h` here: the LZO
distribution's include files were not shipped with the source.

Supporting signals from the same corpus:

- **EP8 changelog**: *"Fixed Server Files Crashes (LZO and packet length bugs)"* and an
  added `MinLzoErr.txt` log — RAN's `MinLzo` wrapper has a documented history of runtime
  failures, reinforcing V030's decision to reimplement the wrapper rather than trust it.
- **RaGEZONE GitLab `ran-online/game-sources`** independently shows the same project
  split — `__NetClient`, `__NetServer`, `__RanClient`, `__RanClientUI`, `__ZLib`,
  `__Emulator`, `__BugTrap`, plus `__Agent`/`__Field`/`__Login`/`__Session` servers —
  matching this repository's `Lib_Network` / `Lib_Client` / `Lib_ClientUI` / `Lib_ZLib` /
  `GameEmulator` / `BugTrap` structure.
- **EP7 distribution notes** confirm VS2003 as the native toolchain ("Delete the 2008
  project file and rename the .vcproj.7.10.old into .vcproj … Make sure to use 2003
  Visual Studio"), corroborating the `.vcproj`-as-authority rule used throughout.

Nothing contradicted a repository finding.

---

## 12. Corrections V030 requires

This audit found **two documentation errors** in `6972871`. Neither changes working code;
both should be fixed.

1. **`lzo2.lib` is not a client-only dependency.** V030 cited
   `legacy/GameClient2/GameClient2.vcproj` as evidence, implying a client concern.
   **25 projects** link it — all four servers, `GameEmulator`, `GameViewer`, `GMTool` and
   every editor — and `Lib_Network` reaches it **transitively**. The scope is the whole
   RAN tree.

2. **The rejection rationale is incomplete.** V030 rejected `lzo2.lib` on toolchain
   grounds (VC7.1 CRT, no source, legacy dependency). Correct, but it did not establish
   that **no LZO source exists anywhere in the tree**, nor that `MinLzo.cpp` includes an
   absent `minlzo.h`. Those facts are the stronger argument and are now recorded.

Also worth noting for V030's `VENDOR.md`: the tree's own LZO headers are **GPL-2
exactly**, while the vendored miniLZO 2.10 is **GPL-2-or-later**. The licensing posture
of the alternative V030 rejected is therefore not *more* permissive — the open question
recorded in `VENDOR.md` stands unchanged and is not resolved by this audit.

---

## 13. Unresolved

1. **No LZO source exists in the tree, but RAN's original working copy must have had
   one.** `MinLzo.cpp` cannot compile as committed, and 25 projects link `lzo2.lib`, so
   the RAN developers clearly had a working LZO distribution outside the repository.
   Whether any RAN-derived public repository ships the missing `src/*.c` is unresolved;
   it would be *source*, not binary, and would need its own provenance and licence review
   before use.
2. **`lzo2.lib`'s exact LZO version is unverified.** The tree's headers are 2.02 and the
   member list (including `lzo1x_1k/1l/1o`) is consistent with 2.02+, but the binary
   carries no version string that was extracted. If byte-exact compressed output ever
   matters, this must be pinned against a capture.
3. **`Ran Online Launcher.exe` has no source project.** 16.7 MB, contains `mscoree`
   (.NET), and `LauncherConfig.json` is JSON — it is a non-RAN, likely .NET launcher. Its
   update/patch behaviour is outside this repository's scope.
4. **`MinimalItemExporter` has no ASURA counterpart** and no `.vcproj`, so its intended
   configuration and purpose are undocumented.

---

## 14. Recommendation for V030's codec source (§32 decision)

**Decision: option D — no implementation source exists; canonical miniLZO vendoring is
correct. No change required to `6972871`'s code.**

| Option | Verdict |
| --- | --- |
| A. Existing canonical source in tree | **NO** — disproven. No LZO `.c` anywhere. |
| B. Only `lzo2.lib` exists | **Confirmed true** — and it should remain legacy/reference-only: VC7.1 x86 binary, no auditable source, fails under MSVC 14.x without a build-wide CRT override, and would give `ModernNetwork` the legacy dependency its own header forbids. |
| C. Another source implementation exists | **NO** — `Lib_ZLib` is zlib+minizip, no LZO. `MinLzo.cpp` is a wrapper, not an implementation. |
| D. No implementation source exists | **YES — this is the case.** Vendoring canonical miniLZO was the only option. |

Retrospective verdict: **V030's decision was correct**, and the strongest available
evidence for it — that the tree contains no LZO source at all, and that `Lib_Network`
could not even compile its own wrapper — was not available to V030 at the time and is now
recorded.

The GPL-2.0-or-later question in `modern/network/third_party/minilzo/VENDOR.md` remains
**open** and is a determination for the project owner. This audit does not resolve it;
it only establishes that the rejected alternative is GPL-2 as well.

---

## 15. Evidence index

| Claim | Source |
| --- | --- |
| 34 projects, 27 EXE / 6 lib / 1 DLL | all 30 `.vcproj` + 4 `.vcxproj`-only under `legacy/` |
| project types | `ConfigurationType` in each `.vcproj`; `<ConfigurationType>` in each `.vcxproj` |
| `MiniA.exe` output | `GameClient2/GameClient2.vcproj`, both configurations |
| `[Edit]ABF.exe` / `EditGenItem.exe` | `EditGenItem/EditGenItem.vcproj`, Debug vs Release |
| `Lib_Network` sole network owner | socket/`CNetClient` reference counts across all projects |
| login call site in `Lib_ClientUI` | `Lib_ClientUI/Interface/LoginPage.cpp:258` |
| `CNetClient::SndLogin` implementation | `Lib_Network/s_NetClientMsgLogin.cpp:28` |
| GMTool ODBC-only | `GMTool/{GMToolConfig,GMToolData,GMToolGlobal,GMToolOdbcBase,GMToolOdbcConn,DlgLogin}.cpp` |
| GameEmulator message injection | `GameEmulator/GameEmulatorView3D.cpp:46,65,72,82` |
| Lib_ZLib is zlib+minizip | `legacy/Lib_ZLib/` file listing and `Lib_ZLib.vcxproj` entries |
| Lib_ZLib bridge disabled | root `CMakeLists.txt:46-56` |
| LZO headers 2.02, GPL-2 | `legacy/Tik/Include/lzoconf.h`, `lzo1x.h` |
| absent `minilzo.h` | whole-repo filename search — no match |
| `lzo2.lib` 74 members, x86, `src/*.c` | `lib /list`, `dumpbin /headers`, `/symbols` |
| 25 `lzo2.lib` consumers | `AdditionalDependencies` across all project files |
| LZO transitive via `Lib_Network` | `Lib_Network.vcproj` compiles `MinLzo.cpp` but does not link `lzo2.lib` |
| VC7.1 CRT era | V030 link probe; `msvcp71.dll`/`msvcr71.dll`/`mfc71.dll` in ASURA |
| "listed ≠ used" in binaries | TEA/GARBAGE_DATA/LZO string probe over 26 ASURA EXEs |
| all 26 EXE→project mappings | per-binary string match on project/class names |
| EP11 same missing-`lzoconf.h` failure | RaGEZONE "Share EP11 Source Code w/ Client" |
| EP8 LZO bugfix changelog | RaGEZONE "Complete Ep8 Source Code" |