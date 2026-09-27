// PageEdit2.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkill.h"
#include "PageEdit2.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "GLSkill.h"

#include "DlgSkill.h"
#include "DxEffSinglePropGMan.h"
#include "DxSkinPieceContainer.h"

#include "GLCrowData.h"
#include "DlgCrow.h"

// CPageEdit2 dialog

IMPLEMENT_DYNAMIC(CPageEdit2, CPropertyPage)

CPageEdit2::CPageEdit2(LOGFONT logfont)
	: CPropertyPage(CPageEdit2::IDD),
	m_pFont ( NULL ),
	m_nLevel(0),
	m_nIndexImpact(0),
	m_nIndexSpec(0)
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pSkill = NULL;
	m_pDummySkill = new GLSKILL;
}

CPageEdit2::~CPageEdit2()
{
	SAFE_DELETE ( m_pFont );
	SAFE_DELETE ( m_pDummySkill );
}

void CPageEdit2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageEdit2, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_BACK, OnBnClickedButtonPageBack)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_CANCEL, OnBnClickedButtonPageCancel)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_SAVE, OnBnClickedButtonPageSave)
	ON_BN_CLICKED(IDC_BUTTON_LEVEL_MINUS, OnBnClickedButtonLevelMinus)
	ON_BN_CLICKED(IDC_BUTTON_LEVEL_ADD, OnBnClickedButtonLevelAdd)
	ON_BN_CLICKED(IDC_BUTTON_SKILL, OnBnClickedButtonSkill)
	ON_BN_CLICKED(IDC_BUTTON_SPS_EFFECT, OnBnClickedButtonSpsEffect)
	ON_BN_CLICKED(IDC_BUTTON_HEAD_M, OnBnClickedButtonHeadM)
	ON_BN_CLICKED(IDC_BUTTON_UPPER_M, OnBnClickedButtonUpperM)
	ON_BN_CLICKED(IDC_BUTTON_LOWER_M, OnBnClickedButtonLowerM)
	ON_BN_CLICKED(IDC_BUTTON_HAND_M, OnBnClickedButtonHandM)
	ON_BN_CLICKED(IDC_BUTTON_LHAND_M, OnBnClickedButtonLhandM)
	ON_BN_CLICKED(IDC_BUTTON_RHAND_M, OnBnClickedButtonRhandM)
	ON_BN_CLICKED(IDC_BUTTON_FOOT_M, OnBnClickedButtonFootM)
	ON_BN_CLICKED(IDC_BUTTON_FACE_M, OnBnClickedButtonFaceM)
	ON_BN_CLICKED(IDC_BUTTON_HEAD_F, OnBnClickedButtonHeadF)
	ON_BN_CLICKED(IDC_BUTTON_UPPER_F, OnBnClickedButtonUpperF)
	ON_BN_CLICKED(IDC_BUTTON_LOWER_F, OnBnClickedButtonLowerF)
	ON_BN_CLICKED(IDC_BUTTON_HAND_F, OnBnClickedButtonHandF)
	ON_BN_CLICKED(IDC_BUTTON_LHAND_F, OnBnClickedButtonLhandF)
	ON_BN_CLICKED(IDC_BUTTON_RHAND_F, OnBnClickedButtonRhandF)
	ON_BN_CLICKED(IDC_BUTTON_FOOT_F, OnBnClickedButtonFootF)
	ON_BN_CLICKED(IDC_BUTTON_FACE_F, OnBnClickedButtonFaceF)
	ON_EN_CHANGE(IDC_EDIT_AS_MID, OnEnChangeEditAsMid)
	ON_EN_CHANGE(IDC_EDIT_AS_SID, OnEnChangeEditAsSid)
	ON_EN_CHANGE(IDC_EDIT_LS_MID, OnEnChangeEditLsMid)
	ON_EN_CHANGE(IDC_EDIT_LS_SID, OnEnChangeEditLsSid)
	ON_BN_CLICKED(IDC_BUTTON_IMPACT_MINUS, OnBnClickedButtonImpactMinus)
	ON_BN_CLICKED(IDC_BUTTON_IMPACT_ADD, OnBnClickedButtonImpactAdd)
	ON_BN_CLICKED(IDC_BUTTON_SPEC_MINUS, OnBnClickedButtonSpecMinus)
	ON_BN_CLICKED(IDC_BUTTON_SPEC_ADD2, OnBnClickedButtonSpecAdd2)
	ON_BN_CLICKED(IDC_BUTTON_SUM_CROW, OnBnClickedButtonSumCrow)
	ON_EN_CHANGE(IDC_EDIT_SUM_MID, OnEnChangeEditSumMid)
	ON_EN_CHANGE(IDC_EDIT_SUM_SID, OnEnChangeEditSumSid)
	ON_BN_CLICKED(IDC_BUTTON_LS_SKILL, &CPageEdit2::OnBnClickedButtonLsSkill)
	ON_BN_CLICKED(IDC_CHECK_CURE_NUMB, &CPageEdit2::OnBnClickedCheckCureNumb)
	ON_BN_CLICKED(IDC_CHECK_CURE_STUN, &CPageEdit2::OnBnClickedCheckCureStun)
	ON_BN_CLICKED(IDC_CHECK_CURE_STONE, &CPageEdit2::OnBnClickedCheckCureStone)
	ON_BN_CLICKED(IDC_CHECK_CURE_BURN, &CPageEdit2::OnBnClickedCheckCureBurn)
	ON_BN_CLICKED(IDC_CHECK_CURE_FROZEN, &CPageEdit2::OnBnClickedCheckCureFrozen)
	ON_BN_CLICKED(IDC_CHECK_CURE_MAD, &CPageEdit2::OnBnClickedCheckCureMad)
	ON_BN_CLICKED(IDC_CHECK_CURE_POISON, &CPageEdit2::OnBnClickedCheckCurePoison)
	ON_BN_CLICKED(IDC_CHECK_CURE_CURSE, &CPageEdit2::OnBnClickedCheckCureCurse)
