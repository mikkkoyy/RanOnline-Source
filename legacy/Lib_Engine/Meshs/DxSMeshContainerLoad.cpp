#include "stdafx.h"

#include "DxSkinDefine.h"

#include "DxSkinMesh9.h"
#include "DxSkinMesh9_NORMAL.h"

#include "DxSkinMeshContainer9.h"

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
*/

//----------------------------------------------------------------------------------------------------------------------------------
//													M		A		I		N
//----------------------------------------------------------------------------------------------------------------------------------
HRESULT SMeshContainer::GenerateSkinnedMesh( IDirect3DDevice9 *pd3dDevice, bool &bUseSoftwareVP )
{
    if( !pSkinInfo )	return S_OK;

    bUseSoftwareVP = false;

    SAFE_RELEASE( MeshData.pMesh );
    SAFE_RELEASE( pBoneCombinationBuf );

	GenerateSkinnedMeshNON( pd3dDevice );

    return S_OK;
}

//----------------------------------------------------------------------------------------------------------------------------------
//											N	O	N		I	N	D	E	X	E	D
//----------------------------------------------------------------------------------------------------------------------------------
void SMeshContainer::GenerateSkinnedMeshNON( IDirect3DDevice9 *pd3dDevice )
{
	HRESULT hr(S_OK);

	D3DCAPS9 d3dCaps;
    pd3dDevice->GetDeviceCaps( &d3dCaps );

	hr = pSkinInfo->ConvertToBlendedMesh
                                (
                                    pOrigMesh,
                                    D3DXMESH_MANAGED|D3DXMESHOPT_VERTEXCACHE, 
                                    pAdjacency, 
                                    NULL, NULL, NULL, 
                                    &NumInfl,
                                    &NumAttributeGroups, 
                                    &pBoneCombinationBuf, 
                                    &MeshData.pMesh
                                );
    if (FAILED(hr))
	{
        return;
	}


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

        if (cInfl > d3dCaps.MaxVertexBlendMatrices)
        {
            break;
        }
    }

    // if there is both HW and SW, add the Software Processing flag
    if (iAttributeSW < NumAttributeGroups)
    {
		BOOL bSoftWareNONINDEXED = TRUE;


        LPD3DXMESH pMeshTmp;

        hr = MeshData.pMesh->CloneMeshFVF(D3DXMESH_SOFTWAREPROCESSING|MeshData.pMesh->GetOptions(), 
                                            MeshData.pMesh->GetFVF(),
                                            pd3dDevice, &pMeshTmp);
        if (FAILED(hr))
        {
            return;
        }

        MeshData.pMesh->Release();
        MeshData.pMesh = pMeshTmp;
        pMeshTmp = NULL;
    }

	// allocate a buffer for bone matrices, but only if another mesh has not allocated one of the same size or larger
	if( DxSkinMesh9_NORMAL::g_NumBoneMatricesMax < pSkinInfo->GetNumBones() )
	{
		DxSkinMesh9_NORMAL::g_NumBoneMatricesMax = pSkinInfo->GetNumBones();

		delete[] DxSkinMesh9_NORMAL::g_pBoneMatrices; 
		DxSkinMesh9_NORMAL::g_pBoneMatrices  = new D3DXMATRIXA16[DxSkinMesh9_NORMAL::g_NumBoneMatricesMax];
		if( DxSkinMesh9_NORMAL::g_pBoneMatrices == NULL )
		{
			hr = E_OUTOFMEMORY;
			return;
		}
	}
}
