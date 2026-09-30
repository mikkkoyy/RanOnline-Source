#pragma once

#include <cstdint>

namespace Modern
{
	namespace detail
	{
		// Shared implementation for the strongly typed identifiers below.
		//
		// Tag is a unique incomplete type per identifier, which is what makes
		// an ItemId unassignable to a CharacterId: the two instantiations are
		// distinct types even though their underlying types are identical.
		template <typename Tag, typename Underlying>
		struct StrongId
		{
			using UnderlyingType = Underlying;

			// The invalid sentinel is the all-ones pattern, matching the
			// INVALID_* constants the legacy client uses, so a default
			// constructed id compares equal to an explicitly invalidated one.
			static constexpr Underlying InvalidValue = static_cast<Underlying>(-1);

			constexpr StrongId() noexcept : m_value(InvalidValue) {}
			constexpr explicit StrongId(Underlying value) noexcept : m_value(value) {}

			constexpr Underlying Get() const noexcept { return m_value; }
			constexpr bool IsValid() const noexcept { return m_value != InvalidValue; }

			constexpr explicit operator bool() const noexcept { return IsValid(); }

			friend constexpr bool operator==(StrongId lhs, StrongId rhs) noexcept
			{
				return lhs.m_value == rhs.m_value;
			}

			friend constexpr bool operator!=(StrongId lhs, StrongId rhs) noexcept
			{
				return lhs.m_value != rhs.m_value;
			}

			// Ordering is defined on the underlying value so identifiers can be
			// used directly as ordered container keys.
			friend constexpr bool operator<(StrongId lhs, StrongId rhs) noexcept
			{
				return lhs.m_value < rhs.m_value;
			}

			friend constexpr bool operator>(StrongId lhs, StrongId rhs) noexcept
			{
				return rhs < lhs;
			}

			friend constexpr bool operator<=(StrongId lhs, StrongId rhs) noexcept
			{
				return !(rhs < lhs);
			}

			friend constexpr bool operator>=(StrongId lhs, StrongId rhs) noexcept
			{
				return !(lhs < rhs);
			}

			static constexpr StrongId MakeInvalid() noexcept
			{
				return StrongId(InvalidValue);
			}

		private:
			Underlying m_value;
		};
	}

	// Runtime identity of anything that can be placed in a world.
	using EntityId = detail::StrongId<struct EntityIdTag, uint32_t>;

	// Identity of a player-controlled character.
	using CharacterId = detail::StrongId<struct CharacterIdTag, uint32_t>;

	// Identity of an item type, i.e. the definition rather than a copy of it.
	using ItemId = detail::StrongId<struct ItemIdTag, uint32_t>;

	// Identity of an account that owns characters.
	using AccountId = detail::StrongId<struct AccountIdTag, uint32_t>;

	// Identity of a world/shard instance.
	using WorldId = detail::StrongId<struct WorldIdTag, uint32_t>;

	// Identity of a codex entry.
	//
	// VERTICAL-004. RAN keys a codex by `dwCodexID` and uses `UINT_MAX` as the
	// invalid value (GLCodexData.h:74, and `GLCodex::GetCodex` refusing
	// `dwID >= UINT_MAX` at GLCodex.cpp:176), which is exactly the sentinel
	// StrongId already picks, so no separate convention is needed.
	using CodexId = detail::StrongId<struct CodexIdTag, uint32_t>;
}
