/*!
 * \file GLPVPSchoolWarsData.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */

#pragma once

#include <map>
#include <set>

#include "GLCharDefine.h"
#include "GLDefine.h"
#include "GLPVPSchoolWarsDefine.h"

struct SCHOOLWARS_SCHED
{  
	WORD	wStartDay;		//schedule day
	WORD	wStartHour;		//schedule hour (24 hour format)
	WORD	wStartMinute;	//schedule minute

	SCHOOLWARS_SCHED()
		: wStartDay(0)
		, wStartHour(0)
		, wStartMinute(0)
	{
	}

	bool operator < ( const SCHOOLWARS_SCHED& sData )
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

struct SCHOOLWARS_SCHED_NEXT
{
	DWORD	dwIndex;
	SCHOOLWARS_SCHED	sStartSched;
	
	WORD		wPrevStartHour;
	WORD		wPrevStartMinute;
	WORD		wPrevEndHour;
	WORD		wPrevEndMinute;

	WORD		wBattleStartHour;
	WORD		wBattleStartMinute;
	WORD		wBattleEndHour;
	WORD		wBattleEndMinute;
	
	SCHOOLWARS_SCHED_NEXT()
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

struct SCHOOLWARS_REGISTER_DATA
{  
	DWORD	dwInfoCharID;
	char	szInfoCharName[SCHOOLWARS_CHARNAME_STRING_SIZE];
	WORD	wInfoSchool;
	WORD	wInfoClass;

	SCHOOLWARS_REGISTER_DATA()
		: dwInfoCharID(SCHOOLWARS_PLAYER_NULL)
		, wInfoSchool(0)
		, wInfoClass(0)
	{
		memset( szInfoCharName, 0, sizeof(char) * (SCHOOLWARS_CHARNAME_STRING_SIZE) );
	}

	SCHOOLWARS_REGISTER_DATA ( const SCHOOLWARS_REGISTER_DATA &value )
	{
		operator=(value);
	}

	SCHOOLWARS_REGISTER_DATA& operator = ( const SCHOOLWARS_REGISTER_DATA& rvalue )
	{
		dwInfoCharID	= rvalue.dwInfoCharID;
		wInfoSchool		= rvalue.wInfoSchool;
		wInfoClass		= rvalue.wInfoClass;

		StringCchCopy( szInfoCharName, SCHOOLWARS_CHARNAME_STRING_SIZE, rvalue.szInfoCharName );

		return *this;
	}
};

struct SCHOOLWARS_REGISTER_DATA_QUEUE
{ 
	SCHOOLWARS_REGISTER_DATA sPlayerData;
	WORD	wQueueNum;

	SCHOOLWARS_REGISTER_DATA_QUEUE()
		: sPlayerData()
		, wQueueNum(0)
	{
	}
};

struct SCHOOLWARS_PLAYER_DATA
{  
	//info
	DWORD	dwInfoCharID;
	char	szInfoCharName[SCHOOLWARS_CHARNAME_STRING_SIZE];
	WORD	wInfoSchool;
	WORD	wInfoClass;

	//rank
	WORD	wRankAll;
	WORD	wRankSchool;

	//score
	WORD	wScoreKill;
	WORD	wScoreDeath;
	DWORD	dwScoreHeal;
	WORD	wScoreResu;
	DWORD	dwScoreTotal;
	WORD	wRewardPoint;

	SCHOOLWARS_PLAYER_DATA()
		: dwInfoCharID(SCHOOLWARS_PLAYER_NULL)
		, wInfoSchool(0)
		, wInfoClass(0)

		, wRankAll(0)
		, wRankSchool(0)

		, wScoreKill(0)
		, wScoreDeath(0)
		, dwScoreHeal(0)
		, wScoreResu(0)
		, dwScoreTotal(0)
		, wRewardPoint(0)
	{
		memset( szInfoCharName, 0, sizeof(char) * (SCHOOLWARS_CHARNAME_STRING_SIZE) );
	}

	bool operator < ( const SCHOOLWARS_PLAYER_DATA& sData )
	{			
		if ( dwScoreTotal > sData.dwScoreTotal )
			return true;
		else if ( dwScoreTotal == sData.dwScoreTotal && wScoreDeath < sData.wScoreDeath )	
			return true;

		return  false;
	}

	SCHOOLWARS_PLAYER_DATA ( const SCHOOLWARS_PLAYER_DATA &value )
	{
		operator=(value);
	}

