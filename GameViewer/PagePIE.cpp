// PagePIE.cpp : implementation file
//

#include "stdafx.h"
#include "GameViewer.h"
#include "PagePIE.h"
#include "SheetWithTab.h"
#include "GameViewerView.h"
#include "DxPieceContainer.h"
#include "EtcFunction.h"

// CPagePIE dialog

IMPLEMENT_DYNAMIC(CPagePIE, CPropertyPage)

CPagePIE::CPagePIE(LOGFONT logfont)
	: CPropertyPage(CPagePIE::IDD)
	, m_pFont ( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPagePIE::~CPagePIE()
{
	SAFE_DELETE ( m_pFont );
}

void CPagePIE::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPagePIE, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_PIE_LOAD, OnBnClickedButtonPieLoad)
	ON_BN_CLICKED(IDC_BUTTON_PIE_SAVE, OnBnClickedButtonPieSave)
END_MESSAGE_MAP()


// CPagePIE message handlers
BOOL CPagePIE::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	
	return TRUE;
}

void CPagePIE::OnBnClickedButtonPieLoad()
{
	CString szFilterInput = "Piece (*.pie,*.pie15)|*.pie;*.pie15|";
	CFileDialog dlgInput(true,"*.*",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilterInput,this);
	dlgInput.m_ofn.lpstrInitialDir = DxPieceContainer::GetInstance().GetPath();
	if ( dlgInput.DoModal() != IDOK ) return;

	BOOL bLOAD = CGameViewerView::GetView()->DataLoadPIE( dlgInput.GetFileName().GetString() );
	if ( bLOAD )
	{
		DataShow();
		SetWin_Text( this, IDC_EDIT_FILENAME, dlgInput.GetFileName() );
	}
	else
	{
		SetWin_Text( this, IDC_EDIT_FILENAME, "" );
		DataReset();
	}
}

void CPagePIE::OnBnClickedButtonPieSave()
{
	CString szFilterInput = "Piece (*.pie,*.pie15)|*.pie;*.pie15|";
	CFileDialog dlgInput(false,"*.*",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilterInput,this);
	dlgInput.m_ofn.lpstrInitialDir = DxPieceContainer::GetInstance().GetPath();
	if ( dlgInput.DoModal() != IDOK ) return;

	BOOL bSAVE = CGameViewerView::GetView()->DataSavePIE( dlgInput.GetPathName().GetString() );
}

void CPagePIE::DataReset()
{

}

void CPagePIE::DataShow()
{
	DxPieceEdit* pPiece = CGameViewerView::GetView()->m_pPIE;
	if ( !pPiece )	return;

}
