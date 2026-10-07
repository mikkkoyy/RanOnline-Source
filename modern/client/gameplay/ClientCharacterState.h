#pragma once

// VERTICAL-001: the client's character state.
//
// This holds the last authoritative snapshot and presents it. It does not
// decide anything, and in particular it cannot compute a derived statistic:
// it has no stat input, no class table and no calculator, and the only way to
// change what it holds is to hand it a snapshot the server produced.
//
// That is a deliberate departure from RAN, and it is enforced structurally
// rather than by convention. RAN's client reruns `SUM_ADDITION` on every
// equipment change (GLCharacterMsg.cpp:766, :796) and displays the result, so
// the formula is executed in two places. Here it is executed in one, and this
// type has no path to a second one.
//
// The presentation helpers exist so a future HUD has something honest to bind
// to, and they derive only ratios of published values - they never invent a
// number the server did not send.

#include "character/Character.h"
#include "character/CharacterClassTable.h"
#include "equipment/EquipmentState.h"
#include "gameplay/CharacterSnapshot.h"
#include "math/Vector3.h"
#include "stats/DerivedStats.h"
#include "types/Result.h"

#include <cstdint>

namespace Modern::Client::Gameplay
{
	// The client's view of one character.
	//
	// Default-constructed it holds nothing, and `HasSnapshot()` is false until
	// a snapshot arrives. Every accessor is only meaningful once it is, and each
	// one says so by returning a defined empty answer rather than a plausible
	// wrong one.
	class ClientCharacterState
	{
	public:
		// Adopts a snapshot. Fails with InvalidArgument if the snapshot is not
		// one this client could have received, which is the boundary check that
		// keeps a malformed or hostile payload from becoming state.
		Status Apply(const Modern::Gameplay::CharacterSnapshot& snapshot);

		// Forgets the character, e.g. on disconnect or character change. The
		// only unconditional transition.
		void Clear() noexcept;

		bool HasSnapshot() const noexcept { return m_hasSnapshot; }

		// Identifies the snapshot, or an invalid id when there is none.
		CharacterId GetId() const noexcept;
		const std::string& GetName() const noexcept;

		CharacterClass GetClass() const noexcept;
		CharacterGender GetGender() const noexcept;
		uint16_t GetLevel() const noexcept;
		int64_t GetExperience() const noexcept;

		// The derived statistics *as received*. There is no recalculation and no
		// setter for a single field: a change comes as a new snapshot.
		const Stats::DerivedStats& GetDerivedStats() const noexcept;
		const Stats::BaseStats& GetAllocatedStats() const noexcept;
		const Stats::BaseStats& GetTotalStats() const noexcept;

		uint32_t GetMaxHp() const noexcept;
		uint32_t GetMaxMp() const noexcept;
		uint32_t GetMaxSp() const noexcept;
		uint32_t GetCurrentHp() const noexcept;
		uint32_t GetCurrentMp() const noexcept;
		uint32_t GetCurrentSp() const noexcept;

		const Vector3& GetPosition() const noexcept;

		// Ratios for a future status window. Zero when no snapshot is held, and
		// clamped to [0, 1] because the server may have published a current
		// value from before a maximum dropped.
		float GetHealthFraction() const noexcept;
		float GetManaFraction() const noexcept;
		float GetStaminaFraction() const noexcept;

		// WORLD-ENTRY-002h: applies a 3046's CURRENT values to the presented
		// pools. The maxima in the frame are exposed for assertion but do not
		// mutate `derived`, which is the server's stat result and the client's
		// only source of maxima (see the class header note).
		void ApplyResourceUpdate(uint32_t hp, uint32_t mp, uint32_t sp);

		// ---- Equipment ----
		//
		// What the server said is worn. A read-only view: the client cannot
		// equip, unequip, or change a slot, because the server owns the worn set
		// and a client that could change it would be a second authority.
		const Modern::Gameplay::EquippedList& GetEquipment() const noexcept
		{
			return m_hasSnapshot ? m_snapshot.equipped : EmptyEquipped();
		}

		// The item in one slot, or an empty entry when the slot is empty, the
		// slot is not wearable, or no snapshot has been received.
		const Modern::Gameplay::EquippedItem& GetEquippedItem(EquipmentSlot slot) const noexcept
		{
			return m_hasSnapshot ? m_snapshot.equipped.Get(slot) : EmptyEquipped().Get(slot);
		}

		bool HasEquipped(EquipmentSlot slot) const noexcept
		{
			return m_hasSnapshot && m_snapshot.equipped.Has(slot);
		}

