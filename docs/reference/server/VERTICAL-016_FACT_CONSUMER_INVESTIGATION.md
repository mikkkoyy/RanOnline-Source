# VERTICAL-016 — FACT Consumer / Impact Integration Investigation

**Investigation milestone. No gameplay code was changed.** This document is the
deliverable: it establishes exactly where RAN consumes FACT data after creation,
and which of those consumers can safely attach to systems that already exist in
`modern/`.

---

## 1. Baseline

| | |
| --- | --- |
| Repository | `mikkkoyy/RanOnline-Source` |
| Branch | `main` (tracking `origin/main`) |
| Commit | `d05cc2d2de770fcbc50cf5e7829fc9e41325a5b5` |
| Working tree | clean |

No production source was modified. Per §24 the preferred outcome is no code
change, and that is what the investigation produced.

## 2. Public / Forum Backread

**PUBLIC/FORUM BACKREAD: PARTIAL**

Unlike the previous three milestones, this search surfaced **real RAN public
source repositories**. Recorded with what each does and does not prove:

| Source | What it proves | What it does NOT prove |
| --- | --- | --- |
| `https://gitlab.com/ragezone/ran-online/game-sources` | The canonical RaGEZONE RAN "Game Sources" repository exists and is public | No FACT internals; directory listing is JS-rendered and could not be read |
| `https://gitlab.com/kenngxx0/ran-online-episode-7-source-code` | A fork of the above, EP7, Apache-2.0 | Nothing about aggregation or consumers |
| `https://github.com/yexiuph/RanOnline`, `https://github.com/ezrajohnnunez/ran-online-source-code`, `https://github.com/tablangdelio/ranOnline`, `https://github.com/matheomedrana/Ran-Online-EP7-Complete-Files` | Several independent RAN source mirrors exist | Same — no FACT content was retrievable |
| `https://ragezone.com/2011/07/10/ran-online-episode-7-server-files-and-source-code-released` | EP7 server source was publicly released in 2011, which explains the mirrors | Nothing behavioural |
| `[Tool]__SkillEditor` tree (GitLab) | The skill editor is the tool that **authors** impact/spec data | Attempted to backread; page is JS-rendered and returned no content |

**Assessment.** The repositories are the same EP7 lineage as the checked-in
`legacy/` tree, so they are corroboration of *provenance* rather than an
independent second source. No snippet retrieved contained FACT aggregation or
consumer code, and one targeted page could not be rendered. Consequently **no
behaviour below rests on public material**; all of it is read directly from the
checked-in tree, which remains authoritative.

## 3. Legacy FACT Creation Path

Re-traced from source rather than relying on VERTICAL-015's comments.

`legacy/Lib_Client/G-Logic/GLChar.cpp`
`GLChar::RECEIVE_SKILLFACT` — `:6410-6613`

| Step | Lines | Operation |
| --- | --- | --- |
| 1. slot sentinel | `:6412` | `dwSELECT = SKILLFACT_SIZE` |
| 2. skill lookup | `:6415-6416` | `GLSkillMan::GetData(...)`; `if (!pSkill) return FALSE` |
| 3. level bound | `:6417` | `if (wlevel >= SKILL::MAX_LEVEL) return FALSE` |
| 4. self-buff class gate | `:6420-6507` | class/mainID matrix; `return FALSE` for disallowed self buffs |
| 5. capture-the-flag gate | `:6512` | `if (m_bCaptureTheFlagHoldFlag && emACTION == EMACTION_BUFF) return FALSE` |
| 6. caster identity | `:6516-6517` | `_wCasterCrow`, `_dwCasterID` |
| 7. basic `EMFOR_*` | `:6519-6539` | **14 whitelisted types only** → `emTYPE`, `fMVAR = fBASIC_VAR` |
| 8. impacts | `:6541-6549` | every non-`NONE` impact; `fADDON_VAR = ...fADDON_VAR[wlevel]` |
| 9. specs | `:6551-6601` | **explicit case list**, copying `fVAR1`, `fVAR2`, `dwFLAG`, `dwNativeID` |
| 10. lifetime | `:6609` | `fAGE = sSKILL_DATA.fLIFE` |
| 11. slot selection | `:6611` | `SELECT_SKILLSLOT(skill_id)` |
| 12. storage | `:6612` | `m_sSKILLFACT[dwSELECT] = sSKILLEF` |
| 13. `bHOLD` gate | `:6605` | everything above only runs `if (bHOLD)` |

Two structural facts matter downstream: impacts are copied **wholesale**, but
specs are copied **only from an explicit case list** (`:6553-6587`), and
`bHOLD` means a skill with no whitelisted basic type, no impact and no listed
spec produces no record at all.

Slot selection, `GLChar.cpp:6377-6408`, is unchanged from VERTICAL-015 and was
re-read to confirm: refresh-in-place → first empty slot → smallest remaining
lifetime.

## 4. EMIMPACTA Consumer Matrix

All rows are the skill-FACT aggregation switch in
`legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2323-2350`, inside the single
per-tick pass. "Base" is the value assigned immediately before the loop
(`:2196-2199`, `:120-121`).

