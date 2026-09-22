// DlgSItemCustomEdit.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgSItemCustomEdit.h"

#include "EtcFunction.h"
#include "HLibTimeFunctions.h"
#include "HLibDataConvert.h"
#include "GLItem.h"
#include "GLItemMan.h"
#include "Logic/GMToolData.h"
#include "Logic/GMToolPresetItem.h"
#include "Logic/GMToolGlobal.h"
#include "Logic/GMToolConfig.h"

#include "DlgItem.h"
#include "DlgDatetime.h"
#include ".\dlgsitemcustomedit.h"

#include "SelColorDlg.h"
#include "HLibColorValue.h"

// CDlgSItemCustomEdit dialog

IMPLEMENT_DYNAMIC(CDlgSItemCustomEdit, CDialog)

CDlgSItemCustomEdit::CDlgSItemCustomEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSItemCustomEdit::IDD, pParent)
	, m_emEditType(EMITEMEDIT_TYPE_SIZE)
	, m_wPutOnSlot(SLOT_NSIZE)
	, m_nInvenPosX(-1)
	, m_nInvenPosY(-1)
	, m_pItemCustom(NULL)
	, m_tDisguiseDate(0)
{

}

CDlgSItemCustomEdit::CDlgSItemCustomEdit(
	SITEMCUSTOM* pITEMCUSTOM, 
	EMITEMEDIT_TYPE emEDIT, 
	WORD wSLOT, 
	int nPOSX, 
	int nPOSY, 
	CWnd* pParent /*= NULL*/)
	: CDialog(CDlgSItemCustomEdit::IDD, pParent)
	, m_emEditType(emEDIT)
	, m_wPutOnSlot(wSLOT)
	, m_nInvenPosX(nPOSX)
	, m_nInvenPosY(nPOSY)
	, m_pItemCustom(pITEMCUSTOM)
	, m_tDisguiseDate(0)
{

}

CDlgSItemCustomEdit::~CDlgSItemCustomEdit()
{
}

void CDlgSItemCustomEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SITEMCUSTOM_EDIT_COMBO_PRESET, m_cPreset );
}


BEGIN_MESSAGE_MAP(CDlgSItemCustomEdit, CDialog)
	ON_BN_CLICKED(IDC_SITEMCUSTOM_EDIT_BUTTON_ITEMID, OnBnClickedSitemcustomEditButtonItemid)
	ON_BN_CLICKED(IDC_SITEMCUSTOM_EDIT_BUTTON_DIS_ID, OnBnClickedSitemcustomEditButtonDisId)
	ON_BN_CLICKED(IDC_SITEMCUSTOM_EDIT_BUTTON_DIS_DATE, OnBnClickedSitemcustomEditButtonDisDate)
	ON_BN_CLICKED(IDC_SITEMCUSTOM_EDIT_BUTTON_DIS_REMOVE, OnBnClickedSitemcustomEditButtonDisRemove)
	ON_BN_CLICKED(IDC_SITEMCUSTOM_EDIT_BUTTON_DIS_DEFAULT, OnBnClickedSitemcustomEditButtonDisDefault)
	ON_BN_CLICKED(IDC_SITEMCUSTOM_EDIT_BUTTON_PRESET, OnBnClickedSitemcustomEditButtonPreset)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_EN_CHANGE(IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_MID, OnEnChangeSitemcustomEditEditItemMid)
	ON_EN_CHANGE(IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_SID, OnEnChangeSitemcustomEditEditItemSid)
	ON_EN_CHANGE(IDC_SITEMCUSTOM_EDIT_EDIT_DIS_MID, OnEnChangeSitemcustomEditEditDisMid)
	ON_EN_CHANGE(IDC_SITEMCUSTOM_EDIT_EDIT_DIS_SID, OnEnChangeSitemcustomEditEditDisSid)
	ON_BN_CLICKED(IDC_SITEMCUSTOM_EDIT_BUTTON_COLOR_1, OnBnClickedSitemcustomEditButtonColor1)
	ON_BN_CLICKED(IDC_SITEMCUSTOM_EDIT_BUTTON_COLOR_2, OnBnClickedSitemcustomEditButtonColor2)
	ON_BN_CLICKED(IDC_SITEMCUSTOM_EDIT_BUTTON_COLOR_DEFAULT, OnBnClickedSitemcustomEditButtonColorDefault)
	ON_BN_CLICKED(IDC_SITEMCUSTOM_EDIT_BUTTON_COLOR_DEFAULT_COSTUME, OnBnClickedSitemcustomEditButtonColorDefaultCostume)
	ON_BN_CLICKED(IDC_SITEMCUSTOM_EDIT_BUTTON_COLOR_DEFAULT_RESET, OnBnClickedSitemcustomEditButtonColorDefaultReset)
