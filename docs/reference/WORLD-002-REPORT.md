# WORLD-002 - Modern server to client login response (`NET_MSG_LOGIN_FB`)

Baseline: `033eb52422c56229098b2e1bca14f421ad4c5943` ("FACEBOOK-001")

## Result in one line

The 120-byte `NET_LOGIN_FEEDBACK_DATA` response is implemented end to end as an
**unenveloped-field, unencrypted, V030-batched** message: an explicit little-endian
codec, a server-side responder, and a client receiver that survives arbitrary TCP
fragmentation. The game-server list was investigated and deliberately **not**
implemented, because legacy serves it from a different connection before login.

---

## 1. What the legacy code actually does

### The struct is the small one

`NET_MSG_LOGIN_FB = 2050` (`legacy/Lib_Network/s_NetGlobal.h:771`).

There are two similarly named structs and picking the wrong one silently changes the
message size:

| Struct | Role | Size |
| --- | --- | --- |
| `NET_LOGIN_FEEDBACK_DATA` | **the wire format** | **120 bytes** |
| `NET_LOGIN_FEEDBACK_DATA2` | internal in-process value | not sent |

`NET_LOGIN_FEEDBACK_DATA2` is passed **by pointer** from `CAgentUserCheck::Execute` to
`MsgLogInBack` (`s_CDbActionUser.cpp:396-510`) and is converted field-by-field into
`NET_LOGIN_FEEDBACK_DATA` before anything reaches the socket. It never appears on the
wire. A milestone that encoded DATA2 would have produced a wrong-length frame.

`sizeof(NET_LOGIN_FEEDBACK_DATA) == 120` was confirmed with a throwaway MSVC probe
under the real MBCS/x86 project settings, rather than inferred from the declaration.

### Field layout, as emitted

Offsets below are the ones the codec writes to, and each is asserted by a test:

| Offset | Size | Field |
| --- | --- | --- |
| 0 | 4 | `dwSize` = 120 (message header, counted) |
| 4 | 4 | `nType` = 2050 |
| 8 | 21 | `szDaumGID` |
| 29 | 1 | padding |
| 30 | 2 | `nResult` (`EM_LOGIN_FB_SUB`, success = 0) |
| 32 | 2 | `uChaRemain` |
| 34 | 2 | padding |
| 36 | 4 | `nExtremeM` |
| 40 | 4 | `nExtremeW` |
| 44 | 4 | `nCheckFlag` |
| 48 | 4 | `nPatchProgramVer` |
| 52 | 4 | `nGameProgramVer` |
| 56 | 4 | `dwGameTime` |
| 60 | 4 | `dwPremiumPoint` |
| 64 | 4 | `dwCombatPoint` |
| 68 | 51 | `szEmail` |
| 119 | 1 | padding |

`EM_LOGIN_FB_SUB` is a `uint16_t` over `0..23`, and all 24 values round-trip through
`ToString`/decode.

### Not encrypted

`MsgLogInBack` contains **no `m_Tea` call**. The client casts the received buffer
straight to the struct. This is asserted structurally: the test reads the email out of
the encoded bytes with no key at all, and `nResult = 0` appears literally at offset 30.

This is the asymmetry with WORLD-001, whose request fields *are* minTea-protected.
Both directions are covered by tests, so the asymmetry cannot silently invert later.

### Batched, one message per send

The response is server to client, so it goes through the V030 layer: batched into a
`NET_COMPRESS` envelope (12-byte header, `nType = 170`), LZO-compressed. `MsgLogInBack`
sends **exactly one** `NET_MSG_LOGIN_FB` per mutually exclusive branch, and login
feedback is sent alone, so in practice the batch holds one message.

### Two StringCchCopy limits, not one

This was the one genuine bug the tests caught, and it is easy to get wrong because the
two fields look symmetric. `StringCchCopy`'s second argument is the destination
**buffer size in characters including the terminator**, not the field capacity:

| Field | Declaration | Call | Max characters |
| --- | --- | --- | --- |
| `szEmail` | `char[USR_INFOMAIL_LENGTH+1]` = 51 | `StringCchCopy(szEmail, USR_INFOMAIL_LENGTH, src)` | **49** (byte 50 stays zero) |
| `szDaumGID` | `char[DAUM_MAX_GID_LENGTH+1]` = 21 | `StringCchCopy(szDaumGID, DAUM_MAX_GID_LENGTH+1, src)` | **20** |

Sites: `s_CAgentServerMsgLogin.cpp:460` and `:1161` for the email; `:1681`, `:1870`
and `:2378` for the GID.

A single shared `fieldSize - 1` rule emits one email byte the legacy server never
sends. Both limits are now named constants (`kEmailMaxChars`, `kDaumGidMaxChars`) with
the call sites quoted, so they cannot be silently conflated again.

