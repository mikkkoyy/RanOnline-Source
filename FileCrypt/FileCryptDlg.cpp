
// FileCryptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FileCrypt.h"
#include "FileCryptDlg.h"
#include "EtcFunction.h"
#include "ByteCrypt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFileCryptDlg dialog


CFileCryptDlg::CFileCryptDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFileCryptDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_strInputFolder = "";
	m_strOutputFolder = "";
}

void CFileCryptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_INFO, m_cEdit);
}

BEGIN_MESSAGE_MAP(CFileCryptDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_CMB_FILETYPE, OnCbnSelchangeCmbFiletype)
	ON_BN_CLICKED(IDC_BTN_INPUT, OnBnClickedBtnInput)
	ON_BN_CLICKED(IDC_BTN_OUTPUT, OnBnClickedBtnOutput)
	ON_BN_CLICKED(IDC_BTN_INPLACE, OnBnClickedBtnInplace)
	ON_BN_CLICKED(IDC_BTN_ENCRYPT, OnBnClickedBtnEncrypt)
	ON_BN_CLICKED(IDC_BTN_DECRYPT, OnBnClickedBtnDecrypt)
END_MESSAGE_MAP()


// CFileCryptDlg message handlers

BOOL CFileCryptDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	std::string strFileType[EMCRYPT_FILETYPE_SIZE] = 
	{
		"TextFiles",
		"StreamFiles",
		"StreamFiles(Ran2)",
		"XML Gui Files",
		"Texture Files",
		"X Files",
		"Animation Bin Files",
		"Effect EGP Files",
	};

	std::string strBypassHeader[EMCRYPT_BYPASSHEADER_SIZE] = 
	{
		"No Bypass",
		"Bypass 128 Bytes",
		"Bypass 132 Bytes",
		"Bypass 136 Bytes",
	};

	SetWin_Combo_Init( this, IDC_CMB_FILETYPE, strFileType, EMCRYPT_FILETYPE_SIZE );
	SetWin_Combo_Init( this, IDC_CMB_BYPASSHEADER, strBypassHeader, EMCRYPT_BYPASSHEADER_SIZE );
	SetWin_Combo_Init( this, IDC_CMB_CRYPTTYPE, BYTECRYPT::strCrypt, EMBYTECRYPT_SIZE );
	SetWin_Combo_Del( this, IDC_CMB_CRYPTTYPE, BYTECRYPT::strCrypt[EMBYTECRYPT_RAN2].c_str() );

	SettingChange();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFileCryptDlg::OnPaint()
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
HCURSOR CFileCryptDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFileCryptDlg::InfoAdd( const char *szFormat, ... )
{
	va_list argList;
	char szBuffer[256] = "";
	va_start(argList, szFormat);
	StringCbVPrintf( szBuffer, 256, szFormat, argList );
	va_end(argList);

	int nlen = m_cEdit.GetWindowTextLength();

	if ( m_cEdit.GetWindowTextLength() >= (int)m_cEdit.GetLimitText() )
	{
		m_cEdit.SetSel( 0 , m_cEdit.GetWindowTextLength() );
	}else{
		m_cEdit.SetSel( nlen, nlen );
	}

	m_cEdit.ReplaceSel( szBuffer );	

	nlen = m_cEdit.GetWindowTextLength();
	m_cEdit.SetSel( nlen, nlen );
	m_cEdit.ReplaceSel( "\r\n" );
}

void CFileCryptDlg::InfoReset()
{
	m_cEdit.SetWindowTextA( "" );
}

