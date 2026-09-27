
// GameViewerView.cpp : implementation of the CGameViewerView class
//

#include "stdafx.h"
#include "GameViewer.h"

#include "GameViewerDoc.h"
#include "GameViewerView.h"

#include "RANPARAM.h"
#include "DxResponseMan.h"
#include "DxGlobalStage.h"
#include "GLLandMan.h"
#include "DxLandMan.h"
#include "GLogic.h"
#include "GLogicData.h"
#include "DxInputDevice.h"
#include "SUBPATH.h"
#include "DxParamSet.h"
#include "DxServerInstance.h"
#include "GLMapAxisInfo.h"
#include "Cursor.h"
#include "DxViewPort.h"
#include "DxFontMan.h"
#include "DxGlowMan.h"
#include "DxLightMan.h"
#include "DxBoneCollector.h"
#include "DxSkinMeshMan.h"
#include "DxPostProcess.h"
#include "DxEffProj.h"
#include "DxEffGroupPlayer.h"
#include "DxEffectMan.h"
#include "DxCubeMap.h"
#include "DxDynamicVB.h"
#include "DXUTmisc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern std::string	g_strPICK_BONE;

void CGameViewerView::PostNcDestroy()
{
	CD3DApplication::Cleanup3DEnvironment();

	::SetCursor ( NULL );
	::DestroyCursor ( m_hCursorDefault );

	__super::PostNcDestroy();
}

void CGameViewerView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CGameViewerApp *pApp = (CGameViewerApp*) AfxGetApp();
	pApp->SetActive ( bActivate );
	CGameViewerView::SetActive ( bActivate );

	__super::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CGameViewerView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	if ( m_bCreated )
	{
		ReSizeWindow(cx,cy);

		if ( m_pd3dDevice )
		{
			InvalidateDeviceObjects();
			RestoreDeviceObjects();
		}
	}
}

void CGameViewerView::SetActive ( BOOL bActive )
{
	DxInputDevice::GetInstance().OnActivate ( bActive );
}

BOOL CGameViewerView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	HCURSOR hCursl = CCursor::GetInstance().GetCurCursor();
	if ( !hCursl )
	{
		if ( !m_hCursorDefault )		m_hCursorDefault = LoadCursor ( NULL, IDC_ARROW );
		hCursl = m_hCursorDefault;
	}

	if ( hCursl )	::SetCursor ( hCursl );

	return TRUE;
}


void CGameViewerView::OnInitialUpdate()
{
	CGameViewerApp *pApp = (CGameViewerApp *) AfxGetApp();

	StringCchCopy ( m_szAppPath, MAX_PATH, pApp->m_szAppPath );

	char szFullPath[MAX_PATH] = {0};
	strcpy ( szFullPath, m_szAppPath );
	strcat ( szFullPath, SUBPATH::GLOGIC_FILE );
	GLOGIC::SetPath ( szFullPath );

	BOOL bOK = RANPARAM::LOAD ( m_szAppPath );
	DXPARAMSET::INIT ();

	if (!bOK )
	{
		MessageBox("Unable to Load PARAM.ini!","Error", MB_OK );
	}
	else
	{
		CD3DApplication::SetScreen ( RANPARAM::dwScrWidth, RANPARAM::dwScrHeight, RANPARAM::emScrFormat, RANPARAM::uScrRefreshHz, TRUE );
		if ( FAILED( CD3DApplication::Create ( m_hWnd, m_hWnd, AfxGetInstanceHandle() ) ) )		return;
		m_bCreated = TRUE;
	}

	__super::OnInitialUpdate();
}

HRESULT CGameViewerView::FrameMove3DEnvironment ()
{
	HRESULT hr=S_OK;

	PROFILE_BLOCKSTART();

	if ( m_pd3dDevice )
	{
		hr = CD3DApplication::FrameMove3DEnvironment ();
		if ( FAILED(hr) )	return hr;
	}

	PROFILE_BLOCKEND();
	PROFILE_DUMPOUTPUT();

	return S_OK;
}

HRESULT CGameViewerView::Render3DEnvironment ()
{
	HRESULT hr=S_OK;

	PROFILE_BLOCKSTART();

	if ( m_pd3dDevice )
	{
		hr = CD3DApplication::Render3DEnvironment ();
		if ( FAILED(hr) )	return hr;
	}

	PROFILE_BLOCKEND();
	PROFILE_DUMPOUTPUT();

	return S_OK;
}

HRESULT CGameViewerView::OneTimeSceneInit()
{
	HRESULT hr = S_OK;

	DxResponseMan::GetInstance().OneTimeSceneInit ( m_szAppPath, this, RANPARAM::strFontType, RANPARAM::dwLangSet, RANPARAM::strGDIFont );
	GMTOOL::Create( m_szAppPath );
	CCursor::GetInstance().SetGameCursor ( RANPARAM::bGameCursor );

	return S_OK;
}

