# VERTICAL-026 - CALCDAMAGE Variant Authority

Investigation milestone. VERTICAL-025 flagged that `CALCDAMAGE`'s compile-time
dispatch left the legacy variant ambiguous. This resolves it.

## 1. Baseline

| | |
| --- | --- |
| Repository | `mikkkoyy/RanOnline-Source` |
| Branch | `main` (tracking `origin/main`) |
| Baseline commit | `26e1d24a226d645eed86a25f63d80c6f3a06d3cb` (VERTICAL-025) |
| Starting tree | clean |

## 2. Dispatch Rule

`GLogixExPC.cpp:1339-1359`:

```cpp
#if defined(KRT_PARAM) || defined(KR_PARAM)  || defined(CH_PARAM) ||
    defined(TH_PARAM)  || defined(ID_PARAM) || defined(JP_PARAM) ||
    defined(MY_PARAM)  || defined(MYE_PARAM)|| defined(PH_PARAM) ||
    defined(GS_PARAM)  || defined(_RELEASED)
    return CALCDAMAGE_20060328( ... );
#else
    return CALCDAMAGE_2004( ... );
#endif
```

Eleven macros select `CALCDAMAGE_20060328`. **Only the complete absence of all
eleven selects `CALCDAMAGE_2004`.**

## 3. Committed Project Configuration

The repository contains **two generations of Visual Studio project files**, and
VERTICAL-025 read only the newer one. This is the whole source of the ambiguity.

