// CharEditPageItemFood.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "CharEditPageItemFood.h"
#include "CharEditTab.h"
#include "EtcFunction.h"
#include "GLSkill.h"
#include "DlgSkill.h"
#include "GLCharData.h"
#include "HLibDataConvert.h"

// CCharEditPageItemFood dialog

IMPLEMENT_DYNAMIC(CCharEditPageItemFood, CPropertyPage)

CCharEditPageItemFood::CCharEditPageItemFood(LOGFONT logfont, SCHARDATA2* pData)
	: CPropertyPage(CCharEditPageItemFood::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CCharEditPageItemFood::~CCharEditPageItemFood()
{
	SAFE_DELETE ( m_pFont );
}

void CCharEditPageItemFood::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHAREDIT_ITEMFOOD_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CCharEditPageItemFood, CPropertyPage)
	ON_BN_CLICKED(IDC_CHAREDIT_ITEMFOOD_BUTTON_EDIT, OnBnClickedChareditItemfoodButtonEdit)
	ON_BN_CLICKED(IDC_CHAREDIT_ITEMFOOD_BUTTON_DELETE, OnBnClickedChareditItemfoodButtonDelete)
	ON_BN_CLICKED(IDC_CHAREDIT_ITEMFOOD_BUTTON_CLEAR, OnBnClickedChareditItemfoodButtonClear)
	ON_BN_CLICKED(IDC_CHAREDIT_ITEMFOOD_BUTTON_UNLOCK, OnBnClickedChareditItemfoodButtonUnlock)
END_MESSAGE_MAP()


// CCharEditPageItemFood message handlers
BOOL CCharEditPageItemFood::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "Slot", LVCFMT_LEFT, ( 15* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Skill", LVCFMT_LEFT, ( 40* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 2, "Time", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 3, "State", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	UpdateItemFood();

	return TRUE;  
}

void CCharEditPageItemFood::UpdateItemFood()
{
	m_List.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_List.SetRedraw( FALSE );

	for( int i=0; i<FITEMFACT_SIZE; ++i )
	{
		SFITEMFACT sFITEMFACT = m_pData->m_sFITEMFACT[i];
		std::string strNAME = "Unknown Skill";
		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( sFITEMFACT.sNATIVEID );
		if ( pSkill ){
			strNAME = pSkill->GetName();
		}

		m_List.InsertItem( i, _HLIB::cstringformat( "Slot[%02u]", i ).GetString() );
		m_List.SetItemText( i, 1, _HLIB::cstringformat( "[%03d/%03d] %s", sFITEMFACT.sNATIVEID.wMainID, sFITEMFACT.sNATIVEID.wSubID, strNAME.c_str() ).GetString() );
		m_List.SetItemText( i, 2, _HLIB::cstring_floatg(sFITEMFACT.fAGE).GetString() );
		m_List.SetItemText( i, 3, sFITEMFACT.bENABLE? "Enabled" : "Disabled" );
		m_List.SetItemData ( i, i );
	}
	
	m_List.SetRedraw( TRUE );
}

