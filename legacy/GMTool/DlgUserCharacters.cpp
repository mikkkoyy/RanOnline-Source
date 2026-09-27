// DlgUserCharacters.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgUserCharacters.h"

#include "EtcFunction.h"
#include "HLibDataConvert.h"
#include "Logic/GMToolOdbcBase.h"
#include "Logic/GMToolData.h"
#include "Logic/GMToolGlobal.h"
#include "GLCharDefine.h"
#include "PageUser.h"

// CDlgUserCharacters dialog

IMPLEMENT_DYNAMIC(CDlgUserCharacters, CDialog)

CDlgUserCharacters::CDlgUserCharacters(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgUserCharacters::IDD, pParent)
	, m_pParentWnd(NULL)
	, m_nDialogID(-1)
	, m_dwID(0)
	, m_strNAME("")
{
}

CDlgUserCharacters::CDlgUserCharacters(int nDialogID, DWORD dwID, std::string strNAME, CWnd* pParent /*= NULL*/)
	: CDialog(CDlgUserCharacters::IDD, pParent)
	, m_pParentWnd(pParent)
	, m_nDialogID(nDialogID)
	, m_dwID(dwID)
	, m_strNAME(strNAME)
{
}

CDlgUserCharacters::~CDlgUserCharacters()
{
}

void CDlgUserCharacters::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_USER_CHARACTERS, m_List );
}


BEGIN_MESSAGE_MAP(CDlgUserCharacters, CDialog)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_USERCHARACTERS_BUTTON_READ, OnBnClickedUsercharactersButtonRead)
END_MESSAGE_MAP()


// CDlgUserCharacters message handlers
BOOL CDlgUserCharacters::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString strWindowText;
	strWindowText.Format( "Characters User:(%d) %s", m_dwID, m_strNAME.c_str() );
	SetWindowText( strWindowText.GetString() );

	SetWin_Num_int( this, IDC_USERCHARACTERS_EDIT_NUM, m_dwID );
	SetWin_Text( this, IDC_USERCHARACTERS_EDIT_NAME, m_strNAME.c_str() );


	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "Num/Name", LVCFMT_LEFT, ( 38* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Class", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 2, "School", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 3, "Level", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 4, "Deleted", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	return TRUE;  
}

void CDlgUserCharacters::PostNcDestroy()
{
	CDialog::PostNcDestroy();

	if ( m_pParentWnd ){
		((CPageUser*)m_pParentWnd)->DialogDeleteID(m_nDialogID);
	}

	delete this;
}

void CDlgUserCharacters::OnOK()
{
	if(UpdateData(true)){
		DestroyWindow();
	}
}

void CDlgUserCharacters::OnCancel()
{
	DestroyWindow();
}

void CDlgUserCharacters::OnBnClickedCancel()
{
	OnCancel();
}

void CDlgUserCharacters::DataShow()
{
	SGMTOOL_DATA_USER_CHARACTERS_VEC vecChar;
	CGMToolOdbcBase::GetInstance()->UserCharactersRead( m_dwID, vecChar );

	if ( vecChar.size() <= 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "No Results!" );
		return;
	}

	m_List.DeleteAllItems();

	m_List.SetRedraw( FALSE );

	for( size_t i=0; i<vecChar.size(); ++i){
		SGMTOOL_DATA_USER_CHARACTERS sUserChar = vecChar[i];

		m_List.InsertItem( i, _HLIB::cstringformat( "(%d) %s", sUserChar.dwChaNum, sUserChar.szChaName ).GetString() );
		m_List.SetItemText( i, 1, COMMENT::CHARCLASS[sUserChar.wChaClass].c_str() );
		m_List.SetItemText( i, 2, COMMENT::EMSCHOOL[sUserChar.wChaSchool].c_str() );
		m_List.SetItemText( i, 3, _HLIB::cstring_dword( sUserChar.wChaLevel ).GetString() );
		m_List.SetItemText( i, 4, GMTOOL_GLOBAL::strBasic[sUserChar.bChaDeleted].c_str() );
		m_List.SetItemData( i, sUserChar.dwChaNum );
	}

	m_List.SetRedraw( TRUE );
}

void CDlgUserCharacters::OnBnClickedUsercharactersButtonRead()
{
	DataShow();
}
