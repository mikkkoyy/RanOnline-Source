// WORLD-ENTRY-002k: the damage resolution boundary.
//
// Every case is deterministic and pure - no socket, no thread, no clock, and no
// RNG: every roll is a parameter, exactly as legacy's RANDOM_POS is consumed at
// GLChar.cpp:2416 rather than generated inside the rule.
//
// The arithmetic under test belongs to core and is covered there. What these cases
// pin is the BOUNDARY: which inputs are refused, how a miss is distinguished from
// a hit, and that the applied figure is NEVER the resolution's to choose.

#include "TestHarness.h"
#include "world/DamageResolution.h"

#include <limits>

namespace ModernTests
{
	using namespace Modern;
	using namespace Modern::Server::World;

	namespace
	{
		// A valid input whose roll of 0.0 always HITS: the hit rate is clamped to
		// [20,99] and 0*100 is 0, so the roll can never exceed it.
		DamageInput HitInput()
		{
			DamageInput input;
			input.attackerGaeaId = 1u;
			input.targetGaeaId   = 2u;
			input.targetCrow     = Network::Attack::kCrowPc;
			input.hitRoll        = 0.0f;
			input.damageRoll     = 0.0f;
			input.criticalRoll   = 0.99f; // never crits
			input.crushingRoll   = 0.99f; // never crushes
			input.reflectionRoll = 0.99f;
			return input;
		}

		// The same, with a roll that cannot succeed.
		DamageInput MissInput()
		{
			// The roll ALONE does not decide this. HitCalculator converts it to an
			// integer percentage by TRUNCATION before comparing - (uint32_t)(0.999f *
			// 100.0f) is 99 - and the comparison is hitRate >= roll, so a 99% rate
			// HITS a roll of 0.999. That is legacy's own shape (GLogixExPC.cpp:1322-1327),
			// and it is why the hit/avoid pair has to move as well as the roll: only
			// a rate below the truncated roll can actually miss.
			DamageInput input = HitInput();
			input.stats.avoid = 300; // 100 + 30 - 300 clamps to the 20 floor
			input.hitRoll     = 0.999f; // truncates to 99, comfortably above 20
			return input;
		}
	}

	// ---- acceptance ----------------------------------------------------------

	MODERN_TEST(DamageResolution_AHittingAttackProducesAPositiveDamageFigure)
	{
		const DamageResult result =
		    DamageResolution::Resolve(true, true, HitInput());

		CHECK_EQ(static_cast<int>(result.outcome), static_cast<int>(DamageOutcome::Hit));
		CHECK(result.requestedDamage > 0u);

		// The roll is reported so a test can assert the RATE rather than infer the
		// outcome from whether it happened to hit.
		CHECK(result.hitRate >= 20u);
		CHECK(result.hitRate <= 99u);
	}

	MODERN_TEST(DamageResolution_AppliedDamageStartsUnapplied)
	{
		// The resolution must NOT decide what was removed from HP. It reports what it
		// ASKED for and leaves the applied figure unset, so a caller that forgets to
		// report back cannot accidentally ship the requested number as though it
		// were authoritative.
		const DamageResult result =
		    DamageResolution::Resolve(true, true, HitInput());

		CHECK_EQ(result.appliedDamage, DamageResult::kUnapplied);
		CHECK(result.requestedDamage != DamageResult::kUnapplied);
	}

	// ---- avoidance -----------------------------------------------------------

	MODERN_TEST(DamageResolution_AMissedAttackIsAvoidedNotStruck)
	{
		const DamageResult result = DamageResolution::Resolve(true, true, MissInput());

		CHECK_EQ(static_cast<int>(result.outcome),
		         static_cast<int>(DamageOutcome::Avoided));

		// A miss reports NO damage. If it reported a figure the caller would have to
		// remember to ignore it, and a zero would be ambiguous with a real zero.
		CHECK_EQ(result.requestedDamage, 0u);
		CHECK_EQ(result.appliedDamage, DamageResult::kUnapplied);
	}

	MODERN_TEST(DamageResolution_AMissStillReportsTheRateItRolledAgainst)
	{
		// The roll happened and its outcome is worth knowing even when it missed -
		// it is the only observable difference between "unlucky" and "broken".
		const DamageResult result = DamageResolution::Resolve(true, true, MissInput());
		CHECK(result.hitRate >= 20u);
		CHECK(result.hitRate <= 99u);
	}

	// ---- refusals ------------------------------------------------------------

