#include "StdAfx.h"
#include "GLCodexData.h"

#include "../../Lib_Engine/Common/IniLoader.h"
#include "../../Lib_Engine/G-Logic/GLogic.h"
#include "../Lib_Engine/Common/StringUtils.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif



BOOL SCODEX_FILE_DATA::LOAD ( basestream &SFile, BOOL bServer )
{
	DWORD dwVer(0);
	SFile >> dwVer;
	
	if ( dwVer == VERSION )
	{
		SFile >> dwCodexID;
		SFile >> strCodexTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMCODEX_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMCODEX_NOTIFY)dwData;
		
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

		SFile >> dwCodexProgress;
		
		SFile >> wItemGrade1;
		SFile >> wItemGrade2;
		SFile >> wItemGrade3;
		SFile >> wItemGrade4;
		SFile >> wItemGrade5;
	}
	else if ( dwVer == 0x0009 )
	{
		SFile >> dwCodexID;
		SFile >> strCodexTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMCODEX_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMCODEX_NOTIFY)dwData;
		
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

		SFile >> dwCodexProgress;
	}
	else if ( dwVer == 0x0008 )
	{
		SFile >> dwCodexID;
		SFile >> strCodexTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMCODEX_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMCODEX_NOTIFY)dwData;

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
		SFile >> dwCodexID;
		SFile >> strCodexTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMCODEX_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMCODEX_NOTIFY)dwData;

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
		SFile >> dwCodexID;
		SFile >> strCodexTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMCODEX_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMCODEX_NOTIFY)dwData;

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
		SFile >> dwCodexID;
		SFile >> strCodexTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMCODEX_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMCODEX_NOTIFY)dwData;

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
		SFile >> dwCodexID;
		SFile >> strCodexTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMCODEX_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMCODEX_NOTIFY)dwData;

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
		SFile >> dwCodexID;
		SFile >> strCodexTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMCODEX_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMCODEX_NOTIFY)dwData;

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
		SFile >> dwCodexID;
		SFile >> strCodexTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMCODEX_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMCODEX_NOTIFY)dwData;

		SFile >> dwRewardPoint;	
		SFile >> bRewardBadge;

		SFile >> wProgressLevel;

		SFile >> sidMobKill.dwID;
		SFile >> wProgressMobKill;
	}
	else if ( dwVer == 0x0001 )
	{
		SFile >> dwCodexID;
		SFile >> strCodexTitle;
		SFile >> strBadgeString;
		SFile >> strDescription;

		DWORD dwData(0);
		SFile >> dwData;	emType  = (EMCODEX_TYPE)dwData;
		SFile >> dwData;	emNotify  = (EMCODEX_NOTIFY)dwData;

		SFile >> dwRewardPoint;	
		SFile >> bRewardBadge;

		SFile >> wProgressLevel;
	}
	else
	{
		CDebugSet::ErrorVersion( "SCODEX_FILE_DATA::LOAD", dwVer );
	}

	return TRUE;
}

BOOL SCODEX_FILE_DATA::SAVE ( CSerialFile &SFile )
{
	SFile << (DWORD)VERSION;

	SFile << dwCodexID;
	SFile << strCodexTitle;
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

	SFile << dwCodexProgress;
	
	SFile << wItemGrade1;
	SFile << wItemGrade2;
	SFile << wItemGrade3;
	SFile << wItemGrade4;
	SFile << wItemGrade5;

	return TRUE;
}

