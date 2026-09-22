#include "stdafx.h"
#include "GLSummonData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void SUMMON_DATA_SKILL::Assign ( SKILL::SSKILL_SUMMON_DATA &sdata )
{
	sidCrow			= sdata.sidCrow;
	fDelay			= sdata.fDelay;
	fLife			= sdata.fLife;
	wSkillLevel		= sdata.wSkillLevel;
	wMaxSummon		= (WORD)sdata.dwMaxNum;
}