#pragma once

// VERTICAL-002: resolving an item id to its definition.
//
// Deliberately not a database. RAN loads item data from a packed, encrypted
// `item.isf` / GLogic resource through `GLItemMan`, and parsing that is out of
// scope; what is needed here is the *shape* of the lookup, so a future database
// or resource provider can satisfy the same interface without anything above it
// changing.

#include "item/ItemDefinition.h"
#include "types/Result.h"

#include <vector>

namespace Modern
{
	// Read-only access to item definitions.
	//
	// Returning a pointer rather than a copy keeps a definition shared, which is
	// what a definition is; the caller must not modify it.
	class ItemDefinitionProvider
	{
	public:
		virtual ~ItemDefinitionProvider() = default;

		// The definition for an id, or nullptr when this provider does not know
		// it. A missing definition is not an error the caller has to handle
		// differently from an unknown id, so the lookup reports absence.
		virtual const ItemDefinition* Find(ItemId id) const = 0;
	};

	// A provider over a fixed set of definitions.
	//
	// This is the implementation for a milestone with no database: the tests and
	// any headless tool register the definitions they need and the aggregation
	// behaves exactly as it would against a real provider. It is a real type
	// rather than a test fixture, so a future server can hold one without the
	// interface changing.
	class InMemoryItemDefinitions final : public ItemDefinitionProvider
	{
	public:
		// Adds or replaces a definition. A definition that is not valid, or
		// whose stat block is not finite, is refused and nothing changes.
		//
		// Adding is not reserved: the container grows, so a caller cannot
		// invalidate an existing definition by adding another.
		//
		// SINGLE items only. Loading a table through this is quadratic - the
		// container is kept sorted, so each insertion shifts the tail - and a
		// 18k-row item export exhausts a test's per-case time budget long
		// before it finishes. `AddAll` is the entry point for a table.
		Status Add(const ItemDefinition& definition);

		// Adds many definitions, sorting ONCE.
		//
		// Returns how many were refused, for the reasons `Add` refuses them, so
		// a caller can report what it dropped instead of discovering it by
		// counting afterwards. Duplicate ids resolve the same way `Add` does -
		// the LAST one wins - and are counted rather than silently replacing.
		struct BulkResult
		{
			std::size_t added   = 0;
			std::size_t refused = 0;
			std::size_t replaced = 0;
		};
		BulkResult AddAll(const std::vector<ItemDefinition>& definitions);

		// Removes a definition, which is how a test empties a provider without
		// rebuilding it. Returns NotFound when the id was not registered.
		Status Remove(ItemId id);

		const ItemDefinition* Find(ItemId id) const override;

		size_t GetCount() const noexcept { return m_definitions.size(); }
		void   Clear() noexcept { m_definitions.clear(); }

	private:
		// Kept sorted by id so Find is a binary search and registration order
		// cannot change what a lookup returns.
		std::vector<ItemDefinition> m_definitions;
	};
}
