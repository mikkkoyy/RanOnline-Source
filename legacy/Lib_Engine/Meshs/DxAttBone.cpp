#include "stdafx.h"

#include "../Common/SerialFile.h"
#include "DxSkinMeshMan.h"
#include "DxSkinDefine.h"

#include "../DxCommon/DxInputDevice.h"
#include "../G-Logic/GLPeriod.h"
#include "../DxOctree/DxLandMan.h"
#include "../DxCommon/DxLightMan.h"
#include "../DxEffect/DxEffectMan.h"
#include "../DxCommon/DxDynamicVB.h"

#include "../DxCommon/EDITMESHS.h"
#include "../DxCommon/COLLISION.h"

#include "DxAttBone.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DWORD	DxAttBone::g_dwLOD = 0;
DWORD	DxAttBone::g_dwHIGHDRAW_NUM = 0;

extern BOOL g_bOBJECT100;
extern TSTRING	g_strPICK_BONE;

DxAttBone::DxAttBone(void) :
	m_fScale(1.0f),
	m_fHeight(20.f),
	m_bWorldObj(FALSE),
	m_dwVertexNUM(0),
	m_emPieceStrike(PIECE_HEAD),
	m_emPieceType(PIECE_HEAD),
	m_emWeaponWhereBack(EMPEACE_WEAPON_RSLOT),
	m_bAttackMode(FALSE),
	m_pSkeletonLink( NULL )
{
	memset( m_szFileName, 0, sizeof(char)*FILE_LENGTH );

	m_strSkeletonLink = "";
	m_strBoneTrans = "";
	m_strBoneData = "";
	m_strMaskPiece = "";

	m_affBoneTrans.UseAffineMatrix();
	m_affPeaceZone.UseAffineMatrix();
}

DxAttBone::~DxAttBone(void)
{
}

HRESULT DxAttBone::ClearAll ()
{
	m_fScale = 1.0f;
	m_bWorldObj = FALSE;

	m_EffAniPlayer.RemoveAllEff();

	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		if ( m_PartArray[i].GetFileName() )
		{
			m_PartArray[i].ReSetPart ();
		}
	}

	m_emPieceStrike = PIECE_HEAD;
	m_emPieceType = PIECE_HEAD;
	m_emWeaponWhereBack = EMPEACE_WEAPON_RSLOT;

	m_strSkeletonLink = "";
	m_strBoneTrans = "";
	m_strBoneData = "";
	m_strMaskPiece = "";

	m_bAttackMode = FALSE;

	m_affBoneTrans.UseAffineMatrix();
	m_affPeaceZone.UseAffineMatrix();
	
	m_pSkeletonLink = NULL;

	DxSkinAniControl::ClearAll ();

	return S_OK;
}

void DxAttBone::ResetPiece ( int i )
{
	GASSERT(i<PIECE_SIZE);
	if ( i >= PIECE_SIZE ) return;

	if ( m_PartArray[i].GetFileName() )
	{
		m_PartArray[i].ReSetPart ();
	}
}

HRESULT DxAttBone::SetPiece ( const TCHAR* szFile, LPDIRECT3DDEVICEQ pd3dDevice, DWORD dwFlag, int nLevel, bool bCharacterData /*= FALSE*/ )
{
	if ( dwFlag&FG_MUSTNEWLOAD )
	{
		for ( int i=0; i<PIECE_SIZE; ++i )
		{
			if ( m_PartArray[i].GetFileName() )
			{
				if ( !_tcscmp(m_PartArray[i].GetFileName(),szFile) )
				{
					m_PartArray[i].ReSetPart ();
				}
			}
		}

		DxSkinPieceContainer::GetInstance().DeletePiece ( szFile );
	}

	DxSkinPiece* pSkinPiece = DxSkinPieceContainer::GetInstance().LoadPiece( szFile, pd3dDevice, TRUE );
	if ( !pSkinPiece )	return E_FAIL;

	if ( dwFlag&FG_BONEFILE_CHECK )
	{
		if ( _tcscmp(pSkinPiece->m_szSkeleton,m_strSkeleton.c_str()) )	return E_DIFFERENT_BONEFILE;
	}

	if ( m_PartArray[pSkinPiece->m_emType].GetFileName() )
	{
		m_PartArray[pSkinPiece->m_emType].ReSetPart ();
	}
	
	if( pSkinPiece->m_emType == PIECE_HAIR )
	{
		if( m_PartArray[PIECE_HEADGEAR].GetFileName() )
		{
			m_PartArray[PIECE_HEADGEAR].ReSetPart ();
			DxSkinPiece* pSkinPieceHG = DxSkinPieceContainer::GetInstance().LoadPiece( m_PartArray[PIECE_HEADGEAR].GetFileName(), pd3dDevice, TRUE );
			if ( !pSkinPieceHG )	goto _RETURN;
			m_PartArray[PIECE_HEADGEAR].SetPart( pSkinPieceHG, pd3dDevice, bCharacterData );
		}
	}

_RETURN:

	m_PartArray[pSkinPiece->m_emType].SetPart ( pSkinPiece, pd3dDevice, bCharacterData );
	m_PartArray[pSkinPiece->m_emType].SetGrindLevel( nLevel );
	
	CollectVertexNUM();

	return S_OK;
}

