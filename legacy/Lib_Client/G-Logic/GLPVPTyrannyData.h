/*!
 * \file GLPVPTyrannyData.h
 *
 * \author Juver
 * \date June 2017
 *
 * 
 */

#pragma once

#include <map>
#include <set>

#include "GLCharDefine.h"
#include "GLDefine.h"
#include "GLPVPTyrannyDefine.h"

struct TYRANNY_SCHED
{  
	WORD	wStartDay;		//schedule day
	WORD	wStartHour;		//schedule hour (24 hour format)
	WORD	wStartMinute;	//schedule minute

	TYRANNY_SCHED()
		: wStartDay(0)
		, wStartHour(0)
		, wStartMinute(0)
	{
	}

	bool operator < ( const TYRANNY_SCHED& sData )
	{			
		if ( wStartDay < sData.wStartDay ) 
			return true;

		else if ( wStartDay == sData.wStartDay && 
			wStartHour < sData.wStartHour ) 
			return true;
		
		else if ( wStartDay == sData.wStartDay && 
			wStartHour == sData.wStartHour &&
			wStartMinute < sData.wStartMinute ) 
			return true;

		return  false;
	}
};

struct TYRANNY_SCHED_NEXT
{
	DWORD	dwIndex;
	TYRANNY_SCHED	sStartSched;
	
	WORD		wPrevStartHour;
	WORD		wPrevStartMinute;
	WORD		wPrevEndHour;
	WORD		wPrevEndMinute;

	WORD		wBattleStartHour;
	WORD		wBattleStartMinute;
	WORD		wBattleEndHour;
	WORD		wBattleEndMinute;
	
	TYRANNY_SCHED_NEXT()
		: dwIndex(UINT_MAX)
		, wPrevStartHour(0)
		, wPrevStartMinute(0)
		, wPrevEndHour(0)
		, wPrevEndMinute(0)
		, wBattleStartHour(0)
		, wBattleStartMinute(0)
		, wBattleEndHour(0)
		, wBattleEndMinute(0)
	{
	};

	void CalculateEndTime( WORD wRegisterDuration, WORD wBattleDuration, WORD wRewardDuration )
	{
		wPrevStartHour = wBattleStartHour;
		wPrevStartMinute = wBattleStartMinute;
		wPrevEndHour = wBattleEndHour;
		wPrevEndMinute = wBattleEndMinute;

		wBattleStartHour = sStartSched.wStartHour;
		wBattleStartMinute = sStartSched.wStartMinute;
		wBattleEndHour = sStartSched.wStartHour;
		wBattleEndMinute = sStartSched.wStartMinute;

		WORD wtobattleinterval = wRegisterDuration/60;
		WORD woverallinterval = ( wRegisterDuration + wBattleDuration + wRewardDuration )/60;

		wBattleStartMinute += wtobattleinterval; 
		wBattleEndMinute += woverallinterval;

		while ( wBattleStartMinute >= 60 )
		{
			wBattleStartHour += 1;
			wBattleStartMinute -= 60;
		}

		while ( wBattleStartHour >= 24 )
		{
			wBattleStartHour -= 24;
		}

		while ( wBattleEndMinute >= 60 )
		{
			wBattleEndHour += 1;
			wBattleEndMinute -= 60;
		}

		while ( wBattleEndHour >= 24 )
		{
			wBattleEndHour -= 24;
		}
	}
};

struct TYRANNY_REGISTER_DATA
{  
	DWORD	dwInfoCharID;
	char	szInfoCharName[TYRANNY_CHARNAME_STRING_SIZE];
	WORD	wInfoSchool;
	WORD	wInfoClass;

	TYRANNY_REGISTER_DATA()
		: dwInfoCharID(TYRANNY_PLAYER_NULL)
		, wInfoSchool(0)
		, wInfoClass(0)
	{
		memset( szInfoCharName, 0, sizeof(char) * (TYRANNY_CHARNAME_STRING_SIZE) );
	}

