
// GMTool.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "GMTool.h"
#include "GMToolDlg.h"

#include "MinBugTrap.h"
#include "GLogic.h"
#include "s_CSystemInfo.h"

#include "Logic/GMToolConfig.h"
#include "Logic/GMToolLogs.h"
#include "Logic/GMToolOdbcBase.h"
#include "Logic/GMToolShopData.h"
#include "StringUtils.h"

#include "DlgConfig.h"
#include "DlgLogin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGMToolApp

BEGIN_MESSAGE_MAP(CGMToolApp, CWinApp)
	ON_COMMAND(ID_HELP, OnHelp)
END_MESSAGE_MAP()


// CGMToolApp construction

CGMToolApp::CGMToolApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	BUG_TRAP::BugTrapInstall( std::string(_T("GMTool")));
}


// The one and only CGMToolApp object

CGMToolApp theApp;


// CGMToolApp initialization

BOOL CGMToolApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	//rcc initialize
	GLOGIC::bGLOGIC_PACKFILE = FALSE;
	GLOGIC::bGLOGIC_ZIPFILE = FALSE;
	GLOGIC::bENGLIB_ZIPFILE = FALSE;

	CString StrCmdLine = m_lpCmdLine;

	STRUTIL::ClearSeparator ();
	STRUTIL::RegisterSeparator ( "/" );
	STRUTIL::RegisterSeparator ( " " );

	CStringArray strCmdArray;
	STRUTIL::StringSeparate ( StrCmdLine, strCmdArray );

	for ( int i=0; i<strCmdArray.GetCount(); i++ ){
		CString strTemp = strCmdArray.GetAt(i);

		if ( strTemp == "use_rcc" || strTemp == "USE_RCC" ){
			GLOGIC::bGLOGIC_PACKFILE = FALSE;
			GLOGIC::bGLOGIC_ZIPFILE = TRUE;
			GLOGIC::bENGLIB_ZIPFILE = TRUE;
		}
	}

	AfxEnableControlContainer();

	
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("DevLordX"));

	SERVER_UTIL::CSystemInfo SysInfo;
	GMTOOL_CONFIG::ConfigPathSet( SysInfo.GetAppPath() );
	GMTOOL_LOGS::LogsPathSet( SysInfo.GetAppPath() );

	//load settings file and create new file if loading fails
	if ( !GMTOOL_CONFIG::SettingsLoad() ){
		CDlgConfig dlg;
		if ( dlg.DoModal() == IDOK ){
			GMTOOL_CONFIG::SettingsSave();
		}else{
			return FALSE;
		}
	}

	//open database
	if ( !CGMToolOdbcBase::GetInstance()->DatabaseOpen() ){
		return FALSE;
	}

	//open login
	CDlgLogin dlgLogin;
	if ( dlgLogin.DoModal() == IDOK ){
	}else{
		CGMToolOdbcBase::GetInstance()->DatabaseClose();
		return FALSE;
	}


	//cache itemshop data
	CGMToolOdbcBase::GetInstance()->ShopGetData( GMTOOL_SHOP_DATA::gmtoolSHOPDATA );

	CGMToolDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

