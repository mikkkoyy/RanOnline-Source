// PetEditPageInven.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "PetEditPageInven.h"

#include "PetEditTab.h"
#include "GLPet.h"
#include "GLItemMan.h"
#include "HLibDataConvert.h"
#include "DlgSItemCustomEdit.h"

// CPetEditPageInven dialog

IMPLEMENT_DYNAMIC(CPetEditPageInven, CPropertyPage)

CPetEditPageInven::CPetEditPageInven(LOGFONT logfont, GLPET* pData)
	: CPropertyPage(CPetEditPageInven::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CPetEditPageInven::~CPetEditPageInven()
{
	SAFE_DELETE ( m_pFont );
}

void CPetEditPageInven::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PETINVEN, m_List);
}


BEGIN_MESSAGE_MAP(CPetEditPageInven, CPropertyPage)
	ON_BN_CLICKED(IDC_PETEDITINVEN_BUTTON_EDIT, OnBnClickedPeteditinvenButtonEdit)
	ON_BN_CLICKED(IDC_PETEDITINVEN_BUTTON_DELETE, OnBnClickedPeteditinvenButtonDelete)
	ON_BN_CLICKED(IDC_PETEDITINVEN_BUTTON_CLEAR, OnBnClickedPeteditinvenButtonClear)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PETINVEN, OnNMDblclkListPetinven)
END_MESSAGE_MAP()


// CPetEditPageInven message handlers
BOOL CPetEditPageInven::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "", LVCFMT_LEFT, ( 5* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Slot", LVCFMT_LEFT, ( 40* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 3, "Item", LVCFMT_LEFT, ( 55* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	ShowData();

	return TRUE;  
}

void CPetEditPageInven::ShowData()
{
	m_List.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_List.SetRedraw( FALSE );

	for ( int i=0; i<PET_ACCETYPE_SIZE; ++i )
	{
		const SITEMCUSTOM sITEMCUSTOM = m_pData->m_PutOnItems[i];

		std::string strNAME = "Unknown Item";
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( sITEMCUSTOM.sNativeID );
		if ( pITEM ){
			strNAME = pITEM->GetName();
		}

		m_List.InsertItem( i, "" );
		m_List.SetItemText( i, 1, _HLIB::cstringformat( "[%02d] %s", i, COMMENT::ITEMSLOT_PET[i].c_str() ).GetString() );
		m_List.SetItemText( i, 2, _HLIB::cstringformat( "[%03d/%03d] %s", sITEMCUSTOM.sNativeID.wMainID, sITEMCUSTOM.sNativeID.wSubID, strNAME.c_str() ).GetString() );
		m_List.SetItemData ( i, i );
	}

	m_List.SetRedraw( TRUE );
}

void CPetEditPageInven::OnBnClickedPeteditinvenButtonEdit()
{
	EditItem();
}

void CPetEditPageInven::OnBnClickedPeteditinvenButtonDelete()
{
	if ( !m_pData )	{
		return;
	}

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 ){
		return;
	}

	WORD wSlot = (WORD) m_List.GetItemData ( nSelect );
	if ( wSlot >= PET_ACCETYPE_SIZE ){
		return;
	}

	m_pData->m_PutOnItems[wSlot] = SITEMCUSTOM( SNATIVEID(false) );
	ShowData();
}

void CPetEditPageInven::OnBnClickedPeteditinvenButtonClear()
{
	if ( !m_pData ){
		return;
	}

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All PutOn Items?" ) == IDYES ){
		for ( int i=0; i<PET_ACCETYPE_SIZE; ++i ){
			m_pData->m_PutOnItems[i] = SITEMCUSTOM ( SNATIVEID(false) );
		}

		ShowData();
	}
}

void CPetEditPageInven::OnNMDblclkListPetinven(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	EditItem();
}

void CPetEditPageInven::EditItem()
{
	if ( !m_pData )	{
		return;
	}

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 ){
		return;
	}

	WORD wSlot = (WORD) m_List.GetItemData ( nSelect );
	if ( wSlot >= PET_ACCETYPE_SIZE ){
		return;
	}

	CDlgSItemCustomEdit dlgItemEdit( 
		&m_pData->m_PutOnItems[wSlot], 
		EMITEMEDIT_TYPE_PUTON_PET,
		wSlot,
		-1,
		-1,
		this );

	if ( dlgItemEdit.DoModal() == IDOK ){
		ShowData();
	}
}