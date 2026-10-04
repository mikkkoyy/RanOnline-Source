# WORLD-001 protocol investigation

**Status: investigation COMPLETE. Implementation NOT started. WORLD-001 is not PASS.**

This document exists because the brief (§24) allows separating investigation from
implementation when the investigation is substantial. It is. Everything below is
measured or quoted, and nothing in it is a guess.

No production code has been written for WORLD-001. The tree is at the LOGIN-002
baseline and nothing here changes it.

---

## 0. Naming collision — unresolved, needs a decision

`WORLD-001` and `WORLD-002` are **already used** in this repository:

| Commit | Meaning |
|---|---|
| `aefcb82 WORLD-001 modern client to server login` | `NET_MSG_LOGIN_2` (2049), the Agent login **request** |
| `b6ce32b WORLD-002 modern server to client login response` | `NET_MSG_LOGIN_FB` (2050), the login **reply** |

`docs/MODERNIZATION_STATUS.md:756` independently calls the *world-entry* milestone
"WORLD-001". So the roadmap and the commit history disagree. This investigation
proceeds under the brief's naming and flags the collision rather than silently
picking one.

---

## 1. Authority order actually used

```
modern architecture  >  legacy/ source  >  public sources  >  assumption
```

Public sources were read (§36) and are recorded in §8. They corroborated the
server-role split and the `ConnectLoginServer` signature. They established
**nothing** load-bearing: the compression finding, the message sizes and the
spawn layout all rest on `legacy/`.

---

## 2. The path, end to end

```
Client
  │  1  ConnectLoginServer            -> Login Server
  │  2  REQ_GAME_SVR        1542  ──►  Login Server
  │     SND_GAME_SVR        1552 ×N ──►
  │     SND_GAME_SVR_END    1562  ──►  Login Server closes            [LOGIN-002]
  │  3  ConnectGameServer  -> Agent Server   (same socket re-pointed, legacy)
  │  4  LOGIN_2             2049  ──►
  │     LOGIN_FB            2050  ──►                                     [already built]
  │  5  REQ_CHA_BAINFO      2247  ──►  Agent
  │     CHA_BAINFO          2248  ──►   (character ID array)
  │     REQ_CHA_BINFO       2244 ×N ──► (one per ID)
  │     LOBBY_CHAR_SEL      2332 ×N ──► (one full character per ID)
  │  6  LOBBY_GAME_JOIN     2353  ──►  Agent   { nChaNum }
  │     ── agent->field, server-to-server, invisible to client ──
  │     CONNECT_CLIENT_FIELD 2358 ──►  { field IP, port, gaeaId, slot }
  │  7  client opens a SECOND TCP connection to the Field server
  │     JOIN_FIELD_IDENTITY  2359 ──►  Field  { gaeaId, slot, ck }
  │     LOBBY_CHAR_JOIN      2333 ──►  Field  { gaeaId, map, pos, SCHARDATA… }
```

### 2.1 Which server owns what — proven, not assumed

| Role | Owns | Evidence |
|---|---|---|
| **Login** | the server directory. Nothing else. | `s_CLoginServerMsg.cpp:28-41` — three cases only |
| **Session** | a server registry / switchboard. Server↔server only. Never a position. | `s_CSessionServerMsg.cpp:22-105` |
| **Agent** | authentication, the character list, and the **entry decision**. | `s_CAgentServerMsg.cpp:100` |
| **Field** | **the world, and spawn.** Hosts `GLGaeaServer`. | `s_CFieldServerMsg.cpp:159` → `GLChar::MsgGameJoin()` |

The "enter the game" instruction goes to the **Agent** (`CAgentServer::MsgGameJoin`,
`s_CAgentServerMsg.cpp:100`). The Agent opens a socket to a Field server, the Field
server builds the `GLChar`, and the Agent relays the Field address back. Spawn
itself is emitted by the **Field** server.

The Session server is entirely out-of-band here: the client never talks to it.

---

## 3. Measured packet table

Sizes in the "size" column are **measured** (§7), except where marked *computed*.

### 3.1 Small messages — sizes verified by reading the declarations

