#include "Character.h"
#include "../math/Vector3.h"
#include "../progression/ProgressionData.h"
#include "CharacterBaseData.h"
#include "../item/ItemData.h"
#include "../item/InstanceCustomContribution.h"
#include "CombatStats.h"
#include "CodexContribution.h"

#include <cstdio>
#include <algorithm>

namespace Modern
{
	Character::Character(EntityId id, const std::string& name)
		: m_entity(id, Vector3::Zero, Vector3::Forward)
	{
		SetName(name);
	}

	void Character::SetName(const std::string& name)
	{
		m_nameString = name;
		std::snprintf(m_name, NameCapacity + 1, "%s", name.c_str());
	}

	void Character::Spawn(const Vector3& position, const Vector3& direction)
	{
		m_entity.position = position;
		m_entity.direction = Normalize(direction);
		m_action = ActionType::Idle;
		m_actState = ActState::None;
		m_moving = false;

		// Initialize from base data if available
		RefreshBaseData();
		if (m_hpMax > 0)
			m_hpNow = m_hpMax;
		if (m_mpMax > 0)
			m_mpNow = m_mpMax;
		if (m_spMax > 0)
			m_spNow = m_spMax;
	}

	void Character::Despawn()
	{
		m_moving = false;
		m_action = ActionType::Idle;
	}

	void Character::SetPosition(const Vector3& position)
	{
		m_entity.position = position;
	}

	void Character::SetDirection(const Vector3& direction)
	{
		m_entity.direction = Normalize(direction);
	}

	void Character::SetAction(ActionType action)
	{
		m_action = action;
	}

	void Character::SetClass(uint32_t classId, Gender gender)
	{
		m_classId = classId;
		m_gender = gender;
		RefreshBaseData();
	}

	void Character::SetSchool(uint16_t school)
	{
		m_school = school;
		RefreshBaseData();
	}

	void Character::SetLevel(uint16_t level)
	{
		m_level = std::min(level, m_maxLevel);
		RefreshExpMax();
		ApplyLevelUpStats();
		RefreshBaseData();
	}

	void Character::RefreshExpMax()
	{
		if (m_progression)
			m_expMax = m_progression->GetRequiredExperience(m_level);
		else
			m_expMax = 0;
	}

void Character::SetProgressionData(const ProgressionData& data)
{
	m_progression = &data;
	RefreshExpMax();
}

void Character::SetBaseDataProvider(const ICharacterBaseDataProvider& provider)
{
	m_baseDataProvider = &provider;
	RefreshBaseData();
}

void Character::SetItemDataProvider(const IItemDataProvider& provider)
{
	m_itemDataProvider = &provider;
	RecalculateStats();
}

void Character::SetPassiveSkillProvider(const IPassiveSkillProvider& provider)
{
	m_passiveSkillProvider = &provider;
	RecalculateStats();
}

void Character::SetCodexProvider(const ICodexProvider& provider)
{
	m_codexProvider = &provider;
	RecalculateStats();
}

	void Character::RefreshBaseData()
	{
		if (!m_baseDataProvider)
			return;

		const CharacterBaseData* data = m_baseDataProvider->GetBaseData(m_classId, m_school);
		if (!data)
			data = m_baseDataProvider->GetBaseDataByClassIndex(m_classId);
		if (!data)
			return;

		m_baseData = data;
		m_maxLevel = data->maxLevel;

		// Update recovery rates from base data if set
		if (data->hpRecoverPerSec > 0) m_hpRecoverPerSec = data->hpRecoverPerSec;
		if (data->mpRecoverPerSec > 0) m_mpRecoverPerSec = data->mpRecoverPerSec;
		if (data->spRecoverPerSec > 0) m_spRecoverPerSec = data->spRecoverPerSec;

		// Calculate current stats based on level
		m_currentStats = CalculateStatsAtLevel(*data, m_level);

		// Recalculate full stats including items
		RecalculateStats();
	}