| Impact | Stored | Legacy consumer | Exact operation | Kind | Existing modern consumer | Implementable now? |
| --- | --- | --- | --- | --- | --- | --- |
| `HITRATE` | yes `:2327` | `m_nSUM_HIT` | `+= int(fADDON_VAR)`, base `m_nHIT` `:2198` | flat, additive | `DerivedStats::hit` → `HitInput::attackerHit` | **YES** (§7) |
| `AVOIDRATE` | yes `:2328` | `m_nSUM_AVOID` | `+= int(fADDON_VAR)`, base `m_nAVOID` `:2199` | flat, additive | `DerivedStats::avoid` → `HitInput::targetAvoid` | **YES** (§7) |
| `DAMAGE` | yes `:2329` | `m_gdDAMAGE_SKILL` | `VAR_PARAM(int(fADDON_VAR))` | saturating add on **range**, floor 1 | `CombatInput::attackerPhysicalDamage` → `MagicDamageInput::skillRange` | **PARTIAL** — range owner differs per channel (§8) |
| `DEFENSE` | yes `:2330` | `m_nDEFENSE_SKILL` | `+= int(fADDON_VAR)` | flat, additive | `CombatInput::targetDefense` | **PARTIAL** — needs body/item split decision |
| `VARHP` | yes `:2331` | local `fINCR_HP` | `+= fADDON_VAR` | flat, additive | `ResourceState` current pool | **PARTIAL** — recovery loop missing (§11) |
| `VARMP` | yes `:2332` | local `fINCR_MP` | `+= fADDON_VAR` | flat, additive | `ResourceState` current pool | **PARTIAL** — same |
| `VARSP` | yes `:2333` | local `fINCR_SP` | `+= fADDON_VAR` | flat, additive | `ResourceState` current pool | **PARTIAL** — same |
| `VARAP` | yes `:2334-2338` | `fINCR_HP/MP/SP` | `+= fADDON_VAR` on all three | flat, additive | `ResourceState` | **PARTIAL** — same |
| `DAMAGE_RATE` | yes `:2340` | `m_fDamageRate` | `+= fADDON_VAR`; applied to the range pre-roll at `:1600-1603` | additive rate | `CombatInput::attackerDamageRate` (V013) but **`PhysicalDamageInput` has no `damageRate`** | **NO — gap**, see §8 |
| `DEFENSE_RATE` | yes `:2341` | `m_fDefenseRate` | `+= fADDON_VAR` | additive rate | none | **NO — no modern consumer** |
| `PA` | yes `:2343` | `nSUM_PA` | `+= int(fADDON_VAR)` | flat, additive int | `DerivedStats::meleePower` | **YES** (§7) |
| `SA` | yes `:2344` | `nSUM_SA` | `+= int(fADDON_VAR)` | flat, additive int | `DerivedStats::shootPower` | **YES** (§7) |
| `MA` | yes `:2345` | `nSUM_MA` | `+= int(fADDON_VAR)` | flat, additive int | `DerivedStats::magicAttack` | **YES** (§7) |
| `HP_RATE` | yes `:2346` | `m_fHP_RATE` | `+= fADDON_VAR`; consumed at `:2165` `dwMax * (1 + passiveRate + m_fHP_RATE)` | additive rate, **multiplicative** on max | `DerivedStats` HP maximum | **YES** (§7, §11) |
| `MP_RATE` | yes `:2347` | `m_fMP_RATE` | `+= fADDON_VAR` | additive rate | `DerivedStats` MP maximum | **YES** (§7) |
| `SP_RATE` | yes `:2348` | `m_fSP_RATE` | `+= fADDON_VAR` | additive rate | `DerivedStats` SP maximum | **YES** (§7) |
| `RESIST` | yes `:2349` | `m_sSUMRESIST_SKILL` | `+= int(fADDON_VAR)` | flat, additive int | `DerivedStats::resistances` | **PARTIAL** — axis granularity differs (§8) |

### Impacts present in the enum but with no consumer in this path

`EMIMPACTA_CHANGESTATS`, `EMIMPACTA_HP_RECOVERY_VAR`,
`EMIMPACTA_MP_RECOVERY_VAR`, `EMIMPACTA_SP_RECOVERY_VAR`,
`EMIMPACTA_CP_RECOVERY_VAR`, `EMIMPACTA_CP_AUTO_VAR`
(`GLCharDefine.h:990-995`).

The skill-FACT switch `:2323-2350` ends at `RESIST`; there is **no case for any of
these six**. Each does have six `.cpp` references elsewhere in the tree, which
were **not** traced in this milestone.

**DEFERRED — NOT PROVEN.** No implementation decision may be taken on these six
from an enum name alone. Note also that RAN has a "CP" resource concept with no
modern counterpart at all.

## 5. EMSPECA Consumer Matrix

Already aggregated by VERTICAL-015 (behaviour unchanged and re-confirmed):

