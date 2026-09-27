# legacy-mapping/

Legacy RAN concept -> modern type. The translation layer's specification.

This table is a research artifact, not code. `modern/core` is written so that it
never needs these mappings at runtime; the mappings exist so that adapters in
`modern/compatibility/legacy/` can be written deliberately, and so that a reader
of the modern code can find the legacy original.

Status values:

- **done** — modern type exists, adapter written.
- **modelled** — modern type exists, adapter still to write.
- **planned** — identified, not started.

## Characters

| Legacy | Modern | File | Status |
| ------ | ------ | ---- | ------ |
| `GLChar` | `Modern::Character` | `core/character/Character.h` | done |
| `CharacterBase` / `default.charclass` + `class*.classconst` | `Modern::CharacterBaseData` | `core/character/CharacterBaseData.h` | modelled |
| `SPASSIVE_SKILL_DATA` (stat-affecting fields) | `Modern::PassiveSkillContribution` | `core/character/PassiveSkillData.h` | modelled |
| `m_dw*Increase` (codex progression) | `Modern::CodexContribution` | `core/character/CodexContribution.h` | modelled |
| `GLOGICEX` derived combat stats (`m_nHIT`, `m_nAVOID`, `m_gdDAMAGE`, ...) | `Modern::CombatStats` | `core/character/CombatStats.h` | modelled |
| `cCHARCONST` (class constants) | `Modern::CharacterLevelUpStats` | `core/character/CharacterBaseData.h` | modelled |

Adapter: `CharacterAdapter` in `modern/compatibility/legacy/`.

Providers exist alongside the data so tests can run without legacy data files:
`TestCharacterBaseDataProvider` / `RANCharacterBaseDataProvider`,
`TestCodexProvider` / `ICodexProvider`,
`TestPassiveSkillProvider` / `IPassiveSkillProvider`,
`TestItemDataProvider` / `RANItemDataProvider`,
`TestProgressionData` / `RANProgressionData`.

## Items

| Legacy | Modern | File | Status |
| ------ | ------ | ---- | ------ |
| `SITEM` | `Modern::ItemInstanceData` | `core/item/ItemData.h` | modelled |
| `SITEM` (base row) | `Modern::ItemBaseData` | `core/item/ItemData.h` | modelled |
| `SSUM_ITEM` | `Modern::ItemContribution` | `core/item/ItemData.h` | modelled |
| `SITEMCUSTOM` computed values (`GETDAMAGE`, `GETADDPA`, `GETADDSA`, `GETADDMA`, `GETDEFENSE`, `GETHITRATE`, `GETAVOIDRATE`, ...) | `Modern::InstanceCustomContribution` | `core/item/InstanceCustomContribution.h` | modelled |
| `sADDON` | `Modern::ItemAddon` | `core/item/ItemData.h` | modelled |
| `sVARIATE` | `Modern::ItemVariate` | `core/item/ItemData.h` | modelled |
| `sVOLUME` | `Modern::ItemVolume` | `core/item/ItemData.h` | modelled |
| `emOpt_` random options | `Modern::ItemRandomOption` | `core/item/ItemData.h` | modelled |
| `sGRINDER` | `Modern::ItemGrinding` | `core/item/ItemData.h` | modelled |
| `ITEM_*` constants | `Modern::ItemType` | `core/item/ItemData.h` | modelled |
| `EMADD_` addon types | `Modern::AddonType` | `core/item/ItemData.h` | modelled |
| equip slot / hand / position | `Modern::EquipSlot`, `Modern::EquipPosition` | `core/item/ItemData.h` | modelled |
| random-option tables (`randomOption_*.bin`) | `Modern::ItemRandomOption` | — | planned |
| item table itself | `Item.csv` | `reference/data-formats/Item.csv` | done (export) |

## Progression

| Legacy | Modern | File | Status |
| ------ | ------ | ---- | ------ |
| `lnEXP_MAX_TABLE` / `exptable_max.bin` | `Modern::ProgressionData` | `core/progression/ProgressionData.h` | done (export) |
| `EXP_MAX_LOADFILE` text contract | `ProgressionData` loader | `core/progression/ProgressionData.h` | done |

## Common

| Legacy | Modern | File | Status |
| ------ | ------ | ---- | ------ |
| `D3DXVECTOR3` | `Modern::Vector3` | `core/math/Vector3.h` | done |
| object handles / user-num pairs | `Modern::EntityId` | `core/types/Types.h` | done |
| `DWORD` resource fields | `uint32_t` | `core/types/Types.h` | done |
| `enum class ActionType` (action state) | `Modern::ActionType`, `Modern::ActState` | `core/types/Types.h` | done |
| `Entity`-style base record | `Modern::Entity` | `core/entity/Entity.h` | done |

## Naming

Modern types drop the Hungarian prefix and use `PascalCase` with `AdjectiveNoun`
structure (`PassiveSkillContribution`, not `SPASSIVE_SKILL_DATA`). Fields use
the same convention, so a legacy `m_nHIT` becomes a `hit` member. The mapping
is mechanical and is applied consistently so that the modern headers read as
their own code rather than as a transliteration.

## Not mapped yet

- `Lib_Network` ODBC classes (`COdbcUser`, `COdbcGameChaSave`, ...) — see the
  SQL audit in [`../notes/`](../notes/).
- `Tik` / GGAuth packet layer — see [`../protocol/`](../protocol/).
- `res` / `.egp` field and effect files.
