#include "stdafx.h"

#include <list>
#include <vector>
#include <algorithm>
#include "StlFunctions.h"
#include "GLDefine.h"
#include "SerialFile.h"

#include "DxSkinMesh9_NORMAL.h"
#include "DxSkinVB_ATT.h"

#include "DxSkinMeshContainer9.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct NEWVERTEX
{
	D3DXVECTOR3 m_vDir;		// 방향
	float		m_fAngel;
	DWORD		m_dwIndex;	// 인덱스는 중복됨.
};
typedef std::list<NEWVERTEX>		LIST_NEWVERTEX;
typedef LIST_NEWVERTEX::iterator	LIST_NEWVERTEX_ITER;

typedef std::vector<DWORD>			VEC_INDEX;
typedef VEC_INDEX::iterator			VEC_INDEX_ITER;	//----------------------------------

struct NEWVERTEXMAN
{
	D3DXVECTOR3		m_vPos;			// 위치가 같다면 계속 넣음.
	LIST_NEWVERTEX	m_listVertex;	// 버텍스 리스트

	VEC_INDEX		m_vecIndex;	// 인덱스 리스트
	D3DXVECTOR3		m_vDIr;
	float			m_fThickness;

	void CalDirection();
	void InsertIndex( WORD wIndex );
};
typedef std::list<NEWVERTEXMAN*>		LIST_NEWVERTEXMAN;
typedef LIST_NEWVERTEXMAN::iterator		LIST_NEWVERTEXMAN_ITER;


void NEWVERTEXMAN::CalDirection()
{
	m_vDIr = D3DXVECTOR3(0.f,0.f,0.f);
	LIST_NEWVERTEX_ITER iter = m_listVertex.begin();
	for( ; iter!=m_listVertex.end(); ++iter )
	{
		m_vDIr += (*iter).m_vDir * (*iter).m_fAngel;
	}

	D3DXVec3Normalize( &m_vDIr, &m_vDIr );
}

void NEWVERTEXMAN::InsertIndex( WORD wIndex )
{
	VEC_INDEX_ITER iter = m_vecIndex.begin();
	for( ; iter!=m_vecIndex.end(); ++iter )
	{
		// Note : 같은게 있다면 넣지 않는다.
		if( wIndex == (*iter) )	return;
	}

	m_vecIndex.push_back( wIndex );
}

void InsertNewVertex( LIST_NEWVERTEXMAN& sVertexMan, const D3DXVECTOR3& vPos, WORD wIndex, NEWVERTEX& vVertex )
{
	float fMicroPos(0.01f);
	LIST_NEWVERTEXMAN_ITER iter = sVertexMan.begin();
	for( ; iter!=sVertexMan.end(); ++iter )
	{
		if( ( (*iter)->m_vPos.x < vPos.x+fMicroPos ) && ( (*iter)->m_vPos.x > vPos.x-fMicroPos ) && 
			( (*iter)->m_vPos.y < vPos.y+fMicroPos ) && ( (*iter)->m_vPos.y > vPos.y-fMicroPos ) && 
			( (*iter)->m_vPos.z < vPos.z+fMicroPos ) && ( (*iter)->m_vPos.z > vPos.z-fMicroPos ) )
		{
			// Note : 추가로 값을 집어 넣는다.
			(*iter)->m_listVertex.push_back( vVertex );
			(*iter)->InsertIndex( wIndex );

			return;
		}
	}

	// Note : 없다면 새로운 것이다.
	NEWVERTEXMAN* pNew = new NEWVERTEXMAN;
	pNew->m_vPos = vPos;
	pNew->m_listVertex.push_back( vVertex );
	pNew->InsertIndex( wIndex );
	sVertexMan.push_back( pNew );
}

void CalculateVertex( NEWVERTEX& sNewVertex, WORD wIndex, const D3DXVECTOR3& vPos00, const D3DXVECTOR3& vPos01, const D3DXVECTOR3& vPos02, const D3DXVECTOR3& vNor )
{
	D3DXVECTOR3 vLine[2];
	vLine[0] = vPos00 - vPos01;
	vLine[1] = vPos00 - vPos02;
	D3DXVec3Normalize( &vLine[0], &vLine[0] );
	D3DXVec3Normalize( &vLine[1], &vLine[1] );

	D3DXVECTOR3 vDir = vLine[0] + vLine[1];
	D3DXVec3Normalize( &vDir, &vDir );

	D3DXVECTOR3 vNorN;
	D3DXVec3Normalize( &vNorN, &vNor );
	vDir = vDir + vNorN;
	
	float fDot = D3DXVec3Dot( &vLine[0], &vLine[1] );
	fDot = acosf( fDot );

	sNewVertex.m_dwIndex = wIndex;
	sNewVertex.m_vDir = vDir;
	sNewVertex.m_fAngel = fDot;
}

