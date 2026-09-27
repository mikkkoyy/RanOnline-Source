#include "stdafx.h"
#include "GLItemPet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ITEM
{
	BOOL SPET::LOAD( basestream &SFile )	// PetData
	{
		DWORD dwVer( 0 ), dwSize( 0 );
		SFile >> dwVer;
		SFile >> dwSize;

		switch( dwVer )
		{
		case 0x0100:
			{
				SFile >> dwPetID;
			}break;

		case 0x0101:
			{
				int nType(0);
				SFile >> dwPetID;
				SFile >> nType;
				emType = static_cast<PET_ACCESSORY_TYPE>(nType);
			}break;

		case 0x0102:
			{
				GASSERT( sizeof( ITEM::SPET_102 ) == dwSize );

				int nType(0);
				SFile >> dwPetID;
				SFile >> nType;	emType = static_cast<PET_ACCESSORY_TYPE>(nType);
				SFile >> nType;	emPetType = static_cast<PETTYPE>(nType);
				SFile >> sPetID.dwID;
			}break;

		case VERSION:
			{
				GASSERT( sizeof( ITEM::SPET ) == dwSize );

				int nType(0);
				SFile >> nType;	emPetType = static_cast<PETTYPE>(nType);
				SFile >> dwPetID;
				SFile >> nType;	emType = static_cast<PET_ACCESSORY_TYPE>(nType);
				SFile >> sPetID.dwID;
			}break;

		default:
			{
				CDebugSet::ErrorVersion( "ITEM::SPET", dwVer );
				SFile.SetOffSet( SFile.GetfTell() + dwSize );
				return FALSE;
			}break;
		}

		return TRUE;
	}

	BOOL SPET::SAVE( CSerialFile &SFile )	// PetData
	{
		SFile << static_cast<int>( ITEM::SPET::VERSION );
		SFile.BeginBlock(); // 구조체 사이즈를 저장한다.
		{
			SFile << (DWORD)emPetType;
			SFile << dwPetID;
			SFile << (DWORD)emType;
			SFile << sPetID.dwID;
		}
		SFile.EndBlock();

		return TRUE;
	}

	VOID SPET::SaveCsvHead ( std::fstream &SFile )	// PetData
	{
		SFile << "dwPetID" << ",";
		SFile << "emType" << ",";
		SFile << "emPetType" << ",";
		SFile << "sPetID[MID]" << ",";
		SFile << "sPetID[SID]" << ",";
	}

	VOID SPET::SaveCsv ( std::fstream &SFile )	// PetData
	{
		SFile << dwPetID << ",";
		SFile << emType << ",";
		SFile << emPetType << ",";
		SFile << sPetID.wMainID << ",";
		SFile << sPetID.wSubID << ",";
	}

	VOID SPET::LoadCsv ( CStringArray &StrArray, int &iCsvCur )	// PetData
	{
		dwPetID = (DWORD)atol( StrArray[ iCsvCur++ ] );
		emType = (PET_ACCESSORY_TYPE)atoi( StrArray[ iCsvCur++ ] );
		emPetType = (PETTYPE)atoi( StrArray[ iCsvCur++ ] );
		sPetID.wMainID = (WORD)atoi( StrArray[ iCsvCur++ ] );
		sPetID.wSubID = (WORD)atoi( StrArray[ iCsvCur++ ] );
	}
};

namespace COMMENT
{
	std::string ITEMSLOT_PET[PET_ACCETYPE_SIZE] =
	{
		"Pet Slot A",
		"Pet Slot B",
	};
};