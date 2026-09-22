
// GameEmulatorView.cpp : implementation of the CGameEmulatorView class
//

#include "stdafx.h"
#include "GameEmulator.h"

#include "GameEmulatorDoc.h"
#include "GameEmulatorView.h"

#include "DlgCharset.h"
#include "GLogic.h"
#include "SUBPATH.h"
#include "GLogicData.h"
#include "RANPARAM.h"
#include "DxParamSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGameEmulatorView

IMPLEMENT_DYNCREATE(CGameEmulatorView, CView)

BEGIN_MESSAGE_MAP(CGameEmulatorView, CView)
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_COMMAND(ID_EMULATOR_RUN, OnEmulatorRun)
END_MESSAGE_MAP()

// CGameEmulatorView construction/destruction

CGameEmulatorView::CGameEmulatorView()
	: CD3DApplication()
	, m_bCreated(FALSE)
	, m_hCursorDefault(NULL)
{
	m_bUseDepthBuffer	= TRUE;
	sCharacter = SCHARSET();
}

CGameEmulatorView::~CGameEmulatorView()
{
}

BOOL CGameEmulatorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CGameEmulatorView drawing

void CGameEmulatorView::OnDraw(CDC* /*pDC*/)
{
	//CGameEmulatorDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc);
	//if (!pDoc)
	//	return;

	// TODO: add draw code for native data here
}


// CGameEmulatorView diagnostics

#ifdef _DEBUG
void CGameEmulatorView::AssertValid() const
{
	CView::AssertValid();
}

void CGameEmulatorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

//CGameEmulatorDoc* CGameEmulatorView::GetDocument() const // non-debug version is inline
//{
//	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGameEmulatorDoc)));
//	return (CGameEmulatorDoc*)m_pDocument;
//}
#endif //_DEBUG


// CGameEmulatorView message handlers


void CGameEmulatorView::OnEmulatorRun()
{
	CGameEmulatorApp *pApp = (CGameEmulatorApp *) AfxGetApp();

	char szFullPath[MAX_PATH] = {0};
	strcpy ( szFullPath, pApp->m_szAppPath );
	strcat ( szFullPath,  SUBPATH::GLOGIC_FILE );
	GLOGIC::SetPath ( szFullPath );

	CDlgCharset dlg( this );
	if ( dlg.DoModal() == IDOK )
	{
		sCharacter.Assign( dlg.sCharacter );
	}else{
		return;
	}

	RANPARAM::LOAD ( pApp->m_szAppPath );
	DXPARAMSET::INIT ();

	CD3DApplication::SetScreen ( RANPARAM::dwScrWidth, RANPARAM::dwScrHeight, RANPARAM::emScrFormat, RANPARAM::uScrRefreshHz, TRUE );
	if ( FAILED( CD3DApplication::Create ( m_hWnd, m_hWnd, AfxGetInstanceHandle() ) ) )		return;
	m_bCreated = TRUE;

}
