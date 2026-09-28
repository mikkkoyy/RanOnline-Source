#pragma once

// VERTICAL-003: a character's learned skill state.
//
// This tracks which skills a character has learned and at what level.
// It is the runtime counterpart to SkillDefinition: definitions are
// immutable and shared; state is per-character and mutable.
//
// Legacy origin: SCHARSKILL (GLCharData.h:233) + SKILL_MAP (GLCharData.h:889).
// SCHARSKILL holds a skill native ID and level. The map is keyed by a
// composite DWORD of (wMainID << 16) | wSubID.

#include "skills/SkillDefinition.h"
#include "types/Ids.h"
#include "types/Result.h"

#include <map>
#include <cstdint>

namespace Modern
{
	// One learned skill entry: the skill ID and its current level.
	// Mirrors SCHARSKILL (sNativeID + wLevel).
	struct LearnedSkill
	{
		SkillId id;
		uint8_t level = 0;   // 1..kMaxSkillLevel, 0 = not learned

		constexpr bool IsLearned() const noexcept { return level > 0; }
		constexpr bool IsMaxLevel() const noexcept { return level >= kMaxSkillLevel; }

		constexpr bool operator==(const LearnedSkill& other) const noexcept
		{
			return id == other.id && level == other.level;
		}
		constexpr bool operator!=(const LearnedSkill& other) const noexcept { return !(*this == other); }
	};

	// A character's complete skill state: all learned skills.
	// This is a value type — copying copies the skill set.
	// The map is ordered by SkillId so iteration order is deterministic.
	class SkillState
	{
	public:
		// Learn a skill at level 1. Fails if already learned.
		// Returns InvalidArgument if skill ID is invalid.
		Status LearnSkill(const SkillId& id);

		// Unlearn a skill entirely. Returns NotFound if not learned.
		Status UnlearnSkill(const SkillId& id);

		// Set a learned skill to a specific level.
		// Fails if not learned, level is 0, or level > maxLevel for that skill
		// (maxLevel must be validated by caller against the definition).
		Status SetSkillLevel(const SkillId& id, uint8_t level);

		// Get the learned skill entry, or an empty one (level 0) if not learned.
		LearnedSkill GetSkill(const SkillId& id) const noexcept;

		// Check if a skill is learned (level > 0).
		bool HasSkill(const SkillId& id) const noexcept;

		// Get the level of a learned skill, or 0 if not learned.
		uint8_t GetSkillLevel(const SkillId& id) const noexcept;

		// Get all learned skills in deterministic order (by SkillId).
		const std::map<SkillId, LearnedSkill>& GetAllSkills() const noexcept
		{
			return m_skills;
		}

		// Number of learned skills.
		size_t GetLearnedCount() const noexcept { return m_skills.size(); }

		// Clear all skills.
		void Clear() noexcept { m_skills.clear(); }

		// Equality: same skills at same levels.
		bool operator==(const SkillState& other) const noexcept
		{
			return m_skills == other.m_skills;
		}
		bool operator!=(const SkillState& other) const noexcept { return !(*this == other); }

	private:
		// Ordered map for deterministic iteration (by SkillId).
		std::map<SkillId, LearnedSkill> m_skills;
	};
}