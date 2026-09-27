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

#include "DxSkinChar.h"

#include "../DxEffect/Char/DxEffSelfEffect.h" /*Item Self Effect, Juver, 2017/06/09 */

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DWORD	DxSkinChar::g_dwLOD = 0;
DWORD	DxSkinChar::g_dwHIGHDRAW_NUM = 0;

extern BOOL g_bOBJECT100;
extern TSTRING	g_strPICK_BONE;

DxSkinChar::DxSkinChar(void) :
	m_fScale(1.0f),
	m_fScaleOrig(1.0f),
	m_fHeight(20.f),
	m_bWorldObj(FALSE),
	m_dwVertexNUM(0)
{
	memset( m_szFileName, 0, FILE_LENGTH );

	/*ABL system, Juver, 2017/06/01 */
	for ( int i=0; i<PIECE_SIZE; ++i )
		m_pAttBone[i] = NULL;

	/*vehicle system, Juver, 2017/08/03 */
	m_pVehicle = NULL;
}

DxSkinChar::~DxSkinChar(void)
{
	/*ABL system, Juver, 2017/06/01 */
	for ( int i=0; i<PIECE_SIZE; ++i )
		SAFE_DELETE( m_pAttBone[i] );

	/*vehicle system, Juver, 2017/08/03 */
	SAFE_DELETE( m_pVehicle );
}

