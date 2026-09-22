#include "stdafx.h"
#include "../Common/SerialFile.h"
#include "../DxCommon/EDITMESHS.h"

#include "../G-Logic/GLogic.h"
#include "DxSkinPieceContainer.h"

#include "DxSkinCharData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DxSkinCharDataContainer& DxSkinCharDataContainer::GetInstance()
{
	static DxSkinCharDataContainer Instance;
	return Instance;
}

//-----------------------------------------------------------------------------------[DxSkinCharData]
//
DxSkinCharData::DxSkinCharData(void) :
	m_dwRef(0),

	m_bWorldObj(FALSE),
	m_fScale(1.0f),
	m_vMax(7.f,20.f,7.f),
	m_vMin(-7.f,0.f,-7.f),
	m_fHeight(20.f),
	m_fRadius(7.f)
{
	memset( m_szFileName, 0, FILE_LENGTH );

	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		m_pPieceArray[i] = NULL;
		m_pAttBoneData[i] = NULL; /*ABL system, Juver, 2017/06/01 */
	}

	/*vehicle system, Juver, 2017/08/03 */
	m_pVehicle = NULL;
}

DxSkinCharData::~DxSkinCharData(void)
{
	ClearAll ();
}

HRESULT DxSkinCharData::ClearAll ()
{
	m_dwRef = 0;
	m_fScale = 1.0f;

	m_vMax = D3DXVECTOR3(7,20,7);
	m_vMin = D3DXVECTOR3(-7,0,-7);

	m_vecANIFILE.clear();

	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		//	Note : 만약 이미 읽은 Piece Type 에 지정된 Piece 가 있다면 지워준다.
		//
		if ( m_pPieceArray[i] )
		{
			DxSkinPieceContainer::GetInstance().ReleasePiece ( m_pPieceArray[i]->m_szFileName );
		}
		m_pPieceArray[i] = NULL;

		 /*ABL system, Juver, 2017/06/01 */
		if ( m_pAttBoneData[i] )
		{
			DxAttBoneLinkContainer::GetInstance().ReleaseData( m_pAttBoneData[i]->GetFileName() );
		}

		m_pAttBoneData[i] = NULL;
	}

	/*vehicle system, Juver, 2017/08/03 */
	if ( m_pVehicle )
	{
		DxVehicleDataContainer::GetInstance().ReleaseData( m_pVehicle->GetFileName() );
		m_pVehicle = NULL;
	}

	DxSkinAniControl::ClearAll ();

	return S_OK;
}

HRESULT DxSkinCharData::SetPiece ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread, DWORD dwFlag )
{
	//	Note : 만약 처음 로드된 것을 무시하고 새로이 로드할때에는 원래 로드된걸 지워준다.
	//
	if ( dwFlag&FG_MUSTNEWLOAD )
	{
		for ( int i=0; i<PIECE_SIZE; ++i )
		{
			if ( m_pPieceArray[i] && m_pPieceArray[i]->m_szFileName )
			{
				if ( !strcmp(m_pPieceArray[i]->m_szFileName,szFile) )
				{
					m_pPieceArray[i] = NULL;
				}
			}

			m_pAttBoneData[i] = NULL; /*ABL system, Juver, 2017/06/01 */
		}

		DxSkinPieceContainer::GetInstance().DeletePiece ( szFile );
	}

	//	Note : Piece를 읽어온다.
	//
	DxSkinPiece* pSkinPiece = DxSkinPieceContainer::GetInstance().LoadPiece( szFile, pd3dDevice, bThread );
	if ( !pSkinPiece )	return E_FAIL;

	if ( dwFlag&FG_BONEFILE_CHECK )
	{
		if ( pSkinPiece->m_pmcMesh->pSkinInfo )
		{
			CString strFile1( m_strSkeleton.c_str() );
			strFile1.MakeLower();

			CString strFile2( pSkinPiece->m_szSkeleton );
			strFile2.MakeLower();

			if( _tcscmp(strFile1.GetString(),strFile2.GetString()) != 0 )
			{
				CDebugSet::ToLogFile( "Bone 불일치 %s, %s", strFile1.GetString(), strFile2.GetString() );
				return E_DIFFERENT_BONEFILE;
			}
		}
	}

	/*ABL system, Juver, 2017/06/01 */
	if ( m_pAttBoneData[pSkinPiece->m_emType] )
	{
		DxAttBoneLinkContainer::GetInstance().ReleaseData( m_pAttBoneData[pSkinPiece->m_emType]->GetFileName() );
		m_pAttBoneData[pSkinPiece->m_emType] = NULL;
	}
	
	/*vehicle system, Juver, 2017/08/03 */
	if ( pSkinPiece->m_emType == PIECE_VEHICLE && m_pVehicle )
	{
		DxVehicleDataContainer::GetInstance().ReleaseData( m_pVehicle->GetFileName() );
		m_pVehicle = NULL;
	}

	//	Note : 만약 이미 읽은 Piece Type 에 지정된 Piece 가 있다면 지워준다.
	//
	if ( m_pPieceArray[pSkinPiece->m_emType] )
	{
		DxSkinPieceContainer::GetInstance().ReleasePiece ( szFile );
	}

	//	Note : Piece 를 지정.
	//
	m_pPieceArray[pSkinPiece->m_emType] = pSkinPiece;

	return S_OK;
}

