#include "stdafx.h"
#include "GLItemPetSkin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ITEM
{
	BOOL SPETSKINPACKITEM::LOAD ( basestream &SFile )
	{
		DWORD dwVER(0), dwSIZE(0);
		SFile >> dwVER;
		SFile >> dwSIZE;

		switch (dwVER)
		{
		case VERSION:
			{
				DWORD dwNUM(0);
				SFile >> dwNUM;
				for ( DWORD i=0; i<dwNUM; ++i )
				{
					SPETSKINPACKITEMDATA sITEM;
					SFile.ReadBuffer ( &sITEM, sizeof(SPETSKINPACKITEMDATA) );
					if ( i >= PETSKIN_MAX )	continue;
					vecPetSkinData.push_back ( sITEM );
				}

				SFile >> dwPetSkinTime;
			}break;

		case 0x0100:
			{
				DWORD dwNUM(0);
				SFile >> dwPetSkinTime;
				SFile >> dwNUM;
				for ( DWORD i=0; i<dwNUM; ++i )
				{
					SPETSKINPACKITEMDATA sITEM;
					SFile.ReadBuffer ( &sITEM, sizeof(SPETSKINPACKITEMDATA) );
					if ( i >= PETSKIN_MAX )	continue;
					vecPetSkinData.push_back ( sITEM );
				}
			}break;

		default:
			{
				CDebugSet::ErrorVersion( "ITEM::SPETSKINPACKITEM", dwVER );
				SFile.SetOffSet ( SFile.GetfTell()+dwSIZE );
			}break;
		};

		return TRUE;
	}

	BOOL SPETSKINPACKITEM::SAVE ( CSerialFile &SFile )
	{
		SFile << DWORD(VERSION);
		SFile.BeginBlock();
		{
			SFile << (DWORD) vecPetSkinData.size();
			if ( vecPetSkinData.size() > 0 )
			{
				SFile.WriteBuffer ( &(vecPetSkinData[0]), DWORD(sizeof(SPETSKINPACKITEMDATA)*vecPetSkinData.size()) );
			}

			SFile << dwPetSkinTime;
		}
		SFile.EndBlock();

		return TRUE;
	}

	VOID SPETSKINPACKITEM::SaveCsvHead ( std::fstream &SFile )
	{
		const int iMaxSize = PETSKIN_MAX;

		SFile << "PetSkinTime" << ",";

		for( int i=0; i<iMaxSize; ++i )
		{
			SFile << "PetSkin " << i << " fRATE" << ",";
			SFile << "PetSkin " << i << " fSCALE" << ",";
			SFile << "PetSkin " << i << " nidMOB wMainID" << ",";
			SFile << "PetSkin " << i << " nidMOB wSubID" << ",";			
		}
	}

	VOID SPETSKINPACKITEM::SaveCsv ( std::fstream &SFile )
	{
		const int iMaxSize = PETSKIN_MAX;
		int iSize = (int)vecPetSkinData.size();

		SFile << dwPetSkinTime << ",";

		if( iSize > iMaxSize )
			iSize = iMaxSize;

		for( int i=0; i<iMaxSize; ++i )
		{
			if( i < iSize )
			{
				SFile << vecPetSkinData[i].fRate << ",";
				SFile << vecPetSkinData[i].fScale << ",";
				SFile << vecPetSkinData[i].sMobID.wMainID << ",";
				SFile << vecPetSkinData[i].sMobID.wSubID << ",";
			}
			else
			{
				SFile << "0" << ",";
				SFile << "0" << ",";
				SFile << "0" << ",";
				SFile << "0" << ",";
			}
		}
	}

	VOID SPETSKINPACKITEM::LoadCsv ( CStringArray &StrArray, int &iCsvCur )
	{
		const int iMaxSize = PETSKIN_MAX;

		vecPetSkinData.clear();

		dwPetSkinTime  = atoi( StrArray[ iCsvCur++ ] );

		for( int i=0; i<iMaxSize; ++i )
		{
			SPETSKINPACKITEMDATA stPetSkinPack;
			stPetSkinPack.fRate  = (float)atof( StrArray[ iCsvCur++ ] );
			stPetSkinPack.fScale = (float)atof( StrArray[ iCsvCur++ ] );
			stPetSkinPack.sMobID.wMainID = (WORD)atoi( StrArray[ iCsvCur++ ] );
			stPetSkinPack.sMobID.wSubID  = (WORD)atoi( StrArray[ iCsvCur++ ] );

			if( stPetSkinPack.fRate != 0.0f && stPetSkinPack.fScale != 0.0f )
				vecPetSkinData.push_back( stPetSkinPack );
		}
	}

	void SPETSKINPACKITEM::CLEAR ()
	{
		vecPetSkinData.clear();
	}

	bool SPETSKINPACKITEM::VALID ()
	{
		return !vecPetSkinData.empty();
	}

	bool SPETSKINPACKITEM::INSERT ( const SNATIVEID &sMobID, float fRate, float fScale )
	{
		if ( vecPetSkinData.size() >= (int) PETSKIN_MAX )	return false;

		SPETSKINPACKITEMDATA sPetSkinPack;
		sPetSkinPack.sMobID = sMobID;
		sPetSkinPack.fRate  = fRate;
		sPetSkinPack.fScale = fScale;

		vecPetSkinData.push_back ( sPetSkinPack );

		return true;
	}

	bool SPETSKINPACKITEM::DEL ( int nIndex )
	{
		if ( nIndex < 0 )					return false;
		if ( (int)vecPetSkinData.size() < nIndex )	return false;

		vecPetSkinData.erase ( vecPetSkinData.begin()+nIndex );

		return true;
	}
};