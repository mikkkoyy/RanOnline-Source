// PageEdit2.cpp : implementation file
//

#include "stdafx.h"
#include "EditorMobNpc.h"
#include "PageEdit2.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "DlgSkill.h"
#include "DxEffSinglePropGMan.h"
#include "DxEffCharData.h"


// CPageEdit2 dialog

IMPLEMENT_DYNAMIC(CPageEdit2, CPropertyPage)

CPageEdit2::CPageEdit2(LOGFONT logfont)
	: CPropertyPage(CPageEdit2::IDD),
	m_pFont ( NULL ),
	m_pDataDummy( NULL ),
	m_nAttack( 0 )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pCrow = NULL;
	m_pDummyCrow = new SCROWDATA;

	m_pDataDummy = new DxSkinDataDummy;

	sAniAttack = SANIATTACK();
}

CPageEdit2::~CPageEdit2()
{
	SAFE_DELETE ( m_pFont );
	SAFE_DELETE ( m_pDummyCrow );
	SAFE_DELETE ( m_pDataDummy );
}

void CPageEdit2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PATTERN, m_listPattern );
	DDX_Control(pDX, IDC_LIST_ANI_DIV, m_listDIV );
	DDX_Control(pDX, IDC_LIST_ANIM, m_listANI );
}


BEGIN_MESSAGE_MAP(CPageEdit2, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_BACK, OnBnClickedButtonPageBack)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_CANCEL, OnBnClickedButtonPageCancel)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_SAVE, OnBnClickedButtonPageSave)
	ON_BN_CLICKED(IDC_BUTTON_ATTACK_MINUS, OnBnClickedButtonAttackMinus)
	ON_BN_CLICKED(IDC_BUTTON_ATTACK_ADD, OnBnClickedButtonAttackAdd)
	ON_BN_CLICKED(IDC_BUTTON_PATTERN_ADD, OnBnClickedButtonPatternAdd)
	ON_BN_CLICKED(IDC_BUTTON_PATTERN_EDIT, OnBnClickedButtonPatternEdit)
	ON_BN_CLICKED(IDC_BUTTON_PATTERN_DELETE, OnBnClickedButtonPatternDelete)
	ON_BN_CLICKED(IDC_BUTTON_PATTERN_SAVE, OnBnClickedButtonPatternSave)
	ON_CBN_SELCHANGE(IDC_COMBO_BLOW_TYPE, OnCbnSelchangeComboBlowType)
	ON_BN_CLICKED(IDC_BUTTON_ATTACK_SKILL, OnBnClickedButtonAttackSkill)
	ON_BN_CLICKED(IDC_BUTTON_SELFBODY, OnBnClickedButtonSelfbody)
	ON_BN_CLICKED(IDC_BUTTON_TARGETBODY, OnBnClickedButtonTargetbody)
	ON_BN_CLICKED(IDC_BUTTON_TARGET, OnBnClickedButtonTarget)
	ON_BN_CLICKED(IDC_BUTTON_USE_ANI, OnBnClickedButtonUseAni)
	ON_EN_CHANGE(IDC_EDIT_ATTACK_SKILL_MID, OnEnChangeEditAttackSkillMid)
	ON_EN_CHANGE(IDC_EDIT_ATTACK_SKILL_SID, OnEnChangeEditAttackSkillSid)
END_MESSAGE_MAP()


// CPageEdit2 message handlers
BOOL CPageEdit2::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	LONG lnWidth;
	LV_COLUMN lvColumn;

	m_listPattern.SetExtendedStyle ( m_listPattern.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_listPattern.GetClientRect ( &rectCtrl );
	lnWidth = rectCtrl.right - rectCtrl.left;

	const int nColumnCount = 4;	
	char* szColumnName1[nColumnCount] = { "Num", "PatternType", "DNRate", "Attack" };
	int nColumnWidthPercent1[nColumnCount] = { 11, 58, 16, 15 };

	for ( int i = 0; i < nColumnCount; i++ )
	{		
		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.pszText = szColumnName1[i];
		lvColumn.iSubItem = i;
		lvColumn.cx = ( lnWidth*nColumnWidthPercent1[i] ) / 100;
		m_listPattern.InsertColumn (i, &lvColumn );
	}

	DataDefault();
	DataShow();		

	return TRUE;  
}

