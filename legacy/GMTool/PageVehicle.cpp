// PageVehicle.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "PageVehicle.h"

#include "SheetWithTab.h"
#include "Logic/GMToolGlobal.h"
#include "Logic/GMToolOdbcBase.h"

#include "HLibDataConvert.h"
#include "EtcFunction.h"
#include "DlgVehicleEdit.h"

// CPageVehicle dialog

IMPLEMENT_DYNAMIC(CPageVehicle, CPropertyPage)

CPageVehicle::CPageVehicle(LOGFONT logfont)
	: CPropertyPage(CPageVehicle::IDD)
	, m_pFont ( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	for ( int i=0; i<MAX_PAGEVEHICLE_MODAL_DIALOG; ++i ){
		m_pToolDialog[i] = NULL;
	}
}

CPageVehicle::~CPageVehicle()
{
	SAFE_DELETE ( m_pFont );
}

void CPageVehicle::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_VEHICLE, m_List );
}


BEGIN_MESSAGE_MAP(CPageVehicle, CPropertyPage)
	ON_BN_CLICKED(IDC_VEHICLE_PAGE_BUTTON_SEARCH, OnBnClickedVehiclePageButtonSearch)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_VEHICLE, OnNMDblclkListVehicle)
	ON_BN_CLICKED(IDC_VEHICLE_PAGE_BUTTON_EDIT, OnBnClickedVehiclePageButtonEdit)
END_MESSAGE_MAP()


// CPageVehicle message handlers
BOOL CPageVehicle::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "Vehicle Number", LVCFMT_LEFT, ( 15* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Vehicle ChaNumber", LVCFMT_LEFT, ( 15* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 2, "Vehicle Name", LVCFMT_LEFT, ( 30* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 3, "Item MID", LVCFMT_LEFT, ( 8* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 4, "Item SID", LVCFMT_LEFT, ( 8* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 5, "Vehicle Type", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	SetWin_Combo_Init( this, IDC_VEHICLE_PAGE_COMBO_SEARCH, GMTOOL_GLOBAL::strVehiclePageSearchType, EMVEHICLEPAGE_SEARCH_SIZE );
	SetWin_Combo_Init( this, IDC_VEHICLE_PAGE_COMBO_SEARCH_LIMITNUM, GMTOOL_GLOBAL::strSearchLimit, EMSEARCH_LIMIT_SIZE );

	return TRUE;  
}

int CPageVehicle::DialogNewID()
{
	int nNewDlg = MAX_PAGEVEHICLE_MODAL_DIALOG;
	for ( int i=0; i<MAX_PAGEVEHICLE_MODAL_DIALOG; ++i ){
		if ( !m_pToolDialog[i] ){
			nNewDlg = i;
			break;
		}
	}

	return nNewDlg;
}

void CPageVehicle::DialogDeleteID( int nDialogID )
{
	if ( nDialogID < 0 || nDialogID >= MAX_PAGEVEHICLE_MODAL_DIALOG ){
		return;
	}

	m_pToolDialog[nDialogID] = NULL;
}

int CPageVehicle::DialogGetTotalActive()
{
	int nDlg = 0;
	for ( int i=0; i<MAX_PAGEVEHICLE_MODAL_DIALOG; ++i ){
		if ( m_pToolDialog[i] ){
			nDlg ++;
		}
	}

	return nDlg;
}

