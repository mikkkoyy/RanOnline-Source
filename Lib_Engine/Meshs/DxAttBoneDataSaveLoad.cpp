#include "stdafx.h"
#include "../Common/SerialFile.h"
#include "../DxCommon/EDITMESHS.h"

#include "../G-Logic/GLogic.h"
#include "DxSkinPieceContainer.h"

#include "DxAttBoneData.h"
#include "DxSkinChar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BOOL DxAttBoneData::SaveFile ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	char szPathName[MAX_PATH] = "";
	StringCchCopy( szPathName, MAX_PATH, DxAttBoneDataContainer::GetInstance().GetPath() );
	StringCchCat( szPathName, MAX_PATH, szFile );

	CSerialFile SFile;
	if ( !SFile.OpenFile ( FOT_WRITE, szPathName ) )		return FALSE;

	SFile << (DWORD)VERSION_ABF;

	SFile.SetEncodeType( EMBYTECRYPT_CONTAINER );

	//skeleton name
	SFile << m_strSkeleton;

	//piece
	SFile << (DWORD)PIECE_SIZE_TO_SAVE;
	for ( int i=0; i<PIECE_SIZE_TO_SAVE; ++i )
	{
		SFile << BOOL(m_pPieceArray[i]!=NULL);
		if ( !m_pPieceArray[i] )	continue;

		std::string strPieceFile = m_pPieceArray[i]->m_szFileName;
		SFile << strPieceFile;
	}

	//animation
	SFile << (DWORD)DxSkinAniControl::GETANIAMOUNT();
	for ( size_t n=0; n<m_vecAnim.size(); ++n )
	{
		std::string strNAME = m_vecAnim[n]->pAnimCont->m_szName;
		SFile << strNAME;
	}

	SFile << m_fScale;
	SFile << m_fHeight;
	SFile << m_fRadius;
	SFile << m_vMax;
	SFile << m_vMin;
	SFile << m_bWorldObj;

	SFile << (BOOL)FALSE;

	StringCchCopy( m_szFileName, FILE_LENGTH, szFile );

	return TRUE;
}

BOOL DxAttBoneData::LOAD_Ver_0200( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	//skeleton name
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

		if ( bPieceUse )
			m_strPIECE[i] = strPieceFile.c_str();
		else
			m_strPIECE[i].clear();
	}

	//animation
	DWORD dwAniNum(0);
	SFile >> dwAniNum;
	m_vecANIFILE.clear();
	m_vecANIFILE.reserve ( dwAniNum );

	for ( DWORD i=0; i<dwAniNum; ++i )
	{
		std::string strANIFILE;
		SFile >> strANIFILE;

		CString strANISTRING = strANIFILE.c_str();
		int nIndex = strANISTRING.ReverseFind ( '.' );
		CString strANI = strANISTRING.Left(nIndex) + ".x";
		m_vecANIFILE.push_back ( strANI.GetString() );
	}

	SFile >> m_fScale;
	SFile >> m_fHeight;
	SFile >> m_fRadius;
	SFile >> m_vMax;
	SFile >> m_vMin;
	SFile >> m_bWorldObj;

	BOOL bExist = FALSE;
	SFile >> bExist;

	return TRUE;
}

