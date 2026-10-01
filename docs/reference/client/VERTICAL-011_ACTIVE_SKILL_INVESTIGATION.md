# VERTICAL-011 — Active Skill Combat: Legacy Investigation

Scope: RAN's first castable-skill path, from the client's request to the damage
and the resources it moves. Every legacy claim below is quoted from a file and
line in `legacy/`. Where the source does not prove a behaviour, the entry says
so and the modern code refuses rather than approximates.

The active half is deliberately separate from the passive half. RAN keeps them
apart too: `SUM_PASSIVE` (`GLogixExPC.cpp:918-1002`) aggregates a skill when it
is *learned*; casting runs `CHECHSKILL` -> `ACCOUNTSKILL` -> `PreStrikeProc` ->
`SkillProc`. No function is shared. Forcing modern active execution through
`PassiveContributionAggregator` would collapse that separation.

---

## 1. Entry point

`GLChar::MsgReqSkill` — `legacy/Lib_Client/G-Logic/GLCharSkillMsg.cpp:222`.

In order, before any skill rule runs:

| Line | Check | Kind |
| --- | --- | --- |
| 227 | `m_bEmptyMsg` | transport |
| 228 | `IsValidBody()` | world/state |
| 229 | `m_pLandMan->IsPeaceZone()` | world |
| 236 | `m_fSkillDelay < 0.3f` -> `E_FAIL` | packet rate limiter |
| 240-242 | free-PK map, guid battle, bright event | world |
| 250-254 | `bDefenseSkill` checks | character |
| 258 | `GLSkillMan::GetData(skill_id)` | skill lookup |
| 265-275 | character-class animation table; `wStrikeNum = sAniAttack.m_wDivCount`; reject if 0 | **client animation data** |
| 357 | `CHECHSKILL(skill_id, 1, bDefenseSkill)` | skill rules |
| 453-680 | range, angle, per-target validity | world |
| 980 | `bLowSP = (emCHECK==EMSKILL_NOTSP)` | skill rules |

Line 236 is a flat 0.3 s limiter on the *message*, not on any skill. It guards
packet rate and belongs with protocol, not with skill rules.

Line 265 reads `SANIATTACK::m_wDivCount` out of the **character-class animation
table** (`GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX].m_ANIMATION[...]`,
`GLogicData.cpp:98-100`). It is not a skill field, and it is not available to a
server that has no animation system.

---

## 2. `CHECHSKILL` — the gate

`legacy/Lib_Client/G-Logic/GLogixExPC.cpp:4056`. In order:

| Line | Rule | Result code |
| --- | --- | --- |
| 4058 | `m_bProhibitSkill \|\| m_bCaptureTheFlagHoldFlag` | `EMSKILL_PROHIBIT` |
| 4062 | `m_bSTATE_STUN` | `EMSKILL_PROHIBIT` |
| 4069-4072 | defense-skill branch | — |
| 4074-4076 | `m_ExpSkills.find(skill_id.dwID)` | `EMSKILL_NOTLEARN` |
| 4082-4083 | `m_SKILLDELAY.find(skill_id.dwID)` | `EMSKILL_DELAYTIME` |
| 4085-4086 | `GLSkillMan::GetData(skill_id)` | `EMSKILL_UNKNOWN` |
| 4088 | `sDATA_LVL[sSkill.wLevel]` | — |
| 4092 | `m_sBASIC.emROLE != EMROLE_NORMAL` | `EMSKILL_UNKNOWN` |
| 4124-4160 | weapon/hand requirements | `EMSKILL_NOTITEM` |
| 4240 | `m_sHP.dwNow <= wUSE_HP*wStrikeNum` | `EMSKILL_NOTHP` |
| 4241 | `m_sMP.dwNow < wUSE_MP*wStrikeNum` | `EMSKILL_NOTMP` |
| 4254-4258 | SP, see §5 | `EMSKILL_NOTSP` |
| 4260 | `m_sCombatPoint.dwNow < wUSE_CP*wStrikeNum` | `EMSKILL_NOTCP` |

Note the level is taken from the character's own `m_ExpSkills` at 4075, never
from the message. **A caller cannot choose the cast level in RAN**, and the
modern server ignores its `requestedLevel` parameter for the same reason.

Note also the operator asymmetry at 4240/4241: HP is refused at `<=`, MP at `<`.
Both are reproduced.

---

## 3. Skill lookup and level

