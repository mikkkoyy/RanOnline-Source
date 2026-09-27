
// EditorSkinPieceView.cpp : implementation of the CEditorSkinPieceView class
//

#include "stdafx.h"
#include "EditorSkinPiece.h"

#include "EditorSkinPieceDoc.h"
#include "EditorSkinPieceView.h"

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

void CEditorSkinPieceView::PostNcDestroy()
{
	CD3DApplication::Cleanup3DEnvironment();

	::SetCursor ( NULL );
	::DestroyCursor ( m_hCursorDefault );

	__super::PostNcDestroy();
}

void CEditorSkinPieceView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CEditorSkinPieceApp *pApp = (CEditorSkinPieceApp*) AfxGetApp();
	pApp->SetActive ( bActivate );
	CEditorSkinPieceView::SetActive ( bActivate );

	__super::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CEditorSkinPieceView::OnSize(UINT nType, int cx, int cy)
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

void CEditorSkinPieceView::SetActive ( BOOL bActive )
{
	DxInputDevice::GetInstance().OnActivate ( bActive );
}

BOOL CEditorSkinPieceView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
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


void CEditorSkinPieceView::OnInitialUpdate()
{
	CEditorSkinPieceApp *pApp = (CEditorSkinPieceApp *) AfxGetApp();

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

HRESULT CEditorSkinPieceView::FrameMove3DEnvironment ()
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

HRESULT CEditorSkinPieceView::Render3DEnvironment ()
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

HRESULT CEditorSkinPieceView::OneTimeSceneInit()
{
	HRESULT hr = S_OK;

	DxResponseMan::GetInstance().OneTimeSceneInit ( m_szAppPath, this, RANPARAM::strFontType, RANPARAM::dwLangSet, RANPARAM::strGDIFont );
	GMTOOL::Create( m_szAppPath );
	CCursor::GetInstance().SetGameCursor ( RANPARAM::bGameCursor );

	return S_OK;
}

HRESULT CEditorSkinPieceView::ConfirmDevice ( D3DCAPSQ* pCaps, DWORD dwBehavior, D3DFORMAT Format )
{
	if ( dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING )
		return E_FAIL;

	return S_OK;
}

HRESULT CEditorSkinPieceView::CreateObjects()
{
	DxFontMan::GetInstance().InitDeviceObjects ( m_pd3dDevice );
	CD3DFontPar* pD3dFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );
	CDebugSet::InitDeviceObjects( pD3dFont9 );

	HRESULT hr = S_OK;

	hr = InitDeviceObjects();
	if( FAILED(hr) )	
	{
		CDebugSet::MsgBox( "CEditorSkinPieceView::CreateObjects()" );
		return hr;
	}

	hr = RestoreDeviceObjects();	
	if( FAILED(hr) )	
	{
		CDebugSet::MsgBox( "CEditorSkinPieceView::CreateObjects()" );
		return hr;
	}

	return hr;
}

HRESULT CEditorSkinPieceView::InitDeviceObjects()
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

	if ( m_pPiece )
		m_pPiece->InitDeviceObjects( m_pd3dDevice );

	CCursor::GetInstance().InitDeviceObjects ();
	DxResponseMan::GetInstance().SetRenderState ();

	return S_OK;
}

HRESULT CEditorSkinPieceView::RestoreDeviceObjects()
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

		if ( m_pPiece )
			m_pPiece->RestoreDeviceObjects( m_pd3dDevice );
	}

	DXPARAMSET::INIT ();
	DxGlowMan::GetInstance().SetProjectActiveON();
	DxPostProcess::GetInstance().SetProjectActiveON();

	m_DxEditMRS.OnResetDevice( m_pd3dDevice );

	PROFILE_INIT();

	return S_OK;
}

