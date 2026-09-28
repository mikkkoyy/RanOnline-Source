// VERTICAL-003: skill state implementation.

#include "skills/SkillState.h"

namespace Modern
{
	Status SkillState::LearnSkill(const SkillId& id)
	{
		if (!id.IsValid())
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (m_skills.find(id) != m_skills.end())
		{
			return Status(ErrorCode::AlreadyExists);
		}
		m_skills[id] = LearnedSkill{ id, 1 };
		return Ok();
	}

	Status SkillState::UnlearnSkill(const SkillId& id)
	{
		auto it = m_skills.find(id);
		if (it == m_skills.end())
		{
			return Status(ErrorCode::NotFound);
		}
		m_skills.erase(it);
		return Ok();
	}

	Status SkillState::SetSkillLevel(const SkillId& id, uint8_t level)
	{
		if (level == 0 || level > kMaxSkillLevel)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		auto it = m_skills.find(id);
		if (it == m_skills.end())
		{
			return Status(ErrorCode::NotFound);
		}
		it->second.level = level;
		return Ok();
	}

	LearnedSkill SkillState::GetSkill(const SkillId& id) const noexcept
	{
		auto it = m_skills.find(id);
		if (it == m_skills.end())
		{
			return LearnedSkill{ id, 0 };
		}
		return it->second;
	}

	bool SkillState::HasSkill(const SkillId& id) const noexcept
	{
		auto it = m_skills.find(id);
		return it != m_skills.end() && it->second.IsLearned();
	}

	uint8_t SkillState::GetSkillLevel(const SkillId& id) const noexcept
	{
		auto it = m_skills.find(id);
		if (it == m_skills.end())
		{
			return 0;
		}
		return it->second.level;
	}
}