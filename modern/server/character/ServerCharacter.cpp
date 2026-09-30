// VERTICAL-001: the server's authoritative character. See ServerCharacter.h.

#include "character/ServerCharacter.h"

#include "equipment/ItemContributionAggregator.h"
#include "skills/PassiveContributionAggregator.h"
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
	character.m_definition       = std::move(definition);
	character.m_classIndex        = classIndex;
	character.m_itemDefinitions  = character.m_definition.itemDefinitions;
	character.m_skillDefinitions = character.m_definition.skillDefinitions;
	// The caller no longer supplies an item contribution: it is aggregated
	// from the worn set, which starts empty. A contribution passed here
	// alongside equipment would be a second, competing source.
	character.m_definition.items = Stats::ItemContribution();
	// The caller no longer supplies a passive contribution: it is aggregated
	// from the learned skill set, which starts empty.
	character.m_definition.passives = Stats::PassiveContribution();

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
	// The worn set is the one input the server owns rather than being handed,
	// so it is aggregated here and never passed in. Rebuilding it on every
	// recalculation is what makes a stat input impossible to forget.
	if (m_itemDefinitions != nullptr)
	{
		const Result<ItemContributionResult> aggregated =
			ItemContributionAggregator::Aggregate(m_equipment, *m_itemDefinitions);
		if (aggregated.IsError())
		{
			return aggregated.GetStatus();
		}
		if (!aggregated.GetValue().IsOk())
		{
			// A worn item nothing defines is a data fault, not a bad argument,
			// and it is surfaced as a refusal rather than as a silent zero.
			return Status(ErrorCode::InvalidArgument);
		}
		m_items = aggregated.GetValue().contribution;
	}

	// VERTICAL-003: aggregate passive skill contributions.
	// The learned skill set is the one input the server owns rather than being
	// handed. Equipment is passed for weapon-dependent passives.
	if (m_skillDefinitions != nullptr)
	{
		const Result<PassiveContributionResult> aggregated =
			PassiveContributionAggregator::Aggregate(m_skills, *m_skillDefinitions, m_equipment);
		if (aggregated.IsError())
		{
			return aggregated.GetStatus();
		}
		if (!aggregated.GetValue().IsOk())
		{
			// A learned skill nothing defines is a data fault, not a bad argument.
			return Status(ErrorCode::InvalidArgument);
		}
		m_passives = aggregated.GetValue().contribution;
	}

	Stats::StatCalculationInput input;
		input.characterClass  = m_classIndex;
		input.level           = m_definition.level;
		input.classConstants  = m_definition.classConstants;
		input.allocatedStats  = m_definition.allocatedStats;
		input.items           = m_items;
		input.passives        = m_passives;
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
		// The item contribution is *not* taken from here any more. Equipment is
		// the only source for it, and accepting one here would let a caller
		// override what the character is actually wearing. The parameter stays
		// for source compatibility and is refused if it is non-zero, so a stale
		// caller fails loudly instead of silently losing its contribution.
		//
		// The passive contribution is refused for the same reason and with the
		// same reasoning: the learned skill set is its only source. It used to be
		// assigned to m_definition.passives here, which Recalculate() never reads
		// - so a non-zero value was accepted, reported as success, and discarded.
		// A silently discarded contribution is worse than a refusal, because the
		// caller has no way to tell the two apart.
		if (!Stats::IsZero(items) || !Stats::IsZero(passives))
		{
			return Status(ErrorCode::NotAllowed);
		}
		m_definition.codex = codex;
		return Recalculate();
	}

Status ServerCharacter::LearnSkill(const SkillId& id)
{
	if (m_skillDefinitions == nullptr)
	{
		return Status(ErrorCode::InvalidArgument);
	}
	// Validate the skill exists in our definitions.
	if (m_skillDefinitions->Find(id) == nullptr)
	{
		return Status(ErrorCode::NotFound);
	}
	// Stage the change on a copy, so a failure part-way cannot leave a
	// half-written skill or a contribution that does not match it.
	SkillState staged = m_skills;
	const Status learned = staged.LearnSkill(id);
	if (learned.IsError())
	{
		return learned;
	}

	const SkillState previous = m_skills;
	m_skills = staged;
	const Status calculated = Recalculate();
	if (calculated.IsError())
	{
		m_skills = previous;
		(void) Recalculate();
		return calculated;
	}
	return Ok();
}

