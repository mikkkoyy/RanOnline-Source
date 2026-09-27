// DlgActivityEdit.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgActivityEdit.h"
#include "DlgActivityList.h"
#include "GLActivity.h"
#include "EtcFunction.h"
#include "HLibDataConvert.h"

// CDlgActivityEdit dialog

IMPLEMENT_DYNAMIC(CDlgActivityEdit, CDialog)

CDlgActivityEdit::CDlgActivityEdit(CWnd* pParent /*=NULL*/, SACTIVITY_CHAR_DATA* pData /*= NULL*/)
	: CDialog(CDlgActivityEdit::IDD, pParent)
	, m_pData(NULL)
{
	m_pData = pData;
}

CDlgActivityEdit::~CDlgActivityEdit()
{
}

void CDlgActivityEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgActivityEdit, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgActivityEdit message handlers
BOOL CDlgActivityEdit::OnInitDialog()
{
	CDialog::OnInitDialog();

	DataDefault();
	DataShow();

	return TRUE;
}

BOOL CDlgActivityEdit::DataDefault()
{
	SetWin_Text( this, IDC_EDIT_ACTIVITY_NAME, "" );
	SetWin_Num_int( this, IDC_EDIT_ACTIVITY_PROGRESS_NOW, 0 );
	SetWin_Num_int( this, IDC_EDIT_ACTIVITY_PROGRESS_MAX, 0 );

	return TRUE;
}

BOOL CDlgActivityEdit::DataShow()
{
	if ( !m_pData )	return FALSE;

	SACTIVITY_FILE_DATA* pActivity = GLActivity::GetInstance().GetActivity( m_pData->dwActivityID );
	SetWin_Text( this, IDC_EDIT_ACTIVITY_NAME, _HLIB::cstringformat( "[%d] %s", m_pData->dwActivityID, pActivity? pActivity->strActivityTitle.c_str() : "" ).GetString() );
	
	SetWin_Num_int( this, IDC_EDIT_ACTIVITY_PROGRESS_NOW, m_pData->dwProgressNow );
	SetWin_Num_int( this, IDC_EDIT_ACTIVITY_PROGRESS_MAX, m_pData->dwProgressMax );

	return TRUE;
}

BOOL CDlgActivityEdit::DataSave()
{
	if ( !m_pData )	return FALSE;

	m_pData->dwProgressNow = GetWin_Num_int( this, IDC_EDIT_ACTIVITY_PROGRESS_NOW );
	m_pData->dwProgressMax = GetWin_Num_int( this, IDC_EDIT_ACTIVITY_PROGRESS_MAX );

	return TRUE;
}

void CDlgActivityEdit::OnBnClickedOk()
{
	if ( !DataSave() ) return;

	OnOK();
}

void CDlgActivityEdit::OnBnClickedCancel()
{
	OnCancel();
}