	TYRANNY_REGISTER_DATA ( const TYRANNY_REGISTER_DATA &value )
	{
		operator=(value);
	}

	TYRANNY_REGISTER_DATA& operator = ( const TYRANNY_REGISTER_DATA& rvalue )
	{
		dwInfoCharID	= rvalue.dwInfoCharID;
		wInfoSchool		= rvalue.wInfoSchool;
		wInfoClass		= rvalue.wInfoClass;

		StringCchCopy( szInfoCharName, TYRANNY_CHARNAME_STRING_SIZE, rvalue.szInfoCharName );

		return *this;
	}
};

struct TYRANNY_REGISTER_DATA_QUEUE
{ 
	TYRANNY_REGISTER_DATA sPlayerData;
	WORD	wQueueNum;

	TYRANNY_REGISTER_DATA_QUEUE()
		: sPlayerData()
		, wQueueNum(0)
	{
	}
};

struct TYRANNY_PLAYER_DATA
{  
	//info
	DWORD	dwInfoCharID;
	char	szInfoCharName[TYRANNY_CHARNAME_STRING_SIZE];
	WORD	wInfoSchool;
	WORD	wInfoClass;

	//rank
	WORD	wRankAll;
	WORD	wRankSchool;

	//score
	WORD	wScoreKill;
	WORD	wScoreDeath;
	DWORD	dwScoreDamage;
	DWORD	dwScoreHeal;
	WORD	wScoreResu;
	DWORD	dwScoreTotal;
	WORD	wRewardPoint;

	TYRANNY_PLAYER_DATA()
		: dwInfoCharID(TYRANNY_PLAYER_NULL)
		, wInfoSchool(0)
		, wInfoClass(0)

		, wRankAll(0)
		, wRankSchool(0)

		, wScoreKill(0)
		, wScoreDeath(0)
		, dwScoreDamage(0)
		, dwScoreHeal(0)
		, wScoreResu(0)
		, dwScoreTotal(0)
		, wRewardPoint(0)
	{
		memset( szInfoCharName, 0, sizeof(char) * (TYRANNY_CHARNAME_STRING_SIZE) );
	}

	bool operator < ( const TYRANNY_PLAYER_DATA& sData )
	{			
		if ( dwScoreTotal > sData.dwScoreTotal )
			return true;
		else if ( dwScoreTotal == sData.dwScoreTotal && wScoreDeath < sData.wScoreDeath )	
			return true;

		return  false;
	}

	TYRANNY_PLAYER_DATA ( const TYRANNY_PLAYER_DATA &value )
	{
		operator=(value);
	}

	TYRANNY_PLAYER_DATA& operator = ( const TYRANNY_PLAYER_DATA& rvalue )
	{
		dwInfoCharID	= rvalue.dwInfoCharID;
		wInfoSchool		= rvalue.wInfoSchool;
		wInfoClass		= rvalue.wInfoClass;

		wRankAll		= rvalue.wRankAll;
		wRankSchool		= rvalue.wRankSchool;

		wScoreKill		= rvalue.wScoreKill;
		wScoreDeath		= rvalue.wScoreDeath;
		dwScoreDamage	= rvalue.dwScoreDamage;
		dwScoreHeal		= rvalue.dwScoreHeal;
		wScoreResu		= rvalue.wScoreResu;
		dwScoreTotal	= rvalue.dwScoreTotal;
		wRewardPoint	= rvalue.wRewardPoint;
		
		StringCchCopy( szInfoCharName, TYRANNY_CHARNAME_STRING_SIZE, rvalue.szInfoCharName );

		return *this;
	}
};

struct TYRANNY_TOWER_DATA
{ 
	SNATIVEID	sidCrow;
	WORD		wOwner;

	TYRANNY_TOWER_DATA()
		: sidCrow( NATIVEID_NULL().dwID)
		, wOwner(TYRANNY_SCHOOL_SIZE)
	{
	}
};

struct TYRANNY_SCORE_DATA
{ 
	WORD	wSchool;
	WORD	wCapture;
	WORD	wKills;
	WORD	wDeaths;

