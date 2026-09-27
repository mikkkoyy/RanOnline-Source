#include "stdafx.h"

#include "GLDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace COMMENT
{	
	std::string CROW[CROW_NUM] = 
	{
		"CROW_PC",
		"CROW_NPC",
		"CROW_MOB",
		"CROW_ITEM",
		"CROW_MONEY",
		"CROW_PET",
		"CROW_SUMMON",
		"CROW_MATERIAL",
		"CROW_ZONE_NAME",
		"CROW_GATE_NAME",
	};

	std::string NPC_TYPE[EMNPC_TYPE_SIZE] = 
	{
		"Normal NPC",
		"Post Box",
		"Event Tower",
		"Gambling Game",
	};

	/*pvp capture the flag, Juver, 2018/02/03 */
	std::string CTF_TYPE[EMNPC_CTF_FLAG_TYPE_SIZE] = 
	{
		"None",
		"Flag-A",
		"Flag-B",
	};

	std::string ATTACK_RGTYPE[EMATT_SIZE] =
	{
		"Short Range",
		"Long Range",
	};
};
