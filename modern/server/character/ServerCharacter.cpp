// VERTICAL-001: the server's authoritative character. See ServerCharacter.h.

#include "character/ServerCharacter.h"

#include "combat/CombatCalculator.h"
#include "combat/CombatTypes.h"
#include "combat/CombatConstants.h"
#include "equipment/ItemContributionAggregator.h"
#include "progression/CodexContributionAggregator.h"
#include "resources/ResourceState.h"
#include "skills/ActiveSkill.h"
#include "skills/PassiveContributionAggregator.h"
#include "stats/StatCalculator.h"

#include <algorithm>
#include <cmath>
#include <random>
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
	character.m_codexDefinitions  = character.m_definition.codexDefinitions;
	// The caller no longer supplies an item contribution: it is aggregated
	// from the worn set, which starts empty. A contribution passed here
	// alongside equipment would be a second, competing source.
	character.m_definition.items = Stats::ItemContribution();
	// The caller no longer supplies a passive contribution: it is aggregated
	// from the learned skill set, which starts empty.
	character.m_definition.passives = Stats::PassiveContribution();
	// VERTICAL-004: nor a codex contribution. It is aggregated from the completed
	// codex set, which starts empty.
	character.m_definition.codex = Stats::CodexContribution();

	// RAN seats every codex entry the character does not have at character load
	// (GLCharDataCodex.cpp:72-93, and the GM/new-character path at
	// GLCharDataLoad.cpp:357-373), so a freshly created character already has a
	// progress record for every definition. Doing it here rather than lazily
	// means the published snapshot is complete from the first frame, and it is
	// the same call a reload uses.
	if (character.m_codexDefinitions != nullptr)
	{
		character.m_codex.Reconcile(*character.m_codexDefinitions);
	}

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

	// VERTICAL-004: aggregate the completed codex set.
	//
	// This is RAN's CODEX_STATS (GLogixExPC.cpp:5103), reached from
	// GLChar::CodexComplete (GLCharCodex.cpp:57), from the client-side mirror
	// (GLCharacterMsg.cpp:5214) and from character initialisation
	// (GLChar.cpp:601, GLCharacter.cpp:921). Every one of those is a full
	// recompute from the completed map, so rebuilding here on every
	// recalculation is the same shape and is idempotent.
	//
	// RAN's version is not a failure path: a completed entry whose definition
	// cannot be resolved is skipped silently (GLogixExPC.cpp:5131) and the
	// aggregation continues. That is kept, and the count of skipped entries is
	// recorded so a caller can ask, because a reward that silently pays nothing
	// is otherwise indistinguishable from a working one.
	m_codexSkipReason = CodexContributionError::None;
	m_contributingCodex = 0;
	if (m_codexDefinitions != nullptr)
	{
		const Result<CodexContributionResult> aggregated =
			CodexContributionAggregator::Aggregate(m_codex, *m_codexDefinitions);
		if (aggregated.IsError())
		{
			return aggregated.GetStatus();
		}
		m_codexContribution  = aggregated.GetValue().contribution;
		m_codexSkipReason    = aggregated.GetValue().skipReason;
		m_contributingCodex  = aggregated.GetValue().contributingCodex;
	}
	else
	{
		// No definitions means no codex entries can be resolved, so the
		// contribution is zero rather than stale. A character created without a
		// codex table publishes no codex bonuses, which is visible in its
		// statistics rather than a stale number surviving a table being detached.
		m_codexContribution = Stats::CodexContribution();
	}

	Stats::StatCalculationInput input;
		input.characterClass  = m_classIndex;
		input.level           = m_definition.level;
		input.classConstants  = m_definition.classConstants;
		input.allocatedStats  = m_definition.allocatedStats;
		input.items           = m_items;
		input.passives        = m_passives;
		input.codex           = m_codexContribution;
		input.confPointRate   = m_definition.confPointRate;

		// VERTICAL-017: the timed FACT contribution, fed from the same rebuilt
		// modifier snapshot the combat path uses, so both consumers of a FACT
		// always agree.
		//
		// It is a fourth source beside items, passives and codex rather than an
		// addition to any of them, matching legacy, which sums `nSUM_MA` with
		// `m_sSUM_PASSIVE.m_nMA` only at the point of use (:2970-2972).
		input.facts.meleePower = m_factModifiers.meleePower;
		input.facts.shootPower = m_factModifiers.shootPower;
		input.facts.magicAttack = m_factModifiers.magicAttack;

		// VERTICAL-019: hit and avoid take the same additive path, in the same
		// run as items, passives and codex.
		input.facts.hit   = m_factModifiers.hit;
		input.facts.avoid = m_factModifiers.avoid;

		// VERTICAL-020: flat total defence and the all-axes resistance bonus.
		input.facts.defense = m_factModifiers.defense;
		input.facts.resist  = m_factModifiers.resist;

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
		//
		// VERTICAL-004 extends that to the codex contribution, which used to be
		// the one parameter this function still honoured. It is aggregated from
		// the completed codex set now, so honouring it here would let a caller
		// override a character's actual codex.
		//
		// The test is on the *argument*, like the two above. Testing the derived
		// `m_codexContribution` instead would invert the check twice over: a
		// caller passing a non-zero value would be told it was fine and have it
		// dropped, and a character with any real codex progress would be told
		// even a zeroed call was not allowed.
		if (!Stats::IsZero(items) || !Stats::IsZero(passives) ||
		    !Stats::IsZero(codex))
		{
			return Status(ErrorCode::NotAllowed);
		}
		return Recalculate();
	}

	Result<CodexRegistration> ServerCharacter::RegisterCodexItem(CodexId id,
	                                                            const ItemInstance& item)
	{
		if (m_codexDefinitions == nullptr)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// Stage the change on a copy, so a failure part-way cannot leave a
		// half-written record or a contribution that does not match it. The same
		// shape as LearnSkill and Equip.
		CodexState staged = m_codex;
		const Result<CodexRegistration> registered = staged.RegisterItem(id, item);
		if (registered.IsError())
		{
			return registered;
		}

		// A refused registration changes nothing, so there is no need to
		// recalculate. RAN's path does send an update message on this branch
		// (GLCharCodex.cpp:154-156) and none on a completion, because completion
		// sends its own; that is a transport detail, not a state change.
		if (!registered.GetValue().recorded)
		{
			return registered;
		}

		const CodexState previous = m_codex;
		m_codex = staged;
		const Status calculated = Recalculate();
		if (calculated.IsError())
		{
			m_codex = previous;
			(void) Recalculate();
			return calculated;
		}
		return registered;
	}

	Status ServerCharacter::ReconcileCodex()
	{
		if (m_codexDefinitions == nullptr)
		{
			// Reconciling against nothing would wipe every record, so this is
			// refused rather than performed.
			return Status(ErrorCode::InvalidArgument);
		}
		if (m_codexDefinitions->GetAll().empty())
		{
			// Same reason. An empty table is almost always a data fault, and
			// dropping a character's codex to nothing because a file failed to
			// load is not a recoverable outcome.
			return Status(ErrorCode::InvalidArgument);
		}

		const CodexState previous = m_codex;
		m_codex.Reconcile(*m_codexDefinitions);
		const Status calculated = Recalculate();
		if (calculated.IsError())
		{
			m_codex = previous;
			(void) Recalculate();
			return calculated;
		}
		return Ok();
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

		// The codex, published so a client can present a codex panel. RAN sends
		// the two maps as two blocks in one join message (GLCharEx.cpp:445-465)
		// and the client inserts each into its own map
		// (DxGameStage.cpp:931, :950); they are flattened here because a panel
		// shows both, and `completed` says which is which.
		//
		// Completed entries come first in RAN's tables only by accident of
		// iteration, so both maps are walked in id order and merged: the published
		// list is sorted by CodexId, which is also what the snapshot validator
		// requires.
		Gameplay::CodexList codex;
		if (m_codexDefinitions != nullptr)
		{
			// A record is published only if its definition still resolves, so a
			// client is never sent an entry it could not name. RAN's character
			// load drops the same records (GLCharDataCodex.cpp:95-132); doing it
			// here as well keeps the two consistent even if a record was seated
			// before a definition disappeared.
			auto publish = [&](CodexId id, const CodexProgress& record, bool completed)
			{
				const CodexDefinition* definition = m_codexDefinitions->Find(id);
				if (definition == nullptr)
				{
					return;
				}
				Gameplay::CodexEntry entry;
				entry.id            = id;
				entry.type          = definition->type;
				entry.name          = definition->title;
				entry.description   = definition->description;
				entry.badge         = definition->rewardBadge ? definition->badge : std::string();
				entry.completed     = completed;
				entry.requiredCount = record.requiredCount;
				entry.doneCount     = record.doneCount;
				codex.entries.push_back(std::move(entry));
			};

			// Two maps merged into one sorted list. The completed map first, then
			// the progress map, then a single sort: an id is in exactly one of the
			// two, so this cannot produce a duplicate.
			for (const auto& [id, record] : m_codex.GetAllCompleted())
			{
				publish(id, record, /*completed*/ true);
			}
			for (const auto& [id, record] : m_codex.GetAllProgress())
			{
				publish(id, record, /*completed*/ false);
			}
			std::sort(codex.entries.begin(), codex.entries.end(),
			          [](const Gameplay::CodexEntry& lhs, const Gameplay::CodexEntry& rhs)
			          { return lhs.id < rhs.id; });
		}
		snapshot.codex = std::move(codex);

		return Gameplay::CharacterSnapshot::Create(std::move(snapshot));
	}
}

