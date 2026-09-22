
// EditorLevelView.cpp : implementation of the CEditorLevelView class
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "MainFrm.h"
#include "EditorLevelDoc.h"
#include "EditorLevelView.h"
#include "SheetWithTab.h"

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
#include "DxFogMan.h"
#include "DxGlowMan.h"
#include "DxLightMan.h"
#include "DxBoneCollector.h"
#include "DxSkinMeshMan.h"
#include "DxPostProcess.h"
#include "DxEffProj.h"
#include "DxEnvironment.h"
#include "DxShadowMap.h"
#include "DxEffGroupPlayer.h"
#include "DxEffectMan.h"
#include "DxCubeMap.h"
#include "DxDynamicVB.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CEditorLevelView::PostNcDestroy()
{
	CD3DApplication::Cleanup3DEnvironment();

	::SetCursor ( NULL );
	::DestroyCursor ( m_hCursorDefault );

	__super::PostNcDestroy();
}

void CEditorLevelView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CEditorLevelApp *pApp = (CEditorLevelApp*) AfxGetApp();
	pApp->SetActive ( bActivate );
	CEditorLevelView::SetActive ( bActivate );

	__super::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CEditorLevelView::OnSize(UINT nType, int cx, int cy)
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

void CEditorLevelView::SetActive ( BOOL bActive )
{
	DxInputDevice::GetInstance().OnActivate ( bActive );
}

BOOL CEditorLevelView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
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

void CEditorLevelView::OnInitialUpdate()
{
	CEditorLevelApp *pApp = (CEditorLevelApp *) AfxGetApp();

	StringCchCopy ( m_szAppPath, MAX_PATH, pApp->m_szAppPath );

	char szFullPath[MAX_PATH] = {0};
	strcpy ( szFullPath, m_szAppPath );
	strcat ( szFullPath, SUBPATH::GLOGIC_FILE );
	GLOGIC::SetPath ( szFullPath );
	//CDebugSet::DebugNotifySet( TRUE );

	BOOL bOK = RANPARAM::LOAD ( m_szAppPath );
	DXPARAMSET::INIT ();

	char szEditorPath[MAX_PATH] = {0};
	strcpy ( szEditorPath, m_szAppPath );
	strcat ( szEditorPath, SUBPATH::OBJ_FILE_EDIT );

	m_DxEditBox.SetPath( szEditorPath );
	m_DxEditMat.SetParam( szEditorPath, GetSafeHwnd() );

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

HRESULT CEditorLevelView::FrameMove3DEnvironment ()
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

HRESULT CEditorLevelView::Render3DEnvironment ()
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

HRESULT CEditorLevelView::OneTimeSceneInit()
{
	HRESULT hr = S_OK;

	DxResponseMan::GetInstance().OneTimeSceneInit ( m_szAppPath, this, RANPARAM::strFontType, RANPARAM::dwLangSet, RANPARAM::strGDIFont );

	GMTOOL::Create( m_szAppPath );
	m_sMap.LoadMapsListFile( "mapslist.mst" );

	char szFullPath[MAX_PATH] = {0};

	StringCchCopy ( szFullPath, MAX_PATH, m_szAppPath );
	StringCchCat ( szFullPath, MAX_PATH, SUBPATH::MAP_FILE );
	GLMapAxisInfo::SetPath( szFullPath );

	CCursor::GetInstance().SetGameCursor ( RANPARAM::bGameCursor );

	for( int nMID = 0; nMID < GLCrowDataMan::MAX_CROW_MID; ++ nMID)
	{
		for( int nSID = 0; nSID < GLCrowDataMan::MAX_CROW_SID; ++ nSID )
		{
			PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( nMID, nSID );
			if( pCROW )
			{
				if ( pCROW->m_sAction.m_dwActFlag&EMCROWACT_BOSS )
					pCROW->m_sAction.m_dwActFlag &= ~EMCROWACT_BOSS; //remove boss

				if ( pCROW->m_sAction.m_dwActFlag&EMCROWACT_AUTODROP )
					pCROW->m_sAction.m_dwActFlag &= ~EMCROWACT_AUTODROP; //remove autodrop
			}
		}
	}

	return S_OK;
}

HRESULT CEditorLevelView::ConfirmDevice ( D3DCAPSQ* pCaps, DWORD dwBehavior, D3DFORMAT Format )
{
	if ( dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING )
		return E_FAIL;

	return S_OK;
}

HRESULT CEditorLevelView::CreateObjects()
{
	DxFontMan::GetInstance().InitDeviceObjects ( m_pd3dDevice );
	CD3DFontPar* pD3dFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );
	CDebugSet::InitDeviceObjects( pD3dFont9 );

	HRESULT hr = S_OK;

	hr = InitDeviceObjects();
    if( FAILED(hr) )	
	{
		CDebugSet::MsgBox( "CEditorLevelView::CreateObjects()" );
		return hr;
	}

	hr = RestoreDeviceObjects();	
    if( FAILED(hr) )	
	{
		CDebugSet::MsgBox( "CEditorLevelView::CreateObjects()" );
		return hr;
	}

	return hr;
}