| Spec | Legacy operation | Source | Modern owner | Status |
| --- | --- | --- | --- | --- |
| `NONBLOW` | `m_dwHOLDBLOW = dwSPECFLAG` (**assign**) | `:2357` | → `StatusEffectResolver` | connected (§8 of V015) |
| `MOVEVELO` | `m_fSKILL_MOVE += fSPECVAR1` | `:2360` | none authoritative | **DEFERRED** (§9) |
| `ATTACKVELO` | `m_fATTVELO -= fSPECVAR1` | `:2364` | none | **DEFERRED** (§9) |
| `PROHIBIT_SKILL` | `m_bProhibitSkill = true` | `:2377` | `ActiveSkillResolver::skillProhibited` | **IMPLEMENTABLE NOW** (§6) |
| `PROHIBIT_POTION` | `m_bProhibitPotion = true` | `:2374` | none | **DEFERRED** (§7) |
| `PSY_DAMAGE_REDUCE` | `if (cur < v) cur = v` (**max**) | `:2379-2382` | `CombatInput::targetDamageReduce` | **IMPLEMENTABLE NOW** (§10) |
| `MAGIC_DAMAGE_REDUCE` | `if (cur < v) cur = v` (**max**) | `:2384-2387` | `CombatInput::targetMagicDamageReduce` | **IMPLEMENTABLE NOW** (§10) |
| `PSY_DAMAGE_REFLECTION` | max on amount, **rate paired from the same spec** | `:2389-2394` | `CombatInput::targetDamageReflection{,Rate}` | **IMPLEMENTABLE NOW** (§10) |
| `MAGIC_DAMAGE_REFLECTION` | same | `:2396-2401` | `CombatInput::targetMagicDamageReflection{,Rate}` | **IMPLEMENTABLE NOW** (§10) |

Stored but not aggregated by V015 — see §13.

## 6. Prohibit Skill Call Path

**PROVEN.**

Aggregation: `GLogixExPC.cpp:2377`, inside the per-tick pass. Reset to `false`
at `:2257` (and `:176` at construction), so it is rebuilt every tick.

Read site — the **only** one on the player path:

```
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:4056
GLCHARLOGIC::CHECHSKILL ( SNATIVEID skill_id, WORD wStrikeNum, bool bNotLearn )
:4058   /*prohibit skill logic, Juver, 2017/06/06 */
:4060   if ( m_bProhibitSkill || m_bCaptureTheFlagHoldFlag )  return EMSKILL_PROHIBIT;
:4063   if ( m_bSTATE_STUN )                                   return EMSKILL_PROHIBIT;
:4070/:4077   learned check  -> EMSKILL_NOTLEARN
:4082-4083   delay check     -> EMSKILL_DELAYTIME
```

Answering §11 explicitly, from those lines:

- **Where checked:** `CHECHSKILL`, per skill.
- **What is blocked:** any skill passed through `CHECHSKILL`.
- **All skills?** Yes — the test is the **first statement in the function**,
  before the learned check at `:4077`, so an unlearned skill reports
  `EMSKILL_PROHIBIT` rather than `EMSKILL_NOTLEARN`.
- **Passives affected?** Passives are not cast through `CHECKSKILL`, so no.
- **Categories excluded?** None found.
- **Before SP/resource deduction?** Yes — deduction happens later in `SkillProc`
  (`GLChar.cpp:3003-3009`), well after this check.
- **Before cooldown?** Yes — the delay test is at `:4082-4083`, after `:4060`.
- **Before target validation?** Yes — `CHECKSKILL` takes no target.
- **Can the source FACT be removed/overridden?** No path found; expiry is the
  only route.

Modern counterpart: `Modern::Skills::ActiveSkillInput::skillProhibited`
(VERTICAL-011) already exists, and `ActiveSkillResolver` reads it at
`ActiveSkill.cpp:145`. **What is missing is that `ServerCharacter::CastSkill`
never populates it** from the FACT aggregation. That is the whole gap, and it is
a wiring change, not a rule change.

**Ordering, compared precisely.** Modern `ActiveSkillResolver` checks
`:127` level → `:133` role → `:145` prohibit/stun → `:156` cooldown. Legacy
checks `:4060` prohibit → `:4063` stun → `:4077` learned → `:4082` delay. The
modern sequence therefore evaluates `role` *before* the prohibition, where legacy
evaluates the prohibition first. Both outcomes map to
`ActiveSkillFailure::NotCastable`, so no observable behaviour differs today — but
the divergence should not grow silently if either side is later given a distinct
failure code.

Also note the legacy OR with `m_bCaptureTheFlagHoldFlag` is a PvP flag with no
modern equivalent — **DEFERRED — NOT PROVEN**, and it must not be silently
dropped from the modern rule.

## 7. Prohibit Potion Call Path

**DEFERRED — ITEM/POTION CONSUMER NOT IMPLEMENTED.**

Aggregation: `GLogixExPC.cpp:2374`, reset at `:2256`-block, read only at:

| Read site | Operation |
| --- | --- |
| `GLCharInvenMsg.cpp:5054` | `if ( m_bProhibitPotion ) return E_FAIL;` — item use |
| `GLCharStorageMsg.cpp:224` | `if ( m_bProhibitPotion ) return E_FAIL;` — storage action |
| `GLCharactorReq.cpp:1753`, `:3593` | item-request paths |

Every consumer is an **inventory / item-use / storage** path. `modern/` has no
item-use system, no storage system and no potion concept. Implementing one is
explicitly out of scope for this milestone, so this stays aggregated-but-unconsumed,
exactly as VERTICAL-015 left it.

## 8. NONBLOW / Status Integration

**PROVEN**, and already implemented in VERTICAL-015. Re-confirmed boundary:

```
legacy/GLogixExPC.cpp:2357      m_dwHOLDBLOW = dwSPECFLAG      (assign, not OR)
legacy/GLChar.cpp:3376          if ( !(GETHOLDBLOW() & STATE_TO_DISORDER(type)) )
                                    bBLOW = CHECKSTATEBLOW(...);
```

- **Prevents application?** Yes — it gates the probability call entirely, so the
  roll is never consumed.
