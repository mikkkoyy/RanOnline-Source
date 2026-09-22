#include "stdafx.h"
#include "./GLItemMixMan.h"
#include "./GLogicData.h"
#include "../../Lib_Engine/Common/IniLoader.h"
#include "../../Lib_Engine/Common/StringUtils.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/G-Logic/GLogic.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int nKeysize = (sizeof( DWORD )+sizeof( BYTE ) ) * ITEMMIX_ITEMNUM * 2;

const char* GLItemMixMan::_FILEHEAD = "GLITEM_MIX";

GLItemMixMan& GLItemMixMan::GetInstance()
{
	static GLItemMixMan Instance;
	return Instance;
}

GLItemMixMan::GLItemMixMan () 
	: m_bServer ( false )
{
	memset(m_szFileName, 0, sizeof(char) * (MAX_PATH));
}

GLItemMixMan::~GLItemMixMan ()
{
	CleanUp();
}

HRESULT GLItemMixMan::Import ( const char* szFile )
{
	CleanUp();
	m_bServer = TRUE;

	std::string strLIST_FILE = szFile;

	CIniLoader cFILE;
	std::string strSep( ",[]\t" );
	cFILE.reg_sep( strSep );

	if( !cFILE.open( strLIST_FILE, true ) )
	{
		CDebugSet::ToLogFile ( "ERROR : GLItemMixMan::Import(), File Open %s", szFile );
		return false;
	}

	INT nItemMixKeySize(0);

	cFILE.getflag( "ITEM_MIX_INFO", "ItemMixKeySize", 0, 1, nItemMixKeySize );

	DWORD dwNUM = cFILE.GetKeySize( "ITEM_MIX_LIST", "ITEM_MIX" );
	for ( DWORD i=0; i<dwNUM; ++i )
	{
		ITEM_MIX	sItemMix;

		cFILE.getflag( i, "ITEM_MIX_LIST", "ITEM_MIX", 0, nItemMixKeySize, sItemMix.dwKey );


		for ( int j = 0; j < ITEMMIX_ITEMNUM; ++j )
		{
			cFILE.getflag( i, "ITEM_MIX_LIST", "ITEM_MIX", (j*3)+1, nItemMixKeySize, sItemMix.sMeterialItem[j].sNID.wMainID );
			cFILE.getflag( i, "ITEM_MIX_LIST", "ITEM_MIX", (j*3)+2, nItemMixKeySize, sItemMix.sMeterialItem[j].sNID.wSubID );		
			cFILE.getflag( i, "ITEM_MIX_LIST", "ITEM_MIX", (j*3)+3, nItemMixKeySize, sItemMix.sMeterialItem[j].nNum );
		}

		cFILE.getflag( i, "ITEM_MIX_LIST", "ITEM_MIX", 16, nItemMixKeySize, sItemMix.sResultItem.sNID.wMainID );
		cFILE.getflag( i, "ITEM_MIX_LIST", "ITEM_MIX", 17, nItemMixKeySize, sItemMix.sResultItem.sNID.wSubID );
		cFILE.getflag( i, "ITEM_MIX_LIST", "ITEM_MIX", 18, nItemMixKeySize, sItemMix.sResultItem.nNum );

		cFILE.getflag( i, "ITEM_MIX_LIST", "ITEM_MIX", 19, nItemMixKeySize, sItemMix.dwRate );
		cFILE.getflag( i, "ITEM_MIX_LIST", "ITEM_MIX", 20, nItemMixKeySize, sItemMix.dwPrice );

		//added by CNDev - import official itemmix ported to juver source
		cFILE.getflag( i, "ITEM_MIX_LIST", "ITEM_MIX", 21, nItemMixKeySize, sItemMix.wLevelReq );
		cFILE.getflag( i, "ITEM_MIX_LIST", "ITEM_MIX", 22, nItemMixKeySize, sItemMix.bGenerateRandomValue );
		cFILE.getflag( i, "ITEM_MIX_LIST", "ITEM_MIX", 23, nItemMixKeySize, sItemMix.fTime );
		cFILE.getflag( i, "ITEM_MIX_LIST", "ITEM_MIX", 24, nItemMixKeySize, sItemMix.wCategory );

		insert( sItemMix );
	}

	return S_OK;
}

HRESULT GLItemMixMan::LoadFile ( const char* szFile, BOOL bServer )
{
	CleanUp();

	//always true for editor
	m_bServer = bServer;

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
			CDebugSet::ErrorFile( "GLItemMixMan::LoadFile", szFile );

		return E_FAIL;
	}

	basestream &SFile = *pBStream;

	DWORD dwFILEVER;
	char szFILETYPE[_MAX_FNAME];
	SFile.GetFileType( szFILETYPE, _MAX_FNAME, dwFILEVER );

	if ( dwFILEVER>=VERSION_ENCODE_OLD )
		SFile.SetEncodeType ( EMBYTECRYPT_OLD );

	if ( dwFILEVER>=VERSION_ENCODE )
		SFile.SetEncodeType ( EMBYTECRYPT_ITEMIX );

	DWORD dwNum(0);
	SFile >> dwNum;

	for ( DWORD i=0; i<dwNum; i++ )
	{
		ITEM_MIX sItemMix;
		BOOL bLOAD = sItemMix.LOAD( SFile );
		if ( bLOAD )
			insert( sItemMix );
	}

	return S_OK;
}

