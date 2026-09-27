// VehicleEditPageColor.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "VehicleEditPageColor.h"
#include "VehicleEditTab.h"
#include "GLVEHICLE.h"
#include "HLibDataConvert.h"
#include "HLibColorValue.h"
#include "SelColorDlg.h"

// CVehicleEditPageColor dialog

IMPLEMENT_DYNAMIC(CVehicleEditPageColor, CPropertyPage)

CVehicleEditPageColor::CVehicleEditPageColor(LOGFONT logfont, GLVEHICLE* pData)
	: CPropertyPage(CVehicleEditPageColor::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CVehicleEditPageColor::~CVehicleEditPageColor()
{
	SAFE_DELETE ( m_pFont );
}

void CVehicleEditPageColor::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_VEHICLE_COLOR, m_List);
}


BEGIN_MESSAGE_MAP(CVehicleEditPageColor, CPropertyPage)
	ON_BN_CLICKED(IDC_VEHICLEEDIT_COLOR_BUTTON_EDIT, OnBnClickedVehicleeditColorButtonEdit)
	ON_BN_CLICKED(IDC_VEHICLEEDIT_COLOR_BUTTON_DELETE, OnBnClickedVehicleeditColorButtonDelete)
	ON_BN_CLICKED(IDC_VEHICLEEDIT_COLOR_BUTTON_CLEAR, OnBnClickedVehicleeditColorButtonClear)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_VEHICLE_COLOR, OnNMDblclkListVehicleColor)
END_MESSAGE_MAP()


// CVehicleEditPageColor message handlers
BOOL CVehicleEditPageColor::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "", LVCFMT_LEFT, ( 5* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Slot", LVCFMT_LEFT, ( 40* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 3, "Color", LVCFMT_LEFT, ( 55* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	ShowData();

	return TRUE;  
}

void CVehicleEditPageColor::ShowData()
{
	m_List.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_List.SetRedraw( FALSE );

	for ( int i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
	{
		const WORD wColor = m_pData->m_wColor[i];

		m_List.InsertItem( i, "" );
		m_List.SetItemText( i, 1, _HLIB::cstringformat( "[%02d] %s", i, COMMENT::BIKE_COLOR_SLOT[i].c_str() ).GetString() );
		m_List.SetItemText( i, 2, _HLIB::cstringformat( "[%03d]", wColor).GetString() );
		m_List.SetItemData ( i, i );
	}

	m_List.SetRedraw( TRUE );
}

void CVehicleEditPageColor::EditColor()
{
	if ( !m_pData )	return;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 )	return;

	WORD wSlot = (WORD) m_List.GetItemData ( nSelect );
	if ( wSlot >= BIKE_COLOR_SLOT_PART_SIZE )		return;


	DWORD dwColor = _HLIB::color_dword_word( m_pData->m_wColor[wSlot] );

	CSelColorDlg dlg( BYTE( _HLIB::color_get_int_r( dwColor) ), 
		BYTE( _HLIB::color_get_int_g( dwColor ) ), 
		BYTE( _HLIB::color_get_int_b( dwColor ) ), 
		this );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		m_pData->m_wColor[wSlot] = _HLIB::color_word_dword( _HLIB::color_argb_rgb(crNewColor) );

		ShowData();
	}
}

void CVehicleEditPageColor::OnBnClickedVehicleeditColorButtonEdit()
{
	EditColor();
}

void CVehicleEditPageColor::OnBnClickedVehicleeditColorButtonDelete()
{
	if ( !m_pData )	return;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 )	return;

	WORD wSlot = (WORD) m_List.GetItemData ( nSelect );
	if ( wSlot >= BIKE_COLOR_SLOT_PART_SIZE )		return;

	m_pData->m_wColor[wSlot] = 0;

	ShowData();
}

void CVehicleEditPageColor::OnBnClickedVehicleeditColorButtonClear()
{
	if ( !m_pData )	return;

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Reset All Colors?" ) == IDYES )
	{
		for ( int i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
			m_pData->m_wColor[i] = 0;

		ShowData();
	}
}

void CVehicleEditPageColor::OnNMDblclkListVehicleColor(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	EditColor();
}
