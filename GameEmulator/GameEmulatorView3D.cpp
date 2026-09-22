
// GameEmulatorView.cpp : implementation of the CGameEmulatorView class
//

#include "stdafx.h"
#include "GameEmulator.h"

#include "GameEmulatorDoc.h"
#include "GameEmulatorView.h"

#include "DlgCharset.h"


#include "DxServerInstance.h"
#include "DxGlobalStage.h"

#include "RANPARAM.h"
#include "DxResponseMan.h"
#include "GLOGIC.h"
#include "SUBPATH.h"
#include "dxparamset.h"
#include "CommonWeb.h"
#include "Cursor.h"
#include "DxFontMan.h"
#include "D3DFont.h"
#include "GameTextControl.h"

#include "DxEffectMan.h"
#include "DxGlowMan.h"
#include "DxPostProcess.h"
#include "DxLightMan.h"
#include "DxInputDevice.h"
#include "dxincommand.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


class DxDirectMsgServer : public DxMsgServer
{
public:
	virtual int SendClient ( DWORD dwClient, LPVOID pBuffer )
	{
		if ( GETMYCLIENTID()==dwClient )
			DxGlobalStage::GetInstance().MsgProcess ( reinterpret_cast<NET_MSG_GENERIC*> ( pBuffer ) );

		return 0;
	}

	virtual int SendField ( DWORD dwClient, LPVOID pBuffer )
	{
		GASSERT ( 0 && "SendField()" );
		return 0;
	}

	virtual int SendFieldSvr ( int nSvrNum, LPVOID pBuffer, int nServerChannel=0 )
	{
		GASSERT ( 0 && "SendFieldSvr()" );
		return 0;
	}

	virtual int SendAgent ( DWORD dwClient, LPVOID pBuffer )
	{
		NET_MSG_GENERIC *nmg = (NET_MSG_GENERIC *)pBuffer;

		switch ( nmg->nType )
		{
		default:
			if ( GETMYCLIENTID()==dwClient )
			{
				DxGlobalStage::GetInstance().MsgProcess ( reinterpret_cast<NET_MSG_GENERIC*> ( pBuffer ) );
			}
			break;
		};

		return 0;
	}

	virtual int SendAgent ( LPVOID pBuffer )
	{
		NET_MSG_GENERIC *nmg = (NET_MSG_GENERIC *)pBuffer;

		switch ( nmg->nType )
		{
		case NET_MSG_GCTRL_REQ_GATEOUT:
			GLGaeaServer::GetInstance().MsgProcess ( nmg, 0, GAEAID_NULL );
			break;

		default:
			break;
		};

		return 0;
	}

	virtual void SendAllField ( LPVOID pBuffer )
	{
		GASSERT(0&&"SendAllField()");
	}

	virtual void SendAllField	( LPVOID pBuffer, int nServerChannel=0 )
	{
		GASSERT(0&&"SendAllField()");
	}

	virtual void SendChannel    ( LPVOID pBuffer, int nServerChannel=0 )
	{
		GASSERT(0&&"SendAllField()");
	}

	virtual void SendAllChannel ( LPVOID pBuffer )
	{
		GASSERT(0&&"SendAllField()");
	}

public:
	static DxDirectMsgServer& GetInstance();
};

DxDirectMsgServer& DxDirectMsgServer::GetInstance()
{
	static DxDirectMsgServer Instance;
	return Instance;
}

class DxSetConsoleMsg : public DxConsoleMsg
{
public:
	virtual void Write ( int nType, const char* szFormat, ... )
	{
		va_list argList;
		char szBuffer[1024];

		va_start(argList, szFormat);
		StringCbVPrintf(szBuffer, 1024, szFormat, argList);
		va_end(argList);

		CDebugSet::ToListView ( szBuffer );
	}

	virtual void Write( const TCHAR* msg, ... )
	{
		if (msg == NULL) return;

		TCHAR szBuf[C_BUFFER_SIZE+1] = {0};	

		va_list ap;
		va_start(ap, msg);
		StringCchVPrintf(szBuf, C_BUFFER_SIZE+1, msg, ap);
		va_end(ap);	

		CDebugSet::ToListView ( szBuf );
	}

