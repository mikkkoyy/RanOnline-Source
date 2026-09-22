#include "stdafx.h"

#include "./G-Logic/GLCharDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

WORD school2index ( const EMSCHOOLFLAG emSchool )
{
	switch ( emSchool )
	{
	case GLSCHOOL_00:	return 0;
	case GLSCHOOL_01:	return 1;
	case GLSCHOOL_02:	return 2;
	};

	return 0;
}

EMSCHOOLFLAG index2school ( const WORD wSchool )
{
	switch ( wSchool )
	{
	case 0:	return GLSCHOOL_00;
	case 1:	return GLSCHOOL_01;
	case 2:	return GLSCHOOL_02;
	};

	return GLSCHOOL_00;
}

EMCLASSINDEX CharClassToClassIndex ( const EMCHARCLASS emClass )
{
	switch (emClass)
	{
	case GLCC_BRAWLER_M:
	case GLCC_BRAWLER_W:
	case GLCC_BRAWLER:
		return GLCL_BRAWLER;

	case GLCC_SWORDSMAN_M:
	case GLCC_SWORDSMAN_W:
	case GLCC_SWORDSMAN:
		return GLCL_SWORDSMAN;

	case GLCC_ARCHER_M:
	case GLCC_ARCHER_W:
	case GLCC_ARCHER:
		return GLCL_ARCHER;	

	case GLCC_SHAMAN_M:
	case GLCC_SHAMAN_W:
	case GLCC_SHAMAN:
		return GLCL_SHAMAN;

	case GLCC_EXTREME_M:
	case GLCC_EXTREME_W:
	case GLCC_EXTREME:
		return GLCL_EXTREME;

	case GLCC_GUNNER_M:
	case GLCC_GUNNER_W:
	case GLCC_GUNNER:
		return GLCL_GUNNER;

    case GLCC_ASSASSIN_M:
    case GLCC_ASSASSIN_W:
	case GLCC_ASSASSIN:
		return GLCL_ASSASSIN;

	case GLCC_TRICKER_M:
	case GLCC_TRICKER_W:
	case GLCC_TRICKER:
		return GLCL_TRICKER;

    /*case GLCC_ETC_M:
    case GLCC_ETC_W:
	case GLCC_ETC:
		return GLCL_ETC;

	case GLCC_ACTOR_M:
	case GLCC_ACTOR_W:
	case GLCC_ACTOR:
		return GLCL_ACTOR;*/
	};

	return GLCL_BRAWLER;
}

EMCHARINDEX CharClassToIndex ( const EMCHARCLASS emClass )
{
	switch (emClass)
	{
	case GLCC_BRAWLER_M:	return GLCI_BRAWLER_M;
	case GLCC_SWORDSMAN_M:	return GLCI_SWORDSMAN_M;
	case GLCC_ARCHER_W:		return GLCI_ARCHER_W;
	case GLCC_SHAMAN_W:		return GLCI_SHAMAN_W;
	case GLCC_EXTREME_M:	return GLCI_EXTREME_M;
	case GLCC_EXTREME_W:	return GLCI_EXTREME_W;
	
	case GLCC_BRAWLER_W:	return GLCI_BRAWLER_W;
	case GLCC_SWORDSMAN_W:	return GLCI_SWORDSMAN_W;
	case GLCC_ARCHER_M:		return GLCI_ARCHER_M;
	case GLCC_SHAMAN_M:		return GLCI_SHAMAN_M;

	case GLCC_GUNNER_M:		return GLCI_GUNNER_M;
	case GLCC_GUNNER_W:		return GLCI_GUNNER_W;

	case GLCC_ASSASSIN_M:	return GLCI_ASSASSIN_M;
	case GLCC_ASSASSIN_W:	return GLCI_ASSASSIN_W;
	case GLCC_TRICKER_M:	return GLCI_TRICKER_M;
	case GLCC_TRICKER_W:	return GLCI_TRICKER_W;
	};

	GASSERT(0&&"잘못된 케릭터 클래스입니다.");
	return GLCI_BRAWLER_M;
}

