// DlgConfig.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgConfig.h"

#include "Logic/GMToolConfig.h"
#include "EtcFunction.h"

// CDlgConfig dialog

IMPLEMENT_DYNAMIC(CDlgConfig, CDialog)

CDlgConfig::CDlgConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgConfig::IDD, pParent)
{

}

CDlgConfig::~CDlgConfig()
{
}

void CDlgConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgConfig, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgConfig message handlers
BOOL CDlgConfig::OnInitDialog()
{
	CDialog::OnInitDialog();

	DataShow();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgConfig::DataShow()
{
	SetWin_Text( this, IDC_EDIT_CONFIG_USERDB_SERVER, GMTOOL_CONFIG::m_szUserDBServer );
	SetWin_Text( this, IDC_EDIT_CONFIG_USERDB_DATABASE, GMTOOL_CONFIG::m_szUserDBDatabase );
	SetWin_Text( this, IDC_EDIT_CONFIG_USERDB_USER, GMTOOL_CONFIG::m_szUserDBUser );
	SetWin_Text( this, IDC_EDIT_CONFIG_USERDB_PASS, GMTOOL_CONFIG::m_szUserDBPass );
	SetWin_Num_int( this, IDC_EDIT_CONFIG_USERDB_PORT, GMTOOL_CONFIG::m_nUserDBPort );

	SetWin_Text( this, IDC_EDIT_CONFIG_GAMEDB_SERVER, GMTOOL_CONFIG::m_szGameDBServer );
	SetWin_Text( this, IDC_EDIT_CONFIG_GAMEDB_DATABASE, GMTOOL_CONFIG::m_szGameDBDatabase );
	SetWin_Text( this, IDC_EDIT_CONFIG_GAMEDB_USER, GMTOOL_CONFIG::m_szGameDBUser );
	SetWin_Text( this, IDC_EDIT_CONFIG_GAMEDB_PASS, GMTOOL_CONFIG::m_szGameDBPass );
	SetWin_Num_int( this, IDC_EDIT_CONFIG_GAMEDB_PORT, GMTOOL_CONFIG::m_nGameDBPort );

	SetWin_Text( this, IDC_EDIT_CONFIG_LOGDB_SERVER, GMTOOL_CONFIG::m_szLogDBServer );
	SetWin_Text( this, IDC_EDIT_CONFIG_LOGDB_DATABASE, GMTOOL_CONFIG::m_szLogDBDatabase );
	SetWin_Text( this, IDC_EDIT_CONFIG_LOGDB_USER, GMTOOL_CONFIG::m_szLogDBUser );
	SetWin_Text( this, IDC_EDIT_CONFIG_LOGDB_PASS, GMTOOL_CONFIG::m_szLogDBPass );
	SetWin_Num_int( this, IDC_EDIT_CONFIG_LOGDB_PORT, GMTOOL_CONFIG::m_nLogDBPort );

	SetWin_Text( this, IDC_EDIT_CONFIG_SHOPDB_SERVER, GMTOOL_CONFIG::m_szShopDBServer );
	SetWin_Text( this, IDC_EDIT_CONFIG_SHOPDB_DATABASE, GMTOOL_CONFIG::m_szShopDBDatabase );
	SetWin_Text( this, IDC_EDIT_CONFIG_SHOPDB_USER, GMTOOL_CONFIG::m_szShopDBUser );
	SetWin_Text( this, IDC_EDIT_CONFIG_SHOPDB_PASS, GMTOOL_CONFIG::m_szShopDBPass );
	SetWin_Num_int( this, IDC_EDIT_CONFIG_SHOPDB_PORT, GMTOOL_CONFIG::m_nShopDBPort );
	
	SetWin_Check( this, IDC_EDIT_CONFIG_LOG_SQLCODE, GMTOOL_CONFIG::m_bLogSQLCODE );
	SetWin_Check( this, IDC_EDIT_CONFIG_LOG_SQLERROR, GMTOOL_CONFIG::m_bLogSQLERROR );
	SetWin_Check( this, IDC_EDIT_CONFIG_LOG_DATAERROR, GMTOOL_CONFIG::m_bLogDATAERROR );

	SetWin_Check( this, IDC_EDIT_CONFIG_PRINT_SQLCODE, GMTOOL_CONFIG::m_bPrintSQLCODE );
	SetWin_Check( this, IDC_EDIT_CONFIG_PRINT_SQLERROR, GMTOOL_CONFIG::m_bPrintSQLERROR );
	SetWin_Check( this, IDC_EDIT_CONFIG_PRINT_DATAERROR, GMTOOL_CONFIG::m_bPrintDATAERROR );
}

BOOL CDlgConfig::DataSave()
{
	CString strUserDBServer = GetWin_Text( this, IDC_EDIT_CONFIG_USERDB_SERVER );
	CString strUserDBDatabase = GetWin_Text( this, IDC_EDIT_CONFIG_USERDB_DATABASE );
	CString strUserDBUser = GetWin_Text( this, IDC_EDIT_CONFIG_USERDB_USER );
	CString strUserDBPass = GetWin_Text( this, IDC_EDIT_CONFIG_USERDB_PASS );
	int nUserDBPort = GetWin_Num_int( this, IDC_EDIT_CONFIG_USERDB_PORT );

	CString strGameDBServer = GetWin_Text( this, IDC_EDIT_CONFIG_GAMEDB_SERVER );
	CString strGameDBDatabase = GetWin_Text( this, IDC_EDIT_CONFIG_GAMEDB_DATABASE );
	CString strGameDBUser = GetWin_Text( this, IDC_EDIT_CONFIG_GAMEDB_USER );
	CString strGameDBPass = GetWin_Text( this, IDC_EDIT_CONFIG_GAMEDB_PASS );
	int nGameDBPort = GetWin_Num_int( this, IDC_EDIT_CONFIG_GAMEDB_PORT );

	CString strLogDBServer = GetWin_Text( this, IDC_EDIT_CONFIG_LOGDB_SERVER );
	CString strLogDBDatabase = GetWin_Text( this, IDC_EDIT_CONFIG_LOGDB_DATABASE );
	CString strLogDBUser = GetWin_Text( this, IDC_EDIT_CONFIG_LOGDB_USER );
	CString strLogDBPass = GetWin_Text( this, IDC_EDIT_CONFIG_LOGDB_PASS );
	int nLogDBPort = GetWin_Num_int( this, IDC_EDIT_CONFIG_LOGDB_PORT );

	CString strShopDBServer = GetWin_Text( this, IDC_EDIT_CONFIG_SHOPDB_SERVER );
	CString strShopDBDatabase = GetWin_Text( this, IDC_EDIT_CONFIG_SHOPDB_DATABASE );
	CString strShopDBUser = GetWin_Text( this, IDC_EDIT_CONFIG_SHOPDB_USER );
	CString strShopDBPass = GetWin_Text( this, IDC_EDIT_CONFIG_SHOPDB_PASS );
	int nShopDBPort = GetWin_Num_int( this, IDC_EDIT_CONFIG_SHOPDB_PORT );

	if ( strUserDBServer.IsEmpty() || strUserDBUser.IsEmpty() || strUserDBPass.IsEmpty() || strUserDBDatabase.IsEmpty() ||
		strGameDBServer.IsEmpty() || strGameDBUser.IsEmpty() || strGameDBPass.IsEmpty() ||  strGameDBDatabase.IsEmpty() ||
		strLogDBServer.IsEmpty() || strLogDBUser.IsEmpty() || strLogDBPass.IsEmpty() || strLogDBDatabase.IsEmpty() ||
		strShopDBServer.IsEmpty() || strShopDBUser.IsEmpty() ||  strShopDBPass.IsEmpty() ||  strShopDBDatabase.IsEmpty() ){
		CDebugSet::MsgBox( GetSafeHwnd(), "String Data Empty!" );
		return FALSE;
	}

	if ( nUserDBPort <= 0 || nGameDBPort <= 0 || nLogDBPort <= 0 || nShopDBPort <= 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Port Data Value Invalid!" );
		return FALSE;
	}

	StringCchCopy( GMTOOL_CONFIG::m_szUserDBServer, DB_SVR_NAME_LENGTH, strUserDBServer.GetString() ); 
	StringCchCopy( GMTOOL_CONFIG::m_szUserDBDatabase, DB_SVR_NAME_LENGTH, strUserDBDatabase.GetString() ); 
	StringCchCopy( GMTOOL_CONFIG::m_szUserDBUser, USR_ID_LENGTH, strUserDBUser.GetString() ); 
	StringCchCopy( GMTOOL_CONFIG::m_szUserDBPass, GMC_DB_PASS_LENGTH, strUserDBPass.GetString() ); 
	
	StringCchCopy( GMTOOL_CONFIG::m_szGameDBServer, DB_SVR_NAME_LENGTH, strGameDBServer.GetString() ); 
	StringCchCopy( GMTOOL_CONFIG::m_szGameDBDatabase, DB_SVR_NAME_LENGTH, strGameDBDatabase.GetString() ); 
	StringCchCopy( GMTOOL_CONFIG::m_szGameDBUser, USR_ID_LENGTH, strGameDBUser.GetString() ); 
	StringCchCopy( GMTOOL_CONFIG::m_szGameDBPass, GMC_DB_PASS_LENGTH, strGameDBPass.GetString() ); 
	
	StringCchCopy( GMTOOL_CONFIG::m_szLogDBServer, DB_SVR_NAME_LENGTH, strLogDBServer.GetString() ); 
	StringCchCopy( GMTOOL_CONFIG::m_szLogDBDatabase, DB_SVR_NAME_LENGTH, strLogDBDatabase.GetString() ); 
	StringCchCopy( GMTOOL_CONFIG::m_szLogDBUser, USR_ID_LENGTH, strLogDBUser.GetString() ); 
	StringCchCopy( GMTOOL_CONFIG::m_szLogDBPass, GMC_DB_PASS_LENGTH, strLogDBPass.GetString() ); 

	StringCchCopy( GMTOOL_CONFIG::m_szShopDBServer, DB_SVR_NAME_LENGTH, strShopDBServer.GetString() ); 
	StringCchCopy( GMTOOL_CONFIG::m_szShopDBDatabase, DB_SVR_NAME_LENGTH, strShopDBDatabase.GetString() ); 
	StringCchCopy( GMTOOL_CONFIG::m_szShopDBUser, USR_ID_LENGTH, strShopDBUser.GetString() ); 
	StringCchCopy( GMTOOL_CONFIG::m_szShopDBPass, GMC_DB_PASS_LENGTH, strShopDBPass.GetString() ); 
	
	GMTOOL_CONFIG::m_nUserDBPort = nUserDBPort;
	GMTOOL_CONFIG::m_nGameDBPort = nGameDBPort;
	GMTOOL_CONFIG::m_nLogDBPort = nLogDBPort;
	GMTOOL_CONFIG::m_nShopDBPort = nShopDBPort;
	
	GMTOOL_CONFIG::m_bLogSQLCODE = GetWin_Check( this, IDC_EDIT_CONFIG_LOG_SQLCODE );
	GMTOOL_CONFIG::m_bLogSQLERROR = GetWin_Check( this, IDC_EDIT_CONFIG_LOG_SQLERROR );
	GMTOOL_CONFIG::m_bLogDATAERROR = GetWin_Check( this, IDC_EDIT_CONFIG_LOG_DATAERROR );

	GMTOOL_CONFIG::m_bPrintSQLCODE = GetWin_Check( this, IDC_EDIT_CONFIG_PRINT_SQLCODE );
	GMTOOL_CONFIG::m_bPrintSQLERROR = GetWin_Check( this, IDC_EDIT_CONFIG_PRINT_SQLERROR );
	GMTOOL_CONFIG::m_bPrintDATAERROR = GetWin_Check( this, IDC_EDIT_CONFIG_PRINT_DATAERROR );

	return TRUE;
}

void CDlgConfig::OnBnClickedOk()
{
	if ( DataSave() ) {
		CDialog::OnOK();
	}
}

void CDlgConfig::OnBnClickedCancel()
{
	CDialog::OnCancel();
}
