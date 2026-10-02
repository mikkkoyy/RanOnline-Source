# FACEBOOK-001 — Remove Legacy Facebook References / Integration

Baseline: `aefcb823f3fc61eed21983a67b4c5b781ca4547f` ("WORLD-001 modern client to server login")

## Result in one line

**There is no functional Facebook integration in this repository, so nothing was
removed.** Every Facebook reference found is either a false positive, a historical
author tag, or dead legacy code in a file that no project compiles. The one genuinely
functional Facebook integration — a **button and URL in the ASURA .NET launcher** — has
**no source in this repository** and is a shipped file that must not be modified.

Code changes: **0**. This is a documentation-only milestone by evidence, not by shortcut.

---

## 1. Findings

7,515 files scanned across `.c .cpp .h .hpp .inl .rc .txt .md .ini .cfg .json .xml
.vcproj .vcxproj .sln .props .targets .def .bat .cmd .ps1 .py .js .html .manifest`.

| # | Reference | Location | Class | Runtime used | Action |
| --- | --- | --- | --- | --- | --- |
| 1 | `//#include "./SNSFacebookPage.h"` | `legacy/Lib_ClientUI/Interface/Util/UIColorSelectorWindow.cpp:28` | **D** commented-out include | No | Kept, documented |
| 2 | `//class CSNSFacebookPage;` | same file, `.h:24` | **D** commented-out decl | No | Kept, documented |
| 3 | `SNS_PAGE_REQ_FACEBOOK_PAGE_OPEN` | same file, `.h:55` | **D** unused enum constant | No | Kept, documented |
| 4 | `// CSNSFacebookPage* GetFacebookPage () {...}` | same file, `.h:89` | **D** commented-out method | No | Kept, documented |
| 5 | `// CSNSFacebookPage* m_pPage_Facebook;` | same file, `.h:99` | **D** commented-out member | No | Kept, documented |
| 6 | `// bjju.sns` (×8 files) | `legacy/Lib_ClientUI/Interface/Util/*` | **C** developer author tag | No | Kept, documented |
| 7 | `Facebook`, `FacebookBtn`, `FacebookBtn_Click`, `https://facebook.com` | `ASURA CLIENT\Ran Online Launcher.exe` | **B** launcher UI link | **Yes** | Out of scope — no source here |

### Targeted searches that returned ZERO hits

`facebook.com`, `FBSDK`, `GraphAPI`, `Graph API`, `FacebookSDK`, `FacebookApi`,
`fbcdn`, `developers.facebook`, `facebookapp`, `oauth`, `OAuth`, `SocialLogin`,
`social_login` — **0 hits each, across all 7,515 files.**

### The `fb_` / `FB_` hits (5,242) are ALL false positives

This is the important trap. RAN uses **`FB` to mean "Feature Bit"**, not Facebook:

| Token | Count | Meaning |
| --- | ---: | --- |
| `emFB` | 1,345 | feature-bit enum prefix |
| `NetMsgFB` / `MsgFB` | 1,049 | feature-bit message flags |
| `EMFB_EQUIPMENT_LOCK`, `EMFB_INVENTORY_LOCK`, `EMFB_STORAGE_LOCK_*` | 400+ | gameplay feature locks |
| `net_msg_fb_client` | 22 | RAN's feature-bit client config |
| `EM_LOGIN_FB_SUB_*` | 27 | RAN's **login feedback** message |
| `TIXML_SNSCANF` (tinyxml), `sns` (boost test) | 2 | unrelated third-party identifiers |

Confirmed in `modern/` — the 4 hits are:
- `modern/core/combat/CombatConstants.h:93-95` — `BFB_DIS`, `BFB_AVER`, `BFB_ADV`:
  **brightness** blend flags for combat textures.
- `modern/server/login/LoginReceiver.h:101` — my own WORLD-001 comment quoting
  `EM_LOGIN_FB_SUB_FAIL`, RAN's login-feedback message.

