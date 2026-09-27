
// GameClient2.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"

#ifndef STRICT
#define STRICT
#endif

#include "GameClient2.h"
#include "GameClient2Wnd.h"

#include "RANPARAM.h"
#include "SUBPATH.h"
#include "MinBugTrap.h"
#include "dxparamset.h"
#include "StringUtils.h"
#include "getdxver.h"
#include "GameTextControl.h"
#include "GLogic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DO_RTC_CATCH _rtc_catch;

// CGameClient2App

BEGIN_MESSAGE_MAP(CGameClient2App, CWinApp)
	ON_COMMAND(ID_HELP, OnHelp)
END_MESSAGE_MAP()


// CGameClient2App construction

CGameClient2App::CGameClient2App()
	: m_bIsActive(TRUE)
	, m_bRedrawScreen(FALSE)
	, m_bShutDown(FALSE)
{
	BUG_TRAP::BugTrapInstall( std::string(_T("MiniA")));

#ifdef CH_PARAM
	RANPARAM::emSERVICE_TYPE = EMSERVICE_CHINA;
#endif

#ifdef TH_PARAM
	RANPARAM::emSERVICE_TYPE = EMSERVICE_THAILAND;
#endif

#ifdef MY_PARAM
	RANPARAM::emSERVICE_TYPE = EMSERVICE_MALAYSIA_CN;
#endif

#ifdef MYE_PARAM
	RANPARAM::emSERVICE_TYPE = EMSERVICE_MALAYSIA_EN;
#endif

#ifdef ID_PARAM
	RANPARAM::emSERVICE_TYPE = EMSERVICE_INDONESIA;
#endif

#ifdef PH_PARAM
	//RANPARAM::bScrWndHalfSize = FALSE;
	RANPARAM::emSERVICE_TYPE = EMSERVICE_PHILIPPINES;
#endif

#ifdef VN_PARAM
	RANPARAM::emSERVICE_TYPE = EMSERVICE_VIETNAM;
#endif

#ifdef JP_PARAM
	RANPARAM::emSERVICE_TYPE = EMSERVICE_JAPAN;
#endif

#ifdef TW_PARAM
	RANPARAM::emSERVICE_TYPE = EMSERVICE_FEYA;
#endif

#ifdef HK_PARAM
	RANPARAM::emSERVICE_TYPE = EMSERVICE_FEYA;
#endif

#if defined(KR_PARAM) || defined(KRT_PARAM) 
	RANPARAM::emSERVICE_TYPE = EMSERVICE_KOREA;
#endif

#if defined(GS_PARAM)
	RANPARAM::emSERVICE_TYPE = EMSERVICE_GLOBAL;
#endif

	SecureZeroMemory( m_szAppPath, sizeof(m_szAppPath) );
}


// The one and only CGameClient2App object

CGameClient2App theApp;


// CGameClient2App initialization

