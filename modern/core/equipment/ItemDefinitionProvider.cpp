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

	InMemoryItemDefinitions::BulkResult InMemoryItemDefinitions::AddAll(
	    const std::vector<ItemDefinition>& definitions)
	{
		// Append, then sort once. `Add`'s sorted insert is quadratic over a
		// whole table, and the deployed item export is 18,447 rows - so this is
		// the difference between a load that finishes and one that a watchdog
		// kills.
		//
		// Validation happens on the way in, so a refused definition never
		// reaches the container at all.
		BulkResult result;
		std::vector<ItemDefinition> incoming;
		incoming.reserve(definitions.size());

		for (const ItemDefinition& definition : definitions)
		{
			if (!definition.IsValid() || !definition.stats.IsFinite())
			{
				++result.refused;
				continue;
			}
			incoming.push_back(definition);
		}

		// Sort by id so `Find` stays a binary search and registration order
		// cannot change what a lookup returns - the same invariant `Add` keeps,
		// established here in one pass instead of n.
		std::stable_sort(incoming.begin(), incoming.end(), ById);

		// Merge into what is already there. A duplicate id keeps the LAST one,
		// which is `Add`'s rule, so a caller that layers a private set under a
		// deployed one still wins.
		if (m_definitions.empty())
		{
			result.added = static_cast<std::size_t>(incoming.size());
			m_definitions = std::move(incoming);
			return result;
		}

		std::vector<ItemDefinition> merged;
		merged.reserve(m_definitions.size() + incoming.size());

		std::size_t left = 0;
		std::size_t right = 0;
		while (left < m_definitions.size() && right < incoming.size())
		{
			if (m_definitions[left].id < incoming[right].id)
			{
				merged.push_back(std::move(m_definitions[left++]));
			}
			else if (incoming[right].id < m_definitions[left].id)
			{
				merged.push_back(std::move(incoming[right++]));
				++result.added;
			}
			else
			{
				merged.push_back(std::move(incoming[right++]));
				++left;
				++result.replaced;
			}
		}
		while (left < m_definitions.size())
		{
			merged.push_back(std::move(m_definitions[left++]));
		}
		while (right < incoming.size())
		{
			merged.push_back(std::move(incoming[right++]));
			++result.added;
		}

		m_definitions = std::move(merged);
		return result;
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