void SCODEX_CHAR_DATA::Assign( SCODEX_FILE_DATA& sactivity_file_data )
{
	dwCodexID = sactivity_file_data.dwCodexID;
	emType = sactivity_file_data.emType;
	
	/*Item Codex, Jhoniex 2/5/2024*/
	dwProgressMax = 5;

	if ( sactivity_file_data.wProgressItemUse == 0 )
		dwProgressMax = 4;
	if ( sactivity_file_data.wProgressItemGet == 0 )
		dwProgressMax = 3;
	if ( sactivity_file_data.wProgressMapReach == 0 )
		dwProgressMax = 2;
	if ( sactivity_file_data.wProgressMapKill == 0 )
		dwProgressMax = 1;
	
	wQuantity1 = sactivity_file_data.wProgressMobKill;
	wQuantity2 = sactivity_file_data.wProgressMapKill;
	wQuantity3 = sactivity_file_data.wProgressMapReach;
	wQuantity4 = sactivity_file_data.wProgressItemGet;
	wQuantity5 = sactivity_file_data.wProgressItemUse;
	
	wItemGrade1 = sactivity_file_data.wItemGrade1;
	wItemGrade2 = sactivity_file_data.wItemGrade2;
	wItemGrade3 = sactivity_file_data.wItemGrade3;
	wItemGrade4 = sactivity_file_data.wItemGrade4;
	wItemGrade5 = sactivity_file_data.wItemGrade5;
	
	dwProgressNow = 0;
	sidProgress = sactivity_file_data.sidMobKill;
	
	sidProgressItem1 = sactivity_file_data.sidMobKill;
	sidProgressItem2 = sactivity_file_data.sidMapKill;
	sidProgressItem3 = sactivity_file_data.sidMapReach;
	sidProgressItem4 = sactivity_file_data.sidItemGet;
	sidProgressItem5 = sactivity_file_data.sidItemUse;

	dwProgressItemDone1 = 0;
	dwProgressItemDone2 = 0;
	dwProgressItemDone3 = 0;
	dwProgressItemDone4 = 0;
	dwProgressItemDone5 = 0;
	/*End*/
	/*switch ( emType )
	{
	case EMCODEX_TYPE_REACH_LEVEL:
		{
			dwProgressNow = 1;
			dwProgressMax = sactivity_file_data.wProgressLevel;
		}break;
	case EMCODEX_TYPE_KILL_MOB:
		{
			dwProgressNow = 0;
			dwProgressMax = sactivity_file_data.wProgressMobKill;
			sidProgress	 = sactivity_file_data.sidMobKill;
		}break;
	case EMCODEX_TYPE_KILL_PLAYER:
		{
			dwProgressNow = 0;
			dwProgressMax = sactivity_file_data.wProgressMapKill;
			sidProgress	 = sactivity_file_data.sidMapKill;
		}break;
	case EMCODEX_TYPE_REACH_MAP:
		{
			dwProgressNow = 0;
			dwProgressMax = sactivity_file_data.wProgressMapReach;
			sidProgress	 = sactivity_file_data.sidMapReach;
		}break;
	case EMCODEX_TYPE_TAKE_ITEM:
		{
			dwProgressNow = 0;
			dwProgressMax = sactivity_file_data.wProgressItemGet;
			sidProgress	 = sactivity_file_data.sidItemGet;
		}break;
	case EMCODEX_TYPE_USE_ITEM:
		{
			dwProgressNow = 0;
			dwProgressMax = sactivity_file_data.wProgressItemUse;
			sidProgress	 = sactivity_file_data.sidItemUse;
		}break;
	case EMCODEX_TYPE_REACH_CODEX:
		{

		}break;
	case EMCODEX_TYPE_COMPLETE_QUEST:
		{
			dwProgressNow = 0;
			dwProgressMax = sactivity_file_data.wQuestProgress;
			sidProgress.dwID = sactivity_file_data.dwQuestID;
		}break;
	case EMCODEX_TYPE_CODEX_POINT:
		{
			dwProgressNow = 0;
			dwProgressMax = sactivity_file_data.dwCodexProgress;
		}break;

	case EMCODEX_TYPE_QUESTION_BOX:
		{
			dwProgressNow = 0;
			dwProgressMax = sactivity_file_data.wQBoxProgress;
			sidProgress.dwID = sactivity_file_data.dwQBoxType;
		}break;
	case EMCODEX_TYPE_ETC:
		{
			dwProgressNow = 0;
			//dwProgressMax = sactivity_file_data.wQBoxProgress;
			//sidProgress.dwID = sactivity_file_data.dwQBoxType;
		}break;
	}*/
}

