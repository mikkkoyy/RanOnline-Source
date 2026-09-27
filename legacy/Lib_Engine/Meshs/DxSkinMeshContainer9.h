#pragma once

class CSerialFile;
class basestream;

struct DxSkinVB_ATT;
struct SMATERIAL_PIECE;
struct DXMATERIAL_CHAR_EFF;

#include "DxSMeshDATA.h"

struct DXMATERIALEX
{
	enum
	{
		FLAG_ALPHA_HARD		= 0x0001,
		FLAG_ALPHA_SOFT		= 0x0002,
	};

	DWORD				dwFlags;
	DWORD				m_dwTexID;
	LPDIRECT3DTEXTUREQ	pTexture;

	DXMATERIALEX () :
		dwFlags(NULL),
		m_dwTexID(0),
		pTexture(NULL)
	{
	}

	void RELEASE ( LPD3DXMATERIAL pMaterials );
};

//typedef struct D3DXMATERIAL {
//    D3DMATERIAL9 MatD3D;
//    LPSTR pTextureFilename;
//} D3DXMATERIAL;

//typedef enum D3DXMESHDATATYPE {
//    D3DXMESHTYPE_MESH = 0x001,
//    D3DXMESHTYPE_PMESH = 0x002,
//    D3DXMESHTYPE_PATCHMESH = 0x003,
//    D3DXEDT_FORCE_DWORD = 0x7fffffff
//} D3DXMESHDATATYPE;

//typedef struct D3DXMESHDATA {
//
//    D3DXMESHDATATYPE Type;
//	
//    union
//    {
//    LPD3DXMESH      pMesh;
//    LPD3DXPMESH     pPMesh;
//    LPD3DXPATCHMESH pPatchMesh;
//    }
//	
//} D3DXMESHDATA, *LPD3DXMESHDATA;


//typedef struct _D3DXMESHCONTAINER {
//    LPSTR Name;
//    D3DXMESHDATA MeshData;
//    LPD3DXMATERIAL pMaterials;
//    LPD3DXEFFECTINSTANCE pEffects;
//    DWORD NumMaterials;
//    DWORD *pAdjacency;
//    LPD3DXSKININFO pSkinInfo;
//    struct _D3DXMESHCONTAINER *pNextMeshContainer;
//} D3DXMESHCONTAINER, *LPD3DXMESHCONTAINER;


#define COL_ERR (-1)


struct SVERTEXINFLU
{
	enum { MAX_BONE = 6, };
	const static DWORD	VERSION;

	DWORD		m_dwIndex;
	D3DXVECTOR3	m_vVector;
	D3DXVECTOR3	m_vNormal;

	DWORD		m_dwNumBone;
	DWORD		m_pBone[MAX_BONE];
	float		m_pWeights[MAX_BONE];

	BOOL SaveFile ( CSerialFile &SFile );
	BOOL LoadFile ( basestream &SFile );

	SVERTEXINFLU ();
	void SetBoneNum ( DWORD _dwBone );
};

struct SBoneInfluence
{
	enum { MAX_BONE = 6, };
	DWORD	dwInfue;
	DWORD	pVertices[MAX_BONE];
	float	pWeights[MAX_BONE];

	SBoneInfluence ();
	void SetInfue ( DWORD _dwInfue );
};

//--------------------------------------------------------------------------------------
// Name: struct D3DXMESHCONTAINER_DERIVED
// Desc: Structure derived from D3DXMESHCONTAINER so we can add some app-specific
//       info that will be stored with each mesh
//--------------------------------------------------------------------------------------
struct SMeshContainer : public D3DXMESHCONTAINER
{
	static	DWORD			VERSION;

	DXMATERIALEX*			pMaterialEx;

	// SkinMesh info
	LPD3DXMESH				pOrigMesh;
	LPD3DXATTRIBUTERANGE	pAttributeTable;

	DWORD					NumAttributeGroups; 
	DWORD					NumInfl;
	LPD3DXBUFFER			pBoneCombinationBuf;
	char*					szBoneName;
	DWORD					dwNumBoneMatrix;
	D3DXMATRIX**			ppBoneMatrixPtrs;
	D3DXMATRIX*				pBoneOffsetMatrices;
	DWORD					NumPaletteEntries;
	bool					UseSoftwareVP;
	DWORD					iAttributeSW;		// used to denote the split between SW and HW if necessary for non-indexed skinning

