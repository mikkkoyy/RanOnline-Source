#include "stdafx.h"
#include "GLItemQuestion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ITEM
{
	BOOL SQUESTIONITEM::LOAD( basestream &SFile ) // by ∞Ê¥Î
	{
		DWORD dwVer(0), dwSize(0);
		SFile >> dwVer;
		SFile >> dwSize;

		if ( dwVer==ITEM::SQUESTIONITEM::VERSION )
		{
			GASSERT(sizeof(ITEM::SQUESTIONITEM)==dwSize);

			int nItem(0);
			SFile >> nItem;	emType = static_cast<EMITEM_QUESTION>(nItem);
			SFile >> wParam1;
			SFile >> wParam2;
			SFile >> fTime;
			SFile >> fExp; // ∞Ê«Ëƒ° »πµÊ 0.04~0.1%

		}
		else if ( dwVer==0x0100 )
		{
			GASSERT(sizeof(ITEM::SQUESTIONITEM_100)==dwSize);

			int nItem(0);
			SFile >> nItem;	emType = static_cast<EMITEM_QUESTION>(nItem);
			SFile >> fTime;
			SFile >> fExp; // ∞Ê«Ëƒ° »πµÊ 0.04~0.1%
			SFile >> wParam1;
			SFile >> wParam2;
		}
		else
		{
			CDebugSet::ErrorVersion( "ITEM::SQUESTIONITEM", dwVer );
			SFile.SetOffSet ( SFile.GetfTell()+dwSize );

			return FALSE;
		}

		return TRUE;
	}

	BOOL SQUESTIONITEM::SAVE( CSerialFile &SFile )
	{
		SFile << static_cast<int>(ITEM::SQUESTIONITEM::VERSION);
		SFile.BeginBlock(); // ±∏¡∂√º ªÁ¿Ã¡Ó∏¶ ¿˙¿Â«—¥Ÿ.
		{
			SFile << static_cast<int>(emType);
			SFile << wParam1;
			SFile << wParam2;
			SFile << fTime;
			SFile << fExp; // ∞Ê«Ëƒ° »πµÊ 0.04~0.1%
		}
		SFile.EndBlock();

		return TRUE;
	}

	VOID SQUESTIONITEM::SaveCsvHead ( std::fstream &SFile )
	{
		SFile << "emType" << ",";
		SFile << "fTime" << ",";
		SFile << "fExp" << ",";
		SFile << "wParam1" << ",";
		SFile << "wParam2" << ",";
	}

	VOID SQUESTIONITEM::SaveCsv ( std::fstream &SFile )
	{
		SFile << emType << ",";
		SFile << fTime << ",";
		SFile << fExp << ",";
		SFile << wParam1 << ",";
		SFile << wParam2 << ",";
	}

	VOID SQUESTIONITEM::LoadCsv ( CStringArray &StrArray, int &iCsvCur  )
	{
		emType = (EMITEM_QUESTION)atoi( StrArray[ iCsvCur++ ] );
		fTime = (float)atof( StrArray[ iCsvCur++ ] );
		fExp = (float)atof( StrArray[ iCsvCur++ ] );
		wParam1 = (WORD)atoi( StrArray[ iCsvCur++ ] );
		wParam2 = (WORD)atoi( StrArray[ iCsvCur++ ] );
	}
};