HRESULT CGameViewerView::ConfirmDevice ( D3DCAPSQ* pCaps, DWORD dwBehavior, D3DFORMAT Format )
{
	if ( dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING )
		return E_FAIL;

	return S_OK;
}

HRESULT CGameViewerView::CreateObjects()
{
	DxFontMan::GetInstance().InitDeviceObjects ( m_pd3dDevice );
	CD3DFontPar* pD3dFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );
	CDebugSet::InitDeviceObjects( pD3dFont9 );

	HRESULT hr = S_OK;

	hr = InitDeviceObjects();
	if( FAILED(hr) )	
	{
		CDebugSet::MsgBox( "CRanEditSkinPieceView::CreateObjects()" );
		return hr;
	}

	hr = RestoreDeviceObjects();	
	if( FAILED(hr) )	
	{
		CDebugSet::MsgBox( "CRanEditSkinPieceView::CreateObjects()" );
		return hr;
	}

	return hr;
}

HRESULT CGameViewerView::InitDeviceObjects()
{
	HRESULT hr = S_OK;

	DxViewPort::GetInstance().InitDeviceObjects ( m_pd3dDevice, m_hWnd );
	DxResponseMan::GetInstance().InitDeviceObjects ( m_pd3dDevice );
	GLogicData::GetInstance().InitDeviceObjects ( m_pd3dDevice );

	DXLIGHT sDirectional;
	sDirectional.SetDefault();
	sDirectional.m_Light.Diffuse = D3DXCOLOR(1.1f,1.1f,1.1f,1);
	sDirectional.m_Light.Ambient = D3DXCOLOR(1.1f,1.1f,1.1f,1);
	DxLightMan::SetDefDirect ( sDirectional );

	hr = m_pd3dDevice->GetDeviceCaps ( &m_d3dCaps );
	DxBoneCollector::GetInstance().PreLoad ( _PRELOAD_BONE, m_pd3dDevice );
	DxSkinAniMan::GetInstance().PreLoad ( _PRELOAD_ANI, m_pd3dDevice );
	DxSkinMeshMan::GetInstance().PreLoad( _PRELOAD_SKIN, m_pd3dDevice, FALSE );

	WORD wWidth = 1024; 
	WORD wHeight = 768;

	LPDIRECT3DSURFACEQ pBackBuffer=NULL;
	m_pd3dDevice->GetBackBuffer ( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );

	if ( pBackBuffer )
	{
		pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
		pBackBuffer->Release();

		wWidth = static_cast<WORD> ( m_d3dsdBackBuffer.Width );
		wHeight = static_cast<WORD> ( m_d3dsdBackBuffer.Height );
	}

	DxViewPort::GetInstance().SetCameraType ((CAMERA_TYPE)2);

	D3DXVECTOR3 vFromPt		= D3DXVECTOR3( 0.0f, 70.0f, -70.0f );
	D3DXVECTOR3 vLookatPt	= D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vUpVec		= D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	DxViewPort::GetInstance().SetViewTrans ( vFromPt, vLookatPt, vUpVec );
	DxViewPort::GetInstance().MoveVelocityReset();
	DxViewPort::GetInstance().CameraJump ( D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) );

	if ( m_pSkinChar )	
		m_pSkinChar->InitDeviceObjects( m_pd3dDevice );

	/*ABL system, Juver, 2017/06/02 */
	if ( m_pAttBone )	m_pAttBone->InitDeviceObjects( m_pd3dDevice );
	if ( m_pAttBoneRender )	m_pAttBoneRender->InitDeviceObjects( m_pd3dDevice );
	if ( m_pMaskPiece )m_pMaskPiece->InitDeviceObjects( m_pd3dDevice );

	if ( m_pEffSingle )	
		m_pEffSingle->InitDeviceObjects( m_pd3dDevice );

	if ( m_pPIE )
		m_pPIE->OnCreateDevice( m_pd3dDevice );

	/*vehicle system, Juver, 2017/08/06 */
	if ( m_pVehicle )
		m_pVehicle->InitDeviceObjects( m_pd3dDevice );

	CCursor::GetInstance().InitDeviceObjects ();
	DxResponseMan::GetInstance().SetRenderState ();

	return S_OK;
}

