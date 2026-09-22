
// EditorQuestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditorQuest.h"
#include "EditorQuestDlg.h"
#include "SheetWithTab.h"

#include "GLQuestMan.h"
#include "GLOGIC.h"
#include "DxResponseMan.h"
#include "RANPARAM.h"
#include "DxServerInstance.h"
#include "../Lib_Engine/COMMON/StringFile.h"
#include "../Lib_Engine/COMMON/StringUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


GLMapList CEditorQuestDlg::m_sMapList;

// CEditorQuestDlg dialog




CEditorQuestDlg::CEditorQuestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditorQuestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEditorQuestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEditorQuestDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_QUEST_NEW, OnBnClickedQuestNew)
	ON_BN_CLICKED(ID_QUEST_LOAD, OnBnClickedQuestLoad)
	ON_BN_CLICKED(ID_QUEST_SAVE, OnBnClickedQuestSave)
	ON_BN_CLICKED(IDC_BUTTON_CSV_LOAD, OnBnClickedButtonCsvLoad)
	ON_BN_CLICKED(IDC_BUTTON_CSV_SAVE, OnBnClickedButtonCsvSave)
	ON_BN_CLICKED(IDC_BUTTON_TEXT_IMPORT, OnBnClickedButtonTextImport)
	ON_BN_CLICKED(IDC_BUTTON_TEXT_EXPORT, OnBnClickedButtonTextExport)
	ON_BN_CLICKED(IDC_BUTTON_CLASS_CONVERT, OnBnClickedButtonClassConvert)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_QUEST, OnBnClickedButtonSearchQuest)
END_MESSAGE_MAP()


// CEditorQuestDlg message handlers

