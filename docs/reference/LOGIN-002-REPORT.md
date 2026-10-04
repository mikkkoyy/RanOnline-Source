# LOGIN-002 - Modern TCP Transport & Login Server Runtime

## Result in one line

**PASS.** A real Winsock TCP transport now sits behind `INetworkTransport`, a real
`ModernLoginServer` executable binds loopback and answers `NET_MSG_REQ_GAME_SVR`
over a real socket, and `LoginServerClient` completes the exchange across that
socket - proven by 23 integration tests that use real TCP and no mock, plus 39
transport tests from the preceding commit.

**What this does NOT prove:** that the modern client can talk to the real ASURA
production Login Server at `211.172.252.50:5001`. Nothing in this milestone
connects to that endpoint, and the encrypted ASURA `param.ini` remains
unresolved. See section 12.

---

## 0. Evidence grade

| Layer | Grade | How |
|---|---|---|
| Wire format (1542 / 1552 / 1562) | **PROVEN** | LOGIN-001, unchanged here |
| Raw (no `NET_COMPRESS`) on this path | **PROVEN** | LOGIN-001 + legacy source, re-confirmed in 3 |
| Legacy socket behaviour | **PROVEN** | legacy source, re-read for this milestone (section 11) |
| Modern TCP transport | **PROVEN** | real sockets, 39 tests, 10 consecutive clean Release runs |
| Modern Login Server runtime | **PROVEN** | real sockets + a hand-run executable |
| Modern client over a real socket | **PROVEN** | 23 integration tests |
| Interop with ASURA production | **NOT ATTEMPTED** | out of scope, section 12 |

---

## 1. Runtime architecture

```
ModernLoginServer  (modern/tools/login_server/main.cpp)
      |
      v
LoginServerRuntime (modern/server/login/LoginServerRuntime.h)
      |  owns TcpListener; adopts each accepted socket as a TcpTransport
      v
TcpListener / TcpTransport   (modern/network, NETWORK-001)
      |  bytes only - no message ids, no boundaries, no RAN types
      v
ConnectionFramer            (modern/network/NetworkConnection.h)
      |  accumulates bytes, yields whole validated messages
      v
GameServerListProtocol      (modern/network/GameServerListProtocol.h)
      |
      v
LoginServerResponder        (modern/server/login/LoginServerResponder.h, LOGIN-001)
      |  builds the whole response: N x SND_GAME_SVR, then SND_GAME_SVR_END
      v
TcpTransport::Send
```

And the client:

```
LoginServerSession          (modern/client/login/LoginServerSession.h)
      |  owns the socket, decides when to read, enforces the deadline
      +--> TcpTransport       (real socket)
      |
      v
LoginServerClient           (modern/client/login/LoginServerClient.h, LOGIN-001)
      |  owns the protocol; owns NO socket
      |  BeginConnect / RequestGameServers / Feed / Servers()
      v
ConnectionFramer -> GameServerListProtocol -> GameServerGrid
```

### 1.1 Why the client is two types

`LoginServerClient` (LOGIN-001) deliberately owns no socket. That is not being
undone here; `LoginServerSession` is added *beside* it and owns the socket:

- `LoginServerClient` takes bytes and returns bytes. It is exercised by 164
  `ModernNetworkTests` cases with no network at all.
- `LoginServerSession` moves bytes between a socket and that client, and owns the
  deadline.

This mirrors legacy, where `CNetClient` owned the socket and `MsgGameSvrInfo`
owned the message.

### 1.2 Layering is preserved

`ModernCore` remains socket-free - verified mechanically: no header or source in
`modern/core` references a socket, a `SOCKET`, `WSA*`, `ws2_32`,
`TcpTransport`, `TcpListener` or `INetworkTransport`. `<winsock2.h>` appears in
exactly four files, all `modern/network/*.cpp`. `modern/core` links nothing.
`ws2_32` is a **PRIVATE** link on `ModernNetwork`, so nothing that links it can
reach a socket call even accidentally.

---

## 2. What was added

