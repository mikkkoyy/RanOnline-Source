# LOGIN-001 - Modern Login Server Connection & Game Server List

Baseline: `b6ce32b7d46d61f9425db5f5e4dae4b8fec9be36` ("WORLD-002 modern server login response")

## Result in one line

The pre-login Login Server phase is implemented as a **separate connection and
separate state machine** from the WORLD-001/WORLD-002 Agent login, and its three
messages are implemented exactly as the legacy source proves them - including one
finding that **contradicts** the expectation WORLD-002 carried forward: **the
game-server list is NOT compressed.**

---

## 0. Evidence grade

| | Claim |
| --- | --- |
| **PROVEN** | Read directly from local legacy source; the file and line are cited for every claim below. |
| **INFERRED** | A conclusion drawn from proven facts, with the reasoning stated. |
| **UNKNOWN** | Not determinable from available material. |

Priority used throughout, as required:

```
LOCAL LEGACY SOURCE > BINARY EVIDENCE > PUBLIC SOURCE/FORUM > ASSUMPTION
```

Public/forum material (RaGEZONE, GitLab mirrors of RAN sources) corroborated only
the **topology** - that RAN ships a distinct Login server binary with its own
`svr_login_0.ini`, separate from Agent and Session. It provided **nothing** at field
level. No claim below rests on it.

---

## 1. Connection: Client -> Login Server

### 1.1 PROVEN - one socket, two sequential roles

This is the single most important connection fact, and it refines the architecture
diagram: **legacy does not hold two sockets open.**

```cpp
// s_NetClient.cpp:367-372
int CNetClient::ConnectLoginServer( const char *szAddress, int nPort)
{
    if (m_nOnline == NET_ONLINE)
        CloseConnect();
    return ConnectServer(szAddress, nPort, NET_STATE_LOGIN);
}

// s_NetClient.cpp:374-388
int CNetClient::ConnectAgentServer(int nServerGroup, int nServerNumber)
{
    return ConnectGameServer(nServerGroup, nServerNumber);
}

int CNetClient::ConnectGameServer(int nServerGroup, int nServerNumber)
{
    if (m_nOnline == NET_ONLINE)
        CloseConnect();
    return ConnectServer(m_sGame[nServerGroup][nServerNumber].szServerIP,
                         m_sGame[nServerGroup][nServerNumber].nServicePort,
                         NET_STATE_AGENT);
}
```

So the flow is **close, re-point, reconnect** on the same `CNetClient`. The two
servers are two sequential *roles*, not two simultaneous connections. The client
keeps separate `CNetClient` instances only for Field (`m_pNetField`) and Board
(`m_pNetBoard`), not for Agent.

The two conversations are nonetheless kept rigorously apart, and this is what the
milestone implements:

```cpp
// s_NetClientMsg.cpp:19-44
switch (m_nClientNetState)
{
case NET_STATE_LOGIN : MessageProcessLogin(pNmg); break;   // 1
case NET_STATE_AGENT : // 2
case NET_STATE_FIELD : // 3
case NET_STATE_BOARD : MessageProcessGame(pNmg);  break;   // 4
case NET_STATE_CTRL  : break;                              // 5
}
```

`NET_STATE_LOGIN` (1) routes to `MessageProcessLogin`, which handles
`SND_GAME_SVR`, `SND_GAME_SVR_END`, `VERSION_INFO` and `ENCRYPT_KEY`.
`NET_STATE_AGENT` (2) routes to `MessageProcessGame`. The two share **no message
id**. `Modern::Client::LoginServerClient` and its `LoginServerPhase` are therefore
deliberately **separate types** from `LoginResponseClient` and `LoginPhase`.

### 1.2 PROVEN - endpoint, port, configuration

