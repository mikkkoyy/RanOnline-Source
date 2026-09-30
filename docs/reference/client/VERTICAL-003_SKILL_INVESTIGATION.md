# VERTICAL-003 Investigation Report — Skills + Passive Contribution

Read from `legacy/` before the implementation was finished, then re-verified
against the same files afterwards. Every claim below is traced to a file and
line. RAN terminology is kept; nothing is borrowed from another MMORPG. Where
the legacy code does something the modern core deliberately does not, the
difference is stated rather than smoothed over.

---

## 1. What a learned passive skill is

**Identity** is `SNATIVEID`, a `(wMainID, wSubID)` pair, and it is not a skill
number on its own — it is qualified by a class. The per-character record is
`SCHARSKILL` (`legacy/Lib_Client/G-Logic/GLCharData.h:233`):

```cpp
// legacy/Lib_Client/G-Logic/GLCharData.h:233
struct SCHARSKILL
{
	static DWORD VERSION;
	static DWORD SIZE;

	SNATIVEID	sNativeID;
	WORD		wLevel;

	SCHARSKILL () :
		sNativeID(SNATIVEID::ID_NULL,SNATIVEID::ID_NULL),   // 241
		wLevel(0)
	{
	}
}
```

`SCHARDATA2` holds the whole set in a map keyed by `DWORD`
(`GLCharData.h:889`):

```cpp
// legacy/Lib_Client/G-Logic/GLCharData.h:889
typedef std::map<DWORD,SCHARSKILL>		SKILL_MAP;
```

`m_ExpSkills` of that type is the field the character carries
(`GLCharData.h:895`).

Two consequences the modern design follows:

- **The learned set is ordered.** `std::map` iterates in key order, so
  `SUM_PASSIVE` visits skills in a fixed order for a given set. This is why
  `SkillState` is an ordered map and not a hash set, and why aggregation is
  deterministic by construction rather than by a sort at the end.
- **Level lives on the character, not the definition.** `SCHARSKILL::wLevel` is
  the level *this character* has, and it indexes the per-level tables. A
  definition holds data for every level; the character selects one row.

`wLevel` is a `WORD`, and `SNATIVEID::ID_NULL` is the legacy "no skill" value in
both halves — which is why `SkillId::IsValid()` tests for `0xFFFF` rather than
for zero (§7).

## 2. The aggregation function

`GLCHARLOGIC::SUM_PASSIVE`, `legacy/Lib_Client/G-Logic/GLogixExPC.cpp:863`. This
is the function VERTICAL-003 transcribes. Its shape, in order:

```cpp
// legacy/Lib_Client/G-Logic/GLogixExPC.cpp:863
void GLCHARLOGIC::SUM_PASSIVE ()
{
	m_sSUM_PASSIVE = SPASSIVE_SKILL_DATA();      // 865: reset to zero first

	if ( m_bVehicle ) return;                    // 867
	if ( m_ExpSkills.empty() ) return;           // 869

	for ( iter = m_ExpSkills.begin(); iter != iter_end; ++iter )  // 871-873
	{
		const SCHARSKILL &sCharSkill = (*iter).second;
		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData (
			sCharSkill.sNativeID.wMainID, sCharSkill.sNativeID.wSubID );  // 876
		if ( !pSkill )                             continue;   // 877
		if ( pSkill->m_sBASIC.emROLE != SKILL::EMROLE_PASSIVE ) continue;  // 878
		...
```

Four behaviours are visible in those eleven lines and all four are reproduced:

| Legacy line | Behaviour | Modern equivalent |
| --- | --- | --- |
| `:865` | The accumulator is reset, never accumulated across calls | `PassiveContribution contribution;` built fresh per call |
| `:867` | A character in a vehicle contributes no passives at all | **not modelled** — see §8 |
| `:877` | A learned skill with no definition is skipped silently | **deliberately different** — see §5 |
| `:878` | Only `EMROLE_PASSIVE` skills contribute | `SkillDefinition` is a passive definition; there is no role field to get wrong |

