#pragma once

// VERTICAL-001: the gameplay contract that crosses the client/server boundary.
//
// A value, not a protocol. It is the whole of what a client needs to
// represent a character and the whole of what a server chooses to publish, and
// it contains nothing that belongs to either side's implementation.
//
// It carries no socket, no buffer, no serialisation format, no database
// handle, no renderer object and no Windows type. Transport is a separate
// milestone; when it exists it encodes this struct, and the encoding is not
// this struct's business.
//
// What is here, and why each field earns the crossing:
//
//   - identity (id, name) and the facts a character is made of (class, level,
//     experience, allocated stats): the client cannot derive these, and RAN's
//     server is what owns them (`SCHARDATA`, GLCharData.h:566).
//   - the derived statistics, as a *result*: RAN recomputes these on the client
//     from shared code, but modern computes them once on the server. See the
//     note below, because this is the one place modern deliberately parts
//     company with RAN.
//   - the current resource pools: mutable, changed by combat and consumption,
//     and therefore something the client must be told rather than compute.
//   - the summed base stats: the input every derived value was built from, so
//     a status window can show the allocation without asking again.
//
// What is deliberately absent: the class table row (server data, resolved
// before the snapshot is taken), the stat *inputs* that would let the client
// recompute anything (the equipment, passive and codex contribution blocks, and
// the per-item and per-skill bonus values behind them), and anything RAN keeps
// server-only. The equipment, skill and codex *lists* below are published - a
// panel needs them - but they carry no bonus values, so a client that added them
// up would be a second implementation of the stat pipeline.

// The ownership of derived statistics, stated once, because it is the whole
// point of this milestone.
//
//   Verified legacy behaviour: RAN compiles one class, GLCHARLOGIC, into both
//   the client and the server, and `SUM_ADDITION` lives in it
//   (GLogixExPC.cpp:286). The client therefore recomputes maxima locally, and
//   the server only ever sends the *current* values - `m_sHP.dwNow`,
//   `m_sMP.dwNow`, `m_sSP.dwNow` from a skill-consume feedback
//   (GLCharacterMsg.cpp:1022-1024). Maxima are never transmitted. The one
//   thing that keeps the two sides agreeing is the point rate, which the server
//   supplies in the message that triggers the client's recalculation
//   (`pNetMsg->fCONFT_HP_RATE`, GLCharacterMsg.cpp:796).
//
//   Modern design decision: that duplication is not reproduced. RAN's client
//   and server run the same formula, which is why they agree, but it means
//   there are two implementations to keep in step for the client to display
//   anything at all. Here there is exactly one implementation, the server
//   calls it, and the client receives the answer.
//
//   The consequence for a client: it cannot recompute a derived value, and it
//   is not supposed to. It presents what it was given. If the two sides ever
//   disagree, the client is wrong and the snapshot is right.