- **Only certain states?** Yes — whatever bits the mask names.
- **Assigned or OR'd?** **Assigned.** Two `NONBLOW` specs do not combine; the
  last one aggregated replaces the earlier mask outright.
- **Ordering matter?** Yes, for that reason. It is slot-order dependent.
- **Expiry immediate?** Yes, with the VERTICAL-015 off-by-one: a fact still
  contributes on the tick it expires.
- **Should the status resolver receive the mask directly?** Yes, and it does.

The architecture is exactly as §13 requires, and the domains are not merged:

```
SkillFactContainer
  -> SkillFactModifiers::statusImmunityMask      (aggregation)
  -> ServerCharacter::AdvanceSkillFacts          (authoritative snapshot)
  -> ActiveSkillInput::targetDisorderMask        (value crossing)
  -> StatusEffectResolver                        (decision)
  -> StatusEffectContainer                       (storage)
```

`StatusEffectContainer` never inspects FACT records, and `SkillFactContainer`
never inspects status slots.

### Gap: the hand-off is proven but not wired into the real cast path

VERTICAL-015 proved this chain and tested it at unit level, but
`ServerCharacter::CastSkill` never populates `ActiveSkillInput::targetDisorderMask`
from `GetFactModifiers().statusImmunityMask`. So in an actual cast today, a
character wearing a `NONBLOW` FACT is **not** protected — only a test that injects
the mask by hand sees the immunity. This is a small, fully proven wiring gap and
is listed as item 4 in §17.

## 9. Movement / Attack Velocity

**V015's signs are confirmed correct:**

```
legacy/GLogixExPC.cpp:2360   EMSPECA_MOVEVELO    m_fSKILL_MOVE += fSPECVAR1
legacy/GLogixExPC.cpp:2364   EMSPECA_ATTACKVELO  m_fATTVELO   -= fSPECVAR1
```

The `ATTACKVELO` sign inversion is genuine and is corroborated by the legacy
comment at `:2363`, which states a value of `-0.1` (-10%) makes things *faster*.

**Downstream consumer tracing:**

| Accumulator | Server-side read | Client-side read |
| --- | --- | --- |
| `m_fSKILL_MOVE` | **none found** | `GLCharClient.cpp:1252`, `:3725` |
| `m_fATTVELO` | `GLChar.cpp:2742`, `:4724` — `m_fattTIMER += fElapsedTime * GLCHARLOGIC::GETATTVELO()` | `GLCharClient.cpp:1246`, `:3738` |

So `MOVEVELO` is aggregated on the server and consumed **only by the client** for
animation. `ATTACKVELO` does have one server-side use — an attack-interval timer —
but `GLCHARLOGIC::GETATTVELO()`'s body could not be read from the header-only
declaration at `GLogicEx.h:821` within this milestone.

**DEFERRED — MOVEMENT / ATTACK-SPEED CONSUMER NOT IMPLEMENTED.** `modern/` has no
movement system and no attack-interval timer, and the `GETATTVELO` formula is
**NOT PROVEN**. Creating either would be simulation code, which §14 forbids.

## 10. Damage Reduction / Reflection

**The strongest IMPLEMENTABLE candidate, and it needs no new subsystem.**

Aggregation (`:2379-2401`) writes into `m_sDamageSpec`, which is the same
`DAMAGE_SPEC` structure that `CALCDAMAGE_20060328` reads:

```
legacy/GLogixExPC.cpp:1482-1484
  fDamageReduce         = sDamageSpec.m_fMagicDamageReduce;      // magic
  fDamageReflection     = sDamageSpec.m_fMagicDamageReflection;
  fDamageReflectionRate = sDamageSpec.m_fMagicDamageReflectionRate;
legacy/GLogixExPC.cpp:1409-1411   (defaults, physical path)
  fDamageReduce = sDamageSpec.m_fPsyDamageReduce;               // physical/ranged
```

Modern already owns every destination field, added by VERTICAL-013:

| Legacy field | Modern field |
| --- | --- |
| `m_fPsyDamageReduce` | `CombatInput::targetDamageReduce` |
| `m_fMagicDamageReduce` | `CombatInput::targetMagicDamageReduce` |
| `m_fPsyDamageReflection` / `Rate` | `CombatInput::targetDamageReflection{,Rate}` |
| `m_fMagicDamageReflection` / `Rate` | `CombatInput::targetMagicDamageReflection{,Rate}` |

`SkillFactAggregator` already produces the values. Nothing is duplicated: the
combat calculators are untouched.

**Verified stacking semantics** (from `:2379-2401`):

- Reduction: **maximum**, not sum.
- Reflection: **maximum on the amount**, and the **rate is taken from that same
  spec** — amount and rate are never mixed between different facts.
- Physical/ranged share one pair; magic has its own. They do not interact.

### ⚠ A caveat the next vertical must resolve first

The reduction and reflection specs **max-accumulate into a member struct**,
`m_sDamageSpec`, and no reset of those fields appears in the reset block
(`:2255-2281`) that clears `m_bProhibitSkill`, `m_fSKILL_MOVE` and the rest.

The move/attack-velocity/prohibit flags demonstrably *are* rebuilt from zero each
tick. Whether `m_sDamageSpec` is likewise reset somewhere outside this function
was **NOT PROVEN** in this milestone. If it is not, the practical effect is that
once a reduction FACT is applied the value never drops again for that character.

