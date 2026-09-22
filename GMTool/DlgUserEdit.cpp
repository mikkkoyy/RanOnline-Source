// DlgUserEdit.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgUserEdit.h"


#include "EtcFunction.h"
#include "HLibTimeFunctions.h"
#include "HLibDataConvert.h"
#include "Logic/GMToolData.h"
#include "Logic/GMToolGlobal.h"
#include "Logic/GMToolOdbcBase.h"
#include "DlgDatetime.h"
#include "PageUser.h"

// CDlgUserEdit dialog

IMPLEMENT_DYNAMIC(CDlgUserEdit, CDialog)

CDlgUserEdit::CDlgUserEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgUserEdit::IDD, pParent)
	, m_pParentWnd(NULL)
	, m_nDialogID(-1)
	, m_dwID(0)
	, m_strNAME("")
	, m_pData(NULL)
{

}

CDlgUserEdit::CDlgUserEdit(int nDialogID, DWORD dwID, std::string strNAME, CWnd* pParent /*= NULL*/)
	: CDialog(CDlgUserEdit::IDD, pParent)
	, m_pParentWnd(pParent)
	, m_nDialogID(nDialogID)
	, m_dwID(dwID)
	, m_strNAME(strNAME)
	, m_pData(NULL)
{

}

CDlgUserEdit::~CDlgUserEdit()
{
	SAFE_DELETE( m_pData );
}

void CDlgUserEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgUserEdit, CDialog)
	ON_BN_CLICKED(IDC_USEREDIT_BUTTON_MD5, OnBnClickedUsereditButtonMd5)
	ON_BN_CLICKED(IDC_USEREDIT_BUTTON_MD52, OnBnClickedUsereditButtonMd52)
	ON_BN_CLICKED(IDC_USEREDIT_BUTTON_USERBLOCK_SET, OnBnClickedUsereditButtonUserblockSet)
	ON_BN_CLICKED(IDC_USEREDIT_BUTTON_USERBLOCK_RESET, OnBnClickedUsereditButtonUserblockReset)
	ON_BN_CLICKED(IDC_USEREDIT_BUTTON_CHATBLOCK_SET, OnBnClickedUsereditButtonChatblockSet)
	ON_BN_CLICKED(IDC_USEREDIT_BUTTON_CHATBLOCK_RESET, OnBnClickedUsereditButtonChatblockReset)
	ON_BN_CLICKED(IDC_USEREDIT_BUTTON_PREMIUMDATE_SET, OnBnClickedUsereditButtonPremiumdateSet)
	ON_BN_CLICKED(IDC_USEREDIT_BUTTON_PREMIUMDATE_RESET, OnBnClickedUsereditButtonPremiumdateReset)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDOK2, OnBnClickedOk2)
	
END_MESSAGE_MAP()


// CDlgUserEdit message handlers
BOOL CDlgUserEdit::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString strWindowText;
	strWindowText.Format( "Edit User:(%d) %s", m_dwID, m_strNAME.c_str() );
	SetWindowText( strWindowText.GetString() );

	std::string	strUserLevel[USERLEVEL_INDEX_SIZE];
	for( int i=0; i<USERLEVEL_INDEX_SIZE; ++i )
	{
		CString strText;
		strText.Format( "(%d)%s", UserIndexToType((USERLEVEL_INDEX)i), GMTOOL_GLOBAL::strUserLevelIndex[i].c_str());
		strUserLevel[i] = strText.GetString();
	}

	std::string strAccType[EM_TYPE_SIZE];
	for( int i=0; i<EM_TYPE_SIZE; ++i )
	{
		CString strText;
		strText.Format("(%d)%s", i, GMTOOL_GLOBAL::strAccountLevelIndex[i].c_str());
		strAccType[i] = strText.GetString();
	}

	SetWin_Combo_Init( this, IDC_USEREDIT_COMBO_USERTYPE, strUserLevel, USERLEVEL_INDEX_SIZE );
	SetWin_Combo_Init( this, IDC_USEREDIT_COMBO_LOGINSTATE, GMTOOL_GLOBAL::strUserStatus, GMTOOL_USER_STATUS_SIZE );
	SetWin_Combo_Init( this, IDC_USEREDIT_COMBO_AVAILABLE, GMTOOL_GLOBAL::strBasic, GMTOOL_BASIC_SIZE );
	SetWin_Combo_Init( this, IDC_USEREDIT_COMBO_USERBLOCK, GMTOOL_GLOBAL::strBasic, GMTOOL_BASIC_SIZE );

	SetWin_Combo_Init( this, IDC_USEREDIT_COMBO_USERTYPE2, strAccType, EM_TYPE_SIZE );

	m_pData = new SGMTOOL_DATA_USER;
	int nReturn = CGMToolOdbcBase::GetInstance()->UserDataGet( m_dwID, m_pData );
	if ( nReturn != DB_OK ){
		SAFE_DELETE( m_pData );
		CDebugSet::MsgBox( GetSafeHwnd(), "Unable to Get UserData! UserNumber:%d", m_dwID );
	}

	DataDefault();
	DataShow();
	
	return TRUE;  
}

