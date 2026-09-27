#include "stdafx.h"
#include "GLItemSkillBook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ITEM
{
	void SSKILLBOOK::Assign ( SSKILLBOOK_100 &sOld )
	{
		sSkill_ID = sOld.sSkill_ID;
	}

	void SSKILLBOOK::Assign ( SSKILLBOOK_101 &sOld )
	{
		sSkill_ID = sOld.sSkill_ID;
	}

	VOID SSKILLBOOK::SaveCsvHead ( std::fstream &SFile )
	{
		SFile << "sSkill_ID wMainID" << ",";
		SFile << "sSkill_ID wSubID" << ",";
		SFile << "wVal" << ",";
	}

	VOID SSKILLBOOK::SaveCsv ( std::fstream &SFile )
	{
		SFile << sSkill_ID.wMainID << ",";
		SFile << sSkill_ID.wSubID << ",";
		SFile << wVal << ",";
	}

	VOID SSKILLBOOK::LoadCsv ( CStringArray &StrArray, int &iCsvCur )
	{
		sSkill_ID.wMainID = (WORD)atoi( StrArray[ iCsvCur++ ] );
		sSkill_ID.wSubID = (WORD)atoi( StrArray[ iCsvCur++ ] );
		wVal = (WORD)atoi( StrArray[ iCsvCur++ ] );
	}
};