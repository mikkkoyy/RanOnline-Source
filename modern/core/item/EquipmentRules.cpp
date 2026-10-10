#include "item/EquipmentRules.h"

namespace Modern::Item
{
	namespace
	{
		using Modern::EquipmentSlot;

		// `SLOT_2_SUIT`, GLItemDef.h:247-284, transcribed switch for switch.
		//
		// The four hand slots all map to SUIT_HANDHELD, which is why a weapon can
		// go in any of them and the type-specific rules below decide which one.
		LegacySuit SuitOfSlotImpl(EquipmentSlot slot) noexcept
		{
			switch (slot)
			{
			case EquipmentSlot::Headgear:    return LegacySuit::Headgear;
			case EquipmentSlot::Upper:       return LegacySuit::Upper;
			case EquipmentSlot::Lower:       return LegacySuit::Lower;
			case EquipmentSlot::Hand:        return LegacySuit::Hand;
			case EquipmentSlot::Foot:        return LegacySuit::Foot;

			case EquipmentSlot::RightHand:
			case EquipmentSlot::RightHandExtreme:
			case EquipmentSlot::LeftHand:
			case EquipmentSlot::LeftHandExtreme:
				return LegacySuit::Handheld;

			case EquipmentSlot::Neck:        return LegacySuit::Neck;
			case EquipmentSlot::Wrist:       return LegacySuit::Wrist;
			case EquipmentSlot::RightFinger:
			case EquipmentSlot::LeftFinger:  return LegacySuit::Finger;
			case EquipmentSlot::Vehicle:     return LegacySuit::Vehicle;

			case EquipmentSlot::Waist:       return LegacySuit::Belt;
			case EquipmentSlot::LeftEarring:     return LegacySuit::Earring;
			case EquipmentSlot::Ornament:    return LegacySuit::Ornament;
			case EquipmentSlot::Face:        return LegacySuit::Face;
			case EquipmentSlot::Misc:        return LegacySuit::Misc;

			case EquipmentSlot::LeftAccessory:
			case EquipmentSlot::RightAccessory:
				return LegacySuit::Accessory;
			}
			return LegacySuit::None;
		}
	}

	const char* ToString(EquipRefusal refusal) noexcept
	{
		switch (refusal)
		{
		case EquipRefusal::None:             return "None";
		case EquipRefusal::NotWearableType:  return "NotWearableType";
		case EquipRefusal::SuitMismatch:     return "SuitMismatch";
		case EquipRefusal::WrongSlotForType: return "WrongSlotForType";
		case EquipRefusal::NeedsOffHand:     return "NeedsOffHand";
		}
		return "Unrecognised";
	}

	LegacySuit SuitOfSlot(EquipmentSlot slot) noexcept
	{
		return SuitOfSlotImpl(slot);
	}

	bool IsWearableType(LegacyItemType type) noexcept
	{
		switch (type)
		{
		case LegacyItemType::Suit:
		case LegacyItemType::Arrow:
		case LegacyItemType::Charm:
		case LegacyItemType::Bullet:
		case LegacyItemType::AntiDisappear:
		case LegacyItemType::Revive:
		case LegacyItemType::Vehicle:
			return true;
		default:
			return false;
		}
	}

	EquipRefusal CheckSlot(const ItemDefinition& definition, EquipmentSlot slot) noexcept
	{
		// Rule 2. Anything outside the wearable set is not equipment, however
		// plausible its stat block looks.
		if (!IsWearableType(definition.itemType))
		{
			return EquipRefusal::NotWearableType;
		}

		// Rule 3.
		if (definition.suit != SuitOfSlot(slot))
		{
			return EquipRefusal::SuitMismatch;
		}

		// Rule 4.
		if (definition.itemType == LegacyItemType::Revive ||
		    definition.itemType == LegacyItemType::AntiDisappear)
		{
			if (slot != EquipmentSlot::Ornament)
			{
				return EquipRefusal::WrongSlotForType;
			}
		}

		if (definition.itemType == LegacyItemType::Vehicle)
		{
			if (slot != EquipmentSlot::Vehicle)
			{
				return EquipRefusal::WrongSlotForType;
			}
		}

		// Rule 5's slot half. The weapon half is `RequiredMainHandAttack` and is
		// the caller's to check, because it needs the other hand's contents.
		if (RequiresOffHand(definition.itemType))
		{
			if (slot != EquipmentSlot::LeftHand &&
			    slot != EquipmentSlot::LeftHandExtreme)
			{
				return EquipRefusal::NeedsOffHand;
			}
		}

		return EquipRefusal::None;
	}

	bool RequiresOffHand(LegacyItemType type) noexcept
	{
		return type == LegacyItemType::Arrow || type == LegacyItemType::Charm ||
		       type == LegacyItemType::Bullet;
	}

	LegacyItemAtt RequiredMainHandAttack(LegacyItemType type) noexcept
	{
		switch (type)
		{
		case LegacyItemType::Arrow:  return LegacyItemAtt::Bow;
		case LegacyItemType::Charm:  return LegacyItemAtt::Spear;
		case LegacyItemType::Bullet: return LegacyItemAtt::Gun;
		default:                     return LegacyItemAtt::Nothing;
		}
	}
} // namespace Modern::Item