BOOL CPageEdit2::CrowSet ( PCROWDATA pCROW )
{
	if ( m_pCrow )	return FALSE;
	m_pCrow = pCROW;
	m_pDummyCrow->Assign( *m_pCrow );

	if ( m_bDlgInit ) 
	{
		DataDefault();
		DataShow();	
	}

	return TRUE;
}

void CPageEdit2::CrowClear()
{
	m_pCrow = NULL;
}

void CPageEdit2::OnBnClickedButtonPageBack()
{
	if ( !DataSave() )	return;

	SCROWDATA sCROW;
	sCROW.Assign( *m_pDummyCrow );
	GLCrowDataMan::GetInstance().InsertCrowData ( m_pDummyCrow->m_sBasic.sNativeID, &sCROW );

	m_pSheetTab->ActivePageEdit1( m_CallPage, m_pCrow );
	m_pCrow = NULL;
}

void CPageEdit2::OnBnClickedButtonPageCancel()
{
	m_pCrow = NULL;
	if ( m_pSheetTab )
		m_pSheetTab->ActivePageTree( FALSE );
}

void CPageEdit2::OnBnClickedButtonPageSave()
{
	if ( !DataSave() )	return;

	SCROWDATA sCROW;
	sCROW.Assign( *m_pDummyCrow );
	GLCrowDataMan::GetInstance().InsertCrowData ( m_pDummyCrow->m_sBasic.sNativeID, &sCROW );

	m_pCrow = NULL;
	m_pSheetTab->ActivePageTree( FALSE );
}

void CPageEdit2::OnBnClickedButtonAttackMinus()
{
	AttackSave( m_nAttack );
	m_nAttack --;
	if ( m_nAttack <= 0 )
		m_nAttack = 0;
	AttackShow( m_nAttack );
}

void CPageEdit2::OnBnClickedButtonAttackAdd()
{
	AttackSave( m_nAttack );
	m_nAttack ++;
	if ( m_nAttack >= SCROWDATA::EMMAXATTACK )
		m_nAttack --;
	AttackShow( m_nAttack );
}

void CPageEdit2::InitBlow()
{
	EMSTATE_BLOW emTYPE = static_cast<EMSTATE_BLOW> ( GetWin_Combo_Sel( this, IDC_COMBO_BLOW_TYPE ));

	BOOL bSTATE_BLOW = ( emTYPE!=EMBLOW_NONE );
	BOOL bBLOW_VAR1(TRUE), bBLOW_VAR2(TRUE);
	const char *szBLOW_VAR1 = COMMENT::BLOW_VAR1[emTYPE].c_str();
	const char *szBLOW_VAR2 = COMMENT::BLOW_VAR2[emTYPE].c_str();
	if ( !strcmp(szBLOW_VAR1, COMMENT::BLOW_VAR1[EMBLOW_NONE].c_str()) )		bBLOW_VAR1 = FALSE;
	if ( !strcmp(szBLOW_VAR2, COMMENT::BLOW_VAR2[EMBLOW_NONE].c_str()) )		bBLOW_VAR2 = FALSE;

	SetWin_Enable ( this, IDC_EDIT_BLOW_TIME, bSTATE_BLOW );
	SetWin_Enable ( this, IDC_EDIT_BLOW_CHANCE, bSTATE_BLOW );
	SetWin_Enable ( this, IDC_EDIT_BLOW_VAR1, bBLOW_VAR1 );
	SetWin_Enable ( this, IDC_EDIT_BLOW_VAR2, bBLOW_VAR2 );

	SetWin_Text ( this, IDC_STATIC_BLOW_OPTION1, szBLOW_VAR1 );
	SetWin_Text ( this, IDC_STATIC_BLOW_OPTION2, szBLOW_VAR2 );
}

void CPageEdit2::InitSkill()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_ATTACK_SKILL_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_ATTACK_SKILL_SID );
	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( wMID, wSID );
	SetWin_Text( this, IDC_EDIT_ATTACK_SKILL_NAME, pSKILL? pSKILL->GetName() : "Unknown Skill" );
}

void CPageEdit2::OnCbnSelchangeComboBlowType()
{
	InitBlow();
}

void CPageEdit2::OnBnClickedButtonAttackSkill()
{
	CDlgSkill dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SNATIVEID sID = dlg.m_SELECTED;
		SetWin_Num_int( this, IDC_EDIT_ATTACK_SKILL_MID, sID.wMainID );
		SetWin_Num_int( this, IDC_EDIT_ATTACK_SKILL_SID, sID.wSubID );
		InitSkill();
	}
}

