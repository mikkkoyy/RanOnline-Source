#include "StdAfx.h"
#include "./GLTaxiStation.h"
#include "./GLogicData.h"

#include "../../Lib_Engine/Common/IniLoader.h"
#include "../../Lib_Engine/G-Logic/GLogic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const char* GLTaxiStation::_FILEHEAD = "GLTAXI_STATION";

GLTaxiStation& GLTaxiStation::GetInstance ()
{
	static GLTaxiStation _instance;
	return _instance;
}

GLTaxiStation::GLTaxiStation(void)
	: m_dwBasicCharge ( 0 )
{
	memset(m_szFileName, 0, sizeof(char) * (MAX_PATH));
}

GLTaxiStation::~GLTaxiStation(void)
{
}

void GLTaxiStation::insert ( STAXI_STATION &sSTATION )
{
	//	Note : 기존에 등록된 맵 목록이 있는지 검사
	//
	DWORD dwSIZE = (DWORD) m_vecTaxiMap.size();
	for ( DWORD i=0; i<dwSIZE; ++i )
	{
		STAXI_MAP &sTaxiMap = m_vecTaxiMap[i];
		if ( sTaxiMap.dwMAPID == sSTATION.dwMAPID )
		{
			//	Note : 기존 마을에 등록.
			//
			sTaxiMap.vecTaxiStation.push_back ( sSTATION );
			return;
		}
	}

	//	Note : 새로운 마을 등록.
	//
	STAXI_MAP sTaxiMap_NEW;
	sTaxiMap_NEW.dwMAPID = sSTATION.dwMAPID;
	sTaxiMap_NEW.strMAP = sSTATION.strMAP;
	sTaxiMap_NEW.vecTaxiStation.push_back ( sSTATION );
	m_vecTaxiMap.push_back ( sTaxiMap_NEW );

	return;
}

bool GLTaxiStation::IMPORT ( std::string strFILE )
{
	std::string strLIST_FILE = strFILE;

	CIniLoader cFILE;
	std::string strSep( ",[]\t" );
	cFILE.reg_sep( strSep );

	if( !cFILE.open( strLIST_FILE, true ) )
	{
		CDebugSet::ToLogFile ( "ERROR : GLTaxiStation::IMPORT(), File Open %s", strFILE.c_str() );
		return false;
	}

	m_vecTaxiData.clear();
	m_vecTaxiMap.clear();

	INT nStationKeySize(0);

	cFILE.getflag( "STATION_LIST_INFO", "StationKeySize", 0, 1, nStationKeySize );
	cFILE.getflag( "STATION_LIST_INFO", "StationBasicCharge", 0, 1, m_dwBasicCharge );
	

	DWORD dwNUM = cFILE.GetKeySize( "STATION_LIST", "STATION" );
	for ( DWORD i=0; i<dwNUM; ++i )
	{

		SNATIVEID sMAPID;
		SNATIVEID sNPCID;
		STAXI_STATION sSTATION;

		cFILE.getflag( i, "STATION_LIST", "STATION", 0, nStationKeySize, sSTATION.bUsed );
		cFILE.getflag( i, "STATION_LIST", "STATION", 1, nStationKeySize, sSTATION.dwLINKID );
		cFILE.getflag( i, "STATION_LIST", "STATION", 2, nStationKeySize, sMAPID.wMainID );
		cFILE.getflag( i, "STATION_LIST", "STATION", 3, nStationKeySize, sMAPID.wSubID );
		cFILE.getflag( i, "STATION_LIST", "STATION", 4, nStationKeySize, sSTATION.wPosX );
		cFILE.getflag( i, "STATION_LIST", "STATION", 5, nStationKeySize, sSTATION.wPosY );
		cFILE.getflag( i, "STATION_LIST", "STATION", 6, nStationKeySize, sNPCID.wMainID );
		cFILE.getflag( i, "STATION_LIST", "STATION", 7, nStationKeySize, sNPCID.wSubID );
		cFILE.getflag( i, "STATION_LIST", "STATION", 8, nStationKeySize, sSTATION.dwMapCharge );
		cFILE.getflag( i, "STATION_LIST", "STATION", 9, nStationKeySize, sSTATION.strMAP );
		cFILE.getflag( i, "STATION_LIST", "STATION", 10, nStationKeySize, sSTATION.strSTATION );

		sSTATION.dwMAPID = sMAPID.dwID;
		sSTATION.dwNPCID = sNPCID.dwID;
		m_vecTaxiData.push_back( sSTATION );
	}

	for( int i=0; i<(int)m_vecTaxiData.size(); ++i )
	{
		STAXI_STATION sSTATION = m_vecTaxiData[i];
		if( !sSTATION.bUsed )	continue;
		insert( sSTATION );
	}
	
	return true;
}

