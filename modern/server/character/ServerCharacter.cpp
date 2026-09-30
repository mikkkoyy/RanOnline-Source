// VERTICAL-001: the server's authoritative character. See ServerCharacter.h.

#include "character/ServerCharacter.h"

#include "equipment/ItemContributionAggregator.h"
#include "progression/CodexContributionAggregator.h"
#include "skills/PassiveContributionAggregator.h"
#include "stats/StatCalculator.h"

#include <algorithm>
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

#include "combat/CombatCalculator.h"
#include "combat/CombatTypes.h"
#include "combat/CombatConstants.h"
#include "resources/ResourceState.h"

#include <random>

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
		// VERTICAL-007: low-SP detection (SP == 0 proxy, see note above)
		input.targetLowSP = (target.m_currentSp == 0);

		input.attackerMaxHP = m_derived.maxHp;
		input.attackerCurrentHP = m_currentHp;

		input.targetMaxHP = target.m_derived.maxHp;
		input.targetCurrentHP = target.m_currentHp;

		input.brightnessFB = GameCharacterCalculations::GameBrightFB::Aver;
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

		// Both characters must recalculate if stats changed (they didn't in this simple case)
		Status selfRecalc = Recalculate();
		if (selfRecalc.IsError()) return selfRecalc;
		Status targetRecalc = target.Recalculate();
		if (targetRecalc.IsError()) return targetRecalc;

		return Ok();
	}
}