	// Note : Software Render 시. Lock 안 하기 위한 버퍼 생성.~!
	BYTE*					m_pVB_DATA;
	DWORD					m_dwFVF;
	DWORD					m_dwVertexNUM;
	
	DxSkinVB_ATT*			m_pSlimMesh;

	SMeshContainer			*pGlobalNext;

public:
	SMeshContainer () :
		szBoneName(NULL),
		dwNumBoneMatrix(0),
		pGlobalNext(NULL),
		m_pVB_DATA(NULL),
		m_dwFVF(0L),
		m_dwVertexNUM(0),
		m_pSlimMesh(NULL)
	{
	}

	~SMeshContainer();

public:
	HRESULT GenerateSkinnedMesh ( IDirect3DDevice9 *pd3dDevice, bool &bUseSoftwareVP );
	BOOL IsCartoon_CreateMesh( LPDIRECT3DDEVICEQ pd3dDevice, float fThickness, float fSizeADD );	// 카툰 메쉬가 없다면 생성을 한다.

public:
	LPDIRECT3DTEXTUREQ SetMaterial ( LPDIRECT3DDEVICEQ pd3dDevice, DWORD nIndex, BOOL& bZBiasChange, SMATERIAL_PIECE *pmtrlPiece=NULL, DXMATERIAL_CHAR_EFF* pmtrlSpecular=NULL );
	HRESULT ReSetMaterial ( LPDIRECT3DDEVICEQ pd3dDevice );

public:
	BOOL IsAlphaTex_HARD ( DWORD nMtIndex );
	BOOL IsAlphaTex_SOFT ( DWORD nMtIndex );

public:
	BOOL IsCollision ( const D3DXVECTOR3 &vPoint1, const D3DXVECTOR3 &vPoint2, D3DXVECTOR3 &vDetectPos, DWORD &dwDetectFace );
	BOOL GetClosedPoint ( const D3DXVECTOR3 &vPoint1, const D3DXVECTOR3 &vPoint2, D3DXVECTOR3 &vDetectPos, DWORD &dwDetectIndex,
							D3DXVECTOR3& vCLOSED_POINT, D3DXVECTOR3& vCLOSED_NORMAL );
	BOOL GetClosedPoint2 ( const D3DXVECTOR3 &vPoint1, const D3DXVECTOR3 &vPoint2, D3DXVECTOR3 &vDetectPos, DWORD &dwDetectIndex,
							D3DXVECTOR3& vCLOSED_POINT, D3DXVECTOR3& vCLOSED_NORMAL );
	HRESULT GetVertexInfluences ( const DWORD dwIndex, SVERTEXINFLU *pVertInflu );
	HRESULT CalculateVertexInfluences( SVERTEXINFLU *pVertInflu, D3DXVECTOR3 &vVecOutput, D3DXVECTOR3 &vNorOutput, LPD3DXMATRIX pBoneMatrice, 
										BOOL bIdentity );
	HRESULT CalculateBoundingBox ( LPDIRECT3DDEVICEQ pd3dDevice, LPD3DXMATRIX pBoneMatrices, D3DXVECTOR3 &vMax, D3DXVECTOR3 &vMin );

public:
	DWORD GetNumMaterials ()		{	return NumMaterials; }

public:
	HRESULT UpdateSkinnedMesh( PBYTE pbData, DWORD dwFVF, LPD3DXMATRIX pBoneMatrices );

private:
	void GenerateSkinnedMeshNON( IDirect3DDevice9 *pd3dDevice );

private:
	void CreateLOD_SMeshNON( IDirect3DDevice9* pd3dDevice, LPD3DXMESH pMesh, DWORD* pAdjacencySRC, LPD3DXMESH& pLOD_HIGH, LPD3DXMESH& pLOD_LOW );

public:
	void SaveNORMAL( CSerialFile& SFile );
	void LoadNORMAL( LPDIRECT3DDEVICEQ pd3dDevice, CSerialFile& SFile );
};

typedef SMeshContainer*		PSMESHCONTAINER;



