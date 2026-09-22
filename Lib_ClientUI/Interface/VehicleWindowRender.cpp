#include "StdAfx.h"
#include "VehicleWindowRender.h"

#include "../DxCommon/DxBackUpRendTarget.h"
#include "GLGaeaClient.h"
#include "DxGlobalStage.h"
#include "DxEffcharData.h"
#include "RANPARAM.h"

#include "DxEffectMan.h"

/*bike color , Juver, 2017/11/16 */
#include "GLBikeColorDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern BOOL g_bPREVIEW_CHAR_RENDER;

CVehicleWindowRender::CVehicleWindowRender () 
	: m_pSkinChar( NULL )
	, m_bReady( false )
	, m_fRenderDelay( 0.0f )
	, m_vTrans( 0.0f,0.0f,0.0f )
	, m_vRot( 0.0f,0.0f,0.0f )
	, m_fScale(1.0f)
	, m_sVehicleID(false)
	, m_bUpdateSuit(false)
{
	D3DXMatrixIdentity ( &m_matTrans );
	m_sVehicleInfo = SVEHICLEITEMINFO();
}

CVehicleWindowRender::~CVehicleWindowRender ()
{
}

void CVehicleWindowRender::CreateSubControl ()
{
}

void CVehicleWindowRender::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

HRESULT CVehicleWindowRender::RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;

	if ( m_pSkinChar )
		m_pSkinChar->RestoreDeviceObjects( pd3dDevice );

	hr = CUIGroup::RestoreDeviceObjects( pd3dDevice );
	return hr;
}

HRESULT CVehicleWindowRender::DeleteDeviceObjects ()
{
	HRESULT hr = S_OK;

	if ( m_pSkinChar )
		m_pSkinChar->DeleteDeviceObjects ();

	hr = CUIGroup::DeleteDeviceObjects ();

	return hr;
}

HRESULT CVehicleWindowRender::Render ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;

	if ( m_pSkinChar )
	{
		g_bPREVIEW_CHAR_RENDER = TRUE;

		if ( m_fRenderDelay < RENDER_DELAY_TIME )	return S_OK;

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

HRESULT CVehicleWindowRender::FrameMove( LPDIRECT3DDEVICEQ pd3dDevice, float fTime, float fElapsedTime )
{
	HRESULT hr = S_OK;

	if ( m_pSkinChar )
	{
		m_pSkinChar->FrameMove ( fTime, fElapsedTime, TRUE, FALSE, FALSE );
	}

	if ( m_bReady ) 
	{
		if( m_fRenderDelay < RENDER_DELAY_TIME )
			m_fRenderDelay += fElapsedTime;
	}

	if ( m_bUpdateSuit )
	{
		UpdateSuit( pd3dDevice );
		UpdateAnimation();
		m_bUpdateSuit = false;
	}

	hr = CUIGroup::FrameMove( pd3dDevice, fTime, fElapsedTime );
	return hr;
}

void CVehicleWindowRender::CreateModel( LPDIRECT3DDEVICEQ pd3dDevice )
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
	UpdateAnimation();
	m_fRenderDelay = 0.0f;
}

void CVehicleWindowRender::RenderModel( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( m_pSkinChar )
	{
		D3DXMATRIX matTrans, matScale, matRot;
		D3DXMatrixScaling( &matScale, m_fScale, m_fScale, m_fScale );
		D3DXMatrixTranslation( &matTrans, m_vTrans.x, m_vTrans.y, m_vTrans.z );
		D3DXMatrixRotationYawPitchRoll ( &matRot, m_vRot.x, m_vRot.y, m_vRot.z );
		m_matTrans = matScale*matRot*matTrans;

		if ( m_pSkinChar->m_pVehicle )
			m_pSkinChar->RenderVehicle( pd3dDevice, m_matTrans );
		else
			m_pSkinChar->Render( pd3dDevice, m_matTrans );
	}
}

