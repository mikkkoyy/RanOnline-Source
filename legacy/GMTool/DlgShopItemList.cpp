// DlgShopItemList.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgShopItemList.h"

#include "EtcFunction.h"
#include "GLItemMan.h"
#include "Logic/GMToolGlobal.h"
#include "Logic/GMToolShopData.h"

// CDlgShopItemList dialog

IMPLEMENT_DYNAMIC(CDlgShopItemList, CDialog)

CDlgShopItemList::CDlgShopItemList(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgShopItemList::IDD, pParent)
	, m_dwSELECTED( SNATIVEID::ID_NULL )
{
	m_vecData.clear();
	m_vecResult.clear();
}

CDlgShopItemList::~CDlgShopItemList()
{
}

void CDlgShopItemList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SELECT, m_listSELECT );
}


BEGIN_MESSAGE_MAP(CDlgShopItemList, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnBnClickedButtonSearch)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_LBN_SELCHANGE(IDC_LIST_SELECT, OnLbnSelchangeListSelect)
END_MESSAGE_MAP()


// CDlgShopItemList message handlers
BOOL CDlgShopItemList::OnInitDialog()
{
	CDialog::OnInitDialog();

	ReadData();
	SelectionReset();

	SetWin_Combo_Init( this, IDC_COMBO_SEARCH, GMTOOL_GLOBAL::strShopItemSearch, EMSHOPITEM_SEARCH_SIZE );
	SetWin_Text( this, IDC_EDIT_SEARCH, "" );

	SearchData();

	return TRUE;
}

void CDlgShopItemList::ReadData()
{
	m_vecData.clear();

	SGMTOOL_SHOP_ITEM_MAP_ITER iterpos = GMTOOL_SHOP_DATA::gmtoolSHOPDATA.begin();
	SGMTOOL_SHOP_ITEM_MAP_ITER iterend = GMTOOL_SHOP_DATA::gmtoolSHOPDATA.end();
	for( ; iterpos != iterend; ++iterpos ){
		const SGMTOOL_SHOP_ITEM sDATA = (*iterpos).second;
		m_vecData.push_back( sDATA );
	}
}

void CDlgShopItemList::ShowData()
{
	m_listSELECT.ResetContent();

	for( int i=0; i<(int)m_vecResult.size(); ++i )
	{
		const SGMTOOL_SHOP_ITEM sDATA = m_vecResult[i];

		CString strNAME;
		strNAME.Format ( "[Prod.:%07d][Cat.:%03d][%03d/%03d] %s", sDATA.dwProductNum, sDATA.dwCategory, sDATA.wItemMain, sDATA.wItemSub, sDATA.szItemName );

		int nIndex = m_listSELECT.AddString ( strNAME );
		m_listSELECT.SetItemData ( nIndex, sDATA.dwProductNum );
	}
}

void CDlgShopItemList::SelectionReset()
{
	m_dwSELECTED = SNATIVEID::ID_NULL;
	SetWin_Text( this, IDC_EDIT_SEL_NAME, "" );
}

void CDlgShopItemList::SearchData()
{
	EMSHOPITEM_SEARCH emSearch =  static_cast<EMSHOPITEM_SEARCH> ( GetWin_Combo_Sel( this, IDC_COMBO_SEARCH ) );
	std::string strSEARCH = GetWin_Text( this, IDC_EDIT_SEARCH );

	m_vecResult.clear();

	if ( strSEARCH.size() <= 0 )
	{
		m_vecResult = m_vecData;
	}
	else
	{
		for( int i=0; i<(int)m_vecData.size(); ++i )
		{
			const SGMTOOL_SHOP_ITEM sDATA = m_vecData[i];

			CString strFIND;
			switch( emSearch )
			{
			case EMSHOPITEM_SEARCH_ITEMNAME:
				{
					strFIND.Format( "%s", sDATA.szItemName );
				}break;
			case EMSHOPITEM_SEARCH_PRODUCTNUM:
				{
					strFIND.Format( "%d", sDATA.dwProductNum );
				}break;
			case EMSHOPITEM_SEARCH_CATEGORY:
				{
					strFIND.Format( "%d", sDATA.dwCategory );
				}break;
			case EMSHOPITEM_SEARCH_MID:
				{
					strFIND.Format( "%d", sDATA.wItemMain );
				}break;
			case EMSHOPITEM_SEARCH_SID:
				{
					strFIND.Format( "%d", sDATA.wItemSub );
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
	}

	ShowData();
}

void CDlgShopItemList::OnBnClickedButtonSearch()
{
	SearchData();
}

void CDlgShopItemList::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}

void CDlgShopItemList::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}

void CDlgShopItemList::OnLbnSelchangeListSelect()
{
	int nSel = m_listSELECT.GetCurSel();
	if ( nSel < 0 ){
		return;
	}

	DWORD dwData = (DWORD) m_listSELECT.GetItemData ( nSel );

	SGMTOOL_SHOP_ITEM_MAP_ITER iterpos = GMTOOL_SHOP_DATA::gmtoolSHOPDATA.find(dwData);
	if ( iterpos != GMTOOL_SHOP_DATA::gmtoolSHOPDATA.end() ){
		const SGMTOOL_SHOP_ITEM sDATA = (*iterpos).second;

		m_dwSELECTED = sDATA.dwProductNum;

		CString strNAME;
		strNAME.Format ( "[Prod.:%07d][Cat.:%03d][%03d/%03d] %s", sDATA.dwProductNum, sDATA.dwCategory, sDATA.wItemMain, sDATA.wItemSub, sDATA.szItemName );
		SetWin_Text( this, IDC_EDIT_SEL_NAME, strNAME.GetString() );
	}else{
		SelectionReset();
	}
}
