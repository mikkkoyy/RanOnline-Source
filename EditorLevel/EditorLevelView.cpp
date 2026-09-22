
// EditorLevelView.cpp : implementation of the CEditorLevelView class
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "EditorLevelView.h"
#include "MainFrm.h"
#include "SheetWithTab.h"
#include "GLLandMan.h"
#include "DxLandMan.h"
#include "DxSoundMan.h"
#include "BgmSound.h"
#include "DxViewPort.h"
#include "GLMobSchedule.h"

#include "DxSkinCharData.h"
#include "DxReplaceContainer.h"
#include "DxSkinMeshMan.h"
#include "DxEffGroupPlayer.h"
#include "DxPieceContainer.h"

/*ABL system, Juver, 2017/06/02 */
#include "DxAttBone.h"
#include "DxAttBoneData.h"
#include "DxAttBoneLink.h"

/*vehicle system, Juver, 2017/08/04 */
#include "DxVehicle.h"
#include "DxVehicleData.h"
#include ".\editorlevelview.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern BOOL g_bLEVEL_EDIT_RUN;
extern BOOL g_bWORLD_TOOL;

// CEditorLevelView

IMPLEMENT_DYNCREATE(CEditorLevelView, CView)

BEGIN_MESSAGE_MAP(CEditorLevelView, CView)
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_COMMAND(ID_LEVEL_NEW, OnLevelNew)
	ON_COMMAND(ID_LEVEL_OPEN, OnLevelOpen)
	ON_COMMAND(ID_LEVEL_OPENEX, OnLevelOpenex)
	ON_COMMAND(ID_LEVEL_SAVE, OnLevelSave)

	ON_COMMAND(ID_SETTING_RENDER_DEBUGINFO, OnSettingRenderDebuginfo)
	ON_UPDATE_COMMAND_UI(ID_SETTING_RENDER_DEBUGINFO, OnUpdateSettingRenderDebuginfo)
	ON_COMMAND(ID_SETTING_RENDER_CROWMODEL, OnSettingRenderCrowmodel)
	ON_UPDATE_COMMAND_UI(ID_SETTING_RENDER_CROWMODEL, OnUpdateSettingRenderCrowmodel)
	ON_COMMAND(ID_SETTING_RENDER_CROWSCHEDULE, OnSettingRenderCrowschedule)
	ON_UPDATE_COMMAND_UI(ID_SETTING_RENDER_CROWSCHEDULE, OnUpdateSettingRenderCrowschedule)
	ON_COMMAND(ID_SETTING_RENDER_CROWGENPOS, OnSettingRenderCrowgenpos)
	ON_UPDATE_COMMAND_UI(ID_SETTING_RENDER_CROWGENPOS, OnUpdateSettingRenderCrowgenpos)
	ON_COMMAND(ID_SETTING_RENDER_QUADTREE, OnSettingRenderQuadtree)
	ON_UPDATE_COMMAND_UI(ID_SETTING_RENDER_QUADTREE, OnUpdateSettingRenderQuadtree)
	ON_COMMAND(ID_RENDER_RENDER_GATE, OnRenderRenderGate)
	ON_UPDATE_COMMAND_UI(ID_RENDER_RENDER_GATE, OnUpdateRenderRenderGate)
	ON_COMMAND(ID_SETTING_RENDER_LANDEFFECT, OnSettingRenderLandeffect)
	ON_UPDATE_COMMAND_UI(ID_SETTING_RENDER_LANDEFFECT, OnUpdateSettingRenderLandeffect)
	ON_COMMAND(ID_CAMERA_RESETCAMERAPOS, OnCameraResetcamerapos)
	ON_COMMAND(ID_SETTING_CAMERASPEEDDEFAULT, OnSettingCameraspeeddefault)
	ON_UPDATE_COMMAND_UI(ID_SETTING_CAMERASPEEDDEFAULT, OnUpdateSettingCameraspeeddefault)
	ON_COMMAND(ID_SETTING_CAMERASPEED500, OnSettingCameraspeed500)
	ON_UPDATE_COMMAND_UI(ID_SETTING_CAMERASPEED500, OnUpdateSettingCameraspeed500)
	ON_COMMAND(ID_SETTING_CAMERASPEED1000, OnSettingCameraspeed1000)
	ON_UPDATE_COMMAND_UI(ID_SETTING_CAMERASPEED1000, OnUpdateSettingCameraspeed1000)
	ON_COMMAND(ID_SETTING_CAMERASPEED1500, OnSettingCameraspeed1500)
	ON_UPDATE_COMMAND_UI(ID_SETTING_CAMERASPEED1500, OnUpdateSettingCameraspeed1500)
	ON_COMMAND(ID_SETTING_SOUND_BGM, OnSettingSoundBgm)
	ON_UPDATE_COMMAND_UI(ID_SETTING_SOUND_BGM, OnUpdateSettingSoundBgm)
	ON_COMMAND(ID_SETTING_SOUND_ENV, OnSettingSoundEnv)
	ON_UPDATE_COMMAND_UI(ID_SETTING_SOUND_ENV, OnUpdateSettingSoundEnv)
	ON_COMMAND(ID_SETTING_SOUND_SFX, OnSettingSoundSfx)
	ON_UPDATE_COMMAND_UI(ID_SETTING_SOUND_SFX, OnUpdateSettingSoundSfx)
	ON_COMMAND(ID_SETTING_FOGRANGE_DEFAULT, OnSettingFograngeDefault)
	ON_UPDATE_COMMAND_UI(ID_SETTING_FOGRANGE_DEFAULT, OnUpdateSettingFograngeDefault)
	ON_COMMAND(ID_SETTING_FOGRANGE_5K, OnSettingFogrange5k)
	ON_UPDATE_COMMAND_UI(ID_SETTING_FOGRANGE_5K, OnUpdateSettingFogrange5k)
	ON_COMMAND(ID_SETTING_FOGRANGE_10K, OnSettingFogrange10k)
	ON_UPDATE_COMMAND_UI(ID_SETTING_FOGRANGE_10K, OnUpdateSettingFogrange10k)
	ON_COMMAND(ID_SETTING_FOGRANGE_20K, OnSettingFogrange20k)
	ON_UPDATE_COMMAND_UI(ID_SETTING_FOGRANGE_20K, OnUpdateSettingFogrange20k)
	ON_COMMAND(ID_SETTING_FOGRANGE50K, OnSettingFogrange50k)
	ON_UPDATE_COMMAND_UI(ID_SETTING_FOGRANGE50K, OnUpdateSettingFogrange50k)
	ON_COMMAND(ID_CAMERA_JUMPTOSPHERE, OnCameraJumptosphere)
	ON_COMMAND(ID_EDIT_SMATRIX_MOVE, OnEditSmatrixMove)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SMATRIX_MOVE, OnUpdateEditSmatrixMove)
	ON_COMMAND(ID_EDIT_SMATRIX_ROTATE, OnEditSmatrixRotate)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SMATRIX_ROTATE, OnUpdateEditSmatrixRotate)
	ON_COMMAND(ID_EDIT_SMATRIX_SCALE, OnEditSmatrixScale)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SMATRIX_SCALE, OnUpdateEditSmatrixScale)
	ON_COMMAND(ID_EDIT_BOX_MOVE, OnEditBoxMove)
	ON_UPDATE_COMMAND_UI(ID_EDIT_BOX_MOVE, OnUpdateEditBoxMove)
	ON_COMMAND(ID_EDIT_BOX_SCALE, OnEditBoxScale)
	ON_UPDATE_COMMAND_UI(ID_EDIT_BOX_SCALE, OnUpdateEditBoxScale)
	ON_COMMAND(ID_EDIT_BOX_SELSCALE, OnEditBoxSelscale)
	ON_UPDATE_COMMAND_UI(ID_EDIT_BOX_SELSCALE, OnUpdateEditBoxSelscale)
	ON_COMMAND(ID_EDIT_MATRIX_MOVE, OnEditMatrixMove)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MATRIX_MOVE, OnUpdateEditMatrixMove)
	ON_COMMAND(ID_EDIT_MATRIX_ROTATE, OnEditMatrixRotate)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MATRIX_ROTATE, OnUpdateEditMatrixRotate)
	ON_COMMAND(ID_EDIT_MATRIX_SCALE, OnEditMatrixScale)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MATRIX_SCALE, OnUpdateEditMatrixScale)
	ON_COMMAND(ID_DETECT_SELECT_CROW, OnDetectSelectCrow)
	ON_UPDATE_COMMAND_UI(ID_DETECT_SELECT_CROW, OnUpdateDetectSelectCrow)
	ON_COMMAND(ID_DETECT_SELECT_GATE, OnDetectSelectGate)
	ON_UPDATE_COMMAND_UI(ID_DETECT_SELECT_GATE, OnUpdateDetectSelectGate)
	ON_COMMAND(ID_LEVEL_SAVE_WLD, OnLevelSaveWld)
	ON_COMMAND(ID_RENDER_RENDERGROUPLINE, OnRenderRendergroupline)
	ON_UPDATE_COMMAND_UI(ID_RENDER_RENDERGROUPLINE, OnUpdateRenderRendergroupline)
