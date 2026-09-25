#include "Character.h"
#include "../Math/Vector3.h"
#include "../Progression/ProgressionData.h"
#include "CharacterBaseData.h"
#include "../Item/ItemData.h"

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

	void Character::SetProgressionData(const ProgressionData* data)
	{
		m_progression = data;
		RefreshExpMax();
	}

	void Character::SetBaseDataProvider(const ICharacterBaseDataProvider* provider)
	{
		m_baseDataProvider = provider;
		RefreshBaseData();
	}

	void Character::SetItemDataProvider(const IItemDataProvider* provider)
	{
		m_itemDataProvider = provider;
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

		// Calculate current stats based on level
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

		// Add item stat contributions to current stats (legacy behavior: stats include item bonuses before HP/MP/SP calc)
		m_currentStats.pow  += m_itemContribution.pow;
		m_currentStats.str  += m_itemContribution.str;
		m_currentStats.spi  += m_itemContribution.spi;
		m_currentStats.dex  += m_itemContribution.dex;
		m_currentStats.intel += m_itemContribution.intel;
		m_currentStats.sta  += m_itemContribution.sta;

		// Calculate max resources from stats + class factors + item contributions
		// HP = (STR * fHP_STR + itemHP) * (1 + itemHPRate) + itemHPVolume
		// Note: passive skills and conft rates handled elsewhere if needed
		float strFactor = m_baseData->hpStrFactor;
		float spiFactor = m_baseData->mpSpiFactor;
		float staFactor = m_baseData->spStaFactor;

		float baseHP = m_currentStats.str * strFactor;
		float baseMP = m_currentStats.spi * spiFactor;
		float baseSP = m_currentStats.sta * staFactor;

		// Add flat item HP/MP/SP
		baseHP += m_itemContribution.hp;
		baseMP += m_itemContribution.mp;
		baseSP += m_itemContribution.sp;

		// Apply rate multipliers (1 + rate)
		baseHP *= (1.0f + m_itemContribution.hpRate);
		baseMP *= (1.0f + m_itemContribution.mpRate);
		baseSP *= (1.0f + m_itemContribution.spRate);

		// Add volume (flat)
		baseHP += m_itemContribution.hpVolume;
		baseMP += m_itemContribution.mpVolume;
		baseSP += m_itemContribution.spVolume;

		// Apply class base resources as minimum (removed - legacy formula doesn't clamp to class base)
		// The class baseHP/baseMP/baseSP are naturally produced by the formula at level 1 with no items

		m_hpMax = static_cast<uint32_t>(baseHP);
		m_mpMax = static_cast<uint32_t>(baseMP);
		m_spMax = static_cast<uint32_t>(baseSP);

		// Clamp current values to new max
		if (m_hpNow > m_hpMax) m_hpNow = m_hpMax;
		if (m_mpNow > m_mpMax) m_mpNow = m_mpMax;
		if (m_spNow > m_spMax) m_spNow = m_spMax;
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