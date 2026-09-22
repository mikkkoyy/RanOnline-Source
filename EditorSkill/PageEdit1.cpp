// PageEdit1.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkill.h"
#include "PageEdit1.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "GLSkill.h"
#include "GLStringTable.h"
#include "DxEffcharData.h"
#include "TextureManager.h"
#include "DlgSkill.h"

// CPageEdit1 dialog

IMPLEMENT_DYNAMIC(CPageEdit1, CPropertyPage)

CPageEdit1::CPageEdit1(LOGFONT logfont)
	: CPropertyPage(CPageEdit1::IDD),
	m_pFont ( NULL ),
	m_nElement(0),
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

CPageEdit1::~CPageEdit1()
{
	SAFE_DELETE ( m_pFont );
	SAFE_DELETE ( m_pDummySkill );
}

void CPageEdit1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageEdit1, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_NEXT, OnBnClickedButtonPageNext)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_CANCEL, OnBnClickedButtonPageCancel)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_SAVE, OnBnClickedButtonPageSave)
	ON_CBN_SELCHANGE(IDC_COMBO_ANI_MAIN, OnCbnSelchangeComboAniMain)
	ON_CBN_SELCHANGE(IDC_COMBO_ANI_SUB, OnCbnSelchangeComboAniSub)
	ON_BN_CLICKED(IDC_BUTTON_EFFECT_MINUS, OnBnClickedButtonEffectMinus)
	ON_BN_CLICKED(IDC_BUTTON_EFFECT_ADD, OnBnClickedButtonEffectAdd)
	ON_BN_CLICKED(IDC_BUTTON_EFF_TARGET1, OnBnClickedButtonEffTarget1)
	ON_BN_CLICKED(IDC_BUTTON_EFF_TARGET2, OnBnClickedButtonEffTarget2)
	ON_BN_CLICKED(IDC_BUTTON_EFF_SELF1, OnBnClickedButtonEffSelf1)
	ON_BN_CLICKED(IDC_BUTTON_EFF_SELF2, OnBnClickedButtonEffSelf2)
	ON_BN_CLICKED(IDC_BUTTON_EFF_TARG, OnBnClickedButtonEffTarg)
	ON_BN_CLICKED(IDC_BUTTON_EFF_SELFBODY, OnBnClickedButtonEffSelfbody)
	ON_BN_CLICKED(IDC_BUTTON_EFF_TARGETBODY1, OnBnClickedButtonEffTargetbody1)
	ON_BN_CLICKED(IDC_BUTTON_EFF_TARGETBODY2, OnBnClickedButtonEffTargetbody2)
	ON_BN_CLICKED(IDC_BUTTON_EFF_HOLDOUT, OnBnClickedButtonEffHoldout)
	ON_CBN_SELCHANGE(IDC_COMBO_EMELEMENT, OnCbnSelchangeComboEmelement)
	ON_CBN_SELCHANGE(IDC_COMBO_EMTYPES, OnCbnSelchangeComboEmtypes)
	ON_BN_CLICKED(IDC_BUTTON_ICON, OnBnClickedButtonIcon)
	ON_BN_CLICKED(IDC_BUTTON_SKLVL_MINUS,OnBnClickedButtonSklvlMinus)
	ON_BN_CLICKED(IDC_BUTTON_SKLVL_ADD, OnBnClickedButtonSklvlAdd)
	ON_BN_CLICKED(IDC_BUTTON_SKILL, OnBnClickedButtonSkill)
	ON_EN_CHANGE(IDC_EDIT_REQSKILL_MID, OnEnChangeEditReqskillMid)
	ON_EN_CHANGE(IDC_EDIT_REQSKILL_SID, OnEnChangeEditReqskillSid)
	ON_BN_CLICKED(IDC_BUTTON_IMPACT_MINUS, OnBnClickedButtonImpactMinus)
	ON_BN_CLICKED(IDC_BUTTON_IMPACT_ADD, OnBnClickedButtonImpactAdd)
	ON_BN_CLICKED(IDC_BUTTON_SPEC_MINUS, OnBnClickedButtonSpecMinus)
	ON_BN_CLICKED(IDC_BUTTON_SPEC_ADD, OnBnClickedButtonSpecAdd)
	ON_BN_CLICKED(IDC_BUTTON_EFF_TARGET3, OnBnClickedButtonEffTarget3)
	ON_BN_CLICKED(IDC_BUTTON_EFF_SELF3, OnBnClickedButtonEffSelf3)
	ON_BN_CLICKED(IDC_CHECK_HIDDENWEAPON, OnBnClickedCheckHiddenWeapon)
END_MESSAGE_MAP()


// CPageEdit1 message handlers
BOOL CPageEdit1::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	DataDefault();
	DataShow();		

	return TRUE;  
}

BOOL CPageEdit1::SkillSet ( PGLSKILL pSKILL )
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

void CPageEdit1::SkillClear()
{
	m_pSkill = NULL;
}