void CCharEditPageItemFood::OnBnClickedChareditItemfoodButtonEdit()
{
	if ( !m_pData )	{
		return;
	}

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected!" );
		return;
	}

	WORD wSlot = (WORD) m_List.GetItemData ( nSelect );
	if ( wSlot >= FITEMFACT_SIZE ){
		return;
	}

	CDlgSkill dlg( this );
	if ( dlg.DoModal() == IDOK )
	{
		SNATIVEID sidSkill = dlg.m_SELECTED;
		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( sidSkill );
		if ( !pSkill )	return;
			
		WORD _wSkillLevel = 0;
		SNATIVEID _sidSKILL = pSkill->m_sBASIC.sNATIVEID;

		BOOL bHOLD = FALSE;
		SFITEMFACT sSKILLEF;
		sSKILLEF.bENABLE = TRUE;

		SKILL::CDATA_LVL &sSKILL_DATA = pSkill->m_sAPPLY.sDATA_LVL[_wSkillLevel]; 
		switch ( pSkill->m_sAPPLY.emBASIC_TYPE )
		{
		case SKILL::EMFOR_VARHP:
		case SKILL::EMFOR_VARMP:
		case SKILL::EMFOR_VARSP:
		case SKILL::EMFOR_DEFENSE:
		case SKILL::EMFOR_HITRATE:
		case SKILL::EMFOR_AVOIDRATE:
		case SKILL::EMFOR_VARAP:
		case SKILL::EMFOR_VARDAMAGE:
		case SKILL::EMFOR_VARDEFENSE:
		case SKILL::EMFOR_PA:
		case SKILL::EMFOR_SA:
		case SKILL::EMFOR_MA:
		case SKILL::EMFOR_RESIST:
		case SKILL::EMFOR_SUMMONTIME: /*summon time, Juver, 2017/12/12 */
			bHOLD = TRUE;
			sSKILLEF.emTYPE = pSkill->m_sAPPLY.emBASIC_TYPE;
			sSKILLEF.fMVAR = sSKILL_DATA.fBASIC_VAR;
			break;
		};

		for( int nImpact=0; nImpact<SKILL::MAX_IMPACT; ++nImpact )
		{
			if ( pSkill->m_sAPPLY.sImpacts[nImpact].emADDON != EMIMPACTA_NONE )
			{
				bHOLD = TRUE;
				sSKILLEF.sImpacts[nImpact].emADDON = pSkill->m_sAPPLY.sImpacts[nImpact].emADDON;
				sSKILLEF.sImpacts[nImpact].fADDON_VAR = pSkill->m_sAPPLY.sImpacts[nImpact].fADDON_VAR[_wSkillLevel];
			}
		}

		for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
		{
			switch ( pSkill->m_sAPPLY.sSpecs[nSpec].emSPEC )
			{
			case EMSPECA_REFDAMAGE:
			case EMSPECA_NONBLOW:
			case EMSPECA_PIERCE:
			case EMSPECA_TARRANGE:
			case EMSPECA_MOVEVELO:
			case EMSPECA_ATTACKVELO:
			case EMSPECA_SKILLDELAY:
			case EMSPECA_PSY_DAMAGE_REDUCE:
			case EMSPECA_MAGIC_DAMAGE_REDUCE:
			case EMSPECA_PSY_DAMAGE_REFLECTION:
			case EMSPECA_MAGIC_DAMAGE_REFLECTION:
			case EMSPECA_DEFENSE_SKILL_ACTIVE:
			case EMSPECA_CHANGE_ATTACK_RANGE:		/*skill range spec logic, Juver, 2017/06/06 */
			case EMSPECA_CHANGE_APPLY_RANGE:		/*skill range spec logic, Juver, 2017/06/06 */
			case EMSPECA_TALK_TO_NPC:				/*npc talk buff, Juver, 2017/10/03 */
				bHOLD = TRUE;
				sSKILLEF.sSpecs[nSpec].emSPEC = pSkill->m_sAPPLY.sSpecs[nSpec].emSPEC;
				sSKILLEF.sSpecs[nSpec].fSPECVAR1 = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[_wSkillLevel].fVAR1;
				sSKILLEF.sSpecs[nSpec].fSPECVAR2 = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[_wSkillLevel].fVAR2;
				sSKILLEF.sSpecs[nSpec].dwSPECFLAG = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[_wSkillLevel].dwFLAG;
				sSKILLEF.sSpecs[nSpec].dwNativeID = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[_wSkillLevel].dwNativeID;
				break;
			};
		}

		if ( !bHOLD )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Skill not buff Type!" );
			return;
		}

		sSKILLEF.sNATIVEID	= pSkill->m_sBASIC.sNATIVEID;
		sSKILLEF.wLEVEL		= _wSkillLevel;
		sSKILLEF.fAGE		= sSKILL_DATA.fLIFE;

		m_pData->m_sFITEMFACT[wSlot] = sSKILLEF;

		UpdateItemFood();
	}
}

void CCharEditPageItemFood::OnBnClickedChareditItemfoodButtonDelete()
{
	if ( !m_pData )	{
		return;
	}

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected!" );
		return;
	}

	WORD wSlot = (WORD) m_List.GetItemData ( nSelect );
	if ( wSlot >= FITEMFACT_SIZE ){
		return;
	}

	m_pData->m_sFITEMFACT[wSlot].RESET();

	UpdateItemFood();
}

void CCharEditPageItemFood::OnBnClickedChareditItemfoodButtonClear()
{
	if ( !m_pData ){
		return;
	}

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All Item Food?" ) == IDYES ){
		for ( int i=0; i<FITEMFACT_SIZE; ++i ){
			m_pData->m_sFITEMFACT[i].RESET();
		}

		UpdateItemFood();
	}
}

void CCharEditPageItemFood::OnBnClickedChareditItemfoodButtonUnlock()
{
	if ( !m_pData ){
		return;
	}

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Unlock Additional slots?" ) == IDYES ){
		for ( int i=FITEMFACT_DEFAULT_SIZE; i<FITEMFACT_SIZE; ++i ){
			m_pData->m_sFITEMFACT[i].bENABLE = TRUE;
		}

		UpdateItemFood();
	}
}
