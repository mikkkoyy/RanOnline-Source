# ASURA Network Protocol Authority (VERTICAL-028)

Status: **investigation complete**. Answers the three questions VERTICAL-027 left open,
and corrects one VERTICAL-027 conclusion that the evidence does not support.

Baseline: `be979b1ff974b3b70921d8349c6a61b811e3b48b` ("V027 Modern network boundary foundation").

## Authority order used

1. **ASURA shipped artifacts** — `D:\FILES\project\RanOnline-Build\ASURA CLIENT\`:
   `[1]ServerSession.exe`, `[2]ServerField.exe`, `[3]ServerAgent.exe`, `[4]ServerLogin.exe`,
   `MiniA.exe`, `Hackshield\`, `LauncherConfig.json`, `CFG\`.
2. **Original `*.vcproj`** — pre-conversion project files, authoritative for build macros.
3. **Legacy source** — `legacy/Lib_Network/`.
4. **Converted `*.vcxproj`** — derived, not authoritative.
5. **Public/forum material** — corroboration only.
6. **Inference** — explicitly labelled as such.

ASURA's `LauncherConfig.json` identifies the client as `"Region": "Philippines"`,
`"Executable": "MiniA.exe"`, built from `"ASURA SERVER EP9 (VS2022)"`. The binaries are
therefore from a **later revision than `legacy/`**; where they disagree, they win.

---

## Summary of answers

| Question | Answer |
| --- | --- |
| Active login variant | **Runtime-selected, 9 variants.** Not a compile-time macro. |
| Active crypt | **minTea**, static hardcoded key `"Steven Seagal Neck Break"`. No DH, no RSA. |
| `NET_MSG_COMPRESS` | **Unconditional**, never negotiated. **Breaks V027's framing model.** |

---

## A. The login variant is chosen at runtime, not by a macro

V027 and V026 both framed this as "which variant does ASURA compile in?". That is the
wrong question. `KR_PARAM` does **not** select a login variant.

`CAgentServer::MsgProc` dispatches on the received `nType`, with no `#if` anywhere in the
chain (`legacy/Lib_Network/s_CAgentServerMsg.cpp:61-71`):

```cpp
case NET_MSG_LOGIN_2 :        MsgLogIn( pMsg );        break;  // Taiwan / Hongkong
case CHINA_NET_MSG_LOGIN:     ChinaMsgLogin( pMsg );   break;
case THAI_NET_MSG_LOGIN :     ThaiMsgLogin( pMsg );   break;
case DAUM_NET_MSG_LOGIN :     DaumMsgLogin( pMsg );   break;
case TERRA_NET_MSG_LOGIN:     TerraMsgLogin( pMsg );   break;
case GSP_NET_MSG_LOGIN:       GspMsgLogin( pMsg );     break;
case EXCITE_NET_MSG_LOGIN:    ExciteMsgLogin( pMsg );  break;
case JAPAN_NET_MSG_LOGIN:     JapanMsgLogin( pMsg );   break;
case GS_NET_MSG_LOGIN:        GsMsgLogin( pMsg );      break;
```

V027's unresolved item listed five candidates. There are **nine**. V027's §4 table listed
only four and inferred `DAUM_` from `KR_PARAM`; both were wrong, for the reason below. The
full ID table is `legacy/Lib_Network/s_NetGlobal.h:770-804`; with `NET_MSG_BASE = 992` and
`NET_MSG_LOBBY = 992 + 950 = 1942`:

| Variant | Constant | Value | Payload |
| --- | --- | --- | --- |
| Taiwan/HK | `NET_MSG_LOGIN_2` | 2049 | userid, password, random password, encrypt key |
| Daum | `DAUM_NET_MSG_LOGIN` | 2052 | UUID |
| China | `CHINA_NET_MSG_LOGIN` | 2055 | userid/pass/rand + `RSA_ADD`=4 padding |
| GSP | `GSP_NET_MSG_LOGIN` | 2057 | UUID |
| Terra | `TERRA_NET_MSG_LOGIN` | 2062 | TID |
| Excite | `EXCITE_NET_MSG_LOGIN` | 2072 | — |
| Japan (Gonzo) | `JAPAN_NET_MSG_LOGIN` | 2074 | + `JAPAN_NET_MSG_UUID` (2076) |
| GS | `GS_NET_MSG_LOGIN` | 2077 | — |
| Thai | `THAI_NET_MSG_LOGIN` | 2082 | — |