void CPageEdit1::OnBnClickedButtonPageNext()
{
	if ( !DataSave() )	return;

	*m_pSkill = *m_pDummySkill;

	std::string strNAME = m_pDummySkill->m_sBASIC.szNAME;
	std::string strDESC = m_pDummySkill->m_sEXT_DATA.strCOMMENTS.c_str();
	std::string strNAME_NEW = GetWin_Text(this, IDC_EDIT_NAME2 ).GetString();
	std::string strDESC_NEW  = GetWin_Text(this, IDC_EDIT_COMMENT2 ).GetString();
	GLStringTable::GetInstance().InsertString ( strNAME, strNAME_NEW, GLStringTable::SKILL );
	GLStringTable::GetInstance().InsertString ( strDESC, strDESC_NEW, GLStringTable::SKILL );

	m_pSheetTab->ActivePageEdit2( m_CallPage, m_pSkill );
	m_pSkill = NULL;
}

void CPageEdit1::OnBnClickedButtonPageCancel()
{
	m_pSkill = NULL;
	if ( m_pSheetTab )
		m_pSheetTab->ActivePageTree( FALSE );
}

void CPageEdit1::OnBnClickedButtonPageSave()
{
	if ( !DataSave() )	return;

	*m_pSkill = *m_pDummySkill;

	std::string strNAME = m_pDummySkill->m_sBASIC.szNAME;
	std::string strDESC = m_pDummySkill->m_sEXT_DATA.strCOMMENTS.c_str();
	std::string strNAME_NEW = GetWin_Text(this, IDC_EDIT_NAME2 ).GetString();
	std::string strDESC_NEW  = GetWin_Text(this, IDC_EDIT_COMMENT2 ).GetString();
	GLStringTable::GetInstance().InsertString ( strNAME, strNAME_NEW, GLStringTable::SKILL );
	GLStringTable::GetInstance().InsertString ( strDESC, strDESC_NEW, GLStringTable::SKILL );

	m_pSkill = NULL;
	m_pSheetTab->ActivePageTree( FALSE );
}

void CPageEdit1::CheckReqSkill()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_REQSKILL_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_REQSKILL_SID );
	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( wMID, wSID );
	SetWin_Text( this, IDC_EDIT_REQSKILL_NAME, pSKILL? pSKILL->GetName() : "Unknown Skill" );
}

void CPageEdit1::ShowAnimationNumber()
{
	EMANI_MAINTYPE emMain = static_cast<EMANI_MAINTYPE> ( GetWin_Combo_Sel( this, IDC_COMBO_ANI_MAIN ) );
	EMANI_SUBTYPE emSub = static_cast<EMANI_SUBTYPE> ( GetWin_Combo_Sel( this, IDC_COMBO_ANI_SUB ) );
	SetWin_Num_int( this, IDC_EDIT_ANI_MAIN, emMain );
	SetWin_Num_int( this, IDC_EDIT_ANI_SUB, emSub );
}

void CPageEdit1::InitBasicType()
{
	SKILL::EMTYPES emTYPE = static_cast<SKILL::EMTYPES> ( GetWin_Combo_Sel( this, IDC_COMBO_EMTYPES ) );
	
	BOOL bCURE = ( emTYPE == SKILL::EMFOR_CURE );
	SetWin_Enable ( this, IDC_CHECK_CURE_NUMB, bCURE );
	SetWin_Enable ( this, IDC_CHECK_CURE_STUN, bCURE );
	SetWin_Enable ( this, IDC_CHECK_CURE_STONE, bCURE );
	SetWin_Enable ( this, IDC_CHECK_CURE_BURN, bCURE );
	SetWin_Enable ( this, IDC_CHECK_CURE_FROZEN, bCURE );
	SetWin_Enable ( this, IDC_CHECK_CURE_MAD, bCURE );
	SetWin_Enable ( this, IDC_CHECK_CURE_POISON, bCURE );
	SetWin_Enable ( this, IDC_CHECK_CURE_CURSE, bCURE );
}

void CPageEdit1::OnCbnSelchangeComboAniMain()
{
	ShowAnimationNumber();
}

void CPageEdit1::OnCbnSelchangeComboAniSub()
{
	ShowAnimationNumber();
}

void CPageEdit1::OnBnClickedButtonEffectMinus()
{
	EffectsSave();
	m_nElement --;
	if ( m_nElement <= EMELEMENT_SPIRIT )
		m_nElement = 0;
	EffectsShow();
}

void CPageEdit1::OnBnClickedButtonEffectAdd()
{
	EffectsSave();
	m_nElement ++;
	if ( m_nElement >= EMELEMENT_MAXNUM )
		m_nElement --;
	EffectsShow();
}

void CPageEdit1::OnBnClickedButtonEffTarget1()
{
	Loadegp( IDC_EDIT_EFF_TARGET1 );
}

