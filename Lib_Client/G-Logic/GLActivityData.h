/*!
 * \file GLActivityData.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */

#ifndef GLACTIVITY_DATA_H_
#define GLACTIVITY_DATA_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <map>

#include "../../Lib_Engine/Common/ByteStream.h"
#include "../../Lib_Engine/Common/SerialFile.h"

#include "GLActivityDefine.h"
#include "GLDefine.h"

struct SACTIVITY_FILE_DATA
{
	enum { VERSION = 0x0009 };

	DWORD	dwActivityID;
	std::string	strActivityTitle;
	std::string	strBadgeString;
	std::string strDescription;

	EMACTIVITY_TYPE		emType;
	EMACTIVITY_NOTIFY	emNotify;

	DWORD	dwRewardPoint;	
	BOOL	bRewardBadge;

	WORD	wProgressLevel;

	SNATIVEID sidMobKill;
	WORD	wProgressMobKill;

	SNATIVEID sidMapKill;
	WORD	wProgressMapKill;

	SNATIVEID sidMapReach;
	WORD	wProgressMapReach;

	SNATIVEID sidItemGet;
	WORD	wProgressItemGet;

	SNATIVEID sidItemUse;
	WORD	wProgressItemUse;

	DWORD	dwQBoxType;
	WORD	wQBoxProgress;

	DWORD	dwQuestID;
	WORD	wQuestProgress;

	DWORD	dwActivityProgress;
	
	SACTIVITY_FILE_DATA()
		: dwActivityID(UINT_MAX)
		, emType(EMACTIVITY_TYPE_REACH_LEVEL)
		, emNotify(EMACTIVITY_NOTIFY_NONE)
		, dwRewardPoint(0)
		, bRewardBadge(FALSE)
		, wProgressLevel(0)
		, sidMobKill(NATIVEID_NULL())
		, wProgressMobKill(0)
		, sidMapKill(NATIVEID_NULL())
		, wProgressMapKill(0)
		, sidMapReach(NATIVEID_NULL())
		, wProgressMapReach(0)
		, sidItemGet(NATIVEID_NULL())
		, wProgressItemGet(0)
		, sidItemUse(NATIVEID_NULL())
		, wProgressItemUse(0)
		, dwQBoxType(UINT_MAX)
		, wQBoxProgress(0)
		, dwQuestID(UINT_MAX)
		, wQuestProgress(0)
		, dwActivityProgress(0)
		
	{
		strActivityTitle = "";
		strBadgeString = "";
		strDescription = "";
	};	

	bool operator < ( const SACTIVITY_FILE_DATA& sData )
	{			
		if ( emType < sData.emType )
			return true;
		else if ( emType == sData.emType && dwActivityID < sData.dwActivityID )
			return true;

		return  false;
	}

	SACTIVITY_FILE_DATA& operator = ( const SACTIVITY_FILE_DATA& rvalue )
	{
		dwActivityID		= rvalue.dwActivityID;
		strActivityTitle	= rvalue.strActivityTitle;
		strBadgeString		= rvalue.strBadgeString;
		strDescription		= rvalue.strDescription;

		emType				= rvalue.emType;
		emNotify			= rvalue.emNotify;

		dwRewardPoint		= rvalue.dwRewardPoint;
		bRewardBadge		= rvalue.bRewardBadge;

		wProgressLevel		= rvalue.wProgressLevel;

		sidMobKill			= rvalue.sidMobKill;
		wProgressMobKill	= rvalue.wProgressMobKill;

		sidMapKill			= rvalue.sidMapKill;
		wProgressMapKill	= rvalue.wProgressMapKill;

		sidMapReach			= rvalue.sidMapReach;
		wProgressMapReach	= rvalue.wProgressMapReach;

		sidItemGet			= rvalue.sidItemGet;
		wProgressItemGet	= rvalue.wProgressItemGet;

		sidItemUse			= rvalue.sidItemUse;
		wProgressItemUse	= rvalue.wProgressItemUse;

		dwQBoxType			= rvalue.dwQBoxType;
		wQBoxProgress		= rvalue.wQBoxProgress;

		dwQuestID			= rvalue.dwQuestID;
		wQuestProgress		= rvalue.wQuestProgress;

		dwActivityProgress	= rvalue.dwActivityProgress;

		return *this;
	}

	BOOL LOAD ( basestream &SFile, BOOL bServer );
	BOOL SAVE ( CSerialFile &SFile );
};

typedef std::map<DWORD,SACTIVITY_FILE_DATA>		SACTIVITY_FILE_DATA_MAP;
typedef SACTIVITY_FILE_DATA_MAP::iterator		SACTIVITY_FILE_DATA_MAP_ITER;
typedef std::vector<SACTIVITY_FILE_DATA>		SACTIVITY_FILE_DATA_VEC;


struct SACTIVITY_CHAR_DATA
{
	enum { VERSION = 0x0001 };

	DWORD	dwActivityID;
	EMACTIVITY_TYPE	emType;
	SNATIVEID	sidProgress;
	DWORD	dwProgressNow;
	DWORD	dwProgressMax;

	SACTIVITY_CHAR_DATA()
		: dwActivityID(UINT_MAX)
		, emType(EMACTIVITY_TYPE_REACH_LEVEL)
		, dwProgressNow(0)
		, dwProgressMax(0)
	{
	};

	void Assign( SACTIVITY_FILE_DATA& sactivity_file_data );
	void Correction( SACTIVITY_FILE_DATA& sactivity_file_data );
};

typedef std::map<DWORD,SACTIVITY_CHAR_DATA>		SACTIVITY_CHAR_DATA_MAP;
typedef SACTIVITY_CHAR_DATA_MAP::iterator				SACTIVITY_CHAR_DATA_MAP_ITER;
typedef SACTIVITY_CHAR_DATA_MAP::const_iterator			SACTIVITY_CHAR_DATA_MAP_CITER;

#endif // GLACTIVITY_DATA_H_
