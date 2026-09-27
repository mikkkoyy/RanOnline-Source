#ifndef GLITEM_QUESTION_H_
#define GLITEM_QUESTION_H_

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
	struct SQUESTIONITEM_100
	{
		EMITEM_QUESTION	emType;
		float	fTime;
		float	fExp;
		WORD	wParam1;
		WORD	wParam2;
	};

	struct SQUESTIONITEM // by °æ´ë
	{
		enum { VERSION = 0x0101 };

		EMITEM_QUESTION	emType;
		WORD	wParam1;
		WORD	wParam2;
		float	fTime;
		float	fExp; // °æÇèÄ¡ È¹µæ 0.04~0.1%

		SQUESTIONITEM() 
			: emType(QUESTION_NONE)
			, wParam1(0)
			, wParam2(0)
			, fTime(0.0f)
			, fExp(0.0f)
		{
		}

		BOOL LOAD ( basestream &SFile );
		BOOL SAVE ( CSerialFile &SFile );

		static VOID SaveCsvHead ( std::fstream &SFile );
		VOID SaveCsv ( std::fstream &SFile );
		VOID LoadCsv ( CStringArray &StrArray, int &iCsvCur  );
	};
};

#endif // GLITEM_QUESTION_H_