### Padding

Legacy declares `NET_LOGIN_FEEDBACK_DATA nlfd;` as an uninitialised stack local, so
its padding bytes are indeterminate. This implementation zero-fills instead. That is a
deliberate, documented superset: the bytes are ignored by every reader, and zeroing
makes the output deterministic and testable.

---

## 2. What was built

| File | Role |
| --- | --- |
| `modern/network/LoginResponseProtocol.h/.cpp` | the 2050/120-byte codec, explicit little-endian, no compiler-dependent structs |
| `modern/server/login/LoginResponder.h/.cpp` | verdict to response, through the V030 batcher |
| `modern/client/login/LoginResponseClient.h/.cpp` | envelope unwrap, framing, decode, phase |
| `modern/tests/LoginResponseProtocolTests.cpp` | geometry, fields, malformed input, compression asymmetry |
| `modern/server/LoginResponderTests.cpp` | end-to-end, including the WORLD-001 request |

`ModernClientLogin` moved from `INTERFACE` to `STATIC`, because it now owns a
translation unit.

### The receive path

The original implementation classified **each TCP read** as it arrived and discarded
incomplete envelopes. Both were wrong, and both fail only under fragmentation, which is
why a naive whole-frame test passes anyway:

- a one-byte read cannot be classified at all, so it was dropped;
- a partial envelope was thrown away instead of buffered.

Now bytes accumulate in `m_pending` and are classified once the accumulated stream has
a full 8-byte header. `ConsumeEnvelopes` then consumes every complete envelope, leaving
a partial trailing frame for the next read. `dwSize` is attacker-controlled, so an
impossible size (below the 12-byte envelope, or above the compressed ceiling for a full
batch) is refused rather than waited for, and the buffer cannot be grown without bound.

`NET_MSG_LOGIN_FB` is matched on `message.header.type`, **not** on the payload:
`Message::payload` is the body only and the id lives in the header ahead of it. The
codec is then handed the reassembled frame, header included.

Behaviour preserved: a malformed envelope is dropped and counted, not fatal, matching
legacy (`RcvMsgBuffer.cpp:145-160`). A desynchronised *framing* stream still latches,
because that cannot be resynchronised.

---

## 3. The server list was investigated and not implemented

The list protocol is real and was traced end to end:

| Constant | Value |
| --- | --- |
| `NET_MSG_REQ_GAME_SVR` | 1542 |
| `NET_MSG_SND_GAME_SVR` | 1552 |
| `NET_MSG_SND_GAME_SVR_END` | 1562 |

The client sends a bare 8-byte request on the **Login** connection immediately after
connecting and **before** Agent login (`s_NetClientMsg.cpp:314-333`,
`OuterInterface.cpp:501`). The server answers with repeated `NET_CUR_INFO_LOGIN`
entries terminated by `SND_GAME_SVR_END`, batched through V030
(`s_CLoginServerMsg.cpp`).

It is a **separate pre-login phase on a separate connection**, not a continuation of the
Agent `LOGIN_FB`. Implementing it here would have meant inventing a login-server
connection this milestone does not have. Documented, not implemented.

---

## 4. Scope and limits

- **Launcher**: untouched. Out of scope.
- **V030 compression internals**: unchanged. The existing `NetCompress`,
  `ServerBatchEncoder` and `MinLzo1xCodec` are reused as-is.
- **WORLD-001 request**: unchanged. It stays a raw, un-enveloped, minTea-protected
  `NET_MSG_LOGIN_2`, and the full exchange test asserts the request is *not* enveloped
  so the two directions cannot converge by accident.
- **Not implemented here**: the `LOGIN_FB_OK_FIELD` (2051) follow-up, and live packet
  capture. Capture is unavailable because `param.ini` is ASURA-encrypted and the client
  requires Hackshield, so source and tests are the authority, as they were for V028-V030.

## 5. Test results

New coverage includes geometry and all 24 result codes, field round-trips, both
truncation limits, zero-fill, rejection of wrong id / wrong size / short frame, the
encryption asymmetry, envelope-plus-compression, byte-at-a-time delivery, an envelope
split across two reads, an absurd `dwSize`, a malformed envelope followed by a good one,
a foreign message inside a real batch, and `Reset` clearing buffered bytes.

| Suite | Before | After |
| --- | --- | --- |
| ModernCoreTests | 526 | 526 |
| ModernServerTests | 125 | **138** |
| ModernNetworkTests | 115 | **133** |
| ModernClientAppTests | 16 | 16 |
| ModernClientInputTests | 25 | 25 |
| CTest | 15/15 | 15/15 |

All suites pass in **both Debug and Release**.