// PageMain.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "PageMain.h"

#include "SheetWithTab.h"

#include "logic/GMToolConfig.h"
#include "Logic/GMToolGlobal.h"
#include "Logic/GMToolLogs.h"

// CPageMain dialog

IMPLEMENT_DYNAMIC(CPageMain, CPropertyPage)

CPageMain::CPageMain(LOGFONT logfont)
	: CPropertyPage(CPageMain::IDD)
	, m_pFont ( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageMain::~CPageMain()
{
	SAFE_DELETE ( m_pFont );
}

void CPageMain::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageMain, CPropertyPage)
	ON_BN_CLICKED(IDC_MAIN_PAGE_BUTTON_CLEAR, OnBnClickedMainPageButtonClear)
END_MESSAGE_MAP()


// CPageMain message handlers
BOOL CPageMain::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	CWnd *pWndConsole = GetDlgItem(IDC_EDITCONSOLE);
	GMTOOL_LOGS::ConsoleSet( pWndConsole->GetSafeHwnd() );

	GMTOOL_LOGS::LogInfo( "Welcome [%s]:(%d)%s", 
		GMTOOL_GLOBAL::strUserLevelIndex[ UserTypeToIndex( (EMUSERTYPE) GMTOOL_CONFIG::m_wLoginType ) ].c_str(), 
		GMTOOL_CONFIG::m_dwLoginID, 
		GMTOOL_CONFIG::m_szLoginUser );

	return TRUE;  
}


void CPageMain::OnBnClickedMainPageButtonClear()
{
	if ( !m_bDlgInit )
		return;

	GMTOOL_LOGS::ConsoleClear();
}