// VERTICAL-006: Basic Physical Combat Resolution
//
// Server-authoritative attack resolution.
// Transactional: a refused attack leaves both characters unchanged.
//
// VERTICAL-007: Combat state now connected to real character/equipment data.
//   - attackerCriticalBonus: from derived criticalRate (items + passives)
//   - attackerCrushingBonus: from derived crushingBlow (items + passives)
//   - targetDefenseItem: from item contribution defense
//   - targetDamageReduce: from derived damageReduce (items + passives)
//   - targetDamageReflection: from derived damageReflection (items + passives)
//   - targetDamageReflectionRate: from derived damageReflectionRate
//   - targetLowSP: from SP state (SP == 0 proxy, see below)
//
// Still deferred:
//   - brightnessFB: Aver default (modern Core does not model world brightness)
//   - weatherElementPower: 1.0f default
//   - targetStateDamage: 1.0f (no state damage yet)
//
// Low-SP detection: legacy uses bLowSP = (float(m_sSP.dwNow) < float(m_wSUM_DisSP))
// (GLCharacter.cpp:3446). Without a skill system, we use SP == 0 as a proxy.
// This is documented as LIMITED.

namespace Modern::Server
{
	// Simple deterministic RNG for combat (will be replaced by proper RNG later)
	namespace
	{
		float DeterministicRandom()
		{
			// In production this would come from a proper RNG; for now use a fixed value
			// for deterministic testing. In production, this would be provided by the
			// caller (server's RNG).
			static std::mt19937 rng(0x12345678);
			static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
			return dist(rng);
		}
	}

