// PageChar.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "PageChar.h"

#include "SheetWithTab.h"

#include "EtcFunction.h"
#include "HLibDataConvert.h"
#include "Logic/GMToolGlobal.h"
#include "Logic/GMToolData.h"
#include "Logic/GMToolOdbcBase.h"
#include "GLCharDefine.h"
#include "DlgCharEdit.h"
#include "DlgCharCopy.h"

// CPageChar dialog

IMPLEMENT_DYNAMIC(CPageChar, CPropertyPage)

CPageChar::CPageChar(LOGFONT logfont)
	: CPropertyPage(CPageChar::IDD)
	, m_pFont ( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	for ( int i=0; i<MAX_PAGEUSER_MODAL_DIALOG; ++i ){
		m_pToolDialog[i] = NULL;
	}
}

CPageChar::~CPageChar()
{
	SAFE_DELETE ( m_pFont );
}

void CPageChar::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CHAR, m_List );
}


BEGIN_MESSAGE_MAP(CPageChar, CPropertyPage)
	ON_BN_CLICKED(IDC_CHAR_PAGE_BUTTON_SEARCH, OnBnClickedCharPageButtonSearch)
	ON_BN_CLICKED(IDC_CHAR_PAGE_BUTTON_EDIT, OnBnClickedCharPageButtonEdit)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_CHAR, OnNMDblclkListChar)
	ON_BN_CLICKED(IDC_CHAR_PAGE_BUTTON_DELETEDSET, OnBnClickedCharPageButtonDeletedset)
	ON_BN_CLICKED(IDC_CHAR_PAGE_BUTTON_DELETEDREMOVE, OnBnClickedCharPageButtonDeletedremove)
	ON_BN_CLICKED(IDC_CHAR_PAGE_BUTTON_CHARCOPY, OnBnClickedCharPageButtonCharcopy)
	ON_BN_CLICKED(IDC_CHAR_PAGE_BUTTON_SETOFFLINE, OnBnClickedCharPageButtonSetoffline)
END_MESSAGE_MAP()


// CPageChar message handlers
BOOL CPageChar::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "User Number", LVCFMT_LEFT, ( 15* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Char Number", LVCFMT_LEFT, ( 15* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 2, "Name", LVCFMT_LEFT, ( 15* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 3, "Class", LVCFMT_LEFT, ( 14* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 4, "School", LVCFMT_LEFT, ( 14* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 5, "Level", LVCFMT_LEFT, ( 8* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 6, "Online", LVCFMT_LEFT, ( 8* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 7, "Deleted", LVCFMT_LEFT, ( 8* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	SetWin_Combo_Init( this, IDC_CHAR_PAGE_COMBO_SEARCH, GMTOOL_GLOBAL::strCharPageSearchType, CHARPAGE_SEARCH_TYPE_SIZE );
	SetWin_Combo_Init( this, IDC_CHAR_PAGE_COMBO_SEARCH_LIMITNUM, GMTOOL_GLOBAL::strSearchLimit, EMSEARCH_LIMIT_SIZE );

	return TRUE;  
}

