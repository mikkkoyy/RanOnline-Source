#pragma once

#include "ItemDefinition.h"

#include <cstdint>

namespace Modern
{
	// One concrete copy of an item, owned by somebody.
	//
	// An instance is a reference to a definition plus the state that makes this
	// copy different from every other copy: how many, which serial, and how it
	// is currently bound to something. RAN's SITEM flattens all of this into
	// one 700-byte struct, which is why instance identity and item type had to
	// be kept apart here.
	//
	// Notably absent, and left to future systems: random options, upgrades,
	// enchantment state, and custom bonuses. A previous iteration of this core
	// modelled those as an InstanceCustomContribution aggregated over equipped
	// items; that aggregation belonged to the stat system, not to the instance.
	// See reference/legacy-calculation-port/README.md.
	struct ItemInstance
	{
		// The definition this instance is a copy of.
		ItemId definition = ItemId::MakeInvalid();

		// Distinguishes this copy from other copies of the same definition.
		// Unique per owner; the global uniqueness policy belongs to a future
		// database layer.
		uint64_t serial = 0;

		// How many of this item this instance represents. Never zero.
		uint32_t count = 1;

		// Nothing this instance is attached to. Owned by an inventory, a
		// character slot or a world object; which one is a future system's
		// decision, so the instance does not record it.
		ItemId boundTo = ItemId::MakeInvalid();

		bool IsValid() const
		{
			return definition.IsValid() && count > 0;
		}

		// Whether this instance is a single copy or a stack.
		bool IsStacked() const { return count > 1; }

		// Whether this instance is unattached and therefore bindable.
		bool IsFree() const { return !boundTo.IsValid(); }

		friend bool operator==(const ItemInstance& lhs, const ItemInstance& rhs)
		{
			return lhs.definition == rhs.definition && lhs.serial == rhs.serial;
		}

		friend bool operator!=(const ItemInstance& lhs, const ItemInstance& rhs)
		{
			return !(lhs == rhs);
		}
	};
}
