#include "StdAfx.h"
#include "GLActivity.h"

#include "./GLogicData.h"

#include "../../Lib_Engine/Common/IniLoader.h"
#include "../../Lib_Engine/G-Logic/GLogic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const char* GLActivity::_FILEHEAD = "GLACTIVITY";

GLActivity& GLActivity::GetInstance ()
{
	static GLActivity _instance;
	return _instance;
}

GLActivity::GLActivity()
{
	memset(m_szFileName, 0, sizeof(char) * (MAX_PATH));
}

GLActivity::~GLActivity()
{
}

void GLActivity::CleanUp()
{
	m_mapActivity.clear();
}

HRESULT GLActivity::LoadFile ( const char* szFile, BOOL bServer )
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
			CDebugSet::ErrorFile( "GLActivity::LoadFile", szFile );

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
		SACTIVITY_FILE_DATA sdata;
		BOOL bLoad = sdata.LOAD( SFile, bServer );
		if ( !bLoad )	continue;

		SACTIVITY_FILE_DATA* pdata = GetActivity( sdata.dwActivityID );
		if ( pdata )	continue;

		m_mapActivity.insert( std::make_pair( sdata.dwActivityID, sdata ) );
	}

	return S_OK;
}

HRESULT GLActivity::SaveFile ( const char* szFile )
{
	StringCchCopy(m_szFileName,MAX_PATH,szFile);

	char szFullPath[MAX_PATH];
	StringCchCopy ( szFullPath, MAX_PATH, GetPath().c_str() );
	StringCchCat ( szFullPath, MAX_PATH, m_szFileName );

	CSerialFile SFile;
	SFile.SetFileType ( _FILEHEAD, VERSION );
	if ( !SFile.OpenFile ( FOT_WRITE, szFullPath ) )	return E_FAIL;
	SFile.SetEncodeType ( EMBYTECRYPT_ACTIVITY );

	SFile << (DWORD)m_mapActivity.size();

	SACTIVITY_FILE_DATA_MAP_ITER iterpos = m_mapActivity.begin();
	for( ; iterpos != m_mapActivity.end(); ++iterpos )
	{
		SACTIVITY_FILE_DATA &sdata = (*iterpos).second;
		sdata.SAVE( SFile );
	}

	return S_OK;
}

SACTIVITY_FILE_DATA* GLActivity::GetActivity( DWORD dwID )
{
	if ( dwID >= UINT_MAX )	return NULL;

	SACTIVITY_FILE_DATA_MAP_ITER iter = m_mapActivity.find( dwID );
	if ( iter == m_mapActivity.end() )	return NULL;

	return &(*iter).second;
}
