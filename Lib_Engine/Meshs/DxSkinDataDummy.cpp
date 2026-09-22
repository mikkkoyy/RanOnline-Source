#include "stdafx.h"
#include "DxSkinCharData.h"
#include "../G-Logic/GLogic.h"
#include "../Common/StringUtils.h"
#include "../Common/stlFunctions.h"
#include <algorithm>

#include "DxSkinDataDummy.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DxSkinDataDummy::DxSkinDataDummy(void)
{
	CleanUp ();
}

DxSkinDataDummy::~DxSkinDataDummy(void)
{
	CleanUp ();
}

void DxSkinDataDummy::CleanUp ()
{
	m_strFileName.clear();
	
	m_fScale = 1.0f;

	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		m_strPIECE[i].clear();
	}

	std::for_each ( m_vecANIMINFO.begin(), m_vecANIMINFO.end(), std_afunc::DeleteObject() );
	m_vecANIMINFO.clear();

	m_strSkeleton.clear();
}

BOOL DxSkinDataDummy::LOAD_0100 ( basestream &SFile )
{
	DWORD dwPieceSize(0);
	SFile >> dwPieceSize;
	for ( DWORD i=0; i<dwPieceSize; ++i )
	{
		BOOL bPieceUse = FALSE;
		std::string strPieceFile = "";

		SFile >> bPieceUse;
		if ( bPieceUse )
			SFile >> strPieceFile;

		if ( i >= PIECE_SIZE )	continue;
		m_strPIECE[i] = strPieceFile.c_str();
	}

	char szSkeleton[MAX_PATH] = "";
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*MAX_PATH );
	m_strSkeleton = szSkeleton;

	char szAniName[ACF_SZNAME] = "";
	DWORD dwAniNum(0);
	SFile >> dwAniNum;
	for ( DWORD i=0; i<dwAniNum; ++i )
	{
		SFile.ReadBuffer ( szAniName, sizeof(char)*ACF_SZNAME );

		CString strTEMP = szAniName;
		int nIndex = strTEMP.ReverseFind ( '.' );
		CString strCfgFile = strTEMP.Left(nIndex) + ".cfg";

		SANIMCONINFO *pANIMINFO = new SANIMCONINFO;
		if ( pANIMINFO->LoadFile ( strCfgFile.GetString() ) )
		{
			m_vecANIMINFO.push_back ( pANIMINFO );
		}else{
			SAFE_DELETE(pANIMINFO);
		}
	}

	return TRUE;
}

BOOL DxSkinDataDummy::LOAD_0101 ( basestream &SFile )
{
	SFile >> m_fScale;

	DWORD dwPieceSize(0);
	SFile >> dwPieceSize;
	for ( DWORD i=0; i<dwPieceSize; ++i )
	{
		BOOL bPieceUse = FALSE;
		std::string strPieceFile = "";

		SFile >> bPieceUse;
		if ( bPieceUse )
			SFile >> strPieceFile;

		if ( i >= PIECE_SIZE )	continue;
		m_strPIECE[i] = strPieceFile.c_str();
	}

	SFile >> m_strSkeleton;

	DWORD dwAniNum(0);
	SFile >> dwAniNum;
	for ( DWORD i=0; i<dwAniNum; ++i )
	{
		std::string strANIFILE;
		SFile >> strANIFILE;

		CString  strTEMP = strANIFILE.c_str();
		int nIndex = strTEMP.ReverseFind ( '.' );
		CString strCfgFile = strTEMP.Left(nIndex) + ".cfg";

		SANIMCONINFO *pANIMINFO = new SANIMCONINFO;
		if ( pANIMINFO->LoadFile ( strCfgFile.GetString() ) )
		{
			m_vecANIMINFO.push_back ( pANIMINFO );
		}else{
			SAFE_DELETE(pANIMINFO);
		}
	}


	return TRUE;
}

BOOL DxSkinDataDummy::LOAD_0104 ( basestream &SFile ) 
{
	DWORD dwPieceSize(0);
	SFile >> dwPieceSize;
	for ( DWORD i=0; i<dwPieceSize; ++i )
	{
		BOOL bPieceUse = FALSE;
		std::string strPieceFile = "";

		SFile >> bPieceUse;
		if ( bPieceUse )
			SFile >> strPieceFile;

		if ( i >= PIECE_SIZE )	continue;
		m_strPIECE[i] = strPieceFile.c_str();
	}

	SFile >> m_strSkeleton;

	DWORD dwAniNum(0);
	SFile >> dwAniNum;
	for ( DWORD i=0; i<dwAniNum; ++i )
	{
		std::string strANIFILE;
		SFile >> strANIFILE;

		CString  strTEMP = strANIFILE.c_str();
		int nIndex = strTEMP.ReverseFind ( '.' );
		CString strCfgFile = strTEMP.Left(nIndex) + ".cfg";

		SANIMCONINFO *pANIMINFO = new SANIMCONINFO;
		if ( pANIMINFO->LoadFile ( strCfgFile.GetString() ) )
		{
			m_vecANIMINFO.push_back ( pANIMINFO );
		}else{
			SAFE_DELETE(pANIMINFO);
		}
	}

	SFile >> m_fScale;

	return TRUE;
}