| Dir | Message | ID | Size | Fixed? | Purpose |
|---|---|---:|---:|---|---|
| C→A | `NET_MSG_REQ_CHA_BAINFO` | **2247** | **8** | fixed | request the character list. Bare header, no body |
| A→C | `NET_MSG_CHA_BAINFO` | **2248** | **28** or **76** | fixed, **two sizes** | count + array of character IDs |
| C→A | `NET_MSG_REQ_CHA_BINFO` | **2244** | **12** | fixed | request one character's detail, by ID |
| A→C | `NET_MSG_LOBBY_CHAR_SEL` | **2332** | **1176** | fixed | one full character |
| C→A | `NET_MSG_LOBBY_GAME_JOIN` | **2353** | **12** | fixed | `INT nChaNum` — the selection |
| A→C | `NET_MSG_CONNECT_CLIENT_FIELD` | **2358** | **48** | fixed | Field IP + port + gaeaId + slot |
| C→F | `NET_MSG_JOIN_FIELD_IDENTITY` | **2359** | **24** | fixed | gaeaId + slot + `CRYPT_KEY` |
| F→C | `NET_MSG_LOBBY_CHAR_JOIN` | **2333** | **1022** | fixed | **spawn** |
| A→C | `NET_MSG_LOBBY_CHAR_JOIN_FB` | **2335** | **12** | fixed | selection refused, with a reason |
| F→C | `NET_MSG_LOBBY_GAME_COMPLETE` | **2354** | **8** | fixed | client-synthesised; see §4.4 |

All IDs are `NET_MSG_LOBBY + n`, and `NET_MSG_LOBBY = NET_MSG_BASE + 950 = 1942`
(`s_NetGlobal.h:693`), with `NET_MSG_BASE = 992`.

**The 2248 two-size problem.** `NET_CHA_BBA_INFO` is
`{ nmg; int nChaSNum; int nChaNum[MAX_ONESERVERCHAR_NUM]; }`, and
`MAX_ONESERVERCHAR_NUM` (`s_NetGlobal.h:195-199`) is:

- **16** if `KRT_PARAM`, `_RELEASED`, `KR_PARAM`, `TW_PARAM`, `HK_PARAM`,
  `TH_PARAM`, `MYE_PARAM`, `MY_PARAM`, `CH_PARAM`, `PH_PARAM` or `JP_PARAM`
- **4** otherwise

⇒ 28 bytes or 76 bytes. **The client must decode by `dwSize`, never by a fixed
struct.** This checkout's buildable `.vcxproj` files define **no** country macro
(only dead VS2003 `.vcproj` files define `KR_PARAM`), so a build of *this tree*
emits 28 — while a real Korean release server emits 76.

### 3.2 `SCHARINFO_LOBBY` — payload of 2332, sizeof **1168**

| Off | Size | Field | In scope? |
|---:|---:|---|---|
| 0 | 4 | `m_dwCharID` | ✅ |
| 4 | 33 | `m_szName` | ✅ |
| 40 | 4 | `m_emClass` | ✅ |
| 44 | 2 | `m_wSchool` | ✅ |
| 46–54 | 2 ea | `m_wHair`, `m_wFace`, `m_wSex`, `m_wHairColor` | reserved |
| 56 | 8 | `m_sHP` | ✅ |
| 64 | 16 | `m_sExperience` | reserved |
| 80 | 4 | `m_nBright` | reserved |
| 84 | 2 | `m_wLevel` | ✅ |
| 86 | 12 | `m_sStats` | reserved |
| 104 | **1056** | `m_PutOnItems[22]` (22 × `SITEM_LOBY` 48) | **out of scope** |
| 1160 | 4 | `m_sSaveMapID` | ✅ |
| 1164 | 4 | `m_fScaleRange` | reserved |

> **This is where the arithmetic went wrong.** Two independent readings of the
> source both computed 1080, assuming `SITEM_LOBY` is 44 bytes. It is **48**, so
> the array is 1056 and `SCHARINFO_LOBBY` is **1168** — 88 bytes more than
> believed. `SNETLOBBY_CHARINFO` is therefore **1176**, not 1088. Had WORLD-001
> been written on the arithmetic, every character-detail packet would have been
> malformed and a real client would have desynchronised on the first character.

**Not present, and must not be invented:** position (no `D3DXVECTOR3` anywhere in
this struct — only `m_sSaveMapID`), club/guild, deleted flag, locked flag,
server/channel id, money, inventory, skills. Deleted characters are filtered in
SQL (`AND ChaDeleted=0`, `s_COdbcGame.cpp:199`), and the server group is filtered
the same way (`AND SGNum=%d`).

