// PagePet.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "PagePet.h"

#include "SheetWithTab.h"
#include "Logic/GMToolGlobal.h"
#include "Logic/GMToolOdbcBase.h"

#include "HLibDataConvert.h"
#include "EtcFunction.h"
#include "DlgPetEdit.h"

// CPagePet dialog

IMPLEMENT_DYNAMIC(CPagePet, CPropertyPage)

CPagePet::CPagePet(LOGFONT logfont)
	: CPropertyPage(CPagePet::IDD)
	, m_pFont ( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	for ( int i=0; i<MAX_PAGEPET_MODAL_DIALOG; ++i ){
		m_pToolDialog[i] = NULL;
	}
}

CPagePet::~CPagePet()
{
	SAFE_DELETE ( m_pFont );
}

void CPagePet::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PET, m_List );
}


BEGIN_MESSAGE_MAP(CPagePet, CPropertyPage)
	ON_BN_CLICKED(IDC_PET_PAGE_BUTTON_SEARCH, OnBnClickedPetPageButtonSearch)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PET, OnNMDblclkListPet)
	ON_BN_CLICKED(IDC_PET_PAGE_BUTTON_EDIT, OnBnClickedPetPageButtonEdit)
END_MESSAGE_MAP()


// CPagePet message handlers
BOOL CPagePet::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "Pet Number", LVCFMT_LEFT, ( 15* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Pet ChaNumber", LVCFMT_LEFT, ( 15* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 2, "Pet Name", LVCFMT_LEFT, ( 30* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 3, "Card MID", LVCFMT_LEFT, ( 8* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 4, "Card SID", LVCFMT_LEFT, ( 8* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 5, "Pet Type", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	SetWin_Combo_Init( this, IDC_PET_PAGE_COMBO_SEARCH, GMTOOL_GLOBAL::strPetPageSearchType, EMPETPAGE_SEARCH_SIZE );
	SetWin_Combo_Init( this, IDC_PET_PAGE_COMBO_SEARCH_LIMITNUM, GMTOOL_GLOBAL::strSearchLimit, EMSEARCH_LIMIT_SIZE );

	return TRUE;  
}

int CPagePet::DialogNewID()
{
	int nNewDlg = MAX_PAGEPET_MODAL_DIALOG;
	for ( int i=0; i<MAX_PAGEPET_MODAL_DIALOG; ++i ){
		if ( !m_pToolDialog[i] ){
			nNewDlg = i;
			break;
		}
	}

	return nNewDlg;
}

void CPagePet::DialogDeleteID( int nDialogID )
{
	if ( nDialogID < 0 || nDialogID >= MAX_PAGEPET_MODAL_DIALOG ){
		return;
	}

	m_pToolDialog[nDialogID] = NULL;
}

int CPagePet::DialogGetTotalActive()
{
	int nDlg = 0;
	for ( int i=0; i<MAX_PAGEPET_MODAL_DIALOG; ++i ){
		if ( m_pToolDialog[i] ){
			nDlg ++;
		}
	}

	return nDlg;
}