HRESULT DxAttBone::SetGrindLevel ( int nLevel, EMPIECECHAR emPIECE /*= PIECE_SIZE*/ )
{
	if ( emPIECE >= PIECE_SIZE )
	{
		for ( int i=0; i<PIECE_SIZE; ++i )
			m_PartArray[i].SetGrindLevel(nLevel);
	}else{
		m_PartArray[emPIECE].SetGrindLevel( nLevel );
	}
	
	return S_OK;
}

void DxAttBone::SetHairColor( WORD wColor )
{
	WORD wR, wG, wB;
	wR = wColor >> 10;
	wG = wColor >> 5;
	wG = wG&0x1f;
	wB = wColor&0x1f;

	wR = (WORD)( wR * 8.225806f );	//<< 3;
	wG = (WORD)( wG * 8.225806f );	//<< 3;
	wB = (WORD)( wB * 8.225806f );	//<< 3;

	DWORD dwColor = 0xff000000 + (wR<<16) + (wG<<8) + wB;

	m_PartArray[PIECE_HAIR].SetHairColor( dwColor );
}

WORD DxAttBone::GetHairColor()
{
	DWORD dwColor = m_PartArray[PIECE_HAIR].GetHairColor();

	WORD wR, wG, wB;
	wR = (WORD)((dwColor&0xff0000)>>16);
	wG = (WORD)((dwColor&0xff00)>>8);
	wB = (WORD)(dwColor&0xff);

	wR = wR >> 3;
	wG = wG >> 3;
	wB = wB >> 3;
	return (wR<<10) + (wG<<5) + wB;
}

void DxAttBone::SetColor1( int nIndex, WORD wColor )
{
	WORD wR, wG, wB;
	wR = wColor >> 10;
	wG = wColor >> 5;
	wG = wG&0x1f;
	wB = wColor&0x1f;

	wR = (WORD)( wR * 8.225806f );	//<< 3;
	wG = (WORD)( wG * 8.225806f );	//<< 3;
	wB = (WORD)( wB * 8.225806f );	//<< 3;

	DWORD dwColor = 0xff000000 + (wR<<16) + (wG<<8) + wB;

	if ( nIndex >= PIECE_SIZE )
	{
		for ( int i=0; i<PIECE_SIZE; ++i )
		{
			m_PartArray[i].SetColor1( dwColor );
		}
	}
	else
	{
		m_PartArray[nIndex].SetColor1( dwColor );
	}
}

void DxAttBone::SetColor2( int nIndex, WORD wColor )
{
	WORD wR, wG, wB;
	wR = wColor >> 10;
	wG = wColor >> 5;
	wG = wG&0x1f;
	wB = wColor&0x1f;

	wR = (WORD)( wR * 8.225806f );	//<< 3;
	wG = (WORD)( wG * 8.225806f );	//<< 3;
	wB = (WORD)( wB * 8.225806f );	//<< 3;

	DWORD dwColor = 0xff000000 + (wR<<16) + (wG<<8) + wB;

	if ( nIndex >= PIECE_SIZE )
	{
		for ( int i=0; i<PIECE_SIZE; ++i )
		{
			m_PartArray[i].SetColor2( dwColor );
		}
	}
	else
	{
		m_PartArray[nIndex].SetColor2( dwColor );
	}
}

WORD DxAttBone::GetColor1( int nIndex )
{
	if ( nIndex >= PIECE_SIZE )	return 0;

	DWORD dwColor = m_PartArray[nIndex].GetColor1();

	// 각각 분할
	WORD wR, wG, wB;
	wR = (WORD)((dwColor&0xff0000)>>16);
	wG = (WORD)((dwColor&0xff00)>>8);
	wB = (WORD)(dwColor&0xff);

	// 압축시
	wR = wR >> 3;
	wG = wG >> 3;
	wB = wB >> 3;
	return (wR<<10) + (wG<<5) + wB;
}

