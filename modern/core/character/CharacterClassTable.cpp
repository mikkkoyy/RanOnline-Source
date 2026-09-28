// VERTICAL-001: class-table index mapping. See CharacterClassTable.h.
//
// The table is transcribed from the legacy enum one-for-one. It is a lookup,
// not a rule, and the test asserts every value against the legacy header.

#include "character/CharacterClassTable.h"

namespace Modern
{
	namespace
	{
		struct ClassRow
		{
			CharacterClass    characterClass;
			CharacterGender   gender;
			Stats::CharClassIndex index;
		};

		// legacy/Lib_Client/G-Logic/GLCharDefine.h:235
		//   GLCI_BRAWLER_M = 0,  GLCI_SWORDSMAN_M  = 1,  GLCI_ARCHER_F   = 2
		//   GLCI_SHAMAN_F  = 3,  GLCI_EXTREME_M    = 4,  GLCI_EXTREME_F  = 5
		//   GLCI_BRAWLER_F = 6,  GLCI_SWORDSMAN_F  = 7,  GLCI_ARCHER_M   = 8
		//   GLCI_SHAMAN_M  = 9,  GLCI_GUNNER_M     = 10, GLCI_GUNNER_F   = 11
		//   GLCI_ASSASSIN_M = 12, GLCI_ASSASSIN_F   = 13, GLCI_TRICKER_M  = 14
		//   GLCI_TRICKER_F = 15
		constexpr ClassRow kRows[Stats::kClassCount] =
		{
			{ CharacterClass::Brawler,   CharacterGender::Male,   Stats::CharClassIndex::BrawlerMale    },
			{ CharacterClass::Swordsman, CharacterGender::Male,   Stats::CharClassIndex::SwordsmanMale  },
			{ CharacterClass::Archer,    CharacterGender::Female, Stats::CharClassIndex::ArcherFemale   },
			{ CharacterClass::Shaman,    CharacterGender::Female, Stats::CharClassIndex::ShamanFemale   },
			{ CharacterClass::Extreme,   CharacterGender::Male,   Stats::CharClassIndex::ExtremeMale    },
			{ CharacterClass::Extreme,   CharacterGender::Female, Stats::CharClassIndex::ExtremeFemale  },
			{ CharacterClass::Brawler,   CharacterGender::Female, Stats::CharClassIndex::BrawlerFemale  },
			{ CharacterClass::Swordsman, CharacterGender::Female, Stats::CharClassIndex::SwordsmanFemale},
			{ CharacterClass::Archer,    CharacterGender::Male,   Stats::CharClassIndex::ArcherMale     },
			{ CharacterClass::Shaman,    CharacterGender::Male,   Stats::CharClassIndex::ShamanMale     },
			{ CharacterClass::Gunner,    CharacterGender::Male,   Stats::CharClassIndex::GunnerMale     },
			{ CharacterClass::Gunner,    CharacterGender::Female, Stats::CharClassIndex::GunnerFemale   },
			{ CharacterClass::Assassin,  CharacterGender::Male,   Stats::CharClassIndex::AssassinMale   },
			{ CharacterClass::Assassin,  CharacterGender::Female, Stats::CharClassIndex::AssassinFemale },
			{ CharacterClass::Tricker,   CharacterGender::Male,   Stats::CharClassIndex::TrickerMale    },
			{ CharacterClass::Tricker,   CharacterGender::Female, Stats::CharClassIndex::TrickerFemale  },
		};
	}

	const char* ToString(CharacterGender gender) noexcept
	{
		return gender == CharacterGender::Male ? "male" : "female";
	}

	bool TryToCharClassIndex(CharacterClass characterClass, CharacterGender gender,
	                         Stats::CharClassIndex& out) noexcept
	{
		for (const ClassRow& row : kRows)
		{
			if (row.characterClass == characterClass && row.gender == gender)
			{
				out = row.index;
				return true;
			}
		}
		return false;
	}

	CharacterClass ClassOfCharClassIndex(Stats::CharClassIndex index) noexcept
	{
		const uint8_t value = static_cast<uint8_t>(index);
		return value < Stats::kClassCount ? kRows[value].characterClass
		                                  : CharacterClass::Unset;
	}

	CharacterGender GenderOfCharClassIndex(Stats::CharClassIndex index) noexcept
	{
		const uint8_t value = static_cast<uint8_t>(index);
		return value < Stats::kClassCount ? kRows[value].gender : CharacterGender::Male;
	}
}
