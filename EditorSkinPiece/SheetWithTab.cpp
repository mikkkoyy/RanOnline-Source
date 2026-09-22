#include "stdafx.h"
#include <vector>
#include "resource.h"
#include "SheetWithTab.h"
#include "EditorSkinPieceView.h"
#include "DxEffChar.h"


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
	m_PageEff_00_Single ( logfont ),
	m_PageEff_01_Blur ( logfont ),
	m_PageEff_02_CloneBlur ( logfont ),
	m_PageEff_04_Shock ( logfont ),
	m_PageEff_06_Arrow ( logfont ),
	m_PageEff_08_Level ( logfont ),
	m_PageEff_10_Alpha ( logfont ),
	m_PageEff_11_Neon ( logfont ),
	m_PageEff_12_Mark ( logfont ),
	m_PageEff_13_NoAlpha ( logfont ),
	m_PageEff_14_Reflection2 ( logfont ),
	m_PageEff_15_Ambient ( logfont ),
	m_PageEff_17_MultiTex ( logfont ),
	m_PageEff_18_Ghosting ( logfont ),
	m_PageEff_19_Specular2 ( logfont ),
	m_PageEff_20_Toon ( logfont ),
	m_PageEff_21_TexDiff ( logfont ),
	m_PageEff_22_Particle ( logfont ),
	m_PageEff_23_BonePos ( logfont ),
	m_PageEff_24_BoneList ( logfont ),
	m_PageEff_25_UserColor ( logfont ),
	m_PageEff_26_Normal ( logfont ),
	m_PageEff_27_Line2Bone ( logfont ),
	m_PageEff_28_AroundEff ( logfont ),
	

	m_pFont ( NULL )
{
	m_pWndParent = pParentWnd;
	m_Rect = rect;
	m_bNoTabs = TRUE;
	m_bStacked = FALSE;

	m_PageMain.SetSheetTab ( this );
	m_PageEff_00_Single.SetSheetTab ( this );
	m_PageEff_01_Blur.SetSheetTab ( this );
	m_PageEff_02_CloneBlur.SetSheetTab ( this );
	m_PageEff_04_Shock.SetSheetTab ( this );
	m_PageEff_06_Arrow.SetSheetTab ( this );
	m_PageEff_08_Level.SetSheetTab ( this );
	m_PageEff_10_Alpha.SetSheetTab ( this );
	m_PageEff_11_Neon.SetSheetTab ( this );
	m_PageEff_12_Mark.SetSheetTab ( this );
	m_PageEff_13_NoAlpha.SetSheetTab ( this );
	m_PageEff_14_Reflection2.SetSheetTab ( this );
	m_PageEff_15_Ambient.SetSheetTab ( this );
	m_PageEff_17_MultiTex.SetSheetTab ( this );
	m_PageEff_18_Ghosting.SetSheetTab ( this );
	m_PageEff_19_Specular2.SetSheetTab ( this );
	m_PageEff_20_Toon.SetSheetTab ( this );
	m_PageEff_21_TexDiff.SetSheetTab ( this );
	m_PageEff_22_Particle.SetSheetTab ( this );
	m_PageEff_23_BonePos.SetSheetTab ( this );
	m_PageEff_24_BoneList.SetSheetTab ( this );
	m_PageEff_25_UserColor.SetSheetTab ( this );
	m_PageEff_26_Normal.SetSheetTab ( this );
	m_PageEff_27_Line2Bone.SetSheetTab ( this );
	m_PageEff_28_AroundEff.SetSheetTab ( this );
	
	AddPage ( &m_PageMain );
	AddPage ( &m_PageEff_00_Single );
	AddPage ( &m_PageEff_01_Blur );
	AddPage ( &m_PageEff_02_CloneBlur );
	AddPage ( &m_PageEff_04_Shock );
	AddPage ( &m_PageEff_06_Arrow );
	AddPage ( &m_PageEff_08_Level );
	AddPage ( &m_PageEff_10_Alpha );
	AddPage ( &m_PageEff_11_Neon );
	AddPage ( &m_PageEff_12_Mark );
	AddPage ( &m_PageEff_13_NoAlpha );
	AddPage ( &m_PageEff_14_Reflection2 );
	AddPage ( &m_PageEff_15_Ambient );
	AddPage ( &m_PageEff_17_MultiTex );
	AddPage ( &m_PageEff_18_Ghosting );
	AddPage ( &m_PageEff_19_Specular2 );
	AddPage ( &m_PageEff_20_Toon );
	AddPage ( &m_PageEff_21_TexDiff );
	AddPage ( &m_PageEff_22_Particle );
	AddPage ( &m_PageEff_23_BonePos );
	AddPage ( &m_PageEff_24_BoneList );
	AddPage ( &m_PageEff_25_UserColor );
	AddPage ( &m_PageEff_26_Normal );
	AddPage ( &m_PageEff_27_Line2Bone );
	AddPage ( &m_PageEff_28_AroundEff );

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
	SetActivePage ( SKINPIECEEDIT_PAGE_MAIN );
	m_PageMain.ResetTool();
	m_PageEff_00_Single.ResetTool();
	m_PageEff_01_Blur.ResetTool();
	m_PageEff_02_CloneBlur.ResetTool();
	m_PageEff_04_Shock.ResetTool();
	m_PageEff_06_Arrow.ResetTool();
	m_PageEff_08_Level.ResetTool();
	m_PageEff_10_Alpha.ResetTool();
	m_PageEff_11_Neon.ResetTool();
	m_PageEff_12_Mark.ResetTool();
	m_PageEff_13_NoAlpha.ResetTool();
	m_PageEff_14_Reflection2.ResetTool();
	m_PageEff_15_Ambient.ResetTool();
	m_PageEff_17_MultiTex.ResetTool();
	m_PageEff_18_Ghosting.ResetTool();
	m_PageEff_19_Specular2.ResetTool();
	m_PageEff_20_Toon.ResetTool();
	m_PageEff_21_TexDiff.ResetTool();
	m_PageEff_22_Particle.ResetTool();
	m_PageEff_23_BonePos.ResetTool();
	m_PageEff_24_BoneList.ResetTool();
	m_PageEff_25_UserColor.ResetTool();
	m_PageEff_26_Normal.ResetTool();
	m_PageEff_27_Line2Bone.ResetTool();
	m_PageEff_28_AroundEff.ResetTool();
}

