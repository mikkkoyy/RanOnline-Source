// PageEdit1.cpp : implementation file
//

#include "stdafx.h"
#include "EditorItem.h"
#include "PageEdit1.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "GLStringTable.h"
#include "GLItemMan.h"
#include "GLOGIC.h"
#include "DxEffcharData.h"
#include "DlgCrow.h"
#include "DlgSkill.h"
#include "DlgMap.h"
#include "GLMapList.h"
#include "SelColorDlg.h"
#include "HLibColorValue.h"
#include ".\pageedit1.h"

// CPageEdit1 dialog

IMPLEMENT_DYNAMIC(CPageEdit1, CPropertyPage)

CPageEdit1::CPageEdit1(LOGFONT logfont)
	: CPropertyPage(CPageEdit1::IDD),
	m_pFont ( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pItem = NULL;
	m_pDummyItem = new SITEM;
}

CPageEdit1::~CPageEdit1()
{
	SAFE_DELETE ( m_pFont );
	SAFE_DELETE ( m_pDummyItem );
}

void CPageEdit1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageEdit1, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_SAVE, OnBnClickedButtonPageSave)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_CANCEL, OnBnClickedButtonPageCancel)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_NEXT, OnBnClickedButtonPageNext)
	ON_BN_CLICKED(IDC_BUTTON_ALL_MALE, OnBnClickedButtonAllMale)
	ON_BN_CLICKED(IDC_BUTTON_ALL_FEMALE, OnBnClickedButtonAllFemale)
	ON_BN_CLICKED(IDC_BUTTON_ALL, OnBnClickedButtonAll)
	ON_BN_CLICKED(IDC_BUTTON_NOTHING, OnBnClickedButtonNothing)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_M, OnBnClickedButtonApplyM)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_M, OnBnClickedButtonClearM)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_F, OnBnClickedButtonApplyF)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_F, OnBnClickedButtonClearF)
	ON_BN_CLICKED(IDC_BUTTON_BRAWLER_M, OnBnClickedButtonBrawlerM)
	ON_BN_CLICKED(IDC_BUTTON_BRAWLER_F, OnBnClickedButtonBrawlerF)
	ON_BN_CLICKED(IDC_BUTTON_SWORDSMAN_M, OnBnClickedButtonSwordsmanM)
	ON_BN_CLICKED(IDC_BUTTON_SWORDSMAN_F, OnBnClickedButtonSwordsmanF)
	ON_BN_CLICKED(IDC_BUTTON_ARCHER_M, OnBnClickedButtonArcherM)
	ON_BN_CLICKED(IDC_BUTTON_ARCHER_F, OnBnClickedButtonArcherF)
	ON_BN_CLICKED(IDC_BUTTON_SHAMAN_M, OnBnClickedButtonShamanM)
	ON_BN_CLICKED(IDC_BUTTON_SHAMAN_F, OnBnClickedButtonShamanF)
	ON_BN_CLICKED(IDC_BUTTON_EXTREME_M, OnBnClickedButtonExtremeM)
	ON_BN_CLICKED(IDC_BUTTON_EXTREME_F, OnBnClickedButtonExtremeF)
	ON_BN_CLICKED(IDC_BUTTON_SELFBODY, OnBnClickedButtonSelfbody)
	ON_BN_CLICKED(IDC_BUTTON_TARGET_BODY, OnBnClickedButtonTargetBody)
	ON_BN_CLICKED(IDC_BUTTON_PARTNEREFFECT, OnBnClickedButtonPartnereffect)
	ON_BN_CLICKED(IDC_BUTTON_PETWEAR, OnBnClickedButtonPetwear)
	ON_BN_CLICKED(IDC_BUTTON_RANDOMOPTION, OnBnClickedButtonRandomoption)
	ON_BN_CLICKED(IDC_BUTTON_SHAPEINVEN, OnBnClickedButtonShapeinven)
	ON_BN_CLICKED(IDC_BUTTON_SHAPEFIELD, OnBnClickedButtonShapefield)
	ON_CBN_SELCHANGE(IDC_COMBO_ITEMTYPE, OnCbnSelchangeComboItemtype)
	ON_BN_CLICKED(IDC_BUTTON_PET_CROW_ADD, OnBnClickedButtonPetCrowAdd)
	ON_BN_CLICKED(IDC_BUTTON_SKILL_SCROLL_ADD, OnBnClickedButtonSkillScrollAdd)
	ON_BN_CLICKED(IDC_BUTTON_SPECMAP_ADD, OnBnClickedButtonSpecmapAdd)
	ON_BN_CLICKED(IDC_BUTTON_NPCREC_ADD, OnBnClickedButtonNpcrecAdd)
	ON_EN_CHANGE(IDC_EDIT_PET_MID, OnEnChangeEditPetMid)
	ON_EN_CHANGE(IDC_EDIT_PET_SID, OnEnChangeEditPetSid)
	ON_EN_CHANGE(IDC_EDIT_SKILL_MID, OnEnChangeEditSkillMid)
	ON_EN_CHANGE(IDC_EDIT_SKILL_SID, OnEnChangeEditSkillSid)
	ON_EN_CHANGE(IDC_EDIT_MAP_MID2, OnEnChangeEditMapMid2)
	ON_EN_CHANGE(IDC_EDIT_MAP_SID2, OnEnChangeEditMapSid2)
	ON_EN_CHANGE(IDC_EDIT_MAP_MID, OnEnChangeEditMapMid)
	ON_EN_CHANGE(IDC_EDIT_MAP_SID, OnEnChangeEditMapSid)
	ON_BN_CLICKED(IDC_BUTTON_GUNNER_M, OnBnClickedButtonGunnerM)
	ON_BN_CLICKED(IDC_BUTTON_GUNNER_F, OnBnClickedButtonGunnerF)
	ON_BN_CLICKED(IDC_BUTTON_BRAWLER_M2, OnBnClickedButtonBrawlerM2)
	ON_BN_CLICKED(IDC_BUTTON_BRAWLER_F2, OnBnClickedButtonBrawlerF2)
	ON_BN_CLICKED(IDC_BUTTON_SWORDSMAN_M2, OnBnClickedButtonSwordsmanM2)
	ON_BN_CLICKED(IDC_BUTTON_SWORDSMAN_F2, OnBnClickedButtonSwordsmanF2)
	ON_BN_CLICKED(IDC_BUTTON_ARCHER_M2, OnBnClickedButtonArcherM2)
	ON_BN_CLICKED(IDC_BUTTON_ARCHER_F2, OnBnClickedButtonArcherF2)
	ON_BN_CLICKED(IDC_BUTTON_SHAMAN_M2, OnBnClickedButtonShamanM2)
	ON_BN_CLICKED(IDC_BUTTON_SHAMAN_F2, OnBnClickedButtonShamanF2)
	ON_BN_CLICKED(IDC_BUTTON_EXTREME_M2, OnBnClickedButtonExtremeM2)
	ON_BN_CLICKED(IDC_BUTTON_EXTREME_F2, OnBnClickedButtonExtremeF2)
	ON_BN_CLICKED(IDC_BUTTON_GUNNER_M2, OnBnClickedButtonGunnerM2)
	ON_BN_CLICKED(IDC_BUTTON_GUNNER_F2, OnBnClickedButtonGunnerF2)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_M2, OnBnClickedButtonApplyM2)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_M2, OnBnClickedButtonClearM2)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_F2, OnBnClickedButtonApplyF2)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_F2, OnBnClickedButtonClearF2)
	ON_CBN_SELCHANGE(IDC_COMBO_VEHICLE, OnCbnSelchangeComboVehicle)
	ON_BN_CLICKED(IDC_BUTTON_ASSASSIN_M, OnBnClickedButtonAssassinM)
	ON_BN_CLICKED(IDC_BUTTON_ASSASSIN_F, OnBnClickedButtonAssassinF)
	ON_BN_CLICKED(IDC_BUTTON_TRICKER_M, OnBnClickedButtonTrickerM)
	ON_BN_CLICKED(IDC_BUTTON_TRICKER_F, OnBnClickedButtonTrickerF)
	ON_BN_CLICKED(IDC_BUTTON_ASSASSIN_M2, OnBnClickedButtonAssassinM2)
	ON_BN_CLICKED(IDC_BUTTON_ASSASSIN_F2, OnBnClickedButtonAssassinF2)
	ON_BN_CLICKED(IDC_BUTTON_TRICKER_M2, OnBnClickedButtonTrickerM2)
	ON_BN_CLICKED(IDC_BUTTON_TRICKER_F2, OnBnClickedButtonTrickerF2)
	ON_BN_CLICKED(IDC_BUTTON_VEHICLE_DEFAULT_COLOR_1, OnBnClickedButtonVehicleDefaultColor1)
	ON_BN_CLICKED(IDC_BUTTON_VEHICLE_DEFAULT_COLOR_2, OnBnClickedButtonVehicleDefaultColor2)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_DEFAULT_COLOR_1, OnBnClickedButtonItemDefaultColor1)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_DEFAULT_COLOR_2, OnBnClickedButtonItemDefaultColor2)
