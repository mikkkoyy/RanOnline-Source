// DlgInputNum.cpp : implementation file
//

#include "stdafx.h"
#include "EditorNpcAction.h"
#include "DlgInputNum.h"
#include "EtcFunction.h"

// CDlgInputNum dialog

IMPLEMENT_DYNAMIC(CDlgInputNum, CDialog)

CDlgInputNum::CDlgInputNum(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgInputNum::IDD, pParent)
	, m_nInputNum(0)
{

}

CDlgInputNum::~CDlgInputNum()
{
}

void CDlgInputNum::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgInputNum, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgInputNum message handlers
BOOL CDlgInputNum::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWin_Num_int( this, IDC_EDIT_NUM, m_nInputNum );

	return TRUE;
}


void CDlgInputNum::OnBnClickedOk()
{
	m_nInputNum = GetWin_Num_int( this, IDC_EDIT_NUM );

	CDialog::OnOK();
}