END_MESSAGE_MAP()


// CDlgSItemCustomEdit message handlers
BOOL CDlgSItemCustomEdit::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString strWindowText("");
	switch( m_emEditType )
	{
	case EMITEMEDIT_TYPE_INVEN_ITEM_EDIT:{
			strWindowText.Format( "Edit Item Pos X:%d Y:%d", m_nInvenPosX, m_nInvenPosY );
		}break;
	case EMITEMEDIT_TYPE_PUTON_CHAR:{
			strWindowText.Format( "Edit Slot [%d]%s", m_wPutOnSlot, COMMENT::ITEMSLOT_CHAR[m_wPutOnSlot].c_str() );
		}break;
	case EMITEMEDIT_TYPE_PUTON_PET:{
			strWindowText.Format( "Edit Slot [%d]%s", m_wPutOnSlot, COMMENT::ITEMSLOT_PET[m_wPutOnSlot].c_str() );
		}break;
	case EMITEMEDIT_TYPE_PUTON_VEHICLE:{
			strWindowText.Format( "Edit Slot [%d]%s", m_wPutOnSlot, COMMENT::ITEMSLOT_VEHICLE[m_wPutOnSlot].c_str() );
		}break;
	};

	SetWindowText( strWindowText.GetString() );

	SGMTOOL_PRESET_ITEM_MAP mapPreset = CGMToolPresetItem::GetInstance().m_MapPresetItem;
	SGMTOOL_PRESET_ITEM_MAP_ITER iter_start = mapPreset.begin();
	for( int i=0; iter_start != mapPreset.end(); ++iter_start, ++i ){
		const SGMTOOL_PRESET_ITEM sPRESET = (*iter_start).second;
		m_cPreset.InsertString( i, sPRESET.strNAME.c_str() );
		m_cPreset.SetItemData( i, sPRESET.wID );
	}
	if ( m_cPreset.GetCount() > 0 ){
		m_cPreset.SetCurSel(0);
	}

	std::string strRV[EMR_OPT_SIZE];
	for( int i=0; i<EMR_OPT_SIZE; ++i )
	{
		CString strTemp;
		strTemp.Format( "[%02d](%s)", i, COMMENT::ITEM_RANDOM_OPT[i].c_str() );
		strRV[i] += strTemp.GetString();
	}

	SetWin_Combo_Init( this, IDC_SITEMCUSTOM_EDIT_COMBO_RV_1, strRV, EMR_OPT_SIZE );
	SetWin_Combo_Init( this, IDC_SITEMCUSTOM_EDIT_COMBO_RV_2, strRV, EMR_OPT_SIZE );
	SetWin_Combo_Init( this, IDC_SITEMCUSTOM_EDIT_COMBO_RV_3, strRV, EMR_OPT_SIZE );
	SetWin_Combo_Init( this, IDC_SITEMCUSTOM_EDIT_COMBO_RV_4, strRV, EMR_OPT_SIZE );

	DataDefault();
	DataShow();

	return TRUE;
}

