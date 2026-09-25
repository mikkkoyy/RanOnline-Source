#include "CharacterBaseData.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>

namespace Modern
{
	// EMCHARINDEX mapping (from GLCharDefine.h)
	enum EMCHARINDEX
	{
		GLCI_BRAWLER_M		= 0,
		GLCI_SWORDSMAN_M	= 1,
		GLCI_ARCHER_W		= 2,
		GLCI_SHAMAN_W		= 3,
		GLCI_EXTREME_M		= 4,
		GLCI_EXTREME_W		= 5,
		GLCI_BRAWLER_W		= 6,
		GLCI_SWORDSMAN_W	= 7,
		GLCI_ARCHER_M		= 8,
		GLCI_SHAMAN_M		= 9,
		GLCI_GUNNER_M		= 10,
		GLCI_GUNNER_W		= 11,
		GLCI_ASSASSIN_M		= 12,
		GLCI_ASSASSIN_W		= 13,
		GLCI_TRICKER_M		= 14,
		GLCI_TRICKER_W		= 15,
		GLCI_NUM_8CLASS		= 16
	};

	TestCharacterBaseDataProvider::TestCharacterBaseDataProvider()
	{
		m_classData.resize(GLCI_NUM_8CLASS);

		// Populate with deterministic test values
		// Class 1: Swordsman Male (index 1)
		{
			CharacterBaseData& d = m_classData[GLCI_SWORDSMAN_M];
			d.classId = GLCI_SWORDSMAN_M;
			d.schoolId = 0;
			d.className = "Swordsman";
			d.schoolName = "Saint Morning";
			d.hpStrFactor = 5.0f;
			d.mpSpiFactor = 3.0f;
			d.spStaFactor = 4.0f;
			d.baseStats = CharacterStats(10, 15, 10, 12, 10, 12);
			d.levelUpStats = CharacterLevelUpStats(0.2f, 0.5f, 0.3f, 0.4f, 0.2f, 0.4f);
			d.baseHP = 100;
			d.baseMP = 50;
			d.baseSP = 30;
			d.walkSpeed = 8.0f;
			d.runSpeed = 14.0f;
			d.maxLevel = m_maxLevel;
		}

		// Class 7: Swordsman Female (index 7)
		{
			CharacterBaseData& d = m_classData[GLCI_SWORDSMAN_W];
			d.classId = GLCI_SWORDSMAN_W;
			d.schoolId = 0;
			d.className = "Swordsman";
			d.schoolName = "Saint Morning";
			d.hpStrFactor = 4.5f;
			d.mpSpiFactor = 3.5f;
			d.spStaFactor = 4.0f;
			d.baseStats = CharacterStats(10, 13, 12, 14, 10, 12);
			d.levelUpStats = CharacterLevelUpStats(0.2f, 0.4f, 0.4f, 0.5f, 0.2f, 0.4f);
			d.baseHP = 90;
			d.baseMP = 60;
			d.baseSP = 30;
			d.walkSpeed = 8.0f;
			d.runSpeed = 14.0f;
			d.maxLevel = m_maxLevel;
		}

		// Class 0: Brawler Male (index 0)
		{
			CharacterBaseData& d = m_classData[GLCI_BRAWLER_M];
			d.classId = GLCI_BRAWLER_M;
			d.schoolId = 0;
			d.className = "Brawler";
			d.schoolName = "Saint Morning";
			d.hpStrFactor = 6.0f;
			d.mpSpiFactor = 2.0f;
			d.spStaFactor = 5.0f;
			d.baseStats = CharacterStats(12, 18, 8, 10, 8, 14);
			d.levelUpStats = CharacterLevelUpStats(0.3f, 0.6f, 0.2f, 0.3f, 0.1f, 0.5f);
			d.baseHP = 120;
			d.baseMP = 30;
			d.baseSP = 40;
			d.walkSpeed = 8.0f;
			d.runSpeed = 14.0f;
			d.maxLevel = m_maxLevel;
		}

		// Fill remaining with defaults
		for (size_t i = 0; i < m_classData.size(); ++i)
		{
			if (m_classData[i].className.empty())
			{
				CharacterBaseData& d = m_classData[i];
				d.classId = static_cast<uint32_t>(i);
				d.schoolId = 0;
				d.className = "Class" + std::to_string(i);
				d.schoolName = "Saint Morning";
				d.hpStrFactor = 4.0f;
				d.mpSpiFactor = 3.0f;
				d.spStaFactor = 3.0f;
				d.baseStats = CharacterStats(10, 12, 10, 10, 10, 10);
				d.levelUpStats = CharacterLevelUpStats(0.2f, 0.3f, 0.3f, 0.3f, 0.2f, 0.3f);
				d.baseHP = 80;
				d.baseMP = 40;
				d.baseSP = 30;
				d.walkSpeed = 8.0f;
				d.runSpeed = 14.0f;
				d.maxLevel = m_maxLevel;
			}
		}
	}

	const CharacterBaseData* TestCharacterBaseDataProvider::GetBaseData(uint32_t classId, uint16_t schoolId) const
	{
		if (classId < m_classData.size() && m_classData[classId].schoolId == schoolId)
			return &m_classData[classId];
		return nullptr;
	}

	const CharacterBaseData* TestCharacterBaseDataProvider::GetBaseDataByClassIndex(uint32_t charIndex) const
	{
		if (charIndex < m_classData.size())
			return &m_classData[charIndex];
		return nullptr;
	}

	bool TestCharacterBaseDataProvider::HasClass(uint32_t classId, uint16_t schoolId) const
	{
		return classId < m_classData.size() && m_classData[classId].schoolId == schoolId;
	}

	// RAN provider implementation
	bool RANCharacterBaseDataProvider::LoadFromDirectory(const char* dataDir)
	{
		// TODO: Implement loading from portable text exports
		// Expected files:
		// - charclass.txt (global constants + school info)
		// - class_<index>.txt (per EMCHARINDEX: factors, base stats, levelup stats)
		// - classinit_<school>_<class>.txt (per school/class: initial HP/MP/SP, position, etc.)
		// For now, return false to indicate real data not available
		return false;
	}

	const CharacterBaseData* RANCharacterBaseDataProvider::GetBaseData(uint32_t classId, uint16_t schoolId) const
	{
		if (classId < m_classData.size() && m_classData[classId].schoolId == schoolId)
			return &m_classData[classId];
		return nullptr;
	}

	const CharacterBaseData* RANCharacterBaseDataProvider::GetBaseDataByClassIndex(uint32_t charIndex) const
	{
		if (charIndex < m_classData.size())
			return &m_classData[charIndex];
		return nullptr;
	}

	bool RANCharacterBaseDataProvider::HasClass(uint32_t classId, uint16_t schoolId) const
	{
		return classId < m_classData.size() && m_classData[classId].schoolId == schoolId;
	}
}