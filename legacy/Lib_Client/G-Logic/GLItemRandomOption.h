#ifndef GLITEM_RANDOMOPTION_H_
#define GLITEM_RANDOMOPTION_H_

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
	struct SRANDOM_OPT
	{
		enum { VERSION = 0x0102 };

		char	szNAME[SRANDOM_DATA::NAME_LEN];
		//ran gs update
		bool	bUnableRandomCard;

		bool	bPVPItem;

		SRANDOM_OPT ()
		{
			memset(szNAME, 0, sizeof(char) * SRANDOM_DATA::NAME_LEN);
		}

		BOOL LOAD ( basestream &SFile );
		BOOL SAVE ( CSerialFile &SFile );

		static VOID SaveCsvHead ( std::fstream &SFile );
		VOID SaveCsv ( std::fstream &SFile );
		VOID LoadCsv ( CStringArray &StrArray, int &iCsvCur );
	};
};

#endif // GLITEM_RANDOMOPTION_H_