
// EditorSkinPieceView.cpp : implementation of the CEditorSkinPieceView class
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "MainFrm.h"
#include "EditorSkinPieceDoc.h"
#include "EditorSkinPieceView.h"
#include "SheetWithTab.h"

#include "DxViewPort.h"
#include "DxSkinMeshMan.h"

/*Item Self Effect, Juver, 2017/06/09 */
#include "DxEffSelfEffect.h" 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern int g_nITEMLEVEL;
extern BOOL g_bCHAR_EDIT_RUN;
extern std::string	g_strPICK_BONE;

// CEditorSkinPieceView

IMPLEMENT_DYNCREATE(CEditorSkinPieceView, CView)

BEGIN_MESSAGE_MAP(CEditorSkinPieceView, CView)
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_RENDER_RENDERTRACE, OnRenderRendertrace)
	ON_UPDATE_COMMAND_UI(ID_RENDER_RENDERTRACE, OnUpdateRenderRendertrace)
	ON_COMMAND(ID_RENDER_RENDERPLANE, OnRenderRenderplane)
	ON_UPDATE_COMMAND_UI(ID_RENDER_RENDERPLANE, OnUpdateRenderRenderplane)
	ON_COMMAND(ID_COLLISIONTYPE_TYPE1, OnCollisiontypeType1)
	ON_UPDATE_COMMAND_UI(ID_COLLISIONTYPE_TYPE1, OnUpdateCollisiontypeType1)
	ON_COMMAND(ID_COLLISIONTYPE_TYPE2, OnCollisiontypeType2)
	ON_UPDATE_COMMAND_UI(ID_COLLISIONTYPE_TYPE2, OnUpdateCollisiontypeType2)
	ON_COMMAND(ID_COLLISIONTYPE_TYPE3, OnCollisiontypeType3)
	ON_UPDATE_COMMAND_UI(ID_COLLISIONTYPE_TYPE3, OnUpdateCollisiontypeType3)
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
	ON_COMMAND(ID_RENDER_RENDERDEBUGINFO, OnRenderRenderdebuginfo)
	ON_UPDATE_COMMAND_UI(ID_RENDER_RENDERDEBUGINFO, OnUpdateRenderRenderdebuginfo)
	ON_COMMAND(ID_CAMERA_RESETCAMERAPOS, OnCameraResetcamerapos)
	ON_COMMAND(ID_COLLISIONTYPE_TYPE_1, OnCollisiontypeType1)
	ON_UPDATE_COMMAND_UI(ID_COLLISIONTYPE_TYPE_1, OnUpdateCollisiontypeType1)
	ON_COMMAND(ID_COLLISIONTYPE_TYPE_2, OnCollisiontypeType2)
	ON_UPDATE_COMMAND_UI(ID_COLLISIONTYPE_TYPE_2, OnUpdateCollisiontypeType2)
	ON_COMMAND(ID_COLLISIONTYPE_TYPE_3, OnCollisiontypeType3)
	ON_UPDATE_COMMAND_UI(ID_COLLISIONTYPE_TYPE_3, OnUpdateCollisiontypeType3)
	
	ON_COMMAND(ID_EDIT_MATRIX_MOVE, OnEditMatrixMove)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MATRIX_MOVE, OnUpdateEditMatrixMove)
	ON_COMMAND(ID_EDIT_MATRIX_ROTATE, OnEditMatrixRotate)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MATRIX_ROTATE, OnUpdateEditMatrixRotate)
	ON_COMMAND(ID_EDIT_MATRIX_SCALE, OnEditMatrixScale)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MATRIX_SCALE, OnUpdateEditMatrixScale)
	ON_COMMAND(ID_FILE_SAVEEFFSKIN, OnFileSaveeffskin)
END_MESSAGE_MAP()

// CEditorSkinPieceView construction/destruction

CEditorSkinPieceView::CEditorSkinPieceView()
	: CD3DApplication()
	, m_bCreated(FALSE)
	, m_hCursorDefault(NULL)
	, m_strFileName( "" )
	, m_pPiece( NULL )
	, m_vCOL( 0.0f, 0.0f, 0.0f )
	, m_dwCOL(COL_ERR)
	, m_emCOL( EMCOLLISION_TYPE_ISCOL )
	, m_bRenderBone( FALSE )
	, m_strTraceSelect( "" )
	, m_bRenderPlane( TRUE )
	, m_bRenderTrace( TRUE )
	, m_bRenderDebug( TRUE )
	, m_bEditMRS( FALSE )
{
	memset( m_szAppPath, 0, sizeof(char) * (MAX_PATH) );
	m_bUseDepthBuffer = TRUE;
	D3DXMatrixIdentity ( &m_matLocal );

	g_nITEMLEVEL = 0;
	g_bCHAR_EDIT_RUN = TRUE;
	g_strPICK_BONE = "";

}