void CFileCryptDlg::SettingChange()
{
	EMCRYPT_FILETYPE emFileType = static_cast<EMCRYPT_FILETYPE>( GetWin_Combo_Sel( this, IDC_CMB_FILETYPE) );
	BOOL bStreamFile = ( emFileType == EMCRYPT_FILETYPE_STREAMFILE );
	BOOL bStreamFileR2 = ( emFileType == EMCRYPT_FILETYPE_STREAMFILE_R2 );
	BOOL bStreamAnimation = ( emFileType == EMCRYPT_FILETYPE_ANIMFILE );
	BOOL bStreamEGP = ( emFileType == EMCRYPT_FILETYPE_EGPCRYPT );

	SetWin_Enable( this, IDC_STATIC_ENCODETYPE, bStreamFile );
	SetWin_Enable( this, IDC_STATIC_BYPASSHEADER, bStreamFile );
	SetWin_Enable( this, IDC_CMB_BYPASSHEADER, bStreamFile );
	SetWin_Enable( this, IDC_CMB_CRYPTTYPE, bStreamFile );
	SetWin_Enable( this, IDC_CHECK_CONVERT_ENCRYPT, bStreamFileR2 );

	SetWin_Text( this, IDC_EDIT_INPUT, m_strInputFolder.c_str() );
	SetWin_Text( this, IDC_EDIT_OUTPUT, m_strOutputFolder.c_str() );

	SetWin_Enable( this, IDC_BTN_INPLACE, BOOL( m_strInputFolder.size() > 0 ) );
	SetWin_Enable( this, IDC_BTN_ENCRYPT, BOOL( m_strInputFolder.size() > 0 && m_strOutputFolder.size() > 0 && !bStreamFileR2 ) );
	SetWin_Enable( this, IDC_BTN_DECRYPT, BOOL( m_strInputFolder.size() > 0 && m_strOutputFolder.size() > 0 && !bStreamAnimation ) );
}

void CFileCryptDlg::OnCbnSelchangeCmbFiletype()
{
	SettingChange();
}

void CFileCryptDlg::OnBnClickedBtnInput()
{
	BROWSEINFO biInput;
	ZeroMemory(&biInput, sizeof(BROWSEINFO));
	biInput.hwndOwner = m_hWnd;
	biInput.lpszTitle = "Select Input Folder";
	biInput.pidlRoot = NULL;
	biInput.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_NONEWFOLDERBUTTON | BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
	LPITEMIDLIST pidlFolderInput = SHBrowseForFolder(&biInput);
	if (pidlFolderInput == NULL) return;
	TCHAR szPathInput[_MAX_PATH] = {0};
	SHGetPathFromIDList(pidlFolderInput, szPathInput);
	m_strInputFolder = szPathInput;

	if ( m_strInputFolder.size() >= 250 )
	{
		InfoAdd( "Input path length too long!" );
		m_strInputFolder = "";
	}

	SettingChange();
}

void CFileCryptDlg::OnBnClickedBtnOutput()
{
	BROWSEINFO biOutput;
	ZeroMemory(&biOutput, sizeof(BROWSEINFO));
	biOutput.hwndOwner = m_hWnd;
	biOutput.lpszTitle = "Select Output Folder"; 
	biOutput.pidlRoot = NULL;
	biOutput.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_NONEWFOLDERBUTTON | BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
	LPITEMIDLIST pidlFolderOutput = SHBrowseForFolder(&biOutput);
	if (pidlFolderOutput == NULL) return;
	TCHAR szPathOutput[_MAX_PATH] = {0};
	SHGetPathFromIDList(pidlFolderOutput, szPathOutput);
	m_strOutputFolder = szPathOutput;

	if ( m_strOutputFolder.size() >= 220 )
	{
		InfoAdd( "Output path length too long!" );
		m_strOutputFolder = "";
	}

	SettingChange();
}

void CFileCryptDlg::OnBnClickedBtnInplace()
{
	m_strOutputFolder = m_strInputFolder.c_str();
	SettingChange();
}

void CFileCryptDlg::OnBnClickedBtnEncrypt()
{
	DoCrypt( TRUE );
}

void CFileCryptDlg::OnBnClickedBtnDecrypt()
{
	DoCrypt( FALSE );
}