| Item | Value | Evidence |
| --- | --- | --- |
| Default address | `211.172.252.50` | `RANPARAM.cpp:144` |
| Address buffer | `TCHAR LoginAddress[128]` (MBCS, so 128 bytes) | `RANPARAM.cpp:144` |
| Default port | `5001` | `RANPARAM.cpp:146` |
| Config file | `param.ini` | `RANPARAM_MAIN.cpp:55` |
| Keys | `[SERVER SET] LoginAddress`, `[SERVER SET] nLoginPort` | `RANPARAM_MAIN.cpp:68-69` |
| Per-region override | `China_Region[i].LoginAddress` / `.nLoginPort` | `RANPARAM_MAIN.cpp:97-98`, `RANPARAM.h:75,80` |
| Reconnect | `CloseConnect()` before every connect | `s_NetClient.cpp:367-372` |
| Reuse | One connection is closed before the next role begins | `s_NetClient.cpp:379-382` |

### 1.3 PROVEN - hostnames are NOT supported

```cpp
// s_NetClient.cpp:436-465 - the gethostbyname branch is COMMENTED OUT
// s_NetClient.cpp:474
m_Addr.sin_addr.s_addr = ::inet_addr(szServerIP);
```

`inet_addr` does not resolve names; a hostname yields `INADDR_NONE` and the connect
fails silently. The modern `EndpointAddress::IsNumericIPv4` encodes this as a
validation rule, turning a silent misconnection into a reported one.

### 1.4 PROVEN - no encryption, no compression on this connection

`ConnectLoginServer` uses the three-argument `ConnectServer`, with **no**
`CRYPT_KEY`. Compare `ConnectFieldServer` / `ConnectBoardServer`, which pass `m_ck`
(`s_NetClient.cpp:395-418`). No `m_Tea` call appears anywhere in the list exchange.

---

## 2. Request: `NET_MSG_REQ_GAME_SVR` = 1542

### 2.1 PROVEN - the id

```cpp
// s_NetGlobal.h:675
#define NET_MSG_LGIN   (NET_MSG_BASE + 507)   // 1442
// s_NetGlobal.h:734
NET_MSG_REQ_GAME_SVR = (NET_MSG_LGIN + 100),  // -> 1542
```

Modern derives it from the existing `Protocol::kLoginBase` rather than restating the
number, with a `static_assert` pinning the value.

### 2.2 PROVEN - the request is a bare 8-byte header, no body

```cpp
// s_NetClientMsg.cpp:314-334
int CNetClient::SndReqServerInfo(void)
{
    for (int i=0; i<MAX_SERVER_GROUP; i++)
        for (int j=0; j<MAX_SERVER_NUMBER; j++)
            ::SecureZeroMemory(&m_sGame[i][j], sizeof(G_SERVER_CUR_INFO_LOGIN));
    m_bGameServerInfoEnd = FALSE;

    int nSize;
    NET_MSG_GENERIC nmg;
    nSize   = sizeof(NET_MSG_GENERIC);        // 8
    nmg.dwSize = (DWORD) nSize;
    nmg.nType = NET_MSG_REQ_GAME_SVR;
    return Send( (char *) &nmg, nSize );      // exactly 8 bytes
}
```

WORLD-002's finding is **confirmed from source**. There is no body, and nothing is
invented.

Two side effects are part of the request's meaning and are reproduced in
`LoginServerClient::RequestGameServers`: it **clears the grid** and **clears the END
flag** *before* sending. A second request therefore cannot be answered by the
previous list.

### 2.3 PROVEN - raw

`Send()` is the client's plain transport send. No envelope, no compression.

---

## 3. Response: `NET_MSG_SND_GAME_SVR` = 1552

### 3.1 PROVEN - every field of `NET_CUR_INFO_LOGIN`

```cpp
// s_NetGlobal.h:567-588
struct G_SERVER_CUR_INFO_LOGIN
{
    char szServerIP[MAX_IP_LENGTH+1]; ///< Server IP Address
    int nServicePort;            ///< Server Port
    int nServerGroup;            ///< Server Group Number
    int nServerNumber;           ///< Channel Number or Server Number
    int nServerCurrentClient;    ///< Channel Current Client
    int nServerMaxClient;        ///< Channel Max Client
    bool bPK;                    ///< Channel PK information
    ...
};
// s_NetGlobal.h:3896-3905
struct NET_CUR_INFO_LOGIN
{
    NET_MSG_GENERIC      nmg;
    G_SERVER_CUR_INFO_LOGIN gscil;
    NET_CUR_INFO_LOGIN() { nmg.nType = EMNET_MSG(0); nmg.dwSize = sizeof(NET_CUR_INFO_LOGIN); }
};
```