WORD DxAttBone::GetColor2( int nIndex )
{
	if ( nIndex >= PIECE_SIZE )	return 0;

	DWORD dwColor = m_PartArray[nIndex].GetColor2();

	// 각각 분할
	WORD wR, wG, wB;
	wR = (WORD)((dwColor&0xff0000)>>16);
	wG = (WORD)((dwColor&0xff00)>>8);
	wB = (WORD)(dwColor&0xff);

	// 압축시
	wR = wR >> 3;
	wG = wG >> 3;
	wB = wB >> 3;
	return (wR<<10) + (wG<<5) + wB;
}

void DxAttBone::SetPartRend ( BOOL bRender, EMPIECECHAR emPIECE /*= PIECE_SIZE*/ )
{
	if ( emPIECE >= PIECE_SIZE )
	{
		for ( int i=0; i<PIECE_SIZE; ++i )
			m_PartArray[i].SetRender ( bRender );
	}else{
		m_PartArray[emPIECE].SetRender ( bRender );
	}	
}

void DxAttBone::SetCharData ( DxAttBoneLink* pBoneLink, LPDIRECT3DDEVICEQ pd3dDevice, int nLevel/*=0*/, bool bCharacterData /*= FALSE*/ )
{
	StringCchCopy( m_szFileName, FILE_LENGTH, pBoneLink->GetFileName() );

	m_EffAniPlayer.InitDeviceObjects ( pd3dDevice );

	EMANI_MAINTYPE MType = DxSkinAniControl::GETCURMTYPE();
	EMANI_SUBTYPE SType = DxSkinAniControl::GETCURSTYPE();
	if ( MType == AN_NONE )	MType = AN_GUARD_N;

	DxSkinAniControl::ClearAll ();

	float fCurTime = DxSkinAniControl::GETCURTIME();

	if ( !pBoneLink->GetData() )	return;

	m_fScale = pBoneLink->GetData()->m_fScale;
	m_vMax = pBoneLink->GetData()->m_vMax;
	m_vMin = pBoneLink->GetData()->m_vMin;
	m_fHeight = pBoneLink->GetData()->m_fHeight;
	m_bWorldObj = pBoneLink->GetData()->m_bWorldObj;

	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		if ( pBoneLink->GetData()->GetPiece(i) )
		{
			m_PartArray[i].SetPart ( pBoneLink->GetData()->GetPiece(i), pd3dDevice, bCharacterData );
			m_PartArray[i].SetGrindLevel( nLevel );
		}else{
			m_PartArray[i].ReSetPart ();
		}
	}

	m_strSkeletonLink = pBoneLink->m_strSkeleton.c_str();
	m_strSkeleton = pBoneLink->GetData()->GetSkeletonName();
	m_strBoneData = pBoneLink->GetData()->GetFileName();
	m_emPieceStrike = pBoneLink->m_emPieceStrike;
	m_emPieceType = pBoneLink->m_emPieceType;
	m_emWeaponWhereBack = pBoneLink->m_emWeaponWhereBack;
	m_strBoneTrans = pBoneLink->m_strBoneTrans.c_str();
	m_strMaskPiece = pBoneLink->m_strMaskPiece;

	m_affBoneTrans.UseAffineMatrix();
	m_affPeaceZone.UseAffineMatrix();

	m_affBoneTrans.SetAffineValue( &pBoneLink->m_affBoneTrans );
	m_affPeaceZone.SetAffineValue( &pBoneLink->m_affPeaceZone );

	m_pSkeleton = DxBoneCollector::GetInstance().Load ( m_strSkeleton.c_str(), pd3dDevice );
	m_pSkeletonLink = DxBoneCollector::GetInstance().Load ( m_strSkeletonLink.c_str(), pd3dDevice );

	DxSkinAniControl::VECNODE& vecANI = pBoneLink->GetData()->GetAniList ();
	size_t nSize = vecANI.size();
	for ( size_t n=0; n<nSize; ++n )
	{
		DxSkinAniControl::LoadAnimation ( vecANI[n]->pAnimCont->m_szName, pd3dDevice );
		m_EffAniPlayer.CheckCreateEff( vecANI[n]->pAnimCont );
	}

	DxSkinAniControl::ClassifyAnimation ();
	DxSkinAniControl::SETCURTIME ( fCurTime );
	DxSkinAniControl::SELECTANI ( MType, SType );

	if ( !GETCURANIMNODE() )
	{
		DxSkinAniControl::SELECTANI ( AN_ABL_STAY, SType );
	}

	if ( !GETCURANIMNODE() )
	{
		CDebugSet::ToLogFile ( _T("ERORR : current animation node null point error [ %s ] [ M %d S %d ]"), pBoneLink->GetData()->GetFileName(), MType, SType );
		if( m_vecAnim.size() )		DEFAULTANI();
	}

	CollectVertexNUM();
}

