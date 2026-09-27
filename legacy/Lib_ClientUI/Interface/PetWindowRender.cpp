#include "StdAfx.h"
#include "PetWindowRender.h"

#include "../DxCommon/DxBackUpRendTarget.h"

#include "GLItemMan.h"
#include "GLogicData.h"
#include "DxEffcharData.h"
#include "RANPARAM.h"
#include "GLGaeaClient.h"
#include "DxEffectMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern BOOL g_bPREVIEW_CHAR_RENDER;

CPetWindowRender::CPetWindowRender () 
	: m_pSkinChar( NULL )
	, m_bReady( false )
	, m_fRenderDelay( 0.0f )
	, m_vRot( 0.0f,0.0f,0.0f )
	, m_wLastCOLOR(0)
	, m_wLastSTYLE(0)
	, m_fScale(1.0f)
	, m_vTrans(0.0f,0.0f,0.0f)
{
	D3DXMatrixIdentity ( &m_matTrans );

	for ( WORD i = 0; i < PET_ACCETYPE_SIZE; ++i )
		m_PutOnItems[i] = SITEMCUSTOM();
}

CPetWindowRender::~CPetWindowRender ()
{
}

void CPetWindowRender::CreateSubControl ()
{
}

void CPetWindowRender::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

HRESULT CPetWindowRender::RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;

	if ( m_pSkinChar )
		m_pSkinChar->RestoreDeviceObjects( pd3dDevice );

	hr = CUIGroup::RestoreDeviceObjects( pd3dDevice );
	return hr;
}

HRESULT CPetWindowRender::DeleteDeviceObjects ()
{
	HRESULT hr = S_OK;

	if ( m_pSkinChar )
		m_pSkinChar->DeleteDeviceObjects ();

	hr = CUIGroup::DeleteDeviceObjects ();

	return hr;
}

HRESULT CPetWindowRender::Render ( LPDIRECT3DDEVICEQ pd3dDevice )
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

HRESULT CPetWindowRender::FrameMove( LPDIRECT3DDEVICEQ pd3dDevice, float fTime, float fElapsedTime )
{
	HRESULT hr = S_OK;

	if ( m_pSkinChar )
	{
		m_pSkinChar->FrameMove ( fTime, fElapsedTime, TRUE, FALSE, FALSE );
	}

	if ( m_bReady ) 
		m_fRenderDelay += fElapsedTime;

	GLPetClient* pPetClient = GLGaeaClient::GetInstance().GetPetClient();
	if ( m_bReady && pPetClient && pPetClient->IsVALID() )
	{
		WORD wColor = pPetClient->m_wColor;
		WORD wStyle = pPetClient->m_wStyle;

		BOOL bDiffColor = wColor != m_wLastCOLOR;
		BOOL bDiffStyle = wStyle != m_wLastSTYLE;
		BOOL bDiffItem = FALSE;

		for ( WORD i=0; i<PET_ACCETYPE_SIZE; ++i )
		{
			if ( m_PutOnItems[i] != pPetClient->m_PutOnItems[i] )
				bDiffItem = TRUE;
		}

		if ( bDiffColor || bDiffStyle || bDiffItem )
		{
			UpdateSuit( pd3dDevice );
		}
	}

	hr = CUIGroup::FrameMove( pd3dDevice, fTime, fElapsedTime );
	return hr;
}

void CPetWindowRender::CreateModel( LPDIRECT3DDEVICEQ pd3dDevice )
{
	GLPetClient* pPetClient = GLGaeaClient::GetInstance().GetPetClient();
	if ( !pPetClient )				return;
	if ( !pPetClient ->IsVALID() )	return;

	SNATIVEID sidPet = pPetClient->m_sPetID;

	if( pPetClient->IsUsePetSkinPack() )
		sidPet = pPetClient->m_sPetSkinPackState.petSkinMobID;

	PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( sidPet );
	if ( !pCrowData )	return;

	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().LoadData ( pCrowData->GetSkinObjFile(), pd3dDevice, true  );
	if ( !pSkinChar )	return;

	SAFE_DELETE(m_pSkinChar);
	m_pSkinChar = new DxSkinChar;
	m_pSkinChar->InitDeviceObjects( pd3dDevice );
	m_pSkinChar->SetCharData ( pSkinChar, pd3dDevice );
	m_pSkinChar->SELECTANI( AN_PLACID, AN_SUB_NONE );
	UpdateSuit( pd3dDevice );
	m_fRenderDelay = 0.0f;
	m_vRot = D3DXVECTOR3( 0.0f,0.0f,0.0f );

	m_fScale = 1.0f;
	m_vTrans = D3DXVECTOR3( 0.0f,0.0f,0.0f );

	switch( pPetClient->m_emTYPE )
	{
	case PET_TYPE_A:
	case PET_TYPE_B:
	case PET_TYPE_G:
	case PET_TYPE_H:
	case PET_TYPE_I:
	case PET_TYPE_N:
		{
			m_fScale = 2.0f;
			m_vTrans.y = -7.0f;
		}break;
		
	case PET_TYPE_C: //white tiger
	case PET_TYPE_D: //bear
		{
			m_fScale = 2.4f;
			m_vTrans.y = -7.0f;
		}break;

	case PET_TYPE_E: //owl
	case PET_TYPE_F: //parrot
		{
			m_fScale = 2.2f;
			m_vTrans.y = -40.0f;
		}break;

	case PET_TYPE_J: //fairy
	case PET_TYPE_K: //fox
		{
			m_fScale = 2.0f;
			m_vTrans.y = -13.0f;
		}break;

	case PET_TYPE_L://leopard
		{
			m_fScale = 1.5f;
			m_vTrans.y = -4.0f;
		}break;

	case PET_TYPE_M: //dinosaur
		{
			m_fScale = 1.5f;
			m_vTrans.y = -7.0f;
		}break;
	};

}