void CVehicleWindowRender::UpdateSuit( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( !m_pSkinChar )	return;

	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( !pCharacter )		return;

	EMCHARINDEX emIndex = CharClassToIndex( pCharacter->m_emClass );
	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().FindData ( GLCONST_CHAR::szCharSkin[emIndex] );
	if ( !pSkinChar )	return;

	const GLCONST_CHARCLASS &sCONST = GLCONST_CHAR::cCONSTCLASS[emIndex];

	WORD wFace = pCharacter->m_wFace;
	WORD wHair = pCharacter->m_wHair;
	WORD wHairColor = pCharacter->m_wHairColor;

	if ( sCONST.dwHEADNUM > wFace )
	{
		std::string strHEAD_CPS = sCONST.strHEAD_CPS[wFace];
		PDXCHARPART pCharPart = NULL;
		pCharPart = m_pSkinChar->GetPiece(PIECE_HEAD);
		if ( pCharPart && strcmp(strHEAD_CPS.c_str(),pCharPart->m_szFileName) )
			m_pSkinChar->SetPiece ( strHEAD_CPS.c_str(), pd3dDevice, 0x0, 0, TRUE );

	}

	if ( sCONST.dwHAIRNUM > wHair )
	{
		std::string strHAIR_CPS = sCONST.strHAIR_CPS[wHair];
		PDXCHARPART pCharPart = NULL;
		pCharPart = m_pSkinChar->GetPiece(PIECE_HAIR);
		if ( pCharPart && strcmp(strHAIR_CPS.c_str(),pCharPart->m_szFileName) )
			m_pSkinChar->SetPiece ( strHAIR_CPS.c_str(), pd3dDevice, 0x0, 0, TRUE );
	}

	m_pSkinChar->SetHairColor( wHairColor );

	for ( int i=0; i<SLOT_NSIZE_S_2; i++ )
	{	
		if( !pCharacter->IsCurUseArm( EMSLOT(i) ) ) continue;

		SITEMCUSTOM ItemCustom = pCharacter->m_PutOnItems[i];
		if ( i == SLOT_LHAND  )	ItemCustom = pCharacter->m_PutOnItems[SLOT_RHAND];
		if ( i == SLOT_LHAND_S  )	ItemCustom = pCharacter->m_PutOnItems[SLOT_RHAND_S];

		SNATIVEID nidITEM = ItemCustom.nidDISGUISE;
		if ( nidITEM==SNATIVEID(false) )	nidITEM = ItemCustom.sNativeID;

		if ( i == SLOT_VEHICLE )
			nidITEM = m_sVehicleID;

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
						EMPIECECHAR piece_reset = SLOT_2_UPGRADEPIECE( EMSLOT(i) );
						if ( piece_reset != PIECE_SIZE )	
							m_pSkinChar->ResetPiece( piece_reset );
					}
				}
				else
				{
					/*vehicle system, Juver, 2017/08/06 */
					DxVehicleData*	pVehicleData = DxVehicleDataContainer::GetInstance().LoadData( pItem->GetWearingFileR(emIndex), pd3dDevice, TRUE  );
					if ( pVehicleData )
					{
						m_pSkinChar->SetVehicle ( pVehicleData, pd3dDevice, NULL, ItemCustom.GETGRADE_EFFECT(), TRUE, strSelfEffect.c_str() );

						if ( m_pSkinChar->m_pVehicle )
						{
							for( int j=0; j<VEHICLE_ACCETYPE_SIZE; j++ )
							{
								SITEM* pVehicleItem = GLItemMan::GetInstance().GetItem ( m_sVehicleInfo.m_PutOnItems[j].sNativeID );
								if ( pVehicleItem )
									m_pSkinChar->m_pVehicle->SetPiece( pVehicleItem->GetVehicleWearingFile( pItem->sVehicle.emVehicleSubType ), pd3dDevice, NULL, ItemCustom.GETGRADE_EFFECT(), TRUE );
							}

							/*bike color , Juver, 2017/11/15 */
							if ( pItem->sVehicle.emVehicleType == VEHICLE_TYPE_BIKE )
							{
								for ( int ncolor=0; ncolor<BIKE_COLOR_SLOT_PART_SIZE; ++ncolor )
								{
									EMPIECECHAR piece_color = BIKE_COLOR_SLOT_TO_PIECE( (BIKE_COLOR_SLOT_PART) ncolor );
									BOOL bColor1 = IS_BIKECOLOR_1( (BIKE_COLOR_SLOT_PART) ncolor );
									if ( bColor1 )
										m_pSkinChar->m_pVehicle->SetColor1( piece_color, m_sVehicleInfo.m_wColor[ncolor] );
									else
										m_pSkinChar->m_pVehicle->SetColor2( piece_color, m_sVehicleInfo.m_wColor[ncolor] );
								}
							}

							/* car, cart color, Juver, 2018/02/14 */
							if ( pItem->sVehicle.emVehicleType == VEHICLE_TYPE_CAR || pItem->sVehicle.emVehicleType == VEHICLE_TYPE_CART )
							{
								m_pSkinChar->m_pVehicle->SetColor1( PIECE_SIZE, m_sVehicleInfo.m_wColor[BIKE_COLOR_SLOT_PART_A1] );
								m_pSkinChar->m_pVehicle->SetColor2( PIECE_SIZE, m_sVehicleInfo.m_wColor[BIKE_COLOR_SLOT_PART_A2] );
							}
						}	
					}
					else
						m_pSkinChar->SetPiece ( pItem->GetWearingFileR(emIndex), pd3dDevice, NULL, ItemCustom.GETGRADE_EFFECT(), TRUE, strSelfEffect.c_str() );
				}
			}
		}
	}
}