HRESULT GLItemMixMan::SaveFile ( const char* szFile )
{
	StringCchCopy(m_szFileName,MAX_PATH,szFile);

	char szFullPath[MAX_PATH];
	StringCchCopy ( szFullPath, MAX_PATH, GLOGIC::GetPath() );
	StringCchCat ( szFullPath, MAX_PATH, m_szFileName );

	CSerialFile SFile;
	SFile.SetFileType ( _FILEHEAD, VERSION );
	if ( !SFile.OpenFile ( FOT_WRITE, szFullPath ) )	return E_FAIL;
	SFile.SetEncodeType ( EMBYTECRYPT_ITEMIX );

	if ( !m_bServer )
	{
		CDebugSet::MsgBox( "GLItemMixMan::SaveFile Use m_bServer for editor" );
		return E_FAIL;
	}

	SFile << (DWORD)m_mapKeyItemMix.size();
	ITEMMIX_KEYMAP_ITER iterpos = m_mapKeyItemMix.begin();
	for( ; iterpos != m_mapKeyItemMix.end(); ++iterpos )
	{
		(*iterpos).second.SAVE( SFile ); 
	}

	return S_OK;
}

void GLItemMixMan::CleanUp()
{
	m_mapStrItemMix.clear();
	m_mapKeyItemMix.clear();

	/*product item, Juver, 2017/10/15 */
	m_mapKeyProduct.clear();

	m_bServer = false;
}

void GLItemMixMan::insert ( ITEM_MIX& sItemMix )
{
	if ( m_bServer )
	{
		SortMeterialItem( sItemMix );
		m_mapKeyItemMix.insert( std::make_pair( sItemMix.dwKey, sItemMix ));
	}
	else
	{
		
		TCHAR szKey[nKeysize+1] = {0};

		SortMeterialItem( sItemMix );

		for ( int i = 0; i < ITEMMIX_ITEMNUM; ++i )
		{
			_stprintf( szKey, "%s%0.8x%0.2x", szKey, sItemMix.sMeterialItem[i].sNID.dwID, sItemMix.sMeterialItem[i].nNum );
		}

		std::string strKey = szKey;

		m_mapStrItemMix.insert( std::make_pair( strKey, sItemMix ));		
		
	}

	/*product item, Juver, 2017/10/15 */
	SortMeterialItem( sItemMix );
	m_mapKeyProduct.insert( std::make_pair( sItemMix.dwKey, sItemMix ));

	return;
}

const ITEM_MIX* GLItemMixMan::GetItemMix( const ITEM_MIX& sItemMix )
{
	if ( m_bServer )	return NULL;

	TCHAR szKey[nKeysize+1] = {0};


	for ( int i = 0; i < ITEMMIX_ITEMNUM; ++i )
	{
		_stprintf( szKey, "%s%0.8x%0.2x", szKey, sItemMix.sMeterialItem[i].sNID.dwID, sItemMix.sMeterialItem[i].nNum );
	}

	std::string strKey = szKey;

	ITEMMIX_STRMAP_ITER pos_beg = m_mapStrItemMix.find( strKey );
	ITEMMIX_STRMAP_ITER pos_end = m_mapStrItemMix.end();
	if ( pos_beg == pos_end )
	{
		return NULL;
	}

	return &(*pos_beg).second;
}

const ITEM_MIX* GLItemMixMan::GetItemMix( DWORD dwKey )
{
	if ( !m_bServer ) return NULL;
	
	ITEMMIX_KEYMAP_ITER pos = m_mapKeyItemMix.find(dwKey);
	if ( pos==m_mapKeyItemMix.end() )		return NULL;

	return &(*pos).second;
}

void GLItemMixMan::SortMeterialItem( ITEM_MIX& sItemMix )
{
	std::vector< ITEMMIX_DATA >	vecTemp;
	for( int i = 0; i < ITEMMIX_ITEMNUM; ++i ) 
	{
		vecTemp.push_back( sItemMix.sMeterialItem[i] );
	}

	std::sort( vecTemp.begin(), vecTemp.end(), ITEMMIX_DATA_CMP() );

	std::vector< ITEMMIX_DATA >::iterator pos = vecTemp.begin();
	for( int i = 0; i < ITEMMIX_ITEMNUM; ++i ) 
	{
		sItemMix.sMeterialItem[i] = (ITEMMIX_DATA)(*pos);
		pos++;
	}
}

void GLItemMixMan::SortInvenItem ( SINVENITEM* pInvenItem )
{
	std::vector< SINVENITEM >	vecTemp;
	for( int i = 0; i < ITEMMIX_ITEMNUM; ++i ) 
	{
		vecTemp.push_back( pInvenItem[i] );
	}

	std::sort( vecTemp.begin(), vecTemp.end(), ITEMMIX_INVEN_CMP() );

	std::vector< SINVENITEM >::iterator pos = vecTemp.begin();
	for( int i = 0; i < ITEMMIX_ITEMNUM; ++i ) 
	{
		pInvenItem[i] = (SINVENITEM)(*pos);
		pos++;
	}

	return;
}

/*product item, Juver, 2017/10/15 */
const ITEM_MIX* GLItemMixMan::GetProduct( DWORD dwKey )
{
	ITEMMIX_KEYMAP_ITER pos = m_mapKeyProduct.find(dwKey);
	if ( pos==m_mapKeyProduct.end() )		return NULL;

	return &(*pos).second;
}