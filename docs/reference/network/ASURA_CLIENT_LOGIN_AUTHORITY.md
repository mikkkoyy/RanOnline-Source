# ASURA Client Login Authority (VERTICAL-029)

Status: **RESOLVED** — the ASURA client's login variant is identified from source and
corroborated against the shipped binary. One confirmation step (packet capture) was not
achievable in this environment and is recorded as outstanding, not as a gap in the answer.

Baseline: `53309a1466439925ab3c33ef9945a640f5e4cb9b` ("V028 establish ASURA network protocol authority").

## Answer

```text
ASURA client sends:  NET_MSG_LOGIN_2   =  2049  (0x0801)
struct:              NET_LOGIN_DATA
sizeof:              76 bytes (compiler-verified, x86, MBCS)
field encoding:      minTea, per-field, fixed widths 21 / 21 / 7 / 13,
                     key "Steven Seagal Neck Break"
wire dwSize:         82, 83 or 85  (76 + a 6/7/9-byte garbage token - see §7)
compression:         none on this direction (client sends raw)
outer envelope:      none on this direction
```

The answer is **independent of whether ASURA was built with `KR_PARAM` or `PH_PARAM`**. Both
resolve to the same variant. See §4 — this is the reason the answer is safe despite the
build macro being unproven.

---

## 1. The nine server-accepted variants

From `CAgentServer::MsgProc`, `legacy/Lib_Network/s_CAgentServerMsg.cpp:61-71`. Values
computed from `NET_MSG_BASE = 992` and `NET_MSG_LOBBY = 992 + 950 = 1942`. `sizeof` values
are **compiler-verified**, not hand-counted (§6).

| nType | Constant | Struct | Server handler | Fields after the 8-byte header | sizeof |
| ---: | --- | --- | --- | --- | ---: |
| 2049 | `NET_MSG_LOGIN_2` | `NET_LOGIN_DATA` | `MsgLogIn` | `int nChannel`, `CHAR[7]`, `CHAR[21]`, `CHAR[21]`, `CHAR[13]` | **76** |
| 2052 | `DAUM_NET_MSG_LOGIN` | `DAUM_NET_LOGIN_DATA` | `DaumMsgLogin` | `int nChannel`, `TCHAR[37]` UUID | 52 |
| 2055 | `CHINA_NET_MSG_LOGIN` | `CHINA_NET_LOGIN_DATA` | `ChinaMsgLogin` | `int nChannel`, `TCHAR[11]`, `TCHAR[25]`, `TCHAR[25]` | 76 |
| 2057 | `GSP_NET_MSG_LOGIN` | `GSP_NET_LOGIN_DATA` | `GspMsgLogin` | `int nChannel`, `TCHAR[37]` UUID | 52 |
| 2062 | `TERRA_NET_MSG_LOGIN` | `TERRA_NET_LOGIN_DATA` | `TerraMsgLogin` | `int nChannel`, `TCHAR[97]` TID | 112 |
| 2072 | `EXCITE_NET_MSG_LOGIN` | `EXCITE_NET_LOGIN_DATA` | `ExciteMsgLogin` | `int nChannel`, `CHAR[21]`, `CHAR[15]`, `CHAR[33]` | 84 |
| 2074 | `JAPAN_NET_MSG_LOGIN` | `JAPAN_NET_LOGIN_DATA` | `JapanMsgLogin` | `int nChannel`, `CHAR[17]`, `CHAR[17]`, `CHAR[13]` | 60 |
| 2077 | `GS_NET_MSG_LOGIN` | `GS_NET_LOGIN_DATA` | `GsMsgLogin` | `int nChannel`, `CHAR[21]`, `CHAR[21]`, `CHAR[13]` | 68 |
| 2082 | `THAI_NET_MSG_LOGIN` | `THAI_NET_LOGIN_DATA` | `ThaiMsgLogin` | `int nChannel`, `CHAR[21]`, `CHAR[21]` | 56 |

