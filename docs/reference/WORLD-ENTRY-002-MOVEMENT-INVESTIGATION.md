# WORLD-ENTRY-002 movement investigation

**Status: investigation COMPLETE. Implementation NOT started. WORLD-ENTRY-002 is NOT COMPLETE.**

This document exists because the brief (§30) requires stopping and reporting when the
real legacy path turns out to need a large subsystem, rather than porting thousands of
lines or inventing behaviour. That condition was met, on four independent counts, and
they are documented below with source locations.

The tree is unchanged at the `WORLD-ENTRY-001` commit `4b1b4bd`. No production
movement code was written, and no existing test was weakened.

---

## 1. The path, end to end

```
Client
  │  MOVESTATE  3032  { dwActState }              ──►  Field
  │                                                   GLChar::MsgMoveState
  │                                                     re-derives the authoritative
  │                                                     state, recomputes speed,
  │                                                     SetMaxSpeed on the actor
  │  MOVESTATE_BRD 3033 { dwGaeaID, dwActState }  ◄──  broadcast to characters in
  │                                                     view range, ONLY if changed
```

Separately, and this is the part that blocks the milestone:

```
Client
  │  GOTO       3034  { dwActState, vCurPos, vTarPos }  ──►  Field
  │                                                     GLChar::MsgGoto
  │                                                       destination accepted,
  │                                                       path built on the navmesh
  ▼
GLGaeaServer::FrameMove(fTime, fElapsedTime)          ── the world tick
  └─► GLLandMan::FrameMove(fTime, fElapsedTime)
        └─► GLChar::FrameMove(fTime, fElapsedTime)        (GLChar.cpp:5238)
              └─► m_actorMove.Update(fElapsedTime)        (NaviMesh/actor.cpp)
                    └─► m_Parent->ResolveMotionOnMesh / IsCollision /
                        FindClosestCell / SnapPointToCell
```

`MOVESTATE` is the **state** half of movement (running vs walking, peace mode,
visibility). `GOTO` is the **position** half. They are separate messages and the
milestone needs the second one.

---

## 2. Dispatch chain, traced not assumed

| Step | Location |
|---|---|
| Field server accepts anything `> NET_MSG_GCTRL` and forwards it | `s_CFieldServerMsg.cpp:70-79` → `GLGaeaServer::MsgProcess` |
| `MOVESTATE` and `GOTO` fall into the per-character branch | `GLGaeaServerMsg.cpp:6196-6197` |
| that branch calls `pChar->MsgProcess(nmg)` | `GLGaeaServerMsg.cpp:6517-6523` |
| `GLChar` dispatches the two ids | `GLCharMsg.cpp:4044` (`MsgMoveState`), `:4046` (`MsgGoto`) |
| client receives the broadcasts | `GLCharClient.cpp:1748-1749` |

`NET_MSG_GCTRL = NET_MSG_BASE + 1900 = 992 + 1900 = 2892`
(`NetworkTypes.h` already carries this as `Protocol::kGCtrlBase`).

---

## 3. Measured wire layouts

Measured by **compilation**, not arithmetic, using the WORLD-001 Phase A method: the
struct definitions were sliced programmatically out of the real headers, in-class
function bodies dropped (they cannot affect layout), and compiled under a
reconstruction of the packing context. The probe is reproducible and its output is
quoted below.

Packing context, which is the part Phase A was burned by:

- `GLContrlBaseMsg.h:272` opens `#pragma pack(1)`
- `GLContrlPcMsg.h:355` opens `#pragma pack(1)`, `:4356` closes it
- `SNETPC_MOVESTATE` (`:700`), `SNETPC_MOVESTATE_BRD` (`:717`), `SNETPC_GOTO` and
  `SNETPC_GOTO_BRD` all fall inside that `pack(1)` region
- `SNETPC_MOVESTATE_BRD` and `SNETPC_GOTO_BRD` **derive** from `SNETPC_BROAD`
  (`GLContrlBaseMsg.h:274`), so the probe reproduces that inheritance

```
NET_MSG_GCTRL   = 2892  (NET_MSG_BASE 992 + 1900)
MOVESTATE       = 3032   (GCTRL + 140)
MOVESTATE_BRD   = 3033   (GCTRL + 141)
GOTO            = 3034   (GCTRL + 142)
GOTO_BRD        = 3035   (GCTRL + 143)

--- PACKED (the real context) ---
SNETPC_BROAD                       12
SNETPC_MOVESTATE                   12   actState@8
SNETPC_MOVESTATE_BRD               16   gaeaID@8  actState@12
SNETPC_GOTO                        36   actState@8  cur@12  tar@24
SNETPC_GOTO_BRD                    44   gaeaID@8  actState@12  cur@16  tar@28  delay@40

--- unpacked, for comparison ---
SNETPC_MOVESTATE                   12
SNETPC_MOVESTATE_BRD               16
SNETPC_GOTO                        36
SNETPC_GOTO_BRD                    44
```

