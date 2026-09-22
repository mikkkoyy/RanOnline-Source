// DlgCharEdit.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgCharEdit.h"

#include "CharEditTab.h"
#include "EtcFunction.h"
#include "GLCharData.h"
#include "Logic/GMToolOdbcBase.h"
#include "PageChar.h"

// CDlgCharEdit dialog

IMPLEMENT_DYNAMIC(CDlgCharEdit, CDialog)

CDlgCharEdit::CDlgCharEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCharEdit::IDD, pParent)
	, m_pParentWnd(NULL)
	, m_nDialogID(-1)
	, m_dwUserID(0)
	, m_dwCharID(0)
	, m_strNAME("")
	, m_pData( NULL )
{
}

CDlgCharEdit::CDlgCharEdit(int nDialogID, DWORD dwUserID, DWORD dwCharID, std::string strNAME, CWnd* pParent /*= NULL*/)
	: CDialog(CDlgCharEdit::IDD, pParent)
	, m_pParentWnd(pParent)
	, m_nDialogID(nDialogID)
	, m_dwUserID(dwUserID)
	, m_dwCharID(dwCharID)
	, m_strNAME(strNAME)
	, m_pData( NULL )
{
}

CDlgCharEdit::~CDlgCharEdit()
{
	SAFE_DELETE( m_pSheetTab );
	SAFE_DELETE( m_pData );
}

void CDlgCharEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgCharEdit, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDOK2, OnBnClickedOk2)
END_MESSAGE_MAP()


// CDlgCharEdit message handlers
BOOL CDlgCharEdit::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_pData = new SCHARDATA2();

	int nRETURN = CGMToolOdbcBase::GetInstance()->CharDataGet( m_dwUserID, m_dwCharID, m_pData );
	if ( nRETURN != DB_OK ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Failure to Read Character Data! Info( UserID:%d CharID:%d ChaName:%s )", m_dwUserID, m_dwCharID, m_strNAME.c_str() );
		return TRUE;
	}

	CRect SheetRect;
	CWnd *pWnd;
	pWnd = GetDlgItem ( IDC_STATIC_TOOL );
	pWnd->GetWindowRect ( SheetRect );
	ScreenToClient ( &SheetRect );
	LOGFONT logfont = {0};

	CFont* pFont = GetFont();
	if ( pFont ) pFont->GetLogFont( &logfont );

	m_pSheetTab = new CCharEditTab ( m_pData, SheetRect, logfont, this );
	m_pSheetTab->Create( this, WS_CHILD|WS_VISIBLE|WS_TABSTOP, 0 );
	m_pSheetTab->ModifyStyleEx (0, WS_EX_CONTROLPARENT);
	m_pSheetTab->InitPages();

	SetWin_Num_int( this, IDC_CHAREDIT_EDIT_USERNUM, m_dwUserID );
	SetWin_Num_int( this, IDC_CHAREDIT_EDIT_CHANUM, m_dwCharID );
	SetWin_Text( this, IDC_CHAREDIT_EDIT_CHANAME, m_strNAME.c_str() );
	CString strWindowText;

	strWindowText.Format( "Edit Character:(%d) %s", m_dwCharID, m_strNAME.c_str() );
	SetWindowText( strWindowText.GetString() );

	return TRUE;  
}

void CDlgCharEdit::PostNcDestroy()
{
	CDialog::PostNcDestroy();

	if ( m_pParentWnd ){
		((CPageChar*)m_pParentWnd)->DialogDeleteID(m_nDialogID);
	}

	delete this;
}

void CDlgCharEdit::OnOK()
{
	if(UpdateData(true)){
		DestroyWindow();
	}
}

BOOL CDlgCharEdit::DataSave()
{
	if ( !m_pData ){
		return FALSE;
	}

	if ( m_pSheetTab ){
		if ( !m_pSheetTab->SaveInfo() )
			return FALSE;
	}

	BOOL bONLINE = FALSE;
	CGMToolOdbcBase::GetInstance()->CharOnlineCheck( m_pData->m_dwCharID, bONLINE );

	if ( bONLINE ){
		if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Character: [%d]%s is Online! continue?", m_pData->m_dwCharID, m_pData->m_szName ) != IDYES ){
			return FALSE;
		}
	}

	int nRETURN = CGMToolOdbcBase::GetInstance()->CharDataSave( m_pData );
	if( nRETURN != DB_OK ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Failure to Save Character Data! Info( User:%d Char:%d Name:%s )", m_dwUserID, m_dwCharID, m_strNAME.c_str() );
		return FALSE;
	}

	return TRUE;
}

void CDlgCharEdit::OnCancel()
{
	DestroyWindow();
}

void CDlgCharEdit::OnBnClickedOk()
{
	if ( DataSave()){
		OnOK();
	}
}

void CDlgCharEdit::OnBnClickedCancel()
{
	OnCancel();
}

void CDlgCharEdit::OnBnClickedOk2()
{
	if ( DataSave()){
		CDebugSet::MsgBox( GetSafeHwnd(), "Save Done!" );
	}
}