void CDlgSItemCustomEdit::DataDefault()
{
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_MID, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_SID, NATIVEID_NULL().wSubID );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_MID, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_SID, NATIVEID_NULL().wSubID );
	SetWin_Text( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_DATE, "" );

	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DAMAGE, 0 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DEFENSE, 0 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_FIRE, 0 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_ICE, 0 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_ELECTRIC, 0 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_POISON, 0 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_SPIRIT, 0 );

	SetWin_Check( this, IDC_SITEMCUSTOM_EDIT_CHECK_NONDROP, FALSE );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_TOTAL, 1 );

	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RV_1, 0 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RV_2, 0 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RV_3, 0 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RV_4, 0 );

	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_FIELD_NUM, 0 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_CHANNEL_NUM, 0 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_PET_ID, 0 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_VEHICLE_ID, 0 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_GEN_NUM, 0 );
	SetWin_Text( this, IDC_SITEMCUSTOM_EDIT_EDIT_GEN_TYPE, "" );
	SetWin_Text( this, IDC_SITEMCUSTOM_EDIT_EDIT_BORN_DATE, "" );

	m_tDisguiseDate = 0;

	/*item color, Juver, 2018/01/10 */
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_COLOR_1, 1 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_COLOR_2, 1 );
}

void CDlgSItemCustomEdit::DataShow()
{
	if ( !m_pItemCustom ){
		return;
	}

	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_MID, m_pItemCustom->sNativeID.wMainID );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_SID, m_pItemCustom->sNativeID.wSubID );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_MID, m_pItemCustom->nidDISGUISE.wMainID );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_SID, m_pItemCustom->nidDISGUISE.wSubID );
	m_tDisguiseDate = m_pItemCustom->tDISGUISE;
	SetWin_Text( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_DATE, _HLIB::cstring_timet12( m_tDisguiseDate ).GetString() );

	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DAMAGE, m_pItemCustom->cDAMAGE );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DEFENSE, m_pItemCustom->cDEFENSE );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_FIRE, m_pItemCustom->cRESIST_FIRE );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_ICE, m_pItemCustom->cRESIST_ICE );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_ELECTRIC, m_pItemCustom->cRESIST_ELEC );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_POISON, m_pItemCustom->cRESIST_POISON );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_SPIRIT, m_pItemCustom->cRESIST_SPIRIT );

	SetWin_Check( this, IDC_SITEMCUSTOM_EDIT_CHECK_NONDROP, m_pItemCustom->IsGM_GENITEM() );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_TOTAL, m_pItemCustom->wTurnNum );

	SetWin_Combo_Sel( this, IDC_SITEMCUSTOM_EDIT_COMBO_RV_1, m_pItemCustom->cOptTYPE1 );
	SetWin_Combo_Sel( this, IDC_SITEMCUSTOM_EDIT_COMBO_RV_2, m_pItemCustom->cOptTYPE2 );
	SetWin_Combo_Sel( this, IDC_SITEMCUSTOM_EDIT_COMBO_RV_3, m_pItemCustom->cOptTYPE3 );
	SetWin_Combo_Sel( this, IDC_SITEMCUSTOM_EDIT_COMBO_RV_4, m_pItemCustom->cOptTYPE4 );

	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RV_1, m_pItemCustom->nOptVALUE1 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RV_2, m_pItemCustom->nOptVALUE2 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RV_3, m_pItemCustom->nOptVALUE3 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RV_4, m_pItemCustom->nOptVALUE4 );

	std::string strGEN[13] = 
	{
		"EMGEN_DEFAULT",
		"EMGEN_INIT",
		"EMGEN_SHOP",
		"EMGEN_NPC",
		"EMGEN_QUEST",
		"EMGEN_MOB",
		"EMGEN_GMEDIT",
		"EMGEN_OLD",
		"EMGEN_BILLING",
		"EMGEN_GMEDIT2",
		"EMGEN_ITEMMIX",
		"EMGEN_GATHERING",
		"EMGEN_SYSTEM",
	};

	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_FIELD_NUM, m_pItemCustom->cFieldID );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_CHANNEL_NUM, m_pItemCustom->cChnID );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_PET_ID, m_pItemCustom->dwPetID );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_VEHICLE_ID, m_pItemCustom->dwVehicleID );
	SetWin_Num_LONGLONG( this, IDC_SITEMCUSTOM_EDIT_EDIT_GEN_NUM, m_pItemCustom->lnGenNum );

	SetWin_Text( this, IDC_SITEMCUSTOM_EDIT_EDIT_GEN_TYPE, strGEN[m_pItemCustom->cGenType].c_str() );
	SetWin_Text( this, IDC_SITEMCUSTOM_EDIT_EDIT_BORN_DATE, _HLIB::cstring_timet12( m_pItemCustom->tBORNTIME ).GetString() );

	/*item color, Juver, 2018/01/10 */
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_COLOR_1, m_pItemCustom->wColor1 );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_COLOR_2, m_pItemCustom->wColor2 );
}

