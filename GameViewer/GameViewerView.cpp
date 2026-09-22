
// GameViewerView.cpp : implementation of the CGameViewerView class
//

#include "stdafx.h"
#include "GameViewer.h"

#include "GameViewerDoc.h"
#include "GameViewerView.h"
#include "DxViewPort.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern int g_nITEMLEVEL;
extern BOOL g_bCHAR_EDIT_RUN;

extern BOOL g_bEffectAABBBox;
extern BOOL g_bEffectTool;

// CGameViewerView

IMPLEMENT_DYNCREATE(CGameViewerView, CView)

BEGIN_MESSAGE_MAP(CGameViewerView, CView)
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_COMMAND(ID_RENDER_DEBUG, OnRenderDebug)
	ON_UPDATE_COMMAND_UI(ID_RENDER_DEBUG, OnUpdateRenderDebug)
	ON_COMMAND(ID_RENDER_PLANE, OnRenderPlane)
	ON_UPDATE_COMMAND_UI(ID_RENDER_PLANE, OnUpdateRenderPlane)
	ON_COMMAND(ID_RENDER_AABB, OnRenderAabb)
	ON_UPDATE_COMMAND_UI(ID_RENDER_AABB, OnUpdateRenderAabb)
	ON_COMMAND(ID_RENDER_SKELETON, OnRenderSkeleton)
	ON_UPDATE_COMMAND_UI(ID_RENDER_SKELETON, OnUpdateRenderSkeleton)
	ON_COMMAND(ID_RENDER_RESETCAMERA, OnRenderResetcamera)
	
	ON_COMMAND(ID_ITEMLEVEL_OFF, OnItemlevelOff)
	ON_UPDATE_COMMAND_UI(ID_ITEMLEVEL_OFF, OnUpdateItemlevelOff)
	ON_COMMAND(ID_ITEMLEVEL_01, OnItemlevel01)
	ON_UPDATE_COMMAND_UI(ID_ITEMLEVEL_01, OnUpdateItemlevel01)
	ON_COMMAND(ID_ITEMLEVEL_02, OnItemlevel02)
	ON_UPDATE_COMMAND_UI(ID_ITEMLEVEL_02, OnUpdateItemlevel02)
	ON_COMMAND(ID_ITEMLEVEL_03, OnItemlevel03)
	ON_UPDATE_COMMAND_UI(ID_ITEMLEVEL_03, OnUpdateItemlevel03)
	ON_COMMAND(ID_ITEMLEVEL_04, OnItemlevel04)
	ON_UPDATE_COMMAND_UI(ID_ITEMLEVEL_04, OnUpdateItemlevel04)
	ON_COMMAND(ID_ITEMLEVEL_05, OnItemlevel05)
	ON_UPDATE_COMMAND_UI(ID_ITEMLEVEL_05, OnUpdateItemlevel05)
	ON_COMMAND(ID_ITEMLEVEL_06, OnItemlevel06)
	ON_UPDATE_COMMAND_UI(ID_ITEMLEVEL_06, OnUpdateItemlevel06)
	ON_COMMAND(ID_ITEMLEVEL_07, OnItemlevel07)
	ON_UPDATE_COMMAND_UI(ID_ITEMLEVEL_07, OnUpdateItemlevel07)
	ON_COMMAND(ID_ITEMLEVEL_08, OnItemlevel08)
	ON_UPDATE_COMMAND_UI(ID_ITEMLEVEL_08, OnUpdateItemlevel08)
	ON_COMMAND(ID_ITEMLEVEL_09, OnItemlevel09)
	ON_UPDATE_COMMAND_UI(ID_ITEMLEVEL_09, OnUpdateItemlevel09)
	ON_COMMAND(ID_ITEMLEVEL_10, OnItemlevel10)
	ON_UPDATE_COMMAND_UI(ID_ITEMLEVEL_10, OnUpdateItemlevel10)
	ON_COMMAND(ID_ITEMLEVEL_11, OnItemlevel11)
	ON_UPDATE_COMMAND_UI(ID_ITEMLEVEL_11, OnUpdateItemlevel11)
	ON_COMMAND(ID_SETTING_ATTACKPOS, OnSettingAttackpos)
	ON_UPDATE_COMMAND_UI(ID_SETTING_ATTACKPOS, OnUpdateSettingAttackpos)
	ON_COMMAND(ID_SETTING_LOOPPLAY, OnSettingLoopplay)
	ON_UPDATE_COMMAND_UI(ID_SETTING_LOOPPLAY, OnUpdateSettingLoopplay)
	ON_COMMAND(ID_RENDER_VCFFRONT, OnRenderVcffront)
	ON_UPDATE_COMMAND_UI(ID_RENDER_VCFFRONT, OnUpdateRenderVcffront)
	ON_COMMAND(ID_RENDER_VCFBACK, OnRenderVcfback)
	ON_UPDATE_COMMAND_UI(ID_RENDER_VCFBACK, OnUpdateRenderVcfback)
