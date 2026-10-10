#include "stats/ClassConstantTable.h"

#include <cmath>

#include <string>

namespace Modern::Stats
{
	const char* ToString(CoefficientSource source) noexcept
	{
		switch (source)
		{
		case CoefficientSource::Unavailable: return "Unavailable";
		case CoefficientSource::Recovered:   return "Recovered";
		}
		return "Unrecognised";
	}

	namespace
	{
		// The reason a row carries no data. Shared, so the wording cannot drift.
		constexpr const char* kMissingDataNote =
		    "no authoritative class<N>.classconst row was recovered; do not read the "
		    "coefficients, and do not substitute GLogicData.cpp:609's cCONSTCLASS "
		    "constructor values (proven overwritten at startup, see "
		    "modern/core/movement/MovementSpeed.h:8-26).";

		// The deployed data file for one EMCHARINDEX row, named by the matching
		// `<CLASS>_<GENDER>.SETFILE` key in `default.charclass`. The placement of
		// this array was verified, not assumed: the order is the same as
		// `szSETFILE[]` (GLogicDataLoad.cpp:527-544), and every row's fWALKVELO /
		// fRUNVELO pair reproduces the independently recovered table in
		// modern/core/movement/MovementSpeed.cpp:31-48 row for row.
		constexpr const char* kClassConstFile[ClassConstantTable::kRowCount] = {
			"class0.classconst", //  0 BrawlerMale     BRAWLER_M.SETFILE
			"class1.classconst", //  1 SwordsmanMale   SWORDSMAN_M.SETFILE
			"class2.classconst", //  2 ArcherFemale    ARCHER_W.SETFILE
			"class3.classconst", //  3 ShamanFemale    SHAMAN_W.SETFILE
			"class4.classconst", //  4 ExtremeMale     EXTREME_M.SETFILE
			"class5.classconst", //  5 ExtremeFemale   EXTREME_W.SETFILE
			"class6.classconst", //  6 BrawlerFemale   BRAWLER_W.SETFILE
			"class7.classconst", //  7 SwordsmanFemale SWORDSMAN_W.SETFILE
			"class8.classconst", //  8 ArcherMale      ARCHER_M.SETFILE
			"class9.classconst", //  9 ShamanMale      SHAMAN_M.SETFILE
			"classA.classconst", // 10 GunnerMale      GUNNER_M.SETFILE
			"classB.classconst", // 11 GunnerFemale    GUNNER_W.SETFILE
			"classC.classconst", // 12 AssassinMale    ASSASSIN_M.SETFILE
			"classD.classconst", // 13 AssassinFemale  ASSASSIN_W.SETFILE
			"classE.classconst", // 14 TrickerMale     TRICKER_M.SETFILE
			"classF.classconst", // 15 TrickerFemale   TRICKER_W.SETFILE
		};

		// The provenance string every recovered row carries. Written at most
		// once per row and shared, because a claim repeated sixteen times can
		// drift between rows.
		const std::string BuildRecoveredNote(const char* fileName)
		{
			std::string note = fileName;
			note +=
			    " from the deployed ASURA CLIENT data/glogic; named by its SETFILE "
			    "key in default.charclass (GLogicDataLoad.cpp:557-570, :1131); "
			    "decrypted with the file's own Rijndael v8 key (Rijndael.cpp:943 "
			    "sm_Version[7], :979-986 version>=5 transform); fields read with "
			    "GLCONST_CHARCLASS::LOADFILE's getflag keys "
			    "(GLogicDataLoad.cpp:1169-1216).";
			return note;
		}

		// One deployed row, transcribed field for field from the getflag keys.
		//
		// The parameter order is the loader's own order so a transcription can be
		// diffed against GLogicDataLoad.cpp:1169-1216 line by line. Types are the
		// loader's: the four wBEGIN_* are WORD, everything else that is not a
		// stat is float, and the two stat blocks are six WORD and six float.
		//
		// `note` must outlive the table. A static std::string per row is the
		// simplest way to guarantee that without a leaking allocator, and the
		// table is written exactly once.
		struct DeployedRow
		{
			const char* file;

