// SKILL-010: the two legacy skill-range rules, tested in isolation.
//
// These tests exercise ARITHMETIC ONLY. No distance check, no line of sight,
// no target-mode dispatch and no world service - those are deliberately not
// part of this milestone. See SkillRange.h for why the rules are separate
// from AttackService.
//
// The bonus inputs (GETSUM_TARRANGE and the two SUM pairs) have no
// authoritative modern source yet. Passing a real non-zero value here proves
// the parameter is applied; passing zero proves only the BASE formula, and no
// test below claims otherwise.

#include "TestHarness.h"
#include "skills/SkillRange.h"

#include <cstdint>

namespace ModernTests
{
	using namespace Modern::Skills;

	namespace
	{
		// Legacy keeps the floor literals inline (GameCharacterCalculations.cpp:897
		// and its absence at :940). These restate the expected values rather than
		// reading the constants back, so a wrong constant cannot agree with itself.
		constexpr std::uint16_t kLegacyTargetFloor = 20;
		constexpr std::uint16_t kLegacyFinalFloor = 1;

		// The export's common apply value, and the verified outliers from
		// skill (52, 1)..(52, 8). Legacy clamps neither the top nor applies a
		// floor of 20 to apply range, so these must survive unchanged.
		constexpr std::int32_t kCommonApplyValue = 50;
		constexpr std::int32_t kOutlierApplyValue = 60000;
		constexpr std::int32_t kOutlierApplyValueAlt = 57599;
	} // namespace

	// ===========================================================================
	// The target-range floor
	// ===========================================================================

	// Requirement 3: a wTARRANGE below 20 yields the legacy minimum.
	MODERN_TEST(SkillRange_TargetRangeAppliesTheLegacyMinimumOfTwenty)
	{
		for (std::int32_t base : { 0, 1, 5, 19 })
		{
			CHECK_EQ(SkillTargetRange(base, false, 0, false, 0.0f),
			         kLegacyTargetFloor);
		}
	}

	// The floor is a floor, not a replacement: 20 and 50 pass through.
	MODERN_TEST(SkillRange_TargetRangeLeavesValuesAtOrAboveTheFloorAlone)
	{
		CHECK_EQ(SkillTargetRange(kLegacyTargetFloor, false, 0, false, 0.0f),
		         kLegacyTargetFloor);
		CHECK_EQ(SkillTargetRange(kCommonApplyValue, false, 0, false, 0.0f),
		         static_cast<std::uint16_t>(kCommonApplyValue));
		// Requirement 5: the verified outliers are not clamped at the top.
		CHECK_EQ(SkillTargetRange(kOutlierApplyValue, false, 0, false, 0.0f),
		         static_cast<std::uint16_t>(kOutlierApplyValue));
	}

	// The floor is applied to the BASE, before any bonus. Legacy orders it that
	// way at :896-900, so a low base with a long-range bonus yields
	// 20 + bonus + 5 rather than max(20, 0 + bonus + 5).
	MODERN_TEST(SkillRange_TargetRangeFloorsTheBaseBeforeAddingBonuses)
	{
		// base 0 -> floor 20, then + (30 + 5) = 55
		CHECK_EQ(SkillTargetRange(0, true, 30, false, 0.0f),
		         static_cast<std::uint16_t>(55));
	}

	// ===========================================================================
	// The apply/target floor distinction
	// ===========================================================================

	// Requirement 4: a low application-range input does NOT receive the
	// target-range minimum. This is the asymmetry legacy calls out explicitly
	// at GameCharacterCalculations.h:696-698.
	MODERN_TEST(SkillRange_ApplyRangeHasNoMinimumOfTwenty)
	{
		// 0 and 1 still meet the SHARED final floor at GameCharacterCalculations.cpp
		// :948-949, which is a different rule from the target minimum of 20. That
		// is why they land on 1 while 5 and 19 pass through untouched.
		CHECK_EQ(SkillApplyRange(0, false, 0, false, 0.0f), kLegacyFinalFloor);
		CHECK_EQ(SkillApplyRange(1, false, 0, false, 0.0f), kLegacyFinalFloor);
		for (std::int32_t base : { 5, 10, 19 })
		{
			CHECK_EQ(SkillApplyRange(base, false, 0, false, 0.0f),
			         static_cast<std::uint16_t>(base));
		}
	}