HRESULT CGameViewerView::RestoreDeviceObjects()
{
	HRESULT hr=S_OK;

	DxResponseMan::GetInstance().RestoreDeviceObjects ();

	if ( m_pd3dDevice )
	{
		if ( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_FOGVERTEX )
		{
			m_pd3dDevice->SetRenderState ( D3DRS_FOGENABLE,		TRUE );
		}

		float fFogStart=720.0f, fFogEnd=790.0f, fFongDensity=0.0f;

		m_pd3dDevice->SetRenderState ( D3DRS_FOGSTART,		*((DWORD *)(&fFogStart)) );
		m_pd3dDevice->SetRenderState ( D3DRS_FOGEND,		*((DWORD *)(&fFogEnd)) );
		m_pd3dDevice->SetRenderState ( D3DRS_FOGDENSITY,	*((DWORD *)(&fFongDensity)) );

		m_pd3dDevice->SetRenderState( D3DRS_FOGVERTEXMODE,	D3DFOG_LINEAR );
		m_pd3dDevice->SetRenderState ( D3DRS_FOGTABLEMODE,	D3DFOG_NONE );

		if ( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_FOGRANGE )
		{
			m_pd3dDevice->SetRenderState ( D3DRS_RANGEFOGENABLE,	TRUE );
		}

		HRESULT hr;
		LPDIRECT3DSURFACEQ pBackBuffer;
		m_pd3dDevice->GetBackBuffer ( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );

		pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
		pBackBuffer->Release();

		DxViewPort::GetInstance().SetProjection ( D3DX_PI/4, (float)m_d3dsdBackBuffer.Width, 
			(float)m_d3dsdBackBuffer.Height, 1.0f, 1500000.0f );

		hr = DxViewPort::GetInstance().SetViewPort ();
		if ( FAILED(hr) )	return hr;

		if ( m_pSkinChar )	
			m_pSkinChar->RestoreDeviceObjects( m_pd3dDevice );

		/*ABL system, Juver, 2017/06/02 */
		if ( m_pAttBone )	m_pAttBone->RestoreDeviceObjects( m_pd3dDevice );
		if ( m_pAttBoneRender )	m_pAttBoneRender->RestoreDeviceObjects( m_pd3dDevice );
		if ( m_pMaskPiece )	m_pMaskPiece->RestoreDeviceObjects( m_pd3dDevice );

		if ( m_pEffSingle )	
			m_pEffSingle->RestoreDeviceObjects( m_pd3dDevice );

		if ( m_pPIE )
			m_pPIE->OnResetDevice( m_pd3dDevice );

		/*vehicle system, Juver, 2017/08/06 */
		if ( m_pVehicle )
			m_pVehicle->RestoreDeviceObjects( m_pd3dDevice );
	}

	DXPARAMSET::INIT ();
	DxGlowMan::GetInstance().SetProjectActiveON();
	DxPostProcess::GetInstance().SetProjectActiveON();

	PROFILE_INIT();

	return S_OK;
}