END_MESSAGE_MAP()


// CPageEdit1 message handlers
BOOL CPageEdit1::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	m_MapsList.CleanUp();
	m_MapsList.LoadMapsListFile( "mapslist.mst" );

	DataDefault();
	DataShow();		
	
	return TRUE;  
}

BOOL CPageEdit1::ItemSet ( SITEM* pItem )
{
	if ( m_pItem )	return FALSE;
	m_pItem = pItem;
	*m_pDummyItem = *m_pItem;

	if ( m_bDlgInit ) 
	{
		DataDefault();
		DataShow();	
	}

	return TRUE;
}

void CPageEdit1::ItemClear ()
{
	m_pItem = NULL;
}


void CPageEdit1::OnBnClickedButtonPageSave()
{
	if ( !DataSave() )	return;

	CItemNode sItemNode;
	sItemNode.m_sItem = *m_pDummyItem;

	GLItemMan::GetInstance().InsertItem ( m_pDummyItem->sBasicOp.sNativeID, &sItemNode );
	std::string strNAME = GetWin_Text(this, IDC_EDIT_NAME2 ).GetString();
	std::string strDESC = GetWin_Text(this, IDC_EDIT_COMMENT2 ).GetString();
	GLStringTable::GetInstance().InsertString ( m_pDummyItem->sBasicOp.strName, strNAME, GLStringTable::ITEM );
	GLStringTable::GetInstance().InsertString ( m_pDummyItem->sBasicOp.strComment, strDESC, GLStringTable::ITEM );

	m_pItem = NULL;
	m_pSheetTab->ActivePageTree( FALSE );
}

