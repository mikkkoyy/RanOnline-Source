// LockerPageItem2.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "LockerPageItem2.h"

#include "LockerTab.h"
#include "GLCharData.h"

#include "EtcFunction.h"
#include "HLibDataConvert.h"
#include "GLItemMan.h"
#include "Logic/GMToolGlobal.h"
#include "DlgSItemCustomEdit.h"
#include "DlgItem.h"

// CLockerPageItem2 dialog

IMPLEMENT_DYNAMIC(CLockerPageItem2, CPropertyPage)

CLockerPageItem2::CLockerPageItem2(LOGFONT logfont, SCHARDATA2* pData)
	: CPropertyPage(CLockerPageItem2::IDD)
	, m_pFont( NULL )
	, m_pData( NULL )
	, m_nLockerChannel(-1)
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
	m_nLockerChannel = 1;
}

CLockerPageItem2::~CLockerPageItem2()
{
	SAFE_DELETE ( m_pFont );
}

void CLockerPageItem2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOCKER_PAGE_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CLockerPageItem2, CPropertyPage)
	ON_BN_CLICKED(IDC_LOCKER_PAGE_BUTTON_ADD, OnBnClickedLockerPageButtonAdd)
	ON_BN_CLICKED(IDC_LOCKER_PAGE_BUTTON_EDIT, OnBnClickedLockerPageButtonEdit)
	ON_BN_CLICKED(IDC_LOCKER_PAGE_BUTTON_DELETE, OnBnClickedLockerPageButtonDelete)
	ON_BN_CLICKED(IDC_LOCKER_PAGE_BUTTON_CLEAR, OnBnClickedLockerPageButtonClear)
	ON_NOTIFY(NM_DBLCLK, IDC_LOCKER_PAGE_LIST, OnNMDblclkLockerPageList)
END_MESSAGE_MAP()


// CLockerPageItem2 message handlers
BOOL CLockerPageItem2::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "", LVCFMT_LEFT, ( 5* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Pos[X/Y]", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 2, "Num", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 3, "Item", LVCFMT_LEFT, ( 60* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	SetWin_Text( this, IDC_LOCKER_PAGE_EDIT_TOTAL, "" );

	ShowData();

	return TRUE;  
}

void CLockerPageItem2::ShowData()
{
	m_List.DeleteAllItems();

	if( !m_pData ){
		return;
	}

	if ( m_nLockerChannel < 0 || m_nLockerChannel >= EMSTORAGE_CHANNEL ){
		return;
	}

	m_List.SetRedraw( FALSE );

	GLInventory::CELL_MAP *pItemList = m_pData->m_cStorage[m_nLockerChannel].GetItemList();
	GLInventory::CELL_MAP_ITER iter_start = pItemList->begin();
	for ( int i=0; iter_start != pItemList->end(); ++iter_start, ++i )
	{
		const SINVENITEM* pInvenItem = (*iter_start).second;

		std::string strNAME = "Unknown Item";
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( pInvenItem->sItemCustom.sNativeID );
		if ( pITEM ){
			strNAME = pITEM->GetName();
		}

		m_List.InsertItem( i, "" );
		m_List.SetItemText( i, 1, _HLIB::cstringformat( "[%02d/%02d]", pInvenItem->wPosX, pInvenItem->wPosY ).GetString() );
		m_List.SetItemText( i, 2, _HLIB::cstring_dword( pInvenItem->sItemCustom.wTurnNum ).GetString() );
		m_List.SetItemText( i, 3, _HLIB::cstringformat( "[%03d/%03d] %s", pInvenItem->sItemCustom.sNativeID.wMainID, pInvenItem->sItemCustom.sNativeID.wSubID, strNAME.c_str() ).GetString() );
		SNATIVEID sPOS( pInvenItem->wPosX, pInvenItem->wPosY );
		m_List.SetItemData ( i, sPOS.dwID );
	}

	m_List.SetRedraw( TRUE );

	CString strTotal;
	strTotal.Format( "Total Items:%d", m_pData->m_cStorage[m_nLockerChannel].GetNumItems() );
	SetWin_Text( this, IDC_LOCKER_PAGE_EDIT_TOTAL, strTotal.GetString() );
}

void CLockerPageItem2::EditItem()
{
	if( !m_pData ){
		return;
	}

	if ( m_nLockerChannel < 0 || m_nLockerChannel >= EMSTORAGE_CHANNEL ){
		return;
	}

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ){
		return;
	}

	DWORD dwID = (DWORD) m_List.GetItemData ( nSelect );
	SNATIVEID sPos( dwID );

	SINVENITEM* pINVENITEM = m_pData->m_cStorage[m_nLockerChannel].FindPosItem( sPos.wMainID, sPos.wSubID );
	if ( pINVENITEM ){
		CDlgSItemCustomEdit dlgItemEdit( 
			&pINVENITEM->sItemCustom, 
			EMITEMEDIT_TYPE_INVEN_ITEM_EDIT,
			SLOT_NSIZE,
			pINVENITEM->wPosX,
			pINVENITEM->wPosY,
			this );

		if ( dlgItemEdit.DoModal() == IDOK ){
			ShowData();

			for( int nItem=0; nItem<m_List.GetItemCount(); nItem++){
				DWORD dwID = m_List.GetItemData( nItem );
				if( dwID == SNATIVEID( pINVENITEM->wPosX, pINVENITEM->wPosY ).dwID ){
					m_List.EnsureVisible( nItem, TRUE );
					m_List.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
					GotoDlgCtrl(GetDlgItem(m_List.GetDlgCtrlID())); 
				}
			}
		}
	}
}

