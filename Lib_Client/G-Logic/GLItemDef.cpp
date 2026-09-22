#include "stdafx.h"
#include "./GLItemDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

EMGRINDING_RS_INDEX GrindRsClassToIndex ( const EMGRINDING_RS_FLAGS emClass )
{
	switch (emClass)
	{
	case EMGRINDING_RS_FIRE:	return EMGRINDING_RS_FIRE_INDEX;
	case EMGRINDING_RS_ICE:		return EMGRINDING_RS_ICE_INDEX;
	case EMGRINDING_RS_ELECTRIC:return EMGRINDING_RS_ELECTRIC_INDEX;
	case EMGRINDING_RS_POISON:	return EMGRINDING_RS_POISON_INDEX;
	case EMGRINDING_RS_SPIRIT:	return EMGRINDING_RS_SPIRIT_INDEX;
	};

	GASSERT(0&&"잘못된 저항아이템 클래스입니다.");
	return EMGRINDING_RS_FIRE_INDEX;
}

EMGRINDING_RS_FLAGS GrindIndexToRsClass ( const EMGRINDING_RS_INDEX emIndex )
{
	switch (emIndex)
	{
	case EMGRINDING_RS_FIRE_INDEX:		return EMGRINDING_RS_FIRE;
	case EMGRINDING_RS_ICE_INDEX:		return EMGRINDING_RS_ICE;
	case EMGRINDING_RS_ELECTRIC_INDEX:	return EMGRINDING_RS_ELECTRIC;
	case EMGRINDING_RS_POISON_INDEX:	return EMGRINDING_RS_POISON;
	case EMGRINDING_RS_SPIRIT_INDEX:	return EMGRINDING_RS_SPIRIT;
	};

	GASSERT(0&&"잘못된 저항아이템 클래스입니다.");
	return EMGRINDING_RS_FIRE;
}

namespace COMMENT
{
	std::string ITEMLEVEL[LEVEL_NSIZE] =
	{
		"Common",
		"Rare",
		"Unique",
		"Infinity",
		"Epic",
	};

	std::string ItemTypeName(EMITEM_TYPE Type)
    {
        if (Type < ITEM_NSIZE)
            return ITEMTYPE[Type];
        else
            return std::string("");
    }

	std::string ITEMTYPE[ITEM_NSIZE] =
	{
		"Wearable Item",
		"Arrow",
		"Potion",
		"Skill Scroll",
		"Position Recall Card",
		"Quest Item",
		"Refines",
		"Talisman",
		"Bus Ticket",
		"SkillPoint Reset",
		"StatPoint Reset",
		"SkillStatPoint Reset",
		"ItemBox",
		"Remove Costume ( Delete )",
		"Megaphone",
		"Firecrackers",
		"Character Slot Card",
		"Inventory LineAdd(BackPack)",
		"Locker Rental",
		"Locker Remote",
		"Item Box (Premium)",
		"Personal Store Permit",
		"RandomBox",
		"Remove Costume (Separate)",
		"Character Hair Style",
		"Character Face Style",
		"Question Item",
		"Authentication CD",
		"Character Teleport Card",
		"Club Call (Not Used)",
		"Hair Shop(Not Used)",
		"Character Rename Card",
		"Character Hair Style Card",
		"Character Hair Color Card",
		"Revive Necklace",
		"Pet Card",
		"Pet Food",
		"Pet Rename Card",
		"Pet Color Card",
		"Pet Style Card",
		"Pet Skill Scroll",
		"SMS Send Card",
		"Pet Revive",
		"Item Protection Necklace",
		"Item Remodel",
		"Vehicle",
		"Vehicle Battery-Oil",
		"ItemCard",
		"ExpCard",
		"Character Change Gender Card",
		"Garbage Card",
		"Map Teleport Card",
		"Pet Skin Pack",
		"Character Face Style Card",
		"Taxi Ticket",
		"Material Item",
		"NPC Call Card",
		"NeutronBullets",
		"Food(Lunch)",
		"Pet Dual Skill Card",
		"Reinforcement Rosary",
		"Mitigation Rosary",
		"Bike Color Change Card",
		"Bike Boost Card",
		"Oblivion Potion(New)",
		"Costume Color Change Card",
		"Mailbox Card",
		"Item Food Unlock",
		"Point Card(Refundable)",
		"Point Card(No Refundable)",
		"Item Mix Scroll"
		"Random Option Card",
		"NonDrop Card",						/*nondrop card, Juver, 2017/11/26 */
		"Reset Compounding",
		"Personal Store Searching Card",
		"Change Scale Card",				/*change scale card, Juver, 2018/01/03 */
		"Wrapper",							/*item wrapper, Juver, 2018/01/11 */
		"Wrapper Box",						/*item wrapper, Juver, 2018/01/11 */
		"Change School Card",				/*change school card, Juver, 2018/01/12 */
		"Equipment Lock Enable",			/*equipment lock, Juver, 2018/01/13 */
		"Equipment Lock Recover",			/*equipment lock, Juver, 2018/01/13 */
		"Item Transfer Card",				/*item transfer card, Juver, 2018/01/18 */
		"Car Color Change Card",			/* car, cart color, Juver, 2018/02/14 */
		"Vehicle Booster Card",				/* booster all vehicle, Juver, 2018/02/14 */
		"Sealed Card",
		"Sealed Card",
		"Costume Blessed Paper",
	};

