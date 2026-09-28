// VERTICAL-001: the server's authoritative character. See ServerCharacter.h.

#include "character/ServerCharacter.h"

#include "stats/StatCalculator.h"

#include <cmath>
#include <utility>

namespace Modern::Server
{
	namespace
	{
		bool IsFinite(const Vector3& value) noexcept
		{
			return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
		}
	}

	Result<ServerCharacter> ServerCharacter::Create(ServerCharacterDefinition definition)
	{
		// The class and gender must name a real row of RAN's class table before
		// anything is computed, because the row is what every coefficient comes
		// from.
		Stats::CharClassIndex classIndex{};
		if (!TryToCharClassIndex(definition.characterClass, definition.gender, classIndex))
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (!definition.id.IsValid() || definition.name.empty() ||
		    definition.name.size() > Character::kNameCapacity)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (!Stats::IsValidLevel(definition.level) || definition.experience < 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (!std::isfinite(definition.confPointRate))
		{
			return Status(ErrorCode::InvalidArgument);
		}

		ServerCharacter character;
		character.m_definition = std::move(definition);
		character.m_classIndex  = classIndex;

		const Status calculated = character.Recalculate();
		if (calculated.IsError())
		{
			return calculated;
		}

		// RAN fills a newly created character to full inside INIT_DATA
		// (GLogixExPC.cpp:1256-1262). A character that has never been restored
		// is at zero, which is also a legitimate state for a loaded one.
		return character;
	}

	Status ServerCharacter::Recalculate()
	{
		// The single call into the stat system. Everything the modern tree knows
		// about RAN's stat arithmetic lives behind this line.
		Stats::StatCalculationInput input;
		input.characterClass  = m_classIndex;
		input.level           = m_definition.level;
		input.classConstants  = m_definition.classConstants;
		input.allocatedStats  = m_definition.allocatedStats;
		input.items           = m_definition.items;
		input.passives        = m_definition.passives;
		input.codex           = m_definition.codex;
		input.confPointRate   = m_definition.confPointRate;

		const Result<Stats::DerivedStats> result = Stats::Calculate(input);
		if (result.IsError())
		{
			return result.GetStatus();
		}

		m_derived = result.GetValue();

		// A recalculation can lower a maximum, which can leave a current value
		// above it. RAN does not guard this either - GLDWDATA::LIMIT only
		// clamps downward on the *max* change, and the current value is reset by
		// whatever caused the change. Clamping here keeps the invariant that a
		// published snapshot is always renderable, and the cost of a stat drop
		// is a bar, not a crash.
		if (m_currentHp > m_derived.maxHp) { m_currentHp = m_derived.maxHp; }
		if (m_currentMp > m_derived.maxMp) { m_currentMp = m_derived.maxMp; }
		if (m_currentSp > m_derived.maxSp) { m_currentSp = m_derived.maxSp; }
		return Ok();
	}

	Status ServerCharacter::SetLevel(uint16_t level)
	{
		if (!Stats::IsValidLevel(level))
		{
			return Status(ErrorCode::InvalidArgument);
		}
		m_definition.level = level;
		return Recalculate();
	}

	Status ServerCharacter::SetExperience(int64_t experience)
	{
		if (experience < 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		m_definition.experience = experience;
		return Ok();
	}

	Status ServerCharacter::SetAllocatedStats(const Stats::BaseStats& stats)
	{
		m_definition.allocatedStats = stats;
		return Recalculate();
	}

	Status ServerCharacter::SetContributions(const Stats::ItemContribution& items,
	                                         const Stats::PassiveContribution& passives,
	                                         const Stats::CodexContribution& codex)
	{
		m_definition.items    = items;
		m_definition.passives = passives;
		m_definition.codex    = codex;
		return Recalculate();
	}

	Status ServerCharacter::SetConfPointRate(float rate)
	{
		if (!std::isfinite(rate))
		{
			return Status(ErrorCode::InvalidArgument);
		}
		m_definition.confPointRate = rate;
		return Recalculate();
	}

	Status ServerCharacter::SetPosition(const Vector3& position)
	{
		if (!IsFinite(position))
		{
			return Status(ErrorCode::InvalidArgument);
		}
		m_position = position;
		return Ok();
	}

	Status ServerCharacter::SetCurrentHp(uint32_t value)
	{
		if (value > m_derived.maxHp)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		m_currentHp = value;
		return Ok();
	}

	Status ServerCharacter::SetCurrentMp(uint32_t value)
	{
		if (value > m_derived.maxMp)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		m_currentMp = value;
		return Ok();
	}

	Status ServerCharacter::SetCurrentSp(uint32_t value)
	{
		if (value > m_derived.maxSp)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		m_currentSp = value;
		return Ok();
	}

	void ServerCharacter::RestoreResources()
	{
		m_currentHp = m_derived.maxHp;
		m_currentMp = m_derived.maxMp;
		m_currentSp = m_derived.maxSp;
	}

	Result<Gameplay::CharacterSnapshot> ServerCharacter::BuildSnapshot() const
	{
		Gameplay::CharacterSnapshot snapshot;
		snapshot.id             = m_definition.id;
		snapshot.name           = m_definition.name;
		snapshot.characterClass = m_definition.characterClass;
		snapshot.gender         = m_definition.gender;
		snapshot.level          = m_definition.level;
		snapshot.experience     = m_definition.experience;
		snapshot.allocatedStats = m_definition.allocatedStats;
		snapshot.totalStats     = m_derived.totalStats;
		snapshot.derived        = m_derived;
		snapshot.hp.current     = m_currentHp;
		snapshot.mp.current     = m_currentMp;
		snapshot.sp.current     = m_currentSp;
		snapshot.position       = m_position;

		return Gameplay::CharacterSnapshot::Create(std::move(snapshot));
	}
}