	// A zero base is a real exported value, not "unset", and it is NOT lifted to
	// the target minimum. It only meets the shared final floor of 1. This is the
	// clearest statement of the floor asymmetry: 0 reaches 1, where the target
	// rule would have reached 20.
	MODERN_TEST(SkillRange_ApplyRangeDoesNotLiftAZeroBaseToTwenty)
	{
		CHECK_EQ(SkillApplyRange(0, false, 0, false, 0.0f), kLegacyFinalFloor);
		CHECK_EQ(SkillTargetRange(0, false, 0, false, 0.0f), kLegacyTargetFloor);
	}

	// The shared final floor only engages when the sum goes non-positive.
	MODERN_TEST(SkillRange_BothRulesFloorANonPositiveAccumulationAtOne)
	{
		// Target range: the base is floored to 20 FIRST, then the negative enemy
		// bonus drives the total to -79, and the final floor catches it.
		CHECK_EQ(SkillTargetRange(0, false, 0, true, -99.0f),
		         kLegacyFinalFloor);
		// Apply range has no 20-floor, so the base alone is small enough that the
		// same negative bonus takes it straight past zero.
		CHECK_EQ(SkillApplyRange(0, false, 0, true, -99.0f),
		         kLegacyFinalFloor);
		CHECK_EQ(SkillApplyRange(10, false, 0, true, -99.0f),
		         kLegacyFinalFloor);
		// The same bonus WITHOUT affectsEnemy is not applied at all, so the
		// base survives - proving the floor is not what is being observed above.
		CHECK_EQ(SkillApplyRange(10, false, 0, false, -99.0f),
		         static_cast<std::uint16_t>(10));
	}

	// ===========================================================================
	// The long-range bonus
	// ===========================================================================

	// Requirement 7: GETSUM_TARRANGE() + 5 applies only under the condition.
	MODERN_TEST(SkillRange_LongRangeAddsTheSumBonusPlusFive)
	{
		CHECK_EQ(SkillTargetRange(50, false, 30, false, 0.0f),
		         static_cast<std::uint16_t>(50));
		CHECK_EQ(SkillTargetRange(50, true, 30, false, 0.0f),
		         static_cast<std::uint16_t>(85)); // 50 + 30 + 5
	}

	// The flat +5 is not folded into the bonus by accident: a zero bonus still
	// contributes 5.
	MODERN_TEST(SkillRange_LongRangeStillAddsFiveWithAZeroBonus)
	{
		CHECK_EQ(SkillTargetRange(50, true, 0, false, 0.0f),
		         static_cast<std::uint16_t>(55));
		CHECK_EQ(SkillApplyRange(50, true, 0, false, 0.0f),
		         static_cast<std::uint16_t>(55));
	}

	// ===========================================================================
	// The enemy bonuses
	// ===========================================================================

	// Requirement 8: each SUM input affects only its own calculation.
	MODERN_TEST(SkillRange_EnemyBonusAppliesOnlyWhenItAffectsAnEnemy)
	{
		CHECK_EQ(SkillTargetRange(50, false, 0, false, 40.0f),
		         static_cast<std::uint16_t>(50));
		CHECK_EQ(SkillTargetRange(50, false, 0, true, 40.0f),
		         static_cast<std::uint16_t>(90));
		CHECK_EQ(SkillApplyRange(50, false, 0, false, 40.0f),
		         static_cast<std::uint16_t>(50));
		CHECK_EQ(SkillApplyRange(50, false, 0, true, 40.0f),
		         static_cast<std::uint16_t>(90));
	}

	// The two bonuses are separate parameters and do not bleed into each other.
	MODERN_TEST(SkillRange_AttackAndApplyBonusesDoNotCrossOver)
	{
		// An attack-range bonus with the apply rule is simply not an input the
		// apply rule has, so it cannot reach it: this call passes no such value.
		CHECK_EQ(SkillApplyRange(50, false, 0, true, 0.0f),
		         static_cast<std::uint16_t>(50));
		CHECK_EQ(SkillTargetRange(50, false, 0, true, 0.0f),
		         static_cast<std::uint16_t>(50));
	}

