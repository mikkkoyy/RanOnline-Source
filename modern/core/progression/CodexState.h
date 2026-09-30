#pragma once

// VERTICAL-004: a character's codex state.
//
// This is the runtime counterpart to CodexDefinition. Definitions are immutable
// and shared; this is per-character and mutable, and it is the only thing in the
// modern tree that may change whether a codex entry counts as completed.
//
// Legacy provenance: the two maps SCHARDATA2 holds
// (legacy/Lib_Client/G-Logic/GLCharData.h:975-976):
//
//     m_mapCodexProg    progress, one entry per definition not yet completed
//     m_mapCodexDone    completed
//
// They are the whole model. An entry is in exactly one of them, and the
// contribution aggregator reads only the second.
//
// The record is SCODEX_CHAR_DATA (GLCodexData.h:179-241): the entry id, its
// type, the five required item ids with their quantities and grades, a done flag
// per slot, and the now/max counters. RAN copies the requirements out of the
// definition onto the character (GLCodexData.cpp:388-398), so a progress record
// is self-contained and registering an item never has to re-read the definition
// table. That is reproduced here, and it is why CodexProgress holds
// requirements rather than a pointer to a definition.

#include "item/ItemInstance.h"
#include "progression/CodexDefinition.h"
#include "progression/CodexDefinitionProvider.h"
#include "types/Ids.h"
#include "types/Result.h"

#include <array>
#include <cstdint>
#include <map>

namespace Modern
{
	// One codex entry as a character holds it: in progress, or completed.
	//
	// The same record is used for both, exactly as RAN uses one struct for
	// `m_mapCodexProg` and `m_mapCodexDone`. Which map an entry sits in is what
	// distinguishes it; the record itself does not need to.
	struct CodexProgress
	{
		// The entry this record is about. Invalid means "no record".
		CodexId id = CodexId::MakeInvalid();

		// The type captured when the record was seated.
		//
		// RAN re-reads the type from the live definition in the contribution
		// aggregator (`scodex_char_data.emType` at GLLogixExPC.cpp:5133) rather
		// than from the stored copy, so this field is the one thing on the
		// character that CODEX_STATS ignores. It is kept because
		// SCODEX_CHAR_DATA::Correction compares it to decide whether a
		// reconfigured entry has to lose its progress (GLCodexData.cpp:486-494).
		CodexType type = CodexType::ReachLevel;

		// The requirements as captured from the definition, in slot order.
		std::array<CodexRequirement, kCodexMaxRequirements> requirements{};

		// How many of the five slots count, as computed by
		// CodexDefinition::RequiredSlotCount() when the record was seated.
		uint8_t requiredCount = 0;

		// Whether each slot has been satisfied. RAN stores these as five
		// `DWORD` flags tested against `!= 1` (GLCharCodex.cpp:77, :91, :105,
		// :119, :133) and set to 1; a boolean is the same information without
		// the values a caller could set to something else.
		std::array<bool, kCodexMaxRequirements> done{};

		// How many counted slots are satisfied. Kept as a field because RAN
		// keeps `dwProgressNow` and compares it to `dwProgressMax` to decide
		// completion (GLCharCodex.cpp:147); it is a cached count, not
		// independent state, and nothing writes it without a done flag.
		uint8_t doneCount = 0;

		bool IsValid() const noexcept { return id.IsValid(); }

		// Whether every counted slot is satisfied.
		constexpr bool IsComplete() const noexcept
		{
			return doneCount >= requiredCount;
		}

		// Whether this slot is one of the counted ones.
		constexpr bool IsCounted(uint8_t slot) const noexcept
		{
			return slot < requiredCount;
		}
		constexpr bool operator==(const CodexProgress& other) const noexcept
		{
			return id == other.id && type == other.type && requirements == 
			       other.requirements &&
			       requiredCount == other.requiredCount && done == other.done &&
			       doneCount == other.doneCount;
		}

		// Whether any slot inside the counted window names an item, which is
		// what makes a registration able to satisfy this record. Mirrors
		// CodexDefinition::IsCompletable, but over a captured record so a
		// reconciliation can ask about a retuned entry.
		constexpr bool IsCompletable() const noexcept
		{
			for (uint8_t slot = 0; slot < requiredCount; ++slot)
			{
				if (requirements[slot].IsConfigured())
				{
					return true;
				}
			}
			return false;
		}
	};

	// Why a registration was refused, so a caller can say something better than
	// "invalid argument" and so a test can pin which rule fired.
	enum class CodexRegisterError : uint8_t
	{
		None = 0,
		UnknownCodex,        // no such entry is in progress and none is completed
		AlreadyCompleted,    // the entry is already done, so it cannot be registered into again
		InvalidItem,         // the item instance is not usable
		NotCompletable,      // the entry requires no items, so nothing can complete it
	};

	const char* ToString(CodexRegisterError error) noexcept;

	// What one registration did.
	//
	// Reported rather than returned as a bare success, because a registration
	// that matched nothing and one that completed an entry are both "the call
	// worked" and mean opposite things to a caller deciding what to do with the
	// item.
	struct CodexRegistration
	{
		CodexRegisterError error = CodexRegisterError::None;

