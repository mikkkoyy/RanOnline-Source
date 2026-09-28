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