	TYRANNY_SCORE_DATA()
		: wSchool(TYRANNY_SCHOOL_SIZE)
		, wCapture(0)
		, wKills(0)
		, wDeaths(0)
	{
	}

	bool operator < ( const TYRANNY_SCORE_DATA& sData )
	{			
		if ( wCapture > sData.wCapture )
			return true;
		else if ( wCapture == sData.wCapture && wKills > sData.wKills )	
			return true;
		else if ( wCapture == sData.wCapture && wKills == sData.wKills && wDeaths < sData.wDeaths )	
			return true;

		return  false;
	}

	void Reset()
	{
		wCapture = 0;
		wKills = 0;
		wDeaths = 0;
	}
};

struct TYRANNY_REWARD_DATA
{ 
	DWORD		dwCharID;
	SNATIVEID	sItemReward;
	bool		bSent;

	TYRANNY_REWARD_DATA()
		: dwCharID(TYRANNY_PLAYER_NULL)
		, bSent( FALSE )
	{
	};
};

struct TYRANNY_BUFF_DATA
{ 
	SNATIVEID	sidSkill;
	WORD		wLevel;

	TYRANNY_BUFF_DATA()
		: sidSkill(NATIVEID_NULL())
		, wLevel(0)
	{
	};
};

struct TYRANNY_BUFF_DATA2
{ 
	SNATIVEID	sidSkill;
	WORD		wLevel[TYRANNY_BATTLE_BUFF_LEVEL_NUM];

	TYRANNY_BUFF_DATA2()
		: sidSkill(NATIVEID_NULL())
	{
		for( int i=0; i<TYRANNY_BATTLE_BUFF_LEVEL_NUM; ++i )
			wLevel[i] = 0;
	};
};

typedef std::vector<TYRANNY_SCHED>				TYRANNY_SCHED_VEC;

typedef std::map<DWORD,TYRANNY_REGISTER_DATA>		TYRANNY_REGISTER_DATA_MAP;
typedef TYRANNY_REGISTER_DATA_MAP::iterator			TYRANNY_REGISTER_DATA_MAP_ITER;
typedef std::vector<TYRANNY_REGISTER_DATA>			TYRANNY_REGISTER_DATA_VEC;
typedef std::vector<TYRANNY_REGISTER_DATA_QUEUE>	TYRANNY_REGISTER_DATA_QUEUE_VEC;

typedef std::map<DWORD,TYRANNY_PLAYER_DATA>		TYRANNY_PLAYER_DATA_MAP;
typedef TYRANNY_PLAYER_DATA_MAP::iterator		TYRANNY_PLAYER_DATA_MAP_ITER;
typedef std::vector<TYRANNY_PLAYER_DATA>		TYRANNY_PLAYER_DATA_VEC;

typedef std::vector<TYRANNY_SCORE_DATA>			TYRANNY_SCORE_DATA_VEC;
typedef std::map<DWORD,TYRANNY_REWARD_DATA>		TYRANNY_REWARD_MAP;
typedef TYRANNY_REWARD_MAP::iterator			TYRANNY_REWARD_MAP_ITER;

//  ________                  .____                         ._______  ___                                       
//  \______ \    ____  ___  __|    |      ____  _______   __| _/\   \/  /                                       
//   |    |  \ _/ __ \ \  \/ /|    |     /  _ \ \_  __ \ / __ |  \     /                                        
//   |    `   \\  ___/  \   / |    |___ (  <_> ) |  | \// /_/ |  /     \                                        
//  /_______  / \___  >  \_/  |_______ \ \____/  |__|   \____ | /___/\  \                                       
//          \/      \/                \/                     \/       \_/                                       
//  ________  _______    ____ _________                                                                         
//  \_____  \ \   _  \  /_   |\______  \                                                                        
//   /  ____/ /  /_\  \  |   |    /    /                                                                        
//  /       \ \  \_/   \ |   |   /    /                                                                         
//  \_______ \ \_____  / |___|  /____/                                                                          
//          \/ 