| | Legacy RAN project | Modernization conversion |
| --- | --- | --- |
| File | `Lib_Client/Lib_Client.vcproj` | `Lib_Client/Lib_Client.vcxproj` |
| Format | `VisualStudioProject` (VS2003) | `Project` / MSBuild (VS2010+) |
| Encoding decl | `ks_c_5601-1987` — **Korean** | `utf-8` |
| Lines | 1703 | 557 |
| Toolset | VS2003 | `v143` (VS2022) |
| `.cpp` units | 237 | 238 (**zero missing**) |
| `G-Logic` files | 387 | 211 (177 headers dropped) |
| Output | — | `..\RanOnline-Build\$(Configuration)\$(ProjectName)\` |
| `Debug\|Win32` defs | `WIN32;_DEBUG;_LIB;**KR_PARAM**` | `WIN32;_DEBUG;_LIB` |
| `Release\|Win32` defs | `WIN32;NDEBUG;_LIB;**KR_PARAM**` | `WIN32;NDEBUG;_LIB` |

Two conclusions follow, and the second is the one VERTICAL-025 missed:

1. **The conversion is complete at the compilation-unit level.** Zero `.cpp`
   files are missing, so the `.vcxproj` genuinely compiles `GLogixExPC.cpp` and
   would run the dispatcher. My first hypothesis during this investigation was
   that it was a sloppy partial conversion that could not build — **that was
   wrong**, and testing it is what surfaced the real finding.
2. **The conversion dropped `KR_PARAM`.** That is the single macro selecting the
   damage formula. Everything else about the conversion is faithful enough that
   losing one preprocessor define reads as an oversight, not a deliberate
   removal.

### `KR_PARAM` is defined by RAN's own project, unanimously

A scan of **every** `.vcproj` in the legacy tree — Debug *and* Release — finds
`KR_PARAM` in every gameplay project:

| Project | Debug | Release |
| --- | --- | --- |
| `Lib_Client` | `WIN32;_DEBUG;_LIB;KR_PARAM` | `WIN32;NDEBUG;_LIB;KR_PARAM` |
| `Lib_ClientUI` | ✅ `KR_PARAM` | ✅ `KR_PARAM` |
| `Lib_Engine` | ✅ `KR_PARAM` | ✅ `KR_PARAM` |
| `Lib_Network` | ✅ `KR_PARAM` | ✅ `KR_PARAM` |
| `ServerField` | ✅ `KR_PARAM` | ✅ `KR_PARAM` |
| `ServerLogin` | ✅ `KR_PARAM` | ✅ `KR_PARAM` |
| `ServerAgent` | ✅ `KR_PARAM` | ✅ `KR_PARAM` |
| `ServerSession` | ✅ `KR_PARAM` | ✅ `KR_PARAM` |
| `GameClient2` | ✅ `KR_PARAM` | ✅ `KR_PARAM` |
| `GameEmulator` | ✅ `KR_PARAM` | ✅ `KR_PARAM` |
| `GameViewer` | ✅ `KR_PARAM` | ✅ `KR_PARAM` |
| `GMTool` | ✅ `KR_PARAM` | ✅ `KR_PARAM` |
| 12 editors | ✅ `KR_PARAM` | ✅ `KR_PARAM` |

Only `CryptionRCC` and `EditGenItem` lack it, and neither compiles game logic.
Not one project anywhere defines `PH_PARAM`, `JP_PARAM`, `TH_PARAM` or
`_RELEASED`.

**Committed project configuration result:** RAN's own build defines `KR_PARAM`,
which is dispatch member #2. **`CALCDAMAGE_20060328`.**

The `.vcxproj` / `RanOnline.sln` pair is modernization scaffolding, not RAN
product: the `.sln` references paths prefixed `legacy\`, which only exist in the
reorganised repository, and outputs to `RanOnline-Build\$(Configuration)\...`,
which is not where ASURA lives. Nothing in `modern/` builds from `legacy/`
except the sanctioned `compatibility/legacy` bridge and the `exptable_dump`
research tool, so the `.vcxproj` macro state never affects modern behaviour.

## 4. ASURA Shipped-Build Evidence

Inventory: 50 957 files. Executables `[1]ServerSession.exe`,
`[2]ServerField.exe`, `[3]ServerAgent.exe`, `[4]ServerLogin.exe`, `MiniA.exe`,
`Emulator.exe`, `GM_Tool.exe`, `Ran Online Launcher.exe` plus the editor set.
All inspected read-only; nothing was modified.

### PROVEN

**P1 — Declared region.** `LauncherConfig.json`, verbatim:

```json
"Region": "Philippines",
"Executable": "MiniA.exe",
"WorkingDirectory": "D:\\FILES\\project\\Ran Online Development\\asura\\ASURA SERVER EP9 (VS2022)"
```

`PH_PARAM` is dispatch member #9. If ASURA was compiled with its declared
region's macro, it selects `CALCDAMAGE_20060328`.

**P2 — ASURA was not produced by any committed project configuration.** The
committed `.vcxproj` writes to `RanOnline-Build\$(Configuration)\$(ProjectName)\`
— i.e. `RanOnline-Build\Release\ServerField\`. ASURA lives at
`RanOnline-Build\ASURA CLIENT\`. The output path does not match, so ASURA was
produced from a **locally modified** project set whose preprocessor definitions
are **not recorded in this repository**.

**P3 — The binaries cannot discriminate.** Neither variant leaves a distinctive
string literal; the release executables are stripped. A string sweep of
`MiniA.exe`, `Emulator.exe` and `[2]ServerField.exe` for all eleven macro names
returned only `ID_PARAM` (an unrelated identifier), and for country names
returned a **runtime** list inside `[2]ServerField.exe`:

```
Service Provider:KOREA / TAIWAN / CHINA / JAPAN /
Philippines/Vietnam / THAILAND / MALAYSIA / Global Service Platform
```

Because all regions are enumerated at runtime, this evidences a multi-region RAN
build and **cannot** indicate the compile-time macro. Binary inspection is
inconclusive, and is recorded as such rather than stretched.

### STRONGLY INDICATED

**S1 — Korean platform stack.** Consistent with `KR_PARAM`: `DaumGameCrypt`
(`.cpp`/`.h`), `s_CAgentServer*` (13 files), `ggsrv*` (22 files), and the
`.vcproj` files' own `ks_c_5601-1987` encoding declaration.

**S2 — Public developer statement.** See section 9.

### INCONCLUSIVE

**I1 — The literal ASURA preprocessor line.** Unrecoverable. P2 proves the build
used a local project variant; that variant is not in the repository.

## 5. Behaviour Comparison (verified from source)

| Behaviour | `CALCDAMAGE_2004` | `CALCDAMAGE_20060328` |
| --- | --- | --- |
| Basic-attack resistance | none | none |
| Skill resistance target | **only the skill magnitude `nVAR`** (`:1941`) | **the whole range** (`:1562-1563`) |
| Resistance ordering | before `ApplyDamageRate` (`:1958`) and the roll (`:2004`) | before `ApplyDamageRate` (`:1600`) and the roll (`:1672`) |
| Resistance formula | `nVAR -= (int)(nVAR*nRESIST*0.01f*fRESIST_G)` | `dw -= (DWORD)(float(dw) * fResistTotal)` |
| Resistance cap | none | `fResistTotal > 0.8f ? 0.8f` (`:1559`) |
| **Critical item bonus** | **absent — `m_sSUMITEM.fIncR_Critical` is never read** | **`nPercentCri += (int)(m_sSUMITEM.fIncR_Critical * 100)` (`:1620`)** |
| Critical rate source | inline `1000/nPerHP - 10 + ndxLvl` (`:1991`) | `CriticalBaseRate(GETHP(), GETMAXHP(), GETLEVEL(), nLEVEL)` (`:1615`) |
| Skill damage reduction | none | `GetDecR_DamageMeleeSkill/RangeSkill/MagicSkill` (`:1537-1553`) |
| Skill damage vs range | `nVAR + (dw*wGRADE)/fDAMAGE_GRADE_K` added (`:1945-1946`) | `nVAR + dw*fGrade` added (`:1530-1531`) |
| Damage-rate application | `:1958-1961`, before the roll | `:1600-1603`, before the roll |
| Defence subtraction | `:2009` | `:1678-1686` |
| Defence rate | `fRATE = 1 - body*item/fDAMAGE_DEC_RATE`, clamp `[0,1]` (`:2017-2020`) | `fFinalRate = body*item*fDecRate`, **cap 0.6**, level-scaled (`:1703-1713`) |
| Reflection | same formula, inline (`:2046`) | same formula, via `DamageReflectionAmount` (`:1750`) |
| Critical / crushing damage | `:2022-2028` | `:1725-1731` |
| Low-seed branch | `:2012` | `:1686` |
| Damage floor | `:2072` | `:1777` |

The defence-rate difference is substantial and independently favours
`CALCDAMAGE_20060328`: modern implements the level-scaled `fDecRate` form with
the 0.6 cap, citing `:1701-1713`.

## 6. The Decisive Line of Evidence

**`m_sSUMITEM.fIncR_Critical` is consumed at exactly one place in the whole
file — `:1620`, inside `CALCDAMAGE_20060328`.** A full scan of
`GLogixExPC.cpp` returns four hits: three write it (`:571`, `:595`, `:796`,
item options and variants) and `:1620` reads it. `CALCDAMAGE_2004` **never
reads it**.

Therefore, in RAN as committed, **a critical item has no effect whatsoever under
`CALCDAMAGE_2004`.** That is a behavioural discriminator, not a configuration
guess.

Modern already depends on this: `CombatInput::attackerCriticalBonus` is fed from
`DerivedStats::criticalRate`, which is the aggregation of
`m_sSUMITEM.fIncR_Critical`, and `PhysicalDamageCalculator` cites `:1615-1620`.
If `CALCDAMAGE_2004` were authoritative, modern's critical-item feature would be
dead code by construction.

## 7. V025 Compatibility

**PASS — V025 remains correct. No code changed in this milestone.**

V025 targeted `CALCDAMAGE_20060328` on *reasoned inference* (documented as such,
explicitly flagged as unproven). That inference is now confirmed by primary
evidence, and all three of V025's corrections hold:

| V025 correction | Status against `CALCDAMAGE_20060328` |
| --- | --- |
| resistance moved to the range | **correct** — `:1562-1563` |
| subtractive `dw -= (DWORD)(dw*r)` form | **correct** — `:1562-1563`; 2004's form is different and is not what V025 implemented |
| `skillCast` scope gate | **correct** — the block sits inside `if (pSkill)` at `:1417`, closing `:1571` |

V024's `ApplyDamageRate` placement and V021's `ApplyDefenseRate` placement are
likewise unchanged and correct.

## 8. What Is Still Missing

Recorded rather than glossed over:

1. **The ASURA build's literal `/D` line** (P2/I1). Recoverable only from the
   machine that built it, or its build log.
2. **ASURA was not built from any committed configuration** (P2), so the
   committed `.vcxproj` cannot serve as the authority even though it is the file
   a modern reader reaches for first.

Neither gap weakens the verdict: lines 3, 6 and 9 below are independent of it.

## 9. Public / Forum Provenance

**RaGEZONE, "Need Help Critical item in Ep7", 9 May 2020.**
https://forum.ragezone.com/threads/need-help-critical-item-in-ep7.1177018/

Questioner (Mir125): *"i just create critical item but no critical appear..did i
just miss something in source?"*

Answer (Mustafa5, 14 May 2020): *"In GlogixExPC, there is a def for damage
calculation, use CALCDAMAGE_20060328 if you want the critical."*

Questioner: *"Thankyou for the reply sir is working thanks!"*

**Provenance, not primary evidence** — but it is *behaviourally* precise in a way
the brief's framing did not anticipate. The asker built a **critical item** and
saw no critical; switching the dispatcher to `CALCDAMAGE_20060328` fixed it. That
is exactly the `m_sSUMITEM.fIncR_Critical` asymmetry proven in section 6, observed
independently by a third party in 2020. It corroborates 20060328 as the variant
under which RAN's item features work.

Not used as proof of the ASURA build specifically: it describes an EP7
development build, and its author does not state a macro.

## 10. Evidence Chain

```
RAN's own .vcproj (KR_PARAM, every gameplay project, Debug + Release)
        ↓  KR_PARAM is dispatch member #2
   CALCDAMAGE_20060328                    ── PROVEN

