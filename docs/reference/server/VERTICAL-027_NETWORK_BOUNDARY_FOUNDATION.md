# VERTICAL-027 - Modern Network Boundary Investigation + Foundation

| | |
| --- | --- |
| Repository | `mikkkoyy/RanOnline-Source` |
| Baseline | `226abd85165f02db7eb5759204de8e6ad4015340` (VERTICAL-026) |
| Branch | `main` |

Legacy is reference and oracle only. Nothing under `legacy/` was modified.

Every claim below is tagged:

| Tag | Meaning |
| --- | --- |
| **CONFIRMED** | read directly from legacy source at the cited location |
| **INFERRED** | follows from confirmed facts, with the reasoning stated |
| **UNKNOWN** | not established; recorded so it is not mistaken for a fact |
| **UNRESOLVED** | conflicting or insufficient evidence |

---

## 1. Authority hierarchy

As locked by the brief and honoured throughout:

1. Shipped ASURA behaviour/configuration
2. Original RAN project configuration (`*.vcproj`)
3. Original legacy source
4. Converted `*.vcxproj`
5. Forum/community evidence
6. Engineering inference

**V026's finding is load-bearing here.** `Lib_Network/*.vcproj` (VS2003,
Korean-authored) define `KR_PARAM` in Debug and Release across every project; the
converted `.vcxproj` define none and therefore select different code paths. For
networking, the divergence that matters is documented at section 6: the **country
chain assigns `NET_MSG_BASE = 992` in every branch**, so the protocol constants
are unaffected - but the *auth* path differs, because the LOBBY range carries
per-platform login variants.

The `KR_PARAM` authority finding itself is recorded once, in
`VERTICAL-026_CALCDAMAGE_VARIANT_AUTHORITY.md`, and is not duplicated here. A
standalone `docs/reference/build/LEGACY_PROJECT_CONFIGURATION.md` was considered
and deliberately **not** created: extracting it would fork the record across two
documents that could then disagree, and the brief explicitly warns against
duplication.

---

## 2. Legacy server roles - CONFIRMED

`s_NetGlobal.h:82-86`, verbatim:

```cpp
// server type [type]
// 1 : Login server
// 2 : Session server
// 3 : Field server
// 4 : Agent server, Game Server
NET_SERVER_LOGIN    = 1
NET_SERVER_SESSION  = 2
NET_SERVER_FIELD    = 3
NET_SERVER_AGENT    = 4
```

Matching classes: `CLoginServer` (`s_CLoginServer.h`), `CSessionServer`,
`CFieldServer`, `CAgentServer`, all in `Lib_Network`, each with a thin executable
in `ServerLogin/`, `ServerSession/`, `ServerField/`, `ServerAgent/`.

Observed responsibilities, from the class contents rather than from forum posts:

| Server | Source evidence | Reads as |
| --- | --- | --- |
| Login | `CLoginServer` holds `G_SERVER_CUR_INFO_LOGIN m_sGame[MAX_SERVER_GROUP][MAX_CHANNEL_NUMBER]` (`s_CLoginServer.h`) | publishes the server/channel table; owns no character state |
| Agent | `s_COdbcUser.cpp` (65 KB), `s_COdbcUserCheck.cpp` (42 KB), `s_CAgentServerMsgLogin.cpp` (112 KB) | authentication, user record, character list/select |
| Field | `s_FieldServer.cpp`, `s_FieldServerMsg.cpp`, `s_FieldServerSession.cpp` | world/gameplay; the tree holds no ODBC user files |
| Session | `s_SessionServer.cpp`, `s_SessionServerMsg.cpp` | server-to-server coordination, channel state |

Message routing confirms the split: the LGIN range carries
`NET_MSG_REQ_GAME_SVR`/`NET_MSG_SND_GAME_SVR` (Login handing out servers) and
`NET_MSG_I_AM_AGENT` (`:2404`, "Agent -> Field"). The LOBBY range carries login,
character select and character join.

### Topology - INFERRED

```
CLIENT
  -> Login    (which servers/channels exist)
  -> Agent    (authenticate, character list, character select)
  -> Field    (enter world, gameplay)
  Session    (server-to-server coordination, not client-facing)
```

