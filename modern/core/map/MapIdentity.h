#pragma once

// WORLD-ENTRY-002e: a map's identity, exactly as RAN represents it.
//
// The identity is `SNATIVEID` (legacy/Lib_Engine/G-Logic/GLDefine.h:102-122), a
// union of:
//
//     struct { DWORD dwID; };
//     struct { WORD  wMainID; WORD wSubID; };
//
// so the two components ARE the identity and `dwID` is only their packing. This
// type keeps the two halves and exposes the packing rather than replacing it,
// because the halves are what the map table is written in and what the bounds
// checks are written against.
//
// ---------------------------------------------------------------------------
// WHY THIS IS NOT A STRING
// ---------------------------------------------------------------------------
//
// The tempting design is "the map is called `w_school_01`". That is wrong three
// times over, and 002c measured each way:
//
//   * `mapslist.mst` names a map by `wMainID`/`wSubID`. A `.wld` carries no
//     identity at all - `m_MapID` at offset 132 is 0 in all 87 shipped files -
//     so a name or a filename would have to be invented rather than read.
//   * The `.wld` is SHARED. 16 of the 56 distinct files named by the 99
//     registered maps are named by more than one map id; `suhak.wld` serves three
//     and `w_3school_bilding_inner01.wld` serves eight. A filename is therefore
//     many-to-one with identity and cannot BE the identity.
//   * `GLMapList::FindMapNode` looks up by `sNativeID.dwID` (GLMapList.cpp:246)
//     and `GLAgentServer` indexes `m_pLandMan[wMainID][wSubID]`
//     (GLAgentServer.h:184). The id is the key the game already uses.
//
// ---------------------------------------------------------------------------
// THE VALUE RANGE
// ---------------------------------------------------------------------------
//
// `MAXLANDMID = 512`, `MAXLANDSID = 256` (legacy/Lib_Engine/DxOctree/
// DxLandDef.h:14-15), which is the size of RAN's own `m_pLandMan[512][256]`.
// `SNATIVEID::ID_NULL = 0xFFFF` is the "no map" sentinel.
//
// Measured over the shipped `mapslist.mst`: `wMainID` spans 0..270, and every one
// of the 99 records has `wSubID == 0`. Both halves are carried regardless.

#include <cstdint>
#include <functional>
#include <string>

namespace Modern::Map
{
	struct MapIdentity
	{
		// `SNATIVEID::ID_NULL` (GLDefine.h:106). Both halves 0xFFFF, so a
		// default-constructed identity is the null one - which is what a map id
		// that was never set should mean.
		static constexpr std::uint16_t kNullComponent = 0xFFFF;

		constexpr MapIdentity() noexcept = default;

		constexpr MapIdentity(std::uint16_t mainId, std::uint16_t subId) noexcept
		    : main(mainId), sub(subId)
		{
		}

		static constexpr MapIdentity Null() noexcept
		{
			return MapIdentity(kNullComponent, kNullComponent);
		}

		// The packed form, `SNATIVEID::dwID`. `wMainID` is the low half because it
		// is declared first in the struct the union overlays.
		constexpr std::uint32_t Packed() const noexcept
		{
			return static_cast<std::uint32_t>(main) |
			       (static_cast<std::uint32_t>(sub) << 16);
		}

		static constexpr MapIdentity FromPacked(std::uint32_t packed) noexcept
		{
			return MapIdentity(static_cast<std::uint16_t>(packed & 0xFFFFu),
			                   static_cast<std::uint16_t>((packed >> 16) & 0xFFFFu));
		}

		constexpr bool IsNull() const noexcept
		{
			return main == kNullComponent && sub == kNullComponent;
		}

		// `wMainID < MAXLANDMID || wSubID < MAXLANDSID`, verbatim.
		//
		// The `||` is RAN's, and it is what RAN applies (GLMapList.cpp:165). It
		// reads as a typo for `&&` - with `||` the check only rejects an id whose
		// BOTH halves are out of range, so `main = 600, sub = 0` passes. It is
		// reproduced as found rather than tidied, because the point of this layer
		// is to agree with legacy about which maps exist. `BothComponentsInRange`
		// below is the check RAN's author appears to have meant; it is offered
		// separately so the difference is visible rather than silently resolved.
		constexpr bool InRange() const noexcept { return main < 512 || sub < 256; }
		constexpr bool BothComponentsInRange() const noexcept
		{
			return main < 512 && sub < 256;
		}

		std::uint16_t main = kNullComponent;
		std::uint16_t sub  = kNullComponent;
	};

	inline constexpr bool operator==(MapIdentity lhs, MapIdentity rhs) noexcept
	{
		return lhs.main == rhs.main && lhs.sub == rhs.sub;
	}

	inline constexpr bool operator!=(MapIdentity lhs, MapIdentity rhs) noexcept
	{
		return !(lhs == rhs);
	}

	// Total order on the packed value, which for two's-complement-free unsigned
	// components is (main, then sub). Given only to make an ordered container
	// possible; RAN's own lookup is by hash of `dwID`.
	inline constexpr bool operator<(MapIdentity lhs, MapIdentity rhs) noexcept
	{
		if (lhs.main != rhs.main)
		{
			return lhs.main < rhs.main;
		}
		return lhs.sub < rhs.sub;
	}

	// For `std::unordered_map`. The packed id IS the hash's subject, so two
	// identities that compare equal hash equally - which is the property an
	// unordered container needs and the reason this is not pointer identity.
	struct MapIdentityHash
	{
		std::size_t operator()(MapIdentity id) const noexcept
		{
			return std::hash<std::uint32_t>{}(id.Packed());
		}
	};

	// `"main/sub"`, for logs and test output. Not an identity - it is a
	// rendering of one.
	std::string ToString(MapIdentity id);
}