END_MESSAGE_MAP()


// CPageEdit2 message handlers
BOOL CPageEdit2::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	DataDefault();
	DataShow();		

	return TRUE;  
}

BOOL CPageEdit2::SkillSet ( PGLSKILL pSKILL )
{
	if ( m_pSkill )	return FALSE;
	m_pSkill = pSKILL;
	*m_pDummySkill = *m_pSkill;

	if ( m_bDlgInit ) 
	{
		DataDefault();
		DataShow();	
	}

	return TRUE;
}

void CPageEdit2::SkillClear()
{
	m_pSkill = NULL;
}

void CPageEdit2::OnBnClickedButtonPageBack()
{
	if ( !DataSave() )	return;

	*m_pSkill = *m_pDummySkill;
	m_pSheetTab->ActivePageEdit1( m_CallPage, m_pSkill );
	m_pSkill = NULL;
}

void CPageEdit2::OnBnClickedButtonPageCancel()
{
	m_pSkill = NULL;
	if ( m_pSheetTab )
		m_pSheetTab->ActivePageTree( FALSE );
}

void CPageEdit2::OnBnClickedButtonPageSave()
{
	if ( !DataSave() )	return;

	*m_pSkill = *m_pDummySkill;
	m_pSkill = NULL;
	m_pSheetTab->ActivePageTree( FALSE );
}

void CPageEdit2::OnBnClickedButtonLevelMinus()
{
	LevelSave();
	m_nLevel --;
	if ( m_nLevel <= 0 )
		m_nLevel = 0;
	LevelShow();
}

void CPageEdit2::OnBnClickedButtonLevelAdd()
{
	LevelSave();
	m_nLevel ++;
	if ( m_nLevel >= SKILL::MAX_LEVEL )
		m_nLevel --;
	LevelShow();
}