	void Character::ApplyLevelUpStats()
	{
		if (!m_baseData)
			return;

		// Recalculate stats at current level
		m_currentStats = CalculateStatsAtLevel(*m_baseData, m_level);
		RefreshBaseData();
	}

void Character::RecalculateStats()
{
	if (!m_baseData)
		return;

	// Calculate current stats based on level (includes base stats + level-up gains)
	m_currentStats = CalculateStatsAtLevel(*m_baseData, m_level);

	// Calculate item contributions
	m_itemContribution.Reset();
	if (m_itemDataProvider)
	{
		m_itemContribution = CalculateItemContribution(
			m_itemDataProvider,
			m_equippedItems.data(),
			EquipSlotCount);
	}

	// Calculate instance custom contributions (GETADDPA, GETADDSA, GETDAMAGE, etc.)
	m_instanceContribution.Reset();
	if (m_itemDataProvider)
	{
		m_instanceContribution = CalculateInstanceCustomContribution(
			m_itemDataProvider,
			m_equippedItems.data(),
			EquipSlotCount);
	}

	// Get passive skill contributions
	PassiveSkillContribution passiveContribution;
	if (m_passiveSkillProvider)
	{
		passiveContribution = m_passiveSkillProvider->GetContribution(*this);
	}

	// Get codex contributions
	CodexContribution codexContribution;
	if (m_codexProvider)
	{
		codexContribution = m_codexProvider->GetCodexContribution();
	}

	// Add item stat contributions to current stats (legacy behavior: stats include item bonuses before HP/MP/SP calc)
	m_currentStats.pow  += m_itemContribution.pow;
	m_currentStats.str  += m_itemContribution.str;
	m_currentStats.spi  += m_itemContribution.spi;
	m_currentStats.dex  += m_itemContribution.dex;
	m_currentStats.intel += m_itemContribution.intel;
	m_currentStats.sta  += m_itemContribution.sta;

	// Add passive stat contributions (legacy: m_sSUM_PASSIVE.m_nPA/SA/MA added to stats)
	m_currentStats.pow  += passiveContribution.pa;
	m_currentStats.spi  += passiveContribution.sa;
	m_currentStats.intel += passiveContribution.ma;

	// Calculate max resources from stats + class factors + item contributions + passive contributions
	// Legacy formula (GLOGICEX::SUM_ADDITION):
	//   1. base = STAT * factor + itemFlatHP + passiveFlatHP  -> DWORD truncation
	//   2. base = base * (1 + itemRate + passiveRate) * conftRate  -> DWORD truncation
	//   3. base += itemVolume
	//   4. base += codexIncrease
	//   5. LIMIT()
	// Modern: match truncation steps; passive and conft integrated; codex not yet
	// TODO(verification): Integrate codex increases (m_dwHPIncrease, etc.)
	// TODO(verification): Instance custom values
	// TODO(verification): Additional random options
	// TODO(verification): Charm handling
	float strFactor = m_baseData->hpStrFactor;
	float spiFactor = m_baseData->mpSpiFactor;
	float staFactor = m_baseData->spStaFactor;

	// Step 1: base = STAT * factor + itemFlatHP + passiveFlatHP (truncated to integer, matching legacy DWORD cast)
	uint32_t baseHP = static_cast<uint32_t>(m_currentStats.str * strFactor + m_itemContribution.hp + passiveContribution.hp);
	uint32_t baseMP = static_cast<uint32_t>(m_currentStats.spi * spiFactor + m_itemContribution.mp + passiveContribution.mp);
	uint32_t baseSP = static_cast<uint32_t>(m_currentStats.sta * staFactor + m_itemContribution.sp + passiveContribution.sp);

	// Step 2: Apply rate multipliers (1 + itemRate + passiveRate) * conftPointRate with truncation (matching legacy second DWORD cast)
	// Legacy SUM_ADDITION: DWORD(base * (1 + passiveRate) * fCONFT_POINT_RATE)
	// Legacy UPDATE_MAX_POINT: DWORD(base * (1 + passiveRate + activeRate) * fCONFT_POINT_RATE)
	// Modern: itemRate + passiveRate combined, conft rate applied at same position
	baseHP = static_cast<uint32_t>(baseHP * (1.0f + m_itemContribution.hpRate + passiveContribution.hpRate) * m_conftPointRate);
	baseMP = static_cast<uint32_t>(baseMP * (1.0f + m_itemContribution.mpRate + passiveContribution.mpRate) * m_conftPointRate);
	baseSP = static_cast<uint32_t>(baseSP * (1.0f + m_itemContribution.spRate + passiveContribution.spRate) * m_conftPointRate);

	// Step 3: Add volume (flat, no further truncation in legacy before LIMIT)
	baseHP += static_cast<uint32_t>(m_itemContribution.hpVolume);
	baseMP += static_cast<uint32_t>(m_itemContribution.mpVolume);
	baseSP += static_cast<uint32_t>(m_itemContribution.spVolume);

	// Step 4: Add codex increases (legacy: m_dwHPIncrease added AFTER rate multiplication)
	// Legacy: m_sHP.dwMax += m_dwHPIncrease
	baseHP += codexContribution.hpIncrease;
	baseMP += codexContribution.mpIncrease;
	baseSP += codexContribution.spIncrease;

	// Step 5: Final max values (legacy LIMIT() clamps to max DWORD, not needed here)
	m_hpMax = baseHP;
	m_mpMax = baseMP;
	m_spMax = baseSP;

	// Clamp current values to new max
	if (m_hpNow > m_hpMax) m_hpNow = m_hpMax;
	if (m_mpNow > m_mpMax) m_mpNow = m_mpMax;
	if (m_spNow > m_spMax) m_spNow = m_spMax;

	// Calculate combat stats (legacy GLOGICEX::SUM_ADDITION order)
	// Note: This mirrors the legacy calculation sequence exactly
	// 1. Base values from stats
	// 2. Item flat contributions
	// 3. Passive flat contributions
	// 4. Item rate multipliers
	// 5. Passive rate multipliers
	// 6. Skill/buff/pet/land effects (not yet implemented)

	// Character constants (legacy cCHARCONST)
	constexpr float kHitDexFactor   = 1.0f;  // cCHARCONST.fHIT_DEX
	constexpr float kAvoidDexFactor = 1.0f;  // cCHARCONST.fAVOID_DEX
	constexpr float kDefenseDexFactor = 1.0f; // cCHARCONST.fDEFENSE_DEX

	// Base DP/AP from character base data (m_wSUM_DP, m_wSUM_AP)
	// These would come from class-specific formulas; for now use baseData placeholders
	// Legacy: m_wSUM_DP = class base defense + level scaling
	// Legacy: m_wSUM_AP = class base attack + level scaling
	const int32_t baseDP = m_baseData->baseDefense; // placeholder
	const int32_t baseAP = m_baseData->baseAttack;  // placeholder

	// Reset combat stats
	m_combatStats.Reset();

	// --- HIT ---
	// Legacy: int ( DEX * fHIT_DEX + itemHit + passiveHit + codexHit )
	// Then: int ( hit * (100 + itemHitRatePer + instanceHitRatePer) * 0.01f )
	{
		int32_t hit = static_cast<int32_t>(m_currentStats.dex * kHitDexFactor
			+ m_itemContribution.hitRate
			+ passiveContribution.hitRate
			+ static_cast<int32_t>(codexContribution.hitRateIncrease));
		// Item rate multiplier (percentage) + instance rate
		hit = static_cast<int32_t>(hit * (100.0f + m_itemContribution.hitRatePer + m_instanceContribution.hitRatePer) * 0.01f);
		m_combatStats.hitRate = hit;
	}

	// --- AVOID ---
	// Legacy: int ( DEX * fAVOID_DEX + itemAvoid + passiveAvoid + codexAvoid )
	// Then: int ( avoid * (100 + itemAvoidRatePer + instanceAvoidRatePer) * 0.01f )
	{
		int32_t avoid = static_cast<int32_t>(m_currentStats.dex * kAvoidDexFactor
			+ m_itemContribution.avoidRate
			+ passiveContribution.avoidRate
			+ static_cast<int32_t>(codexContribution.avoidRateIncrease));
		avoid = static_cast<int32_t>(avoid * (100.0f + m_itemContribution.avoidRatePer + m_instanceContribution.avoidRatePer) * 0.01f);
		m_combatStats.avoidRate = avoid;
	}

	// --- DEFENSE ---
	// Legacy: int ( DP + DEX * fDEFENSE_DEX ) = m_nDEFENSE_BODY
	// Then: int ( m_nDEFENSE_BODY + itemDefense + passiveDefense + instanceDefense + codexDefense ) = m_nDEFENSE
	// Then: ApplyDefenseRate( defense, defenseRate )
	{
		int32_t defenseBody = static_cast<int32_t>(baseDP + m_currentStats.dex * kDefenseDexFactor);
		m_combatStats.defenseBody = defenseBody;

		int32_t defense = defenseBody
			+ m_itemContribution.defense
			+ passiveContribution.defense
			+ m_instanceContribution.defense
			+ static_cast<int32_t>(codexContribution.defenseIncrease);
		m_combatStats.defense = defense;

		// Defense rate: base 1.0 + passive + later skill/buff/pet/land
		m_combatStats.defenseRate = 1.0f + passiveContribution.defenseRate;
		// Apply rate
		m_combatStats.defenseSkill = static_cast<int32_t>(defense * m_combatStats.defenseRate);
	}

	// --- DAMAGE ---
	// Legacy: int ( AP + passiveDamage + codexAttack ) = m_gdDAMAGE (low=high for melee)
	// Then: m_gdDAMAGE_SKILL = m_gdDAMAGE
	// Then: m_gdDAMAGE_PHYSIC = m_gdDAMAGE_SKILL + itemDamage + instanceDamage
	// Then: VAR_PARAM with PA (melee) or SA (range) + instance PA
	// Then: ApplyDamageRate to physical damage
	{
		CombatStats::DamageRange baseDamage;
		baseDamage.low = baseAP + passiveContribution.damage + static_cast<int32_t>(codexContribution.attackIncrease);
		baseDamage.high = baseAP + passiveContribution.damage + static_cast<int32_t>(codexContribution.attackIncrease);
		m_combatStats.baseDamage = baseDamage;

		// Skill damage (after skill buffs, not yet implemented)
		m_combatStats.skillDamage = baseDamage;

		// Physical damage: add item damage
		CombatStats::DamageRange physicalDamage = baseDamage;
		physicalDamage.low += static_cast<int32_t>(m_itemContribution.damageLow);
		physicalDamage.high += static_cast<int32_t>(m_itemContribution.damageHigh);

		// Add instance custom damage (legacy: GETDAMAGE() added to m_sSUMITEM.gdDamage)
		physicalDamage.low += static_cast<int32_t>(m_instanceContribution.damageLow);
		physicalDamage.high += static_cast<int32_t>(m_instanceContribution.damageHigh);

		// VAR_PARAM: add PA (melee) or SA (ranged) - for now assume melee
		physicalDamage.low += m_currentStats.pow;  // PA equivalent
		physicalDamage.high += m_currentStats.pow;

		// Add instance custom PA/SA (legacy: GETADDPA/GETADDSA added to m_sSUMITEM.nPA/nSA)
		physicalDamage.low += m_instanceContribution.addPA;
		physicalDamage.high += m_instanceContribution.addPA;

		// Damage rate: base 1.0 + passive + later skill/buff/pet/land
		m_combatStats.damageRate = 1.0f + passiveContribution.damageRate;

		// Apply damage rate with DWORD truncation (legacy behavior)
		physicalDamage.low = static_cast<uint32_t>(physicalDamage.low * m_combatStats.damageRate);
		physicalDamage.high = static_cast<uint32_t>(physicalDamage.high * m_combatStats.damageRate);

		// Legacy clamp: if >= 50000, set to 1
		if (physicalDamage.low >= 50000) physicalDamage.low = 1;
		if (physicalDamage.high >= 50000) physicalDamage.high = 1;

		m_combatStats.physicalDamage = physicalDamage;
	}

	// --- RESIST ---
	// Legacy: passiveResist + itemResist + codexResist
	// Later: skill/buff/pet/land add, then LIMIT()
	{
		int32_t codexResist = static_cast<int32_t>(codexContribution.resistanceIncrease);
		m_combatStats.resistFire   = CombatStats::ApplyResistanceLimit(passiveContribution.resistFire   + m_itemContribution.resistFire + codexResist);
		m_combatStats.resistIce    = CombatStats::ApplyResistanceLimit(passiveContribution.resistIce    + m_itemContribution.resistIce + codexResist);
		m_combatStats.resistElec   = CombatStats::ApplyResistanceLimit(passiveContribution.resistElec   + m_itemContribution.resistElec + codexResist);
		m_combatStats.resistPoison = CombatStats::ApplyResistanceLimit(passiveContribution.resistPoison + m_itemContribution.resistPoison + codexResist);
		m_combatStats.resistSpirit = CombatStats::ApplyResistanceLimit(passiveContribution.resistSpirit + m_itemContribution.resistSpirit + codexResist);
		// Legacy LIMIT() applied: clamp negative to 0 (no upper bound)
	}

	// --- PIERCE ---
	// Legacy: passivePierce + skillPierce
	m_combatStats.pierce = passiveContribution.pierce;

	// --- TARGET RANGE ---
	// Legacy: passiveTargetRange + skillTargetRange
	m_combatStats.targetRange = passiveContribution.targetRange;

	// --- SKILL RANGES ---
	// Legacy: passive + skill
	m_combatStats.skillAttackRange = passiveContribution.skillAttackRange;
	m_combatStats.skillApplyRange  = passiveContribution.skillApplyRange;

	// --- VELOCITY ---
	// Legacy: move = passiveMove + stateMove + pet/land
	// Legacy: attack = passiveAttack + stateAttack + pet/land
	m_combatStats.moveVelocity  = passiveContribution.moveVelocity;
	m_combatStats.attackVelocity = passiveContribution.attackVelocity;

	// --- SKILL DELAY ---
	// Legacy: delay = passiveDelay + stateDelay + skill/quest/pet
	m_combatStats.skillDelay = passiveContribution.skillDelay;

	// --- DAMAGE SPEC ---
	// Legacy: passiveDamageSpec copied, then skills can max-increase values
	m_combatStats.damageSpec = passiveContribution.damageSpec;

	// TODO(verification): Integrate skill/buff/pet/land effects
	// TODO(verification): Instance custom values for PA/SA/MA (GETADDMA)
	// TODO(verification): Additional random options
	// TODO(verification): Charm handling
}

