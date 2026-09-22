
// GameViewerView.h : interface of the CGameViewerView class
//
#include "D3DApp.h"

#include "DxSkinChar.h"
#include "DxEffSingle.h"

/*ABL system, Juver, 2017/06/02 */
#include "DxAttBone.h"
#include "DxAttBoneRender.h"

#include "DxPieceContainer.h"

/*vehicle system, Juver, 2017/08/05 */
#include "DxVehicle.h"
#include "DxVehicleData.h"

#pragma once


class CGameViewerView : public CView, public CD3DApplication
{
protected: // create from serialization only
	CGameViewerView();
	DECLARE_DYNCREATE(CGameViewerView)

// Attributes
public:
	//CGameViewerDoc* GetDocument() const;
	static	CGameViewerView * GetView();

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CGameViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	D3DXMATRIX		m_matLocal;
	DxSkinChar*		m_pSkinChar;
	DxEffSingleGroup* m_pEffSingle;
	EFF_PROPGROUP*  m_pPropGroup;

	/*ABL system, Juver, 2017/06/02 */
	DxAttBone*			m_pAttBone;
	DxAttBoneRender*	m_pAttBoneRender;
	DxSkinPiece*		m_pMaskPiece;

	/*vehicle system, Juver, 2017/08/05 */
	DxVehicle*			m_pVehicle;

	DxPieceEdit*		m_pPIE;

public:
	BOOL	m_bRenderDebug;
	BOOL	m_bRenderPlane;
	BOOL	m_bRenderAABB;
	BOOL	m_bRenderSkeleton;

	BOOL	m_bAniAttack;
	BOOL	m_bAniLoop;
	DWORD	m_dwANIPLAY;

	/*vehicle system, Juver, 2017/08/07 */
	BOOL	m_bVCFFront;

public:
	void	DataReset();
	BOOL	DataLoadCHF( std::string strFILE );

	/*ABL system, Juver, 2017/06/02 */
	BOOL	DataLoadABF( std::string strFILE );
	BOOL	DataLoadABL( std::string strFILE );

	BOOL	DataLoadEGP( std::string strFILE );
	BOOL	DataSaveEGP( std::string strFILE );

	BOOL	DataLoadPIE( std::string strFILE );
	BOOL	DataSavePIE( std::string strFILE );

	/*vehicle system, Juver, 2017/08/05 */
	BOOL	DataLoadVCF( std::string strFILE );

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

	afx_msg void OnRenderDebug();
	afx_msg void OnUpdateRenderDebug(CCmdUI *pCmdUI);
	afx_msg void OnRenderPlane();
	afx_msg void OnUpdateRenderPlane(CCmdUI *pCmdUI);
	afx_msg void OnRenderAabb();
	afx_msg void OnUpdateRenderAabb(CCmdUI *pCmdUI);
	afx_msg void OnRenderSkeleton();
	afx_msg void OnUpdateRenderSkeleton(CCmdUI *pCmdUI);
	afx_msg void OnRenderResetcamera();
	
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
	afx_msg void OnSettingAttackpos();
	afx_msg void OnUpdateSettingAttackpos(CCmdUI *pCmdUI);
	afx_msg void OnSettingLoopplay();
	afx_msg void OnUpdateSettingLoopplay(CCmdUI *pCmdUI);
	afx_msg void OnRenderVcffront();
	afx_msg void OnUpdateRenderVcffront(CCmdUI *pCmdUI);
	afx_msg void OnRenderVcfback();
	afx_msg void OnUpdateRenderVcfback(CCmdUI *pCmdUI);
};

//#ifndef _DEBUG  // debug version in GameViewerView.cpp
//inline CGameViewerDoc* CGameViewerView::GetDocument() const
//   { return reinterpret_cast<CGameViewerDoc*>(m_pDocument); }
//#endif

