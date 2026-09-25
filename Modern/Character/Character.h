#pragma once

#include "../Core/Types.h"
#include "../Math/Vector3.h"
#include "../Entity/Entity.h"
#include "../Progression/ProgressionData.h"
#include "../Item/ItemData.h"
#include "CharacterBaseData.h"

#include <cstdint>
#include <string>
#include <array>

namespace Modern
{
	constexpr uint16_t kDefaultMaxLevel = 255;
	constexpr size_t   kExpTableCapacity = 300;

	enum class Gender : uint8_t
	{
		Female = 0,
		Male   = 1,
	};

	struct CharacterInfo
	{
		EntityId    id        = EntityId::MakeInvalid();
		std::string name;
		uint32_t    classId   = 0;
		Gender      gender    = Gender::Female;
		uint16_t    school    = 0;
		uint16_t    level     = 1;
		int64_t     expNow    = 0;
		int64_t     expMax    = 0;
		uint32_t    hpNow     = 0;
		uint32_t    hpMax     = 0;
		uint32_t    mpNow     = 0;
		uint32_t    mpMax     = 0;
		uint32_t    spNow     = 0;
		uint32_t    spMax     = 0;
		Vector3     position;
		Vector3     direction;
		ActionType  action    = ActionType::Idle;
		uint32_t    actState  = ActState::None;

		bool IsAlive() const;
		bool IsDead() const;
	};

	class Character
	{
	public:
		static constexpr float WalkSpeed    = 8.0f;
		static constexpr float RunSpeed     = 14.0f;
		static constexpr size_t NameCapacity = 32;
		static constexpr size_t EquipSlotCount = 21;  // SLOT_NSIZE_S_2

		static constexpr float kDefaultHpRecoverPerSec = 0.003f;
		static constexpr float kDefaultMpRecoverPerSec = 0.003f;
		static constexpr float kDefaultSpRecoverPerSec = 0.005f;

		Character() = default;
		Character(EntityId id, const std::string& name);

		void Spawn(const Vector3& position, const Vector3& direction);
		void Despawn();

		void MoveTo(const Vector3& target);
		void Walk();
		void Run();
		void Stop();
		void TurnTo(const Vector3& direction);

		void Die();
		void Revive(const Vector3& position, const Vector3& direction);

		void Update(float elapsedSeconds);

		void SetName(const std::string& name);
		void SetClass(uint32_t classId, Gender gender);
		void SetSchool(uint16_t school);
		void SetLevel(uint16_t level);

		void SetMaxHP(uint32_t max);
		void SetMaxMP(uint32_t max);
		void SetMaxSP(uint32_t max);
		void SetHP(uint32_t now);
		void SetMP(uint32_t now);
		void SetSP(uint32_t now);

		void DamageHP(uint32_t amount);
		void HealHP(uint32_t amount);
		void ConsumeMP(uint32_t amount);
		void ConsumeSP(uint32_t amount);
		void RecoverMP(uint32_t amount);
		void RecoverSP(uint32_t amount);

		void SetRecoveryRates(float hpPerSec, float mpPerSec, float spPerSec);
		void RecoverAll(float elapsedSeconds);

		void SetProgressionData(const ProgressionData* data);
		void SetBaseDataProvider(const ICharacterBaseDataProvider* provider);
		void SetItemDataProvider(const IItemDataProvider* provider);
		void SetMaxLevel(uint16_t maxLevel) { m_maxLevel = maxLevel; }
		void AddExperience(int64_t amount);

		// Equipment
		bool EquipItem(EquipSlot slot, uint32_t itemId);
		void UnequipItem(EquipSlot slot);
		const ItemInstanceData* GetEquippedItem(EquipSlot slot) const;
		bool IsSlotValid(EquipSlot slot) const;

		void SetPosition(const Vector3& position);
		void SetDirection(const Vector3& direction);
		void SetAction(ActionType action);
		void SetActState(uint32_t state) { m_actState = state; }
		void AddActState(uint32_t state) { m_actState |= state; }
		void RemoveActState(uint32_t state) { m_actState &= ~state; }

		EntityId        GetId() const { return m_entity.id; }
		const std::string& GetName() const { return m_nameString; }
		const Vector3&  GetPosition() const { return m_entity.position; }
		const Vector3&  GetDirection() const { return m_entity.direction; }
		ActionType      GetAction() const { return m_action; }
		uint32_t        GetActState() const { return m_actState; }
		bool            IsActState(uint32_t state) const { return (m_actState & state) != 0; }
		uint16_t        GetLevel() const { return m_level; }
		uint16_t        GetSchool() const { return m_school; }
		uint32_t        GetClassId() const { return m_classId; }
		Gender          GetGender() const { return m_gender; }
		uint16_t        GetMaxLevel() const { return m_maxLevel; }

		uint32_t GetHP() const { return m_hpNow; }
		uint32_t GetMP() const { return m_mpNow; }
		uint32_t GetSP() const { return m_spNow; }
		uint32_t GetMaxHP() const { return m_hpMax; }
		uint32_t GetMaxMP() const { return m_mpMax; }
		uint32_t GetMaxSP() const { return m_spMax; }

		int64_t GetExp() const { return m_expNow; }
		int64_t GetExpMax() const { return m_expMax; }

		float CurrentSpeed() const;
		bool IsMoving() const { return m_moving; }
		bool IsAlive() const;
		bool IsDead() const;

		CharacterInfo Inspect() const;

	private:
		void DieInternal();
		void RefreshExpMax();
		void RefreshBaseData();
		void ApplyLevelUpStats();
		void RecalculateStats();  // Recalculate including item contributions

		Entity                              m_entity;
		std::string                         m_nameString;
		char                                m_name[NameCapacity + 1] = {};
		uint32_t                            m_classId = 0;
		Gender                              m_gender  = Gender::Female;
		uint16_t                            m_school  = 0;
		uint16_t                            m_level   = 1;
		uint16_t                            m_maxLevel = kDefaultMaxLevel;
		int64_t                             m_expNow  = 0;
		int64_t                             m_expMax  = 0;
		const ProgressionData*              m_progression = nullptr;
		const ICharacterBaseDataProvider*   m_baseDataProvider = nullptr;
		const IItemDataProvider*            m_itemDataProvider = nullptr;
		const CharacterBaseData*            m_baseData = nullptr;
		CharacterStats                      m_currentStats;

		// Equipment state
		std::array<ItemInstanceData, EquipSlotCount> m_equippedItems;
		ItemContribution                    m_itemContribution;

		uint32_t                            m_hpNow = 0;
		uint32_t                            m_hpMax = 0;
		uint32_t                            m_mpNow = 0;
		uint32_t                            m_mpMax = 0;
		uint32_t                            m_spNow = 0;
		uint32_t                            m_spMax = 0;

		float                               m_hpRecoverPerSec = kDefaultHpRecoverPerSec;
		float                               m_mpRecoverPerSec = kDefaultMpRecoverPerSec;
		float                               m_spRecoverPerSec = kDefaultSpRecoverPerSec;

		ActionType                          m_action  = ActionType::Idle;
		uint32_t                            m_actState = ActState::None;
		bool                                m_moving  = false;
		Vector3                             m_moveTarget;
	};
}