	// Equipment methods
	bool Character::EquipItem(EquipSlot slot, uint32_t itemId)
	{
		if (static_cast<size_t>(slot) >= EquipSlotCount || slot == EquipSlot::Invalid)
			return false;

		if (itemId == 0)
			return false;

		if (!m_itemDataProvider)
			return false;

		const ItemBaseData* itemData = m_itemDataProvider->GetItemData(itemId);
		if (!itemData)
			return false;

		if (!itemData->CanEquipInSlot(slot))
			return false;

		// Check requirements
		if (m_level < itemData->reqLevelMin || m_level > itemData->reqLevelMax)
			return false;

		// Unequip current item in slot if any
		size_t idx = static_cast<size_t>(slot);
		if (m_equippedItems[idx].itemId != 0)
		{
			UnequipItem(slot);
		}

		// Equip new item
		m_equippedItems[idx] = ItemInstanceData(itemId);
		RecalculateStats();
		return true;
	}

	void Character::UnequipItem(EquipSlot slot)
	{
		if (static_cast<size_t>(slot) >= EquipSlotCount || slot == EquipSlot::Invalid)
			return;

		m_equippedItems[static_cast<size_t>(slot)] = ItemInstanceData();
		RecalculateStats();
	}

	const ItemInstanceData* Character::GetEquippedItem(EquipSlot slot) const
	{
		if (static_cast<size_t>(slot) >= EquipSlotCount || slot == EquipSlot::Invalid)
			return nullptr;

		const ItemInstanceData& inst = m_equippedItems[static_cast<size_t>(slot)];
		if (inst.itemId == 0)
			return nullptr;
		return &inst;
	}