END_MESSAGE_MAP()

// CEditorLevelView construction/destruction

CEditorLevelView::CEditorLevelView()
	: CD3DApplication()
	, m_bCreated(FALSE)
	, m_hCursorDefault(NULL)
	, m_pGLLandServer( NULL )
	, m_pDxLandMan( NULL )
	, m_strFileName( "" )
	, m_bSettingRenderDebugInfo( TRUE )
	, m_bSettingRenderCrowModel( TRUE )
	, m_bSettingRenderCrowSched( TRUE )
	, m_bSettingRenderCrowGenPos( FALSE ) //too slow
	, m_bSettingRenderQuadTree( TRUE )
	, m_bSettingRenderGate( TRUE )
	, m_bSettingRenderLandEffect( TRUE )
	, m_bSettingSoundBGM( TRUE )
	, m_bSettingSoundENV( TRUE )
	, m_bSettingSoundSFX( TRUE )
	, m_bEDITSMAT( FALSE )
	, m_bEDITBOX( FALSE )
	, m_bEDITMRS( FALSE )
	, m_bDETECTCROW( FALSE )
	, m_bDETECTGATE( FALSE )
	, m_vPos( 0.0f, 0.0f, 0.0f )
	, m_pSelectSched( NULL )
	, m_pSelectedGate( NULL )
	, m_bRenderGroupLine(TRUE)
{
	memset( m_szAppPath, 0, sizeof(char) * (MAX_PATH) );
	m_bUseDepthBuffer	= TRUE;

	g_bLEVEL_EDIT_RUN = TRUE;
	g_bWORLD_TOOL = TRUE;
}