HRESULT CGameViewerView::FrameMove()
{
	PROFILE_BEGIN("FrameMove");

	CheckControlfp();
	GLPeriod::GetInstance().UpdateWeather ( m_fElapsedTime );
	DxResponseMan::GetInstance().FrameMove( m_fTime, m_fElapsedTime, m_bDefWin );

	DxEffProjMan::GetInstance().FrameMove( m_fElapsedTime );
	DxViewPort::GetInstance().FrameMove ( m_fTime, m_fElapsedTime );

	if ( m_pSkinChar )	
	{
		m_pSkinChar->FrameMove( m_fTime, m_fElapsedTime, TRUE, FALSE, m_bAniAttack );
		m_pSkinChar->SetPosition( D3DXVECTOR3( 0.0f, 10.0f, 0.0f ) );

		if ( m_pSkinChar->GETCURANIMNODE() )
		{
			EMANI_MAINTYPE emMAIN = m_pSkinChar->GETCURMTYPE();
			EMANI_SUBTYPE emSUB = m_pSkinChar->GETCURSTYPE(); 

			if ( m_pSkinChar->ISENDANIM() )
			{
				if ( m_bAniLoop )
				{
					DWORD dwAMOUNT = m_pSkinChar->GETANIAMOUNT ( emMAIN, emSUB );
					m_dwANIPLAY = m_dwANIPLAY++;
					if ( m_dwANIPLAY >= dwAMOUNT )		
						m_dwANIPLAY = 0;

					m_pSkinChar->SELECTANI ( emMAIN, emSUB, NULL, m_dwANIPLAY );
				}
				else
				{
					m_pSkinChar->SELECTANI ( AN_GUARD_N, AN_SUB_NONE );

					if ( !m_pSkinChar->GETCURANIMNODE() )
						m_pSkinChar->DEFAULTANI();
				}
			}
		}	
	}

	/*ABL system, Juver, 2017/06/02 */
	if ( m_pAttBone )	
	{
		m_pAttBone->FrameMove( m_fTime, m_fElapsedTime, TRUE, FALSE, m_bAniAttack );
		m_pAttBone->SetPosition( D3DXVECTOR3( 0.0f, 10.0f, 0.0f ) );

		if ( m_pMaskPiece )
			m_pMaskPiece->FrameMove( m_fTime, m_fElapsedTime );

		if ( m_pAttBone->GETCURANIMNODE() )
		{
			EMANI_MAINTYPE emMAIN = m_pAttBone->GETCURMTYPE();
			EMANI_SUBTYPE emSUB = m_pAttBone->GETCURSTYPE(); 

			if ( m_pAttBone->ISENDANIM() )
			{
				if ( m_bAniLoop )
				{
					DWORD dwAMOUNT = m_pAttBone->GETANIAMOUNT ( emMAIN, emSUB );
					m_dwANIPLAY = m_dwANIPLAY++;
					if ( m_dwANIPLAY >= dwAMOUNT )		
						m_dwANIPLAY = 0;

					m_pAttBone->SELECTANI ( emMAIN, emSUB, NULL, m_dwANIPLAY );
				}
				else
				{
					m_pAttBone->SELECTANI ( AN_ABL_STAY, AN_SUB_NONE );

					if ( !m_pAttBone->GETCURANIMNODE() )
						m_pAttBone->DEFAULTANI();
				}
			}
		}	
	}

	if ( m_pAttBoneRender )	
	{
		m_pAttBoneRender->FrameMove( m_fTime, m_fElapsedTime, TRUE, FALSE, m_bAniAttack );
		m_pAttBoneRender->SetPosition( D3DXVECTOR3( 0.0f, 10.0f, 0.0f ) );

		if ( m_pAttBoneRender->GETCURANIMNODE() )
		{
			EMANI_MAINTYPE emMAIN = m_pAttBoneRender->GETCURMTYPE();
			EMANI_SUBTYPE emSUB = m_pAttBoneRender->GETCURSTYPE(); 

			if ( m_pAttBoneRender->ISENDANIM() )
			{
				if ( m_bAniLoop )
				{
					DWORD dwAMOUNT = m_pAttBoneRender->GETANIAMOUNT ( emMAIN, emSUB );
					m_dwANIPLAY = m_dwANIPLAY++;
					if ( m_dwANIPLAY >= dwAMOUNT )		
						m_dwANIPLAY = 0;

					m_pAttBoneRender->SELECTANI ( emMAIN, emSUB, NULL, m_dwANIPLAY );
				}
				else
				{
					m_pAttBoneRender->SELECTANI ( AN_ABL_STAY, AN_SUB_NONE );

					if ( !m_pAttBoneRender->GETCURANIMNODE() )
						m_pAttBoneRender->DEFAULTANI();
				}
			}
		}	
	}

	if ( m_pEffSingle )	
		m_pEffSingle->FrameMove( m_fTime, m_fElapsedTime );

	if ( m_pPIE )
		m_pPIE->FrameMove( m_fTime, m_fElapsedTime );


	/*vehicle system, Juver, 2017/08/06 */
	if ( m_pVehicle )	
	{
		m_pVehicle->FrameMove( m_fTime, m_fElapsedTime, TRUE, FALSE, m_bAniAttack );
		m_pVehicle->SetPosition( D3DXVECTOR3( 0.0f, 10.0f, 0.0f ) );

		if ( m_pVehicle->GETCURANIMNODE() )
		{
			EMANI_MAINTYPE emMAIN = m_pVehicle->GETCURMTYPE();
			EMANI_SUBTYPE emSUB = m_pVehicle->GETCURSTYPE(); 

			if ( m_pVehicle->ISENDANIM() )
			{
				if ( m_bAniLoop )
				{
					DWORD dwAMOUNT = m_pVehicle->GETANIAMOUNT ( emMAIN, emSUB );
					m_dwANIPLAY = m_dwANIPLAY++;
					if ( m_dwANIPLAY >= dwAMOUNT )		
						m_dwANIPLAY = 0;

					m_pVehicle->SELECTANI ( emMAIN, emSUB, NULL, m_dwANIPLAY );
				}
				else
				{
					m_pVehicle->SELECTANI ( AN_VEHICLE_STAY, AN_SUB_NONE );

					if ( !m_pVehicle->GETCURANIMNODE() )
						m_pVehicle->DEFAULTANI();
				}
			}
		}	
	}

	DxInputDevice &dxInputDev = DxInputDevice::GetInstance();
	DWORD dwML = dxInputDev.GetMouseState ( DXMOUSE_LEFT );
	bool bCLICK_LEFT = NULL != (dwML&(DXKEY_DOWNED|DXKEY_UP|DXKEY_DUP));

	D3DXVECTOR3 vTargetPt, vFromPt;
	vFromPt = DxViewPort::GetInstance().GetFromPt ();
	BOOL bOk = DxViewPort::GetInstance().GetMouseTargetPosWnd ( vTargetPt );

	//if ( bOk && bCLICK_LEFT && ( dwML&DXKEY_UP ) && !m_bEditMRS )

	if ( bOk && bCLICK_LEFT && ( dwML&DXKEY_UP ) )
	{
		DxSkeleton* pSkeleton = NULL;

		if ( m_pSkinChar )
			pSkeleton = m_pSkinChar->GetSkeleton();

		/*ABL system, Juver, 2017/06/02 */
		if ( m_pAttBone )
			pSkeleton = m_pAttBone->m_pSkeletonLink;

		if ( m_pAttBoneRender )
			pSkeleton = m_pAttBoneRender->GetSkeleton();

		/*vehicle system, Juver, 2017/08/06 */
		if ( m_pVehicle )
			pSkeleton = m_pVehicle->GetSkeleton();

		BOOL bBONE = FALSE;
		std::string strBONE = "";
		if ( m_bRenderSkeleton && pSkeleton )
		{
			bBONE = pSkeleton->IsCollision( vFromPt, vTargetPt, strBONE, 0.006f*DxViewPort::GetInstance().GetDistance() );
		}

		if ( bBONE ) 
		{
			g_strPICK_BONE = strBONE.c_str();
		}else{
			g_strPICK_BONE = "";
		}
	}

	PROFILE_END("FrameMove");

	return S_OK;
}

