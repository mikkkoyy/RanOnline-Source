#include "stdafx.h"
#include <vector>
#include "resource.h"
#include "SheetWithTab.h"
#include "EditorLevelView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CsheetWithTab

IMPLEMENT_DYNAMIC(CsheetWithTab, CPropertySheet)

CsheetWithTab::CsheetWithTab(CRect& rect, LOGFONT logfont, CWnd* pParentWnd)
	: CPropertySheet("Sheet", pParentWnd),
	m_bNoTabs(FALSE),
	m_pWndParent(NULL),
	m_iDistanceFromTop(0),
	
	m_PageMain ( logfont ),
	m_PageBasic ( logfont ),
	m_PageCondition ( logfont ),
	m_PageCrow ( logfont ),
	m_PageGate ( logfont ),
	m_PageEff ( logfont ),
	m_PageLandMark( logfont ),
	m_PageEtc( logfont ),
	m_PageEditCrow( logfont ),
	m_PageEditGate( logfont ),
	m_PageEditEffect( logfont ),
	m_PageEditLandMark( logfont ),
	m_pFont ( NULL )
{
	m_pWndParent = pParentWnd;
	m_Rect = rect;
	m_bNoTabs = TRUE;
	m_bStacked = FALSE;

	m_PageMain.SetSheetTab ( this );
	m_PageBasic.SetSheetTab ( this );
	m_PageCondition.SetSheetTab ( this );
	m_PageCrow.SetSheetTab ( this );
	m_PageGate.SetSheetTab ( this );
	m_PageEff.SetSheetTab ( this );
	m_PageLandMark.SetSheetTab( this );
	m_PageEtc.SetSheetTab( this );
	m_PageEditCrow.SetSheetTab( this );
	m_PageEditGate.SetSheetTab( this );
	m_PageEditEffect.SetSheetTab( this );
	m_PageEditLandMark.SetSheetTab( this );

	AddPage ( &m_PageMain );
	AddPage ( &m_PageBasic );
	AddPage ( &m_PageCondition );
	AddPage ( &m_PageCrow );
	AddPage ( &m_PageGate );
	AddPage ( &m_PageEff );
	AddPage ( &m_PageLandMark );
	AddPage ( &m_PageEtc );
	AddPage ( &m_PageEditCrow );
	AddPage ( &m_PageEditGate );
	AddPage ( &m_PageEditEffect );
	AddPage ( &m_PageEditLandMark );

	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CsheetWithTab::~CsheetWithTab()
{
	SAFE_DELETE( m_pFont );
}

static int module_piButtons[] = { IDOK, IDCANCEL, ID_APPLY_NOW, IDHELP };

BOOL CsheetWithTab::OnInitDialog() 
{
	HWND hTabWnd;
	CRect rectTabCtrl;

	ChangeDialogFont( this, m_pFont, CDF_NONE );

	if( m_Rect.right )
	{
		hTabWnd = (HWND)::GetDlgItem(m_hWnd, AFX_IDC_TAB_CONTROL);
		ASSERT(hTabWnd != NULL);
		::GetWindowRect(hTabWnd, &rectTabCtrl);  // Tab-control rectangle (screen coordinates)
		ScreenToClient(rectTabCtrl);

		if( !m_bNoTabs && rectTabCtrl.right > (m_Rect.Width() - 3) )
		{
			rectTabCtrl.right = m_Rect.Width() - 3; // Set new right position
		}

		rectTabCtrl.bottom = 32; // Height for tabs

		::MoveWindow( hTabWnd, 0, m_iDistanceFromTop, rectTabCtrl.Width(), rectTabCtrl.bottom, TRUE );
		BOOL bResult = (BOOL)Default();          // Process messages

		if( m_bNoTabs )
		{
			::ShowWindow(hTabWnd, SW_HIDE);
			::EnableWindow(hTabWnd, FALSE);
		}

		if (!m_bStacked)
		{
			hTabWnd = (HWND)SendMessage(PSM_GETTABCONTROL);
			if (hTabWnd != NULL)
			{
				CWnd::ModifyStyle(hTabWnd, TCS_MULTILINE, TCS_SINGLELINE, 0);
			}
		}

		for (int i = 0; i < 4; i++)
		{
			HWND hWnd = ::GetDlgItem(m_hWnd, module_piButtons[i]);
			if (hWnd != NULL)
			{
				::ShowWindow(hWnd, SW_HIDE);
				::EnableWindow(hWnd, FALSE);
			}
		}

		MoveWindow( m_Rect.left, m_Rect.top, m_Rect.right, m_Rect.bottom /*- cyDiff*/ );
		return bResult;
	}
	return CPropertySheet::OnInitDialog();
}

BEGIN_MESSAGE_MAP(CsheetWithTab, CPropertySheet)
	//{{AFX_MSG_MAP(CsheetWithTab)
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

HBRUSH CsheetWithTab::CtlColor(CDC* pDc, UINT uCtlColor) 
{
	CWnd*    pwndParent;   // Parent window to property-sheet
	CDC*     pdcParent;    // Parent dc
	COLORREF color;        // color on parent dc           
	pwndParent = GetParent();
	ASSERT( pwndParent->IsKindOf( RUNTIME_CLASS( CWnd ) ) ); 
	pdcParent = pwndParent->GetDC();
	color = pdcParent->GetBkColor();    // Get parent color
	pwndParent->ReleaseDC( pdcParent );
	CBrush brush( color );              // Make a brush
	return (HBRUSH)brush;
}

BOOL CsheetWithTab::PreTranslateMessage(MSG* pMsg)
{
	switch ( pMsg->message )
	{
	case WM_KEYDOWN:
		{
			if ( pMsg->wParam == VK_ESCAPE )
			{
			}
		}break;
	}
	return CPropertySheet::PreTranslateMessage(pMsg);
}

void CsheetWithTab::ChangeDialogFont(CWnd* pWnd, CFont* pFont, int nFlag)
{
	CRect windowRect;
	TEXTMETRIC tmOld, tmNew;
	CDC * pDC = pWnd->GetDC();
	CFont * pSavedFont = pDC->SelectObject(pWnd->GetFont());
	pDC->GetTextMetrics(&tmOld);
	pDC->SelectObject(pFont);
	pDC->GetTextMetrics(&tmNew);
	pDC->SelectObject(pSavedFont);
	pWnd->ReleaseDC(pDC);

	CRect clientRect, newClientRect, newWindowRect;
	pWnd->GetWindowRect(windowRect);
	pWnd->GetClientRect(clientRect);
	double fHeigth = (double)(m_Rect.bottom-m_Rect.top) / clientRect.bottom;
	double fWidth = (double) (m_Rect.right-m_Rect.left) / clientRect.right;

	if (nFlag != CDF_NONE)
	{
		long xDiff = windowRect.Width() - clientRect.Width();
		long yDiff = windowRect.Height() - clientRect.Height();
	
		newClientRect.left = newClientRect.top = 0;
		newClientRect.right = LONG(clientRect.right * fWidth);
		newClientRect.bottom = LONG(clientRect.bottom * fHeigth);

		if (nFlag == CDF_TOPLEFT)
		{
			newWindowRect.left = windowRect.left;
			newWindowRect.top = windowRect.top;
			newWindowRect.right = windowRect.left + newClientRect.right + xDiff;
			newWindowRect.bottom = windowRect.top + newClientRect.bottom + yDiff;
		}else if (nFlag == CDF_CENTER){
			newWindowRect.left = windowRect.left - 
							(newClientRect.right - clientRect.right)/2;
			newWindowRect.top = windowRect.top -
							(newClientRect.bottom - clientRect.bottom)/2;
			newWindowRect.right = newWindowRect.left + newClientRect.right + xDiff;
			newWindowRect.bottom = newWindowRect.top + newClientRect.bottom + yDiff;
		}
		pWnd->MoveWindow(newWindowRect);
	}

	pWnd->SetFont(pFont);

	CWnd* pChildWnd = pWnd->GetWindow(GW_CHILD);
	while (pChildWnd)
	{
		pChildWnd->SetFont(pFont);
		pChildWnd->GetWindowRect(windowRect);

		CString strClass;
		::GetClassName(pChildWnd->m_hWnd, strClass.GetBufferSetLength(32), 31);
		strClass.MakeUpper();
		if(strClass==_T("COMBOBOX"))
		{
			CRect rect;
			pChildWnd->SendMessage(CB_GETDROPPEDCONTROLRECT,0,(LPARAM) &rect);
			windowRect.right = rect.right;
			windowRect.bottom = rect.bottom;
		}

		pWnd->ScreenToClient(windowRect);
		windowRect.left = LONG(windowRect.left * fWidth);
		windowRect.right = LONG(windowRect.right * fWidth);
		windowRect.top = LONG(windowRect.top * fHeigth);
		windowRect.bottom = LONG(windowRect.bottom *fHeigth);
		pChildWnd->MoveWindow(windowRect);
		pChildWnd = pChildWnd->GetWindow(GW_HWNDNEXT);
	}
}

void CsheetWithTab::InitPages()
{
	int nPAGE = GetPageCount();
	for( int i = 0; i < nPAGE; ++ i )
	{
		SetActivePage( i );
	}

	if ( nPAGE > 0 )
		SetActivePage( 0 );
}

void CsheetWithTab::ResetEditor()
{
	SetActivePage ( LEVELEDIT_PAGE_MAIN );
	m_PageBasic.ResetEditor();
	m_PageCondition.ResetEditor();
}

void CsheetWithTab::SelectCrow( std::string strName )
{
	GLLandMan*	pGLLand = CEditorLevelView::GetView()->m_pGLLandServer;
	if ( !pGLLand )	return;

	SGLNODE<GLMobSchedule*>* pSched = pGLLand->GetMobSchMan()->FindMobSch( strName.c_str() );
	if ( !pSched )	return;

	SetActivePage ( LEVELEDIT_PAGE_CROWLIST );
	m_PageCrow.DataSet( pGLLand );
	m_PageCrow.SelectCrow( strName.c_str() );
}

void CsheetWithTab::SelectGate( std::string strName )
{
	GLLandMan*	pGLLand = CEditorLevelView::GetView()->m_pGLLandServer;
	if ( !pGLLand )	return;

	PDXLANDGATE pLandGate = pGLLand->GetLandGateMan().FindLandGate( strName.c_str() );
	if ( !pLandGate )	return;

	SetActivePage ( LEVELEDIT_PAGE_GATELIST );
	m_PageGate.DataSet( pGLLand );
	m_PageGate.SelectGate( strName.c_str() );
}

void CsheetWithTab::PageActive( int nPAGE )
{
	if( nPAGE < 0 )	return;

	GLLandMan*	pGLLand = CEditorLevelView::GetView()->m_pGLLandServer;
	if ( !pGLLand )	return;

	switch( nPAGE )
	{
	case LEVELEDIT_PAGE_MAIN:
		{
			SetActivePage ( LEVELEDIT_PAGE_MAIN );
		}break;
	case LEVELEDIT_PAGE_BASIC:
		{
			SetActivePage ( LEVELEDIT_PAGE_BASIC );
			m_PageBasic.DataSet( pGLLand );
		}break;
	case LEVELEDIT_PAGE_CONDITION:
		{
			SetActivePage ( LEVELEDIT_PAGE_CONDITION );
			m_PageCondition.DataSet( pGLLand );
		}break;
	case LEVELEDIT_PAGE_CROWLIST:
		{
			SetActivePage ( LEVELEDIT_PAGE_CROWLIST );
			m_PageCrow.DataSet( pGLLand );
		}break;
	case LEVELEDIT_PAGE_GATELIST:
		{
			SetActivePage ( LEVELEDIT_PAGE_GATELIST );
			m_PageGate.DataSet( pGLLand );
		}break;
	case LEVELEDIT_PAGE_EFFECTLIST:
		{
			SetActivePage ( LEVELEDIT_PAGE_EFFECTLIST );
			m_PageEff.DataSet( pGLLand );
		}break;
	case LEVELEDIT_PAGE_LANDMARKLIST:
		{
			SetActivePage ( LEVELEDIT_PAGE_LANDMARKLIST );
			m_PageLandMark.DataSet( pGLLand );
		}break;
	case LEVELEDIT_PAGE_ETCHFUNCTION:
		{
			SetActivePage ( LEVELEDIT_PAGE_ETCHFUNCTION );
			m_PageEtc.DataSet( pGLLand );
		}break;
	};

	//moving to pages reset mouse control edits
	CEditorLevelView::GetView()->ResetMouseEdit();
}

void CsheetWithTab::PageEditCrow( GLMobSchedule* pEdit )
{
	GLLandMan*	pGLLand = CEditorLevelView::GetView()->m_pGLLandServer;
	DxLandMan*	pDxLand = CEditorLevelView::GetView()->m_pDxLandMan;
	if ( !pGLLand )	return;
	if ( !pDxLand )	return;

	SetActivePage ( LEVELEDIT_PAGE_CROWEDIT );
	m_PageEditCrow.SetData( pGLLand, pEdit );
}

void CsheetWithTab::PageEditGate( PDXLANDGATE pEdit )
{
	GLLandMan*	pGLLand = CEditorLevelView::GetView()->m_pGLLandServer;
	DxLandMan*	pDxLand = CEditorLevelView::GetView()->m_pDxLandMan;
	if ( !pGLLand )	return;
	if ( !pDxLand )	return;

	SetActivePage ( LEVELEDIT_PAGE_GATEEDIT );
	m_PageEditGate.SetData( pGLLand, pEdit );
}

void CsheetWithTab::PageEditEff( PLANDEFF pEdit )
{
	GLLandMan*	pGLLand = CEditorLevelView::GetView()->m_pGLLandServer;
	DxLandMan*	pDxLand = CEditorLevelView::GetView()->m_pDxLandMan;
	if ( !pGLLand )	return;
	if ( !pDxLand )	return;

	SetActivePage ( LEVELEDIT_PAGE_EFFECTEDIT );
	m_PageEditEffect.SetData( pGLLand, pEdit );
}

void CsheetWithTab::PageEditLandMark( PLANDMARK pEdit )
{
	GLLandMan*	pGLLand = CEditorLevelView::GetView()->m_pGLLandServer;
	DxLandMan*	pDxLand = CEditorLevelView::GetView()->m_pDxLandMan;
	if ( !pGLLand )	return;
	if ( !pDxLand )	return;

	SetActivePage ( LEVELEDIT_PAGE_LANDMARKEDIT );
	m_PageEditLandMark.SetData( pGLLand, pEdit );
}
