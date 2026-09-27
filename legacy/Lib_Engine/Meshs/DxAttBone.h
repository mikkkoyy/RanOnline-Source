#pragma once

#include "DxSkinMesh9.h"
#include "DxSkinObject.h"
#include "DxSkinAniMan.h"
#include "DxSkinPieceContainer.h"
#include "DxCharPart.h"
#include "DxSkinEffResult.h"

#include "../DxEffect/EffAni/DxEffAniPlayer.h"
#include "../DxEffect/EffAni/DxEffAni.h"
#include "../DxEffect/EffKeep/DxEffKeepPlayer.h"

#include "DxAttBoneLink.h"
#include "DxAttBoneData.h"

class DxEffKeepDataMain;

class DxAttBone : public DxSkinAniControl
{
private:
	enum		{ FILE_LENGTH = 64 };

	BOOL			m_bWorldObj;
	float			m_fScale;

	DxEffAniPlayer	m_EffAniPlayer;

//------------------------------------------------------------
//				E	f	f		K	e	e	p
private:
	DxEffKeepPlayer	m_EffKeepPlayer;

public:
	SKINEFFDATA		m_sSkinEffData;

public: 
	void CreateEff( DxEffKeepDataMain* pEff );
	void DeleteEff( DxEffKeepDataMain* pEff );
	void Reset();

public:
	static DWORD		g_dwLOD;
	static DWORD		g_dwHIGHDRAW_NUM;

public:
	TCHAR			m_szFileName[FILE_LENGTH];
	float			m_fHeight;
	D3DXVECTOR3		m_vMax, m_vMin;
	DxCharPart		m_PartArray[PIECE_SIZE];
	DxSkeleton*		m_pSkeletonLink;
	std::string		m_strMaskPiece;

	EMPIECECHAR			m_emPieceStrike;
	EMPIECECHAR			m_emPieceType;
	EMPEACEZONEWEAPON	m_emWeaponWhereBack;
	std::string			m_strSkeletonLink;
	std::string			m_strBoneData;
	std::string			m_strBoneTrans;
	
	DXAFFINEMATRIX		m_affBoneTrans;
	DXAFFINEMATRIX		m_affPeaceZone;

	BOOL				m_bAttackMode;

private:
	DWORD			m_dwVertexNUM;

public:
	HRESULT ClearAll ();

public:
	HRESULT SetPiece ( const TCHAR* szFile, LPDIRECT3DDEVICEQ pd3dDevice, DWORD dwFlag = 0x0, int nLevel=0, bool bCharacterData = FALSE );
	HRESULT SetGrindLevel ( int nLevel, EMPIECECHAR emPIECE = PIECE_SIZE );
	void SetHairColor( WORD wColor );
	WORD GetHairColor();

	void SetColor1( int nIndex, WORD wColor );
	void SetColor2( int nIndex, WORD wColor );
	WORD GetColor1( int nIndex );
	WORD GetColor2( int nIndex );

	PDXCHARPART GetPiece ( int i )		{ return &m_PartArray[i]; }
	void ResetPiece ( int i );
	void SetPartRend ( BOOL bRender, EMPIECECHAR emPIECE = PIECE_SIZE );

	const TCHAR* GetFileName()			{ return m_szFileName; }
	DxEffAniPlayer* GetEffAniPlayer()	{ return &m_EffAniPlayer; }
	void SetWorldObj( BOOL bUse )		{ m_bWorldObj = bUse; }
	BOOL GetWorldObj()					{ return m_bWorldObj; }

	void SetCharData ( DxAttBoneLink* pBoneLink, LPDIRECT3DDEVICEQ pd3dDevice, int nLevel=0, bool bCharacterData = FALSE );
	void SetRadiusHeight( DxAttBoneLink* pBoneLink );
	DWORD	GetVertexNum()	{	return m_dwVertexNUM;	}
	void GetAABBBox( D3DXVECTOR3& vMax, D3DXVECTOR3& vMin )
	{
		vMax = m_vMax;
		vMin = m_vMin;
	}

private:
	void CollectVertexNUM();
	void AvailableVB_InitVB();

public:
	HRESULT FrameMove( const float fTime, const float fElapsedTime, const BOOL bContinue=TRUE, BOOL const bFreeze=FALSE, const BOOL bAttackMode=TRUE );

private:
	HRESULT RenderMatrix( const LPDIRECT3DDEVICEQ pd3dDevice, const D3DXMATRIX &matRot, const BOOL bShadow=FALSE, const BOOL bEff=TRUE, const BOOL bReverse = FALSE );
public:
	HRESULT Render( const LPDIRECT3DDEVICEQ pd3dDevice, const DxSkeleton* pSkeleton, const BOOL bShadow=FALSE, const BOOL bEff=TRUE, const BOOL bReverse = FALSE );
	HRESULT RenderShadow( const LPDIRECT3DDEVICEQ pd3dDevice, const D3DXMATRIX &matRot, const BOOL bDay );
	HRESULT RenderReflect( const LPDIRECT3DDEVICEQ pd3dDevice, const D3DXMATRIX &matRot );

	template<class TYPE>
	HRESULT RenderEX( const LPDIRECT3DDEVICEQ pd3dDevice, const D3DXMATRIX &matRot, const TCHAR* pBoneName, const TYPE& BONEROT )
	{
		D3DXMATRIXA16	matLocalRot, matScale;
		D3DXMatrixIdentity( &matScale );
		matScale._11 = m_fScale;
		matScale._22 = m_fScale;
		matScale._33 = m_fScale;
		D3DXMatrixMultiply( &matLocalRot, &matScale, &matRot );
	
		//	Note : Skin Animation.
		DxSkinAniControl::Render( matLocalRot, pBoneName, BONEROT );

		RenderSKIN( pd3dDevice, matLocalRot );

		return S_OK;
	}
	HRESULT RenderSKIN( const LPDIRECT3DDEVICEQ pd3dDevice, const D3DXMATRIX& matLocalRot );
	void RenderBone( LPDIRECT3DDEVICEQ pd3dDevice );

public:
	HRESULT InitDeviceObjects( LPDIRECT3DDEVICEQ pd3dDevice );
	HRESULT RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );
	HRESULT InvalidateDeviceObjects ();
	HRESULT DeleteDeviceObjects ();

public:
	EMANI_MAINTYPE	GetBoneLinkMainAnim();
	EMANI_SUBTYPE	GetBoneLinkSubAnim();

public:
	DxAttBone(void);
	~DxAttBone(void);
};




