#include "stdafx.h"
#include "GLItemGenerate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ITEM
{
	VOID SGENERATE::SaveCsvHead ( std::fstream &SFile )
	{
		SFile << "dwSpecID" << ",";
		SFile << "dwLimitTime" << ",";
		SFile << "dwLimitTimeGen" << ",";
		SFile << "bGenEnable" << ",";
	}

	VOID SGENERATE::SaveCsv ( std::fstream &SFile )
	{
		SFile << dwSpecID << ",";
		SFile << dwLimitTime << ",";
		SFile << dwLimitTimeGen << ",";
		SFile << bGenEnable << ",";
	}

	VOID SGENERATE::LoadCsv ( CStringArray &StrArray, int &iCsvCur  )
	{
		dwSpecID = (DWORD)atol( StrArray[ iCsvCur++ ] );
		dwLimitTime = (DWORD)atol( StrArray[ iCsvCur++ ] );
		dwLimitTimeGen = (DWORD)atol( StrArray[ iCsvCur++ ] );
		bGenEnable = (BOOL)atol( StrArray[ iCsvCur++ ] );
	}

	void SGENERATE::Assign ( SGENERATE_100 &sOld )
	{
		dwSpecID = sOld.dwSpecID;
		dwLimitTimeGen = sOld.dwLimitTimeGen;
		dwLimitTime = sOld.dwLimitTime;
		bGenEnable = TRUE;
	}

	void SGENERATE::Assign ( SGENERATE_101 &sOld )
	{
		dwSpecID = sOld.dwSpecID;
		dwLimitTimeGen = sOld.dwLimitTimeGen;
		dwLimitTime = sOld.dwLimitTime;
		bGenEnable = TRUE;
	}
};