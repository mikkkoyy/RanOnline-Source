// VERTICAL-004: completed codex entries to a contribution. See
// CodexContributionAggregator.h.

#include "progression/CodexContributionAggregator.h"

namespace Modern
{
	namespace
	{
		// Adds a flat unsigned amount to the one field a type selects.
		//
		// The switch mirrors the eleven `if` statements in
		// GLCHARLOGIC::CODEX_STATS (GLogixExPC.cpp:5133-5154) one for one,
		// including that they are eleven independent tests rather than one
		// eleven-way switch: a type outside the range falls through all eleven
		// and contributes nothing, which is what the None return here
		// reproduces.
		void AddReward(Stats::CodexContribution& contribution,
		               CodexRewardField            field,
		               uint32_t                    point) noexcept
		{
			switch (field)
			{
			case CodexRewardField::Hp:          contribution.hp          += point; return;
			case CodexRewardField::Mp:          contribution.mp          += point; return;
			case CodexRewardField::Sp:          contribution.sp          += point; return;
			case CodexRewardField::Attack:      contribution.attack      += point; return;
			case CodexRewardField::Defense:     contribution.defense     += point; return;
			case CodexRewardField::ShootPower:  contribution.shootPower  += point; return;
			case CodexRewardField::MeleePower:  contribution.meleePower  += point; return;
			case CodexRewardField::MagicAttack: contribution.magicAttack += point; return;
			case CodexRewardField::Resistance:  contribution.resistance  += point; return;
			case CodexRewardField::Hit:         contribution.hit         += point; return;
			case CodexRewardField::Avoid:       contribution.avoid       += point; return;
			case CodexRewardField::None:        break;
			}
		}
	}

	Result<CodexContributionResult> CodexContributionAggregator::Aggregate(
		const CodexState&                 state,
		const CodexDefinitionProvider&     definitions)
	{
		CodexContributionResult result;

		// Only the completed map, exactly as RAN does (GLogixExPC.cpp:5122). The
		// progress map is not consulted at all, which is what makes a reward
		// exactly-once: an entry appears here once, when it completes, and the
		// server recomputes from the whole set on every recalculation rather than
		// incrementing a running total.
		for (const auto& [id, record] : state.GetAllCompleted())
		{
			(void) record;

			const CodexDefinition* definition = definitions.Find(id);
			if (definition == nullptr)
			{
				// RAN's `if ( pcodex_char )` guard (GLogixExPC.cpp:5131) skips the
				// entry and continues. Skipping is not reported by RAN; it is
				// reported here because a completed entry that silently pays
				// nothing is exactly the kind of thing a player reports as a bug
				// and a server has to be able to answer.
				++result.skippedCodex;
				if (result.skipReason == CodexContributionError::None)
				{
					result.skipReason = CodexContributionError::MissingDefinition;
				}
				continue;
			}

			const CodexRewardField field = CodexTypeToField(definition->type);
			if (field == CodexRewardField::None)
			{
				++result.skippedCodex;
				if (result.skipReason == CodexContributionError::None)
				{
					result.skipReason = CodexContributionError::UnmappedType;
				}
				continue;
			}

			// A zero reward point is still a completed entry, so it is counted
			// separately from the ones that paid something. RAN adds it to a
			// field, changing nothing.
			if (definition->rewardPoint > 0)
			{
				++result.contributingCodex;
			}
			AddReward(result.contribution, field, definition->rewardPoint);
		}

		return result;
	}
}