	bool Character::IsSlotValid(EquipSlot slot) const
	{
		return static_cast<size_t>(slot) < EquipSlotCount && slot != EquipSlot::Invalid;
	}

	void Character::SetMaxHP(uint32_t max)
	{
		m_hpMax = max;
		if (m_hpNow > m_hpMax) m_hpNow = m_hpMax;
	}

	void Character::SetMaxMP(uint32_t max)
	{
		m_mpMax = max;
		if (m_mpNow > m_mpMax) m_mpNow = m_mpMax;
	}

	void Character::SetMaxSP(uint32_t max)
	{
		m_spMax = max;
		if (m_spNow > m_spMax) m_spNow = m_spMax;
	}

	void Character::SetHP(uint32_t now)
	{
		m_hpNow = std::min(now, m_hpMax);
	}

	void Character::SetMP(uint32_t now)
	{
		m_mpNow = std::min(now, m_mpMax);
	}

	void Character::SetSP(uint32_t now)
	{
		m_spNow = std::min(now, m_spMax);
	}

	void Character::DamageHP(uint32_t amount)
	{
		if (IsDead()) return;
		if (amount >= m_hpNow)
		{
			m_hpNow = 0;
			DieInternal();
		}
		else
		{
			m_hpNow -= amount;
		}
	}