| File | Purpose |
|---|---|
| `modern/server/login/LoginServerConfig.h/.cpp` | The whole configuration surface: bind address, port, grid. Plus `Validate()` and three fixtures. |
| `modern/server/login/LoginServerRuntime.h/.cpp` | bind / listen / accept / frame / answer / close. Synchronous, one connection per `ServeOneClient`. |
| `modern/client/login/LoginServerSession.h/.cpp` | Client socket ownership + deadline. Returns a four-valued outcome. |
| `modern/tools/login_server/main.cpp` | `ModernLoginServer`, the runnable executable. |
| `modern/server/LoginServerTcpTests.cpp` | 23 real-TCP integration tests. |
| `modern/network/TcpTransport.{h,cpp}` | **One addition:** `CloseOwnedHandle`, for the one place a socket this layer created cannot be adopted (section 9.3). |

`INetworkTransport` is unchanged as an abstraction. The loopback pair stays, and
`ModernNetworkTests` stays a pure rule-test binary that opens no socket. The real
backend is a *second* implementation, reached only by the new test binaries and
by the runtime above.

### 2.1 Test binaries added

| Binary | Cases | Sockets? |
|---|---|---|
| `ModernTcpTransportTests` (NETWORK-001) | 39 | yes |
| `ModernLoginServerTcpTests` (LOGIN-002) | 23 | yes |

Both are separate executables, not extra files in the headless suites. The reason
is the same each time: `ModernNetworkTests` and `ModernServerTests` must stay
runnable with no socket and no network, because that is what makes them
deterministic and safe to run anywhere. Folding real-socket tests into them would
make the rule tests depend on the machine's network stack.

---

## 3. Why raw TCP, re-confirmed

LOGIN-001 proved the game-server-list path is not `NET_COMPRESS`. The legacy
source was re-read for this milestone and the finding holds, with the exact chain:

```
CLoginServer::SendClient          s_CLoginServer.cpp:823-828
    -> CClientManager::SendClient2   (direct, no batching)
    -> memcpy into a pooled PER_IO_OPERATION_DATA   s_CClientManager.cpp:489-527
    -> ::WSASend                                          s_CClientManager.cpp:529-585
```

Compression lives one level up, in `CClientManager::SendClient`
(`s_CClientManager.cpp:437`) via `CSendMsgBuffer::getSendSize`
(`SendMsgBuffer.cpp:116-174`), which emits the `NET_COMPRESS` envelope.
`CLoginServer` never calls that path - it calls `SendClient2` directly, and
`CClientLogin` (`s_CLoginServer.h`) inherits it unmodified.

So `LoginServerRuntime` writes **raw framed messages**. It does not construct a
`NET_COMPRESS` envelope, and it does not route through `NetCompressCodec` or
`ServerBatchEncoder`. V030 compression is untouched and still correct for the
Agent login response, where legacy really does use it.

### 3.1 One deliberate deviation, and why it is invisible

Legacy issues **one `WSASend` per message**. This runtime hands
`LoginServerResponder`'s whole buffer to `TcpTransport::Send` in one call.

The bytes are identical, and that is the only thing that can matter: **TCP has no
message boundaries.** A reader cannot distinguish one 64-byte write from four,
and `ConnectionFramer` on the client exists precisely so that nobody has to try.
Splitting the send to imitate the syscall pattern would mean re-deriving message
boundaries on the send side - a second framing parser, which is the specific
duplication this project keeps refusing.

The send loop inside `TcpTransport` still applies, so a response larger than the
socket buffer is delivered whole or reported as a desynchronised stream
(`TransportFault::PartialSend`).

---

## 4. Deliberate deviation: the connection is closed after END

**Legacy keeps the connection OPEN.** `CLoginServer::MsgSndGameSvrInfo`
(`s_CLoginServerMsg.cpp:95-147`) contains no `CloseClient`, no `closesocket` and
no `shutdown`; it ends at `LockOff()`. The socket is then held open, with
`NET_HEARTBEAT_CLIENT_REQ` every ~2 minutes (`s_CLoginServer.cpp:569-577`,
`HEARTBEAT_TIME` in `s_CServer.h:53`) as the only liveness mechanism, until the
peer leaves or the client moves on - at which point `ConnectGameServer` closes it
and re-points the same `CNetClient` at the chosen Agent
(`s_NetClient.cpp:379-388`).

**This runtime closes after the terminator.** Three reasons, in order of weight:

1. This milestone's client has nothing to do after END. It has no Agent phase,
   no heartbeat responder and no server-selection step yet, so an open socket
   would leave it waiting on a conversation that cannot happen.