CEditorLevelView::~CEditorLevelView()
{
}

BOOL CEditorLevelView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CEditorLevelView drawing

void CEditorLevelView::OnDraw(CDC* /*pDC*/)
{
	//CEditorLevelDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc);
	//if (!pDoc)
	//	return;

	// TODO: add draw code for native data here
}


// CEditorLevelView diagnostics

#ifdef _DEBUG
void CEditorLevelView::AssertValid() const
{
	CView::AssertValid();
}

void CEditorLevelView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

//CEditorLevelDoc* CEditorLevelView::GetDocument() const // non-debug version is inline
//{
//	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CEditorLevelDoc)));
//	return (CEditorLevelDoc*)m_pDocument;
//}
#endif //_DEBUG


// CEditorLevelView message handlers
CEditorLevelView * CEditorLevelView::GetView()
{
	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);

	CView * pView = pFrame->GetActiveView();

	if ( !pView )
		return NULL;

	// Fail if view is of wrong kind
	// (this could occur with splitter windows, or additional
	// views on a single document
	if ( ! pView->IsKindOf( RUNTIME_CLASS(CEditorLevelView) ) )
		return NULL;

	return (CEditorLevelView *) pView;
}

void CEditorLevelView::OnLevelNew()
{
	if ( m_pGLLandServer )
		m_pGLLandServer->CleanUp();

	if ( m_pDxLandMan )
		m_pDxLandMan->CleanUp();

	SAFE_DELETE( m_pGLLandServer );
	SAFE_DELETE( m_pDxLandMan );

	ResetEditor();
	DataCleanUp();

	m_pGLLandServer = new GLLandMan;
	m_pDxLandMan = new DxLandMan;

	m_pGLLandServer->SetD3dDevice( m_pd3dDevice );
	m_pGLLandServer->SetEmulator( TRUE );

	m_pGLLandServer->ReSetMap();
	m_pGLLandServer->CleanUp();
	m_pDxLandMan->CleanUp();

	GLMobSchedule::m_bRENDACT = TRUE;

	m_pGLLandServer->InitDeviceObjects( m_pd3dDevice );
	m_pGLLandServer->RestoreDeviceObjects( m_pd3dDevice );
	m_pDxLandMan->ActiveMap();
	m_pDxLandMan->LoadBasicPos ( m_pd3dDevice, D3DXVECTOR3( 0.0f, 0.0f, 0.0f ));
	m_pDxLandMan->InitDeviceObjects ( m_pd3dDevice );
	m_pDxLandMan->RestoreDeviceObjects ( m_pd3dDevice );
	
	ApplySettings();

	m_strFileName = "";
	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	pFrame->SetWindowText( m_strFileName.c_str() );
}

