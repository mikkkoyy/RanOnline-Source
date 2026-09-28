#pragma once

// VERTICAL-001: the client's character state.
//
// This holds the last authoritative snapshot and presents it. It does not
// decide anything, and in particular it cannot compute a derived statistic:
// it has no stat input, no class table and no calculator, and the only way to
// change what it holds is to hand it a snapshot the server produced.
//
// That is a deliberate departure from RAN, and it is enforced structurally
// rather than by convention. RAN's client reruns `SUM_ADDITION` on every
// equipment change (GLCharacterMsg.cpp:766, :796) and displays the result, so
// the formula is executed in two places. Here it is executed in one, and this
// type has no path to a second one.
//
// The presentation helpers exist so a future HUD has something honest to bind
// to, and they derive only ratios of published values - they never invent a
// number the server did not send.

#include "character/Character.h"
#include "character/CharacterClassTable.h"
#include "gameplay/CharacterSnapshot.h"
#include "math/Vector3.h"
#include "stats/DerivedStats.h"
#include "types/Result.h"

#include <cstdint>

namespace Modern::Client::Gameplay
{
	// The client's view of one character.
	//
	// Default-constructed it holds nothing, and `HasSnapshot()` is false until
	// a snapshot arrives. Every accessor is only meaningful once it is, and each
	// one says so by returning a defined empty answer rather than a plausible
	// wrong one.
	class ClientCharacterState
	{
	public:
		// Adopts a snapshot. Fails with InvalidArgument if the snapshot is not
		// one this client could have received, which is the boundary check that
		// keeps a malformed or hostile payload from becoming state.
		Status Apply(const Modern::Gameplay::CharacterSnapshot& snapshot);

		// Forgets the character, e.g. on disconnect or character change. The
		// only unconditional transition.
		void Clear() noexcept;

		bool HasSnapshot() const noexcept { return m_hasSnapshot; }

		// Identifies the snapshot, or an invalid id when there is none.
		CharacterId GetId() const noexcept;
		const std::string& GetName() const noexcept;

		CharacterClass GetClass() const noexcept;
		CharacterGender GetGender() const noexcept;
		uint16_t GetLevel() const noexcept;
		int64_t GetExperience() const noexcept;

		// The derived statistics *as received*. There is no recalculation and no
		// setter for a single field: a change comes as a new snapshot.
		const Stats::DerivedStats& GetDerivedStats() const noexcept;
		const Stats::BaseStats& GetAllocatedStats() const noexcept;
		const Stats::BaseStats& GetTotalStats() const noexcept;

		uint32_t GetMaxHp() const noexcept;
		uint32_t GetMaxMp() const noexcept;
		uint32_t GetMaxSp() const noexcept;
		uint32_t GetCurrentHp() const noexcept;
		uint32_t GetCurrentMp() const noexcept;
		uint32_t GetCurrentSp() const noexcept;

		const Vector3& GetPosition() const noexcept;

		// Ratios for a future status window. Zero when no snapshot is held, and
		// clamped to [0, 1] because the server may have published a current
		// value from before a maximum dropped.
		float GetHealthFraction() const noexcept;
		float GetManaFraction() const noexcept;
		float GetStaminaFraction() const noexcept;

	private:
		static const Stats::DerivedStats& EmptyDerived() noexcept;
		static const Stats::BaseStats& EmptyStats() noexcept;
		static const std::string& EmptyName() noexcept;
		static const Vector3& EmptyPosition() noexcept;

		bool                            m_hasSnapshot = false;
		Modern::Gameplay::CharacterSnapshot m_snapshot;
	};
}