2. Closing makes the end of the exchange *observable*. Without it, "the list
   arrived" and "the server is still thinking" are indistinguishable.
3. It matches what legacy effectively does moments later.

Restoring the long-lived behaviour is a later milestone's decision. It is
recorded in `LoginServerRuntime.h` rather than in a test, because it is a policy
change and not a bug fix.

---

## 5. Configuration

`LoginServerConfig` is three things and no framework: `bind` (address + port),
`servers` (the grid), and `Validate()`. There is no parser, no file format, no
defaults table and no inheritance.

| Field | Legacy source | Default here |
|---|---|---|
| `bind.host` | `server_ip` via `CCfg` (`s_CServer.cpp:625`) | `127.0.0.1` |
| `bind.port` | `server_service_port` (ships as `12004`, `CFG/ServerLogin.cfg:9`) | `0` = OS-assigned |
| `servers` | `m_sGame[20][10]`, refreshed from the Session Server | a fixed fixture |

`211.172.252.50:5001` is **not** baked in anywhere. It is legacy's *client-side*
default (`RANPARAM.cpp:144-146`) and it does not even match the shipped server
port - see section 11.

### 5.1 A bind address that cannot be read is refused, not defaulted

Legacy substitutes `INADDR_ANY` for an unparsable `server_ip`
(`s_CServer.cpp:627-631`). Reproducing that would mean a typo in a config file
quietly turns a loopback-only server into one exposed to every interface, and the
symptom - clients that should not have connected, connecting - appears nowhere
near the mistake. `Validate()` refuses it instead, before a socket is opened.

### 5.2 An empty grid is a valid configuration

"No game servers are running" is a state a Login Server must be able to report,
and the wire has a way to say it: legacy sends a bare `SND_GAME_SVR_END` and logs
`ERROR:Check Session Server Connection` (`s_CLoginServerMsg.cpp:136-144`).
Silence instead would leave a client that completes only on END waiting forever.
Tested: `LoginServerTcp_EmptyListStillCompletesWithOnlyTheTerminator`.

---

## 6. The `ModernLoginServer` executable

```
ModernLoginServer [--host <addr>] [--port <n>] [--clients <n>] [--quiet]
```

Run by hand and watched:

```
Listening on 127.0.0.1:51260
Client connected: 127.0.0.1:51261
REQ_GAME_SVR received
Sending 3 game servers
SND_GAME_SVR_END sent
Client disconnected: 127.0.0.1:51261
Served 1 client, refused 0.
```

That output is real: the smoke test ran the executable as a separate process and
talked to it from a PowerShell `TcpClient` that shares no code with it - which is
the one thing a unit test cannot show. It sent a hand-built 8-byte
`REQ_GAME_SVR` and received 176 bytes, decoded as three 56-byte frames of type
1552 followed by one 8-byte frame of type 1562.

Command-line options rather than a config file: a file format is a decision that
belongs to whoever owns deployment, and guessing at one now would make it harder
to change later.

### 6.1 Logging

The runtime emits `LoginServerEvent` values and a `LoginServerLogEntry` struct;
the *wording* lives in the executable. That split is the reason those types
exist: a server that formatted its own messages could only be tested by asserting
on how it phrased them. The integration tests assert on the events.

---

## 7. Real TCP integration test evidence

`ModernLoginServerTcpTests`, 23 cases, all over `127.0.0.1`.

| Area | Cases |
|---|---|
| Full exchange, grid contents and order | 1 |
| Empty list completes on END only | 1 |
| Assigned and requested ports both real | 1 |
| Arbitrary read sizes (1, 2, 3, 7, 8, 13, 56, 64, 4096) | 1 |
| Header split in half | 1 |
| Malformed: wrong id, wrong size, truncated header, truncated body, oversized, undersized, garbage | 7 |
| Server survives all of the above, then serves correctly | 1 |
| Three sequential clients, each complete | 1 |
| Each connection sees only its own list | 1 |
| Client: server closes before END | 1 |
| Client: timeout when the server says nothing | 1 |
| Client: malformed response header | 1 |
| Client: unexpected response message | 1 |
| Client: refuses to work without a connection | 1 |
| Config validation | 2 |
| Runtime refuses to serve when not running | 1 |

### 7.1 Fragmentation