void CDlgUserEdit::PostNcDestroy()
{
	CDialog::PostNcDestroy();

	if ( m_pParentWnd ){
		((CPageUser*)m_pParentWnd)->DialogDeleteID(m_nDialogID);
	}

	delete this;
}

void CDlgUserEdit::OnOK()
{
	if(UpdateData(true)){
		DestroyWindow();
	}
}

void CDlgUserEdit::OnCancel()
{
	DestroyWindow();
}

void CDlgUserEdit::DataDefault()
{
	SetWin_Num_int( this, IDC_USEREDIT_EDIT_NUM, 0 );
	SetWin_Text( this, IDC_USEREDIT_EDIT_NAME, "" );
	SetWin_Text( this, IDC_USEREDIT_EDIT_PASS, "" );
	SetWin_Text( this, IDC_USEREDIT_EDIT_PASS2, "" );
	SetWin_Text( this, IDC_USEREDIT_EDIT_EMAIL, "" );
	SetWin_Text( this, IDC_USEREDIT_EDIT_USERBLOCK, "" );
	SetWin_Text( this, IDC_USEREDIT_EDIT_CHATBLOCK, "" );
	SetWin_Text( this, IDC_USEREDIT_EDIT_PREMIUMDATE, "" );
	SetWin_Text( this, IDC_USEREDIT_EDIT_CREATEDATE, "" );
	SetWin_Text( this, IDC_USEREDIT_EDIT_LOGINDATE, "" );
	SetWin_Num_int( this, IDC_USEREDIT_EDIT_SERVERNUMBER, 0 );
	SetWin_Num_int( this, IDC_USEREDIT_EDIT_SERVERGROUP, 0 );
	SetWin_Num_int( this, IDC_USEREDIT_EDIT_CHAREMAIN, 0 );
	SetWin_Num_int( this, IDC_USEREDIT_EDIT_USERPOINT, 0 );
	SetWin_Combo_Sel( this, IDC_USEREDIT_COMBO_USERTYPE, 0 );
	SetWin_Combo_Sel( this, IDC_USEREDIT_COMBO_LOGINSTATE, 0 );
	SetWin_Combo_Sel( this, IDC_USEREDIT_COMBO_AVAILABLE, 0 );
	SetWin_Combo_Sel( this, IDC_USEREDIT_COMBO_USERBLOCK, 0);

	SetWin_Combo_Sel( this, IDC_USEREDIT_COMBO_USERTYPE2, 0 );
}

