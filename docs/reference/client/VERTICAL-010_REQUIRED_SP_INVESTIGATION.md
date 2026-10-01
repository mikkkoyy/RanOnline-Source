# VERTICAL-010 — Required-SP / Item Integration: Legacy Investigation

Scope: the equipment half of RAN's required-SP calculation, which VERTICAL-009
established the *shape* of but could not feed. VERTICAL-009 implemented the
comparison `currentSP < requiredSP` and noted that the required value had no
field to live in. This document establishes what that field is, where it comes
from, and what it is not.

Every legacy claim below is quoted from a file and line in `legacy/`. Where the
source does not prove something, the entry says so and nothing was invented.

---

## 1. `wReqSP` — definition

`legacy/Lib_Client/G-Logic/GLItemSuit.h`, inside `namespace ITEM`. The live
struct is `SSUIT` (`GLItemSuit.h:446`); there are ten versioned predecessors
(`SSUIT_100` … `SSUIT_110`) and every one of them carries the field, so it has
been part of the item format since at least version 0x0112.

`GLItemSuit.h:446-455` (struct head) and the field:

```cpp
struct SSUIT
{
    enum { ADDON_SIZE = 5, VARIATION_SIZE = 5, VOLUME_SIZE = 4, VERSION = 0x0112 };

    EMSUIT       emSuit;      // suit type
    DWORD        dwHAND;
    EMITEM_HAND  emHand;
    GLDWDATA     gdDamage;
    ...
    WORD         wAttRange;   // attack range
    WORD         wReqSP;      // required SP
```

Default, `GLItemSuit.h:487`:

```cpp
, wReqSP(0)
```

### Answering the required questions

| Question | Answer | Evidence |
| --- | --- | --- |
| Data type | `WORD` (16-bit unsigned) | `GLItemSuit.h:466` and every versioned struct |
| Max value | 65535 | implied by `WORD` |
| Is zero valid | Yes, and it is the default | `GLItemSuit.h:487` |
| Every item, or equipment only? | Structurally every `SITEM` has one; only equipment is ever *read* | see §3 |
| Weapons only? | No — any worn hand item | the read is by slot, not by kind (§3) |
| Right/left hand only? | **Yes, only those two slots** | `GLogixExPC.cpp:433-434`, `:3493-3494`, `:4255-4256` |
| Already a modern stat? | No | no required-SP field existed in `modern/` before this milestone |
| Static or runtime? | **Static definition data** | read via `SITEM::sSuitOp` |
| Modifiable by refine/buffs? | Yes, but not on this path | see below |
| `m_wACCEPTP` equivalent | No | §4 |

### Static, with one caveat

`SITEMCUSTOM::GETREQ_SP()` — `legacy/Lib_Client/G-Logic/GLItem.cpp:2911-2927`:

```cpp
WORD SITEMCUSTOM::GETREQ_SP () const
{
    SITEM *pITEM = GLItemMan::GetInstance().GetItem(sNativeID);
    if ( !pITEM )	return 0;
    ITEM::SSUIT &sSUIT = pITEM->sSuitOp;

    WORD wREQSP = sSUIT.wReqSP;

    float fVALUE = GETOptVALUE(EMR_OPT_DIS_SP);
    if ( fVALUE!=0.0f )
    {
        if ( fVALUE+wREQSP>0.0f )	wREQSP = WORD(wREQSP+fVALUE);
        else						wREQSP = 0;
    }

    return wREQSP;
}
```

So a per-copy refine option (`EMR_OPT_DIS_SP`) can shift the value. **`SUM_ITEM`
does not call this accessor.** It reads the raw static field:

`legacy/Lib_Client/G-Logic/GLogixExPC.cpp:430-434`

```cpp
SITEM* pRHAND = GET_SLOT_ITEMDATA ( emRHand );
SITEM* pLHAND = GET_SLOT_ITEMDATA ( emLHand );

if ( pRHAND )	m_wSUM_DisSP += pRHAND->sSuitOp.wReqSP;
if ( pLHAND )	m_wSUM_DisSP += pLHAND->sSuitOp.wReqSP;
```

`GET_SLOT_ITEMDATA` yields `SITEM*` (the shared definition), not the
`SITEMCUSTOM` instance. The modern `ItemStatBlock::requiredSP` therefore models
the static value only, matching the path actually used. The refine modifier
needs per-copy stat data that no modern system produces yet.

### Persistence

