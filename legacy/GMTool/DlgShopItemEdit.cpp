// DlgShopItemEdit.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgShopItemEdit.h"

#include "Logic/GMToolData.h"
#include "EtcFunction.h"
#include "GLItemMan.h"
#include "DlgItem.h"
#include "Logic/GMToolGlobal.h"

// CDlgShopItemEdit dialog

IMPLEMENT_DYNAMIC(CDlgShopItemEdit, CDialog)

CDlgShopItemEdit::CDlgShopItemEdit(CWnd* pParent /*=NULL*/, SGMTOOL_SHOP_ITEM* pDATA /*= NULL*/)
	: CDialog(CDlgShopItemEdit::IDD, pParent)
	, m_pData( NULL )
{
	m_pData = pDATA;
}

CDlgShopItemEdit::~CDlgShopItemEdit()
{
}

void CDlgShopItemEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgShopItemEdit, CDialog)
	ON_BN_CLICKED(IDC_SHOPITEM_BUTTON_EDIT_ITEM, OnBnClickedShopitemButtonEditItem)
	ON_BN_CLICKED(IDC_SHOPITEM_BUTTON_USE_DEFAULT, OnBnClickedShopitemButtonUseDefault)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_EN_CHANGE(IDC_SHOPITEM_EDIT_ITEM_MID, OnEnChangeShopitemEditItemMid)
	ON_EN_CHANGE(IDC_SHOPITEM_EDIT_ITEM_SID, OnEnChangeShopitemEditItemSid)
END_MESSAGE_MAP()


// CDlgShopItemEdit message handlers
BOOL CDlgShopItemEdit::OnInitDialog()
{
	CDialog::OnInitDialog();

	std::string _strItemSec[EMSHOPITEM_ITEMSEC_SIZE];
	for( int i=0; i<EMSHOPITEM_ITEMSEC_SIZE; ++i )
	{
		CString strText;
		strText.Format("(%d)%s", i, GMTOOL_GLOBAL::strItemSec[i].c_str());
		_strItemSec[i] = strText.GetString();
	}

	std::string _strItemCategory[EMSHOPITEM_ITEMCATEGORY_SIZE];
	for( int i=0; i<EMSHOPITEM_ITEMCATEGORY_SIZE; ++i )
	{
		CString strText;
		strText.Format("(%d)%s", i, GMTOOL_GLOBAL::strItemCategory[i].c_str());
		_strItemCategory[i] = strText.GetString();
	}

	SetWin_Combo_Init( this, IDC_SHOPITEM_EDIT_ITEM_SEC_COMBO, _strItemSec, EMSHOPITEM_ITEMSEC_SIZE );
	SetWin_Combo_Init( this, IDC_SHOPITEM_EDIT_ITEM_CATEGORY_COMBO, _strItemCategory, EMSHOPITEM_ITEMCATEGORY_SIZE );

	DataShow();

	return TRUE;
}

void CDlgShopItemEdit::DataShow()
{
	if ( !m_pData )	return;

	SetWin_Num_int( this, IDC_SHOPITEM_EDIT_PRODUCTNUM, m_pData->dwProductNum );
	SetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_MID, m_pData->wItemMain );
	SetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_SID, m_pData->wItemSub );
	SetWin_Text( this, IDC_SHOPITEM_EDIT_ITEM_NAME, m_pData->szItemName );
	SetWin_Text( this, IDC_SHOPITEM_EDIT_ITEM_DURATION, m_pData->szDuration );
	SetWin_Text( this, IDC_SHOPITEM_EDIT_ITEM_IMAGE, m_pData->szItemImage );
	SetWin_Text( this, IDC_SHOPITEM_EDIT_ITEM_COMMENT, m_pData->szItemComment );
	SetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_LIST, m_pData->wItemList );
	//SetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_CATEGORY, m_pData->dwCategory );
	SetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_STOCK, m_pData->dwItemStock );
	SetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_MONEY, m_pData->dwItemMoney );
	//SetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_SEC, m_pData->dwItemSec );
	SetWin_Combo_Sel( this, IDC_SHOPITEM_EDIT_ITEM_CATEGORY_COMBO, m_pData->dwCategory );
	SetWin_Combo_Sel( this, IDC_SHOPITEM_EDIT_ITEM_SEC_COMBO, m_pData->wItemSec );
}

