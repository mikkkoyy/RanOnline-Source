#include "StdAfx.h"
#include "CharacterInfoViewPageStatsRender.h"

#include "../DxCommon/DxBackUpRendTarget.h"

#include "GLItemMan.h"
#include "GLogicData.h"
#include "DxEffcharData.h"
#include "RANPARAM.h"

#include "DxEffectMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern BOOL g_bPREVIEW_CHAR_RENDER;

CCharacterInfoViewPageStatsRender::CCharacterInfoViewPageStatsRender () 
	: m_pSkinChar( NULL )
	, m_bReady( false )
	, m_fRenderDelay( 0.0f )
	, m_vRot( 0.0f,0.0f,0.0f )
{
	D3DXMatrixIdentity ( &m_matTrans );
}

CCharacterInfoViewPageStatsRender::~CCharacterInfoViewPageStatsRender ()
{
}

void CCharacterInfoViewPageStatsRender::CreateSubControl ()
{
}

void CCharacterInfoViewPageStatsRender::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

HRESULT CCharacterInfoViewPageStatsRender::RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;

	if ( m_pSkinChar )
		m_pSkinChar->RestoreDeviceObjects( pd3dDevice );

	hr = CUIGroup::RestoreDeviceObjects( pd3dDevice );
	return hr;
}

HRESULT CCharacterInfoViewPageStatsRender::DeleteDeviceObjects ()
{
	HRESULT hr = S_OK;

	if ( m_pSkinChar )
		m_pSkinChar->DeleteDeviceObjects ();

	hr = CUIGroup::DeleteDeviceObjects ();

	return hr;
}