			// GLogicDataLoad.cpp:1172-1177
			float hpPerStr;
			float mpPerSpi;
			float spPerSta;
			float hitPerDex;
			float avoidPerDex;
			float defensePerDex;

			// GLogicDataLoad.cpp:1179-1187
			float meleePerPow;
			float meleePerDex;
			float shootPerPow;
			float shootPerDex;
			float magicPerDex;
			float magicPerSpi;
			float magicPerIntel;

			// GLogicDataLoad.cpp:1189-1192
			float attackPointConversion;
			float defensePointConversion;
			float meleePowerConversion;
			float shootPowerConversion;

			// GLogicDataLoad.cpp:1194-1197
			uint16_t beginAttackPoint;
			uint16_t beginDefensePoint;
			uint16_t beginMeleePower;
			uint16_t beginShootPower;

			// GLogicDataLoad.cpp:1199-1202
			float levelUpAttackPoint;
			float levelUpDefensePoint;
			float levelUpMeleePower;
			float levelUpShootPower;

			// GLogicDataLoad.cpp:1204-1209
			uint16_t beginPow, beginStr, beginSpi, beginDex, beginInt, beginSta;

			// GLogicDataLoad.cpp:1211-1216
			float upPow, upStr, upSpi, upDex, upInt, upSta;
		};

