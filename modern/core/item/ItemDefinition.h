#pragma once

#include "../types/Ids.h"

#include <cstdint>
#include <string>

namespace Modern
{
	// What an item is for.
	//
	// RAN's SITEM splits item behaviour across a dozen kind flags with several
	// independent switch statements over the same value. A single tag keeps the
	// core honest; the fine-grained capability flags come back when a system
	// actually needs to query them.
	enum class ItemKind : uint8_t
	{
		None      = 0,
		Weapon    = 1,
		Armor     = 2,
		Accessory = 3,
		Consumable = 4,
		Material  = 5,
		Quest     = 6,
		Misc      = 7,
	};

	const char* ToString(ItemKind kind) noexcept;

	// The definition of an item type: the shared, immutable description that
	// many instances refer to.
	//
	// CORE-001 keeps this to identity. RAN's definition also carries ~120
	// derived-stat fields (GETADDPA, GETADDSA, GETDAMAGE, resistance arrays,
	// price, durability, upgrade paths). Those are inputs to a future
	// StatsSystem, and a definition is where they will be reintroduced — not
	// in an item instance, and not in a character.
	struct ItemDefinition
	{
		ItemId      id = ItemId::MakeInvalid();
		ItemKind    kind = ItemKind::None;
		std::string name;
		uint32_t    maxStack = 1;

		bool IsValid() const
		{
			return id.IsValid() && kind != ItemKind::None && !name.empty() && maxStack > 0;
		}

		bool CanStack() const { return maxStack > 1; }

		friend bool operator==(const ItemDefinition& lhs, const ItemDefinition& rhs)
		{
			return lhs.id == rhs.id;
		}

		friend bool operator!=(const ItemDefinition& lhs, const ItemDefinition& rhs)
		{
			return !(lhs == rhs);
		}
	};
}