	static DxSetConsoleMsg& GetInstance();
};

DxSetConsoleMsg& DxSetConsoleMsg::GetInstance()
{
	static DxSetConsoleMsg Instance;
	return Instance;
}

void CGameEmulatorView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

}

HRESULT CGameEmulatorView::FrameMove3DEnvironment ()
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

HRESULT CGameEmulatorView::Render3DEnvironment ()
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

HRESULT CGameEmulatorView::ConfirmDevice ( D3DCAPSQ* pCaps, DWORD dwBehavior, D3DFORMAT Format )
{
	if ( dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING )
		return E_FAIL;
	return S_OK;
}

HRESULT CGameEmulatorView::OneTimeSceneInit()
{
	HRESULT hr;
	CGameEmulatorApp *pApp = (CGameEmulatorApp *) AfxGetApp();
	DxResponseMan::GetInstance().OneTimeSceneInit ( pApp->m_szAppPath, this,RANPARAM::strFontType, RANPARAM::dwLangSet, RANPARAM::strGDIFont );
	RANPARAM::SETUSERID ( "userid" );

	hr = DxFieldInstance::Create ( pApp->m_szAppPath, &DxDirectMsgServer::GetInstance(), &DxSetConsoleMsg::GetInstance(), NULL, SP_MINCOMS, "mapslist.mst", FIELDSERVER_MAX , 1000, true, 0, FALSE, FALSE, 3);	
	if ( FAILED(hr) )	return E_FAIL;

	DxGlobalStage::GetInstance().SetD3DApp(this);
	DxGlobalStage::GetInstance().OneTimeSceneInit ( pApp->m_szAppPath, m_hWndApp, TRUE, static_cast<WORD>(RANPARAM::dwScrWidth), static_cast<WORD>(RANPARAM::dwScrHeight), "mapslist.mst" );

	if( m_bEnableGUI )	CCommonWeb::Get()->Create( (CWnd*)this, &m_bVisibleGUI, &m_rtBoundGUI );

	return S_OK;
}

HRESULT CGameEmulatorView::CreateObjects()
{
	{ 
		DxFontMan::GetInstance().InitDeviceObjects ( m_pd3dDevice );
		CD3DFontPar* pD3dFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );
		CD3DFontPar* pD3dFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, D3DFONT_SHADOW|D3DFONT_ASCII );
		CDebugSet::InitDeviceObjects( pD3dFont9 );
		TCHAR szFullPath[MAX_PATH] = {0};
		StringCchCopy( szFullPath, MAX_PATH, ((CGameEmulatorApp*)AfxGetApp())->m_szAppPath );
		StringCchCat( szFullPath, MAX_PATH, SUBPATH::GUI_FILE_ROOT );
		CGameTextMan::GetInstance().SetPath(szFullPath);
		CGameTextMan::GetInstance().LoadText(RANPARAM::strGameWord.GetString(), CGameTextMan::EM_GAME_WORD, RANPARAM::bXML_USE );
		CGameTextMan::GetInstance().LoadText(RANPARAM::strGameInText.GetString(), CGameTextMan::EM_GAME_IN_TEXT, RANPARAM::bXML_USE );
		CGameTextMan::GetInstance().LoadText(RANPARAM::strGameExText.GetString(), CGameTextMan::EM_GAME_EX_TEXT, RANPARAM::bXML_USE );
		//xml translation
		if ( RANPARAM::strLangPlacer != "en" )
		{
			CGameTextMan::GetInstance().LoadText(RANPARAM::strGameWordNpc.GetString(), CGameTextMan::EM_GAME_NPC_WORD, RANPARAM::bXML_USE );
			CGameTextMan::GetInstance().LoadText(RANPARAM::strGameWordQuest.GetString(), CGameTextMan::EM_GAME_QUEST_WORD, RANPARAM::bXML_USE );
		}
	}

	HRESULT hr = S_OK;

	hr = InitDeviceObjects();
	if( FAILED(hr) )	return hr;

	hr = RestoreDeviceObjects();	
	if( FAILED(hr) )	return hr;

	return hr;
}

