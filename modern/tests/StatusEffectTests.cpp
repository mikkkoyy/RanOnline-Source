#include "TestHarness.h"

#include "status/StatusEffectTypes.h"
#include "status/StatusEffectResolver.h"
#include "status/StatusEffectContainer.h"

namespace
{
	using namespace Modern;
	using namespace Modern::StatusEffect;

	// A skill blow: 30% rate, 10s life, same-level attacker and target.
	StatusApplicationInput MakeBlow(StatusEffectType type = StatusEffectType::Stun)
	{
		StatusApplicationInput input;
		input.type          = type;
		input.actRate       = 30.0f;
		input.lifetime      = 10.0f;
		input.weatherPower  = 1.0f;
		input.attackerLevel = 10;
		input.targetLevel   = 10;
		input.randomRoll    = 0.0f;   // always beats the threshold
		return input;
	}

	StatusEffectState MakeState(StatusEffectType type, float lifetime)
	{
		StatusEffectState state;
		state.type              = type;
		state.remainingLifetime = lifetime;
		return state;
	}
}

// ═══════════════════════════════════════════════════════════════════════
// Type mapping
// ═══════════════════════════════════════════════════════════════════════

// The numeric values are load-bearing: SlotFor subtracts from Frozen. If these
// ever drift, states silently land in the wrong slots.
MODERN_TEST(StatusEffect_LegacyNumericValuesArePreserved)
{
	CHECK_EQ(static_cast<uint8_t>(StatusEffectType::None),   0u);
	CHECK_EQ(static_cast<uint8_t>(StatusEffectType::Numb),   1u);
	CHECK_EQ(static_cast<uint8_t>(StatusEffectType::Stun),   2u);
	CHECK_EQ(static_cast<uint8_t>(StatusEffectType::Stone),  3u);
	CHECK_EQ(static_cast<uint8_t>(StatusEffectType::Burn),   4u);
	CHECK_EQ(static_cast<uint8_t>(StatusEffectType::Frozen), 5u);
	CHECK_EQ(static_cast<uint8_t>(StatusEffectType::Mad),    6u);
	CHECK_EQ(static_cast<uint8_t>(StatusEffectType::Poison), 7u);
	CHECK_EQ(static_cast<uint8_t>(StatusEffectType::Curse),  8u);
}

// GLCharDefine.h:925-940, STATE_TO_DISORDER.
MODERN_TEST(StatusEffect_StateToDisorderMapping)
{
	CHECK_EQ(DisorderFor(StatusEffectType::Numb),   DisorderNumb);
	CHECK_EQ(DisorderFor(StatusEffectType::Stun),   DisorderStun);
	CHECK_EQ(DisorderFor(StatusEffectType::Stone),  DisorderStone);
	CHECK_EQ(DisorderFor(StatusEffectType::Burn),   DisorderBurn);
	CHECK_EQ(DisorderFor(StatusEffectType::Frozen), DisorderFrozen);
	CHECK_EQ(DisorderFor(StatusEffectType::Mad),    DisorderMad);
	CHECK_EQ(DisorderFor(StatusEffectType::Poison), DisorderPoison);
	CHECK_EQ(DisorderFor(StatusEffectType::Curse),  DisorderCurse);
	CHECK_EQ(DisorderFor(StatusEffectType::None),   DisorderNone);

	// Each state is a distinct bit, and DIS_ALL covers all of them.
	CHECK_EQ(DisorderAll,
	         DisorderNumb | DisorderStun | DisorderStone | DisorderBurn |
	         DisorderFrozen | DisorderMad | DisorderPoison | DisorderCurse);
}

// GLCharDefine.h:942-957, STATE_TO_ELEMENT.
MODERN_TEST(StatusEffect_StateToElementMapping)
{
	CHECK_EQ(ElementFor(StatusEffectType::Numb),   BlowElement::Electric);
	CHECK_EQ(ElementFor(StatusEffectType::Stun),   BlowElement::Stun);
	CHECK_EQ(ElementFor(StatusEffectType::Stone),  BlowElement::Stone);
	CHECK_EQ(ElementFor(StatusEffectType::Burn),   BlowElement::Fire);
	CHECK_EQ(ElementFor(StatusEffectType::Frozen), BlowElement::Ice);
	CHECK_EQ(ElementFor(StatusEffectType::Mad),    BlowElement::Mad);
	CHECK_EQ(ElementFor(StatusEffectType::Poison), BlowElement::Poison);
	CHECK_EQ(ElementFor(StatusEffectType::Curse),  BlowElement::Curse);
	// The legacy switch falls through to Spirit.
	CHECK_EQ(ElementFor(StatusEffectType::None),   BlowElement::Spirit);
}