Status ServerCharacter::UnlearnSkill(const SkillId& id)
{
	SkillState staged = m_skills;
	const Status removed = staged.UnlearnSkill(id);
	if (removed.IsError())
	{
		return removed;
	}

	const SkillState previous = m_skills;
	m_skills = staged;
	const Status calculated = Recalculate();
	if (calculated.IsError())
	{
		m_skills = previous;
		(void) Recalculate();
		return calculated;
	}
	return Ok();
}

Status ServerCharacter::SetSkillLevel(const SkillId& id, uint8_t level)
{
	if (level == 0 || level > kMaxSkillLevel)
	{
		return Status(ErrorCode::InvalidArgument);
	}
	if (m_skillDefinitions == nullptr)
	{
		return Status(ErrorCode::InvalidArgument);
	}
	// Validate the skill exists and the level is within its maxLevel.
	const SkillDefinition* def = m_skillDefinitions->Find(id);
	if (def == nullptr)
	{
		return Status(ErrorCode::NotFound);
	}
	if (level > def->maxLevel)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	SkillState staged = m_skills;
	const Status leveled = staged.SetSkillLevel(id, level);
	if (leveled.IsError())
	{
		return leveled;
	}

	const SkillState previous = m_skills;
	m_skills = staged;
	const Status calculated = Recalculate();
	if (calculated.IsError())
	{
		m_skills = previous;
		(void) Recalculate();
		return calculated;
	}
	return Ok();
}

Status ServerCharacter::Equip(EquipmentSlot slot, const ItemInstance& item)
	{
		if (m_itemDefinitions == nullptr)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		// Stage the change on a copy, so a failure part-way cannot leave a
		// half-written slot or a contribution that does not match it.
		EquipmentState staged = m_equipment;
		const Status equipped = staged.Equip(slot, item);
		if (equipped.IsError())
		{
			return equipped;
		}
		// Resolve the definition before committing, so an unknown item is
		// refused while the character is still untouched.
		if (m_itemDefinitions->Find(item.definition) == nullptr)
		{
			return Status(ErrorCode::NotFound);
		}

		const EquipmentState previous = m_equipment;
		m_equipment = staged;
		const Status calculated = Recalculate();
		if (calculated.IsError())
		{
			m_equipment = previous;
			(void) Recalculate();
			return calculated;
		}
		return Ok();
	}

	Status ServerCharacter::Unequip(EquipmentSlot slot)
	{
		EquipmentState staged = m_equipment;
		const Status removed = staged.Unequip(slot);
		if (removed.IsError())
		{
			return removed;
		}

		const EquipmentState previous = m_equipment;
		m_equipment = staged;
		const Status calculated = Recalculate();
		if (calculated.IsError())
		{
			m_equipment = previous;
			(void) Recalculate();
			return calculated;
		}
		return Ok();
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

		// The worn set, published so a client can present it. Slot index and
		// slot value are the same by construction: EquipmentState is an array
		// indexed by slot, so the offset *is* the slot.
		Gameplay::EquippedList equipped;
		equipped.count = m_equipment.GetOccupiedCount();
		for (size_t index = 0; index < kEquipmentSlotCount; ++index)
		{
			const EquipmentEntry& entry = m_equipment.GetSlots()[index];
			if (!entry.HasItem())
			{
				continue;
			}
			Gameplay::EquippedItem published;
			published.slot       = static_cast<EquipmentSlot>(index);
			published.definition = entry.item.definition;
			published.serial     = entry.item.serial;
			if (m_itemDefinitions != nullptr)
			{
				if (const ItemDefinition* definition =
						m_itemDefinitions->Find(entry.item.definition))
				{
					published.kind = definition->kind;
					published.name  = definition->name;
				}
			}
			equipped.items[index] = published;
		}
		snapshot.equipped = equipped;

		// The learned passive skills, published so a client can present them.
		Gameplay::SkillList skills;
		for (const auto& [skillId, learned] : m_skills.GetAllSkills())
		{
			if (!learned.IsLearned())
			{
				continue;
			}
			Gameplay::LearnedSkillEntry entry;
			entry.id    = skillId;
			entry.level = learned.level;
			if (m_skillDefinitions != nullptr)
			{
				if (const SkillDefinition* def = m_skillDefinitions->Find(skillId))
				{
					entry.name = def->name;
				}
			}
			skills.skills.push_back(std::move(entry));
		}
		snapshot.skills = std::move(skills);

		return Gameplay::CharacterSnapshot::Create(std::move(snapshot));
	}
}
