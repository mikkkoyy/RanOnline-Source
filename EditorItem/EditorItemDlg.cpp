
// EditorItemDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditorItem.h"
#include "EditorItemDlg.h"

#include "DxResponseMan.h"
#include "RANPARAM.h"
#include "GLItemMan.h"
#include "DxServerInstance.h"
#include "GLOGIC.h"
#include "GLStringTable.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEditorItemDlg dialog




CEditorItemDlg::CEditorItemDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditorItemDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEditorItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEditorItemDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_LOAD_ITEM, OnBnClickedLoadItem)
	ON_BN_CLICKED(ID_SAVE_ITEM, OnBnClickedSaveItem)
	ON_BN_CLICKED(ID_LOAD_CSV, OnBnClickedLoadCsv)
	ON_BN_CLICKED(ID_SAVE_CSV, OnBnClickedSaveCsv)
	ON_BN_CLICKED(ID_SEARCH, OnBnClickedSearch)
	ON_BN_CLICKED(ID_LOAD_CSV_ROPT, OnBnClickedLoadCsvRopt)
	ON_BN_CLICKED(ID_SAVE_CSV_ROPT, OnBnClickedSaveCsvRopt)
END_MESSAGE_MAP()


// CEditorItemDlg message handlers

BOOL CEditorItemDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	CString strAppPath;

	TCHAR szPath[MAX_PATH] = {0};
	GetModuleFileName(::AfxGetInstanceHandle(), szPath, MAX_PATH);
	strAppPath = szPath;

	if ( !strAppPath.IsEmpty() )
	{
		DWORD dwFind = strAppPath.ReverseFind ( '\\' );
		if ( dwFind != -1 )
		{
			m_strAppPath = strAppPath.Left ( dwFind );

			if ( !m_strAppPath.IsEmpty() )
				if ( m_strAppPath.GetAt(0) == '"' )
					m_strAppPath = m_strAppPath.Right ( m_strAppPath.GetLength() - 1 );

			strcpy ( m_szAppPath, m_strAppPath.GetString() );
		}
	}

	RANPARAM::LOAD( m_szAppPath );
	DxResponseMan::GetInstance().OneTimeSceneInit ( m_szAppPath, this, RANPARAM::strFontType, RANPARAM::dwLangSet, RANPARAM::strGDIFont );
	GLItemMan::GetInstance().OneTimeSceneInit();

	GMTOOL::Create ( m_szAppPath );

	CRect SheetRect;
	CWnd *pWnd;
	pWnd = GetDlgItem ( IDC_STATIC_TOOL );
	pWnd->GetWindowRect ( SheetRect );
	ScreenToClient ( &SheetRect );
	LOGFONT logfont = {0};

	CFont* pFont = GetFont();
	if ( pFont ) pFont->GetLogFont( &logfont );

	m_pSheetTab = new CsheetWithTab ( SheetRect, logfont, this );
	m_pSheetTab->Create( this, WS_CHILD|WS_VISIBLE|WS_TABSTOP, 0 );
	m_pSheetTab->ModifyStyleEx (0, WS_EX_CONTROLPARENT);
	m_pSheetTab->SetActivePage ( PAGE_TREE );

	m_vec.clear();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEditorItemDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEditorItemDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CEditorItemDlg::PostNcDestroy()
{
	SAFE_DELETE ( m_pSheetTab );
	GLItemMan::GetInstance().FinalCleanup ();
	DxResponseMan::GetInstance().FinalCleanup ();
	GMTOOL::CleanUp ();
	CDialog::PostNcDestroy();
}

