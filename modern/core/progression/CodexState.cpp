// VERTICAL-004: a character's codex state. See CodexState.h.

#include "progression/CodexState.h"

#include <algorithm>
#include <vector>

namespace Modern
{
	const char* ToString(CodexRegisterError error) noexcept
	{
		switch (error)
		{
		case CodexRegisterError::None:            return "None";
		case CodexRegisterError::UnknownCodex:    return "UnknownCodex";
		case CodexRegisterError::AlreadyCompleted:return "AlreadyCompleted";
		case CodexRegisterError::InvalidItem:     return "InvalidItem";
		case CodexRegisterError::NotCompletable:  return "NotCompletable";
		}
		return "<invalid codex register error>";
	}

	CodexProgress CodexState::Seat(const CodexDefinition& definition)
	{
		CodexProgress record;
		record.id            = definition.id;
		record.type          = definition.type;
		record.requirements  = definition.requirements;
		record.requiredCount = definition.RequiredSlotCount();
		record.done.fill(false);
		record.doneCount     = 0;
		return record;
	}

	void CodexState::Reconcile(const CodexDefinitionProvider& definitions)
	{
		// RAN walks the definition table and seats whatever is missing
		// (GLCharDataCodex.cpp:72-93). The provider hands them over in ascending
		// id order, and each definition is reconciled independently, so the
		// outcome does not depend on the order - only the intermediate states
		// do, and nothing observes those.
		for (const CodexDefinition& definition : definitions.GetAll())
		{
			// A completed entry is never re-seated. This is the rule that makes a
			// reward exactly-once across a reload, and in RAN it is the `continue`
			// at GLCharDataCodex.cpp:79-80.
			if (m_completed.find(definition.id) != m_completed.end())
			{
				continue;
			}

			const auto progress = m_progress.find(definition.id);
			if (progress == m_progress.end())
			{
				m_progress[definition.id] = Seat(definition);
				continue;
			}

			CodexProgress record = progress->second;

			// A reconfigured type invalidates everything the record knows, as in
			// RAN (GLCodexData.cpp:486-494), which zeroes the counters and re-runs
			// Assign. RAN's `dwProgressNow = 0` there is redundant with Assign
			// setting it anyway; the effect is a re-seat either way.
			if (record.type != definition.type)
			{
				m_progress[definition.id] = Seat(definition);
				continue;
			}

			// Same type: refresh the captured requirements. RAN only refreshes
			// the five item ids and leaves the captured quantities and grades
			// stale (GLCodexData.cpp:497-506), and never recomputes
			// `dwProgressMax`. See the note on CodexState::Reconcile: modern
			// refreshes the whole requirement and the required count, so a
			// retuned table does not leave a character holding requirements that
			// no longer exist.
			//
			// RAN keeps `dwProgressItemDone1..5` and `dwProgressNow` across that
			// refresh, so a table retune never costs a character recorded
			// progress. Carrying that over is safe only while the requirements
			// are unchanged: a done flag means "this exact item and count was
			// registered", which is a claim about the requirement, not about the
			// record. So progress is carried across an identical refresh and
			// dropped when any requirement or the required count actually
			// changes. The alternative - always resetting, which is what this
			// used to do - silently threw away a character's progress on every
			// load, because reconciliation runs on every load.
			const bool requirementsChanged =
				record.requirements  != definition.requirements ||
				record.requiredCount != definition.RequiredSlotCount();

			record.requirements  = definition.requirements;
			record.requiredCount = definition.RequiredSlotCount();

			if (requirementsChanged)
			{
				record.doneCount = 0;
				for (bool& slotDone : record.done)
				{
					slotDone = false;
				}
			}

			// A retune can take away the requirements an entry needed, which can
			// leave nothing in the counted window that any registration could
			// match. Such a record is not completable, and keeping it would put a
			// record in the progress map that nothing can ever satisfy, so it is
			// dropped. RAN keeps it, and the entry simply sits in the map forever
			// - see the note on CodexState::Reconcile for why the modern refusal
			// is worth the divergence.
			if (!record.IsCompletable())
			{
				m_progress.erase(progress);
				continue;
			}
			m_progress[definition.id] = record;
		}

		// Drop any record whose definition no longer exists, from both maps. RAN
		// does this at GLCharDataCodex.cpp:95-132; doing it for the completed map
		// as well is the same code, and matters because a completed entry
		// contributes to the stat pipeline through its definition - an entry
		// whose definition vanished contributes nothing but would still be
		// counted in the published totals.
		std::vector<CodexId> orphaned;
		for (const auto& [id, record] : m_progress)
		{
			(void) record;
			if (definitions.Find(id) == nullptr)
			{
				orphaned.push_back(id);
			}
		}
		for (const auto& [id, record] : m_completed)
		{
			(void) record;
			if (definitions.Find(id) == nullptr)
			{
				orphaned.push_back(id);
			}
		}
		for (const CodexId& id : orphaned)
		{
			m_progress.erase(id);
			m_completed.erase(id);
		}
	}

