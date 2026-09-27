#ifndef GLSUMMONDEF_H_
#define GLSUMMONDEF_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>

enum SUMMON_TYPE
{
	SUMMON_TYPE_NONE    = -1,
	SUMMON_TYPE_A		= 0,
	SUMMON_TYPE_B		= 1,
	SUMMON_TYPE_C		= 2,
	SUMMON_TYPE_D		= 3,
	SUMMON_TYPE_SIZE    = 4,
};

namespace COMMENT
{
	extern std::string SUMMON_TYPE[SUMMON_TYPE_SIZE];
};

#endif // GLSUMMONDEF_H_