EMCHARCLASS CharIndexToClass ( const EMCHARINDEX emIndex )
{
	switch (emIndex)
	{
	case GLCI_BRAWLER_M:	return GLCC_BRAWLER_M;
	case GLCI_SWORDSMAN_M:	return GLCC_SWORDSMAN_M;
	case GLCI_ARCHER_W:		return GLCC_ARCHER_W;
	case GLCI_SHAMAN_W:		return GLCC_SHAMAN_W;

	case GLCI_EXTREME_M:	return GLCC_EXTREME_M;
	case GLCI_EXTREME_W:	return GLCC_EXTREME_W;

	case GLCI_BRAWLER_W:	return GLCC_BRAWLER_W;
	case GLCI_SWORDSMAN_W:	return GLCC_SWORDSMAN_W;
	case GLCI_ARCHER_M:		return GLCC_ARCHER_M;
	case GLCI_SHAMAN_M:		return GLCC_SHAMAN_M;

	case GLCI_GUNNER_M:		return GLCC_GUNNER_M;
	case GLCI_GUNNER_W:		return GLCC_GUNNER_W;

	case GLCI_ASSASSIN_M:	return GLCC_ASSASSIN_M;
	case GLCI_ASSASSIN_W:	return GLCC_ASSASSIN_W;
	case GLCI_TRICKER_M:	return GLCC_TRICKER_M;
	case GLCI_TRICKER_W:	return GLCC_TRICKER_W;
	};

	GASSERT(0&&"잘못된 케릭터 인덱스입니다.");
	return GLCC_BRAWLER_M;
}


EMCHARCLASS CharClassToSex ( const EMCHARCLASS emClass )
{
	switch (emClass)
	{
	case GLCC_BRAWLER_M:	return GLCC_BRAWLER_W;
	case GLCC_SWORDSMAN_M:	return GLCC_SWORDSMAN_W;
	case GLCC_ARCHER_W:		return GLCC_ARCHER_M;
	case GLCC_SHAMAN_W:		return GLCC_SHAMAN_M;
	case GLCC_EXTREME_M:	return GLCC_EXTREME_W;
	case GLCC_EXTREME_W:	return GLCC_EXTREME_M;
	
	case GLCC_BRAWLER_W:	return GLCC_BRAWLER_M;
	case GLCC_SWORDSMAN_W:	return GLCC_SWORDSMAN_M;
	case GLCC_ARCHER_M:		return GLCC_ARCHER_W;
	case GLCC_SHAMAN_M:		return GLCC_SHAMAN_W;

	case GLCC_GUNNER_M:		return GLCC_GUNNER_W;
	case GLCC_GUNNER_W:		return GLCC_GUNNER_M;

	case GLCC_ASSASSIN_M:	return GLCC_ASSASSIN_W;
	case GLCC_ASSASSIN_W:	return GLCC_ASSASSIN_M;
	case GLCC_TRICKER_M:	return GLCC_TRICKER_W;
	case GLCC_TRICKER_W:	return GLCC_TRICKER_M;
	};

	GASSERT(0&&"잘못된 케릭터 클래스입니다.");
	return GLCC_BRAWLER_M;
}

EMCHARINDEX CharIndexToSex ( const EMCHARINDEX emIndex )
{
	switch (emIndex)
	{
	case GLCI_BRAWLER_M:	return GLCI_BRAWLER_W;
	case GLCI_SWORDSMAN_M:	return GLCI_SWORDSMAN_W;
	case GLCI_ARCHER_W:		return GLCI_ARCHER_M;
	case GLCI_SHAMAN_W:		return GLCI_SHAMAN_M;

	case GLCI_EXTREME_M:	return GLCI_EXTREME_W;
	case GLCI_EXTREME_W:	return GLCI_EXTREME_M;

	case GLCI_BRAWLER_W:	return GLCI_SHAMAN_W;
	case GLCI_SWORDSMAN_W:	return GLCI_ARCHER_W;
	case GLCI_ARCHER_M:		return GLCI_SWORDSMAN_M;
	case GLCI_SHAMAN_M:		return GLCI_BRAWLER_M;

	case GLCI_GUNNER_M:		return GLCI_GUNNER_W;
	case GLCI_GUNNER_W:		return GLCI_GUNNER_M;

	case GLCI_ASSASSIN_M:	return GLCI_ASSASSIN_W;
	case GLCI_ASSASSIN_W:	return GLCI_ASSASSIN_M;
	case GLCI_TRICKER_M:	return GLCI_TRICKER_W;
	case GLCI_TRICKER_W:	return GLCI_TRICKER_M;
	};

	GASSERT(0&&"잘못된 케릭터 인덱스입니다.");
	return GLCI_BRAWLER_M;
}