Constant-length inputs (`s_NetGlobal.h`): `USR_ID_LENGTH` 20, `USR_RAND_PASS_LENGTH` 6,
`ENCRYPT_KEY` 12, `RSA_ADD` 4, `UUID_STR_LENGTH` 37, `TERRA_TID_ENCODE` 96,
`EXCITE_TDATE` 14, `EXCITE_MD5` 32, `JAPAN_USER_ID`/`JAPAN_USER_PASS` 16,
`GS_USER_ID`/`GS_USER_PASS` 20. `USR_PASS_LENGTH == USR_ID_LENGTH == 20`.

---

## 2. Client sender location — V028 was wrong

V028 reported:

> "That routine is **not present** in `legacy/Lib_Client/` — searched for `NET_LOGIN_DATA`,
> `szRandomPassword`, `Login(`, with no construction site found."

That conclusion was **incorrect**, and §7 of the V029 brief asked for it to be verified
independently. It was: the file exists, in a different library than V028 searched.

**`legacy/Lib_Network/s_NetClientMsgLogin.cpp`** (346 lines) contains all nine client
senders:

| Sender | Line | Emits |
| --- | ---: | --- |
| `CNetClient::SndLogin` | 28 | `NET_LOGIN_DATA` → `NET_MSG_LOGIN_2` |
| `CNetClient::JapanSndLogin` | 70 | `JAPAN_NET_LOGIN_DATA` |
| `CNetClient::ChinaSndLogin` | 111 | `CHINA_NET_LOGIN_DATA` |
| `CNetClient::DaumSndLogin` | 161 | `DAUM_NET_LOGIN_DATA` |
| `CNetClient::GspSndLogin` | 194 | `GSP_NET_LOGIN_DATA` |
| `CNetClient::TerraSndLogin` | 215 | `TERRA_NET_LOGIN_DATA` |
| `CNetClient::ExciteSndLogin` | 252 | `EXCITE_NET_LOGIN_DATA` |
| `CNetClient::ThaiSndLogin` | 291 | `THAI_NET_LOGIN_DATA` |
| `CNetClient::GsSndLogin` | 326 | `GS_NET_LOGIN_DATA` |

V028 searched `Lib_Client/`; the senders live in `Lib_Network/`. Note also that V028's
`Grep` for `NET_MSG_LOGIN` hit only the *server* dispatch and the *struct definitions* —
both in files it did read. The client file was simply never reached.

**Corrected chain for the answer** (`s_NetClientMsgLogin.cpp:28-68`):

```cpp
int CNetClient::SndLogin(const char* szUserID, const char* szUserPassword,
                         const char* szRandomPassword, int nChannel)
{
    NET_LOGIN_DATA nld;                       // ctor sets dwSize=76, nType=2049
    nld.nChannel  = nChannel;
    ::StringCchCopy(nld.szUserid,         USR_ID_LENGTH+1,        szUserID);
    ::StringCchCopy(nld.szPassword,       USR_PASS_LENGTH+1,      szUserPassword);
    ::StringCchCopy(nld.szRandomPassword, USR_RAND_PASS_LENGTH+1, szRandomPassword);
    ::StringCchCopy(nld.szEnCrypt, ENCRYPT_KEY+1, m_szEncryptKey);

    m_Tea.encrypt (nld.szUserid,         USR_ID_LENGTH+1);
    m_Tea.encrypt (nld.szPassword,       USR_PASS_LENGTH+1);
    m_Tea.encrypt (nld.szRandomPassword, USR_RAND_PASS_LENGTH+1);
    m_Tea.encrypt (nld.szEnCrypt,        ENCRYPT_KEY+1);

    return Send((char *) &nld);
}
```

`nType` and `dwSize` are never reassigned — they come from the `NET_LOGIN_DATA` constructor
(`s_NetGlobal.h:2931-2942`). `CNetClient::Send` (`s_NetClient.cpp:956-964`) reads
`nmg->dwSize` from the buffer and forwards it unchanged; it does not rewrite the header.

---

## 3. What selects the variant

**Not** the country, **not** the episode, **not** a struct name. A single runtime `switch`
on `RANPARAM::emSERVICE_TYPE` (`legacy/Lib_ClientUI/Interface/LoginPage.cpp:239-260`):

