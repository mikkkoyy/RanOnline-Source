#include "stdafx.h"

#ifndef STRICT
	#define STRICT
#endif

#include "DxGlobalStage.h"
#include "SUBPATH.h"
#include "RANPARAM.h"
#include "dxparamset.h"
#include "../Lib_Engine/GUInterface/Cursor.h"
#include "DxCursor.h"

#include "GameClient2.h"
#include "GameClient2Wnd.h"
#include "BlockProg.h"
#include "GLogicData.h"
#include "GLogic.h"
#include "s_NetClient.h"
#include "rol_clipboard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HWND g_hWnd(NULL);

/////////////////////////////////////////////////////////////////////////////
// CGameClient2Wnd

CGameClient2Wnd::CGameClient2Wnd() :
	CD3DApplication(),
	m_bCreated(FALSE),
	m_wndSizeX(1024),
	m_wndSizeY(768),
	m_hCursorDefault(FALSE),
	m_nGGTimer(0),
	m_nGGATimer(0),
	m_nGGA12Timer(0),

	m_hMutex( NULL ),

	/*hackshield implementation, Juver, 2018/06/21 */
	hs_dummy_heartbeat_timer(0.0f)
{
	m_bUseDepthBuffer = TRUE;
	m_pApp = (CGameClient2App*)AfxGetApp();
}

CGameClient2Wnd::~CGameClient2Wnd()
{
}

BEGIN_MESSAGE_MAP(CGameClient2Wnd, CWnd)
	ON_WM_ACTIVATEAPP()
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
	ON_WM_SIZE()
	ON_MESSAGE(NET_NOTIFY,  OnNetNotify)
	ON_WM_SYSCOMMAND()
	ON_WM_MOUSEMOVE()
	ON_WM_GETMINMAXINFO()
	ON_WM_NCACTIVATE()
	ON_WM_TIMER()
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGameClient2Wnd message handlers

