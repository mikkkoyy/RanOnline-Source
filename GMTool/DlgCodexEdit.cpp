// DlgCodexEdit.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgCodexEdit.h"
#include "DlgCodexList.h"
#include "GLCodex.h"
#include "EtcFunction.h"
#include "HLibDataConvert.h"

// CDlgCodexEdit dialog

IMPLEMENT_DYNAMIC(CDlgCodexEdit, CDialog)

CDlgCodexEdit::CDlgCodexEdit(CWnd* pParent /*=NULL*/, SCODEX_CHAR_DATA* pData /*= NULL*/)
	: CDialog(CDlgCodexEdit::IDD, pParent)
	, m_pData(NULL)
{
	m_pData = pData;
}

CDlgCodexEdit::~CDlgCodexEdit()
{
}

void CDlgCodexEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgCodexEdit, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgCodexEdit message handlers
BOOL CDlgCodexEdit::OnInitDialog()
{
	CDialog::OnInitDialog();

	DataDefault();
	DataShow();

	return TRUE;
}

BOOL CDlgCodexEdit::DataDefault()
{
	SetWin_Text( this, IDC_EDIT_CODEX_NAME, "" );
	SetWin_Num_int( this, IDC_EDIT_CODEX_PROGRESS_NOW, 0 );
	SetWin_Num_int( this, IDC_EDIT_CODEX_PROGRESS_MAX, 0 );

	return TRUE;
}

BOOL CDlgCodexEdit::DataShow()
{
	if ( !m_pData )	return FALSE;

	SCODEX_FILE_DATA* pCodex = GLCodex::GetInstance().GetCodex( m_pData->dwCodexID );
	SetWin_Text( this, IDC_EDIT_CODEX_NAME, _HLIB::cstringformat( "[%d] %s", m_pData->dwCodexID, pCodex? pCodex->strCodexTitle.c_str() : "" ).GetString() );
	
	SetWin_Num_int( this, IDC_EDIT_CODEX_PROGRESS_NOW, m_pData->dwProgressNow );
	SetWin_Num_int( this, IDC_EDIT_CODEX_PROGRESS_MAX, m_pData->dwProgressMax );

	return TRUE;
}

BOOL CDlgCodexEdit::DataSave()
{
	if ( !m_pData )	return FALSE;

	m_pData->dwProgressNow = GetWin_Num_int( this, IDC_EDIT_CODEX_PROGRESS_NOW );
	m_pData->dwProgressMax = GetWin_Num_int( this, IDC_EDIT_CODEX_PROGRESS_MAX );

	return TRUE;
}

void CDlgCodexEdit::OnBnClickedOk()
{
	if ( !DataSave() ) return;

	OnOK();
}

void CDlgCodexEdit::OnBnClickedCancel()
{
	OnCancel();
}