This matches the community description, which the brief asked be treated as
corroboration rather than authority. It is recorded as INFERRED because the
source proves the *message ownership* but the physical connection sequence was
not traced end to end in this milestone.

---

## 3. Wire format - CONFIRMED

### Header

`s_NetGlobal.h:2633-2637`:

```cpp
// Generic message
// 8 bytes
struct NET_MSG_GENERIC
{
    DWORD   dwSize;   // Size of Message Data
    EMNET_MSG nType;  // Message type
};
```

`RcvMsgBuffer.cpp:90-118` reads exactly this, and the header comment in
`SendMsgBuffer.h:19-25` documents the batch form:

```
MTU 1500
IPv6 : MTU - 60 = 1440 Bytes
IPv4 : MTU - 40 = 1460 Bytes
----------------------------------------------------------------------
| Size(4) | Type(4) | Compress(1) | Count(2) | Data(...)             |
----------------------------------------------------------------------
```

Two facts that are easy to get wrong and are therefore pinned by tests:

- **`dwSize` includes the header.** Every legacy constructor sets
  `nmg.dwSize = sizeof(StructName)`, and `RcvMsgBuffer.cpp:96` compares
  `m_nRcvSize < (int) pNmg->dwSize` to decide whether a whole message arrived.
- **`NET_COMPRESS`** (`:2811-2823`) is `NET_MSG_GENERIC` plus `bool bCompress`, the
  marker for a combined/compressed batch.

### Message id space - CONFIRMED

`s_NetGlobal.h:645-696`:

```cpp
#if defined(CH_PARAM)
    #define NET_MSG_BASE  992
#elif defined(HK_PARAM)
    #define NET_MSG_BASE  992
... (KR_PARAM, KRT_PARAM, MY_PARAM, MYE_PARAM, PH_PARAM, VN_PARAM,
     TW_PARAM, TH_PARAM, GS_PARAM, ID_PARAM, JP_PARAM - all 992)
#else
    #define NET_MSG_BASE  992
#endif

#define NET_MSG_LGIN        (NET_MSG_BASE +  450)   // 1442
#define NET_MSG_LOBBY       (NET_MSG_BASE +  950)   // 1942
#define NET_MSG_LOBBY_MAX   (NET_MSG_BASE + 1450)   // 2442
#define NET_MSG_GCTRL       (NET_MSG_BASE + 1900)   // 2892
```

**Every country branch assigns 992.** Message ids therefore do not vary by region,
which is worth stating precisely because it means the V026 country-macro
question does **not** reach the protocol constants. Pinned by
`Network_MessageIdsDoNotVaryByRegion`.

### Buffer sizes - CONFIRMED

`s_NetGlobal.h:99-118` and `SendMsgBuffer.h:36-38`:

| Constant | Value | Source | Meaning |
| --- | --- | --- | --- |
| `NET_DATA_BUFSIZE` | 2048 | `:105` | data buffer (LG-7 note: was 1024) |
| `NET_DATA_MSG_BUFSIZE` | 8192 | `:107` | client message buffer |
| `NET_DATA_CLIENT_MSG_BUFSIZE` | 16384 | `:111` | |
| `NET_MAX_CLIENT` | 1000 | `:135` | per-server client cap |
| `NET_TIME_OUT` | 180000 | `:117` | ms, i.e. 3 minutes |
| `NET_DEFAULT_PORT` | 5001 | `:99` | |
| `CSendMsgBuffer::BUFFER_SIZE` | 6144 | `SendMsgBuffer.h:36` | |
| `MAX_PACKET_SIZE` | 2048 | `:37` | |
| `COMPRESS_PACKET_SIZE` | 1000 | `:38` | flush threshold |
| `ENCRYPT_KEY` | 12 | `:205` | crypt key length |

`RcvMsgBuffer.cpp:112-118` is the reference validation, and its comment says
`//packet crash fix`:

```cpp
if (pNmg->dwSize == 0 || pNmg->dwSize > NET_DATA_BUFSIZE || pNmg->dwSize < sizeof(NET_MSG_GENERIC))
{
    resetPosition();
    return NULL;
}
```

### Transport - CONFIRMED