HRESULT CEditorLevelView::InitDeviceObjects()
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

	CCursor::GetInstance().InitDeviceObjects ();
	DxResponseMan::GetInstance().SetRenderState ();
	DxFogMan::GetInstance().SetFogRange ( EMFR_HIGH );

	if ( m_pGLLandServer )
		m_pGLLandServer->InitDeviceObjects( m_pd3dDevice );

	if ( m_pDxLandMan )
		m_pDxLandMan->InitDeviceObjects( m_pd3dDevice );

	m_DxEditBox.InitDeviceObjects( m_pd3dDevice );
	m_DxEditMat.InitDeviceObjects( m_pd3dDevice );
	
	return S_OK;
}

HRESULT CEditorLevelView::RestoreDeviceObjects()
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

		if ( m_pGLLandServer )	
			m_pGLLandServer->RestoreDeviceObjects( m_pd3dDevice );

		if ( m_pDxLandMan )	
			m_pDxLandMan->RestoreDeviceObjects( m_pd3dDevice );
	}

	DXPARAMSET::INIT ();
	DxGlowMan::GetInstance().SetProjectActiveON();
	DxPostProcess::GetInstance().SetProjectActiveON();

	m_DxEditMat.RestoreDeviceObjects( m_pd3dDevice );
	m_DxEditMRS.OnResetDevice( m_pd3dDevice );


	PROFILE_INIT();

	return S_OK;
}

