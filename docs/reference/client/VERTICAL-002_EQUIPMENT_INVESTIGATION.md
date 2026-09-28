# VERTICAL-002 Investigation Report — Character Equipment + Item Contribution

Produced before implementation, from `legacy/`. Every claim below is traced to a
file and line. External material (§21) is recorded as corroboration only and
never overrides the local source. RAN-specific terminology is kept; nothing is
borrowed from another MMORPG.

---

## 1. Legacy item definition found

**`SITEM` → `ITEM::SSUIT`** (`legacy/Lib_Client/G-Logic/GLItem.h`, suite
`GLItemSuit.h:87` onwards). The stat-bearing part of a definition is `SSUIT`:

| Field | Type | Line | Feeds |
| --- | --- | --- | --- |
| `emAttack` | `GLITEM_ATT_102` | `GLItemSuit.h:101` | attack type, not a number |
| `nHitRate` | `short` | `:104` | `nHitRate` (percent) |
| `nAvoidRate` | `short` | `:105` | `nAvoidRate` (percent) |
| `gdDamage` | `GLPADATA` | `:107` | `gdDamage` (low/high range) |
| `nDefense` | `short` | `:108` | `nDefense` |
| `sResist` | `SRESIST` | `:110` | `sResist` (five elements) |
| `sADDON[5]` | `SADDON` | `:115` | typed integer add-ons |
| `sVARIATE[5]` | `SVAR` | `:116` | typed float variations |
| `sVOLUME` | `sVOLUME[4]` | (`:126` region) | typed float volumes |

`ADDON_SIZE = 5`, `VARIATION_SIZE = 5`, `VOLUME_SIZE = 4`
(`GLItemSuit.h:124-126`).

The typed add-on enum is `EMADD_` (`GLItemDef.h:508-535`), 21 values: `HITRATE`,
`AVOIDRATE`, `DAMAGE`, `DEFENSE`, `HP`, `MP`, `SP`, `STATS_POW/STR/SPI/DEX/INT/STA`,
`PA`, `SA`, `MA`, `POWINTDEX`, `STMVIT`, `PASA`, `PASAMA`.

## 2. Legacy item instance found

**`SITEMCUSTOM`**, the per-copy customisation carried by a slot, a stack entry
or a character record. This is where RAN mixes a definition's base values with
the options rolled onto one physical copy. Proven by the accessors themselves:

```cpp
// legacy/Lib_Client/G-Logic/GLItem.cpp:2821  SITEMCUSTOM::GETHITRATE()
SITEM *pITEM = GLItemMan::GetInstance().GetItem(sNativeID);
ITEM::SSUIT &sSUIT = pITEM->sSuitOp;
short nHITRATE = sSUIT.nHitRate;              // definition
float dHITRATE = GETOptVALUE(EMR_OPT_HITRATE); // instance option
float fRATE    = GETOptVALUE(EMR_OPT_HIT_VOL);
...
```

`GETDAMAGE` (`GLItem.cpp:2627`) has the same shape: it starts from
`sSUIT.gdDamage` (definition) and then applies `GETOptVALUE(EMR_OPT_DAMAGE)` and
`GETOptVALUE(EMR_OPT_ATTACK_VOL)`.

**So the split the modern core already draws is the correct one:** a definition
carries the base stat block, an instance carries per-copy state. This milestone
adds the base block to `ItemDefinition` and leaves per-copy options out.

## 3. Equipment slot list

`enum EMSLOT`, `legacy/Lib_Client/G-Logic/GLItemDef.h:207-245`. This is the
authoritative list; it is not a generic MMO slot list.

| # | Name | # | Name |
| - | ------------- | - | ------------- |
| 0 | `SLOT_HEADGEAR` | 11 | `SLOT_RHAND_S` (extreme) |
| 1 | `SLOT_UPPER` | 12 | `SLOT_LHAND_S` (extreme) |
| 2 | `SLOT_LOWER` | 13 | `SLOT_VEHICLE` |
| 3 | `SLOT_HAND` | 14 | `SLOT_LEARRING` |
| 4 | `SLOT_FOOT` | 15 | `SLOT_LACCESSORY` |
| 5 | `SLOT_RHAND` | 16 | `SLOT_RACCESSORY` |
| 6 | `SLOT_LHAND` | 17 | `SLOT_ORNAMENT` |
| 7 | `SLOT_NECK` | 18 | `SLOT_WAIST` |
| 8 | `SLOT_WRIST` | 19 | `SLOT_FACE` |
| 9 | `SLOT_RFINGER` | 20 | `SLOT_MISC` |
| 10 | `SLOT_LFINGER` | | |