TCP over Winsock with **IOCP**. `CServer` (`s_CServer.h`) owns `m_pRecvIOCP`,
`m_pSendIOCP`, `MAX_WORKER_THREAD` handles, an accept thread and an update
thread, plus a `NET_DATA_BUFSIZE` receive buffer. `s_NetGlobal.h:120-122` states
outright: *"This version not support UDP protocol"*.

### Encryption - RESOLVED by VERTICAL-028

> **Superseded by V028.** V027 recorded this as *UNRESOLVED as to which applies* and cited
> `s_NetClient.cpp:1120-1128` as encrypting login data with Apex RSA. **That is incorrect.**
> Apex has no callers; the live crypt is minTea with a hardcoded static key. See
> `docs/reference/network/ASURA_NETWORK_PROTOCOL_AUTHORITY.md` §B.

Present in `Lib_Network`: `DaumGameCrypt.cpp/.h`, `minTea.cpp/.h` (TEA),
`ApexProxy.cpp/.h` (RSA), `gamecode.cpp`, `dhkey.cpp/.h` (**Diffie-Hellman key
exchange**), `des.cpp`. Handshake ids `NET_MSG_REQ_CRYT_KEY` (130) /
`NET_MSG_SND_CRYT_KEY` (140). `s_NetClient.cpp:1134` encrypts the heartbeat key
with TEA.

**Settled in V028:** minTea is active, with the compile-time constant key
`"Steven Seagal Neck Break"` (`minTea.cpp:16-24`) — confirmed present in all four ASURA
server binaries and `MiniA.exe`. `dhkey.*` (DH), `des.cpp`, and `DaumGameCrypt.cpp` have
**zero** callers in the tree. `CRYPT_KEY` is transmitted on accept but its consumer is
commented out. **No DH exchange completes before gameplay traffic, because none exists.**
What fills the "key exchange" role is a random 13-character passphrase generated by
`CLoginServer::GenerateEncrypt()` and pushed over the server backbone to Agent and client,
verified by plain string compare.

---

## 4. Client connection flow - PARTIALLY CONFIRMED

The login/auth/character-select stages are confirmed by message id and struct;
the physical connection sequence is INFERRED.

| Stage | Evidence | Tag |
| --- | --- | --- |
| version check | `NET_MSG_VERSION_REQ` (120) / `VERSION_OK` (100) / `VERSION_INFO` (110) | CONFIRMED |
| crypt key exchange | `REQ_CRYT_KEY` (130) / `SND_CRYT_KEY` (140), plus `NET_MSG_RANDOM_NUM` (141) | CONFIRMED |
| server/channel discovery | `NET_MSG_REQ_GAME_SVR` (LGIN+100) / `SND_GAME_SVR` (LGIN+110), `SND_FULL_SVR_INFO` (LGIN+140) | CONFIRMED |
| login | `NET_MSG_LOGIN_2` (LOBBY+107), `NET_MSG_LOGIN_FB` (LOBBY+108), `DAUM_NET_MSG_LOGIN` (LOBBY+110), `CHINA_`/`GSP_`/`TERRA_` variants (LOBBY+113-121) | CONFIRMED |
| character list | `NET_MSG_REQ_CHA_BINFO` (LOBBY+302), `NET_MSG_CHA_BAINFO` (LOBBY+306) | CONFIRMED |
| character select | `NET_MSG_LOBBY_CHAR_SEL` (LOBBY+390) | CONFIRMED |
| character join | `NET_MSG_LOBBY_CHAR_JOIN` (LOBBY+391), `..._JOIN_FB` (LOBBY+393) | CONFIRMED |
| enter world | `NET_MSG_CONNECT_CLIENT_FIELD` (LOBBY+416), `NET_MSG_JOIN_FIELD_IDENTITY` (LOBBY+417), `NET_MSG_AGENT_REQ_JOIN` (LOBBY+422), `NET_MSG_FIELD_REQ_JOIN` (LOBBY+423) | CONFIRMED |
| heartbeat | `NET_MSG_HEARTBEAT_CLIENT_REQ/ANS` (160/161) | CONFIRMED |

`CNetClient` exposes the stage-shaped API (`s_NetClient.h:186-191`):
`ConnectLoginServer`, `ConnectFieldServer`, `ConnectBoardServer`,
`ConnectAgentServer`, `ConnectGameServer`, `CloseConnect`.