/*ABL system, Juver, 2017/06/01 */
HRESULT DxSkinCharData::SetAttBone( DxAttBoneLink* pBoneLink, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread, DWORD dwFlag /*= 0x0*/ )
{
	if ( !pBoneLink ) return FALSE;
	if ( !pBoneLink->GetData() ) return E_FAIL;

	int nPieceType = pBoneLink->m_emPieceType;
	if ( nPieceType > PIECE_SIZE ) return E_FAIL;

	if ( m_pAttBoneData[nPieceType] && strcmp( m_pAttBoneData[nPieceType]->GetFileName(), pBoneLink->GetFileName() ) != 0 )
	{
		DxAttBoneLinkContainer::GetInstance().ReleaseData( m_pAttBoneData[nPieceType]->GetFileName() );
		m_pAttBoneData[nPieceType] = NULL;
	}
	
	if ( m_pPieceArray[nPieceType] )
		DxSkinPieceContainer::GetInstance().ReleasePiece ( m_pPieceArray[nPieceType]->m_szFileName );

	/*vehicle system, Juver, 2017/08/03 */
	if ( nPieceType == PIECE_VEHICLE && m_pVehicle )
	{
		DxVehicleDataContainer::GetInstance().ReleaseData( m_pVehicle->GetFileName() );
		m_pVehicle = NULL;
	}

	m_pAttBoneData[nPieceType] = pBoneLink;

	if ( pBoneLink->m_strMaskPiece.size() != 0 )
	{
		DxSkinPiece* pSkinPiece = DxSkinPieceContainer::GetInstance().LoadPiece( pBoneLink->m_strMaskPiece.c_str(), pd3dDevice, TRUE );
		if ( pSkinPiece )
		{
			if ( m_pPieceArray[pSkinPiece->m_emType] )
				DxSkinPieceContainer::GetInstance().ReleasePiece( m_pPieceArray[pSkinPiece->m_emType]->m_szFileName );

			m_pPieceArray[pSkinPiece->m_emType] = pSkinPiece;
		}
	}

	return S_OK;
}

/*vehicle system, Juver, 2017/08/03 */
HRESULT DxSkinCharData::SetVehicle( DxVehicleData* pVehicle, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread, DWORD dwFlag /*= 0x0*/ )
{
	if ( !pVehicle ) return FALSE;

	if ( m_pAttBoneData[PIECE_VEHICLE] )
	{
		DxAttBoneLinkContainer::GetInstance().ReleaseData( m_pAttBoneData[PIECE_VEHICLE]->GetFileName() );
		m_pAttBoneData[PIECE_VEHICLE] = NULL;
	}

	if ( m_pPieceArray[PIECE_VEHICLE] )
		DxSkinPieceContainer::GetInstance().ReleasePiece ( m_pPieceArray[PIECE_VEHICLE]->m_szFileName );

	if ( m_pVehicle && strcmp( m_pVehicle->GetFileName(), pVehicle->GetFileName() ) != 0 )
	{
		DxVehicleDataContainer::GetInstance().ReleaseData( m_pVehicle->GetFileName() );
		m_pVehicle = NULL;
	}

	m_pVehicle = pVehicle;

	return S_OK;
}

