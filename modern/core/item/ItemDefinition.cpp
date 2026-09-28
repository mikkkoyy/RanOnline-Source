#include "ItemDefinition.h"

#include <cmath>

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

	bool ItemStatBlock::IsZero() const noexcept
	{
		return (pow + str + spi + dex + intel + sta) == 0 && hp == 0 && mp == 0 && sp == 0 &&
		       hpRecoveryRate == 0.0f && mpRecoveryRate == 0.0f && spRecoveryRate == 0.0f &&
		       meleePower == 0 && shootPower == 0 && magicAttack == 0 && hit == 0 &&
		       avoid == 0 && hitPercent == 0.0f && avoidPercent == 0.0f && defense == 0 &&
		       damageLow == 0 && damageHigh == 0 && resistFire == 0 && resistIce == 0 &&
		       resistElectric == 0 && resistPoison == 0 && resistSpirit == 0;
	}

	bool ItemStatBlock::IsFinite() const noexcept
	{
		// The stat block carries three floats, and CORE-002's `Calculate`
		// refuses a non-finite contribution outright. Checking here means the
		// aggregator can reject a bad definition at the point it is read rather
		// than surfacing later as a refused stat calculation.
		return std::isfinite(hpRecoveryRate) && std::isfinite(mpRecoveryRate) &&
		       std::isfinite(spRecoveryRate) && std::isfinite(hitPercent) &&
		       std::isfinite(avoidPercent);
	}
}
