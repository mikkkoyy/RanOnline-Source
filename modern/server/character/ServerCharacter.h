#pragma once

// VERTICAL-001: the server's authoritative character.
//
 // This is the only place in the modern tree that decides what a character's
 // derived statistics are. It does so by calling Modern::Stats::Calculate, the
 // CORE-002 implementation, with the contributions a full RAN server would
 // hold. The formulas are not restated here and are not duplicated anywhere
 // else; if a stat changes, it changes in modern/core/stats and both sides of
 // the boundary follow.
 //
 // What the server owns, and why each is here rather than on `Character`:
 //
 //   - the class and gender, because they select the class-table row;
 //   - the level, which is a level term in every derived value;
 //   - the experience, which decides level-ups and nothing else in this slice;
 //   - the allocated stats, which are the player's decisions and the largest
 //     single term in the stat sum;
 //   - the three contribution sets (equipment, passive, codex) as *values*. The
 //     systems that produce them are deferred, so nothing here reads equipment or
 //     skills; a future system supplies the value and the recalculation is
 //     already in place;
 //   - the configuration point rate, which RAN's server sends to the client so
 //     the two recomputations agree (GLCharacterMsg.cpp:796). Here it is simply
 //     an input, and there is only one computation to agree with;
 //   - the current resource pools, clamped to the derived maxima.
 //
 // What it deliberately does not own: a socket, a session, a database handle, a
 // map, or a message. It is a headless value with rules.

#include "character/Character.h"
#include "character/CharacterClassTable.h"
#include "equipment/EquipmentState.h"
#include "equipment/ItemDefinitionProvider.h"
#include "gameplay/CharacterSnapshot.h"
#include "math/Vector3.h"
#include "skills/PassiveContributionAggregator.h"
#include "skills/SkillDefinitionProvider.h"
#include "skills/SkillState.h"
#include "stats/BaseStats.h"
#include "stats/Contributions.h"
#include "stats/DerivedStats.h"
#include "types/Result.h"

#include <cstdint>
#include <string>

namespace Modern::Server
{
// The inputs a server needs to bring a character into the world.
	//
	// The class-table row is passed in rather than looked up: RAN loads it from
	// `default.charclass`, a data file that is not in this repository, and
	// CORE-002's `Calculate` takes it as a value for exactly that reason.
	struct ServerCharacterDefinition
	{
		CharacterId                        id;
		std::string                        name;
		CharacterClass                     characterClass = CharacterClass::Unset;
		CharacterGender                    gender        = CharacterGender::Male;
		uint16_t                           level         = Stats::kMinLevel;
		int64_t                            experience    = 0;
		Stats::BaseStats                   allocatedStats;
		Stats::ClassConstants              classConstants;
		Stats::ItemContribution            items;
		Stats::PassiveContribution         passives;
		Stats::CodexContribution           codex;


		// Resolves the ids worn by the character. Required: equipment cannot be
		// aggregated without it, and an equipment-driven character that silently
		// ignored its items would be worse than one that refused to load.
		const ItemDefinitionProvider*    itemDefinitions = nullptr;

		// Resolves skill definitions for passive contribution aggregation.
		// Required when the character has learned passive skills.
		const SkillDefinitionProvider*   skillDefinitions = nullptr;

		float                              confPointRate = 1.0f;
	};

	// An authoritative character.
	//
	// Every mutator that can change a stat input recalculates before it
	// returns, so a caller can never observe a character whose published
	// snapshot disagrees with its own inputs. There is no "recalculate" step to
	// forget to call.
	class ServerCharacter
	{
	public:
		// Fails with InvalidArgument if the definition names no class-table row,
		// is outside the level or experience range, carries a non-finite
		// coefficient, or produces no valid statistics.
		static Result<ServerCharacter> Create(ServerCharacterDefinition definition);

		CharacterId       GetId() const noexcept { return m_definition.id; }
		const std::string& GetName() const noexcept { return m_definition.name; }
		CharacterClass    GetClass() const noexcept { return m_definition.characterClass; }
		CharacterGender   GetGender() const noexcept { return m_definition.gender; }
		uint16_t          GetLevel() const noexcept { return m_definition.level; }
		int64_t           GetExperience() const noexcept { return m_definition.experience; }

		// These three all change a stat input and so all recalculate.
		Status SetLevel(uint16_t level);
		Status SetExperience(int64_t experience);
		Status SetAllocatedStats(const Stats::BaseStats& stats);
		Status SetContributions(const Stats::ItemContribution& items,
		                        const Stats::PassiveContribution& passives,
		                        const Stats::CodexContribution& codex);
		Status SetConfPointRate(float rate);

		// Position is presentation state the server also holds, as RAN stores a
		// save position alongside the character (SCHARDATA2::m_vSavePos). It is
		// not a stat input, so it does not recalculate.
		const Vector3& GetPosition() const noexcept { return m_position; }
		Status SetPosition(const Vector3& position);