HRESULT DxSkinChar::SetPiece ( const TCHAR* szFile, LPDIRECT3DDEVICEQ pd3dDevice, DWORD dwFlag, int nLevel, bool bCharacterData /*= FALSE*/, std::string strSelfEffect /*= ""*/ )
{
	//	Note : 만약 처음 로드된 것을 무시하고 새로이 로드할때에는 원래 로드된걸 지워준다.
	//
	if ( dwFlag&FG_MUSTNEWLOAD )
	{
		for ( int i=0; i<PIECE_SIZE; ++i )
		{
			/*ABL system, Juver, 2017/06/01 */
			SAFE_DELETE( m_pAttBone[i] );

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

	//	Note : Piece를 읽어온다.
	//
	DxSkinPiece* pSkinPiece = DxSkinPieceContainer::GetInstance().LoadPiece( szFile, pd3dDevice, TRUE );
	if ( !pSkinPiece )	return E_FAIL;

	if ( dwFlag&FG_BONEFILE_CHECK )
	{
		if ( _tcscmp(pSkinPiece->m_szSkeleton,m_strSkeleton.c_str()) )	return E_DIFFERENT_BONEFILE;
	}

	/*ABL system, Juver, 2017/06/01 */
	if ( m_pAttBone[pSkinPiece->m_emType] )
	{
		DxAttBoneLinkContainer::GetInstance().ReleaseData( m_pAttBone[pSkinPiece->m_emType]->GetFileName() );
		SAFE_DELETE( m_pAttBone[pSkinPiece->m_emType] );
	}
	

	//	Note : 만약 이미 읽은 Piece Type 에 지정된 Piece 가 있다면 지워준다.
	//
	if ( m_PartArray[pSkinPiece->m_emType].GetFileName() )
	{
		m_PartArray[pSkinPiece->m_emType].ReSetPart ();
	}
	
	/*ABL system, Juver, 2017/06/01 */
	if ( pSkinPiece->m_emType == PIECE_RHAND )	
	{
		if ( m_pAttBone[PIECE_LHAND] )
		{
			DxAttBoneLinkContainer::GetInstance().ReleaseData( m_pAttBone[PIECE_LHAND]->GetFileName() );
			SAFE_DELETE( m_pAttBone[PIECE_LHAND] );
		}
		
		if ( m_PartArray[PIECE_LHAND].GetFileName() )
			m_PartArray[PIECE_LHAND].ReSetPart ();
	}

	/*vehicle system, Juver, 2017/08/03 */
	if ( pSkinPiece->m_emType == PIECE_VEHICLE && m_pVehicle )
	{
		DxVehicleDataContainer::GetInstance().ReleaseData( m_pVehicle->GetFileName() );
		SAFE_DELETE( m_pVehicle );
	}

	// Note : 모자에 셋팅이 이상하게 되어서 이렇게 해야 한다.
	if( pSkinPiece->m_emType == PIECE_HAIR )
	{
		if( m_PartArray[PIECE_HEADGEAR].GetFileName() )
		{
			// 일단 정리
			m_PartArray[PIECE_HEADGEAR].ReSetPart ();

			// 데이터 셋팅
			DxSkinPiece* pSkinPieceHG = DxSkinPieceContainer::GetInstance().LoadPiece( m_PartArray[PIECE_HEADGEAR].GetFileName(), pd3dDevice, TRUE );
			if ( !pSkinPieceHG )	goto _RETURN;
			m_PartArray[PIECE_HEADGEAR].SetPart( pSkinPieceHG, pd3dDevice, bCharacterData );
		}
	}

_RETURN:

	//	Note : Piece 를 지정.
	//
	m_PartArray[pSkinPiece->m_emType].SetPart ( pSkinPiece, pd3dDevice, bCharacterData );
	m_PartArray[pSkinPiece->m_emType].SetGrindLevel( nLevel );
	
	/*Item Self Effect, Juver, 2017/06/09 */
	if( strSelfEffect.size() != 0 )	
	{
		DxEffSelfEffectMan::GetInstance().PutEffect ( &m_PartArray[pSkinPiece->m_emType], strSelfEffect.c_str() );
	}

	// Note : 정점 데이터 재 수집
	CollectVertexNUM();

	return S_OK;
}

/*ABL system, Juver, 2017/06/01 */
BOOL DxSkinChar::SetAttBone( DxAttBoneLink* pBoneLink, LPDIRECT3DDEVICEQ pd3dDevice, DWORD dwFlag /*= 0x0*/, int nLevel/*=0*/, bool bCharacterData /*= FALSE */, std::string strSelfEffect /*= ""*/ )
{
	if ( !pBoneLink ) return FALSE;
	if ( !pBoneLink->GetData() ) return FALSE;

	int nPieceType = pBoneLink->m_emPieceType;
	if ( nPieceType > PIECE_SIZE ) return FALSE;

	if ( m_pAttBone[nPieceType] )
	{
		DxAttBoneLinkContainer::GetInstance().ReleaseData( m_pAttBone[nPieceType]->GetFileName() );
		SAFE_DELETE( m_pAttBone[nPieceType] );
	}

	if ( m_PartArray[nPieceType].GetFileName() )
		m_PartArray[nPieceType].ReSetPart ();

	if ( nPieceType == PIECE_RHAND )	
	{
		if ( m_pAttBone[PIECE_LHAND] )
		{
			DxAttBoneLinkContainer::GetInstance().ReleaseData( m_pAttBone[PIECE_LHAND]->GetFileName() );
			SAFE_DELETE( m_pAttBone[PIECE_LHAND] );
		}

		if ( m_PartArray[PIECE_LHAND].GetFileName() )
			m_PartArray[PIECE_LHAND].ReSetPart ();
	}

	/*vehicle system, Juver, 2017/08/03 */
	if ( nPieceType == PIECE_VEHICLE && m_pVehicle )
	{
		DxVehicleDataContainer::GetInstance().ReleaseData( m_pVehicle->GetFileName() );
		SAFE_DELETE( m_pVehicle );
	}

	m_pAttBone[nPieceType] = new DxAttBone;
	m_pAttBone[nPieceType]->SetCharData( pBoneLink, pd3dDevice, nLevel, bCharacterData );

	/*Item Self Effect, Juver, 2017/06/09 */
	if( strSelfEffect.size() != 0 )	
	{
		DxEffSelfEffectMan::GetInstance().PutEffect ( m_pAttBone[ nPieceType ], strSelfEffect.c_str() );
	}

	if ( pBoneLink->m_strMaskPiece.size() != 0 )
	{
		DxSkinPiece* pSkinPiece = DxSkinPieceContainer::GetInstance().LoadPiece( pBoneLink->m_strMaskPiece.c_str(), pd3dDevice, TRUE );
		if ( pSkinPiece )
		{
			if ( m_PartArray[pSkinPiece->m_emType].GetFileName() )
				m_PartArray[pSkinPiece->m_emType].ReSetPart();

			m_PartArray[pSkinPiece->m_emType].SetPart( pSkinPiece, pd3dDevice, bCharacterData );
			m_PartArray[pSkinPiece->m_emType].SetGrindLevel( nLevel );

			/*Item Self Effect, Juver, 2017/06/09 */
			if( strSelfEffect.size() != 0 )	
			{
				DxEffSelfEffectMan::GetInstance().PutEffect ( &m_PartArray[pSkinPiece->m_emType], strSelfEffect.c_str() );
			}

			CollectVertexNUM();
		}
	}

	return TRUE;
}

/*vehicle system, Juver, 2017/08/05 */
BOOL DxSkinChar::SetVehicle( DxVehicleData* pVehicleData, LPDIRECT3DDEVICEQ pd3dDevice, DWORD dwFlag /*= 0x0*/, int nLevel/*=0*/, bool bCharacterData /*= FALSE */, std::string strSelfEffect /*= ""*/ )
{
	if ( !pVehicleData ) return FALSE;

	if ( m_pAttBone[PIECE_VEHICLE] )
	{
		DxAttBoneLinkContainer::GetInstance().ReleaseData( m_pAttBone[PIECE_VEHICLE]->GetFileName() );
		SAFE_DELETE( m_pAttBone[PIECE_VEHICLE] );
	}

	if ( m_PartArray[PIECE_VEHICLE].GetFileName() )
		m_PartArray[PIECE_VEHICLE].ReSetPart ();

	if ( m_pVehicle )
	{
		DxVehicleDataContainer::GetInstance().ReleaseData( m_pVehicle->GetFileName() );
		SAFE_DELETE( m_pVehicle );
	}

	m_pVehicle = new DxVehicle;
	m_pVehicle->SetCharData( pVehicleData, pd3dDevice, nLevel, bCharacterData );

	/*Item Self Effect, Juver, 2017/06/09 */
	if( strSelfEffect.size() != 0 )	
	{
		DxEffSelfEffectMan::GetInstance().PutEffect ( m_pVehicle, strSelfEffect.c_str() );
	}

	return TRUE;
}

HRESULT DxSkinChar::SetPieceGrindLevel ( int nIndex, int nLevel )
{
	if ( PIECE_SIZE >= nIndex )	return E_FAIL;

	m_PartArray[nIndex].SetGrindLevel(nLevel);

	/*ABL system, Juver, 2017/06/01 */
	if ( m_pAttBone[nIndex] )
		m_pAttBone[nIndex]->SetGrindLevel(nLevel);

	/*vehicle system, Juver, 2017/08/04 */
	if ( nIndex == PIECE_VEHICLE && m_pVehicle )
		m_pVehicle->SetGrindLevel(nLevel);

	return S_OK;
}

void DxSkinChar::SetHairColor( WORD wColor )
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

WORD DxSkinChar::GetHairColor()
{
	DWORD dwColor = m_PartArray[PIECE_HAIR].GetHairColor();

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

void DxSkinChar::SetColor1( int nIndex, WORD wColor )
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

			if ( m_pAttBone[i] )
				m_pAttBone[i]->SetColor1( PIECE_SIZE, wColor );
		}
	}
	else
	{
		m_PartArray[nIndex].SetColor1( dwColor );

		if ( m_pAttBone[nIndex] )
			m_pAttBone[nIndex]->SetColor1( PIECE_SIZE, wColor );
	}
}