`GLItemSuit.cpp:411` writes the header `"wReqSP"`, `:464` writes the value, and
`:516` loads it: `wReqSP = (WORD)atoi( StrArray[ iCsvCur++ ] );`. It is ordinary
item table data, loaded with the item.

---

## 2. `m_wSUM_DisSP` — the aggregate

Declared `legacy/Lib_Client/G-Logic/GLogicEx.h:402`:

```cpp
WORD				m_wSUM_DisSP;					//	??? SP ???.
```

Zero-initialised at `GLogicEx.h:536`, reset at `GLogixExPC.cpp:105`, and computed
in one place, `GLogixExPC.cpp:421-434`:

```cpp
//	Note : ?????? ??g?? ???. ( STATS, LEVEL )
//
m_wACCEPTP = 0;
m_wACCEPTP += CALC_ACCEPTP ( GET_SLOT_NID(emLHand) );
m_wACCEPTP += CALC_ACCEPTP ( GET_SLOT_NID(emRHand) );

//	Note : "SP???? ????? ????g" + "?????? ???? ??? SP ???" 
//
m_wSUM_DisSP = m_wACCEPTP;

SITEM* pRHAND = GET_SLOT_ITEMDATA ( emRHand );
SITEM* pLHAND = GET_SLOT_ITEMDATA ( emLHand );

if ( pRHAND )	m_wSUM_DisSP += pRHAND->sSuitOp.wReqSP;
if ( pLHAND )	m_wSUM_DisSP += pLHAND->sSuitOp.wReqSP;
```

So, exactly:

```
m_wSUM_DisSP = m_wACCEPTP + wReqSP(emRHand) + wReqSP(emLHand)
```

`m_wACCEPTP` is itself derived from the two hand slots only, via
`CALC_ACCEPTP` — see §4.

`WORD` arithmetic, so the sum wraps at 16 bits.

---

## 3. Right-hand and left-hand contribution

Three sites, all identical in shape, and **all three name only the hands**:

| Purpose | Location | Code |
| --- | --- | --- |
| Aggregate (`SUM_ITEM`) | `GLogixExPC.cpp:430-434` | `GET_SLOT_ITEMDATA(emRHand)` / `(emLHand)` |
| Basic-attack gate | `GLogixExPC.cpp:3492-3497` | same |
| Skill gate | `GLogixExPC.cpp:4254-4256` | same |

```cpp
//	GLogixExPC.cpp:3492-3497
WORD wDisSP = GLCONST_CHAR::wBASIC_DIS_SP;
if ( pRHAND )	wDisSP += pRHAND->sSuitOp.wReqSP;
if ( pLHAND )	wDisSP += pLHAND->sSuitOp.wReqSP;

//	SP ????. 
if ( m_sSP.dwNow < (wDisSP*wStrikeNum) )	return EMBEGINA_SP;
```

A search for `wReqSP` across `legacy/Lib_Client/G-Logic/` returns 20 hits. Every
consumer outside the accessor and the serialiser is one of the six lines above.
**No other slot is ever consulted.** A head, upper, accessory or ornament
carrying a `wReqSP` contributes nothing to the SP cost. This is the single most
important thing to get right: a naive implementation summing all 21 equipped
slots would not reproduce RAN.

---

## 4. `m_wACCEPTP` — deferred, and why it costs nothing here

`legacy/Lib_Client/G-Logic/GLogicEx.h:401`:

```cpp
WORD				m_wACCEPTP;						//	???? ??????? ??????? ??? ??g.
```

Computed by `GLCHARLOGIC::CALC_ACCEPTP`, `GLogixExPC.cpp:3443-3461`:

```cpp
WORD GLCHARLOGIC::CALC_ACCEPTP ( SNATIVEID sNativeID )
{
	WORD wATP = 0;
	if ( sNativeID==NATIVEID_NULL() )							return wATP;
	SITEM* pITEM = GLItemMan::GetInstance().GetItem ( sNativeID );
	if ( pITEM==NULL )											return wATP;

	if ( m_sSUMSTATS.wPow < pITEM->sBasicOp.sReqStats.wPow )		wATP += pITEM->sBasicOp.sReqStats.wPow - m_sSUMSTATS.wPow;
	if ( m_sSUMSTATS.wStr < pITEM->sBasicOp.sReqStats.wStr )		wATP += pITEM->sBasicOp.sReqStats.wStr - m_sSUMSTATS.wStr;
	if ( m_sSUMSTATS.wSpi < pITEM->sBasicOp.sReqStats.wSpi )		wATP += pITEM->sBasicOp.sReqStats.wSpi - m_sSUMSTATS.wSpi;
	if ( m_sSUMSTATS.wDex < pITEM->sBasicOp.sReqStats.wDex )		wATP += pITEM->sBasicOp.sReqStats.wDex - m_sSUMSTATS.wDex;
	if ( m_sSUMSTATS.wInt < pITEM->sBasicOp.sReqStats.wInt )		wATP += pITEM->sBasicOp.sReqStats.wInt - m_sSUMSTATS.wInt;
	if ( m_sSUMSTATS.wSta < pITEM->sBasicOp.sReqStats.wSta )		wATP += pITEM->sBasicOp.sReqStats.wSta - m_sSUMSTATS.wSta;

	if ( m_wLevel < pITEM->sBasicOp.wReqLevelDW )					wATP += pITEM->sBasicOp.wReqLevelDW - m_wLevel;

	return wATP;
}
```