BOOL CDlgSItemCustomEdit::DataSave()
{
	if ( !m_pItemCustom ){
		return FALSE;
	}

	int nMID = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_MID );
	int nSID = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_SID );
	int nDIS_MID = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_MID );
	int nDIS_SID = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_SID );

	int nDamage = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DAMAGE );
	int nDefense = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DEFENSE );
	int nResFire = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_FIRE );
	int nResIce = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_ICE );
	int nResElectric = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_ELECTRIC );
	int nResPoison = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_POISON );
	int nResSpirit = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_SPIRIT );

	int nOptionType1 = GetWin_Combo_Sel( this, IDC_SITEMCUSTOM_EDIT_COMBO_RV_1 );
	int nOptionType2 = GetWin_Combo_Sel( this, IDC_SITEMCUSTOM_EDIT_COMBO_RV_2 );
	int nOptionType3 = GetWin_Combo_Sel( this, IDC_SITEMCUSTOM_EDIT_COMBO_RV_3 );
	int nOptionType4 = GetWin_Combo_Sel( this, IDC_SITEMCUSTOM_EDIT_COMBO_RV_4 );

	int nOptionVal1 = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RV_1 );
	int nOptionVal2 = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RV_2 );
	int nOptionVal3 = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RV_3 );
	int nOptionVal4 = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RV_4 );

	BOOL bNonDrop = GetWin_Check( this, IDC_SITEMCUSTOM_EDIT_CHECK_NONDROP );
	int nTurnNum = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_TOTAL );

	/*item color, Juver, 2018/01/10 */
	WORD wColor1 = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_COLOR_1 );
	WORD wColor2 = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_COLOR_2 );

	SITEM* pItem = GLItemMan::GetInstance().GetItem( nMID, nSID );
	if ( !pItem ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Item [%d~%d]", nMID, nSID );
		return FALSE;
	}

	SITEM* pItemDIS = GLItemMan::GetInstance().GetItem( nDIS_MID, nDIS_SID );
	if ( pItemDIS && !pItemDIS->sBasicOp.IsDISGUISE() ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Costume Item is Not Costume" );
		return FALSE;
	}

	if ( !pItemDIS && SNATIVEID( nDIS_MID, nDIS_SID ) != NATIVEID_NULL() ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid costume ID: %d~%d. Click 'Remove' Button to properly remove costume", nDIS_MID, nDIS_SID );
		return FALSE;
	}

	if ( pItemDIS && pItemDIS->IsTIMELMT() ){
		if ( m_tDisguiseDate <= CTime::GetCurrentTime().GetTime() ){
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid costume Expiration Time!" );
			return FALSE;
		}
	}

	if ( !pItemDIS || ( pItemDIS && !pItemDIS->IsTIMELMT() ) ){
		m_tDisguiseDate = 0;
	}

	
	if ( nDamage < 0 || nDamage >= UCHAR_MAX ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Damage Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_SITEMCUSTOM_EDIT_EDIT_DAMAGE)); 
		return FALSE;
	}

	if ( nDefense < 0 || nDefense >= UCHAR_MAX ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Defense Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_SITEMCUSTOM_EDIT_EDIT_DEFENSE)); 
		return FALSE;
	}

	if ( nResFire < 0 || nResFire >= UCHAR_MAX ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Res Fire Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_SITEMCUSTOM_EDIT_EDIT_RES_FIRE)); 
		return FALSE;
	}

	if ( nResIce < 0 || nResIce >= UCHAR_MAX ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Res Ice Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_SITEMCUSTOM_EDIT_EDIT_RES_ICE)); 
		return FALSE;
	}

	if ( nResElectric < 0 || nResElectric >= UCHAR_MAX ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Res Electric Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_SITEMCUSTOM_EDIT_EDIT_RES_ELECTRIC)); 
		return FALSE;
	}

	if ( nResPoison < 0 || nResPoison >= UCHAR_MAX ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Res Poison Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_SITEMCUSTOM_EDIT_EDIT_RES_POISON)); 
		return FALSE;
	}

	if ( nResSpirit < 0 || nResSpirit >= UCHAR_MAX ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Res Spirit Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_SITEMCUSTOM_EDIT_EDIT_RES_SPIRIT)); 
		return FALSE;
	}


	if ( nOptionType1 < 0 || nOptionType1 >= EMR_OPT_SIZE ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid RV Option 1 Type!" );
		return FALSE;
	}

	if ( nOptionType2 < 0 || nOptionType2 >= EMR_OPT_SIZE ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid RV Option 2 Type!" );
		return FALSE;
	}

	if ( nOptionType3 < 0 || nOptionType3 >= EMR_OPT_SIZE ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid RV Option 3 Type!" );
		return FALSE;
	}

	if ( nOptionType4 < 0 || nOptionType4 >= EMR_OPT_SIZE ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid RV Option 4 Type!" );
		return FALSE;
	}

	if ( nOptionVal1 <= SHRT_MIN || nOptionVal1 >= SHRT_MAX ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid RV Option 1 Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_SITEMCUSTOM_EDIT_EDIT_RV_1)); 
		return FALSE;
	}

	if ( nOptionVal2 <= SHRT_MIN || nOptionVal2 >= SHRT_MAX ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid RV Option 2 Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_SITEMCUSTOM_EDIT_EDIT_RV_2)); 
		return FALSE;
	}

	if ( nOptionVal3 <= SHRT_MIN || nOptionVal3 >= SHRT_MAX ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid RV Option 3 Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_SITEMCUSTOM_EDIT_EDIT_RV_3)); 
		return FALSE;
	}

	if ( nOptionVal4 <= SHRT_MIN || nOptionVal4 >= SHRT_MAX ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid RV Option 4 Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_SITEMCUSTOM_EDIT_EDIT_RV_4)); 
		return FALSE;
	}

	if ( nTurnNum < 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Quantity Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_SITEMCUSTOM_EDIT_EDIT_TOTAL)); 
		return FALSE;
	}

	m_pItemCustom->sNativeID.wMainID = (WORD)nMID;
	m_pItemCustom->sNativeID.wSubID = (WORD)nSID;
	m_pItemCustom->nidDISGUISE.wMainID = (WORD)nDIS_MID;
	m_pItemCustom->nidDISGUISE.wSubID = (WORD)nDIS_SID;
	m_pItemCustom->tDISGUISE = m_tDisguiseDate;

	m_pItemCustom->cDAMAGE = (BYTE)nDamage;
	m_pItemCustom->cDEFENSE = (BYTE)nDefense;
	m_pItemCustom->cRESIST_FIRE = (BYTE)nResFire;
	m_pItemCustom->cRESIST_ICE = (BYTE)nResIce;
	m_pItemCustom->cRESIST_ELEC = (BYTE)nResElectric;
	m_pItemCustom->cRESIST_POISON = (BYTE)nResPoison;
	m_pItemCustom->cRESIST_SPIRIT = (BYTE)nResSpirit;

	m_pItemCustom->cOptTYPE1 = (BYTE)nOptionType1;
	m_pItemCustom->cOptTYPE2 = (BYTE)nOptionType2;
	m_pItemCustom->cOptTYPE3 = (BYTE)nOptionType3;
	m_pItemCustom->cOptTYPE4 = (BYTE)nOptionType4;

	m_pItemCustom->nOptVALUE1 = (short)nOptionVal1;
	m_pItemCustom->nOptVALUE2 = (short)nOptionVal2;
	m_pItemCustom->nOptVALUE3 = (short)nOptionVal3;
	m_pItemCustom->nOptVALUE4 = (short)nOptionVal4;

	m_pItemCustom->wTurnNum = (WORD)nTurnNum;
	m_pItemCustom->tDISGUISE = m_tDisguiseDate;

	if ( bNonDrop && !m_pItemCustom->IsGM_GENITEM() ){
		m_pItemCustom->SetGM_GEN( GMTOOL_CONFIG::m_dwLoginID );
	}

	if ( !bNonDrop && m_pItemCustom->IsGM_GENITEM() ){
		m_pItemCustom->lnGenNum = 0;
		m_pItemCustom->cFieldID = 0;
		m_pItemCustom->cGenType = EMGEN_INIT;
	}
	
	if ( m_pItemCustom->tBORNTIME == 0 ){
		m_pItemCustom->tBORNTIME = CTime::GetCurrentTime().GetTime();
	}

	/*item color, Juver, 2018/01/10 */
	m_pItemCustom->wColor1 = wColor1;
	m_pItemCustom->wColor2 = wColor2;

	return TRUE;
}