void DxSkinChar::SetColor2( int nIndex, WORD wColor )
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

			if ( m_pAttBone[i] )
				m_pAttBone[i]->SetColor2( PIECE_SIZE, wColor );
		}
	}
	else
	{
		m_PartArray[nIndex].SetColor2( dwColor );

		if ( m_pAttBone[nIndex] )
			m_pAttBone[nIndex]->SetColor2( PIECE_SIZE, wColor );
	}
}

WORD DxSkinChar::GetColor1( int nIndex )
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

WORD DxSkinChar::GetColor2( int nIndex )
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


HRESULT DxSkinChar::ClearAll ()
{
	m_fScale = 1.0f;
	m_fScaleOrig = 1.0f;
	m_bWorldObj = FALSE;

	m_EffAniPlayer.RemoveAllEff();

	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		//	Note : 만약 이미 읽은 Piece Type 에 지정된 Piece 가 있다면 지워준다.
		//
		if ( m_PartArray[i].GetFileName() )
		{
			m_PartArray[i].ReSetPart ();
		}

		/*ABL system, Juver, 2017/06/01 */

		if ( m_pAttBone[i] )
		{
			DxAttBoneLinkContainer::GetInstance().ReleaseData( m_pAttBone[i]->GetFileName() );
		}

		SAFE_DELETE( m_pAttBone[i] );
	}

	/*vehicle system, Juver, 2017/08/03 */
	if ( m_pVehicle )
	{
		DxVehicleDataContainer::GetInstance().ReleaseData( m_pVehicle->GetFileName() );
		SAFE_DELETE( m_pVehicle );
	}

	DxSkinAniControl::ClearAll ();

	return S_OK;
}