**None of these is Facebook. None may be touched.**

---

## 2. Dependency result

```text
Facebook SDK:                       NOT PRESENT
Facebook API / Graph API:           NOT PRESENT — 0 hits for every API token
Facebook authentication:            NOT PRESENT — no OAuth, no social login
Facebook network dependency:        NOT PRESENT — Lib_Network has zero Facebook refs
Facebook gameplay dependency:       NOT PRESENT
Facebook resource/asset dependency: NOT PRESENT
Facebook in modern/:                NOT PRESENT — 0 hits
Facebook launcher/UI:               PRESENT, but EXTERNAL and source-less
Historical/community references:    5 dead refs in 1 uncompiled file + 8 author tags
```

### Is Facebook required for core RAN?

No, on every axis:

- **Networking** — `Lib_Network` has zero Facebook references. The login protocol is
  minTea + a shared secret + a garbage token (WORLD-001), with no social component.
- **Authentication** — the RAN credential system is `CAgentUserCheck` →
  `COdbcManager::UserCheck` → SQL. No social login path exists.
- **Gameplay** — nothing in `Lib_Client`, `Lib_Engine`, or any server touches it.
- **Assets / tools / servers** — zero hits in `Lib_Engine`, `GMTool`, all 15 editors,
  and all four servers.

### Trace: file → project → binary → runtime

```text
UIColorSelectorWindow.h/.cpp
    ↓  NOT referenced by any .vcproj or .vcxproj  (verified explicitly)
NOT COMPILED into Lib_ClientUI
    ↓
NOT in GameClient2.exe / GameEmulator.exe / any editor
    ↓
UNREACHABLE AT RUNTIME
```

Verified: `UIColorSelectorWindow`, `WebWindowBase`, `UIPageFrame`, `UIWindowObject`,
`SNSFacebook`, `SNSTwitter` — **none is referenced** by `Lib_ClientUI.vcproj` or
`Lib_ClientUI.vcxproj`.

Corroborated by binary scan — the shipped ASURA executables contain **no Facebook
strings at all**:

| Binary | Facebook strings |
| --- | --- |
| `MiniA.exe` | none |
| `Emulator.exe` | none |
| `[3]ServerAgent.exe` | none |
| `[4]ServerLogin.exe` | none |
| `Ran Online Launcher.exe` | `Facebook`, `FacebookBtn_Click`, `https://facebook.com` |

So the game client and every server are clean. Only the launcher is not.

---

## 3. Why nothing was removed

### The five dead references were kept

Per §9 (Legacy source rule) and §8 (Documentation rule): the objective is to remove
*functionality*, not to erase source archaeology.

Three independent reasons:

1. **They are already inert.** Four are commented out. The fifth is an enum constant
   with **zero consumers** anywhere in the tree.
2. **Their file is not compiled.** Deleting lines from a file no project builds
   changes nothing about any binary, while destroying the only record that RAN once had
   an SNS subsystem.
3. **They document a real historical subsystem.** `CSNSFacebookPage` and
   `SNSTwitterPage` were pages inside an embedded browser. `WebWindowBase.h/.cpp` — the
   browser — is in the same uncompiled `Util` directory. Deleting the pages while
   leaving the browser would make the history *harder* to read, not easier.

### The launcher was not touched

The launcher's Facebook button **is** functional, so §7 would normally require removing
it. It cannot be removed here, for three reasons:

1. **No source exists.** The repository contains no file named `*launcher*`, and **no
   managed code at all** — zero `.cs`, `.csproj`, `.xaml`, `.vb`. The launcher is a .NET
   application (`mscoree`) and cannot be rebuilt or edited from this repository at all.