**Packing makes no difference to these four structs**, unlike `SITEM_LOBY` (48 vs a
summed 44) and `SNETLOBBY_CHARJOIN` (1022 vs 1032). Every member is 4 or 12 bytes and
naturally aligned, so there is no padding for `pack(1)` to remove. Recorded because
"we checked" is worth more than "it looked fine", and because the reverse assumption
has already produced one 88-byte bug in this project.

| Struct | Size | Fields |
|---|---:|---|
| `GLMSG::SNETPC_MOVESTATE` | **12** | `dwActState` DWORD @8 |
| `GLMSG::SNETPC_MOVESTATE_BRD` | **16** | `dwGaeaID` DWORD @8, `dwActState` DWORD @12 |
| `GLMSG::SNETPC_GOTO` | **36** | `dwActState` @8, `vCurPos` @12, `vTarPos` @24 |
| `GLMSG::SNETPC_GOTO_BRD` | **44** | `dwGaeaID` @8, `dwActState` @12, `vCurPos` @16, `vTarPos` @28, `fDelay` float @40 |

Coordinates are `D3DXVECTOR3` — three IEEE-754 `float`, 12 bytes. **No rotation and
no angle**, consistent with the WORLD-001 finding that RAN sends none anywhere in the
post-login protocol. `vCurPos` in `GOTO` is the client's claim about where it
currently is; the server does not have to believe it.

---

## 4. What the server actually validates

`GLChar::MsgMoveState` (`GLCharMsg.cpp:182-219`) is the whole handler, and it is
short because it is honest:

- The GM visibility flags (`EM_REQ_VISIBLENONE`, `EM_REQ_VISIBLEOFF`) are applied
  **only if `m_dwUserLvl >= USER_GM3`**, which is **20** (`s_NetGlobal.h:313`). A
  normal client sending those bits has them silently dropped.
- `EM_ACT_RUN` and `EM_ACT_PEACEMODE` (`GLCharDefine.h:1158`, `:1160`) are set or
  cleared from the bitmask — the server **re-derives** the state rather than storing
  the client's value.
- If the derived state differs from the old one, the server recomputes
  `GetMoveVelo()` and applies it with `m_actorMove.SetMaxSpeed(fVelo)`.
- Only then does it broadcast `SNETPC_MOVESTATE_BRD` via `SendMsgViewAround`.

Flag values (`GLCharDefine.h:1158-1168`):

```
EM_ACT_RUN        = 0x00000001
EM_ACT_PEACEMODE  = 0x00000004
EM_REQ_VISIBLENONE= 0x00001000
EM_REQ_VISIBLEOFF = 0x00002000
```

So the authoritative-decision pattern the brief asks for genuinely exists here: the
client asks, the server decides, and the server's answer is broadcast.

---

## 5. Why this stops the milestone

The brief asks for a character that **submits a movement request, has world state
advanced by a tick, and receives the resulting authoritative movement message**. Four
independent findings say that means porting a navigation subsystem.

### 5.1 `Actor::Update` cannot run without a `NavigationMesh`

`NaviMesh/actor.cpp`, `Actor::Update(float elapsedTime)`:

```cpp
HRESULT Actor::Update(float elapsedTime)
{
    if (!m_Parent)     return E_FAIL;
    if (!m_PathActive) return S_OK;
    ...
    m_Movement = (*m_NextWaypoint).Position;
    m_Movement -= m_Position;
    float max_distance = m_MaxSpeed * elapsedTime;
    ...
    m_Parent->ResolveMotionOnMesh(m_Position, m_CurrentCellID, NextPosition, &NextCellID);
```

Movement is **waypoint-path driven over a navigation mesh**. Every position change
goes through `ResolveMotionOnMesh`, and the surrounding code also uses
`IsCollision`, `FindClosestCell` and `SnapPointToCell`. There is no seam that lets a
character move "correctly" without one.

### 5.2 The size of that subsystem

`legacy/Lib_Engine/NaviMesh` is **5374 lines** across 15 files. The irreducible core
is roughly 2900: `navigationmesh.cpp` 1050, `navigationcell.cpp` 432 +
`navigationcell.h` 442, `navagationtree.cpp` 551, `navigationheap.h` 265,
`navigationpath.h` 153. Plus `playpen.cpp` 853 and `plane.h` 223.

### 5.3 The map data is not in this repository

