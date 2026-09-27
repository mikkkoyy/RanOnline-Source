#include "stdafx.h"
#include "GMToolPresetSkill.h"
#include "IniLoader.h"
#include "Rijndael.h"
#include <shlwapi.h>

#pragma comment( lib, "shlwapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CGMToolPresetSkill::CGMToolPresetSkill()
{
};

CGMToolPresetSkill::~CGMToolPresetSkill()
{
};

CGMToolPresetSkill& CGMToolPresetSkill::GetInstance ()
{
	static CGMToolPresetSkill Instance;
	return Instance;
}

BOOL CGMToolPresetSkill::PresetLoad()
{
	std::string strFILE  = m_strPATH.c_str();
	strFILE += "\\GMToolPresetSkill.ini";

	if ( !PathFileExists( strFILE.c_str() ) )
		return FALSE;

	CIniLoader cFILE;

	if( !cFILE.open( strFILE, true) )
		return FALSE;

	cFILE.SetUseErrorMsgBox( FALSE );

	std::string strSep( ",[]\t" );
	cFILE.reg_sep( strSep );

	m_MapPresetSkill.clear();

	DWORD dwNUMPRESET_SKILL = cFILE.GetKeySize( "PRESETS", "SKILL_PRESET" );
	for( DWORD i=0; i<dwNUMPRESET_SKILL; ++i )
	{
		int nKey = 2;
		SGMTOOL_PRESET_SKILL sPRESET;
		cFILE.getflag( i, "PRESETS", "SKILL_PRESET", 0, nKey, sPRESET.wID );
		cFILE.getflag( i, "PRESETS", "SKILL_PRESET", 1, nKey, sPRESET.strNAME );
		m_MapPresetSkill[sPRESET.wID] = sPRESET;
	}

	DWORD dwNUMPRESET_DATA = cFILE.GetKeySize( "PRESETS_DATA", "SKILL_PRESET_DATA" );
	for( DWORD i=0; i<dwNUMPRESET_DATA; ++i )
	{
		int nKey = 3;
		int nID = -1;
		SGMTOOL_PRESET_SKILL_DATA sPRESET_DATA;
		cFILE.getflag( i, "PRESETS_DATA", "SKILL_PRESET_DATA", 0, nKey, nID );
		cFILE.getflag( i, "PRESETS_DATA", "SKILL_PRESET_DATA", 1, nKey, sPRESET_DATA.sSKILLID.dwID );
		cFILE.getflag( i, "PRESETS_DATA", "SKILL_PRESET_DATA", 2, nKey, sPRESET_DATA.wLEVEL );

		if ( nID < 0 ){
			continue;
		}

		SGMTOOL_PRESET_SKILL* pPreset = PresetSkillGet( nID );
		if ( !pPreset ){
			continue;
		}

		pPreset->mapData[sPRESET_DATA.sSKILLID.dwID] = sPRESET_DATA;
	}

	return TRUE;
};

BOOL CGMToolPresetSkill::PresetSave()
{
	CString strBuffer;

	strBuffer += _T(";GMTOOL Presets File\n");
	strBuffer += _T(";\n");
	strBuffer += _T("[PRESETS]\n");

	std::string strSep = ",";

	SGMTOOL_PRESET_SKILL_MAP_ITER iter_preset = m_MapPresetSkill.begin();
	for( ; iter_preset != m_MapPresetSkill.end(); ++ iter_preset )
	{
		const  SGMTOOL_PRESET_SKILL sPRESET = (*iter_preset).second; 
		strBuffer.AppendFormat( "SKILL_PRESET	=	%d%s%s \n", 
			sPRESET.wID, 
			strSep.c_str(), 
			sPRESET.strNAME.c_str() ); 
	}

	strBuffer += _T("[PRESETS_DATA]\n");

	iter_preset = m_MapPresetSkill.begin();
	for( ; iter_preset != m_MapPresetSkill.end(); ++ iter_preset )
	{
		SGMTOOL_PRESET_SKILL sPRESET = (*iter_preset).second; 

		SGMTOOL_PRESET_SKILL_DATA_MAP_ITER iter_data = sPRESET.mapData.begin();
		for( ; iter_data != sPRESET.mapData.end(); ++ iter_data )
		{
			const SGMTOOL_PRESET_SKILL_DATA sDATA = (*iter_data).second; 
			strBuffer.AppendFormat( "SKILL_PRESET_DATA	=	%d%s%d%s%d \n", 
				sPRESET.wID, 
				strSep.c_str(),
				sDATA.sSKILLID.dwID, 
				strSep.c_str(),
				sDATA.wLEVEL ); 
		}
	}

	CRijndael oRijndael;
	oRijndael.Initialize( CRijndael::VERSION, CRijndael::sm_Version[CRijndael::VERSION-1].c_str(), CRijndael::sm_KeyLength[CRijndael::VERSION-1] );

	int nLen = oRijndael.GetEncryptStringLength( strBuffer );
	char * szBuffer = new char[nLen];
	memset( szBuffer, 0, nLen );

	oRijndael.EncryptEx( strBuffer.GetString(), szBuffer, strBuffer.GetLength() );
	int nVersion = CRijndael::VERSION;

	if ( nVersion <= 0 && strlen( szBuffer ) <= 0 )	return FALSE;

	std::string strFILE = m_strPATH.c_str();
	strFILE += "\\GMToolPresetSkill.ini";

	CFile file;
	if ( ! file.Open( strFILE.c_str(), CFile::modeCreate|CFile::modeWrite ) )
	{
		delete [] szBuffer;
		return FALSE;
	}

	file.Write( &nVersion, sizeof(int) );
	file.Write( szBuffer, nLen );
	file.Close();

	delete [] szBuffer;

	return TRUE;
}

SGMTOOL_PRESET_SKILL* CGMToolPresetSkill::PresetSkillGet( WORD wID )
{
	SGMTOOL_PRESET_SKILL_MAP_ITER iter = m_MapPresetSkill.find( wID );
	if ( iter != m_MapPresetSkill.end() ){
		return &(iter->second);
	}

	return NULL;
}

BOOL CGMToolPresetSkill::PresetSkillAdd( SGMTOOL_PRESET_SKILL sPRESET )
{
	SGMTOOL_PRESET_SKILL* pPRESET = PresetSkillGet( sPRESET.wID );
	if ( pPRESET ){
		return FALSE;
	}

	m_MapPresetSkill[sPRESET.wID] = sPRESET;
	return TRUE;
}

void CGMToolPresetSkill::PresetSkillDel( WORD wID )
{
	SGMTOOL_PRESET_SKILL_MAP_ITER iter = m_MapPresetSkill.find( wID );
	if ( iter != m_MapPresetSkill.end() )	{
		m_MapPresetSkill.erase( iter );
	}
}

void CGMToolPresetSkill::PresetSkillDelAll()
{
	m_MapPresetSkill.clear();
}

WORD CGMToolPresetSkill::PresetSkillNewID()
{
	for( int i=0; i<MAX_PRESET_SKILL_ID; ++ i )
	{
		SGMTOOL_PRESET_SKILL* pPRESET = PresetSkillGet( i );
		if ( !pPRESET ){
			return i;
		}
	}

	return MAX_PRESET_SKILL_ID;
}