void CEditorLevelView::OnLevelOpen()
{
	CString szFilter = "lev|*.LEV|";
	CFileDialog dlg ( TRUE,".LEV", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = GLLevelFile::GetLevelPath();
	if ( dlg.DoModal() == IDOK )
	{
		GLLevelFile sLev;
		BOOL bOk = sLev.LoadFile ( dlg.GetFileName(), FALSE, m_pd3dDevice );
		if ( bOk )
		{
			if ( m_pGLLandServer )
				m_pGLLandServer->CleanUp();

			if ( m_pDxLandMan )
				m_pDxLandMan->CleanUp();

			SAFE_DELETE( m_pGLLandServer );
			SAFE_DELETE( m_pDxLandMan );

			ResetEditor();
			DataCleanUp();

			m_pGLLandServer = new GLLandMan;
			m_pDxLandMan = new DxLandMan;

			m_pGLLandServer->SetD3dDevice( m_pd3dDevice );
			m_pGLLandServer->SetEmulator( TRUE );

			m_pGLLandServer->CleanUp();
			m_pDxLandMan->CleanUp();

			GLMobSchedule::m_bRENDACT = TRUE;

			BOOL bOK = m_pGLLandServer->LoadFile( dlg.GetFileName() );
			if ( !bOK )	
			{
				SAFE_DELETE( m_pGLLandServer );
				SAFE_DELETE( m_pDxLandMan );
				return;
			}

			bOK = m_pDxLandMan->LoadFile( m_pGLLandServer->GetWldFileName(), m_pd3dDevice, TRUE );
			if ( !bOK )
			{
				SAFE_DELETE( m_pGLLandServer );
				SAFE_DELETE( m_pDxLandMan );
				return;
			}

			m_pDxLandMan->ActiveMap();
			m_pDxLandMan->LoadBasicPos ( m_pd3dDevice, D3DXVECTOR3( 0.0f, 0.0f, 0.0f ));
			m_pDxLandMan->InitDeviceObjects ( m_pd3dDevice );
			m_pDxLandMan->RestoreDeviceObjects ( m_pd3dDevice );

			ApplySettings();

			m_strFileName = dlg.GetFileName();
			CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
			pFrame->SetWindowText( m_strFileName.c_str() );
		}
	}
}

void CEditorLevelView::OnLevelOpenex()
{
	std::string strLEV = "";
	std::string strWLD = "";

	{
		CString szFilter = "lev|*.LEV|";
		CFileDialog dlg ( TRUE,".LEV",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
		dlg.m_ofn.lpstrInitialDir = GLLevelFile::GetLevelPath();
		if ( dlg.DoModal() == IDOK )	
			strLEV = dlg.GetFileName();
	}

	{
		CString szFilter = "WLD|*.wld|";
		CFileDialog dlg ( TRUE,".WLD",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
		dlg.m_ofn.lpstrInitialDir = DxLandMan::GetPath ();
		if ( dlg.DoModal() == IDOK )	
			strWLD = dlg.GetFileName();
	}

	if ( strLEV.size() == 0 || strWLD.size() == 0 )
		return;

	if ( m_pGLLandServer )
		m_pGLLandServer->CleanUp();

	if ( m_pDxLandMan )
		m_pDxLandMan->CleanUp();

	SAFE_DELETE( m_pGLLandServer );
	SAFE_DELETE( m_pDxLandMan );

	ResetEditor();
	DataCleanUp();

	m_pGLLandServer = new GLLandMan;
	m_pDxLandMan = new DxLandMan;

	m_pGLLandServer->SetD3dDevice( m_pd3dDevice );
	m_pGLLandServer->SetEmulator( TRUE );

	m_pGLLandServer->CleanUp();
	m_pDxLandMan->CleanUp();

	GLMobSchedule::m_bRENDACT = TRUE;

	BOOL bOK = m_pGLLandServer->LoadFile( strLEV.c_str(), strWLD.c_str() );
	if ( !bOK )	
	{
		SAFE_DELETE( m_pGLLandServer );
		SAFE_DELETE( m_pDxLandMan );
		return;
	}

	bOK = m_pDxLandMan->LoadFile( strWLD.c_str(), m_pd3dDevice, FALSE );
	if ( !bOK )
	{
		SAFE_DELETE( m_pGLLandServer );
		SAFE_DELETE( m_pDxLandMan );
		return;
	}

	m_pDxLandMan->ActiveMap();
	m_pDxLandMan->LoadBasicPos ( m_pd3dDevice, D3DXVECTOR3( 0.0f, 0.0f, 0.0f ));
	m_pDxLandMan->InitDeviceObjects ( m_pd3dDevice );
	m_pDxLandMan->RestoreDeviceObjects ( m_pd3dDevice );

	ApplySettings();

	m_strFileName = strLEV.c_str();
	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	pFrame->SetWindowText( m_strFileName.c_str() );
}

void CEditorLevelView::OnLevelSave()
{
	if ( m_pGLLandServer )
	{
		CString szFilter = "lev|*.LEV|";
		CFileDialog dlg ( false,".LEV", m_strFileName.c_str(),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
		dlg.m_ofn.lpstrInitialDir = GLLevelFile::GetLevelPath();
		if ( dlg.DoModal() == IDOK )
		{
			//do collision fix
			MOBSCHEDULELIST *pMobScheduleList = m_pGLLandServer->GetMobSchMan()->GetMobSchList();
			MOBSCHEDULENODE *pHead = pMobScheduleList->m_pHead;
			std::map<std::string,BOOL> mapMobGroupInfo;
			while(pHead)
			{		
				GLMobSchedule *pSchdule = pHead->Data;
				D3DXVECTOR3 voffset( 0.0f, 0.0f, 0.0f );

				//check collision to navi
				pSchdule->ModifierSchList( &voffset, m_pGLLandServer->GetNavi() ) ;

				//clear random gen pos if disabled or generate if enabled
				if ( pSchdule->m_bRendGenPos )
					pSchdule->GenerateRendGenPos( m_pGLLandServer->GetNavi() );
				else
					pSchdule->m_vectorRendGenPos.clear();

				pHead = pHead->pNext;
			}

			BOOL bOk = m_pGLLandServer->SaveFile( dlg.GetFileName() );
			if ( bOk )
			{
				m_strFileName = dlg.GetFileName();
				CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
				pFrame->SetWindowText( m_strFileName.c_str() );

				CDebugSet::MsgBox( "Done Save File: %s", m_strFileName.c_str() );
			}else{
				CDebugSet::MsgBox( "Error Save File: %s", dlg.GetFileName() );
			}
		}
	}
}

void CEditorLevelView::ApplySettings()
{
	//some settings reset after loading some files
	//we needed to re apply settings

	DxBgmSound::GetInstance().SetMute ( !m_bSettingSoundBGM );
	DxSoundMan::GetInstance().SetMapMute ( !m_bSettingSoundENV );
	DxSoundMan::GetInstance().SetSfxMute ( !m_bSettingSoundSFX );
}

void CEditorLevelView::LoadWLD( std::string strName )
{
	if ( m_pDxLandMan )
		m_pDxLandMan->CleanUp();

	SAFE_DELETE( m_pDxLandMan );

	m_pDxLandMan = new DxLandMan;
	m_pDxLandMan->CleanUp();

	GLMobSchedule::m_bRENDACT = TRUE;

	HRESULT	hr = S_OK;

	hr = m_pDxLandMan->LoadFile( strName.c_str(), m_pd3dDevice, FALSE );
	if ( FAILED(hr) )	
	{
		SAFE_DELETE( m_pDxLandMan );
		return;
	}

	m_pDxLandMan->ActiveMap();
	m_pDxLandMan->LoadBasicPos ( m_pd3dDevice, D3DXVECTOR3( 0.0f, 0.0f, 0.0f ));
	m_pDxLandMan->InitDeviceObjects ( m_pd3dDevice );
	m_pDxLandMan->RestoreDeviceObjects ( m_pd3dDevice );

	ApplySettings();
}

void CEditorLevelView::ResetEditor()
{
	CMainFrame	*pMainFrame = (CMainFrame*) AfxGetMainWnd();
	if ( pMainFrame )
	{
		CsheetWithTab*  pSheetTab = pMainFrame->m_wndEditor.m_pSheetTab;
		if ( pSheetTab )
			pSheetTab->ResetEditor();
		
		ResetMouseEdit();
	}

	m_pSelectedGate = NULL;
	m_pSelectSched = NULL;
}

void CEditorLevelView::ResetMouseEdit()
{
	m_bEDITSMAT = FALSE;
	m_bEDITBOX = FALSE;
	m_bEDITMRS = FALSE;

	m_DxEditMRS.SetMatrix( NULL );
	m_DxEditMat.SetAffineMatrix( NULL );
}

void CEditorLevelView::EditSmatSet( BOOL bENABLE )
{
	ResetMouseEdit();
	m_bEDITSMAT = bENABLE;
}

void CEditorLevelView::EditBoxSet( BOOL bENABLE )
{
	ResetMouseEdit();
	m_bEDITBOX = bENABLE;
}

void CEditorLevelView::EditMRSSet( BOOL bENABLE )
{
	ResetMouseEdit();
	m_bEDITMRS = bENABLE;
}

void CEditorLevelView::OnSettingRenderDebuginfo()
{
	m_bSettingRenderDebugInfo = !m_bSettingRenderDebugInfo;
}

void CEditorLevelView::OnUpdateSettingRenderDebuginfo(CCmdUI *pCmdUI)
{
	if ( pCmdUI )	
		pCmdUI->SetCheck( m_bSettingRenderDebugInfo );
}

void CEditorLevelView::OnSettingRenderCrowmodel()
{
	m_bSettingRenderCrowModel = !m_bSettingRenderCrowModel;
}

void CEditorLevelView::OnUpdateSettingRenderCrowmodel(CCmdUI *pCmdUI)
{
	if ( pCmdUI )	
		pCmdUI->SetCheck( m_bSettingRenderCrowModel );
}

void CEditorLevelView::OnSettingRenderCrowschedule()
{
	m_bSettingRenderCrowSched = !m_bSettingRenderCrowSched;
}

void CEditorLevelView::OnUpdateSettingRenderCrowschedule(CCmdUI *pCmdUI)
{
	if ( pCmdUI )	
		pCmdUI->SetCheck( m_bSettingRenderCrowSched );
}

void CEditorLevelView::OnSettingRenderCrowgenpos()
{
	m_bSettingRenderCrowGenPos = !m_bSettingRenderCrowGenPos;
}

void CEditorLevelView::OnUpdateSettingRenderCrowgenpos(CCmdUI *pCmdUI)
{
	if ( pCmdUI )	
		pCmdUI->SetCheck( m_bSettingRenderCrowGenPos );
}

void CEditorLevelView::OnSettingRenderQuadtree()
{
	m_bSettingRenderQuadTree = !m_bSettingRenderQuadTree;
}

void CEditorLevelView::OnUpdateSettingRenderQuadtree(CCmdUI *pCmdUI)
{
	if ( pCmdUI )	
		pCmdUI->SetCheck( m_bSettingRenderQuadTree );
}

void CEditorLevelView::OnRenderRenderGate()
{
	m_bSettingRenderGate = !m_bSettingRenderGate;
}

void CEditorLevelView::OnUpdateRenderRenderGate(CCmdUI *pCmdUI)
{
	if ( pCmdUI )	
		pCmdUI->SetCheck( m_bSettingRenderGate );
}

void CEditorLevelView::OnSettingRenderLandeffect()
{
	m_bSettingRenderLandEffect = !m_bSettingRenderLandEffect;
}

void CEditorLevelView::OnUpdateSettingRenderLandeffect(CCmdUI *pCmdUI)
{
	if ( pCmdUI )	
		pCmdUI->SetCheck( m_bSettingRenderLandEffect );
}

void CEditorLevelView::OnCameraResetcamerapos()
{
	DxViewPort::GetInstance().CameraJump( D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) );
}

void CEditorLevelView::OnSettingCameraspeeddefault()
{
	DxViewPort::GetInstance().MoveVelocityReset();
}

void CEditorLevelView::OnUpdateSettingCameraspeeddefault(CCmdUI *pCmdUI)
{
	float fVelocity = DxViewPort::GetInstance().MoveVelocityGet();
	if ( pCmdUI )	
		pCmdUI->SetCheck( fVelocity == VIEWPORT_MOVEVELO_DEFAULT );
}

void CEditorLevelView::OnSettingCameraspeed500()
{
	DxViewPort::GetInstance().MoveVelocitySet( 500.0f );
}

void CEditorLevelView::OnUpdateSettingCameraspeed500(CCmdUI *pCmdUI)
{
	float fVelocity = DxViewPort::GetInstance().MoveVelocityGet();
	if ( pCmdUI )	
		pCmdUI->SetCheck( fVelocity == 500.0f );
}

void CEditorLevelView::OnSettingCameraspeed1000()
{
	DxViewPort::GetInstance().MoveVelocitySet( 1000.0f );
}

void CEditorLevelView::OnUpdateSettingCameraspeed1000(CCmdUI *pCmdUI)
{
	float fVelocity = DxViewPort::GetInstance().MoveVelocityGet();
	if ( pCmdUI )	
		pCmdUI->SetCheck( fVelocity == 1000.0f );
}

void CEditorLevelView::OnSettingCameraspeed1500()
{
	DxViewPort::GetInstance().MoveVelocitySet( 1500.0f );
}

void CEditorLevelView::OnUpdateSettingCameraspeed1500(CCmdUI *pCmdUI)
{
	float fVelocity = DxViewPort::GetInstance().MoveVelocityGet();
	if ( pCmdUI )	
		pCmdUI->SetCheck( fVelocity == 1500.0f );
}

void CEditorLevelView::OnSettingSoundBgm()
{
	m_bSettingSoundBGM = !m_bSettingSoundBGM;
	DxBgmSound::GetInstance().SetMute ( !m_bSettingSoundBGM );
}

void CEditorLevelView::OnUpdateSettingSoundBgm(CCmdUI *pCmdUI)
{
	if ( pCmdUI )	pCmdUI->SetCheck( m_bSettingSoundBGM );
}

void CEditorLevelView::OnSettingSoundEnv()
{
	m_bSettingSoundENV = !m_bSettingSoundENV;
	DxSoundMan::GetInstance().SetMapMute ( !m_bSettingSoundENV );
}

void CEditorLevelView::OnUpdateSettingSoundEnv(CCmdUI *pCmdUI)
{
	if ( pCmdUI )	pCmdUI->SetCheck( m_bSettingSoundENV );
}

void CEditorLevelView::OnSettingSoundSfx()
{
	m_bSettingSoundSFX = !m_bSettingSoundSFX;
	DxSoundMan::GetInstance().SetSfxMute ( !m_bSettingSoundSFX );
}

void CEditorLevelView::OnUpdateSettingSoundSfx(CCmdUI *pCmdUI)
{
	if ( pCmdUI )	pCmdUI->SetCheck( m_bSettingSoundSFX );
}

void CEditorLevelView::OnSettingFograngeDefault()
{
	DxFogMan::GetInstance().ManualRangeSet( FALSE, 0.0f, 0.0f );
}

void CEditorLevelView::OnUpdateSettingFograngeDefault(CCmdUI *pCmdUI)
{
	BOOL bENABLE = DxFogMan::GetInstance().ManualRangeIsDefault();
	if ( pCmdUI )	
		pCmdUI->SetCheck( bENABLE );
}

void CEditorLevelView::OnSettingFogrange5k()
{
	float fNEAR = 0.0f;
	float fFAR = 0.0f;
	DxFogMan::GetInstance().ManualRangeGet( fNEAR, fFAR );
	DxFogMan::GetInstance().ManualRangeSet( TRUE, fNEAR, 5000.0f );
}

void CEditorLevelView::OnUpdateSettingFogrange5k(CCmdUI *pCmdUI)
{
	float fNEAR = 0.0f;
	float fFAR = 0.0f;
	DxFogMan::GetInstance().ManualRangeGet( fNEAR, fFAR );
	if ( pCmdUI )	
		pCmdUI->SetCheck( fFAR == 5000.0f );
}

void CEditorLevelView::OnSettingFogrange10k()
{
	float fNEAR = 0.0f;
	float fFAR = 0.0f;
	DxFogMan::GetInstance().ManualRangeGet( fNEAR, fFAR );
	DxFogMan::GetInstance().ManualRangeSet( TRUE, fNEAR, 10000.0f );
}

void CEditorLevelView::OnUpdateSettingFogrange10k(CCmdUI *pCmdUI)
{
	float fNEAR = 0.0f;
	float fFAR = 0.0f;
	DxFogMan::GetInstance().ManualRangeGet( fNEAR, fFAR );
	if ( pCmdUI )	
		pCmdUI->SetCheck( fFAR == 10000.0f );
}

void CEditorLevelView::OnSettingFogrange20k()
{
	float fNEAR = 0.0f;
	float fFAR = 0.0f;
	DxFogMan::GetInstance().ManualRangeGet( fNEAR, fFAR );
	DxFogMan::GetInstance().ManualRangeSet( TRUE, fNEAR, 20000.0f );
}

void CEditorLevelView::OnUpdateSettingFogrange20k(CCmdUI *pCmdUI)
{
	float fNEAR = 0.0f;
	float fFAR = 0.0f;
	DxFogMan::GetInstance().ManualRangeGet( fNEAR, fFAR );
	if ( pCmdUI )	
		pCmdUI->SetCheck( fFAR == 20000.0f );
}

void CEditorLevelView::OnSettingFogrange50k()
{
	float fNEAR = 0.0f;
	float fFAR = 0.0f;
	DxFogMan::GetInstance().ManualRangeGet( fNEAR, fFAR );
	DxFogMan::GetInstance().ManualRangeSet( TRUE, fNEAR, 50000.0f );
}

void CEditorLevelView::OnUpdateSettingFogrange50k(CCmdUI *pCmdUI)
{
	float fNEAR = 0.0f;
	float fFAR = 0.0f;
	DxFogMan::GetInstance().ManualRangeGet( fNEAR, fFAR );
	if ( pCmdUI )	
		pCmdUI->SetCheck( fFAR == 50000.0f );
}

void CEditorLevelView::OnCameraJumptosphere()
{
	//jump to m_vpos position
	DxViewPort::GetInstance().CameraJump( m_vPos );
}

void CEditorLevelView::OnEditSmatrixMove()
{
	m_DxEditMat.SetType( TRANS_MAT );
}

void CEditorLevelView::OnUpdateEditSmatrixMove(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->Enable(m_bEDITSMAT);
		pCmdUI->SetCheck( m_bEDITSMAT && m_DxEditMat.GetType() == TRANS_MAT );
	}
}

void CEditorLevelView::OnEditSmatrixRotate()
{
	m_DxEditMat.SetType( ROTATE_MAT );
}

void CEditorLevelView::OnUpdateEditSmatrixRotate(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->Enable(m_bEDITSMAT);
		pCmdUI->SetCheck( m_bEDITSMAT && m_DxEditMat.GetType() == ROTATE_MAT );
	}
}

void CEditorLevelView::OnEditSmatrixScale()
{
	m_DxEditMat.SetType( SCALE_MAT );
}

void CEditorLevelView::OnUpdateEditSmatrixScale(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->Enable(m_bEDITSMAT);
		pCmdUI->SetCheck( m_bEDITSMAT && m_DxEditMat.GetType() == SCALE_MAT );
	}
}