void DxAttBone::SetRadiusHeight( DxAttBoneLink* pBoneLink )
{
	if ( !pBoneLink ) return ;
	if ( !pBoneLink->GetData() )	return;

	m_vMin = pBoneLink->GetData()->m_vMin;
	m_vMax = pBoneLink->GetData()->m_vMax;
	m_fHeight = pBoneLink->GetData()->m_fHeight;
}

void DxAttBone::CollectVertexNUM()
{
	m_dwVertexNUM = 0;

	for( int i=0; i<PIECE_SIZE; ++i )
	{
		m_dwVertexNUM += m_PartArray[i].GetVertexNUM();
	}
}

void DxAttBone::AvailableVB_InitVB()
{
}

HRESULT DxAttBone::FrameMove ( const float fTime, const float fElapsedTime, const BOOL bContinue, const BOOL bFreeze, const BOOL bAttackMode )
{
	int nPrevKeyTime = DxSkinAniControl::GetPrevKeyFrame();

	if ( bFreeze )		DxSkinAniControl::FrameMove ( 0.0f, bContinue );
	else				DxSkinAniControl::FrameMove ( fElapsedTime, bContinue );

	DxSkinAniControl::SetPrevKeyFrame();

	m_sSkinEffData.Reset();	

	for ( int i=0; i<PIECE_REV01; ++i )
	{
		m_PartArray[i].FrameMove ( fTime, fElapsedTime, GetBoneLinkMainAnim(), GetBoneLinkSubAnim(), GETCURKEYTIME() );
	}

	m_EffAniPlayer.FrameMove ( DxSkinAniControl::GETCURANIM(), fTime, fElapsedTime, m_sSkinEffData );
	m_EffAniPlayer.CheckCreateEff ( DxSkinAniControl::GETCURANIM(), nPrevKeyTime, DxSkinAniControl::GETCURKEYTIME() );
	m_EffKeepPlayer.FrameMove( fElapsedTime, m_sSkinEffData );

	m_bAttackMode = bAttackMode;

	return S_OK;
}

