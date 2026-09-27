#include "stdafx.h"
#include "GLCodexDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace COMMENT
{
	std::string CODEX_NOTIFY[EMCODEX_NOTIFY_SIZE] =
	{
		"None",
		"Around",
		"AllServer",
	};

	std::string CODEX_MID[EMCODEX_TYPE_SIZE] =
	{
		"HP",
		"MP",
		"SP",
		"Attack",
		"Defense",
		"Shooting",
		"Melee",
		"Energy",
		"Resistance",
		"Avoidrate",
		"Hitrate",
	};
};