void CPageEdit2::InitStateBlow()
{
	if ( !m_pDummySkill )	return;

	EMSTATE_BLOW emTYPE = m_pDummySkill->m_sAPPLY.emSTATE_BLOW;

	CString strText;
	strText.Format( "Abnormal Condition: %s", COMMENT::BLOW[emTYPE].c_str() );
	SetWin_Text ( this, IDC_STATIC_BLOW, strText.GetString() );

	BOOL bSTATE_BLOW = ( emTYPE!=EMBLOW_NONE );
	BOOL bBLOW_VAR1(TRUE), bBLOW_VAR2(TRUE);
	const char *szBLOW_VAR1 = COMMENT::BLOW_VAR1[emTYPE].c_str();
	const char *szBLOW_VAR2 = COMMENT::BLOW_VAR2[emTYPE].c_str();
	if ( !strcmp(szBLOW_VAR1, COMMENT::BLOW_VAR1[EMBLOW_NONE].c_str()) )		bBLOW_VAR1 = FALSE;
	if ( !strcmp(szBLOW_VAR2, COMMENT::BLOW_VAR2[EMBLOW_NONE].c_str()) )		bBLOW_VAR2 = FALSE;

	SetWin_Enable ( this, IDC_EDIT_BLOW_GEN, bSTATE_BLOW );
	SetWin_Enable ( this, IDC_EDIT_BLOW_VAR1, bBLOW_VAR1 );
	SetWin_Enable ( this, IDC_EDIT_BLOW_VAR2, bBLOW_VAR2 );

	SetWin_Text ( this, IDC_STATIC_BLOW_1, "Gen Percentage" );
	SetWin_Text ( this, IDC_STATIC_BLOW_2, szBLOW_VAR1 );
	SetWin_Text ( this, IDC_STATIC_BLOW_3, szBLOW_VAR2 );
}

void CPageEdit2::InitImpact()
{
	if ( !m_pDummySkill )	return;
	if ( m_nIndexImpact < 0 )	return;
	if ( m_nIndexImpact >= SKILL::MAX_IMPACT )	return;

	EMIMPACT_ADDON emIMPACT = m_pDummySkill->m_sAPPLY.sImpacts[m_nIndexImpact].emADDON;

	CString strText;
	strText.Format( "Impact AddOn: %s", COMMENT::IMPACT_ADDON[emIMPACT].c_str() );
	SetWin_Text ( this, IDC_STATIC_IMPACT, strText.GetString() );
}

