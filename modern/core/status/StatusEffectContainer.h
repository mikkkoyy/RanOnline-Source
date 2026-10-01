#pragma once

// VERTICAL-014: the per-character status slot container.
//
// Legacy stores state blows in a fixed array of EMBLOW_MULTI (4) entries -
// `SSTATEBLOW m_sSTATEBLOWS[EMBLOW_MULTI]` (GLCharClient.h:106) - and writes
// them through `GLChar::STATEBLOW` (GLChar.cpp:6200-6226). This is the modern
// equivalent of that array and nothing more.
//
// The container owns storage and lifetime. It does NOT decide whether a blow
// lands; that is StatusEffectResolver's job, and keeping the two apart is what
// lets the probability rule be tested without a character.

#include "StatusEffectTypes.h"
#include "StatusEffectResolver.h"   // StatusEffectState

#include <array>

namespace Modern::StatusEffect
{
	// The stored form of one slot.
	//
	// `fAGE` in legacy is named "age" but holds the REMAINING lifetime: it is
	// assigned the computed duration at creation (GLChar.cpp:3390) and then
	// decremented every frame (GLCharClient.cpp:3772,
	// `sSTATEBLOW.fAGE -= fElapsedTime`). Naming it `remainingLifetime` here so
	// the direction is not something a reader has to deduce.
	struct StatusSlot
	{
		StatusEffectType type = StatusEffectType::None;
		float remainingLifetime = 0.0f;
		float var1 = 0.0f;
		float var2 = 0.0f;

		constexpr bool Active() const noexcept
		{
			return type != StatusEffectType::None;
		}
	};

	class StatusEffectContainer
	{
	public:
		StatusEffectContainer() = default;

		// Writes a state into the slot its type maps to, exactly as
		// GLChar.cpp:6204-6207 does:
		//
		//   int nIndex = 0;
		//   if ( emBLOW <= EMBLOW_SINGLE ) nIndex = 0;
		//   else                            nIndex = emBLOW - EMBLOW_SINGLE;
		//   m_sSTATEBLOWS[nIndex] = sStateBlow;
		//
		// There is no merge, no refresh rule and no "keep the stronger" rule:
		// a plain assignment. Two consequences that are easy to assume away and
		// are therefore asserted in the tests:
		//
		//   - Re-applying the same state RESETS its duration to the new value.
		//   - Applying Burn over an existing Stun OVERWRITES the stun, because
		//     they share slot 0. Stun is lost early.
		//
		// Returns false only for `None`, which legacy never reaches because the
		// caller guards on it (GLChar.cpp:3364).
		bool Apply(const StatusEffectState& state) noexcept
		{
			const uint8_t slot = SlotFor(state.type);
			if (slot >= kStatusSlotCount)
			{
				return false;
			}

			m_slots[slot].type              = state.type;
			m_slots[slot].remainingLifetime = state.remainingLifetime;
			m_slots[slot].var1              = state.var1;
			m_slots[slot].var2              = state.var2;
			return true;
		}

		// Advances every active slot. Reproduces the expiry test at
		// GLFactEffect.cpp:154-159:
		//
		//   if ( sSTATEBLOW.fAGE <= 0.0f )  sSTATEBLOW.emBLOW = EMBLOW_NONE;
		//
		// combined with the decrement at GLCharClient.cpp:3772. The comparison
		// is `<= 0`, so a slot whose remaining time lands exactly on zero has
		// expired. A negative delta cannot extend a state.
		//
		// Returns how many slots expired on this tick.
		uint32_t Tick(float elapsedSeconds) noexcept
		{
			uint32_t expired = 0;
			for (StatusSlot& slot : m_slots)
			{
				if (!slot.Active())
				{
					continue;
				}
				slot.remainingLifetime -= elapsedSeconds;
				if (slot.remainingLifetime <= 0.0f)
				{
					slot = StatusSlot{};
					++expired;
				}
			}
			return expired;
		}

		// Reproduces GLChar::CURE_STATEBLOW (GLChar.cpp:6228-6243):
		//
		//   for ( int i=0; i<EMBLOW_MULTI; ++i ) {
		//       if ( m_sSTATEBLOWS[i].emBLOW==EMBLOW_NONE ) continue;
		//       if ( STATE_TO_DISORDER(m_sSTATEBLOWS[i].emBLOW) & dwCUREFLAG )
		//           m_sSTATEBLOWS[i].emBLOW = EMBLOW_NONE;
		//   }
		//
		// `cureMask` is a StatusDisorder bitmask; it clears every slot whose
		// disorder intersects it. Returns how many slots were cleared.
		//
		// Note legacy clears only `emBLOW`, leaving fAGE and the variables
		// behind on the dead slot. A cleared slot is therefore not `Active()`
		// and its stale values are unreachable; this container zeroes the whole
		// slot, which is observationally identical through the public surface.
		uint32_t Cure(StatusDisorder cureMask) noexcept
		{
			uint32_t cured = 0;
			const uint32_t mask = static_cast<uint32_t>(cureMask);
			for (StatusSlot& slot : m_slots)
			{
				if (!slot.Active())
				{
					continue;
				}
				if ((static_cast<uint32_t>(DisorderFor(slot.type)) & mask) != 0u)
				{
					slot = StatusSlot{};
					++cured;
				}
			}
			return cured;
		}

		// GLCharClient.h:242 `DISABLEBLOW(i)`, and the full wipe at
		// GLCharacter.cpp:6026 which loops `for (i<EMBLOW_MULTI) DISABLEBLOW(i)`.
		void Clear() noexcept
		{
			for (StatusSlot& slot : m_slots)
			{
				slot = StatusSlot{};
			}
		}

		const StatusSlot* At(uint8_t index) const noexcept
		{
			return index < kStatusSlotCount ? &m_slots[index] : nullptr;
		}

		const StatusSlot* Find(StatusEffectType type) const noexcept
		{
			const uint8_t slot = SlotFor(type);
			if (slot >= kStatusSlotCount || !m_slots[slot].Active())
			{
				return nullptr;
			}
			// Slot 0 is shared, so confirm the stored type really is the one
			// asked for rather than another single-slot state.
			if (m_slots[slot].type != type)
			{
				return nullptr;
			}
			return &m_slots[slot];
		}

		bool Has(StatusEffectType type) const noexcept
		{
			return Find(type) != nullptr;
		}

		uint32_t ActiveCount() const noexcept
		{
			uint32_t count = 0;
			for (const StatusSlot& slot : m_slots)
			{
				if (slot.Active())
				{
					++count;
				}
			}
			return count;
		}

		// The union of the disorders currently held. NOT legacy's
		// `GETHOLDBLOW()`, which is an immunity mask built from EMSPECA_NONBLOW
		// specs (GLogixExPC.cpp:2357) and is unrelated to what is active. This
		// is a query convenience for callers and for tests.
		uint32_t ActiveDisorderMask() const noexcept
		{
			uint32_t mask = 0;
			for (const StatusSlot& slot : m_slots)
			{
				if (slot.Active())
				{
					mask |= static_cast<uint32_t>(DisorderFor(slot.type));
				}
			}
			return mask;
		}

	private:
		std::array<StatusSlot, kStatusSlotCount> m_slots{};
	};
}