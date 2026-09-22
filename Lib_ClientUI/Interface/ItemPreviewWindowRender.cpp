#include "StdAfx.h"
#include "ItemPreviewWindowRender.h"

#include "../DxCommon/DxBackUpRendTarget.h"
#include "GLGaeaClient.h"
#include "DxGlobalStage.h"
#include "DxEffcharData.h"
#include "RANPARAM.h"

#include "DxEffectMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern BOOL g_bPREVIEW_CHAR_RENDER;

CItemPreviewWindowRender::CItemPreviewWindowRender () 
	: m_pSkinChar( NULL )
	, m_bReady( false )
	, m_fRenderDelay( 0.0f )
	, m_vRot( 0.0f,0.0f,0.0f )
{
	D3DXMatrixIdentity ( &m_matTrans );
}

CItemPreviewWindowRender::~CItemPreviewWindowRender ()
{
}

void CItemPreviewWindowRender::CreateSubControl ()
{
}

void CItemPreviewWindowRender::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

HRESULT CItemPreviewWindowRender::RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;

	if ( m_pSkinChar )
		m_pSkinChar->RestoreDeviceObjects( pd3dDevice );

	hr = CUIGroup::RestoreDeviceObjects( pd3dDevice );
	return hr;
}

HRESULT CItemPreviewWindowRender::DeleteDeviceObjects ()
{
	HRESULT hr = S_OK;

	if ( m_pSkinChar )
		m_pSkinChar->DeleteDeviceObjects ();

	hr = CUIGroup::DeleteDeviceObjects ();

	return hr;
}

HRESULT CItemPreviewWindowRender::Render ( LPDIRECT3DDEVICEQ pd3dDevice )
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

HRESULT CItemPreviewWindowRender::FrameMove( LPDIRECT3DDEVICEQ pd3dDevice, float fTime, float fElapsedTime )
{
	HRESULT hr = S_OK;

	if ( m_pSkinChar )
	{
		m_pSkinChar->FrameMove ( fTime, fElapsedTime, TRUE, FALSE, RANPARAM::bItemPreviewStance );
	}

	if ( m_bReady ) 
		m_fRenderDelay += fElapsedTime;

	hr = CUIGroup::FrameMove( pd3dDevice, fTime, fElapsedTime );
	return hr;
}