CEditorSkinPieceView::~CEditorSkinPieceView()
{
	SAFE_DELETE( m_pPiece );
}

BOOL CEditorSkinPieceView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CEditorSkinPieceView drawing

void CEditorSkinPieceView::OnDraw(CDC* /*pDC*/)
{
	//CEditorSkinPieceDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc);
	//if (!pDoc)
	//	return;

	// TODO: add draw code for native data here
}


// CEditorSkinPieceView diagnostics

#ifdef _DEBUG
void CEditorSkinPieceView::AssertValid() const
{
	CView::AssertValid();
}

void CEditorSkinPieceView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

//CEditorSkinPieceDoc* CEditorSkinPieceView::GetDocument() const // non-debug version is inline
//{
//	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CEditorSkinPieceDoc)));
//	return (CEditorSkinPieceDoc*)m_pDocument;
//}
#endif //_DEBUG


// CEditorSkinPieceView message handlers
CEditorSkinPieceView * CEditorSkinPieceView::GetView()
{
	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);

	CView * pView = pFrame->GetActiveView();

	if ( !pView )
		return NULL;

	// Fail if view is of wrong kind
	// (this could occur with splitter windows, or additional
	// views on a single document
	if ( ! pView->IsKindOf( RUNTIME_CLASS(CEditorSkinPieceView) ) )
		return NULL;

	return (CEditorSkinPieceView *) pView;
}

void CEditorSkinPieceView::PieceNew( std::string strFILENAME )
{
	if ( m_pPiece )
	{
		m_pPiece->ClearAll();
		m_pPiece->InvalidateDeviceObjects();
		m_pPiece->DeleteDeviceObjects();
	}

	SAFE_DELETE( m_pPiece );
	m_strFileName = strFILENAME.c_str();
	DxBoneCollector::GetInstance().CleanUp();
	DxSkinMeshMan::GetInstance().CleanUp( m_pd3dDevice );

	m_pPiece = new DxSkinPiece;
	m_pPiece->InitDeviceObjects( m_pd3dDevice );
	m_vCOL = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_dwCOL = COL_ERR;
	g_strPICK_BONE = "";
	m_strTraceSelect = "";
	m_bEditMRS = FALSE;
	m_DxEditMRS.SetMatrix( NULL );

	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	if ( pFrame )
		pFrame->SetWindowText( m_strFileName.c_str() );

	CMainFrame	*pMainFrame = (CMainFrame*) AfxGetMainWnd();
	if ( pMainFrame )
	{
		CsheetWithTab*  pSheetTab = pMainFrame->m_wndEditor.m_pSheetTab;
		if ( pSheetTab )
			pSheetTab->ShowMain();
	}
}

void CEditorSkinPieceView::OnFileNew()
{
	PieceNew( "New Piece" );
}

void CEditorSkinPieceView::OnFileOpen()
{
	CString szFilterInput = "Piece (*.cps,*.aps,*.vps)|*.cps;*.aps;*.vps|";
	CFileDialog dlgInput(true,"*.*",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilterInput,this);
	dlgInput.m_ofn.lpstrInitialDir = DxSkinPieceContainer::GetInstance().GetPath();
	if ( dlgInput.DoModal() != IDOK ) return;

	if ( m_pPiece )
	{
		m_pPiece->ClearAll();
		m_pPiece->InvalidateDeviceObjects();
		m_pPiece->DeleteDeviceObjects();
	}

	SAFE_DELETE( m_pPiece );

	m_strFileName = dlgInput.GetFileName();
	DxBoneCollector::GetInstance().CleanUp();
	DxSkinMeshMan::GetInstance().CleanUp( m_pd3dDevice );

	m_pPiece = new DxSkinPiece;
	m_pPiece->InitDeviceObjects( m_pd3dDevice );
	HRESULT hr = m_pPiece->LoadPiece ( m_strFileName.c_str(), m_pd3dDevice, false/*, false*/ );
	if ( FAILED( hr ) )
	{
		SAFE_DELETE( m_pPiece );
		CDebugSet::MsgBox( "Error Loading Piece file :%s", m_strFileName.c_str() );
		m_strFileName = "";
	}

	m_vCOL = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_dwCOL = COL_ERR;
	g_strPICK_BONE = "";
	m_strTraceSelect = "";
	m_bEditMRS = FALSE;
	m_DxEditMRS.SetMatrix( NULL );

	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	if ( pFrame )
		pFrame->SetWindowText( m_strFileName.c_str() );

	CMainFrame	*pMainFrame = (CMainFrame*) AfxGetMainWnd();
	if ( pMainFrame )
	{
		CsheetWithTab*  pSheetTab = pMainFrame->m_wndEditor.m_pSheetTab;
		if ( pSheetTab )
			pSheetTab->ShowMain();
	}
}

