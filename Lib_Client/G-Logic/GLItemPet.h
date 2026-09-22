#ifndef GLITEM_PET_H_
#define GLITEM_PET_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "../../Lib_Engine/Common/BaseStream.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"
#include "../../Lib_Engine/G-Logic/GLDefine.h"

#include "./GLItemDef.h"

enum PETTYPE
{
	PET_TYPE_NONE   = -1,
	PET_TYPE_A		= 0,
	PET_TYPE_B		= 1,
	PET_TYPE_C		= 2,
	PET_TYPE_D		= 3,
	PET_TYPE_E		= 4,
	PET_TYPE_F		= 5,

	PET_TYPE_G		= 6,
	PET_TYPE_H		= 7,
	PET_TYPE_I		= 8,
	PET_TYPE_J		= 9,
	PET_TYPE_K		= 10,
	PET_TYPE_L		= 11,
	PET_TYPE_M		= 12,
	PET_TYPE_N		= 13,
	//new types of pet
	PET_TYPE_AA		= 14,
	PET_TYPE_AB		= 15,
	PET_TYPE_AC		= 16,
	PET_TYPE_AD		= 17,
	PET_TYPE_AE		= 18,
	PET_TYPE_AF		= 19,
	PET_TYPE_AG		= 20,
	PET_TYPE_AH		= 21,
	PET_TYPE_AI		= 22,
	PET_TYPE_AJ		= 23,
	PET_TYPE_AK		= 24,
	PET_TYPE_AL		= 25,
	PET_TYPE_AM		= 26,
	PET_TYPE_AN		= 27,
	PET_TYPE_AO		= 28,
	PET_TYPE_AP		= 29,
	PET_TYPE_AQ		= 30,
	PET_TYPE_AR		= 31,
	PET_TYPE_AS		= 32,
	PET_TYPE_AT		= 33,
	PET_TYPE_AU		= 34,
	PET_TYPE_AV		= 35,
	PET_TYPE_AW		= 36,
	PET_TYPE_AX		= 37,
	PET_TYPE_AY		= 38,
	PET_TYPE_AZ		= 39,
	PET_TYPE_BA		= 40,
	PET_TYPE_BB		= 41,
	PET_TYPE_BC		= 42,
	PET_TYPE_BD		= 43,
	PET_TYPE_BE		= 44,
	PET_TYPE_BF		= 45,
	PET_TYPE_BG		= 46,
	PET_TYPE_BH		= 47,
	PET_TYPE_BI		= 48,
	PET_TYPE_BJ		= 49,
	PET_TYPE_BK		= 50,
	PET_TYPE_BL		= 51,
	PET_TYPE_BM		= 52,
	PET_TYPE_BN		= 53,
	PET_TYPE_BO		= 54,
	PET_TYPE_BP		= 55,
	PET_TYPE_BQ		= 53,
	PET_TYPE_BR		= 57,
	PET_TYPE_BS		= 58,
	PET_TYPE_BT		= 59,
	PET_TYPE_BU		= 60,
	PET_TYPE_BV		= 61,
	PET_TYPE_BW		= 62,
	PET_TYPE_BX		= 63,
	PET_TYPE_BY		= 64,
	PET_TYPE_BZ		= 65,
	PET_TYPE_CA		= 66,
	PET_TYPE_CB		= 67,
	PET_TYPE_CC		= 68,
	PET_TYPE_CD		= 69,
	PET_TYPE_CE		= 70,
	PET_TYPE_CF		= 71,
	PET_TYPE_CG		= 72,
	PET_TYPE_CH		= 73,
	PET_TYPE_CI		= 74,
	PET_TYPE_CJ		= 75,
	PET_TYPE_CK		= 76,
	PET_TYPE_CL		= 77,
	PET_TYPE_CM		= 78,
	PET_TYPE_CN		= 79,

	PET_TYPE_SIZE   = 80,
};

enum PET_ACCESSORY_TYPE
{
	PET_ACCETYPE_A		= 0,	// 赣府, 个烹
	PET_ACCETYPE_B		= 1,	// 迫, 促府, 部府
	PET_ACCETYPE_SIZE	= 2,
};

namespace ITEM
{
	struct SPET_102
	{
		DWORD				dwPetID;   // 绊蜡ID
		PET_ACCESSORY_TYPE	emType;    // 厩技辑府TYPE
		PETTYPE				emPetType; // TYPE
		SNATIVEID			sPetID;    // CROWID
	};

	struct SPET	// PetData
	{
		enum { VERSION = 0x0103 };

		PETTYPE				emPetType; // TYPE
		DWORD				dwPetID;   // 绊蜡ID
		PET_ACCESSORY_TYPE	emType;    // 厩技辑府TYPE
		SNATIVEID			sPetID;    // CROWID

		SPET() 
			: emPetType(PET_TYPE_SIZE)
			, dwPetID(0)
			, emType(PET_ACCETYPE_SIZE)
			, sPetID(NATIVEID_NULL())
		{
		}

		BOOL LOAD ( basestream &SFile );
		BOOL SAVE ( CSerialFile &SFile );

		static VOID SaveCsvHead ( std::fstream &SFile );
		VOID SaveCsv ( std::fstream &SFile );
		VOID LoadCsv ( CStringArray &StrArray, int &iCsvCur );
	};
};

namespace COMMENT
{
	extern std::string ITEMSLOT_PET[PET_ACCETYPE_SIZE];
};

#endif // GLITEM_PET_H_