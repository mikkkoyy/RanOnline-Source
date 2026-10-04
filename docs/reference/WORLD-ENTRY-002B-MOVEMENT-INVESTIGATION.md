# WORLD-ENTRY-002b investigation: authoritative position movement (GOTO / NaviMesh)

**Status: INVESTIGATION COMPLETE — IMPLEMENTATION BLOCKED.**

Baseline `735c5aa` (WORLD-ENTRY-002a). No production movement code was written, no
002a behaviour was touched, and no existing test was weakened.

---

## 0. Headline: the WORLD-ENTRY-002 blocker is RESOLVED

The 002 investigation stopped on four findings, one of which was decisive:

> "The map data is not in this repository… A navigation mesh with no mesh data cannot
> validate a single step, so the numbers above are a floor, not an estimate."

**That is no longer true, and finding the data was the point of this milestone.**

The navigation meshes are **not** in the legacy source repository, but they **are** in
the deployed ASURA build, in quantity, and the on-disk format has been verified by
parsing real files:

```
D:\FILES\project\RanOnline-Build\ASURA CLIENT\Data\Map\*.wld
  87 files, 386,705,558 bytes
  73 plain "LAND.MAN", 14 obfuscated "Land.Man"
  63 contain a navigation mesh that parses cleanly and passes internal consistency
  checks (0 anomalies across every one)
```

So the question is no longer "can we get the data". It is now "is the port bounded",
and §10 answers that.

---

## 1. Protocol (measured, not inferred)

Measured by compiling a reconstruction of the packing context — method and probe
described in §12. `pack(1)` context: `GLContrlPcMsg.h:355` opens, `:4356` closes.

| Struct | ID | Size | Offsets |
|---|---:|---:|---|
| `GLMSG::SNETPC_GOTO` | **3034** | **36** | `dwActState`@8, `vCurPos`@12, `vTarPos`@24 |
| `GLMSG::SNETPC_GOTO_BRD` | **3035** | **44** | `dwGaeaID`@8, `dwActState`@12, `vCurPos`@16, `vTarPos`@28, `fDelay`@40 |

```
NET_MSG_BASE                = 992
NET_MSG_GCTRL               = 2892
NET_MSG_GCTRL_GOTO          = 3034
NET_MSG_GCTRL_GOTO_BRD      = 3035
```

`s_NetGlobal.h:696` (`NET_MSG_BASE + 1900`) is the live definition. There is a second
`NET_MSG_GCTRL (NET_MSG_BASE + 2011)` at `:679` that looks active but is inside a
`/* */` block spanning `:674-681` — verified by reading the block, which is why the
constant is restated in the probe rather than trusted.

**Packing makes no difference** to either struct (36/44 packed and unpacked); every
member is 4 or 12 bytes and naturally aligned.

Answers to the specific questions:

| Question | Answer | Evidence |
|---|---|---|
| Coordinate type | `D3DXVECTOR3` = 3 × IEEE-754 `float` (12 B) | `GLContrlPcMsg.h:646,662` |
| Rotation / angle | **None.** No `vAngle`, no rotation field | repo-wide, and WORLD-001 already recorded RAN sends no rotation post-login |
| `dwGaeaID` in the request? | **No.** The client cannot claim identity | struct has none |
| Current position sent? | **Yes** — `vCurPos`, the client's *predicted* position | `GLCharacter.cpp:1459` |
| Destination sent? | **Yes** — `vTarPos`, but already navmesh-corrected client-side | `GLCharacter.cpp:1432-1460` |
| Speed sent? | **No.** Never on the wire | confirmed |
| Timestamp sent? | **No** | confirmed |
| Framing | Raw, like every client→server message; the 12 B header is part of `dwSize` | consistent with 002a |
| Other identity in the packet | None | confirmed |

---

## 2. Client path

```
DXKEY_DOWNED ( = DOWN|PRESSED|DRAG, DxInputDevice.h:37 )
  → DxViewPort::GetMouseTargetPosWnd  → vTargetPt  (camera-frustum point, DxViewPort.cpp:1138)
  → GLCharacter::PlayerUpdate           GLCharacter.cpp:2991-2993
  → GLCharacter::ActionMoveTo           GLCharacter.cpp:3410-3435 / :1445
       ├─ client navmesh IsCollision(camera→click) → vCollPos      :1435-1438
       ├─ m_actorMove.GotoLocation(vCollPos±10y)   ← LOCAL PREDICTION :1445-1449
       ├─ build SNETPC_GOTO{ dwActState=m_dwActState,
       │                     vCurPos=m_vPos, vTarPos=m_sTargetID.vPos }  :1457-1462
       └─ NETSENDTOFIELD(&NetMsg)                                       :1482
  → CNetClient::SendToField  (Field socket only)      s_NetClient.cpp:1036
```

