// PageUser.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "PageUser.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "HLibDataConvert.h"
#include "Logic/GMToolGlobal.h"
#include "Logic/GMToolData.h"
#include "Logic/GMToolOdbcBase.h"
#include "DlgDatetime.h"
#include "DlgUserNew.h"
#include "DlgUserEdit.h"
#include "DlgUserLogLogin.h"
#include "DlgUserCharacters.h"
#include "DlgUserItemBank.h"


// CPageUser dialog

IMPLEMENT_DYNAMIC(CPageUser, CPropertyPage)

CPageUser::CPageUser(LOGFONT logfont)
	: CPropertyPage(CPageUser::IDD)
	, m_pFont ( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	for ( int i=0; i<MAX_PAGECHAR_MODAL_DIALOG; ++i ){
		m_pToolDialog[i] = NULL;
	}
}

CPageUser::~CPageUser()
{
	SAFE_DELETE ( m_pFont );
}

void CPageUser::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_USER, m_List );
}


BEGIN_MESSAGE_MAP(CPageUser, CPropertyPage)
	ON_BN_CLICKED(IDC_USER_PAGE_BUTTON_SEARCH, OnBnClickedUserPageButtonSearch)
	ON_BN_CLICKED(IDC_USER_PAGE_BUTTON_NEW, OnBnClickedUserPageButtonNew)
	ON_BN_CLICKED(IDC_USER_PAGE_BUTTON_BLOCK, OnBnClickedUserPageButtonBlock)
	ON_BN_CLICKED(IDC_USER_PAGE_BUTTON_UNBLOCK, OnBnClickedUserPageButtonUnblock)
	ON_BN_CLICKED(IDC_USER_PAGE_BUTTON_CHATBLOCK, OnBnClickedUserPageButtonChatblock)
	ON_BN_CLICKED(IDC_USER_PAGE_BUTTON_CHATUNBLOCK, OnBnClickedUserPageButtonChatunblock)
	ON_BN_CLICKED(IDC_USER_PAGE_BUTTON_LOGLOGIN, OnBnClickedUserPageButtonLoglogin)
	ON_BN_CLICKED(IDC_USER_PAGE_BUTTON_CHARACTERS, OnBnClickedUserPageButtonCharacters)
	ON_BN_CLICKED(IDC_USER_PAGE_BUTTON_SETLOGOUT, OnBnClickedUserPageButtonSetlogout)
	ON_BN_CLICKED(IDC_USER_PAGE_BUTTON_EDIT, OnBnClickedUserPageButtonEdit)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_USER, OnNMDblclkListUser)
	ON_BN_CLICKED(IDC_USER_PAGE_BUTTON_ITEMBANK, OnBnClickedUserPageButtonItembank)
END_MESSAGE_MAP()