void CreateMeshNORMAL( IDirect3DDevice9 *pd3dDevice, const LPD3DXSKININFO pSkinInfo, const DWORD *pAdjacency, const LPD3DXMESH pMesh, LPD3DXMESH& pMeshOUT )
{
	HRESULT hr(S_OK);

	DWORD			iAttributeSW(0);
	DWORD			NumInfl(0);
	DWORD			NumAttributeGroups(0);
	LPD3DXBUFFER	pBoneCombinationBuf(NULL);

	hr = pSkinInfo->ConvertToBlendedMesh
                                (
                                    pMesh,
                                    D3DXMESH_MANAGED|D3DXMESHOPT_VERTEXCACHE, 
                                    pAdjacency, 
                                    NULL, NULL, NULL, 
                                    &NumInfl,
                                    &NumAttributeGroups, 
                                    &pBoneCombinationBuf, 
                                    &pMeshOUT
                                );
    if (FAILED(hr))
        goto e_Exit;


    // If the device can only do 2 matrix blends, ConvertToBlendedMesh cannot approximate all meshes to it
    // Thus we split the mesh in two parts: The part that uses at most 2 matrices and the rest. The first is
    // drawn using the device's HW vertex processing and the rest is drawn using SW vertex processing.
    LPD3DXBONECOMBINATION rgBoneCombinations  = reinterpret_cast<LPD3DXBONECOMBINATION>(pBoneCombinationBuf->GetBufferPointer());

    // look for any set of bone combinations that do not fit the caps
    for (iAttributeSW = 0; iAttributeSW < NumAttributeGroups; iAttributeSW++)
    {
        DWORD cInfl   = 0;

        for (DWORD iInfl = 0; iInfl < NumInfl; iInfl++)
        {
            if (rgBoneCombinations[iAttributeSW].BoneId[iInfl] != UINT_MAX)
            {
                ++cInfl;
            }
        }

		if (cInfl > DxSkinMesh9_NORMAL::m_dwMaxVertexBlendMatrices )
        {
            break;
        }
    }

    // if there is both HW and SW, add the Software Processing flag
    if (iAttributeSW < NumAttributeGroups)
    {
        LPD3DXMESH pMeshTmp;
        hr = pMeshOUT->CloneMeshFVF(D3DXMESH_SOFTWAREPROCESSING | pMeshOUT->GetOptions(), 
                                            pMeshOUT->GetFVF(),
                                            pd3dDevice, &pMeshTmp);
        if (FAILED(hr))
        {
            goto e_Exit;
        }

        pMeshOUT->Release();
        pMeshOUT = pMeshTmp;
        pMeshTmp = NULL;
    }

e_Exit:
	SAFE_RELEASE( pBoneCombinationBuf );
}