EMCHARGENDER CharClassToGender( const EMCHARCLASS emClass )
{
	switch (emClass)
	{
	case GLCC_BRAWLER_M:
	case GLCC_SWORDSMAN_M:
	case GLCC_ARCHER_M:
	case GLCC_SHAMAN_M:	
	case GLCC_EXTREME_M:
	case GLCC_GUNNER_M:
	case GLCC_ASSASSIN_M:
	case GLCC_TRICKER_M:
		return GLGENDER_M;
		break;

	case GLCC_BRAWLER_W:
	case GLCC_SWORDSMAN_W:
	case GLCC_ARCHER_W:
	case GLCC_SHAMAN_W:
	case GLCC_EXTREME_W:
	case GLCC_GUNNER_W:
	case GLCC_ASSASSIN_W:
	case GLCC_TRICKER_W:
		return GLGENDER_W;
		break;
	};

	GASSERT(0&&"CharClassToGender Invalid emClass.");
	return GLGENDER_M;
}

EMCHARGENDER CharIndexToGender( const EMCHARINDEX emIndex )
{
	switch (emIndex)
	{
	case GLCI_BRAWLER_M:
	case GLCI_SWORDSMAN_M:
	case GLCI_ARCHER_M:
	case GLCI_SHAMAN_M:	
	case GLCI_EXTREME_M:
	case GLCI_GUNNER_M:
	case GLCI_ASSASSIN_M:
	case GLCI_TRICKER_M:
		return GLGENDER_M;
		break;

	case GLCI_BRAWLER_W:
	case GLCI_SWORDSMAN_W:
	case GLCI_ARCHER_W:
	case GLCI_SHAMAN_W:
	case GLCI_EXTREME_W:
	case GLCI_GUNNER_W:
	case GLCI_ASSASSIN_W:
	case GLCI_TRICKER_W:
		return GLGENDER_W;
		break;
	};

	GASSERT(0&&"CharIndexToGender Invalid emIndex.");
	return GLGENDER_M;
}

VOID GetCharSkillClassIndex( const EMCHARCLASS emClass, WORD & wBeginIndex, WORD & wEndIndex )
{
	switch ( emClass )
	{
	case GLCC_BRAWLER_M:
	case GLCC_BRAWLER_W:
		{
			wBeginIndex = EMSKILL_BRAWLER_01;
			wEndIndex = EMSKILL_BRAWLER_04;
		}
		break;

	case GLCC_SWORDSMAN_M:
	case GLCC_SWORDSMAN_W:
		{
			wBeginIndex = EMSKILL_SWORDSMAN_01;
			wEndIndex = EMSKILL_SWORDSMAN_04;
		}
		break;

	case GLCC_ARCHER_M:
	case GLCC_ARCHER_W:
		{
			wBeginIndex = EMSKILL_ARCHER_01;
			wEndIndex = EMSKILL_ARCHER_04;
		}
		break;

	case GLCC_SHAMAN_M:
	case GLCC_SHAMAN_W:
		{
			wBeginIndex = EMSKILL_SHAMAN_01;
			wEndIndex = EMSKILL_SHAMAN_04;
		}
		break;	

	case GLCC_EXTREME_M:
	case GLCC_EXTREME_W:
		{
			wBeginIndex = EMSKILL_EXTREME_01;
			wEndIndex = EMSKILL_EXTREME_04;
		}
		break;

	case GLCC_GUNNER_M:
	case GLCC_GUNNER_W:
		{
			wBeginIndex = EMSKILL_GUNNER_01;
			wEndIndex = EMSKILL_GUNNER_04;
		}
		break;

	case GLCC_ASSASSIN_M:
	case GLCC_ASSASSIN_W:
		{
			wBeginIndex = EMSKILL_ASSASSIN_01;
			wEndIndex = EMSKILL_ASSASSIN_04;
		}
		break;

	case GLCC_TRICKER_M:
	case GLCC_TRICKER_W:
		{
			wBeginIndex = EMSKILL_TRICKER_01;
			wEndIndex = EMSKILL_TRICKER_04;
		}
		break;

	default:
		GASSERT(0&&"잘못된 케릭터 인덱스입니다.");
		break;
	};
}