	SCHOOLWARS_PLAYER_DATA& operator = ( const SCHOOLWARS_PLAYER_DATA& rvalue )
	{
		dwInfoCharID	= rvalue.dwInfoCharID;
		wInfoSchool		= rvalue.wInfoSchool;
		wInfoClass		= rvalue.wInfoClass;

		wRankAll		= rvalue.wRankAll;
		wRankSchool		= rvalue.wRankSchool;

		wScoreKill		= rvalue.wScoreKill;
		wScoreDeath		= rvalue.wScoreDeath;
		dwScoreHeal		= rvalue.dwScoreHeal;
		wScoreResu		= rvalue.wScoreResu;
		dwScoreTotal	= rvalue.dwScoreTotal;
		wRewardPoint	= rvalue.wRewardPoint;

		StringCchCopy( szInfoCharName, SCHOOLWARS_CHARNAME_STRING_SIZE, rvalue.szInfoCharName );

		return *this;
	}
};


struct SCHOOLWARS_SCORE_DATA
{ 
	WORD	wSchool;
	WORD	wKills;
	WORD	wDeaths;

	SCHOOLWARS_SCORE_DATA()
		: wSchool(SCHOOLWARS_SCHOOL_SIZE)
		, wKills(0)
		, wDeaths(0)
	{
	}

	bool operator < ( const SCHOOLWARS_SCORE_DATA& sData )
	{			
		if ( wKills > sData.wKills )
			return true;
		else if ( wKills == sData.wKills && wDeaths < sData.wDeaths )	
			return true;

		return  false;
	}

	void Reset()
	{
		wKills = 0;
		wDeaths = 0;
	}
};

struct SCHOOLWARS_REWARD_DATA
{ 
	DWORD		dwCharID;
	SNATIVEID	sItemReward;
	bool		bSent;

	SCHOOLWARS_REWARD_DATA()
		: dwCharID(SCHOOLWARS_PLAYER_NULL)
		, bSent( FALSE )
	{
	};
};


typedef std::vector<SCHOOLWARS_SCHED>					SCHOOLWARS_SCHED_VEC;
typedef std::map<DWORD,SCHOOLWARS_REGISTER_DATA>		SCHOOLWARS_REGISTER_DATA_MAP;
typedef SCHOOLWARS_REGISTER_DATA_MAP::iterator			SCHOOLWARS_REGISTER_DATA_MAP_ITER;
typedef std::vector<SCHOOLWARS_REGISTER_DATA>			SCHOOLWARS_REGISTER_DATA_VEC;
typedef std::vector<SCHOOLWARS_REGISTER_DATA_QUEUE>		SCHOOLWARS_REGISTER_DATA_QUEUE_VEC;

typedef std::map<DWORD,SCHOOLWARS_PLAYER_DATA>			SCHOOLWARS_PLAYER_DATA_MAP;
typedef SCHOOLWARS_PLAYER_DATA_MAP::iterator			SCHOOLWARS_PLAYER_DATA_MAP_ITER;
typedef std::vector<SCHOOLWARS_PLAYER_DATA>				SCHOOLWARS_PLAYER_DATA_VEC;

typedef std::vector<SCHOOLWARS_SCORE_DATA>				SCHOOLWARS_SCORE_DATA_VEC;
typedef std::map<DWORD,SCHOOLWARS_REWARD_DATA>			SCHOOLWARS_REWARD_MAP;
typedef SCHOOLWARS_REWARD_MAP::iterator					SCHOOLWARS_REWARD_MAP_ITER;

//
// /$$$$$$$  /$$$$$$$$ /$$    /$$ /$$        /$$$$$$  /$$$$$$$  /$$$$$$$  /$$   /$$
//| $$__  $$| $$_____/| $$   | $$| $$       /$$__  $$| $$__  $$| $$__  $$| $$  / $$
//| $$  \ $$| $$      | $$   | $$| $$      | $$  \ $$| $$  \ $$| $$  \ $$|  $$/ $$/
//| $$  | $$| $$$$$   |  $$ / $$/| $$      | $$  | $$| $$$$$$$/| $$  | $$ \  $$$$/ 
//| $$  | $$| $$__/    \  $$ $$/ | $$      | $$  | $$| $$__  $$| $$  | $$  >$$  $$ 
//| $$  | $$| $$        \  $$$/  | $$      | $$  | $$| $$  \ $$| $$  | $$ /$$/\  $$
//| $$$$$$$/| $$$$$$$$   \  $/   | $$$$$$$$|  $$$$$$/| $$  | $$| $$$$$$$/| $$  \ $$
//|_______/ |________/    \_/    |________/ \______/ |__/  |__/|_______/ |__/  |__/
//
//
//
//  /$$$$$$   /$$$$$$    /$$    /$$$$$$                                            
// /$$__  $$ /$$$_  $$ /$$$$   /$$__  $$                                           
//|__/  \ $$| $$$$\ $$|_  $$  | $$  \ $$                                           
//  /$$$$$$/| $$ $$ $$  | $$  |  $$$$$$/                                           
// /$$____/ | $$\ $$$$  | $$   >$$__  $$                                           
//| $$      | $$ \ $$$  | $$  | $$  \ $$                                           
//| $$$$$$$$|  $$$$$$/ /$$$$$$|  $$$$$$/                                           
//|________/ \______/ |______/ \______/                                            
//\/ 
//