`LoginServerTcp_ArbitraryReadSizesAllReassembleTheSameGrid` runs the whole
exchange nine times with the read size forced to 1, 2, 3, 7, 8, 13, 56, 64 and
4096 bytes, and asserts the **identical** `GameServerInfo` values each time - not
merely the same count, since a framer that recovered three entries out of order,
or with the wrong client counts, would pass a count-only assertion.

- `1` splits every byte off alone.
- `8` lands exactly on header boundaries.
- `13` splits a 56-byte body in a way no writer would choose.
- `4` (a separate test) cannot even contain `dwSize` in the first read.

The read size is chosen by the *test*, so the boundaries are dictated by neither
the server's writes nor the kernel's segmentation. Every received byte range is
handed to `LoginServerClient::Feed`, which owns the `ConnectionFramer`. There is
no second framing parser anywhere in the client path.

### 7.2 Malformed input

Seven shapes are sent from a raw socket: wrong id, right id with `dwSize` 12,
4 bytes then hang up, a header promising 56 bytes followed by 8 then hang up,
`dwSize` 100000, `dwSize` 1, and 512 bytes of noise.

Each is refused with a specific reason (`UnexpectedMessage`, `BadRequestSize`,
`PeerClosedFirst`, `MalformedFrame`) and counted. No crash, no hang.

**The case that matters most** is
`LoginServerTcp_ServerStillServesCorrectlyAfterEveryKindOfRejection`: after all
seven rejections, a well-behaved client on the *same listener* still gets a
complete and correct three-entry list. A test that only checked "it did not crash"
would pass a server that was left broken.

Notably, the executable smoke test hit the wrong-id path by accident before it
was written as a test - a hand-built request with `nType` bytes `CA 06` instead
of `06 06` produced a clean `unexpected message id 1738` rejection and the server
carried on. That is now a permanent test.

### 7.3 Multiple clients

Three clients, one after another, each getting the complete list, and each seeing
only its own list. Sequential on purpose: the claim is that one connection does
not corrupt another, and a sequential test cannot be flaky for scheduling
reasons. A caller wanting simultaneous clients should serve from several threads.
No production connection manager was built - the goal was only to prove
isolation.

---

## 8. Three levels of test, kept distinct

| Level | Binary | Sockets | What it alone can prove |
|---|---|---|---|
| Protocol | `ModernNetworkTests` (164) | no | Byte-exact wire format, with boundaries chosen byte by byte |
| Protocol, end to end | `ModernServerTests` (177) | no (loopback pair) | Responder and client agree, with controlled fragmentation |
| Transport | `ModernTcpTransportTests` (39) | **yes** | A real socket behaves as the abstraction promises |
| Integration | `ModernLoginServerTcpTests` (23) | **yes** | The whole exchange crosses a real TCP stream |

The loopback transport is **not** deprecated by any of this. A test that controls
its own chunk boundaries is still worth having, and a socket decides its own.

---

## 9. Defects found and fixed while completing this work

### 9.1 NETWORK-001 (previous commit, `ad18673`)

Recorded because they were latent in the inherited WIP and this milestone is what
made them reachable:

1. `TcpListener::Listen` called `Close()` and then asked whether a socket existed,
   so **every bind failed** with `InvalidState` and no platform code. A socket
   that has been bound cannot be bound again; `Listen` now replaces the socket
   only when one is already listening.
2. `TcpTransport::Connect` had the same shape of mistake: it called
   `Disconnect()`, which destroys the socket, and then tested `IsValid()`, which
   could never be true. **Every connect failed.** Both now go through
   `CreateSocket` / `CloseSocket`.
3. Four Winsock error names in the mapping (`WSAEHOSTNOTFOUND`,
   `WSAENAMENOTRESOLVED`, `WSA_SERVICE_NOT_FOUND`, `WSAVERNOTSUPPED`) do not exist
   in the Windows SDK. They had been taken from documentation prose rather than a
   header and had never compiled.
4. `NOMINMAX` is now defined before `<winsock2.h>`. `WinSock2.h` includes
   `<windows.h>` itself, which defines function-like `min`/`max` macros that break
   `std::min(a, b)` with an error whose text mentions neither.
5. `NetworkTransport.h` named `Status` without including `types/Result.h`. Every
   existing consumer happened to include it first; nothing did that by contract.
6. `inet_ntoa` → `InetNtop`, `WSASocket` → `WSASocketW`, so `/W4` stays meaningful
   instead of needing deprecation suppressions.

