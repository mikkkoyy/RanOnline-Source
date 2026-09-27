//8-3-2016

#include "GLCharDefine.h"
#include <set>
#include <map>

#pragma once

#define PKRANK_NUM			10 
#define PKNOTIF_NUM			10
#define HISTORY_TIMER		7

struct SPK_RANK
{
	int		nIndex;
	WORD	wRank;
	DWORD	dwCharID;
	char	szCharName[CHAR_SZNAME];
	BYTE	cClass;
	BYTE	cSchool;
	WORD	wKills;
	WORD	wDeaths;

	SPK_RANK()
		: nIndex( -1 )
		, wRank( 0 )
		, dwCharID( GAEAID_NULL )
		, cClass( 0 )
		, cSchool( 0 )
		, wKills( 0 )
		, wDeaths( 0 )
	{
		memset( szCharName, 0, sizeof(char) * (CHAR_SZNAME +1) );
	}

	SPK_RANK ( const SPK_RANK &value )
	{
		operator=(value);
	}

	SPK_RANK& operator = ( const SPK_RANK& rvalue )
	{
		nIndex		= rvalue.nIndex;
		wRank		= rvalue.wRank;
		dwCharID	= rvalue.dwCharID;
		cClass		= rvalue.cClass;
		cSchool		= rvalue.cSchool;
		wKills		= rvalue.wKills;
		wDeaths		= rvalue.wDeaths;
		StringCchCopy( szCharName, CHAR_SZNAME+1, rvalue.szCharName );

		return *this;
	}

	bool operator < ( const SPK_RANK& sRank )
	{			
		if ( wKills > sRank.wKills ) return true;
		else if ( wKills == sRank.wKills && wDeaths < sRank.wDeaths ) return true;
		return  false;
	}

	bool operator == ( const SPK_RANK& sRank ) 
	{
		if ( wKills == sRank.wKills && wDeaths == sRank.wDeaths ) return true;
		return false;

	}
};

typedef std::map< DWORD, SPK_RANK >			PK_RANK_PLAYER_MAP;
typedef PK_RANK_PLAYER_MAP::iterator		PK_RANK_PLAYER_MAP_ITER;

typedef std::vector<SPK_RANK>				PK_RANK_PLAYER_VEC;
typedef PK_RANK_PLAYER_VEC::iterator		PK_RANK_PLAYER_VEC_ITER;


struct SPK_RANK_SELF
{
	DWORD	dwCharID;
	WORD	wRank;
	WORD	wKills;
	WORD	wDeaths;
	BOOL	bValid;

	SPK_RANK_SELF()
		: dwCharID( GAEAID_NULL )
		, wRank( 0 )
		, wKills( 0 )
		, wDeaths( 0 )
		, bValid( FALSE )
	{
	}

	void RESET()
	{
		dwCharID = GAEAID_NULL;
		wRank	= 0;
		wKills	= 0;
		wDeaths	= 0;
		bValid	= FALSE;
	}
};

struct SPK_HISTORY
{
	DWORD dwKiller;
	DWORD dwKilled;
	char szCharKiller[CHAR_SZNAME];
	char szCharKilled[CHAR_SZNAME];
	BYTE cClassKiller;
	BYTE cClassKilled;
	BYTE cSchoolKiller;
	BYTE cSchoolKilled;
	float fTimer;

	SPK_HISTORY()
		: dwKiller( GAEAID_NULL )
		, dwKilled( GAEAID_NULL )
		, cClassKiller( 0 )
		, cClassKilled( 0 )
		, cSchoolKiller( 0 )
		, cSchoolKilled( 0 )
		, fTimer( 0.0f )
	{
		memset( szCharKiller, 0, sizeof(char) * (CHAR_SZNAME +1) );
		memset( szCharKilled, 0, sizeof(char) * (CHAR_SZNAME +1) );
	};
};

typedef std::vector<SPK_HISTORY>			PK_HISTORY_VEC;
typedef PK_HISTORY_VEC::iterator			PK_HISTORY_VEC_ITER;


struct SPK_REWARD
{
	BOOL		bEnable;
	SNATIVEID	sITEMREWARD[PKRANK_NUM];
	SNATIVEID	sITEMREWARDETC;
	WORD		wMinimumKill;

	SPK_REWARD()
		: bEnable( FALSE )
		, wMinimumKill(0)
	{

	};
};

/*
$$$$$$$\                       $$\                                 $$\ $$\   $$\ 
$$  __$$\                      $$ |                                $$ |$$ |  $$ |
$$ |  $$ | $$$$$$\  $$\    $$\ $$ |       $$$$$$\   $$$$$$\   $$$$$$$ |\$$\ $$  |
$$ |  $$ |$$  __$$\ \$$\  $$  |$$ |      $$  __$$\ $$  __$$\ $$  __$$ | \$$$$  / 
$$ |  $$ |$$$$$$$$ | \$$\$$  / $$ |      $$ /  $$ |$$ |  \__|$$ /  $$ | $$  $$<  
$$ |  $$ |$$   ____|  \$$$  /  $$ |      $$ |  $$ |$$ |      $$ |  $$ |$$  /\$$\ 
$$$$$$$  |\$$$$$$$\    \$  /   $$$$$$$$\ \$$$$$$  |$$ |      \$$$$$$$ |$$ /  $$ |
\_______/  \_______|    \_/    \________| \______/ \__|       \_______|\__|  \__|
*/