	MODERN_TEST(DamageResolution_AnAbsentPartyIsRefusedBeforeAnythingIsRolled)
	{
		const DamageResult noAttacker =
		    DamageResolution::Resolve(/*attackerPresent=*/false, true, HitInput());
		CHECK_EQ(static_cast<int>(noAttacker.outcome),
		         static_cast<int>(DamageOutcome::Refused));
		CHECK(!noAttacker.detail.empty());
		CHECK_EQ(noAttacker.requestedDamage, 0u);

		const DamageResult noTarget =
		    DamageResolution::Resolve(true, /*targetPresent=*/false, HitInput());
		CHECK_EQ(static_cast<int>(noTarget.outcome),
		         static_cast<int>(DamageOutcome::Refused));
		CHECK(!noTarget.detail.empty());

		// Attacker is checked FIRST, so a caller missing both learns that one.
		const DamageResult neither =
		    DamageResolution::Resolve(false, false, HitInput());
		CHECK_EQ(static_cast<int>(neither.outcome),
		         static_cast<int>(DamageOutcome::Refused));
		CHECK(neither.detail.find("attacker") != std::string::npos);
	}

	MODERN_TEST(DamageResolution_AZeroGaeaIdIsRefused)
	{
		// A gaeaId of 0 is never a real identity, and refusing it here means the
		// rule does not depend on the caller having filtered it.
		DamageInput noAttackerId = HitInput();
		noAttackerId.attackerGaeaId = 0;
		CHECK_EQ(static_cast<int>(DamageResolution::Resolve(true, true, noAttackerId).outcome),
		         static_cast<int>(DamageOutcome::Refused));

		DamageInput noTargetId = HitInput();
		noTargetId.targetGaeaId = 0;
		CHECK_EQ(static_cast<int>(DamageResolution::Resolve(true, true, noTargetId).outcome),
		         static_cast<int>(DamageOutcome::Refused));
	}

	// The roll is TRUNCATED to an integer percentage before it is compared.
	//
	// HitCalculator computes (uint32_t)(hitRoll * 100.0f), so 0.999 becomes 99
	// rather than 99.9, and the comparison hitRate >= roll then passes. Legacy
	// has the same shape (GLogixExPC.cpp:1322-1327). The consequence is worth
	// pinning rather than leaving to be rediscovered: a client that reads 0.999 as
	// "0.1% away from hitting" is wrong, and the difference only shows up at the
	// very top of the range.
	MODERN_TEST(DamageResolution_TheRollIsTruncatedSoNinetyNinePercentAlwaysHits)
	{
		DamageInput input = HitInput(); // hit 30 / avoid 30 -> 100, clamped to 99
		input.hitRoll     = 0.999f;

		const DamageResult result = DamageResolution::Resolve(true, true, input);

		// 99 >= 99, so this HITS. It is not a 0.1% chance; it is certain.
		CHECK_EQ(static_cast<int>(result.outcome), static_cast<int>(DamageOutcome::Hit));
		CHECK_EQ(result.hitRate, 99u);

		// And a roll one step lower truncates to the same 99, so it hits too.
		DamageInput highRoll = HitInput();
		highRoll.hitRoll = 0.99f;
		CHECK_EQ(static_cast<int>(DamageResolution::Resolve(true, true, highRoll).outcome),
		         static_cast<int>(DamageOutcome::Hit));
	}

	MODERN_TEST(DamageResolution_ABrokenRollIsRefusedRatherThanBecomingAMiss)
	{
		// The important distinction. Legacy's comparison is
		// `nHitRate >= RANDOM_POS*100` (GLogixExPC.cpp:1327), which is FALSE for a
		// NaN roll - so a broken caller would quietly look like an unlucky one. A
		// roll outside [0,1] is a caller fault and is reported as one.
		const float nan = std::numeric_limits<float>::quiet_NaN();

		for (float bad : { nan, -0.1f, 1.1f,
			               std::numeric_limits<float>::infinity() })
		{
			DamageInput input = HitInput();
			input.hitRoll     = bad;

			const DamageResult result = DamageResolution::Resolve(true, true, input);
			CHECK_EQ(static_cast<int>(result.outcome),
			         static_cast<int>(DamageOutcome::Refused));
		}
	}

	MODERN_TEST(DamageResolution_ANonFiniteSecondaryRollIsAlsoRefused)
	{
		// The damage roll is not the hit roll, and a broken one must not slip past
		// just because the hit was fine.
		DamageInput input   = HitInput();
		input.damageRoll    = std::numeric_limits<float>::quiet_NaN();

		CHECK_EQ(static_cast<int>(DamageResolution::Resolve(true, true, input).outcome),
		         static_cast<int>(DamageOutcome::Refused));
	}

