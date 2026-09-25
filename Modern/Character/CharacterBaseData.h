#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Modern
{
	// EMCHARINDEX values matching RAN's GLCharDefine.h
	enum class CharIndex : uint32_t
	{
		BrawlerM        = 0,
		SwordsmanM      = 1,
		ArcherW         = 2,
		ShamanW         = 3,
		ExtremeM        = 4,
		ExtremeW        = 5,
		BrawlerW        = 6,
		SwordsmanW      = 7,
		ArcherM         = 8,
		ShamanM         = 9,
		GunnerM         = 10,
		GunnerW         = 11,
		AssassinM       = 12,
		AssassinW       = 13,
		TrickerM        = 14,
		TrickerW        = 15,
		Count           = 16
	};

	// Forward declarations
	struct CharacterStats;
	struct CharacterBaseData;

	// Minimal stats structure matching RAN's 6 stats
	struct CharacterStats
	{
		uint16_t pow = 0;  // Power
		uint16_t str = 0;  // Strength
		uint16_t spi = 0;  // Spirit
		uint16_t dex = 0;  // Dexterity
		uint16_t intel = 0; // Intelligence
		uint16_t sta = 0;  // Stamina

		CharacterStats() = default;
		CharacterStats(uint16_t p, uint16_t s, uint16_t sp, uint16_t d, uint16_t i, uint16_t st)
			: pow(p), str(s), spi(sp), dex(d), intel(i), sta(st) {}
	};

	// Level-up stats (floating point per RAN's FCHARSTATS)
	struct CharacterLevelUpStats
	{
		float pow = 0.0f;
		float str = 0.0f;
		float spi = 0.0f;
		float dex = 0.0f;
		float intel = 0.0f;
		float sta = 0.0f;

		CharacterLevelUpStats() = default;
		CharacterLevelUpStats(float p, float s, float sp, float d, float i, float st)
			: pow(p), str(s), spi(sp), dex(d), intel(i), sta(st) {}
	};

	// Portable character class base data
	// Derived from RAN's GLCONST_CHARCLASS and class init files
	struct CharacterBaseData
	{
		uint32_t classId = 0;        // EMCHARINDEX value (0-13)
		uint16_t schoolId = 0;       // 0, 1, 2
		std::string className;       // e.g., "Swordsman"
		std::string schoolName;      // e.g., "Saint Morning"

		// Stat-to-resource conversion factors (from GLCONST_CHARCLASS)
		float hpStrFactor = 0.0f;    // fHP_STR: STR -> HP
		float mpSpiFactor = 0.0f;    // fMP_SPI: SPI -> MP
		float spStaFactor = 0.0f;    // fSP_STA: STA -> SP

		// Base stats at level 1 (from sBEGIN_STATS)
		CharacterStats baseStats;

		// Stats gained per level (from sLVLUP_STATS)
		CharacterLevelUpStats levelUpStats;

		// Initial resources (calculated from base stats + class factors)
		uint32_t baseHP = 0;
		uint32_t baseMP = 0;
		uint32_t baseSP = 0;

		// Movement
		float walkSpeed = 8.0f;
		float runSpeed = 14.0f;

		// Max level for this class/school
		uint16_t maxLevel = 255;

		// Recovery rates (global, can be overridden)
		float hpRecoverPerSec = 0.003f;
		float mpRecoverPerSec = 0.003f;
		float spRecoverPerSec = 0.005f;

		bool IsValid() const { return classId != 0xFFFFFFFF; }
	};

	// Provider interface for character base data
	class ICharacterBaseDataProvider
	{
	public:
		virtual ~ICharacterBaseDataProvider() = default;

		virtual const CharacterBaseData* GetBaseData(uint32_t classId, uint16_t schoolId) const = 0;
		virtual const CharacterBaseData* GetBaseDataByClassIndex(uint32_t charIndex) const = 0;
		virtual bool HasClass(uint32_t classId, uint16_t schoolId) const = 0;
		virtual uint16_t GetMaxLevel() const = 0;
	};

	// Test provider with deterministic values
	class TestCharacterBaseDataProvider : public ICharacterBaseDataProvider
	{
	public:
		TestCharacterBaseDataProvider();

		const CharacterBaseData* GetBaseData(uint32_t classId, uint16_t schoolId) const override;
		const CharacterBaseData* GetBaseDataByClassIndex(uint32_t charIndex) const override;
		bool HasClass(uint32_t classId, uint16_t schoolId) const override;
		uint16_t GetMaxLevel() const override { return m_maxLevel; }

		void SetMaxLevel(uint16_t level) { m_maxLevel = level; }

	private:
		uint16_t m_maxLevel = 255;
		std::vector<CharacterBaseData> m_classData;
	};

	// RAN provider loads from portable text exports
	// (extracted from legacy default.charclass + class*.classconst + class*.charset files)
	class RANCharacterBaseDataProvider : public ICharacterBaseDataProvider
	{
	public:
		RANCharacterBaseDataProvider() = default;

		bool LoadFromDirectory(const char* dataDir);

		const CharacterBaseData* GetBaseData(uint32_t classId, uint16_t schoolId) const override;
		const CharacterBaseData* GetBaseDataByClassIndex(uint32_t charIndex) const override;
		bool HasClass(uint32_t classId, uint16_t schoolId) const override;
		uint16_t GetMaxLevel() const override { return m_maxLevel; }

		void SetMaxLevel(uint16_t level) { m_maxLevel = level; }

	private:
		uint16_t m_maxLevel = 255;
		std::vector<CharacterBaseData> m_classData; // indexed by EMCHARINDEX (0-13)
	};

	// Helper: calculate base resources from stats and class factors
	inline void CalculateBaseResources(const CharacterBaseData& baseData,
		const CharacterStats& stats,
		uint32_t& outHP, uint32_t& outMP, uint32_t& outSP)
	{
		// Matches RAN's UPDATE_MAX_POINT base formula (before item/passive/conft multipliers)
		// HP = STR * fHP_STR, MP = SPI * fMP_SPI, SP = STA * fSP_STA
		outHP = static_cast<uint32_t>(stats.str * baseData.hpStrFactor);
		outMP = static_cast<uint32_t>(stats.spi * baseData.mpSpiFactor);
		outSP = static_cast<uint32_t>(stats.sta * baseData.spStaFactor);

		// Apply class base resources if set
		if (baseData.baseHP > outHP) outHP = baseData.baseHP;
		if (baseData.baseMP > outMP) outMP = baseData.baseMP;
		if (baseData.baseSP > outSP) outSP = baseData.baseSP;
	}

	// Helper: calculate stats at a given level
	inline CharacterStats CalculateStatsAtLevel(const CharacterBaseData& baseData, uint16_t level)
	{
		CharacterStats stats = baseData.baseStats;
		if (level > 1)
		{
			uint16_t levelsGained = level - 1;
			stats.pow  = static_cast<uint16_t>(stats.pow  + baseData.levelUpStats.pow  * levelsGained);
			stats.str  = static_cast<uint16_t>(stats.str  + baseData.levelUpStats.str  * levelsGained);
			stats.spi  = static_cast<uint16_t>(stats.spi  + baseData.levelUpStats.spi  * levelsGained);
			stats.dex  = static_cast<uint16_t>(stats.dex  + baseData.levelUpStats.dex  * levelsGained);
			stats.intel = static_cast<uint16_t>(stats.intel + baseData.levelUpStats.intel * levelsGained);
			stats.sta  = static_cast<uint16_t>(stats.sta  + baseData.levelUpStats.sta  * levelsGained);
		}
		return stats;
	}
}