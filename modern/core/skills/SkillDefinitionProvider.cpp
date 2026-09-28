// VERTICAL-003: the skill definition provider implementation.

#include "skills/SkillDefinitionProvider.h"

#include <algorithm>

namespace Modern
{
	namespace
	{
		struct SkillDefComparator
		{
			using is_transparent = void;

			bool operator()(const SkillDefinition& lhs, const SkillDefinition& rhs) const noexcept
			{
				if (lhs.id.classIndex != rhs.id.classIndex)
					return lhs.id.classIndex < rhs.id.classIndex;
				return lhs.id.skillIndex < rhs.id.skillIndex;
			}

			bool operator()(const SkillDefinition& lhs, const SkillId& rhs) const noexcept
			{
				if (lhs.id.classIndex != rhs.classIndex)
					return lhs.id.classIndex < rhs.classIndex;
				return lhs.id.skillIndex < rhs.skillIndex;
			}

			bool operator()(const SkillId& lhs, const SkillDefinition& rhs) const noexcept
			{
				if (lhs.classIndex != rhs.id.classIndex)
					return lhs.classIndex < rhs.id.classIndex;
				return lhs.skillIndex < rhs.id.skillIndex;
			}

			bool operator()(const SkillId& lhs, const SkillId& rhs) const noexcept
			{
				if (lhs.classIndex != rhs.classIndex)
					return lhs.classIndex < rhs.classIndex;
				return lhs.skillIndex < rhs.skillIndex;
			}
		};
	}

Status InMemorySkillDefinitions::Add(const SkillDefinition& definition)
{
	if (!definition.IsValid())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	const auto position = std::lower_bound(m_definitions.begin(), m_definitions.end(),
	                                        definition, SkillDefComparator{});
	if (position != m_definitions.end() && position->id == definition.id)
	{
		*position = definition;  // replace in place
		return Ok();
	}
	m_definitions.insert(position, definition);
	return Ok();
}

Status InMemorySkillDefinitions::Remove(const SkillId& id)
{
	const auto position = std::lower_bound(m_definitions.begin(), m_definitions.end(),
	                                        id, SkillDefComparator{});
	if (position == m_definitions.end() || position->id != id)
	{
		return Status(ErrorCode::NotFound);
	}
	m_definitions.erase(position);
	return Ok();
}

const SkillDefinition* InMemorySkillDefinitions::Find(const SkillId& id) const
{
	if (!id.IsValid())
	{
		return nullptr;
	}
	const auto position = std::lower_bound(m_definitions.begin(), m_definitions.end(),
	                                        id, SkillDefComparator{});
	if (position == m_definitions.end() || position->id != id)
	{
		return nullptr;
	}
	return &*position;
}
}