// ═══════════════════════════════════════════════════════════════════════
// Slot mapping - GLChar.cpp:6204-6205
// ═══════════════════════════════════════════════════════════════════════

MODERN_TEST(StatusEffect_SingleSlotStatesShareSlotZero)
{
	// Everything at or below EMBLOW_SINGLE (5) is slot 0.
	CHECK_EQ(SlotFor(StatusEffectType::Numb),   0u);
	CHECK_EQ(SlotFor(StatusEffectType::Stun),   0u);
	CHECK_EQ(SlotFor(StatusEffectType::Stone),  0u);
	CHECK_EQ(SlotFor(StatusEffectType::Burn),   0u);
	CHECK_EQ(SlotFor(StatusEffectType::Frozen), 0u);
}

MODERN_TEST(StatusEffect_MadPoisonCurseOccupyTheirOwnSlots)
{
	CHECK_EQ(SlotFor(StatusEffectType::Mad),    1u);
	CHECK_EQ(SlotFor(StatusEffectType::Poison), 2u);
	CHECK_EQ(SlotFor(StatusEffectType::Curse),  3u);
}

MODERN_TEST(StatusEffect_NoneOccupiesNoSlot)
{
	CHECK_GE(SlotFor(StatusEffectType::None), kStatusSlotCount);
}

// ═══════════════════════════════════════════════════════════════════════
// Application probability - CHECKSTATEBLOW
// ═══════════════════════════════════════════════════════════════════════

MODERN_TEST(StatusEffect_NoneIsRefusedWithoutTouchingTheRoll)
{
	StatusApplicationInput input = MakeBlow(StatusEffectType::None);

	const StatusApplicationResult result = ResolveStatusApplication(input);

	CHECK(!result.Applied());
	CHECK_EQ(result.refusal, StatusRefusal::NoBlowType);
}

// The threshold is exposed so the formula is testable, not just the outcome.
MODERN_TEST(StatusEffect_SameLevelUsesTheMiddleOfTheLevelTable)
{
	// nDXLEVEL = target - attacker = 0, so index = 0 + 1 = 1.
	// nSTATEBLOW_LEVEL[1] = +8.
	// threshold = 30 - 30*0.01*0*0.6 + 8 = 38.
	StatusApplicationInput input = MakeBlow();
	input.attackerLevel = 10;
	input.targetLevel   = 10;

	const StatusApplicationResult result = ResolveStatusApplication(input);

	CHECK_EQ(result.levelIndex, 1);
	CHECK_EQ(result.levelModifier, 8.0f);
	CHECK_EQ(result.threshold, 38.0f);
}

// The level term is a level DIFFERENCE, and it favours the attacker:
// attacker above target -> positive modifier.
MODERN_TEST(StatusEffect_AttackerAboveTargetRaisesTheThreshold)
{
	StatusApplicationInput higher = MakeBlow();
	higher.attackerLevel = 12;
	higher.targetLevel   = 10;

	const StatusApplicationResult result = ResolveStatusApplication(higher);

	// nDXLEVEL = -2, index = -1 -> clamped to 0 -> +10.
	CHECK_EQ(result.levelIndex, 0);
	CHECK_EQ(result.levelModifier, 10.0f);
	CHECK_EQ(result.threshold, 40.0f);
}

MODERN_TEST(StatusEffect_TargetAboveTargetLowersTheThreshold)
{
	StatusApplicationInput lower = MakeBlow();
	lower.attackerLevel = 8;
	lower.targetLevel   = 10;

	const StatusApplicationResult result = ResolveStatusApplication(lower);

	// nDXLEVEL = +2, index = 3 -> +3.
	CHECK_EQ(result.levelIndex, 3);
	CHECK_EQ(result.levelModifier, 3.0f);
	CHECK_EQ(result.threshold, 33.0f);
}

MODERN_TEST(StatusEffect_LevelIndexIsClampedAtBothEnds)
{
	// Far below: clamps to index 0.
	CHECK_EQ(ComputeStatusLevelIndex(100, 1), 0);
	// Far above: clamps to index 9 (-10).
	CHECK_EQ(ComputeStatusLevelIndex(1, 100), 9);
	// The boundary values themselves.
	CHECK_EQ(ComputeStatusLevelIndex(10, 9),  0);
	CHECK_EQ(ComputeStatusLevelIndex(10, 18), 9);
	CHECK_EQ(ComputeStatusLevelIndex(10, 10), 1);
}

