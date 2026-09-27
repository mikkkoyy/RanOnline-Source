// PageCondition.cpp : implementation file
//

#include "stdafx.h"
#include "EditorNpcAction.h"
#include "PageCondition.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"

#include "DlgItem.h"
#include "DlgQuest.h"
#include "DlgSkill.h"
#include "DlgSkill2.h"
#include "DlgInputNum.h"

// CPageCondition dialog

IMPLEMENT_DYNAMIC(CPageCondition, CPropertyPage)

CPageCondition::CPageCondition( LOGFONT logfont )
	: CPropertyPage(CPageCondition::IDD)
	, m_pFont ( NULL )
	, m_pOrgCondition(NULL)
{
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	for( int i = 0; i < MAX_NEEDITEM_COUNT; i++ )
	{
		m_TalkNeedItem[i].Reset();
	}
}

CPageCondition::~CPageCondition()
{
	SAFE_DELETE ( m_pFont );
}

void CPageCondition::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LISTCONT_ITEM, m_ListItem );
	DDX_Control(pDX, IDC_LIST_SKILL, m_listSkill );
	DDX_Control(pDX, IDC_LIST_DIS_SKILL, m_listDisSkill );
	DDX_Control(pDX, IDC_LIST_DIS_QUEST, m_listDisQuest );
	DDX_Control(pDX, IDC_LIST_COMQUEST, m_listComQuest );
	DDX_Control(pDX, IDC_LIST_QUEST_STEP, m_listQuestStep );
	DDX_Control(pDX, IDC_LIST_BUFF, m_listBuff ); /*npc talk buff condition, Juver, 2017/10/03 */
	DDX_Control(pDX, IDC_COMBO_SCHOOL, m_comboSchool );
}


BEGIN_MESSAGE_MAP(CPageCondition, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnBnClickedButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_ADD, OnBnClickedButtonItemAdd)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_DEL, OnBnClickedButtonItemDel)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_DELALL, OnBnClickedButtonItemDelall)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_INSERT, OnBnClickedButtonItemInsert)
	ON_BN_CLICKED(IDC_BUTTON_SKILL_ADD, OnBnClickedButtonSkillAdd)
	ON_BN_CLICKED(IDC_BUTTON_SKILL_DEL, OnBnClickedButtonSkillDel)
	ON_BN_CLICKED(IDC_BUTTON_SKILL_DELALL, OnBnClickedButtonSkillDelall)
	ON_BN_CLICKED(IDC_BUTTON_SKILL_INSERT, OnBnClickedButtonSkillInsert)
	ON_BN_CLICKED(IDC_BUTTON_DIS_SKILL_ADD, OnBnClickedButtonDisSkillAdd)
	ON_BN_CLICKED(IDC_BUTTON_DIS_SKILL_DEL, OnBnClickedButtonDisSkillDel)
	ON_BN_CLICKED(IDC_BUTTON_DIS_SKILL_DELLALL, OnBnClickedButtonDisSkillDellall)
	ON_BN_CLICKED(IDC_BUTTON_DIS_SKILL_INSERT, OnBnClickedButtonDisSkillInsert)
	ON_BN_CLICKED(IDC_BUTTON_COMQUEST_ADD, OnBnClickedButtonComquestAdd)
	ON_BN_CLICKED(IDC_BUTTON_COMQUEST_DEL, OnBnClickedButtonComquestDel)
	ON_BN_CLICKED(IDC_BUTTON_COMQUEST_DELALL, OnBnClickedButtonComquestDelall)
	ON_BN_CLICKED(IDC_BUTTON_COMQUEST_INSERT, OnBnClickedButtonComquestInsert)
	ON_BN_CLICKED(IDC_BUTTON_DIS_QUEST_ADD, OnBnClickedButtonDisQuestAdd)
	ON_BN_CLICKED(IDC_BUTTON_DIS_QUEST_DEL, OnBnClickedButtonDisQuestDel)
	ON_BN_CLICKED(IDC_BUTTON_DIS_QUEST_DELALL, OnBnClickedButtonDisQuestDelall)
	ON_BN_CLICKED(IDC_BUTTON_DIS_QUEST_INSERT, OnBnClickedButtonDisQuestInsert)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_ADD, OnBnClickedButtonQuestAdd)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_DEL, OnBnClickedButtonQuestDel)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_INSERT, OnBnClickedButtonQuestInsert)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_STEP, OnBnClickedButtonQuestStep)
	ON_CBN_SELCHANGE(IDC_COMBO_LEVEL_SIGN, OnCbnSelchangeComboLevelSign)
	ON_CBN_SELCHANGE(IDC_COMBO_ELEMENTAL_SIGN, OnCbnSelchangeComboElementalSign)
	ON_CBN_SELCHANGE(IDC_COMBO_BRIGHTPOINT_SIGN, OnCbnSelchangeComboBrightpointSign)
	ON_BN_CLICKED(IDC_CHECK_TIME, OnBnClickedCheckTime)
	ON_BN_CLICKED(IDC_BUTTON_ALL_MALE, OnBnClickedButtonAllMale)
	ON_BN_CLICKED(IDC_BUTTON_ALL_FEMALE, OnBnClickedButtonAllFemale)
	ON_BN_CLICKED(IDC_BUTTON_ALL, OnBnClickedButtonAll)
	ON_BN_CLICKED(IDC_BUTTON_NOTHING, OnBnClickedButtonNothing)
	ON_BN_CLICKED(IDC_BUTTON_BUFF_INSERT, OnBnClickedButtonBuffInsert)
	ON_BN_CLICKED(IDC_BUTTON_BUFF_ADD, OnBnClickedButtonBuffAdd)
	ON_BN_CLICKED(IDC_BUTTON_BUFF_DEL, OnBnClickedButtonBuffDel)
	ON_BN_CLICKED(IDC_BUTTON_BUFF_DELALL, OnBnClickedButtonBuffDelall)