`SUM_ITEM` iterates `for ( int i=0; i<SLOT_NSIZE_S_2; i++ )`
(`GLogixExPC.cpp:446`) with `SLOT_NSIZE_S_2 = 21` (`GLItemDef.h:239`), so the
wearable range is exactly `[0, 21)`. `SLOT_HOLD = 21` shares the value 21 and is
the non-wearable hold slot; `SLOT_TSIZE = 22` counts it. **21 wearable slots.**

`SUM_ITEM` also skips `ITEM_CHARM` (`:456`) and consults `VALID_SLOT_ITEM`
(`:449`).

## 4. Equip validation rules

Validation lives in `GLITEMLMT` / `EMREQUIRE_*` and the
`GLCharactorReq.cpp:166-183` handler, which enumerates at least
`EMREQUIRE_COMPLETE`, `EMREQUIRE_LEVEL` and `EMREQUIRE_SCHOOL`
(`GLCharactorReq.cpp:180`). `GLChar.cpp:524-531` shows the same
`EMREQFAIL emReqFail(EMREQUIRE_COMPLETE)` gate.

This is a large, data-driven requirement system (class, gender, school, level,
weapon type, wear position, per-requirement values). Per §5D and §7, the
requirement system is **deferred**; the modern `EquipmentState` is a container
and the only validation it performs is structural (a wearable slot, a valid
instance). That keeps an unimplemented rule from being silently assumed away, and
keeps the container testable without a character.

## 5. `SSUM_ITEM` construction path

`GLCHARLOGIC::SUM_ITEM` (`GLogixExPC.cpp:441`), called from
`GLCHARLOGIC::INIT_DATA` before `SUM_ADDITION`. Per slot, in order:

1. `m_sSUMITEM.RESET()` (`:444`), then `for i in [0,21)`, skipping invalid slots
   (`:449`) and `ITEM_CHARM` (`:456`).
2. **`sADDON[5]`** (`:458-537`): each typed integer add-on adds into
   `sStats`/`nHP`/`nMP`/`nSP`/`nHitRate`/`nAvoidRate`/`nDefense`/`gdDamage`/
   `nPA`/`nSA`/`nMA`.
3. **`GETOptVALUE(EMR_OPT_HP/MP/SP)`** (`:539-541`): flat resource from the
   *instance*.
4. **`sVARIATE[5]`** (`:544-590`): `EMVAR_HP/MP/SP/AP` add into
   `fIncR_HP/MP/SP`; the rest feed fields CORE-002 does not model.
5. **`GETOptVALUE(EMR_OPT_HP_INC / MP_INC / SP_INC / HMS_INC)`** (`:592-594`):
   recovery rate from the *instance*.
6. **`sVOLUME`** (`:600-643`): `EMVAR_HP/MP/SP/AP` add into `fIncR_HP/MP/SP`.
7. **`GETADDPA` / `GETADDSA` / `GETADDENERGY`** (`:653-655`) — note magic attack
   comes from `GETADDENERGY`, not a `GETADDMA`.
8. **`GETDAMAGE`**, **`GETDEFENSE`**, **`GETAVOIDRATE`**, **`GETHITRATE`**
   (`:657-661`).
9. **`GETRESIST_ELEC/FIRE/ICE/POISON/SPIRIT`** (`:666-670`).
10. **`GET_STAT_POW / GET_STAT_INT / GET_STAT_DEX / GET_STAT_STM /
    GET_STAT_VIT`** (`:672-676`) — RAN's odd mapping: `GET_STAT_INT` feeds
    `wSpi` and `GET_STAT_STM` feeds `wDex`.
11. **`GETAVOIDRATE_PER` / `GETHITRATE_PER`** (`:683-684`).
12. **`GETMaDAMAGE`** (`:688`).

**A disabled case worth recording:** `EMADD_MA` is *commented out* at
`GLogixExPC.cpp:533-535`, with the note that `GETMaDAMAGE()` is needed and is
incompatible. So the `MA` *add-on* does nothing, while the `GETADDENERGY` and
`GETMaDAMAGE` accessors do contribute.

**A legacy defect worth not reproducing:** `SITEMCUSTOM::GETDAMAGE`
(`GLItem.cpp:2627`) writes `sSUIT.gdDamage.dwLow = sSUIT.gdDamage.dwHigh;` when
low exceeds high. `sSUIT` is a reference into the **shared item definition**, so
this mutates global definition state from a getter. Modern must not do that.

## 6. Exact fields feeding `Stats::ItemContribution`

The existing struct (CORE-002) already matches; **no change to it is required.**