HRESULT DxAttBone::RenderMatrix ( const LPDIRECT3DDEVICEQ pd3dDevice, const D3DXMATRIX &matRot, const BOOL bShadow, const BOOL bEff, const BOOL bReverse )
{
	D3DXMATRIXA16	matLocalRot, matScale;
	D3DXMatrixIdentity( &matScale );
	matScale._11 = m_fScale;
	matScale._22 = m_fScale;
	matScale._33 = m_fScale;
	D3DXMatrixMultiply( &matLocalRot, &matScale, &matRot );

	DxSkinAniControl::Render ( matLocalRot );

	g_dwLOD = 0;
#ifdef USE_SKINMESH_LOD

	if( bShadow )
	{
		g_dwLOD = 1;
	}
	else if( g_dwHIGHDRAW_NUM > 10 )
	{
		D3DXVECTOR3 vPos;
		D3DXVECTOR3& vFromPt = DxViewPort::GetInstance().GetFromPt();
		vPos.x = matLocalRot._41;
		vPos.y = matLocalRot._42;
		vPos.z = matLocalRot._43;

		float fLengthSRC = DxViewPort::GetInstance().GetDistance();
		fLengthSRC += 30.f;
		fLengthSRC = fLengthSRC*fLengthSRC;
		
		D3DXVECTOR3 vDir = vPos - vFromPt;
		float fLength = vDir.x*vDir.x + vDir.y*vDir.y + vDir.z*vDir.z;
		if( fLength > fLengthSRC )
		{
			g_dwLOD = 1;
		}
	}
	else
	{
		D3DXVECTOR3 vPos;
		D3DXVECTOR3& vFromPt = DxViewPort::GetInstance().GetFromPt();
		vPos.x = matLocalRot._41;
		vPos.y = matLocalRot._42;
		vPos.z = matLocalRot._43;

		float fLengthSRC = DxViewPort::GetInstance().GetDistance();
		fLengthSRC += 150.f;
		fLengthSRC = fLengthSRC*fLengthSRC;
		
		D3DXVECTOR3 vDir = vPos - vFromPt;
		float fLength = vDir.x*vDir.x + vDir.y*vDir.y + vDir.z*vDir.z;
		if( fLength > fLengthSRC )
		{
			g_dwLOD = 1;
		}
	}

	if( g_dwLOD==0 )
	{
		++g_dwHIGHDRAW_NUM;
	}
#endif
	
	int nStart=0, nEnd = 0, nAdd = 0;
    
	if ( bReverse )
	{
		nStart = PIECE_REVERSE_RENDER_LAST;
		nEnd = -1;
		nAdd = -1;
	}
	else
	{
		nStart = 0;	
		nEnd = PIECE_REV01;
		nAdd = 1;
	}

	int i = nStart;

	if( m_bWorldObj )
	{
		DWORD	dwNormalizeNormals;
		pd3dDevice->GetRenderState( D3DRS_NORMALIZENORMALS, &dwNormalizeNormals );
		pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
		{
			AvailableVB_InitVB();
				
			i = nStart;
			while ( i != nEnd )
			{
				m_PartArray[i].Render ( pd3dDevice, m_pSkeleton, m_sSkinEffData, bShadow, bEff );
				i += nAdd;
			}

			i = nStart;
			while ( i != nEnd )
			{
				m_PartArray[i].RenderGhosting ( pd3dDevice, this, matLocalRot );
				i += nAdd;

			}

			m_EffAniPlayer.Render ( pd3dDevice, this, matLocalRot );
		}
		pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, dwNormalizeNormals );


		for ( int i=0; i<PIECE_REV01; ++i )
		{
			m_PartArray[i].RenderGlow( pd3dDevice, m_pSkeleton );
		}

		for ( int i=0; i<PIECE_REV01; ++i )
		{
			m_PartArray[i].RenderEff( pd3dDevice, m_pSkeleton, m_fScale );
		}

		return S_OK;
	}

	D3DLIGHTQ	sSrcLight00;
	D3DLIGHTQ	sDestLight00;

	pd3dDevice->GetLight ( 0, &sSrcLight00 );

	sDestLight00 = sSrcLight00;

	sDestLight00.Ambient.r += sSrcLight00.Diffuse.r * 0.2f;
	sDestLight00.Ambient.g += sSrcLight00.Diffuse.g * 0.2f;
	sDestLight00.Ambient.b += sSrcLight00.Diffuse.b * 0.2f;
	sDestLight00.Ambient.r += sSrcLight00.Ambient.r * 0.2f;
	sDestLight00.Ambient.g += sSrcLight00.Ambient.g * 0.2f;
	sDestLight00.Ambient.b += sSrcLight00.Ambient.b * 0.2f;
	sDestLight00.Diffuse.r = sSrcLight00.Diffuse.r * 0.6f;
	sDestLight00.Diffuse.g = sSrcLight00.Diffuse.g * 0.6f;
	sDestLight00.Diffuse.b = sSrcLight00.Diffuse.b * 0.6f;

	pd3dDevice->SetLight ( 0, &sDestLight00 );

	pd3dDevice->LightEnable( 1, FALSE );
	pd3dDevice->LightEnable( 2, FALSE );
	pd3dDevice->LightEnable( 3, FALSE );
	pd3dDevice->LightEnable( 4, FALSE );
	pd3dDevice->LightEnable( 5, FALSE );
	pd3dDevice->LightEnable( 6, FALSE );
	pd3dDevice->LightEnable( 7, FALSE );

	D3DXVECTOR3 vCharPos( matLocalRot._41, matLocalRot._42, matLocalRot._43 );
	DxLightMan::GetInstance()->SetCharStaticPLight( pd3dDevice, vCharPos );
	DxLightMan::GetInstance()->SetCharDynamicPLight( pd3dDevice, vCharPos );

	DWORD	dwFogEnable;
	pd3dDevice->GetRenderState( D3DRS_FOGENABLE, &dwFogEnable );
	pd3dDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );

	{
		DWORD	dwNormalizeNormals;
		pd3dDevice->GetRenderState( D3DRS_NORMALIZENORMALS, &dwNormalizeNormals );
		pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );

		AvailableVB_InitVB();

		switch( m_sSkinEffData.m_bNewHeadDRAW )
		{
		case TRUE:

			i = nStart;
			while ( i != nEnd )
			{
				m_PartArray[i].Render( pd3dDevice, m_pSkeleton, m_sSkinEffData, bShadow, bEff );
				i += nAdd;
			}	
			break;

		case FALSE:
			i = nStart;
			while ( i != nEnd )
			{
				m_PartArray[i].Render( pd3dDevice, m_pSkeleton, m_sSkinEffData, bShadow, bEff );
				i += nAdd;
			}
			break;
		};
		
		for ( int i=0; i<PIECE_REV01; ++i )
		{
			m_PartArray[i].RenderGhosting ( pd3dDevice, this, matLocalRot );
		}
		m_EffAniPlayer.Render( pd3dDevice, this, matLocalRot );
		m_EffKeepPlayer.Render( pd3dDevice );

		pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, dwNormalizeNormals );
	}

	for ( int i=0; i<PIECE_REV01; ++i )
	{
		m_PartArray[i].RenderGlow( pd3dDevice, m_pSkeleton );
	}

	for ( int i=0; i<PIECE_REV01; ++i )
	{
		m_PartArray[i].RenderEff( pd3dDevice, m_pSkeleton, m_fScale );
	}

	pd3dDevice->SetRenderState( D3DRS_FOGENABLE, dwFogEnable );

	pd3dDevice->SetLight ( 0, &sSrcLight00 );

	pd3dDevice->LightEnable( 1, FALSE );
	pd3dDevice->LightEnable( 2, FALSE );
	pd3dDevice->LightEnable( 3, FALSE );
	pd3dDevice->LightEnable( 4, FALSE );
	pd3dDevice->LightEnable( 5, FALSE );

	return S_OK;
}

