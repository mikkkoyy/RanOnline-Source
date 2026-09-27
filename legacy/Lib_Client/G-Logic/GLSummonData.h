#ifndef GLSUMMONDDATA_H_
#define GLSUMMONDDATA_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GLCharDefine.h"
#include "GLSkillSpecial.h"
#include "GLSkillBasic.h"
#include "GLDefine.h"

#define  SKILL_SUMMON_SCHOOL_NULL 10
#define  SKILL_SUMMON_EFFECT_TIME 10.0f			//maximum effect time in client to prevent stuck
#define  SKILL_SUMMON_MAX_CLIENT_NUM 10			//maximum summon per char	
#define  SUMMON_VALID_CHECK_TIME 2.0f			//check for summon/owner validity

struct SUMMON_DATA_SKILL
{
	BOOL		bSummon;
	STARID		sTarget;		//target

	SNATIVEID	sidSkill;		//caster skill
	SNATIVEID	sidCrow;		//summon crow
	float		fDelay;			//time to summon
	float		fLife;			//summon lifetime
	WORD		wSkillLevel;	//summon skill level
	WORD		wMaxSummon;		//maximum summon for this skill

	SUMMON_DATA_SKILL() 
		: bSummon(FALSE)		
		, sidSkill(NATIVEID_NULL())
		, sidCrow(NATIVEID_NULL())
		, fDelay(0.0f)
		, fLife(1.0f)
		, wSkillLevel(0)
		, wMaxSummon(1)
	{
	}

	SUMMON_DATA_SKILL ( const SUMMON_DATA_SKILL &value )
	{
		operator=(value);
	}

	SUMMON_DATA_SKILL& operator = ( const SUMMON_DATA_SKILL& rvalue )
	{
		bSummon			= rvalue.bSummon;
		sTarget			= rvalue.sTarget;

		sidSkill		= rvalue.sidSkill;
		sidCrow			= rvalue.sidCrow;
		fDelay			= rvalue.fDelay;
		fLife			= rvalue.fLife;
		wSkillLevel		= rvalue.wSkillLevel;
		wMaxSummon		= rvalue.wMaxSummon;

		return *this;
	}

	void Assign ( SKILL::SSKILL_SUMMON_DATA &sdata );

	void Reset()
	{
		bSummon			= FALSE;
		sTarget			= STARID();

		sidSkill		= NATIVEID_NULL();
		sidCrow			= NATIVEID_NULL();
		fDelay			= 0.0f;
		fLife			= 1.0f;
		wSkillLevel		= 0;
		wMaxSummon		= 1;
	}
};


#endif // GLSUMMONDDATA_H_