void SCODEX_CHAR_DATA::Correction( SCODEX_FILE_DATA& sactivity_file_data )
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
	
	/*Codex correction, jhoniex*/
	if ( sidProgressItem1 != sactivity_file_data.sidMobKill)
		sidProgressItem1 = sactivity_file_data.sidMobKill;
	if ( sidProgressItem2 != sactivity_file_data.sidMapKill)
		sidProgressItem2 = sactivity_file_data.sidMapKill;
	if ( sidProgressItem3 != sactivity_file_data.sidMapReach)
		sidProgressItem3 = sactivity_file_data.sidMapReach;
	if ( sidProgressItem4 != sactivity_file_data.sidItemGet)
		sidProgressItem4 = sactivity_file_data.sidItemGet;
	if ( sidProgressItem5 != sactivity_file_data.sidItemUse)
		sidProgressItem5 = sactivity_file_data.sidItemUse;

	/*switch ( emType )
	{
	case EMCODEX_TYPE_REACH_LEVEL:
		{
			if ( dwProgressMax != sactivity_file_data.wProgressLevel )	
				dwProgressMax = sactivity_file_data.wProgressLevel;
		}break;
	case EMCODEX_TYPE_KILL_MOB:
		{
			if ( dwProgressMax != sactivity_file_data.wProgressMobKill )	
				dwProgressMax = sactivity_file_data.wProgressMobKill;

			if ( sidProgress != sactivity_file_data.sidMobKill )	
				sidProgress	= sactivity_file_data.sidMobKill;
		}break;
	case EMCODEX_TYPE_KILL_PLAYER:
		{
			if ( dwProgressMax != sactivity_file_data.wProgressMapKill )	
				dwProgressMax = sactivity_file_data.wProgressMapKill;

			if ( sidProgress != sactivity_file_data.sidMapKill )	
				sidProgress	 = sactivity_file_data.sidMapKill;
		}break;
	case EMCODEX_TYPE_REACH_MAP:
		{
			if ( dwProgressMax != sactivity_file_data.wProgressMapReach )
				dwProgressMax = sactivity_file_data.wProgressMapReach;

			if ( sidProgress != sactivity_file_data.sidMapReach )
				sidProgress	= sactivity_file_data.sidMapReach;
		}break;
	case EMCODEX_TYPE_TAKE_ITEM:
		{
			if ( dwProgressMax != sactivity_file_data.wProgressItemGet )
				dwProgressMax = sactivity_file_data.wProgressItemGet;

			if ( sidProgress != sactivity_file_data.sidItemGet )
				sidProgress	= sactivity_file_data.sidItemGet;
		}break;
	case EMCODEX_TYPE_USE_ITEM:
		{
			if ( dwProgressMax != sactivity_file_data.wProgressItemUse )
				dwProgressMax = sactivity_file_data.wProgressItemUse;

			if ( sidProgress != sactivity_file_data.sidItemUse )
				sidProgress	 = sactivity_file_data.sidItemUse;
		}break;
	case EMCODEX_TYPE_REACH_CODEX:
		{

		}break;
	case EMCODEX_TYPE_COMPLETE_QUEST:
		{
			if ( dwProgressMax != sactivity_file_data.wQuestProgress )
				dwProgressMax = sactivity_file_data.wQuestProgress;

			if ( sidProgress.dwID != sactivity_file_data.dwQuestID )
				sidProgress.dwID = sactivity_file_data.dwQuestID;
		}break;
	case EMCODEX_TYPE_CODEX_POINT:
		{
			if ( dwProgressMax != sactivity_file_data.dwCodexProgress )	
				dwProgressMax = sactivity_file_data.dwCodexProgress;
		}break;

	case EMCODEX_TYPE_QUESTION_BOX:
		{
			if ( dwProgressMax != sactivity_file_data.wQBoxProgress )
				dwProgressMax = sactivity_file_data.wQBoxProgress;

			if ( sidProgress.dwID != sactivity_file_data.dwQBoxType )
				sidProgress.dwID = sactivity_file_data.dwQBoxType;
		}break;
	case EMCODEX_TYPE_ETC:
		{
			
		}break;
	}*/
}
void SCODEX_FILE_DATA::SaveCsvHead( std::fstream &SFile )
{	
	SFile << "dwCodexID" << ",";
	SFile << "strCodexTitle" << ",";
	//SFile << "strBadgeString" << ",";
	//SFile << "strDescription" << ",";

	SFile << "emType" << ",";
	//SFile << "emNotify" << ",";
	
	SFile << "dwRewardPoint" << ",";	
	//SFile << "bRewardBadge" << ",";

	//SFile << "wProgressLevel" << ",";

	SFile << "Item1.wMainID" << ",";
	SFile << "Item1.wSubID" << ",";
	SFile << "Item1.Quantity" << ",";
	SFile << "Item1.wGrade" << ",";
	
	SFile << "Item2.wMainID" << ",";
	SFile << "Item2.wSubID" << ",";
	SFile << "Item2.Quantity" << ",";
	SFile << "Item2.wGrade" << ",";
	
	SFile << "Item3.wMainID" << ",";
	SFile << "Item3.wSubID" << ",";
	SFile << "Item3.Quantity" << ",";
	SFile << "Item3.wGrade" << ",";
	
	SFile << "Item4.wMainID" << ",";
	SFile << "Item4.wSubID" << ",";
	SFile << "Item4.Quantity" << ",";
	SFile << "Item4.wGrade" << ",";

	SFile << "Item5.wMainID" << ",";
	SFile << "Item5.wSubID" << ",";
	SFile << "Item5.Quantity" << ",";
	SFile << "Item5.wGrade" << ",";

	SFile << std::endl;
}
void SCODEX_FILE_DATA::SaveCsv( std::fstream &SFile )
{	
	SFile << dwCodexID << ",";
	SFile << strCodexTitle << ",";
	//SFile << strBadgeString << ",";
	//SFile << strDescription << ",";

	SFile << emType << ",";
	//SFile << emNotify << ",";
	
	SFile << dwRewardPoint << ",";	
	//SFile << bRewardBadge << ",";

	//SFile << wProgressLevel << ",";
	
	//checker
	if ( sidMobKill.wMainID == 65535 )
		wProgressMobKill = 0;
	if ( sidMapKill.wMainID == 65535 )
		wProgressMapKill = 0;
	if ( sidMapReach.wMainID == 65535 )
		wProgressMapReach = 0;
	if ( sidItemGet.wMainID == 65535 )
		wProgressItemGet = 0;
	if ( sidItemUse.wMainID == 65535 )
		wProgressItemUse = 0;

	SFile << sidMobKill.wMainID << ",";
	SFile << sidMobKill.wSubID << ",";
	SFile << wProgressMobKill << ",";
	SFile << wItemGrade1 << ",";

	SFile << sidMapKill.wMainID << ",";
	SFile << sidMapKill.wSubID << ",";
	SFile << wProgressMapKill << ",";
	SFile << wItemGrade2 << ",";

	SFile << sidMapReach.wMainID << ",";
	SFile << sidMapReach.wSubID << ",";
	SFile << wProgressMapReach << ",";
	SFile << wItemGrade3 << ",";

	SFile << sidItemGet.wMainID << ",";
	SFile << sidItemGet.wSubID << ",";
	SFile << wProgressItemGet << ",";
	SFile << wItemGrade4 << ",";

	SFile << sidItemUse.wMainID << ",";
	SFile << sidItemUse.wSubID << ",";
	SFile << wProgressItemUse << ",";
	SFile << wItemGrade5 << ",";

	SFile << std::endl;
}
void SCODEX_FILE_DATA::LoadCsv( CStringArray &StrArray )
{	
	std::string strTemp;
	int iCsvCur = 0;

	dwCodexID = (DWORD)atoi( StrArray[ iCsvCur++ ] );

	CString strTitle = StrArray.GetAt(iCsvCur++);
	strCodexTitle = strTitle;
	//strCodexTitle = atoi( StrArray[ iCsvCur++ ].GetString() );
	//strBadgeString = (DWORD)atoi( StrArray[ iCsvCur++ ].GetString() );
	//strDescription = (DWORD)atoi( StrArray[ iCsvCur++ ].GetString() );

	emType = (EMCODEX_TYPE)atoi( StrArray[ iCsvCur++ ] );
	//emNotify = (EMCODEX_NOTIFY)atoi( StrArray[ iCsvCur++ ] );
		
	dwRewardPoint = (DWORD)atoi( StrArray[ iCsvCur++ ] );	
	//bRewardBadge = (DWORD)atoi( StrArray[ iCsvCur++ ] );

	//wProgressLevel = (DWORD)atoi( StrArray[ iCsvCur++ ] );

	sidMobKill.wMainID = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	sidMobKill.wSubID = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	wProgressMobKill = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	wItemGrade1 = (DWORD)atoi( StrArray[ iCsvCur++ ] );

	sidMapKill.wMainID = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	sidMapKill.wSubID = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	wProgressMapKill = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	wItemGrade2 = (DWORD)atoi( StrArray[ iCsvCur++ ] );

	sidMapReach.wMainID = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	sidMapReach.wSubID = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	wProgressMapReach = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	wItemGrade3 = (DWORD)atoi( StrArray[ iCsvCur++ ] );

	sidItemGet.wMainID = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	sidItemGet.wSubID = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	wProgressItemGet = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	wItemGrade4 = (DWORD)atoi( StrArray[ iCsvCur++ ] );

	sidItemUse.wMainID = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	sidItemUse.wSubID = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	wProgressItemUse = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	wItemGrade5 = (DWORD)atoi( StrArray[ iCsvCur++ ] );

	//STRUTIL::InputStrCsv( StrArray[ iCsvCur++ ], strTemp );
}