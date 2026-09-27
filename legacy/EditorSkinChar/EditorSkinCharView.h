
// EditorSkinCharView.h : interface of the CEditorSkinCharView class
//

#include "D3DApp.h"
#include "DxSkinChar.h"
#include "DxSkinCharData.h"

/*ABL system, Juver, 2017/05/30 */
#include "DxAttBoneRender.h"
#include "DxAttBoneData.h"
#include "DxAttBone.h"
#include "DxAttBoneLink.h"

/*vehicle system, Juver, 2017/08/05 */
#include "DxVehicle.h"
#include "DxVehicleData.h"


#include "DxEditMat.h"

#pragma once


class CEditorSkinCharView : public CView, public CD3DApplication
{
protected: // create from serialization only
	CEditorSkinCharView();
	DECLARE_DYNCREATE(CEditorSkinCharView)

// Attributes
public:
	//CEditorSkinCharDoc* GetDocument() const;
	static	CEditorSkinCharView * GetView();

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CEditorSkinCharView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

public:
	D3DXMATRIX		m_matLocal;
	DxSkinCharData*	m_pSkinCharData;
	DxSkinChar*		m_pSkinChar;

	/*ABL system, Juver, 2017/05/30 */
	DxAttBoneData*		m_pAttBoneData;
	DxAttBoneRender*	m_pAttBoneRender;
	DxAttBoneLink*		m_pAttBoneLink;
	DxAttBone*			m_pAttBone;
	DxSkinPiece*		m_pMaskPiece;

	/*vehicle system, Juver, 2017/08/05 */
	DxVehicleData*		m_pVehicleData;
	DxVehicle*			m_pVehicle;
	DxSkinChar*			m_pVehicleSkin;
	BOOL				m_bEditBack;
	
public:
	DxEditMat		m_DxEditMat;
	BOOL			m_bEDITSMAT;

	void ResetMouseEdit();
	void EditSmatSet( BOOL bENABLE );

public:
	BOOL	m_bRenderDebug;
	BOOL	m_bRenderPlane;
	BOOL	m_bRenderAABB;
	BOOL	m_bRenderSkeleton;
	BOOL	m_bAniAttack;

public:
	void	ResetData();
	BOOL	CHFLoad( std::string strFileName );
	BOOL	CHFSave( std::string strFileName );
	BOOL	CHFNew();

	/*ABL system, Juver, 2017/05/30 */
	BOOL	ABFLoad( std::string strFileName );
	BOOL	ABFSave( std::string strFileName );
	BOOL	ABFNew();
	BOOL	ABLLoad( std::string strFileName );
	BOOL	ABLSave( std::string strFileName );
	BOOL	ABLNew();

	/*vehicle system, Juver, 2017/08/05 */
	BOOL	VCFLoad( std::string strFileName );
	BOOL	VCFSave( std::string strFileName );
	BOOL	VCFNew();
	BOOL	VCFLoadTest( std::string strFileName );

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

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);	

	afx_msg void OnRenderDebuginfo();
	afx_msg void OnUpdateRenderDebuginfo(CCmdUI *pCmdUI);
	afx_msg void OnRenderPlane();
	afx_msg void OnUpdateRenderPlane(CCmdUI *pCmdUI);
	afx_msg void OnRenderAabbbox();
	afx_msg void OnUpdateRenderAabbbox(CCmdUI *pCmdUI);
	afx_msg void OnRenderSkeleton();
	afx_msg void OnUpdateRenderSkeleton(CCmdUI *pCmdUI);
	afx_msg void OnRenderAttackstance();
	afx_msg void OnUpdateRenderAttackstance(CCmdUI *pCmdUI);
	afx_msg void OnRenderResetcamera();
	afx_msg void OnUpdateRenderResetcamera(CCmdUI *pCmdUI);
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
	afx_msg void OnEditMatrixMove();
	afx_msg void OnUpdateEditMatrixMove(CCmdUI *pCmdUI);
	afx_msg void OnEditMatrixRotate();
	afx_msg void OnUpdateEditMatrixRotate(CCmdUI *pCmdUI);
	afx_msg void OnEditMatrixScale();
	afx_msg void OnUpdateEditMatrixScale(CCmdUI *pCmdUI);
};

//#ifndef _DEBUG  // debug version in EditorSkinCharView.cpp
//inline CEditorSkinCharDoc* CEditorSkinCharView::GetDocument() const
//   { return reinterpret_cast<CEditorSkinCharDoc*>(m_pDocument); }
//#endif

