// EditorItem.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "EditorItem.h"
#include "EditorItemDlg.h"
#include "MinBugTrap.h"
#include "GLItemMan.h"
#include "GLOGIC.h"
#include "RANPARAM.h"
#include "GMTOOL.h"
#include "DxResponseMan.h"
#include "GLStringTable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Helper function for headless CSV export
static bool ExportItemCsv(const CString& strInputIsf, const CString& strOutputCsv)
{
	// Initialize required systems
	char szAppPath[MAX_PATH] = {0};
	GetModuleFileName(::AfxGetInstanceHandle(), szAppPath, MAX_PATH);
	CString strAppPath = szAppPath;
	
	if (!strAppPath.IsEmpty())
	{
		DWORD dwFind = strAppPath.ReverseFind('\\');
		if (dwFind != -1)
		{
			strAppPath = strAppPath.Left(dwFind);
			if (!strAppPath.IsEmpty() && strAppPath.GetAt(0) == '"')
				strAppPath = strAppPath.Right(strAppPath.GetLength() - 1);
		}
	}

	RANPARAM::LOAD(strAppPath.GetString());
	DxResponseMan::GetInstance().OneTimeSceneInit(strAppPath.GetString(), NULL, RANPARAM::strFontType, RANPARAM::dwLangSet, RANPARAM::strGDIFont);
	GLItemMan::GetInstance().OneTimeSceneInit();
	GMTOOL::Create(strAppPath.GetString());

	// Load the .isf file
	HRESULT hr = GLItemMan::GetInstance().LoadFile(strInputIsf.GetString(), TRUE, TRUE);
	if (FAILED(hr))
	{
		_tprintf(_T("ERROR: Failed to load item file: %s\n"), strInputIsf.GetString());
		return false;
	}

	// Load string table for item names
	GLStringTable::GetInstance().CLEAR(GLStringTable::ITEM);
	if (!GLStringTable::GetInstance().LOADFILE(GLItemMan::GetInstance()._STRINGTABLE, GLStringTable::ITEM))
	{
		_tprintf(_T("WARNING: Failed to load item string table\n"));
	}

	// Save to CSV directly (bypass CFileDialog)
	std::fstream streamFILE;
	streamFILE.open(strOutputCsv.GetString(), std::ios_base::out);
	if (!streamFILE.is_open())
	{
		_tprintf(_T("ERROR: Cannot open output file: %s\n"), strOutputCsv.GetString());
		return false;
	}

	// CSV Head
	SITEM::SaveCsvHead(streamFILE);

	// Export all items
	for (int i = 0; i < MAX_MID; ++i)
	{
		for (int j = 0; j < MAX_SID; ++j)
		{
			if (!GLItemMan::GetInstance().m_ppItem[i][j])
				continue;

			CItemNode *pITEMNODE = GLItemMan::GetInstance().m_ppItem[i][j];
			SITEM &sITEM = pITEMNODE->m_sItem;
			sITEM.SaveCsv(streamFILE);
		}
	}

	streamFILE.close();

	_tprintf(_T("SUCCESS: Exported %s to %s\n"), strInputIsf.GetString(), strOutputCsv.GetString());
	return true;
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEditorItemApp

BEGIN_MESSAGE_MAP(CEditorItemApp, CWinApp)
	ON_COMMAND(ID_HELP, OnHelp)
END_MESSAGE_MAP()


// CEditorItemApp construction

CEditorItemApp::CEditorItemApp()
{
	BUG_TRAP::BugTrapInstall( std::string(_T("EditorItem")));
}


// The one and only CEditorItemApp object

CEditorItemApp theApp;


// CEditorItemApp initialization

BOOL CEditorItemApp::InitInstance()
{
	// Check for command-line export mode
	LPWSTR *szArgList = CommandLineToArgvW(GetCommandLineW(), &int argc);
	if (argc >= 4 && wcscmp(szArgList[1], L"--export-csv") == 0)
	{
		CString strInput = szArgList[2];
		CString strOutput = szArgList[3];
		
		InitCommonControlsEx(NULL); // Minimal init for headless mode
		CWinApp::InitInstance();
		
		bool success = ExportItemCsv(strInput, strOutput);
		
		LocalFree(szArgList);
		return FALSE; // Exit immediately
	}
	
	if (szArgList) LocalFree(szArgList);

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// Standard initialization
	SetRegistryKey(_T("DevLordX"));

	CEditorItemDlg dlg;
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
	// application, rather than start the application's message pump.
	return FALSE;
}