// CPageUser message handlers
BOOL CPageUser::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "User Number", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "User Name", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 2, "User Email", LVCFMT_LEFT, ( 13* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 3, "User Type", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 4, "Login State", LVCFMT_LEFT, ( 7* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 5, "Available", LVCFMT_LEFT, ( 6* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 6, "User Block", LVCFMT_LEFT, ( 7* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 7, "User BlockDate", LVCFMT_LEFT, ( 14* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 8, "Chat BlockDate", LVCFMT_LEFT, ( 14* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 9, "User Points", LVCFMT_LEFT, ( 7* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	SetWin_Combo_Init( this, IDC_USER_PAGE_COMBO_SEARCH, GMTOOL_GLOBAL::strUserPageSearchType, USERPAGE_SEARCH_TYPE_SIZE );
	SetWin_Combo_Init( this, IDC_USER_PAGE_COMBO_SEARCH_LIMITNUM, GMTOOL_GLOBAL::strSearchLimit, EMSEARCH_LIMIT_SIZE );

	return TRUE;  
}

void CPageUser::OnBnClickedUserPageButtonSearch()
{
	std::string strSearch = GetWin_Text( this, IDC_USER_PAGE_EDIT_SEARCH );
	USERPAGE_SEARCH_TYPE emType = static_cast<USERPAGE_SEARCH_TYPE> ( GetWin_Combo_Sel( this, IDC_USER_PAGE_COMBO_SEARCH ) );
	EMSEARCH_LIMIT emLIMIT = static_cast<EMSEARCH_LIMIT> ( GetWin_Combo_Sel( this, IDC_USER_PAGE_COMBO_SEARCH_LIMITNUM ) );

	SGMTOOL_USER_SIMPLE_MAP mapUserSimple;
	
	switch( emType )
	{
	case USERPAGE_SEARCH_TYPE_USERNAME:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_USER_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadString( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Search contains invalid characters!" );
				GotoDlgCtrl(GetDlgItem(IDC_USER_PAGE_EDIT_SEARCH)); 
				return;
			}

			if( strSearch.size() < MIN_STRING_SEARCH ){
				CDebugSet::MsgBoxAfx( "Search string too short!" );
				GotoDlgCtrl(GetDlgItem(IDC_USER_PAGE_EDIT_SEARCH)); 
				return;
			}

			CGMToolOdbcBase::GetInstance()->UserFind_UserName( strSearch.c_str(), mapUserSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	case USERPAGE_SEARCH_TYPE_USERNUMBER:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_USER_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadNumber( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Use numbers only!" );
				GotoDlgCtrl(GetDlgItem(IDC_USER_PAGE_EDIT_SEARCH)); 
				return;
			}

			int nSEARCH = atoi( strSearch.c_str() );
			CGMToolOdbcBase::GetInstance()->UserFind_UserNumber( nSEARCH, mapUserSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	case USERPAGE_SEARCH_TYPE_USEREMAIL:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_USER_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadString( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Search contains invalid characters!" );
				GotoDlgCtrl(GetDlgItem(IDC_USER_PAGE_EDIT_SEARCH)); 
				return;
			}

			if( strSearch.size() < MIN_STRING_SEARCH ){
				CDebugSet::MsgBoxAfx( "Search string too short!" );
				GotoDlgCtrl(GetDlgItem(IDC_USER_PAGE_EDIT_SEARCH)); 
				return;
			}

			CGMToolOdbcBase::GetInstance()->UserFind_UserEmail( strSearch.c_str(), mapUserSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	case USERPAGE_SEARCH_TYPE_USERGM:
		{
			CGMToolOdbcBase::GetInstance()->UserFind_UserGM( mapUserSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	case USERPAGE_SEARCH_TYPE_ONLINE:
		{
			CGMToolOdbcBase::GetInstance()->UserFind_Online( mapUserSimple, GetSearchLimitNum( emLIMIT ) );
		}break;
	};

	if ( mapUserSimple.size() <=0 ){
		CDebugSet::MsgBoxAfx( "No Results" );
		return;
	}

	m_List.DeleteAllItems();

	m_List.SetRedraw( FALSE );

	SGMTOOL_USER_SIMPLE_MAP_ITER mapiter = mapUserSimple.begin();
	for( int i=0; mapiter != mapUserSimple.end(); ++mapiter, ++i ){
		const SGMTOOL_USER_SIMPLE sDATA = (*mapiter).second;

		m_List.InsertItem( i, _HLIB::cstring_dword( sDATA.dwUserNum ).GetString() );
		m_List.SetItemText( i, 1, sDATA.szUserID );
		m_List.SetItemText( i, 2, sDATA.szUserEmail );
		m_List.SetItemText( i, 3, _HLIB::cstringformat( "%s(%d)", GMTOOL_GLOBAL::strUserLevelIndex[ UserTypeToIndex( (EMUSERTYPE)sDATA.wUserType ) ].c_str(), sDATA.wUserType ).GetString() );
		m_List.SetItemText( i, 4, GMTOOL_GLOBAL::strUserStatus[sDATA.bUserLoginState].c_str() );
		m_List.SetItemText( i, 5, GMTOOL_GLOBAL::strBasic[sDATA.bUserAvailable].c_str() );
		m_List.SetItemText( i, 6, GMTOOL_GLOBAL::strBasic[sDATA.bUserBlock].c_str() );
		m_List.SetItemText( i, 7, _HLIB::cstring_timet12( sDATA.tUserBlockDate ).GetString() );
		m_List.SetItemText( i, 8, _HLIB::cstring_timet12( sDATA.tChatBlockDate ).GetString() );
		m_List.SetItemText( i, 9, _HLIB::cstring_dword( sDATA.dwUserPoint ).GetString() );
		m_List.SetItemData ( i, sDATA.dwUserNum );
	}

	m_List.SetRedraw( TRUE );
}

BOOL CPageUser::OnlineCheck( DWORD dwUserNum, std::string strUserName )
{
	BOOL bONLINE = FALSE;
	CGMToolOdbcBase::GetInstance()->UserOnlineCheck( dwUserNum, bONLINE );
	
	if ( bONLINE ){
		if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "User: [%d]%s is Online! continue?", dwUserNum, strUserName.c_str() ) != IDYES ){
			return FALSE;
		}
	}

	return TRUE;
}

int CPageUser::DialogNewID()
{
	int nNewDlg = MAX_PAGECHAR_MODAL_DIALOG;
	for ( int i=0; i<MAX_PAGECHAR_MODAL_DIALOG; ++i ){
		if ( !m_pToolDialog[i] ){
			nNewDlg = i;
			break;
		}
	}

	return nNewDlg;
}

void CPageUser::DialogDeleteID( int nDialogID )
{
	if ( nDialogID < 0 || nDialogID >= MAX_PAGECHAR_MODAL_DIALOG ){
		return;
	}

	m_pToolDialog[nDialogID] = NULL;
}

int CPageUser::DialogGetTotalActive()
{
	int nDlg = 0;
	for ( int i=0; i<MAX_PAGECHAR_MODAL_DIALOG; ++i ){
		if ( m_pToolDialog[i] ){
			nDlg ++;
		}
	}

	return nDlg;
}

void CPageUser::OnBnClickedUserPageButtonNew()
{
	CDlgUserNew dlg;
	if ( dlg.DoModal() == IDOK ){

	}
}

void CPageUser::OnBnClickedUserPageButtonBlock()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		CDebugSet::MsgBoxAfx( "Nothing Selected!" );
		return;
	}

	DWORD dwUserNum = (DWORD) m_List.GetItemData ( nSelect );
	std::string strUserName = m_List.GetItemText( nSelect, 1 );

	if ( !OnlineCheck( dwUserNum, strUserName.c_str() ) )	return;

	CDlgDatetime dlg( this );
	dlg.DateSetMin( CTime::GetCurrentTime().GetTime() );
	dlg.DateTimeSet( CTime::GetCurrentTime().GetTime() );
	if ( dlg.DoModal() == IDOK ){
		int nReturn = CGMToolOdbcBase::GetInstance()->UserBlockSet( dwUserNum, TRUE, dlg.DateTimeGet() );
		if ( nReturn == DB_OK ){
			CDebugSet::MsgBoxAfx( "Block User: [%d]%s Done!, Block Date:%s", dwUserNum, strUserName.c_str(), _HLIB::cstring_timet12( dlg.DateTimeGet() ).GetString() );
		}else{
			CDebugSet::MsgBoxAfx( "Block User Failed! Check Logs!" );
		}
	}
}

void CPageUser::OnBnClickedUserPageButtonUnblock()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		CDebugSet::MsgBoxAfx( "Nothing Selected!" );
		return;
	}

	DWORD dwUserNum = (DWORD) m_List.GetItemData ( nSelect );
	std::string strUserName = m_List.GetItemText( nSelect, 1 );

	if ( !OnlineCheck( dwUserNum, strUserName.c_str() ) )	return;

	int nReturn = CGMToolOdbcBase::GetInstance()->UserBlockSet( dwUserNum, FALSE, CTime::GetCurrentTime().GetTime() );
	if ( nReturn == DB_OK ){
		CDebugSet::MsgBoxAfx( "Unblock User: [%d]%s Done!", dwUserNum, strUserName.c_str() );
	}else{
		CDebugSet::MsgBoxAfx( "Unblock User Failed! Check Logs!" );
	}
}

void CPageUser::OnBnClickedUserPageButtonChatblock()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		CDebugSet::MsgBoxAfx( "Nothing Selected!" );
		return;
	}

	DWORD dwUserNum = (DWORD) m_List.GetItemData ( nSelect );
	std::string strUserName = m_List.GetItemText( nSelect, 1 );

	if ( !OnlineCheck( dwUserNum, strUserName.c_str() ) )	return;

	CDlgDatetime dlg( this );
	dlg.DateSetMin( CTime::GetCurrentTime().GetTime() );
	dlg.DateTimeSet( CTime::GetCurrentTime().GetTime() );
	if ( dlg.DoModal() == IDOK ){
		int nReturn = CGMToolOdbcBase::GetInstance()->UserChatBlockSet( dwUserNum, dlg.DateTimeGet() );
		if ( nReturn == DB_OK ){
			CDebugSet::MsgBoxAfx( "ChatBlock User: [%d]%s Done!, ChatBlock Date:%s", dwUserNum, strUserName.c_str(), _HLIB::cstring_timet12( dlg.DateTimeGet() ).GetString() );
		}else{
			CDebugSet::MsgBoxAfx( "ChatBlock User Failed! Check Logs!" );
		}
	}
}

void CPageUser::OnBnClickedUserPageButtonChatunblock()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		CDebugSet::MsgBoxAfx( "Nothing Selected!" );
		return;
	}

	DWORD dwUserNum = (DWORD) m_List.GetItemData ( nSelect );
	std::string strUserName = m_List.GetItemText( nSelect, 1 );

	if ( !OnlineCheck( dwUserNum, strUserName.c_str() ) )	return;

	CTime cTime( 1970, 2, 1, 0, 0, 0 );
	int nReturn = CGMToolOdbcBase::GetInstance()->UserChatBlockSet( dwUserNum, cTime.GetTime() );
	if ( nReturn == DB_OK ){
		CDebugSet::MsgBoxAfx( "Chat Unblock User: [%d]%s Done!", dwUserNum, strUserName.c_str() );
	}else{
		CDebugSet::MsgBoxAfx( "Chat Unblock User Failed! Check Logs!" );
	}
}

void CPageUser::OnBnClickedUserPageButtonLoglogin()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		CDebugSet::MsgBoxAfx( "Nothing Selected!" );
		return;
	}

	int nDialogID = DialogNewID();
	if ( nDialogID == MAX_PAGECHAR_MODAL_DIALOG ){
		CDebugSet::MsgBoxAfx( "Max dialog used!" );
		return;
	}

	DWORD dwUserNum = (DWORD) m_List.GetItemData ( nSelect );
	std::string strUserName = m_List.GetItemText( nSelect, 1 );

	CDlgUserLogLogin* pdlg;
	pdlg = new CDlgUserLogLogin( nDialogID, dwUserNum, strUserName.c_str(), this );
	pdlg->Create( CWnd::GetDesktopWindow() );
	m_pToolDialog[nDialogID] = pdlg;
}