void CEditorLevelView::OnEditBoxMove()
{
	m_DxEditBox.SetEdit( DxEditBox::EM_MOVE );
}

void CEditorLevelView::OnUpdateEditBoxMove(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->Enable(m_bEDITBOX);
		pCmdUI->SetCheck( m_bEDITBOX && m_DxEditBox.GetEdit() == DxEditBox::EM_MOVE );
	}
}

void CEditorLevelView::OnEditBoxScale()
{
	m_DxEditBox.SetEdit( DxEditBox::EM_SCALE );
}

void CEditorLevelView::OnUpdateEditBoxScale(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->Enable(m_bEDITBOX);
		pCmdUI->SetCheck( m_bEDITBOX && m_DxEditBox.GetEdit() == DxEditBox::EM_SCALE );
	}
}

void CEditorLevelView::OnEditBoxSelscale()
{
	m_DxEditBox.SetEdit( DxEditBox::EM_SELECTSCALE );
}

void CEditorLevelView::OnUpdateEditBoxSelscale(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->Enable(m_bEDITBOX);
		pCmdUI->SetCheck( m_bEDITBOX && m_DxEditBox.GetEdit() == DxEditBox::EM_SELECTSCALE );
	}
}

void CEditorLevelView::OnEditMatrixMove()
{
	m_DxEditMRS.SetMode( OBECTMRS_M );
}

