// CharEditPagePutOn.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "CharEditPagePutOn.h"


#include "CharEditTab.h"
#include "EtcFunction.h"
#include "GLCharData.h"
#include "GLItemMan.h"
#include "HLibDataConvert.h"

#include "Logic/GMToolGlobal.h"
#include "DlgSItemCustomEdit.h"
// CCharEditPagePutOn dialog

IMPLEMENT_DYNAMIC(CCharEditPagePutOn, CPropertyPage)

CCharEditPagePutOn::CCharEditPagePutOn(LOGFONT logfont, SCHARDATA2* pData)
	: CPropertyPage(CCharEditPagePutOn::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CCharEditPagePutOn::~CCharEditPagePutOn()
{
	SAFE_DELETE ( m_pFont );
}

void CCharEditPagePutOn::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHAREDIT_PUTON_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CCharEditPagePutOn, CPropertyPage)
	ON_BN_CLICKED(IDC_CHAREDIT_PUTON_BUTTON_EDIT, OnBnClickedChareditPutonButtonEdit)
	ON_BN_CLICKED(IDC_CHAREDIT_PUTON_BUTTON_DELETE, OnBnClickedChareditPutonButtonDelete)
	ON_BN_CLICKED(IDC_CHAREDIT_PUTON_BUTTON_CLEAR, OnBnClickedChareditPutonButtonClear)
	ON_NOTIFY(NM_DBLCLK, IDC_CHAREDIT_PUTON_LIST, OnNMDblclkChareditPutonList)
END_MESSAGE_MAP()


// CCharEditPagePutOn message handlers
BOOL CCharEditPagePutOn::OnInitDialog()
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

void CCharEditPagePutOn::DataShow()
{
	m_List.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_List.SetRedraw( FALSE );

	for ( int i=0; i<SLOT_NSIZE_S_2; ++i )
	{
		const SITEMCUSTOM sITEMCUSTOM = m_pData->m_PutOnItems[i];

		std::string strNAME = "Unknown Item";
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( sITEMCUSTOM.sNativeID );
		if ( pITEM ){
			strNAME = pITEM->GetName();
		}

		m_List.InsertItem( i, "" );
		m_List.SetItemText( i, 1, _HLIB::cstringformat( "[%02d] %s", i, COMMENT::ITEMSLOT_CHAR[i].c_str() ).GetString() );
		m_List.SetItemText( i, 2, _HLIB::cstringformat( "[%03d/%03d] %s", sITEMCUSTOM.sNativeID.wMainID, sITEMCUSTOM.sNativeID.wSubID, strNAME.c_str() ).GetString() );
		m_List.SetItemData ( i, i );
	}

	m_List.SetRedraw( TRUE );
}

void CCharEditPagePutOn::DataEdit()
{
	if ( !m_pData )	{
		return;
	}

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 ){
		return;
	}

	WORD wSlot = (WORD) m_List.GetItemData ( nSelect );
	if ( wSlot >= SLOT_NSIZE_S_2 ){
		return;
	}

	CDlgSItemCustomEdit dlgItemEdit( 
		&m_pData->m_PutOnItems[wSlot], 
		EMITEMEDIT_TYPE_PUTON_CHAR,
		wSlot,
		-1,
		-1,
		this );

	if ( dlgItemEdit.DoModal() == IDOK ){
		DataShow();

		for( int nItem=0; nItem<m_List.GetItemCount(); nItem++){
			WORD wID = (WORD)m_List.GetItemData( nItem );
			if( wID == wSlot ){
				m_List.EnsureVisible( nItem, TRUE );
				m_List.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
				GotoDlgCtrl(GetDlgItem(m_List.GetDlgCtrlID())); 
			}
		}
	}
}

void CCharEditPagePutOn::OnBnClickedChareditPutonButtonEdit()
{
	DataEdit();
}

void CCharEditPagePutOn::OnBnClickedChareditPutonButtonDelete()
{
	if ( !m_pData )	{
		return;
	}

	int nCHECKED = 0;
	for( int nItem=0; nItem<m_List.GetItemCount(); nItem++ ){
		BOOL bChecked = m_List.GetCheck( nItem );
		if( bChecked){
			DWORD dwID = (DWORD) m_List.GetItemData ( nItem );
			if ( dwID >= SLOT_NSIZE_S_2 )	continue;
			m_pData->m_PutOnItems[dwID] = SITEMCUSTOM( SNATIVEID(false) );
			nCHECKED ++;
		}
	}

	if ( nCHECKED == 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected Use Check boxes to select" );
	}

	DataShow();
}

void CCharEditPagePutOn::OnBnClickedChareditPutonButtonClear()
{
	if ( !m_pData ){
		return;
	}

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All PutOn Items?" ) == IDYES ){
		for ( int i=0; i<SLOT_NSIZE_S_2; ++i ){
			m_pData->m_PutOnItems[i] = SITEMCUSTOM ( SNATIVEID(false) );
		}

		DataShow();
	}
}

void CCharEditPagePutOn::OnNMDblclkChareditPutonList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	DataEdit();
}