**Consequence for WORLD-001:** a modern *server* must accept all nine. A modern *client*
must choose one to send, and that choice is the real open question — see *Unresolved*.

The structurally complete variant, and the one whose handler is the reference for crypt,
is `NET_MSG_LOGIN_2` (`s_NetGlobal.h:2916-2945`, handler
`s_CAgentServerMsgLogin.cpp:609-625`):

```cpp
struct NET_LOGIN_DATA
{
    NET_MSG_GENERIC nmg;                            // dwSize, nType
    int             nChannel;
    CHAR            szRandomPassword[USR_RAND_PASS_LENGTH+1];  // 6+1
    CHAR            szPassword      [USR_PASS_LENGTH+1];       // 20+1
    CHAR            szUserid        [USR_ID_LENGTH+1];         // 20+1
    CHAR            szEnCrypt       [ENCRYPT_KEY+1];           // 12+1
};
```

Note it is built from `CHAR`, not `TCHAR`. The `DAUM_`/`TERRA_`/`GSP_` structs use `TCHAR`
for their credential fields, so under a Unicode build their wire size would differ from the
Korean/Taiwanese path. See *Unresolved* item 3.

### Build macros (authority 2)

All four servers and both client projects define `KR_PARAM` and nothing else regional
(`legacy/*.vcproj`, `PreprocessorDefinitions`):

```
ServerLogin.vcproj   : WIN32;_WINDOWS;_DEBUG;KR_PARAM
ServerAgent.vcproj   : WIN32;_WINDOWS;_DEBUG;KR_PARAM
ServerSession.vcproj : WIN32;_WINDOWS;_DEBUG;KR_PARAM
ServerField.vcproj   : WIN32;_WINDOWS;_DEBUG;KR_PARAM
GameClient2.vcproj   : WIN32;_WINDOWS;_DEBUG;KR_PARAM
Lib_Client.vcproj    : WIN32;_DEBUG;_LIB;KR_PARAM
```

`NET_MSG_BASE` is `992` under every branch (`s_NetGlobal.h:655-672`), so the regional macro
does not perturb message IDs. `PH_PARAM` is **not** defined in any project file; V026's
open question about it is closed as *not present in the original build configuration*.

---

## B. The active crypt is minTea with a hardcoded static key

### B.1 The key is a compile-time constant

`legacy/Lib_Network/s_CAgentServer.h:87` declares `minTea m_Tea;` — the **default**
constructor. `legacy/Lib_Network/minTea.cpp:16-24`:

```cpp
minTea::minTea(void)
{
//  char* szKey = "Show me the money by SCV";
//  char* szKey = "Block Sheep Wall by Prob";
    char* szKey = "Steven Seagal Neck Break";
    setKey (szKey);
}
```

`minTea::setKey` is called from exactly two places: the two `minTea` constructors. **No
call site anywhere in the tree overrides it.** The two other matches (`DxViewPort::SetKeyboard`,
`PopupMenu::SetKey`) are unrelated.

So every Agent, Session, Field and client instance uses the same TEA key, hardcoded in
source.

### B.2 Confirmed in the ASURA binaries (authority 1)

Byte-scan of the shipped ASURA artifacts for the literal key:

| Binary | ASCII match |
| --- | --- |
| `[3]ServerAgent.exe` | yes |
| `[4]ServerLogin.exe` | yes |
| `[1]ServerSession.exe` | yes |
| `[2]ServerField.exe` | yes |
| `MiniA.exe` | yes |

This is the strongest available evidence: the key is present in **both** the shipped servers
**and** the shipped client, so minTea is the live crypt in ASURA and not a legacy leftover.

### B.3 What is actually encrypted

TEA is applied **per field, in place, over fixed-size buffers** — not over the whole
message. Server side, `s_CAgentServerMsgLogin.cpp:640-644`:

```cpp
int wPassLength = 0;                                     // md5 login = 0
if ( !RANPARAM::bFeatureRegisterUseMD5 ) wPassLength = 1;

m_Tea.decrypt (pNml->szUserid,         USR_ID_LENGTH+1);            // 21
m_Tea.decrypt (pNml->szPassword,       USR_PASS_LENGTH+wPassLength ); // 21 or 20
m_Tea.decrypt (pNml->szRandomPassword, USR_RAND_PASS_LENGTH+1);    // 7
m_Tea.decrypt (pNml->szEnCrypt,        ENCRYPT_KEY+1);              // 13
```