HRESULT CGameEmulatorView::InitDeviceObjects()
{
	SCHARDATA2 sCharData2;
	BOOL bLOAD = sCharData2.LOADFILE( sCharacter.strCharSet.c_str() );
	if ( !bLOAD )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "pCharData2->LOADFILE failed file :%s", sCharacter.strCharSet.c_str() );
	}

	if ( sCharacter.bMaxLevel )
	{
		sCharData2.m_wLevel = MAX_LEVEL;
	}

	if ( sCharacter.bMaxStats )
	{
		sCharData2.m_sStats.wPow = 50000;
		sCharData2.m_sStats.wStr = 50000;
		sCharData2.m_sStats.wSpi = 50000;
		sCharData2.m_sStats.wDex = 50000;
		sCharData2.m_sStats.wInt = 50000;
		sCharData2.m_sStats.wSta = 50000;
	}

	if ( sCharacter.bAllSkills )
	{
		for( int MID = 0; MID < EMSKILLCLASS_NSIZE; ++ MID )
		{
			for( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; ++ SID )
			{
				PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
				if ( pSKILL )
				{
					if ( (pSKILL->m_sLEARN.dwCLASS&sCharData2.m_emClass) )
					{
						WORD wSkillLevel = SKILL::MAX_LEVEL-1;

						if ( wSkillLevel >= pSKILL->m_sBASIC.dwMAXLEVEL ){
							wSkillLevel = (WORD)pSKILL->m_sBASIC.dwMAXLEVEL-1;
						}

						sCharData2.m_ExpSkills.insert(  std::make_pair( pSKILL->m_sBASIC.sNATIVEID.dwID, SCHARSKILL( pSKILL->m_sBASIC.sNATIVEID, wSkillLevel)) );
					}	
				}
			}
		}
	}

	if ( sCharacter.b1BGold )
	{
		sCharData2.m_lnMoney = 1000000000;
	}
	if ( sCharacter.b100KContrib )
	{
		sCharData2.m_llContributionPoint = 100000;
	}
	if ( sCharacter.b100KActivity )
	{
		sCharData2.m_dwActivityPoint = 100000;
	}
	StringCchCopy( sCharData2.m_szName, CHAR_SZNAME, sCharacter.strCharName.c_str() );

	sCharData2.m_cInventory.SetItemGenTime();
	sCharData2.m_dwUserLvl = sCharacter.nUserLevel;
	sCharData2.m_wHairColor = HAIRCOLOR::GetHairColor( CharClassToIndex(sCharData2.m_emClass), sCharData2.m_wHair );
	sCharData2.m_wSex = CharClassToGender( sCharData2.m_emClass );

	
	PGLCHAR pGLChar = GLGaeaServer::GetInstance().CreatePC ( &sCharData2, 0, 0, FALSE );
	if ( pGLChar )
	{
		pGLChar->MsgGameJoin ();
		DxGlobalStage::GetInstance().MsgProcessFrame ();
	}
	else
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "GLGaeaServer::GetInstance().CreatePC Failed." );
	}

	DxViewPort::GetInstance().InitDeviceObjects ( m_pd3dDevice, m_hWnd );
	DxResponseMan::GetInstance().InitDeviceObjects ( m_pd3dDevice );
	DXLIGHT sDirectional;
	sDirectional.SetDefault();
	sDirectional.m_Light.Diffuse = D3DXCOLOR(0,0,0,0);
	sDirectional.m_Light.Ambient = D3DXCOLOR(0,0,0,0);
	DxLightMan::SetDefDirect ( sDirectional );
	
	DxGlobalStage::GetInstance().InitDeviceObjects ( m_pd3dDevice );
	CCursor::GetInstance().InitDeviceObjects ();

	return S_OK;
}

HRESULT CGameEmulatorView::RestoreDeviceObjects()
{
	HRESULT hr=S_OK;
	DxResponseMan::GetInstance().RestoreDeviceObjects ();
	DxGlobalStage::GetInstance().RestoreDeviceObjects ();
	DXPARAMSET::INIT ();
	DxGlowMan::GetInstance().SetProjectActiveON();
	DxPostProcess::GetInstance().SetProjectActiveON();
	PROFILE_INIT();

	return S_OK;
}

