#include "StdAfx.h"
#include "GLActivityData.h"

#include "../../Lib_Engine/Common/IniLoader.h"
#include "../../Lib_Engine/G-Logic/GLogic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



BOOL SACTIVITY_FILE_DATA::LOAD ( basestream &SFile, BOOL bServer )
{
	DWORD dwVer(0);
	SFile >> dwVer;

	if ( dwVer == VERSION )
	{
		SFile >> dwActivityID;
		SFile >> strActivityTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMACTIVITY_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMACTIVITY_NOTIFY)dwData;
		
		SFile >> dwRewardPoint;	
		SFile >> bRewardBadge;

		SFile >> wProgressLevel;

		SFile >> sidMobKill.dwID;
		SFile >> wProgressMobKill;

		SFile >> sidMapKill.dwID;
		SFile >> wProgressMapKill;

		SFile >> sidMapReach.dwID;
		SFile >> wProgressMapReach;

		SFile >> sidItemGet.dwID;
		SFile >> wProgressItemGet;

		SFile >> sidItemUse.dwID;
		SFile >> wProgressItemUse;

		SFile >> dwQBoxType;
		SFile >> wQBoxProgress;

		SFile >> dwQuestID;
		SFile >> wQuestProgress;

		SFile >> dwActivityProgress;
	}
	else if ( dwVer == 0x0008 )
	{
		SFile >> dwActivityID;
		SFile >> strActivityTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMACTIVITY_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMACTIVITY_NOTIFY)dwData;

		SFile >> dwRewardPoint;	
		SFile >> bRewardBadge;

		SFile >> wProgressLevel;

		SFile >> sidMobKill.dwID;
		SFile >> wProgressMobKill;

		SFile >> sidMapKill.dwID;
		SFile >> wProgressMapKill;

		SFile >> sidMapReach.dwID;
		SFile >> wProgressMapReach;

		SFile >> sidItemGet.dwID;
		SFile >> wProgressItemGet;

		SFile >> sidItemUse.dwID;
		SFile >> wProgressItemUse;

		SFile >> dwQBoxType;
		SFile >> wQBoxProgress;

		SFile >> dwQuestID;
		SFile >> wQuestProgress;
	}
	else if ( dwVer == 0x0007 )
	{
		SFile >> dwActivityID;
		SFile >> strActivityTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMACTIVITY_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMACTIVITY_NOTIFY)dwData;

		SFile >> dwRewardPoint;	
		SFile >> bRewardBadge;

		SFile >> wProgressLevel;

		SFile >> sidMobKill.dwID;
		SFile >> wProgressMobKill;

		SFile >> sidMapKill.dwID;
		SFile >> wProgressMapKill;

		SFile >> sidMapReach.dwID;
		SFile >> wProgressMapReach;

		SFile >> sidItemGet.dwID;
		SFile >> wProgressItemGet;

		SFile >> sidItemUse.dwID;
		SFile >> wProgressItemUse;

		SFile >> dwQBoxType;
		SFile >> wQBoxProgress;
	}
	else if ( dwVer == 0x0006 )
	{
		SFile >> dwActivityID;
		SFile >> strActivityTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMACTIVITY_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMACTIVITY_NOTIFY)dwData;

		SFile >> dwRewardPoint;	
		SFile >> bRewardBadge;

		SFile >> wProgressLevel;

		SFile >> sidMobKill.dwID;
		SFile >> wProgressMobKill;

		SFile >> sidMapKill.dwID;
		SFile >> wProgressMapKill;

		SFile >> sidMapReach.dwID;
		SFile >> wProgressMapReach;

		SFile >> sidItemGet.dwID;
		SFile >> wProgressItemGet;

		SFile >> sidItemUse.dwID;
		SFile >> wProgressItemUse;
	}
	else if ( dwVer == 0x0005 )
	{
		SFile >> dwActivityID;
		SFile >> strActivityTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMACTIVITY_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMACTIVITY_NOTIFY)dwData;

		SFile >> dwRewardPoint;	
		SFile >> bRewardBadge;

		SFile >> wProgressLevel;

		SFile >> sidMobKill.dwID;
		SFile >> wProgressMobKill;

		SFile >> sidMapKill.dwID;
		SFile >> wProgressMapKill;

		SFile >> sidMapReach.dwID;
		SFile >> wProgressMapReach;

		SFile >> sidItemGet.dwID;
		SFile >> wProgressItemGet;
	}
	else if ( dwVer == 0x0004 )
	{
		SFile >> dwActivityID;
		SFile >> strActivityTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMACTIVITY_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMACTIVITY_NOTIFY)dwData;

		SFile >> dwRewardPoint;	
		SFile >> bRewardBadge;

		SFile >> wProgressLevel;

		SFile >> sidMobKill.dwID;
		SFile >> wProgressMobKill;

		SFile >> sidMapKill.dwID;
		SFile >> wProgressMapKill;

		SFile >> sidMapReach.dwID;
		SFile >> wProgressMapReach;
	}
	else if ( dwVer == 0x0003 )
	{
		SFile >> dwActivityID;
		SFile >> strActivityTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMACTIVITY_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMACTIVITY_NOTIFY)dwData;

		SFile >> dwRewardPoint;	
		SFile >> bRewardBadge;

		SFile >> wProgressLevel;

		SFile >> sidMobKill.dwID;
		SFile >> wProgressMobKill;

		SFile >> sidMapKill.dwID;
		SFile >> wProgressMapKill;
	}
	else if ( dwVer == 0x0002 )
	{
		SFile >> dwActivityID;
		SFile >> strActivityTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMACTIVITY_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMACTIVITY_NOTIFY)dwData;

		SFile >> dwRewardPoint;	
		SFile >> bRewardBadge;

		SFile >> wProgressLevel;

		SFile >> sidMobKill.dwID;
		SFile >> wProgressMobKill;
	}
	else if ( dwVer == 0x0001 )
	{
		SFile >> dwActivityID;
		SFile >> strActivityTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMACTIVITY_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMACTIVITY_NOTIFY)dwData;

		SFile >> dwRewardPoint;	
		SFile >> bRewardBadge;

		SFile >> wProgressLevel;
	}
	else
	{
		CDebugSet::ErrorVersion( "SACTIVITY_FILE_DATA::LOAD", dwVer );
	}

	return TRUE;
}

