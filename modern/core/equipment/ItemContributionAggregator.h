#pragma once

// VERTICAL-002: equipped items to a `Stats::ItemContribution`.
//
// This is a transcription, not a calculation. RAN's `GLCHARLOGIC::SUM_ITEM`
// (legacy/Lib_Client/G-Logic/GLogixExPC.cpp:441) adds up every worn item's
// values into `SSUM_ITEM`, and `SUM_ADDITION` then folds that into the derived
// statistics. This class does the first half and nothing else: it produces the
// contribution value CORE-002 already defines, and it restates no formula.
//
// What is aggregated, and what is deliberately not, is recorded per field in
// docs/reference/client/VERTICAL-002_EQUIPMENT_INVESTIGATION.md §6 and §9. In
// short: the definition's base values are aggregated, and RAN's per-copy random
// options and refine state are not, because no modern system produces them yet.

#include "equipment/EquipmentState.h"
#include "equipment/ItemDefinitionProvider.h"
#include "stats/Contributions.h"
#include "types/Result.h"

namespace Modern
{
	// Why an aggregation was refused, so a caller can report something better
	// than "invalid argument".
	enum class ContributionError : uint8_t
	{
		None = 0,
		MissingDefinition,  // a worn instance names an item nothing defines
		NonFinite,         // a definition carries a NaN or an infinity
	};

	struct ItemContributionResult
	{
		Stats::ItemContribution contribution;
		ContributionError      error = ContributionError::None;

		// How many slots contributed a non-zero block. Reported rather than
		// assumed, because it is the difference between "equipped three things
		// and they all did nothing" and "equipped nothing".
		size_t contributingSlots = 0;

		constexpr bool IsOk() const noexcept { return error == ContributionError::None; }
	};

	// Aggregates every worn item into one contribution.
	//
	// Deterministic: the slots are visited in slot order, so the same equipment
	// always produces the same contribution regardless of how it was built.
	//
	// Legacy numeric semantics are preserved where they are part of the verified
	// pipeline. Two matter and both are in `Stats::Calculate` rather than here:
	// the six stat bonuses are 16-bit and wrap when summed, and the damage
	// range is two independent integers. This class accumulates in the
	// contribution's own types and lets `Calculate` apply the wrap, so the
	// addition order here matches SUM_ITEM's without a second truncation rule
	// existing in two places.
	class ItemContributionAggregator
	{
	public:
		// A worn instance whose definition cannot be resolved is an error, not a
		// silent zero: equipment that contributes nothing because nothing knows
		// what it is would hide a data problem behind a plausible number.
		//
		// An instance bound to something else is likewise refused, so an item
		// cannot contribute twice by being equipped and already owned elsewhere.
		static Result<ItemContributionResult> Aggregate(
			const EquipmentState& equipment,
			const ItemDefinitionProvider& provider);
	};
}