void CPageEdit2::InitSpec()
{
	if ( !m_pDummySkill )	return;
	if ( m_nIndexSpec < 0 )		return;
	if ( m_nIndexSpec >= SKILL::MAX_SPEC )	return;


	SetWin_Enable( this, IDC_CHECK_CURE_NUMB, false );
	SetWin_Enable( this, IDC_CHECK_CURE_STUN, false );
	SetWin_Enable( this, IDC_CHECK_CURE_STONE, false );
	SetWin_Enable( this, IDC_CHECK_CURE_BURN, false );
	SetWin_Enable( this, IDC_CHECK_CURE_FROZEN, false );
	SetWin_Enable( this, IDC_CHECK_CURE_MAD, false );
	SetWin_Enable( this, IDC_CHECK_CURE_POISON, false );
	SetWin_Enable( this, IDC_CHECK_CURE_CURSE, false );

	SetWin_Check( this, IDC_CHECK_CURE_NUMB, false );
	SetWin_Check( this, IDC_CHECK_CURE_STUN, false );
	SetWin_Check( this, IDC_CHECK_CURE_STONE, false );
	SetWin_Check( this, IDC_CHECK_CURE_BURN, false );
	SetWin_Check( this, IDC_CHECK_CURE_FROZEN, false );
	SetWin_Check( this, IDC_CHECK_CURE_MAD, false );
	SetWin_Check( this, IDC_CHECK_CURE_POISON, false );
	SetWin_Check( this, IDC_CHECK_CURE_CURSE, false );

	//enable or disable some controls differ by spec type
	EMSPEC_ADDON emSPEC = m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].emSPEC;

	CString strText;
	strText.Format( "Spec AddOn: %s", COMMENT::SPEC_ADDON[emSPEC].c_str() );
	SetWin_Text ( this, IDC_STATIC_SPEC, strText.GetString() );

	BOOL bSPEC_VAR1(TRUE), bSPEC_VAR2(TRUE);
	const char *szSPEC_VAR1 = COMMENT::SPEC_ADDON_VAR1[emSPEC].c_str();
	const char *szSPEC_VAR2 = COMMENT::SPEC_ADDON_VAR2[emSPEC].c_str();
	if ( !strcmp(szSPEC_VAR1, COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL].c_str()) )		bSPEC_VAR1 = FALSE;
	if ( !strcmp(szSPEC_VAR2, COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL].c_str()) )		bSPEC_VAR2 = FALSE;

	SetWin_Enable ( this, IDC_EDIT_SPEC_VAR1, bSPEC_VAR1 );
	SetWin_Enable ( this, IDC_EDIT_SPEC_VAR2, bSPEC_VAR2 );

	SetWin_Text ( this, IDC_STATIC_SPEC_1, szSPEC_VAR1 );
	SetWin_Text ( this, IDC_STATIC_SPEC_2, szSPEC_VAR2 );

	BOOL bCURESPEC = ( emSPEC == EMSPECA_NONBLOW || emSPEC == EMSPECA_RECBLOW );
	if ( bCURESPEC )
	{
		SetWin_Enable( this, IDC_CHECK_CURE_NUMB, bCURESPEC );
		SetWin_Enable( this, IDC_CHECK_CURE_STUN, bCURESPEC );
		SetWin_Enable( this, IDC_CHECK_CURE_STONE, bCURESPEC );
		SetWin_Enable( this, IDC_CHECK_CURE_BURN, bCURESPEC );
		SetWin_Enable( this, IDC_CHECK_CURE_FROZEN, bCURESPEC );
		SetWin_Enable( this, IDC_CHECK_CURE_MAD, bCURESPEC );
		SetWin_Enable( this, IDC_CHECK_CURE_POISON, bCURESPEC );
		SetWin_Enable( this, IDC_CHECK_CURE_CURSE, bCURESPEC );
	}
	BOOL bACTIVESKILL = ( emSPEC == EMSPECA_DEFENSE_SKILL_ACTIVE );
	SetWin_Enable( this, IDC_EDIT_AS_MID, bACTIVESKILL );
	SetWin_Enable( this, IDC_EDIT_AS_SID, bACTIVESKILL );
	SetWin_Enable( this, IDC_EDIT_AS_LVL, bACTIVESKILL );
	SetWin_Enable( this, IDC_BUTTON_SKILL, bACTIVESKILL );

	/*debuff skill logic, Juver, 2017/06/10 */
	BOOL bDEBUFF = ( emSPEC == EMSPECA_BUFF_REMOVE );
	SetWin_Enable( this, IDC_STATIC_DEBUFF, bDEBUFF );
	SetWin_Enable( this, IDC_COMBO_DEBUFF_ACTION, bDEBUFF );

	/*dash skill logic, Juver, 2017/06/17 */
	BOOL bUSEACTION  = ( emSPEC == EMSPECA_ONWARD );
	SetWin_Enable( this, IDC_STATIC_ACTION, bUSEACTION );
	SetWin_Enable( this, IDC_COMBO_ANI_SUB2, bUSEACTION );
		
	BOOL bDAMAGE = 0;

	SetWin_Enable( this, IDC_STATIC_SKILL_MID, bDAMAGE);
	SetWin_Enable( this, IDC_STATIC_SKILL_SID, bDAMAGE);
	SetWin_Enable( this, IDC_EDIT_SKILL_MID  , bDAMAGE);
	SetWin_Enable( this, IDC_EDIT_SKILL_SID  , bDAMAGE);

	SetWin_Enable( this, IDC_EDIT_SPEC_VAR3  , !bDAMAGE);
	SetWin_Enable( this, IDC_EDIT_SPEC_VAR4  , !bDAMAGE);

	
	BOOL bSKILLLINK = ( emSPEC == EMSPECA_SKILLLINK );
	SetWin_Enable( this, IDC_EDIT_AS_MID, bSKILLLINK );
	SetWin_Enable( this, IDC_EDIT_AS_SID, bSKILLLINK );

	SetWin_Enable( this, IDC_EDIT_AS_MID, bSKILLLINK );
	SetWin_Enable( this, IDC_EDIT_AS_SID, bSKILLLINK );

	SetWin_Enable( this, IDC_EDIT_LS_MID, bSKILLLINK );
	SetWin_Enable( this, IDC_EDIT_LS_SID, bSKILLLINK );
	
	BOOL bENHANCEMENT = ( emSPEC == EMSPECA_ENHANCEMENT && !bCURESPEC );
	if ( bENHANCEMENT )
	{
		SetWin_Enable( this, IDC_CHECK_CURE_NUMB, bENHANCEMENT );
		SetWin_Enable( this, IDC_CHECK_CURE_STUN, bENHANCEMENT );
		SetWin_Enable( this, IDC_CHECK_CURE_STONE, bENHANCEMENT );
	}

	BOOL bSWAPPOS = ( emSPEC == EMSPECA_POSITION_SHIFT );
	if ( bSWAPPOS )
	{
		SetWin_Enable( this, IDC_CHECK_CURE_NUMB, bSWAPPOS ); 
		SetWin_Enable( this, IDC_CHECK_CURE_STUN, bSWAPPOS ); 
		SetWin_Enable( this, IDC_CHECK_CURE_STONE, bSWAPPOS ); 

		SetWin_Text( this, IDC_CHECK_CURE_NUMB, COMMENT::SPEC_SWAPPOS_DIR_TYPE[SKILL::EMSPEC_SWAPPOS_DIR_TYPE_ALL].c_str() );
		SetWin_Text( this, IDC_CHECK_CURE_STUN, COMMENT::SPEC_SWAPPOS_DIR_TYPE[SKILL::EMSPEC_SWAPPOS_DIR_TYPE_SELF].c_str() );
		SetWin_Text( this, IDC_CHECK_CURE_STONE, COMMENT::SPEC_SWAPPOS_DIR_TYPE[SKILL::EMSPEC_SWAPPOS_DIR_TYPE_TARGET].c_str() );
	}
	
	BOOL bSkilldelay = ( emSPEC == EMSPECA_SKILLDELAY );
	if ( bSkilldelay )
	{
		SetWin_Enable( this, IDC_CHECK_CURE_NUMB, bSkilldelay ); 
		SetWin_Enable( this, IDC_CHECK_CURE_STUN, bSkilldelay ); 
	}
}

