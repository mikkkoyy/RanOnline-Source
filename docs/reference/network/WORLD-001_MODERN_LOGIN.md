# WORLD-001 — Modern Client → Server Login

Status: **implemented and tested.** Scope is exactly the brief's: modern client →
TCP → modern login server → decode/validate login request. Character list, lobby,
world entry, character creation and gameplay connection are **not** implemented.

Baseline: `38da4728e95c9969134a0e890477382f5f631861` ("V030-A RAN client tools source inventory").

## Evidence grades used throughout

| Tag | Meaning |
| --- | --- |
| **CONFIRMED** | read directly from legacy/ASURA source, or demonstrated by a passing test |
| **PUBLIC CORROBORATED** | independently confirmed in a separate public RAN repository |
| **INFERRED** | follows from confirmed facts, but not itself directly observed |
| **UNRESOLVED** | not determinable from available evidence; **not** assumed anywhere in code |

---

## 1. Protocol summary

**CONFIRMED.** Client → server login travels **raw**. No `NET_COMPRESS` envelope, no
LZO, no batching. Established by V028/V029 from
`CNetClient::SendBuffer2` (`s_NetClient.cpp:815-831`), which passes one message's
`dwSize` straight to `::send`.

### The two views of the packet

| | value | source |
| --- | ---: | --- |
| Logical body (`NET_LOGIN_DATA`) | **76 bytes** | V029 compiler-verified, x86 MSVC |
| Wire frame | **76 + g** | `SendMsgAddGarbageValue` inflates `dwSize` |
| `g` (garbage token) | 6, 7 or 9 | `GARBAGE_DATA`, `RcvMsgBuffer.cpp:11-12` |

### Layout

```
offset  size  field                notes
------  ----  -------------------  ---------------------------------------------
     0     4  dwSize               u32 LE; INCLUDES header and garbage token
     4     4  nType                u32 LE; always 2049 (NET_MSG_LOGIN_2, 0x0801)
     8     g  garbage token        one of "L8IDUL","M7HSET","N6GNET"(6), "K9IHANA"(7),
                                    "O5FDASEOT"(9)
  8+g     4  nChannel              i32 LE
12+g     7  szRandomPassword      see §4 - NOT encrypted
19+g    21  szPassword            minTea
40+g    21  szUserid              minTea
61+g    13  szEnCrypt             minTea
74+g     2  struct padding        indeterminate in legacy, zero here
    --    --  dwSize               = 76 + g
```

Field capacities (`USR_ID_LENGTH` 20, `USR_PASS_LENGTH` 20, `USR_RAND_PASS_LENGTH` 6,
`ENCRYPT_KEY` 12 — all +1 for the NUL, which is transmitted because the whole fixed
field is TEA-processed).

### Order of operations — CONFIRMED

```
TCP receive
    -> NET_MSG_GENERIC framing
    -> is it NET_MSG_LOGIN_2?
    -> IDENTIFY the garbage token by matching bytes at offset 8
    -> STRIP it
    -> NOW validate the body is exactly 76 bytes
    -> minTea-decrypt the fields
    -> validate credentials
```

Stripping before size validation is not optional. Legacy gets the ordering right by
layering: the strip happens in `CNetUser::GetMsg` (`s_CNetUser.cpp:552-577`, with
`bClient == true`) and adjusts `dwSize`, so `MsgLogIn`'s
`sizeof(NET_LOGIN_DATA) != dwSize` check (`s_CAgentServerMsgLogin.cpp:618`) compares
against an already-corrected 76. `LoginProtocol::Decode` performs both steps in one
place, in order.

Garbage is identified by **content**, never by guessing a length
(`SetGarbageNum`, `RcvMsgBuffer.cpp:187-211`). An unrecognised token is rejected, not
tolerated — otherwise garbage bytes would be fed to the field decryptor as ciphertext.

---

## 2. minTea — CONFIRMED

| Property | Value | Source |
| --- | --- | --- |
| Algorithm | TEA, Wheeler & Needham Oct 1998 | `minTea.cpp`, delta `0x9E3779B9` |
| Cycles | `q = 6 + 52/n` per block-pair count `n` | `floor(6 + 52.0f/n)` |
| Key | `"Steven Seagal Neck Break"` | `minTea.cpp:16-24`, default ctor |
| Key schedule | repeat key chars to `TEA_KEY_LENGTH` (16) | `minTea.cpp:29-44` |
| Key usage | `k[p&3 ^ e]`, `e = (sum>>2) & 3` | `minTea.cpp` |