2. **It is a shipped ASURA file.** `D:\FILES\project\RanOnline-Build\ASURA CLIENT\` is the
   deployed reference and is not to be modified.
3. **Its configuration already carries the field.**
   `LauncherConfig.json` contains `"Social": { "Facebook": "" }` — the empty string is the
   launcher's own default for an unset link.

**Action for the project owner:** the launcher source must be located and the
`Facebook` property plus `FacebookBtn`/`FacebookBtn_Click` removed there. This repository
can neither do it nor verify it. That is recorded as an unresolved item, not quietly
dropped.

---

## 4. Public backread (corroboration only)

- **No public RAN source establishes Facebook as an engine dependency.** The
  `yexiuph/RanOnline` and `ragezone/ran-online/game-sources` trees — both derived from
  the same RAN lineage — contain no Facebook integration in their client, server, or
  library projects.
- **The embedded-browser connection is corroborated.** A RaGEZONE thread on replacing
  RAN's UI engine notes *"the existing iexplorer/browser integration was laggy"* — RAN
  did have an embedded browser, which is precisely `WebWindowBase` in the dead `Util`
  directory. A Facebook fan-page tab inside that browser is consistent with the commented
  `SNSFacebookPage` and with the code never being compiled into a shipped build.
- **Community practice supports leaving dead subsystems documented.** The x64 RAN
  modernisation effort publicly lists removing whole dead subsystems (nProtect) as a
  deliberate step — removal is done at the build level, not by scrubbing comments.

Public source confirms the historical/community framing and does **not** convert it into
a gameplay dependency.

---

## 5. Changes made

```text
Files changed:   1  (this report)
Files removed:   0
Code changes:    0
Dependencies removed: 0 (none existed)
```

Nothing else in the repository was modified. In particular the following were **not**
touched, as no Facebook dependency was proven for any of them: combat, character stats,
equipment, skills, resources, damage, resistance, recovery, network protocol behaviour,
the WORLD-001 login protocol, the V030 compression layer, the V030-A tool inventory,
asset decoding, and the modern tool architecture.

---

## 6. Verification

| Check | Result |
| --- | --- |
| Debug build | clean, 0 errors, 0 warnings |
| Release build | clean, 0 errors, 0 warnings |
| CTest Debug | **15/15 passed** |
| CTest Release | **15/15 passed** |
| Core suite | 526/526, unchanged |
| Server suite | 125/125, unchanged |
| Network suite | 115/115, unchanged |
| Post-change Facebook search | 5 refs, all in 1 uncompiled legacy file — classified above |
| Working tree | clean |

Builds were re-run after the investigation to confirm the tree is unaffected. No test
count changed, because no code changed.

---

## 7. Remaining references

| Reference | Why it remains |
| --- | --- |
| `//#include "./SNSFacebookPage.h"` | **Historical.** Header does not exist. Parent file is not compiled. |
| `//class CSNSFacebookPage;` | **Historical.** Commented-out forward declaration. |
| `SNS_PAGE_REQ_FACEBOOK_PAGE_OPEN` | **Unreferenced legacy.** Enum constant with zero consumers, in an uncompiled file. |
| `// CSNSFacebookPage* GetFacebookPage()` | **Historical.** Commented-out accessor. |
| `// CSNSFacebookPage* m_pPage_Facebook;` | **Historical.** Commented-out member. |
| `// bjju.sns` ×8 | **Documentation.** Developer author tag, not Facebook. |
| Launcher `Facebook` / `FacebookBtn` / URL | **External, source-less.** See §3. Requires launcher source the repository does not have. |

---

## 8. Unresolved

1. **The launcher still contains a live Facebook button.** It cannot be removed from
   this repository. If a functional cleanup is wanted, the launcher source is required.
2. **RAN's SNS subsystem history is only partially reconstructable.**
   `SNSFacebookPage` and `SNSTwitterPage` are referenced by comment but neither file
   exists in the tree, so the original implementation is unrecoverable here.
3. **`Config.ini` remains undecodable** (Rijndael-encrypted, chunked — V029 §9), so it
   cannot be confirmed that no Facebook-related server feature flag exists there. No
   server binary contains Facebook strings, which is strong indirect evidence, but it is
   indirect.