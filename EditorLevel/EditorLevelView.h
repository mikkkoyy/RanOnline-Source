
// EditorLevelView.h : interface of the CEditorLevelView class
//
#include "D3DApp.h"

class DxLandMan;
class GLLandMan;
class GLMobSchedule;

#include "GLMapList.h"
#include "DxEditBox.h"
#include "DxEditMat.h"
#include "DxObjectMRS.h"

#pragma once


class CEditorLevelView : public CView, public CD3DApplication
{
protected: // create from serialization only
	CEditorLevelView();
	DECLARE_DYNCREATE(CEditorLevelView)

// Attributes
public:
	//CEditorLevelDoc* GetDocument() const;
	static	CEditorLevelView * GetView();

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CEditorLevelView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

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
	GLLandMan*		m_pGLLandServer;	
	DxLandMan*		m_pDxLandMan;
	std::string		m_strFileName;
	D3DXVECTOR3		m_vPos;
	GLMobSchedule*	m_pSelectSched;
	DxLandGate*		m_pSelectedGate;

	DxEditBox		m_DxEditBox;
	DxEditMat		m_DxEditMat;
	DxObjectMRS		m_DxEditMRS;
	GLMapList		m_sMap;

public:
	BOOL	m_bSettingRenderDebugInfo;
	BOOL	m_bSettingRenderCrowModel;
	BOOL	m_bSettingRenderCrowSched;
	BOOL	m_bSettingRenderCrowGenPos;
	BOOL	m_bSettingRenderQuadTree;
	BOOL	m_bSettingRenderGate;
	BOOL	m_bSettingRenderLandEffect;

	BOOL	m_bSettingSoundBGM;
	BOOL	m_bSettingSoundENV;
	BOOL	m_bSettingSoundSFX;

	BOOL	m_bEDITSMAT;
	BOOL	m_bEDITBOX;
	BOOL	m_bEDITMRS;

	BOOL	m_bDETECTCROW;
	BOOL	m_bDETECTGATE;

	BOOL	m_bRenderGroupLine;

public:
	void ApplySettings();
	void LoadWLD( std::string strName );
	void ResetEditor();

	void ResetMouseEdit();
	void EditSmatSet( BOOL bENABLE );
	void EditBoxSet( BOOL bENABLE );
	void EditMRSSet( BOOL bENABLE );

	void DataCleanUp();

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);	
	afx_msg void OnLevelNew();
	afx_msg void OnLevelOpen();
	afx_msg void OnLevelOpenex();
	afx_msg void OnLevelSave();

	afx_msg void OnSettingRenderDebuginfo();
	afx_msg void OnUpdateSettingRenderDebuginfo(CCmdUI *pCmdUI);
	afx_msg void OnSettingRenderCrowmodel();
	afx_msg void OnUpdateSettingRenderCrowmodel(CCmdUI *pCmdUI);
	afx_msg void OnSettingRenderCrowschedule();
	afx_msg void OnUpdateSettingRenderCrowschedule(CCmdUI *pCmdUI);
	afx_msg void OnSettingRenderCrowgenpos();
	afx_msg void OnUpdateSettingRenderCrowgenpos(CCmdUI *pCmdUI);
	afx_msg void OnSettingRenderQuadtree();
	afx_msg void OnUpdateSettingRenderQuadtree(CCmdUI *pCmdUI);
	afx_msg void OnRenderRenderGate();
	afx_msg void OnUpdateRenderRenderGate(CCmdUI *pCmdUI);
	afx_msg void OnSettingRenderLandeffect();
	afx_msg void OnUpdateSettingRenderLandeffect(CCmdUI *pCmdUI);
	afx_msg void OnCameraResetcamerapos();
	afx_msg void OnSettingCameraspeeddefault();
	afx_msg void OnUpdateSettingCameraspeeddefault(CCmdUI *pCmdUI);
	afx_msg void OnSettingCameraspeed500();
	afx_msg void OnUpdateSettingCameraspeed500(CCmdUI *pCmdUI);
	afx_msg void OnSettingCameraspeed1000();
	afx_msg void OnUpdateSettingCameraspeed1000(CCmdUI *pCmdUI);
	afx_msg void OnSettingCameraspeed1500();
	afx_msg void OnUpdateSettingCameraspeed1500(CCmdUI *pCmdUI);
	afx_msg void OnSettingSoundBgm();
	afx_msg void OnUpdateSettingSoundBgm(CCmdUI *pCmdUI);
	afx_msg void OnSettingSoundEnv();
	afx_msg void OnUpdateSettingSoundEnv(CCmdUI *pCmdUI);
	afx_msg void OnSettingSoundSfx();
	afx_msg void OnUpdateSettingSoundSfx(CCmdUI *pCmdUI);
	afx_msg void OnSettingFograngeDefault();
	afx_msg void OnUpdateSettingFograngeDefault(CCmdUI *pCmdUI);
	afx_msg void OnSettingFogrange5k();
	afx_msg void OnUpdateSettingFogrange5k(CCmdUI *pCmdUI);
	afx_msg void OnSettingFogrange10k();
	afx_msg void OnUpdateSettingFogrange10k(CCmdUI *pCmdUI);
	afx_msg void OnSettingFogrange20k();
	afx_msg void OnUpdateSettingFogrange20k(CCmdUI *pCmdUI);
	afx_msg void OnSettingFogrange50k();
	afx_msg void OnUpdateSettingFogrange50k(CCmdUI *pCmdUI);
	afx_msg void OnCameraJumptosphere();
	afx_msg void OnEditSmatrixMove();
	afx_msg void OnUpdateEditSmatrixMove(CCmdUI *pCmdUI);
	afx_msg void OnEditSmatrixRotate();
	afx_msg void OnUpdateEditSmatrixRotate(CCmdUI *pCmdUI);
	afx_msg void OnEditSmatrixScale();
	afx_msg void OnUpdateEditSmatrixScale(CCmdUI *pCmdUI);
	afx_msg void OnEditBoxMove();
	afx_msg void OnUpdateEditBoxMove(CCmdUI *pCmdUI);
	afx_msg void OnEditBoxScale();
	afx_msg void OnUpdateEditBoxScale(CCmdUI *pCmdUI);
	afx_msg void OnEditBoxSelscale();
	afx_msg void OnUpdateEditBoxSelscale(CCmdUI *pCmdUI);
	afx_msg void OnEditMatrixMove();
	afx_msg void OnUpdateEditMatrixMove(CCmdUI *pCmdUI);
	afx_msg void OnEditMatrixRotate();
	afx_msg void OnUpdateEditMatrixRotate(CCmdUI *pCmdUI);
	afx_msg void OnEditMatrixScale();
	afx_msg void OnUpdateEditMatrixScale(CCmdUI *pCmdUI);
	afx_msg void OnDetectSelectCrow();
	afx_msg void OnUpdateDetectSelectCrow(CCmdUI *pCmdUI);
	afx_msg void OnDetectSelectGate();
	afx_msg void OnUpdateDetectSelectGate(CCmdUI *pCmdUI);
	afx_msg void OnLevelSaveWld();
	afx_msg void OnRenderRendergroupline();
	afx_msg void OnUpdateRenderRendergroupline(CCmdUI *pCmdUI);
};

//#ifndef _DEBUG  // debug version in EditorLevelView.cpp
//inline CEditorLevelDoc* CEditorLevelView::GetDocument() const
//   { return reinterpret_cast<CEditorLevelDoc*>(m_pDocument); }
//#endif