	CodexProgress CodexState::GetProgress(CodexId id) const noexcept
	{
		const auto position = m_progress.find(id);
		return position != m_progress.end() ? position->second : CodexProgress();
	}

	CodexProgress CodexState::GetCompleted(CodexId id) const noexcept
	{
		const auto position = m_completed.find(id);
		return position != m_completed.end() ? position->second : CodexProgress();
	}

	bool CodexState::IsInProgress(CodexId id) const noexcept
	{
		return m_progress.find(id) != m_progress.end();
	}

	bool CodexState::IsCompleted(CodexId id) const noexcept
	{
		return m_completed.find(id) != m_completed.end();
	}

	Result<CodexRegistration> CodexState::RegisterItem(CodexId id, const ItemInstance& item)
	{
		if (!id.IsValid() || !item.IsValid())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const auto progress = m_progress.find(id);
		if (progress == m_progress.end())
		{
			// A completed entry is a different refusal from an unknown one, and
			// only because the caller can act on it: registering into a finished
			// entry is a request that has already been satisfied, while an unknown
			// id is a request about something the character does not have. RAN
			// refuses the first at the request handler
			// (GLCharInvenMsg.cpp:9130-9136) and the second nowhere in particular.
			const bool completed = m_completed.find(id) != m_completed.end();
			return CodexRegistration{ completed ? CodexRegisterError::AlreadyCompleted
			                                    : CodexRegisterError::UnknownCodex,
				                         /*recorded*/ false, /*completed*/ false,
				                         /*doneCount*/ 0, /*requiredCount*/ 0 };
		}

		CodexProgress record = progress->second;
		if (!record.IsCompletable())
		{
			// Seated from a definition that names nothing inside the counted
			// window. RAN has no such guard, so such an entry sits in the progress
			// map forever and can only be completed by a registration matching
			// nothing, which never happens - see §4 of the investigation. Refusing
			// is the honest answer, and it is reported distinctly from a rule
			// outcome that simply did not match, so a caller can tell a broken
			// table apart from a missing item.
			return CodexRegistration{ CodexRegisterError::NotCompletable,
				                         /*recorded*/ false, /*completed*/ false,
				                         record.doneCount, record.requiredCount };
		}

		// Every counted slot naming this item and not already done is satisfied.
		// Independent `if`s, not `else if`, so one stack can satisfy two slots
		// when an entry names the same item twice (GLCharCodex.cpp:75-144).
		bool recorded = false;
		for (uint8_t slot = 0; slot < record.requiredCount; ++slot)
		{
			if (record.done[slot])
			{
				continue;
			}
			const CodexRequirement& requirement = record.requirements[slot];
			if (requirement.item != item.definition)
			{
				continue;
			}
			// RAN compares the stack size for equality, not with `>=`
			// (GLCharCodex.cpp:81). A stack larger than the requirement does not
			// satisfy it, and that is reproduced.
			if (requirement.quantity != item.count)
			{
				continue;
			}
			// The required grade is deliberately not compared: ItemInstance
			// carries no grade, so there is nothing to compare against. See
			// CodexRequirement.
			record.done[slot] = true;
			++record.doneCount;
			recorded = true;
		}

		// RAN clamps before comparing upward (GLCharCodex.cpp:147-151), so the
		// counter can never exceed the maximum even if a slot were satisfied
		// twice.
		if (record.doneCount > record.requiredCount)
		{
			record.doneCount = record.requiredCount;
		}

		CodexRegistration result;
		result.recorded      = recorded;
		result.doneCount     = record.doneCount;
		result.requiredCount = record.requiredCount;
		result.completed     = record.IsComplete();

		if (result.completed)
		{
			// RAN completes by inserting into the done map
			// (GLCharCodex.cpp:19) and erasing the progress record in a second
			// pass (GLCharCodex.cpp:165-172). Doing both here is what makes the
			// reward exactly-once: a completed entry is no longer reachable from
			// the progress map, and RegisterItem only ever looks there.
			m_completed[id] = record;
			m_progress.erase(progress);
		}
		else
		{
			m_progress[id] = record;
		}

		return result;
	}
}