		// The sixteen deployed rows, in EMCHARINDEX order.
		//
		// fHIT_DEX and fAVOID_DEX are 0 in every deployed row - that is the data,
		// and it is load-bearing: in this build hit and avoid have NO dexterity
		// term, so they come entirely from equipment and passives. It is not an
		// omission, and every row repeats it deliberately so a reader cannot
		// mistake one row's zero for a typo.
		constexpr DeployedRow kDeployed[ClassConstantTable::kRowCount] = {
		{ "class0.classconst", 2.0f, 0.65f, 1.2f, 0.0f, 0.0f, 0.032f,
		  0.14f, 0.1f, 0.08f, 0.18f, 0.12f, 0.22f, 0.0f,
		  0.4f, 0.75f, 0.8f, 0.2f,
		  4, 5, 5, 3,
		  1.2f, 0.414f, 0.4f, 0.18f,
		  10, 34, 9, 10, 0, 9,
		  0.3f, 4.5f, 0.61f, 0.4f, 0.0f, 3.2f },

		{ "class1.classconst", 1.9f, 0.89f, 1.1f, 0.0f, 0.0f, 0.035f,
		  0.14f, 0.07f, 0.0f, 0.15f, 0.0f, 0.26f, 0.0f,
		  0.4f, 0.6f, 0.85f, 0.2f,
		  5, 5, 5, 2,
		  1.2f, 0.577f, 0.36f, 0.16f,
		  8, 38, 12, 10, 0, 8,
		  0.3f, 5.0f, 0.68f, 0.33f, 0.0f, 2.8f },

		{ "class2.classconst", 1.8f, 0.8f, 1.0f, 0.0f, 0.0f, 0.024f,
		  0.12f, 0.08f, 0.1f, 0.14f, 0.12f, 0.26f, 0.0f,
		  0.4f, 0.57f, 0.6f, 1.3f,
		  5, 6, 2, 4,
		  1.2f, 0.427f, 0.3f, 0.4f,
		  5, 34, 18, 12, 0, 7,
		  0.3f, 4.4f, 0.64f, 0.47f, 0.0f, 2.4f },

		{ "class3.classconst", 1.55f, 1.5f, 1.06f, 0.0f, 0.0f, 0.01f,
		  0.13f, 0.05f, 0.05f, 0.2f, 0.05f, 0.38f, 0.0f,
		  0.4f, 0.52f, 0.7f, 0.7f,
		  3, 4, 4, 4,
		  1.2f, 0.597f, 0.33f, 0.34f,
		  6, 32, 26, 10, 0, 6,
		  0.3f, 5.2f, 0.7f, 0.38f, 0.0f, 2.7f },

		{ "class4.classconst", 1.5f, 1.3f, 1.1f, 0.0f, 0.0f, 0.023f,
		  0.2f, 0.12f, 0.08f, 0.16f, 0.12f, 0.22f, 0.0f,
		  0.4f, 0.6f, 0.8f, 0.8f,
		  5, 5, 4, 3,
		  1.2f, 0.496f, 0.3f, 0.3f,
		  8, 35, 22, 11, 0, 8,
		  0.3f, 6.6f, 0.68f, 0.4f, 0.0f, 2.8f },

		{ "class5.classconst", 1.5f, 1.3f, 1.1f, 0.0f, 0.0f, 0.023f,
		  0.2f, 0.12f, 0.08f, 0.16f, 0.12f, 0.22f, 0.0f,
		  0.4f, 0.6f, 0.8f, 0.8f,
		  5, 5, 4, 3,
		  1.2f, 0.496f, 0.3f, 0.3f,
		  8, 35, 22, 11, 0, 8,
		  0.3f, 6.6f, 0.68f, 0.4f, 0.0f, 2.8f },

		{ "class6.classconst", 2.0f, 0.65f, 1.2f, 0.0f, 0.0f, 0.032f,
		  0.14f, 0.1f, 0.08f, 0.18f, 0.12f, 0.22f, 0.0f,
		  0.4f, 0.75f, 0.8f, 0.2f,
		  4, 5, 5, 3,
		  1.2f, 0.414f, 0.4f, 0.18f,
		  10, 34, 9, 10, 0, 9,
		  0.3f, 4.5f, 0.61f, 0.4f, 0.0f, 3.2f },

		{ "class7.classconst", 1.9f, 0.89f, 1.1f, 0.0f, 0.0f, 0.035f,
		  0.14f, 0.07f, 0.0f, 0.15f, 0.0f, 0.26f, 0.0f,
		  0.4f, 0.6f, 0.85f, 0.2f,
		  5, 5, 5, 2,
		  1.2f, 0.577f, 0.36f, 0.16f,
		  8, 38, 12, 10, 0, 8,
		  0.3f, 5.0f, 0.68f, 0.33f, 0.0f, 2.8f },

		{ "class8.classconst", 1.8f, 0.8f, 1.0f, 0.0f, 0.0f, 0.024f,
		  0.12f, 0.08f, 0.1f, 0.14f, 0.12f, 0.26f, 0.0f,
		  0.4f, 0.57f, 0.6f, 1.3f,
		  5, 6, 2, 4,
		  1.2f, 0.427f, 0.3f, 0.4f,
		  5, 34, 18, 12, 0, 7,
		  0.3f, 4.4f, 0.64f, 0.47f, 0.0f, 2.4f },

		{ "class9.classconst", 1.8f, 1.3f, 1.06f, 0.0f, 0.0f, 0.027f,
		  0.18f, 0.12f, 0.08f, 0.2f, 0.2f, 0.28f, 0.0f,
		  0.4f, 0.33f, 0.6f, 0.4f,
		  3, 4, 4, 4,
		  1.2f, 0.797f, 0.4f, 0.3f,
		  6, 32, 36, 10, 0, 6,
		  0.3f, 4.2f, 0.75f, 0.33f, 0.0f, 2.6f },

		{ "classA.classconst", 1.75f, 1.1f, 1.0f, 0.0f, 0.0f, 0.03f,
		  0.1f, 0.07f, 0.15f, 0.14f, 0.12f, 0.2f, 0.0f,
		  0.4f, 0.6f, 0.5f, 1.0f,
		  3, 5, 2, 2,
		  1.2f, 0.476f, 0.25f, 0.3f,
		  5, 31, 20, 10, 0, 6,
		  0.3f, 4.0f, 0.6f, 0.3f, 0.0f, 2.4f },

		{ "classB.classconst", 1.75f, 1.1f, 1.0f, 0.0f, 0.0f, 0.03f,
		  0.1f, 0.07f, 0.15f, 0.14f, 0.12f, 0.2f, 0.0f,
		  0.4f, 0.6f, 0.5f, 1.0f,
		  3, 5, 2, 2,
		  1.2f, 0.476f, 0.25f, 0.3f,
		  5, 31, 20, 10, 0, 6,
		  0.3f, 4.0f, 0.6f, 0.3f, 0.0f, 2.4f },

		{ "classC.classconst", 2.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.028f,
		  0.13f, 0.1f, 0.05f, 0.05f, 0.05f, 0.2f, 0.0f,
		  0.4f, 0.46f, 0.6f, 0.4f,
		  4, 1, 4, 4,
		  1.0f, 0.604f, 0.5f, 0.5f,
		  10, 34, 14, 12, 0, 8,
		  0.3f, 4.4f, 0.7f, 0.47f, 0.0f, 2.7f },

		{ "classD.classconst", 2.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.028f,
		  0.13f, 0.1f, 0.05f, 0.05f, 0.05f, 0.2f, 0.0f,
		  0.4f, 0.46f, 0.6f, 0.4f,
		  4, 1, 4, 4,
		  1.0f, 0.604f, 0.5f, 0.5f,
		  10, 34, 14, 12, 0, 8,
		  0.3f, 4.4f, 0.7f, 0.47f, 0.0f, 2.7f },

		{ "classE.classconst", 1.8f, 1.1f, 1.08f, 0.0f, 0.0f, 0.025f,
		  0.13f, 0.05f, 0.05f, 0.17f, 0.06f, 0.26f, 0.0f,
		  0.4f, 0.6f, 0.75f, 1.0f,
		  4, 4, 4, 4,
		  1.2f, 0.512f, 0.33f, 0.35f,
		  8, 35, 15, 11, 0, 7,
		  0.3f, 4.8f, 0.65f, 0.42f, 0.0f, 2.4f },

		{ "classF.classconst", 1.8f, 1.1f, 1.08f, 0.0f, 0.0f, 0.025f,
		  0.13f, 0.05f, 0.05f, 0.17f, 0.06f, 0.26f, 0.0f,
		  0.4f, 0.6f, 0.75f, 1.0f,
		  4, 4, 4, 4,
		  1.2f, 0.512f, 0.33f, 0.35f,
		  8, 35, 15, 11, 0, 7,
		  0.3f, 4.8f, 0.65f, 0.42f, 0.0f, 2.4f },
		};