void DxSkinChar::ResetPiece ( int i )
{
	GASSERT(i<PIECE_SIZE);

	//	Note : 이전에 올려진 Part 가 있다면 이를 내려준다.
	//
	if ( m_PartArray[i].GetFileName() )
	{
		m_PartArray[i].ReSetPart ();
	}

	/*ABL system, Juver, 2017/06/01 */
	if ( m_pAttBone[i] )
	{
		DxAttBoneLinkContainer::GetInstance().ReleaseData( m_pAttBone[i]->GetFileName() );
		SAFE_DELETE( m_pAttBone[i] );
	}

	if ( i == PIECE_RHAND )	
	{
		if ( m_pAttBone[PIECE_LHAND] )
		{
			DxAttBoneLinkContainer::GetInstance().ReleaseData( m_pAttBone[PIECE_LHAND]->GetFileName() );
			SAFE_DELETE( m_pAttBone[PIECE_LHAND] );
		}

		if ( m_PartArray[PIECE_LHAND].GetFileName() )
			m_PartArray[PIECE_LHAND].ReSetPart ();
	}


	/*vehicle system, Juver, 2017/08/03 */
	if ( i == PIECE_VEHICLE && m_pVehicle )
	{
		DxVehicleDataContainer::GetInstance().ReleaseData( m_pVehicle->GetFileName() );
		SAFE_DELETE( m_pVehicle );
	}
}

void DxSkinChar::SetPartRend ( EMPIECECHAR emPIECE, BOOL bRender )
{
	m_PartArray[emPIECE].SetRender ( bRender );

	/*ABL system, Juver, 2017/06/01 */
	if ( m_pAttBone[emPIECE] )
		m_pAttBone[emPIECE]->SetPartRend( bRender );

	/*vehicle system, Juver, 2017/08/03 */
	if ( emPIECE == PIECE_VEHICLE && m_pVehicle )
		m_pVehicle->SetPartRend( bRender );
}

void DxSkinChar::SetScaleAdjust( float fAdjust )
{
	//reset the scale from orig
	m_fScale = m_fScaleOrig;

	//adjust scale
	m_fScale = m_fScale * fAdjust;
}