END_MESSAGE_MAP()

// CGameViewerView construction/destruction

CGameViewerView::CGameViewerView()
	: CD3DApplication()
	, m_bCreated(FALSE)
	, m_hCursorDefault(NULL)
	, m_pSkinChar( NULL )
	, m_pEffSingle( NULL )
	, m_pPropGroup( NULL )
	, m_bRenderDebug( TRUE )
	, m_bRenderPlane( TRUE )
	, m_bRenderAABB( FALSE )
	, m_bRenderSkeleton( FALSE )
	, m_bAniAttack( TRUE )
	, m_bAniLoop( FALSE )
	, m_dwANIPLAY( 0 )
	, m_pPIE(NULL)

	/*ABL system, Juver, 2017/06/02 */
	, m_pAttBone(NULL)
	, m_pAttBoneRender(NULL)
	, m_pMaskPiece(NULL)

	/*vehicle system, Juver, 2017/08/05 */
	, m_pVehicle(NULL)
	, m_bVCFFront(TRUE)
{
	memset( m_szAppPath, 0, sizeof(char) * (MAX_PATH) );
	m_bUseDepthBuffer = TRUE;
	D3DXMatrixIdentity ( &m_matLocal );

	g_nITEMLEVEL = 0;
	g_bCHAR_EDIT_RUN = TRUE;

	g_bEffectAABBBox = m_bRenderAABB;
	g_bEffectTool = TRUE;
}

CGameViewerView::~CGameViewerView()
{
	if ( m_pSkinChar )	
	{
		m_pSkinChar->ClearAll();
		SAFE_DELETE ( m_pSkinChar );
	}

	/*ABL system, Juver, 2017/06/02 */
	if ( m_pAttBone )	
	{
		m_pAttBone->ClearAll();
		SAFE_DELETE ( m_pAttBone );
	}

	/*ABL system, Juver, 2017/06/02 */
	if ( m_pAttBoneRender )	
	{
		m_pAttBoneRender->ClearAll();
		SAFE_DELETE ( m_pAttBoneRender );
	}

	if ( m_pEffSingle )
	{
		m_pEffSingle->CleanUp();
		SAFE_DELETE ( m_pEffSingle );
	}

	if ( m_pPIE )
	{
		m_pPIE->CleanUp();
		SAFE_DELETE(m_pPIE);
	}

	/*vehicle system, Juver, 2017/08/06 */
	if ( m_pVehicle )
	{
		m_pVehicle->ClearAll();
		SAFE_DELETE( m_pVehicle );
	}

	m_pPropGroup = NULL;
}

BOOL CGameViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CGameViewerView drawing

void CGameViewerView::OnDraw(CDC* /*pDC*/)
{
	//CGameViewerDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc);
	//if (!pDoc)
	//	return;

	// TODO: add draw code for native data here
}


// CGameViewerView diagnostics

#ifdef _DEBUG
void CGameViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CGameViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

// CGameViewerDoc* CGameViewerView::GetDocument() const // non-debug version is inline
// {
// 	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGameViewerDoc)));
// 	return (CGameViewerDoc*)m_pDocument;
// }
#endif //_DEBUG


// CGameViewerView message handlers
CGameViewerView * CGameViewerView::GetView()
{
	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);

	CView * pView = pFrame->GetActiveView();

	if ( !pView )
		return NULL;

	// Fail if view is of wrong kind
	// (this could occur with splitter windows, or additional
	// views on a single document
	if ( ! pView->IsKindOf( RUNTIME_CLASS(CGameViewerView) ) )
		return NULL;

	return (CGameViewerView *) pView;
}

void CGameViewerView::OnRenderDebug()
{
	m_bRenderDebug = !m_bRenderDebug;
}

