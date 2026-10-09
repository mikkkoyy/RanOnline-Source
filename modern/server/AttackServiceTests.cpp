// WORLD-ENTRY-002i: the ATTACK validation rule.
//
// Every case here is deterministic and pure - no sockets, no threads, no timers.
// The rule is a function of the request, the two positions and whether the target
// resolved, so each refusal is pinned by calling it with exactly the input that
// produces it.
//
// WHAT IS NOT TESTED HERE, AND WHY
//
// Peace-zone refusal is NOT implemented. Legacy's `m_pLandMan->IsPeaceZone()`
// check (GLCharMsg.cpp:330) has no modern equivalent: MapsList decodes the map
// node but SKIPS its eleven leading flags - PeaceZone among them - so there is no
// per-map predicate to call. Rather than invent one and claim the check, the rule
// omits it and this file records the omission.

#include "TestHarness.h"
#include "world/AttackService.h"

#include <limits>

namespace ModernTests
{
	using namespace Modern;
	using namespace Modern::Server::World;

	namespace
	{
		// A request naming a live player character, which is the only target type
		// the modern world can resolve.
		AttackRequest PcTarget(WireU32 id)
		{
			AttackRequest request;
			request.targetCrow = Network::Attack::kCrowPc;
			request.targetId   = id;
			request.aniSel     = 3;
			return request;
		}

		TargetView ExistingAt(float x, float y, float z)
		{
			TargetView target;
			target.exists  = true;
			target.position = Vector3{ x, y, z };
			return target;
		}

		TargetView Absent()
		{
			return TargetView{};
		}

		const AttackService kRule;
	} // namespace

	// ---- the prototype range --------------------------------------------------

	MODERN_TEST(AttackService_TheRangeIsThePrototypePlusTheVerifiedLegacySlack)
	{
		// The `+ 7` is legacy's (GLCharMsg.cpp:347) and is carried exactly. The rest
		// stands in for item-derived terms that are not available - see the header.
		CHECK(kLegacyRangeSlackUnits == 7.0f);
		CHECK(AttackService::AllowedDistance() ==
		      kPrototypeAttackableDistanceUnits + kLegacyRangeSlackUnits);
		CHECK(AttackService::AllowedDistance() > 0.0f);
	}

	// ---- acceptance -----------------------------------------------------------

	MODERN_TEST(AttackService_AnInRangeAttackIsAccepted)
	{
		const float allowed = AttackService::AllowedDistance();

		const AttackResult result =
		    kRule.Evaluate(/*attackerGaeaId=*/7u, /*spawned=*/true, Vector3{ 0.0f, 0.0f, 0.0f },
		                   PcTarget(9u), ExistingAt(allowed * 0.5f, 0.0f, 0.0f));

		CHECK_EQ(static_cast<int>(result.outcome),
		         static_cast<int>(AttackOutcome::Accepted));
		CHECK(result.accepted);

		// An accepted attack is NOT an announced refusal.
		CHECK(!result.announced);

		// The id on the wire is always the ATTACKER's.
		CHECK_EQ(result.attackerGaeaId, 7u);

		// The target is echoed, never substituted.
		CHECK_EQ(result.targetId, 9u);
		CHECK_EQ(result.targetCrow, Network::Attack::kCrowPc);
		CHECK_EQ(result.aniSel, 3u);
	}

	MODERN_TEST(AttackService_ExactlyTheAllowedDistanceIsAccepted)
	{
		// Legacy is `if (fDist > wAttackAbleDis)`, so the boundary itself is IN
		// range. Getting this the wrong way round would make the rule refuse
		// attacks a real client could legitimately make.
		const float allowed = AttackService::AllowedDistance();

		const AttackResult result =
		    kRule.Evaluate(1u, true, Vector3{ 0.0f, 0.0f, 0.0f }, PcTarget(2u),
		                   ExistingAt(allowed, 0.0f, 0.0f));

		CHECK(result.accepted);
		CHECK(result.distance == allowed);
	}

