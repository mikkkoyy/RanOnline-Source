// VERTICAL-004: the codex definition provider. See CodexDefinitionProvider.h.

#include "progression/CodexDefinitionProvider.h"

#include <algorithm>

namespace Modern
{
	namespace
	{
		bool ById(const CodexDefinition& lhs, const CodexDefinition& rhs)
		{
			return lhs.id < rhs.id;
		}
	}

	Status InMemoryCodexDefinitions::Add(const CodexDefinition& definition)
	{
		if (!definition.IsValid())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const auto position = std::lower_bound(m_definitions.begin(), m_definitions.end(),
		                                        definition, ById);
		if (position != m_definitions.end() && position->id == definition.id)
		{
			*position = definition;  // replace in place
			return Ok();
		}
		m_definitions.insert(position, definition);
		return Ok();
	}

	Status InMemoryCodexDefinitions::Remove(CodexId id)
	{
		const auto position = std::lower_bound(m_definitions.begin(), m_definitions.end(),
		                                        id,
		                                        [](const CodexDefinition& lhs, CodexId rhs)
		                                        { return lhs.id < rhs; });
		if (position == m_definitions.end() || position->id != id)
		{
			return Status(ErrorCode::NotFound);
		}
		m_definitions.erase(position);
		return Ok();
	}

	const CodexDefinition* InMemoryCodexDefinitions::Find(CodexId id) const
	{
		if (!id.IsValid())
		{
			return nullptr;
		}
		const auto position = std::lower_bound(m_definitions.begin(), m_definitions.end(),
		                                        id,
		                                        [](const CodexDefinition& lhs, CodexId rhs)
		                                        { return lhs.id < rhs; });
		if (position == m_definitions.end() || position->id != id)
		{
			return nullptr;
		}
		return &*position;
	}
}