void CPageEdit1::OnBnClickedButtonPageCancel()
{
	m_pItem = NULL;
	if ( m_pSheetTab )
		m_pSheetTab->ActivePageTree( FALSE );
}

void CPageEdit1::OnBnClickedButtonPageNext()
{
	if ( !DataSave() )	return;

	CItemNode sItemNode;
	sItemNode.m_sItem = *m_pDummyItem;
	GLItemMan::GetInstance().InsertItem ( m_pDummyItem->sBasicOp.sNativeID, &sItemNode );
	std::string strNAME = GetWin_Text(this, IDC_EDIT_NAME2 ).GetString();
	std::string strDESC = GetWin_Text(this, IDC_EDIT_COMMENT2 ).GetString();
	GLStringTable::GetInstance().InsertString ( m_pDummyItem->sBasicOp.strName, strNAME, GLStringTable::ITEM );
	GLStringTable::GetInstance().InsertString ( m_pDummyItem->sBasicOp.strComment, strDESC, GLStringTable::ITEM );

	m_pSheetTab->ActivePageEdit2( m_CallPage, m_pItem );
	m_pItem = NULL;
}

void CPageEdit1::EnableClass( BOOL bMALE, BOOL bFEMALE )
{
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_BRAWLER_M, bMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SWORDSMAN_M, bMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ARCHER_M, bMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SHAMAN_M, bMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_EXTREME_M, bMALE );	
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_BRAWLER_F, bFEMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SWORDSMAN_F, bFEMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ARCHER_F, bFEMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SHAMAN_F, bFEMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_EXTREME_F, bFEMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_GUNNER_M, bMALE );	
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_GUNNER_F, bFEMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ASSASSIN_M, bMALE );	
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ASSASSIN_F, bFEMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_TRICKER_M, bMALE );	
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_TRICKER_F, bFEMALE );
}