		// The occupied slot count, for an equipment panel.
		size_t GetOccupiedSlotCount() const noexcept
		{
			return m_hasSnapshot ? m_snapshot.equipped.GetOccupiedCount() : 0u;
		}

		// ---- Skills (VERTICAL-003) ----
		//
		// What the server said the character has learned. A read-only view: the
		// client cannot learn, unlearn, or level up skills, because the server
		// owns the skill state and a client that could change it would be a
		// second authority.
		const Modern::Gameplay::SkillList& GetSkills() const noexcept
		{
			return m_hasSnapshot ? m_snapshot.skills : EmptySkills();
		}

		// Check if a specific skill is learned.
		bool HasSkill(const SkillId& id) const noexcept
		{
			return m_hasSnapshot && m_snapshot.skills.Has(id);
		}

		// Get the level of a learned skill, or 0 if not learned.
		uint8_t GetSkillLevel(const SkillId& id) const noexcept
		{
			return m_hasSnapshot ? m_snapshot.skills.GetLevel(id) : 0u;
		}

		// The learned skill count, for a skill panel.
		size_t GetLearnedSkillCount() const noexcept
		{
			return m_hasSnapshot ? m_snapshot.skills.GetLearnedCount() : 0u;
		}

		static const Modern::Gameplay::SkillList& EmptySkills() noexcept;

		static const Modern::Gameplay::EquippedList& EmptyEquipped() noexcept;

		// ---- Codex (VERTICAL-004) ----
		//
		// What the server said about the character's codex. A read-only view: the
		// client cannot register an item, complete an entry, or change a
		// counter, because the server owns the codex set and a client that could
		// change it would be a second authority.
		//
		// There is no codex contribution here to recompute and no requirement
		// list to match against, which is the structural half of the same
		// guarantee the equipment and skill views have: the client holds a
		// description of the codex and the *result* in `derived`, and no path to
		// producing a third.
		//
		// RAN's client breaks that guarantee for the codex specifically: it keeps
		// its own `m_mapCodexProg` / `m_mapCodexDone` mirror
		// (GLCharacterMsg.cpp:5193, :5206-5212) and calls its own `CODEX_STATS`
		// on completion (GLCharacterMsg.cpp:5214), so the formula runs on both
		// sides. See the note in core/gameplay/CharacterSnapshot.h.
		const Modern::Gameplay::CodexList& GetCodex() const noexcept
		{
			return m_hasSnapshot ? m_snapshot.codex : EmptyCodex();
		}

		// One codex entry, or an empty entry when the character does not have it
		// or no snapshot has been received.
		const Modern::Gameplay::CodexEntry& GetCodexEntry(CodexId id) const noexcept
		{
			const Modern::Gameplay::CodexEntry* entry = GetCodex().Find(id);
			return entry != nullptr ? *entry : EmptyCodexEntry();
		}

		// Whether the character holds an entry at all, finished or not.
		bool HasCodex(CodexId id) const noexcept
		{
			return m_hasSnapshot && m_snapshot.codex.Has(id);
		}

		// Whether an entry is completed. False for an entry the character does not
		// have, which is a different answer from "in progress" and is why
		// HasCodex exists alongside.
		bool IsCodexCompleted(CodexId id) const noexcept
		{
			return m_hasSnapshot && m_snapshot.codex.IsCompleted(id);
		}

		// Whether an entry is held and not yet completed.
		bool IsCodexInProgress(CodexId id) const noexcept
		{
			return m_hasSnapshot && m_snapshot.codex.IsInProgress(id);
		}

		// The held, completed and in-progress counts, for a codex panel header.
		size_t GetCodexCount() const noexcept
		{
			return m_hasSnapshot ? m_snapshot.codex.GetCount() : 0u;
		}
		size_t GetCompletedCodexCount() const noexcept
		{
			return m_hasSnapshot ? m_snapshot.codex.GetCompletedCount() : 0u;
		}
		size_t GetInProgressCodexCount() const noexcept
		{
			return m_hasSnapshot ? m_snapshot.codex.GetInProgressCount() : 0u;
		}

		static const Modern::Gameplay::CodexList& EmptyCodex() noexcept;

	private:
		static const Stats::DerivedStats& EmptyDerived() noexcept;
		static const Stats::BaseStats& EmptyStats() noexcept;
		static const std::string& EmptyName() noexcept;
		static const Vector3& EmptyPosition() noexcept;
		static const Modern::Gameplay::CodexEntry& EmptyCodexEntry() noexcept;

		bool                            m_hasSnapshot = false;
		Modern::Gameplay::CharacterSnapshot m_snapshot;
	};
}