END_MESSAGE_MAP()


// CPageCondition message handlers
BOOL CPageCondition::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	LV_COLUMN lvColumn;
	RECT rectCtrl;
	LONG lnWidth;
	m_ListItem.GetClientRect ( &rectCtrl );
	lnWidth = rectCtrl.right - rectCtrl.left;
	lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.pszText = "Information";
	lvColumn.iSubItem = 0;
	lvColumn.cx = lnWidth ;
	m_ListItem.InsertColumn( 0, &lvColumn );

	DataDefault();
	DataUpdate();

	return TRUE;  
}

void CPageCondition::OnBnClickedButtonCancel()
{
	if ( m_pSheetTab )
		m_pSheetTab->SetActivePage ( m_CallPage );
}

void CPageCondition::OnBnClickedButtonOk()
{
	DataSave();

	if ( m_pOrgCondition )
	{
		*m_pOrgCondition = m_DummyCondition;
		if( m_pOrgCondition->EmptyCondition() )
		{
			CString strTemp;
			strTemp.Format( "A condition that has been set is empty. \n Unless it is intended, should be deleted by pressing Del button." );
			MessageBox( strTemp, "Warning Code 1226", MB_OK );
		}
	}

	if( m_CallPage==PAGE_TALK )
	{
		if( m_pSheetTab->m_PageTalk.GetActionType() == SNpcTalk::EM_BASIC && m_pSheetTab->m_PageTalk.GetActionNO() == SNpcTalk::EM_ITEM_TRADE )
		{
			int nItemCount = m_ListItem.GetItemCount();
			int nIndex = 0;

			for( int i = 0; i < nItemCount; i++  )
			{		
				if( m_ListItem.GetCheck(i) ) 
					nIndex++;	
			}

			if( nIndex > MAX_NEEDITEM_COUNT )
			{
				MessageBox( "Item trade option is selected \n Request Item must not be more than five in check.!", "Error", MB_OK );
				return;
			}

			if( nIndex == 0 )
			{
				MessageBox( "Item trade option is selected \n You need to check atleast one Request Item.!", "Error", MB_OK );
				return;
			}

			nIndex = 0;

			for( int i = 0; i < MAX_NEEDITEM_COUNT; i++ )
			{
				SNPC_ITEM snpc_item;
				snpc_item.Reset();
				m_pSheetTab->m_PageTalk.SetNeedItem( i, snpc_item );
			}

			for( int i = 0; i < nItemCount; i++  )
			{		
				if( m_ListItem.GetCheck(i) ) 
				{
					m_pSheetTab->m_PageTalk.SetNeedItem( nIndex, m_DummyCondition.m_vecHaveItem[i] );
					nIndex++;
				}
			}
		}
	}

	m_pSheetTab->SetActivePage ( m_CallPage );
	if ( m_CallPage==PAGE_ANSWER )		m_pSheetTab->m_PageAnswer.DataShow();
	else if ( m_CallPage==PAGE_TALK )	m_pSheetTab->m_PageTalk.DataUpdate();
}