MODERN_TEST(StatusEffect_ThresholdUsesTheWholeLevelTable)
{
	// Every table entry is reachable and in the documented order.
	const float expected[10] = { 10.0f, 8.0f, 6.0f, 3.0f, 0.0f,
	                             -2.0f, -4.0f, -6.0f, -8.0f, -10.0f };
	for (int i = 0; i < 10; ++i)
	{
		CHECK_EQ(static_cast<float>(kStateBlowLevel[i]), expected[i]);
	}
}

// Below the threshold -> applied; at/above -> refused. The comparison direction
// is legacy's: (roll*100) < threshold, strict.
MODERN_TEST(StatusEffect_RollBelowThresholdApplies)
{
	StatusApplicationInput input = MakeBlow();
	input.randomRoll = 0.37f;    // 37 < 38

	const StatusApplicationResult result = ResolveStatusApplication(input);

	CHECK(result.Applied());
	CHECK_EQ(result.refusal, StatusRefusal::None);
}

MODERN_TEST(StatusEffect_RollExactlyAtThresholdIsRefused)
{
	StatusApplicationInput input = MakeBlow();
	input.randomRoll = 0.38f;    // 38 is not < 38

	const StatusApplicationResult result = ResolveStatusApplication(input);

	CHECK(!result.Applied());
	CHECK_EQ(result.refusal, StatusRefusal::Probability);
}

MODERN_TEST(StatusEffect_RollAboveThresholdIsRefused)
{
	StatusApplicationInput input = MakeBlow();
	input.randomRoll = 0.99f;

	const StatusApplicationResult result = ResolveStatusApplication(input);

	CHECK(!result.Applied());
	CHECK_EQ(result.refusal, StatusRefusal::Probability);
}

// The weather power multiplies the rate at the call site (GLChar.cpp:3378).
MODERN_TEST(StatusEffect_WeatherPowerScalesTheEffectiveRate)
{
	StatusApplicationInput input = MakeBlow();
	input.actRate      = 30.0f;
	input.weatherPower = 0.5f;

	const StatusApplicationResult result = ResolveStatusApplication(input);

	CHECK_EQ(result.effectiveRate, 15.0f);
	// 15 - 0 + 8
	CHECK_EQ(result.threshold, 23.0f);
}

// ═══════════════════════════════════════════════════════════════════════
// Resistance
// ═══════════════════════════════════════════════════════════════════════

// VERIFIED LEGACY BEHAVIOUR, reproduced on purpose.
//
// GLChar.cpp:3369 clamps nBLOWRESIST against fRESIST_G (0.5f) rather than
// fMAX_RESIST (99.0f). Assigning 0.5f to a short truncates it to 0, so any
// resistance of 1 or more becomes 0 and stops affecting the threshold. This
// test pins that, because it is what RAN does and it is not obvious.
MODERN_TEST(StatusEffect_LegacyResistClampZeroesRealResistance)
{
	StatusConstants constants;   // resistClampCeiling defaults to 0.5f

	CHECK_EQ(constants.resistClampCeiling, 0.5f);
	CHECK_EQ(ClampStatusResist(0, constants), 0);
	CHECK_EQ(ClampStatusResist(1, constants), 0);
	CHECK_EQ(ClampStatusResist(50, constants), 0);
	CHECK_EQ(ClampStatusResist(99, constants), 0);
	CHECK_EQ(ClampStatusResist(100000, constants), 0);

	// So a resisted and an unresisted blow have the same threshold.
	StatusApplicationInput plain = MakeBlow();
	plain.targetResist = 0;
	StatusApplicationInput resisted = MakeBlow();
	resisted.targetResist = 50;

	CHECK_EQ(ResolveStatusApplication(plain).threshold,
	         ResolveStatusApplication(resisted).threshold);
}

