#ifndef GLQUEST_START_H_
#define GLQUEST_START_H_

#if _MSC_VER > 1000
#pragma once
#endif


#include "../../Lib_Engine/Common/BaseStream.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"
#include "../../Lib_Engine/G-Logic/GLDefine.h"

#include "./GLCharDefine.h"
#include "./GLogicData.h"

struct GLQUEST_START
{
	enum { VERSION = 0x0004, };

	DWORD		dwCLASS;
	WORD		wSCHOOL;
	WORD		wLEVEL;
	SNATIVEID	nidSKILL;
	SNATIVEID	nidITEM;
	SNATIVEID	nidMAP;
	WORD		wSIZE;
	WORD		wPosX;
	WORD		wPosY;
	int			nJOIN;

	GLQUEST_START () :
		dwCLASS(GLCC_ALL_8CLASS),
		wSCHOOL(MAX_SCHOOL),
		wLEVEL(USHRT_MAX),
		nidSKILL(false),
		nidITEM(false),
		nidMAP(false),
		wSIZE(0),
		wPosX(USHRT_MAX),
		wPosY(USHRT_MAX),
		nJOIN(EMQJ_NONE)
	{
	}

	bool LOAD ( basestream &SFile );
	bool LOAD_001( basestream &SFile );
	bool LOAD_002( basestream &SFile );
	bool LOAD_003( basestream &SFile );
	bool LOAD_004( basestream &SFile );
	bool SAVE ( CSerialFile &SFile );
};


#endif // GLQUEST_START_H_