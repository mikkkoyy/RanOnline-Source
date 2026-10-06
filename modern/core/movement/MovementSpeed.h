#pragma once

// WORLD-ENTRY-002f: base movement speed, from the RECOVERED class data.
//
// This is the `GetMoveVelo()` base term and nothing else.
//
// ---------------------------------------------------------------------------
// THE MEASURED TABLE, AND THE WRONG ONE IT REPLACES
// ---------------------------------------------------------------------------
//
// WORLD-ENTRY-002c §2.3 recovered `fWALKVELO` and `fRUNVELO` for all 16 classes by
// decrypting `default.charclass` -> `class<N>.classconst`. The distinct values are:
//
//     WALK  12.0 13.0 14.0 15.0 16.0
//     RUN   36.0 37.0 39.0 40.0 41.0 42.0 44.0
//
// and `kLegacyClassSpeeds` below is that table, row for row, in `EMCHARINDEX`
// order (GLCharDefine.h:237-252, transcribed as
// Stats::CharClassIndex in modern/core/stats/BaseStats.h).
//
// It REPLACES the 12/34 pair, which is what 002b assumed and 002c disproved: those
// are `cCONSTCLASS`'s CONSTRUCTOR DEFAULTS (GLogicData.h:118-119). RAN overwrites
// both fields from the `.classconst` files at startup, so the defaults never reach
// a running game and are wrong for every one of the 16 classes. They are still
// named here, as `kLegacyConstructor*`, because a caller who reaches for a
// "default" should have to type the word `Constructor` and see what it means.
//
// ---------------------------------------------------------------------------
// THE FULL FORMULA, AND WHY ONLY THE BASE TERM IS COMPUTED HERE
// ---------------------------------------------------------------------------
//
// GLChar.cpp:4966-4974:
//
//     float fDefaultVelo = IsSTATE(EM_ACT_RUN) ? cCONSTCLASS[i].fRUNVELO
//                                              : cCONSTCLASS[i].fWALKVELO;
//     return MoveVelocity( fDefaultVelo,
//                        GETMOVEVELO(),      // state blows
//                        GETMOVE_ITEM(),    // worn equipment
//                        IsSTATE(EM_ACT_RUN) );
//
// and MoveVelocity is a MULTIPLICATION (GameCharacterCalculations.cpp:294-306):
//
//     speed = base * ( GETMOVEVELO() + GETMOVE_ITEM() )
//
// Two facts in that line are load-bearing and easy to get backwards:
//
//   * `GETMOVEVELO()` is a MULTIPLIER that starts at 1.0, not a bonus. A character
//     with no blows has 1.0 and therefore `speed = base * 1.0`. So there is no
//     separate "speed bonus" constant for the base case - and a modern
//     implementation must not treat the term as additive-with-a-zero-default, which
//     would give `speed = base * 0` for an unmodified character.
//
//   * `GETMOVE_ITEM()` is divided by `fRUNVELO` WHETHER OR NOT the character is
//     running (002c §3.1). The divisor is therefore independent of the run/walk
//     branch, and a provider that switched divisors with the run flag would
//     disagree with RAN for a walking character wearing a speed item.
//
// `MoveVelocityTerms` below therefore defaults to `stateMultiplier = 1.0f` and
// `moveItem = 0.0f`, which is NOT a fabricated zero:
//
//   * 1.0 is `GLLogicExPC`'s own initial value for `m_fSTATE_MOVE` (GLogixExPC.cpp:73).
//   * 0.0 is an accurate statement that this milestone models no worn equipment,
//     and RAN's term for "nothing worn" is exactly 0.
//
// So `base * (1.0 + 0.0) == base` is the value RAN would compute for a character
// with no state blows and no equipment - the state this server actually models.
//
// ---------------------------------------------------------------------------
// WHAT IS DEFERRED, AND WHAT IT COSTS
// ---------------------------------------------------------------------------
//
// Not implemented, each a NAMED deferral from 002c rather than an oversight:
//
//     equipment speed          GETMOVE_ITEM()  - the worn set is not in this
//                                                    repository; `ItemDefinition`
//                                                    has no speed field at all
//     vehicle speed            EM_ACT_VEHICLE_BOOSTER
//     disguise speed           m_fDisguise / combine
//     passive skill speed      skill contributions
//     state blows              m_fSTATE_MOVE != 1.0
//     m_fOPTION_MOVE ramps     the pet ramp, clamped to [0,3]
//
// The consequence, stated plainly because it is a behaviour difference: a RAN
// character wearing a speed item moves faster than this one. That is not a bug in
// the formula - the formula is exact - it is the missing input.

