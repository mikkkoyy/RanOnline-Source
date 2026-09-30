#pragma once

// VERTICAL-004: completed codex entries to a Stats::CodexContribution.
//
// This is a transcription, not a calculation. RAN's CODEX_STATS
// (legacy/Lib_Client/G-Logic/GLogixExPC.cpp:5103) sums the reward points of
// every completed codex entry into the eleven `m_dw*Increase` fields, and
// SUM_ADDITION then folds that into the derived statistics. This class does the
// first half and nothing else: it produces the contribution value CORE-002
// already defines and Stats::Calculate already consumes, and it restates no
// formula.
//
// One structural fact about the legacy function shapes this class: CODEX_STATS
// is a full recompute, not an increment. Its `dwStatPoint` and `nIndex`
// parameters are never read in the body, and it starts from eleven local zeros
// (GLogixExPC.cpp:5106-5116), walks the whole completed map, and overwrites all
// eleven fields at the end (:5158-5168) before re-running INIT_DATA. It is
// therefore idempotent by construction, and so is this: it is safe to call on
// every recalculation, which is what the server does.

#include "progression/CodexDefinition.h"
#include "progression/CodexDefinitionProvider.h"
#include "progression/CodexState.h"
#include "stats/Contributions.h"
#include "types/Result.h"

#include <cstdint>

namespace Modern
{
	// Why a codex aggregation was refused, so a caller can report something
	// better than "invalid argument".
	enum class CodexAggregationError : uint8_t
	{
		None = 0,
		MissingDefinition,  // a completed entry names an entry nothing defines
		UnmappedType,       // a definition carries a type outside the verified enum
	};

	// Why a completed entry resolved to no statistic at all.
	//
	// Reported separately from an error because it is a *data* condition rather
	// than a caller's mistake, and because a caller publishing progress to a
	// client needs to know it. RAN handles it by skipping
	// (GLogixExPC.cpp:5131), so a completed entry whose definition has been
	// deleted contributes nothing and nothing says so.
	enum class CodexContributionError : uint8_t
	{
		None = 0,
		MissingDefinition,  // completed, but no definition resolves
		UnmappedType,       // resolved, but the type selects no field
	};

	struct CodexContributionResult
	{
		Stats::CodexContribution  contribution;
		CodexAggregationError     error = CodexAggregationError::None;

		// How many completed entries contributed a non-zero point. Reported
		// rather than assumed, because it is the difference between "finished
		// four entries and they all reward nothing" and "finished nothing".
		size_t contributingCodex = 0;

		// How many completed entries were skipped, and the first reason found.
		// See CodexContributionError.
		size_t                   skippedCodex  = 0;
		CodexContributionError   skipReason    = CodexContributionError::None;

		constexpr bool IsOk() const noexcept { return error == CodexAggregationError::None; }
	};

	// Aggregates the completed codex set into one contribution.
	//
	// Deterministic: entries are visited in CodexId order (map order), so the
	// same completed set always produces the same contribution regardless of the
	// order they were completed in. The accumulation is unsigned, because the
	// legacy accumulator is a `DWORD` (GLogixExPC.cpp:5106) and a codex bonus can
	// only ever raise a value.
	//
	// Only completed entries contribute. RAN iterates `m_mapCodexDone`
	// (GLogixExPC.cpp:5122) and never the progress map, and its per-entry
	// `dwProgressNow < 1` guard is commented out (:5128), so an entry's stored
	// progress plays no part in the contribution - only its type and its
	// definition's reward point do. An entry therefore contributes exactly once
	// per definition that is completed.
	class CodexContributionAggregator
	{
	public:
		// Aggregates every completed codex entry into one contribution.
		//
		// A completed entry whose definition cannot be resolved is reported
		// through `skipReason` and contributes nothing, as in RAN. It is not an
		// error: RAN's character load already deletes records whose definition has
		// gone (GLCharDataCodex.cpp:95-132), so an unresolved entry is a
		// transitional state, and refusing the whole aggregation over it would
		// leave a character unstattable.
		//
		// A definition whose type is outside the verified enum cannot name a field
		// and is treated the same way, for the same reason.
		static Result<CodexContributionResult> Aggregate(
			const CodexState& state,
			const CodexDefinitionProvider& definitions);
	};
}