HRESULT DxAttBone::RenderSKIN( const LPDIRECT3DDEVICEQ pd3dDevice, const D3DXMATRIX& matLocalRot )
{
	D3DLIGHTQ	sSrcLight00;
	D3DLIGHTQ	sDestLight00;

	pd3dDevice->GetLight ( 0, &sSrcLight00 );

	sDestLight00 = sSrcLight00;

	sDestLight00.Ambient.r += sSrcLight00.Diffuse.r * 0.2f;
	sDestLight00.Ambient.g += sSrcLight00.Diffuse.g * 0.2f;
	sDestLight00.Ambient.b += sSrcLight00.Diffuse.b * 0.2f;
	sDestLight00.Ambient.r += sSrcLight00.Ambient.r * 0.2f;
	sDestLight00.Ambient.g += sSrcLight00.Ambient.g * 0.2f;
	sDestLight00.Ambient.b += sSrcLight00.Ambient.b * 0.2f;
	sDestLight00.Diffuse.r = sSrcLight00.Diffuse.r * 0.6f;
	sDestLight00.Diffuse.g = sSrcLight00.Diffuse.g * 0.6f;
	sDestLight00.Diffuse.b = sSrcLight00.Diffuse.b * 0.6f;

	pd3dDevice->SetLight ( 0, &sDestLight00 );

	pd3dDevice->LightEnable( 1, FALSE );
	pd3dDevice->LightEnable( 2, FALSE );
	pd3dDevice->LightEnable( 3, FALSE );
	pd3dDevice->LightEnable( 4, FALSE );
	pd3dDevice->LightEnable( 5, FALSE );
	pd3dDevice->LightEnable( 6, FALSE );
	pd3dDevice->LightEnable( 7, FALSE );

	D3DXVECTOR3 vCharPos( matLocalRot._41, matLocalRot._42, matLocalRot._43 );
	DxLightMan::GetInstance()->SetCharStaticPLight( pd3dDevice, vCharPos );
	DxLightMan::GetInstance()->SetCharDynamicPLight( pd3dDevice, vCharPos );

	DWORD	dwFogEnable;
	pd3dDevice->GetRenderState( D3DRS_FOGENABLE, &dwFogEnable );
	pd3dDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );

	{
		DWORD	dwNormalizeNormals;
		pd3dDevice->GetRenderState( D3DRS_NORMALIZENORMALS, &dwNormalizeNormals );
		pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );

		AvailableVB_InitVB();

		switch( m_sSkinEffData.m_bNewHeadDRAW )
		{
		case TRUE:
			for ( int i=1; i<PIECE_REV01; ++i )
			{
				m_PartArray[i].Render( pd3dDevice, m_pSkeleton, m_sSkinEffData, FALSE, TRUE );
			}
			break;

		case FALSE:
			for ( int i=0; i<PIECE_REV01; ++i )
			{
				m_PartArray[i].Render( pd3dDevice, m_pSkeleton, m_sSkinEffData, FALSE, TRUE );
			}
			break;
		};
		
		for ( int i=0; i<PIECE_REV01; ++i )
		{
			m_PartArray[i].RenderGhosting ( pd3dDevice, this, matLocalRot );
		}

		m_EffAniPlayer.Render( pd3dDevice, this, matLocalRot );
		m_EffKeepPlayer.Render( pd3dDevice );

		pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, dwNormalizeNormals );
	}

	for ( int i=0; i<PIECE_REV01; ++i )
	{
		m_PartArray[i].RenderGlow( pd3dDevice, m_pSkeleton );
	}

	for ( int i=0; i<PIECE_REV01; ++i )
	{
		m_PartArray[i].RenderEff( pd3dDevice, m_pSkeleton, m_fScale );
	}

	pd3dDevice->SetRenderState( D3DRS_FOGENABLE, dwFogEnable );

	pd3dDevice->SetLight ( 0, &sSrcLight00 );

	pd3dDevice->LightEnable( 1, FALSE );
	pd3dDevice->LightEnable( 2, FALSE );
	pd3dDevice->LightEnable( 3, FALSE );
	pd3dDevice->LightEnable( 4, FALSE );
	pd3dDevice->LightEnable( 5, FALSE );

	return S_OK;
}

