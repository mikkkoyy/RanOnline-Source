# VERTICAL-012 — Ranged physical combat

## Summary

Ranged physical combat is not a second combat pipeline. It is the physical
pipeline with a different attack power selected, and it is reached through the
active-skill resolver VERTICAL-011 already built. This milestone fills that
seam:

- `PhysicalDamageCalculator` now applies the attack power to the damage range,
  which is what legacy's `VAR_PARAM` does and which no code did before.
- `AttackType` selects between `meleePower` and `shootPower`.
- `ActiveSkillResolver` accepts `SkillApply::PhysicalRanged` and carries the
  channel through to the combat boundary.

What is deliberately **not** here is ranged reach, projectile travel and ammo
consumption. Those are spatial and item systems, not damage arithmetic, and no
legacy source in this repository justified inventing them. They stay deferred
(§7).

---

## 1. Repository check

Started from `main` at `df9bf4f13f9f035cb85c84ec397e7e0120d79e90`, matching
`origin/main`, with a clean working tree. VERTICAL-009 (`ad99138`) and
VERTICAL-011 are the baselines this work builds on.

## 2. Public and forum backread — negative

Public and forum backread for RAN ranged-combat internals was attempted and
returned **nothing usable**. The results were generic RPG damage-formula
articles and other MMORPG projects; none of them were RAN, and none contained
RAN's actual values. Nothing from that backread is cited here as evidence,
because none of it is evidence. Everything below comes from the legacy source
that is in this repository.

This is worth recording as a finding rather than a gap: it means future
migrations should not expect the public web to supply RAN internals, and should
plan on the checked-in legacy tree instead.

## 3. The legacy evidence

`m_wSUM_PA` is the melee attack power and `m_wSUM_SA` is the shoot attack
power. Legacy picks between them in four places:

| Site | Channel | Power |
| --- | --- | --- |
| `GLogixExPC.cpp:1451` | skill, `EMAPPLY_PHY_SHORT` | `m_wSUM_PA` |
| `GLogixExPC.cpp:1463` | skill, `EMAPPLY_PHY_LONG` | `m_wSUM_SA` |
| `GLogixExPC.cpp:1584` | basic attack, long-range arms | `m_wSUM_SA` |
| `GLogixExPC.cpp:1594` | basic attack, otherwise | `m_wSUM_PA` |

Skills are therefore discriminated by `emAPPLY`, and basic attacks by
`ISLONGRANGE_ARMS()`, which is a property of the right-hand item
(`GLogixExPC.cpp:4768-4772`: `sSuitOp.emAttack > ITEMATT_NEAR`). Both
discriminators are *the same question* — is this attack ranged or not — which is
why the modern code models it as one `AttackType` rather than as a skill-side
flag beside an item-side flag.

The same power also builds the damage range for the whole damage path, not only
for skills (`GLogixExPC.cpp:388-389`, `:3001-3002`).

### `VAR_PARAM` is a saturating add on both ends

`GLDefine.h:364-371`:

```cpp
if ( (int(wLow)  + nValue) < 1 )  wLow  = 1;  else wLow  += nValue;
if ( (int(wHigh) + nValue) < 1 )  wHigh = 1;  else wHigh += nValue;
```

So the attack power is added to **both** endpoints of the range before the
random roll, and each endpoint is floored at `1` rather than clamped to zero.
The floor at `1` is what makes the later minimum-damage branch reachable in
practice, so getting it wrong would silently change low-damage behaviour
everywhere.

## 4. The gap this milestone closes

The modern stat layer was already correct: `StatCalculator.cpp:168-206` builds
`DerivedStats::meleePower` and `DerivedStats::shootPower` with the class/level
term, the POW/DEX term and the `VARIATION` clamp over item, passive and codex
contributions, matching `GLogixExPC.cpp:313-337`. `CombatInput` already carried
`attackerMeleePower` and `attackerShootPower`, and `CombatCalculator.h` already
copied them into `PhysicalDamageInput`.

**Both powers were then ignored.** `PhysicalDamageCalculator` read neither
field. Every physical hit in the modern tree was computed from the weapon range
alone, so attack power had no effect on any damage at all. This was a bug in
VERTICAL-009, not merely a missing ranged feature, and it is why the melee
results below changed.

The fix adds the power once, on the range, and picks it from `AttackType`:

```cpp
const int32_t attackPower = (input.attackType == AttackType::Ranged)
                                ? static_cast<int32_t>(input.shootPower)
                                : static_cast<int32_t>(input.meleePower);

Stats::DamageRange damage = input.physicalDamage;
damage.low  = ApplyAttackPower(damage.low,  attackPower);
damage.high = ApplyAttackPower(damage.high, attackPower);
```