### 9.2 LOGIN-002

7. **The exchange budget was charged per read, not per unit time.** Both
   `LoginServerSession::Pump` and `LoginServerRuntime::ReadMessage` originally
   subtracted the read timeout from a budget on every iteration. A read that
   returns *instantly* still cost the full slice, so a client reading 1 byte at a
   time exhausted its budget after 80 one-byte reads - 80 bytes of a 176-byte
   response - and reported `TimedOut` with a partial list. This is why the
   fragmentation test failed against a server that was working perfectly. Both
   now use a `std::chrono::steady_clock` deadline, which charges only time
   actually waited.
8. **A data race in the integration test.** The server's event log was written by
   the server thread and read by the test thread inside the client loop. This is
   undefined behaviour - vector reallocation invalidating an iterator - and it
   presented as an intermittent failure, worse in Release than Debug, which is the
   signature of a race rather than a logic error. It failed **7 of 12** Release
   runs before being fixed. The log is now mutex-guarded, and the server-side
   assertions moved past the thread join, because a client can hold the complete
   list before the server has recorded having sent it.
9. `FakeServer` in the tests accepted and hung up without reading the client's
   request. On Windows a socket closed with unread data in its receive buffer is
   closed **abortively**, so the client saw an RST and reported `Faulted` while the
   test expected `PeerClosed` - testing the reset path while claiming to test the
   orderly-close path. It now drains first, like a real server does.
10. `TcpTransport::CloseOwnedHandle` was added. `Adopt` deliberately does not
    close its argument when it fails, so a caller that cannot adopt a socket needs
    some way to close it; without this, that socket would leak once per occurrence,
    silently.

### 9.3 Not a defect, but recorded

`LoginServerRuntime` opens no socket of its own until `Start()`, and
`ServeOneClient` before `Start()` returns `InvalidState`. Tested, because a
runtime that reported success while listening on nothing would be a server whose
readiness is a lie.

---

## 10. Test results

```
cmake --build build --config Debug      -> 0 errors, 0 warnings
ctest --test-dir build -C Debug        -> 100% tests passed, 0 failed out of 17
cmake --build build --config Release    -> 0 errors, 0 warnings
ctest --test-dir build -C Release      -> 100% tests passed, 0 failed out of 17
```

| Suite | Baseline | After (Debug) | After (Release) |
|---|---|---|---|
| Core | 526 | **526** | **526** |
| Server | 177 | **177** | **177** |
| Network | 164 | **164** | **164** |
| ClientApp | 16 | **16** | **16** |
| ClientInput | 25 | **25** | **25** |
| TcpTransport | new | 39 | 39 |
| LoginServerTcp | new | 23 | 23 |
| **CTest** | 15/15 | **17/17** | **17/17** |

Every baseline number is unchanged.

### 10.1 Stability

Because two defects above only appeared intermittently, both new suites were run
repeatedly rather than once:

- `ModernLoginServerTcpTests`, Release: **0 failures in 15 consecutive runs**
  (was 7 in 12 before the race fix).
- `ModernTcpTransportTests`, Release: **0 failures in 10 consecutive runs**.

### 10.2 Every blocking operation is bounded

A test that can hang is a test CI cannot trust.

| Operation | Bound |
|---|---|
| `TcpListener::Accept` | explicit deadline, or the listener's option |
| `TcpTransport::Connect` | non-blocking connect + `select()` deadline |
| `TcpTransport::Send` / `Receive` | explicit deadline or option; `SO_SNDTIMEO`/`SO_RCVTIMEO` as backstop |
| `LoginServerSession::Pump` | wall-clock deadline on the **whole** exchange |
| `LoginServerRuntime::ReadMessage` | wall-clock deadline on the whole exchange |
| Test binaries | run under an external watchdog during development; the harnesses are unbuffered so a hang names itself |

The wall-clock deadline, not a per-read counter, is deliberate: a per-read
allowance lets a peer that dribbles one byte before each deadline hold a
connection open forever, which is the same denial of service in miniature that
the server side refuses to allow.

---

## 11. Legacy socket reuse vs modern logical separation

**Legacy: one socket, two sequential roles.** `CNetClient` owns a single
connection slot.