void DxSkinChar::SetCharData ( DxSkinCharData* pCharData, LPDIRECT3DDEVICEQ pd3dDevice, bool bCharacterData /*= FALSE*/ )
{
	StringCchCopy( m_szFileName, FILE_LENGTH, pCharData->GetFileName() );

	m_EffAniPlayer.InitDeviceObjects ( pd3dDevice );

	EMANI_MAINTYPE MType = DxSkinAniControl::GETCURMTYPE();
	EMANI_SUBTYPE SType = DxSkinAniControl::GETCURSTYPE();
	if ( MType == AN_NONE )	MType = AN_GUARD_N;

	DxSkinAniControl::ClearAll ();

	float fCurTime = DxSkinAniControl::GETCURTIME();

	m_fScale = pCharData->m_fScale;
	m_fScaleOrig = pCharData->m_fScale;
	m_vMax = pCharData->m_vMax;
	m_vMin = pCharData->m_vMin;
	m_fHeight = pCharData->m_fHeight;

	m_bWorldObj = pCharData->m_bWorldObj;

	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		/*ABL system, Juver, 2017/06/01 */
		SAFE_DELETE( m_pAttBone[i] );

		/*vehicle system, Juver, 2017/08/03 */
		if ( i == PIECE_VEHICLE )
		{
			SAFE_DELETE( m_pVehicle );

			if ( pCharData->GetVehicle() )
			{
				if ( m_PartArray[i].GetFileName() )
					m_PartArray[i].ReSetPart ();

				m_pVehicle = new DxVehicle;
				m_pVehicle->SetCharData( pCharData->GetVehicle(), pd3dDevice, 0, bCharacterData );
				continue;
			}
		}

		/*ABL system, Juver, 2017/06/01 */
		DxAttBoneLink* pBoneLink = pCharData->GetAttBone(i);
		if ( pBoneLink )
		{
			if ( m_PartArray[i].GetFileName() )
				m_PartArray[i].ReSetPart ();

			m_pAttBone[i] = new DxAttBone;
			m_pAttBone[i]->SetCharData( pBoneLink, pd3dDevice, 0, bCharacterData );

			if ( pBoneLink->m_strMaskPiece.size() )
			{
				DxSkinPiece* pSkinPiece = DxSkinPieceContainer::GetInstance().LoadPiece( pBoneLink->m_strMaskPiece.c_str(), pd3dDevice, TRUE );
				if ( pSkinPiece )
				{
					if ( m_PartArray[pSkinPiece->m_emType].GetFileName() )
						m_PartArray[pSkinPiece->m_emType].ReSetPart ();

					m_PartArray[pSkinPiece->m_emType].SetPart ( pSkinPiece, pd3dDevice, bCharacterData );
				}
			}
		}
		else if ( pCharData->GetPiece(i) )
		{
			m_PartArray[i].SetPart ( pCharData->GetPiece(i), pd3dDevice, bCharacterData );
		}
		else
		{
			m_PartArray[i].ReSetPart ();
		}
	}

	//	Note : 에니메이션 복제.
	//
	m_strSkeleton = pCharData->GetSkeletonName();
	m_pSkeleton = DxBoneCollector::GetInstance().Load ( m_strSkeleton.c_str(), pd3dDevice );

	DxSkinAniControl::VECNODE& vecANI = pCharData->GetAniList ();
	size_t nSize = vecANI.size();
	for ( size_t n=0; n<nSize; ++n )
	{
		DxSkinAniControl::LoadAnimation ( vecANI[n]->pAnimCont->m_szName, pd3dDevice );

		m_EffAniPlayer.CheckCreateEff( vecANI[n]->pAnimCont );	// Aniplayer에 효과가 동작하도록 셋팅
	}

	DxSkinAniControl::ClassifyAnimation ();

	//	Note : 초기 에니메이션 설정.
	//
	DxSkinAniControl::SETCURTIME ( fCurTime );
	DxSkinAniControl::SELECTANI ( MType, SType );

	if ( !GETCURANIMNODE() )
	{
		DxSkinAniControl::SELECTANI ( AN_PLACID, SType );
	}

	if ( !GETCURANIMNODE() )
	{
		CDebugSet::ToLogFile ( _T("ERORR : current animation node null point error [ %s ] [ M %d S %d ]"), pCharData->GetFileName(), MType, SType );

		if( m_vecAnim.size() )		DEFAULTANI();
	}

	// Note : 정점 데이터 재 수집
	CollectVertexNUM();
}

void DxSkinChar::SetRadiusHeight( DxSkinCharData* pCharData )
{
	m_vMin = pCharData->m_vMin;
	m_vMax = pCharData->m_vMax;
	m_fHeight = pCharData->m_fHeight;
}

void DxSkinChar::CollectVertexNUM()
{
	m_dwVertexNUM = 0;

	for( int i=0; i<PIECE_SIZE; ++i )
	{
		m_dwVertexNUM += m_PartArray[i].GetVertexNUM();
	}

	/*ABL system, Juver, 2017/06/01 */
	for( int i=0; i<PIECE_SIZE; ++i )
	{
		if ( m_pAttBone[i] )
			m_dwVertexNUM += m_pAttBone[i]->GetVertexNum();
	}

	/*vehicle system, Juver, 2017/08/04 */
	if ( m_pVehicle )
		m_dwVertexNUM += m_pVehicle->GetVertexNum();
}

void DxSkinChar::AvailableVB_InitVB()
{
}