	MODERN_TEST(AttackService_ADistanceJustOverTheLimitIsRefused)
	{
		const float allowed = AttackService::AllowedDistance();

		const AttackResult result =
		    kRule.Evaluate(1u, true, Vector3{ 0.0f, 0.0f, 0.0f }, PcTarget(2u),
		                   ExistingAt(allowed + 0.5f, 0.0f, 0.0f));

		CHECK(!result.accepted);
		CHECK_EQ(static_cast<int>(result.outcome),
		         static_cast<int>(AttackOutcome::RefusedOutOfRange));

		// The ONLY refusal that reaches the wire.
		CHECK(result.announced);

		// The detail is populated, so an operator reading the log learns why.
		CHECK(!result.detail.empty());
	}

	MODERN_TEST(AttackService_DistanceIsMeasuredInThreeDimensions)
	{
		// GLCharMsg.cpp:344 is `D3DXVec3Length(&D3DXVECTOR3(m_vPos - vTarPos))` - a
		// full 3D length, not a horizontal one. A target directly overhead is
		// therefore in range, which a 2D distance would have refused.
		const float allowed = AttackService::AllowedDistance();

		const AttackResult straightUp =
		    kRule.Evaluate(1u, true, Vector3{ 0.0f, 0.0f, 0.0f }, PcTarget(2u),
		                   ExistingAt(0.0f, 0.0f, allowed * 0.5f));
		CHECK(straightUp.accepted);

		const AttackResult diagonal =
		    kRule.Evaluate(1u, true, Vector3{ 0.0f, 0.0f, 0.0f }, PcTarget(2u),
		                   ExistingAt(allowed, allowed, allowed));
		CHECK(!diagonal.accepted);
		CHECK(diagonal.distance > allowed);
	}

	// ---- refusals -------------------------------------------------------------

	MODERN_TEST(AttackService_AnUnknownTargetIsRefusedSilently)
	{
		// Legacy's `GetTarget(...)` returning NULL is GLCharMsg.cpp:340's E_FAIL,
		// which sends NOTHING. So this refusal must NOT be announced, or the client
		// would receive a 3041 legacy never sends.
		const AttackResult result =
		    kRule.Evaluate(1u, true, Vector3{ 0.0f, 0.0f, 0.0f }, PcTarget(4242u), Absent());

		CHECK(!result.accepted);
		CHECK_EQ(static_cast<int>(result.outcome),
		         static_cast<int>(AttackOutcome::RefusedUnknownTarget));
		CHECK(!result.announced);
	}

	MODERN_TEST(AttackService_AZeroTargetIdIsRefusedAndDistinguishedFromAnUnknownOne)
	{
		const AttackResult zero =
		    kRule.Evaluate(1u, true, Vector3{ 0.0f, 0.0f, 0.0f }, PcTarget(0u), Absent());

		CHECK(!zero.accepted);
		CHECK_EQ(static_cast<int>(zero.outcome),
		         static_cast<int>(AttackOutcome::RefusedNoTargetId));
		CHECK(!zero.announced);

		// Zero is refused even if a target somehow claims to exist at id 0 - the id
		// check comes first, so an id of 0 can never be talked into resolving.
		const AttackResult zeroButPresent =
		    kRule.Evaluate(1u, true, Vector3{ 0.0f, 0.0f, 0.0f }, PcTarget(0u),
		                   ExistingAt(0.0f, 0.0f, 0.0f));
		CHECK_EQ(static_cast<int>(zeroButPresent.outcome),
		         static_cast<int>(AttackOutcome::RefusedNoTargetId));
	}

	MODERN_TEST(AttackService_AMobTargetIsRefusedBecauseNoMobExistsYet)
	{
		// The codec carries CROW_MOB faithfully - the wire format is legacy's and
		// does not get to second-guess it - but the RULE refuses it, because the
		// modern world has no NPC or mob entity to resolve one to.
		for (WireU32 crow : { Network::Attack::kCrowMob, Network::Attack::kCrowNpc })
		{
			AttackRequest request = PcTarget(5u);
			request.targetCrow    = crow;

			const AttackResult result =
			    kRule.Evaluate(1u, true, Vector3{ 0.0f, 0.0f, 0.0f }, request,
			                   ExistingAt(0.0f, 0.0f, 0.0f));

			CHECK(!result.accepted);
			CHECK_EQ(static_cast<int>(result.outcome),
			         static_cast<int>(AttackOutcome::RefusedUnsupportedTargetCrow));
			CHECK(!result.announced);

			// The crow the client sent is echoed, never rewritten to CROW_PC.
			CHECK_EQ(result.targetCrow, crow);
		}
	}

