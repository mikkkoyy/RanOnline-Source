// DlgPresetItemEdit.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgPresetItemEdit.h"

#include "EtcFunction.h"
#include "GLItemDef.h"

#include "Logic/GMToolPresetItem.h"

// CDlgPresetItemEdit dialog

IMPLEMENT_DYNAMIC(CDlgPresetItemEdit, CDialog)

CDlgPresetItemEdit::CDlgPresetItemEdit( CWnd* pParent /*= NULL*/, SGMTOOL_PRESET_ITEM* pPreset /*= NULL*/ )
	: CDialog(CDlgPresetItemEdit::IDD, pParent)
	, m_pPreset( NULL )
{
	m_pPreset = pPreset;
}

CDlgPresetItemEdit::~CDlgPresetItemEdit()
{
}

void CDlgPresetItemEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgPresetItemEdit, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgPresetItemEdit message handlers
BOOL CDlgPresetItemEdit::OnInitDialog()
{
	CDialog::OnInitDialog();

	std::string strRV[EMR_OPT_SIZE];
	for( int i=0; i<EMR_OPT_SIZE; ++i )
	{
		CString strTemp;
		strTemp.Format( "[%02d](%s)", i, COMMENT::ITEM_RANDOM_OPT[i].c_str() );
		strRV[i] += strTemp.GetString();
	}

	SetWin_Combo_Init( this, IDC_PRESET_ITEM_COMBO_RV_OPTION_1, strRV, EMR_OPT_SIZE );
	SetWin_Combo_Init( this, IDC_PRESET_ITEM_COMBO_RV_OPTION_2, strRV, EMR_OPT_SIZE );
	SetWin_Combo_Init( this, IDC_PRESET_ITEM_COMBO_RV_OPTION_3, strRV, EMR_OPT_SIZE );
	SetWin_Combo_Init( this, IDC_PRESET_ITEM_COMBO_RV_OPTION_4, strRV, EMR_OPT_SIZE );

	DataShow();

	return TRUE;  
}

void CDlgPresetItemEdit::DataShow()
{
	if ( !m_pPreset ){
		return;
	}

	SetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_ID, m_pPreset->wID );
	SetWin_Text( this, IDC_PRESET_ITEM_EDIT_NAME, m_pPreset->strNAME.c_str() );

	SetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_DAMAGE, m_pPreset->cDAMAGE );
	SetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_DEFENSE, m_pPreset->cDEFENSE );

	SetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RES_FIRE, m_pPreset->cRESFIRE );
	SetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RES_ICE, m_pPreset->cRESICE );
	SetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RES_ELECTRIC, m_pPreset->cRESELECT );
	SetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RES_POISON, m_pPreset->cRESPOISON );
	SetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RES_SPIRIT, m_pPreset->cRESSPIRIT );

	SetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RV_VALUE_1, m_pPreset->nOPTIONVAL1 );
	SetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RV_VALUE_2, m_pPreset->nOPTIONVAL2 );
	SetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RV_VALUE_3, m_pPreset->nOPTIONVAL3 );
	SetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RV_VALUE_4, m_pPreset->nOPTIONVAL4 );

	SetWin_Combo_Sel( this, IDC_PRESET_ITEM_COMBO_RV_OPTION_1, m_pPreset->cOPTIONTYPE1 );
	SetWin_Combo_Sel( this, IDC_PRESET_ITEM_COMBO_RV_OPTION_2, m_pPreset->cOPTIONTYPE2 );
	SetWin_Combo_Sel( this, IDC_PRESET_ITEM_COMBO_RV_OPTION_3, m_pPreset->cOPTIONTYPE3 );
	SetWin_Combo_Sel( this, IDC_PRESET_ITEM_COMBO_RV_OPTION_4, m_pPreset->cOPTIONTYPE4 );

	SetWin_Check( this, IDC_PRESET_ITEM_CHECK_NONDROP, m_pPreset->bNONDROP );
}