	void Character::HealHP(uint32_t amount)
	{
		if (m_hpNow + amount < m_hpNow) return;
		m_hpNow = std::min(m_hpMax, m_hpNow + amount);
	}

	void Character::ConsumeMP(uint32_t amount)
	{
		if (amount >= m_mpNow)
			m_mpNow = 0;
		else
			m_mpNow -= amount;
	}

	void Character::ConsumeSP(uint32_t amount)
	{
		if (amount >= m_spNow)
			m_spNow = 0;
		else
			m_spNow -= amount;
	}

	void Character::RecoverMP(uint32_t amount)
	{
		if (m_mpNow + amount < m_mpNow) return;
		m_mpNow = std::min(m_mpMax, m_mpNow + amount);
	}

	void Character::RecoverSP(uint32_t amount)
	{
		if (m_spNow + amount < m_spNow) return;
		m_spNow = std::min(m_spMax, m_spNow + amount);
	}

	void Character::SetRecoveryRates(float hpPerSec, float mpPerSec, float spPerSec)
	{
		m_hpRecoverPerSec = hpPerSec;
		m_mpRecoverPerSec = mpPerSec;
		m_spRecoverPerSec = spPerSec;
	}

	void Character::RecoverAll(float elapsedSeconds)
	{
		if (IsDead()) return;
		if (m_hpMax > 0)
			m_hpNow = std::min(m_hpMax, static_cast<uint32_t>(m_hpNow + m_hpMax * m_hpRecoverPerSec * elapsedSeconds));
		if (m_mpMax > 0)
			m_mpNow = std::min(m_mpMax, static_cast<uint32_t>(m_mpNow + m_mpMax * m_mpRecoverPerSec * elapsedSeconds));
		if (m_spMax > 0)
			m_spNow = std::min(m_spMax, static_cast<uint32_t>(m_spNow + m_spMax * m_spRecoverPerSec * elapsedSeconds));
	}

