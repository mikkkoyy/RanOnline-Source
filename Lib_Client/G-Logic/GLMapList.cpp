#include "stdafx.h"
#include "./GLMapList.h"
#include "./GLGaeaServer.h"

#include "../../Lib_Engine/Common/IniLoader.h"
#include "../../Lib_Engine/Common/StringUtils.h"
#include "../../Lib_Engine/Common/BaseString.h"
#include "../../Lib_Engine/G-Logic/GLogic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const char* GLMapList::_FILEHEAD = "GLMAPS_LIST";

GLMapList::GLMapList(void)
{
}

GLMapList::~GLMapList(void)
{
}

BOOL GLMapList::IsRegMapNode ( SMAPNODE &MapCur )
{
	FIELDMAP_ITER iter = m_MapList.begin ();
	FIELDMAP_ITER iter_end = m_MapList.end ();

	for ( ; iter!=iter_end; ++iter )
	{
		const SMAPNODE &sMapNode = (*iter).second;

		if ( sMapNode.sNativeID==MapCur.sNativeID )		return TRUE;
		if ( MapCur.strFile==sMapNode.strFile )			return TRUE;
	}

	return FALSE;
}

//no longer used by server or client only for tool to import old ini
HRESULT GLMapList::LoadMapList_OLD ( const TCHAR* szMapListFile )
{
	GASSERT(szMapListFile);

	m_mapListData.clear();
	m_MapList.clear ();

	std::string strPath = szMapListFile;

	CIniLoader cFILE;

	std::string strSep( ",[]\t" );
	cFILE.reg_sep( strSep );

	if( !cFILE.open( strPath, true ) )
	{
		CDebugSet::ToLogFile ( "ERROR : GLMapList::LoadMapList_OLD(), File Open %s", szMapListFile );
		return E_FAIL;
	}

	INT nMapKeySize(0);
	cFILE.getflag( "MAP_LIST_INFO", "MapKeySize", 0, 1, nMapKeySize );

	DWORD dwNUM = cFILE.GetKeySize( "MAP_LIST", "MAP" );
	for ( DWORD i=0; i<dwNUM; ++i )
	{
		SMAPNODE_DATA sMapNode;
		cFILE.getflag( i, "MAP_LIST", "MAP", 0, nMapKeySize, sMapNode.bUsed );
		cFILE.getflag( i, "MAP_LIST", "MAP", 1, nMapKeySize, sMapNode.strFile );
		cFILE.getflag( i, "MAP_LIST", "MAP", 2, nMapKeySize, sMapNode.sNativeID.wMainID );
		cFILE.getflag( i, "MAP_LIST", "MAP", 3, nMapKeySize, sMapNode.sNativeID.wSubID );
		cFILE.getflag( i, "MAP_LIST", "MAP", 4, nMapKeySize, sMapNode.dwFieldSID );
		cFILE.getflag( i, "MAP_LIST", "MAP", 5, nMapKeySize, sMapNode.bPeaceZone );
		cFILE.getflag( i, "MAP_LIST", "MAP", 6, nMapKeySize, sMapNode.bCommission );
		cFILE.getflag( i, "MAP_LIST", "MAP", 7, nMapKeySize, sMapNode.bPKZone );
		cFILE.getflag( i, "MAP_LIST", "MAP", 8, nMapKeySize, sMapNode.bFreePK );
		cFILE.getflag( i, "MAP_LIST", "MAP", 9, nMapKeySize, sMapNode.bItemDrop );
		cFILE.getflag( i, "MAP_LIST", "MAP", 10, nMapKeySize, sMapNode.bMove );
		cFILE.getflag( i, "MAP_LIST", "MAP", 11, nMapKeySize, sMapNode.bRestart );
		cFILE.getflag( i, "MAP_LIST", "MAP", 12, nMapKeySize, sMapNode.bPetActivity );
		cFILE.getflag( i, "MAP_LIST", "MAP", 13, nMapKeySize, sMapNode.bDECEXP );
		cFILE.getflag( i, "MAP_LIST", "MAP", 14, nMapKeySize, sMapNode.bVehicleActivity );
		cFILE.getflag( i, "MAP_LIST", "MAP", 15, nMapKeySize, sMapNode.bClubBattleZone );				
		cFILE.getflag( i, "MAP_LIST", "MAP", 16, nMapKeySize, sMapNode.strMapName );
		cFILE.getflag( i, "MAP_LIST", "MAP", 17, nMapKeySize, sMapNode.strBGM );
		cFILE.getflag( i, "MAP_LIST", "MAP", 18, nMapKeySize, sMapNode.strLoadingImageName );
		cFILE.getflag( i, "MAP_LIST", "MAP", 19, nMapKeySize, sMapNode.bInstantMap );

		if ( !( sMapNode.sNativeID.wMainID<MAXLANDMID || sMapNode.sNativeID.wSubID<MAXLANDSID ) )
		{
			CDebugSet::MsgBox( "MapID Max Map(%d~%d) Map ID GuideLine is 0~%d / 0~%d.",
				sMapNode.sNativeID.wMainID, sMapNode.sNativeID.wSubID, MAXLANDMID, MAXLANDSID );
			continue;
		}

		if ( sMapNode.strMapName.length() > MAP_NAME_MAX )
		{
			CDebugSet::MsgBox( "MapName too long (%s) Map Name Limit is:%d", sMapNode.strMapName.c_str(), MAP_NAME_MAX );
			continue;
		}

		m_mapListData.insert ( std::make_pair(sMapNode.sNativeID.dwID,sMapNode) );
	}
	
	return S_OK;
}