void CDlgSItemCustomEdit::OnBnClickedSitemcustomEditButtonItemid()
{
	CDlgItem dlg( this );
	if( dlg.DoModal() == IDOK ){
		SNATIVEID sID = dlg.m_SELECTED;
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sID );
		if ( !pItem ){
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Item (%d~%d)", sID.wMainID, sID.wSubID );
			return;
		}

		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_MID, sID.wMainID );
		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_SID, sID.wSubID );
	}
}

void CDlgSItemCustomEdit::OnBnClickedSitemcustomEditButtonDisId()
{
	CDlgItem dlg( this );
	if( dlg.DoModal() == IDOK ){
		SNATIVEID sID = dlg.m_SELECTED;
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sID );
		if ( !pItem ){
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Item (%d~%d)", sID.wMainID, sID.wSubID );
			return;
		}

		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_MID, sID.wMainID );
		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_SID, sID.wSubID );
	}
}

void CDlgSItemCustomEdit::OnBnClickedSitemcustomEditButtonDisDate()
{
	WORD wMID = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_MID );
	WORD wSID = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_SID );

	SITEM* pITEM = GLItemMan::GetInstance().GetItem( wMID, wSID );
	if ( !pITEM ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Costume Item!" );
		return;
	}

	if ( !pITEM->sBasicOp.IsDISGUISE() ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Costume Item is Not Costume" );
		return;
	}

	if ( !pITEM->IsTIMELMT() ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Costume item no expiration! no need to set expire date!" );
		return;
	}

	__time64_t tTimeCur = CTime::GetCurrentTime().GetTime();

	if ( m_tDisguiseDate > tTimeCur )
		tTimeCur = m_tDisguiseDate;

	CDlgDatetime dlg( this );
	dlg.DateSetMin( CTime::GetCurrentTime().GetTime() );
	dlg.DateTimeSet( tTimeCur );
	if ( dlg.DoModal() == IDOK ){
		m_tDisguiseDate = dlg.DateTimeGet();
		SetWin_Text( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_DATE, _HLIB::cstring_timet12( m_tDisguiseDate ).GetString() );
	}
}

