#pragma once

#include "../entity/Entity.h"
#include "../math/Vector3.h"
#include "../types/Ids.h"
#include "../types/Result.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace Modern
{
	// The set of character classes.
	//
	// RAN encodes class and gender together in EMCHARINDEX, giving sixteen
	// distinct values for eight classes. Gender carries no meaning in the core
	// — it only existed to index the legacy class tables — so the class is
	// modelled as the eight types it actually is.
	enum class CharacterClass : uint8_t
	{
		Unset     = 0,
		Brawler   = 1,
		Swordsman = 2,
		Archer    = 3,
		Shaman    = 4,
		Gunner    = 5,
		Assassin  = 6,
		Tricker   = 7,
		Extreme   = 8,
	};

	const char* ToString(CharacterClass value) noexcept;

	// A player character.
	//
	// What a character owns: its identity, its class, its level and
	// experience, where it is, and whether it is in the world. That is the
	// whole of it in CORE-001.
	//
	// What it deliberately does not own, and why:
	//
	//   - HP / MP / SP and recovery rates. These are derived values in RAN,
	//     recomputed from base stats, equipment, passive skills and codex
	//     effects. Owning them on the character is what forced the core to
	//     know about all five. They belong to a future StatsSystem.
	//   - Attack, defence, hit, avoid and resistances. Calculated combat
	//     state, for a future CombatSystem.
	//   - Equipment and contribution aggregates. They are per-stat-system
	//     inputs, not character state, and belong to a future
	//     EquipmentSystem.
	//   - Movement targets, speeds and action/animation state. Belong to a
	//     future MovementSystem.
	//   - The experience curve. Level and experience are facts about the
	//     character; how much experience a level costs is data, and belongs to
	//     a future ProgressionSystem.
	//
	// The lifecycle is explicit and deterministic — Create, Spawn, Despawn,
	// Destroy, Reset — with no global state, no clock and no I/O, so the same
	// call sequence always produces the same state.
	class Character : public Entity<CharacterId>
	{
	public:
		// Matches the legacy m_szName buffer, which is 32 characters plus a
		// terminator.
		static constexpr size_t   kNameCapacity = 32;
		static constexpr uint16_t kMinLevel     = 1;
		static constexpr uint16_t kMaxLevel     = 255;

		Character() = default;

		// Assigns identity and moves to EntityState::Created.
		//
		// Fails with InvalidArgument if the id is invalid, the name is empty,
		// or the name exceeds kNameCapacity.
		static Result<Character> Create(const CharacterId& id, const std::string& name);

		// Places the character in the world at a position, facing a direction.
		// The direction is stored normalised.
		//
		// Fails with InvalidArgument if the position is not finite or the
		// direction has no length, InvalidState if the character is already
		// spawned or was never created, and NotAllowed once destroyed.
		Status Spawn(const Vector3& position, const Vector3& direction);

		// Removes the character from the world, keeping its identity.
		//
		// Fails with InvalidState unless the character is currently spawned.
		Status Despawn();

		// Terminal transition. Releases identity, so the object is safe to
		// hand back to a pool.
		//
		// Fails with InvalidState if the character was never created, and
		// NotAllowed if it is already destroyed.
		Status Destroy();

		// Returns the character to its default-constructed condition from any
		// state, including Destroyed. The only unconditional transition.
		void Reset();

		const std::string& GetName() const { return m_name; }
		Status SetName(const std::string& name);

		CharacterClass GetClass() const { return m_class; }
		bool HasClass() const { return m_class != CharacterClass::Unset; }
		Status SetClass(CharacterClass value);

		uint16_t GetLevel() const { return m_level; }
		Status SetLevel(uint16_t level);

		int64_t GetExperience() const { return m_experience; }
		Status SetExperience(int64_t experience);
		Status AddExperience(int64_t amount);

	private:
		Character(CharacterId id, std::string name);

		std::string      m_name;
		CharacterClass   m_class     = CharacterClass::Unset;
		uint16_t         m_level     = kMinLevel;
		int64_t          m_experience = 0;
	};
}