```cpp
switch ( RANPARAM::emSERVICE_TYPE )
{
case EMSERVICE_THAILAND: pNetClient->ThaiSndLogin(...);  break;
case EMSERVICE_CHINA:    pNetClient->ChinaSndLogin(...); break;
case EMSERVICE_JAPAN:    pNetClient->JapanSndLogin(...); break;
case EMSERVICE_GLOBAL:   pNetClient->GsSndLogin(...);    break;
default:                 pNetClient->SndLogin(...);      break;   // <-- NET_MSG_LOGIN_2
};
```

Only four of the nine variants are reachable from the login page. `DAUM_`, `GSP_`, `TERRA_`
and `EXCITE_` are called from `OuterInterfaceModalMsg.cpp` instead — and `GSP_` (`:447`)
and `EXCITE_` (`:459`) are **commented out**, leaving `DAUM_` (`:443`) and `TERRA_` (`:454`)
reachable only through the external-ID modal, which requires web-account ID params.

`emSERVICE_TYPE` is assigned **only** in the client entry point
(`legacy/GameClient2/GameClient2.cpp:45-92`), from compile-time macros:

| Macro | `emSERVICE_TYPE` | LoginPage branch | nType sent |
| --- | --- | --- | ---: |
| `CH_PARAM` | `EMSERVICE_CHINA` | `case` | 2055 |
| `TH_PARAM` | `EMSERVICE_THAILAND` | `case` | 2082 |
| `MY_PARAM` | `EMSERVICE_MALAYSIA_CN` | default | 2049 |
| `MYE_PARAM` | `EMSERVICE_MALAYSIA_EN` | default | 2049 |
| `ID_PARAM` | `EMSERVICE_INDONESIA` | default | 2049 |
| `PH_PARAM` | `EMSERVICE_PHILIPPINES` | default | 2049 |
| `VN_PARAM` | `EMSERVICE_VIETNAM` | default | 2049 |
| `JP_PARAM` | `EMSERVICE_JAPAN` | `case` | 2074 |
| `TW_PARAM` / `HK_PARAM` | `EMSERVICE_FEYA` | default | 2049 |
| `KR_PARAM` / `KRT_PARAM` | `EMSERVICE_KOREA` | default | 2049 |
| `GS_PARAM` | `EMSERVICE_GLOBAL` | `case` | 2077 |

Enum: `RANPARAM.h:13-28` (12 values, `EMSERVICE_DEFAULT = 0` … `EMSERVICE_GLOBAL = 11`).

**Configuration cannot influence this.** `emSERVICE_TYPE` is set in the
`CGameClient2App` constructor, before `RANPARAM::LOAD()` runs in `InitInstance`, and
`LOAD`/`LOAD_PARAM` never assign it — it is only *read*, by `VALIDIDPARAM`
(`RANPARAM.cpp:225-238`). So the branch is fixed at build time and not overridable from
`param.ini`.

---

## 4. Why the answer does not depend on `KR_PARAM` vs `PH_PARAM`

The original `.vcproj` files define `KR_PARAM` for every server and client project, and
`PH_PARAM` appears in **no** project file. On that basis alone one might assert `KR_PARAM`.

This investigation deliberately does **not** rely on that, for two reasons:

1. V026 already established the shipped ASURA build is a **later revision** than `legacy/`
   (`LauncherConfig.json` names `ASURA SERVER EP9 (VS2022)`; the source here is VS2003
   `Version="7.10"`). A `.vcproj` in the repo is not proof of how the shipped binary was
   compiled.
2. It does not matter. **`EMSERVICE_KOREA` and `EMSERVICE_PHILIPPINES` are both absent from
   the `LoginPage` switch and both fall to `default`**, so both macros produce
   `SndLogin` → `NET_MSG_LOGIN_2`. The same holds for `MY`, `MYE`, `ID`, `VN`, `TW`, `HK`
   and `DEFAULT`. Six of the eleven macros — including both plausible ASURA candidates —
   converge on 2049.

The only macros that would produce a different nType are `CH_PARAM` (2055), `TH_PARAM`
(2082), `JP_PARAM` (2074) and `GS_PARAM` (2077). None is consistent with a Philippine
deployment, and none is what `LauncherConfig.json`'s `"Region": "Philippines"` would
suggest. Confidence: **high**.

---

## 5. `TCHAR` width — the V028 hazard does not materialise

