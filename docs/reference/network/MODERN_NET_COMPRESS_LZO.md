# Modern NET_COMPRESS + LZO Protocol Layer (VERTICAL-030)

Status: **implemented and tested**. Server-to-client compression only.

Baseline: `6b6a72a5ea339a8b481473608df43156d62e519c` ("V029 ASURA client login authority investigation").

Authority: `docs/reference/network/ASURA_NETWORK_PROTOCOL_AUTHORITY.md` (V028) and
`docs/reference/network/ASURA_CLIENT_LOGIN_AUTHORITY.md` (V029).

---

## 1. Direction — the whole point of this layer

Compression exists on **one side only**. This is not a stylistic choice and it is the
single most likely thing for a future contributor to "fix" by accident.

```text
SERVER  ->  CLIENT          batched, LZO-compressed, wrapped in a 12-byte envelope
CLIENT  ->  SERVER          raw NET_MSG_GENERIC, one per send, no envelope
```

Legacy evidence:

| Direction | Path | Behaviour |
| --- | --- | --- |
| server → client | `CClientManager::SendClientFinal` (`s_CClientManager.cpp:420-433`) → `CSendMsgBuffer::getSendSize` (`SendMsgBuffer.cpp:116-172`) → `SendClient2` → `::WSASend` | always enveloped, LZO attempted |
| client → server | `CNetClient::SendNormal` (`:924-953`) → `SendBuffer2` (`:815-831`) → `::send(m_pSndBuffer, nmg->dwSize)` | raw, one message per call |

There is **no raw server→client path**. Every server→client send reaches `WSASend`
through `getSendSize()`, which always emits a `NET_COMPRESS` envelope. The only
conditional behaviour is *inside* the envelope: compressed payload versus raw payload,
selected by `bCompress`. This was traced rather than assumed, because the alternative
reading — "sometimes legacy sends bare messages" — would have changed the design.

Consequently the API is deliberately asymmetric:

```text
NetCompress::EncodeServerToClientBatch(...)     // server -> client
NetCompress::DecodeServerToClientEnvelope(...)  // client receive
```

There is no `Compress()`/`Decompress()` pair on the envelope codec, because a generic
pair is precisely how a client→server login request ends up inside a compression
envelope the server will reject. Client→server framing is `ConnectionFramer`, unchanged
since V027.

---

## 2. NET_COMPRESS layout — 12 bytes

From the compiled struct at `legacy/Lib_Network/s_NetGlobal.h:2811-2819`:

```cpp
struct NET_COMPRESS
{
    NET_MSG_GENERIC nmg;   // 8 bytes: DWORD dwSize; EMNET_MSG nType;
    bool            bCompress;
};
```

| Offset | Width | Field | Value |
| ---: | ---: | --- | --- |
| 0 | 4 | `dwSize` | total frame bytes, **envelope included** |
| 4 | 4 | `nType` | **170** (`NET_MSG_COMPRESS`) |
| 8 | 1 | `bCompress` | 1 = LZO payload, 0 = raw payload |
| 9 | 3 | padding | alignment to 4 bytes |
| 12 | .. | payload | batch bytes, possibly LZO-compressed |

`sizeof(NET_COMPRESS)` is **12** on x86 with MSVC's 1-byte `bool`. The three padding
bytes are real wire data: legacy `memcpy`s the struct, so they exist on the wire and
`dwSize` counts them. A 9-byte envelope would be misread by the shipped client. This is
pinned by `static_assert` in `NetCompressCodec.h` and by `Envelope_HeaderBytesAreExact`.

### No `Count(2)` field

`SendMsgBuffer.h:33` carries a Doxygen diagram:

```
| Size(4) | Type(4) | Compress(1) | Count(2) | Data(...) |
```

**That `Count(2)` does not exist.** It is contradicted by the struct, by the constructor,
and by every write site. The batch carries no message count because it is
self-delimiting: each inner `NET_MSG_GENERIC` declares its own `dwSize`.

V028 found this trap; V027 quoted the diagram without flagging it. Nothing in V030 may
reintroduce a count — `Envelope_HeaderBytesAreExact` asserts `payloadOffset == 12`
specifically to keep the door shut.

---

## 3. Wire diagram