void CLockerPageItem2::OnBnClickedLockerPageButtonAdd()
{
	if ( !m_pData ){
		return;
	}

	if ( m_nLockerChannel < 0 || m_nLockerChannel >= EMSTORAGE_CHANNEL ){
		return;
	}

	SINVENITEM sINVENITEM;
	sINVENITEM.sItemCustom.tBORNTIME = CTime::GetCurrentTime().GetTime();
	sINVENITEM.sItemCustom.wTurnNum = 1;
	sINVENITEM.sItemCustom.cGenType = EMGEN_DEFAULT;
	sINVENITEM.sItemCustom.cChnID = 0;
	sINVENITEM.sItemCustom.cFieldID = 0;
	sINVENITEM.sItemCustom.lnGenNum = 0;

	CDlgItem dlg( this );
	if ( dlg.DoModal() != IDOK ){
		return;
	}

	sINVENITEM.sItemCustom.sNativeID.wMainID = dlg.m_SELECTED.wMainID;
	sINVENITEM.sItemCustom.sNativeID.wSubID = dlg.m_SELECTED.wSubID;

	/*item color, Juver, 2018/01/08 */
	SITEM* pitem_data_check = GLItemMan::GetInstance().GetItem( sINVENITEM.sItemCustom.sNativeID );
	if ( pitem_data_check && pitem_data_check->sBasicOp.bItemColor )
	{
		sINVENITEM.sItemCustom.wColor1 = pitem_data_check->sBasicOp.wItemColor1;
		sINVENITEM.sItemCustom.wColor2 = pitem_data_check->sBasicOp.wItemColor2;
	}

	SITEM* pITEM = GLItemMan::GetInstance().GetItem ( sINVENITEM.sItemCustom.sNativeID );
	if ( !pITEM ) {
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Item! ( %d %d )", 
			sINVENITEM.sItemCustom.sNativeID.wMainID,
			sINVENITEM.sItemCustom.sNativeID.wSubID );
		return;
	}

	WORD wPosX(0), wPosY(0);
	BOOL bOk = m_pData->m_cStorage[m_nLockerChannel].FindInsrtable ( pITEM->sBasicOp.wInvenSizeX, pITEM->sBasicOp.wInvenSizeY, wPosX, wPosY );
	if ( !bOk ){
		CDebugSet::MsgBox( GetSafeHwnd(), "No space to insert item!" );
		return;
	}

	BOOL bINSERT = m_pData->m_cStorage[m_nLockerChannel].InsertItem( sINVENITEM.sItemCustom, wPosX, wPosY );
	if ( !bINSERT ){
		CDebugSet::MsgBox( "Unable to insert item! Report this bug!" );
		return;
	}

	SINVENITEM* pInvenItem = m_pData->m_cStorage[m_nLockerChannel].FindPosItem( wPosX, wPosY );
	if ( pInvenItem && pInvenItem->sItemCustom.sNativeID == sINVENITEM.sItemCustom.sNativeID ){
		if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Edit Inserted Item?") == IDYES ){
			CDlgSItemCustomEdit dlgItemEdit( 
				&pInvenItem->sItemCustom, 
				EMITEMEDIT_TYPE_INVEN_ITEM_EDIT,
				SLOT_NSIZE,
				pInvenItem->wPosX,
				pInvenItem->wPosY,
				this );

			if ( dlgItemEdit.DoModal() == IDOK ){
			}
		}
	}

	ShowData();

	for( int nItem=0; nItem<m_List.GetItemCount(); nItem++){
		DWORD dwID = m_List.GetItemData( nItem );
		if( dwID == SNATIVEID( wPosX, wPosY ).dwID ){
			m_List.EnsureVisible( nItem, TRUE );
			m_List.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
			GotoDlgCtrl(GetDlgItem(m_List.GetDlgCtrlID())); 
		}
	}
}

void CLockerPageItem2::OnBnClickedLockerPageButtonEdit()
{
	EditItem();
}

void CLockerPageItem2::OnBnClickedLockerPageButtonDelete()
{
	if ( !m_pData ){
		return;
	}

	if ( m_nLockerChannel < 0 || m_nLockerChannel >= EMSTORAGE_CHANNEL ){
		return;
	}

	int nCHECKED = 0;
	for( int nItem=0; nItem<m_List.GetItemCount(); nItem++ ){
		BOOL bChecked = m_List.GetCheck( nItem );
		if( bChecked){
			DWORD dwID = (DWORD) m_List.GetItemData ( nItem );
			SNATIVEID sPos( dwID );
			m_pData->m_cStorage[m_nLockerChannel].DeleteItem( sPos.wMainID, sPos.wSubID );
			nCHECKED ++;
		}
	}

	if ( nCHECKED == 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected Use Check boxes to select" );
	}

	ShowData();
}

void CLockerPageItem2::OnBnClickedLockerPageButtonClear()
{
	if ( !m_pData ){
		return;
	}

	if ( m_nLockerChannel < 0 || m_nLockerChannel >= EMSTORAGE_CHANNEL ){
		return;
	}

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All Items on this locker page?" ) == IDYES )
	{
		m_pData->m_cStorage[m_nLockerChannel].DeleteItemAll();
		ShowData();
	}
}

void CLockerPageItem2::OnNMDblclkLockerPageList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	EditItem();
}
