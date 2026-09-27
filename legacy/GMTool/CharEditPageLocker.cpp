// CharEditPageLocker.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "CharEditPageLocker.h"

#include "CharEditTab.h"
#include "GLCharData.h"
#include "LockerTab.h"

// CCharEditPageLocker dialog

IMPLEMENT_DYNAMIC(CCharEditPageLocker, CPropertyPage)

CCharEditPageLocker::CCharEditPageLocker(LOGFONT logfont, SCHARDATA2* pData)
	: CPropertyPage(CCharEditPageLocker::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
	, m_pLockerTab( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CCharEditPageLocker::~CCharEditPageLocker()
{
	SAFE_DELETE ( m_pFont );
	SAFE_DELETE ( m_pLockerTab );
}

void CCharEditPageLocker::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCharEditPageLocker, CPropertyPage)
END_MESSAGE_MAP()


// CCharEditPageLocker message handlers
BOOL CCharEditPageLocker::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	CRect SheetRect;
	CWnd *pWnd;
	pWnd = GetDlgItem ( IDC_STATIC_TOOL );
	pWnd->GetWindowRect ( SheetRect );
	ScreenToClient ( &SheetRect );
	LOGFONT logfont = {0};

	CFont* pFont = GetFont();
	if ( pFont ) pFont->GetLogFont( &logfont );

	m_pLockerTab = new CLockerTab ( m_pData, SheetRect, logfont, this );
	m_pLockerTab->Create( this, WS_CHILD|WS_VISIBLE|WS_TABSTOP, 0 );
	m_pLockerTab->ModifyStyleEx (0, WS_EX_CONTROLPARENT);
	m_pLockerTab->InitPages();

	return TRUE;  
}