namespace COMMENT
{
	std::string ACTIONTYPE[GLAT_SIZE] =
	{
		"GLAT_IDLE",
		"GLAT_MOVE",
		"GLAT_ATTACK",
		"GLAT_SKILL",
		"GLAT_SHOCK",
		"GLAT_PUSHPULL",
		"GLAT_SKILLMOVE", /*push pull skill logic, Juver, 2017/06/05 */
		"GLAT_GATHERING",
		"GLAT_TALK",
		"GLAT_CONFT_END",
		"GLAT_FALLING",
		"GLAT_DIE",
		"GLAT_SKILLDASH", /*dash skill logic, Juver, 2017/06/17 */
	};

	std::string TRIBE[TRIBE_NSIZE] = 
	{
		"Human",
		"Monster",
		"Angel",
		"Devil",
		"Fairy",
		"Genie",
		"Dragon"
	};

	std::string MOVETYPE[MOVETYPE_NSIZE] = 
	{
		"Ground Movement",
		"Air Movement",
		"Water Movement"
	};

	std::string ELEMENT[EMELEMENT_MAXNUM2] = 
	{
		"Vital Energy(Spirit)",
		"Fire",
		"Ice",
		"Electric",
		"Poison",
		"Stone",
		"Confusion(Mad)",
		"Fainting(Stun)",
		"Malediction(Curse)",
		"Weapon Element",
	};

	std::string BRIGHT[BRIGHT_SIZE] =
	{
		"Light(Good)",
		"Dark(Evil)",
		"Both(Neutral)",
	};

	std::string CHARCLASS[GLCI_NUM_8CLASS] =
	{
		"Brawler Male",
		"Swordsman Male",
		"Archer Female",
		"Shaman Female",
		"Extreme Male",
		"Extreme Female",
		"Brawler Female",
		"Swordsman Female",
		"Archer Male",
		"Shaman Male",
		"Gunner Male",
		"Gunner Female",
		"Assassin Male",
		"Assassin Female",
		"Magician Male",
		"Magician Female",
	};

	std::string CHARCLASS2[GLCI2_NUM_8CLASS+1] =
	{
		"Brawler",
		"Swordsman",
		"Archer",
		"Shaman",
		"Extreme",
		"Gunner",
		"Assassin",
		"Magician",
		"All Class",
	};

	std::string EMSCHOOL[GLSCHOOL_NUM] =
	{
		"Sacred Gate",
		"Mystic Peak",
		"Phoenix",
	};

	std::string EMGENDER[GLGENDER_NUM] =
	{
		"Female",
		"Male",
	};

	std::string CHARCOMMENT[GLCI_NUM_8CLASS] = 
	{
		"", "", "", "", "", "", "", "", "", "",
		"", "", "", "", "", "",
	};

	std::string BLOW[EMBLOW_SIZE] =
	{
		"None",
		"Paralysis(Numb)",
		"Faint(Stun)",
		"Petrochemical(Stone)",
		"Burn",
		"Freeze",
		"Chaos(Mad)",
		"Poison",
		"Curse",
	};

	std::string BLOW_VAR1[EMBLOW_SIZE] =
	{
		"None",
		"Movement Speed",
		"None",
		"Movement Speed",
		"None",
		"Movement Speed",
		"None",
		"None",
		"None",
	};

	float BLOW_VAR1_SCALE[EMBLOW_SIZE] =
	{
		1.0f,
		100.0f,
		1.0f,
		100.0f,
		1.0f,
		100.0f,	
		1.0f,
		1.0f,
		1.0f,
	};

	std::string BLOW_VAR2[EMBLOW_SIZE] =
	{
		"None",
		"DelayRate",
		"None",
		"HP Recovery",
		"HP Recovery",
		"Damage Increase",
		"None",
		"HP Recovery",
		"HP Recovery",
	};

	float BLOW_VAR2_SCALE[EMBLOW_SIZE] =
	{
		1.0f,
		100.0f,
		1.0f,
		1.0f,
		1.0f,
		100.0f,	
		1.0f,
		1.0f,
		1.0f,
	};

	std::string IMPACT_ADDON[EIMPACTA_SIZE] =
	{
		"None",
		"Hit Rate",
		"Avoid Rate",
		"Attack",
		"Defense",
		"HP Recover",
		"MP Recover",
		"SP Recover",
		"HP+MP+SP Recover",
		"Attack Percentage",
		"Defense Percentage",
		"Melee",
		"Missile",
		"Energy",
		"Maximum HP",
		"Maximum MP",
		"Maximum SP",
		"Resistance",
		"Change Stats",
		"HP recovery amount +-",
		"MP recovery amount +-",
		"SP recovery amount +-",
		"CP recovery amount +-",
		"CP Auto +-",
	};