BOOL CPageCondition::SetCondition ( SNpcTalkCondition* pConditon )
{
	m_DummyCondition.Reset ();	

	m_pOrgCondition = pConditon;
	m_DummyCondition = (*pConditon);

	if( m_CallPage==PAGE_ANSWER )
	{
		m_ListItem.SetExtendedStyle ( LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT );		
	}else{
		if( m_pSheetTab->m_PageTalk.GetActionType() == SNpcTalk::EM_BASIC && m_pSheetTab->m_PageTalk.GetActionNO() == SNpcTalk::EM_ITEM_TRADE )
		{
			m_ListItem.SetExtendedStyle ( LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT );
		}else{
			m_ListItem.SetExtendedStyle ( LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT );
		}

		for( BYTE i = 0; i < MAX_NEEDITEM_COUNT; i++ )
		{
			m_TalkNeedItem[i] = m_pSheetTab->m_PageTalk.GetNeedItem(i);
		}
	}

	return TRUE;
}
void CPageCondition::OnBnClickedButtonItemAdd()
{
	CDlgItem dlg;
	if( dlg.DoModal() == IDOK )
	{
		CDlgInputNum dlgNum;
		if ( dlgNum.DoModal() == IDOK )
		{
			SNPC_ITEM npc_item;
			npc_item.dwItemID = dlg.m_SELECTED.dwID;
			npc_item.wItemNum = (WORD)dlgNum.m_nInputNum;

			SNATIVEID sID = dlg.m_SELECTED;
			SITEM* pITEM = GLItemMan::GetInstance().GetItem( sID );
			if ( pITEM )
			{
				if ( npc_item.wItemNum == 0 )
					npc_item.wItemNum = pITEM->sDrugOp.wPileNum;

				m_DummyCondition.ADDHAVEITEM ( npc_item );
				UpdateItem();
			}
		}
	}
}

void CPageCondition::OnBnClickedButtonItemDel()
{
	int nIndex = (int)m_ListItem.GetFirstSelectedItemPosition() - 1;	
	if ( nIndex < 0 ) return ;
	DWORD NativeID = (DWORD) m_ListItem.GetItemData ( nIndex );
	SNATIVEID sID;
	sID.dwID = NativeID;

	CString strTemp;
	strTemp.Format ( "Do you want to delete Item?\n[%d/%d]", sID.wMainID, sID.wSubID );

	if ( MessageBox ( strTemp, "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_DummyCondition.DELHAVEITEM ( NativeID );
		UpdateItem();
	}
}

void CPageCondition::OnBnClickedButtonItemDelall()
{
	if ( MessageBox ( "Do you want to delete all Items?", "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_DummyCondition.m_vecHaveItem.clear ();
		UpdateItem();
	}
}

void CPageCondition::OnBnClickedButtonItemInsert()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_ITEM_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_ITEM_SID );
	WORD wItemNum = GetWin_Num_int( this, IDC_EDIT_ITEM_NUM );

	SITEM* pITEM = GLItemMan::GetInstance().GetItem( wMID, wSID );
	if ( !pITEM )
	{
		MessageBox( "Invalid Item!" );
		return;
	}

	SNPC_ITEM snpc_item;
	snpc_item.dwItemID = pITEM->sBasicOp.sNativeID.dwID;
	snpc_item.wItemNum = wItemNum;

	if ( snpc_item.wItemNum == 0 )
		snpc_item.wItemNum = pITEM->sDrugOp.wPileNum;

	m_DummyCondition.ADDHAVEITEM ( snpc_item );
	UpdateItem();
}

void CPageCondition::OnBnClickedButtonSkillAdd()
{
	CDlgSkill dlg;
	if( dlg.DoModal() == IDOK )
	{
		SNATIVEID sID = dlg.m_SELECTED;
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sID );
		if ( pSKILL )
		{
			SSkillCondition sCondition;
			sCondition.dwNID = pSKILL->m_sBASIC.sNATIVEID.dwID;
			sCondition.wLevel = dlg.m_wSELECTEDLEVEL;
			m_DummyCondition.ADDLEARNSKILL ( sCondition );
			UpdateSkill();
		}
	}
}