void CPageEdit1::OnBnClickedButtonAllMale()
{
	EnableClass( TRUE, FALSE );
}

void CPageEdit1::OnBnClickedButtonAllFemale()
{
	EnableClass( FALSE, TRUE );
}

void CPageEdit1::OnBnClickedButtonAll()
{
	EnableClass( TRUE, TRUE );
}

void CPageEdit1::OnBnClickedButtonNothing()
{
	EnableClass( FALSE, FALSE );
}

void CPageEdit1::OnBnClickedButtonApplyM()
{
	std::string strTEXT = GetWin_Text( this, IDC_EDIT_APPLY_M );
	SetWin_Text( this, IDC_EDIT_WEAR_BRAWLER_M, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SWORDSMAN_M, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ARCHER_M, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SHAMAN_M, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_EXTREME_M, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_GUNNER_M, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ASSASSIN_M, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_TRICKER_M, strTEXT.c_str() );
}

void CPageEdit1::OnBnClickedButtonClearM()
{
	std::string strTEXT = "";
	SetWin_Text( this, IDC_EDIT_WEAR_BRAWLER_M, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SWORDSMAN_M, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ARCHER_M, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SHAMAN_M, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_EXTREME_M, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_GUNNER_M, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ASSASSIN_M, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_TRICKER_M, strTEXT.c_str() );
}

void CPageEdit1::OnBnClickedButtonApplyF()
{
	std::string strTEXT = GetWin_Text( this, IDC_EDIT_APPLY_F );
	SetWin_Text( this, IDC_EDIT_WEAR_BRAWLER_F, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SWORDSMAN_F, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ARCHER_F, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SHAMAN_F, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_EXTREME_F, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_GUNNER_F, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ASSASSIN_F, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_TRICKER_F, strTEXT.c_str() );
}

void CPageEdit1::OnBnClickedButtonClearF()
{
	std::string strTEXT = "";
	SetWin_Text( this, IDC_EDIT_WEAR_BRAWLER_F, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SWORDSMAN_F, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ARCHER_F, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SHAMAN_F, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_EXTREME_F, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_GUNNER_F, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ASSASSIN_F, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_TRICKER_F, strTEXT.c_str() );
}

void CPageEdit1::OnBnClickedButtonBrawlerM()
{
	LoadWearingfile(IDC_EDIT_WEAR_BRAWLER_M);
}

void CPageEdit1::OnBnClickedButtonBrawlerF()
{
	LoadWearingfile(IDC_EDIT_WEAR_BRAWLER_F);
}

void CPageEdit1::OnBnClickedButtonSwordsmanM()
{
	LoadWearingfile(IDC_EDIT_WEAR_SWORDSMAN_M);
}

void CPageEdit1::OnBnClickedButtonSwordsmanF()
{
	LoadWearingfile(IDC_EDIT_WEAR_SWORDSMAN_F);
}

void CPageEdit1::OnBnClickedButtonArcherM()
{
	LoadWearingfile(IDC_EDIT_WEAR_ARCHER_M);
}