The `:877` case is the one that had to change. `continue` on a missing definition
means a character who has learned a skill the data no longer contains silently
loses its bonus. In a modern server that is indistinguishable from a data bug,
so the aggregator returns `MissingDefinition` and the server refuses the
recalculation instead.

The vehicle rule (`:867`) is a gameplay condition, not arithmetic, and it has no
modern equivalent yet because there is no vehicle system. It is recorded here so
it is not rediscovered as a mystery later.

## 3. The three passes over one skill

Once a skill has been selected, `SUM_PASSIVE` makes three passes over its data.
They are separate loops over separate tables, and each indexes by the *learned*
level.

### 3.1 Basic type — `sDATA_LVL`

```cpp
// legacy/Lib_Client/G-Logic/GLogixExPC.cpp:916
const SKILL::CDATA_LVL &sDATA_LVL = pSkill->m_sAPPLY.sDATA_LVL[sCharSkill.wLevel];

switch ( pSkill->m_sAPPLY.emBASIC_TYPE )   // 918
```

`CDATA_LVL::fBASIC_VAR` is a single float per level (`GLSkillApply.h:252`), and
`EMTYPES` (`GLSkillApply.h:19`) names what it means. The arms, and where each
one lands:

| `EMTYPES` | Line | Accumulates into | Modern field |
| --- | --- | --- | --- |
| `EMFOR_HP` | `:921` | `m_nHP` | `hp` |
| `EMFOR_MP` | `:925` | `m_nMP` | `mp` |
| `EMFOR_SP` | `:929` | `m_nSP` | `sp` |
| `EMFOR_VARHP` | `:933` | `m_fINCR_HP` | `hpRecoveryRate` |
| `EMFOR_VARMP` | `:937` | `m_fINCR_MP` | `mpRecoveryRate` |
| `EMFOR_VARSP` | `:941` | `m_fINCR_SP` | `spRecoveryRate` |
| `EMFOR_DEFENSE` | `:945` | `m_nDEFENSE` | `defense` |
| `EMFOR_HITRATE` | `:949` | `m_nHIT` | `hit` |
| `EMFOR_AVOIDRATE` | `:953` | `m_nAVOID` | `avoid` |
| `EMFOR_VARAP` | `:956-960` | **all three** `m_fINCR_*` | all three recovery rates |
| `EMFOR_VARDAMAGE` | `:963` | `m_nDAMAGE` | `damage` |
| `EMFOR_VARDEFENSE` | `:967` | `m_nDEFENSE` | `defense` |
| `EMFOR_PA` | `:971` | `m_nPA` | `meleePower` |
| `EMFOR_SA` | `:975` | `m_nSA` | `shootPower` |
| `EMFOR_MA` | `:979` | `m_nMA` | `magicAttack` |
| `EMFOR_HP_RATE` | `:983` | `m_fHP_RATE` | `hpRate` |
| `EMFOR_MP_RATE` | `:987` | `m_fMP_RATE` | `mpRate` |
| `EMFOR_SP_RATE` | `:991` | `m_fSP_RATE` | `spRate` |
| `EMFOR_RESIST` | `:995` | `m_sSUMRESIST` | `resistances` |

Two of these are worth calling out because they are the kind of thing a
transcription gets wrong by accident:

- **`EMFOR_VARAP` writes to three fields at once** (`:956-960`). It is not a
  separate "AP" bucket that something else distributes later; one basic type
  fans out into all three recovery rates.
- **`EMFOR_HITRATE` and `EMFOR_AVOIDRATE` land in `m_nHIT` and `m_nAVOID`**, the
  *flat* fields, not in the percentage fields. `SPASSIVE_SKILL_DATA` has no
  passive percentage fields at all, and the `int()` cast at `:949`/`:953`
  truncates before adding.

`EMFOR_CURE` (7) and the `EMFOR_PET_*` block (19 onwards) have no arm in
`SUM_PASSIVE`; they fall through the switch and contribute nothing. The
`EMFOR_SUMMONTIME` arm at `:999-1001` targets a member CORE-002 does not model.

### 3.2 Impacts — `sImpacts`