HRESULT DxAttBone::RenderShadow( const LPDIRECT3DDEVICEQ pd3dDevice, const D3DXMATRIX &matRot, const BOOL bDay )
{
	DWORD dwFogEnable;
	pd3dDevice->GetRenderState( D3DRS_FOGENABLE, &dwFogEnable );
	pd3dDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );

	float fHeight=0.f;
	if( !bDay )
	{
		fHeight = -matRot._42;
	}

	for ( int i=0; i<PIECE_REV01; ++i )
	{
		m_PartArray[i].RenderShadow( pd3dDevice, m_pSkeleton, fHeight );
	}

	pd3dDevice->SetRenderState( D3DRS_FOGENABLE, dwFogEnable );

	return S_OK;
}

HRESULT DxAttBone::RenderReflect( const LPDIRECT3DDEVICEQ pd3dDevice, const D3DXMATRIX &matRot )
{
	D3DXMATRIX	matLocalRot, matScale;
	D3DXMatrixIdentity( &matScale );
	matScale._11 = m_fScale;
	matScale._22 = m_fScale;
	matScale._33 = m_fScale;
	D3DXMatrixMultiply( &matLocalRot, &matScale, &matRot );

	DxSkinAniControl::Render ( matLocalRot );

	DWORD dwFogEnable;
	DWORD	dwNormalizeNormals;
	pd3dDevice->GetRenderState( D3DRS_NORMALIZENORMALS, &dwNormalizeNormals );
	pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
	pd3dDevice->GetRenderState( D3DRS_FOGENABLE, &dwFogEnable );
	pd3dDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );

	for ( int i=0; i<PIECE_REV01; ++i )
	{
		m_PartArray[i].Render( pd3dDevice, m_pSkeleton, m_sSkinEffData, FALSE, FALSE );
	}

	pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, dwNormalizeNormals );
	pd3dDevice->SetRenderState( D3DRS_FOGENABLE, dwFogEnable );

	return S_OK;
}

void DxAttBone::RenderBone( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if( !m_pSkeleton )	return;

	pd3dDevice->SetRenderState( D3DRS_ZENABLE,		FALSE );
	pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,	FALSE );

	m_pSkeleton->EditLineSphere( pd3dDevice );

	pd3dDevice->SetRenderState( D3DRS_ZENABLE,		TRUE );
	pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,	TRUE );

    if( DxInputDevice::GetInstance().GetMouseState(DXMOUSE_LEFT)&DXKEY_UP )
	{
		D3DXVECTOR3 vTargetPt, vFromPt;
		vFromPt = DxViewPort::GetInstance().GetFromPt ();
		BOOL bTargetted = DxViewPort::GetInstance().GetMouseTargetPosWnd ( vTargetPt );
		if ( bTargetted )
		{
			m_pSkeleton->IsCollision( vFromPt, vTargetPt, g_strPICK_BONE, 0.006f*DxViewPort::GetInstance().GetDistance() );
			CDebugSet::ToView( 9, _T("Select Bone : %s"), g_strPICK_BONE.c_str() );
		}
	}

	if( m_pSkeleton && !g_bOBJECT100 )	
	{
		pd3dDevice->SetRenderState( D3DRS_ZENABLE,		FALSE );
		pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,	FALSE );

		m_pSkeleton->CheckSphere( pd3dDevice, g_strPICK_BONE.c_str() );

		pd3dDevice->SetRenderState( D3DRS_ZENABLE,		TRUE );
		pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,	TRUE );
	}
}

