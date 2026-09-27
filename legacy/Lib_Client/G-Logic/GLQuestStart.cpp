#include "stdafx.h"
#include "GLQuestStart.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool GLQUEST_START::LOAD_001( basestream &SFile )
{
	SFile >> dwCLASS;
	SFile >> wSCHOOL;

	SFile >> nidITEM.dwID;
	SFile >> nidSKILL.dwID;

	SFile >> nidMAP.dwID;
	SFile >> wPosX;
	SFile >> wPosY;
	SFile >> wSIZE;

	SFile >> wLEVEL;

	return true;
}

bool GLQUEST_START::LOAD_002( basestream &SFile )
{
	SFile >> dwCLASS;
	SFile >> wSCHOOL;

	SFile >> nidITEM.dwID;
	SFile >> nidSKILL.dwID;

	SFile >> nidMAP.dwID;
	SFile >> wPosX;
	SFile >> wPosY;
	SFile >> wSIZE;

	SFile >> wLEVEL;

	return true;
}

bool GLQUEST_START::LOAD_003( basestream &SFile )
{
	SFile >> dwCLASS;
	SFile >> wSCHOOL;
	SFile >> wLEVEL;
	SFile >> nidSKILL.dwID;
	SFile >> nidITEM.dwID;
	SFile >> nidMAP.dwID;
	SFile >> wSIZE;
	SFile >> wPosX;
	SFile >> wPosY;

	return true;
}

bool GLQUEST_START::LOAD_004( basestream &SFile )
{
	SFile >> dwCLASS;
	SFile >> wSCHOOL;
	SFile >> wLEVEL;
	SFile >> nidSKILL.dwID;
	SFile >> nidITEM.dwID;
	SFile >> nidMAP.dwID;
	SFile >> wSIZE;
	SFile >> wPosX;
	SFile >> wPosY;
	SFile >> nJOIN;

	return true;
}

bool GLQUEST_START::LOAD ( basestream &SFile )
{
	DWORD dwVERSION(0), dwSIZE(0);

	SFile >> dwVERSION;
	SFile >> dwSIZE;

	switch ( dwVERSION )
	{
	case VERSION:	LOAD_004( SFile ); break;
	case 0x0003:	LOAD_003( SFile ); break;
	case 0x0002:	LOAD_002( SFile ); break;
	case 0x0001:	LOAD_001( SFile ); break;
	default:
		{
			CDebugSet::ErrorVersion( "GLQUEST_START::LOAD", dwVERSION );
			SFile.SetOffSet ( SFile.GetfTell()+dwSIZE );
			break;
		}
	}
	
	return true;
}

bool GLQUEST_START::SAVE ( CSerialFile &SFile )
{
	SFile << (DWORD) VERSION;
	SFile.BeginBlock ();
	{
		SFile << dwCLASS;
		SFile << wSCHOOL;
		SFile << wLEVEL;
		SFile << nidSKILL.dwID;
		SFile << nidITEM.dwID;
		SFile << nidMAP.dwID;
		SFile << wSIZE;
		SFile << wPosX;
		SFile << wPosY;

		SFile << nJOIN;
	}
	SFile.EndBlock ();

	return true;
}