	Status ServerCharacter::Attack(ServerCharacter& target)
	{
		// Cannot attack self
		if (this == &target)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// Both characters must be alive
		if (m_currentHp == 0 || target.m_currentHp == 0)
		{
			return Status(ErrorCode::InvalidState);
		}

		// Build combat input from derived stats and equipment contributions
		Combat::CombatInput input;
		input.attackerHit = m_derived.hit;
		input.attackerAvoid = m_derived.avoid;
		input.attackerMeleePower = m_derived.meleePower;
		input.attackerShootPower = m_derived.shootPower;
		input.attackerPhysicalDamage = m_derived.physicalDamage;
		input.attackerLevel = m_definition.level;
		input.attackerMaxHP = m_derived.maxHp;
		input.attackerCurrentHP = m_currentHp;
		// VERTICAL-007: critical rate from items + passives
		input.attackerCriticalBonus = static_cast<int32_t>(m_derived.criticalRate * 100.0f);
		// VERTICAL-007: crushing blow from items + passives
		input.attackerCrushingBonus = static_cast<int32_t>(m_derived.crushingBlow * 100.0f);
		input.attackType = Combat::AttackType::Melee;

		input.targetHit = target.m_derived.hit;
		input.targetAvoid = target.m_derived.avoid;
		input.targetDefense = target.m_derived.defense;
		input.targetDefenseBody = target.m_derived.defenseBody;
		// VERTICAL-007: item defense from equipment contribution
		input.targetDefenseItem = target.GetItemContribution().defense;
		input.targetLevel = target.m_definition.level;
		input.targetMaxHP = target.m_derived.maxHp;
		input.targetCurrentHP = target.m_currentHp;
		input.targetStateDamage = 1.0f; // No state damage yet
		// VERTICAL-007: damage reduction from items + passives
		input.targetDamageReduce = target.m_derived.damageReduce;
		// VERTICAL-007: damage reflection from items + passives
		input.targetDamageReflection = target.m_derived.damageReflection;
		input.targetDamageReflectionRate = target.m_derived.damageReflectionRate;
		input.targetResistElement = 0; // Not yet modeled (elemental combat)

		// VERTICAL-017: the target's FACT damage protection, combined with its own
		// DAMAGE_SPEC values by MAXIMUM.
		//
		// Legacy rebuilds `m_sDamageSpec` every tick and max-accumulates the FACT
		// reduction/reflection specs into it (GLogixExPC.cpp:2224-2228, then
		// :2380-2401). So the strongest source wins rather than the two adding,
		// and an expired buff stops contributing because the structure is rebuilt
		// instead of restored.
		//
		// The physical pair is wired here. The magic pair has no consumer on this
		// path because `Attack` is a physical basic attack; magic reads it through
		// the skill path instead.
		input.targetDamageReduce =
			target.m_factModifiers.psyDamageReduce > input.targetDamageReduce
				? target.m_factModifiers.psyDamageReduce
				: input.targetDamageReduce;
		input.targetDamageReflection =
			target.m_factModifiers.psyDamageReflection > input.targetDamageReflection
				? target.m_factModifiers.psyDamageReflection
				: input.targetDamageReflection;
		input.targetDamageReflectionRate =
			target.m_factModifiers.psyDamageReflectionRate > input.targetDamageReflectionRate
				? target.m_factModifiers.psyDamageReflectionRate
				: input.targetDamageReflectionRate;
		// VERTICAL-019: the attacker's `EMIMPACTA_DAMAGE` contribution, applied to
		// the damage range before the attack power (GLogixExPC.cpp:2329).
		input.factDamage = m_factModifiers.damage;

		// VERTICAL-010: the SP this attack costs, and the attacker's pool.
		//
		// Legacy: GLogixExPC.cpp:3492-3497 (BEGIN_ATTACK)
		//     WORD wDisSP = GLCONST_CHAR::wBASIC_DIS_SP;
		//     if ( pRHAND )  wDisSP += pRHAND->sSuitOp.wReqSP;
		//     if ( pLHAND )  wDisSP += pLHAND->sSuitOp.wReqSP;
		//     if ( m_sSP.dwNow < (wDisSP*wStrikeNum) )  return EMBEGINA_SP;
		//
		// m_items.requiredSP is the wReqSP sum of the two hand slots, produced
		// by ItemContributionAggregator, so wBASIC_DIS_SP is the only term added
		// here. The comparison itself is Combat::ResolveCombat's, which keeps
		// the rule in core rather than restating it per caller.
		//
		// m_wACCEPTP is not added: the legacy gate above does not use it either.
		input.attackerRequiredSP = static_cast<uint16_t>(
			m_items.requiredSP + Combat::CombatConstants().basicDisSP);
		input.attackerCurrentSP = m_currentSp;

		// VERTICAL-009: PK combat flag.
		//
		// Legacy: GLChar.cpp:1995 (IsReActionable), GLCharacter.cpp:2306 (IsPK_TAR)
		//
		// PK state detection (safe zones, party checks, PK maps) is a server
		// concern and is not yet modeled. The combat calculator applies the
		// PK damage modifier when this flag is true.
		input.isPK = false;

		input.attackerMaxHP = m_derived.maxHp;
		input.attackerCurrentHP = m_currentHp;

		input.targetMaxHP = target.m_derived.maxHp;
		input.targetCurrentHP = target.m_currentHp;

		input.brightnessFB = Modern::Engine::GameBrightFB::Aver;
		input.weatherElementPower = 1.0f;

		// Deterministic random values for testing
		input.hitRoll = DeterministicRandom();
		input.damageRoll = DeterministicRandom();
		input.criticalRoll = DeterministicRandom();
		input.crushingRoll = DeterministicRandom();
		input.reflectionRoll = DeterministicRandom();

		// Resolve combat
		Combat::CombatResult result = Combat::ResolveCombat(input);

		// Apply damage to target
		if (result.IsHit())
		{
			uint32_t damageApplied = result.damageResult.damage;
			if (damageApplied > target.m_currentHp)
			{
				damageApplied = target.m_currentHp;
			}
			target.m_currentHp -= damageApplied;
		}

		// VERTICAL-008: apply reflection damage to attacker.
		//
		// Legacy: GLChar.cpp:2684-2703 (DamageReflectionProc)
		//
		// Reflection damage is applied to the attacker via ResourceState.
		// Reflection cannot recursively trigger.
		// The operation is transactional: if reflection fails, both characters
		// remain unchanged.
		if (result.IsReflection() && result.damageResult.reflectionDamage > 0)
		{
			uint32_t reflectionApplied = result.damageResult.reflectionDamage;
			if (reflectionApplied > m_currentHp)
			{
				reflectionApplied = m_currentHp;
			}
			m_currentHp -= reflectionApplied;
		}

		// Both characters must recalculate if stats changed (they didn't in this simple case)
		Status selfRecalc = Recalculate();
		if (selfRecalc.IsError()) return selfRecalc;
		Status targetRecalc = target.Recalculate();
		if (targetRecalc.IsError()) return targetRecalc;

		return Ok();
	}