void CPetWindowRender::RenderModel( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( m_pSkinChar )
	{
		D3DXMATRIX matTrans, matScale, matRot;

		D3DXMatrixScaling( &matScale, m_fScale, m_fScale, m_fScale );
		D3DXMatrixTranslation( &matTrans, m_vTrans.x, m_vTrans.y, m_vTrans.z );
		D3DXMatrixRotationYawPitchRoll ( &matRot, m_vRot.x, m_vRot.y, m_vRot.z );
		m_matTrans = matScale*matRot*matTrans;

		m_pSkinChar->Render( pd3dDevice, m_matTrans );
	}
}

void CPetWindowRender::UpdateSuit( LPDIRECT3DDEVICEQ pd3dDevice )
{
	GLPetClient* pPetClient = GLGaeaClient::GetInstance().GetPetClient();
	if ( !pPetClient )						return;
	if ( !pPetClient->IsVALID() )			return;
	if ( pPetClient->IsUsePetSkinPack() )	return;
	if ( !m_pSkinChar )						return;

	PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( pPetClient->m_sPetID );
	if ( !pCrowData ) return;

	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().LoadData( pCrowData->GetSkinObjFile(), pd3dDevice, TRUE );
	if ( !pSkinChar ) return;


	const PETSTYLE &sSTYLE = GLCONST_PET::GetPetStyle ( pPetClient->m_emTYPE );
	if ( sSTYLE.wSTYLENum > pPetClient->m_wStyle )
	{
		std::string strSTYLE_CPS = sSTYLE.strSTYLE_CPS[pPetClient->m_wStyle];

		PDXCHARPART pCharPart = m_pSkinChar->GetPiece(PIECE_HAIR);
		if ( pCharPart && strcmp(strSTYLE_CPS.c_str(),pCharPart->m_szFileName) )
		{
			m_pSkinChar->SetPiece ( strSTYLE_CPS.c_str(), pd3dDevice );
		}
	}

	if ( !pPetClient->m_bEnableColor ) 
		m_pSkinChar->SetHairColor( pPetClient->m_wColor );

	m_wLastCOLOR = pPetClient->m_wColor;
	m_wLastSTYLE = pPetClient->m_wStyle;

	for ( WORD i=0; i<PET_ACCETYPE_SIZE; ++i )
	{
		m_PutOnItems[i] = pPetClient->m_PutOnItems[i];

		SITEM* pSlotItem = GLItemMan::GetInstance().GetItem ( pPetClient->m_PutOnItems[i].sNativeID );
		if ( pSlotItem ) m_pSkinChar->SetPiece ( pSlotItem->GetPetWearingFile (), pd3dDevice, FG_MUSTNEWLOAD );
		else
		{
			EMPIECECHAR emPiece = pPetClient->GetPieceFromSlot ( (PET_ACCESSORY_TYPE)i );
			if ( emPiece != PIECE_SIZE ) 
			{
				PDXSKINPIECE pSkinPiece = pSkinChar->GetPiece(emPiece);
				PDXCHARPART pCharPart = m_pSkinChar->GetPiece(emPiece);

				if ( pSkinPiece )
				{
					if ( pCharPart && strcmp(pSkinPiece->m_szFileName,pCharPart->m_szFileName) )
					{
						m_pSkinChar->SetPiece ( pSkinPiece->m_szFileName, pd3dDevice, 0X0, 0, TRUE );
					}
				}else{
					m_pSkinChar->ResetPiece ( emPiece );
				}
			}
		}
	}
}

void CPetWindowRender::ClearRender()
{
	if ( m_pSkinChar )
	{
		m_bReady = false;
		m_fRenderDelay = 0.0f;
		SAFE_DELETE ( m_pSkinChar );
		m_vRot = D3DXVECTOR3( 0.0f,0.0f,0.0f );
	}

	m_wLastCOLOR = 0;
	m_wLastSTYLE = 0;

	for ( WORD i=0; i<PET_ACCETYPE_SIZE; ++i )
	{
		m_PutOnItems[i] = SITEMCUSTOM();
	}

	m_fScale = 1.0f;
	m_vTrans = D3DXVECTOR3( 0.0f,0.0f,0.0f );
}
