
// EditorNpcActionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditorNpcAction.h"
#include "EditorNpcActionDlg.h"

#include "DxServerInstance.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "RANPARAM.h"

#include "../Lib_Engine/Common/SerialFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEditorNpcActionDlg dialog




CEditorNpcActionDlg::CEditorNpcActionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditorNpcActionDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_strFileName = "";
}

void CEditorNpcActionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEditorNpcActionDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_NTK_NEW, OnBnClickedNtkNew)
	ON_BN_CLICKED(ID_NTK_LOAD, OnBnClickedNtkLoad)
	ON_BN_CLICKED(ID_NTK_SAVE, OnBnClickedNtkSave)
	ON_BN_CLICKED(IDC_BUTTON_NTK_SAVEALL, OnBnClickedButtonNtkSaveall)
	ON_BN_CLICKED(IDC_BUTTON_NTK_CLASSCNV, OnBnClickedButtonNtkClasscnv)
	ON_BN_CLICKED(IDC_BUTTON_NTK_DECOMPILE_DAT, OnBnClickedButtonNtkDecompileDat)
END_MESSAGE_MAP()


// CEditorNpcActionDlg message handlers

BOOL CEditorNpcActionDlg::OnInitDialog()
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
	GMTOOL::Create(m_strAppPath.GetString());

	CRect SheetRect;
	CWnd* pWnd = NULL;

	pWnd = GetDlgItem ( IDC_STATIC_TOOL );
	pWnd->GetWindowRect ( SheetRect );
	ScreenToClient ( &SheetRect );

	LOGFONT logfont = {0};

	CFont* pFont = GetFont();
	if ( pFont ) pFont->GetLogFont( &logfont );

	m_pSheetTab = new CsheetWithTab ( SheetRect, logfont, this );
	m_pSheetTab->Create( this, WS_CHILD|WS_VISIBLE|WS_TABSTOP, 0 );
	m_pSheetTab->ModifyStyleEx (0, WS_EX_CONTROLPARENT);
	m_pSheetTab->SetActivePage ( PAGE_TABLE );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEditorNpcActionDlg::OnPaint()
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
HCURSOR CEditorNpcActionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CEditorNpcActionDlg::PostNcDestroy()
{
	SAFE_DELETE ( m_pSheetTab );
	GMTOOL::CleanUp();
	CDialog::PostNcDestroy();
}

void CEditorNpcActionDlg::OnClose()
{
	if ( m_pSheetTab->GetDialogueSet()->IsModify() )
	{
		int nret = MessageBox ( "Press the 'Yes' button to save.", "Caution", MB_YESNOCANCEL );
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
	}

	if ( m_pSheetTab && m_pSheetTab->GetActiveIndex()!=PAGE_TABLE ) 
	{
		MessageBox ( "Close the active window before quit.", "Caution", MB_OK );
		return;
	}

	CDialog::OnClose();
}

void CEditorNpcActionDlg::EnableSaveLoad( BOOL bENABLE )
{
	DWORD dwBUTTON[] = { ID_NTK_NEW, ID_NTK_LOAD, ID_NTK_SAVE, IDC_BUTTON_NTK_SAVEALL, IDC_BUTTON_NTK_CLASSCNV };
	DWORD dwBUTTONNUM = sizeof(dwBUTTON)/sizeof(DWORD);

	for ( DWORD i=0; i<dwBUTTONNUM; ++i )
		SetWin_Enable( this, dwBUTTON[i], bENABLE );
}

void CEditorNpcActionDlg::OnBnClickedNtkNew()
{
	DoNew();
}

void CEditorNpcActionDlg::OnBnClickedNtkLoad()
{
	DoLoad();
}

void CEditorNpcActionDlg::OnBnClickedNtkSave()
{
	DoSave();
}

void CEditorNpcActionDlg::OnBnClickedButtonNtkSaveall()
{
	SaveAll();
}

void CEditorNpcActionDlg::OnBnClickedButtonNtkClasscnv()
{
	ClassConvert();
}

void CEditorNpcActionDlg::OnBnClickedButtonNtkDecompileDat()
{
	DecompileDat();
}

void CEditorNpcActionDlg::DecompileDat()
{
	CString szFilter = "NPC Talk File (*.dat)|*.dat|";
	CFileDialog dlg(TRUE,".dat",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this );

	dlg.m_ofn.lpstrInitialDir = CNpcDialogueSet::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		CString strFilePath;
		strFilePath.Format ( "%s%s", CNpcDialogueSet::GetPath(), dlg.GetFileName() );

		CSerialFile SFile;

		if ( !SFile.OpenFile ( FOT_READ, strFilePath.GetString() ) )		
			return;

		WORD _wVer(0);
		WORD _wNum(0);

		SFile >> _wVer;
		SFile >> _wNum;

		int nCnt = 0;

		for( WORD i=0; i<_wNum; ++i )
		{
			CNpcDialogueSet mTalk;
			mTalk.ResetModified ();
			mTalk.RemoveAllDialogue ();

			WORD _wInnerVer(0);
			SFile >> _wInnerVer;
			switch( _wInnerVer )
			{
			case 0x16:
				{
					BOOL bOK = mTalk.LOAD_0016( SFile );
					if ( bOK )
					{
						CString strSavePath;
						strSavePath.Format ( "%s%s", CNpcDialogueSet::GetPath(), mTalk.m_strFileName.GetString() );
						BOOL bSAVE = mTalk.Save( strSavePath.GetString() );
						if ( bSAVE )	nCnt++;
					}
				}break;
			
			case 0x17:
				{
					BOOL bOK = mTalk.LOAD_0017( SFile );
					if ( bOK )
					{
						CString strSavePath;
						strSavePath.Format ( "%s%s", CNpcDialogueSet::GetPath(), mTalk.m_strFileName.GetString() );
						BOOL bSAVE = mTalk.Save( strSavePath.GetString() );
						if ( bSAVE )	nCnt++;
					}
				}break;
				
			default:
				CDebugSet::ErrorVersion( "CEditorNpcActionDlg::DecompileDat", _wInnerVer );
				break;
			};
		}

		CDebugSet::MsgBox( "Done Decompiling Total:%03u Saved:%03d", _wNum, nCnt );
	}
}