void CGameViewerView::OnUpdateRenderDebug(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->SetCheck( m_bRenderDebug );
	}
}

void CGameViewerView::OnRenderPlane()
{
	m_bRenderPlane = !m_bRenderPlane;
}

void CGameViewerView::OnUpdateRenderPlane(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->SetCheck( m_bRenderPlane );
	}
}

void CGameViewerView::OnRenderAabb()
{
	m_bRenderAABB = !m_bRenderAABB;
	g_bEffectAABBBox = m_bRenderAABB;
}

void CGameViewerView::OnUpdateRenderAabb(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->SetCheck( m_bRenderAABB );
	}
}

void CGameViewerView::OnRenderSkeleton()
{
	m_bRenderSkeleton = !m_bRenderSkeleton;
}

void CGameViewerView::OnUpdateRenderSkeleton(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->SetCheck( m_bRenderSkeleton );
	}
}

void CGameViewerView::OnRenderResetcamera()
{
	D3DXVECTOR3 vFromPt		= D3DXVECTOR3( 0.0f, 70.0f, -70.0f );
	D3DXVECTOR3 vLookatPt	= D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vUpVec		= D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	DxViewPort::GetInstance().SetViewTrans ( vFromPt, vLookatPt, vUpVec );
	DxViewPort::GetInstance().MoveVelocityReset();
	DxViewPort::GetInstance().CameraJump ( D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) );
}


void CGameViewerView::OnItemlevelOff()
{
	g_nITEMLEVEL = 0;
}

void CGameViewerView::OnUpdateItemlevelOff(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 0 );
}

void CGameViewerView::OnItemlevel01()
{
	g_nITEMLEVEL = 1;
}

void CGameViewerView::OnUpdateItemlevel01(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 1 );
}

void CGameViewerView::OnItemlevel02()
{
	g_nITEMLEVEL = 2;
}

void CGameViewerView::OnUpdateItemlevel02(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 2 );
}

void CGameViewerView::OnItemlevel03()
{
	g_nITEMLEVEL = 3;
}

void CGameViewerView::OnUpdateItemlevel03(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 3 );
}

void CGameViewerView::OnItemlevel04()
{
	g_nITEMLEVEL = 4;
}

void CGameViewerView::OnUpdateItemlevel04(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 4 );
}

void CGameViewerView::OnItemlevel05()
{
	g_nITEMLEVEL = 5;
}

void CGameViewerView::OnUpdateItemlevel05(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 5 );
}

void CGameViewerView::OnItemlevel06()
{
	g_nITEMLEVEL = 6;
}

void CGameViewerView::OnUpdateItemlevel06(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 6 );
}

void CGameViewerView::OnItemlevel07()
{
	g_nITEMLEVEL = 7;
}

void CGameViewerView::OnUpdateItemlevel07(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 7 );
}

void CGameViewerView::OnItemlevel08()
{
	g_nITEMLEVEL = 8;
}

void CGameViewerView::OnUpdateItemlevel08(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 8 );
}

void CGameViewerView::OnItemlevel09()
{
	g_nITEMLEVEL = 9;
}

void CGameViewerView::OnUpdateItemlevel09(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 9 );
}

void CGameViewerView::OnItemlevel10()
{
	g_nITEMLEVEL = 10;
}

void CGameViewerView::OnUpdateItemlevel10(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 10 );
}

void CGameViewerView::OnItemlevel11()
{
	g_nITEMLEVEL = 11;
}

void CGameViewerView::OnUpdateItemlevel11(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 11 );
}

void CGameViewerView::OnSettingAttackpos()
{
	m_bAniAttack = !m_bAniAttack;
}

void CGameViewerView::OnUpdateSettingAttackpos(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( m_bAniAttack );
}

void CGameViewerView::OnSettingLoopplay()
{
	m_bAniLoop = !m_bAniLoop;
}

void CGameViewerView::OnUpdateSettingLoopplay(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( m_bAniLoop );
}

void CGameViewerView::OnRenderVcffront()
{
	m_bVCFFront = TRUE;
}

void CGameViewerView::OnUpdateRenderVcffront(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( m_bVCFFront );
}

void CGameViewerView::OnRenderVcfback()
{
	m_bVCFFront = FALSE;
}

void CGameViewerView::OnUpdateRenderVcfback(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( !m_bVCFFront );
}