void CPageEdit1::OnBnClickedButtonArcherF()
{
	LoadWearingfile(IDC_EDIT_WEAR_ARCHER_F);
}

void CPageEdit1::OnBnClickedButtonShamanM()
{
	LoadWearingfile(IDC_EDIT_WEAR_SHAMAN_M);
}

void CPageEdit1::OnBnClickedButtonShamanF()
{
	LoadWearingfile(IDC_EDIT_WEAR_SHAMAN_F);
}

void CPageEdit1::OnBnClickedButtonExtremeM()
{
	LoadWearingfile(IDC_EDIT_WEAR_EXTREME_M);
}

void CPageEdit1::OnBnClickedButtonExtremeF()
{
	LoadWearingfile(IDC_EDIT_WEAR_EXTREME_F);
}

void CPageEdit1::OnBnClickedButtonSelfbody()
{
	CString szFilter = "Effect File (*.effskin)|*.effskin|";
	CFileDialog dlg(TRUE,".effskin",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = DxEffcharDataMan::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_EDIT_EFFECT_SELFBODY, dlg.GetFileName().GetString() );
}

void CPageEdit1::OnBnClickedButtonTargetBody()
{
	CString szFilter = "Effect File (*.effskin_a)|*.effskin_a|";
	CFileDialog dlg(TRUE,".effskin_a",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = DxEffcharDataMan::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_EDIT_EFFECT_TARGETBODY, dlg.GetFileName().GetString() );
}

void CPageEdit1::OnBnClickedButtonPartnereffect()
{
	CString szFilter = "Effect File (*.egp)|*.egp|";
	CFileDialog dlg(TRUE,".egp",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = DxEffSinglePropGMan::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_EDIT_EFFECT_TARGET, dlg.GetFileName().GetString() );
}

void CPageEdit1::OnBnClickedButtonPetwear()
{
	LoadWearingfile(IDC_EDIT_PETWEAR);
}

void CPageEdit1::OnBnClickedButtonRandomoption()
{
	CString szFilter = "Item Random Option File (*.bin)|*.BIN|";
	CFileDialog dlg(TRUE,".bin",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetServerPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_EDIT_RANDOM_OPTION, dlg.GetFileName().GetString() );
}

void CPageEdit1::OnBnClickedButtonShapeinven()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".x",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_EDIT_INVENTORY, dlg.GetFileName().GetString() );
}

void CPageEdit1::OnBnClickedButtonShapefield()
{
	CString szFilter = "Object File (*.x)|*.x|";
	CFileDialog dlg(TRUE,".x",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = DxSimpleMeshMan::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_EDIT_FIELD, dlg.GetFileName().GetString() );
}

void CPageEdit1::InitType()
{
	EMITEM_TYPE emTYPE = static_cast<EMITEM_TYPE>( GetWin_Combo_Sel( this, IDC_COMBO_ITEMTYPE ) );
}

void CPageEdit1::OnCbnSelchangeComboItemtype()
{
	InitType();
}

void CPageEdit1::OnBnClickedButtonPetCrowAdd()
{
	CDlgCrow dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SNATIVEID sID = dlg.m_SELECTED;
		PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( sID );
		if ( pCROW )
		{
			SetWin_Num_int( this, IDC_EDIT_PET_MID, pCROW->m_sBasic.sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_PET_SID, pCROW->m_sBasic.sNativeID.wSubID );
			CheckPetCrow();
		}
	}
}

void CPageEdit1::OnEnChangeEditPetMid()
{
	CheckPetCrow();
}

void CPageEdit1::OnEnChangeEditPetSid()
{
	CheckPetCrow();
}

void CPageEdit1::CheckPetCrow()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_PET_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_PET_SID );

	std::string strNAME = "Unknown Crow";
	PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( wMID, wSID );
	if ( pCROW )	
		strNAME = pCROW->GetName();
	SetWin_Text( this, IDC_EDIT_PET_NAME, strNAME.c_str() );
}