WORLD-001 §10 already recorded this and it still holds: the map files that bind maps
to Field servers are **absent** from the legacy tree. A navigation mesh with no mesh
data cannot validate a single step, so the numbers above are a floor, not an estimate.

### 5.4 The authoritative speed is not computable from what exists

`GetMoveVelo()` (`GLChar.cpp:4966`) is:

```cpp
float fDefaultVelo = IsSTATE(EM_ACT_RUN)
    ? GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX].fRUNVELO
    : GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX].fWALKVELO;
return GameCharacterCalculations::MoveVelocity(
        fDefaultVelo,
        ..., GLCHARLOGIC::GETMOVEVELO()
        ..., GLCHARLOGIC::GETMOVE_ITEM()
        ..., IsSTATE(EM_ACT_RUN));
```

The base walk/run velocity comes from `cCONSTCLASS`, loaded from
`default.charclass` — **the same data file WORLD-002's `ServerCharacter` already
documents as not present in this repository**. And the speed bonus comes from
`GETMOVE_ITEM()`, i.e. **worn equipment**, which WORLD-ENTRY-001 explicitly excludes.

So "authoritative movement speed" has no derivable source here. A modern server would
have to take it from configuration — which is a reasonable engineering choice, but it
is a **deviation from RAN** and the brief requires deviations to be documented rather
than assumed.

### 5.5 Multi-client broadcast also depends on the land system

`SendMsgViewAround` (`GLChar.h:629`) is **view-range scoped**, implemented over
`GLLandMan`'s character lists and sectors. "A's movement reaches B" therefore needs
the land/sector/view system as well, which is the same subsystem family as 5.1.

---

## 6. What was NOT done, deliberately

- **No straight-line lerp standing in for navmesh movement.** It would satisfy
  "position changed after a tick" while being behaviour RAN does not have. That is
  inventing a protocol, which §8 and §9 forbid, and it would be the single most
  expensive thing to unpick later — because every downstream system would be built on
  a movement model that is wrong.
- **No `PLAYER_MOVED` invented message.** §14 requires the real one. `MOVESTATE_BRD`
  and `GOTO_BRD` are the real ones; neither is written yet.
- **No `MOVESTATE` alone shipped and called COMPLETE.** It is a genuine, fully
  pinned slice, but it carries no position, so it cannot satisfy §13 or §26's
  authoritative-position requirements. Building it and reporting COMPLETE would be a
  false claim.
- No socket, framing, compression or connection code was touched. The Field TCP
  connection established by WORLD-ENTRY-001 is the right transport and is ready.

---

## 7. Proposed smaller next phase

**WORLD-ENTRY-002a — authoritative movement STATE over the Field connection.**

Scope, all of it already proven above and with zero unknowns:

1. `MovementProtocol.{h,cpp}` in `modern/network`: `MOVESTATE` 3032 (12 bytes in) and
   `MOVESTATE_BRD` 3033 (16 bytes out), with `static_assert`s on the measured sizes
   and offsets, in the Phase A style.
2. `WorldCharacter` gains an `actState` field only — no position semantics, no
   velocity.
3. A deterministic `MovementService::ApplyMoveState(gaeaId, requestedBits)` that
   reproduces `MsgMoveState`: gate the GM visibility flags at `USER_GM3`, re-derive
   `EM_ACT_RUN` / `EM_ACT_PEACEMODE`, and report whether the authoritative state
   changed. **No tick is involved** — this message is not time-based, and pretending
   otherwise would be a second invented model.
4. `FieldRoleRuntime` handles 3032 after a successful 2359 and emits 3033 when the
   authoritative state changed, to that session only. Broadcasting to other sessions
   waits for the land system.
5. Client `WorldEntryClient` gains `BuildMoveState` / move-state decoding on the
   **existing** Field connection, reusing `MessageReader` and `ConnectionFramer`.
6. Tests: layout and byte-level offsets, the `USER_GM3` gate, broadcast-only-on-change,
   rejection before 2359, malformed shapes, fragmentation, coalescing with the next
   message, and a real-TCP continuation of the WORLD-ENTRY-001 flow.
7. Speed is **not** part of this phase. `SetMaxSpeed` is recorded as the point where
   the `default.charclass` question must be answered.

**Then WORLD-ENTRY-002b — position**, once someone decides where navigation mesh data
comes from. That is a data question before it is a code question, and it should be
answered before any of it is written.

---

## 8. Status of everything else

Unchanged and verified at `4b1b4bd`:

```
ModernNetworkTests  193/193
ModernServerTests   212/212
CTest Debug         18/18
CTest Release       18/18
```

The 18th CTest entry is the WORLD-ENTRY-001 two-connection integration binary.