Heartbeat uses the same key over a 13-byte buffer (`s_CAgentServerMsg.cpp:43-56`,
`s_NetClientMsg.cpp:291-297`).

Two consequences that matter for implementation:

- The encrypted length is a **compile-time constant** (21/21/7/13), independent of the
  actual password length. Passwords longer than `USR_PASS_LENGTH` cannot be sent.
- The MD5/non-MD5 distinction changes the **encrypted byte count** (`+1`), so it is part of
  the wire format, not just an application policy.

### B.4 There is a second, shared secret — and it is not Diffie-Hellman

`NET_LOGIN_DATA::szEnCrypt` is not a negotiated key. It is a **pre-shared passphrase**
distributed over the server backbone at runtime:

1. `CLoginServer::GenerateEncrypt()` (`s_CLoginServer.cpp:307-314`) generates a random
   `ENCRYPT_KEY+1` = 13-character string into `m_szEncrypt`.
2. It is forwarded as `NET_MSG_SND_ENCRYPT_KEY` (`s_NetGlobal.h:3798-3812`) to **all online
   Agent slots** (`s_CSessionServerMsg.cpp:539-553`).
3. Each Agent stores it (`CAgentServer::SessionMsgEncryptKey`, `s_CAgentServerSession.cpp:193-200`)
   and relays it to its game client.
4. The client stores it (`CNetClient::MsgEncryptKey`, `s_NetClientMsg.cpp:159-165`),
   TEA-encrypts it into `szEnCrypt`, and the Agent verifies by **plain string compare**
   against its own copy (`s_CAgentServerMsgLogin.cpp:663-680`; mismatch ⇒
   `EM_LOGIN_FB_SUB_FAIL`, connection dropped).

So the "key exchange" is: *a random shared secret generated by one server and pushed to
everyone*, verified by equality. It is a **proof of shared backbone membership**, not a key
agreement. The same secret reappears in every heartbeat as a liveness proof.

`CRYPT_KEY` (`GetNewCryptKey`/`SetCryptKey`) *is* sent on accept
(`s_CAgentServerMsg.cpp:554-560`), but the code that would consume it is **commented out**
(`s_CAgentServerMsgLogin.cpp:628-635`). `CRYPT_KEY` is inert.

### B.5 Dead crypt code (authority 3)

| Unit | Verdict | Evidence |
| --- | --- | --- |
| `dhkey.cpp` / `dhkey.h` | **Dead** — no consumers | only self-referential hits in the whole tree |
| `des.cpp` (`CMessageStreamDES`) | **Dead** | only self-referential hits |
| `DaumGameCrypt.cpp`, `DaumGameAuth.cpp` | **Dead** | only self-referential hits |
| `ApexProxy.cpp` | **Partially live — see below** | — |

**Diffie-Hellman is not used.** `dhkey.*` has zero call sites in `legacy/`. V027 was right to
suspect it; it is not the answer.

### B.6 nProtect/Hackshield is a separate, live layer

This contradicts a naive "nProtect is dead code" reading of `legacy/`. `ApexProxy.cpp` has
no callers inside `Lib_Network`, yet the ASURA **servers** do contain the strings:

```
[3]ServerAgent.exe  => ApexProxy, nProtect, GameGuard
[4]ServerLogin.exe  => ApexProxy, nProtect, GameGuard
[1]ServerSession.exe=> ApexProxy, nProtect, GameGuard
MiniA.exe           => GameGuard
```