	float IMPACT_ADDON_SCALE[EIMPACTA_SIZE] =
	{
		1.0f,	1.0f,	1.0f,	1.0f,	1.0f,
		100.0f,	100.0f,	100.0f,	100.0f,	100.0f,
		100.0f,	1.0f,	1.0f,	1.0f,	1.0f,
		1.0f,	1.0f,	1.0f,	1.0f,	1.0f,
		1.0f,	1.0f,	1.0f,	1.0f,
	};

	std::string IMPACT_TAR[TAR_SIZE] =
	{
		"Self",
		"Target",
		"From Self To Target",
		"Specific Location",
		"Specified"
	};

	std::string IMPACT_REALM[REALM_SIZE] =
	{
		"Target Self",
		"Target Area",
		"Keep Target Area",
		"Cone Target",
	};

	std::string IMPACT_SIDE[SIDE_SIZE] =
	{
		"Target MySide",
		"Target Enemy",
		"Target Everyone",
	};

	std::string SPEC_ADDON[EMSPECA_NSIZE] =
	{
		"None",
		"Distract Movement",
		"Return Damage",
		"Revive",
		"Steal HP",
		"Steal MP",
		"Steal SP",
		"Share HP",
		"Share MP",
		"Share SP",
		"Prevent Abnormal Condition",
		"Avoid Abnormal Condition",
		"Pierce",
		"Target Range",
		"Movement Speed",
		"Dash",
		"Invisible",
		"Find Invisible",
		"Attack Speed",
		"Skill Delay"
		"Crushing Blow",
		"Physical Damage Absorb",
		"Magic Damage Absorb",
		"Return Physical Damage",
		"Return Magic Damage",
		"Remove Buff",
		"Stun",
		"Active Type Skill",
		"Pull",
		"Push",
		"Continuous Damage",
		"Curse",
		"Change Attack Range",
		"Change Coverage Area",
		"Prohibit Using Potion",
		"Prohibit Using Skill",
		"Damage Absorption",
		"Item Drop Rate",
		"Gold Drop Rate",
		"EXP Rate",
		"Special NPC Dialogue",
		"Special Item Drop",
		"Teleport",
		"Shift Position",
		"Enhancement",
		"Illusion",
		"Vehicle Off",
		"Duration Change",
		"Stigma",
		"Transform",
		"Effect Duration",
		"Damage Loop",
		"Pet Off",
		"Immunity",
		"Hostile",
		"Hostile Off",
		"Target Fix",
		"Dominate",
		"Random EXP Rate",
		"Skill Link",
	};

	std::string SPEC_ADDON_VAR1[EMSPECA_NSIZE] =
	{
		"None",
		"Distance",
		"None",
		"Evasion Rate",
		"Absorb Rate",
		"Absorb Rate",
		"Absorb Rate",
		"None",
		"None",
		"None",
		"None",
		"None",
		"Pierce",
		"Weapon Reach",
		"Movement Speed",
		"Chance",
		"None",
		"None",
		"Attack Speed",
		"Skill Delay",
		"Distance",
		"Absorb Rate",
		"Absorb Rate",
		"Return Rate",
		"Return Rate",
		"None",
		"None",
		"Rate",	
		"Rate", 
		"Rate",
		"Duration",
		"Reflect Rate",
		"Range", 
		"Range", 
		"None", 
		"None", 
		"Number of", 
		"Increase Rate", 
		"Increase Rate", 
		"Increase Rate", 
		"None", 
		"None", 
		"None", 
		"Rate", 
		"Chance", 
		"None", 
		"Chance", 
		"Change1", 
		"Effect Distance", 
		"Transform ID", 
		"Rate", 
		"Rate", 
		"Rate", 
		"Type", 
		"Rate", 
		"None", 
		"Rate", 
		"Rate", 
		"Rate", 
		"None", 
	};