### 3.3 `SNETLOBBY_CHARJOIN` — the spawn, sizeof **1022**

`#pragma pack(1)`; measured.

| Off | Size | Field | In scope? |
|---:|---:|---|---|
| 0 | 8 | `nmg` | header |
| 8 | 21 | `szUserID` | ✅ |
| 29 | 4 | `dwClientID` | ✅ |
| 33 | 4 | `dwGaeaID` | ✅ **entity id** |
| 37 | 4 | `sMapID` | ✅ |
| 41 | 12 | `vPos` | ✅ **authoritative position** |
| 53 | **600** | `Data` (`SCHARDATA`) | partial — see below |
| 653 | 2 | `wSKILLQUICK_ACT` | reserved |
| 655 | 240 | `sSKILLQUICK[60]` | reserved |
| 895 | 48 | `sACTIONQUICK[6]` | reserved |
| 943–987 | 44 | ten `dwNum*` counts | reserved (all zero) |
| 987 | 1 | `bBIGHEAD` | reserved |
| 988 | 1 | `bBIGHAND` | reserved |
| 989 | 4 | `sStartMapID` | ✅ |
| 993 | 4 | `dwStartGate` | ✅ |
| 997 | 4 | `sLastCallMapID` | reserved |
| 1001 | 12 | `vLastCallPos` | reserved |
| 1013 | 1 | `bTracingChar` | reserved |
| 1014 | 4 | `dwThaiCCafeClass` | reserved |
| 1018 | 4 | `nMyCCafeClass` | reserved |

The fields WORLD-001 needs **inside** `SCHARDATA` (600 bytes), by measured offset:
`m_dwUserID` 0 · `m_dwCharID` 76 · `m_szName` 80 · `m_emClass` 120 ·
`m_wSchool` 124 · `m_wLevel` 144 · `m_sHP` 304 · `m_sMP` 312 · `m_sSP` 320.
Everything else in the 600 bytes is reserved for milestones this one excludes.

**Deliberately absent, and that is a finding, not an omission:**

- **No rotation/angle anywhere in the RAN post-login protocol.** Repo-wide search
  for `vAngle` / `SNETPC_ANGLE` returns nothing. Do not add a spawn heading field.
- **No map channel in the spawn message.** The channel is fixed at login and never
  re-sent.
- **Equipment is not in the spawn message** — 22 separate
  `SNETLOBBY_CHARPUTON_EX` messages follow in the same burst.

---

## 4. Behaviour that is easy to get wrong

### 4.1 The character list has NO terminator

2248 (count + IDs) then exactly `nChaSNum` × 2332. The client detects completion
by **counting**: `IsStartReady() { return m_nStartCharNum == m_nStartCharLoad; }`
(`DxLobyStage.h:150`). There is no end-of-list message anywhere in the tree.

This differs from LOGIN-001's `SND_GAME_SVR_END`, and the difference matters: a
modern client that waited for a terminator would wait forever.

### 4.2 Compression is asymmetric, and NOT negotiated

| Direction | Wrapped in `NET_COMPRESS`? |
|---|---|
| Agent → client | **YES**, unconditionally |
| Field → client | **YES**, unconditionally |
| Login → client | **NO** (`s_CLoginServer.cpp:827` calls `SendClient2`) |
| Session → client | **NO** (`s_CSessionServer.cpp:413`) |
| client → anything | **NO** |
| server ↔ server | **NO** |

There is no handshake, flag or capability byte. `NET_COMPRESS` = **170**.

**One `NET_COMPRESS` envelope may contain SEVERAL logical messages**, because
Agent/Field batch up to `COMPRESS_PACKET_SIZE` (1000) bytes before flushing. So
the client must unwrap an envelope into a message *stream* and feed all of it to
`ConnectionFramer`.

Modern already has this: `NetCompressCodec::DecodeServerToClientEnvelope` returns
exactly "a concatenation of complete `NET_MSG_GENERIC` messages, ready to be fed to
`ConnectionFramer`". **Reuse it; do not write a second unwrapper.**

`bCompress == false` means *raw payload inside the envelope* — the envelope is
still present. A receiver must key off the flag, not off the presence of the id.

### 4.3 Character ownership is enforced — findable, but only in SQL