and `D:\FILES\project\RanOnline-Build\ASURA CLIENT\Hackshield\` ships a full nProtect
payload (`asc_main.dll`, `hshield.dat`, `ehsvc.dll`, `brinicle.dll`, …) plus a 2.9 MB
`hshield.log`. The protocol carries GameGuard messages in the GCTRL range
(`s_NetGlobal.h:927-933`: `NET_MSG_GAMEGUARD_AUTH` = GCTRL+20 … `_ANSWER_2` = GCTRL+25), and
the Agent probes GameGuard during accept (`MsgSndGameGuardFirstCheck`, `s_CAgentServerThread.cpp:243`).

**Interpretation (inference, flagged):** nProtect is an *authentication/anti-cheat*
sub-protocol layered on top of the transport crypt, and the ASURA build integrates it from
source not present in `legacy/`. It does **not** replace minTea for payload confidentiality
— the TEA key is demonstrably still in both binaries. See *Unresolved* item 2.

---

## C. Compression is unconditional and it invalidates V027's framing model

### C.1 There is no negotiation

`NET_MSG_COMPRESS = 170` (`s_NetGlobal.h:720`), defined on both client and server. Every
outbound batch is wrapped in it, unconditionally. There is no handshake message, no
capability flag, no opt-in.

### C.2 The wire format is TWO-LEVEL, not one-level

This is the substantive correction to V027.

Outbound (`SendMsgBuffer.cpp:65-172`), on every flush
(`CClientManager::SendClientFinal`, `s_CClientManager.cpp:420-433`):

1. Messages accumulate in `m_pBuffer` until `m_dwPos + dwSize >= COMPRESS_PACKET_SIZE`
   (**1000**, `SendMsgBuffer.h:43`). Below the threshold they keep queueing.
2. The **whole batch** is LZO-compressed (`CMinLzo::lzoCompress`).
3. The result is prefixed with a 12-byte `NET_COMPRESS` header.

```cpp
struct NET_COMPRESS                 // s_NetGlobal.h:2811-2819
{
    NET_MSG_GENERIC nmg;            // dwSize = 12 + payload, nType = 170
    bool            bCompress;
};
```

If LZO **fails**, the batch is still wrapped, with `bCompress = false` and the payload sent
**raw**:

```cpp
memcpy(m_pSendBuffer+sizeof(NET_COMPRESS), m_pBuffer, m_dwPos);
pNmc->bCompress = false;
```

So the outer frame is **not** a dispatched message, and its payload is **not** one message —
it is a concatenation of complete messages, each carrying its own `dwSize`/`nType`:

```
+----------------------------+-----------------------------------------------+
| NET_COMPRESS (12 bytes)    | Msg1 hdr+body | Msg2 hdr+body | Msg3 hdr+body |
| nType = 170                |  [possibly LZO-compressed as a single blob]   |
+----------------------------+-----------------------------------------------+
```

Inbound (`RcvMsgBuffer.cpp:88-186`) mirrors it: if `nType == 170`, decompress in place if
`bCompress`, then `getOneMsg` (`:213-269`) peels **one** message at a time by `dwSize` and
shifts the buffer.

Legacy documents this itself in the comment block at `RcvMsgBuffer.cpp:59-82` and `:84-87`
(`| Type1 | Size1 | Data1 | Type2 | Size2 | Data2 | ...`).

### C.3 Transformation order

**Outbound:** TEA-encrypt message fields → serialize → batch → LZO → wrap in
`NET_COMPRESS` → `WSASend`.
**Inbound:** receive → unwrap/decompress → peel message → TEA-decrypt fields → dispatch.

TEA is **inside** compression. Confirmed structurally: `m_Tea.decrypt` runs inside message
handlers on an already-received struct, while `SendClient` only queues (`s_CNetUser.cpp:1047`
→ `addMsg`), and compression happens later at flush time (`SendClientFinal`). Encrypting the
outer envelope instead would have been the opposite order and is not what the code does.

### C.4 Legacy quirks a faithful implementation must know

- **`bCompress == false` strips 12 bytes, not `dwSize`** (`RcvMsgBuffer.cpp:165-183`),
  whereas the compressed path strips `dwSize`. Correct only because an uncompressed batch
  payload is exactly the concatenated messages.
- **Header-first reads.** The server issues `WSARecv` for exactly `sizeof(NET_MSG_GENERIC)`
  = 8 bytes on accept, setting `NET_PACKET_HEAD` mode (`s_CAgentServerThread.cpp:216-227`).
- **`GARBAGE_DATA` anti-tamper is client-side only.** `getMsg(bool bClient)` applies it only
  when `bClient` is true (`RcvMsgBuffer.cpp:229-249`, `SetGarbageNum` at `:187-211`), matching
  the comment that the server-side path deliberately omits it. It shifts `dwSize` by a
  detected prefix length; a modern client must not receive server traffic through that path.
- **Validation guards** (`RcvMsgBuffer.cpp:100-106`, repeated at `:216-224`): reject
  `dwSize == 0`, `dwSize > NET_DATA_BUFSIZE` (2048), `dwSize < sizeof(NET_MSG_GENERIC)`.
  `NET_DATA_CLIENT_MSG_BUFSIZE` is 16384 — a **larger** receive buffer than the 2048 guard,
  which is why the two limits must not be conflated.

### C.5 Trap: the `Count(2)` field in the header comment does not exist

`SendMsgBuffer.h:29-34` carries a Doxygen diagram:

```
 * | Size(4) | Type(4) | Compress(1) | Count(2) | Data(...)             |