This is recorded as an open question, not as a bug. **The next vertical must
prove the reset before wiring these four fields**, otherwise it will reproduce an
accumulation that may not exist in RAN.

## 11. Resource Impacts

Two distinct mechanisms, which must not be conflated:

**(a) Recovery of the *current* pool.** `VARHP`/`VARMP`/`VARSP`/`VARAP` feed
**function-local** floats `fINCR_HP/fINCR_MP/fINCR_SP` (`:2331-2338`), consumed
within the same pass to restore current values over time. They never touch a
maximum.

**(b) Maximums.** `HP_RATE`/`MP_RATE`/`SP_RATE` accumulate into `m_fHP_RATE`
etc. (`:2346-2348`) and are consumed **multiplicatively** on the maximum:

```
legacy/GLogixExPC.cpp:2165
  m_sHP.dwMax = DWORD ( m_sHP.dwMax
                        * (1 + m_sSUM_PASSIVE.m_fHP_RATE + m_fHP_RATE)
                        * fCONFT_POINT_RATE );
```

Note that `m_sSUM_PASSIVE.m_fHP_RATE` (a **learned passive**) and `m_fHP_RATE` (a
**timed FACT**) are distinct fields summed at the same expression. This is direct
source evidence for §6's rule that FACT aggregation must not be folded into
`PassiveContribution`: legacy itself keeps them separate and adds them at the
point of use.

**Expiry semantics:** because the whole accumulator set is rebuilt from zero each
tick (§15), an expired rate FACT stops contributing on the next pass — but note
the maximum itself is only recomputed when the stat pipeline runs, so the exact
moment the maximum shrinks depends on the recalculation trigger, which is
**NOT PROVEN** here.

`modern/` has `ResourceState` (current/max per kind) and `DerivedStats`
(maximums), so both branches have a plausible owner. The recovery **loop** does
not exist, which is what makes (a) PARTIAL.

## 12. TAR_BUFF

**DEFERRED — NOT PROVEN / WORLD-ENTITY REGISTRY REQUIRED.**

The only `TAR_*` handling reachable from the FACT creation path in this milestone
is the *self-buff* matrix at `GLChar.cpp:6420-6507`, which gates
`TAR_SELF + REALM_SELF + SIDE_OUR` by class and skill main-ID and refuses
disallowed combinations. That is a **filter**, not a target resolver: it rejects
before any entity lookup happens.

Resolving *which entity* a `TAR_BUFF` skill lands on requires the entity registry
and land/area manager (`GLGaeaServer::GetTarget`, `m_pLandMan`), neither of which
exists in `modern/`. Per §18 no fake registry is created.

**NOT PROVEN** for this milestone: whether the target must already hold a related
FACT, and whether validation is server-side at the cast site or at application
time. Both require code paths that were not traced here.

## 13. Special FACT Effects

Stored by `RECEIVE_SKILLFACT` (`:6551-6601`) but **not aggregated** by
VERTICAL-015:

| Spec | Legacy consumer | Status |
| --- | --- | --- |
| `EMSPECA_SKILLDELAY` | `m_fSKILLDELAY += fSPECVAR1` `:2365`, folded into cooldown at `:2984` | **PARTIAL** — modern cooldown exists, but the fact-owned delay term does not |
| `EMSPECA_INVISIBLE` / `RECVISIBLE` | `m_bINVISIBLE`/`m_bRECVISIBLE` `:2361-2362` | **DEFERRED — CLIENT PRESENTATION** |
| `EMSPECA_PIERCE` | `m_nSUM_PIERCE += int(fSPECVAR1)` `:2358` | **DEFERRED — needs hit-pierce rules** |
| `EMSPECA_TARRANGE` | `m_fSUM_TARRANGE += fSPECVAR1` `:2359` | **DEFERRED — needs world targeting** |
| `EMSPECA_CHANGE_ATTACK_RANGE` / `CHANGE_APPLY_RANGE` | `:2429`, `:2434` | **DEFERRED — needs range/spatial system** |
| `EMSPECA_STUN` | `:2441` | **DEFERRED — V014 status path owns this** |
| `EMSPECA_CONTINUOUS_DAMAGE` | `:2448`, read at `GLChar.cpp:5359-5360` | **DEFERRED — NO DoT SYSTEM** |
| `EMSPECA_CURSE` | `:2465` | **DEFERRED — curse damage not modelled** |
| `EMSPECA_IGNORE_DAMAGE` | `:2472` | **DEFERRED — no modern damage-immunity flag** |
| `EMSPECA_IMMUNE` | `:2477` | **DEFERRED — NOT PROVEN** |
| `EMSPECA_STIGMA` | `:2410` | **DEFERRED — stigma system** |
| `EMSPECA_ENHANCEMENT` | `m_sIncreaseEff.SET(...)` `:2369` | **DEFERRED — enhancement subsystem** |
| `EMSPECA_DEFENSE_SKILL_ACTIVE` | `:2403-2408` | **DEFERRED — defence-skill system** |
| `EMSPECA_REFDAMAGE` | `:6555` (stored), consumer not traced | **DEFERRED — NOT PROVEN** |
| `EMSPECA_TALK_TO_NPC` | `:6575` (stored), consumer not traced | **DEFERRED — NOT PROVEN** |
| `EMSPECA_DAMAGE_LOOP` | `:6588-6599` — mutates `m_sHP.dwMax` directly | **DEFERRED — HP-link system** |

Per §19, the fact that legacy *stores* these proves transport only, not that a
consumer should be implemented. None were implemented.