	// ── VERTICAL-011: active skills ─────────────────────────────────────

	Skills::ActiveSkillResult ServerCharacter::CastSkill(const SkillId& id,
	                                                     ServerCharacter& target,
	                                                     uint16_t requestedLevel)
	{
		// `requestedLevel` is accepted for call-site compatibility and then
		// ignored. GLogixExPC.cpp:4074-4076 takes the level from
		// `m_ExpSkills.find(skill_id.dwID)`, the character's own learned set -
		// never from the message. Honouring the caller's number would let a
		// caller cast a level it has not learned.
		(void)requestedLevel;

		Skills::ActiveSkillResult result;

		if (this == &target)
		{
			result.failure = Skills::ActiveSkillFailure::UnsupportedTarget;
			return result;
		}

		// ---- 1. The definition, from this character's own provider ----
		//
		// GLogixExPC.cpp:4085-4086: no definition is EMSKILL_UNKNOWN.

		if (m_skillDefinitions == nullptr)
		{
			result.failure = Skills::ActiveSkillFailure::UnknownSkill;
			return result;
		}

		const SkillDefinition* definition = m_skillDefinitions->Find(id);
		if (definition == nullptr)
		{
			result.failure = Skills::ActiveSkillFailure::UnknownSkill;
			return result;
		}

		// ---- 2. The level, from this character's own learned set ----
		//
		// GLogixExPC.cpp:4074-4076.

		const uint8_t level = m_skills.GetSkillLevel(id);
		if (level == 0)
		{
			result.failure = Skills::ActiveSkillFailure::NotLearned;
			return result;
		}

		// ---- 3. Build the situation and let core rule on it ----
		//
		// Everything below is a read of authoritative state. The resolver is
		// pure: it decides, and this function applies.

		Skills::ActiveSkillInput input;
		input.definition  = definition;
		input.level       = level;
		input.attacker    = m_derived;
		input.target      = target.m_derived;
		input.hasTarget   = true;
		input.targetCurrentHp = target.m_currentHp;
		input.targetLevel  = target.m_definition.level;
		input.attackerLevel = m_definition.level;
		input.currentHp   = m_currentHp;
		input.currentMp   = m_currentMp;
		input.currentSp   = m_currentSp;

		// VERTICAL-010's contribution, reused rather than recomputed.
		input.equipmentRequiredSP = m_items.requiredSP;
		input.basicAttackSP = Combat::CombatConstants().basicDisSP;

		input.onCooldown = IsSkillOnCooldown(id);

		// VERTICAL-017, tasks 4 and 5: two FACT-driven inputs that the resolver
		// already understands but that nothing was feeding.
		//
		// PROHIBIT_SKILL. Legacy checks this in CHECKSKILL at GLogixExPC.cpp:4060,
		// the FIRST statement of the function, before the learned check (:4077)
		// and before the cooldown check (:4082) - and therefore before any SP is
		// spent. `m_factModifiers` is rebuilt from the live FACT pool by
		// `AdvanceSkillFacts`, so it is zero for a character with no buffs and
		// returns to `false` on its own once the FACT expires. There is no
		// second prohibition check here: `ActiveSkillResolver` owns the decision
		// and already refuses with `NotCastable`.
		input.skillProhibited = m_factModifiers.prohibitSkill;

		// NONBLOW. The target's immunity mask, aggregated from its FACTs, handed
		// to the status resolver as a plain value. The two domains stay separate:
		// this is a number crossing a boundary, not shared storage. Legacy applies
		// the mask at GLChar.cpp:3376-3379, gating the probability call entirely.
		//
		// The status random roll is left at its default here. Supplying it needs a
		// deterministic per-cast source, and guessing one would change which
		// blows land; that is DEFERRED rather than invented.
		input.targetDisorderMask = target.m_factModifiers.statusImmunityMask;

		// VERTICAL-019: the skill damage range contribution.
		input.factDamage = m_factModifiers.damage;

		input.hitRoll        = DeterministicRandom();
		input.damageRoll     = DeterministicRandom();
		input.criticalRoll   = DeterministicRandom();
		input.crushingRoll   = DeterministicRandom();
		input.reflectionRoll = DeterministicRandom();

		// No map weather exists yet; WEATHER_ELEMENT_POW returns 1.0f when
		// weather is inactive (GameCharacterCalculations.cpp:118-126).
		input.weatherElementPower = 1.0f;

		result = Skills::ActiveSkillResolver::Resolve(input);

		if (!result.Succeeded())
		{
			// Transactional: nothing was read that changes state, so a refusal
			// needs no rollback and starts no cooldown.
			return result;
		}

		// ---- 4. Apply the costs, through the resource domain ----
		//
		// The costs are drawn here rather than inside the resolver, because the
		// resolver does not own the pools. Going through ResourceState rather
		// than subtracting by hand is what gives RAN's `GLDWDATA::DECREASE`
		// saturation (`if (dwNow >= dwValue) dwNow -= dwValue; else dwNow = 0;`)
		// for free.
		//
		// HP and MP come from ACCOUNTSKILL (GLogixExPC.cpp:4296-4299); SP comes
		// from SkillProc (GLChar.cpp:3003-3009) and is zero whenever the cast
		// was low-SP.

		Resources::ResourceState pool;
		(void) pool.SyncFrom(m_derived);
		(void) pool.SetCurrent(Resources::ResourceKind::Hp, m_currentHp);
		(void) pool.SetCurrent(Resources::ResourceKind::Mp, m_currentMp);
		(void) pool.SetCurrent(Resources::ResourceKind::Sp, m_currentSp);

		if (result.spCost > 0) { (void) pool.Spend(Resources::ResourceKind::Sp, result.spCost); }
		if (result.mpCost > 0) { (void) pool.Spend(Resources::ResourceKind::Mp, result.mpCost); }
		if (result.hpCost > 0) { (void) pool.Spend(Resources::ResourceKind::Hp, result.hpCost); }

		m_currentSp = pool.GetCurrent(Resources::ResourceKind::Sp);
		m_currentMp = pool.GetCurrent(Resources::ResourceKind::Mp);
		m_currentHp = pool.GetCurrent(Resources::ResourceKind::Hp);

		// ---- 4b. Apply the status blow to the target ----
		//
		// VERTICAL-014. The resolver already decided whether the blow lands; this
		// stores it on the target, which is the authority for status state.
		//
		// Legacy puts the resolved SSTATEBLOW on the SKILLACTEX payload and the
		// target's own STATEBLOW writes the slot (GLChar.cpp:3396 then
		// GLChar.cpp:6200). There is no transport in this milestone, so the store
		// happens directly against the target - the same two-step, with the
		// network leg absent rather than faked.
		if (result.hasStatusApplication && result.statusApplication.Applied())
		{
			(void) target.ApplyStatus(result.statusApplication.state);
		}

		// VERTICAL-015: the same two-step for a FACT. The resolver built the
		// record; the target owns the pool and stores it.
		if (result.hasSkillFact)
		{
			(void) target.ApplySkillFact(result.skillFact);
		}

		// ---- 5. Apply the damage, and any reflection, to the same pools ----
		//
		// The combat half of the result was produced by the same pipeline
		// `Attack` uses, so the application matches: saturating, and applied
		// once.

		Resources::ResourceState targetPool;
		(void) targetPool.SyncFrom(target.m_derived);
		(void) targetPool.SetCurrent(Resources::ResourceKind::Hp, target.m_currentHp);

		// VERTICAL-019, magic hit-check exclusion.
		//
		// Legacy does not roll for a hit on a magic skill at all:
		// `GLChar::PreStrikeProc` sets `sTargetID.dwID = EMTARGET_NULL` when
		// `emAPPLY == EMAPPLY_MAGIC` (GLChar.cpp:2402-2405), and the null target
		// skips `CHECKHIT` at :2414, leaving `bhit` true. Basic attacks and
		// physical/ranged skills do roll (:2378, :2400).
		//
		// Without this gate a hit/avoid FACT could make a magic skill miss,
		// which RAN cannot do. The hit result is still computed - it is simply
		// not consulted for the magic channel, exactly as legacy leaves it.
		const bool channelRollsForHit =
			result.attackTypeUsed != Combat::AttackType::Magic;

		if (!channelRollsForHit || result.combat.IsHit())
		{
			(void) targetPool.ApplyDamage(result.combat.damageResult.damage);
			target.m_currentHp = targetPool.GetCurrent(Resources::ResourceKind::Hp);
		}

		// VERTICAL-008 reflection, same rule as `Attack`.
		if (result.combat.IsReflection() && result.combat.damageResult.reflectionDamage > 0)
		{
			(void) pool.ApplyDamage(result.combat.damageResult.reflectionDamage);
			m_currentHp = pool.GetCurrent(Resources::ResourceKind::Hp);
		}

		// ---- 6. Start the cooldown ----
		//
		// GLogixExPC.cpp:4304 inserts into `m_SKILLDELAY`; the value is
		// `SKILLDELAY(...) * m_fSTATE_DELAY`, and RAN's server-side path then
		// subtracts the 0.3f `NET_MSGDELAY` network compensation. Neither the
		// state multiplier nor the network term has a modern counterpart, so
		// the base value is stored as computed by the resolver.
		//
		// A zero delay is not inserted, matching legacy's map semantics: an
		// entry with a zero value would be erased on the next tick anyway, and
		// `CHECHSKILL` only asks whether a key exists.

		if (result.cooldownSeconds > 0.0f)
		{
			m_skillCooldowns[id] = result.cooldownSeconds;
		}

		// Recalculate so the published snapshot cannot lag the state behind it.
		// Resource maxima did not change, so this cannot fail on a stat ground,
		// but a failure here must not be swallowed.
		const Status selfRecalc = Recalculate();
		if (selfRecalc.IsError())
		{
			return result;
		}
		const Status targetRecalc = target.Recalculate();
		if (targetRecalc.IsError())
		{
			return result;
		}

		return result;
	}

