// DlgLockerGoldEdit.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgLockerGoldEdit.h"

#include "EtcFunction.h"

// CDlgLockerGoldEdit dialog

IMPLEMENT_DYNAMIC(CDlgLockerGoldEdit, CDialog)

CDlgLockerGoldEdit::CDlgLockerGoldEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgLockerGoldEdit::IDD, pParent)
	, m_llValue(0)
{

}

CDlgLockerGoldEdit::CDlgLockerGoldEdit(LONGLONG llVal, CWnd* pParent /*= NULL*/)
	: CDialog(CDlgLockerGoldEdit::IDD, pParent)
	, m_llValue(0)
{
	m_llValue = llVal;
}

CDlgLockerGoldEdit::~CDlgLockerGoldEdit()
{
}

void CDlgLockerGoldEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgLockerGoldEdit, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgLockerGoldEdit message handlers
BOOL CDlgLockerGoldEdit::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWin_Num_LONGLONG( this, IDC_EDIT_LOCKER_GOLD, m_llValue );

	return TRUE;
}

void CDlgLockerGoldEdit::OnBnClickedOk()
{
	m_llValue = GetWin_Num_LONGLONG( this, IDC_EDIT_LOCKER_GOLD );

	CDialog::OnOK();
}