| `Stats::ItemContribution` | Source in `SUM_ITEM` |
| --- | --- |
| `stats` (6 x uint16) | `EMADD_STATS_*`, `EMADD_POWINTDEX`, `EMADD_STMVIT`, `GET_STAT_*` |
| `hp` / `mp` / `sp` | `EMADD_HP/MP/SP`, `GETOptVALUE(EMR_OPT_HP/MP/SP)` |
| `hpRecoveryRate` / `mpRecoveryRate` / `spRecoveryRate` | `EMVAR_HP/MP/SP/AP` (variate and volume), `GETOptVALUE(EMR_OPT_*_INC)`, `EMR_OPT_HMS_INC` |
| `meleePower` | `EMADD_PA`, `EMADD_PASA`, `EMADD_PASAMA`, `GETADDPA` |
| `shootPower` | `EMADD_SA`, `EMADD_PASA`, `EMADD_PASAMA`, `GETADDSA` |
| `magicAttack` | `EMADD_PASAMA`, `GETADDENERGY`, `GETMaDAMAGE` |
| `hit` / `avoid` | `EMADD_HITRATE/AVOIDRATE`, `GETHITRATE`, `GETAVOIDRATE` |
| `hitRatePercent` / `avoidRatePercent` | `GETHITRATE_PER`, `GETAVOIDRATE_PER` |
| `defense` | `EMADD_DEFENSE`, `GETDEFENSE` |
| `damageLow` / `damageHigh` | `EMADD_DAMAGE`, `GETDAMAGE` |
| `resistances` (5) | `GETRESIST_ELEC/FIRE/ICE/POISON/SPIRIT` |

## 7. Random-option behaviour

**Confirmed by source, and deliberately deferred.** The `SITEMCUSTOM::GET*`
accessors start from the definition and then add `GETOptVALUE(EMR_OPT_*)`, which
reads options rolled onto that one copy — so RAN's item contribution genuinely
mixes base and per-copy values in a single pipeline. The specific formulas
(`nHITRATE = short(nHITRATE + fRATE + dHITRATE)` and the rate branches in
`GETHITRATE`, `GLItem.cpp:2821`) are conditional on the option being non-zero,
so a copy with no options is exactly its definition.

VERTICAL-002 therefore aggregates the **definition base only**, and the modern
`ItemInstance` keeps its existing shape, whose own comment already records that
random options, upgrades and enchantment state belong to a future system. The
first roll-capable system must add them to the instance and extend the
aggregator, not to the definition.

## 8. Upgrade / refine behaviour

`default.charclass`-style base data is loaded for the *class*, not the item, and
item refining is a server-side write to the item record. Corroborated externally
(RAN's official item list: refining raises weapon power, defense and Fire / Ice /
Electric / Poison resistances; the RaGEZONE item-editor guide notes stat fields
cap at 65535 and wrap). None of it is present in the definition-vs-instance
split this milestone models, so it is **deferred** with the options.

## 9. Deferred and not-used classification

| Classification | Items |
| --- | --- |
| **CONFIRMED — implemented** | 21 wearable slots; the six base stats; flat HP/MP/SP; three recovery rates; melee / shoot / magic power; hit and avoid; hit and avoid percentages; defense; damage low/high; five resistances |
| **DEFERRED — needs an instance system** | `GETOptVALUE(EMR_OPT_*)` random options; refining / upgrade state; `EMVAR_*` values CORE-002 does not model |
| **NOT USED BY THE STAT PIPELINE** | movement speed, attack speed, critical rate, crushing blow, the three damage-reduction rates, `nHP/MP/SP_Potion_Rate`, `wAttRange`, `wReModelNum`, `emAttack` type, `bBothHand` |
| **UNKNOWN** | nothing material; the two items in the rows above were each traced to source |

`EMADD_MA` is confirmed *disabled* upstream (§5) and is therefore not implemented.

## 10. Modern architecture mapping

```
ItemDefinition            + ItemStatBlock (base values only)   [extended]
ItemInstance              unchanged; no option state yet
EquipmentState            slot -> ItemInstance, 21 slots       [new, core]
ItemDefinitionProvider    in-memory, deterministic             [new, core]
ItemContributionAggregator EquipmentState + provider
                            -> Stats::ItemContribution        [new, core]
ServerCharacter           + EquipmentState; recalculates      [extended]
CharacterSnapshot         + equipped slot summary            [extended]
ClientCharacterState      + read-only equipment view          [extended]
```

Nothing restates a stat formula: the aggregator produces a
`Stats::ItemContribution` and `Stats::Calculate` remains the only place RAN's
arithmetic lives.

## 11. External corroboration (§21), non-authoritative

- RAN item data lives in `item.isf` with names in `itemstrtable`; the item editor
  exposes a "wear position" field — consistent with `SSUIT` + `EMSLOT`.
- The official RAN item list shows refining raising weapon power, defense and
  Fire / Ice / Electric / Poison resistances — consistent with `gdDamage`,
  `nDefense` and `SRESist` being the upgraded fields.
- The RaGEZONE item-editor guide notes stat fields max at 65535 and wrap, and
  that HP recovery `1` means 100% per second — consistent with `short` stat fields
  and a fractional recovery rate.

None of this contradicts the local source, and none of it was used to invent a
field. RCC parsing is explicitly out of scope.
