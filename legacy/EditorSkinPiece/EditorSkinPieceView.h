
// EditorSkinPieceView.h : interface of the CEditorSkinPieceView class
//
#include "D3DApp.h"
#include "DxSkinPieceContainer.h"
#include "DxObjectMRS.h"

#pragma once


enum EMCOLLISION_TYPE
{
	EMCOLLISION_TYPE_ISCOL	= 0,
	EMCOLLISION_TYPE_CLP1	= 1,
	EMCOLLISION_TYPE_CLP2	= 2,
};

class CEditorSkinPieceView : public CView, public CD3DApplication
{
protected: // create from serialization only
	CEditorSkinPieceView();
	DECLARE_DYNCREATE(CEditorSkinPieceView)

// Attributes
public:
	//CEditorSkinPieceDoc* GetDocument() const;
	static	CEditorSkinPieceView * GetView();

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CEditorSkinPieceView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	std::string		m_strFileName;
	DxSkinPiece*	m_pPiece;
	D3DXMATRIX		m_matLocal;
	D3DXVECTOR3		m_vCOL;
	DWORD			m_dwCOL;
	BOOL			m_bRenderBone;
	std::string		m_strTraceSelect;

public:
	EMCOLLISION_TYPE		m_emCOL;
	BOOL		m_bRenderPlane;
	BOOL		m_bRenderTrace;
	BOOL		m_bRenderDebug;

protected:

public:
	HRESULT ReSizeWindow ( int cx, int cy )
	{
		if ( cx < 10 )	cx = 10;
		if ( cy < 10 )	cy = 10;

		m_d3dpp.BackBufferWidth  = cx;
		m_d3dpp.BackBufferHeight = cy;

		return Resize3DEnvironment();
	}

private:
	char	m_szAppPath[MAX_PATH];
	BOOL	m_bCreated;
	HCURSOR	m_hCursorDefault;

protected:
	virtual void PostNcDestroy();
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);

public:
	void SetActive ( BOOL bActive );

public:
	virtual void OnInitialUpdate();
	virtual HRESULT FrameMove3DEnvironment();
	virtual HRESULT Render3DEnvironment();
	HRESULT ConfirmDevice( D3DCAPSQ*, DWORD, D3DFORMAT );

	HRESULT OneTimeSceneInit();
	HRESULT CreateObjects();
	HRESULT InitDeviceObjects();
	HRESULT RestoreDeviceObjects();
	HRESULT FrameMove();
	HRESULT Render();
	HRESULT InvalidateDeviceObjects();
	HRESULT DeleteDeviceObjects();
	HRESULT FinalCleanup();
	LPDIRECT3DDEVICEQ Get3DDevice()	{	return m_pd3dDevice;	}

public:
	void	PieceNew( std::string strFILENAME );

public:
	DxObjectMRS		m_DxEditMRS;
	BOOL			m_bEditMRS;

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);	
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnRenderRendertrace();
	afx_msg void OnUpdateRenderRendertrace(CCmdUI *pCmdUI);
	afx_msg void OnRenderRenderplane();
	afx_msg void OnUpdateRenderRenderplane(CCmdUI *pCmdUI);
	afx_msg void OnItemlevelOff();
	afx_msg void OnUpdateItemlevelOff(CCmdUI *pCmdUI);
	afx_msg void OnItemlevel01();
	afx_msg void OnUpdateItemlevel01(CCmdUI *pCmdUI);
	afx_msg void OnItemlevel02();
	afx_msg void OnUpdateItemlevel02(CCmdUI *pCmdUI);
	afx_msg void OnItemlevel03();
	afx_msg void OnUpdateItemlevel03(CCmdUI *pCmdUI);
	afx_msg void OnItemlevel04();
	afx_msg void OnUpdateItemlevel04(CCmdUI *pCmdUI);
	afx_msg void OnItemlevel05();
	afx_msg void OnUpdateItemlevel05(CCmdUI *pCmdUI);
	afx_msg void OnItemlevel06();
	afx_msg void OnUpdateItemlevel06(CCmdUI *pCmdUI);
	afx_msg void OnItemlevel07();
	afx_msg void OnUpdateItemlevel07(CCmdUI *pCmdUI);
	afx_msg void OnItemlevel08();
	afx_msg void OnUpdateItemlevel08(CCmdUI *pCmdUI);
	afx_msg void OnItemlevel09();
	afx_msg void OnUpdateItemlevel09(CCmdUI *pCmdUI);
	afx_msg void OnItemlevel10();
	afx_msg void OnUpdateItemlevel10(CCmdUI *pCmdUI);
	afx_msg void OnItemlevel11();
	afx_msg void OnUpdateItemlevel11(CCmdUI *pCmdUI);
	afx_msg void OnRenderRenderdebuginfo();
	afx_msg void OnUpdateRenderRenderdebuginfo(CCmdUI *pCmdUI);
	afx_msg void OnCameraResetcamerapos();
	afx_msg void OnCollisiontypeType1();
	afx_msg void OnUpdateCollisiontypeType1(CCmdUI *pCmdUI);
	afx_msg void OnCollisiontypeType2();
	afx_msg void OnUpdateCollisiontypeType2(CCmdUI *pCmdUI);
	afx_msg void OnCollisiontypeType3();
	afx_msg void OnUpdateCollisiontypeType3(CCmdUI *pCmdUI);
	
	afx_msg void OnEditMatrixMove();
	afx_msg void OnUpdateEditMatrixMove(CCmdUI *pCmdUI);
	afx_msg void OnEditMatrixRotate();
	afx_msg void OnUpdateEditMatrixRotate(CCmdUI *pCmdUI);
	afx_msg void OnEditMatrixScale();
	afx_msg void OnUpdateEditMatrixScale(CCmdUI *pCmdUI);
	afx_msg void OnFileSaveeffskin();
};

//#ifndef _DEBUG  // debug version in EditorSkinPieceView.cpp
//inline CEditorSkinPieceDoc* CEditorSkinPieceView::GetDocument() const
//   { return reinterpret_cast<CEditorSkinPieceDoc*>(m_pDocument); }
//#endif