void CEditorSkinPieceView::OnFileSave()
{
	if ( m_pPiece )
	{
		CString szFilterInput = "Piece (*.cps,*.aps,*.vps)|*.cps;*.aps;*.vps|";
		CFileDialog dlgInput(false,"*.*", m_pPiece->m_szFileName ,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilterInput,this);
		dlgInput.m_ofn.lpstrInitialDir = DxSkinPieceContainer::GetInstance().GetPath();
		if ( dlgInput.DoModal() != IDOK ) return;

		HRESULT hr = m_pPiece->SavePiece( dlgInput.GetFileName() );
		if ( FAILED( hr ) )
		{
			CDebugSet::MsgBox( "Error Saving file :%s", dlgInput.GetFileName() );
		}else{
			m_strFileName = dlgInput.GetFileName();
			CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
			if ( pFrame )
				pFrame->SetWindowText( m_strFileName.c_str() );

			CMainFrame	*pMainFrame = (CMainFrame*) AfxGetMainWnd();
			if ( pMainFrame )
			{
				CsheetWithTab*  pSheetTab = pMainFrame->m_wndEditor.m_pSheetTab;
				if ( pSheetTab )
				{
					if ( pSheetTab->GetActivePage() == &pSheetTab->m_PageMain )
					{
						pSheetTab->ShowMain();
					}
				}
			}
		}
	}
}

void CEditorSkinPieceView::OnRenderRendertrace()
{
	m_bRenderTrace = !m_bRenderTrace;
}

void CEditorSkinPieceView::OnUpdateRenderRendertrace(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( m_bRenderTrace );
}

void CEditorSkinPieceView::OnRenderRenderplane()
{
	m_bRenderPlane = !m_bRenderPlane;
}

void CEditorSkinPieceView::OnUpdateRenderRenderplane(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( m_bRenderPlane );
}

void CEditorSkinPieceView::OnItemlevelOff()
{
	g_nITEMLEVEL = 0;
}

void CEditorSkinPieceView::OnUpdateItemlevelOff(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 0 );
}

void CEditorSkinPieceView::OnItemlevel01()
{
	g_nITEMLEVEL = 1;
}

void CEditorSkinPieceView::OnUpdateItemlevel01(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 1 );
}

void CEditorSkinPieceView::OnItemlevel02()
{
	g_nITEMLEVEL = 2;
}

void CEditorSkinPieceView::OnUpdateItemlevel02(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 2 );
}

void CEditorSkinPieceView::OnItemlevel03()
{
	g_nITEMLEVEL = 3;
}

void CEditorSkinPieceView::OnUpdateItemlevel03(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 3 );
}

void CEditorSkinPieceView::OnItemlevel04()
{
	g_nITEMLEVEL = 4;
}

void CEditorSkinPieceView::OnUpdateItemlevel04(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 4 );
}

void CEditorSkinPieceView::OnItemlevel05()
{
	g_nITEMLEVEL = 5;
}

void CEditorSkinPieceView::OnUpdateItemlevel05(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 5 );
}

void CEditorSkinPieceView::OnItemlevel06()
{
	g_nITEMLEVEL = 6;
}

void CEditorSkinPieceView::OnUpdateItemlevel06(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 6 );
}

void CEditorSkinPieceView::OnItemlevel07()
{
	g_nITEMLEVEL = 7;
}

void CEditorSkinPieceView::OnUpdateItemlevel07(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 7 );
}

void CEditorSkinPieceView::OnItemlevel08()
{
	g_nITEMLEVEL = 8;
}

void CEditorSkinPieceView::OnUpdateItemlevel08(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 8 );
}

void CEditorSkinPieceView::OnItemlevel09()
{
	g_nITEMLEVEL = 9;
}