`GLSkillMan::GetData(skill_id)` is a global table lookup
(`legacy/Lib_Client/G-Logic/GLSkill.h:50`), keyed by `(wMainID, wSubID)`. The
modern `SkillDefinitionProvider::Find` is the same shape.

`MAX_LEVEL = 9` (`GLSkillDefine.h:16`) sizes `sDATA_LVL[9]`
(`GLSkillApply.h:580`). `m_sBASIC.dwMAXLEVEL` (`GLSkillBasic.h:527`, default 9)
is a per-skill authoring cap that the fixed array forces to be <= 9.

There is **no accessor** for `sDATA_LVL`; every call site subscripts raw. The
modern `SkillDefinition::GetLevelData` returns the level-0 record for an
out-of-range level instead of indexing past the end, which is a deliberate
departure from a raw subscript.

---

## 4. The data a cast actually needs

`SKILL::CDATA_LVL` — `legacy/Lib_Client/G-Logic/GLSkillApply.h:245-295`:

| Field | Line | Modern |
| --- | --- | --- |
| `fDELAYTIME` | 247 | `SkillLevelData::delayTime` |
| `fBASIC_VAR` | 254 | `SkillLevelData::basicVar` (existing) |
| `wUSE_HP` | 259 | `SkillLevelData::useHp` |
| `wUSE_MP` | 260 | `SkillLevelData::useMp` |
| `wUSE_SP` | 261 | `SkillLevelData::useSp` |
| `wAPPLYRANGE` | 249 | deferred, needs a world |
| `wAPPLYNUM` / `wAPPLYANGLE` | 250-251 | deferred, needs a world |
| `wPIERCENUM` / `wTARNUM` | 252-253 | deferred, needs a world |
| `wUSE_ARROWNUM` / `CHARMNUM` / `BULLETNUM` | 255-257 | deferred, needs an inventory |
| `wUSE_EXP` / `wUSE_CP` / `*_PTY` | 258, 262, 264-266 | deferred, no combat-point or party system |

`SKILL::SSKILLBASIC` — `GLSkillBasic.h:517-592`: `emROLE` (531), `emAPPLY` (530),
`emIMPACT_SIDE` (537), `emIMPACT_TAR` (538), `emIMPACT_REALM` (539),
`emUSE_LITEM`/`emUSE_RITEM` (541-542), `dwGRADE` (528), `wTARRANGE` (534).
`emBASIC_TYPE` and `emELEMENT` are **not** in `SSKILLBASIC` — they are in
`SKILL::SAPPLY` (`GLSkillApply.h:578-579`).

The modern `SkillDefinition` gained `role`, `apply`, `targetKind`, `impactSide`
and the four per-level cost fields. Everything else stayed out.

---

## 5. SP: the gate and the charge are different

This is the asymmetry VERTICAL-010 predicted, and it is confirmed.

**The gate**, `GLogixExPC.cpp:4254-4258`:

```cpp
WORD wDisSP = sSKILL_DATA.wUSE_SP;
if ( pRHAND )	wDisSP += pRHAND->sSuitOp.wReqSP;
if ( pLHAND )	wDisSP += pLHAND->sSuitOp.wReqSP;

if ( m_sSP.dwNow < wDisSP*wStrikeNum )									return EMSKILL_NOTSP;
```

Equipment hand terms, no `m_wACCEPTP`, multiplied by the strike count.

**The charge**, `GLChar.cpp:3003-3009` in `SkillProc`:

```cpp
if ( !bLowSP )
{
    WORD wDisSP = m_wSUM_DisSP + sSKILL_DATA.wUSE_SP;
    m_sSP.DECREASE ( wDisSP );
}
```

`m_wSUM_DisSP`, which **does** contain `m_wACCEPTP`; **not** multiplied by the
strike count; and skipped entirely when low-SP.

Three differences: the `m_wACCEPTP` term, the strike multiplier, and the
low-SP skip.

**Modern behaviour.** The charge is `equipmentRequiredSP + wUSE_SP`, the same
value the gate measured. This differs from legacy by one term: `m_wACCEPTP` is
absent because VERTICAL-010 established the modern model has no field for it (it
is a per-item stat-deficit penalty, `CALC_ACCEPTP`,
`GLogixExPC.cpp:3443-3461`), and because the legacy *gate* does not use it
either. Gate and charge agreeing is worth more here than matching a term the
gate ignores. Recorded as a departure in §10.

