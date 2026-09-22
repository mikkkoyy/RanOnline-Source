
// GameEmulatorView.h : interface of the CGameEmulatorView class
//
#include "D3DApp.h"
#include "EmulatorData.h"

#pragma once

struct SCHARDATA2;

class CGameEmulatorView : public CView, public CD3DApplication
{
protected: // create from serialization only
	CGameEmulatorView();
	DECLARE_DYNCREATE(CGameEmulatorView)

private:
	BOOL		m_bCreated;
	HCURSOR		m_hCursorDefault;

	SCHARSET sCharacter;

public:
	virtual void OnInitialUpdate();
	virtual HRESULT FrameMove3DEnvironment();
	virtual HRESULT Render3DEnvironment();

private:
	HRESULT ConfirmDevice( D3DCAPSQ*,DWORD,D3DFORMAT );
	HRESULT OneTimeSceneInit();
	HRESULT CreateObjects();
	HRESULT InitDeviceObjects();	
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

public:
	void SetActive ( BOOL bActive );

protected:
	virtual void PostNcDestroy();
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);

// Attributes
public:
	//CGameEmulatorDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CGameEmulatorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnEmulatorRun();
};

//#ifndef _DEBUG  // debug version in GameEmulatorView.cpp
//inline CGameEmulatorDoc* CGameEmulatorView::GetDocument() const
//   { return reinterpret_cast<CGameEmulatorDoc*>(m_pDocument); }
//#endif