**PUBLIC CORROBORATED.** `yexiuph/RanOnline` (Apache-2.0, EP7) carries a byte-identical
default constructor, *including both commented-out alternative keys*
("Show me the money by SCV", "Block Sheep Wall by Prob"). Its `SndLogin` is
logically identical to ours — the same four `StringCchCopy` calls and the same four
`m_Tea.encrypt` calls at the same widths.

The key is protocol, not configuration. Changing it changes the wire format.

### The in-place cipher is NOT "encrypt 21 bytes" — CONFIRMED

This is the single most important property, and getting it wrong produces a packet no
RAN server accepts.

`minTea::encrypt(char*, int nMaxLength)` (`minTea.h:272`) does:

1. scan backwards from `nMaxLength` while the byte is NUL → `len` (the string length)
2. grow to a minimum of **5** (`while (len <= 4) ++len`)
3. round up to a multiple of **4**
4. TEA-encrypt exactly that many bytes
5. write them back; **bytes past them are left untouched**

So the ciphertext length **varies per field** and is not the field width:

| plaintext length | 1–8 | 9–12 | 13–16 | 17–20 | 21 |
| --- | ---: | ---: | ---: | ---: | ---: |
| ciphertext bytes | 8 | 12 | 16 | 20 | 24 |

Tested by `Login_TeaCipherLengthIsNotTheFieldWidth` and
`Login_TeaRoundTripsAcrossEveryCipherLength`.

> **Legacy cannot be compiled in this repository.** `minTea.cpp` includes `"minlzo.h"`
> (absent) and uses an `acCArray` container (also absent). V030-A established the same
> class of gap. The rules above are derived from the source that *is* present and are
> pinned by round-trip tests rather than assumed.

---

## 3. §8 GATE — `bFeatureRegisterUseMD5` — RESOLVED

This was V029's most important open item. **It does not affect the wire format.**

Full reference set in the tree (5 references, exhaustive):

| Location | Role |
| --- | --- |
| `RANPARAM_FEATURE.cpp:42` | definition, `BOOL bFeatureRegisterUseMD5 = FALSE;` |
| `RANPARAM_FEATURE.cpp:131` | read from `[GAME_FEATURE] bFeatureRegisterUseMD5` |
| `s_CAgentServerMsgLogin.cpp:639` | **the only login-side use** |
| `s_NetClientMsgRegister.cpp:45,63` | register-side use (sends MD5 hex) |

Answering each of the brief's ten questions:

1. **Initialised** — `RANPARAM_FEATURE.cpp:42`, default `FALSE`.
2. **Configured by** — `[GAME_FEATURE] bFeatureRegisterUseMD5` in `Config.ini`
   (`RANPARAM_FEATURE.cpp:90-95`), loaded by `RANPARAM::LOAD_FEATURE`
   (`RANPARAM_MAIN.cpp:25`). Not a country macro.
3. **Read by** — `s_CAgentServerMsgLogin.cpp:639`, and only there on the login path.
4. **Wire width changed?** **No.** The struct field is always 21 bytes, `dwSize` is
   always 76 + g.
5. **Encryption width changed?** **Yes, and only this.** It changes the server's TEA
   *decrypt window* from `USR_PASS_LENGTH+1` (21) to `USR_PASS_LENGTH` (20):
   ```cpp
   int wPassLength = 0;                                        // md5 login = 0
   if ( !RANPARAM::bFeatureRegisterUseMD5 ) wPassLength = 1;
   m_Tea.decrypt (pNml->szPassword, USR_PASS_LENGTH+wPassLength );
   ```
6. **Database comparison changed?** **No.** `CAgentUserCheck::Execute` passes
   `m_strPasswd.GetString()` straight to `COdbcManager::UserCheck`
   (`s_CDbActionUser.cpp:396-410`) — no hashing on this path.
7. **Client always sends 21 bytes?** **Yes** — `s_NetClientMsgLogin.cpp:60`, with no
   MD5 branch in `SndLogin` at all.
8. **Does the server accept both modes?** It has one configured width, but both yield
   the same result because minTea recovers the ciphertext length by scanning back over
   NULs. A short ciphertext decrypts identically under either window.