	void Character::DieInternal()
	{
		m_moving = false;
		m_action = ActionType::Die;
		m_actState |= ActState::Dead;
	}

	void Character::Die()
	{
		if (IsDead()) return;
		m_hpNow = 0;
		DieInternal();
	}

	void Character::Revive(const Vector3& position, const Vector3& direction)
	{
		m_actState &= ~ActState::Dead;
		m_action = ActionType::Idle;
		m_entity.position = position;
		m_entity.direction = Normalize(direction);
		m_moving = false;
		m_hpNow = m_hpMax;
		m_mpNow = m_mpMax;
		m_spNow = m_spMax;
	}

	void Character::MoveTo(const Vector3& target)
	{
		if (IsDead()) return;
		m_moveTarget = target;
		const Vector3 delta = target - m_entity.position;
		if (delta.IsZero())
		{
			m_moving = false;
			m_action = ActionType::Idle;
			return;
		}
		m_entity.direction = Normalize(delta);
		m_moving = true;
		m_action = ActionType::Move;
	}

	void Character::Walk()
	{
		RemoveActState(ActState::Run);
		if (m_moving) m_action = ActionType::Move;
	}

	void Character::Run()
	{
		AddActState(ActState::Run);
		if (m_moving) m_action = ActionType::Move;
	}