HRESULT CEditorLevelView::FrameMove()
{
	PROFILE_BEGIN("FrameMove");

	CheckControlfp();
	GLPeriod::GetInstance().UpdateWeather ( m_fElapsedTime );
	DxResponseMan::GetInstance().FrameMove( m_fTime, m_fElapsedTime, m_bDefWin );

	if ( m_pGLLandServer )	
	{
		SGLNODE<GLMobSchedule*>* pCur = m_pGLLandServer->GetMobSchMan()->GetMobSchList()->m_pHead;
		while ( pCur )
		{
			GLMobSchedule* pMobSch = pCur->Data;
			pMobSch->m_fTimer = 0.0f;
			pCur = pCur->pNext;
		}

		m_pGLLandServer->FrameMove( m_fTime, m_fElapsedTime );
	}

	if ( m_pDxLandMan ) 
		m_pDxLandMan->FrameMove( m_fTime, m_fElapsedTime );

	DxEffProjMan::GetInstance().FrameMove( m_fElapsedTime );
	DxViewPort::GetInstance().FrameMove ( m_fTime, m_fElapsedTime );

	m_DxEditBox.FrameMove( m_fTime, m_fElapsedTime );
	m_DxEditMat.FrameMove( m_fTime, m_fElapsedTime );

	DxInputDevice &dxInputDev = DxInputDevice::GetInstance();
	DWORD dwML = dxInputDev.GetMouseState ( DXMOUSE_LEFT );
	bool bCLICK_LEFT = NULL != (dwML&(DXKEY_DOWNED|DXKEY_UP|DXKEY_DUP));

	D3DXVECTOR3 vTargetPt, vFromPt;
	vFromPt = DxViewPort::GetInstance().GetFromPt ();
	BOOL bOk = DxViewPort::GetInstance().GetMouseTargetPosWnd ( vTargetPt );

	if ( bCLICK_LEFT && dwML&DXKEY_UP )
	{
		if ( bOk && m_pDxLandMan && m_pDxLandMan->GetNaviMesh() )
		{
			D3DXVECTOR3 vColPos;
			DWORD dwColID;
			BOOL bCol;
			m_pDxLandMan->GetNaviMesh()->IsCollision( vFromPt, vTargetPt, vColPos, dwColID, bCol );

			if ( bCol )
				m_vPos = vColPos;
		}

		if ( bOk && m_pGLLandServer && m_pGLLandServer->GetNavi() )
		{
			m_pSelectSched = NULL;
			m_pSelectedGate = NULL;

			GLMobSchedule* pSched = m_pGLLandServer->GetMobSchMan()->GetCollisionMobSchedule( vFromPt, vTargetPt );
			if ( pSched )
			{
				m_pSelectSched = pSched;

				CMainFrame	*pMainFrame = (CMainFrame*) AfxGetMainWnd();
				if ( pMainFrame && m_bDETECTCROW )
				{
					CsheetWithTab*  pSheetTab = pMainFrame->m_wndEditor.m_pSheetTab;
					if ( pSheetTab )
						pSheetTab->SelectCrow( pSched->m_szName );
				}
			}

			DxLandGate* pGate = m_pGLLandServer->GetLandGateMan().DetectGate( m_vPos );
			if ( pGate )
			{
				m_pSelectedGate = pGate;

				CMainFrame	*pMainFrame = (CMainFrame*) AfxGetMainWnd();
				if ( pMainFrame && m_bDETECTGATE )
				{
					CsheetWithTab*  pSheetTab = pMainFrame->m_wndEditor.m_pSheetTab;
					if ( pSheetTab )
						pSheetTab->SelectGate( pGate->GetName() );
				}
			}
		}
	}
		
	PROFILE_END("FrameMove");

	return S_OK;
}