```
ConnectLoginServer   s_NetClient.cpp:367-372
    CloseConnect()               // closes any existing connection first
    ConnectServer(addr, port, NET_STATE_LOGIN)

ConnectGameServer    s_NetClient.cpp:379-388
    CloseConnect()               // closes THAT connection
    NET_STATE_AGENT              // re-points the same CNetClient
```

The same object, re-pointed. `CNetClient::MessageProcess` routes on
`m_nClientNetState`: `NET_STATE_LOGIN` (1) to `MessageProcessLogin`,
`NET_STATE_AGENT` (2) to `MessageProcessGame` (`s_NetClientMsg.cpp:19-44`). The
two never share a handler or a message id.

**Modern: separate objects, separate sockets.** `LoginServerSession` (Login
phase) and `LoginResponseClient` (Agent phase, WORLD-002) are distinct types with
distinct sockets. `LoginServerSession` even refuses to connect twice
(`AlreadyExists`), because one session is one conversation and the second connect
would otherwise leave the first connection's bytes in the protocol object with
nothing to say which server they came from.

**This is deliberate, not an oversight.** Physical socket reuse maximises the
blast radius of a framing bug: one mis-sized message desynchronises both
conversations. Separate objects make that impossible by construction, and cost a
socket. The trade-off becomes worth revisiting when a real session-reuse
requirement exists - which it does not yet, since the Agent phase has no
transport attached at all. LOGIN-002 records the difference here rather than
forcing reuse prematurely.

### 11.1 Legacy thread model, and why none of it was copied

| Legacy | Modern |
|---|---|
| Accept thread `CServerListenProc` (`s_CServer.h:226`) | none - `ServeOneClient` accepts on the caller's thread |
| IOCP worker pool, `min(cpu * S_HEURISTIC_NUM, cfg.max_thread, 6)` (`s_CServer.cpp:306-319`) | none |
| Update thread `CServerUpdateProc` (`s_CServer.h:238`) | none |
| `WSAAccept` + `CreateIoCompletionPort` + overlapped `WSARecv` | blocking `accept` + `recv`, each with a deadline |
| `listen(s, NET_CLIENT_LISTEN)` = backlog 50 (`s_NetGlobal.h:115`) | backlog 16, clamped to `SOMAXCONN` |

None of it is needed to serve one request, and all of it would make an automated
test non-deterministic - the one property a test suite cannot trade away. Spec 19
and 20 asked for exactly this.

Two further differences, both improvements and both recorded so they are not
mistaken for oversights:

- **`WSACleanup` is reference counted here.** Legacy calls `WSAStartup` in
  `CServer::CServer` and `WSACleanup` in `~CServer` with no counter
  (`s_NetGlobal.cpp:23-44`), balanced only by 1 server object = 1 pair - and legacy
  has three stray unbalanced `WSACleanup()` calls on failure paths
  (`s_CServer.cpp:556`, `:645`, `:701`). `WinSockRuntime` is reference counted and
  asserts its own balance in a test.
- **Legacy's client send loop is buggy and is not reproduced.**
  `CNetClient::SendBuffer2` (`s_NetClient.cpp:815-869`) retries a partial send
  from offset 0 - `memmove` only happens on a complete write - so a short write
  duplicates the head of the message. `TcpTransport::Send` advances the offset
  and loops.

### 11.2 Public backread (corroboration only)

Searched before and during implementation, as the brief asked. **None of this is
authority.** Where public material and `legacy/` disagree, `legacy/` wins; the
only thing public sources are used for here is to check that the local reading is
not an artefact of one checkout.

| Public finding | Corroborates | Agrees with `legacy/`? |
|---|---|---|
| RaGEZONE's RAN source mirror is laid out as `[Server]__Login`, `[Server]__Session`, `[Server]__Field`, `[Server]__Agent`, `[Lib]__NetClient`, `[Lib]__NetServer` | The four-server-role structure in `s_NetGlobal.h:82-86` and the split between a client network library and a server one | Yes |
| A forum post quoting `int ConnectLoginServer(const char *szAddress, int nPort=…)` | The signature and the address+port shape at `s_NetClient.cpp:367` | Yes |
| A forum post showing `CLoginServer::SessionSndSvrInfo` in a file named `s_LoginServerSession.cpp`, building a `NET_SERVER_INFO` from `CCfg::GetInstance()->GetServicePort()` and `m_szAddress` | The Login Server keeping its Session-Server conversation in separate translation units, and its address/port coming from `CCfg` | Yes, and it adds `GetProxyIp()`, an IP-proxy feature this checkout does not have - a variant branch, not a contradiction |
| Forum troubleshooting where the client's `nLoginPort` disagreed with the server `.cfg` | The in-tree mismatch recorded in section 5: client default `5001` vs shipped server port `12004` | Yes, and it is a well-known deployment footgun in the community |
| An x64-porting thread describing the `DWORD` → `DWORD_PTR`/`uintptr_t` migration as "the most painful" part of the port | The decision in NETWORK-001 to store socket handles as `uintptr_t` so the header width matches `SOCKET` exactly on both architectures | Yes |