`ApplyAttackPower` is an anonymous-namespace helper that reproduces the
`VAR_PARAM` saturation, including the `< 1 → 1` floor. The signed intermediate
matters: legacy casts to `int` before adding so an overflowing or wrapped
unsigned addition is compared and clamped as a signed value.

## 5. Reflection

`GLogixExPC.cpp:1468-1469` zeroes both reflection terms for `EMAPPLY_PHY_LONG`,
which VERTICAL-009 already implemented. That behaviour was written and tested in
VERTICAL-009 while ranged skills could never reach the calculator, so it was
**unreachable code**. Filling the seam makes it live for the first time, which
is why this milestone adds tests that a reflecting target still reflects
against a melee skill but not against a ranged one, using a reflection roll that
would otherwise trigger.

The rollback case is covered too: `RangedSkill_PowerSelectionFollowsTheChannelNotMagnitude`
reverses which power is larger and checks that the *ranged* result becomes the
weaker one. A test that only checks ranged-is-stronger would pass even if the
calculator simply always preferred `shootPower`.

## 6. Test changes to VERTICAL-009 / VERTICAL-010 cases

Adding attack power raised melee damage, which broke three existing cases. All
three were recalculated from the verified legacy pipeline rather than relaxed,
and the arithmetic is written into the comments so the expectations can be
re-derived:

- `Combat_DamageRangeMinRoll` — the fixture's `{10,20}` plus melee power `3`
  gives `{13,23}`, so the minimum roll is `13`. Was an upper bound of `10`, which
  only held because the power was being dropped.
- `CombatReflection_DamageReducePlusReflection` — this one is worth spelling out
  because it is easy to get wrong. The rolled damage is `18`, but `targetLevel`
  is `100` against `attackerLevel` of `1`, so `nExtFORCE` adds
  `int(0.5 * 99 / 10) = 4` for a pre-defence `22`; the reduction is then
  `DamageReduceAmount(22, 0.3, 100, 300) = int(2.2) = 2`, giving `20`. The old
  assertion was `< 20`, so the added power moved the result onto the boundary
  exactly. Pinned to `20`.
- `RequiredSPMatrix_HighSPIsNotLowSP` — this was not a power problem. The
  low-SP penalty is `* 0.5` in floating point then truncated, so on the odd
  value `9` the low-SP result is `4` and `4 * 2 != 9`. The old assertion was
  arithmetically unsatisfiable for this fixture. It now asserts the rule itself
  (low-SP strictly less, and within the rounding of a doubling), with the
  exact-halving case still pinned by `RequiredSPMatrix_LowSPHalvesSkillDamage`
  on a fixture chosen to be even.

`ActiveSkill_RangedApplyRejected` was retired, since it asserted the deferral
this milestone removes. It is replaced by `ActiveSkill_RangedApplyAccepted`;
`ActiveSkill_MagicApplyRejected` is untouched and still passes.

## 7. Deliberately still deferred

None of the following were implemented, because none of them are damage
arithmetic and no checked-in legacy source justified guessing at them:

- Range/reach limits and the `ISLONGRANGE_ARMS()` item classification from
  `GLogixExPC.cpp:4768-4772`. This needs the world and targeting systems.
- Projectile travel, and ammo consumption.
- The `ActiveSkillResolver` continues to refuse zone/realm targeting and every
  other channel it refused before.

Magic and elemental damage remain VERTICAL-013. `ActiveSkillResolver` now
refuses `SkillApply::Magic` explicitly by name rather than by falling through a
`PhysicalMelee` comparison, so that the next milestone has an obvious place to
start and so the refusal cannot be mistaken for an oversight.

## 8. Verification

Both configurations built clean with no new warnings.

| Suite | Result |
| --- | --- |
| ModernCoreTests | 303/303 (10 new) |
| ModernServerTests | 70/70 |
| CTest | 14/14 Debug and Release |
| Client suites | 12/12 |

---

## References

- `legacy/Lib_Client/G-Logic/GLogixExPC.cpp:313-337, 388-389, 1451, 1463, 1468-1469, 1584, 1594, 3001-3002, 4768-4772`
- `legacy/Lib_Engine/G-Logic/GLDefine.h:364-371`
- `modern/core/combat/PhysicalDamageCalculator.h`
- `modern/core/combat/CombatCalculator.h`
- `modern/core/skills/ActiveSkill.cpp`
- `modern/core/stats/StatCalculator.cpp:168-206`
- `docs/reference/client/VERTICAL-009_PHYSICAL_COMBAT_INVESTIGATION.md`
- `docs/reference/client/VERTICAL-011_ACTIVE_SKILL_INVESTIGATION.md`