void CsheetWithTab::ShowMain()
{
	SetActivePage ( SKINPIECEEDIT_PAGE_MAIN );
	m_PageMain.SetData( CEditorSkinPieceView::GetView()->m_pPiece );

	CEditorSkinPieceView::GetView()->m_bRenderBone = FALSE;
	CEditorSkinPieceView::GetView()->m_bEditMRS = FALSE;
	CEditorSkinPieceView::GetView()->m_DxEditMRS.SetMatrix( NULL );
}

void CsheetWithTab::EffEdit( DxSkinPiece* pPiece, DxEffChar* pEff, BOOL bADD /*= FALSE */ )
{
	if ( !pPiece )	return;
	if ( !pEff )	return;

	DWORD dwTYPEID = pEff->GetTypeID();
	switch ( dwTYPEID )
	{
	case EMEFFCHAR_SINGLE:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_00_SINGLE );
			m_PageEff_00_Single.DataSet( pPiece, (DxEffCharSingle*)pEff, bADD );
		}break;
	case EMEFFCHAR_BLUR:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_01_BLUR );
			m_PageEff_01_Blur.DataSet( pPiece, (DxEffCharBlur*)pEff, bADD );
		}break;
	case EMEFFCHAR_CLONEBLUR:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_02_CLONEBLUR );
			m_PageEff_02_CloneBlur.DataSet( pPiece, (DxEffCharCloneBlur*)pEff, bADD );
		}break;
	case EMEFFCHAR_SHOCK:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_04_SHOCK );
			m_PageEff_04_Shock.DataSet( pPiece, (DxEffCharShock*)pEff, bADD );
		}break;
	case EMEFFCHAR_ARROW:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_06_ARROW );
			m_PageEff_06_Arrow.DataSet( pPiece, (DxEffCharArrow*)pEff, bADD );
		}break;
	case EMEFFCHAR_LEVEL:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_08_LEVEL );
			m_PageEff_08_Level.DataSet( pPiece, (DxEffCharLevel*)pEff, bADD );
		}break;
	case EMEFFCHAR_ALPHA:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_10_ALPHA );
			m_PageEff_10_Alpha.DataSet( pPiece, (DxEffCharAlpha*)pEff, bADD );
		}break;
	case EMEFFCHAR_NEON:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_11_NEON );
			m_PageEff_11_Neon.DataSet( pPiece, (DxEffCharNeon*)pEff, bADD );
		}break;
	case EMEFFCHAR_MARK:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_12_MARK );
			m_PageEff_12_Mark.DataSet( pPiece, (DxEffCharMark*)pEff, bADD );
		}break;
	case EMEFFCHAR_NOALPHA:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_13_NOALPHA );
			m_PageEff_13_NoAlpha.DataSet( pPiece, (DxEffCharNoAlpha*)pEff, bADD );
		}break;
	case EMEFFCHAR_REFLECTION2:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_14_REFLECTION2 );
			m_PageEff_14_Reflection2.DataSet( pPiece, (DxEffCharReflection2*)pEff, bADD );
		}break;
	case EMEFFCHAR_AMBIENT:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_15_AMBIENT );
			m_PageEff_15_Ambient.DataSet( pPiece, (DxEffCharAmbient*)pEff, bADD );
		}break;
	case EMEFFCHAR_MULTITEX:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_17_MULTITEX );
			m_PageEff_17_MultiTex.DataSet( pPiece, (DxEffCharMultiTex*)pEff, bADD );
		}break;
	case EMEFFCHAR_GHOSTING:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_18_GHOSTING );
			m_PageEff_18_Ghosting.DataSet( pPiece, (DxEffCharGhosting*)pEff, bADD );
		}break;
	case EMEFFCHAR_SPECULAR2:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_19_SPECULAR2 );
			m_PageEff_19_Specular2.DataSet( pPiece, (DxEffCharSpecular2*)pEff, bADD );
		}break;
	case EMEFFCHAR_TOON:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_20_TOON );
			m_PageEff_20_Toon.DataSet( pPiece, (DxEffCharToon*)pEff, bADD );
		}break;
	case EMEFFCHAR_TEXDIFF:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_21_TEXDIFF );
			m_PageEff_21_TexDiff.DataSet( pPiece, (DxEffCharTexDiff*)pEff, bADD );
		}break;
	case EMEFFCHAR_PARTICLE:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_22_PARTICLE );
			m_PageEff_22_Particle.DataSet( pPiece, (DxEffCharParticle*)pEff, bADD );
		}break;
	case EMEFFCHAR_BONEPOSEFF:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_23_BONEPOS );
			m_PageEff_23_BonePos.DataSet( pPiece, (DxEffCharBonePosEff*)pEff, bADD );
		}break;
	case EMEFFCHAR_BONELISTEFF:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_24_BONELIST );
			m_PageEff_24_BoneList.DataSet( pPiece, (DxEffCharBoneListEff*)pEff, bADD );
		}break;
	case EMEFFCHAR_USERCOLOR:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_25_USERCOLOR );
			m_PageEff_25_UserColor.DataSet( pPiece, (DxEffCharUserColor*)pEff, bADD );
		}break;
	case EMEFFCHAR_NORMALMAP:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_26_NORMAL );
			m_PageEff_26_Normal.DataSet( pPiece, (DxEffCharNormal*)pEff, bADD );
		}break;
	case EMEFFCHAR_LINE2BONE:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_27_LINE2BONE );
			m_PageEff_27_Line2Bone.DataSet( pPiece, (DxEffCharLine2BoneEff*)pEff, bADD );
		}break;
	case EMEFFCHAR_AROUNDEFFECT:
		{
			SetActivePage ( SKINPIECEEDIT_PAGE_EFF_28_AROUNDEFF );
			m_PageEff_28_AroundEff.DataSet( pPiece, (DxEffCharAroundEff*)pEff, bADD );
		}break;
	};
}
