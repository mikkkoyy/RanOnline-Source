// DlgUserLogLogin.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgUserLogLogin.h"

#include "EtcFunction.h"
#include "HLibTimeFunctions.h"
#include "HLibDataConvert.h"
#include "Logic/GMToolOdbcBase.h"
#include "Logic/GMToolData.h"
#include "Logic/GMToolGlobal.h"
#include "PageUser.h"

// CDlgUserLogLogin dialog

IMPLEMENT_DYNAMIC(CDlgUserLogLogin, CDialog)

CDlgUserLogLogin::CDlgUserLogLogin(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgUserLogLogin::IDD, pParent)
	, m_pParentWnd(NULL)
	, m_nDialogID(-1)
	, m_dwID(0)
	, m_strNAME("")
{
}

CDlgUserLogLogin::CDlgUserLogLogin(int nDialogID, DWORD dwID, std::string strNAME, CWnd* pParent /*= NULL*/)
	: CDialog(CDlgUserLogLogin::IDD, pParent)
	, m_pParentWnd(pParent)
	, m_nDialogID(nDialogID)
	, m_dwID(dwID)
	, m_strNAME(strNAME)
{
}

CDlgUserLogLogin::~CDlgUserLogLogin()
{
}

void CDlgUserLogLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOGLOGIN, m_List );
}


BEGIN_MESSAGE_MAP(CDlgUserLogLogin, CDialog)
	ON_BN_CLICKED(IDC_USERLOGLOGIN_BUTTON_READ_ALL, OnBnClickedUserlogloginButtonReadAll)
	ON_BN_CLICKED(IDC_USERLOGLOGIN_BUTTON_READ_IP, OnBnClickedUserlogloginButtonReadIp)
	ON_BN_CLICKED(IDC_USERLOGLOGIN_BUTTON_CLEAR, OnBnClickedUserlogloginButtonClear)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgUserLogLogin message handlers
BOOL CDlgUserLogLogin::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString strWindowText;
	strWindowText.Format( "LogLogin User:(%d) %s", m_dwID, m_strNAME.c_str() );
	SetWindowText( strWindowText.GetString() );

	SetWin_Num_int( this, IDC_USERLOGLOGIN_EDIT_NUM, m_dwID );
	SetWin_Text( this, IDC_USERLOGLOGIN_EDIT_NAME, m_strNAME.c_str() );

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "IP", LVCFMT_LEFT, ( 49* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Date", LVCFMT_LEFT, ( 49* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	return TRUE;  
}

void CDlgUserLogLogin::PostNcDestroy()
{
	CDialog::PostNcDestroy();

	if ( m_pParentWnd ){
		((CPageUser*)m_pParentWnd)->DialogDeleteID(m_nDialogID);
	}

	delete this;
}

void CDlgUserLogLogin::OnOK()
{
	if(UpdateData(true)){
		DestroyWindow();
	}
}

void CDlgUserLogLogin::OnCancel()
{
	DestroyWindow();
}

void CDlgUserLogLogin::OnBnClickedUserlogloginButtonReadAll()
{
	SGMTOOL_DATA_USER_IPLOG_VEC vecLog;
	CGMToolOdbcBase::GetInstance()->UserIPLOGRead( m_dwID, vecLog );

	if ( vecLog.size() <= 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "No Results!" );
		return;
	}
	
	m_List.DeleteAllItems();

	m_List.SetRedraw( FALSE );

	for( size_t i=0; i<vecLog.size(); ++i){
		SGMTOOL_DATA_USER_IPLOG sLog = vecLog[i];
		m_List.InsertItem( i, sLog.szLogIpAddress );
		m_List.SetItemText( i, 1, _HLIB::cstring_timet12( sLog.tLogDate ).GetString() );
	}

	m_List.SetRedraw( TRUE );
}

void CDlgUserLogLogin::OnBnClickedUserlogloginButtonReadIp()
{
	SGMTOOL_DATA_USER_IPLOG_MAP mapLog;
	CGMToolOdbcBase::GetInstance()->UserIPLOGRead( m_dwID, mapLog );

	if ( mapLog.size() <= 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "No Results!" );
		return;
	}

	m_List.DeleteAllItems();

	m_List.SetRedraw( FALSE );

	SGMTOOL_DATA_USER_IPLOG_MAP_ITER iter = mapLog.begin();
	for( int i=0; iter != mapLog.end(); ++ iter, ++ i ){
		const SGMTOOL_DATA_USER_IPLOG sLog = (*iter).second;
		m_List.InsertItem( i, sLog.szLogIpAddress );
		m_List.SetItemText( i, 1, _HLIB::cstring_timet12( sLog.tLogDate ).GetString() );
	}

	m_List.SetRedraw( TRUE );
}

void CDlgUserLogLogin::OnBnClickedUserlogloginButtonClear()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete all login log for user:(%d)%s ?", m_dwID, m_strNAME.c_str()) == IDYES ){
		CGMToolOdbcBase::GetInstance()->UserIPLOGErase( m_dwID );
		m_List.DeleteAllItems();
	}
}

void CDlgUserLogLogin::OnBnClickedCancel()
{
	OnCancel();
}