void CPageEdit1::OnBnClickedButtonSkillScrollAdd()
{
	CDlgSkill dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SNATIVEID sID = dlg.m_SELECTED;
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sID );
		if ( pSKILL )
		{
			SetWin_Num_int( this, IDC_EDIT_SKILL_MID, pSKILL->m_sBASIC.sNATIVEID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_SKILL_SID, pSKILL->m_sBASIC.sNATIVEID.wSubID );
			CheckSkillScroll();
		}
	}
}

void CPageEdit1::OnEnChangeEditSkillMid()
{
	CheckSkillScroll();
}

void CPageEdit1::OnEnChangeEditSkillSid()
{
	CheckSkillScroll();
}

void CPageEdit1::CheckSkillScroll()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_SKILL_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_SKILL_SID );

	std::string strNAME = "Unknown Skill";
	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( wMID, wSID );
	if ( pSKILL )	
		strNAME = pSKILL->GetName();
	SetWin_Text( this, IDC_EDIT_SKILL_NAME, strNAME.c_str() );
}

void CPageEdit1::OnBnClickedButtonNpcrecAdd()
{
	CDlgCrow dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SNATIVEID sID = dlg.m_SELECTED;
		PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( sID );
		if ( pCROW )
		{
			SetWin_Num_int( this, IDC_EDIT_MAP_MID2, pCROW->m_sBasic.sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_MAP_SID2, pCROW->m_sBasic.sNativeID.wSubID );
			CheckNpcRecCrow();
		}
	}
}

void CPageEdit1::OnEnChangeEditMapMid2()
{
	CheckNpcRecCrow();
}

void CPageEdit1::OnEnChangeEditMapSid2()
{
	CheckNpcRecCrow();
}

void CPageEdit1::CheckNpcRecCrow()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_MAP_MID2 );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_MAP_SID2 );

	std::string strNAME = "Unknown Crow";
	PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( wMID, wSID );
	if ( pCROW )	
		strNAME = pCROW->GetName();
	SetWin_Text( this, IDC_EDIT_MAP_NAME2, strNAME.c_str() );
}


void CPageEdit1::OnBnClickedButtonSpecmapAdd()
{
	CDlgMap dlg;
	if( dlg.DoModal() == IDOK )
	{
		SMAPNODE* pNode = m_MapsList.FindMapNode( dlg.m_SELECTED );
		if ( pNode )
		{
			SetWin_Num_int( this, IDC_EDIT_MAP_MID, pNode->sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_MAP_SID, pNode->sNativeID.wSubID );
			
			CheckSpecMap();
		}
	}
}

void CPageEdit1::CheckSpecMap()
{
	SNATIVEID sID;
	sID.wMainID = GetWin_Num_int( this, IDC_EDIT_MAP_MID );
	sID.wSubID = GetWin_Num_int( this, IDC_EDIT_MAP_SID );

	std::string strTEXT = "UnknownMap";
	SMAPNODE* pNode = m_MapsList.FindMapNode( sID );
	if ( pNode )	strTEXT = pNode->strMapName.c_str();
	SetWin_Text( this, IDC_EDIT_MAP_NAME, strTEXT.c_str() );
}

void CPageEdit1::OnEnChangeEditMapMid()
{
	CheckSpecMap();
}

void CPageEdit1::OnEnChangeEditMapSid()
{
	CheckSpecMap();
}

void CPageEdit1::OnBnClickedButtonGunnerM()
{
	LoadWearingfile(IDC_EDIT_WEAR_GUNNER_M);
}

void CPageEdit1::OnBnClickedButtonGunnerF()
{
	LoadWearingfile(IDC_EDIT_WEAR_GUNNER_F);
}

void CPageEdit1::OnBnClickedButtonBrawlerM2()
{
	LoadWearingfile(IDC_EDIT_WEAR_BRAWLER_M2 );
}

void CPageEdit1::OnBnClickedButtonBrawlerF2()
{
	LoadWearingfile(IDC_EDIT_WEAR_BRAWLER_F2 );
}