9. **Country-specific?** **No** — a GAME_FEATURE config flag.
10. **Registration, login, or both?** **Both** — but asymmetrically. Registration
    *does* MD5 on the client (`:45,63`); `SndLogin` does **not**. With MD5 mode on,
    registration stores an MD5 hex while login sends plaintext. That is a genuine
    legacy inconsistency, recorded here and deliberately **not** reproduced.

**Both widths are implemented and tested** (`LoginProtocol::PasswordDecryptWidth`), with
the default `Full21` matching the legacy default. `Login_Md5ConfigurationDoesNotChangeTheWire`
proves the bytes are identical either way; `Login_Md5TruncatedWindowAcrossEveryPasswordLength`
sweeps all 19 usable password lengths.

**UNRESOLVED:** the value for ASURA. `Config.ini` is Rijndael-encrypted and chunked, and
was not decoded (V029 §9). This does not affect the wire, which is why the vertical
proceeded.

---

## 4. A legacy quirk that is wire behaviour, not a bug — CONFIRMED

**The 7-byte `szRandomPassword` field is never encrypted.**

- The field is `USR_RAND_PASS_LENGTH+1` = 7 bytes; a 6-digit random number gives an
  effective length of 6, which pads to an **8**-byte ciphertext.
- `minTea::encrypt` computes the ciphertext, then hits
  `if (nEncryptedLength > nMaxLength) return false;` (`minTea.cpp`) — 8 > 7 — and
  **copies nothing**.
- `CNetClient::SndLogin` ignores that return value, so the field goes out as the
  plaintext the constructor left there.
- The server mirrors it exactly: `m_Tea.decrypt(szRandomPassword, 7)` hits the same
  guard, declines, and leaves the field alone. The Agent reads the plaintext value and
  checks it.

The failure is **symmetric**, which is why it has never surfaced as a problem.
`MinTea::CanEncryptInPlace` / `CanDecryptInPlace` reproduce it, and two tests pin it:
`Login_RandomPasswordFieldIsNotEncryptedAndThatIsCorrect` and
`Login_RandomPasswordTravelsAsPlaintextOnTheWire` — the latter asserts the random
password bytes are readable on the wire while the password is not.

**An implementation that encrypted all 7 bytes would be *less* compatible.**

---

## 5. Architecture

```
        core          <-  network  server  client
          |
     compatibility/legacy --> legacy/     (untouched by WORLD-001)
```

| Component | Target | Role |
| --- | --- | --- |
| `network/MinTeaCodec.h/.cpp` | `ModernNetwork` | the cipher; the only place TEA exists |
| `network/LoginProtocol.h/.cpp` | `ModernNetwork` | `NET_MSG_LOGIN_2` encode/decode, garbage token |
| `server/login/LoginReceiver.h/.cpp` | `ModernServer` | frame → verdict; shared-secret check |
| `client/login/World001LoginClient.h` | `ModernClientLogin` | builds the request; owns no socket |

**One dependency change:** `ModernServer` now links `ModernNetwork` as well as `Modern`.
A server that receives bytes depending on the network codec is the intended direction,
and `LoginReceiver` takes an already-framed buffer — it cannot see a socket. The change
is recorded in `modern/server/CMakeLists.txt`.

`ModernClientLogin` is an `INTERFACE` library because it is header-only; a `STATIC`
target with no compilable source emits no archive and fails to link.

### Boundary separation — the point of the split

- `ILoginAuthenticator` receives **already-decoded plaintext credentials**. It has no
  idea TEA or framing exist, and must never be handed ciphertext — so a future account
  store cannot accidentally compare encrypted bytes.
- `LoginReceiver` knows framing and never sees an account.
- The shared-secret check lives in `LoginReceiver`, not the authenticator: it proves the
  client is on the same server cluster, it is not a credential.

`InMemoryLoginAuthenticator` ships for testability and holds plaintext in process
memory — acceptable in tests, unacceptable in production. That is exactly why it sits
behind the interface.

---

## 6. Deviations from legacy — every one justified