BOOL DxSkinDataDummy::LOAD_0200 ( basestream &SFile )
{
	//skeleton
	SFile >> m_strSkeleton;

	//piece
	DWORD dwPieceSize(0);
	SFile >> dwPieceSize;
	for ( DWORD i=0; i<dwPieceSize; ++i )
	{
		BOOL bPieceUse = FALSE;
		std::string strPieceFile = "";

		SFile >> bPieceUse;
		if ( bPieceUse )
			SFile >> strPieceFile;

		if ( i >= PIECE_SIZE )	continue;
		m_strPIECE[i] = strPieceFile.c_str();
	}

	//animation
	DWORD dwAniNum(0);
	SFile >> dwAniNum;
	for ( DWORD i=0; i<dwAniNum; ++i )
	{
		std::string strANIFILE;
		SFile >> strANIFILE;

		CString  strTEMP = strANIFILE.c_str();
		int nIndex = strTEMP.ReverseFind ( '.' );
		CString strCfgFile = strTEMP.Left(nIndex) + ".cfg";

		SANIMCONINFO *pANIMINFO = new SANIMCONINFO;
		if ( pANIMINFO->LoadFile ( strCfgFile.GetString() ) )
		{
			m_vecANIMINFO.push_back ( pANIMINFO );
		}else{
			SAFE_DELETE(pANIMINFO);
		}
	}

	SFile >> m_fScale;

	return TRUE;
}

//	Note : 스킨 파일을 읽음.
//		BOOL DxSkinDataDummy::LoadFile () 에서도 같은 구조로 로딩을 행하므로 이 로드/새이브 부분이 바뀔시
//		병행해서 같이 수정이 필요하다.
//
BOOL DxSkinDataDummy::LoadFile ( const char* szFile )
{
	char szPathName[MAX_PATH] = {0};
	StringCchCopy( szPathName, MAX_PATH, DxSkinCharDataContainer::GetInstance().GetPath() );
	StringCchCat( szPathName, MAX_PATH, szFile );

	std::auto_ptr<basestream> pBStream( GLOGIC::openfile_basestream(GLOGIC::bENGLIB_ZIPFILE, 
																	GLOGIC::strSKINOBJ_ZIPFILE.c_str(),
																	szPathName, 
																	szFile ) );

	if ( !pBStream.get() )
	{
		if ( strlen( szFile ) > 0 )
			CDebugSet::ErrorFile( "DxSkinDataDummy::LoadFile", szFile );

		return FALSE;
	}

	basestream &SFile = *pBStream;

	DWORD dwVer;
	SFile >> dwVer;

	if ( dwVer >= DxSkinCharData::VERSION_ENCRYPT )
		SFile.SetEncodeType( EMBYTECRYPT_CONTAINER );

	switch ( dwVer )
	{
	case 0x0200:
		LOAD_0200( SFile );
		break;

	case 0x0108:
	case 0x0107:
	case 0x0106:
	case 0x0104:
		LOAD_0104( SFile );
		break;

	case 0x0103:
	case 0x0102:
	case 0x0101:
		LOAD_0101( SFile );
		break;

	case 0x0100:
		LOAD_0100( SFile );
		break;

	default:
		{
			CDebugSet::ErrorVersion( "DxSkinDataDummy::LoadFile", SFile.GetFileName(), dwVer );
			return FALSE;
		}break;
	};

	//	Note : 스킨 케릭터 이를 설정.
	//
	m_strFileName = szFile;

	return TRUE;
}

SANIMCONINFO* DxSkinDataDummy::FindAniInfo ( const char* szName )
{
	size_t nSIZE = m_vecANIMINFO.size();
	for ( size_t n=0; n<nSIZE; ++n )
	{
		if ( !strcmp(m_vecANIMINFO[n]->m_szName,szName) )	return m_vecANIMINFO[n];
	}

	return NULL;
}