void CEditorLevelView::OnUpdateEditMatrixMove(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->Enable(m_bEDITMRS);
		pCmdUI->SetCheck( m_bEDITMRS && m_DxEditMRS.GetMode() == OBECTMRS_M );
	}
}

void CEditorLevelView::OnEditMatrixRotate()
{
	m_DxEditMRS.SetMode( OBECTMRS_R );
}

void CEditorLevelView::OnUpdateEditMatrixRotate(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->Enable(m_bEDITMRS);
		pCmdUI->SetCheck( m_bEDITMRS && m_DxEditMRS.GetMode() == OBECTMRS_R );
	}
}

void CEditorLevelView::OnEditMatrixScale()
{
	m_DxEditMRS.SetMode( OBECTMRS_S );
}

void CEditorLevelView::OnUpdateEditMatrixScale(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->Enable(m_bEDITMRS);
		pCmdUI->SetCheck( m_bEDITMRS && m_DxEditMRS.GetMode() == OBECTMRS_S );
	}
}

void CEditorLevelView::OnDetectSelectCrow()
{
	m_bDETECTCROW = !m_bDETECTCROW;
}

void CEditorLevelView::OnUpdateDetectSelectCrow(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->SetCheck( m_bDETECTCROW );
	}
}

void CEditorLevelView::OnDetectSelectGate()
{
	m_bDETECTGATE = !m_bDETECTGATE;
}

