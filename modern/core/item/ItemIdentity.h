#pragma once

// Item identity: legacy's SNATIVEID pair, mapped without collisions.
//
// ---------------------------------------------------------------------------
// WHY THIS EXISTS
// ---------------------------------------------------------------------------
//
// RAN identifies an item by a PAIR of 16-bit values, `SNATIVEID`
// (legacy/Lib_Client/G-Logic/GLDefine.h):
//
//     struct SNATIVEID { WORD wMainID; WORD wSubID; };
//
// The modern core identifies one by a single `ItemId`, which is a
// `StrongId<ItemIdTag, uint32_t>` (modern/core/types/Ids.h). The pair therefore
// has to be carried in that scalar without losing anything, because an item's
// identity is what every later system - equipment, aggregation, combat stats -
// looks it up by.
//
// ---------------------------------------------------------------------------
// THE MAPPING
// ---------------------------------------------------------------------------
//
//     ItemId = (wMainID << 16) | wSubID
//
// It is exact for every pair the format allows, because both halves are WORD:
// `Unpack(ItemId)` returns the original pair for any of the 2^32 combinations,
// so the encoding is a bijection on `[0, 65535] x [0, 65535]`.
//
// ONE CONSEQUENCE WORTH STATING: `ItemId::MakeInvalid()` is 0xFFFFFFFF, which is
// exactly `(0xFFFF, 0xFFFF)`. A pair of that value therefore cannot be packed
// into a valid id. That is not a defect for the deployed data - the exported
// item table's largest wMainID is 1996 and its largest wSubID is 906 - but it is
// asserted rather than assumed, by
// `ItemIdentity_NoDeployedItemUsesTheInvalidSentinel`.

#include "item/ItemDefinition.h"
#include "types/Ids.h"

#include <cstdint>

namespace Modern
{
	// One half-pair of a legacy `SNATIVEID`, in the legacy field order.
	struct ItemNativeId
	{
		uint16_t mainId = 0;
		uint16_t subId  = 0;

		constexpr bool operator==(const ItemNativeId& other) const noexcept
		{
			return mainId == other.mainId && subId == other.subId;
		}
		constexpr bool operator!=(const ItemNativeId& other) const noexcept
		{
			return !(*this == other);
		}
	};

	// Packs a legacy native id into the modern scalar. Exact for every pair
	// `SNATIVEID` can hold.
	constexpr ItemId PackItemId(uint16_t mainId, uint16_t subId) noexcept
	{
		return ItemId((static_cast<uint32_t>(mainId) << 16) |
		              static_cast<uint32_t>(subId));
	}

	constexpr ItemId PackItemId(const ItemNativeId& nativeId) noexcept
	{
		return PackItemId(nativeId.mainId, nativeId.subId);
	}

	// Recoveres the pair an id was packed from.
	//
	// `MakeInvalid()` has no pair. It is returned as (0,0) rather than as
	// (0xFFFF,0xFFFF) - a caller that cares must check `IsValid()` first, and a
	// caller that does not gets the default rather than a fabricated full-scale
	// native id.
	constexpr ItemNativeId UnpackItemId(ItemId id) noexcept
	{
		if (!id.IsValid())
		{
			return ItemNativeId{};
		}

		const uint32_t raw = id.Get();
		return ItemNativeId{ static_cast<uint16_t>(raw >> 16),
			                 static_cast<uint16_t>(raw & 0xFFFFu) };
	}

	// Whether a pair can be packed into a DISTINCT valid id.
	//
	// False for (0xFFFF, 0xFFFF), whose packed form is the invalid sentinel. A
	// real item table cannot use it, and a caller that is offered one now has a
	// way to say so rather than silently producing an id that fails `IsValid()`.
	constexpr bool CanPackItemId(uint16_t mainId, uint16_t subId) noexcept
	{
		return PackItemId(mainId, subId).IsValid();
	}

} // namespace Modern