void DxSkinCharData::ResetPiece ( LPDIRECT3DDEVICEQ pd3dDevice, int i )
{
	GASSERT(i<PIECE_SIZE);

	PDXSKINPIECE pSkinPiece = GetPiece (i);
	
	//	Note : 이전에 올려진 Piece 가 있다면 이를 내려준다.
	//
	if ( pSkinPiece )
	{
		DxSkinPieceContainer::GetInstance().ReleasePiece ( pSkinPiece->m_szFileName );
		m_pPieceArray[i] = NULL;
	}

	/*ABL system, Juver, 2017/06/01 */
	if ( m_pAttBoneData[i] )
	{
		DxAttBoneLinkContainer::GetInstance().ReleaseData( m_pAttBoneData[i]->GetFileName() );
		m_pAttBoneData[i] = NULL;
	}

	/*vehicle system, Juver, 2017/08/03 */
	if ( i == PIECE_VEHICLE )
	{
		if ( m_pVehicle )
		{
			DxVehicleDataContainer::GetInstance().ReleaseData( m_pVehicle->GetFileName() );
			m_pVehicle = NULL;
		}
	}
}

HRESULT DxSkinCharData::CalculateBoundingBox ( LPDIRECT3DDEVICEQ pd3dDevice, D3DXVECTOR3 &_vMax, D3DXVECTOR3 &_vMin )
{
	D3DXVECTOR3 vLastMax(-FLT_MAX,-FLT_MAX,-FLT_MAX), vLastMin(FLT_MAX,FLT_MAX,FLT_MAX);
	D3DXVECTOR3 vMax, vMin;

	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		if ( !m_pPieceArray[i] )
		{
			continue;//	조각이 일부분 없을수도 있으므로 오류는 무시함. ( 정상으로 판단. )
		}
		
		HRESULT hr = m_pPieceArray[i]->CalculateBoundingBox ( pd3dDevice, vMax, vMin );
		
		if ( FAILED(hr) )
		{
			continue;//	조각이 일부분 없을수도 있으므로 오류는 무시함. ( 정상으로 판단. )
		}

		if ( vLastMax.x < vMax.x )	vLastMax.x = vMax.x;
		if ( vLastMax.y < vMax.y )	vLastMax.y = vMax.y;
		if ( vLastMax.z < vMax.z )	vLastMax.z = vMax.z;

		if ( vLastMin.x > vMin.x )	vLastMin.x = vMin.x;
		if ( vLastMin.y > vMin.y )	vLastMin.y = vMin.y;
		if ( vLastMin.z > vMin.z )	vLastMin.z = vMin.z;
	}

	/*ABL system, Juver, 2017/06/01 */
	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		if ( !m_pAttBoneData[i] )	continue;
		if ( !m_pAttBoneData[i]->GetData() )	continue;

		vMax = m_pAttBoneData[i]->GetData()->m_vMax;
		vMin = m_pAttBoneData[i]->GetData()->m_vMin;

		if ( vLastMax.x < vMax.x )	vLastMax.x = vMax.x;
		if ( vLastMax.y < vMax.y )	vLastMax.y = vMax.y;
		if ( vLastMax.z < vMax.z )	vLastMax.z = vMax.z;

		if ( vLastMin.x > vMin.x )	vLastMin.x = vMin.x;
		if ( vLastMin.y > vMin.y )	vLastMin.y = vMin.y;
		if ( vLastMin.z > vMin.z )	vLastMin.z = vMin.z;
	}

	/*vehicle system, Juver, 2017/08/03 */
	if ( m_pVehicle )
	{
		vMax = m_pVehicle->m_vMax;
		vMin = m_pVehicle->m_vMin;

		if ( vLastMax.x < vMax.x )	vLastMax.x = vMax.x;
		if ( vLastMax.y < vMax.y )	vLastMax.y = vMax.y;
		if ( vLastMax.z < vMax.z )	vLastMax.z = vMax.z;

		if ( vLastMin.x > vMin.x )	vLastMin.x = vMin.x;
		if ( vLastMin.y > vMin.y )	vLastMin.y = vMin.y;
		if ( vLastMin.z > vMin.z )	vLastMin.z = vMin.z;
	}

	_vMax = vLastMax;
	_vMin = vLastMin;

	return S_OK;
}