`CAgentServer::MsgGameJoin` validates only `IsAccountPass` and records the chosen
ID (`s_CAgentServerMsg.cpp:648`). The ownership check is a **SQL predicate**:

```sql
FROM ChaInfo WHERE ChaNum=%d AND UserNum=%d
```
(`s_COdbcGameChaGet.cpp:41`), bound to the session's own `UserNum`.

A modern in-memory repository must make this explicit and testable rather than
leave it implicit in a query — the brief requires that behaviour, and it should be
a stated invariant, not an emergent property of a storage engine.

### 4.4 There is no world-entry acknowledgement

`NET_MSG_GAME_JOIN_OK` (2355) exists but both send sites are commented out
(`s_CFieldServerMsg.cpp:433-447`, `s_CAgentServerMsg.cpp:900-912`), and it was
Field→Session, never Field→Client. `NET_MSG_LOBBY_GAME_COMPLETE` (2354) is
**synthesised by the client** once it has received the whole burst
(`DxGameStage.cpp:581`).

⇒ A modern server has nothing to acknowledge. Completion is the client's
conclusion. Do not invent an ack message.

### 4.5 Position is persistent, with a documented fallback

The spawn position is the **DB save position** (`ChaSaveMap`, `ChaSavePosX/Y/Z`,
read at `s_COdbcGameChaGet.cpp:29-31`). The school start point is used only when
the saved map is unresolvable, the map's `bRestart` flag is set, the character is
dead, or it is a PVP-event map (`GLAgentServerMsg.cpp:87-240`).

⇒ A modern repository must store a save position per character and spawn there.
Movement is out of scope, so nothing writes it back yet; the field exists and is
honoured on entry.

---

## 5. Crypt: no boundary needed — proven

| Question | Answer | Evidence |
|---|---|---|
| Is `dhkey.cpp` Diffie-Hellman used? | **No.** It compiles and links, and `new CDHKey(64)` runs, but its only entry point `EncryptLoginDataWithApexRSA` has its sole call site **commented out**. | `s_NetClientMsgLogin.cpp:145-147` |
| Is `m_Bit::buf_encode` (the cipher `CRYPT_KEY` would drive) used? | **No.** All four call sites commented out. | `s_NetClientMsgLogin.cpp:49-57` and three more |
| Does `CRYPT_KEY` protect anything? | **No.** Server hardcodes `{1,1}`; the client **ignores the transmitted value** and hardcodes `{1,1}`. | `s_CClientManager.cpp:93-102`, `s_NetClientMsg.cpp:135-144` |
| Is minTea used on world entry? | **No.** Plaintext. | all 10 live `m_Tea` call sites are 4 fields of the login request, one heartbeat field, and country login variants |
| What does minTea encrypt, precisely? | 4 named fields of a login request (`szUserid`, `szPassword`, `szRandomPassword`, `szEnCrypt`) + `szEnCrypt` in an Agent-state heartbeat. | `s_NetClientMsgLogin.cpp:59-62` |

⇒ **WORLD-001 needs no crypto boundary.** 2353, 2358, 2359 and 2333 are plaintext.
2359 still carries 4 `CRYPT_KEY` bytes; they are sent as `{1,1}` to keep framing
aligned, and the report must say plainly that they protect nothing.

---

## 6. Country variant: no guesswork needed

`NET_MSG_BASE` is **992 in all 14 branches** of the `#if` chain
(`s_NetGlobal.h:644-672`), and the live offsets (`450/950/1450/1900`,
`s_NetGlobal.h:692-696`) are unconditional. **Every message ID in this document is
the same in every configuration.**

The buildable `.vcxproj` files define **no** country macro; only dead VS2003
`.vcproj` files define `KR_PARAM`. `DAUM_*`, `CHINA_*`, `GSP_*`, `TERRA_*` login
paths are all present and live but are parallel *entry points* to the same
post-login flow. A modern client needs `NET_MSG_LOGIN_2` only.

The one genuinely country-dependent value is `MAX_ONESERVERCHAR_NUM` (§3.1), which
changes a wire size.

---

## 7. How the sizes were measured

Not by arithmetic. Both large packets embed a **default-aligned** struct inside a
`#pragma pack(1)` struct, and the naive calculation gets one of them wrong.