**The login range carries per-platform login message variants.** V027 listed four
(`DAUM_`, `CHINA_`, `GSP_`, `TERRA_`) and inferred that `DAUM_` would be the Korean path
under V026's `KR_PARAM` authority.

> **Corrected by V028:** that inference is wrong. `KR_PARAM` does not select a login
> variant at all. The Agent dispatches **nine** variants at runtime on `nType` with no
> `#if` gating (`s_CAgentServerMsg.cpp:61-71`) — adding `THAI_`, `EXCITE_`, `JAPAN_`, `GS_`
> to V027's four. A compatible server must accept all nine. Which variant the ASURA
> **client** sends remains **UNRESOLVED** (V028 Unresolved item 1). Does not affect the
> framing foundation.

---

## 5. Enter world, movement, combat - INVESTIGATED, NOT IMPLEMENTED

Recorded for the future seams, as the brief requires.

- **Movement** (`WORLD-002`): message ids exist in the GCTRL range
  (`NET_MSG_GCTRL = 2892`), but the sync protocol was **not traced** in this
  milestone. UNRESOLVED by design, not by omission.
- **Combat**: legacy sends attack/skill requests and receives damage events.
  The modern direction is fixed and is not legacy's:

  ```
  CLIENT REQUEST -> SERVER VALIDATION -> MODERN CORE COMBAT
                 -> AUTHORITATIVE RESULT -> NETWORK EVENT -> CLIENT PRESENTATION
  ```

  Modern Core remains the combat authority; the network layer transports
  requests and results and never computes damage.

No movement or combat networking was implemented.

---

## 6. Modern boundary - IMPLEMENTED

New target `ModernNetwork`, depending on `Modern` and never the reverse:

```
core  <-  network / database / server / client
core  X   legacy
```

`modern/core/CMakeLists.txt` states core "must stay free of Windows UI, DirectX,
sockets, PostgreSQL and any legacy RAN type". That rule is why the network layer
is a separate target rather than a directory inside core, and why
`ModernNetworkTests` is a separate executable rather than more files in
`ModernCoreTests` - loosening `ModernCoreTests` to reach one header would have
weakened a rule the whole project depends on.

```
                    ┌────────────────────┐
                    │     MODERN CLIENT  │
                    └─────────┬──────────┘
                              │
                         Network
                              │
                    ┌─────────▼──────────┐
                    │   MODERN SERVER    │
                    │  Network Boundary  │
                    │  Session           │
                    │  Message Routing   │
                    └─────────┬──────────┘
                              │
                    ┌─────────▼──────────┐
                    │    MODERN CORE     │
                    └────────────────────┘
```

### Layer responsibilities

| Layer | File | Owns | Must not know |
| --- | --- | --- | --- |
| Transport | `NetworkTransport.h`, `LoopbackTransport.h` | bytes between endpoints, state, disconnect | messages, framing, sessions, gameplay |
| Framing | `NetworkConnection.h` | message boundaries, capacity, resync | payload meaning, routing, gameplay |
| Codec | `NetworkCodec.h` | encode, decode, **validation** | routing, sessions, gameplay |
| Routing | `MessageRouter.h` | message id -> handler, per owner | transport, framing, gameplay |
| Session | `ServerSession.h` | lifecycle, authorisation, liveness | gameplay rules |

`NetworkTransport.h` names what it must not know: Character, Stats, Equipment,
Skills, Damage, HP, MP, SP. Handlers receive a decoded `Message`, never a raw
buffer, so a handler cannot read past the end of a packet and the validation
boundary has already run before gameplay code is reached.

### Two decisions worth defending

**1. Explicit little-endian encoding, not struct memcpy.** Legacy declares
`NET_MSG_GENERIC` as a native struct and copies it wholesale. On x86 that is
little-endian with a 1-byte MSVC `bool`, so the wire bytes happen to match. The
modern codec encodes field by field with fixed-width types. **The bytes are
identical on x86 - wire compatibility is preserved - but the guarantee is now
explicit and testable rather than a property of one compiler.** Pinned byte by
byte by `Codec_IntegersAreLittleEndianOnTheWire`.

**2. Fixed-width wire types, no platform `int`.** `WireU8`..`WireI64` in
`NetworkTypes.h`. Nothing that reaches the wire uses `int` or `long`.

---