#include "character/Character.h"
#include "equipment/EquipmentState.h"
#include "character/CharacterClassTable.h"
#include "math/Vector3.h"
#include "progression/CodexDefinition.h"
#include "skills/SkillState.h"
#include "stats/BaseStats.h"
#include "stats/DerivedStats.h"
#include "types/Ids.h"
#include "types/Result.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace Modern::Gameplay
{
	// The current value of one of RAN's three resources.
	//
	// RAN calls this a `GLDWDATA` pool of `dwNow` and `dwMax`
	// (GLDefine.h:400). Only `dwNow` is ever synchronised, so only the current
	// value crosses; the maximum is part of `DerivedStats`.
	struct ResourcePool
	{
		uint32_t current = 0;

		constexpr bool operator==(const ResourcePool& other) const noexcept
		{
			return current == other.current;
		}
	};

	// One worn item, as the client is entitled to know about it.
	//
	// The server publishes what a slot holds so a HUD can name the item without
	// a second item database on the client. It deliberately carries no stat
	// values: those are in `derived`, computed once by the server. A client that
	// received per-item bonuses and added them up itself would be a second
	// implementation, which is what this milestone exists to prevent.
	//
	// `name` is included so a UI needs no lookup to label the slot.
	struct EquippedItem
	{
		EquipmentSlot slot = EquipmentSlot::Headgear;
		ItemId        definition = ItemId::MakeInvalid();
		uint64_t      serial   = 0;
		ItemKind      kind     = ItemKind::None;
		std::string   name;

		constexpr bool operator==(const EquippedItem& other) const noexcept
		{
			return slot == other.slot && definition == other.definition &&
			       serial == other.serial && kind == other.kind &&
			       name == other.name;
		}
	};

	// A fixed-capacity list of learned skills, so a snapshot is a value.
	//
	// The client is entitled to know what passive skills the character has
	// learned and at what level, so a skill UI can display them. It deliberately
	// carries no stat values: those are in `derived`, computed once by the
	// server. A client that received per-skill bonuses and added them up itself
	// would be a second implementation, which is what this milestone exists to
	// prevent.
	//
	// The list is sparse: skills are stored at their definition's skill index
	// within class, so iteration order matches the server's aggregation order.
	// Since we don't know the max number of skills per class, we use a vector
	// of learned skill entries instead of a fixed array.
	struct LearnedSkillEntry
	{
		SkillId  id = SkillId{};
		uint8_t  level = 0;
		std::string name;   // For UI display without a second lookup.

		constexpr bool IsLearned() const noexcept { return level > 0; }

		constexpr bool operator==(const LearnedSkillEntry& other) const noexcept
		{
			return id == other.id && level == other.level && name == other.name;
		}
	};

    // A variable-capacity list of learned skills.
    // Using vector instead of fixed array because the max skill count per class
    // is not a fixed constant like equipment slots.
    struct SkillList
    {
        std::vector<LearnedSkillEntry> skills;

        // Find a learned skill by ID.
        const LearnedSkillEntry* Find(const SkillId& id) const noexcept
        {
            for (const auto& entry : skills)
            {
                if (entry.id == id)
                {
                    return &entry;
                }
            }
            return nullptr;
        }

        // Check if a skill is learned.
        bool Has(const SkillId& id) const noexcept
        {
            return Find(id) != nullptr;
        }

        // Get the level of a learned skill, or 0 if not learned.
        uint8_t GetLevel(const SkillId& id) const noexcept
        {
            if (const auto* entry = Find(id))
            {
                return entry->level;
            }
            return 0;
        }

        size_t GetLearnedCount() const noexcept { return skills.size(); }

bool operator==(const SkillList& other) const noexcept
    {
        return skills == other.skills;
    }
};

    // One codex entry, as the client is entitled to know about it.
    //
    // The client needs the entry's name to label a codex list, its type to pick
    // an icon or a category, and how far along it is to draw a progress bar. It
    // deliberately carries no reward values and no requirement details: the
    // bonus is already folded into `derived` by the server, and the requirements
    // are server data the client has no business reading.
    //
    // `description` is empty for entries that have none, which is the common
    // case; it is here so a tooltip needs no second lookup. `badge` is the
    // cosmetic badge RAN shows next to a completed entry and is likewise
    // presentation only - RAN never grants a codex badge, see
    // docs/reference/client/VERTICAL-004_CODEX_INVESTIGATION.md §8.
    struct CodexEntry
    {
        CodexId   id = CodexId::MakeInvalid();
        CodexType type = CodexType::ReachLevel;
        std::string name;
        std::string description;
        std::string badge;

        // Whether the entry is completed, and the progress counters for a codex
        // panel. `requiredCount` is 1..5 for an entry the server can complete
        // and 0 for one that names no items, which the server publishes rather
        // than hides.
        bool     completed    = false;
        uint8_t  doneCount     = 0;
        uint8_t  requiredCount = 0;

        constexpr bool IsComplete() const noexcept { return completed; }

        // Progress as a fraction, for a bar. 0 for an entry that requires
        // nothing, rather than dividing by it.
        float GetProgressFraction() const noexcept;

        bool operator==(const CodexEntry& other) const noexcept
        {
            return id == other.id && type == other.type && name == other.name &&
                   description == other.description && badge == other.badge &&
                   completed == other.completed && doneCount == other.doneCount &&
                   requiredCount == other.requiredCount;
        }
    };

    // A variable-capacity list of codex entries the character holds.
    //
    // Both maps from RAN are flattened into one list because the client is
    // entitled to see either: a codex panel shows finished and unfinished
    // entries together, and `completed` says which is which. RAN sends them as
    // two separate blocks in the same join message
    // (GLCharEx.cpp:445-465, and the client's two inserts at
    // DxGameStage.cpp:931 and :950).
    //
    // The list is sorted by CodexId, which is the order the server's ordered
    // maps produce and therefore the order a legacy client would have shown
    // them in.
    struct CodexList
    {
        std::vector<CodexEntry> entries;

        const CodexEntry* Find(CodexId id) const noexcept
        {
            for (const auto& entry : entries)
            {
                if (entry.id == id)
                {
                    return &entry;
                }
            }
            return nullptr;
        }

        bool Has(CodexId id) const noexcept
        {
            return Find(id) != nullptr;
        }

        // Whether an entry is completed. An entry the character does not have at
        // all is not completed.
        bool IsCompleted(CodexId id) const noexcept
        {
            const CodexEntry* entry = Find(id);
            return entry != nullptr && entry->completed;
        }

        bool IsInProgress(CodexId id) const noexcept
        {
            const CodexEntry* entry = Find(id);
            return entry != nullptr && !entry->completed;
        }

        // How many of each kind, for a "12 / 40" header. Counted from the
        // published entries rather than sent as a separate number, so there is
        // no second figure for a client to hold that disagrees with the list it
        // was sent.
        size_t GetCount() const noexcept { return entries.size(); }
        size_t GetCompletedCount() const noexcept
        {
            size_t completed = 0;
            for (const auto& entry : entries)
            {
                if (entry.completed)
                {
                    ++completed;
                }
            }
            return completed;
        }
        size_t GetInProgressCount() const noexcept
        {
            return entries.size() - GetCompletedCount();
        }

        bool operator==(const CodexList& other) const noexcept
        {
            return entries == other.entries;
        }
    };


// A fixed-capacity list of worn items, so a snapshot is a value: it must be
// copyable, comparable and free of allocation-time surprises. kEquipmentSlotCount
// is RAN's wearable range, so it is the most entries a character can ever have.
//
// Defined after EquippedItem rather than nested in it: it holds
// `std::array<EquippedItem, kEquipmentSlotCount>`, which requires EquippedItem
// to be complete. A separate type is the honest shape; the EquippedItem name
// is just an organizational qualifier, not ownership.
struct EquippedList
	{
		std::array<EquippedItem, kEquipmentSlotCount> items{};
		size_t count = 0;

		// Slots in slot order, which is the order the server aggregated them
		// in. A caller can iterate the whole array; `count` says how much of it
		// is meaningful.
		const std::array<EquippedItem, kEquipmentSlotCount>& GetItems() const noexcept
		{
			return items;
		}

		// The item in one slot, or an empty entry when nothing is worn there
		// or the slot is not wearable.
		const EquippedItem& Get(EquipmentSlot slot) const noexcept
		{
			if (!IsValidSlot(slot))
			{
				return items[0];
			}
			return items[static_cast<size_t>(slot)];
		}

		bool Has(EquipmentSlot slot) const noexcept
		{
			return Get(slot).definition.IsValid();
		}

		size_t GetOccupiedCount() const noexcept { return count; }

		constexpr bool operator==(const EquippedList& other) const noexcept
		{
			return count == other.count && items == other.items;
		}
	};

	// One character, as the client is entitled to see it.
	//
	// A plain value with a validating factory. There is no mutator: a snapshot
	// is what the server said at a point in time, and the client's response to
	// a newer one is to hold a different snapshot rather than edit this one.
	struct CharacterSnapshot
	{
		CharacterId       id;
		std::string       name;
		CharacterClass    characterClass = CharacterClass::Unset;
		CharacterGender   gender         = CharacterGender::Male;
		uint16_t          level          = Stats::kMinLevel;
		int64_t           experience     = 0;

		// The six stats as allocated onto the character, and their sum after
		// class, level, allocation and equipment.
		Stats::BaseStats    allocatedStats;
		Stats::BaseStats    totalStats;

		// The result of Modern::Stats::Calculate, received rather than
		// recomputed.
		Stats::DerivedStats derived;

		// What the server says is worn. Present so the client can present it;
		// it never contributes to a value in this struct.
		EquippedList equipped;

		// What the server says the character has learned. Present so the client
		// can present it; it never contributes to a value in this struct.
		SkillList skills;

		// What the server says about the character's codex. Present so the client
		// can present it; it never contributes to a value in this struct, because
		// the bonuses are already in `derived`.
		CodexList codex;

		ResourcePool hp;
		ResourcePool mp;
		ResourcePool sp;

		Vector3 position;

		constexpr bool operator==(const CharacterSnapshot& other) const noexcept
		{
			return id == other.id && name == other.name &&
			       characterClass == other.characterClass && gender == other.gender &&
			       level == other.level && experience == other.experience &&
			       allocatedStats == other.allocatedStats && totalStats == other.totalStats &&
			       derived == other.derived && hp == other.hp && mp == other.mp &&
			       sp == other.sp && equipped == other.equipped && skills == other.skills &&
			       codex == other.codex && position == other.position;
		}

		// A snapshot is valid when it identifies a character that could exist:
		// a valid id, a non-empty name within the identity limit, a real class,
		// a level in RAN's range, a non-negative experience, and a current pool
		// that does not exceed the maximum the server published.
		static bool IsValid(const CharacterSnapshot& snapshot) noexcept;

		// Builds a validated snapshot, failing with InvalidArgument rather than
		// producing one a client would have to second-guess.
		static Result<CharacterSnapshot> Create(CharacterSnapshot snapshot) noexcept;

		// The fraction of the maximum resource remaining, for a future bar.
		// Returns 0 for a zero maximum rather than dividing by it, and is
		// clamped to [0, 1] so a stale current value cannot render a bar past
		// its end. Presentation only: it derives nothing about the character.
		float GetHealthFraction() const noexcept;
		float GetManaFraction() const noexcept;
		float GetStaminaFraction() const noexcept;
	};
}
