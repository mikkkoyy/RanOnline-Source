#include "stdafx.h"
#include "GLItemDrug.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ITEM
{
	void SDRUG::Assign ( SDRUG_100 &sDRUG )
	{
		bInstance = sDRUG.bInstance;
		emDrug = sDRUG.emDrug;
		wPileNum = sDRUG.wPileNum;

		bRatio = sDRUG.bRatio;

		wCureVolume = sDRUG.wCureVolume;
		wArrowNum = sDRUG.wArrowNum;

		dwCureDISORDER = sDRUG.dwCureDISORDER;
	}

	void SDRUG::Assign ( SDRUG_101 &sDRUG )
	{
		emDrug = sDRUG.emDrug;
		bInstance = sDRUG.bInstance;
		tTIME_LMT = sDRUG.tTIME_LMT;
		bRatio = sDRUG.bRatio;
		wPileNum = sDRUG.wPileNum;
		wCureVolume = sDRUG.wCureVolume;
		wArrowNum = sDRUG.wArrowNum;
		dwCureDISORDER = sDRUG.dwCureDISORDER;
	}

	VOID SDRUG::SaveCsvHead ( std::fstream &SFile )
	{
		SFile << "tTIME_LMT" << ",";
		SFile << "bInstance" << ",";
		SFile << "emDrug" << ",";
		SFile << "wPileNum" << ",";

		SFile << "bRatio" << ",";

		SFile << "wCureVolume" << ",";

		SFile << "dwCureDISORDER" << ",";
	}

	VOID SDRUG::SaveCsv ( std::fstream &SFile )
	{
		SFile << tTIME_LMT << ",";
		SFile << bInstance << ",";
		SFile << emDrug << ",";
		SFile << wPileNum << ",";

		SFile << bRatio << ",";

		SFile << wCureVolume << ",";

		SFile << dwCureDISORDER << ",";
	}

	VOID SDRUG::LoadCsv ( CStringArray &StrArray, int &iCsvCur )
	{
		tTIME_LMT = (__time64_t)_atoi64( StrArray[ iCsvCur++ ] );
		bInstance = (BOOL)atol( StrArray[ iCsvCur++ ] );
		emDrug = (EMITEM_DRUG)atoi( StrArray[ iCsvCur++ ] );
		wPileNum = (WORD)atoi( StrArray[ iCsvCur++ ] );

		bRatio = (BOOL)atol( StrArray[ iCsvCur++ ] );

		wCureVolume = (WORD)atoi( StrArray[ iCsvCur++ ] );

		dwCureDISORDER = (DWORD)atol( StrArray[ iCsvCur++ ] );
	}
};