Strike count is 1 in both, because every call site passes a literal 1 (§9).

### Low SP does not refuse a cast

The server's first check at `GLCharSkillMsg.cpp:357-365` rejects anything but
`EMSKILL_OK` — the `EMSKILL_NOTSP` tolerance is commented out. The
*running-cast* re-check at `GLChar.cpp:4797-4798` does tolerate it:

```cpp
EMSKILLCHECK emCHECK = GLCHARLOGIC::CHECHSKILL ( m_idACTIVESKILL, 1, IsDefenseSkill() );
if ( emCHECK != EMSKILL_OK && emCHECK != EMSKILL_NOTSP )   { ...; return FALSE; }
...
BOOL bLowSP =  (emCHECK==EMSKILL_NOTSP) ? TRUE : FALSE;
PreStrikeProc ( TRUE, bLowSP );
```

So the same code base both rejects and tolerates it, depending on which entry
point is used. The cast that actually runs is the tolerated one, and
`ActiveSkillResolver` follows it: low SP degrades, it does not refuse. That is
why `LowSpState` is a flag on a successful result rather than a failure code.

### What low SP changes for a skill

- **No SP charge.** `GLChar.cpp:3005`, the `if (!bLowSP)` guard.
- **Hit and damage penalties.** The skill's damage goes through `CALCDAMAGE`,
  which applies `fLOWSP_DAMAGE` at `GLChar.cpp:2488-2491` and `fLOWSP_HIT_DROP`
  at `GLogixExPC.cpp:1322-1324`, exactly as a basic attack does.
- **Skill variable values are halved.** `GLChar.cpp:3129-3134`:
  `nVAR_HP /= 2; nVAR_MP /= 2; nVAR_SP /= 2;` with the comment
  "apply damage-reduction effect when SP is insufficient". This is the HP/MP/SP
  *drain* branch, not the damage branch, so it does not affect a damage skill.
- `fLOWSP_AVOID_DROP` has no call site anywhere and is not applied here either.

---

## 6. Resource consumption

Two different functions draw two different resources.

`ACCOUNTSKILL` — `GLogixExPC.cpp:4270`, called once per cast at
`GLChar.cpp:4816`:

```cpp
m_sHP.DECREASE ( sSKILL_DATA.wUSE_HP*wStrikeNum );
m_sMP.DECREASE ( sSKILL_DATA.wUSE_MP*wStrikeNum );
m_sCombatPoint.DECREASE ( sSKILL_DATA.wUSE_CP*wStrikeNum );
```

`SkillProc` — `GLChar.cpp:2960`, once per animation division: SP only, §5.

So: HP and MP per cast, SP per strike. `ACCOUNTSKILL` also inserts the
cooldown (§7) and draws **no** SP.

Combat points, EXP and the party-cost `*_PTY` fields are deferred: there is no
combat-point or party system in the modern server.

---

## 7. Cooldown

`m_SKILLDELAY` is `std::map<DWORD,float>` (`GLogicEx.h:38-39`, member at
`:334`), keyed by `skill_id.dwID`, holding **seconds remaining**.

Value, `ACCOUNTSKILL` at `GLogixExPC.cpp:4297-4304`:

```cpp
float fDelayTime = GLOGICEX::SKILLDELAY(pSkill->m_sBASIC.dwGRADE, sSkill.wLevel, GETLEVEL(), sSKILL_DATA.fDELAYTIME);
fDelayTime = fDelayTime * m_fSTATE_DELAY;
if ( bServer )		APPLY_MSGDELAY ( fDelayTime );
m_SKILLDELAY.insert ( std::make_pair(skill_id.dwID,fDelayTime) );
```

Formula, `legacy/Lib_Engine/Common/GameCharacterCalculations.cpp:118-126`:

```cpp
float SkillDelay(GameUInt32 dwSKILL_GRADE, GameUInt16 wSKILL_LEV, GameUInt16 wCHAR_LEVEL, float fDelay)
{
    return static_cast<float>(dwSKILL_GRADE * wSKILL_LEV)
         / static_cast<float>(wCHAR_LEVEL) + fDelay;
}
```

Tick, `GLogixExPC.cpp:3864-3879`: decrement by elapsed, erase at `<= 0`.

Gate, `GLogixExPC.cpp:4082-4083`: `find() != end()` — **presence**, not a
positive remaining time. A zero-delay skill is therefore never insertable.