void CPageCondition::OnBnClickedButtonSkillDel()
{
	int nIndex = m_listSkill.GetCurSel ();
	if ( nIndex < 0 ) return ;
	DWORD dwNativeID = (DWORD) m_listSkill.GetItemData ( nIndex );
	SNATIVEID sID;
	sID.dwID = dwNativeID;

	CString strTemp;
	strTemp.Format ( "Do you want to delete Skill?\n[%d/%d]", sID.wMainID, sID.wSubID );

	if ( MessageBox ( strTemp, "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_DummyCondition.DELLEARNSKILL ( dwNativeID );
		UpdateSkill();
	}
}

void CPageCondition::OnBnClickedButtonSkillDelall()
{
	if ( MessageBox ( "Do you want to delete all Skills?", "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_DummyCondition.m_vecLearnSkill.clear();
		UpdateSkill();
	}
}

void CPageCondition::OnBnClickedButtonSkillInsert()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_SKILL_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_SKILL_SID );
	WORD wLEVEL = GetWin_Num_int( this, IDC_EDIT_SKILL_LEVEL );

	if ( wLEVEL > SKILL::MAX_LEVEL || wLEVEL < 0 )
	{
		MessageBox( "Invalid Skill Level!" );
		return;
	}

	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( wMID, wSID );
	if ( !pSKILL )
	{
		MessageBox( "Invalid Skill!" );
		return;
	}

	SSkillCondition sCondition;
	sCondition.dwNID = pSKILL->m_sBASIC.sNATIVEID.dwID;
	sCondition.wLevel = wLEVEL;
	m_DummyCondition.ADDLEARNSKILL ( sCondition );
	UpdateSkill();
}

void CPageCondition::OnBnClickedButtonDisSkillAdd()
{
	CDlgSkill2 dlg;
	if( dlg.DoModal() == IDOK )
	{
		SNATIVEID sID = dlg.m_SELECTED;
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sID );
		if ( pSKILL )
		{
			m_DummyCondition.ADDDISSKILL ( pSKILL->m_sBASIC.sNATIVEID.dwID );
			UpdateSkillDis();
		}
	}
}

void CPageCondition::OnBnClickedButtonDisSkillDel()
{
	int nIndex = m_listDisSkill.GetCurSel ();
	if ( LB_ERR==nIndex )	return;

	DWORD dwData = (DWORD) m_listDisSkill.GetItemData ( nIndex );
	SNATIVEID sID;
	sID.dwID = dwData;

	CString strTemp;
	strTemp.Format ( "Do you want to delete Skill?\n[%d/%d]", sID.wMainID, sID.wSubID );

	if ( MessageBox ( strTemp, "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_DummyCondition.DELDISSKILL ( dwData );
		UpdateSkillDis();
	}
}

void CPageCondition::OnBnClickedButtonDisSkillDellall()
{
	if ( MessageBox ( "Do you want to delete all Skills?", "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_DummyCondition.m_vecDisSkill.clear();
		UpdateSkillDis();
	}
}

void CPageCondition::OnBnClickedButtonDisSkillInsert()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_DIS_SKILL_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_DIS_SKILL_SID );

	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( wMID, wSID );
	if ( !pSKILL )
	{
		MessageBox( "Invalid Skill!" );
		return;
	}
	
	m_DummyCondition.ADDDISSKILL ( pSKILL->m_sBASIC.sNATIVEID.dwID );
	UpdateSkillDis();
}

void CPageCondition::OnBnClickedButtonComquestAdd()
{
	CDlgQuest dlg;
	if( dlg.DoModal() == IDOK )
	{
		DWORD dwQUEST = dlg.m_SelectedQuest;
		GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( dwQUEST );;
		if ( pQUEST )	
		{
			m_DummyCondition.ADDCOMQUEST ( pQUEST->m_sNID.dwID );
			UpdateQuestCom();
		}
	}
}