void CDlgSItemCustomEdit::OnBnClickedSitemcustomEditButtonDisRemove()
{
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_MID, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_SID, NATIVEID_NULL().wSubID );
}

void CDlgSItemCustomEdit::OnBnClickedSitemcustomEditButtonDisDefault()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Use Default Item Expiration?" ) == IDYES ){
		WORD wMID = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_MID );
		WORD wSID = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_SID );

		SITEM* pITEM = GLItemMan::GetInstance().GetItem( wMID, wSID );
		if ( !pITEM ){
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Costume Item!" );
			return;
		}

		if ( !pITEM->sBasicOp.IsDISGUISE() ){
			CDebugSet::MsgBox( GetSafeHwnd(), "Costume Item is Not Costume" );
			return;
		}

		if ( !pITEM->IsTIMELMT() ){
			CDebugSet::MsgBox( GetSafeHwnd(), "Costume item no expiration! no need to set expire date!" );
			return;
		}

		if ( pITEM->sDrugOp.tTIME_LMT != 0 ){
			__time64_t &tTIME = m_tDisguiseDate;
			CTime tCurTime = CTime::GetCurrentTime();
			CTime tSeedTime(tTIME);
			CTimeSpan tLMT (pITEM->sDrugOp.tTIME_LMT);
			tLMT += CTimeSpan(0,0,05,0);
			tSeedTime = tCurTime + tLMT;
			tTIME = tSeedTime.GetTime();
			SetWin_Text( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_DATE, _HLIB::cstring_timet12( m_tDisguiseDate ).GetString() );
		}
	}
}