void CPageChar::OnBnClickedCharPageButtonSearch()
{
	std::string strSearch = GetWin_Text( this, IDC_CHAR_PAGE_EDIT_SEARCH );
	CHARPAGE_SEARCH_TYPE emType = static_cast<CHARPAGE_SEARCH_TYPE> ( GetWin_Combo_Sel( this, IDC_CHAR_PAGE_COMBO_SEARCH ) );
	EMSEARCH_LIMIT emLIMIT = static_cast<EMSEARCH_LIMIT> ( GetWin_Combo_Sel( this, IDC_CHAR_PAGE_COMBO_SEARCH_LIMITNUM ) );

	SGMTOOL_DATA_CHAR_SIMPLE_MAP mapCharSimple;

	switch( emType )
	{
	case CHARPAGE_SEARCH_TYPE_CHANAME:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_CHAR_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadString( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Search contains invalid characters!" );
				GotoDlgCtrl(GetDlgItem(IDC_CHAR_PAGE_EDIT_SEARCH)); 
				return;
			}

			if( strSearch.size() < MIN_STRING_SEARCH ){
				CDebugSet::MsgBoxAfx( "Search string too short!" );
				GotoDlgCtrl(GetDlgItem(IDC_CHAR_PAGE_EDIT_SEARCH)); 
				return;
			}

			CGMToolOdbcBase::GetInstance()->CharFind_ChaName( strSearch.c_str(), mapCharSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	case CHARPAGE_SEARCH_TYPE_CHANUMBER:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_CHAR_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadNumber( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Use numbers only!" );
				GotoDlgCtrl(GetDlgItem(IDC_CHAR_PAGE_EDIT_SEARCH)); 
				return;
			}

			int nSEARCH = atoi( strSearch.c_str() );
			CGMToolOdbcBase::GetInstance()->CharFind_ChaNumber( nSEARCH, mapCharSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	case CHARPAGE_SEARCH_TYPE_USERNUMBER:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_CHAR_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadNumber( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Use numbers only!" );
				GotoDlgCtrl(GetDlgItem(IDC_CHAR_PAGE_EDIT_SEARCH)); 
				return;
			}

			int nSEARCH = atoi( strSearch.c_str() );
			CGMToolOdbcBase::GetInstance()->CharFind_UserNumber( nSEARCH, mapCharSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	case CHARPAGE_SEARCH_TYPE_ONLINE:
		{
			CGMToolOdbcBase::GetInstance()->CharFind_Online( mapCharSimple, GetSearchLimitNum( emLIMIT ) );
		}break;
	case CHARPAGE_SEARCH_TYPE_ALL:
		{
			CGMToolOdbcBase::GetInstance()->CharFind_ALL( mapCharSimple );
		}
		break;
	};

	if ( mapCharSimple.size() <=0 ){
		CDebugSet::MsgBoxAfx( "No Results" );
		return;
	}

	m_List.DeleteAllItems();

	m_List.SetRedraw( FALSE );

	SGMTOOL_DATA_CHAR_SIMPLE_MAP_ITER mapiter = mapCharSimple.begin();
	for( int i=0; mapiter != mapCharSimple.end(); ++mapiter, ++i ){
		const SGMTOOL_DATA_CHAR_SIMPLE sDATA = (*mapiter).second;

		m_List.InsertItem( i, _HLIB::cstring_dword( sDATA.dwUserNum ).GetString() );
		m_List.SetItemText( i, 1, _HLIB::cstring_dword( sDATA.dwChaNum ).GetString() );
		m_List.SetItemText( i, 2, sDATA.szChaName );
		m_List.SetItemText( i, 3, COMMENT::CHARCLASS[sDATA.wChaClass].c_str() );
		m_List.SetItemText( i, 4, COMMENT::EMSCHOOL[sDATA.wChaSchool].c_str() );
		m_List.SetItemText( i, 5, _HLIB::cstring_dword( sDATA.wChaLevel ).GetString() );
		m_List.SetItemText( i, 6, GMTOOL_GLOBAL::strBasic[sDATA.bChaOnline].c_str() );
		m_List.SetItemText( i, 7, GMTOOL_GLOBAL::strBasic[sDATA.bChaDeleted].c_str() );
		m_List.SetItemData ( i, sDATA.dwChaNum );
	}

	m_List.SetRedraw( TRUE );
}

BOOL CPageChar::OnlineCheck( DWORD dwChaNum, std::string strChaName )
{
	BOOL bONLINE = FALSE;
	CGMToolOdbcBase::GetInstance()->CharOnlineCheck( dwChaNum, bONLINE );

	if ( bONLINE ){
		if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Character: [%d]%s is Online! continue?", dwChaNum, strChaName.c_str() ) != IDYES ){
			return FALSE;
		}
	}

	return TRUE;
}

int CPageChar::DialogNewID()
{
	int nNewDlg = MAX_PAGEUSER_MODAL_DIALOG;
	for ( int i=0; i<MAX_PAGEUSER_MODAL_DIALOG; ++i ){
		if ( !m_pToolDialog[i] ){
			nNewDlg = i;
			break;
		}
	}

	return nNewDlg;
}

void CPageChar::DialogDeleteID( int nDialogID )
{
	if ( nDialogID < 0 || nDialogID >= MAX_PAGEUSER_MODAL_DIALOG ){
		return;
	}

	m_pToolDialog[nDialogID] = NULL;
}

int CPageChar::DialogGetTotalActive()
{
	int nDlg = 0;
	for ( int i=0; i<MAX_PAGEUSER_MODAL_DIALOG; ++i ){
		if ( m_pToolDialog[i] ){
			nDlg ++;
		}
	}

	return nDlg;
}

void CPageChar::OnBnClickedCharPageButtonEdit()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		CDebugSet::MsgBoxAfx( "Nothing Selected!" );
		return;
	}

	DWORD dwUserNum = (DWORD)atoi( m_List.GetItemText ( nSelect, 0 ) );
	DWORD dwCharNum = (DWORD)atoi( m_List.GetItemText ( nSelect, 1 ) );
	std::string strChaName = m_List.GetItemText( nSelect, 2 );

	if ( !OnlineCheck( dwCharNum, strChaName.c_str() ) )	return;

	int nDialogID = DialogNewID();
	if ( nDialogID == MAX_PAGEUSER_MODAL_DIALOG ){
		CDebugSet::MsgBoxAfx( "Max dialog used!" );
		return;
	}

	CDlgCharEdit* pdlg;
	pdlg = new CDlgCharEdit( nDialogID, dwUserNum, dwCharNum, strChaName.c_str(), this );
	pdlg->Create( CWnd::GetDesktopWindow() );
	m_pToolDialog[nDialogID] = pdlg;
}