```cpp
// legacy/Lib_Client/G-Logic/GLogixExPC.cpp:1004-1008
for( int nImpact=0; nImpact<SKILL::MAX_IMPACT; ++nImpact )
{
	const float &fADDON = pSkill->m_sAPPLY.sImpacts[nImpact].fADDON_VAR[sCharSkill.wLevel];
	switch ( pSkill->m_sAPPLY.sImpacts[nImpact].emADDON )
```

`MAX_IMPACT = 5` (`GLSkillDefine.h:18`). Every arm **adds** — there is no
priority, no largest-wins rule, and no interaction between impacts:

| `EMIMPACTA_` | Line | Accumulates into | Modern field |
| --- | --- | --- | --- |
| `HITRATE` | `:1011` | `m_nHIT` | `hit` |
| `AVOIDRATE` | `:1014` | `m_nAVOID` | `avoid` |
| `DAMAGE` | `:1018` | `m_nDAMAGE` | `damage` |
| `DEFENSE` | `:1022` | `m_nDEFENSE` | `defense` |
| `VARHP` / `VARMP` / `VARSP` | `:1026`/`:1029`/`:1032` | `m_fINCR_*` | recovery rates |
| `VARAP` | `:1036-1038` | all three `m_fINCR_*` | all three recovery rates |
| `DAMAGE_RATE` | `:1042` | `m_fDAMAGE_RATE` | **not modelled** |
| `DEFENSE_RATE` | `:1046` | `m_fDEFENSE_RATE` | **not modelled** |
| `PA` / `SA` / `MA` | `:1050`/`:1054`/`:1058` | `m_nPA`/`m_nSA`/`m_nMA` | the three powers |
| `HP_RATE` / `MP_RATE` / `SP_RATE` | `:1062`/`:1066`/`:1070` | `m_f*_RATE` | the three rates |
| `RESIST` | `:1074` | `m_sSUMRESIST` | `resistances` |

### 3.3 Specs — `sSpecs`, and the one non-additive rule

```cpp
// legacy/Lib_Client/G-Logic/GLogixExPC.cpp:1082-1086
for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
{
	const SKILL::SSPEC &sSPEC = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[sCharSkill.wLevel];
	switch ( pSkill->m_sAPPLY.sSpecs[nSpec].emSPEC )
```

`MAX_SPEC = 5` (`GLSkillDefine.h:17`). **Almost** all of these add — but four do
not, and this is the single most important thing in the function:

```cpp
// legacy/Lib_Client/G-Logic/GLogixExPC.cpp:1109-1112
case EMSPECA_PSY_DAMAGE_REDUCE:
	if ( m_sSUM_PASSIVE.m_sDamageSpec.m_fPsyDamageReduce < sSPEC.fVAR1 )
		m_sSUM_PASSIVE.m_sDamageSpec.m_fPsyDamageReduce = sSPEC.fVAR1;
	break;
```

`PSY_DAMAGE_REDUCE` (`:1109`), `MAGIC_DAMAGE_REDUCE` (`:1114`),
`PSY_DAMAGE_REFLECTION` (`:1119`) and `MAGIC_DAMAGE_REFLECTION` (`:1127`) are
**maximum, not sum**. The other arms add. `EMSPECA_ATTACKVELO` (`:1102`) even
*subtracts*.

VERTICAL-003 does not model specs at all, so none of this is transcribed yet. It
is recorded because the day someone adds spec support, a straight `+=` will be
wrong for four of the arms, and the difference is not visible in any small
example.

## 4. Weapon gating

```cpp
// legacy/Lib_Client/G-Logic/GLogixExPC.cpp:882-914
bool bvalid_left(true), bvalid_right(true);
GLSKILL_ATT emSKILL_LEFT  = pSkill->m_sBASIC.emUSE_LITEM;
GLSKILL_ATT emSKILL_RIGHT = pSkill->m_sBASIC.emUSE_RITEM;

EMSLOT emRHand = GetCurRHand();
EMSLOT emLHand = GetCurLHand();

if ( emSKILL_LEFT != SKILLATT_NOCARE )
{
	bvalid_left = false;
	SITEM* pItem = GET_SLOT_ITEMDATA(emLHand);
	if ( pItem )
	{
		emITEM_LEFT = pItem->sSuitOp.emAttack;
		bvalid_left = CHECHSKILL_ITEM(emSKILL_LEFT, emITEM_LEFT, bHiddenWeapon);
	}
}
// ... same for the right hand ...

if ( !(bvalid_left && bvalid_right) )  continue;   // 914
```

