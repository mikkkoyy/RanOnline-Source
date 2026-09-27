#include "stdafx.h"
#include "DxSkinCharData.h"

#include "../Common/SerialFile.h"
#include "../G-Logic/GLogic.h"
#include "DxSkinPieceContainer.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//	Note : 스킨 파일을 읽음.
//		BOOL DxSkinDataDummy::LoadFile () 에서도 같은 구조로 로딩을 행하므로 이 로드/새이브 부분이 바뀔시
//		병행해서 같이 수정이 필요하다.
//
BOOL DxSkinCharData::LoadFile ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread, bool bREPRESH )
{
	char szPathName[MAX_PATH] = "";
	StringCchCopy( szPathName, MAX_PATH, DxSkinCharDataContainer::GetInstance().GetPath() );
	StringCchCat( szPathName, MAX_PATH, szFile );

	std::auto_ptr<basestream> pBStream( GLOGIC::openfile_basestream(GLOGIC::bENGLIB_ZIPFILE, 
										GLOGIC::strSKINOBJ_ZIPFILE.c_str(), 
										szPathName, 
										szFile ) );

	if ( !pBStream.get() )
	{
		if ( strlen( szFile ) > 0 )
			CDebugSet::ErrorFile( "DxSkinCharData::LoadFile", szFile );

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
	case VERSION_CHF:
		LOAD_0200( SFile, pd3dDevice );
		break;

	case 0x0108:
	case 0x0107:
		LOAD_0107_0108( SFile, pd3dDevice );
		break;

	case 0x0106:
		LOAD_0106( SFile, pd3dDevice );
		break;

	case 0x0104:
		LOAD_0104( SFile, pd3dDevice );
		break;

	case 0x0103:
		LOAD_0103( SFile, pd3dDevice );
		break;

	case 0x0102:
	case 0x0101:
		LOAD_0101_0102( SFile, pd3dDevice );
		break;

	case 0x0100:
		LOAD_0100( SFile, pd3dDevice );
		break;
	
	default:
		{
			CDebugSet::ErrorVersion( "DxSkinCharData::LoadFile", SFile.GetFileName(), dwVer );
			return FALSE;
		}break;
	};

	//	Note : 본파일 읽기.
	//
	m_pSkeleton = DxBoneCollector::GetInstance().Load ( m_strSkeleton.c_str(), pd3dDevice );
	if ( !m_pSkeleton )		return FALSE;

	for ( DWORD i=0; i<PIECE_SIZE; ++i )
	{
		if ( !m_strPIECE[i].empty() )
		{
			/*vehicle system, Juver, 2017/08/03 */
			if ( i == PIECE_VEHICLE )
			{
				DxVehicleData* pVehicle = DxVehicleDataContainer::GetInstance().LoadData( m_strPIECE[i].c_str(), pd3dDevice, bThread );
				if ( pVehicle )
				{
					SetVehicle( pVehicle, pd3dDevice, bThread );
					continue;
				}
			}

			/*ABL system, Juver, 2017/06/01 */
			DxAttBoneLink* pBoneLink = DxAttBoneLinkContainer::GetInstance().LoadData( m_strPIECE[i].c_str(), pd3dDevice, bThread );
			if ( pBoneLink )
			{
				SetAttBone( pBoneLink, pd3dDevice, bThread );
			}else{
				SetPiece ( m_strPIECE[i].c_str(), pd3dDevice, bThread );
			}
		}
	}

	for ( DWORD i=0; i<(DWORD)m_vecANIFILE.size(); ++i )
	{
		DxSkinAniControl::LoadAnimation ( m_vecANIFILE[i].c_str(), pd3dDevice, bREPRESH );
	}

	DxSkinAniControl::ClassifyAnimation ();

	//	Note : 초기 에니메이션 설정.
	//
	DxSkinAniControl::SELECTANI ( AN_GUARD_N );

	if ( !DxSkinAniControl::GETCURANIMNODE() )
	{
		CDebugSet::ToLogFile ( "select ani fail %s", szFile );
	}

	//	Note : 스킨 케릭터 이를 설정.
	StringCchCopy( m_szFileName, FILE_LENGTH, szFile );

	return TRUE;
}