HRESULT DxSkinChar::FrameMove ( const float fTime, const float fElapsedTime, const BOOL bContinue, const BOOL bFreeze, const BOOL bAttackMode )
{
	int nPrevKeyTime = DxSkinAniControl::GetPrevKeyFrame();

	if ( bFreeze )		DxSkinAniControl::FrameMove ( 0.0f, bContinue );
	else				DxSkinAniControl::FrameMove ( fElapsedTime, bContinue );

	DxSkinAniControl::SetPrevKeyFrame();

	m_sSkinEffData.Reset();		// Data Reset.

	for ( int i=0; i<PIECE_REV01; ++i )
	{
		m_PartArray[i].FrameMove ( fTime, fElapsedTime, GETCURMTYPE(), GETCURSTYPE(), GETCURKEYTIME() );
	}

	/*ABL system, Juver, 2017/06/01 */
	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		if ( m_pAttBone[i] )
			m_pAttBone[i]->FrameMove( fTime, fElapsedTime, bContinue, bFreeze, bAttackMode );
	}

	/*vehicle system, Juver, 2017/08/04 */
	if ( m_pVehicle )
		m_pVehicle->FrameMove( fTime, fElapsedTime, bContinue, bFreeze, bAttackMode );

	m_PartArray[PIECE_RHAND].SetAttackMode ( bAttackMode );
	m_PartArray[PIECE_LHAND].SetAttackMode ( bAttackMode );

	m_EffAniPlayer.FrameMove ( DxSkinAniControl::GETCURANIM(), fTime, fElapsedTime, m_sSkinEffData );
	m_EffAniPlayer.CheckCreateEff ( DxSkinAniControl::GETCURANIM(), nPrevKeyTime, DxSkinAniControl::GETCURKEYTIME() );

	// Note : Edge Color 디폴트 셋팅.
	m_EffKeepPlayer.FrameMove( fElapsedTime, m_sSkinEffData );

	return S_OK;
}

/*vehicle system, Juver, 2017/08/06 */
HRESULT DxSkinChar::RenderVehicle ( const LPDIRECT3DDEVICEQ pd3dDevice, const D3DXMATRIX &matRot, const BOOL bRide, const BOOL bShadow, const BOOL bEff, const BOOL bReverse )
{
	if ( !m_pVehicle )
	{
		Render( pd3dDevice, matRot, bShadow, bEff, bReverse );
		return S_OK;
	}

	float fOrigScale = m_fScale;
	m_fScale = 1.0f;

	m_pVehicle->SetScale( fOrigScale );
	m_pVehicle->Render( pd3dDevice, matRot, bShadow, bEff, bReverse );

	DxBoneTrans *pBoneCur = m_pVehicle->GetSkeleton()->FindBone ( m_pVehicle->m_strCharFront.c_str() );
	D3DXVECTOR3 vLocalTrans = m_pVehicle->m_affCharFront.m_pAffineParts->vTrans;
	D3DXVECTOR3 vLocalRotate = m_pVehicle->m_affCharFront.m_pAffineParts->vRotate;
	D3DXVECTOR3	vLocalScale = m_pVehicle->m_affCharFront.m_pAffineParts->vScale;

	if ( bRide )
	{
		pBoneCur = m_pVehicle->GetSkeleton()->FindBone ( m_pVehicle->m_strCharBack.c_str() );
		vLocalTrans = m_pVehicle->m_affCharBack.m_pAffineParts->vTrans;
		vLocalRotate = m_pVehicle->m_affCharBack.m_pAffineParts->vRotate;
		vLocalScale = m_pVehicle->m_affCharBack.m_pAffineParts->vScale;
	}

	if ( !pBoneCur )	return E_FAIL;

	vLocalTrans.x -=0.5f;

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

	Render( pd3dDevice, matCombine3, bShadow, bEff, bReverse );

	m_fScale = fOrigScale;

	return S_OK;
}