		// The current resource pools. A value above the derived maximum is
		// refused rather than clamped, so an out-of-range request is visible
		// instead of silently becoming full.
		Status SetCurrentHp(uint32_t value);
		Status SetCurrentMp(uint32_t value);
		Status SetCurrentSp(uint32_t value);

		// Fills every resource to its derived maximum. RAN does the same for a
		// newly created character inside INIT_DATA (GLogixExPC.cpp:1256-1262).
		void RestoreResources();

		// The last computed statistics. Never stale: every mutator that can
		// change it recomputed first.
		const Stats::DerivedStats& GetDerivedStats() const noexcept { return m_derived; }

		// The stat input as currently configured, for inspection and tests.
		const Stats::BaseStats& GetAllocatedStats() const noexcept
		{
			return m_definition.allocatedStats;
		}
		const Stats::BaseStats& GetTotalStats() const noexcept { return m_derived.totalStats; }
		const Stats::CharClassIndex GetClassIndex() const noexcept { return m_classIndex; }
		const Stats::ClassConstants& GetClassConstants() const noexcept
		{
			return m_definition.classConstants;
		}
		float GetConfPointRate() const noexcept
		{
			return m_definition.confPointRate;
		}

		// ---- Skills (VERTICAL-003) ----
		//
		// Learn, unlearn, and level up passive skills. Each operation rebuilds
		// the passive contribution and recalculates before it returns, so the
		// published statistics can never lag the learned skill set. RAN does the
		// same through SUM_PASSIVE followed by SUM_ADDITION
		// (GLogixExPC.cpp:863 then :1254).
		//
		// All are transactional. A call that fails leaves the skill set, the
		// contribution and the statistics exactly as they were.
		Status LearnSkill(const SkillId& id);
		Status UnlearnSkill(const SkillId& id);
		Status SetSkillLevel(const SkillId& id, uint8_t level);

		// The learned skill set, for inspection and tests.
		const SkillState& GetSkills() const noexcept { return m_skills; }

		// The aggregated passive contribution currently in force. This is the
		// value `Calculate` was last given, and it is what a skill change
		// rebuilds.
		const Stats::PassiveContribution& GetPassiveContribution() const noexcept
		{
			return m_passives;
		}

		// ---- Equipment ----
		//
		// Equip and unequip are the only operations that change what the character
		// contributes, and each one rebuilds the item contribution and recalculates
		// before it returns, so the published statistics can never lag the worn
		// set. RAN does the same through SUM_ITEM followed by SUM_ADDITION
		// (GLogixExPC.cpp:444 then :1254).
		//
		// Both are transactional. A call that fails leaves the worn set, the
		// contribution and the statistics exactly as they were: RAN's
		// `SLOT_ITEM` and `RELEASE_SLOT_ITEM` mutate first and can leave a
		// half-applied state on a failed path, and a server that publishes a
		// half-applied state is worse than one that refuses.
		//
		// Equipment validation beyond the structural kind is deferred: RAN's
		// `GLITEMLMT` / `EMREQUIRE_*` requirement system is not modelled yet, and
		// these calls do not pretend to enforce it. See the investigation §4.
		Status Equip(EquipmentSlot slot, const ItemInstance& item);

		Status Unequip(EquipmentSlot slot);

		// The worn set, for inspection and tests.
		const EquipmentState& GetEquipment() const noexcept { return m_equipment; }

		// The aggregated item contribution currently in force. This is the
		// value `Calculate` was last given, and it is what an equipment change
		// rebuilds.
		const Stats::ItemContribution& GetItemContribution() const noexcept
		{
			return m_items;
		}

		// What a client is entitled to see. Fails with InvalidState only if the
		// character was never created, which the factory prevents.
		Result<Gameplay::CharacterSnapshot> BuildSnapshot() const;

	private:
		ServerCharacter() = default;

		// The one place derived statistics are produced. Every mutator that
		// changes a stat input funnels through here.
		Status Recalculate();

		ServerCharacterDefinition m_definition;
		Stats::CharClassIndex      m_classIndex = Stats::CharClassIndex::BrawlerMale;
		EquipmentState            m_equipment;
		const ItemDefinitionProvider* m_itemDefinitions = nullptr;
		Stats::ItemContribution    m_items;

		// VERTICAL-003: skill state and passive contribution.
		SkillState                      m_skills;
		const SkillDefinitionProvider*  m_skillDefinitions = nullptr;
		Stats::PassiveContribution      m_passives;

		Stats::DerivedStats        m_derived;
		Vector3                    m_position;
		uint32_t                   m_currentHp = 0;
		uint32_t                   m_currentMp = 0;
		uint32_t                   m_currentSp = 0;
	};
}
