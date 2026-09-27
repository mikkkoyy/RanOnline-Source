// DlgUserItemBank.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgUserItemBank.h"

#include "EtcFunction.h"
#include "HLibDataConvert.h"
#include "PageUser.h"
#include "Logic/GMToolGlobal.h"
#include "Logic/GMToolOdbcBase.h"
#include "Logic/GMToolShopData.h"
#include "Logic/GMToolOdbcBase.h"
#include "DlgShopItemList.h"


// CDlgUserItemBank dialog

IMPLEMENT_DYNAMIC(CDlgUserItemBank, CDialog)

CDlgUserItemBank::CDlgUserItemBank(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgUserItemBank::IDD, pParent)
	, m_pParentWnd(NULL)
	, m_nDialogID(-1)
	, m_dwID(0)
	, m_strNAME("")
{
}

CDlgUserItemBank::CDlgUserItemBank(int nDialogID, DWORD dwID, std::string strNAME, CWnd* pParent /*= NULL*/)
	: CDialog(CDlgUserItemBank::IDD, pParent)
	, m_pParentWnd(pParent)
	, m_nDialogID(nDialogID)
	, m_dwID(dwID)
	, m_strNAME(strNAME)
{
}

CDlgUserItemBank::~CDlgUserItemBank()
{
}

void CDlgUserItemBank::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_USER_ITEMBANK, m_List );
}


BEGIN_MESSAGE_MAP(CDlgUserItemBank, CDialog)
	ON_BN_CLICKED(IDC_USER_ITEMBANK_BUTTON_REFRESH, OnBnClickedUserItembankButtonRefresh)
	ON_BN_CLICKED(IDC_USER_ITEMBANK_BUTTON_INSERTITEM, OnBnClickedUserItembankButtonInsertitem)
	ON_BN_CLICKED(IDC_USER_ITEMBANK_BUTTON_DELETE, OnBnClickedUserItembankButtonDelete)
	ON_BN_CLICKED(IDC_USER_ITEMBANK_BUTTON_CLEAR, OnBnClickedUserItembankButtonClear)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgUserItemBank message handlers
BOOL CDlgUserItemBank::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString strWindowText;
	strWindowText.Format( "ItemBank User:(%d) %s", m_dwID, m_strNAME.c_str() );
	SetWindowText( strWindowText.GetString() );

	SetWin_Num_int( this, IDC_USER_ITEMBANK_EDIT_NUM, m_dwID );
	SetWin_Text( this, IDC_USER_ITEMBANK_EDIT_NAME, m_strNAME.c_str() );

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "", LVCFMT_LEFT, ( 5* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Purkey", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 2, "ProductNum", LVCFMT_LEFT, ( 15* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 3, "Item", LVCFMT_LEFT, ( 30* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 4, "PurchaseDate", LVCFMT_LEFT, ( 25* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	DataGet();
	DataShow();

	return TRUE;  
}

void CDlgUserItemBank::PostNcDestroy()
{
	CDialog::PostNcDestroy();

	if ( m_pParentWnd ){
		((CPageUser*)m_pParentWnd)->DialogDeleteID(m_nDialogID);
	}

	delete this;
}

void CDlgUserItemBank::OnOK()
{
	if(UpdateData(true)){
		DestroyWindow();
	}
}

void CDlgUserItemBank::OnCancel()
{
	DestroyWindow();
}

void CDlgUserItemBank::DataGet()
{
	m_mapUserBank.clear();

	int nRESULT = CGMToolOdbcBase::GetInstance()->UserBankGet( m_strNAME.c_str(), m_mapUserBank );
	if ( nRESULT == DB_ERROR ){
		CDebugSet::MsgBox( "Unable to Get user ItemBank! User:[%d] %s", m_dwID, m_strNAME.c_str() );
	}
}

void CDlgUserItemBank::DataShow()
{
	m_List.DeleteAllItems();

	m_List.SetRedraw( FALSE );

	SGMTOOL_USER_BANK_ITEM_MAP_ITER iterpos = m_mapUserBank.begin();
	for( int i=0; iterpos != m_mapUserBank.end(); ++ iterpos, ++i ){
		const SGMTOOL_USER_BANK_ITEM sData = (*iterpos).second;

		m_List.InsertItem( i, "" );
		m_List.SetItemText( i, 1, _HLIB::cstring_dword(sData.dwPurKey).GetString() );
		m_List.SetItemText( i, 2, _HLIB::cstring_dword(sData.dwProductNum).GetString() );
		m_List.SetItemText( i, 3, _HLIB::cstringformat( "[%03d/%03d] %s", sData.wItemMain, sData.wItemSub, sData.szItemName ).GetString() );
		m_List.SetItemText( i, 4, _HLIB::cstring_timet12(sData.tPurchaseDate).GetString() );

		m_List.SetItemData ( i, sData.dwPurKey );
	}

	m_List.SetRedraw( TRUE );
}

void CDlgUserItemBank::OnBnClickedUserItembankButtonRefresh()
{
	DataGet();
	DataShow();
}

void CDlgUserItemBank::OnBnClickedUserItembankButtonInsertitem()
{
	CDlgShopItemList dlg( this );
	if ( dlg.DoModal() == IDOK ){

		SGMTOOL_SHOP_ITEM_MAP_ITER iterpos = GMTOOL_SHOP_DATA::gmtoolSHOPDATA.find(dlg.m_dwSELECTED);
		if ( iterpos != GMTOOL_SHOP_DATA::gmtoolSHOPDATA.end() ){
			const SGMTOOL_SHOP_ITEM sDATA = (*iterpos).second;

			CGMToolOdbcBase::GetInstance()->UserBankInsert( m_strNAME.c_str(), sDATA.dwProductNum );

			DataGet();
			DataShow();
		}	
	}
}

void CDlgUserItemBank::OnBnClickedUserItembankButtonDelete()
{
	int nCHECKED = 0;
	for( int nItem=0; nItem<m_List.GetItemCount(); nItem++ ){
		BOOL bChecked = m_List.GetCheck( nItem );
		if( bChecked){
			DWORD dwPurKey = (DWORD) m_List.GetItemData ( nItem );
			
			int nRESULT = CGMToolOdbcBase::GetInstance()->UserBankMarkTaken( m_strNAME.c_str(), dwPurKey );
			if ( nRESULT == DB_OK ){
				SGMTOOL_USER_BANK_ITEM_MAP_ITER iterpos = m_mapUserBank.find(dwPurKey);
				if ( iterpos != m_mapUserBank.end() ){
					m_mapUserBank.erase( iterpos );
				}
			}

			nCHECKED ++;
		}
	}

	if ( nCHECKED == 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected Use Check boxes to select" );
	}

	DataShow();
}

void CDlgUserItemBank::OnBnClickedUserItembankButtonClear()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "WIPE this users ITEM BANK! Continue?" ) == IDYES ){
		int nRESULT = CGMToolOdbcBase::GetInstance()->UserBankWipe( m_strNAME.c_str() );
		if ( nRESULT == DB_OK ){
			m_mapUserBank.clear();
			DataGet();
			DataShow();
		}
	}
}

void CDlgUserItemBank::OnBnClickedCancel()
{
	OnCancel();
}