void CPageEdit2::OnBnClickedButtonSelfbody()
{
	CString szFilter = "Effect File (*.effskin_a)|*.effskin_a|";
	CFileDialog dlg(TRUE,".effskin_a",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = DxEffcharDataMan::GetInstance().GetPath ();

	if ( dlg.DoModal() == IDOK )
		SetWin_Text ( this, IDC_EDIT_ATTACK_SELFBODY, dlg.GetFileName().GetString() );
}

void CPageEdit2::OnBnClickedButtonTargetbody()
{
	CString szFilter = "Effect File (*.effskin_a)|*.effskin_a|";
	CFileDialog dlg(TRUE,".effskin_a",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = DxEffcharDataMan::GetInstance().GetPath ();

	if ( dlg.DoModal() == IDOK )
		SetWin_Text ( this, IDC_EDIT_ATTACK_TARGETBODY, dlg.GetFileName().GetString() );
}

void CPageEdit2::OnBnClickedButtonTarget()
{
	CString szFilter = "Effect File (*.egp)|*.egp|";
	CFileDialog dlg(TRUE,".egp",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = DxEffSinglePropGMan::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_ATTACK_TARGET, dlg.GetFileName().GetString() );
}

void CPageEdit2::InitAniList()
{
	m_listANI.ResetContent();

	if( !m_pDummyCrow )	return;
	if( !m_pDataDummy )	return;

	m_pDataDummy->CleanUp();

	BOOL bOK = m_pDataDummy->LoadFile ( m_pDummyCrow->GetSkinObjFile() );
	if ( bOK )
	{
		m_listANI.SetRedraw( FALSE );

		for( size_t i=0; i < m_pDataDummy->m_vecANIMINFO.size(); ++i )
		{
			SANIMCONINFO* pINFO = m_pDataDummy->m_vecANIMINFO[i];
			int nInsert = m_listANI.AddString( pINFO->m_szName );
			m_listANI.SetItemData( nInsert, (DWORD_PTR)pINFO );
		}

		m_listANI.SetRedraw( TRUE );
	}
}

void CPageEdit2::InitAniInfo( )
{
	//shows animation info
	SetWin_Text( this, IDC_EDIT_ANI_MAIN, COMMENT::ANI_MAINTYPE_CHAR[sAniAttack.m_MainType].c_str() );
	SetWin_Text( this, IDC_EDIT_ANI_SUB, COMMENT::ANI_SUBTYPE_SKILL[sAniAttack.m_SubType].c_str() );
	SetWin_Num_int( this, IDC_EDIT_ANI_TIME_START, sAniAttack.m_dwSTime );
	SetWin_Num_int( this, IDC_EDIT_ANI_TIME_END, sAniAttack.m_dwETime );
	SetWin_Num_int( this, IDC_EDIT_ANI_TIME_UNIT, sAniAttack.m_UNITTIME );
	SetWin_Num_int( this, IDC_EDIT_ANI_FLAGS, sAniAttack.m_dwFlags );
	SetWin_Num_int( this, IDC_EDIT_ANI_DIV, sAniAttack.m_wDivCount );

	m_listDIV.ResetContent();
	m_listDIV.SetRedraw( FALSE );

	for( int i=0; i < sAniAttack.m_wDivCount; ++i )
	{
		CString strDIV;
		strDIV.Format( "[%d]", sAniAttack.m_wDivFrame[i] );
		int nINSERT = m_listDIV.AddString( strDIV.GetString() );
		m_listDIV.SetItemData( nINSERT, sAniAttack.m_wDivFrame[i] );
	}

	m_listDIV.SetRedraw( TRUE );
}

void CPageEdit2::OnBnClickedButtonUseAni()
{
	if( !m_pDummyCrow )	return;
	if( !m_pDataDummy )	return;

	int nIndex = m_listANI.GetCurSel();
	if ( nIndex==LB_ERR )	return;

	SANIMCONINFO* pINFO = (SANIMCONINFO*)m_listANI.GetItemData ( nIndex );
	if ( !pINFO )	return;

	SetWin_Text( this, IDC_EDIT_ANI_NAME, pINFO->m_szName );
	sAniAttack.Assign( pINFO );
	
	InitAniInfo();
}

void CPageEdit2::OnEnChangeEditAttackSkillMid()
{
	InitSkill();
}

void CPageEdit2::OnEnChangeEditAttackSkillSid()
{
	InitSkill();
}