HRESULT DxSkinChar::Render ( const LPDIRECT3DDEVICEQ pd3dDevice, const D3DXMATRIX &matRot, const BOOL bShadow, const BOOL bEff, const BOOL bReverse )
{
	D3DXMATRIXA16	matLocalRot, matScale;
	D3DXMatrixIdentity( &matScale );
	matScale._11 = m_fScale;
	matScale._22 = m_fScale;
	matScale._33 = m_fScale;
	D3DXMatrixMultiply( &matLocalRot, &matScale, &matRot );

	//	Note : Skin Animation.
	DxSkinAniControl::Render ( matLocalRot );

	g_dwLOD = 0;
#ifdef USE_SKINMESH_LOD
	// Note : 그림자일 경우는 저급으로 뿌린다.
	if( bShadow )
	{
		g_dwLOD = 1;
	}
	else if( g_dwHIGHDRAW_NUM > 10 )
	{
		// Note : 한계가 넘었을 경우 최소한 보장해 줘야 하는 애들을 위함
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
		if( fLength > fLengthSRC )	// 거리가 (카메라거리+30.f) 이상일 경우 LOD 레벨을 바꾼다.
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
		if( fLength > fLengthSRC )	// 거리가 (카메라거리+150.f) 이상일 경우 LOD 레벨을 바꾼다.
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
    
	// 피스 순서를 거꾸로 그린다. ( 펫 전용 )
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


	// Note : 조명의 변화는 받지 않는다.
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

			/*ABL system, Juver, 2017/06/01 */
			i = nStart;
			while ( i != nEnd )
			{
				if ( m_pAttBone[i] )
					m_pAttBone[i]->Render( pd3dDevice, m_pSkeleton, bShadow, bEff, bReverse );

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


		//	Note : Glow 
		for ( int i=0; i<PIECE_REV01; ++i )
		{
			m_PartArray[i].RenderGlow( pd3dDevice, m_pSkeleton );
		}

		//	Note : Single Eff
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
		
		/*ABL system, Juver, 2017/06/01 */
		i = nStart;
		while ( i != nEnd )
		{
			if ( m_pAttBone[i] )
				m_pAttBone[i]->Render( pd3dDevice, m_pSkeleton, bShadow, bEff, bReverse );

			i += nAdd;
		}

		for ( int i=0; i<PIECE_REV01; ++i )
		{
			m_PartArray[i].RenderGhosting ( pd3dDevice, this, matLocalRot );
		}
		m_EffAniPlayer.Render( pd3dDevice, this, matLocalRot );
		m_EffKeepPlayer.Render( pd3dDevice );

		pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, dwNormalizeNormals );
	}

	//	Note : Glow 
	for ( int i=0; i<PIECE_REV01; ++i )
	{
		m_PartArray[i].RenderGlow( pd3dDevice, m_pSkeleton );
	}

	//	Note : Single Eff
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

HRESULT DxSkinChar::RenderSKIN( const LPDIRECT3DDEVICEQ pd3dDevice, const D3DXMATRIX& matLocalRot )
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

		/*ABL system, Juver, 2017/06/01 */
		for ( int i=0; i<PIECE_REV01; ++i )
		{
			if ( m_pAttBone[i] )
				m_pAttBone[i]->RenderSKIN( pd3dDevice, matLocalRot );
		}

		for ( int i=0; i<PIECE_REV01; ++i )
		{
			m_PartArray[i].RenderGhosting ( pd3dDevice, this, matLocalRot );
		}
		m_EffAniPlayer.Render( pd3dDevice, this, matLocalRot );
		m_EffKeepPlayer.Render( pd3dDevice );

		pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, dwNormalizeNormals );
	}

	//	Note : Glow 
	for ( int i=0; i<PIECE_REV01; ++i )
	{
		m_PartArray[i].RenderGlow( pd3dDevice, m_pSkeleton );
	}

	//	Note : Single Eff
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

HRESULT DxSkinChar::RenderShadow( const LPDIRECT3DDEVICEQ pd3dDevice, const D3DXMATRIX &matRot, const BOOL bDay )
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

	/*ABL system, Juver, 2017/06/01 */
	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		if ( m_pAttBone[i] )
			m_pAttBone[i]->RenderShadow( pd3dDevice, matRot, bDay );
	}

	/*vehicle system, Juver, 2017/08/04 */
	if ( m_pVehicle )
		m_pVehicle->RenderShadow( pd3dDevice, matRot, bDay );

	pd3dDevice->SetRenderState( D3DRS_FOGENABLE, dwFogEnable );

	return S_OK;
}