		// One note per row, materialised once. The table is immutable, so these
		// live for the program's lifetime by construction.
		const std::string* BuildNotes()
		{
			static std::string notes[ClassConstantTable::kRowCount];
			static bool built = false;
			if (!built)
			{
				for (std::size_t position = 0; position < ClassConstantTable::kRowCount; ++position)
				{
					notes[position] = BuildRecoveredNote(kDeployed[position].file);
				}
				built = true;
			}
			return notes;
		}

		bool AllCoefficientsFinite(const ClassConstants& value) noexcept
		{
			return IsFinite(value);
		}
	}

	bool ValidateRow(ClassConstantRow& row) noexcept
	{
		if (!IsValidClass(row.index))
		{
			row.source = CoefficientSource::Unavailable;
			row.note   = "refused: index is not one of the sixteen legacy "
			             "EMCHARINDEX values";
			return false;
		}

		if (row.source != CoefficientSource::Recovered)
		{
			if (row.note == nullptr || row.note[0] == '\0')
			{
				row.note = kMissingDataNote;
			}
			return false;
		}

		if (row.note == nullptr || row.note[0] == '\0')
		{
			// Provenance that cannot be stated cannot be audited, so a
			// value-bearing row without one is not a recovered row.
			row.source = CoefficientSource::Unavailable;
			row.note   = "refused: a Recovered row must name its source file and row";
			return false;
		}

		if (!AllCoefficientsFinite(row.constants))
		{
			// Demoted, not repaired. See the header: a table that repairs itself
			// cannot be audited.
			row.source = CoefficientSource::Unavailable;
			row.note   = "refused: a coefficient is non-finite (IsFinite)";
			return false;
		}

		return true;
	}