BOOL CEditorQuestDlg::OnInitDialog()
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
	GMTOOL::Create(m_strAppPath.GetString());

	m_sMapList.LoadMapsListFile ( "mapslist.mst" );

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
	m_pSheetTab->SetActivePage ( QUEST_TREE_PAGE );

	m_vec.clear();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CEditorQuestDlg::PostNcDestroy()
{
	SAFE_DELETE ( m_pSheetTab );

	GLQuestMan::GetInstance().Clean();
	DxResponseMan::GetInstance().FinalCleanup ();
	GMTOOL::CleanUp ();

	CDialog::PostNcDestroy();
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEditorQuestDlg::OnPaint()
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
HCURSOR CEditorQuestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CEditorQuestDlg::OnClose()
{
	int nret = MessageBox ( "Press the 'YES' Button to save.", "Caution", MB_YESNOCANCEL );
	switch ( nret )
	{
	case IDYES:
		if ( !DoSave () )	return;	
		break;

	case IDNO:
		break;

	case IDCANCEL:
		return;
	};

	DxResponseMan::GetInstance().FinalCleanup();
	CDialog::OnClose();
}

BOOL CEditorQuestDlg::DoSave ()
{
	CString szFilter = "Quest List File (*.lst)|*.lst|";
	CFileDialog dlg(FALSE,".qst",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);

	dlg.m_ofn.lpstrInitialDir = GLQuestMan::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr;
		hr = GLQuestMan::GetInstance().SaveFile ( dlg.GetFileName().GetString() );
		if ( FAILED ( hr ) )
		{
			MessageBox ( "Failure to Save File" );
			return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}

int CEditorQuestDlg::GetID( CString& strID )
{
	int begin=0;

	CString strTemp = strID.Tokenize("<>",begin);
	if ( strTemp != "QUEST_ID" ) return -1;
	strTemp = strID.Tokenize("<>",begin);

	return atoi( strTemp );
}

void CEditorQuestDlg::OnBnClickedQuestNew()
{
	int nret = MessageBox ( "Press the 'YES' Button to Save.", "Caution", MB_YESNOCANCEL );
	switch ( nret )
	{
	case IDYES:
		if ( !DoSave () )	return;
		break;

	case IDNO:
		break;

	case IDCANCEL:
		return;
	};

	SetWindowText( "EditorQuest" );

	GLQuestMan::GetInstance().Clean();

	m_vec.clear();

	if ( m_pSheetTab )
		m_pSheetTab->m_PageQuestTree.CleanAllQuest ();
}

void CEditorQuestDlg::OnBnClickedQuestLoad()
{
	CString szFilter = "Quest List File (*.lst)|*.lst|";
	CFileDialog dlg(TRUE,".qst",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter,this);

	dlg.m_ofn.lpstrInitialDir = GLQuestMan::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr;
		GLQuestMan::GetInstance().Clean();
		hr = GLQuestMan::GetInstance().LoadFile ( dlg.GetFileName().GetString(), TRUE );

		if ( FAILED ( hr ) ) 
			MessageBox ( "Failure to Load File." );

		if ( m_pSheetTab )
		{
			m_pSheetTab->m_PageQuestTree.CheckInvalid();
			m_pSheetTab->m_PageQuestTree.UpdateTree();
		}

		m_vec.clear();
	}
}

void CEditorQuestDlg::OnBnClickedQuestSave()
{
	DoSave ();
}

void CEditorQuestDlg::OnBnClickedButtonCsvLoad()
{
	CFileDialog dlg( TRUE, ".csv", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "csv file (*.csv)|*.csv|", this );

	dlg.m_ofn.lpstrInitialDir = GLQuestMan::GetInstance().GetPath();
	if ( dlg.DoModal() != IDOK )
		return;

	CStringFile StrFile( 10240 );
	if( !StrFile.Open ( dlg.GetPathName().GetString() ) )
		return;

	STRUTIL::ClearSeparator();
	STRUTIL::RegisterSeparator( "," );

	CString strLine;
	CStringArray StrArray;
	CStringArray StrArrayProgress;

	CString strError;
	int MID = 0;

	GLQuestMan::MAPQUEST& m_mapQuestMap = GLQuestMan::GetInstance().GetMap();
	GLQuestMan::MAPQUEST_ITER iter_pos;
	GLQuestMan::SNODE sNode;

	while( StrFile.GetNextLine( strLine ) ) // head
	{
		StrFile.GetNextLine( strLine ); // data

		STRUTIL::StringSeparate( strLine, StrArray );

		MID = atoi( StrArray[1] );


		iter_pos = m_mapQuestMap.find( MID );

		if ( iter_pos == m_mapQuestMap.end() ) 
		{
			strError.Format("%d ID not Found",MID);
			MessageBox(strError,_T("Error"), MB_OK );

			StrFile.GetNextLine( strLine ); // progress Head
			StrFile.GetNextLine( strLine ); // progress Data
			continue;
		}
		else
		{
			StrFile.GetNextLine( strLine ); // progress Head
			StrFile.GetNextLine( strLine ); // progress Data
			STRUTIL::StringSeparate( strLine, StrArrayProgress );

			sNode = (GLQuestMan::SNODE) iter_pos->second;
			sNode.pQUEST->LoadCsv( StrArray, StrArrayProgress );
			sNode.pQUEST->SAVE( sNode.strFILE.c_str() );
		}
	}

	MessageBox( _T("CSV Load Success"), _T("Success"), MB_OK );

	if ( m_pSheetTab )
	{
		m_pSheetTab->m_PageQuestTree.CheckInvalid();
		m_pSheetTab->UpdateQuestTreePage();
	}

	m_vec.clear();
}

void CEditorQuestDlg::OnBnClickedButtonCsvSave()
{
	CFileDialog dlg( FALSE, ".csv", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "csv file (*.csv)|*.csv|", this );

	dlg.m_ofn.lpstrInitialDir = GLQuestMan::GetInstance().GetPath();
	if ( dlg.DoModal() != IDOK )
		return;

	std::fstream streamFILE;
	streamFILE.open( dlg.GetPathName().GetString(), std::ios_base::out );

	GLQuestMan::MAPQUEST& m_mapQuestMap = GLQuestMan::GetInstance().GetMap();
	GLQuestMan::MAPQUEST_ITER iter_pos;
	GLQuestMan::SNODE sNode;

	for ( iter_pos = m_mapQuestMap.begin(); iter_pos != m_mapQuestMap.end(); iter_pos++ )
	{
		sNode = (GLQuestMan::SNODE) iter_pos->second;
		sNode.pQUEST->SaveCsv( streamFILE, sNode.strFILE.c_str() );
		streamFILE << std::endl;
	}

	MessageBox( _T("CSV Save Success"), _T("Success"), MB_OK );
}

void CEditorQuestDlg::OnBnClickedButtonTextImport()
{
	CFileDialog dlg( TRUE, ".txt", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "txt file (*.txt)|*.txt|", this );

	dlg.m_ofn.lpstrInitialDir = GLQuestMan::GetInstance().GetPath();
	if ( dlg.DoModal() != IDOK )
		return;

	CStringFile StrFile( 10240 );
	if( !StrFile.Open ( dlg.GetPathName().GetString() ) )
		return;

	CString strLine;
	CString strFlag;
	CString strError;
	int MID = 0;

	GLQuestMan::MAPQUEST& m_mapQuestMap = GLQuestMan::GetInstance().GetMap();
	GLQuestMan::MAPQUEST_ITER iter_pos;
	GLQuestMan::SNODE sNode;

	while( StrFile.GetNextLine( strLine ) ) // <begin>
	{
		if ( strLine != "<BEGIN>") continue;

		StrFile.GetNextLine( strLine ); // <QUEST_ID>

		MID = GetID( strLine );

		iter_pos = m_mapQuestMap.find( MID );

		if ( iter_pos == m_mapQuestMap.end() ) 
		{
			strError.Format("%d ID not Found",MID);
			MessageBox(strError,_T("Error"), MB_OK );

			continue;
		}
		else
		{
			sNode = (GLQuestMan::SNODE) iter_pos->second;
			sNode.pQUEST->ImportText( StrFile );
			sNode.pQUEST->SAVE( sNode.strFILE.c_str() );
		}
	}

	MessageBox( _T("Text Import Success"), _T("Success"), MB_OK );

	if ( m_pSheetTab )
	{
		m_pSheetTab->m_PageQuestTree.CheckInvalid();
		m_pSheetTab->UpdateQuestTreePage ();
	}

	m_vec.clear();
}

void CEditorQuestDlg::OnBnClickedButtonTextExport()
{
	CFileDialog dlg( FALSE, ".txt", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "text file (*.txt)|*.txt|", this );

	dlg.m_ofn.lpstrInitialDir = GLQuestMan::GetInstance().GetPath();
	if ( dlg.DoModal() != IDOK )
		return;

	std::fstream streamFILE;
	streamFILE.open( dlg.GetPathName().GetString(), std::ios_base::out );

	GLQuestMan::MAPQUEST& m_mapQuestMap = GLQuestMan::GetInstance().GetMap();
	GLQuestMan::MAPQUEST_ITER iter_pos;
	GLQuestMan::SNODE sNode;

	for ( iter_pos = m_mapQuestMap.begin(); iter_pos != m_mapQuestMap.end(); iter_pos++ )
	{
		sNode = (GLQuestMan::SNODE) iter_pos->second;
		sNode.pQUEST->ExportText( streamFILE );
		streamFILE << std::endl;
	}

	MessageBox( _T("Text Export Success"), _T("Success"), MB_OK );
}

void CEditorQuestDlg::OnBnClickedButtonClassConvert()
{
	if ( m_pSheetTab )
		m_pSheetTab->ClassConvert();
}

void CEditorQuestDlg::OnBnClickedButtonSearchQuest()
{
	if ( m_pSheetTab )
	{
		m_SearchDlg.SetContainer( &m_vec );
		if ( m_SearchDlg.DoModal() == IDOK )
		{
			if ( m_pSheetTab )
			{
				if ( m_pSheetTab->GetActivePage () != &m_pSheetTab->m_PageQuestTree )
				{
					m_pSheetTab->m_PageQuestBasic.QuestClear();
					m_pSheetTab->m_PageQuestStep.QuestClear();
					m_pSheetTab->ActiveQuestTreePage();
				}

				m_pSheetTab->m_PageQuestTree.SelectTree( m_SearchDlg.m_dwQUESTID );
			}
		}
	}
}
