// SKILL-018: skill runtime implementation.

#include "world/SkillRuntime.h"

namespace Modern::Server::World
{
	Status SkillRuntime::LearnSkill(const SkillId& id)
	{
		if (m_definitions == nullptr)
		{
			return Status(ErrorCode::InvalidState);
		}
		if (m_definitions->Find(id) == nullptr)
		{
			return Status(ErrorCode::NotFound);
		}
		return m_skills.LearnSkill(id);
	}

	Status SkillRuntime::UnlearnSkill(const SkillId& id)
	{
		return m_skills.UnlearnSkill(id);
	}

	Status SkillRuntime::SetSkillLevel(const SkillId& id, uint8_t level)
	{
		if (m_definitions == nullptr)
		{
			return Status(ErrorCode::InvalidState);
		}
		const SkillDefinition* def = m_definitions->Find(id);
		if (def == nullptr)
		{
			return Status(ErrorCode::NotFound);
		}
		if (level > def->maxLevel)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		return m_skills.SetSkillLevel(id, level);
	}

	uint8_t SkillRuntime::GetSkillLevel(const SkillId& id) const noexcept
	{
		return m_skills.GetSkillLevel(id);
	}

	bool SkillRuntime::HasSkill(const SkillId& id) const noexcept
	{
		return m_skills.HasSkill(id);
	}

	bool SkillRuntime::IsSkillOnCooldown(const SkillId& id) const noexcept
	{
		return m_cooldowns.find(id) != m_cooldowns.end();
	}

	float SkillRuntime::GetSkillCooldownRemaining(const SkillId& id) const noexcept
	{
		const auto it = m_cooldowns.find(id);
		if (it == m_cooldowns.end())
		{
			return 0.0f;
		}
		return it->second;
	}

	void SkillRuntime::StartCooldown(const SkillId& id, float seconds) noexcept
	{
		if (seconds <= 0.0f)
		{
			return;
		}
		m_cooldowns[id] = seconds;
	}

	void SkillRuntime::AdvanceCooldowns(float elapsedSeconds) noexcept
	{
		if (elapsedSeconds <= 0.0f)
		{
			return;
		}
		for (auto it = m_cooldowns.begin(); it != m_cooldowns.end();)
		{
			it->second -= elapsedSeconds;
			if (it->second <= 0.0f)
			{
				it = m_cooldowns.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

} // namespace Modern::Server::World