**Cadence** — two send paths, and this matters:

- **Path A**, throttled to **0.2 s** (`m_fELAPS_MOVE`, `GLCharacter.cpp:258`), fires on
  every frame the left button is held and the mouse moves. So a click-drag re-plans and
  re-sends at ~5 Hz for the whole drag.
- **Path B**, end-of-frame `memcmp` drain (`:4476-4486`), sends at most once per frame
  and catches discrete clicks, reaction moves and desync resyncs.
- **Idle walking sends nothing.** No heartbeat position message exists.

**The client predicts locally and never waits for the server.** `m_actorMove.GotoLocation`
runs *before* the send (`:1445` vs `:1482`), and nothing in that path awaits `GOTO_BRD`.

---

## 3. Server path — GOTO to authoritative position

```
NET_MSG_GCTRL_GOTO (3034)
 → CFieldServer::MsgProcess, nType > NET_MSG_GCTRL      s_CFieldServerMsg.cpp:70-79
 → GLGaeaServer::MsgProcess case                          GLGaeaServerMsg.cpp:6197 → :6522
 → GLChar::MsgProcess case                                GLCharMsg.cpp:4046
 → GLChar::MsgGoto                            GLCharMsg.cpp:224-324
      ├─ EM_ACT_RUN applied from the packet                 :254-262
      ├─ |m_vPos - vCurPos| > 60.0f → snap-back, abort     :264-288
      ├─ m_TargetID.vPos = vTarPos        ← NO VALIDATION   :290
      ├─ TurnAction(GLAT_MOVE)                              :291
      ├─ m_actorMove.GotoLocation(vTarPos+(0,+10,0), vTarPos+(0,-10,0))  :293-297
      │     → vertical probe; Actor::GotoLocation SETS A PATH ONLY (actor.cpp:418-495)
      ├─ SetMaxSpeed(GetMoveVelo())                         :306-307
      ├─ SNETPC_GOTO_BRD → SendMsgViewAround                :311-318
      └─ MsgSendUpdateState                                 :321
```

`GotoLocation` takes **a 20-unit vertical probe centred on `vTarPos`**, not a
character→destination segment. It resolves to a floor hit via `IsCollision`, then either
a 2-waypoint line-of-sight path (no A*) or a full A* (`actor.cpp:437-457`).

**Position changes in exactly one place**, on the server tick, never in the handler:

```
CFieldServer::UpdateProc loop          s_CFieldServerThread.cpp:234
 → DxFieldInstance::FrameMove          DxServerInstance.cpp:285-333
 → GLGaeaServer::FrameMove             GLGaeaServer.cpp:1933
 → GLLandMan::FrameMove                GLLandMan.cpp:2406
 → GLChar::FrameMove                   GLChar.cpp:5238
      gate: IsSTATE(EM_GETVA_AFTER)     :5305
      case GLAT_MOVE:                   :6076
        SetMaxSpeed(GetMoveVelo())      :6089
        m_actorMove.Update(fElapsedTime) :6090   ← the ONLY movement advance
        m_vPos = m_actorMove.Position() :6099, :6182
```

`Actor::Update` (`actor.cpp:302-415`) advances by
`max_distance = m_MaxSpeed * elapsedTime` (`:327`), then resolves every step through
`ResolveMotionOnMesh` (`:354`).

---

## 4. Navigation — the core finding

### 4.1 Where the mesh lives

The navmesh is **embedded inside the `.wld` map file**, not a separate asset:

```
<app>\Data\Map\<name>.wld          SUBPATH.cpp:28  MAP_FILE = "\Data\Map\"
  header    128-byte magic "LAND.MAN" + DWORD version   SerialFile.cpp:50-59
  then      MapID DWORD + MapName[128]                  DxLandManSaveLoad.cpp:1903-1905
  then      filemark table: DWORD ver, DWORD size,
            { dwNAVI_MARK, dwWEATHER_MARK,
              dwGATE_MARK, dwCOLL_MARK }                DxLandDef.cpp:50-56
  at        132 + dwNAVI_MARK  →  DWORD bExist, then the mesh
```

