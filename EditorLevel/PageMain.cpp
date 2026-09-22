// PageMain.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "PageMain.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"

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
	ON_BN_CLICKED(IDC_BUTTON_OPEN, OnBnClickedButtonOpen)
END_MESSAGE_MAP()


// CPageMain message handlers
BOOL CPageMain::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	std::string strPAGES[LEVELEDIT_PAGE_SELECTION] = 
	{
		"Main Page",
		"BasicInfo Page",
		"Condition Page",
		"CrowList Page",
		"GateList Page",
		"EffectList Page",
		"LandMarkList Page",
		"Etch Functions",
	};

	SetWin_Combo_Init( this, IDC_COMBO_PAGE, strPAGES, LEVELEDIT_PAGE_SELECTION );

	return TRUE;  
}

void CPageMain::OnBnClickedButtonOpen()
{
	int nPAGE = GetWin_Combo_Sel( this, IDC_COMBO_PAGE );
	
	if ( m_pSheetTab )
		m_pSheetTab->PageActive( nPAGE );
}