| # | Deviation | Why |
| --- | --- | --- |
| 1 | Explicit little-endian field serialisation instead of `memcpy` of a native struct | The brief's §10 rule. Bytes on the wire are identical to legacy on x86; the guarantee becomes explicit and testable rather than dependent on compiler and alignment. |
| 2 | Struct padding written as **zero** | Legacy leaves it indeterminate and never reads it. Zero is reproducible; `dwSize` counts it either way. |
| 3 | Garbage identify-and-strip done in the codec, not one layer above | Legacy's ordering is a side effect of layering. Doing it explicitly makes the invariant visible. Same result. |
| 4 | Unknown garbage token **rejected** | Legacy would also refuse (`SetGarbageNum` returns −1), but only implicitly. Refusing explicitly prevents garbage reaching the decryptor. |
| 5 | Empty user id rejected at the protocol boundary | Legacy queries the database with an empty string. Catching it keeps the failure out of persistence. |
| 6 | Cipher errors other than the length-decline are **not** silently ignored | Legacy ignores every return. Tolerating only the documented decline is the narrow reading; swallowing a real fault would be indefensible. |
| 7 | Trailing field content: encrypt/decrypt **must** tolerate a non-NUL tail | minTea handles this natively; the modern layer must not assume NUL padding. |

No deviation changes any protocol-visible byte.

---

## 7. Tests

Added 30 network tests and 11 server tests. All deterministic: no socket, no clock, no
RNG, no database, no external server, no HackShield, no `MiniA.exe`, no real
credentials.

### Brief's required list

| # | Test |
| --- | --- |
| 1 Header | `Login_FrameHeaderIsExact` |
| 2 Body 76 bytes | `Login_LogicalBodyIsSeventySixBytes` |
| 3 TEA 21/21/7/13 | `Login_TeaRoundTripsUserIdField21`, `…PasswordField21`, `…RandomPasswordField7`(→§4), `…EncryptField13` |
| 4 Full packet | `Login_FullPacketStructure`, `Login_ChannelIsLittleEndian` |
| 5 Garbage 6 | `Login_GarbageLengthSixIsAccepted` |
| 6 Garbage 7 | `Login_GarbageLengthSevenIsAccepted` |
| 7 Garbage 9 | `Login_GarbageLengthNineIsAccepted` |
| 8 Invalid garbage | `Login_UnknownGarbageTokenIsRejected`, `Login_ArbitrarySuffixLengthIsRejected`, `Login_MissingGarbageTokenIsRejected`, `Login_EncoderRefusesAnUnknownToken` |
| 9 Too short | `Login_TruncatedPacketIsRejected` |
| 10 Too large | `Login_OversizedBodyIsRejected` |
| 11 Wrong type | `Login_WrongMessageTypeIsRejected` |
| 12 Wrong credentials | `LoginReceiver_WrongPasswordIsRejectedAsBadPassword`, `…UnknownAccountIsDistinguished…` |
| 13 Valid credentials | `LoginReceiver_ValidCredentialsAreAccepted`, `…EveryGarbageTokenAuthenticates` |
| 14 MD5 configuration | `Login_Md5ConfigurationDoesNotChangeTheWire`, `Login_Md5TruncatedWindowAcrossEveryPasswordLength` |
| 16 Loopback | `Login_EndToEndOverLoopbackTransport` |

Plus: the direction guard `Login_NeverRoutesThroughTheCompressionLayer`, which fails if a
login frame is ever a `NET_COMPRESS` envelope.

### Actual results — executed this milestone

| Suite | Debug | Release |
| --- | --- | --- |
| ModernCoreTests | 526/526 | 526/526 |
| ModernServerTests | **125/125** (was 114) | **125/125** |
| ModernNetworkTests | **115/115** (was 85) | **115/115** |
| ModernClientAppTests | 16/16 | 16/16 |
| ModernClientInputTests | 25/25 | 25/25 |
| CTest | 15/15 | 15/15 |

Zero errors, zero warnings. Core is unchanged at 526.

---

## 8. Not implemented — explicitly

Out of scope and **not** written: login feedback packet (`NET_MSG_LOGIN_FB`), character
list, character creation, lobby/server list, world entry, field connection, session and
agent handoff, gameplay, movement, inventory, skills, combat, chat, guild, quest,
database migration, account registration, password reset, launcher, HackShield,
GameGuard.

`LoginResult` says accepted or rejected and nothing else. No code implies a character
exists.

