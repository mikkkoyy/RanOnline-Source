#pragma once

// VERTICAL-015: the authoritative FACT slot pool.
//
// Reproduces `SSKILLFACT m_sSKILLFACT[SKILLFACT_SIZE]` (14 entries) and the
// three things legacy does with it: choose a slot, store, and expire.
//
// Slot selection is NOT a scan for a free slot and NOT a hash. It is a
// three-rule cascade in `GLChar::SELECT_SKILLSLOT` (GLChar.cpp:6377-6408), and
// each rule has a consequence worth stating plainly:
//
//   1. If this skill is ALREADY present, reuse ITS slot. Re-casting refreshes
//      that record rather than consuming a second slot.
//   2. Otherwise take the first empty slot.
//   3. Otherwise evict the slot with the SMALLEST remaining lifetime.
//
// Rule 3 is the one that is easy to get wrong. With a full pool, RAN does not
// refuse the new buff and does not evict the strongest one - it evicts whatever
// is closest to expiring.

#include "SkillFactTypes.h"

#include <array>

namespace Modern::Skills
{
	class SkillFactContainer
	{
	public:
		SkillFactContainer() = default;

		// GLChar.cpp:6377-6408, verbatim in behaviour.
		//
		//   for (i < SKILLREALFACT_SIZE)
		//       if ( m_sSKILLFACT[i].sNATIVEID == skill_id )  return i;
		//   fAGE = FLT_MAX;
		//   for (i < SKILLREALFACT_SIZE) {
		//       if ( m_sSKILLFACT[i].sNATIVEID == SNATIVEID(false) )  return i;
		//       if ( m_sSKILLFACT[i].fAGE < fAGE ) { fAGE = ...; dwSELECT = i; }
		//   }
		//   return dwSELECT;
		//
		// Note the second loop returns the FIRST empty slot it meets, before
		// considering the eviction candidate - so an empty slot always wins over
		// evicting anything, whatever its remaining time.
		//
		// Legacy returns `UINT_MAX` when the pool size is zero; that cannot
		// happen here because the size is a compile-time 14. The out-of-range
		// case is reported as 14 instead, and `Apply` refuses it, so a caller
		// can never write out of bounds.
		uint8_t SelectSlot(const SkillId& skillId) const noexcept
		{
			for (uint8_t i = 0; i < kSkillFactSlotCount; ++i)
			{
				if (m_slots[i].Occupied() && m_slots[i].skillId == skillId)
				{
					return i;
				}
			}

			float lowest = 0.0f;
			uint8_t candidate = kSkillFactSlotCount;
			bool   haveCandidate = false;

			for (uint8_t i = 0; i < kSkillFactSlotCount; ++i)
			{
				if (!m_slots[i].Occupied())
				{
					return i;
				}
				if (!haveCandidate || m_slots[i].remainingLifetime < lowest)
				{
					lowest = m_slots[i].remainingLifetime;
					candidate = i;
					haveCandidate = true;
				}
			}

			return candidate;
		}

		// Stores a fact in its selected slot. A plain assignment
		// (`m_sSKILLFACT[dwSELECT] = sSKILLEF`, GLChar.cpp:6612), so this
		// refreshes an existing record for the same skill and evicts a weaker
		// one when the pool is full. It does not stack, merge, or compare
		// strengths.
		//
		// Refuses a fact that holds nothing, mirroring the `bHOLD` gate: legacy
		// returns without storing rather than occupying a slot with an inert
		// record.
		bool Apply(const SkillFact& fact) noexcept
		{
			if (!FactHoldsAnything(fact))
			{
				return false;
			}

			const uint8_t slot = SelectSlot(fact.skillId);
			if (slot >= kSkillFactSlotCount)
			{
				return false;
			}

			m_slots[slot] = fact;
			return true;
		}

		// DISABLESKEFF (GLCharClient.h:241): `m_sSKILLFACT[i].sNATIVEID =
		// NATIVEID_NULL()`. Only the skill id is cleared - the specs, impacts
		// and lifetime are left behind on the dead record. That is unobservable
		// through this class because an unoccupied slot is never aggregated, so
		// the whole slot is zeroed here instead.
		bool Remove(const SkillId& skillId) noexcept
		{
			for (uint8_t i = 0; i < kSkillFactSlotCount; ++i)
			{
				if (m_slots[i].Occupied() && m_slots[i].skillId == skillId)
				{
					m_slots[i] = SkillFact{};
					return true;
				}
			}
			return false;
		}

		void Clear() noexcept
		{
			for (SkillFact& slot : m_slots)
			{
				slot = SkillFact{};
			}
		}

		// The lifetime primitive: decrement, then expire at `<= 0`
		// (GLogixExPC.cpp:2292-2295).
		//
		// The game loop normally uses `AdvanceSkillFacts` instead, because
		// legacy decrements and aggregates in ONE pass and an expiring fact
		// still contributes on that pass. This method exists for the lifetime
		// rules in isolation; do not call both in the same tick.
		uint32_t Tick(float elapsedSeconds) noexcept
		{
			uint32_t expired = 0;
			for (SkillFact& slot : m_slots)
			{
				if (!slot.Occupied())
				{
					continue;
				}
				slot.remainingLifetime -= elapsedSeconds;
				if (slot.remainingLifetime <= 0.0f)
				{
					slot = SkillFact{};
					++expired;
				}
			}
			return expired;
		}

		// One slot, for inspection and tests.
		const SkillFact* At(uint8_t index) const noexcept
		{
			return index < kSkillFactSlotCount ? &m_slots[index] : nullptr;
		}

		// Mutable access for the lifetime pass. Legacy decrements `fAGE` in
		// place on the stored record (GLogixExPC.cpp:2292), so the advance loop
		// needs a real reference rather than a copy.
		SkillFact* MutableAt(uint8_t index) noexcept
		{
			return index < kSkillFactSlotCount ? &m_slots[index] : nullptr;
		}

		const SkillFact* Find(const SkillId& skillId) const noexcept
		{
			for (const SkillFact& slot : m_slots)
			{
				if (slot.Occupied() && slot.skillId == skillId)
				{
					return &slot;
				}
			}
			return nullptr;
		}

		bool Has(const SkillId& skillId) const noexcept
		{
			return Find(skillId) != nullptr;
		}

		uint32_t ActiveCount() const noexcept
		{
			uint32_t count = 0;
			for (const SkillFact& slot : m_slots)
			{
				if (slot.Occupied())
				{
					++count;
				}
			}
			return count;
		}

		// True when every slot is taken. The eviction rule only becomes
		// reachable at this point.
		bool Full() const noexcept
		{
			return ActiveCount() == kSkillFactSlotCount;
		}

	private:
		std::array<SkillFact, kSkillFactSlotCount> m_slots{};
	};
}