	const ClassConstantTable& ClassConstantTable::Verified() noexcept
	{
		static const ClassConstantTable table = []
		{
			ClassConstantTable built;
			const std::string* notes = BuildNotes();

			for (std::size_t position = 0; position < ClassConstantTable::kRowCount; ++position)
			{
				const DeployedRow& src = kDeployed[position];

				ClassConstantRow& row = built.m_rows[position];
				row.index   = static_cast<CharClassIndex>(static_cast<uint8_t>(position));
				row.source  = CoefficientSource::Recovered;
				row.note    = notes[position].c_str();

				ClassConstants& cc = row.constants;

				cc.beginStats.pow  = src.beginPow;
				cc.beginStats.str  = src.beginStr;
				cc.beginStats.spi  = src.beginSpi;
				cc.beginStats.dex  = src.beginDex;
				cc.beginStats.intel = src.beginInt;
				cc.beginStats.sta  = src.beginSta;

				cc.levelUpStats.pow   = src.upPow;
				cc.levelUpStats.str   = src.upStr;
				cc.levelUpStats.spi   = src.upSpi;
				cc.levelUpStats.dex   = src.upDex;
				cc.levelUpStats.intel = src.upInt;
				cc.levelUpStats.sta   = src.upSta;

				cc.beginAttackPoint  = src.beginAttackPoint;
				cc.beginDefensePoint = src.beginDefensePoint;
				cc.beginMeleePower   = src.beginMeleePower;
				cc.beginShootPower   = src.beginShootPower;

				cc.levelUpAttackPoint  = src.levelUpAttackPoint;
				cc.levelUpDefensePoint = src.levelUpDefensePoint;
				cc.levelUpMeleePower   = src.levelUpMeleePower;
				cc.levelUpShootPower   = src.levelUpShootPower;

				cc.attackPointConversion  = src.attackPointConversion;
				cc.defensePointConversion = src.defensePointConversion;
				cc.meleePowerConversion   = src.meleePowerConversion;
				cc.shootPowerConversion   = src.shootPowerConversion;

				cc.hpPerStr  = src.hpPerStr;
				cc.mpPerSpi  = src.mpPerSpi;
				cc.spPerSta  = src.spPerSta;

				cc.hitPerDex     = src.hitPerDex;
				cc.avoidPerDex   = src.avoidPerDex;
				cc.defensePerDex = src.defensePerDex;

				cc.meleePerPow = src.meleePerPow;
				cc.meleePerDex = src.meleePerDex;
				cc.shootPerPow = src.shootPerPow;
				cc.shootPerDex = src.shootPerDex;

				cc.magicPerDex   = src.magicPerDex;
				cc.magicPerSpi   = src.magicPerSpi;
				cc.magicPerIntel = src.magicPerIntel;
			}

			return built;
		}();

		return table;
	}

	const ClassConstantRow* ClassConstantTable::Find(CharClassIndex index) const noexcept
	{
		const auto position = static_cast<uint8_t>(index);
		if (position >= kRowCount)
		{
			return nullptr;
		}

		const ClassConstantRow& row = m_rows[position];
		return static_cast<uint8_t>(row.index) == position ? &row : nullptr;
	}

	const ClassConstantRow& ClassConstantTable::RowAt(std::size_t position) const noexcept
	{
		return m_rows[position < kRowCount ? position : 0];
	}

	std::size_t ClassConstantTable::RecoveredCount() const noexcept
	{
		std::size_t recovered = 0;
		for (std::size_t position = 0; position < ClassConstantTable::kRowCount; ++position)
		{
			if (m_rows[position].source == CoefficientSource::Recovered)
			{
				++recovered;
			}
		}
		return recovered;
	}

} // namespace Modern::Stats