void CEditorLevelView::OnUpdateDetectSelectGate(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->SetCheck( m_bDETECTGATE );
	}
}

void CEditorLevelView::DataCleanUp()
{
	DxSkinCharDataContainer::GetInstance().CleanUp ();

	/*ABL system, Juver, 2017/06/02 */
	DxAttBoneDataContainer::GetInstance().CleanUp ();
	DxAttBoneLinkContainer::GetInstance().CleanUp ();

	/*vehicle system, Juver, 2017/08/04 */
	DxVehicleDataContainer::GetInstance().CleanUp();

	DxReplaceContainer::GetInstance().CleanUp ();
	DxSkinPieceContainer::GetInstance().CleanUp ();
	DxSkinAniMan::GetInstance().ClearBinLoadingData ();
	DxBoneCollector::GetInstance().DoInterimClean ( m_pd3dDevice );
	DxSkinAniMan::GetInstance().DoInterimClean ( m_pd3dDevice );
	DxSkinMeshMan::GetInstance().DoInterimClean ( m_pd3dDevice );
	DxEffSinglePropGMan::GetInstance().CleanUp ();
	DxEffGroupPlayer::GetInstance().RemoveAllEff ();
	DxPieceContainer::GetInstance().CleanUp();
}

void CEditorLevelView::OnLevelSaveWld()
{
	if ( !m_pDxLandMan )	return;

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Saving wld will clear current session any unsaved data will be lost!, continue?") != IDYES ) return;

	CString szFilter = "wld|*.wld|";
	CFileDialog dlg ( false,".wld", "",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = DxLandMan::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		BOOL bOk = m_pDxLandMan->SaveFile( dlg.GetPathName() );
		if ( bOk )
		{
			CDebugSet::MsgBox( "Done Save WLD File: %s Please Reload Session!", dlg.GetPathName() );

			OnLevelNew();

		}else{
			CDebugSet::MsgBox( "Error Save WLD File: %s", dlg.GetPathName() );
		}
	}
}

void CEditorLevelView::OnRenderRendergroupline()
{
	m_bRenderGroupLine = !m_bRenderGroupLine;
}

void CEditorLevelView::OnUpdateRenderRendergroupline(CCmdUI *pCmdUI)
{
	if ( pCmdUI )
	{
		pCmdUI->SetCheck( m_bRenderGroupLine );
	}
}