Every field name comes from legacy's own comment. **No `unknown0`/`unknown1` fields
were needed** - the struct is fully self-documenting.

Constants: `MAX_IP_LENGTH = 20` (`s_NetGlobal.h:146`).

### 3.2 PROVEN - layout, compiler-verified

`sizeof` and every offset were measured with a throwaway MSVC probe compiled as
x86 against verbatim copies of the declarations - **not** inferred from a diagram
and not from the modern compiler's layout.

| Offset | Size | Field | Type on wire |
| --- | --- | --- | --- |
| 0 | 4 | `dwSize` = 56 | unsigned |
| 4 | 4 | `nType` = 1552 | unsigned |
| 8 | 21 | `szServerIP` | `char[21]`, MBCS, NUL-terminated |
| 29 | 3 | *padding* | alignment: next `int` needs offset 32 |
| 32 | 4 | `nServicePort` | **signed** `int` |
| 36 | 4 | `nServerGroup` | **signed** `int` |
| 40 | 4 | `nServerNumber` | **signed** `int` |
| 44 | 4 | `nServerCurrentClient` | **signed** `int` |
| 48 | 4 | `nServerMaxClient` | **signed** `int` |
| 52 | 1 | `bPK` | `bool`, MSVC 1 byte |
| 53 | 3 | *padding* | to 4-byte alignment |

```
sizeof(NET_MSG_GENERIC)         = 8
sizeof(G_SERVER_CUR_INFO_LOGIN) = 48
sizeof(NET_CUR_INFO_LOGIN)      = 56      <- the wire entry size
```

**PROVEN - all five integers are signed.** Reading them unsigned would turn a
negative load figure into ~4 billion; the modern type is `WireI32` and a test
asserts negative round-trips.

**PROVEN - there is no server NAME field.** The list identifies a server by IP and
port alone. The brief asked about a server name; the honest answer is that the wire
does not carry one. A display name is a separate client-side concern.

### 3.3 PROVEN - padding is indeterminate in legacy, and zeroed here

`NET_CUR_INFO_LOGIN ncil;` at `s_CLoginServerMsg.cpp:109` is an **uninitialised
stack local**, and its constructor sets only `nmg.nType` and `nmg.dwSize`. Bytes
29-31 and 53-55 therefore carry stack residue.

The modern codec zero-fills them. This is a deliberate, documented **superset**:
every reader ignores those bytes, and determinism is what makes the frame testable.
Same decision, and same reasoning, as WORLD-002's 120-byte `NET_LOGIN_FEEDBACK_DATA`.
A test asserts the frame is byte-identical across builds.

### 3.4 String handling - explicitly checked, not assumed

WORLD-002 proved that apparently similar RAN string fields can have different
effective limits, so the `StringCchCopy` call sites were checked individually
rather than applying a generic rule.

Here the answer is clean, and it is *not* `fieldSize - 1` by accident:

- The field is `char[MAX_IP_LENGTH+1]` = 21 bytes, and the server fills it by
  plain `std::string` assignment (`ncil.gscil = m_sGame[i][j]`), not through
  `StringCchCopy` with a surprising capacity argument.
- Therefore the correct limit is **20 characters plus a terminator**, and the modern
  codec **refuses** a 21-character address rather than truncating it - truncation
  would hand the player a different server than intended.
- Unused bytes are zero.
- On decode, a field with **no NUL in 21 bytes is rejected**. Legacy reads these
  with C-string functions and would run past the end.
- Encoding note: a dotted-quad IPv4 is pure ASCII, so MBCS-vs-UTF-8 is not a live
  concern *for this specific field*. That is stated rather than assumed, and the
  general MBCS policy remains as documented in WORLD-001/WORLD-002.

### 3.5 PROVEN - the server-side filter