BOOL SACTIVITY_FILE_DATA::SAVE ( CSerialFile &SFile )
{
	SFile << (DWORD)VERSION;

	SFile << dwActivityID;
	SFile << strActivityTitle;
	SFile << strBadgeString;
	SFile << strDescription;

	SFile << (DWORD)emType;
	SFile << (DWORD)emNotify;

	SFile << dwRewardPoint;	
	SFile << bRewardBadge;

	SFile << wProgressLevel;

	SFile << sidMobKill.dwID;
	SFile << wProgressMobKill;

	SFile << sidMapKill.dwID;
	SFile << wProgressMapKill;

	SFile << sidMapReach.dwID;
	SFile << wProgressMapReach;

	SFile << sidItemGet.dwID;
	SFile << wProgressItemGet;

	SFile << sidItemUse.dwID;
	SFile << wProgressItemUse;

	SFile << dwQBoxType;
	SFile << wQBoxProgress;

	SFile << dwQuestID;
	SFile << wQuestProgress;

	SFile << dwActivityProgress;

	return TRUE;
}

void SACTIVITY_CHAR_DATA::Assign( SACTIVITY_FILE_DATA& sactivity_file_data )
{
	dwActivityID = sactivity_file_data.dwActivityID;
	emType = sactivity_file_data.emType;

	switch ( emType )
	{
	case EMACTIVITY_TYPE_REACH_LEVEL:
		{
			dwProgressNow = 1;
			dwProgressMax = sactivity_file_data.wProgressLevel;
		}break;
	case EMACTIVITY_TYPE_KILL_MOB:
		{
			dwProgressNow = 0;
			dwProgressMax = sactivity_file_data.wProgressMobKill;
			sidProgress	 = sactivity_file_data.sidMobKill;
		}break;
	case EMACTIVITY_TYPE_KILL_PLAYER:
		{
			dwProgressNow = 0;
			dwProgressMax = sactivity_file_data.wProgressMapKill;
			sidProgress	 = sactivity_file_data.sidMapKill;
		}break;
	case EMACTIVITY_TYPE_REACH_MAP:
		{
			dwProgressNow = 0;
			dwProgressMax = sactivity_file_data.wProgressMapReach;
			sidProgress	 = sactivity_file_data.sidMapReach;
		}break;
	case EMACTIVITY_TYPE_TAKE_ITEM:
		{
			dwProgressNow = 0;
			dwProgressMax = sactivity_file_data.wProgressItemGet;
			sidProgress	 = sactivity_file_data.sidItemGet;
		}break;
	case EMACTIVITY_TYPE_USE_ITEM:
		{
			dwProgressNow = 0;
			dwProgressMax = sactivity_file_data.wProgressItemUse;
			sidProgress	 = sactivity_file_data.sidItemUse;
		}break;
	case EMACTIVITY_TYPE_REACH_ACTIVITY:
		{

		}break;
	case EMACTIVITY_TYPE_COMPLETE_QUEST:
		{
			dwProgressNow = 0;
			dwProgressMax = sactivity_file_data.wQuestProgress;
			sidProgress.dwID = sactivity_file_data.dwQuestID;
		}break;
	case EMACTIVITY_TYPE_ACTIVITY_POINT:
		{
			dwProgressNow = 0;
			dwProgressMax = sactivity_file_data.dwActivityProgress;
		}break;

	case EMACTIVITY_TYPE_QUESTION_BOX:
		{
			dwProgressNow = 0;
			dwProgressMax = sactivity_file_data.wQBoxProgress;
			sidProgress.dwID = sactivity_file_data.dwQBoxType;
		}break;
	}
}