HRESULT CEditorLevelView::Render()
{
	//render from emulator

	if ( !m_pd3dDevice )	return S_FALSE;

	PROFILE_BEGIN("Render");

	D3DCOLOR colorClear = D3DCOLOR_XRGB(150,150,150);

	DxFogMan::GetInstance().RenderFogSB ( m_pd3dDevice );
	colorClear = DxFogMan::GetInstance().GetFogColor();

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

		DxWeatherMan::GetInstance()->Render_Prev ( m_pd3dDevice );

		if ( m_pDxLandMan )	
			DxEnvironment::GetInstance().Render ( m_pd3dDevice, NULL, m_pDxLandMan );

		if ( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_FOGVERTEX )		
			m_pd3dDevice->SetRenderState ( D3DRS_FOGENABLE, TRUE );

		DxSkyMan::GetInstance().Render( m_pd3dDevice );

		if ( m_pDxLandMan )
			m_pDxLandMan->Render( m_pd3dDevice, CV ); 

		DxShadowMap::GetInstance().ClearShadow( m_pd3dDevice );

		if ( m_pGLLandServer )
		{
			if ( m_bSettingRenderCrowModel )	m_pGLLandServer->Render( m_pd3dDevice, CV ); 

			if ( m_bSettingRenderCrowSched && m_pGLLandServer->GetMobSchMan() )
			{
				SGLNODE<GLMobSchedule*>* pCur = m_pGLLandServer->GetMobSchMan()->GetMobSchList()->m_pHead;
				while ( pCur )
				{
					GLMobSchedule* pMobSch = pCur->Data;
					if ( !pMobSch )	continue;
					pMobSch->Render( m_pd3dDevice, CV ); 
					pCur = pCur->pNext;
				}

				if ( m_pSelectSched )
					m_pSelectSched->Render( m_pd3dDevice, CV, 0xffff0000 );
			}

			if ( m_bSettingRenderCrowGenPos && m_pGLLandServer->GetMobSchMan() )
			{
				SGLNODE<GLMobSchedule*>* pCur = m_pGLLandServer->GetMobSchMan()->GetMobSchList()->m_pHead;
				while ( pCur )
				{
					GLMobSchedule* pMobSch = pCur->Data;
					if ( !pMobSch )	continue;
					pMobSch->RenderGenPos ( m_pd3dDevice ); 
					pCur = pCur->pNext;
				}

				if ( m_pSelectSched )
					m_pSelectSched->RenderGenPos( m_pd3dDevice, 0xffff0000 );
			}


			if ( m_bRenderGroupLine && m_pGLLandServer->GetMobSchMan() )
			{
				SGLNODE<GLMobSchedule*>* pCur = m_pGLLandServer->GetMobSchMan()->GetMobSchList()->m_pHead;
				while ( pCur )
				{
					GLMobSchedule* pMobSch = pCur->Data;
					if ( pMobSch && pMobSch->m_strGroupName.size() && !pMobSch->m_bLeaderMonster )
					{
						PGLCROW pcrow_find = m_pGLLandServer->GET_GROUPLEADER( pMobSch->m_strGroupName );
						if ( pcrow_find && pcrow_find->GetMobSchedule() )
						{
							if ( COLLISION::IsCollisionVolume(CV, pcrow_find->GetMobSchedule()->m_vMax, pcrow_find->GetMobSchedule()->m_vMin) )
							{
								EDITMESHS::RENDERLINE( m_pd3dDevice, pcrow_find->GetMobSchedule()->m_pAffineParts->vTrans + D3DXVECTOR3(0,4,0),
									pMobSch->m_pAffineParts->vTrans + D3DXVECTOR3(0,4,0), 0xffff00ff );
							}
						}
					}

					pCur = pCur->pNext;
				}
			}
		}

		DxShadowMap::GetInstance().LastImageBlur( m_pd3dDevice );

		if ( m_pDxLandMan )	
			m_pDxLandMan->Render_EFF( m_pd3dDevice, CV ); 

		if ( m_pGLLandServer )	
			m_pGLLandServer->Render_EFF( m_pd3dDevice, CV );  

		DxSkyMan::GetInstance().Render_AFTER( m_pd3dDevice );
		DxEffGroupPlayer::GetInstance().Render ( m_pd3dDevice );

		OPTMManager::GetInstance().Render( m_pd3dDevice );

		if ( m_pDxLandMan)	
			m_pDxLandMan->RenderPickAlpha( m_pd3dDevice );

		if ( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_FOGVERTEX )		
			m_pd3dDevice->SetRenderState ( D3DRS_FOGENABLE, FALSE );

		DxWeatherMan::GetInstance()->Render ( m_pd3dDevice );
		DxCubeMap::GetInstance().Render ( m_pd3dDevice );
		DxGlowMan::GetInstance().RenderTex ( m_pd3dDevice );
		DxGlowMan::GetInstance().Render ( m_pd3dDevice );
		DxGlowMan::GetInstance().RenderBURN( m_pd3dDevice );
		DxEnvironment::GetInstance().UserToRefraction ( m_pd3dDevice );
		DxEnvironment::GetInstance().RenderWave ( m_pd3dDevice, DxEffectMan::GetInstance().GetBumpTexture() );
		DxPostProcess::GetInstance().Render( m_pd3dDevice );
		DxEffProjMan::GetInstance().Render( m_pd3dDevice );

		if ( m_pGLLandServer )
		{
			if ( m_bSettingRenderQuadTree )	
				m_pGLLandServer->RendQuad( m_pd3dDevice );

			if ( m_bSettingRenderGate )	
			{
				m_pGLLandServer->GetLandGateMan().Render( m_pd3dDevice, 0xff00ffff );
				if ( m_pSelectedGate )
					m_pSelectedGate->Render( m_pd3dDevice, 0xffff0000 );
			}

			if ( m_bSettingRenderLandEffect )
			{
				SLEVEL_ETC_FUNC* pLevelEtcFunc = m_pGLLandServer->GetLevelEtcFunc();

				if ( pLevelEtcFunc )
				{
					if ( pLevelEtcFunc->m_bUseFunction[EMETCFUNC_LANDEFFECT] )
					{
						for( int i = 0; i < (int)pLevelEtcFunc->m_vecLandEffect.size(); i++ )
						{
							SLANDEFFECT landEffect = pLevelEtcFunc->m_vecLandEffect[i];
							D3DXVECTOR3 vMaxPos( landEffect.vMaxPos.x, 10.0f, landEffect.vMaxPos.y );
							D3DXVECTOR3 vMinPos( landEffect.vMinPos.x, -10.0f, landEffect.vMinPos.y );
							EDITMESHS::RENDERAABB ( m_pd3dDevice, vMaxPos, vMinPos, 0xffffff00 );
						}
					}
				}
			}
		}

		if ( m_pDxLandMan )
			EDITMESHS::RENDERSPHERE( m_pd3dDevice, m_vPos, 4.0, NULL, D3DXCOLOR( 0.0f, 1.0f, 0.0f, 1.0f) );

		if ( m_bEDITBOX )	
			m_DxEditBox.Render( m_pd3dDevice );

		if ( m_bEDITSMAT )	
			m_DxEditMat.Render( m_pd3dDevice );

		if ( m_bEDITMRS )	
			m_DxEditMRS.Render( m_pd3dDevice );

		CCursor::GetInstance().Render ( m_pd3dDevice, DxInputDevice::GetInstance().GetMouseLocateX(), DxInputDevice::GetInstance().GetMouseLocateY() );
		m_pd3dDevice->EndScene();
	}

	if (!m_bSettingRenderDebugInfo )
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

	PROFILE_END("Render");
	return S_OK;
}