Offsets are relative to byte 132 (`m_DefaultOffSet`); `SetOffSet(x)` seeks to `x+132`
(`SerialFile.cpp:165-177`).

`.glmap` (extension `GLLevelFile.cpp:11`) is the index that names the `.wld` per map
(`GLLevelFile.cpp:164,169` → `GLLandManSet.cpp:109-114`).

### 4.2 Generation is offline, and the tool is missing

Mesh generation lives in `DxLandMan::Import` (`DxLandMan.cpp:806-846`) and
`DxSetLandMan::CreateNaviMesh` (`:382-400`), driven by `ExportProgress`. **Neither has
any caller in this repository** — the baking tool is not in the tree. Runtime only ever
deserialises.

Source confirms `legacy/ServerField`, `ServerSession`, `ServerAgent`, `ServerLogin` and
`GMTool` contain **zero** `Navi`/`Navigation` source lines; they reference the directory
as an include path only.

### 4.3 The on-disk format — understood, and VERIFIED against real data

`NavigationSaveLoad.cpp:11-49` (write) / `:51-107` (read), all via `CSerialFile`, which
`fread`/`fwrite`s host representations — **native little-endian, no conversion**.

```
DWORD bExist
int   nVertex ; D3DXVECTOR3 vertex[nVertex]          (12 B each)
DWORD nCell   ; NavigationCell cell[nCell]
for each cell, for side in {AB,BC,CA}:
    DWORD bExist ; if bExist: DWORD LinkID
```

`NavigationCell` record — **188 bytes, measured**:

| Field | Offset | Bytes |
|---|---:|---:|
| `m_CellID` | 0 | 4 |
| `m_Vertex[3]` | 4 | 12 |
| `m_Side[3]` (`Line2D`) | 16 | 84 |
| `m_CellPlane` (`Plane`) | 100 | 28 |
| `m_CenterPoint` | 128 | 12 |
| `m_WallMidpoint[3]` | 140 | 36 |
| `m_WallDistance[3]` | 176 | 12 |

**Correction to earlier notes:** a 164-byte record appears in circulation and is wrong.
`Line2D` is three `D3DXVECTOR2` (8 B) plus a `bool` = 25 B rounded to **28**, not 20.
Measured by compiling a reconstruction; §12.

Verification on `ep3_boss_bdgside_east.wld` with the measured 188 B stride:

```
version 0x113   bExist=1   nVertex=848   nCell=424
CellID == array index violations : 0     (424/424)
cells with |normal.y| > 1         : 0
links: total=1134, per-side [424,304,406]
out-of-range LinkIDs              : 0
vertex bounds  x[-109.991 .. 109.99]  y[12.507 .. 12.507]  z[46.276 .. 2000.433]
```

All three checks matter: the first is the invariant `NavigationMesh::GetCell` silently
depends on (`navigationmesh.h:140-144` indexes by **array position**, not `CellID`), the
second proves the `Plane` stride is right, and the third proves the link pass lands on a
boundary. With the wrong 164 B stride the same file yields 423 bad IDs, 285 bad planes
and 1047 out-of-range links — so this is a real discriminator, not a formality.

### 4.4 Availability summary (87 files)

| Class | Count | Navmesh |
|---|---:|---|
| Plain `LAND.MAN`, mesh parsed and verified | **63** | yes |
| Plain `LAND.MAN`, genuinely no mesh (lobby/select/log_in) | 10 | no |
| Obfuscated `Land.Man` (`WLDCrypt.h:8`, XOR keys `:18-19`) | 14 | not yet decoded |

The 14 obfuscated files need `Common/WLDCrypt.cpp` first, exactly as
`DxLandManSaveLoad.cpp:729` does. The source and both XOR constants
(`0x99701AE`, `0x92617BE`) are present, so this is a bounded follow-up, not an unknown.

### 4.5 Still missing: the map index

**No `.glmap` files exist in the ASURA build** (0 of 87). So the *navigation* data is
available and decodable, but the **map-ID → `.wld` filename binding is not**. RAN keeps
that in `.glmap` or a DB table; neither is available here. A modern server that knows
only `WorldCharacter::saveMapId` cannot yet resolve which `.wld` to load.

---

## 5. Position

