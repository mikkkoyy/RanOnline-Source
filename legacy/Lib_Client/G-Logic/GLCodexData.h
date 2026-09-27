/*!
 * \file GLCodexData.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */

#ifndef GLCODEX_DATA_H_
#define GLCODEX_DATA_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <map>

#include "../../Lib_Engine/Common/ByteStream.h"
#include "../../Lib_Engine/Common/SerialFile.h"

#include "GLCodexDefine.h"
#include "GLDefine.h"

struct SCODEX_FILE_DATA
{
	enum { VERSION = 0x0010 };

	DWORD	dwCodexID;
	std::string	strCodexTitle;
	std::string	strBadgeString;
	std::string strDescription;

	EMCODEX_TYPE		emType;
	EMCODEX_NOTIFY	emNotify;

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

	DWORD	dwCodexProgress;
	
	
	WORD	wItemGrade1;
	WORD	wItemGrade2;
	WORD	wItemGrade3;
	WORD	wItemGrade4;
	WORD	wItemGrade5;
	
	SCODEX_FILE_DATA()
		: dwCodexID(UINT_MAX)
		, emType(EMCODEX_TYPE_REACH_LEVEL)
		, emNotify(EMCODEX_NOTIFY_NONE)
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
		, dwCodexProgress(0)
		
		, wItemGrade1(0)
		, wItemGrade2(0)
		, wItemGrade3(0)
		, wItemGrade4(0)
		, wItemGrade5(0)
		
	{
		strCodexTitle = "";
		strBadgeString = "";
		strDescription = "";
	};	

	bool operator < ( const SCODEX_FILE_DATA& sData )
	{			
		if ( emType < sData.emType )
			return true;
		else if ( emType == sData.emType && dwCodexID < sData.dwCodexID )
			return true;

		return  false;
	}

	SCODEX_FILE_DATA& operator = ( const SCODEX_FILE_DATA& rvalue )
	{
		dwCodexID		= rvalue.dwCodexID;
		strCodexTitle	= rvalue.strCodexTitle;
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

		dwCodexProgress	= rvalue.dwCodexProgress;
		
		wItemGrade1	= rvalue.wItemGrade1;
		wItemGrade2	= rvalue.wItemGrade2;
		wItemGrade3	= rvalue.wItemGrade3;
		wItemGrade4	= rvalue.wItemGrade4;
		wItemGrade5	= rvalue.wItemGrade5;

		return *this;
	}

	BOOL LOAD ( basestream &SFile, BOOL bServer );
	BOOL SAVE ( CSerialFile &SFile );
	

	static void SaveCsvHead( std::fstream &SFile );
	void SaveCsv( std::fstream &SFile );
	void LoadCsv( CStringArray &StrArray );
};

typedef std::map<DWORD,SCODEX_FILE_DATA>		SCODEX_FILE_DATA_MAP;
typedef SCODEX_FILE_DATA_MAP::iterator		SCODEX_FILE_DATA_MAP_ITER;
typedef std::vector<SCODEX_FILE_DATA>		SCODEX_FILE_DATA_VEC;


struct SCODEX_CHAR_DATA
{
	enum { VERSION = 0x0001 };

	DWORD	dwCodexID;
	EMCODEX_TYPE	emType;
	SNATIVEID	sidProgress;
	DWORD	dwProgressNow;
	DWORD	dwProgressMax;
	
	SNATIVEID	sidProgressItem1;
	SNATIVEID	sidProgressItem2;
	SNATIVEID	sidProgressItem3;
	SNATIVEID	sidProgressItem4;
	SNATIVEID	sidProgressItem5;
	
	DWORD	dwProgressItemDone1;
	DWORD	dwProgressItemDone2;
	DWORD	dwProgressItemDone3;
	DWORD	dwProgressItemDone4;
	DWORD	dwProgressItemDone5;

	WORD	wQuantity1;
	WORD	wQuantity2;
	WORD	wQuantity3;
	WORD	wQuantity4;
	WORD	wQuantity5;
	
	WORD	wItemGrade1;
	WORD	wItemGrade2;
	WORD	wItemGrade3;
	WORD	wItemGrade4;
	WORD	wItemGrade5;

	SCODEX_CHAR_DATA()
		: dwCodexID(UINT_MAX)
		, emType(EMCODEX_TYPE_REACH_LEVEL)
		, dwProgressNow(0)
		, dwProgressMax(0)
		
		, dwProgressItemDone1(0)
		, dwProgressItemDone2(0)
		, dwProgressItemDone3(0)
		, dwProgressItemDone4(0)
		, dwProgressItemDone5(0)

		, wQuantity1(0)
		, wQuantity2(0)
		, wQuantity3(0)
		, wQuantity4(0)
		, wQuantity5(0)
		
		, wItemGrade1(0)
		, wItemGrade2(0)
		, wItemGrade3(0)
		, wItemGrade4(0)
		, wItemGrade5(0)
	{
	};

	void Assign( SCODEX_FILE_DATA& sactivity_file_data );
	void Correction( SCODEX_FILE_DATA& sactivity_file_data );
};

typedef std::map<DWORD,SCODEX_CHAR_DATA>		SCODEX_CHAR_DATA_MAP;
typedef SCODEX_CHAR_DATA_MAP::iterator				SCODEX_CHAR_DATA_MAP_ITER;
typedef SCODEX_CHAR_DATA_MAP::const_iterator			SCODEX_CHAR_DATA_MAP_CITER;

#endif // GLCODEX_DATA_H_
