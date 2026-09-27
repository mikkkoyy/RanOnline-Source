#ifndef GLITEM_RVCARD_H_
#define GLITEM_RVCARD_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "../../Lib_Engine/Common/BaseStream.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"
#include "../../Lib_Engine/G-Logic/GLDefine.h"
#include "GLItemDef.h"

namespace ITEM
{
	struct SRVCARD
	{
		enum { VERSION = 0x0001 };

		EMRANDOM_OPT	emOption;
		WORD			wValue;
		BOOL			bCheckExist;
		BOOL			bReplaceOpt;
		BOOL			bUseSuit[SUIT_NSIZE];

		SRVCARD() 
			: emOption(EMR_OPT_NULL)
			, wValue(0)
			, bCheckExist(FALSE)
			, bReplaceOpt(FALSE)
		{
			for( int i=0; i<SUIT_NSIZE; ++i )
				bUseSuit[i] = FALSE;
		}

		SRVCARD& operator = ( const SRVCARD& rvalue );

		BOOL LOAD ( basestream &SFile );
		BOOL SAVE ( CSerialFile &SFile );

		static VOID SaveCsvHead ( std::fstream &SFile );
		VOID SaveCsv ( std::fstream &SFile );
		VOID LoadCsv ( CStringArray &StrArray, int &iCsvCur );
	};
};


#endif // GLITEM_RVCARD_H_