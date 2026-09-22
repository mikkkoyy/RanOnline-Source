#ifndef GLITEM_RANDOMBOX_H_
#define GLITEM_RANDOMBOX_H_

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
	struct SRANDOMITEM_100
	{
		float		fRATE;
		SNATIVEID	nidITEM;

		SRANDOMITEM_100()
			: fRATE(0)
		{
		}
	};

	struct SRANDOMITEM
	{
		SNATIVEID	nidITEM;
		float		fRATE;

		SRANDOMITEM()
			: fRATE(0)
		{
		}
	};

	struct SRANDOMBOX
	{
		enum { VERSION = 0x0101 };

		std::vector<SRANDOMITEM>	vecBOX;

		BOOL LOAD ( basestream &SFile );
		BOOL SAVE ( CSerialFile &SFile );

		static VOID SaveCsvHead ( std::fstream &SFile );
		VOID SaveCsv ( std::fstream &SFile );
		VOID LoadCsv ( CStringArray &StrArray, int &iCsvCur  );

		void CLEAR ();
		bool VALID ();

		bool INSERT ( const SNATIVEID &nidITEM, float fRATE );
		bool DEL ( int nIndex );
	};
};

#endif // GLITEM_RANDOMBOX_H_