void CVehicleWindowRender::ClearRender()
{
	if ( m_pSkinChar )
	{
		m_bReady = false;
		m_fRenderDelay = 0.0f;
		SAFE_DELETE ( m_pSkinChar );
	}
}

void CVehicleWindowRender::UpdateAnimation()
{
	SITEM* pItem = GLItemMan::GetInstance().GetItem( m_sVehicleID );
	if ( pItem && m_pSkinChar )
	{
		EMANI_MAINTYPE emMain = AN_PLACID;
		EMANI_SUBTYPE emSub = AN_SUB_NONE;
		BOOL bRun = RANPARAM::bVehicleWindowRun;

		switch ( pItem->sVehicle.emVehicleType )
		{
		case VEHICLE_TYPE_BOARD:
			{
				emMain = bRun? AN_RUN : AN_PLACID;
				emSub = AN_SUB_HOVERBOARD;
			}break;

			/* vehicle car, cart, Juver, 2018/02/13 */
		case VEHICLE_TYPE_BIKE:
		case VEHICLE_TYPE_CAR:
		case VEHICLE_TYPE_CART:
			{
				emMain = CHECK_VEHICLE_MAIN( m_sVehicleID );
				emSub = CHECK_VEHICLE_SUB( bRun? AN_RUN : AN_PLACID );
			}break;
		};

		if ( m_pSkinChar && ( m_pSkinChar->GETCURMTYPE() != emMain || m_pSkinChar->GETCURSTYPE() != emSub ) )
		{
			m_pSkinChar->SELECTANI( emMain, emSub );

			if ( m_pSkinChar->m_pVehicle )	
			{
				m_pSkinChar->m_pVehicle->SELECTANI( bRun? AN_VEHICLE_RUN : AN_VEHICLE_STAY , AN_SUB_NONE );
			}
		}

		EMANI_MAINTYPE emMainVehicle = bRun? AN_VEHICLE_RUN : AN_VEHICLE_STAY;
		EMANI_SUBTYPE emSubVehicle = AN_SUB_NONE;

		if ( m_pSkinChar->m_pVehicle && ( m_pSkinChar->m_pVehicle->GETCURMTYPE() != emMainVehicle || m_pSkinChar->m_pVehicle->GETCURSTYPE() != emSubVehicle ) )	
		{
			m_pSkinChar->m_pVehicle->SELECTANI( emMainVehicle, emSubVehicle );
		}

	};
}

void CVehicleWindowRender::RenderVehicle( SNATIVEID sID, SVEHICLEITEMINFO sInfo )
{
	SITEM* pItem = GLItemMan::GetInstance().GetItem( sID );
	if ( pItem )
	{
		if ( m_sVehicleID != sID )
		{
			ClearRender();
		}

		switch ( pItem->sVehicle.emVehicleType )
		{
		case VEHICLE_TYPE_BOARD:
			{
				m_vTrans = D3DXVECTOR3( 0.0f, -17.0f, 0.0f );
				m_vRot = D3DXVECTOR3( -0.8f, -0.05f, 0.05f );
				m_fScale = 1.0f;
			}break;

		case VEHICLE_TYPE_BIKE:
			{
				m_vTrans = D3DXVECTOR3( 0.0f, -6.0f, 12.0f );
				m_vRot = D3DXVECTOR3( 0.8f, -0.05f, -0.05f );
				m_fScale = 1.0f;
			}break;

			/* vehicle car, cart, Juver, 2018/02/14 */
		case VEHICLE_TYPE_CAR:
		case VEHICLE_TYPE_CART:
			{
				m_vTrans = D3DXVECTOR3( 0.0f, -8.0f, 22.0f );
				m_vRot = D3DXVECTOR3( 0.8f, -0.05f, -0.05f );
				m_fScale = 0.8f;
			}break;
		};


		m_sVehicleInfo = sInfo;
		m_sVehicleID = pItem->sBasicOp.sNativeID;
		m_bReady = true;
	}else{
		ClearRender();
	}
}