BOOL IsPointInOder( LPDIRECT3DDEVICEQ pd3dDevice, const LPD3DXMESH pMesh, float fThickness, float fSizeADD, LIST_NEWVERTEXMAN& sNewVertexMan )
{
	if( !pMesh )								return FALSE;
	if( pMesh->GetOptions()&D3DXMESH_32BIT )	return FALSE;

	HRESULT hr(S_OK);
	DWORD dwNumFaces = pMesh->GetNumFaces ();
	DWORD dwNumVertices = pMesh->GetNumVertices ();
	
	
	DWORD dwSkinFVF = pMesh->GetFVF ();
	DWORD dwPosFVF = dwSkinFVF&D3DFVF_POSITION_MASK;
	DWORD dwSkinVertSize = D3DXGetFVFVertexSize( dwSkinFVF );
	DWORD dwPosSize = D3DXGetFVFVertexSize( dwPosFVF );

	// Note : 각점들의 정보를 모으기
	//
	BYTE *pbData = NULL;
	hr = pMesh->LockVertexBuffer ( D3DLOCK_READONLY, (VOID**)&pbData );
	if ( FAILED(hr) )	return FALSE;

	WORD *pIndex = NULL;
	hr = pMesh->LockIndexBuffer ( D3DLOCK_READONLY, (VOID**)&pIndex );
	if ( FAILED(hr) )	return FALSE;

	WORD wIndex[3];
	D3DXVECTOR3* vPos[3];
	D3DXVECTOR3* vNor[3];
	NEWVERTEX	sNewVertex;
	for ( DWORD i=0; i<dwNumFaces; ++i )
	{
		wIndex[0] = pIndex[i*3+0];
		wIndex[1] = pIndex[i*3+1];
		wIndex[2] = pIndex[i*3+2];
		vPos[0] = (D3DXVECTOR3*)( pbData + dwSkinVertSize * wIndex[0] );
		vPos[1] = (D3DXVECTOR3*)( pbData + dwSkinVertSize * wIndex[1] );
		vPos[2] = (D3DXVECTOR3*)( pbData + dwSkinVertSize * wIndex[2] );
		vNor[0] = (D3DXVECTOR3*)( pbData + dwSkinVertSize * wIndex[0] + dwPosSize );
		vNor[1] = (D3DXVECTOR3*)( pbData + dwSkinVertSize * wIndex[1] + dwPosSize );
		vNor[2] = (D3DXVECTOR3*)( pbData + dwSkinVertSize * wIndex[2] + dwPosSize );

		CalculateVertex( sNewVertex, wIndex[0], *vPos[0], *vPos[1], *vPos[2], *vNor[0] );
		InsertNewVertex( sNewVertexMan, *vPos[0], wIndex[0], sNewVertex );

		CalculateVertex( sNewVertex, wIndex[1], *vPos[1], *vPos[2], *vPos[0], *vNor[1] );
		InsertNewVertex( sNewVertexMan, *vPos[1], wIndex[1], sNewVertex );

		CalculateVertex( sNewVertex, wIndex[2], *vPos[2], *vPos[0], *vPos[1], *vNor[2] );
		InsertNewVertex( sNewVertexMan, *vPos[2], wIndex[2], sNewVertex );
	}
	pMesh->UnlockIndexBuffer ();

	pMesh->UnlockVertexBuffer ();

	// Note : 각점들을 정리하기 ( 노멀라이즈 )
	//
	LIST_NEWVERTEXMAN_ITER iter = sNewVertexMan.begin();
	for( ; iter!=sNewVertexMan.end(); ++iter )
	{
		(*iter)->CalDirection();
		(*iter)->m_fThickness = fThickness + (fSizeADD*RANDOM_POS);	// 사이즈. ~!
	}

	return TRUE;
}