| Property | Value | Evidence |
|---|---|---|
| Type | 3 × `float`, 12 B | WORLD-001, `GLContrlPcMsg.h:646` |
| Units | RAN world units | — |
| Axis | X/Z ground plane, Y vertical | probe verticals at `:10`/`:−10`; `Normal().y` wall test `actor.cpp:358` |
| Local vs world | **World**; the mesh is baked with `D3DXVec3TransformCoord` by frame matrix (`navigationmesh.cpp:711-724`) | |
| Destination semantics | Client-supplied floor point; server accepts it **unvalidated** (`GLCharMsg.cpp:290`) | |
| Vertical resolution | ±10 unit probe; if nothing is hit, `bSucceed=FALSE` and **no path is built** | `actor.cpp:487` |
| Rotation | none on the wire; facing derived from movement direction (`GLChar.cpp:6103-6111`) | |

---

## 6. Timing

| Property | Finding |
|---|---|
| Step law | `max_distance = m_MaxSpeed * elapsedTime` — **speed × real clock delta** (`actor.cpp:327`) |
| Delta source | `timeGetTime()*0.001`, genuine wall clock (`DxServerInstance.cpp:290,303`) |
| Nominal frame constant | `0.020f` (`DxServerInstance.cpp:294`) — **but `return S_FALSE` is commented out at `:297`, so it is not enforced** |
| Event-thread pacing | 10 ms, only if `use_event_thread` is enabled — **default off** (`s_CCfg.cpp:72`) |
| Anti-flood property | Because dt is server-measured, `sum(MaxSpeed*dt) ≤ MaxSpeed*wall_seconds` holds at any packet rate. **A client cannot accelerate itself by flooding GOTO.** |
| Arrival | Hard **0.01** threshold, then snap to waypoint (`actor.cpp:347,375`); signal is `m_NextPosition == FLT_MAX` |
| Blocked movement | **Slides**: wall projection then `Direction *= 0.98f` (`navigationmesh.cpp:311`). Comment says 10%; code is 2%. No stuck timeout, no re-path |
| Y suppression | `Normal().y <= 0.0001f` freezes Y at `m_CorrectY` (`actor.cpp:358`) |
| Partial completion | Yes — `ResolveMotionOnMesh` clamps the step at the intersection point |

---

## 7. Speed

```cpp
// GLChar.cpp:4966
float fDefaultVelo = IsSTATE(EM_ACT_RUN)
    ? cCONSTCLASS[m_CHARINDEX].fRUNVELO
    : cCONSTCLASS[m_CHARINDEX].fWALKVELO;
return MoveVelocity(fDefaultVelo, GETMOVEVELO(), GETMOVE_ITEM(), IsSTATE(EM_ACT_RUN));
// GameCharacterCalculations.cpp:294 →  baseVelocity * (sumMoveVelocity + itemMoveVelocity)
```

**New finding, correcting 002a/002:** `default.charclass` **does exist** in the
deployed assets — `Data\glogic\default.charclass`, 18,404 bytes. The earlier
investigations said it was absent; they were searching the source repository, not the
build.

It is **not readable as text**. It is AES/Rijndael-encrypted:

- loaded by `GLCONST_CHAR::LOADFILE("default.charclass", bServer)` (`GLogicData.cpp:1179`)
- `cFILE.open(strPath, /*bDEC=*/true, …)` (`GLogicDataLoad.cpp:49`)
- first `int` is the version; shipped asset says **8**; then
  `CRijndael::Initialize(version, sm_Version[version-1], …)` (`StringFile.cpp:83-92`)
- **the keys are in source** (`Rijndael.cpp:933-952`), index 7 =
  `"lvdqkrmf$rpgo!@#$htjgj@#qksskrkr"`, key length 32
- a second 256-entry substitution layer (`ByteCrypt.cpp byte_decode`,
  `EMBYTECRYPT_CLASSCONST = 10`) follows

Decryption was attempted and abandoned: compiling `Rijndael.cpp` outside the legacy PCH
pulls in a `Method.h` / `CString` dependency chain and a broken include
(`"../Lib_Engine/Common/compbyte.h"` resolves to a doubled path that does not exist).
That is friction, not a wall.

**Equipment remains genuinely out of scope** (`GETMOVE_ITEM()` = worn items), so
`MoveVelocity` cannot be fully reproduced regardless. `IMovementSpeedProvider` therefore
**stays unimplemented**, per §21 — but the reason is now narrower and factual: base
walk/run velocity is recoverable with a bounded decoder; the equipment multiplier is not,
and inventing it is forbidden.

---

## 8. Broadcast

**`SNETPC_GOTO_BRD`, id 3035, 44 bytes** — the position message, sent **once per accepted
GOTO**, not per tick (`GLCharMsg.cpp:311-318`):