Two multipliers are not carried: `m_fSTATE_DELAY` (a NUMB-blow debuff scale,
`GLogixExPC.cpp:2501`) and `APPLY_MSGDELAY` (a flat 0.3f network compensation,
`GLConctrlBaseMsg.h:18-27`). Neither has a modern counterpart; one is a status
effect, the other is transport.

The 0.3f `m_fSkillDelay` limiter at `GLCharSkillMsg.cpp:236` is a separate
message-rate guard on `GLChar`, not a per-skill cooldown, and is not ported.

**Modern.** `ServerCharacter` owns `std::map<SkillId,float>` and ticks it
through `AdvanceSkillCooldowns(elapsed)` — time is passed in, so core has no
clock and a test drives expiry deterministically.

---

## 8. Damage

`fBASIC_VAR` is a bare `float` with no type of its own. `emBASIC_TYPE` says what
it means, and for the three active resource types **the sign is the meaning**:

`GLChar.cpp:3075-3123` (and the refactored `ApplySkillTarget_*` at `:9966-10020`):

```cpp
case SKILL::EMFOR_HP:
    if ( sSKILL_DATA.fBASIC_VAR < 0.0f )   // damage
    {
        dwDamageFlag = CALCDAMAGE ( nVAR_HP, ... , sAniAttack.m_wDivCount );
        nVAR_HP = - nVAR_HP;
    }
    else                                     // heal
    {
        int nDX = pACTOR->GetMaxHP() - pACTOR->GetNowHP();
        nVAR_HP += (int) min(nDX, sSKILL_DATA.fBASIC_VAR);
    }
```

A passive ignores the sign and reads the magnitude as a stat delta
(`GLogixExPC.cpp:921-1000`). One field, two readings, selected by role.

The magnitude is formed inside `CALCDAMAGE_20060328`,
`GLogixExPC.cpp:1521-1530`:

```cpp
float fSKILL_VAR = sSKILL_DATA.fBASIC_VAR;
int nVAR = abs ( int(fSKILL_VAR*fPOWER) );
float fGrade = (float) wGRADE / GLCONST_CHAR::fDAMAGE_GRADE_K;     // fDAMAGE_GRADE_K = 10.0f
gdDamage.dwLow  += DWORD (nVAR + ((float) gdDamage.dwLow  * fGrade));
gdDamage.dwHigh += DWORD (nVAR + ((float) gdDamage.dwHigh * fGrade));
```

`fPOWER` is `GLOGICEX::WEATHER_ELEMENT_POW(emELMT, dwWeatherFlag, ...)`,
`GameCharacterCalculations.cpp:118ff`, which returns `1.0f` when weather is
inactive.

`wGRADE` is the right-hand item's damage grade, `GET_GRADE(EMGRINDING_DAMAGE)`.
The modern item model has no grade field, so the `fGrade` term is **absent** and
only `nVAR` is added to the attacker's damage range. Recorded as a departure.

Everything after that — defence, critical, crushing, damage reduction,
reflection, and the low-SP penalties — is the existing physical pipeline, which
is exactly what a physical skill reuses. `ResolveCombat` is called with the
skill's range and the two characters' derived stats.

The elemental axis is real and separate: `emELEMENT` selects the resistance
lookup and is scaled by weather, and never changes what `fBASIC_VAR` means.
Not implemented; it belongs to VERTICAL-013.

---

## 9. Strike count

`SANIATTACK::m_wDivCount` (`legacy/Lib_Engine/Meshs/DxSkinAniControl.h:78`) is
assigned from `SANIMCONINFO::m_wStrikeCount` (`:90`) and loaded into the
character-class animation table (`GLogicData.cpp:98-100`). **It is not a skill
field.** There is no strike count in `SSKILLBASIC` or `CDATA_LVL`.

Every `CHECHSKILL` and `ACCOUNTSKILL` call site passes a literal `1`:
`GLCharSkillMsg.cpp:357`, `:997`, `GLChar.cpp:4797`, `:4816`,
`GLCharacterSkill.cpp:71`, `:361`. So although the code multiplies HP, MP, SP
and CP by `wStrikeNum` at `GLogixExPC.cpp:4240-4260` and `:4296-4302`, **the
multiplier is always 1 in practice**.

Damage is **divided**, not multiplied — `GLogixExPC.cpp:1784-1787`:

```cpp
if ( dwDivCount>1 )  {  rResultDAMAGE /= dwDivCount;  }
```

