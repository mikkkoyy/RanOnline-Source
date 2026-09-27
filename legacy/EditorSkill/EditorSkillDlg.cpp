
// EditorSkillDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkill.h"
#include "EditorSkillDlg.h"

#include "DxResponseMan.h"
#include "RANPARAM.h"
#include "GLSkill.h"
#include "DxServerInstance.h"
#include "GLOGIC.h"
#include "GLStringTable.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEditorSkillDlg dialog




CEditorSkillDlg::CEditorSkillDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditorSkillDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEditorSkillDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEditorSkillDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_LOAD_SKILL, OnBnClickedLoadSkill)
	ON_BN_CLICKED(ID_SAVE_SKILL, OnBnClickedSaveSkill)
	ON_BN_CLICKED(ID_LOAD_CSV, OnBnClickedLoadCsv)
	ON_BN_CLICKED(ID_SAVE_CSV, OnBnClickedSaveCsv)
	ON_BN_CLICKED(ID_SEARCH, OnBnClickedSearch)
END_MESSAGE_MAP()


// CEditorSkillDlg message handlers

BOOL CEditorSkillDlg::OnInitDialog()
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
	GLSkillMan::GetInstance().OneTimeSceneInit();

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

void CEditorSkillDlg::OnPaint()
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
HCURSOR CEditorSkillDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CEditorSkillDlg::PostNcDestroy()
{
	SAFE_DELETE ( m_pSheetTab );
	GLSkillMan::GetInstance().FinalCleanup ();
	DxResponseMan::GetInstance().FinalCleanup ();
	GMTOOL::CleanUp ();
	CDialog::PostNcDestroy();
}

bool CEditorSkillDlg::DataLoad()
{
	CString szFilter = "Skill Setting File (*.ssf)|*.ssf|";
	CFileDialog dlg(TRUE,".ssf",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr = GLSkillMan::GetInstance().LoadFile ( dlg.GetFileName().GetString(), TRUE );
		if ( FAILED ( hr ) )
		{
			MessageBox ( "Failure to load Skill Setting File." );
		}

		GLStringTable::GetInstance().CLEAR( GLStringTable::SKILL );
		if( !GLStringTable::GetInstance().LOADFILE( GLSkillMan::GetInstance()._STRINGTABLE, GLStringTable::SKILL ) )
		{
			MessageBox ( "Failure to load Skill String Table." );
		}

		m_vec.clear();

		if ( m_pSheetTab )
			m_pSheetTab->ActivePageTree( TRUE );
	}
	return true;
}

bool CEditorSkillDlg::DataSave()
{
	CString szFilter = "Skill Setting File (*.ssf)|*.ssf|";
	CFileDialog dlg(FALSE,".ssf",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr;
		hr = GLSkillMan::GetInstance().SaveFile ( dlg.GetFileName().GetString() );
		if ( FAILED ( hr ) )
		{
			MessageBox ( "Failure to Save File" );
			return false;
		}

		if ( !GLStringTable::GetInstance().SAVEFILE( GLStringTable::SKILL ) )
		{
			MessageBox ( "Failure to Save File" );
			return false;
		}

		return true;
	}

	return false;
}

void CEditorSkillDlg::OnClose()
{
	if ( GLSkillMan::GetInstance().IsModify() )
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

void CEditorSkillDlg::OnBnClickedLoadSkill()
{
	DataLoad();
}

void CEditorSkillDlg::OnBnClickedSaveSkill()
{
	DataSave();
}

void CEditorSkillDlg::OnBnClickedLoadCsv()
{
	if( GLSkillMan::GetInstance().LoadCsvFile( this ) == S_OK )
	{
		m_vec.clear();

		if ( m_pSheetTab )
			m_pSheetTab->ActivePageTree( TRUE );

		GLStringTable::GetInstance().CLEAR( GLStringTable::SKILL );
		GLStringTable::GetInstance().LOADFILE( GLSkillMan::GetInstance()._STRINGTABLE, GLStringTable::SKILL );
		MessageBox( _T("CSV Load Success"), _T("Success"), MB_OK );
	}
}

void CEditorSkillDlg::OnBnClickedSaveCsv()
{
	if( GLSkillMan::GetInstance().SaveCsvFile( this ) == S_OK )
		MessageBox( _T("CSV Save Success"), _T("Success"), MB_OK );
}

void CEditorSkillDlg::OnBnClickedSearch()
{
	if ( m_pSheetTab )
	{
		m_SearchDlg.SetContainer( &m_vec );
		if ( m_SearchDlg.DoModal() == IDOK )
		{
			if ( m_pSheetTab )
			{
				if ( m_pSheetTab->GetActivePage() != &m_pSheetTab->m_PageTree )
				{
					m_pSheetTab->m_PageEdit1.SkillClear();
					m_pSheetTab->m_PageEdit2.SkillClear();
					m_pSheetTab->ActivePageTree( TRUE );
				}

				m_pSheetTab->m_PageTree.SelectTree( m_SearchDlg.m_sID.wMainID, m_SearchDlg.m_sID.wSubID );
			}
		}
	}
}
