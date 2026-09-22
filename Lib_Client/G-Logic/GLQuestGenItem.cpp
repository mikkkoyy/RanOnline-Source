#include "stdafx.h"
#include "GLQuestGenItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SGENQUESTITEM& SGENQUESTITEM::operator= ( const SGENQUESTITEM& rvalue )
{
	sNID = rvalue.sNID;
	wNUM = rvalue.wNUM;
	fGEN_RATE = rvalue.fGEN_RATE;
	vecGEN_MOB = rvalue.vecGEN_MOB;

	return *this;
}

bool SGENQUESTITEM::Find ( DWORD dwMOB )
{
	bool bfound = std::binary_search ( vecGEN_MOB.begin(), vecGEN_MOB.end(), dwMOB );
	return bfound;
}

void SGENQUESTITEM::GENMOB_ERASE ( DWORD dwIndex )
{
	GASSERT ( vecGEN_MOB.size() > dwIndex && "GLQUEST_STEP::GENMOB_ERASE()" );
	if ( vecGEN_MOB.size() <= dwIndex )	return;

	DWQARRAY_ITER iter = vecGEN_MOB.begin()+dwIndex;
	vecGEN_MOB.erase ( iter, iter+1 );
}

bool SGENQUESTITEM::LOAD ( basestream &SFile )
{
	DWORD dwVER(0);
	SFile >> dwVER;

	if ( dwVER==VERSION )
	{
		SFile >> sNID.dwID;
		SFile >> wNUM;
		SFile >> fGEN_RATE;
		SFile >> vecGEN_MOB;

		std::sort ( vecGEN_MOB.begin(), vecGEN_MOB.end() );
	}
	else
	{
		CDebugSet::ErrorVersion( "SGENQUESTITEM::LOAD", dwVER );
	}

	return true;
}

bool SGENQUESTITEM::SAVE ( CSerialFile &SFile )
{
	SFile << (DWORD) VERSION;
	SFile << sNID.dwID;
	SFile << wNUM;
	SFile << fGEN_RATE;
	SFile << vecGEN_MOB;

	return true;
}