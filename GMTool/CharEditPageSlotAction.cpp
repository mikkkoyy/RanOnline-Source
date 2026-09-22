// CharEditPageSlotAction.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "CharEditPageSlotAction.h"


#include "CharEditTab.h"
#include "GLCharData.h"
#include "GLItemMan.h"
#include "DlgInvenList.h"
#include "HLibDataConvert.h"

// CCharEditPageSlotAction dialog

IMPLEMENT_DYNAMIC(CCharEditPageSlotAction, CPropertyPage)

CCharEditPageSlotAction::CCharEditPageSlotAction(LOGFONT logfont, SCHARDATA2* pData)
	: CPropertyPage(CCharEditPageSlotAction::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CCharEditPageSlotAction::~CCharEditPageSlotAction()
{
	SAFE_DELETE ( m_pFont );
}

void CCharEditPageSlotAction::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHAREDIT_ACTIONSLOT_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CCharEditPageSlotAction, CPropertyPage)
	ON_BN_CLICKED(IDC_CHAREDIT_ACTIONSLOT_BUTTON_EDIT, OnBnClickedChareditActionslotButtonEdit)
	ON_BN_CLICKED(IDC_CHAREDIT_ACTIONSLOT_BUTTON_DELETE, OnBnClickedChareditActionslotButtonDelete)
	ON_BN_CLICKED(IDC_CHAREDIT_ACTIONSLOT_BUTTON_CLEAR, OnBnClickedChareditActionslotButtonClear)
	ON_NOTIFY(NM_DBLCLK, IDC_CHAREDIT_ACTIONSLOT_LIST, OnNMDblclkChareditActionslotList)
END_MESSAGE_MAP()


// CCharEditPageSlotAction message handlers
BOOL CCharEditPageSlotAction::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "", LVCFMT_LEFT, ( 5* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Slot", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 2, "Item", LVCFMT_LEFT, ( 70* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	DataShow();

	return TRUE;  
}

void CCharEditPageSlotAction::DataShow()
{
	m_List.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_List.SetRedraw( FALSE );

	for( int i=0; i<EMACTIONQUICK_SIZE; ++i )
	{
		SNATIVEID sID = m_pData->m_sACTIONQUICK[i].sNID;

		std::string strName = "Unknown Item";
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sID );
		if ( pItem ){
			strName = pItem->GetName();
		}

		m_List.InsertItem( i, "" );
		m_List.SetItemText( i, 1, _HLIB::cstringformat( "Slot %02d", i ).GetString() );
		m_List.SetItemText( i, 2, _HLIB::cstringformat( "[%03d/%03d] %s", sID.wMainID, sID.wSubID, strName.c_str() ).GetString() );
		m_List.SetItemData( i, i );
	}

	m_List.SetRedraw( TRUE );
}

void CCharEditPageSlotAction::DataEdit()
{
	if ( !m_pData ){
		return;
	}

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwSLOT = (DWORD) m_List.GetItemData ( nSelect );

	if ( dwSLOT >= EMACTIONQUICK_SIZE ){
		return;
	}

	CDlgInvenList dlg( m_pData, this );
	if( dlg.DoModal() == IDOK ){

		SINVENITEM sINVEN = dlg.m_SELECTED;

		SITEM* pITEM = GLItemMan::GetInstance().GetItem( sINVEN.sItemCustom.sNativeID );
		if ( pITEM ){
			EMITEM_TYPE emTYPE = pITEM->sBasicOp.emItemType;
			if ( emTYPE != ITEM_CURE && 
				emTYPE != ITEM_TELEPORT_CARD &&
				emTYPE != ITEM_RECALL &&
				emTYPE != ITEM_PET_CARD )
			{
				CDebugSet::MsgBox( GetSafeHwnd(), "Selected item cannot be used on slot!" );
				return;
			}
				
			m_pData->m_sACTIONQUICK[dwSLOT].sNID = sINVEN.sItemCustom.sNativeID;
			m_pData->m_sACTIONQUICK[dwSLOT].wACT = EMACT_SLOT_DRUG;

			DataShow();

			for( int nItem=0; nItem<m_List.GetItemCount(); nItem++){
				DWORD dwID = m_List.GetItemData( nItem );
				if( dwID == dwSLOT ){
					m_List.EnsureVisible( nItem, TRUE );
					m_List.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
					GotoDlgCtrl(GetDlgItem(m_List.GetDlgCtrlID())); 
				}
			}
		}
	}
}

void CCharEditPageSlotAction::OnBnClickedChareditActionslotButtonEdit()
{
	DataEdit();
}

void CCharEditPageSlotAction::OnBnClickedChareditActionslotButtonDelete()
{
	if ( !m_pData ){
		return;
	}

	int nCHECKED = 0;
	for( int nItem=0; nItem<m_List.GetItemCount(); nItem++ )
	{
		BOOL bChecked = m_List.GetCheck( nItem );
		if( bChecked){
			DWORD dwID = (DWORD) m_List.GetItemData ( nItem );
			m_pData->m_sACTIONQUICK[dwID].RESET();
			nCHECKED ++;
		}
	}

	if ( nCHECKED == 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected Use Check boxes to select" );
	}

	DataShow();
}

void CCharEditPageSlotAction::OnBnClickedChareditActionslotButtonClear()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All Slot Data?" ) == IDYES ){
		for( int i=0; i<EMACTIONQUICK_SIZE; ++i ){
			m_pData->m_sACTIONQUICK[i].RESET();
		}

		DataShow();
	}
}

void CCharEditPageSlotAction::OnNMDblclkChareditActionslotList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	DataEdit();
}
