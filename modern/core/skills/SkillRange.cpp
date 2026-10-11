// SKILL-010: see SkillRange.h for why these two rules exist and why they are
// not folded into AttackService.
//
// These are transcriptions of
// legacy/Lib_Engine/Common/GameCharacterCalculations.cpp:887 and :933. The
// legacy functions are already portable and deterministic, so the port is a
// copy, not a reinterpretation. Where a line looks like it could be tidied -
// the redundant cast, the bare `20`, the bare `5` - it is left alone,
// because each of those is the literal legacy wrote and each one is load-
// bearing for an edge case:
//
//   * `20` is present in SkillTargetRange and ABSENT from SkillApplyRange.
//   * the cast to `int` truncates a fractional SUM bonus toward zero rather
//     than rounding it, and the two differ for negative bonuses.
//   * the accumulation is `int`, so a 60000 base plus a bonus cannot wrap the
//     way a `uint16_t` accumulator would.
//
// Both functions are noexcept, allocation-free and free of RNG, I/O and any
// world service. The only inputs are their parameters.

#include "SkillRange.h"

namespace Modern::Skills
{
	std::uint16_t SkillTargetRange(std::int32_t baseTargetRange,
	                               bool           isPhysicalLongRange,
	                               std::int32_t   targetRangeBonus,
	                               bool           affectsEnemy,
	                               float           skillAttackRangeBonus) noexcept
	{
		// Legacy `:894-908`. Accumulator is `int`, matching legacy's `int nRANGE`.
		int nRANGE = static_cast<int>(baseTargetRange);

		// The floor is applied to the BASE, before any bonus. A skill with
		// wTARRANGE 0 still reaches 20 unless a bonus is applied on top.
		if (nRANGE < 20)
			nRANGE = 20;

		if (isPhysicalLongRange)
			nRANGE += targetRangeBonus + 5;

		// `static_cast<int>` truncates toward zero - see the file header.
		if (affectsEnemy)
			nRANGE += static_cast<int>(skillAttackRangeBonus);

		if (nRANGE <= 0)
			nRANGE = 1;

		return static_cast<std::uint16_t>(nRANGE);
	}

	std::uint16_t SkillApplyRange(std::int32_t baseApplyRange,
	                              bool           isPhysicalLongRange,
	                              std::int32_t   targetRangeBonus,
	                              bool           affectsEnemy,
	                              float           skillApplyRangeBonus) noexcept
	{
		// Legacy `:940-951`.
		//
		// Identical to the target rule apart from one line that is
		// deliberately NOT here: there is no `if (nRANGE < 20)`. Legacy
		// preserves this asymmetry deliberately (GameCharacterCalculations.h:
		// 696-698), so a low per-level wAPPLYRANGE reaches exactly that low a
		// value, and adding the floor would quietly widen every such skill.
		int nRANGE = static_cast<int>(baseApplyRange);

		if (isPhysicalLongRange)
			nRANGE += targetRangeBonus + 5;

		if (affectsEnemy)
			nRANGE += static_cast<int>(skillApplyRangeBonus);

		if (nRANGE <= 0)
			nRANGE = 1;

		return static_cast<std::uint16_t>(nRANGE);
	}

} // namespace Modern::Skills