V028 flagged that `DAUM_`/`TERRA_`/`GSP_` structs use `TCHAR` while `NET_LOGIN_DATA` uses
`CHAR`, and warned against assuming equal wire sizes. That warning was correct to raise,
and the answer is favourable — but only because of build configuration, and it should not
be assumed by default.

**All 29 original `.vcproj` files specify `CharacterSet="2"`** — no project deviates. In
the VS2003 project schema `CharacterSet="2"` is the Multi-Byte Character Set, which
defines `_MBCS` and makes **`TCHAR` == `char` == 1 byte**. No project defines `_UNICODE`
or `UNICODE`.

This was not taken on faith: a standalone x86 MSVC probe replicating the struct
definitions carries `static_assert(sizeof(TCHAR) == 1, ...)` and compiles clean
(§6). So `TCHAR` and `CHAR` fields are the same width **in this build**, and the
`CHINA_`/`DAUM_`/`GSP_`/`TERRA_` sizes in §1 are correct as single-byte layouts.

The hazard remains live for any future port that switches to Unicode: `CHINA_NET_LOGIN_DATA`
would then change from 76 to 140 bytes, `TERRA_NET_LOGIN_DATA` from 112 to 204, and so on.

---

## 6. Sizes are compiler-verified

The nine `sizeof` values were produced by compiling a standalone probe containing verbatim
copies of the nine struct definitions from `s_NetGlobal.h`, built with x86 MSVC
(`cl /EHsc /W4`, pointer size 4, MBCS). It was compiled outside the repository and is not
committed; it exists to replace hand arithmetic with a compiler, and it corrected two
mistakes:

- **`GSP_NET_LOGIN_DATA` is 52, not 64.** The struct field is `TCHAR szUUID[UUID_STR_LENGTH]`
  (37). `GSP_USERID` (51) is only the *copy length* used in `GspSndLogin`
  (`s_NetClientMsgLogin.cpp:205`) — which means `StringCchCopy(msg.szUUID, GSP_USERID, ...)`
  declares a 51-character destination for a 37-byte buffer. That is a **latent buffer
  overflow** in the GSP path. It is unreachable (the only caller is commented out), so it
  does not affect ASURA, but it is a real defect in the legacy tree.
- **`"O5FDASEOT"` is 9 characters, not 8**, so the largest garbage token is 9 bytes
  (§7).

---

## 7. The wire bytes the ASURA client sends

`CNetClient::SendNormal` (`s_NetClient.cpp:924-953`) inserts a **garbage token** between the
header and the body whenever the connection state is not `NET_STATE_LOGIN`:

```cpp
if( m_nClientNetState != NET_STATE_LOGIN )
{
    char *sendBuffer = SendMsgAddGarbageValue( buff, nSize );
    ::memcpy(m_pSndBuffer+m_nSndBytes, sendBuffer, nSize);
    m_nSndBytes += nSize;
}
```

`SendMsgAddGarbageValue` (`:893-922`) writes the 8-byte header with `dwSize += nGarbageLen`,
then the token, then the body. `GetGarbageMsg` (`:872-891`) picks from
`GARBAGE_DATA` (`RcvMsgBuffer.cpp:11-12`) and returns `strlen`:

| Token | Length |
| --- | ---: |
| `K9IHANA` | 7 |
| `L8IDUL` | 6 |
| `M7HSET` | 6 |
| `N6GNET` | 6 |
| `O5FDASEOT` | 9 |

The login is sent to the **Agent**, and `ConnectAgentServer` connects with
`NET_STATE_AGENT` (`s_NetClient.cpp:387`), **not** `NET_STATE_LOGIN`. So the token **is**
present on the login packet.

**Predicted first login packet:**

```text
offset 0  DWORD dwSize   = 82 | 83 | 85        (76 + 6|7|9)
offset 4  DWORD nType    = 2049  (0x0801)
offset 8  char   token[6|7|9]                  "L8IDUL" | "K9IHANA" | "O5FDASEOT"
offset .. DWORD nChannel = 0
           CHAR szRandomPassword[7]   minTea-encrypted
           CHAR szPassword[21]         minTea-encrypted
           CHAR szUserid[21]           minTea-encrypted
           CHAR szEnCrypt[13]          minTea-encrypted
```

This is directly testable against a capture, and is the single cheapest confirmation
available to whoever has a runnable client.

### This corrects V028 in three places

