// ReplaceContainer.h: interface for the CReplaceContainer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <string>
#include <map>

#include "../Common/basestream.h"
#include "../Common/SerialFile.h"
#include "../Common/StlFunctions.h"
#include "./DxSkinMeshContainer9.h"
#include "./DxSkinMesh9.h"
#include "./DxPieceDefine.h"
#include "./DxSkinAniMan.h"
#include "./DxSTrace.h"
#include "./DxSkinPieceMaterial.h"
#include "./DxUserSlot.h"

#ifndef DxEffChar
class DxEffChar;
#endif

typedef std::vector<DxEffChar*>						GLEFFCHAR_VEC;
typedef GLEFFCHAR_VEC::iterator						GLEFFCHAR_VEC_ITER;
typedef GLEFFCHAR_VEC::reverse_iterator				GLEFFCHAR_VEC_RITER;

struct DxSkinPiece
{
public:
	enum 
	{ 
		VERSION_CPS		= 0x0201,  
		VERSION_ENCRYPT = 0x0200,
	};

public:
	char*				m_szFileName;

	DWORD				m_dwRef;

	char*				m_szXFileName;
	DxSkinMesh9*		m_pSkinMesh;

	char*				m_szSkeleton;
	DxSkeleton*			m_pSkeleton;

	EMPIECECHAR			m_emType;
	EMPEACEZONEWEAPON	m_emWeaponWhereBack;
	char*				m_szMeshName;
	PSMESHCONTAINER		m_pmcMesh;
	DWORD				m_dwVertexNUM;			// 정점 갯수를 미리.. 넣어 놓는다.

	DWORD				m_dwMaterialNum;
	SMATERIAL_PIECE*	m_pMaterialPiece;

	DXSUSER_SLOT		m_dxUserSlot;
	// Note : 무슨 효과가 들어있는지를 알아내는 Flag 실시간 변함
	//
	DWORD				m_dwFlags;

	// Note : 효과 셋팅, Save, Load
	DWORD				m_dwFlag;

	//	Note : 추적 위치값 들.
	//
	VECTRACE			m_vecTrace;

	//	Note : 스킨 조각에 부착된 효과들.
	//
	GLEFFCHAR_VEC		m_vecEFFECT;

protected:
	void SumEffFlag ();

public:
	void SelectMesh ( PSMESHCONTAINER pmcMesh );

public:
	void AddEffList ( DxEffChar* pEffChar );
	void DelEffList ( DxEffChar* pEffChar );
	int	EffUp( DWORD dwIndex );
	int	EffDown( DWORD dwIndex );

public:
	HRESULT SetSkinMesh ( const char* szXFileName, const char* szSkeleton, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );

public:
	HRESULT LoadPiece ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );
	HRESULT LoadPiece_0101 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );
	HRESULT LoadPiece_0102 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );
	HRESULT LoadPiece_0103 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );
	HRESULT LoadPiece_0104 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );
	HRESULT LoadPiece_0105 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );
	HRESULT LoadPiece_0106 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );
	HRESULT LoadPiece_0107 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );

	HRESULT LoadPiece_0108 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );
	HRESULT LoadPiece_0109 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );
	HRESULT LoadPiece_0110 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );
	HRESULT LoadPiece_0112_0113 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );
	HRESULT LoadPiece_0114_0115 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );
	HRESULT LoadPiece_0116 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );
	HRESULT LoadPiece_0200 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );
	HRESULT LoadPiece_0201 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );

	HRESULT SavePiece ( const char* szFile );

public:
	HRESULT InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );
	HRESULT RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );
	HRESULT InvalidateDeviceObjects ();
	HRESULT DeleteDeviceObjects();
	void ClearAll ();

public:
	HRESULT FrameMove ( float fTime, float fElapsedTime );
	HRESULT Render ( LPDIRECT3DDEVICEQ pd3dDevice, D3DXMATRIX &matRot, DxSkeleton* pSkeleton );
	HRESULT MeshEditRender ( LPDIRECT3DDEVICEQ pd3dDevice, D3DXMATRIX &matRot, DxSkeleton* pSkeleton );
	HRESULT RenderTEST( LPDIRECT3DDEVICEQ pd3dDevice, D3DXMATRIX &matRot, DxSkeleton* pSkeleton );

public:
	HRESULT DrawMeshContainer ( LPDIRECT3DDEVICEQ pd3dDevice, BOOL bOriginDraw=TRUE, BOOL bAlpha=FALSE, BOOL bShadow=FALSE, BOOL bAlphaTex=TRUE );
	HRESULT DrawTraceVert ( LPDIRECT3DDEVICEQ pd3dDevice );
	HRESULT DrawSelTraceVert ( LPDIRECT3DDEVICEQ pd3dDevice, const char* szSelVert );

public:
	SVERTEXINFLU* GetTracePos ( std::string strTrace );
	void SetTracePos ( std::string strTrace, SVERTEXINFLU &sVertInflu );
	void DelTracePos ( std::string strTrace );

	//	pBoneMatrice -> (주의) 영향을 받는 본의 갯수가 여러개일 경우에 문제가 생김.
	HRESULT CalculateVertexInflu ( std::string strTrace, D3DXVECTOR3 &vVert, D3DXVECTOR3 &vNormal, LPD3DXMATRIX pBoneMatrice=NULL );

	HRESULT CalculateBoundingBox ( LPDIRECT3DDEVICEQ pd3dDevice, D3DXVECTOR3 &vMax, D3DXVECTOR3 &vMin );

public:
	DWORD GetVertexNUM()	{ return m_dwVertexNUM; }	// 정점 갯수.

public:
	void CreateCartoonMesh( LPDIRECT3DDEVICEQ pd3dDevice, float fThickness, float fSizeADD );

public:
	DxSkinPiece ();
	~DxSkinPiece ();
};
typedef DxSkinPiece*	PDXSKINPIECE;


class DxSkinPieceContainer  
{
private:
	typedef std::map<std::string,DxSkinPiece*>				PIECEMAP;
	typedef std::map<std::string,DxSkinPiece*>::iterator	PIECEMAP_ITER;

protected:
	char			m_szPath[MAX_PATH];

public:
	void			SetPath ( char* szPath )	{ StringCchCopy( m_szPath, MAX_PATH, szPath ); }
	char*			GetPath ()					{ return m_szPath; }

protected:
	LPDIRECT3DDEVICEQ	m_pd3dDevice;
	PIECEMAP			m_mapSkinPiece;

public:
	DxSkinPiece* LoadPiece ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );
	HRESULT ReleasePiece ( const char* szFile );
	HRESULT DeletePiece ( const char* szFile );
	DxSkinPiece* FindPiece ( const char* szFile );

public:
	HRESULT CleanUp ();

public:
	HRESULT InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );
	HRESULT RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );
	HRESULT InvalidateDeviceObjects ();
	HRESULT DeleteDeviceObjects();
	HRESULT FinalCleanup();

protected:
	DxSkinPieceContainer();

public:
	virtual ~DxSkinPieceContainer();

public:
	static DxSkinPieceContainer& GetInstance();
};