void CPageCondition::OnBnClickedButtonComquestDel()
{
	int nIndex = m_listComQuest.GetCurSel ();
	if ( LB_ERR==nIndex )	return;

	DWORD dwData = (DWORD) m_listComQuest.GetItemData ( nIndex );

	CString strTemp;
	strTemp.Format ( "Do you want to delete Quest?\n[%d]", dwData );

	if ( MessageBox ( strTemp, "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_DummyCondition.DELCOMQUEST ( dwData );
		UpdateQuestCom();
	}
}

void CPageCondition::OnBnClickedButtonComquestDelall()
{
	if ( MessageBox ( "Do you want to delete all Quests?", "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_DummyCondition.m_vecCompleteQuest.clear();
		UpdateQuestCom();
	}
}

void CPageCondition::OnBnClickedButtonComquestInsert()
{
	DWORD dwID = GetWin_Num_int( this, IDC_EDIT_COMQUEST_ID );
	GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( dwID );;
	if ( !pQUEST )	
	{
		MessageBox( "Invalid Quest!" );
		return;
	}

	m_DummyCondition.ADDCOMQUEST ( pQUEST->m_sNID.dwID );
	UpdateQuestCom();
}

void CPageCondition::OnBnClickedButtonDisQuestAdd()
{
	CDlgQuest dlg;
	if( dlg.DoModal() == IDOK )
	{
		DWORD dwQUEST = dlg.m_SelectedQuest;
		GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( dwQUEST );;
		if ( pQUEST )	
		{
			m_DummyCondition.ADDDISQUEST ( pQUEST->m_sNID.dwID );
			UpdateQuestDis();
		}
	}
}

void CPageCondition::OnBnClickedButtonDisQuestDel()
{
	int nIndex = m_listDisQuest.GetCurSel ();
	if ( LB_ERR==nIndex )	return;

	DWORD dwData = (DWORD) m_listDisQuest.GetItemData ( nIndex );

	CString strTemp;
	strTemp.Format ( "Do you want to delete Quest?\n[%d]", dwData );

	if ( MessageBox ( strTemp, "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_DummyCondition.DELDISQUEST ( dwData );
		UpdateQuestDis();
	}
}

void CPageCondition::OnBnClickedButtonDisQuestDelall()
{
	if ( MessageBox ( "Do you want to delete all Quests?", "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_DummyCondition.m_vecDisQuest.clear();
		UpdateQuestDis();
	}
}

void CPageCondition::OnBnClickedButtonDisQuestInsert()
{
	DWORD dwID = GetWin_Num_int( this, IDC_EDIT_DIS_QUEST_ID );
	GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( dwID );;
	if ( !pQUEST )	
	{
		MessageBox( "Invalid Quest!" );
		return;
	}

	m_DummyCondition.ADDDISQUEST ( pQUEST->m_sNID.dwID );
	UpdateQuestDis();
}

void CPageCondition::OnBnClickedButtonQuestAdd()
{
	CDlgQuest dlg;
	if( dlg.DoModal() == IDOK )
	{
		DWORD dwQUEST = dlg.m_SelectedQuest;
		GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( dwQUEST );;
		if ( pQUEST )	
		{
			m_DummyCondition.m_dwQUEST_NID = pQUEST->m_sNID.dwID;
			m_DummyCondition.m_dwQUEST_STEP = 0;
			UpdateQuestProg();
		}
	}
}

void CPageCondition::OnBnClickedButtonQuestDel()
{
	CString strTemp;
	strTemp.Format ( "Do you want to delete Quest?\n[%d]", m_DummyCondition.m_dwQUEST_NID );

	if ( MessageBox ( strTemp, "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_DummyCondition.RESET_QUEST ();
		UpdateQuestProg();
	}
}

void CPageCondition::OnBnClickedButtonQuestInsert()
{
	DWORD dwID = GetWin_Num_int( this, IDC_EDIT_QUEST_INSERT_ID );
	GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( dwID );;
	if ( !pQUEST )	
	{
		MessageBox( "Invalid Quest!" );
		return;
	}

	m_DummyCondition.m_dwQUEST_NID = pQUEST->m_sNID.dwID;
	m_DummyCondition.m_dwQUEST_STEP = 0;
	UpdateQuestProg();
}

void CPageCondition::OnBnClickedButtonQuestStep()
{
	int nIndex = m_listQuestStep.GetCurSel ();
	if ( LB_ERR==nIndex )	return;
	m_DummyCondition.m_dwQUEST_STEP = nIndex;
	UpdateQuestProg();
}

void CPageCondition::OnCbnSelchangeComboLevelSign()
{
	m_DummyCondition.m_signLevel = (EM_CONDITIONSIGN) GetWin_Combo_Sel ( this, IDC_COMBO_LEVEL_SIGN );
	SetWin_Enable ( this, IDC_EDIT_LEVEL, m_DummyCondition.IsTERM_LEVEL() );
}

void CPageCondition::OnCbnSelchangeComboElementalSign()
{
	m_DummyCondition.m_signElemental = (EM_CONDITIONSIGN) GetWin_Combo_Sel ( this, IDC_COMBO_ELEMENTAL_SIGN );
	SetWin_Enable ( this, IDC_EDIT_ELEMENTAL, m_DummyCondition.IsTERM_ELEMENTAL() );
}

void CPageCondition::OnCbnSelchangeComboBrightpointSign()
{
	m_DummyCondition.m_signActionPoint = (EM_CONDITIONSIGN) GetWin_Combo_Sel ( this, IDC_COMBO_BRIGHTPOINT_SIGN );
	SetWin_Enable ( this, IDC_EDIT_BRIGHTPOINT, m_DummyCondition.IsTERM_ACTIONP() );
}

void CPageCondition::OnBnClickedCheckTime()
{
	m_DummyCondition.m_bTime = GetWin_Check ( this, IDC_CHECK_TIME );
	UpdateTime();
}

void CPageCondition::UpdateClass( BOOL bMALE, BOOL bFEMALE )
{
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_BRAWLER_M, bMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SWORDSMAN_M, bMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ARCHER_M, bMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SHAMAN_M, bMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_EXTREME_M, bMALE );

	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ARCHER_F, bFEMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SHAMAN_F, bFEMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_BRAWLER_F, bFEMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SWORDSMAN_F, bFEMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_EXTREME_F, bFEMALE );

	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_GUNNER_M, bMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_GUNNER_F, bFEMALE );

	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ASSASSIN_M, bMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ASSASSIN_F, bFEMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_TRICKER_M, bMALE );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_TRICKER_F, bFEMALE );
}