HRESULT CEditorSkinPieceView::FrameMove()
{
	PROFILE_BEGIN("FrameMove");

	CheckControlfp();
	GLPeriod::GetInstance().UpdateWeather ( m_fElapsedTime );
	DxResponseMan::GetInstance().FrameMove( m_fTime, m_fElapsedTime, m_bDefWin );

	DxEffProjMan::GetInstance().FrameMove( m_fElapsedTime );
	DxViewPort::GetInstance().FrameMove ( m_fTime, m_fElapsedTime );

	if ( m_pPiece )
		m_pPiece->FrameMove( m_fTime, m_fElapsedTime );

	DxInputDevice &dxInputDev = DxInputDevice::GetInstance();
	DWORD dwML = dxInputDev.GetMouseState ( DXMOUSE_LEFT );
	bool bCLICK_LEFT = NULL != (dwML&(DXKEY_DOWNED|DXKEY_UP|DXKEY_DUP));

	D3DXVECTOR3 vTargetPt, vFromPt;
	vFromPt = DxViewPort::GetInstance().GetFromPt ();
	BOOL bOk = DxViewPort::GetInstance().GetMouseTargetPosWnd ( vTargetPt );

	//no collision detect when matrix edit is on
	if ( bOk && bCLICK_LEFT && ( dwML&DXKEY_UP ) && !m_bEditMRS )
	{
		m_strTraceSelect = "";

		//skin collision
		BOOL bSKIN = FALSE;
		D3DXVECTOR3 vPosSkin;
		DWORD dwIDSkin;

		if ( m_pPiece &&  m_pPiece->m_pmcMesh )
		{
			D3DXVECTOR3 vP;
			D3DXVECTOR3 vN;
	
			if ( m_emCOL  == EMCOLLISION_TYPE_CLP1 ){
				bSKIN = m_pPiece->m_pmcMesh->GetClosedPoint( vFromPt, vTargetPt, vPosSkin, dwIDSkin, vP, vN );
			}else if ( m_emCOL == EMCOLLISION_TYPE_CLP2 ){
				bSKIN = m_pPiece->m_pmcMesh->GetClosedPoint2( vFromPt, vTargetPt, vPosSkin, dwIDSkin, vP, vN );
			}else if ( m_emCOL == EMCOLLISION_TYPE_ISCOL ){
				bSKIN = m_pPiece->m_pmcMesh->IsCollision( vFromPt, vTargetPt, vPosSkin, dwIDSkin );
			}
		}

		if ( bSKIN )	
		{
			m_vCOL = vPosSkin;
			m_dwCOL = dwIDSkin;
		}else{
			m_vCOL = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
			m_dwCOL = COL_ERR;
		}

		//bone (skeleton) collision
		BOOL bBONE = FALSE;
		std::string strBONE = "";
		if ( m_pPiece &&  m_pPiece->m_pmcMesh && m_pPiece->m_pSkeleton && m_bRenderBone )
		{
			bBONE = m_pPiece->m_pSkeleton->IsCollision( vFromPt, vTargetPt, strBONE, 0.006f*DxViewPort::GetInstance().GetDistance() );
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

HRESULT CEditorSkinPieceView::Render()
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

		if ( m_pPiece )
		{
			D3DLIGHT9 light;
			D3DUtil_InitLight( light, D3DLIGHT_DIRECTIONAL, 0.0f, 10.0f, -5.0f );
			light.Direction = DxViewPort::GetInstance().GetLookDir();
			m_pd3dDevice->SetLight(0, &light);

			m_pd3dDevice->LightEnable( 0, TRUE );
			m_pd3dDevice->LightEnable( 1, FALSE );
			m_pd3dDevice->LightEnable( 2, FALSE );
			m_pd3dDevice->LightEnable( 3, FALSE );
			m_pd3dDevice->LightEnable( 4, FALSE );
			m_pd3dDevice->LightEnable( 5, FALSE );
			m_pd3dDevice->LightEnable( 6, FALSE );
			m_pd3dDevice->LightEnable( 7, FALSE );

			D3DXVECTOR3 vCharPos( m_matLocal._41, m_matLocal._42, m_matLocal._43 );
			DxLightMan::GetInstance()->SetCharStaticPLight( m_pd3dDevice, vCharPos );
			DxLightMan::GetInstance()->SetCharDynamicPLight( m_pd3dDevice, vCharPos );

			DWORD	dwNormalizeNormals;
			m_pd3dDevice->GetRenderState( D3DRS_NORMALIZENORMALS, &dwNormalizeNormals );
			m_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
			m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_XRGB(100, 100, 100));

			m_pPiece->Render( m_pd3dDevice, m_matLocal, NULL );
			m_pPiece->RenderTEST( m_pd3dDevice, m_matLocal, NULL);

			if ( m_bRenderTrace )
			{
				m_pPiece->DrawTraceVert( m_pd3dDevice );
			}

			if ( m_strTraceSelect.size() > 0 )
			{
				m_pPiece->DrawSelTraceVert( m_pd3dDevice, m_strTraceSelect.c_str() );
			}

			if ( m_vCOL != D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) && m_dwCOL != COL_ERR )
			{
				EDITMESHS::RENDERSPHERE ( m_pd3dDevice, m_vCOL, 0.35f, NULL, 0xff00ff00 );
			}

			if ( m_bRenderBone )
			{
				if ( m_pPiece->m_pSkeleton )
				{
					DWORD dwZFunc;
					m_pd3dDevice->GetRenderState ( D3DRS_ZFUNC,		&dwZFunc );
					m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		D3DCMP_ALWAYS );
					m_pPiece->m_pSkeleton->EditLineSphere( m_pd3dDevice );
					m_pPiece->m_pSkeleton->CheckSphere( m_pd3dDevice, g_strPICK_BONE.c_str() );
					m_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,		dwZFunc );
				}
			}

			m_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, dwNormalizeNormals );
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

		if ( m_bEditMRS )	
			m_DxEditMRS.Render( m_pd3dDevice );

		CCursor::GetInstance().Render ( m_pd3dDevice, DxInputDevice::GetInstance().GetMouseLocateX(), DxInputDevice::GetInstance().GetMouseLocateY() );
		m_pd3dDevice->EndScene();
	}

	if (!m_bRenderDebug )
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

	CString strBONEPOS = "";
	if ( m_pPiece && m_pPiece->m_pSkeleton && g_strPICK_BONE.size() )
	{
		DxBoneTrans* pBoneTran = m_pPiece->m_pSkeleton->FindBone( g_strPICK_BONE.c_str() );
		if( pBoneTran )
		{
			D3DXVECTOR3 vBonePos = D3DXVECTOR3( pBoneTran->matCombined._41, pBoneTran->matCombined._42, pBoneTran->matCombined._43 );
			strBONEPOS.Format( "%4.2f-%4.2f-%4.2f", vBonePos.x, vBonePos.y, vBonePos.z );
		}
	}

	CDebugSet::Render ();
	CDebugSet::ToView( 0, 15, "%d-%4.2f-%4.2f-%4.2f", m_dwCOL, m_vCOL.x, m_vCOL.y, m_vCOL.z );
	CDebugSet::ToView( 0, 16, "%s", g_strPICK_BONE.c_str() );
	CDebugSet::ToView( 0, 17, "%s", strBONEPOS.GetString() );

	PROFILE_END("Render");
	return S_OK;
}

HRESULT CEditorSkinPieceView::InvalidateDeviceObjects()
{
	DxResponseMan::GetInstance().InvalidateDeviceObjects ();

	if ( m_pPiece )
		m_pPiece->InvalidateDeviceObjects();

	m_DxEditMRS.OnLostDevice( m_pd3dDevice );

	return S_OK;
}

HRESULT CEditorSkinPieceView::DeleteDeviceObjects()
{
#ifdef _DEBUG
	_CrtCheckMemory();
#endif //_DEBUG

	if ( m_pPiece )
		m_pPiece->DeleteDeviceObjects();

	DxResponseMan::GetInstance().DeleteDeviceObjects ();
	GLogicData::GetInstance().DeleteDeviceObjects ();
	CCursor::GetInstance().DeleteDeviceObjects ();

	return S_OK;
}

HRESULT CEditorSkinPieceView::FinalCleanup()
{
	CDebugSet::FinalCleanup ();
	GLogicData::GetInstance().ClearData ();
	DxResponseMan::GetInstance().FinalCleanup ();

	return S_OK;
}