#include "stats/BaseStats.h"

#include <cstddef>

namespace Modern::Movement
{
	// One class's base walk and run velocity, in the units RAN's `GetMoveVelo()`
	// returns: world units per second.
	struct ClassMoveSpeed
	{
		float walkVelocity = 0.0f;
		float runVelocity  = 0.0f;
	};

	// `cCONSTCLASS`'s CONSTRUCTOR DEFAULTS (GLogicData.h:118-119), which 002b used
	// before 002c proved them wrong for every class.
	//
	// Named and NOT used. They exist so that a caller reaching for a fallback has
	// to name it after what it actually is.
	inline constexpr float kLegacyConstructorWalkVelocity = 12.0f;
	inline constexpr float kLegacyConstructorRunVelocity  = 34.0f;

	// `m_fSTATE_MOVE`'s initial value, GLLogicExPC.cpp:73. The DEFAULT for the state
	// term, and the reason the default speed equals the base speed rather than zero.
	inline constexpr float kDefaultStateMultiplier = 1.0f;

	// The two terms of `MoveVelocity` beyond the base velocity.
	//
	// Deliberately a struct rather than two loose floats: the point of naming them
	// is that a caller must say which one it is setting, and that a value of 0 for
	// `moveItem` reads as a DECLARATION ("nothing worn") rather than as an
	// oversight.
	struct MoveVelocityTerms
	{
		// GETMOVEVELO(). A multiplier, default 1.0. Never 0 by default.
		float stateMultiplier = kDefaultStateMultiplier;

		// GETMOVE_ITEM(). An ADDITIVE term on the same scale as the multiplier, so
		// the default 0 contributes nothing. DEFERRED: see the header.
		float moveItem = 0.0f;

		// The divisor for `moveItem`, `fRUNVELO`, regardless of whether the character
		// is running (002c §3.1). Carried here so a future provider that DOES supply
		// an item term cannot pick the divisor by accident.
		float itemDivisor = 0.0f;
	};

	// The 16 measured rows, in `EMCHARINDEX` order.
	//
	// WORLD-ENTRY-002c §2.3. `kClassCount` is `Stats::kClassCount`, so the table and
	// the class enum cannot drift apart in size - a static_assert below checks it.
	// The whole measured table, and how many rows it has.
	//
	// Exposed so a caller - a test, or an operator dumping the table - can walk all
	// sixteen rows rather than trusting one lookup. ClassSpeedFor is the ergonomic
	// accessor; this is the evidence.
	const ClassMoveSpeed* ClassSpeedTable() noexcept;
	std::size_t         ClassSpeedCount() noexcept;

	// One class's row, or nullptr for an index outside the sixteen.
	const ClassMoveSpeed* ClassSpeedFor(Stats::CharClassIndex index) noexcept;

	// The base term alone: walk or run, chosen by the authoritative `EM_ACT_RUN`
	// bit. Fails for a class index outside the 16.
	bool TryGetBaseVelocity(Stats::CharClassIndex index, bool running, float& out) noexcept;

	// `speed = base * ( stateMultiplier + moveItem / itemDivisor )`
	//
	// Reproduces GameCharacterCalculations.cpp:294-306, including the division by
	// `fRUNVELO` and its independence from the run flag. A zero divisor yields
	// `base * stateMultiplier` rather than a division by zero - and the caller's
	// `MoveVelocityTerms` default has a zero divisor on purpose, because it has no
	// item term to divide.
	float MoveVelocity(float baseVelocity, const MoveVelocityTerms& terms, bool running);
}