HRESULT DxSkinChar::RenderReflect( const LPDIRECT3DDEVICEQ pd3dDevice, const D3DXMATRIX &matRot )
{
	D3DXMATRIX	matLocalRot, matScale;
	D3DXMatrixIdentity( &matScale );
	matScale._11 = m_fScale;
	matScale._22 = m_fScale;
	matScale._33 = m_fScale;
	D3DXMatrixMultiply( &matLocalRot, &matScale, &matRot );

	//	Note : Skin Animation.
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

	/*ABL system, Juver, 2017/06/01 */
	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		if ( m_pAttBone[i] )
			m_pAttBone[i]->RenderReflect( pd3dDevice, matLocalRot );
	}

	/*vehicle system, Juver, 2017/08/04 */
	if ( m_pVehicle )
		m_pVehicle->RenderReflect( pd3dDevice, matLocalRot );

	pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, dwNormalizeNormals );
	pd3dDevice->SetRenderState( D3DRS_FOGENABLE, dwFogEnable );

	return S_OK;
}

void DxSkinChar::RenderBone( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if( !m_pSkeleton )	return;

	pd3dDevice->SetRenderState( D3DRS_ZENABLE,		FALSE );
	pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,	FALSE );

	m_pSkeleton->EditLineSphere( pd3dDevice );

	pd3dDevice->SetRenderState( D3DRS_ZENABLE,		TRUE );
	pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,	TRUE );

	// Note : 본을 클릭하기 위함.
    if( DxInputDevice::GetInstance().GetMouseState(DXMOUSE_LEFT)&DXKEY_UP )
	{
		// Note : 픽킹을 하여 체크 된 Bone Name을 얻는다.
		//			현재 픽킹 범위를 넓혀 놓았다.
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

HRESULT DxSkinChar::InitDeviceObjects( LPDIRECT3DDEVICEQ pd3dDevice )
{
	// Note : m_PartArray, m_EffAniPlayer 은 다른 경로에서 초기화를 해 준다.

	return S_OK;
}

HRESULT DxSkinChar::RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	for ( int i=0; i<PIECE_REV01; ++i )
	{
		m_PartArray[i].RestoreDeviceObjects ( pd3dDevice );
	}

	/*ABL system, Juver, 2017/06/01 */
	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		if ( m_pAttBone[i] )
			m_pAttBone[i]->RestoreDeviceObjects ( pd3dDevice );
	}

	/*vehicle system, Juver, 2017/08/04 */
	if ( m_pVehicle )
		m_pVehicle->RestoreDeviceObjects( pd3dDevice );

	m_EffAniPlayer.RestoreDeviceObjects ();

	return S_OK;
}

HRESULT DxSkinChar::InvalidateDeviceObjects ()
{
	for ( int i=0; i<PIECE_REV01; ++i )
	{
		m_PartArray[i].InvalidateDeviceObjects ();
	}

	/*ABL system, Juver, 2017/06/01 */
	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		if ( m_pAttBone[i] )
			m_pAttBone[i]->InvalidateDeviceObjects ();
	}

	/*vehicle system, Juver, 2017/08/04 */
	if ( m_pVehicle )
		m_pVehicle->InvalidateDeviceObjects();

	m_EffAniPlayer.InvalidateDeviceObjects ();

	return S_OK;
}

HRESULT DxSkinChar::DeleteDeviceObjects ()
{
	for ( int i=0; i<PIECE_REV01; ++i )
	{
		m_PartArray[i].DeleteDeviceObjects ();
	}

	/*ABL system, Juver, 2017/06/01 */
	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		if ( m_pAttBone[i] )
			m_pAttBone[i]->DeleteDeviceObjects ();
	}

	/*vehicle system, Juver, 2017/08/04 */
	if ( m_pVehicle )
		m_pVehicle->DeleteDeviceObjects();

	m_EffAniPlayer.DeleteDeviceObjects ();

	return S_OK;
}

//------------------------------------------------------------
//				E	f	f		K	e	e	p
void DxSkinChar::CreateEff( DxEffKeepDataMain* pEff )
{
	m_EffKeepPlayer.CreateEff( pEff );
}

void DxSkinChar::DeleteEff( DxEffKeepDataMain* pEff )
{
	
}

void DxSkinChar::Reset()
{
	m_EffKeepPlayer.Reset();
}
