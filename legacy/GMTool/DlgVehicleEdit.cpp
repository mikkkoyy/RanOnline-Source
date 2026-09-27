// DlgVehicleEdit.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgVehicleEdit.h"

#include "EtcFunction.h"
#include "Logic/GMToolOdbcBase.h"
#include "PageVehicle.h"
#include "GLVEHICLE.h"
#include "VehicleEditTab.h"

// CDlgVehicleEdit dialog

IMPLEMENT_DYNAMIC(CDlgVehicleEdit, CDialog)

CDlgVehicleEdit::CDlgVehicleEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgVehicleEdit::IDD, pParent)
	, m_pParentWnd(NULL)
	, m_nDialogID(-1)
	, m_dwVehicleCharID(0)
	, m_dwVehicleID(0)
	, m_strNAME("")
	, m_pData( NULL )
{

}

CDlgVehicleEdit::CDlgVehicleEdit(int nDialogID, DWORD dwVEHICLECHARID, DWORD dwVEHICLEID, std::string strNAME, CWnd* pParent /*= NULL*/)
	: CDialog(CDlgVehicleEdit::IDD, pParent)
	, m_pParentWnd(pParent)
	, m_nDialogID(nDialogID)
	, m_dwVehicleCharID(dwVEHICLECHARID)
	, m_dwVehicleID(dwVEHICLEID)
	, m_strNAME(strNAME)
	, m_pData( NULL )
{
}

CDlgVehicleEdit::~CDlgVehicleEdit()
{
	SAFE_DELETE( m_pSheetTab );
	SAFE_DELETE( m_pData );
}

void CDlgVehicleEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgVehicleEdit, CDialog)
	ON_BN_CLICKED(IDOK2, OnBnClickedOk2)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgVehicleEdit message handlers
BOOL CDlgVehicleEdit::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_pData = new GLVEHICLE();

	int nRETURN = CGMToolOdbcBase::GetInstance()->VehicleDataGet( m_dwVehicleID, m_dwVehicleCharID, m_pData );
	if ( nRETURN != DB_OK ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Failure to Read Vehicle Data! Info( VehicleNum:%d VehicleChaNum:%d VehicleName:%s )", m_dwVehicleID, m_dwVehicleCharID, m_strNAME.c_str() );
		return TRUE;
	}

	CRect SheetRect;
	CWnd *pWnd;
	pWnd = GetDlgItem ( IDC_STATIC_TOOL_VEHICLEEDIT );
	pWnd->GetWindowRect ( SheetRect );
	ScreenToClient ( &SheetRect );
	LOGFONT logfont = {0};

	CFont* pFont = GetFont();
	if ( pFont ) pFont->GetLogFont( &logfont );

	m_pSheetTab = new CVehicleEditTab ( m_pData, SheetRect, logfont, this );
	m_pSheetTab->Create( this, WS_CHILD|WS_VISIBLE|WS_TABSTOP, 0 );
	m_pSheetTab->ModifyStyleEx (0, WS_EX_CONTROLPARENT);
	m_pSheetTab->InitPages();

	SetWin_Num_int( this, IDC_VEHICLEEDIT_EDIT_VEHICLECHANUM, m_dwVehicleCharID );
	SetWin_Num_int( this, IDC_VEHICLEEDIT_EDIT_VEHICLENUM, m_dwVehicleID );
	SetWin_Text( this, IDC_VEHICLEEDIT_EDIT_VEHICLENAME, m_strNAME.c_str() );

	CString strWindowText;
	strWindowText.Format( "Edit Vehicle:(%d) %s", m_dwVehicleID, m_strNAME.c_str() );
	SetWindowText( strWindowText.GetString() );

	return TRUE;  
}

void CDlgVehicleEdit::PostNcDestroy()
{
	CDialog::PostNcDestroy();

	if ( m_pParentWnd ){
		((CPageVehicle*)m_pParentWnd)->DialogDeleteID(m_nDialogID);
	}

	delete this;
}

void CDlgVehicleEdit::OnOK()
{
	if(UpdateData(true)){
		DestroyWindow();
	}
}

void CDlgVehicleEdit::OnCancel()
{
	DestroyWindow();
}

void CDlgVehicleEdit::OnBnClickedOk2()
{
	if ( DataSave()){
		CDebugSet::MsgBox( GetSafeHwnd(), "Save Done!" );
	}
}

void CDlgVehicleEdit::OnBnClickedOk()
{
	if ( DataSave()){
		OnOK();
	}
}

void CDlgVehicleEdit::OnBnClickedCancel()
{
	OnCancel();
}

BOOL CDlgVehicleEdit::DataSave()
{
	if ( !m_pData ){
		return FALSE;
	}

	if ( m_pSheetTab ){
		if ( !m_pSheetTab->SaveInfo() )
			return FALSE;
	}

	BOOL bONLINE = FALSE;
	CGMToolOdbcBase::GetInstance()->CharOnlineCheck( m_dwVehicleCharID, bONLINE );
	if ( bONLINE ){
		if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Owner[%d] of this Vehicle: [%d]%s is Online! continue?", m_dwVehicleCharID, m_dwVehicleID, m_strNAME.c_str() ) != IDYES ){
			return FALSE;
		}
	}

	int nRETURN = CGMToolOdbcBase::GetInstance()->VehicleDataSave( m_pData );
	if( nRETURN != DB_OK ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Failure to Save Vehicle Data! Info( VehicleOwner:%d  Vehicle:[%d]%s )", m_dwVehicleCharID, m_dwVehicleID, m_strNAME.c_str() );
		return FALSE;
	}

	return TRUE;
}
