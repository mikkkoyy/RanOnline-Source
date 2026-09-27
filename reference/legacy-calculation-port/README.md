# Legacy Calculation Port (reference only — not built)

This directory holds an earlier, work-in-progress port of the legacy RAN
`GLOGICEX` character calculation chain out of `legacy/` and into a modern
C++ core. It was moved here on 2026-09-27 by CORE-001, and it is **not part of
any build target**.

## Why it moved

The port does not compile, and — more importantly — it is not a foundation. It
reproduces RAN's derived-stat architecture field for field:

| Ported concept              | Legacy origin                                  |
| --------------------------- | ---------------------------------------------- |
| 21 equipment slots          | `GLChar::m_sSTUFFEQUIP` / `SLOT_NSIZE_S_2`      |
| HP / MP / SP pools          | `m_sHP`, `m_sMP`, `m_sSP` + recovery rates      |
| `ActState` bitfield         | `GLChar::GetSTATE()` / `m_dwActSTATE`           |
| `ItemContribution`          | `SSUM_ITEM`                                     |
| `InstanceCustomContribution`| `GETADDPA` / `GETADDSA` / `GETDAMAGE` getters   |
| `PassiveSkillContribution`  | `m_sSUM_PASSIVE`                                |
| `CodexContribution`         | `m_dwHPIncrease` and friends                    |
| `CombatStats`               | derived `m_nHIT` / `m_nDEFENSE` / `m_gdDAMAGE`  |

CORE-001 fixes the core's scope to identity, class, level, experience,
position, direction and lifecycle. Everything this port computes — calculated
stats, equipment contributions, passive skills, codex effects, resource pools —
belongs to a future `StatsSystem` / `EquipmentSystem` / `CombatSystem` and is
explicitly out of CORE-001 scope.

The port is also internally inconsistent, which is why it never compiled:

- `ItemData.h` declared `CalculateInstanceCustomContribution` without including
  the header that defines `InstanceCustomContribution`.
- `PassiveSkillData.h` defined an inline overload taking `const Character&`
  and called `Character` members through a bare forward declaration.
- `PassiveSkillContribution::DamageSpec` and `CombatStats::DamageSpec` are
  structurally identical but unrelated types, so they could not be assigned
  between.
- `Character.h` declared `m_instanceContribution` and drove a 21-slot
  recalculation path that no longer had a coherent header chain.

## Status

Preserved as a research artefact. It is a faithful record of the formulas RAN
used, and it is the right starting point if the calculated-stat systems are
rebuilt later. It is **not** a reference implementation, and it is not
maintained.

## How to use it

Read it as documentation of the legacy formulas. If the systems are
reimplemented, port the *rules* and redesign the *model* — do not copy these
types into `modern/core`.

Flattening the directory into this one broke the relative includes, so these
files do not compile as they stand. To read them together:

- `CombatStats.h` and `PassiveSkillData.h` still say `#include "../types/Types.h"`
  and `#include "../item/ItemData.h"`; `Types.h` no longer exists anywhere and
  `ItemData.h` is now a sibling.
- `ItemData.h` needs `#include "InstanceCustomContribution.h"`, which is a
  sibling here and was the original missing include.

See [../../docs/MODERN_ARCHITECTURE.md](../../docs/MODERN_ARCHITECTURE.md) for
the dependency rules this directory exists to respect.
