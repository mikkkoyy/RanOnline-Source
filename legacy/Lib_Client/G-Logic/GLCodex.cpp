#include "StdAfx.h"
#include "GLCodex.h"

#include "./GLogicData.h"

#include "../../Lib_Engine/Common/IniLoader.h"
#include "../../Lib_Engine/G-Logic/GLogic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const char* GLCodex::_FILEHEAD = "GLCODEX";

GLCodex& GLCodex::GetInstance ()
{
	static GLCodex _instance;
	return _instance;
}

GLCodex::GLCodex()
{
	memset(m_szFileName, 0, sizeof(char) * (MAX_PATH));
}

GLCodex::~GLCodex()
{
}

void GLCodex::CleanUp()
{
	m_mapCodex.clear();
}
HRESULT GLCodex::Import ( const char* szFile )
{
	CleanUp();
	//m_bServer = TRUE;

	std::string strLIST_FILE = szFile;

	CIniLoader cFILE;
	std::string strSep( "	,[]\t" );
	cFILE.reg_sep( strSep );

	if( !cFILE.open( strLIST_FILE, true ) )
	{
		CDebugSet::ToLogFile ( "ERROR : GLCodex::Import(), File Open %s", szFile );
		return false;
	}

	INT nItemCodexKeySize(0);

	cFILE.getflag( "ITEM_CODEX_INFO", "ItemCodexKeySize", 0, 1, nItemCodexKeySize );

	DWORD dwNUM = cFILE.GetKeySize( "ITEM_CODEX_LIST", "ITEM_CODEX" );
	for ( DWORD i=0; i<dwNUM; ++i )
	{
		SCODEX_FILE_DATA sdata;
		
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 0, nItemCodexKeySize, sdata.dwCodexID );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 1, nItemCodexKeySize, sdata.strCodexTitle );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 2, nItemCodexKeySize, sdata.emType );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 3, nItemCodexKeySize, sdata.emNotify );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 4, nItemCodexKeySize, sdata.dwRewardPoint );

		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 5, nItemCodexKeySize, sdata.sidMobKill.wMainID );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 6, nItemCodexKeySize, sdata.sidMobKill.wSubID );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 7, nItemCodexKeySize, sdata.wProgressMobKill );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 8, nItemCodexKeySize, sdata.wItemGrade1 );
		
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 9, nItemCodexKeySize, sdata.sidMapKill.wMainID );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 10, nItemCodexKeySize, sdata.sidMapKill.wSubID );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 11, nItemCodexKeySize, sdata.wProgressMapKill );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 12, nItemCodexKeySize, sdata.wItemGrade2 );
		
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 13, nItemCodexKeySize, sdata.sidMapReach.wMainID );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 14, nItemCodexKeySize, sdata.sidMapReach.wSubID );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 15, nItemCodexKeySize, sdata.wProgressMapReach );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 16, nItemCodexKeySize, sdata.wItemGrade3 );
		
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 17, nItemCodexKeySize, sdata.sidItemGet.wMainID );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 18, nItemCodexKeySize, sdata.sidItemGet.wSubID );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 19, nItemCodexKeySize, sdata.wProgressItemGet );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 20, nItemCodexKeySize, sdata.wItemGrade4 );
		
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 21, nItemCodexKeySize, sdata.sidItemUse.wMainID );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 22, nItemCodexKeySize, sdata.sidItemUse.wSubID );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 23, nItemCodexKeySize, sdata.wProgressItemUse );
		cFILE.getflag( i, "ITEM_CODEX_LIST", "ITEM_CODEX", 24, nItemCodexKeySize, sdata.wItemGrade5 );
		//insert( sdata );
		
		m_mapCodex.insert( std::make_pair( sdata.dwCodexID, sdata ) );
	}

	return S_OK;
}
HRESULT GLCodex::LoadFile ( const char* szFile, BOOL bServer )
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
			CDebugSet::ErrorFile( "GLCodex::LoadFile", szFile );

		return E_FAIL;
	}

	basestream &SFile = *pBStream;

	DWORD dwFILEVER;
	char szFILETYPE[_MAX_FNAME];
	SFile.GetFileType( szFILETYPE, _MAX_FNAME, dwFILEVER );

	if ( dwFILEVER>=VERSION_ENCODE )
		SFile.SetEncodeType ( EMBYTECRYPT_ACTIVITY );

	CleanUp();

	DWORD dwnum(0);
	SFile >> dwnum;

	for ( DWORD i=0; i<dwnum; i++ )
	{
		SCODEX_FILE_DATA sdata;
		BOOL bLoad = sdata.LOAD( SFile, bServer );
		if ( !bLoad )	continue;

		SCODEX_FILE_DATA* pdata = GetCodex( sdata.dwCodexID );
		if ( pdata )	continue;

		m_mapCodex.insert( std::make_pair( sdata.dwCodexID, sdata ) );
	}

	return S_OK;
}