```text
SERVER
  │
  ├─ NET_MSG_GENERIC A
  ├─ NET_MSG_GENERIC B
  └─ NET_MSG_GENERIC C
          │
          ▼
    inner byte stream          (A || B || C, no separators, no count)
          │
          ▼
       LZO 1X                  lzo1x_1_compress; skipped if not smaller
          │
          ▼
   NET_COMPRESS envelope       dwSize | nType=170 | bCompress | pad | payload
          │
          ▼
        TCP
          │
          ▼
       CLIENT
          │
          ▼
    NET_COMPRESS decode        validate nType, dwSize, bounds
          │
          ▼
      LZO inflate              lzo1x_decompress_safe, bounded destination
          │
          ▼
 ConnectionFramer             one message at a time, by dwSize
          │
          ├─ A
          ├─ B
          └─ C
```

`Batch_ThreeMessagesSurviveEnvelopeInOrder` walks this exact path: three messages in,
one envelope out, three messages back out of the V027 framer in the original order.

---

## 4. LZO

| Property | Value |
| --- | --- |
| Compressor | `lzo1x_1_compress` (`MinLzo.cpp:112`) |
| Decompressor | `lzo1x_decompress_safe` (`MinLzo.cpp:158`) |
| Work memory | `LZO1X_1_MEM_COMPRESS`, allocated per codec instance |
| Init | `lzo_init()` (`MinLzo.cpp:62`) |
| Dictionary on decompress | `NULL` (`MinLzo.cpp:162`) |

`lzo1x_decompress_safe` is deliberate, not incidental. Legacy's own comment at
`MinLzo.cpp:157` reads `//packet crash fix` immediately above the switch from
`lzo1x_decompress` to `lzo1x_decompress_safe`. Canonical LZO usage documentation names
the unsafe `lzo1x_decompress`; RAN moved off it for good reason, and so does this layer.

### Two behaviours that are easy to get wrong

**The decompress length is IN/OUT.** `lzo1x_decompress_safe` takes the destination
*capacity* on entry and returns the bytes written. Legacy passes `NET_DATA_BUFSIZE`
(`RcvMsgBuffer.cpp:125`, the "packet crash fix"). Passing zero does not crash — it
returns `LZO_E_INPUT_OVERRUN`. This was reproduced verbatim during the V030 feasibility
work and cost a debugging cycle; it is called out in `CompressionCodec.h`.

**Compression is allowed to fail, and the failure is a valid protocol state.** Legacy
treats "output is not smaller than input" as `MINLZO_CAN_NOT_COMPRESS`, not as an error,
and the caller responds by shipping the batch **raw but still enveloped, with
`bCompress = false`** (`SendMsgBuffer.cpp:151-166`). A codec that reported success on
incompressible data would set `bCompress = 1` for bytes that are not compressed, and the
peer would hand them to the LZO decoder. `CompressionOutcome::NotCompressible` exists
to make that impossible to express by accident.

One legacy check was deliberately **not** reproduced: `MinLzo.cpp:164` rejects when
`decompressed >= compressed`, a heuristic that holds for compressible input and
wrongly rejects valid results for incompressible input. The modern layer validates the
declared size against the actual output instead.

---

## 5. Batching — 1000 is a flush trigger, not a cap

`CSendMsgBuffer::addMsg` (`SendMsgBuffer.cpp:65-113`):

```text
dwTotal = batched + incoming
if (dwTotal < 1000)              buffer it
else if (batch is empty)         batch this message ALONE, flush
else                             flush the batch WITHOUT this message, then start anew
```

`CClientManager::SendClientFinal` calls `getSendSize()` for **every** connection each
tick, unconditionally.

Consequences, all reproduced and tested:

1. **1000 is not a maximum.** A batch that stops at 999 bytes is still transmitted at
   end of tick. Tests must assert *flush timing*, never "this size is rejected".
2. **The flush side starts AT 1000.** Buffering requires `< 1000`, so 999 buffers while
   1000 and 1001 both flush. `Batching_NineHundredNinetyNineBuffers` and
   `Batching_ExactlyOneThousandFlushes` pin the boundary from both sides.
   *(An earlier draft of this layer and its test had this backwards — 1000 initially did
   not flush. The implementation matched legacy; the test did not. The test was wrong.)*
3. **A single oversized message flushes alone** rather than waiting for a partner, so
   batches are bounded by the largest single message, not by the threshold.

Legacy returns `BUFFER_SEND_ADD` from a function with no output parameter for the
payload, which is why its intended driver survives only as a comment
(`SendMsgBuffer.cpp:39-62`). `ServerBatchEncoder` returns `BatchAction` instead, which
is the same information without the reconstruction.