BOOL SMeshContainer::IsCartoon_CreateMesh( LPDIRECT3DDEVICEQ pd3dDevice, float fThickness, float fSizeADD )
{
	HRESULT hr(S_OK);

	//Note : List 
	LIST_NEWVERTEXMAN	sNewVertexMan;

	if( pSkinInfo )
	{
		//	Note : 일단 NORMAL쪽 Mesh 생성
		LPD3DXMESH pMeshNORMAL(NULL);
		CreateMeshNORMAL( pd3dDevice, pSkinInfo, pAdjacency, pOrigMesh, pMeshNORMAL );

		// Note : 점들을 모은다.
		IsPointInOder( pd3dDevice, pMeshNORMAL, fThickness, fSizeADD, sNewVertexMan );

		// Note : 카툰 메쉬를 생성한다.
		SAFE_DELETE( m_pSlimMesh );
		m_pSlimMesh = new DxSkinVB_ATT;
		m_pSlimMesh->CloneMeshNONINDEXED( pd3dDevice, pSkinInfo, pAdjacency, pOrigMesh );

		// Note : 카툰 메쉬에 지금까지 정리한 값을 넣는다. ( 위치값만 변화 시킨다. )
		BYTE *pbData = NULL;
		hr = m_pSlimMesh->m_pScaleVB->Lock( 0, 0, (VOID**)&pbData, D3DLOCK_READONLY );
		if ( FAILED(hr) )	return FALSE;

		DWORD dwIndex(0L);
		D3DXVECTOR3* pVector(NULL);
		LIST_NEWVERTEXMAN_ITER iter = sNewVertexMan.begin();
		for( ; iter!=sNewVertexMan.end(); ++iter )
		{
			for( DWORD i=0; i<(*iter)->m_vecIndex.size(); ++i )
			{
				dwIndex = (WORD)(*iter)->m_vecIndex[i];

				pVector = (D3DXVECTOR3*)( pbData + m_pSlimMesh->m_dwScaleSizeFVF * dwIndex );
				*pVector += ((*iter)->m_vDIr*(*iter)->m_fThickness);
			}
		}

		m_pSlimMesh->m_pScaleVB->Unlock();

		// Note : 모든 데이터 정리.
		//
		std::for_each ( sNewVertexMan.begin(), sNewVertexMan.end(), std_afunc::DeleteObject() );
		sNewVertexMan.clear();
		SAFE_RELEASE( pMeshNORMAL );
	}
	else
	{
		// Note : 점들을 모은다.
		IsPointInOder( pd3dDevice, MeshData.pMesh, fThickness, fSizeADD, sNewVertexMan );

		// Note : 카툰 메쉬를 생성한다.
		SAFE_DELETE( m_pSlimMesh );
		m_pSlimMesh = new DxSkinVB_ATT;
		m_pSlimMesh->CloneMeshNONINDEXED( pd3dDevice, MeshData.pMesh );

		// Note : 카툰 메쉬에 지금까지 정리한 값을 넣는다. ( 위치값만 변화 시킨다. )
		BYTE *pbData = NULL;
		hr = m_pSlimMesh->m_pScaleVB->Lock( 0, 0, (VOID**)&pbData, D3DLOCK_READONLY );
		if ( FAILED(hr) )	return FALSE;

		DWORD dwIndex(0L);
		D3DXVECTOR3* pVector(NULL);
		LIST_NEWVERTEXMAN_ITER iter = sNewVertexMan.begin();
		for( ; iter!=sNewVertexMan.end(); ++iter )
		{
			for( DWORD i=0; i<(*iter)->m_vecIndex.size(); ++i )
			{
				dwIndex = (WORD)(*iter)->m_vecIndex[i];

				pVector = (D3DXVECTOR3*)( pbData + m_pSlimMesh->m_dwScaleSizeFVF * dwIndex );
				*pVector += ((*iter)->m_vDIr*(*iter)->m_fThickness);
			}
		}

		m_pSlimMesh->m_pScaleVB->Unlock();

		// Note : 모든 데이터 정리.
		//
		std::for_each ( sNewVertexMan.begin(), sNewVertexMan.end(), std_afunc::DeleteObject() );
		sNewVertexMan.clear();
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: CAllocateHierarchy::FilterMesh
// Desc: Alter or optimize the mesh before adding it to the new mesh container
//--------------------------------------------------------------------------------------
HRESULT FilterMesh( IDirect3DDevice9* pd3dDevice, ID3DXMesh* pMeshIn, ID3DXMesh** ppMeshOut )
{
	HRESULT hr = S_OK;
	ID3DXMesh* pTempMesh = NULL;
	ID3DXMesh* pNewMesh = NULL;

	// Start clean
	(*ppMeshOut) = NULL;

	// Create a new vertex declaration to hold all the required data
	const D3DVERTEXELEMENT9 VertexDecl[] =
    {
        { 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,  0 },
        { 0, 44, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
        { 0, 60, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
        
        D3DDECL_END()
    };
    
    // Clone mesh to the new vertex format
    hr = pMeshIn->CloneMesh( pMeshIn->GetOptions(), VertexDecl, pd3dDevice, &pTempMesh );
	if( FAILED(hr) )
		goto e_Exit;
	
    
    // Compute tangents, which are required for normal mapping
    hr = D3DXComputeTangentFrameEx( pTempMesh, D3DDECLUSAGE_TEXCOORD, 0, D3DDECLUSAGE_TANGENT, 0,
                                    D3DX_DEFAULT, 0, D3DDECLUSAGE_NORMAL, 0,
                                    0, NULL, -1, 0, -1, &pNewMesh, NULL );
	if( FAILED(hr) )
		goto e_Exit;
    
    // Copy the mesh and return
	(*ppMeshOut) = pNewMesh;
	pNewMesh = NULL;
	hr = S_OK;

e_Exit:
	SAFE_RELEASE( pNewMesh );
    SAFE_RELEASE( pTempMesh );
	return S_OK;
}

#include "DxMethods.h"

DWORD			SMeshContainer::VERSION = 0x100;

void SMeshContainer::SaveNORMAL( CSerialFile& SFile )
{
	SFile << VERSION;

	if( m_pSlimMesh )
	{
		SFile << (BOOL)TRUE;
		m_pSlimMesh->Save( SFile );
	}
	else
	{
		SFile << (BOOL)FALSE;
	}
}

void SMeshContainer::LoadNORMAL( LPDIRECT3DDEVICEQ pd3dDevice, CSerialFile& SFile )
{
	SAFE_DELETE( m_pSlimMesh );

	DWORD dwVer(0L);
	SFile >> dwVer;

	BOOL bExist(FALSE);
	SFile >> bExist;
	if( bExist )
	{
		m_pSlimMesh = new DxSkinVB_ATT;
		m_pSlimMesh->Load( pd3dDevice, SFile, MeshData.pMesh );
	}
}


