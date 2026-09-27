// VehicleEditPageBasic.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "VehicleEditPageBasic.h"

#include "VehicleEditTab.h"
#include "GLVEHICLE.h"
#include "EtcFunction.h"

// CVehicleEditPageBasic dialog

IMPLEMENT_DYNAMIC(CVehicleEditPageBasic, CPropertyPage)

CVehicleEditPageBasic::CVehicleEditPageBasic(LOGFONT logfont, GLVEHICLE* pData)
	: CPropertyPage(CVehicleEditPageBasic::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CVehicleEditPageBasic::~CVehicleEditPageBasic()
{
	SAFE_DELETE ( m_pFont );
}

void CVehicleEditPageBasic::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VEHICLEEDITBASIC_SLIDER_VEHICLEFULL, m_slider_Battery );
}


BEGIN_MESSAGE_MAP(CVehicleEditPageBasic, CPropertyPage)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_VEHICLEEDITBASIC_SLIDER_VEHICLEFULL, OnNMCustomdrawVehicleeditbasicSliderVehiclefull)
END_MESSAGE_MAP()


// CVehicleEditPageBasic message handlers
BOOL CVehicleEditPageBasic::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	SetWin_Combo_Init( this, IDC_VEHICLEEDITBASIC_COMBO_VEHICLETYPE, COMMENT::VEHICLE_TYPE, VEHICLE_TYPE_SIZE );
	m_slider_Battery.SetRange(0, 1000, TRUE);

	DataDefault();
	DataShow();

	return TRUE;  
}

void CVehicleEditPageBasic::DataDefault()
{
	SetWin_Combo_Sel( this, IDC_VEHICLEEDITBASIC_COMBO_VEHICLETYPE, 0 );
	m_slider_Battery.SetPos(0);
	SetWin_Num_int ( this, IDC_VEHICLEEDITBASIC_EDIT_VEHICLEFULL, 0 );

	/*vehicle booster system, Juver, 2017/08/10 */
	SetWin_Check( this, IDC_VEHICLEEDITBASIC_CHECK_BOOSTER, FALSE );
}

void CVehicleEditPageBasic::DataShow()
{
	if ( !m_pData ){
		return;
	}

	SetWin_Combo_Sel( this, IDC_VEHICLEEDITBASIC_COMBO_VEHICLETYPE, COMMENT::VEHICLE_TYPE[m_pData->m_emTYPE].c_str() );
	m_slider_Battery.SetPos(m_pData->m_nFull);

	/*vehicle booster system, Juver, 2017/08/10 */
	SetWin_Check( this, IDC_VEHICLEEDITBASIC_CHECK_BOOSTER, m_pData->m_bBooster );
}

BOOL CVehicleEditPageBasic::DataSave()
{
	if( !m_pData ){
		return FALSE;
	}

	m_pData->m_emTYPE = static_cast<VEHICLE_TYPE> ( GetWin_Combo_Sel( this, IDC_VEHICLEEDITBASIC_COMBO_VEHICLETYPE) );
	m_pData->m_nFull = m_slider_Battery.GetPos();

	/*vehicle booster system, Juver, 2017/08/10 */
	m_pData->m_bBooster = GetWin_Check( this, IDC_VEHICLEEDITBASIC_CHECK_BOOSTER )? true:false;

	return TRUE;
}

void CVehicleEditPageBasic::OnNMCustomdrawVehicleeditbasicSliderVehiclefull(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int nData = m_slider_Battery.GetPos();
	SetWin_Num_int ( this, IDC_VEHICLEEDITBASIC_EDIT_VEHICLEFULL, nData );
}