HRESULT CGameViewerView::Render()
{
	//render from emulator
	if ( !m_pd3dDevice )	return S_FALSE;

	PROFILE_BEGIN("Render");

	D3DCOLOR colorClear = D3DCOLOR_XRGB(100,100,100);
	HRESULT hr = m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,colorClear, 1.0f, 0L );

	if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
	{
		DxGlowMan::GetInstance().RenderTex ( m_pd3dDevice );
		DxLightMan::GetInstance()->Render ( m_pd3dDevice );
		DXLIGHT &Light = *DxLightMan::GetInstance()->GetDirectLight ();
		m_pd3dDevice->SetVertexShaderConstantF ( VSC_LIGHTDIRECT, (float*)&Light.m_Light.Direction, 1 );
		m_pd3dDevice->SetVertexShaderConstantF ( VSC_LIGHTDIFFUSE, (float*)&Light.m_Light.Diffuse, 1 );
		m_pd3dDevice->SetVertexShaderConstantF ( VSC_LIGHTAMBIENT, (float*)&Light.m_Light.Ambient, 1 );
		D3DXVECTOR3 &vFromPt = DxViewPort::GetInstance().GetFromPt ();
		m_pd3dDevice->SetVertexShaderConstantF ( VSC_CAMERAPOSITION, (float*)&vFromPt, 1 );

		D3DLIGHTQ	pLight;
		D3DXVECTOR4	vPointPos;
		D3DXVECTOR3	vPointDiff;
		for ( int i=0; i<7; i++ )
		{
			if ( DxLightMan::GetInstance()->GetClosedLight(i+1) )
			{
				pLight = DxLightMan::GetInstance()->GetClosedLight(i+1)->m_Light;
				vPointDiff = D3DXVECTOR3 ( pLight.Diffuse.r, pLight.Diffuse.g, pLight.Diffuse.b );
				vPointPos.x = pLight.Position.x;
				vPointPos.y = pLight.Position.y;
				vPointPos.z = pLight.Position.z;
				vPointPos.w = pLight.Range;
			}else{
				vPointPos = D3DXVECTOR4 ( 0.f, 0.f, 0.f, 0.1f );
				vPointDiff = D3DXVECTOR3 ( 0.f, 0.f, 0.f );
			}
			m_pd3dDevice->SetVertexShaderConstantF (i*2+VSC_PLIGHTPOS01, (float*)&vPointPos, 1);
			m_pd3dDevice->SetVertexShaderConstantF (i*2+VSC_PLIGHTDIFF01, (float*)&vPointDiff, 1);
		}

		D3DXMATRIX matView = DxViewPort::GetInstance().GetMatView();
		D3DXMATRIX matProj = DxViewPort::GetInstance().GetMatProj();
		m_pd3dDevice->SetVertexShaderConstantF ( VSC_SKIN_DEFAULT, (float*)&D3DXVECTOR4 (1.f, 0.5f, 0.f, 765.01f), 1 );
		D3DXVECTOR3	vLightVector = DxLightMan::GetInstance()->GetDirectLight()->m_Light.Direction;
		D3DXVec3TransformNormal ( &vLightVector, &vLightVector, &matView );
		D3DXVec3Normalize ( &vLightVector, &vLightVector);
		vLightVector = -vLightVector;
		m_pd3dDevice->SetVertexShaderConstantF ( VSC_LIGHTDIRECT_VIEW, (float*)&vLightVector, 1 );
		D3DXMatrixTranspose( &matView, &matView );
		D3DXMatrixTranspose( &matProj, &matProj );
		D3DXMATRIX matIdentity;
		D3DXMatrixIdentity( &matIdentity );
		m_pd3dDevice->SetTransform( D3DTS_WORLD,  &matIdentity );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,	D3DTOP_MODULATE );	

		if ( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_FOGVERTEX )		
			m_pd3dDevice->SetRenderState ( D3DRS_FOGENABLE, FALSE );

		CLIPVOLUME &CV = DxViewPort::GetInstance().GetClipVolume ();

		if ( m_bRenderPlane )
		{
			D3DXVECTOR3 vCenter ( 0.0f, 0.f, 0.0f );
			D3DXVECTOR3 vSize ( 20.0f, 0.f, 20.0f );
			D3DXVECTOR3 vSnap ( 5.0f, 0, 5.0f );

			DWORD dwZFunc;
			m_pd3dDevice->GetRenderState ( D3DRS_ZFUNC,		&dwZFunc );
			m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		D3DCMP_ALWAYS );
			EDITMESHS::RENDERPLANE ( m_pd3dDevice, vCenter, vSize, vSnap );
			m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		dwZFunc );
		}

		if ( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_FOGVERTEX )		
			m_pd3dDevice->SetRenderState ( D3DRS_FOGENABLE, TRUE );

		if ( m_bRenderAABB )
		{
			D3DXVECTOR3 vMIN, vMAX;

			if ( m_pSkinChar )	m_pSkinChar->GetAABBBox( vMAX, vMIN );

			/*ABL system, Juver, 2017/06/02 */
			if ( m_pAttBone )	m_pAttBone->GetAABBBox( vMAX, vMIN );
			if ( m_pAttBoneRender )	m_pAttBoneRender->GetAABBBox( vMAX, vMIN );

			/*vehicle system, Juver, 2017/08/06 */
			if ( m_pVehicle )	m_pVehicle->GetAABBBox( vMAX, vMIN );

			if ( vMIN != D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) &&
				vMAX != D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) )
			{
				DWORD dwZFunc;
				m_pd3dDevice->GetRenderState ( D3DRS_ZFUNC,		&dwZFunc );
				m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		D3DCMP_ALWAYS );
				EDITMESHS::RENDERAABB( m_pd3dDevice, vMAX, vMIN );
				m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		dwZFunc );
			}
		}

		D3DLIGHT9 light;
		D3DUtil_InitLight( light, D3DLIGHT_DIRECTIONAL, 0.0f, 10.0f, -5.0f );
		light.Direction = DxViewPort::GetInstance().GetLookDir();
		m_pd3dDevice->SetLight(0, &light);

		if ( m_pSkinChar )
		{
			/*vehicle system, Juver, 2017/08/06 */
			if ( m_pSkinChar->m_pVehicle )
				m_pSkinChar->RenderVehicle( m_pd3dDevice, m_matLocal, !m_bVCFFront );
			else
				m_pSkinChar->Render( m_pd3dDevice, m_matLocal );

			if ( m_bRenderSkeleton )
			{
				/*vehicle system, Juver, 2017/08/06 */
				if ( m_pSkinChar->m_pVehicle )
				{
					DxSkeleton* pSkeleton = m_pSkinChar->m_pVehicle->GetSkeleton();
					if ( pSkeleton )
					{
						DWORD dwZFunc;
						m_pd3dDevice->GetRenderState ( D3DRS_ZFUNC,		&dwZFunc );
						m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		D3DCMP_ALWAYS );
						pSkeleton->EditLineSphere( m_pd3dDevice );
						pSkeleton->CheckSphere( m_pd3dDevice, g_strPICK_BONE.c_str() );
						m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		dwZFunc );
					}
				}
				
				DxSkeleton* pSkeleton = m_pSkinChar->GetSkeleton();
				if ( pSkeleton )
				{
					DWORD dwZFunc;
					m_pd3dDevice->GetRenderState ( D3DRS_ZFUNC,		&dwZFunc );
					m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		D3DCMP_ALWAYS );
					pSkeleton->EditLineSphere( m_pd3dDevice );
					pSkeleton->CheckSphere( m_pd3dDevice, g_strPICK_BONE.c_str() );
					m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		dwZFunc );
				}
			}
		}

		/*ABL system, Juver, 2017/06/02 */
		if ( m_pAttBone )
		{
			if ( m_pAttBone->m_pSkeletonLink )
			{
				if ( m_pAttBone->m_pSkeletonLink->pBoneRoot )
				{
					m_pAttBone->m_pSkeletonLink->pBoneRoot->ResetBone();
					m_pAttBone->m_pSkeletonLink->UpdateBones( m_pAttBone->m_pSkeletonLink->pBoneRoot, m_matLocal );
				}

				m_pAttBone->Render( m_pd3dDevice, m_pAttBone->m_pSkeletonLink );

				if ( m_pMaskPiece )
					m_pMaskPiece->Render( m_pd3dDevice, m_matLocal, m_pAttBone->m_pSkeletonLink );
			}

			if ( m_bRenderSkeleton )
			{
				DxSkeleton* pSkeleton = m_pAttBone->GetSkeleton();
				if ( pSkeleton )
				{
					DWORD dwZFunc;
					m_pd3dDevice->GetRenderState ( D3DRS_ZFUNC,		&dwZFunc );
					m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		D3DCMP_ALWAYS );
					pSkeleton->EditLineSphere( m_pd3dDevice );
					pSkeleton->CheckSphere( m_pd3dDevice, g_strPICK_BONE.c_str() );
					m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		dwZFunc );
				}

				DxSkeleton* pSkeletonLink = m_pAttBone->m_pSkeletonLink;
				if ( pSkeletonLink )
				{
					DWORD dwZFunc;
					m_pd3dDevice->GetRenderState ( D3DRS_ZFUNC,		&dwZFunc );
					m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		D3DCMP_ALWAYS );
					pSkeletonLink->EditLineSphere( m_pd3dDevice );
					pSkeletonLink->CheckSphere( m_pd3dDevice, g_strPICK_BONE.c_str() );
					m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		dwZFunc );
				}
			}
		}

		if ( m_pAttBoneRender )
		{
			m_pAttBoneRender->Render( m_pd3dDevice, m_matLocal );

			if ( m_bRenderSkeleton )
			{
				DxSkeleton* pSkeleton = m_pAttBoneRender->GetSkeleton();
				if ( pSkeleton )
				{
					DWORD dwZFunc;
					m_pd3dDevice->GetRenderState ( D3DRS_ZFUNC,		&dwZFunc );
					m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		D3DCMP_ALWAYS );
					pSkeleton->EditLineSphere( m_pd3dDevice );
					pSkeleton->CheckSphere( m_pd3dDevice, g_strPICK_BONE.c_str() );
					m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		dwZFunc );
				}
			}
		}

		if ( m_pEffSingle )
			m_pEffSingle->Render( m_pd3dDevice );


		if ( m_pPIE )
			m_pPIE->Render( m_pd3dDevice, CV, m_matLocal );


		/*vehicle system, Juver, 2017/08/06 */
		if ( m_pVehicle )
		{
			m_pVehicle->Render( m_pd3dDevice, m_matLocal );

			if ( m_bRenderSkeleton )
			{
				DxSkeleton* pSkeleton = m_pVehicle->GetSkeleton();
				if ( pSkeleton )
				{
					DWORD dwZFunc;
					m_pd3dDevice->GetRenderState ( D3DRS_ZFUNC,		&dwZFunc );
					m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		D3DCMP_ALWAYS );
					pSkeleton->EditLineSphere( m_pd3dDevice );
					pSkeleton->CheckSphere( m_pd3dDevice, g_strPICK_BONE.c_str() );
					m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		dwZFunc );
				}
			}
		}

		DxEffGroupPlayer::GetInstance().Render ( m_pd3dDevice );
		OPTMManager::GetInstance().Render( m_pd3dDevice );

		if ( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_FOGVERTEX )		
			m_pd3dDevice->SetRenderState ( D3DRS_FOGENABLE, FALSE );

		DxWeatherMan::GetInstance()->Render ( m_pd3dDevice );
		DxCubeMap::GetInstance().Render ( m_pd3dDevice );
		DxGlowMan::GetInstance().RenderTex ( m_pd3dDevice );
		DxGlowMan::GetInstance().Render ( m_pd3dDevice );
		DxGlowMan::GetInstance().RenderBURN( m_pd3dDevice );
		DxPostProcess::GetInstance().Render( m_pd3dDevice );
		DxEffProjMan::GetInstance().Render( m_pd3dDevice );

		CCursor::GetInstance().Render ( m_pd3dDevice, DxInputDevice::GetInstance().GetMouseLocateX(), DxInputDevice::GetInstance().GetMouseLocateY() );
		m_pd3dDevice->EndScene();
	}

	if ( !m_bRenderDebug )
	{
		PROFILE_END("Render");
		return S_OK;
	}

	D3DCOLOR fontColor        = D3DCOLOR_ARGB(255,255,0,0);
	D3DCOLOR fontWarningColor = D3DCOLOR_ARGB(255,255,0,0);
	TCHAR szMsg[MAX_PATH] = TEXT("");

	CD3DFontPar* pD3dFont = DxFontMan::GetInstance().FindFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );
	if ( !pD3dFont )	return S_OK;

	FLOAT fNextLine = 40.0f; 

	lstrcpy( szMsg, m_strDeviceStats );
	fNextLine -= 20.0f;
	pD3dFont->DrawText( 2, fNextLine, fontColor, szMsg );

	lstrcpy( szMsg, m_strFrameStats );
	fNextLine -= 20.0f;
	pD3dFont->DrawText( 2, fNextLine, fontColor, szMsg );

	CDebugSet::Render ();

	EMANI_MAINTYPE emMAIN = AN_NONE;
	EMANI_SUBTYPE emSUB = AN_SUB_NONE;

	if ( m_pSkinChar )
	{
		emMAIN = m_pSkinChar->GETCURMTYPE();
		emSUB = m_pSkinChar->GETCURSTYPE();

		if ( emMAIN < AN_TYPE_SIZE && emSUB < AN_SUB_00_SIZE )
		{
			CDebugSet::ToView ( 0, 5, COMMENT::ANI_MAINTYPE_CHAR[emMAIN].c_str() );
			CDebugSet::ToView ( 0, 6, COMMENT::ANI_SUBTYPE_WEAPON[emSUB].c_str() );
			CDebugSet::ToView ( 0, 7, COMMENT::ANI_SUBTYPE_SKILL[emSUB].c_str() );
		}
	}

	/*ABL system, Juver, 2017/06/02 */
	if ( m_pAttBone )
	{
		emMAIN = m_pAttBone->GETCURMTYPE();
		emSUB = m_pAttBone->GETCURSTYPE();

		if ( emMAIN < AN_TYPE_SIZE && emSUB < AN_SUB_00_SIZE )
		{
			CDebugSet::ToView ( 0, 5, COMMENT::ANI_MAINTYPE_ABL[emMAIN].c_str() );;
			CDebugSet::ToView ( 0, 6, COMMENT::ANI_SUBTYPE_SKILL[emSUB].c_str() );
		}
	}

	if ( m_pAttBoneRender )
	{
		emMAIN = m_pAttBoneRender->GETCURMTYPE();
		emSUB = m_pAttBoneRender->GETCURSTYPE();

		if ( emMAIN < AN_TYPE_SIZE && emSUB < AN_SUB_00_SIZE )
		{
			CDebugSet::ToView ( 0, 5, COMMENT::ANI_MAINTYPE_ABL[emMAIN].c_str() );;
			CDebugSet::ToView ( 0, 6, COMMENT::ANI_SUBTYPE_SKILL[emSUB].c_str() );
		}
	}

	/*vehicle system, Juver, 2017/08/06 */
	if ( m_pVehicle )
	{
		emMAIN = m_pVehicle->GETCURMTYPE();
		emSUB = m_pVehicle->GETCURSTYPE();

		if ( emMAIN < AN_TYPE_SIZE && emSUB < AN_SUB_00_SIZE )
		{
			CDebugSet::ToView ( 0, 5, COMMENT::ANI_MAINTYPE_VEHICLE[emMAIN].c_str() );;
			CDebugSet::ToView ( 0, 6, COMMENT::ANI_SUBTYPE_SKILL[emSUB].c_str() );
		}
	}

	CDebugSet::ToView ( 0, 8, "AniPlay:%d", m_dwANIPLAY );
	CDebugSet::ToView( 0, 15, "%s", g_strPICK_BONE.c_str() );


	PROFILE_END("Render");
	return S_OK;
}