| Field | Source | Note |
|---|---|---|
| `dwGaeaID` | `m_dwGaeaID` | whose move |
| `dwActState` | `m_dwActState` | server state **after** the run bit was applied — not the raw request |
| `vCurPos` | `m_vPos` | **server** position; the client's claim is discarded |
| `vTarPos` | `m_TargetID.vPos` | the client's raw destination, relayed unvalidated |
| `fDelay` | hard-coded `0.0f` | **dead field** — written once, never read anywhere |

### The finding that matters most for a modern implementation

**RAN broadcasts no authoritative position after the movement advances.**
`GLChar::FrameMove` (`:5238-6198`) emits no position message of any kind; its only
broadcasts are brightness, quest-fact-end and PK-combo-end. The server's authoritative
`m_vPos` after each `Actor::Update` is never transmitted.

The client learns of other players' movement purely from the *initial* `GOTO_BRD`, then
simulates the identical walk locally on its own copy of the same navmesh
(`GLCharClient.cpp:1720-1742` — which reads only `dwActState` and `vTarPos`, discarding
`vCurPos` and `fDelay`).

The only server→client position corrections are **event-driven**, never periodic:
`SNETPC_JUMP_POS_BRD` (3064, desync snap-back) and `SNET_GM_MOVE2GATE_FB` (3830).
`SNET_POSITIONCHK_BRD` (3055) is received into a debug sphere and has no server sender.

**Consequence for §18:** there is no RAN message that reports "authoritative position
after a tick". A milestone that requires one would have to **invent** it, which §31
forbids. The honest modern equivalent is RAN's actual model: authoritative *destination
acceptance* plus server-side advancement, with clients predicting — i.e. no per-tick
position stream at all.

---

## 9. `SendMsgViewAround`

`GLCharEx.cpp:1276-1319`. Walks the per-character view list and `SENDTOCLIENT`s to each,
plus a mirror to on-map GMs (`:1316`).

Recipients are selected by `UpdateViewAround` (`GLCharEx.cpp:1378-1775`), rebuilt once
per character per tick (`GLLandMan.cpp:2553`):

1. **Quadtree sector query** over a ±`MAX_VIEWRANGE` XZ square —
   `MAX_VIEWRANGE = 250` (`GLogicData.h:39`; GM camera 5000, `:1394-1397`)
2. **Distance filter** — `bRect.IsWithIn(int(x), int(z))`, an **axis-aligned square**
   on truncated ints, *not* a Euclidean radius (`:1513`)
3. **Self-exclusion** — `pPChar->m_dwClientID != m_dwClientID` (`:1513`), so **the mover
   never receives its own `GOTO_BRD`**
4. Entries carry a per-tick `dwFRAME` stamp; un-stamped entries are dropped out
   (`:1754,1772`)

Sector membership is maintained by `GLLandMan::MoveChar` re-linking the quadtree node as a
character crosses cells (`GLLandMan.cpp:1465-1498`, called at `:2514`).

**Minimum viable subset for a movement milestone:** the ±250 XZ square test is
reproducible in a few lines without the quadtree, so sector infrastructure is *not*
strictly required. That is a genuine narrowing of the 002 assessment.

---

## 10. Modern feasibility

### Classification: **A** (data exists and format is understood)

Per §13, specifically about the navigation question: the runtime data exists (63 verified
maps) and the format is understood well enough to parse and validate it. This is a
genuine upgrade from 002's classification, which on this axis was **C**.

### But §28's implementation gate is NOT satisfied

The gate is independent of the data question, and three items fail:

1. **The minimum dependency graph is not bounded.** The irreducible core is ~2,890 lines
   (`navigationmesh.cpp` 1050, `navigationcell.cpp` 432, `navagationtree.cpp` 551,
   `actor.cpp` 526, `NavigationSaveLoad.cpp` 131 + 7 headers), and it drags in
   `SerialFile`, `DxCommon/collision.h`, `DxLandMan.h` — which alone pulls the entire
   map/terrain engine — and D3DX9 throughout. `navigationmesh.cpp:7` is included purely
   for `ExportProgress`, so a 5-symbol shim plus a D3DX9 removal is mandatory. The
   modern tree links neither D3DX9 nor any of that.

2. **Speed is not computable.** `MaxSpeed` is required *before* `Update` advances
   anything. Base velocity needs an AES decoder for `default.charclass`; the equipment
   multiplier has no available source at all. §31 forbids inventing either.