void CPagePet::OnBnClickedPetPageButtonSearch()
{
	std::string strSearch = GetWin_Text( this, IDC_PET_PAGE_EDIT_SEARCH );
	EMPETPAGE_SEARCH_TYPE emType = static_cast<EMPETPAGE_SEARCH_TYPE> ( GetWin_Combo_Sel( this, IDC_PET_PAGE_COMBO_SEARCH ) );
	EMSEARCH_LIMIT emLIMIT = static_cast<EMSEARCH_LIMIT> ( GetWin_Combo_Sel( this, IDC_PET_PAGE_COMBO_SEARCH_LIMITNUM ) );

	SGMTOOL_PETDATA_SIMPLE_MAP mapPetSimple;

	switch( emType )
	{
	case EMPETPAGE_SEARCH_PETNAME:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_PET_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadString( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Search contains invalid characters!" );
				GotoDlgCtrl(GetDlgItem(IDC_PET_PAGE_EDIT_SEARCH)); 
				return;
			}

			if( strSearch.size() < MIN_STRING_SEARCH ){
				CDebugSet::MsgBoxAfx( "Search string too short!" );
				GotoDlgCtrl(GetDlgItem(IDC_PET_PAGE_EDIT_SEARCH)); 
				return;
			}

			CGMToolOdbcBase::GetInstance()->PetFind_Name( strSearch.c_str(), mapPetSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	case EMPETPAGE_SEARCH_PETID:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_PET_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadNumber( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Use numbers only!" );
				GotoDlgCtrl(GetDlgItem(IDC_PET_PAGE_EDIT_SEARCH)); 
				return;
			}

			int nSEARCH = atoi( strSearch.c_str() );
			CGMToolOdbcBase::GetInstance()->PetFind_PetID( nSEARCH, mapPetSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	case EMPETPAGE_SEARCH_PETOWNER:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_PET_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadNumber( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Use numbers only!" );
				GotoDlgCtrl(GetDlgItem(IDC_PET_PAGE_EDIT_SEARCH)); 
				return;
			}

			int nSEARCH = atoi( strSearch.c_str() );
			CGMToolOdbcBase::GetInstance()->PetFind_CharID( nSEARCH, mapPetSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	case EMPETPAGE_SEARCH_CARD_MID:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_PET_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadNumber( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Use numbers only!" );
				GotoDlgCtrl(GetDlgItem(IDC_PET_PAGE_EDIT_SEARCH)); 
				return;
			}

			int nSEARCH = atoi( strSearch.c_str() );
			CGMToolOdbcBase::GetInstance()->PetFind_MID( nSEARCH, mapPetSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	case EMPETPAGE_SEARCH_CARD_SID:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_PET_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadNumber( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Use numbers only!" );
				GotoDlgCtrl(GetDlgItem(IDC_PET_PAGE_EDIT_SEARCH)); 
				return;
			}

			int nSEARCH = atoi( strSearch.c_str() );
			CGMToolOdbcBase::GetInstance()->PetFind_SID( nSEARCH, mapPetSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	case EMPETPAGE_SEARCH_PETTYPE:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_PET_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadNumber( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Use numbers only!" );
				GotoDlgCtrl(GetDlgItem(IDC_PET_PAGE_EDIT_SEARCH)); 
				return;
			}

			int nSEARCH = atoi( strSearch.c_str() );
			CGMToolOdbcBase::GetInstance()->PetFind_Type( nSEARCH, mapPetSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	};

	if ( mapPetSimple.size() <=0 ){
		CDebugSet::MsgBoxAfx( "No Results" );
		return;
	}

	m_List.DeleteAllItems();

	m_List.SetRedraw( FALSE );

	SGMTOOL_PETDATA_SIMPLE_MAP_ITER mapiter = mapPetSimple.begin();
	for( int i=0; mapiter != mapPetSimple.end(); ++mapiter, ++i ){
		const SGMTOOL_PETDATA_SIMPLE sDATA = (*mapiter).second;

		m_List.InsertItem( i, _HLIB::cstring_dword( sDATA.dwPetNum ).GetString() );
		m_List.SetItemText( i, 1, _HLIB::cstring_dword( sDATA.dwPetChaNum ).GetString() );
		m_List.SetItemText( i, 2, sDATA.szPetName );
		m_List.SetItemText( i, 3, _HLIB::cstring_dword( sDATA.sIDPET.wMainID ).GetString() );
		m_List.SetItemText( i, 4, _HLIB::cstring_dword( sDATA.sIDPET.wSubID ).GetString() );
		m_List.SetItemText( i, 5, _HLIB::cstringformat( "[%02d]%s", sDATA.wPetType, COMMENT::PET_TYPE[sDATA.wPetType].c_str() ).GetString() );
		m_List.SetItemData ( i, sDATA.dwPetNum );
	}

	m_List.SetRedraw( TRUE );
}

void CPagePet::OnNMDblclkListPet(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		return;
	}

	DWORD dwPetNum = (DWORD)atoi( m_List.GetItemText ( nSelect, 0 ) );
	DWORD dwPetChaNum = (DWORD)atoi( m_List.GetItemText ( nSelect, 1 ) );
	std::string strPetname = m_List.GetItemText( nSelect, 2 );

	BOOL bONLINE = FALSE;
	CGMToolOdbcBase::GetInstance()->CharOnlineCheck( dwPetChaNum, bONLINE );
	if ( bONLINE ){
		if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Owner[%d] of this pet: [%d]%s is Online! continue?", dwPetChaNum, dwPetNum, strPetname.c_str() ) != IDYES ){
			return;
		}
	}

	int nDialogID = DialogNewID();
	if ( nDialogID == MAX_PAGEPET_MODAL_DIALOG ){
		CDebugSet::MsgBoxAfx( "Max dialog used!" );
		return;
	}

	CDlgPetEdit* pdlg;
	pdlg = new CDlgPetEdit( nDialogID, dwPetChaNum, dwPetNum, strPetname.c_str(), this );
	pdlg->Create( CWnd::GetDesktopWindow() );
	m_pToolDialog[nDialogID] = pdlg;
}

void CPagePet::OnBnClickedPetPageButtonEdit()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		CDebugSet::MsgBoxAfx( "Nothing Selected!" );
		return;
	}

	DWORD dwPetNum = (DWORD)atoi( m_List.GetItemText ( nSelect, 0 ) );
	DWORD dwPetChaNum = (DWORD)atoi( m_List.GetItemText ( nSelect, 1 ) );
	std::string strPetname = m_List.GetItemText( nSelect, 2 );

	BOOL bONLINE = FALSE;
	CGMToolOdbcBase::GetInstance()->CharOnlineCheck( dwPetChaNum, bONLINE );
	if ( bONLINE ){
		if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Owner[%d] of this pet: [%d]%s is Online! continue?", dwPetChaNum, dwPetNum, strPetname.c_str() ) != IDYES ){
			return;
		}
	}

	int nDialogID = DialogNewID();
	if ( nDialogID == MAX_PAGEPET_MODAL_DIALOG ){
		CDebugSet::MsgBoxAfx( "Max dialog used!" );
		return;
	}

	CDlgPetEdit* pdlg;
	pdlg = new CDlgPetEdit( nDialogID, dwPetChaNum, dwPetNum, strPetname.c_str(), this );
	pdlg->Create( CWnd::GetDesktopWindow() );
	m_pToolDialog[nDialogID] = pdlg;
}