Method: the struct definitions were sliced **programmatically** out of the legacy
headers (never retyped), in-class function bodies were dropped (they cannot affect
layout), and the result compiled with MSVC Win32 under a faithful reconstruction
of the packing context. The real headers could not be compiled because they
require the DirectX 2005 SDK, which is not installed.

Two corrections this produced, both of which would have shipped as bugs:

1. **`SITEM_LOBY` is 48 bytes, not 44** ⇒ `m_PutOnItems` is 1056, not 968 ⇒
   `SCHARINFO_LOBBY` is **1168**, not 1080 ⇒ `SNETLOBBY_CHARINFO` is **1176**, not
   1088. An 88-byte error in a per-character packet.
2. **`pack(1)` removes 10 bytes of trailing/inter-member padding** from
   `SNETLOBBY_CHARJOIN`: 1032 unpacked, **1022** packed. Measuring without the
   pragma gives the wrong answer.

Residual risk, stated rather than hidden: `TCHAR` was taken as `char` (MBCS),
which is correct for this tree — every `.vcxproj` is MultiByte and no
`_UNICODE` is defined anywhere. A Unicode build would change `SCHARDATA`'s size.

---

## 8. Public backread (corroboration only)

| Public finding | Corroborates |
|---|---|
| RaGEZONE's RAN mirror is laid out `[Server]__Login/__Session/__Field/__Agent`, `[Lib]__NetClient`, `[Lib]__NetServer` | the four-role split and the `s_NetGlobal.h:82-86` role constants |
| A forum post quoting `int ConnectLoginServer(const char *szAddress, int nPort=…)` | `s_NetClient.cpp:367` |
| A forum post showing `CLoginServer::SessionSndSvrInfo` in `s_LoginServerSession.cpp`, built from `CCfg::GetServicePort()` and `m_szAddress` | CCfg-driven bind address/port; adds `GetProxyIp()`, an IP-proxy feature absent from this checkout |
| Forum threads where the client's `nLoginPort` disagreed with the server `.cfg` | the in-tree 5001 vs 12004 mismatch |
| An x64-porting thread calling the `DWORD`→`DWORD_PTR` migration "the most painful" part | NETWORK-001's decision to store handles as `uintptr_t` |

Nothing public established the compression finding, the sizes, or the spawn
layout. Those rest on `legacy/` alone.

---

## 9. Design agreed for implementation

Decisions taken (user-confirmed):

1. **Two listeners in one process** — an Agent role and a Field role on separate
   ports, so the client genuinely opens a second TCP connection and 2358/2359 are
   exercised rather than stubbed. The server↔server hop (2356/2357) becomes an
   in-process call and is documented as the one simplification.
2. **Authoritative subset, zero-filled reserved regions** — measure the layout,
   implement only what WORLD-001 needs, zero everything else so offsets and total
   size stay wire-correct. No excluded subsystem is built.

Planned boundaries, subject to being cut down if it turns out to be more than the
milestone needs:

```
modern/network/CharacterListProtocol.{h,cpp}     2247 / 2248 / 2244 / 2332
modern/network/WorldEntryProtocol.{h,cpp}        2353 / 2358 / 2359 / 2333 / 2335
modern/server/world/WorldCharacter.{h,cpp}       authoritative character record
modern/server/world/CharacterRepository.{h,cpp}  in-memory, ownership enforced explicitly
modern/server/world/WorldSession.{h,cpp}         Agent role: list, selection, redirect
modern/server/world/WorldFieldSession.{h,cpp}    Field role: identity, spawn
modern/client/login/CharacterListClient.{h,cpp}  client side of the list exchange
modern/client/login/WorldEntryClient.{h,cpp}     selection, redirect, field identity, spawn
```

Reused unchanged: `TcpTransport`, `TcpListener`, `ConnectionFramer`,
`NetCompressCodec` (both directions), `Lzo1xCodec`, `GameServerListProtocol`,
`LoginServerClient`, and the LOGIN-002 session pattern.

---

## 10. What is still open

- [ ] Implementation of the above — **not started**.
- [ ] Whether `WORLD-001` is the right name, given §0.
- [ ] Test design is specified in the brief (§17) but no test has been written.
- [ ] `mapslist.mst` — the file binding maps to Field servers — **is not present
      in the legacy tree**. A real deployment needs it; a modern fixture does not,
      because WORLD-001 spawns into one configured map.