void SACTIVITY_CHAR_DATA::Correction( SACTIVITY_FILE_DATA& sactivity_file_data )
{
	//type changes reset everything
	if ( emType != sactivity_file_data.emType )
	{
		sidProgress = NATIVEID_NULL();
		dwProgressNow = 0;
		dwProgressMax = 0;

		Assign( sactivity_file_data );
		return;
	}

	switch ( emType )
	{
	case EMACTIVITY_TYPE_REACH_LEVEL:
		{
			if ( dwProgressMax != sactivity_file_data.wProgressLevel )	
				dwProgressMax = sactivity_file_data.wProgressLevel;
		}break;
	case EMACTIVITY_TYPE_KILL_MOB:
		{
			if ( dwProgressMax != sactivity_file_data.wProgressMobKill )	
				dwProgressMax = sactivity_file_data.wProgressMobKill;

			if ( sidProgress != sactivity_file_data.sidMobKill )	
				sidProgress	= sactivity_file_data.sidMobKill;
		}break;
	case EMACTIVITY_TYPE_KILL_PLAYER:
		{
			if ( dwProgressMax != sactivity_file_data.wProgressMapKill )	
				dwProgressMax = sactivity_file_data.wProgressMapKill;

			if ( sidProgress != sactivity_file_data.sidMapKill )	
				sidProgress	 = sactivity_file_data.sidMapKill;
		}break;
	case EMACTIVITY_TYPE_REACH_MAP:
		{
			if ( dwProgressMax != sactivity_file_data.wProgressMapReach )
				dwProgressMax = sactivity_file_data.wProgressMapReach;

			if ( sidProgress != sactivity_file_data.sidMapReach )
				sidProgress	= sactivity_file_data.sidMapReach;
		}break;
	case EMACTIVITY_TYPE_TAKE_ITEM:
		{
			if ( dwProgressMax != sactivity_file_data.wProgressItemGet )
				dwProgressMax = sactivity_file_data.wProgressItemGet;

			if ( sidProgress != sactivity_file_data.sidItemGet )
				sidProgress	= sactivity_file_data.sidItemGet;
		}break;
	case EMACTIVITY_TYPE_USE_ITEM:
		{
			if ( dwProgressMax != sactivity_file_data.wProgressItemUse )
				dwProgressMax = sactivity_file_data.wProgressItemUse;

			if ( sidProgress != sactivity_file_data.sidItemUse )
				sidProgress	 = sactivity_file_data.sidItemUse;
		}break;
	case EMACTIVITY_TYPE_REACH_ACTIVITY:
		{

		}break;
	case EMACTIVITY_TYPE_COMPLETE_QUEST:
		{
			if ( dwProgressMax != sactivity_file_data.wQuestProgress )
				dwProgressMax = sactivity_file_data.wQuestProgress;

			if ( sidProgress.dwID != sactivity_file_data.dwQuestID )
				sidProgress.dwID = sactivity_file_data.dwQuestID;
		}break;
	case EMACTIVITY_TYPE_ACTIVITY_POINT:
		{
			if ( dwProgressMax != sactivity_file_data.dwActivityProgress )	
				dwProgressMax = sactivity_file_data.dwActivityProgress;
		}break;

	case EMACTIVITY_TYPE_QUESTION_BOX:
		{
			if ( dwProgressMax != sactivity_file_data.wQBoxProgress )
				dwProgressMax = sactivity_file_data.wQBoxProgress;

			if ( sidProgress.dwID != sactivity_file_data.dwQBoxType )
				sidProgress.dwID = sactivity_file_data.dwQBoxType;
		}break;
	}
}