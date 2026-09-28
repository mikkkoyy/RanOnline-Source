#pragma once

// VERTICAL-002: what a character is wearing.
//
// `enum EMSLOT` in legacy/Lib_Client/G-Logic/GLItemDef.h:207-245 is the
// authoritative list, and it is not a generic MMO slot list: RAN has two
// hand-tool slots per side, an "extreme" variant of each, a vehicle, and four
// accessory slots beyond neck/wrist/rings. The values below are the legacy
// values, because they index a character's saved record.
//
// `GLCHARLOGIC::SUM_ITEM` iterates `i in [0, SLOT_NSIZE_S_2)` with
// `SLOT_NSIZE_S_2 = 21` (GLogixExPC.cpp:446), so exactly these twenty-one slots
// are wearable. `SLOT_HOLD` shares the value 21 and is the non-wearable hold
// slot; `SLOT_TSIZE = 22` counts it. It is therefore *not* a slot here, and the
// enum stops at Misc.

#include "item/ItemInstance.h"
#include "types/Result.h"

#include <array>
#include <cstdint>

namespace Modern
{
	enum class EquipmentSlot : uint8_t
	{
		Headgear    = 0,
		Upper       = 1,
		Lower       = 2,
		Hand        = 3,
		Foot        = 4,
		RightHand   = 5,
		LeftHand    = 6,
		Neck        = 7,
		Wrist       = 8,
		RightFinger = 9,
		LeftFinger  = 10,
		// The "extreme" class hand tools, the RAN equivalent of a second
		// weapon slot for that class.
		RightHandExtreme = 11,
		LeftHandExtreme  = 12,
		Vehicle          = 13,
		LeftEarring      = 14,
		LeftAccessory    = 15,
		RightAccessory   = 16,
		Ornament         = 17,
		Waist            = 18,
		Face             = 19,
		Misc             = 20,
	};

	// SLOT_NSIZE_S_2, the wearable range SUM_ITEM walks.
	constexpr size_t kEquipmentSlotCount = 21;
	constexpr uint8_t kFirstEquipmentSlot = 0;
	constexpr uint8_t kLastEquipmentSlot  = 20;

	const char* ToString(EquipmentSlot slot) noexcept;

	constexpr bool IsValidSlot(EquipmentSlot slot) noexcept
	{
		const uint8_t value = static_cast<uint8_t>(slot);
		return value <= kLastEquipmentSlot;
	}

	// The slots a weapon may occupy, and those an armour piece may. RAN decides
	// this from the item's attack type and suit, not from the slot; the modern
	// split is by `ItemKind`, which is the honest part of it. Kept as a
	// predicate so a future system can refine it with the real data.
	constexpr bool IsWeaponSlot(EquipmentSlot slot) noexcept
	{
		const uint8_t value = static_cast<uint8_t>(slot);
		return value == 5 || value == 6 || value == 11 || value == 12;
	}

	// What one slot holds.
	//
	// An empty slot is a default-constructed `ItemInstance`, whose `definition`
	// is invalid. That is the explicit empty state: there is no separate flag,
	// and no null pointer, so a forgotten check fails on a validity test rather
	// than dereferencing.
	struct EquipmentEntry
	{
		ItemInstance item;

		bool IsEmpty() const noexcept { return !item.definition.IsValid(); }
		bool HasItem() const noexcept { return !IsEmpty(); }
	};

	// A character's worn items: slot to instance.
	//
	// A container and nothing else. It performs no character validation and
	// holds no requirement rules: RAN's `GLITEMLMT` / `EMREQUIRE_*` system
	// (class, gender, school, level, weapon type, wear position) is deferred, and
	// inventing a partial version of it here would let an unimplemented rule
	// look satisfied. See the investigation report §4.
	//
	// It is a value: copying it copies the worn set, and it owns nothing. The
	// instances it holds are shared with whatever inventory supplied them, which
	// is why an instance is identified by definition and serial rather than
	// stored by pointer.
	class EquipmentState
	{
	public:
		// Puts an instance in a slot, replacing whatever was there. Replacing
		// is not a special case: a slot holds at most one instance, so equip and
		// replace are the same operation.
		//
		// Refuses an out-of-range slot or an invalid instance, leaving the
		// state untouched, so a rejected call never leaves a slot half-written.
		Status Equip(EquipmentSlot slot, const ItemInstance& item);

		// Empties a slot. Refuses an out-of-range slot; removing from an
		// already-empty slot succeeds and changes nothing, because a repeated
		// unequip is not an error condition.
		Status Unequip(EquipmentSlot slot);

		// Empties every slot.
		void Clear() noexcept { m_slots.fill(EquipmentEntry()); }

		EquipmentEntry Get(EquipmentSlot slot) const noexcept;

		const ItemInstance& GetEquipped(EquipmentSlot slot) const noexcept;

		bool HasEquipped(EquipmentSlot slot) const noexcept;

		// How many slots hold something. `GetOccupiedCount()` is the aggregate
		// an aggregator and a HUD both want, and it is computed once here
		// rather than by each caller walking the array.
		size_t GetOccupiedCount() const noexcept;

		// The slots, in slot order, for a caller that must iterate
		// deterministically. Returning the whole array keeps the iteration
		// order a property of the container rather than of each caller.
		const std::array<EquipmentEntry, kEquipmentSlotCount>& GetSlots() const noexcept
		{
			return m_slots;
		}

	private:
		std::array<EquipmentEntry, kEquipmentSlotCount> m_slots{};
	};
}