// The clamp ceiling is a parameter, so the intended fMAX_RESIST behaviour is
// reachable without touching the rule. Under it resistance reduces both the
// threshold and the duration, which is what the formula in GLChar.cpp:3378 and
// :3387 is plainly for.
MODERN_TEST(StatusEffect_WithIntendedClampResistanceReducesThreshold)
{
	StatusConstants constants;
	constants.resistClampCeiling = StatusConstants::kIntendedResistClamp;

	CHECK_EQ(ClampStatusResist(50, constants), 50);
	CHECK_EQ(ClampStatusResist(99, constants), 99);
	// Above fMAX_RESIST it still clamps.
	CHECK_EQ(ClampStatusResist(500, constants), 99);

	StatusApplicationInput plain = MakeBlow();
	plain.targetResist = 0;
	StatusApplicationInput resisted = plain;
	resisted.targetResist = 50;

	const StatusApplicationResult plainResult    = ResolveStatusApplication(plain, constants);
	const StatusApplicationResult resistedResult = ResolveStatusApplication(resisted, constants);

	// 30 - 30*0.01*50*0.6 + 8 = 38 - 9 = 29
	CHECK_EQ(resistedResult.threshold, 29.0f);
	CHECK_LT(resistedResult.threshold, plainResult.threshold);
}

// ═══════════════════════════════════════════════════════════════════════
// Immunity - GLChar.cpp:3376
// ═══════════════════════════════════════════════════════════════════════

// A target holding the disorder is skipped BEFORE the probability check, so the
// roll is not consulted and the outcome does not depend on it.
MODERN_TEST(StatusEffect_ImmuneTargetIsRefusedRegardlessOfRoll)
{
	StatusApplicationInput immune = MakeBlow(StatusEffectType::Stun);
	immune.targetDisorderMask = static_cast<uint32_t>(DisorderStun);
	immune.randomRoll = 0.0f;

	const StatusApplicationResult result = ResolveStatusApplication(immune);

	CHECK(!result.Applied());
	CHECK_EQ(result.refusal, StatusRefusal::TargetImmune);

	// Even a roll that would otherwise always win is refused.
	immune.randomRoll = 0.0f;
	CHECK_EQ(ResolveStatusApplication(immune).refusal, StatusRefusal::TargetImmune);
}

MODERN_TEST(StatusEffect_ImmunityIsPerDisorder)
{
	// Immune to stun, but the blow is burn.
	StatusApplicationInput burn = MakeBlow(StatusEffectType::Burn);
	burn.targetDisorderMask = static_cast<uint32_t>(DisorderStun);

	CHECK(ResolveStatusApplication(burn).Applied());
}

// ═══════════════════════════════════════════════════════════════════════
// Duration and state variables
// ═══════════════════════════════════════════════════════════════════════

// GLChar.cpp:3386-3387: fLIFE = fLIFE * fPOWER, then reduced by resistance.
MODERN_TEST(StatusEffect_DurationIsLifetimeTimesWeatherPower)
{
	StatusApplicationInput input = MakeBlow();
	input.lifetime     = 10.0f;
	input.weatherPower = 2.0f;

	const StatusApplicationResult result = ResolveStatusApplication(input);

	CHECK_EQ(result.duration, 20.0f);
	CHECK_EQ(result.state.remainingLifetime, 20.0f);
}

MODERN_TEST(StatusEffect_DurationIsReducedByResistanceUnderIntendedClamp)
{
	StatusConstants constants;
	constants.resistClampCeiling = StatusConstants::kIntendedResistClamp;

	// 10 - (10 * 50/100 * 0.5) = 10 - 2.5 = 7.5
	CHECK_EQ(ComputeStatusDuration(10.0f, 1.0f, 50, constants), 7.5f);
	CHECK_EQ(ComputeStatusDuration(10.0f, 1.0f, 0, constants), 10.0f);
}

// The variables are carried through untouched. Their per-state meaning is a
// data question, not a rule question; nothing here interprets them.
MODERN_TEST(StatusEffect_StateVariablesArePreservedExactly)
{
	StatusApplicationInput input = MakeBlow();
	input.var1 = -12.5f;
	input.var2 = 7.25f;

	const StatusApplicationResult result = ResolveStatusApplication(input);

	CHECK(result.Applied());
	CHECK_EQ(result.state.var1, -12.5f);
	CHECK_EQ(result.state.var2, 7.25f);
}

// ═══════════════════════════════════════════════════════════════════════
// Container: creation, expiry, ticking
// ═══════════════════════════════════════════════════════════════════════

MODERN_TEST(StatusContainer_EmptyByDefault)
{
	StatusEffectContainer container;

	CHECK_EQ(container.ActiveCount(), 0u);
	CHECK(!container.Has(StatusEffectType::Stun));
	CHECK_EQ(container.ActiveDisorderMask(), 0u);
}