### Related evidence on `fVAR1`/`fVAR2` semantics

The state-blow loop (`GLogixExPC.cpp:2498-2520`) is the clearest statement of
what those variables mean:

```
:2500-2503   EMBLOW_NUMB    m_fSTATE_MOVE  += fSTATE_VAR1;
                               m_fSTATE_DELAY += fSTATE_VAR2;
:2506-2509   EMBLOW_STUN    m_fSTATE_MOVE  = 0.0f;  (hard set, not accumulate)
:2512-2515   EMBLOW_STONE   m_fSTATE_MOVE  += fSTATE_VAR1;
                               m_fIncHP      += fSTATE_VAR2 * fElapsedTime;
:2518-2520   EMBLOW_BURN    m_fIncHP      += fSTATE_VAR2 * fElapsedTime;
:2523-2526   EMBLOW_FROZEN  m_fSTATE_MOVE  += fSTATE_VAR1;
                               m_fSTATE_DAMAGE += fSTATE_VAR2;
```

So `fVAR1` is generally a movement term and `fVAR2` a second-order term whose
meaning is **per state**. This is the evidence VERTICAL-014 used to store them
without interpreting them, and it remains the right call.

## 14. Multiple FACT Stacking

Every rule differs per modifier; there is no single FACT stacking rule.

| Modifier | Combination rule | Evidence |
| --- | --- | --- |
| `PA`/`SA`/`MA` | **SUM** (`+= int(v)`) | `:2343-2345` |
| `DAMAGE` | **SUM**, via saturating `VAR_PARAM` on the range | `:2329` |
| `HITRATE`/`AVOIDRATE` | **SUM** (`+= int(v)`) | `:2327-2328` |
| `DEFENSE` | **SUM** | `:2330` |
| `VARHP/MP/SP`, `VARAP` | **SUM** | `:2331-2338` |
| `DAMAGE_RATE`, `DEFENSE_RATE` | **SUM** | `:2340-2341` |
| `HP/MP/SP_RATE` | **SUM** | `:2346-2348` |
| `RESIST` | **SUM** | `:2349` |
| `MOVEVELO` | **SUM** | `:2360` |
| `ATTACKVELO` | **SUM**, then sign-flipped | `:2364` |
| `NONBLOW` | **LAST-WINS / REPLACE** (assignment) | `:2357` |
| `PSY/MAGIC_DAMAGE_REDUCE` | **MAX** | `:2379-2387` |
| `PSY/MAGIC_DAMAGE_REFLECTION` | **MAX** on amount, **PAIR-REPLACE** rate from that spec | `:2389-2401` |
| `PROHIBIT_SKILL`/`PROHIBIT_POTION` | **OR / boolean set** | `:2374`, `:2377` |

**Slot order matters** in exactly three places, all because of last-wins
semantics: `NONBLOW` (assignment) and the reflection pairs (whose rate follows
whichever amount won). Everything additive and every maximum is
order-independent.

## 15. Expiry / Restoration

**PROVEN, and the architecture is not save/restore.**

```
legacy/GLogixExPC.cpp:2255-2281
  the accumulator set is reset to its defaults at the top of the function
  (m_bProhibitSkill = false at :2257, m_fSKILL_MOVE = 0 at :3697 on the
  client, and the rest just above)
  :2280   for ( i < SKILLFACT_SIZE )  if ( sNATIVEID == NULL )  continue;
  :2292   sSKEFF.fAGE -= fElapsedTime;
  :2295   if ( !bClient && sSKEFF.fAGE <= 0.0f )  DISABLESKEFF(i);
```

Legacy never saves an old value and restores it. It **rebuilds every modifier
from the currently active FACT set on every tick**, so returning to baseline is a
consequence of the fact ceasing to be counted, not an action.

Verified for: movement, attack velocity, prohibit flags, immunity, reductions,
reflections, hit/avoid, damage and the rate accumulators.

**Documented exceptions / caveats:**

1. **Off-by-one.** `:2295` disables *after* the `continue` guard at `:2280` has
   already passed, so the spec switches still run. A fact contributes on the tick
   it expires. Reproduced in VERTICAL-015.
2. **`m_sDamageSpec`** — the reduction/reflection specs write into a member
   struct, and no reset of those fields is visible in `:2255-2281`. See §10.
   **NOT PROVEN** whether it is reset elsewhere.
3. **Derived maxima.** `m_sHP.dwMax` is only recomputed when the stat pipeline
   runs (`:2165`), so a `HP_RATE` FACT's disappearance does not by itself shrink
   the stored maximum. Depends on the recalculation trigger — **NOT PROVEN**.
4. **Orphan FACTs.** `:2283-2284` skips a fact whose skill definition cannot be
   resolved, but does **not** disable it. Such a fact keeps ageing out and
   contributes nothing meanwhile.

## 16. Existing Modern Consumer Mapping