HRESULT GLMapList::LoadMapsListFile ( const char* szFile, LPF_SERVER_INFO pServerInfo/*=NULL*/, int nServerNum/*=0*/, BOOL bTOOL /*= FALSE*/ )
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
			CDebugSet::ErrorFile( "GLMapList::LoadMapsListFile", szFile );

		return E_FAIL;
	}

	basestream &SFile = *pBStream;

	DWORD dwFILEVER;
	char szFILETYPE[_MAX_FNAME];
	SFile.GetFileType( szFILETYPE, _MAX_FNAME, dwFILEVER );

	if ( dwFILEVER>=VERSION_ENCODE_OLD )
		SFile.SetEncodeType ( EMBYTECRYPT_OLD );

	if ( dwFILEVER>=VERSION_ENCODE )
		SFile.SetEncodeType ( EMBYTECRYPT_MAPSLIST );

	m_mapListData.clear();
	m_MapList.clear ();

	DWORD dwNum(0);
	SFile >> dwNum;

	for ( DWORD i=0; i<dwNum; i++ )
	{
		SMAPNODE_DATA sMapNode;
		BOOL bOK = sMapNode.LOAD( SFile );
		if ( bOK )
			m_mapListData.insert ( std::make_pair( sMapNode.sNativeID.dwID, sMapNode ) );
	}

	if ( bTOOL )	return S_OK;

	FIELDMAP_DATA_ITER iterpos = m_mapListData.begin();
	for( ; iterpos != m_mapListData.end(); ++iterpos )
	{
		const SMAPNODE_DATA sDATA = (*iterpos).second;
		if ( !sDATA.bUsed )	continue;

		if ( !( sDATA.sNativeID.wMainID<MAXLANDMID || sDATA.sNativeID.wSubID<MAXLANDSID ) )
		{
			CDebugSet::MsgBox( "MapID Max Map(%d~%d) Map ID GuideLine is 0~%d / 0~%d.",
				sDATA.sNativeID.wMainID, sDATA.sNativeID.wSubID, MAXLANDMID, MAXLANDSID );
			continue;
		}

		if ( sDATA.strMapName.length() > MAP_NAME_MAX )
		{
			CDebugSet::MsgBox( "MapName too long (%s) Map Name Limit is:%d", sDATA.strMapName.c_str(), MAP_NAME_MAX );
			continue;
		}

		if ( pServerInfo && nServerNum )
		{
			if ( sDATA.dwFieldSID >= (DWORD)nServerNum )
			{
				CDebugSet::MsgBox( "FieldServer ID of Map (%s) is Overflow:%d / %d", sDATA.strMapName.c_str(), sDATA.dwFieldSID, nServerNum );
			}

			if ( !pServerInfo[0*MAX_CHANNEL_NUMBER+sDATA.dwFieldSID].IsValid() )
			{
				CDebugSet::MsgBox( "Map (%s) Not valid FieldServer ID:%d", sDATA.strMapName.c_str(), sDATA.dwFieldSID );
			}
		}

		SMAPNODE sMapNode;
		sMapNode = sDATA;
		if ( IsRegMapNode(sMapNode) )
		{
			CDebugSet::MsgBox( "Map Duplicate(%d~%d~%s)", sDATA.sNativeID.wMainID, sDATA.sNativeID.wSubID, sDATA.strMapName.c_str());
			continue;
		}

		if ( nServerNum )
		{
			GLLevelFile cLevelFile;
			BOOL bOK = cLevelFile.LoadFile ( sMapNode.strFile.c_str(), TRUE, NULL );
			if ( !bOK )
			{
				CONSOLEMSG_WRITE ( "cLevelFile.LoadFile(), %s fail", sMapNode.strFile.c_str() );
				continue;
			}

			sMapNode.sLEVEL_REQUIRE  = *cLevelFile.GetLevelRequire();
			sMapNode.sLEVEL_ETC_FUNC = *cLevelFile.GetLevelEtcFunc();
		}

		m_MapList.insert ( std::make_pair(sMapNode.sNativeID.dwID,sMapNode) );
	}


	return S_OK;
}