void DxSkinCharData::SetRadiusHeight()
{
	m_vMax = D3DXVECTOR3( 0.f, 0.f, 0.f );
	m_vMin = D3DXVECTOR3( 0.f, 0.f, 0.f );

	// Note : 높이 수정
	m_vMax.y = m_fHeight;

	// Note : 반지름 수정
	m_vMax.x = m_fRadius;
	m_vMax.z = m_fRadius;
	m_vMin.x = -m_fRadius;
	m_vMin.z = -m_fRadius;
}

void DxSkinCharData::RenderAABBBox( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if( !strlen( m_szFileName ) )	return;

	EDITMESHS::RENDERAABB( pd3dDevice, m_vMax, m_vMin );

	/*ABL system, Juver, 2017/06/01 */
	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		if ( m_pAttBoneData[i] && m_pAttBoneData[i]->GetData() )
			m_pAttBoneData[i]->GetData()->RenderAABBBox( pd3dDevice );
	}

	/*vehicle system, Juver, 2017/08/03 */
	if ( m_pVehicle )
	{
		EDITMESHS::RENDERAABB( pd3dDevice, m_pVehicle->m_vMax, m_pVehicle->m_vMin );
	}
}

//--------------------------------------------------------------------------[DxSkinCharDataContainer]
//
DxSkinCharDataContainer::DxSkinCharDataContainer ()
{
	memset( m_szPath, 0, sizeof(char)*MAX_PATH );
}

DxSkinCharDataContainer::~DxSkinCharDataContainer ()
{
}

HRESULT DxSkinCharDataContainer::CleanUp ()
{
	SKINCHARDATAMAP_ITER iter = m_mapSkinCharData.begin ();
	SKINCHARDATAMAP_ITER iterEnd = m_mapSkinCharData.end ();
	
	for ( ; iter!=iterEnd; iter++ )
	{
		delete (*iter).second;
	}

	m_mapSkinCharData.clear ();

	return S_OK;
}

DxSkinCharData* DxSkinCharDataContainer::FindData ( const char* szFile )
{
	SKINCHARDATAMAP_ITER iter = m_mapSkinCharData.find ( std::string(szFile) );
	if ( iter != m_mapSkinCharData.end() )	return iter->second;

	return NULL;
}

BOOL DxSkinCharDataContainer::DeleteData ( const char* szFile )
{
	SKINCHARDATAMAP_ITER iter = m_mapSkinCharData.find ( std::string(szFile) );
	if ( iter == m_mapSkinCharData.end() )	return FALSE;

	delete (*iter).second;
	m_mapSkinCharData.erase ( iter );

	return TRUE;
}

void DxSkinCharDataContainer::ReleaseData ( const char* szFile )
{
	DxSkinCharData* pRes = FindData ( szFile );
	if ( !pRes )	return;
	
	if ( pRes->m_dwRef<=1 )
	{
		DeleteData ( szFile );
		return;
	}

	pRes->m_dwRef--;
	return;
}

DxSkinCharData* DxSkinCharDataContainer::LoadData( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	DxSkinCharData* pCharData;

	pCharData = FindData( szFile );
	if ( pCharData )
	{
		pCharData->m_dwRef++;
		return pCharData;
	}

	pCharData = new DxSkinCharData;
	BOOL bOk = pCharData->LoadFile( szFile, pd3dDevice, bThread );
	if ( !bOk )
	{
		SAFE_DELETE(pCharData);
		return NULL;
	}
	pCharData->m_dwRef++;

	CDebugSet::ToListView ( "DxSkinCharData::LoadFile, %s", szFile );

	//	Note : 트리에 달기.
	//
	m_mapSkinCharData[std::string(pCharData->GetFileName())] = pCharData;

	return pCharData;
}