1. **Directional asymmetry.** V028 §C.3 presented one outbound/inbound order, implying
   compression applied symmetrically. It does not. The **server** batches and LZO-wraps
   (`CClientManager::SendClientFinal` → `CSendMsgBuffer::getSendSize`), but the **client**
   sends **raw, one message per `::send()`**, with no `NET_COMPRESS` envelope and no
   compression (`CNetClient::SendBuffer2`, `s_NetClient.cpp:815-831`, which passes
   `nmg->dwSize` straight to `::send`). The client *receives* through the same
   `CRcvMsgBuffer` and therefore does handle the envelope inbound
   (`s_NetClient.cpp:1110`). Compression is inbound-to-client and outbound-from-server
   only. A capture of the login packet needs **no LZO decompression at all** — which is
   why §18's layering concern does not apply to this direction.
2. **Garbage direction.** V028 §C.4 said `GARBAGE_DATA` handling is "client-side only",
   reasoning from `getOneMsg(bool bClient)`. It is the opposite. `bClient == true` means
   *"this slot is a game client"*, and `CNetUser::GetMsg` sets it for any slot that is not
   a server-to-server slot (`s_CNetUser.cpp:552-577`). So the **server** strips and
   validates the garbage on messages from a game client; the client program itself calls
   `getMsg(FALSE)` (`s_NetClient.cpp:1110`) and does not strip. The **client inserts**
   garbage and the **server removes** it.
3. **The `dwSize` check ordering.** `CAgentServer::MsgLogIn` requires
   `sizeof(NET_LOGIN_DATA) == pNml->nmg.dwSize` (`s_CAgentServerMsgLogin.cpp:618`). That
   holds because garbage removal happens in `CNetUser::GetMsg` *before* dispatch. A
   reimplementation that validates sizes before stripping garbage will reject every real
   login packet.

---

## 8. ASURA artifacts

Executable identity is settled from the project file, not assumed:
`legacy/GameClient2/GameClient2.vcproj` has
`OutputFile="$(SolutionDir)_Bin\$(ConfigurationName)\MiniA.exe"`, so **MiniA.exe is the
GameClient2 build**. Corroborated in source: `BUG_TRAP::BugTrapInstall(std::string(_T("MiniA")))`
(`GameClient2.cpp:43`) and the command-line token `"zxc" + "..." + "drun"`
(`GameClient2.cpp:131`), which matches `LauncherConfig.json`'s
`"Arguments": "zxc...drun"`. Startup chain is
`Ran Online Launcher.exe` → `MiniA.exe <args>`.

Byte-scan of the shipped `MiniA.exe`:

| String | Present | Meaning |
| --- | --- | --- |
| `K9IHANA` `L8IDUL` `M7HSET` `N6GNET` `O5FDASEOT` | **all 5** | the §7 obfuscation path is compiled in |
| `Steven Seagal Neck Break` | yes | minTea, as V028 found |
| `zxc` / `drun` | yes | launcher argument check |
| `LoginPage` | yes | the §3 selection site |
| `Wrong Message Size` | no | server-side string, correctly absent |

`param.ini`, `option.ini`, `config.ini` and all four `CFG\[n]Server*.cfg` are encrypted and
yield no plaintext. `CFG\IPFilter.cfg` is plaintext (`allow 30.0.0.12`).

**On `"Invalid web account"`:** that literal *is* in `MiniA.exe`, but it sits inside a
**runtime** `if` (`GameClient2.cpp:146-156`), not `#ifdef`, so it compiles into every
regional build. Its presence is **not** evidence that ASURA used `PH_PARAM`. This is the
§15 trap — string presence is evidence of presence, not of execution.

---

## 9. Packet capture — attempted, BLOCKED

A capture was the preferred next step and an attempt was made. It is blocked by two
independent environment facts, both verified:

1. **The client's target address cannot be redirected cheaply.** `RANPARAM::LOAD_PARAM`
   reads `[SERVER SET] LoginAddress` / `nLoginPort` from `param.ini`
   (`RANPARAM_MAIN.cpp:60-69`), and `param.ini` is **Rijndael/AES**-encrypted
   (`StringFile.cpp:84-99`, `Rijndael.cpp:931-956`). The eight version keys are hardcoded
   in `Rijndael.cpp:933-941`; the shipped `param.ini` carries version `8`
   (leading `DWORD`), selecting `sm_Version[7]` = `"lvdqkrmf$rpgo!@#$htjgj@#qksskrkr"` at
   32 bytes, `blockSize` 16, `ECB`, `ZEROES` padding. A naive whole-file AES-256-ECB
   decrypt of the 1008-byte body did **not** produce plaintext, because
   `CStringFile::GetNextLine` (`StringFile.cpp:165-181`) decrypts in fixed-size buffer
   chunks via `DecryptEx`, not as one stream. Resolving the chunk geometry was out of scope,
   and in any case **rewriting `param.ini` means modifying a shipped file** — prohibited —
   or copying a **5,660 MB** install.
2. **The client will not run here.** `InitInstance` calls `hs_start()` and
   `hs_start_service()` and returns `FALSE` — exiting — if either fails
   (`GameClient2.cpp:110-119`). That is nProtect Hackshield, and `Hackshield\` ships a full
   payload (`asc_main.dll`, `ehsvc.dll`, `brinicle.dll`, 2.9 MB `hshield.log`) needing its
   licensed service/driver environment. Per the project's Mini-A rule, MiniA is also not
   the gameplay-testing authority and must not be used to claim gameplay behaviour.

Per §24, this is recorded as a **blocker on the capture**, not on the answer. The source
chain is complete and self-consistent, and the binary contains the distinctive literals of
exactly that code path. Public backread corroborates the technique if someone wants to
pursue it: PyroStrex's released RAN EP6 tooling exists specifically to decrypt `param.ini`,
edit the address, and re-encrypt it for launcher redirection (RaGEZONE, *"PyroStrex's RAN
EP6 S2 Decryption & Encryption Algorithm"*) — feasible, simply outside this milestone.

---

## 10. Evidence table

| Question | Result | Evidence | Confidence |
| --- | --- | --- | --- |
| Client sender location | `Lib_Network/s_NetClientMsgLogin.cpp`, `CNetClient::SndLogin:28` | source | **HIGH** |
| Login nType | `NET_MSG_LOGIN_2` = **2049** | source: `LoginPage.cpp:239-260` → `s_NetClientMsgLogin.cpp:28-68` | **HIGH** |
| Struct | `NET_LOGIN_DATA` | source `s_NetGlobal.h:2916-2945` | **HIGH** |
| Wire size | **76** bytes body; **82/83/85** on the wire incl. garbage token | compiler probe; `SendMsgAddGarbageValue` | **HIGH** (source) / capture pending |
| TCHAR width | **1 byte** (`CharacterSet="2"` MBCS, all 29 vcproj) | `.vcproj` + `static_assert` probe | **HIGH** |
| Compression envelope | inbound-to-client only; **client sends raw** | `SendBuffer2` vs `SendClientFinal` | **HIGH** |
| minTea | key `"Steven Seagal Neck Break"`, per-field | V028 + `MiniA.exe` byte scan | **HIGH** |
| Region/provider selection | compile-time macro → `emSERVICE_TYPE`; not config-overridable | `GameClient2.cpp:45-92`; `RANPARAM_MAIN.cpp` | **HIGH** |
| Actual ASURA build macro | **UNRESOLVED** — but immaterial, KR and PH both yield 2049 | — | n/a |
| Empirical capture | **NOT OBTAINED** — AES config + Hackshield gate | §9 | — |

---

## 11. Remaining unknowns

1. **Which macro ASURA was actually compiled with.** Unprovable from the repo (`.vcproj`
   says `KR_PARAM`; the shipped build is a later EP9/VS2022 revision) and not provable from
   the binary, because the discriminating literals sit in runtime branches. **Immaterial to
   the answer** — see §4 — but it should be recorded rather than assumed.
2. **No packet capture.** The §7 prediction (82/83/85 bytes, `nType` 2049, garbage token
   bytes 8..8+n) is unverified against live traffic. Cheapest outstanding confirmation in
   the whole investigation.
3. **`RANPARAM::bFeatureRegisterUseMD5` for ASURA.** Carried over from V028 and still open.
   It changes the server's decrypted password width (`USR_PASS_LENGTH` vs
   `USR_PASS_LENGTH+1`, `s_CAgentServerMsgLogin.cpp:637-639`), and note the **client always
   encrypts 21 bytes** regardless (`s_NetClientMsgLogin.cpp:60`), so the two must agree for
   the password to decrypt correctly. Not resolvable without decoding the encrypted
   `param.ini`.
4. **Exact LZO chunk geometry** for the server→client direction, which V030 will need.
   Established so far: `lzo1x_1_compress` and `lzo1x_decompress_safe` via `CMinLzo`
   (`MinLzo.cpp:112`, `:158`), linked against `lzo2.lib` (`GameClient2.vcproj`
   `AdditionalDependencies`).
5. **Server→client compression still unverified empirically.** V028 proved it from source
   and it is consistent, but no capture has confirmed a real `NET_COMPRESS` frame.

---

## 12. Corrections to earlier verticals

- **V028**: "the login-send routine is not present in `legacy/Lib_Client/`" — **wrong**; it
  is `legacy/Lib_Network/s_NetClientMsgLogin.cpp`. A whole-repo search finds it
  immediately.
- **V028**: "`GARBAGE_DATA` anti-tamper is client-side only" — **reversed**; the server
  strips it from game-client messages, the client inserts it.
- **V028**: single outbound/inbound compression order — **client sends uncompressed and
  unwrapped**. Compression is directional.
- **V028** (correctly retracted in V029 §4): the `TCHAR`/`CHAR` hazard is real in principle
  but does not bite, because the build is MBCS.
- **V026/V027**: `KR_PARAM` does not select a login variant. Retained as retracted.

## Evidence index

| Claim | Source |
| --- | --- |
| Nine-variant server dispatch | `legacy/Lib_Network/s_CAgentServerMsg.cpp:61-71` |
| ID values | `legacy/Lib_Network/s_NetGlobal.h:770-804` |
| Nine login structs | `legacy/Lib_Network/s_NetGlobal.h:2916-3110` |
| Length constants | `legacy/Lib_Network/s_NetGlobal.h:170-237`; `minUuid.h:30` |
| Client senders | `legacy/Lib_Network/s_NetClientMsgLogin.cpp:28-346` |
| Variant selection switch | `legacy/Lib_ClientUI/Interface/LoginPage.cpp:239-260` |
| Unreachable/commented variants | `legacy/Lib_ClientUI/Interface/OuterInterfaceModalMsg.cpp:443-459` |
| Macro → service type | `legacy/GameClient2/GameClient2.cpp:45-92` |
| `EMSERVICE_TYPE` enum | `legacy/Lib_Engine/Utils/RANPARAM.h:13-28` |
| `emSERVICE_TYPE` not config-set | `legacy/Lib_Engine/Utils/RANPARAM.cpp:225-238`; `RANPARAM_MAIN.cpp:13-69` |
| `MiniA.exe` is GameClient2 | `legacy/GameClient2/GameClient2.vcproj` `OutputFile` |
| `CharacterSet="2"` MBCS | all 29 `legacy/**/*.vcproj` |
| Garbage insertion (client) | `legacy/Lib_Network/s_NetClient.cpp:872-953` |
| Garbage removal (server) | `legacy/Lib_Network/s_CNetUser.cpp:552-577`; `RcvMsgBuffer.cpp:213-269` |
| Client sends raw | `legacy/Lib_Network/s_NetClient.cpp:815-831` |
| Server batches + compresses | `legacy/Lib_Network/SendMsgBuffer.cpp:116-172`; `s_CClientManager.cpp:420-433` |
| Connect state for login | `legacy/Lib_Network/s_NetClient.cpp:375-387` |
| `dwSize` check after strip | `legacy/Lib_Network/s_CAgentServerMsgLogin.cpp:609-625` |
| Hackshield gate | `legacy/GameClient2/GameClient2.cpp:110-119` |
| `param.ini` AES | `legacy/Lib_Engine/Common/StringFile.cpp:84-99,165-181`; `Rijndael.cpp:931-956` |
| LZO variant | `legacy/Lib_Network/MinLzo.cpp:78,112,158` |
| `param.ini` redirection technique | RaGEZONE, "PyroStrex's RAN EP6 S2 Decryption & Encryption Algorithm" |