HRESULT CGameViewerView::InvalidateDeviceObjects()
{
	DxResponseMan::GetInstance().InvalidateDeviceObjects ();

	if ( m_pSkinChar )	
		m_pSkinChar->InvalidateDeviceObjects();

	/*ABL system, Juver, 2017/06/02 */
	if ( m_pAttBone )	m_pAttBone->InvalidateDeviceObjects();
	if ( m_pAttBoneRender )	m_pAttBoneRender->InvalidateDeviceObjects();
	if ( m_pMaskPiece ) m_pMaskPiece->InvalidateDeviceObjects();

	if ( m_pEffSingle )
		m_pEffSingle->InvalidateDeviceObjects();

	if ( m_pPIE )
		m_pPIE->OnLostDevice();

	/*vehicle system, Juver, 2017/08/06 */
	if ( m_pVehicle )
		m_pVehicle->InvalidateDeviceObjects();
	
	return S_OK;
}

HRESULT CGameViewerView::DeleteDeviceObjects()
{
#ifdef _DEBUG
	_CrtCheckMemory();
#endif //_DEBUG

	if ( m_pSkinChar )	
		m_pSkinChar->DeleteDeviceObjects();

	/*ABL system, Juver, 2017/06/02 */
	if ( m_pAttBone )	m_pAttBone->DeleteDeviceObjects();
	if ( m_pAttBoneRender )	m_pAttBoneRender->DeleteDeviceObjects();
	if ( m_pMaskPiece ) m_pMaskPiece->DeleteDeviceObjects();

	if ( m_pEffSingle )	
		m_pEffSingle->DeleteDeviceObjects();

	if ( m_pPIE )
		m_pPIE->OnDestroyDevice();

	/*vehicle system, Juver, 2017/08/06 */
	if ( m_pVehicle )
		m_pVehicle->DeleteDeviceObjects();

	DxResponseMan::GetInstance().DeleteDeviceObjects ();
	GLogicData::GetInstance().DeleteDeviceObjects ();
	CCursor::GetInstance().DeleteDeviceObjects ();

	return S_OK;
}

HRESULT CGameViewerView::FinalCleanup()
{
	CDebugSet::FinalCleanup ();
	GLogicData::GetInstance().ClearData ();
	DxResponseMan::GetInstance().FinalCleanup ();

	if ( m_pEffSingle )	
		m_pEffSingle->FinalCleanup();

	return S_OK;
}