void CDlgSItemCustomEdit::OnBnClickedSitemcustomEditButtonPreset()
{
	int nIndex = m_cPreset.GetCurSel();
	if ( nIndex == CB_ERR ){
		return;
	}

	WORD wID = (WORD)m_cPreset.GetItemData( nIndex );
	SGMTOOL_PRESET_ITEM* pPRESET = CGMToolPresetItem::GetInstance().PresetItemGet( wID );
	if ( pPRESET ){
		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DAMAGE, pPRESET->cDAMAGE );
		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DEFENSE, pPRESET->cDEFENSE );

		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_FIRE, pPRESET->cRESFIRE );
		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_ICE, pPRESET->cRESICE );
		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_ELECTRIC, pPRESET->cRESELECT );
		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_POISON, pPRESET->cRESPOISON );
		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RES_SPIRIT, pPRESET->cRESSPIRIT );

		SetWin_Combo_Sel( this, IDC_SITEMCUSTOM_EDIT_COMBO_RV_1, pPRESET->cOPTIONTYPE1 );
		SetWin_Combo_Sel( this, IDC_SITEMCUSTOM_EDIT_COMBO_RV_2, pPRESET->cOPTIONTYPE2 );
		SetWin_Combo_Sel( this, IDC_SITEMCUSTOM_EDIT_COMBO_RV_3, pPRESET->cOPTIONTYPE3 );
		SetWin_Combo_Sel( this, IDC_SITEMCUSTOM_EDIT_COMBO_RV_4, pPRESET->cOPTIONTYPE4 );

		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RV_1, pPRESET->nOPTIONVAL1 );
		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RV_2, pPRESET->nOPTIONVAL2 );
		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RV_3, pPRESET->nOPTIONVAL3 );
		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_RV_4, pPRESET->nOPTIONVAL4 );

		SetWin_Check( this, IDC_SITEMCUSTOM_EDIT_CHECK_NONDROP, pPRESET->bNONDROP );
	}
}

void CDlgSItemCustomEdit::OnBnClickedOk()
{
	if ( DataSave() ){
		CDialog::OnOK();
	}
}

void CDlgSItemCustomEdit::OnBnClickedCancel()
{
	CDialog::OnCancel();
}

void CDlgSItemCustomEdit::OnEnChangeSitemcustomEditEditItemMid()
{
	GMTOOL_GLOBAL::DataCheckItem( this, 
		IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_MID,
		IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_SID,
		IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_NAME );
}

void CDlgSItemCustomEdit::OnEnChangeSitemcustomEditEditItemSid()
{
	GMTOOL_GLOBAL::DataCheckItem( this, 
		IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_MID,
		IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_SID,
		IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_NAME );
}

