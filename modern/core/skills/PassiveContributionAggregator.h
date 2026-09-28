#pragma once

// VERTICAL-003: learned passive skills to a Stats::PassiveContribution.
//
// This is a transcription, not a calculation. RAN's SUM_PASSIVE
// (legacy/Lib_Client/G-Logic/GLogixExPC.cpp:863) adds up every learned
// passive skill's values into SPASSIVE_SKILL_DATA, and SUM_ADDITION then
// folds that into the derived statistics. This class does the first half and
// nothing else: it produces the contribution value CORE-002 already defines,
// and it restates no formula.
//
// What is aggregated, and what is deliberately not, is recorded per field in
// docs/reference/client/VERTICAL-003_SKILL_INVESTIGATION.md. In short:
// the skill's basic apply type and per-level value are aggregated, along
// with impacts. Specs (pierce, range, velocity, delay, damage reduce) are
// not part of the verified stat pipeline and are not aggregated here.

#include "skills/SkillDefinition.h"
#include "skills/SkillState.h"
#include "skills/SkillDefinitionProvider.h"
#include "equipment/EquipmentState.h"
#include "stats/Contributions.h"
#include "types/Result.h"

namespace Modern
{
	// Why a passive aggregation was refused, so a caller can report something
	// better than "invalid argument".
	enum class PassiveAggregationError : uint8_t
	{
		None = 0,
		MissingDefinition,  // a learned skill names a skill nothing defines
		NonFinite,          // a definition carries a NaN or an infinity
	};

	struct PassiveContributionResult
	{
		Stats::PassiveContribution contribution;
		PassiveAggregationError    error = PassiveAggregationError::None;

		// How many skills contributed a non-zero block. Reported rather than
		// assumed, because it is the difference between "learned three skills
		// and they all did nothing" and "learned nothing".
		size_t contributingSkills = 0;

		constexpr bool IsOk() const noexcept { return error == PassiveAggregationError::None; }
	};

	// Aggregates every learned passive skill into one contribution.
	//
	// Deterministic: skills are visited in SkillId order (map order), so the
	// same skill set always produces the same contribution regardless of
	// learning order.
	//
	// Legacy numeric semantics are preserved where they are part of the
	// verified pipeline. Two matter and both are in Stats::Calculate rather
	// than here: the six stat bonuses are handled by the stat sum (which
	// wraps), and the damage range is two independent integers. This class
	// accumulates in the contribution's own types and lets Calculate apply
	// the wrap, so the addition order here matches SUM_PASSIVE without a
	// second truncation rule existing in two places.
	class PassiveContributionAggregator
	{
	public:
		// Aggregates all learned passive skills into one contribution.
		//
		// A learned skill whose definition cannot be resolved is an error,
		// not a silent zero: skills that contribute nothing because nothing
		// knows what they are would hide a data problem behind a plausible
		// number.
		//
		// Equipment-dependent passives: if a skill requires a specific
		// weapon type in a hand slot, that slot is checked against the
		// currently equipped item. If the requirement is not met, the skill
		// contributes nothing (not an error).
		//
		// Active skills (emROLE != EMROLE_PASSIVE) are skipped silently —
		// they are tracked in SkillState but do not contribute to stats.
		static Result<PassiveContributionResult> Aggregate(
			const SkillState& skills,
			const SkillDefinitionProvider& provider,
			const EquipmentState& equipment);
	};
}