	std::string ITEMSUIT[SUIT_NSIZE] =
	{
		"HeadGear(Hats)",
		"UpperBody(Coat/Jacket)",
		"LowerBody(Pant/Skirt)",
		"Hand(Gloves)",
		"Foot(Shoes)",
		"Handheld(Weapon)",
		"Necklace",
		"Bracelet",
		"Ring",
		"Pet A",
		"Pet A"
		"Vehicle",
		"Vehicle Skin",
		"Vehicle Parts A",
		"Vehicle Parts B",
		"Vehicle Parts C",
		"Vehicle Parts D",
		"Vehicle Parts E",
		"Vehicle Parts F",
		"Belt",
		"Earring",
		"Accessory",
		"Ornament",
		"Face",
		"Misc",
	};

	std::string ITEMDRUG[ITEM_DRUG_SIZE] =
	{
		"None",
		"HP Recovery",
		"MP Recovery",
		"SP Recovery",
		"HP+MP Recovery",
		"HP+SP Recovery",
		"HP+MP+SP Recovery",
		"Abnormal Condition Cure",
		"Return to Campus",
		"Recall StartPoint Position",
		"Recall BackPoint Position",
		"Revive",
		"HP Recovery+Abnormal Condition Cure",
		"HP+MP+SP Recovery+Abnormal Condition Cure",
		"Teleport to Special Map",
		"CP Recovery",
	};

	std::string ITEMATTACK[ITEMATT_NSIZE] =
	{
		"Nothing",
		"Sword",
		"Blade",
		"Dagger",
		"Spear",
		"Club",
		"Gauntlet",
		"Bow",
		"Throw",
		"Dual Gun",
		"Rail Gun",
		"Portal Gun",
		"Scythe",
		"Dual Spear",
		"Shuriken",
		"Fist",
		"Wand",
		"Cube",
		"Whip",
		"Shield",
		"Hammer",
		"Umbrella",
		"No Type Check",
		"Portal/Rail Gun",
		"Guns",
		"Sword/Blade",
	};

	std::string ITEMADDON[EMADD_SIZE] =
	{
		"None",
		"Hit Rate",
		"Avoid Rate",
		"Attack",
		"Defense",
		"Maximum HP",
		"Maximum MP",
		"Maximum SP",
		"Stats POW",
		"Stats STR",
		"Stats SPI",
		"Stats DEX",
		"Stats INT",
		"Stats STM",
		"Melee",
		"Missile",
		"Energy",
		"Pow/Int/Dex",
		"Stm / Vit",
		"Melee / Shooting",
		"Melee / Shooting / Energy",
	};

	std::string ITEMVAR[EMVAR_SIZE] =
	{
		"None",
		"HP Recover Percentage",
		"MP Recover Percentage",
		"SP Recover Percentage",
		"HP+MP+SP Recover Percentage",
		"Movement Speed Percentage",
		"Attack Speed Percentage",
		"Critical Rate Percentage",
		"CrushingBlow Rate Percentage",
	};

	std::string ITEMVOL[EMVAR_SIZE] = 
	{
		"None",
		"HP Recover",
		"MP Recover",
		"SP Recover",
		"HP+MP+SP Recover",
		"Movement Speed",
		"Attack Speed",
		"Critical Rate",
		"None",
	};

	float ITEMVAR_SCALE[EMVAR_SIZE] =
	{
		1.0f,
		100.0f,
		100.0f,
		100.0f,
		100.0f,
		100.0f,
		100.0f,
		100.0f,
		100.0f,
	};

	bool IsITEMVAR_SCALE ( EMITEM_VAR emITEM_VAR )
	{
		return ITEMVAR_SCALE[emITEM_VAR]==100.0f;
	};

	std::string ITEM_QUE_TYPE[QUESTION_SIZE] =
	{
		"None",
		"Speed Up",
		"Crazy Time",
		"Power Up",
		"EXP Time",
		"EXP Get",
		"Lucky",
		"Bomb!!",
		"Mob Gen",
		"Speed Up Max",
		"Madness Time",
		"Power Up Max",
		"Heal",
	};