BOOL DxSkinCharData::LOAD_0100( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
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

	char szSkeleton[MAX_PATH] = "";
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*MAX_PATH );
	m_strSkeleton = szSkeleton;

	char szAniName[ACF_SZNAME] = "";

	DWORD dwAniNum(0);
	SFile >> dwAniNum;

	m_vecANIFILE.clear();
	m_vecANIFILE.reserve ( dwAniNum );

	for ( DWORD i=0; i<dwAniNum; ++i )
	{
		SFile.ReadBuffer ( szAniName, sizeof(char)*ACF_SZNAME );
		m_vecANIFILE.push_back ( szAniName );
	}

	BOOL bBoundBox(FALSE);
	SFile >> bBoundBox;
	if ( bBoundBox )
	{
		SFile >> m_vMax;
		SFile >> m_vMin;
	}
	else
	{
		m_vMax = D3DXVECTOR3(7.f,20.f,7.f);
		m_vMin = D3DXVECTOR3(-7.f,0.f,-7.f);
	}

	return TRUE;
}

BOOL DxSkinCharData::LOAD_0101_0102( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
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

		m_vecANIFILE.push_back ( strANIFILE.c_str() );
	}

	BOOL bBoundBox(FALSE);
	SFile >> bBoundBox;
	if ( bBoundBox )
	{
		SFile >> m_vMax;
		SFile >> m_vMin;
	}
	else
	{
		m_vMax = D3DXVECTOR3(7.f,20.f,7.f);
		m_vMin = D3DXVECTOR3(-7.f,0.f,-7.f);
	}

	BOOL bExist = FALSE;
	SFile >> bExist;
	if( bExist )	SFile >> m_bWorldObj;
	else			return TRUE;

	bExist = FALSE;
	SFile >> bExist;
	if( bExist )	
	{
		SFile >> m_fHeight;
		SFile >> m_fRadius;
	}
	else			return TRUE;

	SFile >> bExist;

	return TRUE;
}

BOOL DxSkinCharData::LOAD_0103( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
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

		m_vecANIFILE.push_back ( strANIFILE.c_str() );
	}

	BOOL bBoundBox(FALSE);
	SFile >> bBoundBox;
	if ( bBoundBox )
	{
		SFile >> m_vMax;
		SFile >> m_vMin;
	}
	else
	{
		m_vMax = D3DXVECTOR3(7.f,20.f,7.f);
		m_vMin = D3DXVECTOR3(-7.f,0.f,-7.f);
	}

	BOOL bExist = FALSE;
	SFile >> bExist;
	if( bExist )	SFile >> m_bWorldObj;
	else			return TRUE;

	bExist = FALSE;
	SFile >> bExist;
	if( bExist )	
	{
		SFile >> m_fHeight;
		SFile >> m_fRadius;
	}
	else			return TRUE;

	//unknown data
	DWORD	dwBoneScale(0);
	SFile >> dwBoneScale;
	for ( DWORD i=0; i < dwBoneScale; ++i )
	{
		SBONESCALE_100	sBoneScaleOld;
		SFile.ReadBuffer( &sBoneScaleOld, sizeof( SBONESCALE_100 ) );

		SBONESCALE	sBoneScale;
		sBoneScale.Assign( sBoneScaleOld );
	}

	SFile >> bExist;

	return TRUE;
}