void CPageEdit2::CheckActiveSkill()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_AS_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_AS_SID );
	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( wMID, wSID );
	SetWin_Text( this, IDC_EDIT_AS_NAME, pSKILL? pSKILL->GetName() : "Unknown Skill" );
}

void CPageEdit2::CheckLinkSkill()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_LS_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_LS_SID );
	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( wMID, wSID );
	SetWin_Text( this, IDC_EDIT_LS_NAME, pSKILL? pSKILL->GetName() : "Unknown Skill" );
}

void CPageEdit2::OnBnClickedButtonSkill()
{
	CDlgSkill dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SNATIVEID sID = dlg.m_SELECTED;
		SetWin_Num_int( this, IDC_EDIT_AS_MID, sID.wMainID );
		SetWin_Num_int( this, IDC_EDIT_AS_SID, sID.wSubID );
		CheckActiveSkill();
	}
}

void CPageEdit2::InitSpecialSkill()
{
	if ( !m_pDummySkill )	return;

	SKILL::EMSPECIALSKILLTYPE	emSSTYPE = m_pDummySkill->m_sSPECIAL_SKILL.emSSTYPE;

	BOOL bTRANSFORM = ( emSSTYPE == SKILL::EMSSTYPE_TRANSFORM );
	SetWin_Enable( this, IDC_EDIT_HEAD_M, bTRANSFORM );
	SetWin_Enable( this, IDC_EDIT_UPPER_M, bTRANSFORM );
	SetWin_Enable( this, IDC_EDIT_LOWER_M, bTRANSFORM );
	SetWin_Enable( this, IDC_EDIT_HAND_M, bTRANSFORM );
	SetWin_Enable( this, IDC_EDIT_LHAND_M, bTRANSFORM );
	SetWin_Enable( this, IDC_EDIT_RHAND_M, bTRANSFORM );
	SetWin_Enable( this, IDC_EDIT_FOOT_M, bTRANSFORM );
	SetWin_Enable( this, IDC_EDIT_FACE_M, bTRANSFORM );
	SetWin_Enable( this, IDC_EDIT_HEAD_F, bTRANSFORM );
	SetWin_Enable( this, IDC_EDIT_UPPER_F, bTRANSFORM );
	SetWin_Enable( this, IDC_EDIT_LOWER_F, bTRANSFORM );
	SetWin_Enable( this, IDC_EDIT_HAND_F, bTRANSFORM );
	SetWin_Enable( this, IDC_EDIT_LHAND_F, bTRANSFORM );
	SetWin_Enable( this, IDC_EDIT_RHAND_F, bTRANSFORM );
	SetWin_Enable( this, IDC_EDIT_FOOT_F, bTRANSFORM );
	SetWin_Enable( this, IDC_EDIT_FACE_F, bTRANSFORM );
	SetWin_Enable( this, IDC_BUTTON_HEAD_M, bTRANSFORM );
	SetWin_Enable( this, IDC_BUTTON_UPPER_M, bTRANSFORM );
	SetWin_Enable( this, IDC_BUTTON_LOWER_M, bTRANSFORM );
	SetWin_Enable( this, IDC_BUTTON_HAND_M, bTRANSFORM );
	SetWin_Enable( this, IDC_BUTTON_LHAND_M, bTRANSFORM );
	SetWin_Enable( this, IDC_BUTTON_RHAND_M, bTRANSFORM );
	SetWin_Enable( this, IDC_BUTTON_FOOT_M, bTRANSFORM );
	SetWin_Enable( this, IDC_BUTTON_FACE_M, bTRANSFORM );
	SetWin_Enable( this, IDC_BUTTON_HEAD_F, bTRANSFORM );
	SetWin_Enable( this, IDC_BUTTON_UPPER_F, bTRANSFORM );
	SetWin_Enable( this, IDC_BUTTON_LOWER_F, bTRANSFORM );
	SetWin_Enable( this, IDC_BUTTON_HAND_F, bTRANSFORM );
	SetWin_Enable( this, IDC_BUTTON_LHAND_F, bTRANSFORM );
	SetWin_Enable( this, IDC_BUTTON_RHAND_F, bTRANSFORM );
	SetWin_Enable( this, IDC_BUTTON_FOOT_F, bTRANSFORM );
	SetWin_Enable( this, IDC_BUTTON_FACE_F, bTRANSFORM );

	BOOL bSUMMON = ( emSSTYPE == SKILL::EMSSTYPE_SUMMON );
	SetWin_Enable( this, IDC_EDIT_SUM_MID, bSUMMON );
	SetWin_Enable( this, IDC_EDIT_SUM_SID, bSUMMON );
	SetWin_Enable( this, IDC_EDIT_SUM_DELAY, bSUMMON );
	SetWin_Enable( this, IDC_EDIT_SUM_LIFE, bSUMMON );
	SetWin_Enable( this, IDC_EDIT_SUM_NUM, bSUMMON );
	SetWin_Enable( this, IDC_EDIT_SUM_SKILLLEVEL, bSUMMON );
	SetWin_Enable( this, IDC_BUTTON_SUM_CROW, bSUMMON );
}