HRESULT CCharacterInfoViewPageStatsRender::Render ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;

	if ( m_pSkinChar )
	{
		g_bPREVIEW_CHAR_RENDER = TRUE;

		if ( m_fRenderDelay < 0.3f )	return S_OK;

		DWORD dwAlphaBlendEnable, dwSrcBlend, dwDestBlend, dwZWriteEnable, dwZEnable, dwZFunc, dwClipPlaneEnable, dwFogEnable, dwAmbient, dwLighting;
		DWORD dwColorOP, dwAlphaOP, dwMinFilter, dwMagFilter, dwMipFilter;
		D3DXMATRIX		matOrgView, matOrgProj, matOrgWorld;
		D3DVIEWPORTQ OrgViewPort;

		pd3dDevice->GetViewport(&OrgViewPort);

		pd3dDevice->GetTransform ( D3DTS_VIEW, &matOrgView );
		pd3dDevice->GetTransform ( D3DTS_PROJECTION, &matOrgProj );
		pd3dDevice->GetTransform ( D3DTS_WORLD, &matOrgWorld );

		DxBackUpRendTarget sBackupTarget ( pd3dDevice );

		pd3dDevice->GetRenderState( D3DRS_ALPHABLENDENABLE,	&dwAlphaBlendEnable );
		pd3dDevice->GetRenderState( D3DRS_SRCBLEND,			&dwSrcBlend );
		pd3dDevice->GetRenderState( D3DRS_DESTBLEND,		&dwDestBlend );
		pd3dDevice->GetRenderState( D3DRS_ZWRITEENABLE,		&dwZWriteEnable );
		pd3dDevice->GetRenderState( D3DRS_ZENABLE,			&dwZEnable );
		pd3dDevice->GetRenderState( D3DRS_ZFUNC,			&dwZFunc);
		pd3dDevice->GetRenderState( D3DRS_CLIPPLANEENABLE,	&dwClipPlaneEnable );
		pd3dDevice->GetRenderState( D3DRS_FOGENABLE,		&dwFogEnable );
		pd3dDevice->GetRenderState( D3DRS_AMBIENT,			&dwAmbient );
		pd3dDevice->GetRenderState( D3DRS_LIGHTING,			&dwLighting );

		pd3dDevice->GetTextureStageState( 0, D3DTSS_COLOROP,	&dwColorOP );
		pd3dDevice->GetTextureStageState( 0, D3DTSS_ALPHAOP,	&dwAlphaOP );

		pd3dDevice->GetSamplerState( 0, D3DSAMP_MINFILTER,	&dwMinFilter );
		pd3dDevice->GetSamplerState( 0, D3DSAMP_MAGFILTER,	&dwMagFilter );
		pd3dDevice->GetSamplerState( 0, D3DSAMP_MIPFILTER,	&dwMipFilter );
		
		pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0xffa08080, 1.0f, 0 ) ;

		pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,		TRUE );
		pd3dDevice->SetRenderState( D3DRS_ZENABLE,			D3DZB_TRUE );
		pd3dDevice->SetRenderState( D3DRS_ZFUNC,			D3DCMP_LESSEQUAL);
		pd3dDevice->SetRenderState( D3DRS_AMBIENT,			D3DCOLOR_XRGB(220,220,220) );
		pd3dDevice->SetRenderState( D3DRS_LIGHTING,			FALSE );

		D3DXMATRIX		matView, matProj, matWorld;

		D3DVIEWPORTQ ViewPort;
		ViewPort.X      = (DWORD)GetGlobalPos().left;
		ViewPort.Y      = (DWORD)GetGlobalPos().top;
		ViewPort.Width  = (DWORD)GetGlobalPos().sizeX;
		ViewPort.Height = (DWORD)GetGlobalPos().sizeY;
		ViewPort.MinZ = 0.0f;
		ViewPort.MaxZ = 1.0f;
		pd3dDevice->SetViewport(&ViewPort);

		D3DXVECTOR3 vFromPt		= D3DXVECTOR3( 0.0f, 0.3f, -35.0f );
		D3DXVECTOR3 vLookatPt	= D3DXVECTOR3( -0.02f, 0.0f, 5.0f );
		D3DXVECTOR3 vUpVec		= D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
		D3DXMatrixLookAtLH ( &matView, &vFromPt, &vLookatPt, &vUpVec );
		pd3dDevice->SetTransform ( D3DTS_VIEW, &matView );

		float fFieldOfView = D3DX_PI/4.0f;
		float fAspectRatio = ((float)ViewPort.Width) / (float)ViewPort.Height;
		D3DXMatrixPerspectiveFovLH ( &matProj, fFieldOfView, fAspectRatio, 1.0f, 100.0f );
		pd3dDevice->SetTransform ( D3DTS_PROJECTION, &matProj );

		D3DXMatrixIdentity ( &matWorld );
		pd3dDevice->SetTransform ( D3DTS_WORLD, &matWorld );

		RenderModel( pd3dDevice );

		sBackupTarget.RestoreTarget ( pd3dDevice );

		pd3dDevice->SetTransform ( D3DTS_VIEW, &matOrgView );
		pd3dDevice->SetTransform ( D3DTS_PROJECTION, &matOrgProj );
		pd3dDevice->SetTransform ( D3DTS_WORLD, &matOrgWorld );

		pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,	dwAlphaBlendEnable );
		pd3dDevice->SetRenderState( D3DRS_SRCBLEND,			dwSrcBlend );
		pd3dDevice->SetRenderState( D3DRS_DESTBLEND,		dwDestBlend );
		pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,		dwZWriteEnable );
		pd3dDevice->SetRenderState( D3DRS_ZENABLE,			dwZEnable );
		pd3dDevice->SetRenderState( D3DRS_ZFUNC,			dwZFunc);
		pd3dDevice->SetRenderState( D3DRS_CLIPPLANEENABLE,	dwClipPlaneEnable );
		pd3dDevice->SetRenderState( D3DRS_FOGENABLE,		dwFogEnable );
		pd3dDevice->SetRenderState( D3DRS_AMBIENT,			dwAmbient );
		pd3dDevice->SetRenderState( D3DRS_LIGHTING,			dwLighting );

		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,	dwColorOP );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,	dwAlphaOP );

		pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER,	dwMinFilter );
		pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER,	dwMagFilter );
		pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER,	dwMipFilter );

		pd3dDevice->SetViewport(&OrgViewPort);	

		g_bPREVIEW_CHAR_RENDER = FALSE;
	}else{
		if ( m_bReady )	CreateModel( pd3dDevice );
	}

	hr = CUIGroup::Render ( pd3dDevice );
	return hr;
}

HRESULT CCharacterInfoViewPageStatsRender::FrameMove( LPDIRECT3DDEVICEQ pd3dDevice, float fTime, float fElapsedTime )
{
	HRESULT hr = S_OK;

	if ( m_pSkinChar )
	{
		m_pSkinChar->FrameMove ( fTime, fElapsedTime, TRUE, FALSE, FALSE );
	}

	if ( m_bReady ) 
		m_fRenderDelay += fElapsedTime;

	hr = CUIGroup::FrameMove( pd3dDevice, fTime, fElapsedTime );
	return hr;
}