BOOL CDlgPresetItemEdit::DataSave()
{
	if ( !m_pPreset ){
		return FALSE;
	}

	//WORD wID = (WORD)GetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_ID );
	std::string strName = GetWin_Text( this, IDC_PRESET_ITEM_EDIT_NAME );

	int nDamage = GetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_DAMAGE );
	int nDefense = GetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_DEFENSE );

	int nResFire = GetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RES_FIRE );
	int nResIce = GetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RES_ICE );
	int nResElect = GetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RES_ELECTRIC );
	int nResPoison = GetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RES_POISON );
	int nResSpirit = GetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RES_SPIRIT );

	int nOptionVal1 = GetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RV_VALUE_1 );
	int nOptionVal2 = GetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RV_VALUE_2 );
	int nOptionVal3 = GetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RV_VALUE_3 );
	int nOptionVal4 = GetWin_Num_int( this, IDC_PRESET_ITEM_EDIT_RV_VALUE_4 );

	int nOptionType1 = GetWin_Combo_Sel( this, IDC_PRESET_ITEM_COMBO_RV_OPTION_1 );
	int nOptionType2 = GetWin_Combo_Sel( this, IDC_PRESET_ITEM_COMBO_RV_OPTION_2 );
	int nOptionType3 = GetWin_Combo_Sel( this, IDC_PRESET_ITEM_COMBO_RV_OPTION_3 );
	int nOptionType4 = GetWin_Combo_Sel( this, IDC_PRESET_ITEM_COMBO_RV_OPTION_4 );

	bool bNonDrop = GetWin_Check( this, IDC_PRESET_ITEM_CHECK_NONDROP )? true:false;

	if ( strName.empty() ){
		CDebugSet::MsgBoxAfx( "Name cannot be empty!" );
		GotoDlgCtrl(GetDlgItem(IDC_PRESET_ITEM_EDIT_NAME)); 
		return FALSE;
	}

	if ( nDamage < 0 || nDamage >= UCHAR_MAX ){
		CDebugSet::MsgBoxAfx( "Invalid Damage Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_PRESET_ITEM_EDIT_DAMAGE)); 
		return FALSE;
	}

	if ( nDefense < 0 || nDefense >= UCHAR_MAX ){
		CDebugSet::MsgBoxAfx( "Invalid Defense Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_PRESET_ITEM_EDIT_DEFENSE)); 
		return FALSE;
	}

	if ( nResFire < 0 || nResFire >= UCHAR_MAX ){
		CDebugSet::MsgBoxAfx( "Invalid Res Fire Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_PRESET_ITEM_EDIT_RES_FIRE)); 
		return FALSE;
	}

	if ( nResIce < 0 || nResIce >= UCHAR_MAX ){
		CDebugSet::MsgBoxAfx( "Invalid Res Ice Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_PRESET_ITEM_EDIT_RES_ICE)); 
		return FALSE;
	}

	if ( nResElect < 0 || nResElect >= UCHAR_MAX ){
		CDebugSet::MsgBoxAfx( "Invalid Res Elect Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_PRESET_ITEM_EDIT_RES_ELECTRIC)); 
		return FALSE;
	}

	if ( nResPoison < 0 || nResPoison >= UCHAR_MAX ){
		CDebugSet::MsgBoxAfx( "Invalid Res Poison Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_PRESET_ITEM_EDIT_RES_POISON)); 
		return FALSE;
	}

	if ( nResSpirit < 0 || nResSpirit >= UCHAR_MAX ){
		CDebugSet::MsgBoxAfx( "Invalid Res Spirit Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_PRESET_ITEM_EDIT_RES_SPIRIT)); 
		return FALSE;
	}

	if ( nOptionType1 < 0 || nOptionType1 >= EMR_OPT_SIZE ){
		CDebugSet::MsgBoxAfx( "Invalid RV Option 1 Type!" );
		return FALSE;
	}

	if ( nOptionType2 < 0 || nOptionType2 >= EMR_OPT_SIZE ){
		CDebugSet::MsgBoxAfx( "Invalid RV Option 2 Type!" );
		return FALSE;
	}

	if ( nOptionType3 < 0 || nOptionType3 >= EMR_OPT_SIZE ){
		CDebugSet::MsgBoxAfx( "Invalid RV Option 3 Type!" );
		return FALSE;
	}

	if ( nOptionType4 < 0 || nOptionType4 >= EMR_OPT_SIZE ){
		CDebugSet::MsgBoxAfx( "Invalid RV Option 4 Type!" );
		return FALSE;
	}

	if ( nOptionVal1 <= SHRT_MIN || nOptionVal1 >= SHRT_MAX ){
		CDebugSet::MsgBoxAfx( "Invalid RV Option 1 Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_PRESET_ITEM_EDIT_RV_VALUE_1)); 
		return FALSE;
	}

	if ( nOptionVal2 <= SHRT_MIN || nOptionVal2 >= SHRT_MAX ){
		CDebugSet::MsgBoxAfx( "Invalid RV Option 2 Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_PRESET_ITEM_EDIT_RV_VALUE_2)); 
		return FALSE;
	}

	if ( nOptionVal3 <= SHRT_MIN || nOptionVal3 >= SHRT_MAX ){
		CDebugSet::MsgBoxAfx( "Invalid RV Option 3 Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_PRESET_ITEM_EDIT_RV_VALUE_3)); 
		return FALSE;
	}

	if ( nOptionVal4 <= SHRT_MIN || nOptionVal4 >= SHRT_MAX ){
		CDebugSet::MsgBoxAfx( "Invalid RV Option 4 Value!" );
		GotoDlgCtrl(GetDlgItem(IDC_PRESET_ITEM_EDIT_RV_VALUE_4)); 
		return FALSE;
	}

	//m_pPreset->wID = wID;
	m_pPreset->strNAME = strName.c_str();
	m_pPreset->cDAMAGE = (BYTE)nDamage;
	m_pPreset->cDEFENSE = (BYTE)nDefense;
	m_pPreset->cRESFIRE = (BYTE)nResFire;
	m_pPreset->cRESICE = (BYTE)nResIce;
	m_pPreset->cRESELECT = (BYTE)nResElect;
	m_pPreset->cRESPOISON = (BYTE)nResPoison;
	m_pPreset->cRESSPIRIT = (BYTE)nResSpirit;
	m_pPreset->cOPTIONTYPE1 = (BYTE)nOptionType1;
	m_pPreset->cOPTIONTYPE2 = (BYTE)nOptionType2;
	m_pPreset->cOPTIONTYPE3 = (BYTE)nOptionType3;
	m_pPreset->cOPTIONTYPE4 = (BYTE)nOptionType4;
	m_pPreset->nOPTIONVAL1 = (short)nOptionVal1;
	m_pPreset->nOPTIONVAL2 = (short)nOptionVal2;
	m_pPreset->nOPTIONVAL3 = (short)nOptionVal3;
	m_pPreset->nOPTIONVAL4 = (short)nOptionVal4;

	m_pPreset->bNONDROP = bNonDrop;

	return TRUE;
}

void CDlgPresetItemEdit::OnBnClickedOk()
{
	if ( !DataSave() ){
		return;
	}

	CDialog::OnOK();
}