void CPageEdit2::OnBnClickedButtonSpsEffect()
{
	Loadegp( IDC_EDIT_SPS_EFFECT );
}

void CPageEdit2::OnBnClickedButtonHeadM()
{
	LoadPiece( IDC_EDIT_HEAD_M );
}

void CPageEdit2::OnBnClickedButtonUpperM()
{
	LoadPiece( IDC_EDIT_UPPER_M );
}

void CPageEdit2::OnBnClickedButtonLowerM()
{
	LoadPiece( IDC_EDIT_LOWER_M );
}

void CPageEdit2::OnBnClickedButtonHandM()
{
	LoadPiece( IDC_EDIT_HAND_M );
}

void CPageEdit2::OnBnClickedButtonLhandM()
{
	LoadPiece( IDC_EDIT_LHAND_M );
}

void CPageEdit2::OnBnClickedButtonRhandM()
{
	LoadPiece( IDC_EDIT_RHAND_M );
}

void CPageEdit2::OnBnClickedButtonFootM()
{
	LoadPiece( IDC_EDIT_FOOT_M );
}

void CPageEdit2::OnBnClickedButtonFaceM()
{
	LoadPiece( IDC_EDIT_FACE_M );
}

void CPageEdit2::OnBnClickedButtonHeadF()
{
	LoadPiece( IDC_EDIT_HEAD_F );
}

void CPageEdit2::OnBnClickedButtonUpperF()
{
	LoadPiece( IDC_EDIT_UPPER_F );
}

void CPageEdit2::OnBnClickedButtonLowerF()
{
	LoadPiece( IDC_EDIT_LOWER_F );
}

void CPageEdit2::OnBnClickedButtonHandF()
{
	LoadPiece( IDC_EDIT_HAND_F );
}

void CPageEdit2::OnBnClickedButtonLhandF()
{
	LoadPiece( IDC_EDIT_LHAND_F );
}

void CPageEdit2::OnBnClickedButtonRhandF()
{
	LoadPiece( IDC_EDIT_RHAND_F );
}

void CPageEdit2::OnBnClickedButtonFootF()
{
	LoadPiece( IDC_EDIT_FOOT_F );
}

