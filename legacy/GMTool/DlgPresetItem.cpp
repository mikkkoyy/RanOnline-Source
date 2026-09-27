// DlgPresetItem.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgPresetItem.h"


#include "DlgPresetItemEdit.h"
#include "Logic/GMToolPresetItem.h"
#include "Logic/GMToolData.h"
#include "Logic/GMToolGlobal.h"
#include "HLibDataConvert.h"

// CDlgPresetItem dialog

IMPLEMENT_DYNAMIC(CDlgPresetItem, CDialog)

CDlgPresetItem::CDlgPresetItem(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPresetItem::IDD, pParent)
{

}

CDlgPresetItem::~CDlgPresetItem()
{
}

void CDlgPresetItem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PRESET_ITEM, m_List );
}


BEGIN_MESSAGE_MAP(CDlgPresetItem, CDialog)
	ON_BN_CLICKED(IDC_PRESET_ITEM_BUTTON_NEW, OnBnClickedPresetItemButtonNew)
	ON_BN_CLICKED(IDC_PRESET_ITEM_BUTTON_EDIT, OnBnClickedPresetItemButtonEdit)
	ON_BN_CLICKED(IDC_PRESET_ITEM_BUTTON_DELETE, OnBnClickedPresetItemButtonDelete)
	ON_BN_CLICKED(IDC_PRESET_ITEM_BUTTON_CLEAR, OnBnClickedPresetItemButtonClear)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PRESET_ITEM, OnNMDblclkListPresetItem)
END_MESSAGE_MAP()


// CDlgPresetItem message handlers
BOOL CDlgPresetItem::OnInitDialog()
{
	CDialog::OnInitDialog();

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "ID", LVCFMT_LEFT, ( 3* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Name", LVCFMT_LEFT, ( 14* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 2, "Damage", LVCFMT_LEFT, ( 6* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 3, "Defense", LVCFMT_LEFT, ( 6* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 4, "Fire~Ice~Elect~Poison~Spirit", LVCFMT_LEFT, ( 16* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 5, "RvOpt1(RvVal1)", LVCFMT_LEFT, ( 12* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 6, "RvOpt2(RvVal2", LVCFMT_LEFT, ( 12* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 7, "RvOpt3(RvVal3)", LVCFMT_LEFT, ( 12* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 8, "RvOpt4(RvVal4)", LVCFMT_LEFT, ( 12* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 9, "NDrop", LVCFMT_LEFT, ( 5* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	DataShow();

	return TRUE;  
}

void CDlgPresetItem::DataShow()
{
	m_List.DeleteAllItems();
	m_List.SetRedraw( FALSE );

	SGMTOOL_PRESET_ITEM_MAP mapPreset = CGMToolPresetItem::GetInstance().m_MapPresetItem;
	SGMTOOL_PRESET_ITEM_MAP_ITER iter_start = mapPreset.begin();

	for ( int i=0; iter_start != mapPreset.end(); ++iter_start, ++i ){
		const SGMTOOL_PRESET_ITEM &sPRESET = (*iter_start).second;

		m_List.InsertItem( i, _HLIB::cstring_dword( sPRESET.wID ).GetString() );
		m_List.SetItemText( i, 1, sPRESET.strNAME.c_str() );
		m_List.SetItemText( i, 2, _HLIB::cstring_int( sPRESET.cDAMAGE ).GetString() );
		m_List.SetItemText( i, 3, _HLIB::cstring_int( sPRESET.cDEFENSE ).GetString() );
		m_List.SetItemText( i, 4, _HLIB::cstringformat( "%d~%d~%d~%d~%d", sPRESET.cRESFIRE, sPRESET.cRESICE, sPRESET.cRESELECT, sPRESET.cRESPOISON, sPRESET.cRESSPIRIT ).GetString() );
		m_List.SetItemText( i, 5, _HLIB::cstringformat( "%d(%d)", sPRESET.cOPTIONTYPE1, sPRESET.nOPTIONVAL1 ).GetString() );
		m_List.SetItemText( i, 6, _HLIB::cstringformat( "%d(%d)", sPRESET.cOPTIONTYPE2, sPRESET.nOPTIONVAL2 ).GetString() );
		m_List.SetItemText( i, 7, _HLIB::cstringformat( "%d(%d)", sPRESET.cOPTIONTYPE3, sPRESET.nOPTIONVAL3 ).GetString() );
		m_List.SetItemText( i, 8, _HLIB::cstringformat( "%d(%d)", sPRESET.cOPTIONTYPE4, sPRESET.nOPTIONVAL4 ).GetString() );
		m_List.SetItemText( i, 9, _HLIB::cstringformat( "%d", sPRESET.bNONDROP ).GetString() );

		m_List.SetItemData ( i, sPRESET.wID );
	}

	m_List.SetRedraw( TRUE );
}


void CDlgPresetItem::OnBnClickedPresetItemButtonNew()
{
	WORD wID = CGMToolPresetItem::GetInstance().PresetItemNewID();
	if( wID == MAX_PRESET_ITEM_ID ){
		CDebugSet::MsgBox( GetSafeHwnd(), "No Vacant Preset ID! Max Preset:%d", MAX_PRESET_ITEM_ID );
		return;
	}

	SGMTOOL_PRESET_ITEM sPreset;
	sPreset.wID = wID;

	CDlgPresetItemEdit dlg( this, &sPreset );
	if ( dlg.DoModal() == IDOK ){
		CGMToolPresetItem::GetInstance().PresetItemAdd( sPreset );

		DataShow();
		for( int nItem = 0; nItem < m_List.GetItemCount(); nItem++){
			DWORD dwID = m_List.GetItemData( nItem );
			if( dwID == wID ){
				m_List.EnsureVisible( nItem, TRUE );
				m_List.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
				GotoDlgCtrl(GetDlgItem(m_List.GetDlgCtrlID())); 
			}
		}
	}
}

void CDlgPresetItem::OnBnClickedPresetItemButtonEdit()
{
	DataEdit();
}

void CDlgPresetItem::OnBnClickedPresetItemButtonDelete()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	WORD wID = (WORD) m_List.GetItemData ( nSelect );
	CGMToolPresetItem::GetInstance().PresetItemDel( wID );

	DataShow();

	int nNewSelect = nSelect > 1? nSelect-1:nSelect;
	m_List.EnsureVisible( nNewSelect, TRUE );
	m_List.SetItemState( nNewSelect, LVIS_SELECTED, LVIS_SELECTED );
	GotoDlgCtrl(GetDlgItem(m_List.GetDlgCtrlID())); 
}

void CDlgPresetItem::OnBnClickedPresetItemButtonClear()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All Item Presets?" ) == IDYES ){
		CGMToolPresetItem::GetInstance().PresetItemDelAll();
		DataShow();
	}
}

void CDlgPresetItem::OnNMDblclkListPresetItem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	DataEdit();
}

void CDlgPresetItem::DataEdit()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	WORD wID = (WORD) m_List.GetItemData ( nSelect );

	SGMTOOL_PRESET_ITEM* pPreset = CGMToolPresetItem::GetInstance().PresetItemGet( wID );
	if ( pPreset ){
		CDlgPresetItemEdit dlg( this, pPreset );
		if ( dlg.DoModal() == IDOK ){
			DataShow();

			for( int nItem = 0; nItem < m_List.GetItemCount(); nItem++){
				DWORD dwID = m_List.GetItemData( nItem );
				if( dwID == wID ){
					m_List.EnsureVisible( nItem, TRUE );
					m_List.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
					GotoDlgCtrl(GetDlgItem(m_List.GetDlgCtrlID())); 
				}
			}
		}
	}
}
