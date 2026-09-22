#include "stdafx.h"
#include "../Common/SerialFile.h"
#include "../DxCommon/EDITMESHS.h"

#include "../G-Logic/GLogic.h"
#include "DxSkinPieceContainer.h"

#include "DxAttBoneData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DxAttBoneDataContainer& DxAttBoneDataContainer::GetInstance()
{
	static DxAttBoneDataContainer Instance;
	return Instance;
}

//-----------------------------------------------------------------------------------[DxAttBoneData]
//
DxAttBoneData::DxAttBoneData(void) :
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
	}
}

DxAttBoneData::~DxAttBoneData(void)
{
	ClearAll ();
}

HRESULT DxAttBoneData::ClearAll ()
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

	DxSkinAniControl::ClearAll ();

	return S_OK;
}

HRESULT DxAttBoneData::SetPiece ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread, DWORD dwFlag )
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
				CDebugSet::ToLogFile( "Bone ºÒÀÏÄ¡ %s, %s", strFile1.GetString(), strFile2.GetString() );
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

void DxAttBoneData::ResetPiece ( LPDIRECT3DDEVICEQ pd3dDevice, int i )
{
	GASSERT(i<PIECE_SIZE);

	PDXSKINPIECE pSkinPiece = GetPiece (i);
	
	if ( pSkinPiece )
	{
		DxSkinPieceContainer::GetInstance().ReleasePiece ( pSkinPiece->m_szFileName );
		m_pPieceArray[i] = NULL;
	}
}


HRESULT DxAttBoneData::CalculateBoundingBox ( LPDIRECT3DDEVICEQ pd3dDevice, D3DXVECTOR3 &_vMax, D3DXVECTOR3 &_vMin )
{
	D3DXVECTOR3 vLastMax(-FLT_MAX,-FLT_MAX,-FLT_MAX), vLastMin(FLT_MAX,FLT_MAX,FLT_MAX);
	D3DXVECTOR3 vMax, vMin;

	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		if ( !m_pPieceArray[i] )	continue;

		HRESULT hr = m_pPieceArray[i]->CalculateBoundingBox ( pd3dDevice, vMax, vMin );
		
		if ( FAILED(hr) )	continue;

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

void DxAttBoneData::SetRadiusHeight()
{
	m_vMax = D3DXVECTOR3( 0.f, 0.f, 0.f );
	m_vMin = D3DXVECTOR3( 0.f, 0.f, 0.f );

	m_vMax.y = m_fHeight;
	m_vMax.x = m_fRadius;
	m_vMax.z = m_fRadius;
	m_vMin.x = -m_fRadius;
	m_vMin.z = -m_fRadius;
}

void DxAttBoneData::RenderAABBBox( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if( !strlen( m_szFileName ) )	return;

	EDITMESHS::RENDERAABB( pd3dDevice, m_vMax, m_vMin );
}

//--------------------------------------------------------------------------[DxAttBoneDataContainer]
//
DxAttBoneDataContainer::DxAttBoneDataContainer ()
{
	memset( m_szPath, 0, sizeof(char)*MAX_PATH );
}

DxAttBoneDataContainer::~DxAttBoneDataContainer ()
{
}

HRESULT DxAttBoneDataContainer::CleanUp ()
{
	ATTBONEDATAMAP_ITER iter = m_mapAttBoneData.begin ();
	ATTBONEDATAMAP_ITER iterEnd = m_mapAttBoneData.end ();
	
	for ( ; iter!=iterEnd; iter++ )
	{
		delete (*iter).second;
	}

	m_mapAttBoneData.clear ();

	return S_OK;
}

DxAttBoneData* DxAttBoneDataContainer::FindData ( const char* szFile )
{
	ATTBONEDATAMAP_ITER iter = m_mapAttBoneData.find ( std::string(szFile) );
	if ( iter != m_mapAttBoneData.end() )	return iter->second;

	return NULL;
}

BOOL DxAttBoneDataContainer::DeleteData ( const char* szFile )
{
	ATTBONEDATAMAP_ITER iter = m_mapAttBoneData.find ( std::string(szFile) );
	if ( iter == m_mapAttBoneData.end() )	return FALSE;

	delete (*iter).second;
	m_mapAttBoneData.erase ( iter );

	return TRUE;
}

void DxAttBoneDataContainer::ReleaseData ( const char* szFile )
{
	DxAttBoneData* pRes = FindData ( szFile );
	if ( !pRes )	return;
	
	if ( pRes->m_dwRef<=1 )
	{
		DeleteData ( szFile );
		return;
	}

	pRes->m_dwRef--;
	return;
}

DxAttBoneData* DxAttBoneDataContainer::LoadData( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	if ( !szFile ) return NULL;
	if ( strlen(szFile) == 0 ) return NULL;

	DxAttBoneData* pCharData;
	pCharData = FindData( szFile );
	if ( pCharData )
	{
		pCharData->m_dwRef++;
		return pCharData;
	}

	pCharData = new DxAttBoneData;
	BOOL bOk = pCharData->LoadFile( szFile, pd3dDevice, bThread );
	if ( !bOk )
	{
		SAFE_DELETE(pCharData);
		return NULL;
	}
	pCharData->m_dwRef++;

	CDebugSet::ToListView ( "DxAttBoneData::LoadFile, %s", szFile );

	m_mapAttBoneData[std::string(pCharData->GetFileName())] = pCharData;

	return pCharData;
}