void CPageVehicle::OnBnClickedVehiclePageButtonSearch()
{
	std::string strSearch = GetWin_Text( this, IDC_VEHICLE_PAGE_EDIT_SEARCH );
	EMVEHICLEPAGE_SEARCH_TYPE emType = static_cast<EMVEHICLEPAGE_SEARCH_TYPE> ( GetWin_Combo_Sel( this, IDC_VEHICLE_PAGE_COMBO_SEARCH ) );
	EMSEARCH_LIMIT emLIMIT = static_cast<EMSEARCH_LIMIT> ( GetWin_Combo_Sel( this, IDC_VEHICLE_PAGE_COMBO_SEARCH_LIMITNUM ) );

	SGMTOOL_VEHICLEDATA_SIMPLE_MAP mapVehicleSimple;

	switch( emType )
	{
	case EMVEHICLEPAGE_SEARCH_NAME:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_VEHICLE_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadString( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Search contains invalid characters!" );
				GotoDlgCtrl(GetDlgItem(IDC_VEHICLE_PAGE_EDIT_SEARCH)); 
				return;
			}

			if( strSearch.size() < MIN_STRING_SEARCH ){
				CDebugSet::MsgBoxAfx( "Search string too short!" );
				GotoDlgCtrl(GetDlgItem(IDC_VEHICLE_PAGE_EDIT_SEARCH)); 
				return;
			}

			CGMToolOdbcBase::GetInstance()->VehicleFind_Name( strSearch.c_str(), mapVehicleSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	case EMVEHICLEPAGE_SEARCH_VEHICLENUM:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_VEHICLE_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadNumber( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Use numbers only!" );
				GotoDlgCtrl(GetDlgItem(IDC_VEHICLE_PAGE_EDIT_SEARCH)); 
				return;
			}

			int nSEARCH = atoi( strSearch.c_str() );
			CGMToolOdbcBase::GetInstance()->VehicleFind_VehicleID( nSEARCH, mapVehicleSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	case EMVEHICLEPAGE_SEARCH_VEHICLEOWNER:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_VEHICLE_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadNumber( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Use numbers only!" );
				GotoDlgCtrl(GetDlgItem(IDC_VEHICLE_PAGE_EDIT_SEARCH)); 
				return;
			}

			int nSEARCH = atoi( strSearch.c_str() );
			CGMToolOdbcBase::GetInstance()->VehicleFind_CharID( nSEARCH, mapVehicleSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	case EMVEHICLEPAGE_SEARCH_MID:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_VEHICLE_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadNumber( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Use numbers only!" );
				GotoDlgCtrl(GetDlgItem(IDC_VEHICLE_PAGE_EDIT_SEARCH)); 
				return;
			}

			int nSEARCH = atoi( strSearch.c_str() );
			CGMToolOdbcBase::GetInstance()->VehicleFind_MID( nSEARCH, mapVehicleSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	case EMVEHICLEPAGE_SEARCH_SID:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_VEHICLE_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadNumber( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Use numbers only!" );
				GotoDlgCtrl(GetDlgItem(IDC_VEHICLE_PAGE_EDIT_SEARCH)); 
				return;
			}

			int nSEARCH = atoi( strSearch.c_str() );
			CGMToolOdbcBase::GetInstance()->VehicleFind_SID( nSEARCH, mapVehicleSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	case EMVEHICLEPAGE_SEARCH_VEHICLETYPE:
		{
			if( strSearch.size() <= 0 ){
				CDebugSet::MsgBoxAfx( "Invalid Search!" );
				GotoDlgCtrl(GetDlgItem(IDC_VEHICLE_PAGE_EDIT_SEARCH)); 
				return;
			}

			if ( GMTOOL_GLOBAL::IsBadNumber( strSearch.c_str() ) ){
				CDebugSet::MsgBoxAfx( "Use numbers only!" );
				GotoDlgCtrl(GetDlgItem(IDC_VEHICLE_PAGE_EDIT_SEARCH)); 
				return;
			}

			int nSEARCH = atoi( strSearch.c_str() );
			CGMToolOdbcBase::GetInstance()->VehicleFind_Type( nSEARCH, mapVehicleSimple, GetSearchLimitNum( emLIMIT ) );
		}break;

	};

	if ( mapVehicleSimple.size() <=0 ){
		CDebugSet::MsgBoxAfx( "No Results" );
		return;
	}

	m_List.DeleteAllItems();

	m_List.SetRedraw( FALSE );

	SGMTOOL_VEHICLEDATA_SIMPLE_MAP_ITER mapiter = mapVehicleSimple.begin();
	for( int i=0; mapiter != mapVehicleSimple.end(); ++mapiter, ++i ){
		const SGMTOOL_VEHICLEDATA_SIMPLE sDATA = (*mapiter).second;

		m_List.InsertItem( i, _HLIB::cstring_dword( sDATA.dwVehicleNum ).GetString() );
		m_List.SetItemText( i, 1, _HLIB::cstring_dword( sDATA.dwVehicleChaNum ).GetString() );
		m_List.SetItemText( i, 2, sDATA.szVehicleName );
		m_List.SetItemText( i, 3, _HLIB::cstring_dword( sDATA.sIDVEHICLE.wMainID ).GetString() );
		m_List.SetItemText( i, 4, _HLIB::cstring_dword( sDATA.sIDVEHICLE.wSubID ).GetString() );
		m_List.SetItemText( i, 5, _HLIB::cstringformat( "[%02d]%s", sDATA.wVehicleType, COMMENT::VEHICLE_TYPE[sDATA.wVehicleType].c_str() ).GetString() );
		m_List.SetItemData ( i, sDATA.dwVehicleNum );
	}

	m_List.SetRedraw( TRUE );
}

void CPageVehicle::OnNMDblclkListVehicle(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		return;
	}

	DWORD dwVehicleNum = (DWORD)atoi( m_List.GetItemText ( nSelect, 0 ) );
	DWORD dwVehicleChaNum = (DWORD)atoi( m_List.GetItemText ( nSelect, 1 ) );
	std::string strVehicleName = m_List.GetItemText( nSelect, 2 );

	BOOL bONLINE = FALSE;
	CGMToolOdbcBase::GetInstance()->CharOnlineCheck( dwVehicleChaNum, bONLINE );
	if ( bONLINE ){
		if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Owner[%d] of this vehicle: [%d]%s is Online! continue?", dwVehicleChaNum, dwVehicleNum, strVehicleName.c_str() ) != IDYES ){
			return;
		}
	}

	int nDialogID = DialogNewID();
	if ( nDialogID == MAX_PAGEVEHICLE_MODAL_DIALOG ){
		CDebugSet::MsgBoxAfx( "Max dialog used!" );
		return;
	}

	CDlgVehicleEdit* pdlg;
	pdlg = new CDlgVehicleEdit( nDialogID, dwVehicleChaNum, dwVehicleNum, strVehicleName.c_str(), this );
	pdlg->Create( CWnd::GetDesktopWindow() );
	m_pToolDialog[nDialogID] = pdlg;
}

void CPageVehicle::OnBnClickedVehiclePageButtonEdit()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) {
		CDebugSet::MsgBoxAfx( "Nothing Selected!" );
		return;
	}

	DWORD dwVehicleNum = (DWORD)atoi( m_List.GetItemText ( nSelect, 0 ) );
	DWORD dwVehicleChaNum = (DWORD)atoi( m_List.GetItemText ( nSelect, 1 ) );
	std::string strVehicleName = m_List.GetItemText( nSelect, 2 );

	BOOL bONLINE = FALSE;
	CGMToolOdbcBase::GetInstance()->CharOnlineCheck( dwVehicleChaNum, bONLINE );
	if ( bONLINE ){
		if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Owner[%d] of this vehicle: [%d]%s is Online! continue?", dwVehicleChaNum, dwVehicleNum, strVehicleName.c_str() ) != IDYES ){
			return;
		}
	}

	int nDialogID = DialogNewID();
	if ( nDialogID == MAX_PAGEVEHICLE_MODAL_DIALOG ){
		CDebugSet::MsgBoxAfx( "Max dialog used!" );
		return;
	}

	CDlgVehicleEdit* pdlg;
	pdlg = new CDlgVehicleEdit( nDialogID, dwVehicleChaNum, dwVehicleNum, strVehicleName.c_str(), this );
	pdlg->Create( CWnd::GetDesktopWindow() );
	m_pToolDialog[nDialogID] = pdlg;
}