## 7. Security / validation foundation

Implemented in `Codec::DecodeMessage` and `ConnectionFramer`:

| Threat | Where refused | Test |
| --- | --- | --- |
| size below header | `DecodeMessage` | `Codec_DecodeRejectsSizeBelowHeader` |
| size zero | `DecodeMessage` | `Codec_DecodeRejectsZeroSize` |
| size above `kMaxPacketSize` | `DecodeMessage` | `Codec_DecodeRejectsOversizedSize` |
| truncated message | `DecodeMessage` | `Codec_DecodeRejectsTruncatedMessage` |
| **zero message id** | `DecodeMessage` | `Codec_DecodeRejectsZeroType` |
| truncated payload | `Reader` | `Codec_TruncatedPayloadIsRefusedNotOverread` |
| string length overrun | `Reader` | `Codec_StringLengthOverrunIsRefused` |
| array count beyond buffer | `Reader` | `Codec_ArrayCountBeyondBufferIsRefused` |
| oversized string at encode | `WriteString` | `Codec_OversizedStringIsRefusedAtEncode` |
| unbounded buffer growth | `ConnectionFramer::Feed` | `Framing_OversizedFeedIsRefusedNotTruncated` |
| desynchronised stream | framer **latches** failure | `Framing_InvalidLengthLatchesFailure` |

The zero-id check is **modern hardening** that legacy does not perform, and is
labelled as such at both the code and the test. The latch is the important one:
once a header cannot be valid, the stream position is no longer trustworthy, so
the framer refuses to keep parsing rather than guessing where to resynchronise.

Anti-cheat is out of scope. The rule is the structural one: **the client is a
request, never an authoritative state command**, and gameplay lives in Core.

---

## 8. Tests

47 new, all deterministic. No socket, no clock, no RNG, no server cast
comparisons.

| Area | Count | Notable |
| --- | --- | --- |
| Protocol constants | 3 | transcription from legacy, so a "tidy-up" cannot pass silently |
| Codec | 16 | round trip, byte order, truncation, overrun, all five decode rejections |
| Framing | 6 | one-at-a-time delivery, header/body split, latch, oversized |
| Transport | 4 | pair, partial receive, empty receive, disconnect |
| End to end | 1 | bytes -> transport -> framer in uneven chunks |
| Router | 6 | routing, per-owner scoping, unknown id, duplicate, clear |
| Session | 11 | full path, every authorisation gate, rejection, timeout |

The byte-at-a-time and header/body-split framing tests are the ones that earn
their keep: a real socket decides its own chunking, and a framer that only works
when a whole message arrives at once is the single most common framing defect.
`LoopbackTransport` exists so that case is reachable.

`Session_EnterWorldRequiresASelectedCharacter` encodes a real judgement: a join
without a selection closes the session rather than being merely refused, because
keeping a half-entered world is worse than dropping the connection.

---

## 9. Two bugs found and fixed during implementation

Recorded because both were silent.

**1. Dangling peer pointers in `LoopbackTransport`.** The first version held a
raw `LoopbackTransport* m_peer`, set inside `CreatePair`. `CreatePair` returns by
value, so the returned halves are copies whose back-pointers still referenced
locals that died at the return. Symptom: immediate access violation with **no
output at all**. Fixed by having both halves share one heap channel.

**2. Transport direction was wrong.** After the first fix, `Send` wrote to the
sending half's own outbound queue while `Receive` read the same one, so bytes
never crossed. Three transport tests failed with all zeros. Fixed by
distinguishing `Outbound` (where this half writes) from `Inbound` (where the peer
wrote) - which is the distinction the original single-queue shortcut hid.

Neither was caught by inspection; both were caught by tests that asserted
something observable.

---

## 10. Files changed

