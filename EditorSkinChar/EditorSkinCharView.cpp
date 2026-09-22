
// EditorSkinCharView.cpp : implementation of the CEditorSkinCharView class
//

#include "stdafx.h"
#include "EditorSkinChar.h"
#include "EditorSkinCharDoc.h"
#include "EditorSkinCharView.h"
#include "DxViewPort.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern int g_nITEMLEVEL;
extern BOOL g_bCHAR_EDIT_RUN;

// CEditorSkinCharView

IMPLEMENT_DYNCREATE(CEditorSkinCharView, CView)

BEGIN_MESSAGE_MAP(CEditorSkinCharView, CView)
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_COMMAND(ID_RENDER_DEBUGINFO, OnRenderDebuginfo)
	ON_UPDATE_COMMAND_UI(ID_RENDER_DEBUGINFO, OnUpdateRenderDebuginfo)
	ON_COMMAND(ID_RENDER_PLANE, OnRenderPlane)
	ON_UPDATE_COMMAND_UI(ID_RENDER_PLANE, OnUpdateRenderPlane)
	ON_COMMAND(ID_RENDER_AABBBOX, OnRenderAabbbox)
	ON_UPDATE_COMMAND_UI(ID_RENDER_AABBBOX, OnUpdateRenderAabbbox)
	ON_COMMAND(ID_RENDER_SKELETON, OnRenderSkeleton)
	ON_UPDATE_COMMAND_UI(ID_RENDER_SKELETON, OnUpdateRenderSkeleton)
	ON_COMMAND(ID_RENDER_ATTACKSTANCE, OnRenderAttackstance)
	ON_UPDATE_COMMAND_UI(ID_RENDER_ATTACKSTANCE, OnUpdateRenderAttackstance)
	ON_COMMAND(ID_RENDER_RESETCAMERA, OnRenderResetcamera)
	ON_UPDATE_COMMAND_UI(ID_RENDER_RESETCAMERA, OnUpdateRenderResetcamera)
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
	ON_COMMAND(ID_EDIT_MATRIX_MOVE, OnEditMatrixMove)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MATRIX_MOVE, OnUpdateEditMatrixMove)
	ON_COMMAND(ID_EDIT_MATRIX_ROTATE, OnEditMatrixRotate)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MATRIX_ROTATE, OnUpdateEditMatrixRotate)
	ON_COMMAND(ID_EDIT_MATRIX_SCALE, OnEditMatrixScale)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MATRIX_SCALE, OnUpdateEditMatrixScale)
END_MESSAGE_MAP()

// CEditorSkinCharView construction/destruction

CEditorSkinCharView::CEditorSkinCharView()
	: CD3DApplication()
	, m_bCreated(FALSE)
	, m_hCursorDefault(NULL)
	, m_pSkinChar( NULL )
	, m_pSkinCharData( NULL )

	/*ABL system, Juver, 2017/05/30 */
	, m_pAttBoneData( NULL )
	, m_pAttBoneRender( NULL )
	, m_pAttBoneLink( NULL )
	, m_pAttBone( NULL )
	, m_pMaskPiece(NULL)

	/*vehicle system, Juver, 2017/08/05 */
	, m_pVehicleData(NULL)
	, m_pVehicle(NULL)
	, m_pVehicleSkin(NULL)
	, m_bEditBack(FALSE)

	, m_bRenderDebug( TRUE )
	, m_bRenderPlane( TRUE )
	, m_bRenderAABB( FALSE )
	, m_bRenderSkeleton( TRUE )

	, m_bEDITSMAT(FALSE)
{
	memset( m_szAppPath, 0, sizeof(char) * (MAX_PATH) );
	m_bUseDepthBuffer = TRUE;
	D3DXMatrixIdentity ( &m_matLocal );

	g_nITEMLEVEL = 0;
	g_bCHAR_EDIT_RUN = TRUE;
}

CEditorSkinCharView::~CEditorSkinCharView()
{
	SAFE_DELETE( m_pSkinChar );

	/*ABL system, Juver, 2017/06/02 */
	SAFE_DELETE( m_pAttBone );
	SAFE_DELETE( m_pAttBoneRender );

	/*vehicle system, Juver, 2017/08/05 */
	SAFE_DELETE( m_pVehicle );
	SAFE_DELETE( m_pVehicleSkin );
}

BOOL CEditorSkinCharView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CEditorSkinCharView drawing

void CEditorSkinCharView::OnDraw(CDC* /*pDC*/)
{
	//CEditorSkinCharDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc);
	//if (!pDoc)
	//	return;

	// TODO: add draw code for native data here
}


// CEditorSkinCharView diagnostics

#ifdef _DEBUG
void CEditorSkinCharView::AssertValid() const
{
	CView::AssertValid();
}

void CEditorSkinCharView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

//CEditorSkinCharDoc* CEditorSkinCharView::GetDocument() const // non-debug version is inline
//{
//	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CEditorSkinCharDoc)));
//	return (CEditorSkinCharDoc*)m_pDocument;
//}
#endif //_DEBUG


// CEditorSkinCharView message handlers
CEditorSkinCharView * CEditorSkinCharView::GetView()
{
	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);

	CView * pView = pFrame->GetActiveView();

	if ( !pView )
		return NULL;

	// Fail if view is of wrong kind
	// (this could occur with splitter windows, or additional
	// views on a single document
	if ( ! pView->IsKindOf( RUNTIME_CLASS(CEditorSkinCharView) ) )
		return NULL;

	return (CEditorSkinCharView *) pView;
}

void CEditorSkinCharView::OnRenderDebuginfo()
{
	m_bRenderDebug = !m_bRenderDebug;
}

