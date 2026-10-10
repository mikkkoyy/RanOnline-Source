#pragma once

// Which item may go in which slot: the legacy `CHECKSLOT_ITEM` rules.
//
// `GLCHARLOGIC::CHECKSLOT_ITEM` (legacy GLogixExPC.cpp:3071-3162) is the only
// place legacy decides whether an item can be worn, and it is a sequence of
// plain predicates over the item definition and the slot. This header carries
// the part of it that the modern core can evaluate - everything except the
// two-hand cross-checks that need to know what is already in the OTHER hand.
//
// THE RULES, IN LEGACY'S ORDER
//
//   1. The item must exist (the caller resolves it; a missing definition is
//      `MissingDefinition`, not `Incompatible`).
//   2. `emItemType` must be one of ITEM_SUIT, ITEM_ARROW, ITEM_CHARM,
//      ITEM_BULLET, ITEM_ANTI_DISAPPEAR, ITEM_REVIVE, ITEM_VEHICLE. Anything
//      else is not wearable. (:3080-3087)
//   3. `emSuit == SLOT_2_SUIT(slot)`. (:3090-3091)
//   4. ITEM_REVIVE and ITEM_ANTI_DISAPPEAR go only in SLOT_ORNAMENT;
//      ITEM_VEHICLE only in SLOT_VEHICLE. (:3093-3103)
//   5. ITEM_ARROW, ITEM_CHARM and ITEM_BULLET go only in the off hand - and
//      only if the MAIN hand already holds the matching weapon type. The
//      weapon half needs state this rule does not have, so it is reported as a
//      separate predicate. (:3108-3157)
//
// `SLOT_2_SUIT` (GLItemDef.h:247-284) is transcribed below as
// `SuitOfSlot`, and it is what makes rule 3 a table lookup rather than a guess.

#include "equipment/EquipmentState.h"
#include "item/ItemDefinition.h"

#include <cstdint>

namespace Modern::Item
{
	// Why an item could not be equipped.
	enum class EquipRefusal : std::uint8_t
	{
		None = 0,
		NotWearableType,   // emItemType is outside the wearable set
		SuitMismatch,      // emSuit is not the suit the slot accepts
		WrongSlotForType,  // the type has a dedicated slot and this is not it
		NeedsOffHand,      // an arrow/charm/bullet requires the off hand
	};

	const char* ToString(EquipRefusal refusal) noexcept;

	// `SLOT_2_SUIT`, GLItemDef.h:247-284.
	//
	// Returns `LegacySuit::None` for a slot that has no suit, which is what the
	// legacy `default:` branch returns as `SUIT_NSIZE`.
	LegacySuit SuitOfSlot(Modern::EquipmentSlot slot) noexcept;

	// Whether a DEFINITION may occupy a slot, ignoring what is in the other
	// hand. Covers rules 1-4; rule 1 is the caller's resolution.
	bool IsWearableType(LegacyItemType type) noexcept;

	// The full rule-2-4 check. `None` means "yes, subject to rule 5".
	EquipRefusal CheckSlot(const ItemDefinition& definition,
	                       Modern::EquipmentSlot slot) noexcept;

	// Rule 5's arrow/charm/bullet predicate: whether this type must go in the
	// off hand rather than the main hand.
	//
	// Legacy tests `emSlot != emLHand`, where `emLHand` is
	// `GLCHARLOGIC::GetCurLHand()` - SLOT_LHAND or SLOT_LHAND_S depending on
	// whether the character uses the arm-substitute pair. The caller therefore
	// states which slot is the off hand for THIS character, because the rule
	// itself cannot know.
	bool RequiresOffHand(LegacyItemType type) noexcept;

	// The weapon an arrow/charm/bullet needs in the main hand, or `Nothing`.
	//
	//     ITEM_ARROW  requires ITEMATT_BOW     (GLogixExPC.cpp:3141)
	//     ITEM_CHARM  requires ITEMATT_SPEAR   (:3147)
	//     ITEM_BULLET requires GUN/RAILGUN/PORTALGUN (:3153-3155)
	LegacyItemAtt RequiredMainHandAttack(LegacyItemType type) noexcept;

} // namespace Modern::Item
