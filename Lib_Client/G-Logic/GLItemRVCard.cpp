#include "stdafx.h"
#include "GLItemRVCard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ITEM
{
	SRVCARD& SRVCARD::operator = ( const SRVCARD& rvalue )
	{
		emOption = rvalue.emOption;
		wValue = rvalue.wValue;
		bCheckExist = rvalue.bCheckExist;
		bReplaceOpt = rvalue.bReplaceOpt;

		for ( int i=0; i<SUIT_NSIZE; ++i )	
			bUseSuit[i] = rvalue.bUseSuit[i];

		return *this;
	}


	BOOL SRVCARD::LOAD( basestream &SFile )	
	{
		DWORD dwVer( 0 ), dwSize( 0 );
		SFile >> dwVer;
		SFile >> dwSize;

		switch( dwVer )
		{
		case VERSION:
			{
				int nType (0);
				SFile >> nType;	emOption = static_cast<EMRANDOM_OPT>(nType);
				SFile >> wValue;
				SFile >> bCheckExist;
				SFile >> bReplaceOpt;
				
				DWORD dwNum(0);
				SFile >> dwNum;
				for( DWORD i=0; i<dwNum; ++i ){
					BOOL bUse = FALSE;
					SFile >> bUse;

					if ( i >= SUIT_NSIZE )	continue;

					bUseSuit[i] = bUse;
				}
			}break;

		default:
			{
				CDebugSet::ErrorVersion( "ITEM::SRVCARD", dwVer );
				SFile.SetOffSet( SFile.GetfTell() + dwSize );
				return FALSE;
			}break;
		}

		return TRUE;
	}

	BOOL SRVCARD::SAVE( CSerialFile &SFile )
	{
		SFile << static_cast<int>( ITEM::SRVCARD::VERSION );
		SFile.BeginBlock();
		{
			SFile << (DWORD)emOption;
			SFile << wValue;
			SFile << bCheckExist;
			SFile << bReplaceOpt;
			
			SFile << (DWORD)SUIT_NSIZE;
			for( int i=0; i<SUIT_NSIZE; ++i ){
				SFile << bUseSuit[i];
			}
		}
		SFile.EndBlock();

		return TRUE;
	}

	VOID SRVCARD::SaveCsvHead ( std::fstream &SFile )
	{
		SFile << "emOption" << ",";
		SFile << "wValue" << ",";
		SFile << "bCheckExist" << ",";
		SFile << "bReplaceOpt" << ",";

		for( int i=0; i<SUIT_NSIZE; ++i )
			SFile << "bUseSuit " << i << ",";
	}

	VOID SRVCARD::SaveCsv ( std::fstream &SFile )
	{
		SFile << emOption << ",";
		SFile << wValue << ",";
		SFile << bCheckExist << ",";
		SFile << bReplaceOpt << ",";

		for( int i=0; i<SUIT_NSIZE; ++i )
			SFile << bUseSuit[i] << ",";

	}

	VOID SRVCARD::LoadCsv ( CStringArray &StrArray, int &iCsvCur )
	{
		emOption = (EMRANDOM_OPT)atoi( StrArray[ iCsvCur++ ] );
		wValue = (WORD)atoi( StrArray[ iCsvCur++ ] );
		bCheckExist = (BOOL)atol( StrArray[ iCsvCur++ ] );
		bReplaceOpt = (BOOL)atol( StrArray[ iCsvCur++ ] );

		for( int i=0; i<SUIT_NSIZE; ++i )
			bUseSuit[i] = (WORD)atoi( StrArray[ iCsvCur++ ] );
	}
};