	void Character::Stop()
	{
		m_moving = false;
		m_action = ActionType::Idle;
	}

	void Character::TurnTo(const Vector3& direction)
	{
		if (direction.IsZero()) return;
		m_entity.direction = Normalize(direction);
		if (!m_moving) m_action = ActionType::Idle;
		else m_action = ActionType::Move;
	}

	void Character::AddExperience(int64_t amount)
	{
		if (amount < 0) amount = 0;
		m_expNow += amount;
		if (m_level >= m_maxLevel)
		{
			RefreshExpMax();
			return;
		}
		while (m_level < m_maxLevel)
		{
			int64_t need = m_progression ? m_progression->GetRequiredExperience(m_level) : 0;
			if (need <= 0) break;
			if (m_expNow < need) break;
			m_expNow -= need;
			++m_level;
			m_hpNow = m_hpMax;
			m_mpNow = m_mpMax;
			m_spNow = m_spMax;
			RefreshExpMax();
			ApplyLevelUpStats();
		}
	}

	float Character::CurrentSpeed() const
	{
		return (m_actState & ActState::Run) ? RunSpeed : WalkSpeed;
	}

	bool Character::IsAlive() const
	{
		if (IsDeadAction(m_action))
			return false;
		if ((m_actState & ActState::Dead) != 0)
			return false;
		return true;
	}

	bool Character::IsDead() const
	{
		return !IsAlive();
	}

	void Character::Update(float elapsedSeconds)
	{
		if (IsDead()) return;
		if (m_moving)
		{
			const Vector3 delta = m_moveTarget - m_entity.position;
			const float distance = delta.Length();
			const float step = CurrentSpeed() * elapsedSeconds;
			if (distance <= step || distance == 0.0f)
			{
				m_entity.position = m_moveTarget;
				Stop();
			}
			else
			{
				m_entity.position += delta * (step / distance);
			}
		}
		RecoverAll(elapsedSeconds);
	}

	CharacterInfo Character::Inspect() const
	{
		CharacterInfo info;
		info.id          = m_entity.id;
		info.name        = m_nameString;
		info.classId     = m_classId;
		info.gender      = m_gender;
		info.school      = m_school;
		info.level       = m_level;
		info.expNow      = m_expNow;
		info.expMax      = m_expMax;
		info.hpNow       = m_hpNow;
		info.hpMax       = m_hpMax;
		info.mpNow       = m_mpNow;
		info.mpMax       = m_mpMax;
		info.spNow       = m_spNow;
		info.spMax       = m_spMax;
		info.position    = m_entity.position;
		info.direction   = m_entity.direction;
		info.action      = m_action;
		info.actState    = m_actState;
		return info;
	}

	bool CharacterInfo::IsAlive() const
	{
		if (IsDeadAction(action))
			return false;
		if ((actState & ActState::Dead) != 0)
			return false;
		return true;
	}

	bool CharacterInfo::IsDead() const
	{
		return !IsAlive();
	}
}