void CPageCondition::OnBnClickedButtonAllMale()
{
	UpdateClass( TRUE, FALSE );
}

void CPageCondition::OnBnClickedButtonAllFemale()
{
	UpdateClass( FALSE, TRUE );
}

void CPageCondition::OnBnClickedButtonAll()
{
	UpdateClass( TRUE, TRUE );
}

void CPageCondition::OnBnClickedButtonNothing()
{
	UpdateClass( FALSE, FALSE );
}

void CPageCondition::OnBnClickedButtonBuffInsert()
{
	/*npc talk buff condition, Juver, 2017/10/03 */
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_BUFF_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_BUFF_SID );
	WORD wLEVEL = GetWin_Num_int( this, IDC_EDIT_BUFF_LEVEL );

	if ( wLEVEL > SKILL::MAX_LEVEL || wLEVEL < 0 )
	{
		MessageBox( "Invalid Skill Level!" );
		return;
	}

	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( wMID, wSID );
	if ( !pSKILL )
	{
		MessageBox( "Invalid Skill!" );
		return;
	}

	SNPC_BUFF npc_buff;
	npc_buff.dwID = pSKILL->m_sBASIC.sNATIVEID.dwID;
	npc_buff.wLevel = wLEVEL;
	m_DummyCondition.ADDBUFF ( npc_buff );
	UpdateBuff();
}

void CPageCondition::OnBnClickedButtonBuffAdd()
{
	/*npc talk buff condition, Juver, 2017/10/03 */
	CDlgSkill dlg;
	if( dlg.DoModal() == IDOK )
	{
		SNATIVEID sID = dlg.m_SELECTED;
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sID );
		if ( pSKILL )
		{
			SNPC_BUFF npc_buff;
			npc_buff.dwID = pSKILL->m_sBASIC.sNATIVEID.dwID;
			npc_buff.wLevel = dlg.m_wSELECTEDLEVEL;
			m_DummyCondition.ADDBUFF ( npc_buff );
			UpdateBuff();
		}
	}
}

void CPageCondition::OnBnClickedButtonBuffDel()
{
	/*npc talk buff condition, Juver, 2017/10/03 */
	int nIndex = m_listBuff.GetCurSel ();
	if ( nIndex < 0 ) return ;
	DWORD dwNativeID = (DWORD) m_listBuff.GetItemData ( nIndex );
	SNATIVEID sID;
	sID.dwID = dwNativeID;

	CString strTemp;
	strTemp.Format ( "Do you want to delete Buff?\n[%d/%d]", sID.wMainID, sID.wSubID );

	if ( MessageBox ( strTemp, "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_DummyCondition.DELBUFF ( dwNativeID );
		UpdateBuff();
	}
}

void CPageCondition::OnBnClickedButtonBuffDelall()
{
	if ( MessageBox ( "Do you want to delete all Buffs?", "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_DummyCondition.m_vecBuff.clear();
		UpdateBuff();
	}
}