```cpp
// s_CLoginServerMsg.cpp:119-134
for (int i=0; i<MAX_SERVER_GROUP; ++i)
    for (int j=0; j<MAX_CHANNEL_NUMBER; ++j)
        if (m_sGame[i][j].nServerMaxClient > 0)
        {
            ncil.nmg.dwSize = sizeof(NET_CUR_INFO_LOGIN);
            ncil.nmg.nType = NET_MSG_SND_GAME_SVR;
            ncil.gscil     = m_sGame[i][j];
            SendClient(dwClient, (char*) &ncil);
            dwCount++;
        }
```

**Only servers advertising `nServerMaxClient > 0` are offered.** The modern
responder applies exactly this filter and counts the skips so a misconfigured
fixture is visible rather than mysteriously short.

---

## 4. Terminator: `NET_MSG_SND_GAME_SVR_END` = 1562

### 4.1 PROVEN - bare 8-byte message with **no count**

```cpp
// s_CLoginServerMsg.cpp:141-144
NET_MSG_GENERIC nmg;
nmg.dwSize = sizeof(NET_MSG_GENERIC);   // 8
nmg.nType = NET_MSG_SND_GAME_SVR_END;
SendClient(dwClient, (char*) &nmg);
```

There is no count, no length and no body. **The client cannot know the list length
in advance.**

### 4.2 PROVEN - the terminator is sent UNCONDITIONALLY, including for an empty list

This was the behaviour most likely to be assumed wrong, so it is settled outright by
the source:

```cpp
// s_CLoginServerMsg.cpp:136-144
if (dwCount == 0)
{
    CConsoleMessage::GetInstance()->Write(_T("ERROR:Check Session Server Connection"));
}

NET_MSG_GENERIC nmg;
nmg.dwSize = sizeof(NET_MSG_GENERIC);
nmg.nType = NET_MSG_SND_GAME_SVR_END;
SendClient(dwClient, (char*) &nmg);      // sent even when dwCount == 0
```

An empty list is therefore **`SND_GAME_SVR_END` alone** - not silence. Silence
would strand any client that completes on the terminator. A test asserts this exact
8-byte response for an empty fixture.

### 4.3 PROVEN - client completion

```cpp
// s_NetClientMsg.cpp:183-186
void CNetClient::MsgGameSvrInfoEnd(NET_MSG_GENERIC* nmg)
{
    m_bGameServerInfoEnd = TRUE;
}
```

The terminator is the only completion signal. The modern client therefore enters
`GameServersReady` **only** on the terminator - a test feeds all entries without the
terminator and asserts the client is still not complete.

---

## 5. Compression - the finding that contradicts WORLD-002

### 5.1 PROVEN - the game-server list is NOT compressed

WORLD-002 recorded an assumption that the Login Server's responses are "batched
through the established server-to-client compression path". **That assumption is
wrong, and this milestone corrects it.**

The Agent's response *is* compressed:

```cpp
// s_CAgentServer.cpp:712
return m_pClientManager->SendClient(dwClient, pBuffer);   // -> batching

// s_CClientManager.cpp:437-487
int CClientManager::SendClient(DWORD dwClient, LPVOID pBuffer)
{
    ...
    int nResult = m_pClient[dwClient].addSendMsg(pNmg, dwSendSize);   // CSendMsgBuffer
    ... SendClient2(dwClient, m_pClient[dwClient].getSendBuffer());  // flushed envelope
}
```

The Login Server's is **not**:

```cpp
// s_CLoginServer.cpp:823-828  <-- what MsgSndGameSvrInfo actually calls
int CLoginServer::SendClient(DWORD dwClient, LPVOID pBuffer)
{
    if (pBuffer == NULL) return NET_ERROR;
    return m_pClientManager->SendClient2(dwClient, pBuffer);   // NOT SendClient
}

// s_CClientManager.cpp:489-527
int CClientManager::SendClient2(DWORD dwClient, LPVOID pBuffer)
{
    ...
    pNmg = (NET_MSG_GENERIC*) pBuffer;
    dwSndBytes = pNmg->dwSize;
    CopyMemory( pIoWrite->Buffer, pNmg, dwSndBytes );   // raw copy, no envelope
    return SendClient2( dwClient, pIoWrite, dwSndBytes );
}
```

Three independent confirmations that no compression layer is involved:

1. `CLoginServer::m_pClientManager` is a `CClientLogin*` (`s_CLoginServer.h:46`),
   and `class CClientLogin : public CClientManager` (`s_CClientLogin.h:26`)
   **does not override `SendClient2`** - grep for `SendClient` in
   `s_CClientLogin.{h,cpp}` returns nothing.
2. `addSendMsg` - the only writer of compressed output - is reached **exclusively**
   from `CClientManager::SendClient`. The Login Server never calls it.
3. `SendMsgBuffer.cpp` (which builds the `NET_COMPRESS` envelope at :146-159) is
   reachable only through `addSendMsg`.

### 5.2 Consequence for this implementation

The list travels **raw**: a plain concatenation of `NET_MSG_GENERIC` frames on one
TCP stream - exactly like the WORLD-001 login request travels raw in the other
direction.

This milestone therefore **reuses V030's framing and deliberately does NOT reuse
its compression.** Routing these messages through `NetCompress` /
`ServerBatchEncoder` would produce bytes no RAN client accepts. The choice is
asserted by tests on both sides: the responder's first frame must be
`SND_GAME_SVR` and not `NET_MSG_COMPRESS`, and the client's parse uses the same
`ConnectionFramer` the WORLD-001 request uses.

No compression or framing code was duplicated. `ConnectionFramer` is reused as-is,
with one added accessor (`Capacity()`) so a caller can feed in bounded slices.

### 5.3 One scaling consequence, found and fixed

`ConnectionFramer`'s capacity (2048) bounds **buffered partial data**, not stream
length - but `Feed()` refuses a single chunk larger than capacity. A full 200-entry
response is **11208 bytes**, so feeding it in one call would be wrongly refused.
`LoginServerClient::Feed` therefore feeds in slices bounded by the room actually
left, draining between them. A test delivers all 200 entries in one burst.

---

## 6. Data structure: the list is a grid, not a sequence

### 6.1 PROVEN

```cpp
// client storage
G_SERVER_CUR_INFO_LOGIN m_sGame[MAX_SERVER_GROUP][MAX_CHANNEL_NUMBER];  // s_NetClient.h:153
// server storage
G_SERVER_CUR_INFO_LOGIN m_sGame[MAX_SERVER_GROUP][MAX_CHANNEL_NUMBER];  // s_CLoginServer.h:50

// client receive
// s_NetClientMsg.cpp:192-207
if ((ncil->gscil.nServerGroup >= MAX_SERVER_GROUP) ||
    (ncil->gscil.nServerNumber >= MAX_CHANNEL_NUMBER))
{
    return;                                  // (1) dropped, silently
}
else
{
    m_sGame[ncil->gscil.nServerGroup][ncil->gscil.nServerNumber] = ncil->gscil;  // (2)
}
```

Limits: `MAX_SERVER_GROUP = 20`, `MAX_CHANNEL_NUMBER = 10`
(`s_NetGlobal.h:143,150`) -> **maximum 200 entries**, a derived figure rather than an
invented one.

Three consequences the modern `GameServerGrid` reproduces rather than smooths over:

1. An entry whose group or number is out of range is **dropped individually and
   silently**; one bad entry does not abort the list.
2. Two entries at the same coordinates **collide; the last wins**.
3. Order is **grid order** - group ascending, then channel ascending - which is
   exactly the server's nested loop. Not arrival order, not sorted.

A plain `std::vector` would get all three wrong, which is why this is modelled as a
grid. The distinction between (1) and corruption is preserved too: a well-formed
entry that cannot be placed is **counted and skipped**, while bytes that do not
decode are fatal.

### 6.2 Cosmetic legacy inconsistency (recorded, not a bug)

`s_NetClient.h:153` declares the array with `MAX_CHANNEL_NUMBER`, but
`s_NetClientMsg.cpp:319` clears it with `MAX_SERVER_NUMBER`. Both are `10`
(`s_NetGlobal.h:144,150`), so the spellings agree in value. It would become a real
bug if either constant changed.

### 6.3 INFERRED - ordering must be preserved, not normalised