HRESULT DxAttBone::InitDeviceObjects( LPDIRECT3DDEVICEQ pd3dDevice )
{
	return S_OK;
}

HRESULT DxAttBone::RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	for ( int i=0; i<PIECE_REV01; ++i )
	{
		m_PartArray[i].RestoreDeviceObjects ( pd3dDevice );
	}

	m_EffAniPlayer.RestoreDeviceObjects ();

	return S_OK;
}

HRESULT DxAttBone::InvalidateDeviceObjects ()
{
	for ( int i=0; i<PIECE_REV01; ++i )
	{
		m_PartArray[i].InvalidateDeviceObjects ();
	}

	m_EffAniPlayer.InvalidateDeviceObjects ();

	return S_OK;
}

HRESULT DxAttBone::DeleteDeviceObjects ()
{
	for ( int i=0; i<PIECE_REV01; ++i )
	{
		m_PartArray[i].DeleteDeviceObjects ();
	}

	m_EffAniPlayer.DeleteDeviceObjects ();

	return S_OK;
}

void DxAttBone::CreateEff( DxEffKeepDataMain* pEff )
{
	m_EffKeepPlayer.CreateEff( pEff );
}

void DxAttBone::DeleteEff( DxEffKeepDataMain* pEff )
{
}

void DxAttBone::Reset()
{
	m_EffKeepPlayer.Reset();
}

EMANI_MAINTYPE DxAttBone::GetBoneLinkMainAnim()
{
	switch ( GETCURMTYPE() )
	{
	case AN_ABL_STAY:	return	AN_GUARD_N;
	case AN_ABL_ATTACK:	return	AN_ATTACK;
	case AN_ABL_FREE:	return	AN_DIE;
	};

	return	AN_NONE;
}

EMANI_SUBTYPE DxAttBone::GetBoneLinkSubAnim()
{
	return GETCURSTYPE();
}


HRESULT DxAttBone::Render( const LPDIRECT3DDEVICEQ pd3dDevice, const DxSkeleton* pSkeleton, const BOOL bShadow/*=FALSE*/, const BOOL bEff/*=TRUE*/, const BOOL bReverse /*= FALSE*/ )
{
	if( !pSkeleton )	return E_FAIL;
	if( !m_affBoneTrans.m_pAffineParts )	return E_FAIL;
	if( !m_affPeaceZone.m_pAffineParts )	return E_FAIL;

	DxBoneTrans *pBoneCur = pSkeleton->FindBone ( m_strBoneTrans.c_str() );
	D3DXVECTOR3 vLocalTrans = m_affBoneTrans.m_pAffineParts->vTrans;
	D3DXVECTOR3 vLocalRotate = m_affBoneTrans.m_pAffineParts->vRotate;
	D3DXVECTOR3	vLocalScale = m_affBoneTrans.m_pAffineParts->vScale;
	
	if ( !m_bAttackMode && ( m_emPieceType == PIECE_RHAND || m_emPieceType == PIECE_LHAND ) )
	{
		pBoneCur = pSkeleton->FindBone ( SKINTYPE_STRING::m_szWHEREBACKTYPE[ m_emWeaponWhereBack ] );

		vLocalTrans = m_affPeaceZone.m_pAffineParts->vTrans;
		vLocalRotate = m_affPeaceZone.m_pAffineParts->vRotate;
		vLocalScale = m_affPeaceZone.m_pAffineParts->vScale;
	}

	if ( !pBoneCur )	return E_FAIL;

	D3DXMATRIX matBone, matScale, matRotate, matTrans;
	D3DXMatrixIdentity( &matBone );
	D3DXMatrixIdentity( &matScale );

	matBone = pBoneCur->matCombined;
	D3DXMatrixScaling ( &matScale, vLocalScale.x, vLocalScale.y, vLocalScale.z );
	D3DXMatrixRotationYawPitchRoll( &matRotate, D3DXToRadian( vLocalRotate.y ),D3DXToRadian( vLocalRotate.x ),D3DXToRadian( vLocalRotate.z ) );
	D3DXMatrixTranslation( &matTrans, vLocalTrans.x, vLocalTrans.y, vLocalTrans.z );

	D3DXMATRIX matCombine1, matCombine2, matCombine3;
	D3DXMatrixMultiply( &matCombine1, &matScale, &matRotate );
	D3DXMatrixMultiply( &matCombine2, &matCombine1, &matTrans );
	D3DXMatrixMultiply( &matCombine3, &matCombine2, &matBone );

	return RenderMatrix( pd3dDevice, matCombine3, bShadow, bEff, bReverse );
}