HRESULT GLMapList::SaveMapsListFile ( const char* szFile )
{
	StringCchCopy(m_szFileName,MAX_PATH,szFile);

	char szFullPath[MAX_PATH];
	StringCchCopy ( szFullPath, MAX_PATH, GLOGIC::GetPath() );
	StringCchCat ( szFullPath, MAX_PATH, m_szFileName );

	CSerialFile SFile;
	SFile.SetFileType ( _FILEHEAD, VERSION );
	if ( !SFile.OpenFile ( FOT_WRITE, szFullPath ) )	return E_FAIL;
	SFile.SetEncodeType ( EMBYTECRYPT_MAPSLIST );

	SFile << (DWORD)m_mapListData.size();

	FIELDMAP_DATA_ITER iter = m_mapListData.begin ();
	for ( ; iter!=m_mapListData.end(); ++iter )
	{
		(*iter).second.SAVE( SFile ); 
	}

	return S_OK;
}

SMAPNODE* GLMapList::FindMapNode ( const SNATIVEID &sNativeID )
{
	FIELDMAP_ITER iter = m_MapList.find ( sNativeID.dwID );
	if ( iter!=m_MapList.end() )	return &(*iter).second;

	return NULL;
}

const char* GLMapList::GetMapName ( const SNATIVEID &sNativeID )
{
	SMAPNODE* pMap = FindMapNode ( sNativeID );
	if ( !pMap )	return NULL;

	return pMap->strMapName.c_str();
}

const SLEVEL_REQUIRE* GLMapList::GetLevelRequire ( const SNATIVEID &sNativeID )
{
	SMAPNODE* pMap = FindMapNode ( sNativeID );
	if ( !pMap )	return NULL;

	return &pMap->sLEVEL_REQUIRE;
}

const SLEVEL_ETC_FUNC* GLMapList::GetLevelEtcFunc ( const SNATIVEID &sNativeID )
{
	SMAPNODE* pMap = FindMapNode ( sNativeID );
	if ( !pMap )	return NULL;

	return &pMap->sLEVEL_ETC_FUNC;
}

HRESULT GLMapList::CleanUp ()
{
	m_mapListData.clear();
	m_MapList.clear ();

	return S_OK;
}

BOOL GLMapList::InsertMapList( SMAPNODE &mapnew )
{
	FIELDMAP_ITER iter = m_MapList.begin ();
	FIELDMAP_ITER iter_end = m_MapList.end ();

	for ( ; iter!=iter_end; ++iter )
	{
		const SMAPNODE &sMapNode = (*iter).second;
		if ( sMapNode.sNativeID==mapnew.sNativeID )		return FALSE;
	}

	m_MapList.insert ( std::make_pair(mapnew.sNativeID.dwID,mapnew) );

	return TRUE;
}

BOOL GLMapList::EraseMapList( SNATIVEID sidNew)
{
	FIELDMAP_ITER iter = m_MapList.find (sidNew.dwID);
	if ( iter != m_MapList.end() )
	{
		m_MapList.erase( iter );
		return TRUE;
	}

	return FALSE;
}