HRESULT CGameEmulatorView::FrameMove()
{
	PROFILE_BEGIN("FrameMove");

	CheckControlfp();
	GLPeriod::GetInstance().UpdateWeather ( m_fElapsedTime );
	GLGaeaServer::GetInstance().FrameMove ( m_fTime, m_fElapsedTime );
	DxResponseMan::GetInstance().FrameMove( m_fTime, m_fElapsedTime, m_bDefWin );
	DxGlobalStage::GetInstance().FrameMove ( m_fTime, m_fElapsedTime );
	DxViewPort::GetInstance().FrameMove ( m_fTime, m_fElapsedTime );

	PROFILE_END("FrameMove");

	return S_OK;
}

HRESULT CGameEmulatorView::Render()
{
	PROFILE_BEGIN("Render");

	D3DCOLOR colorClear = D3DCOLOR_XRGB(89,135,179);
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
		DxGlobalStage::GetInstance().Render ();
		RenderText();
		CCursor::GetInstance().Render ( m_pd3dDevice, DxInputDevice::GetInstance().GetMouseLocateX(), DxInputDevice::GetInstance().GetMouseLocateY() );
		m_pd3dDevice->EndScene();
	}

	PROFILE_END("Render");
	return S_OK;
}


HRESULT CGameEmulatorView::RenderText()
{
	if ( !dxincommand::bDISP_CONSOLE && !dxincommand::bDISP_FPS )	return S_OK;

	D3DCOLOR fontColor  = D3DCOLOR_ARGB(255,255,255,255);

	TCHAR szMsg[MAX_PATH] = TEXT("");

	CD3DFontPar* pD3dFont = DxFontMan::GetInstance().FindFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );
	if ( !pD3dFont )	return S_OK;

	if ( dxincommand::bDISP_FPS )
	{
		FLOAT fNextLine = 40.0f; 

		lstrcpy( szMsg, m_strDeviceStats );
		fNextLine -= 20.0f;
		pD3dFont->DrawText( 2, fNextLine, fontColor, szMsg );

		lstrcpy( szMsg, m_strFrameStats );
		fNextLine -= 20.0f;
		pD3dFont->DrawText( 2, fNextLine, fontColor, szMsg );
	}

	if ( dxincommand::bDISP_CONSOLE )
	{
		CDebugSet::Render ();
	}

	return S_OK;
}

HRESULT CGameEmulatorView::InvalidateDeviceObjects()
{
	DxResponseMan::GetInstance().InvalidateDeviceObjects ();
	DxGlobalStage::GetInstance().InvalidateDeviceObjects ();
	return S_OK;
}

HRESULT CGameEmulatorView::DeleteDeviceObjects()
{
#ifdef _DEBUG
	_CrtCheckMemory();
#endif //_DEBUG

	DxResponseMan::GetInstance().DeleteDeviceObjects ();
	DxGlobalStage::GetInstance().DeleteDeviceObjects ();
	CCursor::GetInstance().DeleteDeviceObjects ();

	return S_OK;
}

HRESULT CGameEmulatorView::FinalCleanup()
{
	DxFieldInstance::CleanUp ();
	DxGlobalStage::GetInstance().FinalCleanup ();
	DxResponseMan::GetInstance().FinalCleanup ();

	return S_OK;
}

void CGameEmulatorView::SetActive ( BOOL bActive )
{
	DxInputDevice::GetInstance().OnActivate ( bActive );
}

void CGameEmulatorView::PostNcDestroy()
{
	CD3DApplication::Cleanup3DEnvironment();
	::SetCursor ( NULL );
	::DestroyCursor ( m_hCursorDefault );
	__super::PostNcDestroy();
}

void CGameEmulatorView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CGameEmulatorApp *pApp = (CGameEmulatorApp*) AfxGetApp();
	pApp->SetActive ( bActivate );
	CGameEmulatorView::SetActive ( bActivate );

	__super::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CGameEmulatorView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	if ( m_bCreated )
	{
		ReSizeWindow(cx,cy);
		if ( m_pd3dDevice )
		{
			DxGlobalStage::GetInstance().ReSizeWindow ( cx, cy );
		}
	}
}

BOOL CGameEmulatorView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
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