#ifndef GLQUEST_GENITEM_H_
#define GLQUEST_GENITEM_H_

#if _MSC_VER > 1000
#pragma once
#endif


#include "../../Lib_Engine/Common/BaseStream.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"
#include "../../Lib_Engine/G-Logic/GLDefine.h"

#include <vector>

typedef std::vector<DWORD>			DWQARRAY;
typedef DWQARRAY::iterator			DWQARRAY_ITER;

struct SGENQUESTITEM
{
	enum { VERSION = 0x0001, };

	SNATIVEID	sNID;
	WORD		wNUM;
	float		fGEN_RATE;
	DWQARRAY	vecGEN_MOB;

	SGENQUESTITEM () :
		sNID(false),
		wNUM(1),
		fGEN_RATE(10.0f)
	{
	}

	SGENQUESTITEM& operator= ( const SGENQUESTITEM& rvalue );

	bool Find ( DWORD dwMOB );
	void GENMOB_ERASE ( DWORD dwIndex );

	bool LOAD ( basestream &SFile );
	bool SAVE ( CSerialFile &SFile );
};


#endif // GLQUEST_GENITEM_H_