void CDlgUserEdit::DataShow()
{
	if ( !m_pData ){
		return;
	}

	SetWin_Num_int( this, IDC_USEREDIT_EDIT_NUM, m_pData->dwUserNum );
	SetWin_Text( this, IDC_USEREDIT_EDIT_NAME, m_pData->szUserID );
	SetWin_Text( this, IDC_USEREDIT_EDIT_PASS, m_pData->szUserPass );
	SetWin_Text( this, IDC_USEREDIT_EDIT_PASS2, m_pData->szUserPass2 );
	SetWin_Text( this, IDC_USEREDIT_EDIT_EMAIL, m_pData->szUserEmail );
	SetWin_Text( this, IDC_USEREDIT_EDIT_USERBLOCK, _HLIB::cstring_timet12( m_pData->tUserBlockDate ).GetString() );
	SetWin_Text( this, IDC_USEREDIT_EDIT_CHATBLOCK, _HLIB::cstring_timet12( m_pData->tChatBlockDate ).GetString() );
	SetWin_Text( this, IDC_USEREDIT_EDIT_PREMIUMDATE, _HLIB::cstring_timet12( m_pData->tPremiumDate ).GetString() );
	SetWin_Text( this, IDC_USEREDIT_EDIT_CREATEDATE, _HLIB::cstring_timet12( m_pData->tCreateDate ).GetString() );
	SetWin_Text( this, IDC_USEREDIT_EDIT_LOGINDATE, _HLIB::cstring_timet12( m_pData->tLastLoginDate ).GetString() );
	SetWin_Num_int( this, IDC_USEREDIT_EDIT_SERVERNUMBER, m_pData->wServerNumber );
	SetWin_Num_int( this, IDC_USEREDIT_EDIT_SERVERGROUP, m_pData->wServerGroup );
	SetWin_Num_int( this, IDC_USEREDIT_EDIT_CHAREMAIN, m_pData->wChaRemain );
	SetWin_Num_int( this, IDC_USEREDIT_EDIT_USERPOINT, m_pData->dwUserPoint );
	SetWin_Combo_Sel( this, IDC_USEREDIT_COMBO_USERTYPE, UserTypeToIndex( (EMUSERTYPE)m_pData->wUserType ) );
	SetWin_Combo_Sel( this, IDC_USEREDIT_COMBO_LOGINSTATE, GMTOOL_GLOBAL::strUserStatus[m_pData->bUserLoginState].c_str() );
	SetWin_Combo_Sel( this, IDC_USEREDIT_COMBO_AVAILABLE, GMTOOL_GLOBAL::strBasic[m_pData->bUserAvailable].c_str() );
	SetWin_Combo_Sel( this, IDC_USEREDIT_COMBO_USERBLOCK, GMTOOL_GLOBAL::strBasic[m_pData->bUserBlock].c_str());

	SetWin_Combo_Sel( this, IDC_USEREDIT_COMBO_USERTYPE2, m_pData->wUserType2 );
}

