#pragma once

// SKILL-010: the two legacy skill-range calculations, as pure functions.
//
// ---------------------------------------------------------------------------
// WHY THIS EXISTS, AND WHY IT IS NOT AttackService
// ---------------------------------------------------------------------------
//
// A basic attack and a skill cast reach by DIFFERENT rules, and getting them
// from one place would hide that:
//
//   basic attack   targetBodyRadius + attackerBodyRadius + weapon wAttRange
//                  + 2, then + 7 slack      (GLCharMsg.cpp:343-347)
//   skill TARGET   wTARRANGE, floored at 20, + SUM_TARRANGE + 5 when long
//                  range, + the SUM_SKILL_ATTACKRANGE pair when it affects an
//                  enemy, then a floor of 1 (GameCharacterCalculations.cpp:887)
//   skill APPLY    wAPPLYRANGE, NO floor,    + the same two bonuses, then a
//                  floor of 1              (GameCharacterCalculations.cpp:933)
//
// `AttackService::AllowedDistanceFor` implements the first and is correct for
// the 3036 path. It is NOT generalised here: a mode flag would let a caller
// pick the wrong tolerance (+7 against +1) or the wrong floor (20 against
// none) without anything in the type system objecting. Keeping these separate
// is what stops the two rule sets from silently sharing behaviour.
//
// ---------------------------------------------------------------------------
// THE TWO SOURCE FIELDS ARE NOT INTERCHANGEABLE
// ---------------------------------------------------------------------------
//
//     target range  <-  SSKILLBASIC::wTARRANGE    ->  SkillDefinition::targetRange
//     apply  range  <-  CDATA_LVL::wAPPLYRANGE    ->  SkillLevelData::applyRange
//
// They are per SKILL and per LEVEL respectively, live in different legacy
// structs, and legacy names them one character apart. `wSkillRange` in
// GLCharSkillMsg.cpp:317 is `GETSKILLRANGE_TAR`, i.e. `wTARRANGE` - not
// `wAPPLYRANGE`. Legacy's own TAR_SPEC branch (`:510`) and TAR_SELF branch
// (`:483`) therefore read different fields.
//
// ---------------------------------------------------------------------------
// THE SUM BONUSES ARE NOT AVAILABLE YET - READ THIS BEFORE TRUSTING A RESULT
// ---------------------------------------------------------------------------
//
// Three inputs have no authoritative modern source today:
//
//     GETSUM_TARRANGE()
//     m_fSUM_SKILL_ATTACKRANGE + m_sSUM_PASSIVE.m_fSUM_SKILL_ATTACKRANGE
//     m_fSUM_SKILL_APPLYRANGE + m_sSUM_PASSIVE.m_fSUM_SKILL_APPLYRANGE
//
// They are supplied here as explicit parameters rather than read from
// somewhere plausible-looking. A caller passing zero is computing the BASE
// formula only - which is a valid thing to want and is what the tests do, but
// it is NOT evidence that a production character has no such bonuses.
// `AttackService.h:53-56` records the same gap for the basic-attack rule.
//
// Range parity for a character that actually carries these bonuses is
// therefore NOT established until their owner exists. Nothing here guesses one.
//
// ---------------------------------------------------------------------------
// UNITS
// ---------------------------------------------------------------------------
//
// World units, the same space `AttackService` measures in and the same space
// legacy compares a distance against in. `SkillTargetRange` and
// `SkillApplyRange` return a REACH, not a tolerance: the caller compares the
// distance to the caster against it. The distance comparison, the line of
// sight check and the per-target-mode dispatch are deliberately NOT here -
// this is arithmetic only.
//
// ---------------------------------------------------------------------------
// WHY THE RETURN TYPE IS A PLAIN REACH, NOT A RESULT TYPE
// ---------------------------------------------------------------------------
//
// Legacy's two functions cannot fail: both end in a floor of 1 and return a
// `GameUInt16`, so every input maps to a value. Inventing a failure case here
// would mean disagreeing with the shipped game, so neither returns a result
// type.
//
// "Unavailable range data" is therefore a LOAD question, and it is answered
// where the data is read. `SkillBasicTable.cpp` parses `wAPPLYRANGE` in the
// same block as the other seven per-level fields, and a row whose value is
// absent, non-numeric, negative or above 65535 rejects that SAPPLY row and
// with it the whole skill definition. A `SkillLevelData` that exists at all
// therefore carries a value that was actually in the export.
//
// That is what keeps an absent value from becoming a valid-looking zero. A
// genuine 0 in the table is preserved as a genuine 0 - the two are told apart
// by whether the definition was admitted, not by a sentinel in the field,
// because legacy `CDATA_LVL` has no sentinel for "unset" either.

#include <cstdint>

namespace Modern::Skills
{
	// The constant legacy adds to a long-range reach: `GETSUM_TARRANGE() + 5`
	// (GameCharacterCalculations.cpp:900, :943).
	inline constexpr std::int32_t kLongRangeFlatBonusUnits = 5;

	// The target-range floor. The apply-range rule has NO equivalent, and that
	// asymmetry is legacy's, stated explicitly at GameCharacterCalculations.h:
	// 696-698 and reproduced here rather than smoothed over.
	inline constexpr std::int32_t kMinimumTargetRangeUnits = 20;

	// The final floor both rules share, applied only when the accumulated value
	// is still non-positive (`:905-906`, `:948-949`).
	inline constexpr std::int32_t kMinimumComputedRangeUnits = 1;

	// `GLCHARLOGIC::GETSKILLRANGE_TAR` -> `SkillTargetRange`
	// (GameCharacterCalculations.cpp:887).
	//
	// `baseTargetRange` is `SSKILLBASIC::wTARRANGE`, a per-skill reach.
	// `targetRangeBonus` is `GETSUM_TARRANGE()`.
	// `skillAttackRangeBonus` is the SUM_SKILL_ATTACKRANGE pair.
	//
	// Faithful to legacy, including the order: the floor is applied to the BASE
	// before any bonus is added, so a low base with a large bonus yields the
	// bonus rather than the floor.
	std::uint16_t SkillTargetRange(std::int32_t baseTargetRange,
	                               bool           isPhysicalLongRange,
	                               std::int32_t   targetRangeBonus,
	                               bool           affectsEnemy,
	                               float           skillAttackRangeBonus) noexcept;

	// `GLCHARLOGIC::GETSKILLRANGE_APPLY` -> `SkillApplyRange`
	// (GameCharacterCalculations.cpp:933).
	//
	// `baseApplyRange` is `sDATA_LVL[level].wAPPLYRANGE`, a per-LEVEL reach.
	// `targetRangeBonus` is `GETSUM_TARRANGE()`.
	// `skillApplyRangeBonus` is the SUM_SKILL_APPLYRANGE pair.
	//
	// Identical to `SkillTargetRange` except for the missing floor of 20. That
	// difference is the whole reason these are two functions and not one with
	// a flag.
	std::uint16_t SkillApplyRange(std::int32_t baseApplyRange,
	                              bool           isPhysicalLongRange,
	                              std::int32_t   targetRangeBonus,
	                              bool           affectsEnemy,
	                              float           skillApplyRangeBonus) noexcept;

} // namespace Modern::Skills