BOOL CGameClient2App::InitInstance()
{
	AfxEnableControlContainer();
	SetAppPath();

	/*hackshield implementation, Juver, 2018/06/18 */
	if( !hs_start() ){
		hs_stop();
		return FALSE;
	}

	if( !hs_start_service() ){
		hs_stop_service();
		return FALSE;
	}

	RANPARAM::LOAD ( m_szAppPath );
	DXPARAMSET::INIT ();

	CString StrCmdLine = m_lpCmdLine;

	STRUTIL::ClearSeparator ();
	STRUTIL::RegisterSeparator ( "/" );
	STRUTIL::RegisterSeparator ( " " );

	BOOL bAPM = FALSE;
	CString strCommand = CString("zxc") + "..." + "drun";

	CStringArray strCmdArray;
	STRUTIL::StringSeparate ( StrCmdLine, strCmdArray );

	for ( int i=0; i<strCmdArray.GetCount(); i++ )
	{
		CString strTemp = strCmdArray.GetAt(i);

		if ( strTemp == strCommand )
		{
			bAPM = TRUE;
		}
	}

	if( RANPARAM::emSERVICE_TYPE==EMSERVICE_MALAYSIA_CN || 
		RANPARAM::emSERVICE_TYPE==EMSERVICE_MALAYSIA_EN ||
		RANPARAM::emSERVICE_TYPE==EMSERVICE_PHILIPPINES ||
		RANPARAM::emSERVICE_TYPE==EMSERVICE_VIETNAM )
	{
		if ( !RANPARAM::VALIDIDPARAM() )
		{
			MessageBox ( NULL, "Invalid web account.", "error", MB_OK );
			return FALSE;
		}
	}
	
	if ( !bAPM )
	{
		MessageBox ( NULL, "Please Use Launcher to Play!", "Info", MB_OK );
		return FALSE;
	}


	DWORD dwDirectXVersion = 0;
	TCHAR strDirectXVersion[10];
	HRESULT hr = getdxversion ( &dwDirectXVersion, strDirectXVersion, 10 );

	if ( hr==S_OK && dwDirectXVersion<0x00090003 )
	{
		MessageBox ( NULL, "DirectX Version too old. must install DirectX 9.0c", "error", MB_OK );
		return FALSE;
	}

	TCHAR szFullPath[MAX_PATH] = {0};
	StringCchCopy( szFullPath, MAX_PATH, ((CGameClient2App*)AfxGetApp())->m_szAppPath );
	StringCchCat( szFullPath, MAX_PATH, SUBPATH::GUI_FILE_ROOT );

	CGameTextMan::GetInstance().SetPath(szFullPath);
	CGameTextMan::GetInstance().LoadText(RANPARAM::strGameWord.GetString(), CGameTextMan::EM_GAME_WORD, RANPARAM::bXML_USE );
	CGameTextMan::GetInstance().LoadText(RANPARAM::strGameInText.GetString(), CGameTextMan::EM_GAME_IN_TEXT, RANPARAM::bXML_USE );
	CGameTextMan::GetInstance().LoadText(RANPARAM::strGameExText.GetString(), CGameTextMan::EM_GAME_EX_TEXT, RANPARAM::bXML_USE );

	CGameClient2Wnd* pWnd = new CGameClient2Wnd();
	if ( pWnd->Create() == FALSE )	return FALSE;

	m_pWnd = pWnd;
	
	return TRUE;
}

void CGameClient2App::SetAppPath()
{
	CString strAppPath;

	TCHAR szPath[MAX_PATH] = {0};
	GetModuleFileName(::AfxGetInstanceHandle(), szPath, MAX_PATH);
	strAppPath = szPath;

	if ( !strAppPath.IsEmpty() )
	{
		DWORD dwFind = strAppPath.ReverseFind ( '\\' );
		if ( dwFind != -1 )
		{
			m_strAppPath = strAppPath.Left ( dwFind );

			if ( !m_strAppPath.IsEmpty() )
				if ( m_strAppPath.GetAt(0) == '"' )
					m_strAppPath = m_strAppPath.Right ( m_strAppPath.GetLength()-1 );

			StringCchCopy ( m_szAppPath, MAX_PATH, m_strAppPath.GetString() );
		}
	}else {
		MessageBox ( NULL, "SetAppPath Error", "Error", MB_OK );
		return;
	}
}

int CGameClient2App::ExitInstance() 
{
	m_pWnd = NULL;

	/*hackshield implementation, Juver, 2018/06/18 */
	hs_stop_service();
	hs_stop();

	return CWinApp::ExitInstance();
}

int CGameClient2App::Run() 
{
	MSG msg;
	BOOL bGotMsg;

	PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE );

	while( WM_QUIT != msg.message )
	{
		if ( m_bShutDown == TRUE || m_pWnd == NULL )
		{
			ExitInstance ();
			return 0;
		}

		if( m_bIsActive )
			bGotMsg = PeekMessage( &msg, NULL, 0, 0, PM_REMOVE );
		else
			bGotMsg = GetMessage( &msg, NULL, 0, 0 );

		if( bGotMsg )
		{
			TranslateMessage (&msg);
			DispatchMessage (&msg);

			if ( !m_bIsActive && !m_bShutDown )
			{
				if ( FAILED ( m_pWnd->FrameMove3DEnvironment() ) )
					m_pWnd->PostMessage ( WM_CLOSE, 0, 0 );
			}
		}else{
			if ( m_bIsActive )
			{
				if ( FAILED ( m_pWnd->Render3DEnvironment() ) )
					m_pWnd->PostMessage ( WM_CLOSE, 0, 0 );
			}
		}		
	}

	ExitInstance ();
	return static_cast<int>(msg.wParam);
}

void CGameClient2App::SetActive ( BOOL bActive )
{
	m_bIsActive = bActive;
	m_bRedrawScreen = bActive;
}