MODERN_TEST(StatusContainer_ApplyStoresIntoTheMappedSlot)
{
	StatusEffectContainer container;

	CHECK(container.Apply(MakeState(StatusEffectType::Stun, 10.0f)));

	CHECK(container.Has(StatusEffectType::Stun));
	CHECK_EQ(container.ActiveCount(), 1u);
	CHECK_EQ(container.At(0)->type, StatusEffectType::Stun);
	CHECK_EQ(container.At(0)->remainingLifetime, 10.0f);
}

MODERN_TEST(StatusContainer_NoneIsNotApplicable)
{
	StatusEffectContainer container;

	CHECK(!container.Apply(MakeState(StatusEffectType::None, 10.0f)));
	CHECK_EQ(container.ActiveCount(), 0u);
}

// GLCharClient.cpp:3772 / GLFactEffect.cpp:154-159: fAGE -= elapsed, expires at
// <= 0.
MODERN_TEST(StatusContainer_TickBeforeExpiryKeepsTheStateActive)
{
	StatusEffectContainer container;
	container.Apply(MakeState(StatusEffectType::Stun, 10.0f));

	CHECK_EQ(container.Tick(3.0f), 0u);
	CHECK(container.Has(StatusEffectType::Stun));
	CHECK_EQ(container.At(0)->remainingLifetime, 7.0f);
}

// The boundary is <= 0, so landing exactly on zero expires.
MODERN_TEST(StatusContainer_TickExactlyToZeroExpiresTheState)
{
	StatusEffectContainer container;
	container.Apply(MakeState(StatusEffectType::Stun, 10.0f));

	CHECK_EQ(container.Tick(10.0f), 1u);
	CHECK(!container.Has(StatusEffectType::Stun));
	CHECK_EQ(container.ActiveCount(), 0u);
}

MODERN_TEST(StatusContainer_TickBeyondExpiryExpiresTheState)
{
	StatusEffectContainer container;
	container.Apply(MakeState(StatusEffectType::Stun, 10.0f));

	CHECK_EQ(container.Tick(25.0f), 1u);
	CHECK(!container.Has(StatusEffectType::Stun));
	CHECK_EQ(container.At(0)->type, StatusEffectType::None);
}

MODERN_TEST(StatusContainer_ExpiredSlotsAreNotTickedAgain)
{
	StatusEffectContainer container;
	container.Apply(MakeState(StatusEffectType::Stun, 1.0f));

	CHECK_EQ(container.Tick(5.0f), 1u);
	CHECK_EQ(container.Tick(5.0f), 0u);
	CHECK_EQ(container.ActiveCount(), 0u);
}

// ═══════════════════════════════════════════════════════════════════════
// Stacking - GLChar.cpp:6207 is a plain assignment
// ═══════════════════════════════════════════════════════════════════════

// There is no "keep the stronger" rule and no stacking: a second application
// overwrites, resetting the duration.
MODERN_TEST(StatusContainer_ReapplyingTheSameStateResetsDuration)
{
	StatusEffectContainer container;
	container.Apply(MakeState(StatusEffectType::Poison, 10.0f));
	container.Tick(6.0f);
	CHECK_EQ(container.At(2)->remainingLifetime, 4.0f);

	container.Apply(MakeState(StatusEffectType::Poison, 10.0f));

	CHECK_EQ(container.ActiveCount(), 1u);
	CHECK_EQ(container.At(2)->remainingLifetime, 10.0f);
}

// The consequence that is easy to miss: Burn and Stun share slot 0, so a burn
// landing on a stunned target silently ends the stun early.
MODERN_TEST(StatusContainer_BurnOverwritesAnExistingStun)
{
	StatusEffectContainer container;
	container.Apply(MakeState(StatusEffectType::Stun, 30.0f));
	CHECK(container.Has(StatusEffectType::Stun));

	container.Apply(MakeState(StatusEffectType::Burn, 5.0f));

	CHECK(!container.Has(StatusEffectType::Stun));
	CHECK(container.Has(StatusEffectType::Burn));
	CHECK_EQ(container.ActiveCount(), 1u);
}

// Independent slots coexist.
MODERN_TEST(StatusContainer_IndependentStatesCoexist)
{
	StatusEffectContainer container;
	container.Apply(MakeState(StatusEffectType::Poison, 10.0f));
	container.Apply(MakeState(StatusEffectType::Curse, 20.0f));
	container.Apply(MakeState(StatusEffectType::Mad, 30.0f));

	CHECK_EQ(container.ActiveCount(), 3u);
	CHECK(container.Has(StatusEffectType::Poison));
	CHECK(container.Has(StatusEffectType::Curse));
	CHECK(container.Has(StatusEffectType::Mad));
}

