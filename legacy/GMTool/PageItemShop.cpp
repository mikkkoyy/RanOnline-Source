// PageItemShop.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "PageItemShop.h"

#include "SheetWithTab.h"
#include "Logic/GMToolGlobal.h"
#include "Logic/GMToolShopData.h"
#include "EtcFunction.h"
#include "HLibDataConvert.h"
#include "DlgItem.h"
#include "DlgShopItemEdit.h"
#include "GLItemMan.h"
#include "Logic//GMToolOdbcBase.h"

// CPageItemShop dialog

IMPLEMENT_DYNAMIC(CPageItemShop, CPropertyPage)

CPageItemShop::CPageItemShop(LOGFONT logfont)
	: CPropertyPage(CPageItemShop::IDD)
	, m_pFont ( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_vecResult.clear();
}

CPageItemShop::~CPageItemShop()
{
	m_vecResult.clear();

	SAFE_DELETE ( m_pFont );
}

void CPageItemShop::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ITEMSHOP, m_List);
}

BEGIN_MESSAGE_MAP(CPageItemShop, CPropertyPage)
	ON_BN_CLICKED(IDC_ITEMSHOP_PAGE_BUTTON_SEARCH, OnBnClickedItemshopPageButtonSearch)
	ON_BN_CLICKED(IDC_ITEMSHOP_PAGE_BUTTON_ADD, OnBnClickedItemshopPageButtonAdd)
	ON_BN_CLICKED(IDC_ITEMSHOP_PAGE_BUTTON_EDIT, OnBnClickedItemshopPageButtonEdit)
	ON_BN_CLICKED(IDC_ITEMSHOP_PAGE_BUTTON_DELETE, OnBnClickedItemshopPageButtonDelete)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ITEMSHOP, OnNMDblclkListItemshop)
	ON_BN_CLICKED(IDC_BUTTON1, &CPageItemShop::OnBnClickedButton1)
END_MESSAGE_MAP()