void CItemPreviewWindowRender::CreateModel( LPDIRECT3DDEVICEQ pd3dDevice )
{
	GLCharacter* pChar = GLGaeaClient::GetInstance().GetCharacter();
	if ( !pChar )		return;

	EMCHARINDEX emIndex = CharClassToIndex( DxGlobalStage::GetInstance().GetGameStage()->GetCharJoinData().m_CharData2.m_emClass );
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

void CItemPreviewWindowRender::RenderModel( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( m_pSkinChar )
	{
		float fScale = 1.0f;
		D3DXMATRIX matTrans, matScale, matRot;

		D3DXMatrixScaling( &matScale, fScale, fScale, fScale );
		D3DXMatrixTranslation( &matTrans, 0.0f, -10.0f, 0.0f );
		D3DXMatrixRotationYawPitchRoll ( &matRot, m_vRot.x, m_vRot.y, m_vRot.z );
		m_matTrans = matScale*matRot*matTrans;

		m_pSkinChar->Render( pd3dDevice, m_matTrans );
	}
}

void CItemPreviewWindowRender::UpdateSuit( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( !m_pSkinChar )	return;

	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( !pCharacter )		return;

	EMCHARINDEX emIndex = CharClassToIndex( pCharacter->m_emClass );
	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().FindData ( GLCONST_CHAR::szCharSkin[emIndex] );
	if ( !pSkinChar )	return;

	const GLCONST_CHARCLASS &sCONST = GLCONST_CHAR::cCONSTCLASS[emIndex];

	if ( sCONST.dwHEADNUM > m_sPreviewData.wFace )
	{
		std::string strHEAD_CPS = sCONST.strHEAD_CPS[m_sPreviewData.wFace];
		PDXCHARPART pCharPart = NULL;
		pCharPart = m_pSkinChar->GetPiece(PIECE_HEAD);
		if ( pCharPart && strcmp(strHEAD_CPS.c_str(),pCharPart->m_szFileName) )
			m_pSkinChar->SetPiece ( strHEAD_CPS.c_str(), pd3dDevice, 0x0, 0, TRUE );

	}

	if ( sCONST.dwHAIRNUM > m_sPreviewData.wHair )
	{
		std::string strHAIR_CPS = sCONST.strHAIR_CPS[m_sPreviewData.wHair];
		PDXCHARPART pCharPart = NULL;
		pCharPart = m_pSkinChar->GetPiece(PIECE_HAIR);
		if ( pCharPart && strcmp(strHAIR_CPS.c_str(),pCharPart->m_szFileName) )
			m_pSkinChar->SetPiece ( strHAIR_CPS.c_str(), pd3dDevice, 0x0, 0, TRUE );
	}

	m_pSkinChar->SetHairColor( m_sPreviewData.wHairColor );

	for ( int i=0; i<EMITEM_PREVIEW_SLOT_TOTAL; i++ )
	{
		SITEMCUSTOM ItemCustom = m_sPreviewData.sPutOn[i];

		SITEM* pItem = GLItemMan::GetInstance().GetItem( ItemCustom.sNativeID );
		if ( pItem )
		{
			if ( pItem->sBasicOp.emItemType != ITEM_VEHICLE )
			{
				std::string strRight = pItem->GetWearingFileR( emIndex );
				std::string strLeft = pItem->GetWearingFileL( emIndex );
				std::string strSelfEffect = pItem->GetSelfBodyEffect();

				{
					DxAttBoneLink* pBoneLink = DxAttBoneLinkContainer::GetInstance().LoadData( strRight.c_str(), pd3dDevice, TRUE );
					if ( pBoneLink )	m_pSkinChar->SetAttBone( pBoneLink, pd3dDevice, NULL, ItemCustom.GETGRADE_EFFECT(), TRUE, strSelfEffect.c_str() );
					else	m_pSkinChar->SetPiece ( strRight.c_str(), pd3dDevice, NULL, ItemCustom.GETGRADE_EFFECT(), TRUE, strSelfEffect.c_str() );
				}
				{
					DxAttBoneLink* pBoneLink = DxAttBoneLinkContainer::GetInstance().LoadData( strLeft.c_str(), pd3dDevice, TRUE );
					if ( pBoneLink )	m_pSkinChar->SetAttBone( pBoneLink, pd3dDevice, NULL, ItemCustom.GETGRADE_EFFECT(), TRUE, strSelfEffect.c_str() );
					else	m_pSkinChar->SetPiece ( strLeft.c_str(), pd3dDevice, NULL, ItemCustom.GETGRADE_EFFECT(), TRUE, strSelfEffect.c_str() );
				}
			}

			/*upgrade effect, Juver, 2017/09/01 */
			BOOL bResetUpgradeEffect = TRUE;
			if( ItemCustom.GETGRADE_EFFECT() >= 6 && ( i == EMITEM_PREVIEW_SLOT_UPPER || i == EMITEM_PREVIEW_SLOT_LOWER || i == EMITEM_PREVIEW_SLOT_HAND || i == EMITEM_PREVIEW_SLOT_FOOT ) )
			{
				EMCHARGENDER emGender = CharClassToGender( pCharacter->m_emClass );
				std::string strPartName[4] = { "body", "leg", "hand", "foot" };
				std::string strGender[2] = { "w", "m" };

				CString strUpgradeFileName;
				strUpgradeFileName.Format("%s_bs_%s_enchent+%d.cps", strGender[emGender].c_str(), strPartName[i-1].c_str(), ItemCustom.GETGRADE_EFFECT() );
				HRESULT hrSetUpgrade = m_pSkinChar->SetPiece ( strUpgradeFileName.GetString(), pd3dDevice, NULL, ItemCustom.GETGRADE_EFFECT(), TRUE );
				if ( hrSetUpgrade == S_OK )	bResetUpgradeEffect = FALSE;
			}

			if ( bResetUpgradeEffect )
			{
				EMPIECECHAR piece_reset = SLOT_2_UPGRADEPIECE( EMSLOT(i) ); //i is not emslot but the same enum
				if ( piece_reset != PIECE_SIZE )	
					m_pSkinChar->ResetPiece( piece_reset );
			}
		}
	}
	
	ChangeStance();
}

void CItemPreviewWindowRender::ClearRender()
{
	if ( m_pSkinChar )
	{
		m_bReady = false;
		m_fRenderDelay = 0.0f;
		SAFE_DELETE ( m_pSkinChar );
	}
}

void CItemPreviewWindowRender::ChangeStance()
{
	if ( !m_pSkinChar )	return;

	if ( RANPARAM::bItemPreviewStance )
	{
		SITEM* pRHand = GLItemMan::GetInstance().GetItem( m_sPreviewData.sPutOn[5].sNativeID );
		if ( pRHand )
		{
			EMANI_MAINTYPE emMain = AN_GUARD_N;
			EMANI_SUBTYPE emSub = CHECK_ANISUB( pRHand, NULL );

			if ( m_pSkinChar->GETCURMTYPE() != emMain || m_pSkinChar->GETCURSTYPE() != emSub )
			{
				m_pSkinChar->SELECTANI( emMain, emSub );
			}
		}
	}
	else
	{
		m_pSkinChar->SELECTANI( AN_PLACID, AN_SUB_NONE );
	}
}