void CDlgSItemCustomEdit::OnEnChangeSitemcustomEditEditDisMid()
{
	GMTOOL_GLOBAL::DataCheckItem( this, 
		IDC_SITEMCUSTOM_EDIT_EDIT_DIS_MID,
		IDC_SITEMCUSTOM_EDIT_EDIT_DIS_SID,
		IDC_SITEMCUSTOM_EDIT_EDIT_DIS_NAME );
}

void CDlgSItemCustomEdit::OnEnChangeSitemcustomEditEditDisSid()
{
	GMTOOL_GLOBAL::DataCheckItem( this, 
		IDC_SITEMCUSTOM_EDIT_EDIT_DIS_MID,
		IDC_SITEMCUSTOM_EDIT_EDIT_DIS_SID,
		IDC_SITEMCUSTOM_EDIT_EDIT_DIS_NAME );
}

void CDlgSItemCustomEdit::OnBnClickedSitemcustomEditButtonColor1()
{
	/*item color, Juver, 2018/01/10 */
	WORD wColor = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_COLOR_1 );
	DWORD dwColor = _HLIB::color_dword_word( wColor );

	CSelColorDlg dlg( BYTE( _HLIB::color_get_int_r( dwColor) ), 
		BYTE( _HLIB::color_get_int_g( dwColor ) ), 
		BYTE( _HLIB::color_get_int_b( dwColor ) ), 
		this );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_COLOR_1, _HLIB::color_word_dword( _HLIB::color_argb_rgb(crNewColor) ) );
	}
}

void CDlgSItemCustomEdit::OnBnClickedSitemcustomEditButtonColor2()
{
	/*item color, Juver, 2018/01/10 */
	WORD wColor = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_COLOR_2 );
	DWORD dwColor = _HLIB::color_dword_word( wColor );

	CSelColorDlg dlg( BYTE( _HLIB::color_get_int_r( dwColor) ), 
		BYTE( _HLIB::color_get_int_g( dwColor ) ), 
		BYTE( _HLIB::color_get_int_b( dwColor ) ), 
		this );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_COLOR_2, _HLIB::color_word_dword( _HLIB::color_argb_rgb(crNewColor) ) );
	}
}

void CDlgSItemCustomEdit::OnBnClickedSitemcustomEditButtonColorDefault()
{
	/*item color, Juver, 2018/01/10 */
	WORD wMID = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_MID );
	WORD wSID = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_ITEM_SID );

	SITEM* pitem_data_check = GLItemMan::GetInstance().GetItem( wMID, wSID );
	if ( pitem_data_check && pitem_data_check->sBasicOp.bItemColor )
	{
		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_COLOR_1,  pitem_data_check->sBasicOp.wItemColor1 );
		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_COLOR_2, pitem_data_check->sBasicOp.wItemColor2 );
	}
}

void CDlgSItemCustomEdit::OnBnClickedSitemcustomEditButtonColorDefaultCostume()
{
	/*item color, Juver, 2018/01/10 */
	WORD wMID = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_MID );
	WORD wSID = GetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_DIS_SID );

	SITEM* pitem_data_check = GLItemMan::GetInstance().GetItem( wMID, wSID );
	if ( pitem_data_check && pitem_data_check->sBasicOp.bItemColor )
	{
		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_COLOR_1,  pitem_data_check->sBasicOp.wItemColor1 );
		SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_COLOR_2, pitem_data_check->sBasicOp.wItemColor2 );
	}
}

void CDlgSItemCustomEdit::OnBnClickedSitemcustomEditButtonColorDefaultReset()
{
	/*item color, Juver, 2018/01/10 */
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_COLOR_1, ITEMCOLOR_WHITE );
	SetWin_Num_int( this, IDC_SITEMCUSTOM_EDIT_EDIT_COLOR_2, ITEMCOLOR_WHITE );
}