void CCharacterInfoViewPageStatsRender::CreateModel( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( m_sInfoChar.dwCharID == GAEAID_NULL )	return;

	EMCHARINDEX emIndex = CharClassToIndex( m_sInfoChar.emClass );
	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().LoadData ( GLCONST_CHAR::szCharSkin[emIndex], pd3dDevice, true  );

	if ( !pSkinChar )	return;

	SAFE_DELETE(m_pSkinChar);
	m_pSkinChar = new DxSkinChar;
	m_pSkinChar->InitDeviceObjects( pd3dDevice );
	m_pSkinChar->SetCharData ( pSkinChar, pd3dDevice );
	m_pSkinChar->SELECTANI( AN_PLACID, AN_SUB_NONE );
	UpdateSuit( pd3dDevice );
	m_fRenderDelay = 0.0f;
	m_vRot = D3DXVECTOR3( 0.0f,0.0f,0.0f );
}

void CCharacterInfoViewPageStatsRender::RenderModel( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( m_pSkinChar )
	{
		float fScale = 1.0f;
		D3DXMATRIX matTrans, matScale, matRot;

		D3DXMatrixScaling( &matScale, fScale, fScale, fScale );
		D3DXMatrixTranslation( &matTrans, 0.0f, -12.0f, 0.0f );
		D3DXMatrixRotationYawPitchRoll ( &matRot, m_vRot.x, m_vRot.y, m_vRot.z );
		m_matTrans = matScale*matRot*matTrans;

		m_pSkinChar->SetScaleAdjust( m_sInfoChar.fScaleRange );
		m_pSkinChar->Render( pd3dDevice, m_matTrans );
	}
}