void CPageUser::OnBnClickedUserPageButtonCharacters()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		CDebugSet::MsgBoxAfx( "Nothing Selected!" );
		return;
	}

	int nDialogID = DialogNewID();
	if ( nDialogID == MAX_PAGECHAR_MODAL_DIALOG ){
		CDebugSet::MsgBoxAfx( "Max dialog used!" );
		return;
	}

	DWORD dwUserNum = (DWORD) m_List.GetItemData ( nSelect );
	std::string strUserName = m_List.GetItemText( nSelect, 1 );

	CDlgUserCharacters* pdlg;
	pdlg = new CDlgUserCharacters( nDialogID, dwUserNum, strUserName.c_str(), this );
	pdlg->Create( CWnd::GetDesktopWindow() );
	m_pToolDialog[nDialogID] = pdlg;
}

void CPageUser::OnBnClickedUserPageButtonSetlogout()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		CDebugSet::MsgBoxAfx( "Nothing Selected!" );
		return;
	}

	DWORD dwUserNum = (DWORD) m_List.GetItemData ( nSelect );
	std::string strUserName = m_List.GetItemText( nSelect, 1 );

	int nReturn = CGMToolOdbcBase::GetInstance()->UserLogOut( dwUserNum );

	if ( nReturn == DB_OK ){
		CDebugSet::MsgBoxAfx( "Set Offline User: [%d]%s Done!", dwUserNum, strUserName.c_str() );
	}else{
		CDebugSet::MsgBoxAfx( "Set Offline User Failed! Check Logs!" );
	}
}