bool CEditorItemDlg::DataLoad()
{
	CString szFilter = "Item Setting File (*.isf)|*.isf|";
	CFileDialog dlg(TRUE,".isf",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr = GLItemMan::GetInstance().LoadFile ( dlg.GetFileName().GetString(), TRUE, TRUE );
		if ( FAILED ( hr ) )
		{
			MessageBox ( "Failure to load Item Setting File." );
		}

		GLStringTable::GetInstance().CLEAR( GLStringTable::ITEM );
		if( !GLStringTable::GetInstance().LOADFILE( GLItemMan::GetInstance()._STRINGTABLE, GLStringTable::ITEM ) )
		{
			MessageBox ( "Failure to load Item String Table." );
		}

		m_vec.clear();

		if ( m_pSheetTab )
			m_pSheetTab->ActivePageTree( TRUE );
	}
	return true;
}

bool CEditorItemDlg::DataSave()
{
	CString szFilter = "Item Setting File (*.isf)|*.isf|";
	CFileDialog dlg(FALSE,".isf",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr;
		hr = GLItemMan::GetInstance().SaveFile ( dlg.GetFileName().GetString() );
		if ( FAILED ( hr ) )
		{
			MessageBox ( "Failure to Save File" );
			return false;
		}

		if ( !GLStringTable::GetInstance().SAVEFILE( GLStringTable::ITEM ) )
		{
			MessageBox ( "Failure to Save File" );
			return false;
		}

		return true;
	}

	return false;
}

void CEditorItemDlg::OnClose()
{
	if ( GLItemMan::GetInstance().IsModify() )
	{
		int nret = MessageBox ( "Press the 'YES' Button to Save.", "Caution", MB_YESNOCANCEL );
		switch ( nret )
		{
		case IDYES:
			if ( !DataSave () )	return;
			break;

		case IDNO:
			break;

		case IDCANCEL:
			return;
		};
	}

	CDialog::OnClose();
}

void CEditorItemDlg::OnBnClickedLoadItem()
{
	DataLoad();
}

void CEditorItemDlg::OnBnClickedSaveItem()
{
	DataSave();
}

void CEditorItemDlg::OnBnClickedLoadCsv()
{
	if( GLItemMan::GetInstance().LoadCsvFile( this ) == S_OK )
	{
		m_vec.clear();

		if ( m_pSheetTab )
			m_pSheetTab->ActivePageTree( TRUE );

		GLStringTable::GetInstance().CLEAR( GLStringTable::ITEM );
		GLStringTable::GetInstance().LOADFILE( GLItemMan::GetInstance()._STRINGTABLE, GLStringTable::ITEM );
		MessageBox( _T("CSV Load Success"), _T("Success"), MB_OK );
	}
}

void CEditorItemDlg::OnBnClickedSaveCsv()
{
	if( GLItemMan::GetInstance().SaveCsvFile( this ) == S_OK )
		MessageBox( _T("CSV Save Success"), _T("Success"), MB_OK );
}

void CEditorItemDlg::OnBnClickedSearch()
{
	if ( m_pSheetTab )
	{
		m_SearchDlg.SetContainer( &m_vec );
		if ( m_SearchDlg.DoModal() == IDOK )
		{
			if ( m_pSheetTab )
			{
				if ( m_pSheetTab->GetActivePage () != &m_pSheetTab->m_PageTree )
				{
					m_pSheetTab->m_PageEdit1.ItemClear();
					m_pSheetTab->m_PageEdit2.ItemClear ();
					m_pSheetTab->ActivePageTree( TRUE );
				}

				m_pSheetTab->m_PageTree.SelectTree( m_SearchDlg.m_sID.wMainID, m_SearchDlg.m_sID.wSubID );
			}
		}
	}
}

void CEditorItemDlg::OnBnClickedLoadCsvRopt()
{
	if( GLItemMan::GetInstance().LoadRandomOptCsvFile( this ) == S_OK )
	{
		MessageBox( _T("CSV RandomOpt Load Success"), _T("Success"), MB_OK );
	}
}

void CEditorItemDlg::OnBnClickedSaveCsvRopt()
{
	if( GLItemMan::GetInstance().SaveRandomOptCsvFile( this ) == S_OK )
		MessageBox( _T("CSV RandomOpt Save Success"), _T("Success"), MB_OK );
}
