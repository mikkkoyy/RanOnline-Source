// VehicleEditPageInven.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "VehicleEditPageInven.h"

#include "VehicleEditTab.h"
#include "GLVEHICLE.h"
#include "GLItemMan.h"
#include "HLibDataConvert.h"
#include "DlgSItemCustomEdit.h"

// CVehicleEditPageInven dialog

IMPLEMENT_DYNAMIC(CVehicleEditPageInven, CPropertyPage)

CVehicleEditPageInven::CVehicleEditPageInven(LOGFONT logfont, GLVEHICLE* pData)
	: CPropertyPage(CVehicleEditPageInven::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CVehicleEditPageInven::~CVehicleEditPageInven()
{
	SAFE_DELETE ( m_pFont );
}

void CVehicleEditPageInven::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_VEHICLEINVEN, m_List);
}


BEGIN_MESSAGE_MAP(CVehicleEditPageInven, CPropertyPage)
	ON_BN_CLICKED(IDC_VEHICLEEDITINVEN_BUTTON_EDIT, OnBnClickedVehicleeditinvenButtonEdit)
	ON_BN_CLICKED(IDC_VEHICLEEDITINVEN_BUTTON_DELETE, OnBnClickedVehicleeditinvenButtonDelete)
	ON_BN_CLICKED(IDC_VEHICLEEDITINVEN_BUTTON_CLEAR, OnBnClickedVehicleeditinvenButtonClear)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_VEHICLEINVEN, OnNMDblclkListVehicleinven)
END_MESSAGE_MAP()


// CVehicleEditPageInven message handlers
BOOL CVehicleEditPageInven::OnInitDialog()
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

void CVehicleEditPageInven::ShowData()
{
	m_List.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_List.SetRedraw( FALSE );

	for ( int i=0; i<VEHICLE_ACCETYPE_SIZE; ++i )
	{
		const SITEMCUSTOM sITEMCUSTOM = m_pData->m_PutOnItems[i];

		std::string strNAME = "Unknown Item";
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( sITEMCUSTOM.sNativeID );
		if ( pITEM ){
			strNAME = pITEM->GetName();
		}

		m_List.InsertItem( i, "" );
		m_List.SetItemText( i, 1, _HLIB::cstringformat( "[%02d] %s", i, COMMENT::ITEMSLOT_VEHICLE[i].c_str() ).GetString() );
		m_List.SetItemText( i, 2, _HLIB::cstringformat( "[%03d/%03d] %s", sITEMCUSTOM.sNativeID.wMainID, sITEMCUSTOM.sNativeID.wSubID, strNAME.c_str() ).GetString() );
		m_List.SetItemData ( i, i );
	}

	m_List.SetRedraw( TRUE );
}

void CVehicleEditPageInven::OnBnClickedVehicleeditinvenButtonEdit()
{
	EditItem();
}

void CVehicleEditPageInven::OnBnClickedVehicleeditinvenButtonDelete()
{
	if ( !m_pData )	{
		return;
	}

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 ){
		return;
	}

	WORD wSlot = (WORD) m_List.GetItemData ( nSelect );
	if ( wSlot >= VEHICLE_ACCETYPE_SIZE ){
		return;
	}

	m_pData->m_PutOnItems[wSlot] = SITEMCUSTOM( SNATIVEID(false) );
	ShowData();
}

void CVehicleEditPageInven::OnBnClickedVehicleeditinvenButtonClear()
{
	if ( !m_pData ){
		return;
	}

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All PutOn Items?" ) == IDYES ){
		for ( int i=0; i<VEHICLE_ACCETYPE_SIZE; ++i ){
			m_pData->m_PutOnItems[i] = SITEMCUSTOM ( SNATIVEID(false) );
		}

		ShowData();
	}
}

void CVehicleEditPageInven::OnNMDblclkListVehicleinven(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	EditItem();
}

void CVehicleEditPageInven::EditItem()
{
	if ( !m_pData )	{
		return;
	}

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 ){
		return;
	}

	WORD wSlot = (WORD) m_List.GetItemData ( nSelect );
	if ( wSlot >= VEHICLE_ACCETYPE_SIZE ){
		return;
	}

	CDlgSItemCustomEdit dlgItemEdit( 
		&m_pData->m_PutOnItems[wSlot], 
		EMITEMEDIT_TYPE_PUTON_VEHICLE,
		wSlot,
		-1,
		-1,
		this );

	if ( dlgItemEdit.DoModal() == IDOK ){
		ShowData();
	}
}