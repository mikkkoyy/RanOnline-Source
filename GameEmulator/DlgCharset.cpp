// DlgCharset.cpp : implementation file
//

#include "stdafx.h"
#include "GameEmulator.h"
#include "DlgCharset.h"


#include "EtcFunction.h"
#include "GLCharDefine.h"
#include "s_NetGlobal.h"
#include "GLogic.h"
#include ".\dlgcharset.h"


// CDlgCharset dialog

IMPLEMENT_DYNAMIC(CDlgCharset, CDialog)

CDlgCharset::CDlgCharset(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCharset::IDD, pParent)
	, sCharacter()
{

}

CDlgCharset::~CDlgCharset()
{
}

void CDlgCharset::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgCharset, CDialog)
	ON_BN_CLICKED(IDC_BTN_CHARSET, OnBnClickedBtnCharset)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgCharset message handlers
BOOL CDlgCharset::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWin_Text( this, IDC_EDIT_CHARSET, sCharacter.strCharSet.c_str() );
	SetWin_Text( this, IDC_EDIT_CHARNAME, sCharacter.strCharName.c_str() );

	CheckRadioButton( IDC_RD_NORMAL, IDC_RD_ADMIN, IDC_RD_NORMAL );

	return TRUE;
}

void CDlgCharset::OnBnClickedBtnCharset()
{
	CString szFilter = "charset file  |*.charset|";
	CFileDialog dlg ( TRUE,".charset",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		sCharacter.strCharSet = dlg.GetFileName().GetString();	
		SetWin_Text( this, IDC_EDIT_CHARSET, sCharacter.strCharSet.c_str() );
	}
}

void CDlgCharset::OnBnClickedOk()
{
	GetSettings();

	CDialog::OnOK();
}

void CDlgCharset::GetSettings()
{
	sCharacter = SCHARSET();
	sCharacter.nUserLevel = USER_COMMON;
	sCharacter.bMaxLevel = GetWin_Check( this, IDC_CHK_MAXLEVEL );
	sCharacter.bMaxStats = GetWin_Check( this, IDC_CHK_MAXSTATS );
	sCharacter.bAllSkills = GetWin_Check( this, IDC_CHK_ALLSKILLS );
	sCharacter.b1BGold = GetWin_Check( this, IDC_CHK_1BGOLD );
	sCharacter.b100KContrib = GetWin_Check( this, IDC_CHK_100K_CONTRIB );
	sCharacter.b100KActivity = GetWin_Check( this, IDC_CHK_100K_ACTIVITY );

	if ( IsDlgButtonChecked( IDC_RD_GM ))
	{
		sCharacter.nUserLevel =  USER_GM3;
	}
	else if ( IsDlgButtonChecked( IDC_RD_ADMIN ))
	{
		sCharacter.nUserLevel = USER_MASTER;
	}

	std::string strCharacterName = GetWin_Text( this, IDC_EDIT_CHARNAME );
	if ( strCharacterName.size() )
	{
		sCharacter.strCharName = strCharacterName.c_str();
	}

	std::string strCharSetFile = GetWin_Text( this, IDC_EDIT_CHARSET );
	if ( strCharSetFile.size() )
	{
		sCharacter.strCharSet = strCharSetFile.c_str();
	}
}