BOOL DxSkinCharData::LOAD_0104( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
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

		m_vecANIFILE.push_back ( strANIFILE.c_str() );
	}

	SFile >> m_fScale;

	BOOL bExist = FALSE;
	SFile >> bExist;
	if( bExist )	
	{
		SFile >> m_fHeight;
		SFile >> m_fRadius;
	}
	else			return TRUE;

	//unknown data
	DWORD	dwBoneScale(0);
	SFile >> dwBoneScale;
	for ( DWORD i=0; i < dwBoneScale; ++i )
	{
		SBONESCALE_100	sBoneScaleOld;
		SFile.ReadBuffer( &sBoneScaleOld, sizeof( SBONESCALE_100 ) );

		SBONESCALE	sBoneScale;
		sBoneScale.Assign( sBoneScaleOld );
	}

	BOOL bBoundBox(FALSE);
	SFile >> bBoundBox;
	if ( bBoundBox )
	{
		SFile >> m_vMax;
		SFile >> m_vMin;
	}
	else
	{
		m_vMax = D3DXVECTOR3(7.f,20.f,7.f);
		m_vMin = D3DXVECTOR3(-7.f,0.f,-7.f);
	}

	bExist = FALSE;
	SFile >> bExist;
	if( bExist )	SFile >> m_bWorldObj;
	else			return TRUE;

	SFile >> bExist;

	return TRUE;
}

BOOL DxSkinCharData::LOAD_0106( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
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
	
	//unknown data
	float fVAL(0.0f);
	SFile >> fVAL;

	BOOL bExist = FALSE;
	SFile >> bExist;
	if( bExist )	
	{
		SFile >> m_fHeight;
		SFile >> m_fRadius;
	}
	else			return TRUE;

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
	}
	else
	{
		m_vMax = D3DXVECTOR3(7.f,20.f,7.f);
		m_vMin = D3DXVECTOR3(-7.f,0.f,-7.f);
	}

	bExist = FALSE;
	SFile >> bExist;
	if( bExist )	SFile >> m_bWorldObj;
	else			return TRUE;

	SFile >> bExist;

	return TRUE;
}

BOOL DxSkinCharData::LOAD_0107_0108( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
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

	//unknown data
	float fVAL(0.0f);
	SFile >> fVAL;

	BOOL bExist = FALSE;
	SFile >> bExist;
	if( bExist )	
	{
		SFile >> m_fHeight;
		SFile >> m_fRadius;
	}
	else			return TRUE;

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
	}
	else
	{
		m_vMax = D3DXVECTOR3(7.f,20.f,7.f);
		m_vMin = D3DXVECTOR3(-7.f,0.f,-7.f);
	}

	bExist = FALSE;
	SFile >> bExist;
	if( bExist )	SFile >> m_bWorldObj;
	else			return TRUE;

	SFile >> bExist;

	//there are still data that needs reading 

	return TRUE;
}

BOOL DxSkinCharData::LOAD_0200( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
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

	return TRUE;
}

BOOL DxSkinCharData::SaveFile ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	char szPathName[MAX_PATH] = "";
	StringCchCopy( szPathName, MAX_PATH, DxSkinCharDataContainer::GetInstance().GetPath() );
	StringCchCat( szPathName, MAX_PATH, szFile );

	CSerialFile SFile;
	if ( !SFile.OpenFile ( FOT_WRITE, szPathName ) )		return FALSE;

	SFile << (DWORD)VERSION_CHF;

	SFile.SetEncodeType( EMBYTECRYPT_CONTAINER );

	//skeleton name
	SFile << m_strSkeleton;

	//piece
	SFile << (DWORD)PIECE_SIZE_TO_SAVE;
	for ( int i=0; i<PIECE_SIZE_TO_SAVE; ++i )
	{
		/*vehicle system, Juver, 2017/08/03 */
		if ( i == PIECE_VEHICLE )
		{
			if ( m_pVehicle )
			{
				SFile << (BOOL)TRUE;
				std::string strPieceFile = m_pVehicle->GetFileName();
				SFile << strPieceFile;
				continue;
			}
		}

		if ( m_pAttBoneData[i] )
		{
			SFile << (BOOL)TRUE;
			std::string strPieceFile = m_pAttBoneData[i]->GetFileName();
			SFile << strPieceFile;
		}else if ( m_pPieceArray[i] ){
			SFile << (BOOL)TRUE;
			std::string strPieceFile = m_pPieceArray[i]->m_szFileName;
			SFile << strPieceFile;
		}else{
			SFile << (BOOL)FALSE;
			continue;
		}
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
	
	StringCchCopy( m_szFileName, FILE_LENGTH, szFile );

	return TRUE;
}