void CCharacterInfoViewPageStatsRender::UpdateSuit( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( !m_pSkinChar )	return;

	if ( m_sInfoChar.dwCharID == GAEAID_NULL )	return;

	EMCHARINDEX emIndex = CharClassToIndex( m_sInfoChar.emClass );
	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().FindData ( GLCONST_CHAR::szCharSkin[emIndex] );
	if ( !pSkinChar )	return;

	const GLCONST_CHARCLASS &sCONST = GLCONST_CHAR::cCONSTCLASS[emIndex];

	if ( sCONST.dwHEADNUM > m_sInfoChar.wFace )
	{
		std::string strHEAD_CPS = sCONST.strHEAD_CPS[m_sInfoChar.wFace];
		PDXCHARPART pCharPart = NULL;
		pCharPart = m_pSkinChar->GetPiece(PIECE_HEAD);
		if ( pCharPart && strcmp(strHEAD_CPS.c_str(),pCharPart->m_szFileName) )
			m_pSkinChar->SetPiece ( strHEAD_CPS.c_str(), pd3dDevice, 0x0, 0, TRUE );

	}

	if ( sCONST.dwHAIRNUM > m_sInfoChar.wHair )
	{
		std::string strHAIR_CPS = sCONST.strHAIR_CPS[m_sInfoChar.wHair];
		PDXCHARPART pCharPart = NULL;
		pCharPart = m_pSkinChar->GetPiece(PIECE_HAIR);
		if ( pCharPart && strcmp(strHAIR_CPS.c_str(),pCharPart->m_szFileName) )
			m_pSkinChar->SetPiece ( strHAIR_CPS.c_str(), pd3dDevice, 0x0, 0, TRUE );
	}

	m_pSkinChar->SetHairColor( m_sInfoChar.wHairColor );

	for ( int i=0; i<SLOT_NSIZE_S_2; i++ )
	{	
		if( m_sInfoChar.bUseArmSub )
		{
			if( i == SLOT_RHAND || i == SLOT_LHAND )	continue;
		}else{
			if( i == SLOT_RHAND_S || i == SLOT_LHAND_S )	continue;
		}

		SITEMCUSTOM ItemCustom = m_sInfoChar.sPutOnItems[i];
		if ( i == SLOT_LHAND  )	ItemCustom = ItemCustom = m_sInfoChar.sPutOnItems[SLOT_RHAND];
		if ( i == SLOT_LHAND_S  )	ItemCustom = m_sInfoChar.sPutOnItems[SLOT_RHAND_S];
		if ( i == SLOT_VEHICLE )	continue;

		SNATIVEID nidITEM = ItemCustom.nidDISGUISE;
		if ( nidITEM==SNATIVEID(false) )	nidITEM = ItemCustom.sNativeID;

		if ( nidITEM == SNATIVEID(false) )
		{
			PDXSKINPIECE pSkinPiece = NULL;
			PDXCHARPART pCharPart = NULL;

			/*ABL system, Juver, 2017/06/02 */
			DxAttBoneLink* pBoneLink = NULL;
			DxAttBone* pAttBone = NULL;

			EMPIECECHAR emPiece = SLOT_2_PIECE(EMSLOT(i));
			if ( emPiece!=PIECE_SIZE )
			{
				pSkinPiece = pSkinChar->GetPiece(emPiece);
				pCharPart = m_pSkinChar->GetPiece(emPiece);

				/*ABL system, Juver, 2017/06/02 */
				pBoneLink = pSkinChar->GetAttBone(emPiece);
				pAttBone = m_pSkinChar->GetAttBone(emPiece);
			}

			/*ABL system, Juver, 2017/06/02 */
			if ( pBoneLink )
			{
				if ( pAttBone && strcmp( pBoneLink->GetFileName(), pAttBone->GetFileName() ) )
				{
					m_pSkinChar->SetAttBone( pBoneLink, pd3dDevice, 0X0, 0, TRUE );
				}
				else if ( !pAttBone )
				{
					m_pSkinChar->SetAttBone( pBoneLink, pd3dDevice, 0X0, 0, TRUE );
				}
			}
			else if ( pSkinPiece )
			{
				if ( pCharPart && strcmp(pSkinPiece->m_szFileName,pCharPart->m_szFileName) )
					m_pSkinChar->SetPiece ( pSkinPiece->m_szFileName, pd3dDevice, 0X0, 0, TRUE );
			}
			else
			{
				if ( pCharPart )	m_pSkinChar->ResetPiece(emPiece);
			}

			/*upgrade effect, Juver, 2017/09/01 */
			EMPIECECHAR piece_reset = SLOT_2_UPGRADEPIECE( EMSLOT(i) );
			if ( piece_reset != PIECE_SIZE )	
				m_pSkinChar->ResetPiece( piece_reset );
		}else{
			SITEM* pItem = GLItemMan::GetInstance().GetItem ( nidITEM );
			if ( pItem ) 
			{
				/*Item Self Effect, Juver, 2017/06/09 */
				std::string strSelfEffect = pItem->GetSelfBodyEffect();

				/*ABL system, Juver, 2017/06/02 */
				if ( pItem->sBasicOp.emItemType != ITEM_VEHICLE )
				{
					std::string strFileName = pItem->GetWearingFileR( emIndex );
					if ( i == SLOT_LHAND || i == SLOT_LHAND_S )	strFileName = pItem->GetWearingFileL( emIndex );

					DxAttBoneLink* pBoneLink = DxAttBoneLinkContainer::GetInstance().LoadData( strFileName.c_str(), pd3dDevice, TRUE );
					if ( pBoneLink )	m_pSkinChar->SetAttBone( pBoneLink, pd3dDevice, NULL, ItemCustom.GETGRADE_EFFECT(), TRUE, strSelfEffect.c_str() );
					else	m_pSkinChar->SetPiece ( strFileName.c_str(), pd3dDevice, NULL, ItemCustom.GETGRADE_EFFECT(), TRUE, strSelfEffect.c_str() );

					/*upgrade effect, Juver, 2017/09/01 */
					BOOL bResetUpgradeEffect = TRUE;
					if( ItemCustom.GETGRADE_EFFECT() >= 6 && ( i == SLOT_UPPER || i == SLOT_LOWER || i == SLOT_HAND || i == SLOT_FOOT ) )
					{
						EMCHARGENDER emGender = CharClassToGender( m_sInfoChar.emClass );
						std::string strPartName[4] = { "body", "leg", "hand", "foot" };
						std::string strGender[2] = { "w", "m" };

						CString strUpgradeFileName;
						strUpgradeFileName.Format("%s_bs_%s_enchent+%d.cps", strGender[emGender].c_str(), strPartName[i-1].c_str(), ItemCustom.GETGRADE_EFFECT() );
						HRESULT hrSetUpgrade = m_pSkinChar->SetPiece ( strUpgradeFileName.GetString(), pd3dDevice, NULL, ItemCustom.GETGRADE_EFFECT(), TRUE );
						if ( hrSetUpgrade == S_OK )	bResetUpgradeEffect = FALSE;
					}

					if ( bResetUpgradeEffect )
					{
						EMPIECECHAR piece_reset = SLOT_2_UPGRADEPIECE( EMSLOT(i) );
						if ( piece_reset != PIECE_SIZE )	
							m_pSkinChar->ResetPiece( piece_reset );
					}
				}
			}
		}
	}
}

void CCharacterInfoViewPageStatsRender::ClearRender()
{
	if ( m_pSkinChar )
	{
		m_bReady = false;
		m_fRenderDelay = 0.0f;
		SAFE_DELETE ( m_pSkinChar );
		m_vRot = D3DXVECTOR3( 0.0f,0.0f,0.0f );
	}
}