`SkillProc` runs once per division from `SkillProcess` (`GLChar.cpp:4749`,
`:4778`), each rolling its own hit through `CHECKHIT` in `PreStrikeProc`
(`GLChar.cpp:2400-2411`), and each deducting SP in `SkillProc`.

**Modern.** Single strike. The division, the per-strike hit roll and the
per-strike SP charge all depend on the animation division, which is client
animation data. Deferred as one item (§10) rather than simulated with a
multiplier, because a multiplier is not what legacy does.

---

## 10. Targeting and range

`emIMPACT_TAR` — `legacy/Lib_Client/G-Logic/GLCharDefine.h:859-868`:
`TAR_SELF=0`, `TAR_SPEC=1`, `TAR_SELF_TOSPEC=2` (pierce line), `TAR_ZONE=3`
(area), `TAR_SPECIFIC=4`.

`emIMPACT_SIDE` — `GLCharDefine.h:880-887`: `SIDE_OUR=0`, `SIDE_ENEMY=1`,
`SIDE_ANYBODY=2`. There is **no `SIDE_SELF`**; the friendly case is `SIDE_OUR`.

Range has two independent systems and they must not be collapsed:

- **Skill range** — `GETSKILLRANGE_TAR` (`GLogixExPC.cpp:4490`) and
  `GETSKILLRANGE_APPLY` (`:4522`), formulas at
  `GameCharacterCalculations.cpp:887-909` and `:933-952`. The target form floors
  the base at 20; the apply form does not. Both add `GETSUM_TARRANGE() + 5` for
  `EMAPPLY_PHY_LONG` and a spec bonus for `SIDE_ENEMY`.
- **Attack range** — `m_wATTRANGE` from the right-hand item's `wAttRange`
  (`GLogixExPC.cpp:1281-1282`), used only by basic attacks (`GLCharMsg.cpp:345`).
  There is no `GETSUM_ATTACKRANGE`; the two systems are disjoint.

Both skill-range functions need `m_fSUM_SKILL_ATTACKRANGE` /
`m_fSUM_SKILL_APPLYRANGE`, stat accumulators from passives that the modern
`PassiveContribution` does not carry. The range *call sites* additionally need
`pTARGET->GetBodyRadius()` and a distance, i.e. positions.

**Modern.** `TAR_SPEC` with a target the server already holds, and `TAR_SELF`
is a no-op for a damaging skill. `TAR_SELF_TOSPEC`, `TAR_ZONE` and
`TAR_SPECIFIC` are refused with `UnsupportedTarget`. No distance is checked and
no coordinate system is invented.

---

## 11. Effects

`SKILL::SAPPLY` also carries `emSTATE_BLOW` / `sSTATE_BLOW[]` (`GLSkillApply.h:583-584`),
`sImpacts[5]` (`:586`) and `sSpecs[5]` (`:587`). Applying them needs a status
system the modern server does not have. The active branch that exists here is
resource change; everything else is deferred (§13).

Classification used while scoping this milestone:

| Category | Contents | Outcome |
| --- | --- | --- |
| A — required for basic active combat | role/level/resource validation, SP cost, cooldown, physical damage | implemented |
| B — compatible with existing systems | equipment required-SP contribution | implemented, reuses VERTICAL-010 |
| C — needs world/resource/status | zone, realm, angle, range, status blows, specs, party costs | deferred |
| D — needs magic/elemental | `EMAPPLY_MAGIC`, `emELEMENT` resistance, weather | deferred, VERTICAL-013 |
| E — needs animation data | strike count, per-division hit and charge, `fAttVelo`, `emMType` | deferred |

---

## 12. Implemented

