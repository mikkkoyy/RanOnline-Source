// DlgItemEdit.cpp : implementation file
//

#include "stdafx.h"
#include "EditorQuest.h"
#include "DlgItemEdit.h"
#include "EtcFunction.h"
#include "GLItemMan.h"

// CDlgItemEdit dialog

IMPLEMENT_DYNAMIC(CDlgItemEdit, CDialog)

CDlgItemEdit::CDlgItemEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgItemEdit::IDD, pParent)
{

}

CDlgItemEdit::~CDlgItemEdit()
{
}

void CDlgItemEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgItemEdit, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgItemEdit message handlers
BOOL CDlgItemEdit::OnInitDialog()
{
	CDialog::OnInitDialog();

	SITEM* pITEM = GLItemMan::GetInstance().GetItem ( m_sITEMCUSTOM.sNativeID );
	if ( !pITEM )	return TRUE;

	SetWin_Num_int ( this, IDC_EDIT_MID, m_sITEMCUSTOM.sNativeID.wMainID );
	SetWin_Num_int ( this, IDC_EDIT_SID, m_sITEMCUSTOM.sNativeID.wSubID );
	SetWin_Text ( this, IDC_EDIT_NAME, pITEM->GetName() );

	SetWin_Num_int ( this, IDC_EDIT_DAMAGE, m_sITEMCUSTOM.cDAMAGE );
	SetWin_Num_int ( this, IDC_EDIT_DEFENSE, m_sITEMCUSTOM.cDEFENSE );

	SetWin_Num_int ( this, IDC_EDIT_ELECTRIC, m_sITEMCUSTOM.cRESIST_ELEC );
	SetWin_Num_int ( this, IDC_EDIT_FIRE, m_sITEMCUSTOM.cRESIST_FIRE );
	SetWin_Num_int ( this, IDC_EDIT_ICE, m_sITEMCUSTOM.cRESIST_ICE );
	SetWin_Num_int ( this, IDC_EDIT_POISON, m_sITEMCUSTOM.cRESIST_POISON );
	SetWin_Num_int ( this, IDC_EDIT_SPIRIT, m_sITEMCUSTOM.cRESIST_SPIRIT );

	SetWin_Num_int ( this, IDC_EDIT_TURNNUM, m_sITEMCUSTOM.wTurnNum );

	return TRUE; 
}

void CDlgItemEdit::OnBnClickedOk()
{
	m_sITEMCUSTOM.cDAMAGE = GetWin_Num_int ( this, IDC_EDIT_DAMAGE );
	m_sITEMCUSTOM.cDEFENSE = GetWin_Num_int ( this, IDC_EDIT_DEFENSE );
	m_sITEMCUSTOM.cRESIST_ELEC = GetWin_Num_int ( this, IDC_EDIT_ELECTRIC );
	m_sITEMCUSTOM.cRESIST_FIRE = GetWin_Num_int ( this, IDC_EDIT_FIRE );
	m_sITEMCUSTOM.cRESIST_ICE = GetWin_Num_int ( this, IDC_EDIT_ICE );
	m_sITEMCUSTOM.cRESIST_POISON = GetWin_Num_int ( this, IDC_EDIT_POISON );
	m_sITEMCUSTOM.cRESIST_SPIRIT = GetWin_Num_int ( this, IDC_EDIT_SPIRIT );
	m_sITEMCUSTOM.wTurnNum = GetWin_Num_int ( this, IDC_EDIT_TURNNUM );

	OnOK();
}

void CDlgItemEdit::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}