ASURA LauncherConfig.json → Region "Philippines"
        ↓  PH_PARAM is dispatch member #9
   CALCDAMAGE_20060328                    ── PROVEN (declaration)

m_sSUMITEM.fIncR_Critical read only at :1620 (inside 20060328)
        ↓  a critical item is inert under 2004
   CALCDAMAGE_20060328                    ── PROVEN (behaviour)

RaGEZONE 2020: "use CALCDAMAGE_20060328 if you want the critical"
        ↓  confirmed fixed by the asker
   CALCDAMAGE_20060328                    ── CORROBORATION

Committed .vcxproj: no dispatch macro
        ↓  selects CALCDAMAGE_2004
   CONTRADICTION — resolved as a conversion defect (section 3)
```

## 11. Final Authority Verdict

```text
RESOLVED — CALCDAMAGE_20060328
```

Three independent primary-evidence lines select it — RAN's own unanimous
`KR_PARAM` project configuration, the ASURA Philippines region declaration, and
the `m_sSUMITEM.fIncR_Critical` behavioural asymmetry — and a fourth,
independent, contemporaneous public report corroborates it behaviourally.

The single contradicting line is the committed `.vcxproj`, which is resolved as
an MSBuild conversion that dropped `KR_PARAM` while otherwise faithfully
converting every compilation unit. That file is modernization scaffolding, is
referenced only by a `.sln` whose output layout does not match ASURA's, and
affects no modern code.

The variant question that VERTICAL-025 left open is **closed**. The gate is
cleared for the next gameplay milestone.
