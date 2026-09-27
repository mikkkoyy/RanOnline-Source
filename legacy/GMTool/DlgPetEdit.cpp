// DlgPetEdit.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgPetEdit.h"

#include "EtcFunction.h"
#include "Logic/GMToolOdbcBase.h"
#include "PagePet.h"
#include "GLPet.h"
#include "PetEditTab.h"

// CDlgPetEdit dialog

IMPLEMENT_DYNAMIC(CDlgPetEdit, CDialog)

CDlgPetEdit::CDlgPetEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPetEdit::IDD, pParent)
	, m_pParentWnd(NULL)
	, m_nDialogID(-1)
	, m_dwPetCharID(0)
	, m_dwPetID(0)
	, m_strNAME("")
	, m_pData( NULL )
{

}

CDlgPetEdit::CDlgPetEdit(int nDialogID, DWORD dwPETCHARID, DWORD dwPETID, std::string strNAME, CWnd* pParent /*= NULL*/)
	: CDialog(CDlgPetEdit::IDD, pParent)
	, m_pParentWnd(pParent)
	, m_nDialogID(nDialogID)
	, m_dwPetCharID(dwPETCHARID)
	, m_dwPetID(dwPETID)
	, m_strNAME(strNAME)
	, m_pData( NULL )
{
}

CDlgPetEdit::~CDlgPetEdit()
{
	SAFE_DELETE( m_pSheetTab );
	SAFE_DELETE( m_pData );
}

void CDlgPetEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgPetEdit, CDialog)
	ON_BN_CLICKED(IDOK2, OnBnClickedOk2)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgPetEdit message handlers
BOOL CDlgPetEdit::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_pData = new GLPET();

	int nRETURN = CGMToolOdbcBase::GetInstance()->PetDataGet( m_dwPetID, m_dwPetCharID, m_pData );
	if ( nRETURN != DB_OK ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Failure to Read Pet Data! Info( PetID:%d PetCharID:%d PetName:%s )", m_dwPetID, m_dwPetCharID, m_strNAME.c_str() );
		return TRUE;
	}

	CRect SheetRect;
	CWnd *pWnd;
	pWnd = GetDlgItem ( IDC_STATIC_TOOL_PETEDIT );
	pWnd->GetWindowRect ( SheetRect );
	ScreenToClient ( &SheetRect );
	LOGFONT logfont = {0};

	CFont* pFont = GetFont();
	if ( pFont ) pFont->GetLogFont( &logfont );

	m_pSheetTab = new CPetEditTab ( m_pData, SheetRect, logfont, this );
	m_pSheetTab->Create( this, WS_CHILD|WS_VISIBLE|WS_TABSTOP, 0 );
	m_pSheetTab->ModifyStyleEx (0, WS_EX_CONTROLPARENT);
	m_pSheetTab->InitPages();

	SetWin_Num_int( this, IDC_PETEDIT_EDIT_PETCHANUM, m_dwPetCharID );
	SetWin_Num_int( this, IDC_PETEDIT_EDIT_PETNUM, m_dwPetID );
	SetWin_Text( this, IDC_PETEDIT_EDIT_PETNAME, m_strNAME.c_str() );

	CString strWindowText;
	strWindowText.Format( "Edit Pet:(%d) %s", m_dwPetID, m_strNAME.c_str() );
	SetWindowText( strWindowText.GetString() );

	return TRUE;  
}

void CDlgPetEdit::PostNcDestroy()
{
	CDialog::PostNcDestroy();

	if ( m_pParentWnd ){
		((CPagePet*)m_pParentWnd)->DialogDeleteID(m_nDialogID);
	}

	delete this;
}

void CDlgPetEdit::OnOK()
{
	if(UpdateData(true)){
		DestroyWindow();
	}
}

void CDlgPetEdit::OnCancel()
{
	DestroyWindow();
}

void CDlgPetEdit::OnBnClickedOk2()
{
	if ( DataSave()){
		CDebugSet::MsgBox( GetSafeHwnd(), "Save Done!" );
	}
}

void CDlgPetEdit::OnBnClickedOk()
{
	if ( DataSave()){
		OnOK();
	}
}

void CDlgPetEdit::OnBnClickedCancel()
{
	OnCancel();
}

BOOL CDlgPetEdit::DataSave()
{
	if ( !m_pData ){
		return FALSE;
	}

	if ( m_pSheetTab ){
		if ( !m_pSheetTab->SaveInfo() )
			return FALSE;
	}

	BOOL bONLINE = FALSE;
	CGMToolOdbcBase::GetInstance()->CharOnlineCheck( m_dwPetCharID, bONLINE );
	if ( bONLINE ){
		if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Owner[%d] of this pet: [%d]%s is Online! continue?", m_dwPetCharID, m_dwPetID, m_strNAME.c_str() ) != IDYES ){
			return FALSE;
		}
	}

	int nRETURN = CGMToolOdbcBase::GetInstance()->PetDataSave( m_pData );
	if( nRETURN != DB_OK ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Failure to Save Pet Data! Info( PetOwner:%d Pet:[%d]%s )", m_dwPetCharID, m_dwPetID, m_strNAME.c_str() );
		return FALSE;
	}

	return TRUE;
}