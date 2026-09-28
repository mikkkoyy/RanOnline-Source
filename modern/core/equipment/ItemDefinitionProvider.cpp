// VERTICAL-002: the item definition provider. See ItemDefinitionProvider.h.

#include "equipment/ItemDefinitionProvider.h"

#include <algorithm>

namespace Modern
{
	namespace
	{
		bool ById(const ItemDefinition& lhs, const ItemDefinition& rhs)
		{
			return lhs.id < rhs.id;
		}
	}

	Status InMemoryItemDefinitions::Add(const ItemDefinition& definition)
	{
		if (!definition.IsValid() || !definition.stats.IsFinite())
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

	Status InMemoryItemDefinitions::Remove(ItemId id)
	{
		const auto position = std::lower_bound(m_definitions.begin(), m_definitions.end(),
		                                        id,
		                                        [](const ItemDefinition& lhs, ItemId rhs)
		                                        { return lhs.id < rhs; });
		if (position == m_definitions.end() || position->id != id)
		{
			return Status(ErrorCode::NotFound);
		}
		m_definitions.erase(position);
		return Ok();
	}

	const ItemDefinition* InMemoryItemDefinitions::Find(ItemId id) const
	{
		if (!id.IsValid())
		{
			return nullptr;
		}
		const auto position = std::lower_bound(m_definitions.begin(), m_definitions.end(),
		                                        id,
		                                        [](const ItemDefinition& lhs, ItemId rhs)
		                                        { return lhs.id < rhs; });
		if (position == m_definitions.end() || position->id != id)
		{
			return nullptr;
		}
		return &*position;
	}
}
