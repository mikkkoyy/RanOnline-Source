#ifndef GLITEM_GRINDING_H_
#define GLITEM_GRINDING_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "../../Lib_Engine/Common/BaseStream.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"
#include "../../Lib_Engine/G-Logic/GLDefine.h"

#include "./GLItemDef.h"

namespace ITEM
{
	struct SGRINDING_100
	{
		SGRIND				sNO[EMGRINDING_NO];
	};

	struct SGRINDING_101
	{
		EMGRINDING_CLASS	emCLASS;
		SGRIND				sNO[EMGRINDING_NO];
	};

	struct SGRINDING_102
	{
		EMGRINDING_CLASS	emCLASS;
		bool				bHIGH;
		EMGRINDING_TYPE		emTYPE;
	};

	struct SGRINDING_103
	{
		EMGRINDING_CLASS	emCLASS;
		EMGRINDING_LEVEL	emLEVEL;
		EMGRINDING_TYPE		emTYPE;
	};

	struct SGRINDING_104
	{
		EMGRINDING_LEVEL	emLEVEL;
		EMGRINDING_TYPE		emTYPE;
		EMGRINDING_CLASS	emCLASS;
	};

	//	연마 아이템, 관련 데이터.
	struct SGRINDING
	{
		enum { VERSION = 0x0200 };

		EMGRINDING_LEVEL	emLEVEL;
		EMGRINDING_TYPE		emTYPE;
		EMGRINDING_CLASS	emCLASS;
		std::string			strGrind;
		BOOL				bNoFail; /*no fail upgrade, Juver, 2017/11/26 */

		SGRINDING ()
			: emLEVEL(EMGRINDING_LEVEL_NORMAL)
			, emTYPE(EMGRINDING_NONE)
			, emCLASS(EMGRINDING_CLASS_ARM)
			, bNoFail(FALSE) /*no fail upgrade, Juver, 2017/11/26 */
		{
			strGrind = "";
		}

		void Assign ( SGRINDING_100 &sOldData )
		{
			strGrind = "";
		}

		void Assign ( SGRINDING_101 &sOldData )
		{
			emCLASS = sOldData.emCLASS;

			switch ( sOldData.sNO[0].emTYPE )
			{
			case 3:	emTYPE = EMGRINDING_DAMAGE;		break;
			case 4:	emTYPE = EMGRINDING_DEFENSE;	break;
			};

			strGrind = "";
		}

		void Assign (  SGRINDING_102 &sOldData )
		{
			emCLASS = sOldData.emCLASS;
			emTYPE = sOldData.emTYPE;

			if ( !sOldData.bHIGH )	emLEVEL = EMGRINDING_LEVEL_NORMAL;
			else emLEVEL = EMGRINDING_LEVEL_HIGH;

			strGrind = "";
		}

		void Assign (  SGRINDING_103 &sOldData )
		{
			emLEVEL = sOldData.emLEVEL;
			emTYPE = sOldData.emTYPE;
			emCLASS = sOldData.emCLASS;

			strGrind = "";
		}

		void Assign (  SGRINDING_104 &sOldData )
		{
			emLEVEL = sOldData.emLEVEL;
			emTYPE = sOldData.emTYPE;
			emCLASS = sOldData.emCLASS;

			strGrind = "";
		}

		bool LOAD_0105 ( basestream &SFile );
		bool LOAD ( basestream &SFile );
		bool SAVE ( CSerialFile &SFile );

		static VOID SaveCsvHead ( std::fstream &SFile );
		VOID SaveCsv ( std::fstream &SFile );
		VOID LoadCsv ( CStringArray &StrArray, int &iCsvCur );
	};
};

#endif // GLITEM_GRINDING_H_