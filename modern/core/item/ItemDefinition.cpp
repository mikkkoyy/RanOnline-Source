#include "ItemDefinition.h"

namespace Modern
{
	const char* ToString(ItemKind kind) noexcept
	{
		switch (kind)
		{
			case ItemKind::None:       return "None";
			case ItemKind::Weapon:     return "Weapon";
			case ItemKind::Armor:      return "Armor";
			case ItemKind::Accessory:  return "Accessory";
			case ItemKind::Consumable: return "Consumable";
			case ItemKind::Material:   return "Material";
			case ItemKind::Quest:      return "Quest";
			case ItemKind::Misc:       return "Misc";
		}

		return "Unknown";
	}
}
