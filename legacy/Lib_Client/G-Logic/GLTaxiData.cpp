#include "stdafx.h"
#include "GLTaxiData.h"
#include "StringUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL STAXI_STATION::LOAD ( basestream &SFile )
{
	DWORD dwVer(0);

	SFile >> dwVer;

	if ( dwVer == VERSION )
	{
		SFile >> dwLINKID;
		SFile >> bUsed;
		SFile >> dwMAPID;
		SFile >> wPosX;
		SFile >> wPosY;
		SFile >> dwNPCID;
		SFile >> dwMapCharge;
		SFile >> strMAP;
		SFile >> strSTATION;
	}
	else
	{
		CDebugSet::ErrorVersion( "STAXI_STATION::LOAD", dwVer );
	}

	return TRUE;
}

BOOL STAXI_STATION::SAVE ( CSerialFile &SFile )
{
	SFile << (DWORD)VERSION;

	SFile << dwLINKID;
	SFile << bUsed;
	SFile << dwMAPID;
	SFile << wPosX;
	SFile << wPosY;
	SFile << dwNPCID;
	SFile << dwMapCharge;
	SFile << strMAP;
	SFile << strSTATION;

	return TRUE;
}