BOOL DxAttBoneData::LOAD_Ver_0100_0102( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
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

		if ( bPieceUse )
			m_strPIECE[i] = strPieceFile.c_str();
		else
			m_strPIECE[i].clear();
	}

	SFile >> m_strSkeleton;

	DWORD dwAniNum(0);
	SFile >> dwAniNum;
	m_vecANIFILE.clear();
	m_vecANIFILE.reserve ( dwAniNum );
	for ( DWORD i=0; i<dwAniNum; ++i )
	{
		std::string strANIFILE;
		SFile >> strANIFILE;

		CString strANISTRING = strANIFILE.c_str();
		int nIndex = strANISTRING.ReverseFind ( '.' );
		CString strANI = strANISTRING.Left(nIndex) + ".x";
		m_vecANIFILE.push_back ( strANI.GetString() );
	}

	SFile >> m_fScale;

	BOOL bExist = FALSE;
	SFile >> bExist;
	if( bExist )	
	{
		SFile >> m_fHeight;
		SFile >> m_fRadius;
	}
	else	return TRUE;
	
	//unknown data
	DWORD	dwBoneScale(0);
	SFile >> dwBoneScale;
	for ( DWORD i=0; i < dwBoneScale; ++i )
	{
		SBONESCALE	sBoneScale;
		SFile >> sBoneScale.vScale;
		SFile >> sBoneScale.strBONE;
	}

	BOOL bBoundBox(FALSE);
	SFile >> bBoundBox;
	if ( bBoundBox )
	{
		SFile >> m_vMax;
		SFile >> m_vMin;
	}else{
		m_vMax = D3DXVECTOR3(7.f,20.f,7.f);
		m_vMin = D3DXVECTOR3(-7.f,0.f,-7.f);
	}

	bExist = FALSE;
	SFile >> bExist;
	if( bExist )	
		SFile >> m_bWorldObj;
	else			
		return TRUE;

	SFile >> bExist;

	return TRUE;
}

BOOL DxAttBoneData::LoadFile ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread, bool bREPRESH /*=false*/ )
{
	std::string strCheckFileExt = szFile;
	std::transform ( strCheckFileExt.begin(), strCheckFileExt.end(), strCheckFileExt.begin(), std::tolower );
	std::string::size_type idx = strCheckFileExt.find( ".abf" );
	if ( idx == std::string::npos )	return FALSE;

	char szPathName[MAX_PATH] = "";
	StringCchCopy( szPathName, MAX_PATH, DxAttBoneDataContainer::GetInstance().GetPath() );
	StringCchCat( szPathName, MAX_PATH, szFile );

	std::auto_ptr<basestream> pBStream( GLOGIC::openfile_basestream(GLOGIC::bENGLIB_ZIPFILE, 
										GLOGIC::strSKINOBJ_ZIPFILE.c_str(), 
										szPathName, 
										szFile ) );

	if ( !pBStream.get() )
	{
		if ( strlen( szFile ) > 0 )
			CDebugSet::ErrorFile( "DxAttBoneData::LoadFile", szFile );

		return FALSE;
	}

	basestream &SFile = *pBStream;

	// Note : Init Data
	m_fHeight = 20.f;
	m_fRadius = 7.f;

	DWORD dwVer;
	SFile >> dwVer;

	if ( dwVer >= VERSION_ENCRYPT )
		SFile.SetEncodeType( EMBYTECRYPT_CONTAINER );

	switch ( dwVer )
	{
	case VERSION_ABF:
		LOAD_Ver_0200( SFile, pd3dDevice );
		break;

	case 0x0102:
	case 0x0101:
	case 0x0100:
		LOAD_Ver_0100_0102( SFile, pd3dDevice );
		break;

	default:
		{
			CDebugSet::ErrorVersion( "DxAttBoneData::LoadFile", SFile.GetFileName(), dwVer );
			return FALSE;
		}break;
	};

	m_pSkeleton = DxBoneCollector::GetInstance().Load ( m_strSkeleton.c_str(), pd3dDevice );
	if ( !m_pSkeleton )		return FALSE;

	for ( DWORD i=0; i<PIECE_SIZE; ++i )
	{
		if ( !m_strPIECE[i].empty() )
		{
			SetPiece ( m_strPIECE[i].c_str(), pd3dDevice, bThread );
		}
	}

	for ( DWORD i=0; i<(DWORD)m_vecANIFILE.size(); ++i )
	{
		DxSkinAniControl::LoadAnimation ( m_vecANIFILE[i].c_str(), pd3dDevice, bREPRESH );
	}

	DxSkinAniControl::ClassifyAnimation ();
	DxSkinAniControl::SELECTANI ( AN_ABL_STAY );

	if ( !DxSkinAniControl::GETCURANIMNODE() )
	{
		CDebugSet::ToLogFile ( "select ani fail %s", szFile );
	}

	StringCchCopy( m_szFileName, FILE_LENGTH, szFile );

	return TRUE;
}