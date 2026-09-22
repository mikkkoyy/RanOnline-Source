

/////////////////////////////////////////////////////////////////////////////
// CGameClient2Wnd window

#pragma once

#include "D3DApp.h"


/*hackshield implementation, Juver, 2018/06/21 */
#define HS_DUMMY_HEARTBEAT 25.0f

class CGameClient2Wnd : public CWnd, public CD3DApplication
{
private:
	int			m_nScreenWinX;
	int			m_nScreenWinY;
	int			m_wndSizeX;
	int			m_wndSizeY;

	HCURSOR		m_hCursorDefault;

private:
	BOOL		m_bCreated;
	std::string m_strTEXTURE;

	UINT_PTR	m_nGGTimer;
	UINT_PTR	m_nGGATimer;
	UINT_PTR	m_nGGA12Timer;

	HANDLE		m_hMutex;


	/*hackshield implementation, Juver, 2018/06/21 */
	float		hs_dummy_heartbeat_timer;

public:
	LPDIRECT3DDEVICEQ GetD3dDevice ()	{ return m_pd3dDevice; }

public:
	HRESULT InitDeviceObjects();

private:
	HRESULT ConfirmDevice( D3DCAPSQ*,DWORD,D3DFORMAT );
	HRESULT OneTimeSceneInit();
	HRESULT CreateObjects();
	HRESULT RestoreDeviceObjects();
	HRESULT FrameMove();
	HRESULT Render();
	HRESULT InvalidateDeviceObjects();
	HRESULT DeleteDeviceObjects();
	HRESULT FinalCleanup();

	HRESULT RenderText();
	HRESULT ReSizeWindow ( int cx, int cy )
	{
		if ( cx < 10 )	cx = 10;
		if ( cy < 10 )	cy = 10;

		m_d3dpp.BackBufferWidth  = cx;
		m_d3dpp.BackBufferHeight = cy;

		return Resize3DEnvironment();
	}

	virtual HRESULT Resize3DEnvironment();

public:
	virtual HRESULT FrameMove3DEnvironment();
	virtual HRESULT Render3DEnvironment();

// Construction
public:
	CGameClient2Wnd();

// Attributes
public:
  CGameClient2App* m_pApp;

// Operations
public:
  BOOL Create();
 
protected:
	virtual void PostNcDestroy();

// Implementation
public:
	virtual ~CGameClient2Wnd();

	// Generated message map functions
protected:
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnNetNotify(WPARAM wParam, LPARAM lParam);

	afx_msg void OnActivateApp(BOOL bActive, DWORD hTask);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
protected:
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
};

/////////////////////////////////////////////////////////////////////////////
