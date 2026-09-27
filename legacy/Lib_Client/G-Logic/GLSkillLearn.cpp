#include "stdafx.h"
#include "GLSkillLearn.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace SKILL
{
	VOID SLEARN::SaveCsvHead ( std::fstream &SFile )
	{
		SFile << "emBRIGHT" << ",";
		SFile << "dwCLASS" << ",";
		SFile << "sSKILL wMainID" << ",";
		SFile << "sSKILL wSubID" << ",";

		for( int i=1; i<SKILL::MAX_LEVEL+1; ++i )
		{
			SFile << "sLVL_STEP " << i << " dwSKP" << ",";
			SFile << "sLVL_STEP " << i << " dwLEVEL" << ",";

			SFile << "sLVL_STEP " << i << " sSTATS wPow" << ",";
			SFile << "sLVL_STEP " << i << " sSTATS wStr" << ",";
			SFile << "sLVL_STEP " << i << " sSTATS wSpi" << ",";
			SFile << "sLVL_STEP " << i << " sSTATS wDex" << ",";
			SFile << "sLVL_STEP " << i << " sSTATS wInt" << ",";
			SFile << "sLVL_STEP " << i << " sSTATS wSta" << ",";

			SFile << "sLVL_STEP " << i << " dwSKILL_LVL" << ",";
		}

		SFile << "bOnlyOneStats" << ",";
		SFile << "sHiddenWeapon wMainID" << ",";
		SFile << "sHiddenWeapon wSubID" << ",";
		SFile << "bHiddenWeapon" << ",";
	}

	VOID SLEARN::SaveCsv ( std::fstream &SFile )
	{
		SFile << emBRIGHT << ",";
		SFile << dwCLASS << ",";
		SFile << sSKILL.wMainID << ",";
		SFile << sSKILL.wSubID << ",";

		for( int i=0; i<SKILL::MAX_LEVEL; ++i )
		{
			SFile << sLVL_STEP[i].dwSKP << ",";
			SFile << sLVL_STEP[i].dwLEVEL << ",";

			SFile << sLVL_STEP[i].sSTATS.wPow << ",";
			SFile << sLVL_STEP[i].sSTATS.wStr << ",";
			SFile << sLVL_STEP[i].sSTATS.wSpi << ",";
			SFile << sLVL_STEP[i].sSTATS.wDex << ",";
			SFile << sLVL_STEP[i].sSTATS.wInt << ",";
			SFile << sLVL_STEP[i].sSTATS.wSta << ",";

			SFile << sLVL_STEP[i].dwSKILL_LVL << ",";
		}

		SFile << bOnlyOneStats << ",";
		SFile << sHiddenWeapon.wMainID << ",";
		SFile << sHiddenWeapon.wSubID << ",";
		SFile << bHiddenWeapon << ",";
	}

	VOID SLEARN::LoadCsv ( CStringArray &StrArray, int &iCsvCur )
	{
		emBRIGHT = (EMBRIGHT)atoi( StrArray[ iCsvCur++ ] );
		dwCLASS = (DWORD)atol( StrArray[ iCsvCur++ ] );
		sSKILL.wMainID = (WORD)atoi( StrArray[ iCsvCur++ ] );
		sSKILL.wSubID = (WORD)atoi( StrArray[ iCsvCur++ ] );

		for( int i=0; i<SKILL::MAX_LEVEL; ++i )
		{
			sLVL_STEP[i].dwSKP = (DWORD)atol( StrArray[ iCsvCur++ ] );
			sLVL_STEP[i].dwLEVEL = (DWORD)atol( StrArray[ iCsvCur++ ] );

			sLVL_STEP[i].sSTATS.wPow = (WORD)atoi( StrArray[ iCsvCur++ ] );
			sLVL_STEP[i].sSTATS.wStr = (WORD)atoi( StrArray[ iCsvCur++ ] );
			sLVL_STEP[i].sSTATS.wSpi = (WORD)atoi( StrArray[ iCsvCur++ ] );
			sLVL_STEP[i].sSTATS.wDex = (WORD)atoi( StrArray[ iCsvCur++ ] );
			sLVL_STEP[i].sSTATS.wInt = (WORD)atoi( StrArray[ iCsvCur++ ] );
			sLVL_STEP[i].sSTATS.wSta = (WORD)atoi( StrArray[ iCsvCur++ ] );

			sLVL_STEP[i].dwSKILL_LVL = (DWORD)atol( StrArray[ iCsvCur++ ] );
		}

		bOnlyOneStats = (BOOL)atol( StrArray[ iCsvCur++ ] );
		sHiddenWeapon.wMainID = (WORD)atol( StrArray[ iCsvCur++ ] );
		sHiddenWeapon.wSubID = (WORD)atol( StrArray[ iCsvCur++ ] );
		bHiddenWeapon = (BOOL)atol( StrArray[ iCsvCur++ ] );
	}

	VOID SLEARN::LoadCsv_oldj ( CStringArray &StrArray, int &iCsvCur )
	{
		emBRIGHT = (EMBRIGHT)atoi( StrArray[ iCsvCur++ ] );
		dwCLASS = (DWORD)atol( StrArray[ iCsvCur++ ] );
		sSKILL.wMainID = (WORD)atoi( StrArray[ iCsvCur++ ] );
		sSKILL.wSubID = (WORD)atoi( StrArray[ iCsvCur++ ] );

		for( int i=0; i<SKILL::MAX_LEVEL; ++i )
		{
			sLVL_STEP[i].dwSKP = (DWORD)atol( StrArray[ iCsvCur++ ] );
			sLVL_STEP[i].dwLEVEL = (DWORD)atol( StrArray[ iCsvCur++ ] );
			sLVL_STEP[i].sSTATS.wPow = (WORD)atoi( StrArray[ iCsvCur++ ] );
			sLVL_STEP[i].sSTATS.wStr = (WORD)atoi( StrArray[ iCsvCur++ ] );
			sLVL_STEP[i].sSTATS.wSpi = (WORD)atoi( StrArray[ iCsvCur++ ] );
			sLVL_STEP[i].sSTATS.wDex = (WORD)atoi( StrArray[ iCsvCur++ ] );
			sLVL_STEP[i].sSTATS.wInt = (WORD)atoi( StrArray[ iCsvCur++ ] );
			sLVL_STEP[i].sSTATS.wSta = (WORD)atoi( StrArray[ iCsvCur++ ] );
			sLVL_STEP[i].dwSKILL_LVL = (DWORD)atol( StrArray[ iCsvCur++ ] );
		}
	}
};