`LoginReceiver` does not send a reply. The legacy accept path also sends a random
password, a channel/server list and a full-server notice
(`s_CAgentServerMsgLogin.cpp:680-760`), and the failure path sends `NET_MSG_LOGIN_FB`.
Those are the next vertical: the server→client response path, which **does** require the
V030 compression layer.

---

## 9. Unresolved

1. **`bFeatureRegisterUseMD5` for ASURA** — its value is unread (`Config.ini` is
   Rijndael-encrypted and chunked). Does not affect the wire; only the decrypt window.
   Documented, not assumed.
2. **No packet capture.** Everything here is derived from source and validated by
   round-trip tests. No real ASURA login packet has been observed, so the 76/82/83/85
   sizes and the padding bytes remain source-derived rather than measured. V029 records
   why capture is blocked.
3. **The shared-secret distribution path is not modelled.** `m_szEncryptKey` is pushed
   by the Login server over the server backbone (V028 §B.4). `LoginReceiver` only
   compares it. How a modern cluster publishes and rotates it is unspecified.
4. **The 20-character password edge case in MD5 mode.** With a 20-byte decrypt window a
   20-character password has its NUL outside the decrypted region, so legacy truncates
   it to 19. Not reproduced as a special case — it follows from the legacy code and is
   reachable only when MD5 mode is on, whose ASURA value is unresolved.
5. **`acCArray` semantics inferred.** `minTea.cpp` cannot be compiled here, so
   `GetLength`/`SetLength`/`PushLast` behaviour is reconstructed from usage. The wire
   behaviour is unaffected either way (the pushed NUL lands in an already-NUL region),
   but the inference is stated rather than hidden.

---

## 10. Evidence index

| Claim | Source |
| --- | --- |
| `NET_LOGIN_DATA` layout, 76 bytes | `legacy/Lib_Network/s_NetGlobal.h:2916-2945`; V029 compiler probe |
| `NET_MSG_LOGIN_2` = 2049 | `legacy/Lib_Network/s_NetGlobal.h:770` |
| client sends it, raw | `legacy/Lib_Network/s_NetClientMsgLogin.cpp:28-68` |
| no envelope client→server | `legacy/Lib_Network/s_NetClient.cpp:815-831` |
| garbage insertion | `legacy/Lib_Network/s_NetClient.cpp:893-922`, `:872-891` |
| `GARBAGE_DATA` tokens | `legacy/Lib_Network/RcvMsgBuffer.cpp:11-12` |
| server identifies garbage | `legacy/Lib_Network/RcvMsgBuffer.cpp:187-211` |
| server strips garbage | `legacy/Lib_Network/RcvMsgBuffer.cpp:229-249`; `s_CNetUser.cpp:552-577` |
| server size check after strip | `legacy/Lib_Network/s_CAgentServerMsgLogin.cpp:618` |
| MD5 decrypt window | `legacy/Lib_Network/s_CAgentServerMsgLogin.cpp:637-642` |
| MD5 config source | `legacy/Lib_Engine/Utils/RANPARAM_FEATURE.cpp:42,90-95,131` |
| password goes to DB unhashed | `legacy/Lib_Network/s_CDbActionUser.cpp:396-410` |
| minTea key | `legacy/Lib_Network/minTea.cpp:16-24` |
| minTea key schedule | `legacy/Lib_Network/minTea.cpp:29-44` |
| TEA rounds, delta, `e`, `k[]` | `legacy/Lib_Network/minTea.cpp` (`encrypt(UINT*,UINT,UINT*)`) |
| in-place NUL-strip + padding | `legacy/Lib_Network/minTea.cpp` (`encrypt(char*,acCArray&,int)`) |
| length-decline no-op | `legacy/Lib_Network/minTea.cpp` (`if (nEncryptedLength > nMaxLength) return false`) |
| shared-secret check | `legacy/Lib_Network/s_CAgentServerMsgLogin.cpp:664-677` |
| `CharacterSet="2"` MBCS | all 29 `legacy/**/*.vcproj` (V029) |
| login call site (V030-A) | `legacy/Lib_ClientUI/Interface/LoginPage.cpp:258` |
| public TEA key + widths | `yexiuph/RanOnline` `Dependency/NetGlobal/minTea.cpp`, `[Lib]__NetClient/Sources/s_NetClientMsgLogin.cpp` |
| `lzo2.lib`, no LZO source | V030-A §5; `yexiuph/RanOnline` `Dependency/lzo/` (same shape) |