Because the server's send order is the grid loop and the client's storage is the
grid, iterating the grid reproduces the send order exactly. The modern responder
emits in that order and the client's `Servers()` returns it in that order. No
sorting by name or id is performed anywhere, because legacy does not.

---

## 7. Framing and connection lifecycle

### 7.1 PROVEN - framing

`MSG_LIST` (`s_CSMsgList.h:14-31`) holds **one** `NET_MSG_GENERIC` in a
`NET_DATA_BUFSIZE` buffer, zeroed by its constructor. There is no envelope on this
path. Message boundaries come from each frame's own `dwSize`, which is precisely
what `ConnectionFramer` implements - so it is reused, and no second framing parser
was written.

### 7.2 Lifecycle as implemented

| Step | Legacy | Modern |
| --- | --- | --- |
| connect | `ConnectLoginServer(addr, port)` after `CloseConnect()` | `BeginConnect(endpoint)` -> `CompleteConnect()` |
| request | `SndReqServerInfo()` - clears grid, sends 8 bytes | `RequestGameServers(request)` - clears grid, builds 8 bytes |
| receive | `MsgGameSvrInfo` / `MsgGameSvrInfoEnd` | `Feed(...)` -> `GameServersReady` |
| complete | `m_bGameServerInfoEnd = TRUE` | `IsComplete()` |
| disconnect | `CloseConnect()` before the next role | `Disconnect()` -> `Disconnected` |

The connect is split in two on purpose: the socket outcome is the transport's to
report, and collapsing the two would let the client imply a connection it never
made.

### 7.3 Where the endpoint comes from, and the limitation

**PROVEN:** `RANPARAM::LoginAddress` / `RANPARAM::nLoginPort`, defaulting to
`211.172.252.50:5001`, read from `[SERVER SET]` in `param.ini`.

**BLOCKED - the real ASURA endpoint is unavailable.** The shipped
`ASURA CLIENT\param.ini` is encrypted: its first bytes are `08 00 00 00` followed by
high-entropy data, and a search for the literal `LoginAddress` finds nothing. The
compiled-in defaults are therefore used as the test fixture, and no endpoint
configuration was invented.

---

## 8. Live integration: BLOCKED

**Not claimed as PASS.** Four independent blockers, any one of which is sufficient:

1. **modern contains no socket code at all.** No winsock, no `<winsock2.h>`, no
   `socket()` call anywhere in `modern/`. The only transport is
   `LoopbackTransport`, an in-process pair of byte queues. A real loopback-socket
   integration is not writable until a socket transport exists.
2. **No modern Login Server process exists.** `LoginServerResponder` is a library
   component, not a server binary; there is no listening endpoint to connect to.
3. **RAN's `param.ini` is encrypted**, so the real endpoint cannot be read (§7.3).
4. **A legacy RAN Login Server could not be stood up either.** It populates
   `m_sGame` from a live Session Server via `SessionReqSvrInfo()`
   (`s_CLoginServerMsg.cpp:112-117`), and needs `svr_login_0.ini` plus Session,
   Agent and Field processes. Not available here.

What *was* done instead, and is reported as such:
`LoginServer_ExchangeOverTheTransportAbstraction` drives both halves through
`INetworkTransport` rather than by handing bytes across a function call - draining
the response in 64-byte transport chunks, which also exercises the client's
slicing. It is an integration test, and it is honestly **not** a TCP test.

Per §27, no gameplay testing was performed, and `MiniA.exe` was not used.

---

## 9. What was implemented

| File | Role |
| --- | --- |
| `modern/network/GameServerListProtocol.h` | ids, layout, offsets, `GameServerInfo`, `GameServerGrid`, `EndpointAddress` |
| `modern/network/GameServerListProtocol.cpp` | explicit little-endian codec; grid semantics; IPv4 validation |
| `modern/client/login/LoginServerClient.h` | `LoginServerPhase`, connect/request/receive/disconnect |
| `modern/client/login/LoginServerClient.cpp` | sliced feeding, entry placement, terminator handling |
| `modern/server/login/LoginServerResponder.h` | sibling of `LoginResponder`; grid-order filtered response |
| `modern/server/login/LoginServerResponder.cpp` | entries then unconditional terminator |
| `modern/tests/GameServerListProtocolTests.cpp` | ids, geometry, fields, malformed input, grid |
| `modern/server/LoginServerTests.cpp` | phases, entry counts, ordering, fragmentation, E2E, transport |