---

## 6. Validation rules

`NetCompress::DecodeEnvelope` rejects, without allocating:

- a buffer shorter than 12 bytes
- an `nType` other than 170
- a `dwSize` below 12
- a `dwSize` larger than the bytes actually received (truncation)
- a zero-length payload

`DecodeServerToClientEnvelope` additionally bounds the decompressed size. The ceiling
(`maxOutput`, default `kMaxDecompressedBatch` = 16384) is passed through as the
destination capacity to `lzo1x_decompress_safe`, so a hostile or truncated frame cannot
force an oversized write.

**Payload contents are validated in a separate stage, deliberately.** An envelope
claiming to be compressed has not been proven so until LZO accepts it; a raw payload's
inner messages are not proven until `ConnectionFramer` reads them. Both stages are
tested: `Batch_InnerFramerRejectsTruncatedInnerMessage` and
`Batch_InnerFramerRejectsInvalidInnerMessageSize` confirm a structurally valid envelope
can still carry a bad message stream, and that the framer catches it.

Malformed input returns `Status` / `CompressionOutcome`. Nothing throws, and no test
path can terminate the server.

### Limits used

| Constant | Value | Source |
| --- | ---: | --- |
| `kCompressEnvelopeSize` | 12 | `sizeof(NET_COMPRESS)` |
| `kCompressThreshold` | 1000 | `SendMsgBuffer.h:43` |
| `kMaxPacketSize` | 2048 | `SendMsgBuffer.h:37` |
| `kMaxDecompressedBatch` | 16384 | `NET_DATA_CLIENT_MSG_BUFSIZE` |

Note that the batch limit and the per-message limit are different numbers on purpose: a
*batch* of many messages legitimately exceeds the 2048-byte single-message cap, which is
exactly why conflating them would be wrong.

---

## 7. Relationship to minTea

minTea is **not** moved into, wrapped by, or aware of this layer.

Per V029, the ordering is:

```text
TEA-encrypt message fields   (minTea, per field, fixed widths)
        ↓
serialise to NET_MSG_GENERIC
        ↓
batch                        (ServerBatchEncoder)
        ↓
LZO 1X                       (CompressionCodec)
        ↓
NET_COMPRESS envelope        (NetCompressCodec)
        ↓
transport
```

Encryption is **inside** compression: fields are encrypted before the message is
serialised, so LZO sees already-encrypted bytes. Nothing in
`CompressionCodec.h`, `NetCompressCodec.h` or `ServerBatchEncoder.h` references the
encrypt key, and the compression layer would behave identically if minTea were absent.

---

## 8. Relationship to V027

Nothing in V027 was rewritten. `ConnectionFramer`, `NetworkTypes`, `NetworkCodec`,
`MessageRouter`, `ServerSession` and `LoopbackTransport` are unchanged, and the layer
sits *around* the framer rather than inside it:

```text
transport -> [NetCompress decode] -> ConnectionFramer -> MessageRouter -> ServerSession
```

The only V027-facing addition is `WellKnownMessage::kCompress` (= 170), which already
existed in `NetworkTypes.h` and is reused unchanged. No new message id was invented.

Two test names changed intent during V030 and are recorded in the commit history:
the 1000-byte boundary test was corrected after the implementation proved the original
assertion wrong.

---

## 9. Implementation

| File | Role |
| --- | --- |
| `modern/network/CompressionCodec.h/.cpp` | the only place LZO is named; `Lzo1xCodec` interface + `MinLzo1xCodec` backend |
| `modern/network/NetCompressCodec.h/.cpp` | 12-byte envelope encode/decode + validation |
| `modern/network/ServerBatchEncoder.h/.cpp` | batching state machine, 1000-byte flush trigger |
| `modern/network/third_party/minilzo/` | vendored miniLZO 2.10, unmodified |
| `modern/tests/NetworkCompressTests.cpp` | 38 new deterministic tests |

`Lzo1xCodec` is an injected interface so the backend can be replaced without touching
the envelope codec, the batcher, or their tests. There is no global singleton; a server
with concurrent senders should hold one codec per thread, because `MinLzo1xCodec` owns a
work buffer and is not internally synchronised.

### Dependency decision

`legacy/Tik/Library/lzo2.lib` is RAN's own prebuilt LZO2 library and was **rejected**:

1. It is a VC7.1 (VS2003) binary. Under MSVC 14.x it fails with
   `LNK2019: unresolved external symbol __except_handler4_common` unless
   `/NODEFAULTLIB:MSVCRT` suppresses the legacy CRT. A feasibility probe confirmed the
   override produces a working round-trip, but a build-wide CRT override is a poor trade.
2. It has no corresponding source anywhere in the tree — an unauditable binary.
3. It would make `ModernNetwork` depend on `legacy/`, which that target's own header
   comment forbids and which the root `CMakeLists.txt` deliberately avoids.

Vendored instead: **miniLZO 2.10** from `oberhumer.com`, archive SHA-1
`c7432708d49017a3f0b4f44c99d336f8a1be84f5`, **verified against the hash upstream
publishes**. Same `lzo1x_1_compress` / `lzo1x_decompress_safe` entry points legacy calls,
so the wire format is unaffected.

> **Open legal question.** miniLZO is distributed under **GPL-2.0-or-later** per the header
> of `minilzo.c`. LZO has a history of also being offered under terms permitting non-GPL
> use, but that allowance is **not** stated in the distributed headers, and this repository
> ships no project-level licence or `COPYING`. Whether embedding it here is compatible with
> this project's licensing is a determination for the project owner. Files are vendored
> byte-identical to upstream so the audit trail is unambiguous, and all LZO code is confined
> behind `Lzo1xCodec`. See `modern/network/third_party/minilzo/VENDOR.md`.

---

## 10. Tests

38 new tests in `ModernNetworkTests`, all deterministic: no socket, no clock, no RNG, no
external server. LZO payloads are validated **semantically** (decode and compare) rather
than against fixed bytes, because compression output is not guaranteed identical across
builds; envelope header bytes *are* asserted exactly, because those are fixed by the
legacy struct layout.

Coverage against the brief's required list:

| Required | Test |
| --- | --- |
| envelope encode/decode | `Envelope_RoundTripsCompressedBatch`, `Envelope_HeaderBytesAreExact` |
| empty/invalid envelope rejection | `Envelope_RejectsEmptyBatch`, `Envelope_RejectsInvalidMessageType` |
| LZO round-trip | `Compression_RoundTripsDeterministicPayload` |
| repetitive payload round-trip | `Compression_RoundTripsDeterministicPayload` |
| binary payload round-trip | `Compression_BinaryPayloadRoundTrips` |
| multiple `NET_MSG_GENERIC` messages | `Batch_ThreeMessagesSurviveEnvelopeInOrder` |
| 1000-byte threshold behaviour | `Batching_NineHundredNinetyNineBuffers`, `Batching_ExactlyOneThousandFlushes`, `Batching_OneThousandAndOneFlushes` |
| compressed-size validation | `Envelope_RejectsDeclaredSizeBeyondReceivedBytes` |
| uncompressed-size validation | `Decompression_DeclaredSizeMismatchIsCaughtByCeiling`, `Envelope_RawPayloadLargerThanOutputCeilingIsRejected` |
| malformed LZO rejection | `Decompression_RejectsMalformedLzoStream`, `Decompression_RejectsTruncatedCompressedStream` |
| truncated envelope rejection | `Envelope_RejectsTruncatedEnvelope` (every length 0..11) |
| inner framer receives decompressed messages | `Batch_ThreeMessagesSurviveEnvelopeInOrder` |

Plus: raw-fallback-still-wrapped, no phantom count, direction asymmetry, batching
`BUFFER_SEND_ADD` shape, oversized-message-alone flush, reset, fragment handling.

### Actual results

Executed during V030, both configurations:

| Suite | Debug | Release |
| --- | --- | --- |
| ModernCoreTests | 526/526 | 526/526 |
| ModernServerTests | 114/114 | 114/114 |
| ModernNetworkTests | **85/85** (was 47) | **85/85** |
| CTest | 15/15 | 15/15 |

Zero errors, zero warnings. Core and Server counts are unchanged from the V029 baseline
(526 / 114) — V030 touched neither.

---

## 11. Public/forum backread

Corroboration only; legacy source remained authority throughout.

- **LZO canonical usage** (`doc/LZO.TXT`, upstream): `lzo_init()` then
  `lzo1x_1_compress()` for compression, `lzo1x_decompress()` for decompression. Matches
  RAN's compressor; RAN's use of `lzo1x_decompress_safe` is the documented "packet crash
  fix" improvement over the canonical unsafe entry point.
