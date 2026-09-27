/*!
 * \file GLPVPCaptureTheFlagData.h
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
#include "GLPVPCaptureTheFlagDefine.h"

struct CAPTURE_THE_FLAG_SCHED
{  
	WORD	wStartDay;		//schedule day
	WORD	wStartHour;		//schedule hour (24 hour format)
	WORD	wStartMinute;	//schedule minute

	CAPTURE_THE_FLAG_SCHED()
		: wStartDay(0)
		, wStartHour(0)
		, wStartMinute(0)
	{
	}

	bool operator < ( const CAPTURE_THE_FLAG_SCHED& sData )
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

struct CAPTURE_THE_FLAG_SCHED_NEXT
{
	DWORD	dwIndex;
	CAPTURE_THE_FLAG_SCHED	sStartSched;
	
	WORD		wPrevStartHour;
	WORD		wPrevStartMinute;
	WORD		wPrevEndHour;
	WORD		wPrevEndMinute;

	WORD		wBattleStartHour;
	WORD		wBattleStartMinute;
	WORD		wBattleEndHour;
	WORD		wBattleEndMinute;
	
	CAPTURE_THE_FLAG_SCHED_NEXT()
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

struct CAPTURE_THE_FLAG_REGISTER_DATA
{  
	DWORD	dwInfoCharID;
	char	szInfoCharName[CAPTURE_THE_FLAG_CHARNAME_STRING_SIZE];
	WORD	wInfoSchool;
	WORD	wInfoClass;
	WORD	wInfoTeam;

	CAPTURE_THE_FLAG_REGISTER_DATA()
		: dwInfoCharID(CAPTURE_THE_FLAG_PLAYER_NULL)
		, wInfoSchool(0)
		, wInfoClass(0)
		, wInfoTeam(CAPTURE_THE_FLAG_TEAM_SIZE)
	{
		memset( szInfoCharName, 0, sizeof(char) * (CAPTURE_THE_FLAG_CHARNAME_STRING_SIZE) );
	}

	CAPTURE_THE_FLAG_REGISTER_DATA ( const CAPTURE_THE_FLAG_REGISTER_DATA &value )
	{
		operator=(value);
	}

	CAPTURE_THE_FLAG_REGISTER_DATA& operator = ( const CAPTURE_THE_FLAG_REGISTER_DATA& rvalue )
	{
		dwInfoCharID	= rvalue.dwInfoCharID;
		wInfoSchool		= rvalue.wInfoSchool;
		wInfoClass		= rvalue.wInfoClass;
		wInfoTeam		= rvalue.wInfoTeam;

		StringCchCopy( szInfoCharName, CAPTURE_THE_FLAG_CHARNAME_STRING_SIZE, rvalue.szInfoCharName );

		return *this;
	}
};

struct CAPTURE_THE_FLAG_PLAYER_DATA
{  
	//info
	DWORD	dwInfoCharID;
	char	szInfoCharName[CAPTURE_THE_FLAG_CHARNAME_STRING_SIZE];
	WORD	wInfoSchool;
	WORD	wInfoClass;
	WORD	wInfoTeam;

	//rank
	WORD	wRankAll;
	WORD	wRankTeam;

	//score
	WORD	wScoreFlag;
	WORD	wScoreKill;
	WORD	wScoreDeath;
	DWORD	dwScoreDamage;
	DWORD	dwScoreHeal;
	WORD	wScoreResu;

	DWORD	dwScoreTotal;
	WORD	wRewardPoint;

	CAPTURE_THE_FLAG_PLAYER_DATA()
		: dwInfoCharID(CAPTURE_THE_FLAG_PLAYER_NULL)
		, wInfoSchool(0)
		, wInfoClass(0)
		, wInfoTeam(CAPTURE_THE_FLAG_TEAM_SIZE)

		, wRankAll(0)
		, wRankTeam(0)

		, wScoreFlag(0)
		, wScoreKill(0)
		, wScoreDeath(0)
		, dwScoreDamage(0)
		, dwScoreHeal(0)
		, wScoreResu(0)

		, dwScoreTotal(0)
		, wRewardPoint(0)
	{
		memset( szInfoCharName, 0, sizeof(char) * (CAPTURE_THE_FLAG_CHARNAME_STRING_SIZE) );
	}

	bool operator < ( const CAPTURE_THE_FLAG_PLAYER_DATA& sData )
	{			
		if ( dwScoreTotal > sData.dwScoreTotal )
			return true;
		else if ( dwScoreTotal == sData.dwScoreTotal && wScoreDeath < sData.wScoreDeath )	
			return true;

		return  false;
	}

	CAPTURE_THE_FLAG_PLAYER_DATA ( const CAPTURE_THE_FLAG_PLAYER_DATA &value )
	{
		operator=(value);
	}

	CAPTURE_THE_FLAG_PLAYER_DATA& operator = ( const CAPTURE_THE_FLAG_PLAYER_DATA& rvalue )
	{
		dwInfoCharID	= rvalue.dwInfoCharID;
		wInfoSchool		= rvalue.wInfoSchool;
		wInfoClass		= rvalue.wInfoClass;
		wInfoTeam		= rvalue.wInfoTeam;

		wRankAll		= rvalue.wRankAll;
		wRankTeam		= rvalue.wRankTeam;

		wScoreFlag		= rvalue.wScoreFlag;
		wScoreKill		= rvalue.wScoreKill;
		wScoreDeath		= rvalue.wScoreDeath;
		dwScoreDamage	= rvalue.dwScoreDamage;
		dwScoreHeal		= rvalue.dwScoreHeal;
		wScoreResu		= rvalue.wScoreResu;

		dwScoreTotal	= rvalue.dwScoreTotal;
		wRewardPoint	= rvalue.wRewardPoint;
		
		StringCchCopy( szInfoCharName, CAPTURE_THE_FLAG_CHARNAME_STRING_SIZE, rvalue.szInfoCharName );

		return *this;
	}
};

struct CAPTURE_THE_FLAG_SCORE_DATA
{ 
	WORD	wTeam;
	WORD	wCapture;
	WORD	wKills;
	WORD	wDeaths;

	CAPTURE_THE_FLAG_SCORE_DATA()
		: wTeam(CAPTURE_THE_FLAG_TEAM_SIZE)
		, wCapture(0)
		, wKills(0)
		, wDeaths(0)
	{
	}

	bool operator < ( const CAPTURE_THE_FLAG_SCORE_DATA& sData )
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

struct CAPTURE_THE_FLAG_REWARD_DATA
{ 
	DWORD		dwCharID;
	SNATIVEID	sItemReward;
	bool		bSent;

	CAPTURE_THE_FLAG_REWARD_DATA()
		: dwCharID(CAPTURE_THE_FLAG_PLAYER_NULL)
		, bSent( FALSE )
	{
	};
};


typedef std::vector<CAPTURE_THE_FLAG_SCHED>					CAPTURE_THE_FLAG_SCHED_VEC;

typedef std::map<DWORD,CAPTURE_THE_FLAG_REGISTER_DATA>		CAPTURE_THE_FLAG_REGISTER_DATA_MAP;
typedef CAPTURE_THE_FLAG_REGISTER_DATA_MAP::iterator		CAPTURE_THE_FLAG_REGISTER_DATA_MAP_ITER;
typedef std::vector<CAPTURE_THE_FLAG_REGISTER_DATA>			CAPTURE_THE_FLAG_REGISTER_DATA_VEC;

typedef std::map<DWORD,CAPTURE_THE_FLAG_PLAYER_DATA>		CAPTURE_THE_FLAG_PLAYER_DATA_MAP;
typedef CAPTURE_THE_FLAG_PLAYER_DATA_MAP::iterator			CAPTURE_THE_FLAG_PLAYER_DATA_MAP_ITER;
typedef std::vector<CAPTURE_THE_FLAG_PLAYER_DATA>			CAPTURE_THE_FLAG_PLAYER_DATA_VEC;

typedef std::vector<CAPTURE_THE_FLAG_SCORE_DATA>			CAPTURE_THE_FLAG_SCORE_DATA_VEC;
typedef std::map<DWORD,CAPTURE_THE_FLAG_REWARD_DATA>		CAPTURE_THE_FLAG_REWARD_MAP;
typedef CAPTURE_THE_FLAG_REWARD_MAP::iterator				CAPTURE_THE_FLAG_REWARD_MAP_ITER;

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