void CPageEdit1::OnBnClickedButtonEffTarget2()
{
	Loadegp( IDC_EDIT_EFF_TARGET2 );
}

void CPageEdit1::OnBnClickedButtonEffSelf1()
{
	Loadegp( IDC_EDIT_EFF_SELF1 );
}

void CPageEdit1::OnBnClickedButtonEffSelf2()
{
	Loadegp( IDC_EDIT_EFF_SELF2 );
}

void CPageEdit1::OnBnClickedButtonEffTarg()
{
	Loadegp( IDC_EDIT_EFF_TARG );
}

void CPageEdit1::OnBnClickedButtonEffSelfbody()
{
	Loadeffskin( IDC_EDIT_EFF_SELFBODY );
}

void CPageEdit1::OnBnClickedButtonEffTargetbody1()
{
	Loadeffskin( IDC_EDIT_EFF_TARGETBODY1);
}

void CPageEdit1::OnBnClickedButtonEffTargetbody2()
{
	Loadeffskin( IDC_EDIT_EFF_TARGETBODY2 );
}

void CPageEdit1::OnBnClickedButtonEffHoldout()
{
	Loadegp( IDC_EDIT_EFF_HOLDOUT );
}

void CPageEdit1::OnCbnSelchangeComboEmelement()
{
}

void CPageEdit1::OnCbnSelchangeComboEmtypes()
{
	InitBasicType();
}

void CPageEdit1::Loadegp( int nID )
{
	CString szFilter = "Effect File (*.egp)|*.egp|";
	CFileDialog dlg(TRUE,".egp",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = DxEffSinglePropGMan::GetInstance().GetPath ();

	if ( dlg.DoModal() == IDOK )
		SetWin_Text ( this, nID, dlg.GetFileName().GetString() );
}

void CPageEdit1::Loadeffskin( int nID )
{
	CString szFilter = "Effect File (*.effskin_a)|*.effskin_a|";
	CFileDialog dlg(TRUE,".effskin_a",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = DxEffcharDataMan::GetInstance().GetPath ();

	if ( dlg.DoModal() == IDOK )
		SetWin_Text ( this, nID, dlg.GetFileName().GetString() );
}

void CPageEdit1::Loadtexture( int nID )
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".dds",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath();

	if ( dlg.DoModal() == IDOK )
		SetWin_Text ( this, nID, dlg.GetFileName().GetString() );
}

void CPageEdit1::OnBnClickedButtonIcon()
{
	Loadtexture( IDC_EDIT_ICONFILE );
}

void CPageEdit1::OnBnClickedButtonSklvlMinus()
{
	SkillLearnSave();
	m_nLevel --;
	if ( m_nLevel <= 0 )
		m_nLevel = 0;
	SkillLearnShow();
}

void CPageEdit1::OnBnClickedButtonSklvlAdd()
{
	SkillLearnSave();
	m_nLevel ++;
	if ( m_nLevel >= SKILL::MAX_LEVEL )
		m_nLevel --;
	SkillLearnShow();
}

void CPageEdit1::OnBnClickedButtonSkill()
{
	CDlgSkill dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SNATIVEID sID = dlg.m_SELECTED;
		SetWin_Num_int( this, IDC_EDIT_REQSKILL_MID, sID.wMainID );
		SetWin_Num_int( this, IDC_EDIT_REQSKILL_SID, sID.wSubID );
		CheckReqSkill();
	}
}

void CPageEdit1::OnEnChangeEditReqskillMid()
{
	CheckReqSkill();
}

void CPageEdit1::OnEnChangeEditReqskillSid()
{
	CheckReqSkill();
}

void CPageEdit1::OnBnClickedButtonImpactMinus()
{
	SaveImpact();
	m_nIndexImpact --;
	if ( m_nIndexImpact <= 0 )
		m_nIndexImpact = 0;
	ShowImpact();
}

void CPageEdit1::OnBnClickedButtonImpactAdd()
{
	SaveImpact();
	m_nIndexImpact ++;
	if ( m_nIndexImpact >= SKILL::MAX_IMPACT )
		m_nIndexImpact--;
	ShowImpact();
}

void CPageEdit1::OnBnClickedButtonSpecMinus()
{
	SaveSpec();
	m_nIndexSpec --;
	if ( m_nIndexSpec <=0 )
		m_nIndexSpec = 0;
	ShowSpec();
}

void CPageEdit1::OnBnClickedButtonSpecAdd()
{
	SaveSpec();
	m_nIndexSpec ++;
	if ( m_nIndexSpec >= SKILL::MAX_SPEC )
		m_nIndexSpec--;
	ShowSpec();
}

void CPageEdit1::OnBnClickedButtonEffTarget3()
{
	Loadegp( IDC_EDIT_EFF_TARGET3 );
}

void CPageEdit1::OnBnClickedButtonEffSelf3()
{
	Loadegp( IDC_EDIT_EFF_SELF3 );
}