BOOL CDlgShopItemEdit::DataSave()
{
	if ( !m_pData )	return FALSE;

	WORD wMID = GetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_MID );
	WORD wSID = GetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_SID );
	SITEM* pItem = GLItemMan::GetInstance().GetItem( wMID, wSID );
	if ( !pItem )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Item( %d~%d )", wMID, wSID );
		return FALSE;
	}

	std::string strItemName = GetWin_Text( this, IDC_SHOPITEM_EDIT_ITEM_NAME );
	std::string strDuration = GetWin_Text( this, IDC_SHOPITEM_EDIT_ITEM_DURATION );
	std::string strItemImage = GetWin_Text( this, IDC_SHOPITEM_EDIT_ITEM_IMAGE );
	std::string strItemComment = GetWin_Text( this, IDC_SHOPITEM_EDIT_ITEM_COMMENT );
	if ( strItemName.size() <= 0 )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "ItemName Cannot be empty!" );
		return FALSE;
	}

	
	EMSHOPITEM_ITEMSEC emItemSec = static_cast<EMSHOPITEM_ITEMSEC> ( GetWin_Combo_Sel( this, IDC_SHOPITEM_EDIT_ITEM_SEC_COMBO ) );
	EMSHOPITEM_ITEMCATEGORY emItemCategory = static_cast<EMSHOPITEM_ITEMCATEGORY> ( GetWin_Combo_Sel( this, IDC_SHOPITEM_EDIT_ITEM_CATEGORY_COMBO ) );

	StringCchCopy( m_pData->szItemName, SHOP_ITEMNAME_LENGTH, strItemName.c_str() );
	StringCchCopy( m_pData->szDuration, SHOP_DURATION_LENGTH, strDuration.c_str() );
	StringCchCopy( m_pData->szItemImage, SHOP_ITEMIMAGE_LENGTH, strItemImage.c_str() );
	StringCchCopy( m_pData->szItemComment, SHOP_ITEMCOMMENT_LENGTH, strItemComment.c_str() );

	m_pData->wItemMain = wMID;
	m_pData->wItemSub = wSID;
	m_pData->wItemList = GetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_LIST );
	//m_pData->dwCategory = GetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_CATEGORY );
	m_pData->dwItemStock = GetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_STOCK );
	m_pData->dwItemMoney = GetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_MONEY );
	//m_pData->dwItemSec = GetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_SEC );
	m_pData->dwCategory = (WORD)emItemCategory;
	m_pData->wItemSec = (WORD)emItemSec ;

	return TRUE;
}

void CDlgShopItemEdit::OnBnClickedShopitemButtonEditItem()
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

	SetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_MID, sidITEM.wMainID );
	SetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_SID, sidITEM.wSubID );
}

void CDlgShopItemEdit::OnBnClickedShopitemButtonUseDefault()
{
	WORD wMID = GetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_MID );
	WORD wSID = GetWin_Num_int( this, IDC_SHOPITEM_EDIT_ITEM_SID );
	SITEM* pItem = GLItemMan::GetInstance().GetItem( wMID, wSID );
	if ( pItem )
	{
		SetWin_Text( this, IDC_SHOPITEM_EDIT_ITEM_NAME, pItem->GetName() );
	}	
}

void CDlgShopItemEdit::OnBnClickedOk()
{
	if ( DataSave() ){
		CDialog::OnOK();
	}
}

void CDlgShopItemEdit::OnBnClickedCancel()
{
	CDialog::OnCancel();
}

void CDlgShopItemEdit::OnEnChangeShopitemEditItemMid()
{
	GMTOOL_GLOBAL::DataCheckItem( this, 
		IDC_SHOPITEM_EDIT_ITEM_MID,
		IDC_SHOPITEM_EDIT_ITEM_SID,
		IDC_SHOPITEM_EDIT_ITEMNAME );
}

void CDlgShopItemEdit::OnEnChangeShopitemEditItemSid()
{
	GMTOOL_GLOBAL::DataCheckItem( this, 
		IDC_SHOPITEM_EDIT_ITEM_MID,
		IDC_SHOPITEM_EDIT_ITEM_SID,
		IDC_SHOPITEM_EDIT_ITEMNAME );
}