MODERN_TEST(StatusContainer_AllFourSlotsCanBeOccupied)
{
	StatusEffectContainer container;
	container.Apply(MakeState(StatusEffectType::Frozen, 10.0f));  // slot 0
	container.Apply(MakeState(StatusEffectType::Mad,    10.0f));  // slot 1
	container.Apply(MakeState(StatusEffectType::Poison, 10.0f));  // slot 2
	container.Apply(MakeState(StatusEffectType::Curse,  10.0f));  // slot 3

	CHECK_EQ(container.ActiveCount(), 4u);
}

MODERN_TEST(StatusContainer_ClearEmptiesEverySlot)
{
	StatusEffectContainer container;
	container.Apply(MakeState(StatusEffectType::Frozen, 10.0f));
	container.Apply(MakeState(StatusEffectType::Curse, 10.0f));

	container.Clear();

	CHECK_EQ(container.ActiveCount(), 0u);
	CHECK_EQ(container.ActiveDisorderMask(), 0u);
}

// ═══════════════════════════════════════════════════════════════════════
// Cure - GLChar.cpp:6228-6243
// ═══════════════════════════════════════════════════════════════════════

MODERN_TEST(StatusContainer_CureClearsMatchingDisordersOnly)
{
	StatusEffectContainer container;
	container.Apply(MakeState(StatusEffectType::Stun,   10.0f));  // slot 0
	container.Apply(MakeState(StatusEffectType::Poison, 10.0f));  // slot 2

	CHECK_EQ(container.Cure(DisorderStun), 1u);

	CHECK(!container.Has(StatusEffectType::Stun));
	CHECK(container.Has(StatusEffectType::Poison));
}

MODERN_TEST(StatusContainer_CureAllClearsEverything)
{
	StatusEffectContainer container;
	container.Apply(MakeState(StatusEffectType::Frozen, 10.0f));
	container.Apply(MakeState(StatusEffectType::Mad,    10.0f));
	container.Apply(MakeState(StatusEffectType::Poison, 10.0f));
	container.Apply(MakeState(StatusEffectType::Curse,  10.0f));

	CHECK_EQ(container.Cure(DisorderAll), 4u);
	CHECK_EQ(container.ActiveCount(), 0u);
}

// A cure mask that no active state matches changes nothing, which is
// GLChar.cpp:6234's `continue` for empty slots generalised.
MODERN_TEST(StatusContainer_CureWithUnmatchedMaskIsANoOp)
{
	StatusEffectContainer container;
	container.Apply(MakeState(StatusEffectType::Stun, 10.0f));

	CHECK_EQ(container.Cure(DisorderPoison), 0u);
	CHECK(container.Has(StatusEffectType::Stun));
}

// ═══════════════════════════════════════════════════════════════════════
// End-to-end: resolve then store
// ═══════════════════════════════════════════════════════════════════════

MODERN_TEST(Status_ResolveThenApplyToContainer)
{
	StatusApplicationInput input = MakeBlow(StatusEffectType::Poison);
	input.var1 = 3.5f;
	input.lifetime = 12.0f;

	StatusEffectContainer container;

	const StatusApplicationResult result = ResolveStatusApplication(input);

	if (result.Applied())
	{
		container.Apply(result.state);
	}

	CHECK(container.Has(StatusEffectType::Poison));
	CHECK_EQ(container.At(2)->remainingLifetime, 12.0f);
	CHECK_EQ(container.At(2)->var1, 3.5f);

	container.Tick(12.0f);
	CHECK(!container.Has(StatusEffectType::Poison));
}

MODERN_TEST(Status_ResolvedStatesSurviveAContainerLifetimeCycle)
{
	StatusEffectContainer container;

	StatusApplicationInput poison = MakeBlow(StatusEffectType::Poison);
	poison.lifetime = 8.0f;
	container.Apply(ResolveStatusApplication(poison).state);

	StatusApplicationInput curse = MakeBlow(StatusEffectType::Curse);
	curse.lifetime = 3.0f;
	container.Apply(ResolveStatusApplication(curse).state);

	CHECK_EQ(container.ActiveCount(), 2u);

	container.Tick(4.0f);

	// Poison survives, curse has expired.
	CHECK(container.Has(StatusEffectType::Poison));
	CHECK(!container.Has(StatusEffectType::Curse));
	CHECK_EQ(container.At(2)->remainingLifetime, 4.0f);
}