void CPageEdit2::OnBnClickedButtonFaceF()
{
	LoadPiece( IDC_EDIT_FACE_F );
}

void CPageEdit2::Loadegp( int nID )
{
	CString szFilter = "Effect File (*.egp)|*.egp|";
	CFileDialog dlg(TRUE,".egp",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = DxEffSinglePropGMan::GetInstance().GetPath ();

	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, nID, dlg.GetFileName().GetString() );
}

void CPageEdit2::LoadPiece( int nID )
{
	CString szFilter = "Wearing File (*.cps)|*.cps|";
	CFileDialog dlg(TRUE,".cps",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );

	dlg.m_ofn.lpstrInitialDir = DxSkinPieceContainer::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, nID, dlg.GetFileName().GetString() );
}

void CPageEdit2::OnEnChangeEditAsMid()
{
	CheckActiveSkill();
}

void CPageEdit2::OnEnChangeEditAsSid()
{
	CheckActiveSkill();
}

void CPageEdit2::OnEnChangeEditLsMid()
{
	CheckLinkSkill();
}

void CPageEdit2::OnEnChangeEditLsSid()
{
	CheckLinkSkill();
}


void CPageEdit2::OnBnClickedButtonImpactMinus()
{
	ImpactDataSave();

	m_nIndexImpact --;
	if ( m_nIndexImpact <= 0 )
		m_nIndexImpact = 0;

	ImpactDataShow();
}

void CPageEdit2::OnBnClickedButtonImpactAdd()
{
	ImpactDataSave();

	m_nIndexImpact ++;
	if ( m_nIndexImpact >= SKILL::MAX_IMPACT )
		m_nIndexImpact--;

	ImpactDataShow();
}

void CPageEdit2::OnBnClickedButtonSpecMinus()
{
	SpecDataSave();

	m_nIndexSpec --;
	if ( m_nIndexSpec <=0 )
		m_nIndexSpec = 0;

	SpecDataShow();
}

void CPageEdit2::OnBnClickedButtonSpecAdd2()
{
	SpecDataSave();

	m_nIndexSpec ++;
	if ( m_nIndexSpec >= SKILL::MAX_SPEC )
		m_nIndexSpec--;

	SpecDataShow();
}

void CPageEdit2::OnBnClickedButtonSumCrow()
{
	CDlgCrow dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SNATIVEID sID = dlg.m_SELECTED;
		SetWin_Num_int( this, IDC_EDIT_SUM_MID, sID.wMainID );
		SetWin_Num_int( this, IDC_EDIT_SUM_SID, sID.wSubID );
		CheckSummonCrow();
	}
}

void CPageEdit2::OnEnChangeEditSumMid()
{
	CheckSummonCrow();
}

void CPageEdit2::OnEnChangeEditSumSid()
{
	CheckSummonCrow();
}

void CPageEdit2::CheckSummonCrow()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_SUM_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_SUM_SID );
	SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData( wMID, wSID );
	SetWin_Text( this, IDC_EDIT_SUM_CROWNAME, pCROW? pCROW->GetName() : "Unknown Crow" );
}

void CPageEdit2::OnBnClickedButtonLsSkill()
{
	CDlgSkill dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SNATIVEID sID = dlg.m_SELECTED;
		SetWin_Num_int( this, IDC_EDIT_LS_MID, sID.wMainID );
		SetWin_Num_int( this, IDC_EDIT_LS_SID, sID.wSubID );
		CheckLinkSkill();
	}
}

void CPageEdit2::OnBnClickedCheckCureNumb()
{
	LevelSave();
	LevelShow();
}

void CPageEdit2::OnBnClickedCheckCureStun()
{
	LevelSave();
	LevelShow();
}

void CPageEdit2::OnBnClickedCheckCureStone()
{
	LevelSave();
	LevelShow();
}

void CPageEdit2::OnBnClickedCheckCureBurn()
{
	LevelSave();
	LevelShow();
}

void CPageEdit2::OnBnClickedCheckCureFrozen()
{
	LevelSave();
	LevelShow();
}

void CPageEdit2::OnBnClickedCheckCureMad()
{
	LevelSave();
	LevelShow();
}

void CPageEdit2::OnBnClickedCheckCurePoison()
{
	LevelSave();
	LevelShow();
}

void CPageEdit2::OnBnClickedCheckCureCurse()
{
	LevelSave();
	LevelShow();
}
