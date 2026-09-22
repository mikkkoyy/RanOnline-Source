#include "stdafx.h"
#include "../Common/SerialFile.h"
#include "../DxCommon/EDITMESHS.h"

#include "../G-Logic/GLogic.h"
#include "DxSkinPieceContainer.h"

#include "DxVehicleData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DxVehicleDataContainer& DxVehicleDataContainer::GetInstance()
{
	static DxVehicleDataContainer Instance;
	return Instance;
}

DxVehicleData::DxVehicleData(void) :
	m_dwRef(0),
	m_bWorldObj(FALSE),
	m_fScale(1.0f),
	m_vMax(7.f,20.f,7.f),
	m_vMin(-7.f,0.f,-7.f),
	m_fHeight(20.f),
	m_fRadius(7.f)
{
	memset( m_szFileName, 0, FILE_LENGTH );

	m_strCharFront = "";
	m_strCharBack = "";

	m_affCharFront = DXAFFINEPARTS();
	m_affCharBack = DXAFFINEPARTS();

	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		m_pPieceArray[i] = NULL;
	}
}

DxVehicleData::~DxVehicleData(void)
{
	ClearAll ();
}

HRESULT DxVehicleData::ClearAll ()
{
	m_dwRef = 0;
	m_fScale = 1.0f;

	m_vMax = D3DXVECTOR3(7,20,7);
	m_vMin = D3DXVECTOR3(-7,0,-7);

	m_vecANIFILE.clear();

	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		if ( m_pPieceArray[i] )
		{
			DxSkinPieceContainer::GetInstance().ReleasePiece ( m_pPieceArray[i]->m_szFileName );
		}
		m_pPieceArray[i] = NULL;
	}

	m_strCharFront = "";
	m_strCharBack = "";

	m_affCharFront = DXAFFINEPARTS();
	m_affCharBack = DXAFFINEPARTS();

	DxSkinAniControl::ClearAll ();

	return S_OK;
}

HRESULT DxVehicleData::SetPiece ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread, DWORD dwFlag )
{
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
		}

		DxSkinPieceContainer::GetInstance().DeletePiece ( szFile );
	}

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
				CDebugSet::ToLogFile( "DxVehicleData::SetPiece bone mismatched %s, %s", strFile1.GetString(), strFile2.GetString() );
				return E_DIFFERENT_BONEFILE;
			}
		}
	}

	if ( m_pPieceArray[pSkinPiece->m_emType] )
	{
		DxSkinPieceContainer::GetInstance().ReleasePiece ( szFile );
	}

	m_pPieceArray[pSkinPiece->m_emType] = pSkinPiece;

	return S_OK;
}

void DxVehicleData::ResetPiece ( LPDIRECT3DDEVICEQ pd3dDevice, int i )
{
	GASSERT(i<PIECE_SIZE);

	PDXSKINPIECE pSkinPiece = GetPiece (i);
	
	if ( pSkinPiece )
	{
		DxSkinPieceContainer::GetInstance().ReleasePiece ( pSkinPiece->m_szFileName );
		m_pPieceArray[i] = NULL;
	}
}

HRESULT DxVehicleData::CalculateBoundingBox ( LPDIRECT3DDEVICEQ pd3dDevice, D3DXVECTOR3 &_vMax, D3DXVECTOR3 &_vMin )
{
	D3DXVECTOR3 vLastMax(-FLT_MAX,-FLT_MAX,-FLT_MAX), vLastMin(FLT_MAX,FLT_MAX,FLT_MAX);
	D3DXVECTOR3 vMax, vMin;

	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		if ( !m_pPieceArray[i] )
		{
			continue;
		}
		
		HRESULT hr = m_pPieceArray[i]->CalculateBoundingBox ( pd3dDevice, vMax, vMin );
		
		if ( FAILED(hr) )
		{
			continue;
		}

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

void DxVehicleData::SetRadiusHeight()
{
	m_vMax = D3DXVECTOR3( 0.f, 0.f, 0.f );
	m_vMin = D3DXVECTOR3( 0.f, 0.f, 0.f );

	m_vMax.y = m_fHeight;
	m_vMax.x = m_fRadius;
	m_vMax.z = m_fRadius;
	m_vMin.x = -m_fRadius;
	m_vMin.z = -m_fRadius;
}

void DxVehicleData::RenderAABBBox( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if( !strlen( m_szFileName ) )	return;

	EDITMESHS::RENDERAABB( pd3dDevice, m_vMax, m_vMin );
}

//--------------------------------------------------------------------------[DxVehicleDataContainer]
//
DxVehicleDataContainer::DxVehicleDataContainer ()
{
	memset( m_szPath, 0, sizeof(char)*MAX_PATH );
}

DxVehicleDataContainer::~DxVehicleDataContainer ()
{
}

HRESULT DxVehicleDataContainer::CleanUp ()
{
	VEHICLEDATAMAP_ITER iter = m_mapVehicleData.begin ();
	VEHICLEDATAMAP_ITER iterEnd = m_mapVehicleData.end ();
	
	for ( ; iter!=iterEnd; iter++ )
	{
		delete (*iter).second;
	}

	m_mapVehicleData.clear ();

	return S_OK;
}

DxVehicleData* DxVehicleDataContainer::FindData ( const char* szFile )
{
	VEHICLEDATAMAP_ITER iter = m_mapVehicleData.find ( std::string(szFile) );
	if ( iter != m_mapVehicleData.end() )	return iter->second;

	return NULL;
}

BOOL DxVehicleDataContainer::DeleteData ( const char* szFile )
{
	VEHICLEDATAMAP_ITER iter = m_mapVehicleData.find ( std::string(szFile) );
	if ( iter == m_mapVehicleData.end() )	return FALSE;

	delete (*iter).second;
	m_mapVehicleData.erase ( iter );

	return TRUE;
}

void DxVehicleDataContainer::ReleaseData ( const char* szFile )
{
	DxVehicleData* pRes = FindData ( szFile );
	if ( !pRes )	return;
	
	if ( pRes->m_dwRef<=1 )
	{
		DeleteData ( szFile );
		return;
	}

	pRes->m_dwRef--;
	return;
}

DxVehicleData* DxVehicleDataContainer::LoadData( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	DxVehicleData* pCharData;

	pCharData = FindData( szFile );
	if ( pCharData )
	{
		pCharData->m_dwRef++;
		return pCharData;
	}

	pCharData = new DxVehicleData;
	BOOL bOk = pCharData->LoadFile( szFile, pd3dDevice, bThread );
	if ( !bOk )
	{
		SAFE_DELETE(pCharData);
		return NULL;
	}
	pCharData->m_dwRef++;

	CDebugSet::ToListView ( "DxVehicleData::LoadFile, %s", szFile );

	m_mapVehicleData[std::string(pCharData->GetFileName())] = pCharData;

	return pCharData;
}