`GLSKILL_ATT` is `GLSkillBasic.h:97`; `emUSE_LITEM`/`emUSE_RITEM` are
`GLSkillBasic.h:395-396`. `SKILLATT_NOCARE` means "no requirement" and is the
constructor default (`GLSkillBasic.h:454-455`).

`CHECHSKILL_ITEM` is an inline in `legacy/Lib_Client/G-Logic/GLogicEx.h:1274`:

```cpp
// legacy/Lib_Client/G-Logic/GLogicEx.h:1274
inline bool CHECHSKILL_ITEM ( GLSKILL_ATT emSKILL, GLITEM_ATT emITEM, const bool bHiddenWeapon )
{
	if ( emITEM == ITEMATT_FIST && bHiddenWeapon )  { return true; }

	switch ( emSKILL )
	{
	case SKILLATT_NOTHING:      return ( emITEM == ITEMATT_NOTHING );
	case SKILLATT_SWORD:        return ( emITEM == ITEMATT_SWORD );
	case SKILLATT_SWORDBLADE:   return ( emITEM == ITEMATT_SWORD || emITEM == ITEMATT_BLADE );
	...
```

So the legacy check is a **weapon-type** match against
`SITEM::sSuitOp.emAttack`, with a hidden-fist special case and two compound
cases (`SWORDBLADE`, `SHOTGUN`, `GUN`).

**LIMITED — what the modern core does instead.** `ItemDefinition` has no attack
type. VERTICAL-002 recorded this too, and it was not resolved here. What the
aggregator can check is whether the required slot holds *an item at all*, which
is the `if (pItem)` part of `:896-900` and `:907-911` and the `bvalid = false`
initialisation. So a sword-gated passive is active whenever the right hand is
occupied, whatever is in it.

The consequence to keep in mind: a *dagger* in the right hand will activate a
sword-gated passive. The gate is weaker than the legacy one in exactly one
direction, and it cannot become stricter without an item attack type. This is
asserted as far as the data allows in
`Server_WeaponDependentPassiveNeedsTheSlotOccupied`, and the test says so at the
point of assertion rather than leaving it to a reader to discover.

A failing gate is not an error — `:914` just `continue`s to the next skill. The
modern aggregator does the same: a gated-off skill contributes nothing and the
call still succeeds.

## 5. The accumulator

`SPASSIVE_SKILL_DATA`, `legacy/Lib_Client/G-Logic/GLCharData.h:1123`:

```cpp
// legacy/Lib_Client/G-Logic/GLCharData.h:1123
struct SPASSIVE_SKILL_DATA
{
	short	m_nHP;
	short	m_nMP;
	short	m_nSP;

	short	m_nDAMAGE;
	short	m_nDEFENSE;

	short	m_nHIT;
	short	m_nAVOID;

	short	m_nPIERCE;
	float	m_fTARRANGE;

	float	m_fMOVEVELO;
	float	m_fATTVELO;
	float	m_fSKILLDELAY;

	float	m_fINCR_HP;
	float	m_fINCR_MP;
	float	m_fINCR_SP;

	float	m_fDAMAGE_RATE;
	float	m_fDEFENSE_RATE;

	DAMAGE_SPEC m_sDamageSpec;

	short	m_nPA;
	short	m_nSA;
	short	m_nMA;

	float	m_fHP_RATE;
	float	m_fMP_RATE;
	float	m_fSP_RATE;

	SRESIST	m_sSUMRESIST;
	...
	int		m_nSummonTime;
```

**The integer fields are `short`.** The `DWORD(...)` cast at `:921` reads like an
unsigned accumulation but assigns into a `short`, so a passive total above 32767
wraps or goes negative in the legacy build before `SUM_ADDITION` ever sees it.
`Stats::PassiveContribution` uses `int32_t` and `float`. This is a deliberate
divergence: the modern type does not reproduce a 16-bit overflow, because a
stat total that wraps is a bug rather than a behaviour to preserve. It is
recorded here because it is a real numeric difference, not a cosmetic one.