		// True when at least one slot was satisfied by this call. False means the
		// item matched no requirement, matched one that was already done, or
		// named the right item with the wrong count.
		//
		// This is the value a caller must use to decide whether the registered
		// item is spent. RAN does not: GLChar::MsgReqRegisterCodex deletes the
		// item unconditionally after DoCodexRegisterItem returns
		// (GLCharInvenMsg.cpp:9195-9199), so a registration that matched nothing
		// still costs the player the item. That is a data-loss bug and is not
		// reproduced. See §6 of the investigation.
		bool recorded = false;

		// True when this call completed the entry, which also means `recorded`
		// is true.
		bool completed = false;

		// The state of the entry after the call.
		uint8_t doneCount     = 0;
		uint8_t requiredCount = 0;

		constexpr bool IsOk() const noexcept { return error == CodexRegisterError::None; }
	};

	// A character's codex state: the entries in progress and the entries done.
	//
	// A value type - copying copies both sets.
	class CodexState
	{
	public:
		// Brings the state in line with a set of definitions.
		//
		// This is RAN's character-load reconciliation, `SETCODEX_BYBUF`
		// (GLCharDataCodex.cpp:40-134), and the GM/new-character variant in
		// GLCharDataLoad.cpp:357-373, in one operation. For every definition:
		//
		//   - already completed: left alone, never re-seated. This is what makes
		//     the reward exactly-once across a reload, in RAN because a done
		//     entry is skipped at GLCharDataCodex.cpp:79-80 and in modern for
		//     the same reason;
		//   - in progress: its captured requirements are refreshed from the
		//     definition, and its type re-read. If the type changed, the record
		//     is re-seated from zero, as RAN does at GLCodexData.cpp:486-494.
		//   - otherwise: seated with zero progress.
		//
		// Then any record whose definition no longer exists is dropped, which
		// RAN also does (GLCharDataCodex.cpp:95-132).
		//
		// One deliberate difference: RAN's Correction refreshes only the five item
		// ids and leaves the captured quantities and grades stale
		// (GLCodexData.cpp:497-506), so re-tuning a codex table mid-season
		// leaves existing characters holding old requirements. Modern refreshes
		// the whole requirement and recomputes the required count. §5 of the
		// investigation.
		void Reconcile(const CodexDefinitionProvider& definitions);

		// The progress record for an entry, or an empty record when the entry is
		// not in progress.
		CodexProgress GetProgress(CodexId id) const noexcept;

		// The record for a completed entry, or an empty record when it is not
		// completed. Mirrors GLCHARLOGIC::GetCodexDone
		// (GLogixExPC.cpp:5078), which returns null rather than an empty record.
		CodexProgress GetCompleted(CodexId id) const noexcept;

		bool IsInProgress(CodexId id) const noexcept;
		bool IsCompleted(CodexId id) const noexcept;

		size_t GetProgressCount() const noexcept { return m_progress.size(); }
		size_t GetCompletedCount() const noexcept { return m_completed.size(); }

		const std::map<CodexId, CodexProgress>& GetAllProgress() const noexcept
		{
			return m_progress;
		}
		const std::map<CodexId, CodexProgress>& GetAllCompleted() const noexcept
		{
			return m_completed;
		}

		// Registers one stack of an item against one entry.
		//
		// This is GLChar::DoCodexRegisterItem (GLCharCodex.cpp:60-179) and it is
		// the only way progress advances in the modern tree. The rules, all
		// transcribed:
		//
		//   - the entry must be in progress. A completed entry is refused, and
		//     RAN refuses it a step earlier at the request handler
		//     (GLCharInvenMsg.cpp:9130-9136);
		//   - every counted slot naming this item and not already done is
		//     satisfied, and each one increments the count separately. The slots
		//     are independent `if`s in RAN, not `else if`
		//     (GLCharCodex.cpp:75-144), so one stack can satisfy two slots when
		//     an entry names the same item twice;
		//   - a slot matches when the item matches and the stack count equals the
		//     required quantity exactly (GLCharCodex.cpp:81, :95, :109, :123,
		//     :137);
		//   - the required grade is not checked. See CodexRequirement.
		//
		// When every counted slot is satisfied the entry is moved from progress
		// to completed and `completed` is set. RAN clamps the counter to the
		// maximum first (GLCharCodex.cpp:149) and erases the progress record in
		// a second pass (GLCharCodex.cpp:165-172), which is how a completed
		// entry can never be found by this function again.
		//
		// `recorded` is false when the call matched nothing. That is a success,
		// not a failure: the call was well-formed and the state is unchanged.
		// Callers that spend the item must test `recorded`.
		Result<CodexRegistration> RegisterItem(CodexId id, const ItemInstance& item);

		void ClearProgress() noexcept { m_progress.clear(); }
		void ClearCompleted() noexcept { m_completed.clear(); }
		void Clear() noexcept { m_progress.clear(); m_completed.clear(); }

		bool operator==(const CodexState& other) const noexcept
		{
			return m_progress == other.m_progress && m_completed == other.m_completed;
		}
		bool operator!=(const CodexState& other) const noexcept { return !(*this == other); }

	private:
		// Builds a zero-progress record from a definition. RAN's
		// SCODEX_CHAR_DATA::Assign (GLCodexData.cpp:371-413).
		static CodexProgress Seat(const CodexDefinition& definition);

		std::map<CodexId, CodexProgress> m_progress;
		std::map<CodexId, CodexProgress> m_completed;
	};
}