// CPageItemShop message handlers
BOOL CPageItemShop::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "", LVCFMT_LEFT, ( 3* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "ProductNum", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 2, "Item", LVCFMT_LEFT, ( 36* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 3, "Category", LVCFMT_LEFT, ( 7* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 4, "Stock", LVCFMT_LEFT, ( 7* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 5, "Price", LVCFMT_LEFT, ( 7* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 6, "Comment", LVCFMT_LEFT, ( 30* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	SetWin_Combo_Init( this, IDC_ITEMSHOP_PAGE_COMBO_SEARCH, GMTOOL_GLOBAL::strShopItemSearch2, EMSHOPITEM2_SEARCH_SIZE );
	SetWin_Text( this, IDC_ITEMSHOP_PAGE_EDIT_SEARCH, "" );
	
	ShowAllItems();

	return TRUE;  
}

void CPageItemShop::ShowAllItems()
{
	m_vecResult.clear();
	
	for( SGMTOOL_SHOP_ITEM_MAP_ITER iteritem = GMTOOL_SHOP_DATA::gmtoolSHOPDATA.begin();
		iteritem != GMTOOL_SHOP_DATA::gmtoolSHOPDATA.end();
		++ iteritem )
	{
		const SGMTOOL_SHOP_ITEM sDATA = (*iteritem).second;
		m_vecResult.push_back( sDATA );
	}

	ShowResults();
}

void CPageItemShop::ShowResults()
{
	m_List.DeleteAllItems();

	m_List.SetRedraw( FALSE );

	for( int i=0; i<(int)m_vecResult.size(); ++i )
	{
		const SGMTOOL_SHOP_ITEM sDATA = m_vecResult[i];

		m_List.InsertItem( i, "" );
		m_List.SetItemText( i, 1, _HLIB::cstring_dword( sDATA.dwProductNum ).GetString() );
		m_List.SetItemText( i, 2, _HLIB::cstringformat( "[%03d/%03d] %s", sDATA.wItemMain, sDATA.wItemSub, sDATA.szItemName ) );
		m_List.SetItemText( i, 3, _HLIB::cstring_dword( sDATA.dwCategory ).GetString() );
		m_List.SetItemText( i, 4, _HLIB::cstring_dword( sDATA.dwItemStock ).GetString() );
		m_List.SetItemText( i, 5, _HLIB::cstring_dword( sDATA.dwItemMoney ).GetString() );
		m_List.SetItemText( i, 6, sDATA.szItemComment );
		m_List.SetItemData ( i, sDATA.dwProductNum );
	}

	m_List.SetRedraw( TRUE );
}

void CPageItemShop::OnBnClickedItemshopPageButtonSearch()
{
	EMSHOPITEM2_SEARCH emSEARCH = static_cast<EMSHOPITEM2_SEARCH>( GetWin_Combo_Sel( this, IDC_ITEMSHOP_PAGE_COMBO_SEARCH ) );
	std::string strSEARCH = GetWin_Text( this, IDC_ITEMSHOP_PAGE_EDIT_SEARCH );

	m_vecResult.clear();

	if ( strSEARCH.size() <= 0 )
	{
		ShowAllItems();
		return;
	}
	else
	{
		for( SGMTOOL_SHOP_ITEM_MAP_ITER iteritem = GMTOOL_SHOP_DATA::gmtoolSHOPDATA.begin();
			iteritem != GMTOOL_SHOP_DATA::gmtoolSHOPDATA.end();
			++ iteritem )
		{
			const SGMTOOL_SHOP_ITEM sDATA = (*iteritem).second;
			
			CString strFIND;
			switch( emSEARCH )
			{
			case EMSHOPITEM2_SEARCH_ITEMNAME:
				{
					strFIND.Format( "%s", sDATA.szItemName );
				}break;
			case EMSHOPITEM2_SEARCH_PRODUCTNUM:
				{
					strFIND.Format( "%d", sDATA.dwProductNum );
				}break;
			case EMSHOPITEM2_SEARCH_CATEGORY:
				{
					strFIND.Format( "%d", sDATA.dwCategory );
				}break;
			case EMSHOPITEM2_SEARCH_MID:
				{
					strFIND.Format( "%d", sDATA.wItemMain );
				}break;
			case EMSHOPITEM2_SEARCH_SID:
				{
					strFIND.Format( "%d", sDATA.wItemSub );
				}break;
			case EMSHOPITEM2_SEARCH_STOCK:
				{
					strFIND.Format( "%d", sDATA.dwItemStock );
				}break;
			case EMSHOPITEM2_SEARCH_PRICE:
				{
					strFIND.Format( "%d", sDATA.dwItemMoney );
				}break;
			case EMSHOPITEM2_SEARCH_COMMENT:
				{
					strFIND.Format( "%s", sDATA.szItemComment );
				}break;
			};

			if ( strFIND.GetLength() <= 0 )	continue;

			std::string strSEARCH1 = strSEARCH.c_str();
			std::string strSEARCH2 = strFIND.GetString();
			std::transform ( strSEARCH1.begin(), strSEARCH1.end(), strSEARCH1.begin(), tolower );
			std::transform ( strSEARCH2.begin(), strSEARCH2.end(), strSEARCH2.begin(), tolower );

			unsigned found = strSEARCH2.find(strSEARCH1.c_str());
			if (found!=std::string::npos)
			{
				m_vecResult.push_back( sDATA );
			}
		}

		ShowResults();
	}
}

void CPageItemShop::OnBnClickedItemshopPageButtonAdd()
{
	CDlgItem dlg( this );
	if ( dlg.DoModal() != IDOK ){
		return;
	}

	SNATIVEID sidITEM = dlg.m_SELECTED;
	SITEM* pITEM = GLItemMan::GetInstance().GetItem ( sidITEM );
	if ( !pITEM ) {
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Item! ( %d %d )", 
			sidITEM.wMainID,
			sidITEM.wSubID );
		return;
	}

	SGMTOOL_SHOP_ITEM sDATA;
	sDATA.wItemMain = pITEM->sBasicOp.sNativeID.wMainID;
	sDATA.wItemSub = pITEM->sBasicOp.sNativeID.wSubID;
	StringCchCopy( sDATA.szItemName, SHOP_ITEMNAME_LENGTH, pITEM->GetName() );

	int nReturn = CGMToolOdbcBase::GetInstance()->ShopItemNew( &sDATA );
	if( nReturn != DB_OK ){
		CDebugSet::MsgBox( "Failure to insert item!" );
		return;
	}

	GMTOOL_SHOP_DATA::gmtoolSHOPDATA.insert( std::make_pair( sDATA.dwProductNum, sDATA ) );

	SGMTOOL_SHOP_ITEM* pShopItem = GMTOOL_SHOP_DATA::FindShopItem( sDATA.dwProductNum );
	if ( pShopItem ){
		if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Edit Inserted Item?") == IDYES ){
			CDlgShopItemEdit dlg( this, pShopItem );
			if ( dlg.DoModal() == IDOK ){
				nReturn = CGMToolOdbcBase::GetInstance()->ShopItemUpdate( pShopItem );
				if( nReturn != DB_OK ){
					CDebugSet::MsgBox( "Failure to save item!" );
				}
			}
		}
	}
	
	ShowAllItems();

	for( int nItem=0; nItem<m_List.GetItemCount(); nItem++){
		DWORD dwID = m_List.GetItemData( nItem );
		if( dwID == sDATA.dwProductNum ){
			m_List.EnsureVisible( nItem, TRUE );
			m_List.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
			GotoDlgCtrl(GetDlgItem(m_List.GetDlgCtrlID())); 
		}
	}
}

void CPageItemShop::EditItem()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ){
		return;
	}

	DWORD dwProductNum = (DWORD) m_List.GetItemData ( nSelect );

	SGMTOOL_SHOP_ITEM* pShopItem = GMTOOL_SHOP_DATA::FindShopItem( dwProductNum );
	if ( pShopItem )
	{
		CDlgShopItemEdit dlg( this, pShopItem );
		if ( dlg.DoModal() == IDOK )
		{
			int nReturn = CGMToolOdbcBase::GetInstance()->ShopItemUpdate( pShopItem );
			if( nReturn != DB_OK ){
				CDebugSet::MsgBox( "Failure to save item!" );
			}

			ShowAllItems();

			for( int nItem=0; nItem<m_List.GetItemCount(); nItem++){
				DWORD dwID = m_List.GetItemData( nItem );
				if( dwID == dwProductNum ){
					m_List.EnsureVisible( nItem, TRUE );
					m_List.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
					GotoDlgCtrl(GetDlgItem(m_List.GetDlgCtrlID())); 
				}
			}
		}
	}
}

void CPageItemShop::OnBnClickedItemshopPageButtonEdit()
{
	EditItem();
}

void CPageItemShop::OnBnClickedItemshopPageButtonDelete()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ){
		CDebugSet::MsgBoxAfx( "Nothing Selected!" );
		return;
	}

	DWORD dwProductNum = (DWORD) m_List.GetItemData ( nSelect );

	SGMTOOL_SHOP_ITEM* pShopItem = GMTOOL_SHOP_DATA::FindShopItem( dwProductNum );
	if ( pShopItem )
	{
		if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete Item:[%d] %s \n WARNING! Deleting item will also delete this item in ShopPurchase, Continue?", 
			pShopItem->dwProductNum, pShopItem->szItemName ) == IDYES )
		{
			int nReturn = CGMToolOdbcBase::GetInstance()->ShopItemDelete( pShopItem );
			if( nReturn != DB_OK ){
				CDebugSet::MsgBox( "Failure to delete item!" );
				return;
			}

			GMTOOL_SHOP_DATA::DeleteShopItem( pShopItem->dwProductNum );
			ShowAllItems();

			if ( m_List.GetItemCount() > 0 )
			{
				if ( nSelect > m_List.GetItemCount()-1 )
					nSelect = m_List.GetItemCount()-1;

				m_List.EnsureVisible( nSelect, TRUE );
				m_List.SetItemState( nSelect, LVIS_SELECTED, LVIS_SELECTED );
				GotoDlgCtrl(GetDlgItem(m_List.GetDlgCtrlID())); 
			}
		}
	}
}

void CPageItemShop::OnNMDblclkListItemshop(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	EditItem();
}

void CPageItemShop::OnBnClickedButton1()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ){
		return;
	}

	DWORD dwProductNum = (DWORD) m_List.GetItemData ( nSelect );

	SGMTOOL_SHOP_ITEM_MAP_ITER iterpos = GMTOOL_SHOP_DATA::gmtoolSHOPDATA.find(dwProductNum);
	if ( iterpos != GMTOOL_SHOP_DATA::gmtoolSHOPDATA.end() ){
		const SGMTOOL_SHOP_ITEM sDATA = (*iterpos).second;

		if ( DB_OK == CGMToolOdbcBase::GetInstance()->UserBankInsertAll( sDATA.dwProductNum ) )
		{
			MessageBox("Send to Bank OK!");
		}
	}
}