`m_sSUMRESIST` is a `SRESIST` — five `short` elements, `nFire` / `nIce` / `nElectric` / `nPoison` / `nSpirit` (`GLCharDefine.h:678-684`).
The accumulation
at `:995` and `:1074` reads `m_sSUM_RESIST += (int)(fBASIC_VAR)`, which is worth
a second look, because a `SRESIST` has no single scalar to add to and the
element-wise `operator+=(const SRESIST&)` at `GLCharDefine.h:736` does not apply.
The overload that *does* apply is:

```cpp
// legacy/Lib_Client/G-Logic/GLCharDefine.h:765
SRESIST& operator += ( const int rvalue )
{
	nFire += rvalue;
	nIce += rvalue;

	nElectric += rvalue;
	nPoison += rvalue;

	nSpirit += rvalue;

	return *this;
}
```

So one `EMFOR_RESIST` value lands on **all five elements equally**, as a
truncated integer, not on a chosen element. The aggregator transcribes this
rather than inventing a per-element mapping, because a guess here would be
indistinguishable from the truth in any test that only checks one element.

## 6. The definition

`SAPPLY` (`GLSkillApply.h:573`) holds the three tables in one place:

| Field | Type | Line | Modern |
| --- | --- | --- | --- |
| `emBASIC_TYPE` | `SKILL::EMTYPES` | `:578` | `PassiveApplyType` |
| `sDATA_LVL` | `CDATA_LVL[]` per level | — | `levelData[].basicVar` |
| `sImpacts` | `SIMPACTS[]`, `MAX_IMPACT` | — | `impacts[5]` |
| `sSpecs` | `MAX_SPEC` | — | `levelData[].specs[5]` |

`SBASIC` holds the identity and the weapon requirements: `emROLE`
(`GLSkillBasic.h:477`), `emUSE_LITEM`/`emUSE_RITEM` (`:487-488`), and
`m_sLEARN` (which carries `bHiddenWeapon`, read at `GLogixExPC.cpp:883`).

The constants are in `legacy/Lib_Client/G-Logic/GLSkillDefine.h:16-18`:

```cpp
// legacy/Lib_Client/G-Logic/GLSkillDefine.h:16-18
	MAX_LEVEL		= 9,
	MAX_SPEC		= 5,
	MAX_IMPACT		= 5,
```

These are the `kMaxSkillLevel` / `kMaxSkillSpecs` / `kMaxSkillImpacts` constants,
pinned by `SkillId_ConstantsMatchRan`. `MAX_LEVEL = 9` is why `SkillState`
refuses a level of 0 and anything above 9, and why a `uint8_t` is sufficient
where the legacy code uses a `WORD`.

## 7. Why `SkillId::IsValid()` tests 0xFFFF

`SCHARSKILL`'s constructor uses `SNATIVEID::ID_NULL` for both halves
(`GLCharData.h:238`), so *that* is the legacy "no skill" value — not zero. The
modern `SkillId` follows it:

```cpp
// modern/core/skills/SkillDefinition.h:44
constexpr bool IsValid() const noexcept
{
    return classIndex != 0xFFFF && skillIndex != 0xFFFF;
}
```

The consequence is that a default-constructed `SkillId{}` is `{0, 0}` and is
**valid** — it names class 0, skill 0, which is a real id. Unlike `ItemId`,
`SkillId` has no `MakeInvalid()`, so `0xFFFF` is the only way to spell an
invalid one.

This is a trap and it is worth stating plainly: a zero-initialised `SkillId` is
learnable at the `SkillState` level. What actually keeps that safe is
`ServerCharacter::LearnSkill` resolving the id against the definition provider
*before* touching any state (`ServerCharacter.cpp:201`), so a zeroed id finds no
definition and is refused as `NotFound`. The value test in `SkillState` is the
weaker of the two defences, not the only one. Pinned by
`SkillId_ValidityIsASentinelNotAZero`.

