// VERTICAL-002: the equipment container. See EquipmentState.h.

#include "equipment/EquipmentState.h"

#include <cstddef>

namespace Modern
{
	namespace
	{
		const ItemInstance& EmptyItem() noexcept
		{
			static const ItemInstance empty;
			return empty;
		}
	}

	const char* ToString(EquipmentSlot slot) noexcept
	{
		switch (slot)
		{
			case EquipmentSlot::Headgear:        return "Headgear";
			case EquipmentSlot::Upper:           return "Upper";
			case EquipmentSlot::Lower:           return "Lower";
			case EquipmentSlot::Hand:            return "Hand";
			case EquipmentSlot::Foot:            return "Foot";
			case EquipmentSlot::RightHand:       return "RightHand";
			case EquipmentSlot::LeftHand:        return "LeftHand";
			case EquipmentSlot::Neck:            return "Neck";
			case EquipmentSlot::Wrist:           return "Wrist";
			case EquipmentSlot::RightFinger:     return "RightFinger";
			case EquipmentSlot::LeftFinger:      return "LeftFinger";
			case EquipmentSlot::RightHandExtreme:return "RightHandExtreme";
			case EquipmentSlot::LeftHandExtreme: return "LeftHandExtreme";
			case EquipmentSlot::Vehicle:         return "Vehicle";
			case EquipmentSlot::LeftEarring:     return "LeftEarring";
			case EquipmentSlot::LeftAccessory:   return "LeftAccessory";
			case EquipmentSlot::RightAccessory:  return "RightAccessory";
			case EquipmentSlot::Ornament:        return "Ornament";
			case EquipmentSlot::Waist:           return "Waist";
			case EquipmentSlot::Face:            return "Face";
			case EquipmentSlot::Misc:            return "Misc";
		}
		return "Unknown";
	}

	Status EquipmentState::Equip(EquipmentSlot slot, const ItemInstance& item)
	{
		if (!IsValidSlot(slot) || !item.IsValid())
		{
			return Status(ErrorCode::InvalidArgument);
		}
		m_slots[static_cast<size_t>(slot)] = EquipmentEntry{ item };
		return Ok();
	}

	Status EquipmentState::Unequip(EquipmentSlot slot)
	{
		if (!IsValidSlot(slot))
		{
			return Status(ErrorCode::InvalidArgument);
		}
		m_slots[static_cast<size_t>(slot)] = EquipmentEntry();
		return Ok();
	}

	EquipmentEntry EquipmentState::Get(EquipmentSlot slot) const noexcept
	{
		if (!IsValidSlot(slot))
		{
			return EquipmentEntry();
		}
		return m_slots[static_cast<size_t>(slot)];
	}

	const ItemInstance& EquipmentState::GetEquipped(EquipmentSlot slot) const noexcept
	{
		if (!IsValidSlot(slot))
		{
			return EmptyItem();
		}
		return m_slots[static_cast<size_t>(slot)].item;
	}

	bool EquipmentState::HasEquipped(EquipmentSlot slot) const noexcept
	{
		return Get(slot).HasItem();
	}

	size_t EquipmentState::GetOccupiedCount() const noexcept
	{
		size_t count = 0;
		for (const EquipmentEntry& entry : m_slots)
		{
			if (entry.HasItem())
			{
				++count;
			}
		}
		return count;
	}
}
