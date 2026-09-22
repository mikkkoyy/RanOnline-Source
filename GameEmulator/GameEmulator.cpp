
// GameEmulator.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "GameEmulator.h"
#include "MainFrm.h"

#include "GameEmulatorDoc.h"
#include "GameEmulatorView.h"

#include "gassert.h"
#include "MinBugTrap.h"
#include "DxGlobalStage.h"
#include "StringUtils.h"
#include "GLogic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DO_RTC_CATCH	_do_rtc_catch;
// CGameEmulatorApp

BEGIN_MESSAGE_MAP(CGameEmulatorApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	//ON_COMMAND(ID_FILE_NEW, OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
END_MESSAGE_MAP()


// CGameEmulatorApp construction

CGameEmulatorApp::CGameEmulatorApp()
	: m_bIsActive(TRUE)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	BUG_TRAP::BugTrapInstall( std::string(_T("GameEmulator")));
	memset( m_szAppPath, 0, sizeof(char) * (MAX_PATH));
}

// The one and only CGameEmulatorApp object

CGameEmulatorApp theApp;


// CGameEmulatorApp initialization

BOOL CGameEmulatorApp::InitInstance()
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

	CString strAppPath;
	TCHAR szPath[MAX_PATH] = {0};
	GetModuleFileName(::AfxGetInstanceHandle(), szPath, MAX_PATH);
	strAppPath = szPath;

	if ( !strAppPath.IsEmpty() )
	{
		m_strAppPath = strAppPath.Left ( strAppPath.ReverseFind ( '\\' ) );

		if ( !m_strAppPath.IsEmpty() )
			if ( m_strAppPath.GetAt(0) == '"' )
				m_strAppPath = m_strAppPath.Right ( m_strAppPath.GetLength() - 1 );
	}

	strcpy ( m_szAppPath, m_strAppPath.GetString() );

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}


	AfxEnableControlContainer();

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("DevLordX"));
	LoadStdProfileSettings(0);  // Load standard INI file options (including MRU)


	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CGameEmulatorDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CGameEmulatorView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return TRUE;
}

int CGameEmulatorApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	return CWinApp::ExitInstance();
}

// CGameEmulatorApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CGameEmulatorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CGameEmulatorApp message handlers
int CGameEmulatorApp::Run()
{
	MSG msg;

	while (1)
	{
		if ( PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) )
		{
			if ( GetMessage(&msg, NULL, 0, 0) == FALSE )
			{
				ExitInstance();
				return (int)msg.wParam;
			}

			DxGlobalStage::GetInstance().MsgProc ( &msg );
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if ( m_bIsActive == TRUE )
			{
				CMainFrame *pFrame = (CMainFrame *) AfxGetApp()->m_pMainWnd;
				CGameEmulatorView *pView = (CGameEmulatorView*) pFrame->GetActiveView();

				if ( FAILED ( pView->Render3DEnvironment() ) )
				{
					AfxGetMainWnd()->PostMessage ( WM_CLOSE, 0, 0 );
				}
			}else{
				WaitMessage();
			}
		}
	}
}