	float SPEC_ADDON_VAR1_SCALE[EMSPECA_NSIZE] =
	{
		1.0f,	1.0f,	1.0f,	100.0f,	100.0f,
		100.0f,	100.0f,	1.0f,	1.0f,	1.0f,
		1.0f,	1.0f,	1.0f,	1.0f,	100.0f,
		1.0f,	1.0f,	1.0f,	100.0f,	1.0f,
		1.0f,	100.0f,	100.0f,	100.0f,	100.0f,
		1.0f,	1.0f,	100.0f, 100.0f, 100.0f,
		1.0f,	100.0f,	1.0f,	1.0f,	1.0f,
		1.0f,	1.0f,	100.0f, 100.0f, 100.0f,
		1.0f,	1.0f,	1.0f,	100.0f,	100.0f,
		1.0f,	1.0f,	1.0f,	1.0f,	1.0f,
		1.0f,	1.0f,	1.0f,	1.0f,	1.0f,
		1.0f,	1.0f,	1.0f,	1.0f,	1.0f,	
	};

	std::string SPEC_ADDON_VAR2[EMSPECA_NSIZE] =
	{
		"None",
		"Chance",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"Speed",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"Chance",
		"Chance",
		"None",
		"None",
		"None",
		"Moving Speed", 
		"Range", 
		"Damage", 
		"None",
		"None",
		"None",
		"None",
		"None",
		"Absorption", 
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"Speed",
		"Increase",
		"None",
		"Duration",
		"Change2",
		"Value",
		"Emoticon ID",
		"Release Condition",
		"None",
		"None",
		"Type2",
		"Duration",
		"None",
		"Distance",
		"Damage Change",
		"Range",
		"None",
	};

	float SPEC_ADDON_VAR2_SCALE[EMSPECA_NSIZE] =
	{
		1.0f,	100.0f,	1.0f,	1.0f,	1.0f,
		1.0f,	1.0f,	1.0f,	1.0f,	1.0f,
		1.0f,	1.0f,	1.0f,	1.0f,	1.0f,
		1.0f,	1.0f,	1.0f,	1.0f,	1.0f,
		1.0f,	1.0f,	1.0f,	100.0f,	100.0f,	
		1.0f,	1.0f,	1.0f,	1.0f,	1.0f,
		1.0f,	1.0f,	1.0f,	1.0f,	1.0f,
		1.0f,	1.0f,	1.0f,	1.0f,	1.0f,
		1.0f,	1.0f,	1.0f,	2.0f,	1.0f,
		1.0f,	1.0f,	1.0f,	1.0f,	1.0f,
		1.0f,	1.0f,	1.0f,	1.0f,	1.0f,
		1.0f,	1.0f,	1.0f,	50.0f,	1.0f,
	};

	std::string SKILLCLASS[EMSKILLCLASS_NSIZE] =
	{
		"Brawler-class1",
		"Brawler-class2",
		"Brawler-class3",
		"Brawler-class4",
		"Swordsman-class1",
		"Swordsman-class2",
		"Swordsman-class3",
		"Swordsman-class4",
		"Archer-class1",
		"Archer-class2",
		"Archer-class3",
		"Archer-class4",
		"Shaman-class1",
		"Shaman-class2",
		"Shaman-class3",
		"Shaman-class4",
		"NPC_01",
		"NPC_02",
		"NPC_03",
		"NPC_04",
		"NPC_05",
		"NPC_06",
		"NPC_07",
		"NPC_08",
		"NPC_09",
		"NPC_10",
		"NPC_11(Pet SKill)",
		"NPC_12",
		"NPC_13",
		"NPC_14",
		"Extreme-class1",
		"Extreme-class2",
		"Extreme-class3",
		"Extreme-class4",
		"NPC_19",
		"NPC_20",
		"Gunner-class1",
		"Gunner-class2",
		"Gunner-class3",
		"Gunner-class4",
		"Food",
		"Summon",
		"Magician-Animal",
		"Assassin-class1",
		"Assassin-class2",
		"Assassin-class3",
		"Assassin-class4",
		"ETC-class1",
		"ETC-class2",
		"ETC-class3",
		"ETC-class4",
		"CaptureTheField",
		"Event",
		"ETC",
		"Test",
		"Magician-multi",
		"Magician-class1",
		"Magician-class2",
		"Magician-class3",
		"Magician-class4",
	};

    std::string SPEC_INC_TYPE[EMSPEC_INC_TYPE_NSIZE] =
    {
        "Increase",
        "Decrease"
    };

	std::string LANDEFFECT_TYPE[EMLANDEFFECT_SIZE] =
	{
		"Attack Speed",
		"Movement Speed",
		"Maximum HP",
		"Maximum MP",
		"HP+MP+SP Recover",
		"Attack Rate",
		"Defense Rate",
		"Resistance",
		"Remove All Buffs",
		"SafeZone"
	};
};