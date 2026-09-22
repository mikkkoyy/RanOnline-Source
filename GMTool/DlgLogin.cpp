// DlgLogin.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgLogin.h"

#include "EtcFunction.h"
#include "s_NetGlobal.h"
#include "Logic/GMToolOdbcBase.h"
#include "Logic/GMToolConfig.h"
#include "Logic/GMToolGlobal.h"
#include "Logic/GMToolData.h"

// CDlgLogin dialog

IMPLEMENT_DYNAMIC(CDlgLogin, CDialog)

CDlgLogin::CDlgLogin(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgLogin::IDD, pParent)
{

}

CDlgLogin::~CDlgLogin()
{
}

void CDlgLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgLogin, CDialog)
	ON_BN_CLICKED(IDC_LOGIN_BUTTON_LOGIN, OnBnClickedLoginButtonLogin)
END_MESSAGE_MAP()


// CDlgLogin message handlers
BOOL CDlgLogin::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWin_Check( this, IDC_LOGIN_CHECK_MD5, TRUE );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgLogin::OnOK()
{
	if ( !DoLogin() )	return;

	CDialog::OnOK();
}

void CDlgLogin::OnCancel()
{
	CDialog::OnCancel();
}

void CDlgLogin::OnBnClickedLoginButtonLogin()
{
	OnOK();
}

BOOL CDlgLogin::DoLogin()
{
	CString strUser = GetWin_Text( this, IDC_LOGIN_EDIT_USER );
	CString strPass = GetWin_Text( this, IDC_LOGIN_EDIT_PASS );
	BOOL bMD5 = GetWin_Check( this, IDC_LOGIN_CHECK_MD5 );

	if ( strUser.IsEmpty() ){
		CDebugSet::MsgBoxAfx( "User Empty!" );
		GotoDlgCtrl(GetDlgItem(IDC_LOGIN_EDIT_USER)); 
		return FALSE;
	}

	if ( strPass.IsEmpty() ){
		CDebugSet::MsgBoxAfx( "Pass Empty!" );
		GotoDlgCtrl(GetDlgItem(IDC_LOGIN_EDIT_PASS)); 
		return FALSE;
	}

	if ( strUser.GetLength() >= USR_ID_LENGTH ){
		CDebugSet::MsgBoxAfx( "User cannot be more than: %d characters", USR_ID_LENGTH );
		GotoDlgCtrl(GetDlgItem(IDC_LOGIN_EDIT_USER)); 
		return FALSE;
	}

	if ( strPass.GetLength() >= USR_PASS_LENGTH ){
		CDebugSet::MsgBoxAfx( "Pass cannot be more than: %d characters", USR_PASS_LENGTH );
		GotoDlgCtrl(GetDlgItem(IDC_LOGIN_EDIT_PASS)); 
		return FALSE;
	}

	if ( GMTOOL_GLOBAL::IsBadString( strUser.GetString() ) ){
		CDebugSet::MsgBoxAfx( "User contains invalid characters!" );
		GotoDlgCtrl(GetDlgItem(IDC_LOGIN_EDIT_USER)); 
		return FALSE;
	}

	if ( GMTOOL_GLOBAL::IsBadString( strPass.GetString() ) ){
		CDebugSet::MsgBoxAfx( "Pass contains invalid characters!" );
		GotoDlgCtrl(GetDlgItem(IDC_LOGIN_EDIT_PASS)); 
		return FALSE;
	}

	std::string strPassword = strPass.GetString();
	if ( bMD5 ){
		strPassword = GMTOOL_GLOBAL::GMToolStringConvertToMD5( strPass.GetString(), USR_PASS_LENGTH );
	}

	TCHAR	szUserName[USR_ID_LENGTH];
	TCHAR	szPassword[USR_PASS_LENGTH];
	StringCchCopy( szUserName, USR_ID_LENGTH, strUser.GetString() );
	StringCchCopy( szPassword, USR_PASS_LENGTH, strPassword.c_str() );

	SGMTOOL_USER_GM sDATA;
	int nReturn = CGMToolOdbcBase::GetInstance()->GMLogin( szUserName, szPassword, &sDATA );
	if ( nReturn == DB_ERROR ){
		CDebugSet::MsgBoxAfx( "Unable to Connect to Database" );
		return FALSE;
	}

	if ( !sDATA.bValid ){
		CDebugSet::MsgBoxAfx( "Invalid GM User or Pass" );
		return FALSE;
	}

	if ( sDATA.dwUserNum <= 0 ){
		CDebugSet::MsgBoxAfx( "Invalid GM User or Pass" );
		return FALSE;
	}

	if ( sDATA.wUserType < USER_GM3 ){
		CDebugSet::MsgBoxAfx( "Invalid GM Type" );
		return FALSE;
	}

	BOOL bValidInfo = GMTOOL_CONFIG::SetLoginInfo( strUser.GetString(), sDATA.wUserType, sDATA.dwUserNum );
	if ( !bValidInfo ){
		return FALSE;
	}

	return TRUE;
}