HRESULT CEditorLevelView::InvalidateDeviceObjects()
{
	DxResponseMan::GetInstance().InvalidateDeviceObjects ();

	if ( m_pGLLandServer )	
		m_pGLLandServer->InvalidateDeviceObjects();

	if ( m_pDxLandMan ) 
		m_pDxLandMan->InvalidateDeviceObjects();

	m_DxEditMat.InvalidateDeviceObjects();
	m_DxEditMRS.OnLostDevice( m_pd3dDevice );

	return S_OK;
}

HRESULT CEditorLevelView::DeleteDeviceObjects()
{
#ifdef _DEBUG
	_CrtCheckMemory();
#endif //_DEBUG

	DxResponseMan::GetInstance().DeleteDeviceObjects ();
	GLogicData::GetInstance().DeleteDeviceObjects ();

	if ( m_pGLLandServer )	
		m_pGLLandServer->DeleteDeviceObjects();

	if ( m_pDxLandMan )	
		m_pDxLandMan->DeleteDeviceObjects();

	CCursor::GetInstance().DeleteDeviceObjects ();

	m_DxEditBox.DeleteDeviceObjects();
	m_DxEditMat.DeleteDeviceObjects();

	return S_OK;
}

HRESULT CEditorLevelView::FinalCleanup()
{
	m_DxEditMat.FinalCleanup();

	CDebugSet::FinalCleanup ();

	if ( m_pGLLandServer )	
		m_pGLLandServer->CleanUp();

	if ( m_pDxLandMan )	
		m_pDxLandMan->CleanUp();

	GLogicData::GetInstance().ClearData ();
	DxResponseMan::GetInstance().FinalCleanup ();
	
	return S_OK;
}