3. **There is no RAN per-tick position broadcast to reproduce** (§8). Satisfying a
   "position advances and is broadcast" requirement would require inventing a message.

A fourth, smaller gap: **the `.glmap` map index is absent**, so map-ID → `.wld`
resolution is unsolved.

**Verdict: BLOCKED.** Not because the data is missing — it was found — but because the
remaining work is a multi-thousand-line D3DX-dependent port plus two unsourceable
values. That is the §29 condition "the minimum correct implementation is too large".

---

## 11. What is proven, what is missing

**Proven**

- GOTO 3034 (36 B) and GOTO_BRD 3035 (44 B), measured, packing-independent.
- Full client and server chains, with exact locations.
- Server advances position only in `Actor::Update`, by speed × server clock delta.
- `vTarPos` is accepted unvalidated; `vCurPos` gets a weak 60-unit sanity check.
- Navigation meshes exist in the deployed build; format understood and **verified by
  parsing**; generation is offline and its tool is absent from the tree.
- RAN broadcasts a destination once and then **never** transmits position again.

**Missing / required before implementation**

1. **Map index** — `.glmap` files, or an authoritative map-ID → `.wld` table.
2. **`default.charclass` decoder** — bounded (keys in source, version 8 known) but needs
   the legacy PCH worked around. Yields `fWALKVELO` / `fRUNVELO`.
3. **Equipment speed source** — `GETMOVE_ITEM()`. No candidate data identified.
4. **A decision on the D3DX9 dependency** — port the ~2,890-line core with D3DX types
   replaced by modern equivalents, or extract the mesh maths into a standalone library
   first.
5. **A decision on §8** — accept RAN's model (authoritative destination, client-side
   prediction, no position stream), or explicitly authorise a new position message as a
   documented deviation.

---

## 12. Probes used

Investigation-only, run from a temp directory, not added to the repository.

| Probe | Purpose | Result |
|---|---|---|
| `goto_probe.cpp` | measure GOTO/GOTO_BRD sizes and offsets under the reconstructed `pack(1)` context | 36/44 bytes, offsets as tabulated; **control** (MOVESTATE 12/16) matches production `static_assert`s |
| `cell_probe.cpp` | measure `Line2D`, `Plane`, and the `NavigationCell` record stride | `Line2D`=28, `Plane`=28, record=**188** |
| `wld_probe.ps1` | scan all 87 `.wld`, locate `dwNAVI_MARK`, read header counts | 63 with mesh, 0 parse errors |
| `wld_deep_probe.ps1` | full cell + link parse with consistency checks | 0 anomalies at stride 188; garbage at 164 |
| `charclass_probe.cpp` | decrypt `default.charclass` | **failed** — legacy PCH dependency chain; keys located but not exercised |

The `goto_probe` control is the important methodological point: it measures two structs
whose sizes are independently pinned by `static_assert` in shipping modern code, so
agreement validates the method before its GOTO numbers are trusted.

---

## 13. Recommended next actions

Not implementation. In order:

1. **Establish the map index.** Cheapest, unblocks everything else. Either locate
   `.glmap` files in another RAN asset drop, or derive map-ID → `.wld` from the `.wld`
   headers themselves (`m_MapID` is stored at byte 132 per `DxLandManSaveLoad.cpp:1903`,
   and is readable in the plain files — worth a probe).
2. **Decode the 14 obfuscated `.wld`** via `Common/WLDCrypt.cpp`. Bounded; reuses keys
   already in source.
3. **Build a standalone `.wld` navmesh reader** as an *investigation* tool first —
   `NavigationCell` records only, no D3DX. If that reads all 63 maps cleanly, the format
   question is closed permanently and the port becomes mechanical.
4. **Decode `default.charclass`** for `fWALKVELO` / `fRUNVELO`, and record the values as
   measured constants.
5. **Only then** scope the movement port, with the D3DX9-removal question answered
   explicitly and the §8 broadcast decision made on the record.

Step 3 is the highest-value next probe: it is small, it needs no game code, and it
converts the remaining uncertainty into a checked-in decoder.

---

## 14. Regression status

Unchanged from `735c5aa`:

```
ModernNetworkTests         193/193
ModernServerTests          228/228
ModernWorldEntryTcpTests    14/14
CTest Debug                 18/18
CTest Release               18/18
```

No 002a behaviour was modified. The per-connection LZO codec, the synchronised
repository, the atomic `FieldEntryRegistry::Claim` and the Field role's concurrent
connections are all untouched.