```
SkillFactContainer (14 slots, V015)
   │
   ├── SkillFactModifiers (aggregation, V015)
   │     │
   │     ├── statusImmunityMask ──> ActiveSkillInput::targetDisorderMask
   │     │                          ──> StatusEffectResolver ──> StatusEffectContainer
   │     │                          CONNECTED (V015)
   │     │
   │     ├── prohibitSkill ──> ActiveSkillInput::skillProhibited
   │     │                     ──> ActiveSkillResolver refusal
   │     │                     GAP: CastSkill never populates it
   │     │
   │     ├── psyDamageReduce / magicDamageReduce
   │     ├── psyDamageReflection{,Rate} / magicDamageReflection{,Rate}
   │     │        ──> CombatInput target* fields (V013) ──> existing calculators
   │     │        READY, pending the m_sDamageSpec reset question (§10)
   │     │
   │     ├── moveVelocity / attackVelocity ──> NOTHING
   │     │        DEFERRED (§9)
   │     │
   │     └── prohibitPotion ──> NOTHING (no item-use system)
   │              DEFERRED (§7)
   │
   └── impacts (stored, unaggregated)
         │
         ├── PA/SA/MA ──────> StatCalculationInput ──> DerivedStats::{melee,shoot,magic}Power
         ├── HP/MP/SP_RATE ─> StatCalculationInput ──> DerivedStats maxima
         ├── HITRATE/AVOIDRATE ─> StatCalculationInput ──> DerivedStats::{hit,avoid}
         ├── DAMAGE ────────> CombatInput::attackerPhysicalDamage
         ├── DAMAGE_RATE ───> CombatInput::attackerDamageRate  (magic only; GAP in physical)
         ├── VARHP/MP/SP ───> recovery loop MISSING
         ├── RESIST ────────> StatCalculationInput ──> DerivedStats::resistances
         └── 6 recovery/CP ──> NOT PROVEN
```

### The duplication check (§6) — three concrete outcomes

1. **PA/SA/MA must NOT go into `PassiveContribution`.** Legacy keeps
   `nSUM_MA` separate from `m_sSUM_PASSIVE.m_nMA` and adds them at the point of
   use (`:2970-2972`), and modern mirrors this with three distinct contribution
   structs. FACT needs a **fourth** input on `StatCalculationInput` — a
   `FactContribution` — not a new home inside the passive one. Doing otherwise
   would make a timed buff indistinguishable from a learned passive, and would
   survive expiry.

2. **Damage reduction / reflection already have owners.** VERTICAL-013 added the
   four magic/physical pairs to `CombatInput`. FACT should feed those fields. No
   new combat formula, and the V006-V013 calculators stay untouched.

3. **A pre-existing gap surfaced.** `PhysicalDamageInput` has **no**
   `damageRate` field, while legacy applies `ApplyDamageRate` to the range at
   `:1600-1603` for *every* channel. Only the magic calculator (V013) has it.
   This is **not** caused by FACT and is recorded, not fixed, here — fixing it
   would change V009 physical expectations, which §24 forbids.

## 17. IMPLEMENTABLE NOW

Only where legacy behaviour is proven, a modern owner exists, and expiry
semantics are unambiguous.

| # | Item | Legacy evidence | Modern owner | Why safe |
| --- | --- | --- | --- | --- |
| 1 | **`PROHIBIT_SKILL` → `CastSkill`** | `:2377` aggregate; `:4060` check, first statement in `CHECHSKILL`, before learned/cooldown/target | `ActiveSkillInput::skillProhibited` (V011) already exists; `CastSkill` populates it | Pure wiring. The resolver already implements the refusal and the ordering. Caveat: legacy also ORs the capture-the-flag flag, which has no modern equivalent |
| 2 | **Damage reduction + reflection → `CombatInput`** | `:2379-2401`; read at `:1409-1411` and `:1482-1484` | `CombatInput::target{Damage,MagicDamage}{Reduce,Reflection,ReflectionRate}` (V013) | Fields exist; calculators untouched; stacking already verified in V015 |
| 3 | **`PA`/`SA`/`MA` → `FactContribution`** | `:2343-2345`; legacy sums it separately from passives at `:2970-2972` | new fourth `StatCalculationInput` contribution → `DerivedStats` | Additive, order-independent, rebuilt from zero on expiry. Requires a *new input field*, not a new stat system |
| 4 | **NONBLOW mask → `CastSkill`** | `:2357` → `:3376-3379`; already proven end-to-end in V015 unit tests | `ServerCharacter::AdvanceSkillFacts` → `ActiveSkillInput::targetDisorderMask` | The consumer already exists on both sides; only the server-side assignment is missing |

Item 2 should not land before the `m_sDamageSpec` reset question (§10) is
resolved.

## 18. PARTIAL

| Item | Blocked by |
| --- | --- |
| `HITRATE`/`AVOIDRATE` | Same wiring as #3, but they feed the hit path; `HitCalculator` exists, so this is close to ready |
| `HP/MP/SP_RATE` | Consumer proven (`:2165`, multiplicative), but the maximum-recalculation trigger on expiry is **NOT PROVEN** |
| `VARHP/MP/SP`, `VARAP` | No recovery loop exists in `modern/` |
| `EMIMPACTA_DAMAGE` | Which range owns it differs per channel (skill range vs `m_gdDAMAGE_SKILL` vs the physical range at `:2997-3002`); the boundary is not yet decided |
| `EMSPECA_SKILLDELAY` | Cooldown exists; the fact-owned delay term and its interaction with V011's `requiredSP` are untraced |

## 19. DEFERRED

