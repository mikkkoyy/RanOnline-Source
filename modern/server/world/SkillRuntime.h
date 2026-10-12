#pragma once

// SKILL-018: authoritative skill state for a live world character.
//
// ---------------------------------------------------------------------------
// WHAT THIS IS
// ---------------------------------------------------------------------------
//
// A focused, transport-free component that owns the skill-specific state a
// world character carries: which skills are learned, at what level, and what
// cooldowns are running. It is the world-server counterpart to the skill
// half of ServerCharacter, without any of the state that already has an
// authoritative owner elsewhere.
//
// ---------------------------------------------------------------------------
// WHAT THIS DELIBERATELY DOES NOT OWN
// ---------------------------------------------------------------------------
//
//   * Position          - WorldMovementRuntime actors, keyed by sessionId
//   * Current HP/MP/SP  - ResourceSyncService, keyed by gaeaId
//   * Derived stats      - ICombatStatsProvider over WorldCharacter
//   * Identity/session  - FieldSession and WorldCharacter
//
// Owning any of those would create a second answer to a question the world
// server already answers authoritatively. The component reads what it needs
// through the services that own it; it never caches a copy.
//
// ---------------------------------------------------------------------------
// LIFECYCLE
// ---------------------------------------------------------------------------
//
// A value member of FieldSession. Created when the session is created,
// destroyed when the session is destroyed. It cannot outlive the character
// appearance it belongs to, so a reconnect starts from a clean state rather
// than inheriting a previous appearance's skills or cooldowns.
//
// ---------------------------------------------------------------------------
// TIME
// ---------------------------------------------------------------------------
//
// Cooldowns are advanced by an elapsed-seconds parameter, never by reading a
// clock. This matches ServerCharacter::AdvanceSkillCooldowns and
// ResourceSyncService::Advance, and it keeps the component deterministic
// under test.

#include "skills/SkillDefinitionProvider.h"
#include "skills/SkillState.h"
#include "types/Ids.h"
#include "types/Result.h"

#include <cstdint>
#include <map>

namespace Modern::Server::World
{
	// The authoritative skill state for one live world character.
	//
	// Transport-free: no socket, no packet, no codec, no clock, no file or
	// database access. Every dependency is injected or borrowed.
	//
	// The definition provider is BORROWED and may be null. A null provider
	// means "no skill definitions are known", which is the state a deployment
	// without a skill table is in. LearnSkill and SetSkillLevel refuse rather
	// than guess when they cannot validate against a definition.
	class SkillRuntime
	{
	public:
		// Borrows the definition provider. Null is legal and means "no skill
		// definitions are known" - see the class comment.
		explicit SkillRuntime(const SkillDefinitionProvider* definitions) noexcept
			: m_definitions(definitions)
		{
		}

		// ---- learned skills -------------------------------------------------

		// Learn a skill at level 1. Refuses if the definition provider cannot
		// resolve the skill, or if it is already learned.
		Status LearnSkill(const SkillId& id);

		// Unlearn a skill entirely. Returns NotFound if not learned.
		Status UnlearnSkill(const SkillId& id);

		// Set a learned skill to a specific level. Refuses if the skill is not
		// learned, the level is 0 or above kMaxSkillLevel, or the definition
		// provider cannot resolve the skill or reports a lower maxLevel.
		Status SetSkillLevel(const SkillId& id, uint8_t level);

		// The level of a learned skill, or 0 if not learned.
		uint8_t GetSkillLevel(const SkillId& id) const noexcept;

		// Whether a skill is learned (level > 0).
		bool HasSkill(const SkillId& id) const noexcept;

		// Read-only access to the underlying skill state.
		const SkillState& GetSkillState() const noexcept { return m_skills; }

		// ---- cooldowns ------------------------------------------------------

		// Whether a skill is currently on cooldown.
		bool IsSkillOnCooldown(const SkillId& id) const noexcept;

		// Seconds of cooldown remaining for a skill, or 0 if none.
		float GetSkillCooldownRemaining(const SkillId& id) const noexcept;

		// Starts a cooldown for a skill. A zero or negative duration is a no-op.
		void StartCooldown(const SkillId& id, float seconds) noexcept;

		// Advances every cooldown by elapsed seconds and retires the finished
		// ones. Time is passed in, never read.
		void AdvanceCooldowns(float elapsedSeconds) noexcept;

		// The cooldown map, for inspection and tests. Keyed by SkillId.
		const std::map<SkillId, float>& GetCooldowns() const noexcept
		{
			return m_cooldowns;
		}

	private:
		// Borrowed, never owned. Null is legal and means "no skill definitions
		// are known".
		const SkillDefinitionProvider* m_definitions;

		// Learned skills and their levels. Reused directly - no duplicate type.
		SkillState m_skills;

		// Per-skill cooldown, in seconds remaining. Same representation as
		// ServerCharacter::m_skillCooldowns.
		std::map<SkillId, float> m_cooldowns;
	};

} // namespace Modern::Server::World