| File | Why |
| --- | --- |
| `modern/network/NetworkTypes.h` | Wire primitives, protocol constants with legacy citations, `MessageHeader`, server roles, handshake ids |
| `modern/network/NetworkTypes.cpp` | `ToString(SessionState)` - a log that cannot tell two states apart is worse than no log |
| `modern/network/NetworkCodec.h` | Explicit LE encode/decode, bounds-checked `Reader`, message validation - the security boundary |
| `modern/network/NetworkConnection.h` | `ConnectionFramer` - message boundaries, capacity bound, failure latch |
| `modern/network/NetworkTransport.h` | `INetworkTransport` interface + `Endpoint`; names what it must not know |
| `modern/network/LoopbackTransport.h` | In-memory transport pair; enables deterministic framing tests and single-process harnesses |
| `modern/network/MessageRouter.h` | Per-owner id -> handler; unknown ids ignored, duplicates refused |
| `modern/network/ServerSession.h` | Lifecycle + authorisation + injected-clock liveness |
| `modern/network/CMakeLists.txt` | New target, links Modern only |
| `modern/CMakeLists.txt` | Registers the new subdirectory |
| `modern/tests/NetworkTests.cpp` | 47 tests + `main` |
| `modern/tests/CMakeLists.txt` | New `ModernNetworkTests` executable and CTest entry |
| `docs/MODERNIZATION_STATUS.md` | V027 row, summary, next-task pointer |

**Not changed:** `legacy/` (nothing), `modern/core/`, `modern/server/`,
`modern/client/`, all combat/stat/skill/resource code, ASURA.

---

## 11. Remaining work

| Milestone | Scope |
| --- | --- |
| **WORLD-001** | Login -> character list -> character select -> enter world -> spawn, over this boundary |
| **WORLD-002** | Movement, server validation, synchronisation. **Needs the legacy movement trace first - not done in V027.** |
| **COMBAT-ONLINE-001** | Client request -> server validation -> Core combat -> HP -> client |
| **INVENTORY-001** | Inventory -> equipment -> stats -> client |
| **PERSISTENCE-001** | Account / character / inventory persistence |

Before any wire-compatible client can be written, three **UNRESOLVED** items
must be settled, and all three are investigation, not code:

1. **Which platform login variant is active** for the ASURA/PH deployment
   (`DAUM_`, `CHINA_`, `GSP_`, `TERRA_`, or `NET_MSG_LOGIN_2`).
2. **Which crypt is active** and whether the DH exchange completes before
   gameplay traffic.
3. **Whether `NET_MSG_COMPRESS` batching is negotiated or unconditional** -
   `bCompress` distinguishes a combined batch from a compressed one, and V027's
   framer treats every message as a single unit.

The third is the one that could change the framing layer, which is why it is
recorded rather than deferred silently.

---

## RESOLVED by VERTICAL-028 — read this before using the framing layer

All three items above were settled in `docs/reference/network/ASURA_NETWORK_PROTOCOL_AUTHORITY.md`.
V027's framing section is **incomplete**, not wrong, and item 3 did change the picture:

1. **Login variant is not a macro.** `KR_PARAM` does not select it. The Agent dispatches
   **nine** variants at runtime on `nType`, ungated
   (`s_CAgentServerMsg.cpp:61-71`). A server must accept all nine. Which one the ASURA
   *client* sends is still **UNRESOLVED** (V028 Unresolved item 1).
2. **Crypt is minTea with the hardcoded static key `"Steven Seagal Neck Break"`**
   (`minTea.cpp:16-24`), confirmed present in all four ASURA server binaries *and*
   `MiniA.exe`. **No Diffie-Hellman** - `dhkey.*` has zero call sites. `CRYPT_KEY` is sent on
   accept but the code consuming it is commented out. `szEnCrypt` is a pre-shared random
   passphrase pushed by the Login server over the backbone, verified by string compare.
3. **`NET_MSG_COMPRESS` is unconditional and the wire format is TWO-LEVEL.** An outer
   `NET_COMPRESS` envelope (`nType` 170, 12 bytes) wraps an LZO-compressed **batch** of
   complete messages, each carrying its own `dwSize`. The outer header is not a dispatched
   message and its payload is not one message. On LZO failure the batch is still wrapped,
   with `bCompress = false`.

**Consequence for `ConnectionFramer`:** it correctly handles the *inner* message stream and
its header validation matches the legacy `packet crash fix` guards, but it has no envelope
stage. It is referenced only by `modern/tests/NetworkTests.cpp`, so this is a latent gap
rather than a live defect; all its tests remain truthful. Before WORLD-001 talks to a real
client it needs (a) an envelope stage that unwraps/decompresses, and (b) a vendored minilzo
- a dependency decision, deliberately deferred rather than guessed.
