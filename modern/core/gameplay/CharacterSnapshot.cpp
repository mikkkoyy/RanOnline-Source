// VERTICAL-001: the shared character snapshot. See CharacterSnapshot.h.

#include "gameplay/CharacterSnapshot.h"

#include <algorithm>

namespace Modern::Gameplay
{
	namespace
	{
		float FractionOf(uint32_t current, uint32_t maximum) noexcept
		{
			if (maximum == 0)
			{
				return 0.0f;
			}
			const float fraction = static_cast<float>(current) / static_cast<float>(maximum);
			return std::min(1.0f, std::max(0.0f, fraction));
		}
	}

	// VERTICAL-004: the codex panel's bar.
	//
	// Presentation only, and it derives nothing about the character beyond what
	// the server published. Clamped for the same reason GetHealthFraction is: a
	// count that outran the requirement would otherwise draw a bar past its end,
	// and `IsValid` refuses such a snapshot outright - the clamp is belt and
	// braces for a value that reached here by some other route.
	float CodexEntry::GetProgressFraction() const noexcept
	{
		return FractionOf(doneCount, requiredCount);
	}

	bool CharacterSnapshot::IsValid(const CharacterSnapshot& snapshot) noexcept
	{
		if (!snapshot.id.IsValid())
		{
			return false;
		}
		if (snapshot.name.empty() || snapshot.name.size() > Character::kNameCapacity)
		{
			return false;
		}
		if (snapshot.characterClass == CharacterClass::Unset)
		{
			return false;
		}
		if (!Stats::IsValidLevel(snapshot.level))
		{
			return false;
		}
		if (snapshot.experience < 0)
		{
			return false;
		}
		// A current pool above the published maximum is not a value a client
		// can render and a server should not have sent. It is refused rather
		// than clamped, so a bug in the publisher is visible instead of
		// quietly displayed as a full bar.
		if (snapshot.hp.current > snapshot.derived.maxHp)
		{
			return false;
		}
		if (snapshot.mp.current > snapshot.derived.maxMp)
		{
			return false;
		}
		if (snapshot.sp.current > snapshot.derived.maxSp)
		{
			return false;
		}
		// A published slot must be wearable, and the count must not claim more
		// entries than the array can hold. The list is sparse: items are stored
		// at their slot index (not packed contiguously), so count is the number
		// of occupied slots, not a high-water mark. A slot naming an item that
		// is not there, and a count that disagrees with the array, are both
		// publisher bugs and are refused rather than displayed.
		if (snapshot.equipped.count > kEquipmentSlotCount)
		{
			return false;
		}
		{
			size_t occupied = 0;
			for (size_t i = 0; i < kEquipmentSlotCount; ++i)
			{
				const Gameplay::EquippedItem& item = snapshot.equipped.items[i];
				const bool present = item.definition.IsValid();
				if (present)
				{
					++occupied;
				}
				if (!present && !item.name.empty())
				{
					return false;  // named but absent
				}
			}
if (occupied != snapshot.equipped.count)
			{
				return false;  // published count disagrees with the array
			}
		}

		// Validate skills list: each entry must have a valid id and level > 0.
		for (const auto& skill : snapshot.skills.skills)
		{
			if (!skill.id.IsValid())
			{
				return false;
			}
			if (skill.level == 0 || skill.level > kMaxSkillLevel)
			{
				return false;
			}
		}

		// Validate codex entries. An entry must name a real one, carry a type in
		// the verified enum range, and be named - a bare id with no title cannot
		// be labelled. The counters must agree with each other: a required count
		// outside the definition's own capacity is a publisher bug, and a done
		// count past the required count is the same. A completed entry must have
		// reached its requirement, because the server only completes an entry by
		// satisfying it (CodexState::RegisterItem), and an entry that claims to
		// be finished with work outstanding would pay its reward into `derived`
		// while its panel says it is unfinished.
		CodexId previousCodex = CodexId::MakeInvalid();
		for (const auto& entry : snapshot.codex.entries)
		{
			if (!entry.id.IsValid() || entry.name.empty())
			{
				return false;
			}
			// Qualified: an unqualified IsValid inside this member would resolve to
			// the member itself rather than to the codex type validator.
			if (!Modern::IsValid(entry.type))
			{
				return false;
			}
			if (entry.requiredCount > kCodexMaxRequirements)
			{
				return false;
			}
			if (entry.doneCount > entry.requiredCount)
			{
				return false;
			}
			if (entry.completed && entry.doneCount < entry.requiredCount)
			{
				return false;
			}
			// Sorted and unique, so a panel renders the list in the order the
			// server's ordered maps produced and a duplicate cannot appear twice.
			if (previousCodex.IsValid() && !(previousCodex < entry.id))
			{
				return false;
			}
			previousCodex = entry.id;
		}
		return true;
	}

Result<CharacterSnapshot> CharacterSnapshot::Create(CharacterSnapshot snapshot) noexcept
	{
		if (!IsValid(snapshot))
		{
			return Status(ErrorCode::InvalidArgument);
		}
		return snapshot;
	}

	float CharacterSnapshot::GetHealthFraction() const noexcept
	{
		return FractionOf(hp.current, derived.maxHp);
	}

	float CharacterSnapshot::GetManaFraction() const noexcept
	{
		return FractionOf(mp.current, derived.maxMp);
	}

	float CharacterSnapshot::GetStaminaFraction() const noexcept
	{
		return FractionOf(sp.current, derived.maxSp);
	}
}