void CPageEdit1::OnBnClickedButtonSwordsmanM2()
{
	LoadWearingfile(IDC_EDIT_WEAR_SWORDSMAN_M2 );
}

void CPageEdit1::OnBnClickedButtonSwordsmanF2()
{
	LoadWearingfile(IDC_EDIT_WEAR_SWORDSMAN_F2 );
}

void CPageEdit1::OnBnClickedButtonArcherM2()
{
	LoadWearingfile(IDC_EDIT_WEAR_ARCHER_M2 );
}

void CPageEdit1::OnBnClickedButtonArcherF2()
{
	LoadWearingfile(IDC_EDIT_WEAR_ARCHER_F2 );
}

void CPageEdit1::OnBnClickedButtonShamanM2()
{
	LoadWearingfile(IDC_EDIT_WEAR_SHAMAN_M2 );
}

void CPageEdit1::OnBnClickedButtonShamanF2()
{
	LoadWearingfile(IDC_EDIT_WEAR_SHAMAN_F2 );
}

void CPageEdit1::OnBnClickedButtonExtremeM2()
{
	LoadWearingfile(IDC_EDIT_WEAR_EXTREME_M2 );
}

void CPageEdit1::OnBnClickedButtonExtremeF2()
{
	LoadWearingfile(IDC_EDIT_WEAR_EXTREME_F2 );
}

void CPageEdit1::OnBnClickedButtonGunnerM2()
{
	LoadWearingfile(IDC_EDIT_WEAR_GUNNER_M2 );
}

void CPageEdit1::OnBnClickedButtonGunnerF2()
{
	LoadWearingfile(IDC_EDIT_WEAR_GUNNER_F2 );
}

void CPageEdit1::OnBnClickedButtonApplyM2()
{
	std::string strTEXT = GetWin_Text( this, IDC_EDIT_APPLY_M2 );
	SetWin_Text( this, IDC_EDIT_WEAR_BRAWLER_M2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SWORDSMAN_M2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ARCHER_M2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SHAMAN_M2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_EXTREME_M2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_GUNNER_M2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ASSASSIN_M2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_TRICKER_M2, strTEXT.c_str() );
}

void CPageEdit1::OnBnClickedButtonClearM2()
{
	std::string strTEXT = "";
	SetWin_Text( this, IDC_EDIT_WEAR_BRAWLER_M2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SWORDSMAN_M2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ARCHER_M2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SHAMAN_M2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_EXTREME_M2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_GUNNER_M2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ASSASSIN_M2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_TRICKER_M2, strTEXT.c_str() );
}

void CPageEdit1::OnBnClickedButtonApplyF2()
{
	std::string strTEXT = GetWin_Text( this, IDC_EDIT_APPLY_F2 );
	SetWin_Text( this, IDC_EDIT_WEAR_BRAWLER_F2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SWORDSMAN_F2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ARCHER_F2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SHAMAN_F2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_EXTREME_F2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_GUNNER_F2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ASSASSIN_F2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_TRICKER_F2, strTEXT.c_str() );
}

void CPageEdit1::OnBnClickedButtonClearF2()
{
	std::string strTEXT = "";
	SetWin_Text( this, IDC_EDIT_WEAR_BRAWLER_F2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SWORDSMAN_F2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ARCHER_F2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SHAMAN_F2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_EXTREME_F2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_GUNNER_F2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ASSASSIN_F2, strTEXT.c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_TRICKER_F2, strTEXT.c_str() );
}

void CPageEdit1::OnCbnSelchangeComboVehicle()
{
	InitVehicleTypes();
}

void CPageEdit1::OnBnClickedButtonAssassinM()
{
	LoadWearingfile(IDC_EDIT_WEAR_ASSASSIN_M );
}

void CPageEdit1::OnBnClickedButtonAssassinF()
{
	LoadWearingfile(IDC_EDIT_WEAR_ASSASSIN_F );
}