	MODERN_TEST(DamageResolution_AnInvertedDamageRangeIsRefused)
	{
		// RandomDamageRange interpolates `low + (high-low)*roll`. With high below
		// low that runs BACKWARDS, so the range must be checked rather than trusted.
		DamageInput input          = HitInput();
		input.stats.lowDamage      = 50u;
		input.stats.highDamage     = 10u;

		const DamageResult result = DamageResolution::Resolve(true, true, input);
		CHECK_EQ(static_cast<int>(result.outcome),
		         static_cast<int>(DamageOutcome::Refused));
		CHECK(result.detail.find("inverted") != std::string::npos);
	}

	// ---- the prototype inputs are honest -------------------------------------

	MODERN_TEST(DamageResolution_ThePrototypeRangeRollsBetweenItsEndpoints)
	{
		// The figure must move with the damage roll and stay inside the range, which
		// is the observable difference between "the roll is wired up" and "the roll is
		// ignored". Exact parity is NOT claimed - the range endpoints are prototype
		// values, not recovered RAN ones.
		DamageInput low = HitInput();
		low.damageRoll  = 0.0f;
		const DamageResult atLow = DamageResolution::Resolve(true, true, low);

		DamageInput high          = HitInput();
		high.damageRoll           = 0.999f;
		const DamageResult atHigh = DamageResolution::Resolve(true, true, high);

		REQUIRE(atLow.outcome == DamageOutcome::Hit);
		REQUIRE(atHigh.outcome == DamageOutcome::Hit);

		CHECK(atHigh.requestedDamage >= atLow.requestedDamage);
		CHECK(atLow.requestedDamage >= kPrototypeLowDamage);
		CHECK(atLow.requestedDamage <= kPrototypeHighDamage);
	}

	MODERN_TEST(DamageResolution_ThePrototypeHitPairIsRANsOwnPlaceholder)
	{
		// m_nSUM_HIT = m_nSUM_AVOID = 30 is RAN's own stand-in before equipment is
		// aggregated (GLogicExNPC.cpp:722, GLogixExPC.cpp:2525). With it the hit
		// rate starts at the legacy midpoint of 100 + 30 - 30 = 100, clamped to 99.
		CHECK_EQ(kPrototypeHit, 30);
		CHECK_EQ(kPrototypeAvoid, 30);

		DamageInput input  = HitInput();
		input.stats.hit    = kPrototypeHit;
		input.stats.avoid  = kPrototypeAvoid;

		const DamageResult result = DamageResolution::Resolve(true, true, input);
		CHECK_EQ(result.hitRate, 99u); // 100 + 0, clamped
	}

	// ---- purity --------------------------------------------------------------

	MODERN_TEST(DamageResolution_EvaluatingTwiceGivesTheSameAnswer)
	{
		// No hidden state and no accumulation: the same inputs must produce the same
		// decision, or a retry would differ from a first attempt and a test could not
		// assert anything about it.
		const DamageInput input = HitInput();

		const DamageResult first  = DamageResolution::Resolve(true, true, input);
		const DamageResult second = DamageResolution::Resolve(true, true, input);

		CHECK_EQ(static_cast<int>(first.outcome), static_cast<int>(second.outcome));
		CHECK_EQ(first.requestedDamage, second.requestedDamage);
		CHECK_EQ(first.appliedDamage, second.appliedDamage);
		CHECK_EQ(first.hitRate, second.hitRate);
		CHECK_EQ(first.damageFlag, second.damageFlag);
	}

	MODERN_TEST(DamageResolution_ChangingTheRangeChangesTheDamage)
	{
		// Guards against the resolution ignoring its inputs and returning a constant.
		DamageInput weak          = HitInput();
		weak.stats.lowDamage      = 1u;
		weak.stats.highDamage     = 2u;
		DamageInput strong         = HitInput();
		strong.stats.lowDamage    = 100u;
		strong.stats.highDamage   = 200u;

		const DamageResult weakResult   = DamageResolution::Resolve(true, true, weak);
		const DamageResult strongResult = DamageResolution::Resolve(true, true, strong);

		REQUIRE(weakResult.outcome == DamageOutcome::Hit);
		REQUIRE(strongResult.outcome == DamageOutcome::Hit);
		CHECK(strongResult.requestedDamage > weakResult.requestedDamage);
	}
} // namespace ModernTests
