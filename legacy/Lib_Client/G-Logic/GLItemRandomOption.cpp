#include "stdafx.h"
#include "GLItemRandomOption.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ITEM
{
	BOOL SRANDOM_OPT::LOAD ( basestream &SFile )
	{
		SFile.ReadBuffer ( szNAME, SRANDOM_DATA::NAME_LEN );
		SFile >> bUnableRandomCard;
		SFile >> bPVPItem;
		return TRUE;
	}

	BOOL SRANDOM_OPT::SAVE ( CSerialFile &SFile )
	{
		SFile.WriteBuffer ( szNAME, SRANDOM_DATA::NAME_LEN );
		SFile << bUnableRandomCard;

		SFile << bPVPItem;

		return TRUE;
	}

	VOID SRANDOM_OPT::SaveCsvHead ( std::fstream &SFile )
	{
		SFile << "szNAME" << ",";
		SFile << "bUnableRandomCard" << ",";
		SFile << "bPVPItem" << ",";
	}

	VOID SRANDOM_OPT::SaveCsv ( std::fstream &SFile )
	{
		if( strlen( szNAME ) < 1 )
		{
			SFile << " " << ",";
			SFile << bUnableRandomCard << ",";
			SFile << bPVPItem << ",";
		}
		else
		{
			SFile << szNAME << ",";
			SFile << bUnableRandomCard << ",";
			SFile << bPVPItem << ",";
		}
		
	}

	VOID SRANDOM_OPT::LoadCsv ( CStringArray &StrArray, int &iCsvCur )
	{
		DWORD dwSize = sizeof(char)*SRANDOM_DATA::NAME_LEN;
		memset( szNAME, 0, dwSize );

		if( strlen( StrArray[ iCsvCur ] ) > 1 && StrArray[ iCsvCur ].GetAt( 0 ) != ' ' )
			StringCchCopy( szNAME, dwSize, StrArray[ iCsvCur ] );

		++iCsvCur;

		bUnableRandomCard = (bool)atoi( StrArray[ iCsvCur++ ] );
		bPVPItem = (bool)atoi( StrArray[ iCsvCur++ ] );

	}
};