### 9.1 Wire representation vs application model

Separate, as required. `GameServerInfo` carries `std::string ip`, `WireI32`
signed fields and a `bool`; the wire's `char[21]`, alignment padding and MSVC
`bool` representation are the codec's business. The `GameServerGrid` is the
application-level view of legacy's sparse 2-D array and exposes a dense
canonically-ordered vector through `Servers()`.

### 9.2 WORLD-001 / WORLD-002 untouched

No existing protocol code was rewritten or refactored. The single change to
existing code is a **new read-only accessor**, `ConnectionFramer::Capacity()`,
added because `LoginServerClient` must know its remaining room to slice a large
burst. No behaviour changed. Both prior suites remain green.

---

## 10. Test results

New coverage: ids; entry geometry including both padding runs; all five signed
integers; `bPK` both ways; over-long IP refused rather than truncated;
unterminated IP refused on decode; wrong declared size / wrong id / wrong length /
impossible `bPK` byte; extra bytes on the terminator; stray byte after the
terminator (left pending - asserted, because the alternative claim would be wrong);
truncated request and truncated terminator; hostile `dwSize`; client-controlled
count cannot allocate; out-of-range entry skipped while the list still completes;
grid order vs insertion order; duplicates collapsing; grid full at 200;
`maxClients <= 0` skipped; request-twice discards the previous list; feed-before-
request refused; hostname and zero-port refused with no half-connected state;
disconnect and reset; byte-at-a-time; two-bytes-at-a-time; header split; body
split; terminator alone; empty / 1 / 2 / 20 / 200 entries; end-to-end fixture
equality; and the transport-abstraction exchange.

| Suite | Baseline | After |
| --- | --- | --- |
| ModernCoreTests | 526 | 526 |
| ModernServerTests | 138 | **177** |
| ModernNetworkTests | 133 | **164** |
| ModernClientAppTests | 16 | 16 |
| ModernClientInputTests | 25 | 25 |
| CTest | 15/15 | 15/15 |

All suites pass in **both Debug and Release**, 0 errors, 0 new warnings.

---

## 11. Deviations from the brief, and why

| Brief | What was done | Why |
| --- | --- | --- |
| "WORLD-002 found evidence the list is batched through V030 compression" | **Not compressed.** Raw frames; V030's framing reused, its compression deliberately not | Proven from `CLoginServer::SendClient` -> `SendClient2` (§5.1). Compressing would break the wire. |
| "§24 test `SND_GAME_SVR` -> batch -> `NET_COMPRESS` -> decode" | Tested the **raw** path instead, and asserted the wire is *not* enveloped | Implementing the batched path would encode a bug. |
| "Server list order: do not sort" | Group-then-channel grid order preserved, tested against shuffled insertion | Proven by the server's nested loop. |
| "Server-list END sent even when zero servers" | Yes - terminator alone, tested | Proven at `s_CLoginServerMsg.cpp:136-144`. |
| Two simultaneous connections (diagram) | One connection slot, two sequential roles | Proven at `s_NetClient.cpp:367-388`. Kept as separate types/phases regardless. |
| `GameServerInfo` fields "only after source establishes them" | All seven named from legacy's own comments; **no** `unknown0`/`unknown1` needed | The struct is fully self-documenting. |
| §26 live connection test | **BLOCKED**, reported as such | No socket code in modern; no server process; encrypted config (§8). |

## 12. Unresolved

- The real ASURA Login endpoint remains unreadable (`param.ini` encrypted).
- `NET_MSG_REQ_FULL_SVR_INFO` / `SND_FULL_SVR_INFO` and the other
  `NET_MSG_LGIN`-relative ids exist in the same family but belong to Session and
  are **not** part of the game-server list; untouched here.
- No socket transport exists in `modern/`. A real TCP integration test - and any
  eventual live Login Server - depends on adding one.
