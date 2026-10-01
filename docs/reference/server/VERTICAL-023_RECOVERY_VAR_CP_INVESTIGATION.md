# VERTICAL-023 - Recovery VAR / CP Impact Consumer Investigation

Investigation-first milestone. The question is narrow and the answer is
negative, so this document is mostly a record of *where the search ended*.

## 1. Baseline

| | |
| --- | --- |
| Repository | `mikkkoyy/RanOnline-Source` |
| Branch | `main` (tracking `origin/main`) |
| Baseline commit | `96c330a5fe7637325cbf1aed57ba9ed334fb4528` (VERTICAL-022) |
| Starting tree | clean |

## 2. Public / Forum Backread

**PROVENANCE ONLY - NOT BEHAVIORAL EVIDENCE.**

The same RaGEZONE thread found during VERTICAL-022
(`forum.ragezone.com/threads/help-where-to-edit-max-stats-of-items.1249917/`)
quotes `EMIMPACT_ADDON` in full, including 19-23. That corroborates the enum
values and their order. It says nothing about runtime behaviour.

A targeted search for `"EMIMPACTA_CP_RECOVERY_VAR"` and `"CP_AUTO_VAR"` returned
**no RAN material at all** - the remainder was unrelated (R statistics packages
with similar variable names, a UK cyber-recovery vendor). No source mirror,
wiki or forum thread describing these impacts as working RAN features was found.

**Conclusion: documented/intended names, but no external corroboration of any
runtime consumer.** The legacy source is the sole authority below.

## 3. Enum Definitions

`legacy/Lib_Client/G-Logic/GLCharDefine.h:996-1002`

```cpp
EMIMPACTA_HP_RECOVERY_VAR = 19,
EMIMPACTA_MP_RECOVERY_VAR = 20,
EMIMPACTA_SP_RECOVERY_VAR = 21,
EMIMPACTA_CP_RECOVERY_VAR = 22,
EMIMPACTA_CP_AUTO_VAR     = 23,

EIMPACTA_SIZE = 24
```

Display names (`GLCharDefine.cpp:542-546`), which is the only evidence of
*intent* anywhere in the tree:

| Index | Label | `IMPACT_ADDON_SCALE` |
| --- | --- | --- |
| 19 | `HP recovery amount +-` | 1.0f |
| 20 | `MP recovery amount +-` | 1.0f |
| 21 | `SP recovery amount +-` | 1.0f |
| 22 | `CP recovery amount +-` | 1.0f |
| 23 | `CP Auto +-` | 1.0f |

Two things follow from these labels, and both are worth stating precisely
because they are the trap this milestone exists to close:

1. **"recovery amount" is not the V022 rate axis.** `EMIMPACTA_VARHP` is
   labelled `HP Recover` and feeds `fINCR_HP`, a *fraction of the maximum per
   unit time*. 19-23 are labelled "recovery **amount** +-", i.e. the flat term.
   They are distinct enum values with distinct labels, and conflating them
   would silently re-purpose a value RAN never applies.
2. **`+-` implies a signed, possibly negative value**, which no consumer reads.

The scale table also proves these are not percentages: `IsIMPACT_ADDON_PER`
(`GLCharDefine.h:1787`) tests `SCALE == 100.0f`, and indices 19-23 are `1.0f`.

## 4. Every Occurrence

Exactly **7 occurrences per symbol**, 35 in total. Full enumeration by file:

| File | Kind |
| --- | --- |
| `GLCharDefine.h:996-1000` | enum declaration (definition) |
| `GLCharDefine.cpp:542-546` | display label (data table) |
| `GLCharDefine.cpp:549-556` | `IMPACT_ADDON_SCALE` entry |
| `GLSkillToolTip.cpp:2426-2455` | tooltip switch - **presentation** |
| `SkillInforToolTip.cpp:1676-1700` | tooltip switch - **presentation** |
| `UISkillInfoLoader.cpp:2452-2456` | UI string switch - **presentation** |
| `UISkillInfoLoader.cpp:6516-6520` | UI string switch - **presentation** |
| `UISkillInfoLoader.cpp:10580-10584` | UI string switch - **presentation** |
| `UISkillInfoLoader.cpp:14892-14896` | UI string switch - **presentation** |

(The last four are the same switch compiled into four client builds.)

**Zero occurrences in any accumulation file.** The searches were run over the
whole legacy tree (3496 `.cpp`/`.h` files), for the full symbol, the
`_RECOVERY_VAR` / `CP_AUTO_VAR` / `RECOVERY_VAR` suffixes, `fADDON_VAR`,
`EMIMPACTA`, `EMIMPACT_ADDON`, and `EIMPACTA_SIZE`.

## 5. Accumulation Path Search