Two things the public material did **not** establish, and which are therefore
recorded as local-source findings only:

- That `SendClient2` skips compression. That is the load-bearing claim of
  section 3, and it rests entirely on reading `s_CLoginServer.cpp:823-828` and
  `s_CClientManager.cpp:489-527` in this checkout. It is corroborated only in the
  weak sense that no public source contradicts it.
- That the Login Server keeps the connection open after `SND_GAME_SVR_END`. Same
  provenance: `s_CLoginServerMsg.cpp:95-147` in this checkout, read in full.

---

## 12. What is deliberately NOT claimed

**The modern client has NOT been shown to work against the real ASURA production
Login Server at `211.172.252.50:5001`.**

This milestone proves only:

```
Modern Client  <==== REAL TCP ====>  Modern Login Server
```

using the RAN protocol as established by LOGIN-001, on localhost.

It does **not** prove interop with `211.172.252.50:5001`, and no test here
contacts it. Reasons, all pre-existing:

- The ASURA `param.ini` remains encrypted and unresolved.
- Legacy's own client and server defaults disagree in-tree: `RANPARAM.cpp:146`
  says port `5001`, while `CFG/ServerLogin.cfg:9` ships `12004`.
- Nothing in this milestone supplies the login key negotiation or the Session
  Server feed that a production server list would come from. The grid here is a
  fixture, and the production source is `m_sGame` refreshed from the Session
  Server.

Spec 28 asked for exactly this restraint, and the ASURA endpoint was not
contacted at any point.

### 12.1 Also out of scope, and untouched

Agent login, `LOGIN_2`, `LOGIN_FB`, character selection, lobby, world entry,
gameplay, combat, movement. `GameServerListProtocol`, `LoginProtocol`,
`LoginResponseProtocol`, `MinTeaCodec`, `NetCompressCodec`, `ServerBatchEncoder`,
the combat systems, the renderer, the asset system, the tools and the launcher
are all unmodified.

**Launcher: UNCHANGED.** `Ran Online Launcher.exe` was not inspected, rebuilt,
replaced or reconfigured, and no Facebook launcher work was touched.

---

## 13. Success criteria

| # | Criterion | Status |
|---|---|---|
| 1 | Real Winsock TCP in `ModernNetwork` | PASS (`TcpTransport`, `TcpListener`) |
| 2 | Core remains socket-free | PASS (verified mechanically) |
| 3 | Login Server listens on localhost | PASS |
| 4 | `LoginServerClient` connects through real TCP | PASS |
| 5 | `NET_MSG_REQ_GAME_SVR` crosses a real socket | PASS |
| 6 | `NET_MSG_SND_GAME_SVR` crosses a real socket | PASS |
| 7 | `NET_MSG_SND_GAME_SVR_END` crosses a real socket | PASS |
| 8 | TCP fragmentation handled | PASS (9 read sizes + split header) |
| 9 | Partial sends handled | PASS (512 KiB payload through the send loop) |
| 10 | Malformed packets rejected safely | PASS (7 shapes + survival) |
| 11 | Real TCP integration test passes | PASS (23 cases) |
| 12 | LOGIN-001 protocol tests green | PASS (Network 164 unchanged) |
| 13 | WORLD-001/WORLD-002 green | PASS (Server 177 unchanged) |
| 14 | Debug build clean | PASS |
| 15 | Release build clean | PASS |
| 16 | No launcher changes | PASS |
| 17 | Changes committed | see final report |
| 18 | Changes pushed | see final report |
| 19 | GitHub HEAD matches local HEAD | see final report |
| 20 | Working tree clean | see final report |