```

**There is no `Count` field.** The actual `NET_COMPRESS` is header + `bool bCompress` only,
i.e. 12 bytes with alignment padding. The comment predates the struct and was never
updated (its neighbours include two commented-out candidate keys and an empty `\todo`).

An implementer who trusts that diagram will emit or expect a 2-byte message count that the
real client and servers never send. V027 quoted the diagram without flagging it as stale.
There is no count on the wire; the batch is self-delimiting because each inner message
carries its own `dwSize`.

---

## Verdict on VERTICAL-027's `ConnectionFramer`

V027 recorded the framer as "provisionally valid only for uncompressed individual
messages". That is now settled, and the answer is **incomplete rather than wrong**.

`modern/network/NetworkConnection.h` is used **only** by `modern/tests/NetworkTests.cpp`;
no production code references it. So this is a latent gap, not a live defect.

What is correct:
- Header validation (`dwSize` below 8, above `kMaxPacketSize`, zero type → latch failure)
  mirrors the legacy `packet crash fix` guards.
- Accumulating concatenated messages and splitting partial feeds is exactly `getOneMsg`.
- All existing tests remain truthful; none asserts the single-level model as complete.

What is missing — required before WORLD-001 can talk to a real client:
1. An **envelope stage** ahead of the framer: detect `nType == 170`, honour `bCompress`,
   LZO-decompress, and feed the resulting message stream inward.
2. LZO (minilzo) is **not vendored** in `modern/`. This is a dependency decision, not a
   detail.
3. Re-check `dwSize > NET_DATA_BUFSIZE` (2048) as the per-message guard rather than
   `kMaxPacketSize` alone, and keep the 16384 client-buffer distinction explicit.

**No code was changed for this.** The gap is recorded here for the vertical that implements
the wire protocol; implementing the envelope plus login now would be the production login
that V028 is explicitly scoped to exclude.

---

## First message for WORLD-001

Not yet determined, and deliberately not guessed. The evidence establishes the *shape* of
the sequence but not which variant ASURA's client opens with.

The server-side accept sequence (`s_CAgentServerThread.cpp:216-243`) shows what the **Agent**
emits on accept, in order:

1. `WSARecv` 8 bytes, `NET_PACKET_HEAD` mode
2. `MsgSndCryptKey` — `NET_CRYPT_KEY` (`CRYPT_KEY`), **inert**, see B.4
3. `MsgSndRandomNumber` — `NET_RANDOMPASS_NUMBER`
4. `MsgSndGameGuardFirstCheck`

plus `SessionSndSvrCurState()` just before. So the client is passive at the start and the
Agent pushes. Note items 2–4 target a *client slot*, which for a game client means the
client must already be connected and have registered.

WORLD-001's first message cannot be fixed until the client→Agent variant is settled. See
*Unresolved* item 1.

---

## Unresolved — do not guess

1. **Which login variant the ASURA client sends.** The server accepts all nine at runtime,
   so the protocol is fully mapped, but choosing what a modern client emits requires either a
   packet capture of ASURA traffic or the `MiniA.exe`/`GameClient2` login-send routine. That
   routine is **not present** in `legacy/Lib_Client/` — searched for `NET_LOGIN_DATA`,
   `szRandomPassword`, `Login(`, with no construction site found. Highest-value next step.
2. **Extent of nProtect interposition.** `nProtect`/`GameGuard` strings are in the ASURA
   servers and a full `Hackshield\` payload ships, but the integrating source is absent from
   `legacy/`. Whether GameGuard gates *before* `NET_MSG_LOGIN_2` and whether any traffic is
   additionally transformed by it is **unknown**.
3. **`TCHAR` vs `CHAR` in the DAUM/TERRA/GSP structs.** Those handlers read credential
   fields declared `TCHAR`. Under the project's Unicode configuration their wire size differs
   from the `CHAR`-based `NET_LOGIN_DATA`. The default server `dwSize` for those variants is
   therefore **unverified**, and a fixed-width implementation must not assume they match.
4. **`RANPARAM::bFeatureRegisterUseMD5` value for ASURA.** It changes the encrypted password
   byte count (`+1`, see B.3), so it is wire-visible. `RANPARAM.cpp` reads it from an
   encrypted `config.ini`/`param.ini`; the ASURA `config.ini` is obfuscated and was not
   decoded.
5. **Exact LZO variant and dictionary.** `CMinLzo` wraps minilzo, but the variant selection
   is not pinned down in source, and byte-exact interoperability with `MiniA.exe` is
   unconfirmed. No capture was taken.

---

## Evidence index

| Claim | Source |
| --- | --- |
| 9 runtime login variants | `legacy/Lib_Network/s_CAgentServerMsg.cpp:61-71` |
| Login ID table | `legacy/Lib_Network/s_NetGlobal.h:770-804` |
| `NET_LOGIN_DATA` layout | `legacy/Lib_Network/s_NetGlobal.h:2916-2945` |
| TEA field decryption + MD5 width | `legacy/Lib_Network/s_CAgentServerMsgLogin.cpp:640-644` |
| TEA static key | `legacy/Lib_Network/minTea.cpp:16-24`; `s_CAgentServer.h:87` |
| TEA key in ASURA binaries | byte scan of `[1..4]Server*.exe`, `MiniA.exe` |
| Shared-secret generation | `legacy/Lib_Network/s_CLoginServer.cpp:307-314` |
| Secret fan-out to Agents | `legacy/Lib_Network/s_CSessionServerMsg.cpp:539-553` |
| Secret relay to game client | `legacy/Lib_Network/s_CAgentServerSession.cpp:193-200`; `s_NetClientMsg.cpp:159-165` |
| `CRYPT_KEY` code commented out | `legacy/Lib_Network/s_CAgentServerMsgLogin.cpp:628-635` |
| `dhkey`/`des`/`Daum` dead | whole-tree reference search, self-referential only |
| nProtect strings in ASURA | byte scan of `[3]ServerAgent.exe`, `[4]ServerLogin.exe`, `[1]ServerSession.exe` |
| GameGuard message IDs | `legacy/Lib_Network/s_NetGlobal.h:927-933` |
| Compression threshold 1000 | `legacy/Lib_Network/SendMsgBuffer.h:43` |
| Batch + LZO + wrap | `legacy/Lib_Network/SendMsgBuffer.cpp:65-172` |
| Uncompressed fallback still wrapped | `legacy/Lib_Network/SendMsgBuffer.cpp:151-166` |
| Decompress then peel | `legacy/Lib_Network/RcvMsgBuffer.cpp:88-186`, `:213-269` |
| Two-level wire diagram (legacy's own) | `legacy/Lib_Network/RcvMsgBuffer.cpp:59-82` |
| Validation guards | `legacy/Lib_Network/RcvMsgBuffer.cpp:100-106`, `:216-224` |
| Flush point (compression after queueing) | `legacy/Lib_Network/s_CClientManager.cpp:420-433`; `s_CNetUser.cpp:1047` |
| Accept sequence | `legacy/Lib_Network/s_CAgentServerThread.cpp:216-243` |
| `KR_PARAM` everywhere | `legacy/ServerLogin.vcproj`, `ServerAgent.vcproj`, `ServerSession.vcproj`, `ServerField.vcproj`, `GameClient2.vcproj`, `Lib_Client.vcproj` |
| `NET_MSG_BASE == 992` all branches | `legacy/Lib_Network/s_NetGlobal.h:655-672` |
| ASURA identity / region | `D:\FILES\project\RanOnline-Build\ASURA CLIENT\LauncherConfig.json` |
| ASURA region consistent with PH sources | phcorner.org EP7 setup thread; RaGEZONE "Ran Development" |

Public sources corroborate the episode/sourcing picture (EP7/EP9, "Asura" as a named
source, PH-based) but contain **no** protocol-level detail that contradicts the above, and
nothing that could have resolved any *Unresolved* item.