#include "stdafx.h"
#include "GLLevelHead.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL SLEVEL_HEAD::LOAD ( basestream &SFile )
{
	DWORD dwVer, dwSize;
	SFile >> dwVer;
	SFile >> dwSize;

	if ( dwVer==VERSION )
	{
		LOAD_0102( SFile, dwSize );
	}
	else if ( dwVer==0x0101 )
	{
		LOAD_0101( SFile, dwSize );
	}
	else
	{
		CDebugSet::ErrorVersion( "SLEVEL_HEAD::LOAD", dwVer );
		SFile.SetOffSet ( SFile.GetfTell()+dwSize );
	}

	return TRUE;
}

void SLEVEL_HEAD::LOAD_0101 ( basestream &SFile, DWORD dwSIZE )
{
	DWORD dwRead = SFile.GetfTell();

	SFile >> m_strWldFile;
	SFile >> m_strMapName;

	DWORD dwData(0);
	SFile >> dwData;	m_emBright = (EMBRIGHT) dwData;
	SFile >> dwData;	m_eDivision = (EMDIVISION) dwData;

	dwRead = SFile.GetfTell() - dwRead;

	if( dwRead != dwSIZE )
		CDebugSet::MsgBox( "SLEVEL_HEAD::LOAD_0101 Incorrect Read Size:%d Expected:%d", dwRead, dwSIZE );
}

void SLEVEL_HEAD::LOAD_0102 ( basestream &SFile, DWORD dwSIZE )
{
	DWORD dwRead = SFile.GetfTell();

	SFile >> m_strMapName;
	SFile >> m_strWldFile;
	
	DWORD dwData(0);
	SFile >> dwData;	m_eDivision = (EMDIVISION) dwData;
	SFile >> dwData;	m_emBright = (EMBRIGHT) dwData;
	
	dwRead = SFile.GetfTell() - dwRead;
	
	if( dwRead != dwSIZE )
		CDebugSet::MsgBox( "SLEVEL_HEAD::LOAD_0102 Incorrect Read Size:%d Expected:%d", dwRead, dwSIZE );
}

BOOL SLEVEL_HEAD::SAVE ( CSerialFile &SFile )
{
	SFile << DWORD(VERSION);
	SFile.BeginBlock();
	{
		SFile << m_strMapName;
		SFile << m_strWldFile;
		SFile << (DWORD)m_eDivision;
		SFile << (DWORD)m_emBright;
	}
	SFile.EndBlock();

	return TRUE;
}

void SLEVEL_HEAD::Assign ( const SLEVEL_HEAD_100 &sOldData )
{
	m_strWldFile = sOldData.m_szWldFile;
	m_strMapName = sOldData.m_szMapName;
	m_eDivision = sOldData.m_eDivision;
}
