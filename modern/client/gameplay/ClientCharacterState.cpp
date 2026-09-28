// VERTICAL-001: the client's character state. See ClientCharacterState.h.

#include "gameplay/ClientCharacterState.h"

namespace Modern::Client::Gameplay
{
	const Stats::DerivedStats& ClientCharacterState::EmptyDerived() noexcept
	{
		static const Stats::DerivedStats empty;
		return empty;
	}

	const Stats::BaseStats& ClientCharacterState::EmptyStats() noexcept
	{
		static const Stats::BaseStats empty;
		return empty;
	}

	const std::string& ClientCharacterState::EmptyName() noexcept
	{
		static const std::string empty;
		return empty;
	}

	const Vector3& ClientCharacterState::EmptyPosition() noexcept
	{
		static const Vector3 empty;
		return empty;
	}

	Status ClientCharacterState::Apply(const Modern::Gameplay::CharacterSnapshot& snapshot)
	{
		// The client does not trust an incoming snapshot; it re-runs the same
		// validity rules the server used to produce it. A snapshot that could not
		// have come from a server is refused rather than displayed.
		if (!Modern::Gameplay::CharacterSnapshot::IsValid(snapshot))
		{
			return Status(ErrorCode::InvalidArgument);
		}
		m_snapshot    = snapshot;
		m_hasSnapshot = true;
		return Ok();
	}

	void ClientCharacterState::Clear() noexcept
	{
		m_snapshot    = Modern::Gameplay::CharacterSnapshot();
		m_hasSnapshot = false;
	}

	CharacterId ClientCharacterState::GetId() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.id : CharacterId::MakeInvalid();
	}

	const std::string& ClientCharacterState::GetName() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.name : EmptyName();
	}

	CharacterClass ClientCharacterState::GetClass() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.characterClass : CharacterClass::Unset;
	}

	CharacterGender ClientCharacterState::GetGender() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.gender : CharacterGender::Male;
	}

	uint16_t ClientCharacterState::GetLevel() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.level : Stats::kMinLevel;
	}

	int64_t ClientCharacterState::GetExperience() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.experience : 0;
	}

	const Stats::DerivedStats& ClientCharacterState::GetDerivedStats() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.derived : EmptyDerived();
	}

	const Stats::BaseStats& ClientCharacterState::GetAllocatedStats() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.allocatedStats : EmptyStats();
	}

	const Stats::BaseStats& ClientCharacterState::GetTotalStats() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.totalStats : EmptyStats();
	}

	uint32_t ClientCharacterState::GetMaxHp() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.derived.maxHp : 0u;
	}

	uint32_t ClientCharacterState::GetMaxMp() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.derived.maxMp : 0u;
	}

	uint32_t ClientCharacterState::GetMaxSp() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.derived.maxSp : 0u;
	}

	uint32_t ClientCharacterState::GetCurrentHp() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.hp.current : 0u;
	}

	uint32_t ClientCharacterState::GetCurrentMp() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.mp.current : 0u;
	}

	uint32_t ClientCharacterState::GetCurrentSp() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.sp.current : 0u;
	}

	const Vector3& ClientCharacterState::GetPosition() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.position : EmptyPosition();
	}

	float ClientCharacterState::GetHealthFraction() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.GetHealthFraction() : 0.0f;
	}

	float ClientCharacterState::GetManaFraction() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.GetManaFraction() : 0.0f;
	}

	float ClientCharacterState::GetStaminaFraction() const noexcept
	{
		return m_hasSnapshot ? m_snapshot.GetStaminaFraction() : 0.0f;
	}
}