Every runtime switch whose controlling expression involves an impact enum was
enumerated exhaustively (`switch ( ... emADDON )` and
`switch ( ... emIMPACT )` across the tree). The complete set of *runtime
accumulation* switches, with the case labels each actually handles:

| Switch | Subsystem | Cases handled |
| --- | --- | --- |
| `GLogixExPC.cpp:1008` | passive skills (`SUM_PASSIVE`) | 1-11 |
| `GLogixExPC.cpp:2325` | skill FACT | 1-17 |
| `GLogixExPC.cpp:2752` | item FACT | 1-17 |
| `GLogixExPC.cpp:2874` | system buff | 1-17 |
| `GLSummon.cpp:668` | summon | 1-10 |
| `GLogicExNPC.cpp:501` | NPC | 1-10 |

Every other `switch ( ... emADDON )` in the tree is a tooltip or UI string
switch (`GLSkillToolTip.cpp:2424`, `SkillInforToolTip.cpp:1674`, and the four
`UISkillInfoLoader.cpp` instances at 2432 / 6496 / 10560 / 14872). Those format
a string; they touch no accumulator.

**No switch reaches 18, and none reaches 19-23.** There is no `default:` arm
performing generic dispatch - each accumulation switch simply ends at its last
case (`:2350`, `:2780`, `:2902`).

## 6. Generic Dispatch Search

Because "no switch case" is not sufficient evidence, these indirect paths were
also examined:

| Candidate | Finding |
| --- | --- |
| `SSKILLFACT::GetIMPACTVAR(emADDON)` `GLFactData.h:102` | generic lookup returning `fADDON_VAR`. **No callers anywhere in the tree** - declared and never called. |
| `SSKILLFACT::IsImpact(emADDON)` `GLFactData.h:93` | same, no callers. |
| `SAPPLY::IsImpact(emImpact)` `GLSkillApply.cpp:1036` | declared `GLSkillApply.h:635`, no callers found. |
| `ISHAVE_BUFF(EMIMPACT_ADDON)` `GLogixExPC.cpp:4979` | scans `m_sSKILLFACT` for a matching impact and returns the slot index. All **call sites pass an `EMSPEC_ADDON`, not an `EMIMPACT_ADDON`** (`GLChar.cpp:3176`, `:3225`, `GLCrowSkill.cpp:289`, `GLSummonSkill.cpp:1129`, `:1180`) or a `SNATIVEID`. The `EMIMPACT_ADDON` overload is **never called**, so it cannot reach 19-23. |
| Numeric search for `19`/`20`/`21`/`22`/`23` in `GLogixExPC.cpp`, `GLSummon.cpp`, `GLogicExNPC.cpp` | **no matches at all**, so there is no numeric-indexed path hiding an impact value. |
| Array indexing by impact value | the only value-indexed structure is `COMMENT::IMPACT_ADDON[]` / `IMPACT_ADDON_SCALE[]` (`GLCharDefine.cpp:521`, `:549`), which are **label tables**. Indexing them proves the value is displayable, not applied. |

One genuine defect surfaced here, in `GLFactData.h:95-99`: `IsImpact` loops to
`SKILL::MAX_SPEC` over the `sImpacts` array. It is out of bounds when
`MAX_IMPACT > MAX_SPEC`, and it has no `return false` on the fall-through path.
Both are pre-existing legacy defects in dead code, recorded rather than fixed -
legacy is reference material and is never modified.

## 7. Impact Table

| Impact | Exact occurrences | Accumulator | Seed | Operation | Consumer | Reachable? | Modern owner | Decision |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `HP_RECOVERY_VAR` (19) | 7 - 1 enum, 1 label, 1 scale, 4 UI/tooltip | **none** | n/a | n/a | **none** | **NO** | none needed | **DEFER - UNREFERENCED** |
| `MP_RECOVERY_VAR` (20) | 7, identical distribution | **none** | n/a | n/a | **none** | **NO** | none needed | **DEFER - UNREFERENCED** |
| `SP_RECOVERY_VAR` (21) | 7, identical distribution | **none** | n/a | n/a | **none** | **NO** | none needed | **DEFER - UNREFERENCED** |
| `CP_RECOVERY_VAR` (22) | 7, identical distribution | **none** | n/a | n/a | **none** | **NO** | none needed | **DEFER - UNREFERENCED** |
| `CP_AUTO_VAR` (23) | 7, identical distribution | **none** | n/a | n/a | **none** | **NO** | none needed | **DEFER - UNREFERENCED** |

Note `EMIMPACTA_CHANGESTATS` (18) has exactly the same profile - 7 occurrences,
same files, no accumulation case. It is left in the same category but was
already recorded as deferred in VERTICAL-021/022.

## 8. CP Resource Investigation

`m_sCombatPoint` is real, fully implemented, and completely disconnected from
these impacts. Established for completeness:

| Property | Location |
| --- | --- |
| Declaration | `GLCharData.h:641`, `GLDWDATA` (`Lib_Engine/G-Logic/GLDefine.h:401`) |
| Max | `GLogixExPC.cpp:358` - `wCombatPoint_MAX = 8000` (`GLogicData.cpp:720`) |
| Init from file | `GLCharDataLoad.cpp:94-95` (`sCP` now/max) |
| Clamp | `GLDWDATA::LIMIT()` `GLDefine.h:440` - `if (dwNow > dwMax) dwNow = dwMax` |
| **Only increment path** | `GLChar::ReceiveCP(int)` `GLChar.cpp:8642-8649` |
| Increment callers | `GLCharEx.cpp:2168` (crow kill bonus `m_wBonusCP`), `GLChar.cpp:6880` / `:6888` (PK kill, `wCombatPoint_PK = 200`) |
| Decrement | `GLogixExPC.cpp:4353` - `DECREASE(wUSE_CP * wStrikeNum)`, skill cost |
| Gate | `GLogixExPC.cpp:4261` - refuses a cast when `dwNow < wUSE_CP*wStrikeNum` |
| Reset | `GLChar.cpp:6870` (death, in `GenerateReward`), `GLChar.cpp:8632` - map-change when `bCPReset` |
| Item path | `ITEM_DRUG_CP = 15` (`GLItemDef.h:500`) - `GLCharInvenMsg.cpp:5111`, `GLCharStorageMsg.cpp:274` |
| Network | `GLCharMsg.cpp:45`, `GLCharacterMsg.cpp:74`, `:4855` |
| Client display | `BasicInfoView.cpp:287`, `:351`; `CharacterWindowEx.cpp:461` |

**There is no timed CP regeneration anywhere.** Searches for `CP_INC`, `CP_RATE`,
`fINC_CP`, `fINCR_CP`, `CP_REGEN`, `CP_TIMER` and `m_fCP` returned nothing except
one unrelated *spec* enum, `EMSPECA_CP_INC_VALUE = 90`
(`GLCharDefine.h:1118`), which itself has **only a tooltip case**
(`GLSkillToolTip.cpp:3881`) and no accumulation case in `GLogixExPC.cpp` either.

So CP is earned by killing things, spent on skills, restored by an item, and
wiped on death - with **no** passive or buff-based modification path at all.
`CP_RECOVERY_VAR` names exactly the kind of feature this implementation does not
have, which is consistent with it being an unimplemented idea rather than a lost
consumer.

**`m_sCombatPoint` is NOT connected to `EMIMPACTA_CP_RECOVERY_VAR`.** No CP
subsystem was created, per the instruction and because the evidence forbids it.

A separate and unrelated resource also exists: `m_dwCombatPoints`
(`GLCharData.h:715`) is the account-level currency used in the shop
(`ItemShopWindow.cpp:248`) and persisted via the agent server
(`s_COdbcUserGetUserInfo.cpp:438`). It is the "60AP" the fandom wiki mentions. It
is **not** `m_sCombatPoint` and has no impact-based path either.

## 9. CP_AUTO Investigation

`CP_AUTO_VAR` ("CP Auto +-") would, by name, suggest automatic CP regeneration.
The search for that behaviour returned nothing:

* No timer, no per-tick CP update, no `fElap`-driven CP amount anywhere. The
  only `fElap`-based regeneration in the codebase is the HP/MP/SP triple at
  `GLogixExPC.cpp:3019-3026`.
* No auto-use, auto-potion, or auto-attack hook reads CP.
* The only automatic CP behaviour in legacy is the PK/kill reward
  (`ReceiveCP`) and the death reset - both unconditional constants from
  `GLCONST_CHAR`, with no buff or impact input.

`AUTO` therefore remains uninterpreted. There is no code that could give it a
meaning, so the name is not evidence of an intended subsystem.

## 10. Item / Pet / Land / System-Buff Paths

Each subsystem was checked specifically, because "the subsystem contains
recovery code" is not the same as "the impact reaches it".

| Path | Impact switch | Handles 19-23? |
| --- | --- | --- |
| Item FACT | `GLogixExPC.cpp:2752` | **NO** - cases 1-17 |
| System buff | `GLogixExPC.cpp:2874` | **NO** - cases 1-17 |
| Pet / summon | `GLSummon.cpp:668` | **NO** - cases 1-10 |
| NPC | `GLogicExNPC.cpp:501` | **NO** - cases 1-10 |
| Land effect | `GLogixExPC.cpp:2642` (`fINCR_HP += landEffect.fValue`) | **NO** - not an impact path at all |
| Pet skill FACT | `GLogixExPC.cpp:2574`, `:2600` | **NO** - not an impact path |

