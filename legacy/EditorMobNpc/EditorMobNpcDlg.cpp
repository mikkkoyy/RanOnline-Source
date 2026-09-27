
// EditorMobNpcDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditorMobNpc.h"
#include "EditorMobNpcDlg.h"

#include "DxResponseMan.h"
#include "RANPARAM.h"
#include "GLCrowData.h"
#include "DxServerInstance.h"
#include "GLOGIC.h"
#include "GLStringTable.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEditorMobNpcDlg dialog




CEditorMobNpcDlg::CEditorMobNpcDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditorMobNpcDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEditorMobNpcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEditorMobNpcDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_LOAD_MNSF, OnBnClickedLoadMnsf)
	ON_BN_CLICKED(ID_SAVE_MNSF, OnBnClickedSaveMnsf)
	ON_BN_CLICKED(ID_LOAD_CSV, OnBnClickedLoadCsv)
	ON_BN_CLICKED(ID_SAVE_CSV, OnBnClickedSaveCsv)
	ON_BN_CLICKED(ID_SEARCH, OnBnClickedSearch)
	ON_BN_CLICKED(ID_DIVIDE_HP, OnBnClickedDivideHp)
	ON_BN_CLICKED(ID_DIVIDE_DAMAGE, OnBnClickedDivideDamage)
END_MESSAGE_MAP()


// CEditorMobNpcDlg message handlers

BOOL CEditorMobNpcDlg::OnInitDialog()
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
	GLCrowDataMan::GetInstance().OneTimeSceneInit();

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

void CEditorMobNpcDlg::OnPaint()
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
HCURSOR CEditorMobNpcDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CEditorMobNpcDlg::PostNcDestroy()
{
	SAFE_DELETE ( m_pSheetTab );
	GLCrowDataMan::GetInstance().FinalCleanup ();
	DxResponseMan::GetInstance().FinalCleanup ();
	GMTOOL::CleanUp ();
	CDialog::PostNcDestroy();
}

bool CEditorMobNpcDlg::DataLoad()
{
	CString szFilter = "MobNpc Setting File (*.mnsf)|*.mnsf|";
	CFileDialog dlg(TRUE,".mnsf",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr = GLCrowDataMan::GetInstance().LoadFile ( dlg.GetFileName().GetString(), TRUE, TRUE );
		if ( FAILED ( hr ) )
		{
			MessageBox ( "Failure to load MobNpc Setting File." );
		}

		GLStringTable::GetInstance().CLEAR( GLStringTable::CROW );
		if( !GLStringTable::GetInstance().LOADFILE( GLCrowDataMan::GetInstance()._STRINGTABLE, GLStringTable::CROW ) )
		{
			MessageBox ( "Failure to load MobNpc String Table." );
		}

		m_vec.clear();

		if ( m_pSheetTab )
			m_pSheetTab->ActivePageTree( TRUE );
	}
	return true;
}

bool CEditorMobNpcDlg::DataSave()
{
	CString szFilter = "MobNpc Setting File (*.mnsf)|*.mnsf|";
	CFileDialog dlg(FALSE,".mnsf",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr;
		hr = GLCrowDataMan::GetInstance().SaveFile ( dlg.GetFileName().GetString() );
		if ( FAILED ( hr ) )
		{
			MessageBox ( "Failure to Save File" );
			return false;
		}

		if ( !GLStringTable::GetInstance().SAVEFILE( GLStringTable::CROW ) )
		{
			MessageBox ( "Failure to Save File" );
			return false;
		}

		return true;
	}

	return false;
}

void CEditorMobNpcDlg::OnClose()
{
	if ( GLCrowDataMan::GetInstance().IsModify() )
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

void CEditorMobNpcDlg::OnBnClickedLoadMnsf()
{
	DataLoad();
}

void CEditorMobNpcDlg::OnBnClickedSaveMnsf()
{
	DataSave();
}

void CEditorMobNpcDlg::OnBnClickedLoadCsv()
{
	if( GLCrowDataMan::GetInstance().LoadCsvFile( this ) == S_OK )
	{
		m_vec.clear();

		if ( m_pSheetTab )
			m_pSheetTab->ActivePageTree( TRUE );

		GLStringTable::GetInstance().CLEAR( GLStringTable::CROW );
		GLStringTable::GetInstance().LOADFILE( GLCrowDataMan::GetInstance()._STRINGTABLE, GLStringTable::CROW );
		MessageBox( _T("CSV Load Success"), _T("Success"), MB_OK );
	}
}

void CEditorMobNpcDlg::OnBnClickedSaveCsv()
{
	if( GLCrowDataMan::GetInstance().SaveCsvFile( this ) == S_OK )
		MessageBox( _T("CSV Save Success"), _T("Success"), MB_OK );
}

void CEditorMobNpcDlg::OnBnClickedSearch()
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
					m_pSheetTab->m_PageEdit1.CrowClear();
					m_pSheetTab->m_PageEdit2.CrowClear();
					m_pSheetTab->ActivePageTree( TRUE );
				}

				m_pSheetTab->m_PageTree.SelectTree( m_SearchDlg.m_sID.wMainID, m_SearchDlg.m_sID.wSubID );
			}
		}
	}
}

void CEditorMobNpcDlg::OnBnClickedDivideHp()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Divide all mob hp by 2?") != IDYES )	return;

	for ( int MID = 0; MID < GLCrowDataMan::MAX_CROW_MID; MID++ )
	{
		for ( int SID = 0; SID < GLCrowDataMan::MAX_CROW_SID; SID++ )
		{        		
			PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData ( MID, SID );
			if ( pCROW )
			{
				if ( pCROW->m_emCrow == CROW_MOB )
				{
					if ( pCROW->m_dwHP >= 2 )
						pCROW->m_dwHP = pCROW->m_dwHP / 2;
				}
			}
		}
	}
}

void CEditorMobNpcDlg::OnBnClickedDivideDamage()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Divide all mob damage by 2?") != IDYES )	return;

	for ( int MID = 0; MID < GLCrowDataMan::MAX_CROW_MID; MID++ )
	{
		for ( int SID = 0; SID < GLCrowDataMan::MAX_CROW_SID; SID++ )
		{        		
			PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData ( MID, SID );
			if ( pCROW )
			{
				if ( pCROW->m_emCrow == CROW_MOB )
				{
					for ( int i=0; i<SCROWDATA::EMMAXATTACK; ++i )
					{
						if ( pCROW->m_sCrowAttack[i].sDamage.wLow >= 2 )
							pCROW->m_sCrowAttack[i].sDamage.wLow = pCROW->m_sCrowAttack[i].sDamage.wLow / 2;

						if ( pCROW->m_sCrowAttack[i].sDamage.wHigh >= 2 )
							pCROW->m_sCrowAttack[i].sDamage.wHigh = pCROW->m_sCrowAttack[i].sDamage.wHigh / 2;

						if ( pCROW->m_sCrowAttack[i].sDamage.wLow >= pCROW->m_sCrowAttack[i].sDamage.wHigh )
						{
							pCROW->m_sCrowAttack[i].sDamage.wHigh = pCROW->m_sCrowAttack[i].sDamage.wLow;
							pCROW->m_sCrowAttack[i].sDamage.wHigh += 2;
						}
					}
				}
			}
		}
	}
}