| Behaviour | Legacy source | Modern |
| --- | --- | --- |
| definition lookup | `GLogixExPC.cpp:4085` | `ActiveSkillResolver` step 1 |
| learned check | `:4074-4076` | server, from `SkillState` |
| level resolution | `:4075` | server, `GetSkillLevel`, caller value ignored |
| level bound | `GLSkillDefine.h:16` | `kMaxSkillLevel` |
| role gate | `:4092` | `SkillRole::Normal` |
| prohibition / stun | `:4058`, `:4062` | `skillProhibited`, `stunned` |
| cooldown gate | `:4082` | `onCooldown` |
| HP gate `<=` | `:4240` | `InsufficientHp` |
| MP gate `<` | `:4241` | `InsufficientMp` |
| SP cost | `:4254-4256` | `RequiredSP` |
| SP gate `<` | `:4258` | `EvaluateLowSp` |
| low-SP no charge | `GLChar.cpp:3005` | `spCost = 0` when low |
| HP/MP charge | `GLogixExPC.cpp:4296-4299` | `hpCost`, `mpCost` |
| cooldown value | `GameCharacterCalculations.cpp:118-126` | `CooldownSeconds` |
| cooldown tick | `GLogixExPC.cpp:3864-3879` | `AdvanceSkillCooldowns` |
| damage magnitude | `GLogixExPC.cpp:1524` | `basicDamage` |
| damage applied | `GLogixExPC.cpp:1527-1530` | range + `basicDamage` |
| rest of the damage pipeline | VERTICAL-006..009 | `Combat::ResolveCombat` |

---

## 13. Deferred

| Item | Reason |
| --- | --- |
| `EMAPPLY_PHY_LONG` damage | VERTICAL-012 |
| `EMAPPLY_MAGIC`, `emELEMENT`, weather scaling | VERTICAL-013 |
| `TAR_ZONE`, `TAR_SELF_TOSPEC`, `TAR_SPECIFIC` | need positions and an entity registry |
| `SIDE_OUR` / `SIDE_ANYBODY` buffs | need a status system |
| heal (`fBASIC_VAR >= 0`) | a resource path, not combat; refused as `UnsupportedEffect` |
| `EMFOR_MP` / `EMFOR_SP` skill drain | separate resist-reduced branch, `GLChar.cpp:3094-3123` |
| strike count, per-division hit and charge | client animation data |
| `wAPPLYRANGE`, `wAPPLYANGLE`, `wAPPLYNUM`, `wPIERCENUM`, `wTARNUM` | need positions |
| `wUSE_ARROWNUM` / `CHARMNUM` / `BULLETNUM` | need an inventory |
| `wUSE_CP`, `wUSE_EXP`, `*_PTY` | no combat-point, EXP-spend or party system |
| weapon-grade scaling of skill damage (`fGrade`) | modern items have no grade field |
| `m_wACCEPTP` in the SP charge | no modern field; VERTICAL-010, and the legacy gate ignores it too |
| `m_fSTATE_DELAY`, `NET_MSGDELAY` | debuff and transport terms |
| 0.3 s `m_fSkillDelay` message limiter | protocol, not a skill rule |
| immunity flags (`m_emImmuneApplyType`) | needs a damage-spec status model |
| life steal, EXP awards, `nGATHER_*` | need an EXP and spec system |

### Departures, stated plainly

1. **The SP charge omits `m_wACCEPTP`.** Legacy charges
   `m_wSUM_DisSP + wUSE_SP`; modern charges
   `ItemContribution::requiredSP + wUSE_SP`. One term smaller, and the two
   values then agree with each other where legacy's do not.
2. **The skill damage omits the `fGrade` term.**
   `gdDamage.dwLow += nVAR + (gdDamage.dwLow * fGrade)` becomes
   `range.low += nVAR`. `wGRADE` is a per-item damage grade the modern item
   model does not carry.
3. **A zero character level returns the bare delay** instead of dividing by
   zero. `CooldownSeconds` guards it; RAN crashes or yields an infinity.
4. **A zero-magnitude skill is refused** (`NoDamageMagnitude`). Legacy charges
   and deals nothing; refusing says the data is unusable.
5. **Low-SP casts are not rejected.** RAN's server-side first check rejects
   `EMSKILL_NOTSP` while its running-cast re-check tolerates it; the tolerated
   path is the one that produces the cast, and it is the one followed.

---

## 14. Tests

`modern/tests/ActiveSkillTests.cpp`, 34 cases, driving the resolver directly
with injected rolls. Covers the ten required cases plus the refusals, the
low-SP boundary and charge, the HP/MP operator asymmetry, the cooldown formula
and its zero-level guard, the weather scaling, and determinism.

`modern/server/ServerCharacterTests.cpp`, 12 cases, covering what only the
server can prove: that the level comes from the learned set and not the caller,
that a refusal is transactional, that a real cast damages a real target and
spends real resources, that the cooldown blocks and then expires, that an
unsupported skill is refused rather than faked, and that the required SP picks
up VERTICAL-010's equipment contribution.

Results: Core 259 → 293, Server 58 → 70, 14/14 CTest in Debug and Release, 0
errors and 0 warnings.