BOOL CGameClient2Wnd::Create() 
{
	CGameClient2App *pRanClientApp = (CGameClient2App *) AfxGetApp();

	CRect rectWnd;
	LONG nWindowStyle = WS_POPUP | WS_VISIBLE;
	
	if ( RANPARAM::bScrWindowed )
	{
		m_nScreenWinX = GetSystemMetrics(SM_CXSCREEN);
		m_nScreenWinY = GetSystemMetrics(SM_CYSCREEN);

		m_wndSizeX = RANPARAM::dwScrWidth;
		m_wndSizeY = RANPARAM::dwScrHeight;

		if ( m_wndSizeX>m_nScreenWinX || m_wndSizeY>m_nScreenWinY )
		{
			m_wndSizeX = m_nScreenWinX;
			m_wndSizeY = m_nScreenWinY;
		}

		if ( RANPARAM::bScrWndFullSize )
		{
			m_wndSizeX = m_nScreenWinX;
			m_wndSizeY = m_nScreenWinY;

			/*float fScreen = m_nScreenWinX / float(m_nScreenWinY);
			if ( fScreen > (16.0f/9.0f) )
			{
			m_wndSizeX = m_nScreenWinX/2;
			}*/

			rectWnd = CRect( 0, 0, m_wndSizeX, m_wndSizeY );
		}else{
			if ( m_wndSizeX>=(m_nScreenWinX) || m_wndSizeY>=(m_nScreenWinY) )
			{
				m_wndSizeX = 1024;
				m_wndSizeY = 768;

				if ( m_wndSizeX>=m_nScreenWinX || m_wndSizeY>=m_nScreenWinY )
				{
					m_wndSizeX = m_nScreenWinX-60;
					m_wndSizeY = m_nScreenWinY-80;
				}
			}

			nWindowStyle = WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION |  WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_CLIPCHILDREN;

			RECT rt = {0, 0, m_wndSizeX, m_wndSizeY};
			AdjustWindowRectEx(&rt, nWindowStyle, FALSE, WS_EX_APPWINDOW);

			m_wndSizeX = rt.right - rt.left;
			m_wndSizeY = rt.bottom - rt.top;
			INT nX = (m_nScreenWinX - m_wndSizeX) / 2;
			INT nY = (m_nScreenWinY - m_wndSizeY) / 2;

			rectWnd = CRect( nX, nY, m_wndSizeX, m_wndSizeY );
		}
	}
	else
	{
		m_nScreenWinX = GetSystemMetrics(SM_CXSCREEN);
		m_nScreenWinY = GetSystemMetrics(SM_CYSCREEN);

		m_wndSizeX = m_nScreenWinX;
		m_wndSizeY = m_nScreenWinY;

		rectWnd = CRect( 0, 0, m_wndSizeX, m_wndSizeY );
	}

	/*m_hMutex = CreateMutex( NULL, TRUE, "RAN-ONLINE" );

	if ( GetLastError() == ERROR_ALREADY_EXISTS ) 
	{ 
		return FALSE;
	}*/

	LPCSTR strClass = AfxRegisterWndClass(CS_DBLCLKS, ::LoadCursor(NULL, IDC_ARROW), (HBRUSH)GetStockObject(BLACK_BRUSH), AfxGetApp()->LoadIcon(IDR_MAINFRAME) );
	if( !strClass )
	{
		AfxMessageBox( _T( "Class Registration Failed\n" ) );
		return FALSE;
	}

	if ( CreateEx ( WS_EX_APPWINDOW, strClass, AfxGetAppName(), nWindowStyle, rectWnd.left, rectWnd.top, rectWnd.right, rectWnd.bottom, NULL, NULL, 0 ) == FALSE )
	{
		return FALSE;
	}

	SetWindowText (_T(RANPARAM::ClientWindowTitle));

	UpdateWindow();

	CD3DApplication::SetScreen( RANPARAM::dwScrWidth, RANPARAM::dwScrHeight, RANPARAM::emScrFormat, RANPARAM::uScrRefreshHz, RANPARAM::bScrWindowed );

	if ( FAILED( CD3DApplication::Create ( m_hWnd, m_hWnd, AfxGetInstanceHandle() ) ) )
		return FALSE;
	
	m_bCreated = TRUE;
	g_hWnd = m_hWnd;

	return TRUE;
}

void CGameClient2Wnd::PostNcDestroy() 
{
	::UnregisterClass( AfxGetAppName(), AfxGetInstanceHandle() ); 

	Cleanup3DEnvironment();

	CWnd::PostNcDestroy();

	m_pApp->ShutDown();

	if ( RANPARAM::bGameCursor )
	{
		::SetCursor ( NULL );
		if ( m_hCursorDefault )		::DestroyCursor ( m_hCursorDefault );
	}

	delete this;

	HWND hTrayWnd = ::FindWindow("Shell_TrayWnd", NULL);
	::SendMessage( hTrayWnd, WM_COMMAND, 416, 0 );
}

void CGameClient2Wnd::OnActivateApp(BOOL bActive, DWORD hTask)
{
	CWnd::OnActivateApp(bActive, hTask);
}

BOOL CGameClient2Wnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if ( RANPARAM::bGameCursor )
	{
		HCURSOR hCursl = CCursor::GetInstance().GetCurCursor();
		if ( hCursl )
		{
			CCursor::GetInstance().SetCursorNow ();
			return TRUE;
		}
	}

	return CWnd::OnSetCursor ( pWnd, nHitTest, message );
}

void CGameClient2Wnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if ( RANPARAM::bGameCursor )
	{
		HCURSOR hCursl = CCursor::GetInstance().GetCurCursor();
		if ( hCursl )
		{
			CCursor::GetInstance().SetCursorNow ();
			return;
		}
	}

	__super::OnMouseMove(nFlags, point);
}

void CGameClient2Wnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch(nChar)
	{
	case VK_ESCAPE:
		break;
	};

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

LRESULT CGameClient2Wnd::OnNetNotify(WPARAM wParam, LPARAM lParam)
{
	DxGlobalStage::GetInstance().OnNetNotify ( wParam, lParam );
	return NULL;
}