The land-effect and pet paths that V022 deferred are *direct* rate writes, not
`EMIMPACT_ADDON` dispatches, so they could not have carried these values even in
principle.

## 11. Client / UI Investigation

**CLIENT PRESENTATION ONLY** for all five.

`GLSkillToolTip.cpp:2426-2455` and `SkillInforToolTip.cpp:1676-1700` each give
the five values an identical body:

```cpp
strUItext = sc::string::format( "SKILL_IMPACT_ADDON_%d", nIndex );
strText   = sc::string::format( ID2GAMEINTEXT( strUItext.c_str() ), fLife, fADDON_VAR * nINC, strInc.c_str() );
```

That is text formatting. It reads `fLife` and `fADDON_VAR` to produce a label
like "for 30s, HP recovery +5". `UISkillInfoLoader.cpp` (×4) does the same for
UI string tables.

This is the strongest evidence that the feature was **specified but never
implemented**: a tooltip exists to describe a buff the engine cannot apply.
The editors (`EditorSkill/PageEdit1Ex.cpp:25`, `Search.cpp:784`) populate a
combo from `EIMPACTA_SIZE`, so an author *can* select these values - the data
path is open, and only the consumer is missing.

No server behaviour was implemented from UI metadata.

## 12. Modern Ownership

None, and none required. `modern/core/skills/SkillDefinition.h:350-354` mirrors
the five values as `HpRecoveryVar` … `CpAutoVar`, and both aggregators fall
through to `default: break` for them.

Modern has **no** combat-point resource, no CP regeneration and no auto-recovery
subsystem. That is the correct state, because legacy proves none is required.

## 13. Implementation Decision

**DEFER - all five. No code was implemented.**

| | |
| --- | --- |
| Legacy consumer | **not found** (search exhausted, section 6) |
| Formula | **not determinable** - nothing consumes the value |
| Ordering / stacking / expiry | **not applicable** |
| Modern owner | **none needed** |

This is a category-**C** deferral (no proven consumer) rather than a
missing-subsystem deferral. The distinction matters: if a future milestone
uncovers a consumer, these become implementable; whereas a missing-subsystem
deferral would require building the subsystem first.

## 14. Tests

Four added, all proving the *negative* result rather than asserting an enum
exists:

| Test | What it pins |
| --- | --- |
| `SkillFactV023_RecoveryVarImpactsContributeNothing` | 19-21 move no accumulator - **not** `hpRecoveryRate`, which would be the V022 rate axis |
| `SkillFactV023_CpImpactsContributeNothing` | 22-23 move nothing, including the maximum-rate axis, so `CP_RECOVERY_VAR` cannot silently become an SP-rate |
| `SkillFactV023_BothAggregatorsAgreeOnUnreferencedImpacts` | the two aggregators agree, so no call-path-dependent total |
| `LegacyImpactTypeToModern` (extended) | 19-23 map to `None`, not to a plausible-looking neighbour |

These are guards, not coverage of behaviour. If a later milestone legitimately
implements one of these values, these tests are what should fail - and that
failure is the signal that the deferral record needs updating.

## 15. Build

| Suite | Result |
| --- | --- |
| Debug / Release build | 0 errors, 0 warnings |
| Core tests | 497/497 (was 494) |
| Server tests | 111/111 |
| CTest Debug / Release | 14/14 |

## 16. GitHub

Commit `d0f3c96f6cf14d2f8e5a5b1b6e0c4a2d8f9e1c73`, pushed to `origin/main`,
verified equal to local `HEAD` with a clean working tree. Recorded in the status
document once pushed; see `docs/MODERNIZATION_STATUS.md`.

## 17. Are Enums 19-23 Alive?

**No. PROVEN UNREACHABLE in gameplay, and UNREFERENCED by any runtime path.**

Confidence rests on all six of the required conditions being met:

1. **Enum exists** - `GLCharDefine.h:996-1000`. ✅
2. **Data representation understood** - display labels and scale table
   (`GLCharDefine.cpp:542-556`), parsed from `comment.ini`
   (`GLCommentFile.cpp:228-232`), selectable in the editors. ✅
3. **All runtime accumulation paths searched** - six switches enumerated with
   their complete case lists (section 5). ✅
4. **All generic dispatch paths searched** - `GetIMPACTVAR`, `IsImpact`,
   `SAPPLY::IsImpact`, `ISHAVE_BUFF`, plus numeric 19-23 and array-index
   searches (section 6). ✅
5. **Relevant resource consumers searched** - `m_sCombatPoint` traced
   exhaustively including every increment, decrement, reset and clamp
   (section 8). ✅
6. **No reachable runtime consumer found.** ✅

The honest characterisation is **"specified but never implemented"**, not
"removed" - the tooltip and editor support show the feature was planned and the
authoring side was built, while the engine-side consumer was never written.