	MODERN_TEST(AttackService_AnUnspawnedAttackerIsRefused)
	{
		const AttackResult result =
		    kRule.Evaluate(0u, /*spawned=*/false, Vector3{ 0.0f, 0.0f, 0.0f }, PcTarget(9u),
		                   ExistingAt(0.0f, 0.0f, 0.0f));

		CHECK(!result.accepted);
		CHECK_EQ(static_cast<int>(result.outcome),
		         static_cast<int>(AttackOutcome::RefusedNotSpawned));
		CHECK(!result.announced);
	}

	// ---- ordering -------------------------------------------------------------

	MODERN_TEST(AttackService_TheRefusalOrderIsTheLegacyOrder)
	{
		// Legacy checks, in order: spawn precondition, peace zone, target lookup,
		// then range. Giving the rule more than one wrong input at a time and
		// pinning WHICH refusal comes back is what proves the order rather than
		// just the presence of the checks.
		//
		// An unspawned attacker with an unresolvable target and a zero id is
		// RefusedNotSpawned, not the id or the target - so spawn is checked first.
		AttackRequest zero = PcTarget(0u);
		const AttackResult result =
		    kRule.Evaluate(0u, false, Vector3{}, zero, Absent());
		CHECK_EQ(static_cast<int>(result.outcome),
		         static_cast<int>(AttackOutcome::RefusedNotSpawned));

		// Spawned, but a mob with a zero id: the crowd check precedes the id check,
		// matching legacy where the target lookup precedes any use of the id.
		AttackRequest mobZero = PcTarget(0u);
		mobZero.targetCrow    = Network::Attack::kCrowMob;
		const AttackResult mobResult =
		    kRule.Evaluate(1u, true, Vector3{}, mobZero, Absent());
		CHECK_EQ(static_cast<int>(mobResult.outcome),
		         static_cast<int>(AttackOutcome::RefusedUnsupportedTargetCrow));

		// Spawned, valid crowd, zero id: now the id is what fails.
		const AttackResult idResult =
		    kRule.Evaluate(1u, true, Vector3{}, zero, Absent());
		CHECK_EQ(static_cast<int>(idResult.outcome),
		         static_cast<int>(AttackOutcome::RefusedNoTargetId));
	}

	// ---- the range comparison is NaN-safe ------------------------------------

	MODERN_TEST(AttackService_ANonFiniteDistanceIsRefusedRatherThanAccepted)
	{
		// The comparison is written `!(distance <= allowed)`, not `distance >
		// allowed`. With a NaN distance the second form is false, so a NaN would be
		// ACCEPTED - turning an impossible position into a free hit. This is the
		// same trap GotoProtocol's 60-unit rule guards against for vCurPos.
		const float nan = std::numeric_limits<float>::quiet_NaN();

		const AttackResult result =
		    kRule.Evaluate(1u, true, Vector3{ 0.0f, 0.0f, 0.0f }, PcTarget(2u),
		                   ExistingAt(nan, nan, nan));

		CHECK(!result.accepted);
		CHECK_EQ(static_cast<int>(result.outcome),
		         static_cast<int>(AttackOutcome::RefusedOutOfRange));
	}

	// ---- the rule is pure -----------------------------------------------------

	MODERN_TEST(AttackService_EvaluatingTwiceGivesTheSameAnswer)
	{
		// No hidden state and no accumulation: the same inputs must produce the
		// same decision, or a retry would differ from a first attempt.
		const float allowed = AttackService::AllowedDistance();

		AttackRequest request = PcTarget(9u);
		TargetView    target  = ExistingAt(allowed * 2.0f, 0.0f, 0.0f);

		const AttackResult first =
		    kRule.Evaluate(3u, true, Vector3{ 0.0f, 0.0f, 0.0f }, request, target);
		const AttackResult second =
		    kRule.Evaluate(3u, true, Vector3{ 0.0f, 0.0f, 0.0f }, request, target);

		CHECK_EQ(static_cast<int>(first.outcome), static_cast<int>(second.outcome));
		CHECK_EQ(first.accepted, second.accepted);
		CHECK_EQ(first.announced, second.announced);
		CHECK(first.distance == second.distance);
	}
} // namespace ModernTests