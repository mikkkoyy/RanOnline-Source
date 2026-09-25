#pragma once

#include <cstdint>
#include <vector>

namespace Modern
{
	struct ProgressionData
	{
		virtual ~ProgressionData() = default;

		virtual int64_t GetRequiredExperience(uint16_t level) const = 0;
		virtual uint16_t GetMaxLevel() const = 0;
		virtual size_t GetLevelCount() const = 0;
		virtual const int64_t* GetTableData() const = 0;

		bool IsValidLevel(uint16_t level) const
		{
			return level < GetMaxLevel();
		}
	};

	// Deterministic test progression: 100 * level (matches Phase 2 test table)
	struct TestProgressionData : ProgressionData
	{
		static constexpr uint16_t kDefaultMaxLevel = 255;

		TestProgressionData(uint16_t maxLevel = kDefaultMaxLevel)
			: m_maxLevel(maxLevel)
		{
			m_table.resize(maxLevel);
			for (uint16_t i = 0; i < maxLevel; ++i)
				m_table[i] = static_cast<int64_t>(100) * static_cast<int64_t>(i);
		}

		int64_t GetRequiredExperience(uint16_t level) const override
		{
			if (level >= m_maxLevel) return 0;
			return m_table[level];
		}

		uint16_t GetMaxLevel() const override { return m_maxLevel; }
		size_t GetLevelCount() const override { return m_table.size(); }
		const int64_t* GetTableData() const override { return m_table.data(); }

	private:
		uint16_t m_maxLevel = kDefaultMaxLevel;
		std::vector<int64_t> m_table;
	};

	// RAN progression data loaded from plain-text format (one int64 per line, level-indexed)
	// Matches the legacy EXP_MAX_LOADFILE contract: line i = lnEXP_MAX_TABLE[i] =
	// EXP required to go from level i to i+1. First line (index 0) is placeholder.
	// Real data source: exptable_max.bin decoded via legacy pack loader.
	struct RANProgressionData : ProgressionData
	{
		RANProgressionData() = default;
		explicit RANProgressionData(const std::vector<int64_t>& table, uint16_t maxLevel = 255)
			: m_table(table), m_maxLevel(maxLevel) {}

		bool LoadFromTextFile(const char* filePath);
		bool LoadFromText(const char* text, size_t length);

		int64_t GetRequiredExperience(uint16_t level) const override
		{
			if (level >= m_maxLevel) return 0;
			if (level >= m_table.size()) return 0;
			return m_table[level];
		}

		uint16_t GetMaxLevel() const override { return m_maxLevel; }
		size_t GetLevelCount() const override { return m_table.size(); }
		const int64_t* GetTableData() const override { return m_table.data(); }

		void SetMaxLevel(uint16_t maxLevel) { m_maxLevel = maxLevel; }

	private:
		std::vector<int64_t> m_table;
		uint16_t m_maxLevel = 255;
	};
}