## 8. Deferred and not-modelled

| Classification | Items | Why |
| --- | --- | --- |
| **CONFIRMED — implemented** | every `EMTYPES` basic arm in §3.1; every `EMIMPACTA_*` arm in §3.2 except the two rate arms; the two weapon-slot requirements; `MAX_LEVEL` / `MAX_IMPACT` / `MAX_SPEC`; the `EMROLE_PASSIVE` filter; the `SRESIST` all-five-elements rule | each traced to a line above |
| **DELIBERATELY DIFFERENT** | a learned skill with no definition | legacy `continue`s silently (`:877`); modern refuses the recalculation, so a data fault is visible |
| **LIMITED** | weapon-type matching | `ItemDefinition` has no attack type; slot occupancy only (§4) |
| **CARRIED, NO EFFECT** | `EMIMPACTA_DAMAGE_RATE`, `EMIMPACTA_DEFENSE_RATE` | the enum values exist so a definition parses, but `PassiveContribution` has no destination and the aggregator ignores them (`PassiveContributionAggregator.cpp:181-187`) |
| **DEFERRED — not in the stat pipeline** | `EMSPECA_*` (all arms, including the four max-not-sum ones) | needs a spec concept; §3.3 |
| **NOT MODELLED** | `EMFOR_CURE`, `EMFOR_PET_*`, `EMFOR_SUMMONTIME`, `m_nPIERCE`, `m_fTARRANGE`, `m_fMOVEVELO`, `m_fATTVELO`, `m_fSKILLDELAY`, `m_fDAMAGE_RATE`, `m_fDEFENSE_RATE`, `DAMAGE_SPEC` | CORE-002 has no destination for them; adding one would be a stat-system change, not a skill change |
| **DEFERRED — outside a passive** | `SLEARN` (`GLSkillLearn.h:75`) and its `SLEARN_LVL` prerequisite tables | the prerequisite and SP-cost system; `SLEARN` is a learning rule, not a stat contribution |
| **NOT A STAT CONCERN** | `bHiddenWeapon`, `GetCurRHand()` / `GetCurLHand()` resolution | the hidden-fist special case needs a weapon *state* this layer does not have |
| **NOT MODELLED — no modern equivalent** | the vehicle exclusion (`:867`) | no vehicle system exists yet |
| **DIVERGENT, INTENTIONALLY** | `short` accumulators | §5 |

## 9. Modern architecture mapping

```
SkillDefinition            identity, name, maxLevel, per-level basic values,
                           5 impacts, 5 specs, two weapon requirements  [new, core]
SkillState                 SkillId -> level, ordered, a value type      [new, core]
SkillDefinitionProvider    read-only Find + InMemorySkillDefinitions    [new, core]
PassiveContributionAggregator  SkillState + provider + EquipmentState
                           -> Stats::PassiveContribution                [new, core]
ServerCharacter            + SkillState; owns it; recalculates on
                           every change                                 [extended]
CharacterSnapshot          + SkillList (id, level, name)               [extended]
ClientCharacterState       + read-only skill views                     [extended]
```

Nothing restates a stat formula: the aggregator produces a
`Stats::PassiveContribution`, and `Stats::Calculate` remains the only place RAN's
arithmetic lives. `ServerCharacter::SetContributions` refuses a non-zero passive
contribution, so the learned set cannot be bypassed by a second source.

## 10. Where the tests are

`modern/tests/SkillTests.cpp` covers the transcription directly, in the core
library, with no server and no client:

- each `PassiveApplyType` and `PassiveImpactType` value landing in the
  contribution field the legacy switch arm names
- the value read being the one for the **learned** level, for both the basic
  value and each impact
- additive stacking, and that learning order does not change the result
- `MissingDefinition` and non-finite definitions being refusals, not zeros
- the weapon-slot gate, and the two hands being independent
- a learned level above the definition's own `maxLevel` being skipped

The server-side consequences are in `modern/server/ServerCharacterTests.cpp`
and the client's read-only behaviour in
`modern/client/gameplay/ClientGameplayTests.cpp`. See §13.6 of
`docs/MODERN_ARCHITECTURE.md`.