	// Both conditions together, and in legacy's order.
	MODERN_TEST(SkillRange_BothConditionsCombine)
	{
		CHECK_EQ(SkillTargetRange(50, true, 30, true, 40.0f),
		         static_cast<std::uint16_t>(125)); // 50 + 35 + 40
		CHECK_EQ(SkillApplyRange(50, true, 30, true, 40.0f),
		         static_cast<std::uint16_t>(125));
	}

	// Legacy casts the float SUM to int, which TRUNCATES toward zero rather
	// than rounding. 40.9 -> 40 and -40.9 -> -40, both of which differ from
	// rounding, so this pins the conversion.
	MODERN_TEST(SkillRange_FractionalEnemyBonusTruncatesTowardZero)
	{
		CHECK_EQ(SkillTargetRange(0, false, 0, true, 40.9f),
		         static_cast<std::uint16_t>(kLegacyTargetFloor + 40));
		CHECK_EQ(SkillApplyRange(10, false, 0, true, 40.9f),
		         static_cast<std::uint16_t>(50));
	}

	// ===========================================================================
	// Outlier preservation
	// ===========================================================================

	// Requirement 5: 60000 and 57599 pass through the calculation untouched.
	MODERN_TEST(SkillRange_ApplyRangePreservesTheVerifiedOutliers)
	{
		CHECK_EQ(SkillApplyRange(kOutlierApplyValue, false, 0, false, 0.0f),
		         static_cast<std::uint16_t>(kOutlierApplyValue));
		CHECK_EQ(SkillApplyRange(kOutlierApplyValueAlt, false, 0, false, 0.0f),
		         static_cast<std::uint16_t>(kOutlierApplyValueAlt));

		// With bonuses the value grows rather than saturating at 65535, which is
		// what a uint16 accumulator would do. Legacy accumulates in int
		// (`:894`), so a 60000 base plus a bonus is representable.
		CHECK_EQ(SkillApplyRange(kOutlierApplyValue, true, 100, true, 100.0f),
		         static_cast<std::uint16_t>(60205));
	}

	// ===========================================================================
	// Independence of the two rules
	// ===========================================================================

	// Requirement 9: the rules read different fields, so a base in one cannot
	// move the other.
	MODERN_TEST(SkillRange_TheTwoRulesAreIndependent)
	{
		const std::int32_t tar = 40;
		const std::int32_t apply = 70;

		CHECK_EQ(SkillTargetRange(tar, false, 0, false, 0.0f),
		         static_cast<std::uint16_t>(tar));
		CHECK_EQ(SkillApplyRange(apply, false, 0, false, 0.0f),
		         static_cast<std::uint16_t>(apply));

		// Moving wTARRANGE leaves the apply result alone, and vice versa.
		CHECK_EQ(SkillTargetRange(tar + 25, false, 0, false, 0.0f),
		         static_cast<std::uint16_t>(tar + 25));
		CHECK_EQ(SkillApplyRange(apply, false, 0, false, 0.0f),
		         static_cast<std::uint16_t>(apply));

		CHECK_EQ(SkillApplyRange(apply + 25, false, 0, false, 0.0f),
		         static_cast<std::uint16_t>(apply + 25));
		CHECK_EQ(SkillTargetRange(tar, false, 0, false, 0.0f),
		         static_cast<std::uint16_t>(tar));
	}

	// ===========================================================================
	// Basic-attack isolation (requirement 10)
	// ===========================================================================

	// The skill rules carry no +7 slack and no +2 term; those live only in
	// AttackService. A 40-unit target range is 40, not 40 + 7.
	MODERN_TEST(SkillRange_NoAttackSlackIsApplied)
	{
		CHECK_EQ(SkillTargetRange(40, false, 0, false, 0.0f),
		         static_cast<std::uint16_t>(40));
		CHECK_EQ(SkillApplyRange(40, false, 0, false, 0.0f),
		         static_cast<std::uint16_t>(40));
	}
} // namespace ModernTests