HRESULT GLCodex::SaveFile ( const char* szFile )
{
	StringCchCopy(m_szFileName,MAX_PATH,szFile);

	char szFullPath[MAX_PATH];
	StringCchCopy ( szFullPath, MAX_PATH, GetPath().c_str() );
	StringCchCat ( szFullPath, MAX_PATH, m_szFileName );

	CSerialFile SFile;
	SFile.SetFileType ( _FILEHEAD, VERSION );
	if ( !SFile.OpenFile ( FOT_WRITE, szFullPath ) )	return E_FAIL;
	SFile.SetEncodeType ( EMBYTECRYPT_ACTIVITY );

	SFile << (DWORD)m_mapCodex.size();

	SCODEX_FILE_DATA_MAP_ITER iterpos = m_mapCodex.begin();
	for( ; iterpos != m_mapCodex.end(); ++iterpos )
	{
		SCODEX_FILE_DATA &sdata = (*iterpos).second;
		sdata.SAVE( SFile );
	}

	return S_OK;
}

SCODEX_FILE_DATA* GLCodex::GetCodex( DWORD dwID )
{
	if ( dwID >= UINT_MAX )	return NULL;

	SCODEX_FILE_DATA_MAP_ITER iter = m_mapCodex.find( dwID );
	if ( iter == m_mapCodex.end() )	return NULL;

	return &(*iter).second;
}
bool GLCodex::SaveCsvs( std::fstream &SFile )
{
	SCODEX_FILE_DATA::SaveCsvHead( SFile );
	SCODEX_FILE_DATA_MAP_ITER iterpos = m_mapCodex.begin();
	for( ; iterpos != m_mapCodex.end(); ++iterpos )
	{
		SCODEX_FILE_DATA &sdata = (*iterpos).second;
		sdata.SaveCsv( SFile );
	}

	return true;
}
bool GLCodex::LoadCsv( std::string& strFileName )
{
	CStringFile StrFile( 10240 );
	if( !StrFile.Open( strFileName.c_str() ) )
		return false;

	m_mapCodex.clear();

	STRUTIL::ClearSeparator();
	STRUTIL::RegisterSeparator( "," );

	CString strLine;
	CStringArray StrArray;

	StrFile.GetNextLine( strLine );
	STRUTIL::StringSeparate( strLine, StrArray );
	int iHead = (int)StrArray.GetCount();
	
	int iLine = 1;
	
	while( StrFile.GetNextLine( strLine ) )
	{
		iLine++;
		STRUTIL::StringSeparate( strLine, StrArray );

		int iCount = (int)StrArray.GetCount();
		if( iCount < iHead )
		{
			char szError[ 256 ];
			StringCchPrintf( szError, 256, "%d Line = include blank column, next item load failed Count%d / Head%d", iLine, iCount, iHead );
			MessageBox( NULL, szError, _T( "Fail" ), MB_OK );
			return false;
		}

		SCODEX_FILE_DATA sdata;
		sdata.LoadCsv( StrArray );
		
		m_mapCodex.insert( std::make_pair( sdata.dwCodexID, sdata ) );
	}

	return true;
}