It is a **stat-deficit penalty**: for each hand item, how far the character is
below that item's stat requirement, plus a level shortfall. It is a real part of
`m_wSUM_DisSP`.

**Why it is deferred.** It needs three things the modern model does not have:
per-item `sReqStats` (required pow/str/spi/dex/int/sta), per-item
`wReqLevelDW`, and the character's summed stat vector to compare against. Modern
`ItemStatBlock` has no required-stats or required-level field, and there is no
per-item requirement table. Inventing a value would be fabricating a penalty.

**Why deferring it costs nothing for this milestone.** `m_wACCEPTP` is *not*
part of the low-SP gate. Look at `GLogixExPC.cpp:3492-3494` in §3: the gate
rebuilds `wDisSP` from `wBASIC_DIS_SP` and the two hands, and never reads
`m_wSUM_DisSP` or `m_wACCEPTP`. `m_wSUM_DisSP` — the field that *does* include
`m_wACCEPTP` — is consumed only by the SP **deduction** in `GLChar.cpp`:

```cpp
//	GLChar.cpp:2431 (AvoidProc) and :2506 (PreStrikeProc)
WORD wDisSP = m_wSUM_DisSP + GLCONST_CHAR::wBASIC_DIS_SP;
m_sSP.DECREASE ( wDisSP );
```

So the legacy is internally asymmetric: the **gate** excludes `m_wACCEPTP`, the
**deduction** includes it. Since VERTICAL-010 implements the gate and modern
models no SP deduction at all, excluding `m_wACCEPTP` here is faithful, not a
shortcut. It must be picked up by whichever milestone implements SP deduction,
and the contribution field is the right home for it then.

---

## 5. Basic SP cost

`legacy/Lib_Client/G-Logic/GLogicData.cpp:264`:

```cpp
WORD		wBASIC_DIS_SP		= 1;			//	?? ????? ??? SP ??.
```

Declared `GLogicData.h:424`. Already carried by VERTICAL-009 as
`CombatConstants::basicDisSP`; this milestone reuses that constant and does not
duplicate it.

Note the gate's own arithmetic does **not** go through `m_wSUM_DisSP`:

```
gate:        wDisSP = wBASIC_DIS_SP + wReqSP(emRHand) + wReqSP(emLHand)
deduction:   wDisSP = m_wSUM_DisSP + wBASIC_DIS_SP
             m_wSUM_DisSP = m_wACCEPTP + wReqSP(emRHand) + wReqSP(emLHand)
```

For the hand terms the two agree. They differ only in `m_wACCEPTP`.

### `wStrikeNum`

The gate multiplies by the strike count: `m_sSP.dwNow < (wDisSP*wStrikeNum)`.
Modern combat resolves a single strike, so the multiplier is 1. A combo count
belongs with the attack-sequence system and is out of scope here.

---

## 6. Skill SP cost

`legacy/Lib_Client/G-Logic/GLogixExPC.cpp:4254-4258`:

```cpp
WORD wDisSP = sSKILL_DATA.wUSE_SP;
if ( pRHAND )	wDisSP += pRHAND->sSuitOp.wReqSP;
if ( pLHAND )	wDisSP += pLHAND->sSuitOp.wReqSP;

if ( m_sSP.dwNow < wDisSP*wStrikeNum )									return EMSKILL_NOTSP;
```

and `GLChar.cpp:2454`, `:3007` for the deduction, against
`m_wSUM_DisSP + sSKILL_DATA.wUSE_SP`.

The shape is identical to the basic path with `wBASIC_DIS_SP` replaced by the
skill's own `wUSE_SP`. So a future skill milestone needs no new plumbing:

```
skill required SP = ItemContribution::requiredSP + SkillDefinition::useSP
```

The contribution field added here is the reusable half. VERTICAL-010 does not
implement skill SP or active skill combat.

---

## 7. Low SP is a property of the *attacker*

This is not in the VERTICAL-009 document and it corrects a real defect in
VERTICAL-009's implementation.

`GLCharMsg.cpp:604-612` is the attacker's own message handler:

```cpp
EMBEGINATTACK_FB emBeginFB = BEGIN_ATTACK(wStrikeNum);
if ( emBeginFB!=EMBEGINA_OK && emBeginFB!=EMBEGINA_SP )		return E_FAIL;

CheckInstanceItem ( );

BOOL bLowSP = (emBeginFB==EMBEGINA_SP) ? TRUE: FALSE;
PreStrikeProc ( FALSE, bLowSP );
```

`BEGIN_ATTACK` (`GLogixExPC.cpp:3488`) is a method on the *attacker's* logic
class and reads `m_sSP.dwNow` — the attacker's own SP pool. The resulting
`bLowSP` is handed to that same character's `PreStrikeProc`, which applies
`fLOWSP_DAMAGE` to the damage it is about to deal (`GLChar.cpp:2488-2491`).

The victim is never consulted. `bLowSP` means "this attacker cannot pay", and it
degrades the attacker's own output.

VERTICAL-009 fed `CombatInput::targetLowSP` from `target.m_currentSp`. That
cannot produce the documented behaviour for any real required-SP value: an
attacker needing 31 SP would never be low-SP against a target holding 100 SP.
VERTICAL-010 corrects it. The rules are now in `Combat::ResolveCombat`, computed
from the attacker's own pool, and `CombatInput` no longer carries a target SP
field at all — which is the structural form of the legacy rule.

---

## 8. Modern mapping

| Legacy | Modern | Location |
| --- | --- | --- |
| `ITEM::SSUIT::wReqSP` (`WORD`) | `ItemStatBlock::requiredSP` (`uint16_t`) | `modern/core/item/ItemDefinition.h` |
| `SUM_ITEM` hand sum | `ItemContribution::requiredSP` | `modern/core/stats/Contributions.h` |
| the two hand guards | `ItemContributionAggregator` slot test | `modern/core/equipment/ItemContributionAggregator.cpp` |
| `wBASIC_DIS_SP` | `CombatConstants::basicDisSP` | unchanged from VERTICAL-009 |
| `wDisSP` at the gate | `CombatInput::attackerRequiredSP` | `modern/core/combat/CombatCalculator.h` |
| `m_sSP.dwNow` (attacker) | `CombatInput::attackerCurrentSP` | same |
| `EMBEGINA_SP` | `ResolveCombat`'s `lowSP` local | same |
| `bLowSP` into hit/damage | `HitInput::lowSP`, `PhysicalDamageInput::lowSP` | unchanged |
| `ServerCharacter` supply | `m_items.requiredSP + basicDisSP` | `modern/server/character/ServerCharacter.cpp` |

`uint16_t` was chosen after verifying the legacy type is `WORD`, and the
accumulation wraps at 16 bits because legacy's `WORD` sum does.

### Why the field went in `ItemStatBlock` and not somewhere else

`ItemStatBlock` is the modern equivalent of the per-item stat payload that
`SUM_ITEM` reads, and `ItemContribution` is the equivalent of `SSUM_ITEM`. The
value therefore follows the established path: definition → stat block →
aggregation → contribution → caller. No field was added to
`ItemDefinition`, `DerivedStats` or `CombatInput` beyond the SP quantities the
combat boundary genuinely needs.

`ItemStatBlock::IsZero()` had to learn the field. The aggregator skips any block
it considers empty (`ItemContributionAggregator.cpp:47`) *before* the hand-slot
test, so without this a weapon whose only stat is a required-SP cost would be
skipped and never contribute. That is a real dependency, not a drive-by fix, and
it is pinned by `RequiredSP_OnlyStatStillAggregates`.

`CombatInput::attackerRequiredSP` already existed from VERTICAL-009 and keeps its
name and its role; it now receives the real value. No parallel field was added.

---

## 9. Tests

Core, `modern/tests/CombatTests.cpp` — the required-SP arithmetic and the low-SP
boundary, all with injected rolls:

| Case | Asserts |
| --- | --- |
| `RequiredSPMatrix_NoHandsRequiredSPIsOne` | 0 + 1 = 1 |
| `RequiredSPMatrix_RightHandOnly` | 20 + 1 = 21 |
| `RequiredSPMatrix_LeftHandOnly` | 10 + 1 = 11 |
| `RequiredSPMatrix_BothHands` | 30 + 1 = 31 |
| `RequiredSPMatrix_ExactBoundaryIsNotLowSP` | 31 / 31 is not low SP |
| `RequiredSPMatrix_OneBelowBoundaryIsLowSP` | 30 / 31 is low SP, damage halved |
| `RequiredSPMatrix_HighSPIsNotLowSP` | above the requirement is not low SP |
| `RequiredSPMatrix_ZeroSPIsLowSP` | 0 SP with a requirement is low SP |
| `RequiredSPMatrix_ZeroRequirementIsNeverLowSP` | required 0 is never exceeded |
| `RequiredSPMatrix_LowSPFollowsTheAttackerNotTheTarget` | the attacker's pool decides |
| `RequiredSPMatrix_LowSPFormulasUnchanged` | VERTICAL-009 regression: 0.50 damage, 0.75 hit |
| `RequiredSPMatrix_EquipmentTermChangesTheOutcome` | the equipment term really changes the result |

The `MakeSPInput` fixture uses `targetDefense = 9` on purpose. Rolled damage is
15, so this leaves 6 and `6 * 0.5f` is exactly 3. With the usual 10 the
pre-halving value is 5, `5 * 0.5f` truncates to 2, and every "is exactly half"
assertion would be testing integer rounding rather than the rule.

Core, `modern/tests/EquipmentTests.cpp` — the slot rule:

`RequiredSP_NoHandsContributesZero`, `RequiredSP_RightHandOnly`,
`RequiredSP_LeftHandOnly`, `RequiredSP_BothHandsSum`,
`RequiredSP_NonHandSlotsDoNotContribute` (nine non-hand slots equipped, all
ignored, still counted as contributing slots),
`RequiredSP_HandPlusOtherSlotsStillOnlyHands`, `RequiredSP_OnlyStatStillAggregates`,
`RequiredSP_NonEquipmentSlotIgnored`, `RequiredSP_SumsWithWordWrap`,
`RequiredSP_UnequipRemovesCost`, `RequiredSP_DoesNotDisturbOtherStats`.

Server, `modern/server/ServerCharacterTests.cpp` — the plumbing survives a real
character: `ServerRequiredSP_EquippedWeaponContributes`,
`ServerRequiredSP_BothHandsSum`, `ServerRequiredSP_ArmourDoesNotContribute`
(armour declares 99 and contributes 0), `ServerRequiredSP_BareHandsContributeZero`,
`ServerRequiredSP_UnequipDropsTheCost`, `ServerRequiredSP_LowSPAttackerStillAttacks`.

Results: Core 236 → 259, Server 52 → 58, 14/14 CTest in Debug and Release.

---

## 10. Remaining limitations

| Item | Status | Reason |
| --- | --- | --- |
| `m_wACCEPTP` | deferred | needs `sReqStats` and `wReqLevelDW`; **excluded from the legacy gate too**, so it does not affect low-SP determination. Belongs to the SP-deduction milestone. |
| `EMR_OPT_DIS_SP` refine option | deferred | per-copy stat data; `SUM_ITEM` does not use `GETREQ_SP()` either |
| SP consumption | deferred | no modern system deducts SP; `GLChar.cpp:2429`/`:2504` guard the deduction with `if (!bLowSP)` |
| Skill SP (`wUSE_SP`) | deferred | VERTICAL-011; `ItemContribution::requiredSP` is the reusable half |
| `wStrikeNum` | deferred | single-strike combat; a combo count belongs with the attack-sequence system |
| Snapshot visibility | not added | `requiredSP` is not published to the client; not a client-facing value in RAN |

## 11. Adjacent defect found, not fixed

`ItemStatBlock::IsZero()` (`modern/core/item/ItemDefinition.cpp`) also omits
`hpRecoveryFlat`, `mpRecoveryFlat` and `spRecoveryFlat`, which VERTICAL-005 added
to the struct. An item whose only stats are flat recovery is therefore treated
as contributing nothing and skipped by the aggregator. `operator==` does list
all three, so the predicates disagree with each other.

This is the same class of bug as the `requiredSP` omission fixed in this
milestone, but it is not a required-SP dependency and fixing it would change
behaviour for existing equipment tests. Left alone deliberately and reported
here rather than fixed quietly.
