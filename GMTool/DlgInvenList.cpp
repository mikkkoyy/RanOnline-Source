// DlgInvenList.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgInvenList.h"

#include "EtcFunction.h"
#include "HLibDataConvert.h"
#include "Logic/GMToolGlobal.h"
#include "GLItemMan.h"

// CDlgInvenList dialog

IMPLEMENT_DYNAMIC(CDlgInvenList, CDialog)

CDlgInvenList::CDlgInvenList(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgInvenList::IDD, pParent)
	, m_pData(NULL)
{

}

CDlgInvenList::CDlgInvenList(SCHARDATA2* pData, CWnd* pParent /*= NULL*/)
	: CDialog(CDlgInvenList::IDD, pParent)
	, m_pData(NULL)
{
	m_pData = pData;
}


CDlgInvenList::~CDlgInvenList()
{
}

void CDlgInvenList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INVEN_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CDlgInvenList, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnBnClickedButtonSearch)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_INVEN_LIST, OnLvnItemchangedInvenList)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgInvenList message handlers
BOOL CDlgInvenList::OnInitDialog()
{
	CDialog::OnInitDialog();

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "", LVCFMT_LEFT, ( 5* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Pos[X/Y]", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 2, "Num", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 3, "Item", LVCFMT_LEFT, ( 60* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	ReadData();
	SelectionReset();

	SetWin_Combo_Init( this, IDC_COMBO_SEARCH, GMTOOL_GLOBAL::strItemDataSearchType, EMITEMDATA_SEARCH_SIZE );
	SetWin_Text( this, IDC_EDIT_SEARCH, "" );

	SearchData();

	return TRUE;  
}

void CDlgInvenList::ReadData()
{
	m_vecData.clear();

	if ( !m_pData ){
		return;
	}

	GLInventory::CELL_MAP *pItemList = m_pData->m_cInventory.GetItemList();
	GLInventory::CELL_MAP_ITER iter_start = pItemList->begin();
	GLInventory::CELL_MAP_ITER iter_end = pItemList->end();

	for ( int i=0; iter_start != iter_end; ++iter_start, ++i )
	{
		const SINVENITEM* pInvenItem = (*iter_start).second;
		m_vecData.push_back( *pInvenItem );
	}
}

void CDlgInvenList::ShowData()
{
	m_List.DeleteAllItems();
	m_List.SetRedraw( FALSE );

	for ( int i=0; i<(int)m_vecResult.size(); i++ ){
		SINVENITEM sData = m_vecResult[i];

		SITEM* pITEM = GLItemMan::GetInstance().GetItem( sData.sItemCustom.sNativeID );
		if ( !pITEM ){
			continue;
		}

		m_List.InsertItem( i, "" );
		m_List.SetItemText( i, 1, _HLIB::cstringformat( "[%02d/%02d]", sData.wPosX, sData.wPosY ).GetString() );
		m_List.SetItemText( i, 2, _HLIB::cstring_dword( sData.sItemCustom.wTurnNum ).GetString() );
		m_List.SetItemText( i, 3, _HLIB::cstringformat( "[%03d/%03d] %s", sData.sItemCustom.sNativeID.wMainID, sData.sItemCustom.sNativeID.wSubID, pITEM->GetName() ).GetString() );
		SNATIVEID sPOS( sData.wPosX, sData.wPosY );
		m_List.SetItemData ( i, sPOS.dwID );
	}

	m_List.SetRedraw( TRUE );
}

void CDlgInvenList::SelectionReset()
{
	m_SELECTED = SINVENITEM();
	SetWin_Text( this, IDC_EDIT_SEL_NAME, "" );
}

void CDlgInvenList::SearchData()
{
	EMITEMDATA_SEARCH emSearch =  static_cast<EMITEMDATA_SEARCH> ( GetWin_Combo_Sel( this, IDC_COMBO_SEARCH ) );
	std::string strSEARCH = GetWin_Text( this, IDC_EDIT_SEARCH );

	m_vecResult.clear();

	if ( strSEARCH.size() <= 0 )
	{
		m_vecResult = m_vecData;
	}
	else
	{
		for( int i=0; i<(int)m_vecData.size(); ++ i )
		{
			SINVENITEM sDATA = m_vecData[i];
			SITEM* pItem = GLItemMan::GetInstance().GetItem ( sDATA.sItemCustom.sNativeID );
			if ( !pItem )	continue;

			CString strFIND;
			switch( emSearch )
			{
			case EMITEMDATA_SEARCH_NAME:
				{
					strFIND.Format( "%s", pItem->GetName() );
				}break;
			case EMITEMDATA_SEARCH_MID:
				{
					strFIND.Format( "%d", pItem->sBasicOp.sNativeID.wMainID );
				}break;
			case EMITEMDATA_SEARCH_SID:
				{
					strFIND.Format( "%d", pItem->sBasicOp.sNativeID.wSubID );
				}break;
			case EMITEMDATA_SEARCH_TYPE:
				{
					strFIND.Format( "%d", pItem->sBasicOp.emItemType );
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

void CDlgInvenList::OnBnClickedButtonSearch()
{
	SearchData();
}

void CDlgInvenList::OnLvnItemchangedInvenList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	SelectionReset();

	if ( !m_pData ){
		return;
	}

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = (DWORD) m_List.GetItemData ( nSelect );

	SNATIVEID sPOS( dwID );
	SINVENITEM* pInvenItem = m_pData->m_cInventory.FindPosItem( sPOS.wMainID, sPOS.wSubID );
	if ( pInvenItem ){

		SITEM* pITEM = GLItemMan::GetInstance().GetItem( pInvenItem->sItemCustom.sNativeID );
		if ( pITEM ){
			m_SELECTED = *pInvenItem;

			CString strNAME;
			strNAME.Format ( "[%03d/%03d] %s", m_SELECTED.sItemCustom.sNativeID.wMainID, m_SELECTED.sItemCustom.sNativeID.wSubID, pITEM->GetName() );
			SetWin_Text( this, IDC_EDIT_SEL_NAME, strNAME.GetString() );
		}
	}
}

void CDlgInvenList::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}

void CDlgInvenList::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}