void CPageEdit1::OnBnClickedButtonTrickerM()
{
	LoadWearingfile(IDC_EDIT_WEAR_TRICKER_M );
}

void CPageEdit1::OnBnClickedButtonTrickerF()
{
	LoadWearingfile(IDC_EDIT_WEAR_TRICKER_F );
}

void CPageEdit1::OnBnClickedButtonAssassinM2()
{
	LoadWearingfile(IDC_EDIT_WEAR_ASSASSIN_M2 );
}

void CPageEdit1::OnBnClickedButtonAssassinF2()
{
	LoadWearingfile(IDC_EDIT_WEAR_ASSASSIN_F2 );
}

void CPageEdit1::OnBnClickedButtonTrickerM2()
{
	LoadWearingfile(IDC_EDIT_WEAR_TRICKER_M2 );
}

void CPageEdit1::OnBnClickedButtonTrickerF2()
{
	LoadWearingfile(IDC_EDIT_WEAR_TRICKER_F2 );
}

void CPageEdit1::OnBnClickedButtonVehicleDefaultColor1()
{
	/*bike color , Juver, 2017/11/13 */
	WORD wColor = GetWin_Num_int( this, IDC_EDIT_VEHICLE_COLOR_1 );
	DWORD dwColor = _HLIB::color_dword_word( wColor );

	CSelColorDlg dlg( BYTE( _HLIB::color_get_int_r( dwColor) ), 
		BYTE( _HLIB::color_get_int_g( dwColor ) ), 
		BYTE( _HLIB::color_get_int_b( dwColor ) ), 
		this );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_EDIT_VEHICLE_COLOR_1, _HLIB::color_word_dword( _HLIB::color_argb_rgb(crNewColor) ) );
	}
}

void CPageEdit1::OnBnClickedButtonVehicleDefaultColor2()
{
	/*bike color , Juver, 2017/11/13 */
	WORD wColor = GetWin_Num_int( this, IDC_EDIT_VEHICLE_COLOR_2 );
	DWORD dwColor = _HLIB::color_dword_word( wColor );

	CSelColorDlg dlg( BYTE( _HLIB::color_get_int_r( dwColor) ), 
		BYTE( _HLIB::color_get_int_g( dwColor ) ), 
		BYTE( _HLIB::color_get_int_b( dwColor ) ), 
		this );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_EDIT_VEHICLE_COLOR_2, _HLIB::color_word_dword( _HLIB::color_argb_rgb(crNewColor) ) );
	}
}

void CPageEdit1::OnBnClickedButtonItemDefaultColor1()
{
	/*item color, Juver, 2018/01/05 */
	WORD wColor = GetWin_Num_int( this, IDC_EDIT_ITEM_COLOR_1 );
	DWORD dwColor = _HLIB::color_dword_word( wColor );

	CSelColorDlg dlg( BYTE( _HLIB::color_get_int_r( dwColor) ), 
		BYTE( _HLIB::color_get_int_g( dwColor ) ), 
		BYTE( _HLIB::color_get_int_b( dwColor ) ), 
		this );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_EDIT_ITEM_COLOR_1, _HLIB::color_word_dword( _HLIB::color_argb_rgb(crNewColor) ) );
	}
}

void CPageEdit1::OnBnClickedButtonItemDefaultColor2()
{
	/*item color, Juver, 2018/01/05 */
	WORD wColor = GetWin_Num_int( this, IDC_EDIT_ITEM_COLOR_2 );
	DWORD dwColor = _HLIB::color_dword_word( wColor );

	CSelColorDlg dlg( BYTE( _HLIB::color_get_int_r( dwColor) ), 
		BYTE( _HLIB::color_get_int_g( dwColor ) ), 
		BYTE( _HLIB::color_get_int_b( dwColor ) ), 
		this );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_EDIT_ITEM_COLOR_2, _HLIB::color_word_dword( _HLIB::color_argb_rgb(crNewColor) ) );
	}
}