void CPageUser::OnBnClickedUserPageButtonEdit()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		CDebugSet::MsgBoxAfx( "Nothing Selected!" );
		return;
	}

	int nDialogID = DialogNewID();
	if ( nDialogID == MAX_PAGECHAR_MODAL_DIALOG ){
		CDebugSet::MsgBoxAfx( "Max dialog used!" );
		return;
	}

	DWORD dwUserNum = (DWORD) m_List.GetItemData ( nSelect );
	std::string strUserName = m_List.GetItemText( nSelect, 1 );

	if ( !OnlineCheck( dwUserNum, strUserName.c_str() ) )	return;

	CDlgUserEdit* pdlg;
	pdlg = new CDlgUserEdit( nDialogID, dwUserNum, strUserName.c_str(), this );
	pdlg->Create( CWnd::GetDesktopWindow() );
	m_pToolDialog[nDialogID] = pdlg;
}

void CPageUser::OnNMDblclkListUser(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		return;
	}

	int nDialogID = DialogNewID();
	if ( nDialogID == MAX_PAGECHAR_MODAL_DIALOG ){
		CDebugSet::MsgBoxAfx( "Max dialog used!" );
		return;
	}

	DWORD dwUserNum = (DWORD) m_List.GetItemData ( nSelect );
	std::string strUserName = m_List.GetItemText( nSelect, 1 );

	if ( !OnlineCheck( dwUserNum, strUserName.c_str() ) )	return;

	CDlgUserEdit* pdlg;
	pdlg = new CDlgUserEdit( nDialogID, dwUserNum, strUserName.c_str(), this );
	pdlg->Create( CWnd::GetDesktopWindow() );
	m_pToolDialog[nDialogID] = pdlg;
}

void CPageUser::OnBnClickedUserPageButtonItembank()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		CDebugSet::MsgBoxAfx( "Nothing Selected!" );
		return;
	}

	int nDialogID = DialogNewID();
	if ( nDialogID == MAX_PAGECHAR_MODAL_DIALOG ){
		CDebugSet::MsgBoxAfx( "Max dialog used!" );
		return;
	}

	DWORD dwUserNum = (DWORD) m_List.GetItemData ( nSelect );
	std::string strUserName = m_List.GetItemText( nSelect, 1 );

	CDlgUserItemBank* pdlg;
	pdlg = new CDlgUserItemBank( nDialogID, dwUserNum, strUserName.c_str(), this );
	pdlg->Create( CWnd::GetDesktopWindow() );
	m_pToolDialog[nDialogID] = pdlg;
}