void CPageChar::OnNMDblclkListChar(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		return;
	}

	DWORD dwUserNum = (DWORD)atoi( m_List.GetItemText ( nSelect, 0 ) );
	DWORD dwCharNum = (DWORD)atoi( m_List.GetItemText ( nSelect, 1 ) );
	std::string strChaName = m_List.GetItemText( nSelect, 2 );

	if ( !OnlineCheck( dwCharNum, strChaName.c_str() ) )	return;

	int nDialogID = DialogNewID();
	if ( nDialogID == MAX_PAGEUSER_MODAL_DIALOG ){
		CDebugSet::MsgBoxAfx( "Max dialog used!" );
		return;
	}

	CDlgCharEdit* pdlg;
	pdlg = new CDlgCharEdit( nDialogID, dwUserNum, dwCharNum, strChaName.c_str(), this );
	pdlg->Create( CWnd::GetDesktopWindow() );
	m_pToolDialog[nDialogID] = pdlg;
}

void CPageChar::OnBnClickedCharPageButtonDeletedset()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		CDebugSet::MsgBoxAfx( "Nothing Selected!" );
		return;
	}

	DWORD dwChaNum = (DWORD) m_List.GetItemData ( nSelect );
	std::string strChaName = m_List.GetItemText( nSelect, 2 );

	if ( !OnlineCheck( dwChaNum, strChaName.c_str() ) )	return;

	int nReturn = CGMToolOdbcBase::GetInstance()->CharSetDeleted( dwChaNum, TRUE );
	if ( nReturn == DB_OK ){
		CDebugSet::MsgBoxAfx( "Set Deleted Character: [%d]%s Done!", dwChaNum, strChaName.c_str() );
	}else{
		CDebugSet::MsgBoxAfx( "Set Deleted Character Failed! Check Logs!" );
	}
}

void CPageChar::OnBnClickedCharPageButtonDeletedremove()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		CDebugSet::MsgBoxAfx( "Nothing Selected!" );
		return;
	}

	DWORD dwChaNum = (DWORD) m_List.GetItemData ( nSelect );
	std::string strChaName = m_List.GetItemText( nSelect, 2 );

	if ( !OnlineCheck( dwChaNum, strChaName.c_str() ) )	return;

	int nReturn = CGMToolOdbcBase::GetInstance()->CharSetDeleted( dwChaNum, FALSE );
	if ( nReturn == DB_OK ){
		CDebugSet::MsgBoxAfx( "Recover Deleted Character: [%d]%s Done!", dwChaNum, strChaName.c_str() );
	}else{
		CDebugSet::MsgBoxAfx( "Recover Deleted Character Failed! Check Logs!" );
	}
}

void CPageChar::OnBnClickedCharPageButtonCharcopy()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		CDebugSet::MsgBoxAfx( "Nothing Selected!" );
		return;
	}

	DWORD dwUserNum = (DWORD)atoi( m_List.GetItemText ( nSelect, 0 ) );
	DWORD dwCharNum = (DWORD)atoi( m_List.GetItemText ( nSelect, 1 ) );
	std::string strChaName = m_List.GetItemText( nSelect, 2 );

	CDlgCharCopy dlg( this, dwCharNum, dwUserNum, strChaName.c_str() );
	if ( dlg.DoModal() == IDOK ){

	}
}

void CPageChar::OnBnClickedCharPageButtonSetoffline()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		CDebugSet::MsgBoxAfx( "Nothing Selected!" );
		return;
	}

	DWORD dwChaNum = (DWORD) m_List.GetItemData ( nSelect );
	std::string strChaName = m_List.GetItemText( nSelect, 2 );

	int nReturn = CGMToolOdbcBase::GetInstance()->CharSetOnline( dwChaNum, FALSE );
	if ( nReturn == DB_OK ){
		CDebugSet::MsgBoxAfx( "SetOffline Character: [%d]%s Done!", dwChaNum, strChaName.c_str() );
	}else{
		CDebugSet::MsgBoxAfx( "SetOffline Character Failed! Check Logs!" );
	}
}
