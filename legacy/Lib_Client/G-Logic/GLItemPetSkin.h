#ifndef GLITEM_PETSKIN_H_
#define GLITEM_PETSKIN_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "../../Lib_Engine/Common/BaseStream.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"
#include "../../Lib_Engine/G-Logic/GLDefine.h"

namespace ITEM
{
	struct SPETSKINPACKITEMDATA
	{
		SNATIVEID sMobID; // ¹Ù²ð ¸÷ ID
		float	  fScale; // ¹Ù²ï ¸÷ÀÇ Å©±â
		float	  fRate;  // ¹Ù²ð È®·ü

		SPETSKINPACKITEMDATA() :
		sMobID(NATIVEID_NULL()),
			fScale(1.0f),
			fRate(0.0f)			
		{
		}
	};

	struct SPETSKINPACKITEM	// PetData
	{
		enum { VERSION = 0x0101, PETSKIN_MAX = 10 };

		std::vector<SPETSKINPACKITEMDATA> vecPetSkinData;
		DWORD							  dwPetSkinTime; // Àû¿ëµÇ´Â ½Ã°£.

		SPETSKINPACKITEM() :
		dwPetSkinTime(0)
		{
		}

		BOOL LOAD ( basestream &SFile );
		BOOL SAVE ( CSerialFile &SFile );

		static VOID SaveCsvHead ( std::fstream &SFile );
		VOID SaveCsv ( std::fstream &SFile );
		VOID LoadCsv ( CStringArray &StrArray, int &iCsvCur );

		void CLEAR ();
		bool VALID ();

		bool INSERT ( const SNATIVEID &sMobID, float fRate, float fScale );
		bool DEL ( int nIndex );
	};
};

#endif // GLITEM_PETSKIN_H_