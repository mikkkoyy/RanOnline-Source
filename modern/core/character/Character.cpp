#include "Character.h"

namespace Modern
{
	const char* ToString(CharacterClass value) noexcept
	{
		switch (value)
		{
			case CharacterClass::Unset:     return "Unset";
			case CharacterClass::Brawler:   return "Brawler";
			case CharacterClass::Swordsman: return "Swordsman";
			case CharacterClass::Archer:    return "Archer";
			case CharacterClass::Shaman:    return "Shaman";
			case CharacterClass::Gunner:    return "Gunner";
			case CharacterClass::Assassin:  return "Assassin";
			case CharacterClass::Tricker:   return "Tricker";
			case CharacterClass::Extreme:   return "Extreme";
		}

		return "Unknown";
	}

	namespace
	{
		bool IsNameAcceptable(const std::string& name)
		{
			return !name.empty() && name.size() <= Character::kNameCapacity;
		}

		// A name at capacity is rejected rather than truncated: silently
		// shortening an identity is the kind of lossy behaviour that only
		// shows up much later as two characters sharing a name.
		Status ValidateName(const std::string& name)
		{
			return IsNameAcceptable(name) ? Ok() : Status(ErrorCode::InvalidArgument);
		}

		// Modifiers other than the lifecycle transitions are only meaningful
		// for a character that exists, so a default-constructed or destroyed
		// character reports NotAllowed instead of accepting a write.
		Status RequireExisting(EntityState state)
		{
			if (state == EntityState::Destroyed)
			{
				return Status(ErrorCode::NotAllowed);
			}

			if (state == EntityState::Uninitialized)
			{
				return Status(ErrorCode::InvalidState);
			}

			return Ok();
		}
	}

	Character::Character(CharacterId id, std::string name)
		: m_name(std::move(name))
	{
		SetId(id);
		SetState(EntityState::Created);
	}

	Result<Character> Character::Create(const CharacterId& id, const std::string& name)
	{
		if (!id.IsValid())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const Status nameStatus = ValidateName(name);
		if (nameStatus.IsError())
		{
			return nameStatus;
		}

		return Character(id, name);
	}

	Status Character::Spawn(const Vector3& position, const Vector3& direction)
	{
		if (GetState() == EntityState::Destroyed)
		{
			return Status(ErrorCode::NotAllowed);
		}

		// Spawning is only possible from the states that are not yet in the
		// world: freshly created, or despawned and re-entering.
		if (GetState() != EntityState::Created && GetState() != EntityState::Despawned)
		{
			return Status(ErrorCode::InvalidState);
		}

		if (!position.IsFinite() || !direction.IsFinite())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		if (direction.IsZero())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		PlaceAt(position, Normalize(direction));
		SetState(EntityState::Spawned);
		return Ok();
	}

	Status Character::Despawn()
	{
		if (GetState() == EntityState::Destroyed)
		{
			return Status(ErrorCode::NotAllowed);
		}

		if (GetState() != EntityState::Spawned)
		{
			return Status(ErrorCode::InvalidState);
		}

		SetState(EntityState::Despawned);
		return Ok();
	}

	Status Character::Destroy()
	{
		if (GetState() == EntityState::Destroyed)
		{
			return Status(ErrorCode::NotAllowed);
		}

		if (GetState() == EntityState::Uninitialized)
		{
			return Status(ErrorCode::InvalidState);
		}

		ClearCore();
		m_name.clear();
		m_class      = CharacterClass::Unset;
		m_level      = kMinLevel;
		m_experience = 0;

		// ClearCore() returns the entity to Uninitialized; Destroyed is
		// terminal and must survive until an explicit Reset().
		SetState(EntityState::Destroyed);
		return Ok();
	}

	void Character::Reset()
	{
		ClearCore();
		m_name.clear();
		m_class      = CharacterClass::Unset;
		m_level      = kMinLevel;
		m_experience = 0;
	}

	Status Character::SetName(const std::string& name)
	{
		const Status existing = RequireExisting(GetState());
		if (existing.IsError())
		{
			return existing;
		}

		const Status nameStatus = ValidateName(name);
		if (nameStatus.IsError())
		{
			return nameStatus;
		}

		m_name = name;
		return Ok();
	}

	Status Character::SetClass(CharacterClass value)
	{
		const Status existing = RequireExisting(GetState());
		if (existing.IsError())
		{
			return existing;
		}

		// A character must be given a real class. Accepting Unset here would
		// just move the "no class" failure downstream, to whichever system
		// happened to read it first.
		if (value == CharacterClass::Unset)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		m_class = value;
		return Ok();
	}

	Status Character::SetLevel(uint16_t level)
	{
		const Status existing = RequireExisting(GetState());
		if (existing.IsError())
		{
			return existing;
		}

		if (level < kMinLevel || level > kMaxLevel)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		m_level = level;
		return Ok();
	}

	Status Character::SetExperience(int64_t experience)
	{
		const Status existing = RequireExisting(GetState());
		if (existing.IsError())
		{
			return existing;
		}

		if (experience < 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		m_experience = experience;
		return Ok();
	}

	Status Character::AddExperience(int64_t amount)
	{
		const Status existing = RequireExisting(GetState());
		if (existing.IsError())
		{
			return existing;
		}

		// Negative amounts are rejected rather than subtracted, so callers
		// cannot drain experience by accident. Penalising a character is a
		// rule that belongs to the progression system, not to the character.
		if (amount < 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// Saturate instead of wrapping: experience is a long counter that runs
		// for the life of an account, and a wrapped value would look like a
		// plausible small number rather than an obviously broken one.
		if (amount > INT64_MAX - m_experience)
		{
			m_experience = INT64_MAX;
			return Ok();
		}

		m_experience += amount;
		return Ok();
	}
}