void CEditorSkinCharView::OnUpdateRenderDebuginfo(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
		pCmdUI->SetCheck( m_bRenderDebug );
}

void CEditorSkinCharView::OnRenderPlane()
{
	m_bRenderPlane = !m_bRenderPlane;
}

void CEditorSkinCharView::OnUpdateRenderPlane(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
		pCmdUI->SetCheck( m_bRenderPlane );
}

void CEditorSkinCharView::OnRenderAabbbox()
{
	m_bRenderAABB = !m_bRenderAABB;
}

void CEditorSkinCharView::OnUpdateRenderAabbbox(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
		pCmdUI->SetCheck( m_bRenderAABB );
}

void CEditorSkinCharView::OnRenderSkeleton()
{
	m_bRenderSkeleton = !m_bRenderSkeleton;
}

void CEditorSkinCharView::OnUpdateRenderSkeleton(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
		pCmdUI->SetCheck( m_bRenderSkeleton );
}

void CEditorSkinCharView::OnRenderAttackstance()
{
	m_bAniAttack = !m_bAniAttack;
}

void CEditorSkinCharView::OnUpdateRenderAttackstance(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( m_bAniAttack );
}

void CEditorSkinCharView::OnRenderResetcamera()
{
	D3DXVECTOR3 vFromPt		= D3DXVECTOR3( 0.0f, 70.0f, -70.0f );
	D3DXVECTOR3 vLookatPt	= D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vUpVec		= D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	DxViewPort::GetInstance().SetViewTrans ( vFromPt, vLookatPt, vUpVec );
	DxViewPort::GetInstance().MoveVelocityReset();
	DxViewPort::GetInstance().CameraJump ( D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) );
}

void CEditorSkinCharView::OnUpdateRenderResetcamera(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}

void CEditorSkinCharView::OnItemlevelOff()
{
	g_nITEMLEVEL = 0;
}

void CEditorSkinCharView::OnUpdateItemlevelOff(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 0 );
}

void CEditorSkinCharView::OnItemlevel01()
{
	g_nITEMLEVEL = 1;
}

void CEditorSkinCharView::OnUpdateItemlevel01(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 1 );
}

void CEditorSkinCharView::OnItemlevel02()
{
	g_nITEMLEVEL = 2;
}

void CEditorSkinCharView::OnUpdateItemlevel02(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 2 );
}

void CEditorSkinCharView::OnItemlevel03()
{
	g_nITEMLEVEL = 3;
}

void CEditorSkinCharView::OnUpdateItemlevel03(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 3 );
}

void CEditorSkinCharView::OnItemlevel04()
{
	g_nITEMLEVEL = 4;
}

void CEditorSkinCharView::OnUpdateItemlevel04(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 4 );
}

void CEditorSkinCharView::OnItemlevel05()
{
	g_nITEMLEVEL = 5;
}

void CEditorSkinCharView::OnUpdateItemlevel05(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 5 );
}

void CEditorSkinCharView::OnItemlevel06()
{
	g_nITEMLEVEL = 6;
}

void CEditorSkinCharView::OnUpdateItemlevel06(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 6 );
}

void CEditorSkinCharView::OnItemlevel07()
{
	g_nITEMLEVEL = 7;
}

void CEditorSkinCharView::OnUpdateItemlevel07(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 7 );
}

void CEditorSkinCharView::OnItemlevel08()
{
	g_nITEMLEVEL = 8;
}

void CEditorSkinCharView::OnUpdateItemlevel08(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 8 );
}

void CEditorSkinCharView::OnItemlevel09()
{
	g_nITEMLEVEL = 9;
}

void CEditorSkinCharView::OnUpdateItemlevel09(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 9 );
}

void CEditorSkinCharView::OnItemlevel10()
{
	g_nITEMLEVEL = 10;
}

void CEditorSkinCharView::OnUpdateItemlevel10(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 10 );
}

void CEditorSkinCharView::OnItemlevel11()
{
	g_nITEMLEVEL = 11;
}

void CEditorSkinCharView::OnUpdateItemlevel11(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 11 );
}

void CEditorSkinCharView::ResetMouseEdit()
{
	m_bEDITSMAT = FALSE;

	m_DxEditMat.SetFrameMatrix( m_matLocal );
	m_DxEditMat.SetAffineMatrix( NULL );
}

void CEditorSkinCharView::EditSmatSet( BOOL bENABLE )
{
	ResetMouseEdit();
	m_bEDITSMAT = bENABLE;
}

void CEditorSkinCharView::OnEditMatrixMove()
{
	m_DxEditMat.SetType( TRANS_MAT );
}

void CEditorSkinCharView::OnUpdateEditMatrixMove(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->Enable(m_bEDITSMAT);
		pCmdUI->SetCheck( m_bEDITSMAT && m_DxEditMat.GetType() == TRANS_MAT );
	}
}

void CEditorSkinCharView::OnEditMatrixRotate()
{
	m_DxEditMat.SetType( ROTATE_MAT );
}

void CEditorSkinCharView::OnUpdateEditMatrixRotate(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->Enable(m_bEDITSMAT);
		pCmdUI->SetCheck( m_bEDITSMAT && m_DxEditMat.GetType() == ROTATE_MAT );
	}
}

void CEditorSkinCharView::OnEditMatrixScale()
{
	m_DxEditMat.SetType( SCALE_MAT );
}

void CEditorSkinCharView::OnUpdateEditMatrixScale(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->Enable(m_bEDITSMAT);
		pCmdUI->SetCheck( m_bEDITSMAT && m_DxEditMat.GetType() == SCALE_MAT );
	}
}