BOOL CDlgUserEdit::DataSave()
{
	if ( !m_pData ){
		return FALSE;
	}

	BOOL bONLINE = FALSE;
	CGMToolOdbcBase::GetInstance()->UserOnlineCheck( m_pData->dwUserNum, bONLINE );

	if ( bONLINE ){
		if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "User: [%d]%s is Online! continue?", m_pData->dwUserNum, m_pData->szUserID ) != IDYES ){
			return FALSE;
		}
	}

	std::string strPass = GetWin_Text( this, IDC_USEREDIT_EDIT_PASS );
	std::string strPass2 = GetWin_Text( this, IDC_USEREDIT_EDIT_PASS2 );
	std::string strEmail = GetWin_Text( this, IDC_USEREDIT_EDIT_EMAIL );
	int nCharRemain = GetWin_Num_int( this, IDC_USEREDIT_EDIT_CHAREMAIN );
	int nUserPoints = GetWin_Num_int( this, IDC_USEREDIT_EDIT_USERPOINT );
	BOOL bLoginState = GetWin_Combo_Sel( this, IDC_USEREDIT_COMBO_LOGINSTATE );
	BOOL bAvailable = GetWin_Combo_Sel( this, IDC_USEREDIT_COMBO_AVAILABLE );
	BOOL bUserBlock = GetWin_Combo_Sel( this, IDC_USEREDIT_COMBO_USERBLOCK );
	USERLEVEL_INDEX emUserTypeIndex = static_cast<USERLEVEL_INDEX> ( GetWin_Combo_Sel( this, IDC_USEREDIT_COMBO_USERTYPE ) );
	EMACCTYPE emAccTypeIndex = static_cast<EMACCTYPE> ( GetWin_Combo_Sel( this, IDC_USEREDIT_COMBO_USERTYPE2 ) );

	if ( strPass.size() < MIN_USER_PASS || strPass.size() > MAX_USER_PASS ){
		CDebugSet::MsgBox( GetSafeHwnd(), "UserPass can only have %d to %d characters", MIN_USER_PASS, MAX_USER_PASS );
		GotoDlgCtrl(GetDlgItem(IDC_USEREDIT_EDIT_PASS)); 
		return FALSE;
	}

	if ( strPass2.size() < MIN_USER_PASS || strPass2.size() > MAX_USER_PASS ){
		CDebugSet::MsgBox( GetSafeHwnd(), "UserPass2 can only have %d to %d characters", MIN_USER_PASS, MAX_USER_PASS );
		GotoDlgCtrl(GetDlgItem(IDC_USEREDIT_EDIT_PASS2)); 
		return FALSE;
	}

	if ( strEmail.size() > USR_EMAIL_LENGTH ){
		CDebugSet::MsgBox( GetSafeHwnd(), "UserEmail can only have 0 to %d characters", USR_EMAIL_LENGTH );
		GotoDlgCtrl(GetDlgItem(IDC_USEREDIT_EDIT_EMAIL)); 
		return FALSE;
	}

	if ( GMTOOL_GLOBAL::IsBadString( strPass.c_str() ) ){
		CDebugSet::MsgBox( GetSafeHwnd(), "UserPass contains invalid characters!" );
		GotoDlgCtrl(GetDlgItem(IDC_USEREDIT_EDIT_PASS)); 
		return FALSE;
	}

	if ( GMTOOL_GLOBAL::IsBadString( strPass2.c_str() ) ){
		CDebugSet::MsgBox( GetSafeHwnd(), "UserPass2 contains invalid characters!" );
		GotoDlgCtrl(GetDlgItem(IDC_USEREDIT_EDIT_PASS2)); 
		return FALSE;
	}

	if ( GMTOOL_GLOBAL::IsBadString2( strEmail.c_str() ) ){
		CDebugSet::MsgBox( GetSafeHwnd(), "UserEmail contains invalid characters!" );
		GotoDlgCtrl(GetDlgItem(IDC_USEREDIT_EDIT_EMAIL)); 
		return FALSE;
	}

	if ( nCharRemain < 0 || nCharRemain >= MAX_ONESERVERCHAR_NUM ){
		CDebugSet::MsgBox( GetSafeHwnd(), "ChaRemain can only have 0 to %d value", MAX_ONESERVERCHAR_NUM );
		GotoDlgCtrl(GetDlgItem(IDC_USEREDIT_EDIT_CHAREMAIN)); 
		return FALSE;
	}

	if ( nUserPoints < 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "User Points invalid" );
		GotoDlgCtrl(GetDlgItem(IDC_USEREDIT_EDIT_USERPOINT)); 
		return FALSE;
	}

	StringCchCopy( m_pData->szUserPass, USR_PASS_LENGTH, strPass.c_str() );
	StringCchCopy( m_pData->szUserPass2, USR_PASS_LENGTH, strPass2.c_str() );
	StringCchCopy( m_pData->szUserEmail, USR_EMAIL_LENGTH, strEmail.c_str() );
	m_pData->wUserType = (WORD)UserIndexToType( emUserTypeIndex );
	m_pData->wChaRemain = (WORD)nCharRemain;
	m_pData->dwUserPoint = (WORD)nUserPoints;
	m_pData->bUserLoginState = bLoginState;
	m_pData->bUserAvailable = bAvailable;
	m_pData->bUserBlock = bUserBlock;
	m_pData->wUserType2 = (WORD)emAccTypeIndex ;

	int nReturn = CGMToolOdbcBase::GetInstance()->UserDataSave( m_pData );
	if ( nReturn != DB_OK ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Unable to Save UserData! UserNumber:%d", m_pData->dwUserNum );
		return FALSE;
	}

	return TRUE;
}

void CDlgUserEdit::OnBnClickedUsereditButtonMd5()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Do you want to convert UserPass to MD5?" ) == IDYES )
	{
		std::string strTEXT = GMTOOL_GLOBAL::GMToolStringConvertToMD5( GetWin_Text( this, IDC_USEREDIT_EDIT_PASS ).GetString(), MAX_USER_PASS );
		SetWin_Text( this, IDC_USEREDIT_EDIT_PASS, strTEXT.c_str() );
	}
}

void CDlgUserEdit::OnBnClickedUsereditButtonMd52()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Do you want to convert UserPass2 to MD5?" ) == IDYES )
	{
		std::string strTEXT = GMTOOL_GLOBAL::GMToolStringConvertToMD5( GetWin_Text( this, IDC_USEREDIT_EDIT_PASS2 ).GetString(), MAX_USER_PASS );
		SetWin_Text( this, IDC_USEREDIT_EDIT_PASS2, strTEXT.c_str() );
	}
}