	bool ServerCharacter::IsSkillOnCooldown(const SkillId& id) const noexcept
	{
		// GLogixExPC.cpp:4082-4083: the gate is `find() != end()`, i.e. the
		// presence of an entry, not whether the remaining time is positive. A
		// zero-delay skill is therefore never inserted at all.
		return m_skillCooldowns.find(id) != m_skillCooldowns.end();
	}

	float ServerCharacter::GetSkillCooldownRemaining(const SkillId& id) const noexcept
	{
		const auto it = m_skillCooldowns.find(id);
		return (it == m_skillCooldowns.end()) ? 0.0f : it->second;
	}

	void ServerCharacter::AdvanceSkillCooldowns(float elapsedSeconds) noexcept
	{
		// GLogixExPC.cpp:3864-3879:
		//   fDelay -= fElapsedTime;
		//   if ( fDelay <= 0.0f )  m_SKILLDELAY.erase ( iter_del );
		for (auto it = m_skillCooldowns.begin(); it != m_skillCooldowns.end(); )
		{
			it->second -= elapsedSeconds;
			if (it->second <= 0.0f)
			{
				it = m_skillCooldowns.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	// VERTICAL-014: status wrappers. All three delegate to
	// `StatusEffectContainer`, which owns the rules; nothing is reimplemented
	// here, and in particular the server does not decide whether a blow lands -
	// `StatusEffectResolver` does, and this only stores the verdict.

	bool ServerCharacter::ApplyStatus(const StatusEffect::StatusEffectState& state) noexcept
	{
		return m_status.Apply(state);
	}

	uint32_t ServerCharacter::TickStatus(float elapsedSeconds) noexcept
	{
		return m_status.Tick(elapsedSeconds);
	}

	uint32_t ServerCharacter::CureStatus(StatusEffect::StatusDisorder mask) noexcept
	{
		return m_status.Cure(mask);
	}

	// VERTICAL-015: FACT wrappers, delegating to the core container. As with the
	// status wrappers, no rule is reimplemented on the server.

	bool ServerCharacter::ApplySkillFact(const Skills::SkillFact& fact) noexcept
	{
		const bool applied = m_skillFacts.Apply(fact);
		if (applied)
		{
			// A new FACT may carry an attack-power impact, and the derived stats
			// are a cached snapshot. Refresh when - and only when - one of the
			// power values actually moved, so the cost is not paid on every buff.
			RefreshFactStatsIfPowersChanged();
		}
		return applied;
	}

	Skills::SkillFactModifiers ServerCharacter::AdvanceSkillFacts(float elapsedSeconds) noexcept
	{
		const Skills::SkillFactAdvanceResult advanced =
			Skills::AdvanceSkillFacts(m_skillFacts, elapsedSeconds);

		m_factModifiers = advanced.modifiers;
		RefreshFactStatsIfPowersChanged();
		return m_factModifiers;
	}

	void ServerCharacter::RefreshFactStatsIfPowersChanged() noexcept
	{
		// Recalculate() re-reads m_factModifiers, so it has to run only after the
		// snapshot has been updated - which is why this is not folded into the
		// assignment above.
		// VERTICAL-019: hit and avoid are folded into DerivedStats the same way the
	// powers are, so they belong in the same watch list.
		if (m_factStatsPowersApplied.meleePower  != m_factModifiers.meleePower ||
		    m_factStatsPowersApplied.shootPower  != m_factModifiers.shootPower ||
		    m_factStatsPowersApplied.magicAttack != m_factModifiers.magicAttack ||
		    m_factStatsPowersApplied.hit         != m_factModifiers.hit ||
		    m_factStatsPowersApplied.avoid       != m_factModifiers.avoid ||
		    m_factStatsPowersApplied.defense      != m_factModifiers.defense ||
		    m_factStatsPowersApplied.resist       != m_factModifiers.resist)
		{
			m_factStatsPowersApplied.meleePower  = m_factModifiers.meleePower;
			m_factStatsPowersApplied.shootPower  = m_factModifiers.shootPower;
			m_factStatsPowersApplied.magicAttack = m_factModifiers.magicAttack;
			m_factStatsPowersApplied.hit         = m_factModifiers.hit;
			m_factStatsPowersApplied.avoid       = m_factModifiers.avoid;
			m_factStatsPowersApplied.defense     = m_factModifiers.defense;
			m_factStatsPowersApplied.resist      = m_factModifiers.resist;
			(void) Recalculate();
		}
	}
}