- **LZO output bound**: upstream examples size the destination as
  `in + in/16 + 64 + 3`, which is what `CompressionCodec.cpp` and
  `NetCompress::MaxCompressedSize` use. Other wrappers in the wild append a 4-byte
  original-length trailer; **RAN does not**, and neither does this layer — the envelope's
  `dwSize` plus LZO's self-terminating stream is what bounds the read.
- **RAN EP8 changelog** (RaGEZONE) records "Fixed Server Files Crashes (LZO and packet
  length bugs)" and a change so "game will only identify packets that are comming from
  game.exe". Consistent with the envelope being validated by `nType` and with the
  framing layer having a history of length defects — which is the reason the threshold
  boundary and the truncation cases are pinned by tests here.

No public source contradicted any legacy finding, and nothing was substituted from a
forum implementation.

---

## 12. Remaining unknowns

1. **No packet capture of a real `NET_COMPRESS` frame.** The layer is derived from
   source and validated by round-trip tests against the same LZO algorithm RAN used, but
   no ASURA server→client frame has been observed. This is the same gap V029 carries, and
   it is why a capture, once obtainable, is worth doing: it would confirm the 12-byte
   envelope, the padding bytes, and the `bCompress` fallback in one shot.
2. **The 3 padding bytes' values.** Legacy leaves them indeterminate (struct padding
   never initialised). This layer writes zeros — equally compatible, and reproducible —
   but no capture has confirmed what the shipped servers actually emit.
3. **Client-side receive is implemented but unexercised end-to-end.** `MiniA.exe` is
   gated on nProtect Hackshield and its `param.ini` is Rijndael-encrypted, both recorded
   in V029 §9. The decode path is unit-tested against synthetic frames only.
4. **`lzo1x_decompress` vs `lzo1x_decompress_safe` on old peers.** This layer always
   uses the safe entry point, matching current legacy. A pre-"packet crash fix" peer that
   only implemented the unsafe path would still decode correctly, since the wire format
   is identical; this is a note, not a risk.
5. **Batch flush cadence relative to `SendClientFinal`.** The end-of-tick flush is
   implemented as an explicit `Flush()` call. Where it is driven from in the real server
   loop is part of WORLD-001, not of this layer.

---

## 13. Evidence index

| Claim | Source |
| --- | --- |
| `NET_COMPRESS` struct, 12 bytes | `legacy/Lib_Network/s_NetGlobal.h:2811-2819` |
| `NET_MSG_COMPRESS` = 170 | `legacy/Lib_Network/s_NetGlobal.h:720` |
| stale `Count(2)` diagram (trap) | `legacy/Lib_Network/SendMsgBuffer.h:29-34` |
| batching rule, 1000 threshold | `legacy/Lib_Network/SendMsgBuffer.cpp:65-113`; `SendMsgBuffer.h:43` |
| batch emission + `bCompress` fallback | `legacy/Lib_Network/SendMsgBuffer.cpp:116-172` |
| `BUFFER_SEND_ADD` driver (commented) | `legacy/Lib_Network/SendMsgBuffer.cpp:39-62` |
| end-of-tick flush, all connections | `legacy/Lib_Network/s_CClientManager.cpp:420-433` |
| `SendClient2` reads `dwSize` from envelope | `legacy/Lib_Network/s_CClientManager.cpp:489-527` |
| client sends raw, no envelope | `legacy/Lib_Network/s_NetClient.cpp:815-831`, `:924-953` |
| receive: envelope decode then peel | `legacy/Lib_Network/RcvMsgBuffer.cpp:88-186`, `:213-269` |
| decompress destination bound | `legacy/Lib_Network/RcvMsgBuffer.cpp:125` |
| LZO entry points | `legacy/Lib_Network/MinLzo.cpp:112`, `:158` |
| `lzo_init`, work memory | `legacy/Lib_Network/MinLzo.cpp:62`, `:78` |
| `CAN_NOT_COMPRESS` rule | `legacy/Lib_Network/MinLzo.cpp:118-131` |
| `packet crash fix` on safe decompress | `legacy/Lib_Network/MinLzo.cpp:157` |
| `lzo2.lib` link (and modern incompatibility) | `legacy/GameClient2/GameClient2.vcproj` `AdditionalDependencies`; V030 feasibility probe |
| vendored source hash | `modern/network/third_party/minilzo/VENDOR.md` |