	std::string ITEM_QUE_VAR1[QUESTION_SIZE] =
	{
		"None",
		"Movement Speed",
		"Movement Speed",
		"Attack",
		"EXP",
		"None",
		"Drop Rate",
		"Damage",
		"Monster MID",
		"Movement Speed",
		"Movement Speed",
		"Attack",
		"HP Recover",
	};

	std::string ITEM_QUE_VAR2[QUESTION_SIZE] =
	{
		"None",
		"None",
		"Attack Speed",
		"None",
		"None",
		"None",
		"None",
		"None",
		"Monster SID",
		"None",
		"Attack Speed",
		"None",
		"None",
	};

	std::string GRINDING_TYPE[EMGRINDING_NSIZE] =
	{
		"None",
		"Attack",
		"Defense",
		"Resistance(Fire)",
		"Resistance(Ice)",
		"Resistance(Electric)",
		"Resistance(Poison)",
		"Resistance(Air)",
	};

	std::string GRINDING_LEVEL[EMGRINDING_LEVEL_SIZE] =
	{
		"Basic",
		"High",
		"Highest",
	};

	std::string GRINDING_RESIST[EMGRINDING_RS_NSIZE] =
	{
		"Fire",
		"Ice",
		"Electric",
		"Poison",
		"Air",
	};

	std::string GRINDING_CLASS[EMGRINDING_CLASS_SIZE] =
	{
		"Weapon",
		"Armor",
	};

	std::string ITEM_RANDOM_OPT[EMR_OPT_SIZE] =
	{
		"None",
		"Attack Percentage",
		"Defense Percentage",
		"HitRate",
		"AvoidRate",
		"HP",
		"MP",
		"SP",
		"HP Recover",
		"MP Recover",
		"SP Recover",
		"HP+MP+SP Recover",
		"Attack Upgrade",
		"Defense Upgrade",
		"Attack Range",
		"SP Usage",
		"Resistance",
		"Movement Speed"
		//Direct Value
		"Damage",
		"Defense",
		"Hitrate",
		"Avoidrate",
		"Power",
		"Vitality",
		"Intelligence",
		"Dextirity",
		"Stamina",
		"Melee",
		"Messile",
		"Energy"
	};

	float ITEM_RANDOM_OPT_SCALE[EMR_OPT_SIZE] =
	{
		1.0f,
		1.0f,	//	공격력.
		1.0f,	//	방어력.

		1.0f,	//	명중율.
		1.0f,	//	회피율.

		1.0f,	//	hp 증가.
		1.0f,	//	mp 증가.
		1.0f,	//	sp 증가.

		100.0f,	//	hp 증가율.
		100.0f,	//	mp 증가율.
		100.0f,	//	sp 증가율.
		100.0f,	//	hp, mp, sp 증가율.

		1.0f,	//	공격력 연마.
		1.0f,	//	방어력 연마.

		1.0f,	//	공격 영역.
		1.0f,	//	sp 소모량.
		1.0f,	//	저항.

		1.0f,	// 이동속도 증가

		1.0f,	//	공격력(정수).
		1.0f,	//	방어력(정수).

		1.0f,	//	명중율(정수).
		1.0f,	//	회피율(정수).

		1.0f,	//	힘(정수).
		1.0f,	//	체력(정수).
		1.0f,	//	정신(정수).
		1.0f,	//	민첩(정수).
		1.0f,	//	근력(정수).

		1.0f,	//	격투치(정수).
		1.0f,	//	사격치(정수).
		1.0f,	//	마력치(정수).

		1.0f,	//	포션 hp 증가량(정수).
		1.0f,	//	포션 mp 증가량(정수).
		1.0f,	//	포션 sp 증가량(정수).
	};


	DWORD	ITEMCOLOR[LEVEL_NSIZE] = 
	{
		0xffff0000,
		0xff00ff00,
		0xff0000ff,
		0xffffff00,
		0xffff00ff
	};


	std::string COOLTYPE[EMCOOL_SIZE] =
	{
		"ItemMID/SID",
		"ItemType",
	};

	std::string ITEMSLOT_CHAR[SLOT_NSIZE_S_2] =
	{
		"Head",
		"Upper",
		"Lower",
		"Hand",
		"Foot",
		"R-Hand",
		"L-Hand",
		"Necklace",
		"Bracelet",
		"R-Finger",
		"L-Finger",
		"R-Hand_S",
		"L-Hand_S",
		"Vehicle",
		"Belt",
		"Earring",
		"Accessory",
		"Ornament",
	};
};


