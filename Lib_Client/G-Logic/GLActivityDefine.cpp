#include "stdafx.h"
#include "GLActivityDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace COMMENT
{
	std::string ACTIVITY_NOTIFY[EMACTIVITY_NOTIFY_SIZE] =
	{
		"None",
		"Around",
		"AllServer",
	};

	std::string ACTIVITY_MID[EMACTIVITY_TYPE_SIZE] =
	{
		"Level Achievement",
		"Defeat Monster",
		"Defeat Player",
		"Reach Map",
		"Collect Item",
		"Consume Item",
		"Complete Activity",
		"Complete Quest",
		"Collect Point",
		"Obtain QBox",
	};
};