void CDlgUserEdit::OnBnClickedUsereditButtonUserblockSet()
{
	if ( !m_pData ){
		return;
	}

	__time64_t ttime = m_pData->tUserBlockDate;
	if ( ttime < CTime::GetCurrentTime().GetTime() ){
		ttime = CTime::GetCurrentTime().GetTime();
	}

	CDlgDatetime dlg( this );
	dlg.DateSetMin( CTime::GetCurrentTime().GetTime() );
	dlg.DateTimeSet( ttime );
	if ( dlg.DoModal() == IDOK ){
		m_pData->tUserBlockDate = dlg.DateTimeGet();
		SetWin_Text( this, IDC_USEREDIT_EDIT_USERBLOCK, _HLIB::cstring_timet12( m_pData->tUserBlockDate ).GetString() );
	}
}

void CDlgUserEdit::OnBnClickedUsereditButtonUserblockReset()
{
	if ( !m_pData ){
		return;
	}

	m_pData->tUserBlockDate = CTime::GetCurrentTime().GetTime();
	SetWin_Text( this, IDC_USEREDIT_EDIT_USERBLOCK, _HLIB::cstring_timet12( m_pData->tUserBlockDate ).GetString() );
}

void CDlgUserEdit::OnBnClickedUsereditButtonChatblockSet()
{
	if ( !m_pData ){
		return;
	}

	__time64_t ttime = m_pData->tChatBlockDate;
	if ( ttime < CTime::GetCurrentTime().GetTime() ){
		ttime = CTime::GetCurrentTime().GetTime();
	}

	CDlgDatetime dlg( this );
	dlg.DateSetMin( CTime::GetCurrentTime().GetTime() );
	dlg.DateTimeSet( ttime );
	if ( dlg.DoModal() == IDOK ){
		m_pData->tChatBlockDate = dlg.DateTimeGet();
		SetWin_Text( this, IDC_USEREDIT_EDIT_CHATBLOCK, _HLIB::cstring_timet12( m_pData->tChatBlockDate ).GetString() );
	}
}

void CDlgUserEdit::OnBnClickedUsereditButtonChatblockReset()
{
	if ( !m_pData ){
		return;
	}

	CTime cTime( 1970, 2, 1, 0, 0, 0 );
	m_pData->tChatBlockDate = cTime.GetTime();
	SetWin_Text( this, IDC_USEREDIT_EDIT_CHATBLOCK, _HLIB::cstring_timet12( m_pData->tChatBlockDate ).GetString() );
}

void CDlgUserEdit::OnBnClickedUsereditButtonPremiumdateSet()
{
	if ( !m_pData ){
		return;
	}

	__time64_t ttime = m_pData->tPremiumDate;
	if ( ttime < CTime::GetCurrentTime().GetTime() ){
		ttime = CTime::GetCurrentTime().GetTime();
	}

	CDlgDatetime dlg( this );
	dlg.DateSetMin( CTime::GetCurrentTime().GetTime() );
	dlg.DateTimeSet( ttime );
	if ( dlg.DoModal() == IDOK ){
		m_pData->tPremiumDate = dlg.DateTimeGet();
		SetWin_Text( this, IDC_USEREDIT_EDIT_PREMIUMDATE, _HLIB::cstring_timet12( m_pData->tPremiumDate ).GetString() );
	}	
}

void CDlgUserEdit::OnBnClickedUsereditButtonPremiumdateReset()
{
	if ( !m_pData ){
		return;
	}

	CTime cTime( 1970, 2, 1, 0, 0, 0 );
	m_pData->tPremiumDate = cTime.GetTime();
	SetWin_Text( this, IDC_USEREDIT_EDIT_PREMIUMDATE, _HLIB::cstring_timet12( m_pData->tPremiumDate ).GetString() );
}

void CDlgUserEdit::OnBnClickedOk()
{
	if ( DataSave()){
		OnOK();
	}
}

void CDlgUserEdit::OnBnClickedCancel()
{
	OnCancel();
}

void CDlgUserEdit::OnBnClickedOk2()
{
	if ( DataSave()){
		CDebugSet::MsgBox( GetSafeHwnd(), "Save Done!" );
	}
}