HRESULT GLTaxiStation::LoadFile ( const char* szFile )
{
	StringCchCopy(m_szFileName,MAX_PATH,szFile);

	char szFullPath[MAX_PATH] = {0};
	StringCchCopy ( szFullPath, MAX_PATH, GLOGIC::GetPath() );
	StringCchCat ( szFullPath, MAX_PATH, m_szFileName );

	std::auto_ptr<basestream> pBStream( GLOGIC::openfile_basestream(GLOGIC::bGLOGIC_ZIPFILE, 
										GLOGIC::strGLOGIC_ZIPFILE.c_str(),
										szFullPath, 
										szFile,
										EMBYTECRYPT_NONE,
										GLOGIC::bGLOGIC_PACKFILE ) );

	if ( !pBStream.get() )
	{
		if ( strlen( szFile ) > 0 )
			CDebugSet::ErrorFile( "GLTaxiStation::LoadFile", szFile );

		return E_FAIL;
	}

	basestream &SFile = *pBStream;

	DWORD dwFILEVER;
	char szFILETYPE[_MAX_FNAME];
	SFile.GetFileType( szFILETYPE, _MAX_FNAME, dwFILEVER );

	if ( dwFILEVER>=VERSION_ENCODE_OLD )
		SFile.SetEncodeType ( EMBYTECRYPT_OLD );

	if ( dwFILEVER>=VERSION_ENCODE )
		SFile.SetEncodeType ( EMBYTECRYPT_TAXISTATION );

	m_vecTaxiData.clear();
	m_vecTaxiMap.clear();

	SFile >> m_dwBasicCharge;

	DWORD dwNum(0);
	SFile >> dwNum;

	for ( DWORD i=0; i<dwNum; i++ )
	{
		STAXI_STATION sSTATION;
		BOOL bLOAD = sSTATION.LOAD( SFile );
		if ( bLOAD )
			m_vecTaxiData.push_back( sSTATION );
	}

	for( int i=0; i<(int)m_vecTaxiData.size(); ++i )
	{
		STAXI_STATION sSTATION = m_vecTaxiData[i];
		if( !sSTATION.bUsed )	continue;
		insert( sSTATION );
	}

	return S_OK;
}

HRESULT GLTaxiStation::SaveFile ( const char* szFile )
{
	StringCchCopy(m_szFileName,MAX_PATH,szFile);

	char szFullPath[MAX_PATH];
	StringCchCopy ( szFullPath, MAX_PATH, GLOGIC::GetPath() );
	StringCchCat ( szFullPath, MAX_PATH, m_szFileName );

	CSerialFile SFile;
	SFile.SetFileType ( _FILEHEAD, VERSION );
	if ( !SFile.OpenFile ( FOT_WRITE, szFullPath ) )	return E_FAIL;
	SFile.SetEncodeType ( EMBYTECRYPT_TAXISTATION );

	std::sort( m_vecTaxiData.begin(), m_vecTaxiData.end(), TAXISTATION_CMP() );

	SFile << m_dwBasicCharge;

	SFile << (DWORD)m_vecTaxiData.size();
	for( int i=0; i<(int)m_vecTaxiData.size(); ++i )
	{
		m_vecTaxiData[i].SAVE( SFile );
	}

	return S_OK;
}

DWORD GLTaxiStation::GetTaxiMapNum ()
{
	return (DWORD)m_vecTaxiMap.size();
}

STAXI_MAP* GLTaxiStation::GetTaxiMap ( int nIndex )
{
	if ( m_vecTaxiMap.size() <= nIndex )	
		return NULL;

	return &m_vecTaxiMap[nIndex];
}

STAXI_STATION* GLTaxiStation::GetStation ( int nMapIndex, int nStationIndex )
{
	STAXI_MAP* pTaxiMap = GetTaxiMap( nMapIndex );
	
	if ( !pTaxiMap ) return NULL;

	if ( pTaxiMap->vecTaxiStation.size() <= nStationIndex )	
		return NULL;

	return &pTaxiMap->vecTaxiStation[nStationIndex];
}