void CEditorSkinPieceView::OnUpdateItemlevel09(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 9 );
}

void CEditorSkinPieceView::OnItemlevel10()
{
	g_nITEMLEVEL = 10;
}

void CEditorSkinPieceView::OnUpdateItemlevel10(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 10 );
}

void CEditorSkinPieceView::OnItemlevel11()
{
	g_nITEMLEVEL = 11;
}

void CEditorSkinPieceView::OnUpdateItemlevel11(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( g_nITEMLEVEL == 11 );
}

void CEditorSkinPieceView::OnRenderRenderdebuginfo()
{
	m_bRenderDebug = !m_bRenderDebug;
}

void CEditorSkinPieceView::OnUpdateRenderRenderdebuginfo(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( m_bRenderDebug );
}

void CEditorSkinPieceView::OnCameraResetcamerapos()
{
	D3DXVECTOR3 vFromPt		= D3DXVECTOR3( 0.0f, 70.0f, -70.0f );
	D3DXVECTOR3 vLookatPt	= D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vUpVec		= D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	DxViewPort::GetInstance().SetViewTrans ( vFromPt, vLookatPt, vUpVec );
	DxViewPort::GetInstance().MoveVelocityReset();
	DxViewPort::GetInstance().CameraJump ( D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) );
}

void CEditorSkinPieceView::OnCollisiontypeType1()
{
	m_emCOL = EMCOLLISION_TYPE_ISCOL;
}

void CEditorSkinPieceView::OnUpdateCollisiontypeType1(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( m_emCOL == EMCOLLISION_TYPE_ISCOL );
}

void CEditorSkinPieceView::OnCollisiontypeType2()
{
	m_emCOL = EMCOLLISION_TYPE_CLP1;
}

void CEditorSkinPieceView::OnUpdateCollisiontypeType2(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( m_emCOL == EMCOLLISION_TYPE_CLP1 );
}

void CEditorSkinPieceView::OnCollisiontypeType3()
{
	m_emCOL = EMCOLLISION_TYPE_CLP2;
}

void CEditorSkinPieceView::OnUpdateCollisiontypeType3(CCmdUI *pCmdUI)
{
	if (pCmdUI)
		pCmdUI->SetCheck( m_emCOL == EMCOLLISION_TYPE_CLP2 );
}


void CEditorSkinPieceView::OnEditMatrixMove()
{
	m_DxEditMRS.SetMode( OBECTMRS_M );
}

void CEditorSkinPieceView::OnUpdateEditMatrixMove(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->Enable(m_bEditMRS);
		pCmdUI->SetCheck( m_bEditMRS && m_DxEditMRS.GetMode() == OBECTMRS_M );
	}
}

void CEditorSkinPieceView::OnEditMatrixRotate()
{
	m_DxEditMRS.SetMode( OBECTMRS_R );
}

void CEditorSkinPieceView::OnUpdateEditMatrixRotate(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->Enable(m_bEditMRS);
		pCmdUI->SetCheck( m_bEditMRS && m_DxEditMRS.GetMode() == OBECTMRS_R );
	}
}

void CEditorSkinPieceView::OnEditMatrixScale()
{
	m_DxEditMRS.SetMode( OBECTMRS_S );
}

void CEditorSkinPieceView::OnUpdateEditMatrixScale(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->Enable(m_bEditMRS);
		pCmdUI->SetCheck( m_bEditMRS && m_DxEditMRS.GetMode() == OBECTMRS_S );
	}
}

void CEditorSkinPieceView::OnFileSaveeffskin()
{
	/*Item Self Effect, Juver, 2017/06/09 */

	if ( !m_pPiece )	return;

	CString szFilterInput = "effSkin (*.effskin)|*.effskin|";
	CFileDialog dlgInput(false,"*.*", "",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilterInput,this);
	dlgInput.m_ofn.lpstrInitialDir = DxEffSelfEffectMan::GetInstance().GetPath();
	if ( dlgInput.DoModal() != IDOK ) return;

	DxEffSelfEffect pEff;
	pEff.Import( m_pPiece->m_emType, m_pPiece->m_vecEFFECT );
	BOOL bOK = pEff.SaveFile( dlgInput.GetFileName() );
	if( bOK )
		CDebugSet::MsgBoxAfx( "Save Done %s", dlgInput.GetFileName() );
	else
		CDebugSet::MsgBoxAfx( "Save Failed %s", dlgInput.GetFileName() );
}