`DEFENSE` (no modern owner for the body/item split), `DEFENSE_RATE` (no modern
consumer), `RESIST` (axis granularity differs), `DAMAGE_RATE` into physical (the
`PhysicalDamageInput` gap — recorded, not fixed here), all six recovery/CP
impacts (**NOT PROVEN**), `MOVEVELO`, `ATTACKVELO`, `PROHIBIT_POTION`,
`INVISIBLE`/`RECVISIBLE`, `PIERCE`, `TARRANGE`, `CHANGE_*_RANGE`, `STUN`,
`CONTINUOUS_DAMAGE`, `CURSE`, `IGNORE_DAMAGE`, `IMMUNE`, `STIGMA`,
`ENHANCEMENT`, `DEFENSE_SKILL_ACTIVE`, `REFDAMAGE`, `TALK_TO_NPC`,
`DAMAGE_LOOP`, and `TAR_BUFF` target resolution.

`DEFERRED — NOT PROVEN` specifically: the six recovery/CP impacts, `EMSPECA_REFDAMAGE`,
`EMSPECA_TALK_TO_NPC`, `EMSPECA_IMMUNE`, `GETATTVELO`'s formula, the
`m_sDamageSpec` reset, and the maximum-recalculation trigger.

## 20. Recommended Next Vertical

**VERTICAL-017 — FACT combat/skill consumer wiring**, scoped to the three items
in §17, in this order:

1. Resolve the `m_sDamageSpec` reset question (one focused legacy read). Without
   it, item 2 risks reproducing an accumulation RAN may not have.
2. Wire `prohibitSkill` **and** the NONBLOW mask into `CastSkill` — both are
   proven, both are pure wiring, and both are currently invisible in a real cast.
3. Add `FactContribution` to `StatCalculationInput` for PA/SA/MA (and, if
   desired, hit/avoid), leaving `PassiveContribution` untouched.

A separate later slice should take the resource impacts and rates once a
recovery loop exists. Movement, potions, `TAR_BUFF` and the special effects each
need their own missing subsystem first.

## 21. Exact Legacy Evidence

```
legacy/Lib_Client/G-Logic/GLChar.cpp:6377-6408    SELECT_SKILLSLOT          slot cascade
legacy/Lib_Client/G-Logic/GLChar.cpp:6410-6613    RECEIVE_SKILLFACT        creation
legacy/Lib_Client/G-Logic/GLChar.cpp:6420-6507    self-buff class gate
legacy/Lib_Client/G-Logic/GLChar.cpp:6541-6549    impact copy              wholesale
legacy/Lib_Client/G-Logic/GLChar.cpp:6551-6601    spec copy                whitelisted only
legacy/Lib_Client/G-Logic/GLChar.cpp:6609         fAGE = fLIFE
legacy/Lib_Client/G-Logic/GLChar.cpp:3376-3379   GETHOLDBLOW immunity gate
legacy/Lib_Client/G-Logic/GLChar.cpp:2742,4724   GETATTVELO -> attack timer
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2255-2281 reset of accumulators
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2292-2295 tick + expire + off-by-one
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2309-2349 EMFOR_*/EMIMPACTA_* aggregation
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2353-2401 EMSPECA_* aggregation
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2357     NONBLOW assignment
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2360,2364 MOVEVELO / ATTACKVELO signs
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2374,2377 PROHIBIT_POTION / PROHIBIT_SKILL
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2379-2401 damage reduce / reflection max
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2498-2526 state-blow fVAR1/fVAR2 meaning
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:1600-1603 ApplyDamageRate on the range
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2165      HP max = *(1+passiveRate+fACT_RATE)
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2970-2972 attack powers: passive and FACT summed separately
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:2997-3002 physical range build
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:4056-4083 CHECHSKILL, prohibit at :4060
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:1409-1411 psy reduce/reflection defaults
legacy/Lib_Client/G-Logic/GLogixExPC.cpp:1473-1487 EMAPPLY_MAGIC reads magic values
legacy/Lib_Client/G-Logic/GLCharInvenMsg.cpp:5054 prohibit potion, item use
legacy/Lib_Client/G-Logic/GLCharStorageMsg.cpp:224 prohibit potion, storage
legacy/Lib_Client/G-Logic/GLCharactorReq.cpp:1753,3593 prohibit potion, item req
legacy/Lib_Client/G-Logic/GLCharClient.cpp:1252,3725 MOVEVELO, client only
legacy/Lib_Client/G-Logic/GLCharDefine.h:968-993 EMIMPACT_ADDON
legacy/Lib_Client/G-Logic/GLCharDefine.h           EMSPEC_ADDON
legacy/Lib_Client/G-Logic/GLFactData.h:50-74      SSKILLFACT
legacy/Lib_Client/G-Logic/GLCharData.h:200-201    SKILLREALFACT_SIZE / SKILLFACT_SIZE = 14
legacy/Lib_Client/G-Logic/GLSkillDefine.h:17-18   MAX_SPEC / MAX_IMPACT = 5
legacy/Lib_Client/G-Logic/GLogicEx.h:821          GETATTVELO declaration (body not read)
```

Modern files inspected for consumer mapping:
`modern/core/skills/SkillFactTypes.h`, `SkillFactContainer.h`,
`SkillFactAggregator.h`, `ActiveSkill.h/.cpp`, `SkillDefinition.h`,
`modern/core/stats/DerivedStats.h`, `Contributions.h`, `StatCalculator.cpp`,
`modern/core/combat/CombatTypes.h`, `CombatCalculator.h`,
`PhysicalDamageCalculator.h`, `MagicDamageCalculator.h`,
`modern/core/resources/ResourceState.h`,
`modern/server/character/ServerCharacter.h/.cpp`.