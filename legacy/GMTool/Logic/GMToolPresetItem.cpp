#include "stdafx.h"
#include "GMToolPresetItem.h"
#include "IniLoader.h"
#include "Rijndael.h"
#include <shlwapi.h>
#pragma comment( lib, "shlwapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CGMToolPresetItem::CGMToolPresetItem()
{
};

CGMToolPresetItem::~CGMToolPresetItem()
{
};

CGMToolPresetItem& CGMToolPresetItem::GetInstance ()
{
	static CGMToolPresetItem Instance;
	return Instance;
}

BOOL CGMToolPresetItem::PresetLoad()
{
	std::string strFILE  = m_strPATH.c_str();
	strFILE += "\\GMToolPresetItem.ini";

	if ( !PathFileExists( strFILE.c_str() ) )
		return FALSE;

	CIniLoader cFILE;

	if( !cFILE.open( strFILE, true) )
		return FALSE;

	cFILE.SetUseErrorMsgBox( FALSE );

	std::string strSep( ",[]\t" );
	cFILE.reg_sep( strSep );

	m_MapPresetItem.clear();

	DWORD dwNUMPRESET_ITEM = cFILE.GetKeySize( "PRESETS", "ITEM_PRESET" );

	for( DWORD i=0; i<dwNUMPRESET_ITEM; ++i )
	{
		int nKey = 18;
		SGMTOOL_PRESET_ITEM sPRESET;
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 0, nKey, sPRESET.wID );
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 1, nKey, sPRESET.strNAME );
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 2, nKey, sPRESET.cDAMAGE );
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 3, nKey, sPRESET.cDEFENSE );
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 4, nKey, sPRESET.cRESFIRE );
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 5, nKey, sPRESET.cRESICE );
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 6, nKey, sPRESET.cRESELECT );
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 7, nKey, sPRESET.cRESPOISON );
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 8, nKey, sPRESET.cRESSPIRIT );
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 9, nKey, sPRESET.cOPTIONTYPE1 );
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 10, nKey, sPRESET.cOPTIONTYPE2 );
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 11, nKey, sPRESET.cOPTIONTYPE3 );
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 12, nKey, sPRESET.cOPTIONTYPE4 );
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 13, nKey, sPRESET.nOPTIONVAL1 );
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 14, nKey, sPRESET.nOPTIONVAL2 );
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 15, nKey, sPRESET.nOPTIONVAL3 );
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 16, nKey, sPRESET.nOPTIONVAL4 );
		cFILE.getflag( i, "PRESETS", "ITEM_PRESET", 17, nKey, sPRESET.bNONDROP );

		m_MapPresetItem[sPRESET.wID] = sPRESET;
	}

	return TRUE;
};

BOOL CGMToolPresetItem::PresetSave()
{
	CString strBuffer;

	strBuffer += _T(";GMTOOL Presets File\n");
	strBuffer += _T(";\n");
	strBuffer += _T("[PRESETS]\n");

	std::string strSep = ",";
	SGMTOOL_PRESET_ITEM_MAP_ITER iter_start = m_MapPresetItem.begin();
	SGMTOOL_PRESET_ITEM_MAP_ITER iter_end = m_MapPresetItem.end();
	for( ; iter_start != iter_end; ++ iter_start )
	{
		const  SGMTOOL_PRESET_ITEM sPRESETITEM = (*iter_start).second; 

		strBuffer.AppendFormat( "ITEM_PRESET	=	%d%s%s%s%d%s%d%s%d%s%d%s%d%s%d%s%d%s%d%s%d%s%d%s%d%s%d%s%d%s%d%s%d%s%d \n", 
		sPRESETITEM.wID, strSep.c_str(),
		sPRESETITEM.strNAME.c_str(), strSep.c_str(),
		sPRESETITEM.cDAMAGE, strSep.c_str(),
		sPRESETITEM.cDEFENSE, strSep.c_str(),
		sPRESETITEM.cRESFIRE, strSep.c_str(),
		sPRESETITEM.cRESICE, strSep.c_str(),
		sPRESETITEM.cRESELECT, strSep.c_str(),
		sPRESETITEM.cRESPOISON, strSep.c_str(),
		sPRESETITEM.cRESSPIRIT, strSep.c_str(),
		sPRESETITEM.cOPTIONTYPE1, strSep.c_str(),
		sPRESETITEM.cOPTIONTYPE2, strSep.c_str(),
		sPRESETITEM.cOPTIONTYPE3, strSep.c_str(),
		sPRESETITEM.cOPTIONTYPE4, strSep.c_str(),
		sPRESETITEM.nOPTIONVAL1, strSep.c_str(),
		sPRESETITEM.nOPTIONVAL2, strSep.c_str(),
		sPRESETITEM.nOPTIONVAL3, strSep.c_str(),
		sPRESETITEM.nOPTIONVAL4, strSep.c_str(),
		sPRESETITEM.bNONDROP );
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
	strFILE += "\\GMToolPresetItem.ini";

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

SGMTOOL_PRESET_ITEM* CGMToolPresetItem::PresetItemGet( WORD wID )
{
	SGMTOOL_PRESET_ITEM_MAP_ITER iter = m_MapPresetItem.find( wID );
	if ( iter != m_MapPresetItem.end() ){
		return &(iter->second);
	}

	return NULL;
}

BOOL CGMToolPresetItem::PresetItemAdd( SGMTOOL_PRESET_ITEM sPRESET )
{
	SGMTOOL_PRESET_ITEM* pPRESET = PresetItemGet( sPRESET.wID );
	if ( pPRESET ){
		return FALSE;
	}

	m_MapPresetItem[sPRESET.wID] = sPRESET;
	return TRUE;
}

void CGMToolPresetItem::PresetItemDel( WORD wID )
{
	SGMTOOL_PRESET_ITEM_MAP_ITER iter = m_MapPresetItem.find( wID );
	if ( iter != m_MapPresetItem.end() ){
		m_MapPresetItem.erase( iter );
	}
}

void CGMToolPresetItem::PresetItemDelAll()
{
	m_MapPresetItem.clear();
}

WORD CGMToolPresetItem::PresetItemNewID()
{
	for( int i=0; i<MAX_PRESET_ITEM_ID; ++ i ){
		SGMTOOL_PRESET_ITEM* pPRESET = PresetItemGet( i );
		if ( !pPRESET ){
			return i;
		}
	}

	return MAX_PRESET_ITEM_ID;
}
