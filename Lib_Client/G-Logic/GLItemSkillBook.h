#ifndef GLITEM_SKILLBOOK_H_
#define GLITEM_SKILLBOOK_H_

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
	struct SSKILLBOOK_100
	{
		SNATIVEID	sSkill_ID;
	};

	struct SSKILLBOOK_101
	{
		SNATIVEID	sSkill_ID;
	};

	//	Skill 서적, 관련 데이터.
	struct SSKILLBOOK
	{
		enum { VERSION = 0x0102 };

		SNATIVEID	sSkill_ID;		//	습득 Skill.
		WORD		wVal;			//official unknown data

		SSKILLBOOK ()
			: sSkill_ID(NATIVEID_NULL())
			, wVal(0)
		{
		}

		void Assign ( SSKILLBOOK_100 &sOld );
		void Assign ( SSKILLBOOK_101 &sOld );

		static VOID SaveCsvHead ( std::fstream &SFile );
		VOID SaveCsv ( std::fstream &SFile );
		VOID LoadCsv ( CStringArray &StrArray, int &iCsvCur );
	};
};

#endif // GLITEM_SKILLBOOK_H_