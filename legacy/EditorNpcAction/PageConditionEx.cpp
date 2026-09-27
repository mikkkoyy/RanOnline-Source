#include "stdafx.h"
#include "EditorNpcAction.h"
#include "PageCondition.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"

#include "GLogicData.h"
#include "GLItemMan.h"
#include "GLQuest.h"
#include "GLQuestMan.h"

BOOL CPageCondition::DataDefault()
{
	SetWin_Combo_Init ( this, IDC_COMBO_LEVEL_SIGN, strCONDITIONSIGN, CONDITIONSIGN_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_ELEMENTAL_SIGN, strCONDITIONSIGN, CONDITIONSIGN_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_BRIGHTPOINT_SIGN, strCONDITIONSIGN, CONDITIONSIGN_SIZE );

	std::string strHOUR[24] = 
	{
		"Hour:00", "Hour:01", "Hour:02", "Hour:03", "Hour:04", "Hour:05", 
		"Hour:06", "Hour:07", "Hour:08", "Hour:09", "Hour:10", "Hour:11", 
		"Hour:12", "Hour:13", "Hour:14", "Hour:15", "Hour:16", "Hour:17", 
		"Hour:18", "Hour:19", "Hour:20", "Hour:21", "Hour:22", "Hour:23", 
	};

	SetWin_Combo_Init ( this, IDC_COMBO_TIME_BEGIN, strHOUR, 24 );
	SetWin_Combo_Init ( this, IDC_COMBO_TIME_END, strHOUR, 24 );

	m_comboSchool.ResetContent();

	for ( DWORD i = 0; i < MAX_SCHOOL; i++ )
	{
		std::string strSCHOOLNAME = GLCONST_CHAR::strSCHOOLNAME[i];
		if ( strSCHOOLNAME.empty() )
		{
			int nIndex = m_comboSchool.AddString ( "All School" );
			m_comboSchool.SetItemData ( nIndex, MAX_SCHOOL );
			break;
		}

		int nIndex = m_comboSchool.AddString ( strSCHOOLNAME.c_str() );
		m_comboSchool.SetItemData ( nIndex, i );
	}

	return TRUE;
}

BOOL CPageCondition::DataUpdate()
{
	SetWin_Check ( this, IDC_CHECK_REQ_ENABLE, m_DummyCondition.m_bEnable );

	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_BRAWLER_M, (m_DummyCondition.m_dwClass&GLCC_BRAWLER_M) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SWORDSMAN_M, (m_DummyCondition.m_dwClass&GLCC_SWORDSMAN_M) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ARCHER_F, (m_DummyCondition.m_dwClass&GLCC_ARCHER_W) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SHAMAN_F, (m_DummyCondition.m_dwClass&GLCC_SHAMAN_W) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_BRAWLER_F, (m_DummyCondition.m_dwClass&GLCC_BRAWLER_W) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SWORDSMAN_F, (m_DummyCondition.m_dwClass&GLCC_SWORDSMAN_W) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ARCHER_M, (m_DummyCondition.m_dwClass&GLCC_ARCHER_M) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SHAMAN_M, (m_DummyCondition.m_dwClass&GLCC_SHAMAN_M) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_EXTREME_M, (m_DummyCondition.m_dwClass&GLCC_EXTREME_M) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_EXTREME_F, (m_DummyCondition.m_dwClass&GLCC_EXTREME_W) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_GUNNER_M, (m_DummyCondition.m_dwClass&GLCC_GUNNER_M) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_GUNNER_F, (m_DummyCondition.m_dwClass&GLCC_GUNNER_W) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ASSASSIN_M, (m_DummyCondition.m_dwClass&GLCC_ASSASSIN_M) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ASSASSIN_F, (m_DummyCondition.m_dwClass&GLCC_ASSASSIN_W) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_TRICKER_M, (m_DummyCondition.m_dwClass&GLCC_TRICKER_M) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_TRICKER_F, (m_DummyCondition.m_dwClass&GLCC_TRICKER_W) );

	SetWin_Enable ( this, IDC_EDIT_LEVEL, m_DummyCondition.IsTERM_LEVEL() );
	SetWin_Enable ( this, IDC_EDIT_ELEMENTAL, m_DummyCondition.IsTERM_ELEMENTAL() );
	SetWin_Enable ( this, IDC_EDIT_BRIGHTPOINT, m_DummyCondition.IsTERM_ACTIONP() );

	SetWin_Num_int ( this, IDC_EDIT_LEVEL, m_DummyCondition.m_wLevel);
	SetWin_Combo_Sel ( this, IDC_COMBO_LEVEL_SIGN, strCONDITIONSIGN[m_DummyCondition.m_signLevel ].c_str() );

	SetWin_Num_int ( this, IDC_EDIT_ELEMENTAL, m_DummyCondition.m_nElemental );
	SetWin_Combo_Sel ( this, IDC_COMBO_ELEMENTAL_SIGN, strCONDITIONSIGN[m_DummyCondition.m_signElemental].c_str() );

	SetWin_Num_int ( this, IDC_EDIT_BRIGHTPOINT, m_DummyCondition.m_nActionPoint );
	SetWin_Combo_Sel ( this, IDC_COMBO_BRIGHTPOINT_SIGN, strCONDITIONSIGN[m_DummyCondition.m_signActionPoint].c_str() );

	std::string strSCHOOLNAME = "All School";
	if ( m_DummyCondition.m_wSchool!=MAX_SCHOOL )		
		strSCHOOLNAME = GLCONST_CHAR::strSCHOOLNAME[m_DummyCondition.m_wSchool];
	SetWin_Combo_Sel ( this, IDC_COMBO_SCHOOL, strSCHOOLNAME.c_str() );

	SetWin_Num_int ( this, IDC_EDIT_MONEY, m_DummyCondition.m_dwMONEY );
	SetWin_Num_int ( this, IDC_EDIT_PARTY, m_DummyCondition.m_dwPARTYMEN_NUM );

	UpdateTime();
	UpdateItem();
	UpdateSkill();
	UpdateSkillDis();
	UpdateQuestCom();
	UpdateQuestDis();
	UpdateQuestProg();
	UpdateBuff(); /*npc talk buff condition, Juver, 2017/10/03 */

	return TRUE;
}

BOOL CPageCondition::DataSave()
{
	m_DummyCondition.m_bEnable = GetWin_Check ( this, IDC_CHECK_REQ_ENABLE )? true:false;

	m_DummyCondition.m_signLevel = (EM_CONDITIONSIGN) GetWin_Combo_Sel ( this, IDC_COMBO_LEVEL_SIGN );
	m_DummyCondition.m_wLevel = (WORD) GetWin_Num_int ( this, IDC_EDIT_LEVEL );

	m_DummyCondition.m_dwClass = GLCC_NONE;
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_BRAWLER_M, m_DummyCondition.m_dwClass, GLCC_BRAWLER_M );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_SWORDSMAN_M, m_DummyCondition.m_dwClass, GLCC_SWORDSMAN_M );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_ARCHER_F, m_DummyCondition.m_dwClass, GLCC_ARCHER_W );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_SHAMAN_F, m_DummyCondition.m_dwClass, GLCC_SHAMAN_W );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_BRAWLER_F, m_DummyCondition.m_dwClass, GLCC_BRAWLER_W );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_SWORDSMAN_F, m_DummyCondition.m_dwClass, GLCC_SWORDSMAN_W );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_ARCHER_M, m_DummyCondition.m_dwClass, GLCC_ARCHER_M );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_SHAMAN_M, m_DummyCondition.m_dwClass, GLCC_SHAMAN_M );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_EXTREME_M, m_DummyCondition.m_dwClass, GLCC_EXTREME_M );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_EXTREME_F, m_DummyCondition.m_dwClass, GLCC_EXTREME_W );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_GUNNER_M, m_DummyCondition.m_dwClass, GLCC_GUNNER_M );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_GUNNER_F, m_DummyCondition.m_dwClass, GLCC_GUNNER_W );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_ASSASSIN_M, m_DummyCondition.m_dwClass, GLCC_ASSASSIN_M );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_ASSASSIN_F, m_DummyCondition.m_dwClass, GLCC_ASSASSIN_W );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_TRICKER_M, m_DummyCondition.m_dwClass, GLCC_TRICKER_M );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_TRICKER_F, m_DummyCondition.m_dwClass, GLCC_TRICKER_W );

	m_DummyCondition.m_wSchool = (WORD) GetWin_Combo_Sel ( this, IDC_COMBO_SCHOOL );

	m_DummyCondition.m_signElemental = (EM_CONDITIONSIGN) GetWin_Combo_Sel ( this, IDC_COMBO_ELEMENTAL_SIGN );
	m_DummyCondition.m_nElemental = GetWin_Num_int ( this, IDC_EDIT_ELEMENTAL );

	m_DummyCondition.m_signActionPoint = (EM_CONDITIONSIGN) GetWin_Combo_Sel ( this, IDC_COMBO_BRIGHTPOINT_SIGN );
	m_DummyCondition.m_nActionPoint = GetWin_Num_int ( this, IDC_EDIT_BRIGHTPOINT );

	m_DummyCondition.m_bTime = GetWin_Check ( this, IDC_CHECK_TIME );
	m_DummyCondition.m_dwTimeBegin = GetWin_Combo_Sel ( this, IDC_COMBO_TIME_BEGIN );
	m_DummyCondition.m_dwTimeEnd = GetWin_Combo_Sel ( this, IDC_COMBO_TIME_END );

	m_DummyCondition.m_dwMONEY = GetWin_Num_int ( this, IDC_EDIT_MONEY );
	m_DummyCondition.m_dwPARTYMEN_NUM = GetWin_Num_int ( this, IDC_EDIT_PARTY );

	return TRUE;
}

void CPageCondition::UpdateTime()
{
	SetWin_Check ( this, IDC_CHECK_TIME, m_DummyCondition.m_bTime );
	SetWin_Enable ( this, IDC_COMBO_TIME_BEGIN, m_DummyCondition.IsTERM_TIME() );
	SetWin_Enable ( this, IDC_COMBO_TIME_END, m_DummyCondition.IsTERM_TIME() );
	SetWin_Combo_Sel ( this, IDC_COMBO_TIME_BEGIN, m_DummyCondition.m_dwTimeBegin );
	SetWin_Combo_Sel ( this, IDC_COMBO_TIME_END, m_DummyCondition.m_dwTimeEnd );
}

void CPageCondition::UpdateItem()
{
	SetWin_Enable ( this, IDC_BUTTON_ITEM_DEL, m_DummyCondition.IsTERM_ITEM() );
	SetWin_Enable ( this, IDC_BUTTON_ITEM_DELALL, m_DummyCondition.IsTERM_ITEM() );

	m_ListItem.DeleteAllItems();	
	m_ListItem.SetRedraw( FALSE );

	SNpcTalkCondition::HAVEITEM_ITER iter = m_DummyCondition.m_vecHaveItem.begin();
	SNpcTalkCondition::HAVEITEM_ITER iter_end = m_DummyCondition.m_vecHaveItem.end();
	int nCount = 0;
	for ( ; iter!=iter_end; ++iter )
	{		
		SNPC_ITEM snpc_item = (*iter);
		SNATIVEID sID( snpc_item.dwItemID );

		CString strNAME = "NO ITEM";
		SITEM *pItem = GLItemMan::GetInstance().GetItem ( sID.wMainID, sID.wSubID );
		if ( pItem )	
			strNAME = pItem->GetName();

		CString strTemp;
		strTemp.Format ( "[%d/%d] %s (Num:%d)", sID.wMainID, sID.wSubID, strNAME, snpc_item.wItemNum );

		int nIndex = m_ListItem.InsertItem( nCount, strTemp );
		m_ListItem.SetItemData ( nIndex, snpc_item.dwItemID );

		nCount++;
	}	

	m_ListItem.SetRedraw( TRUE );

	if( m_CallPage == PAGE_TALK )
	{
		if( m_pSheetTab->m_PageTalk.GetActionType() == SNpcTalk::EM_BASIC && m_pSheetTab->m_PageTalk.GetActionNO() == SNpcTalk::EM_ITEM_TRADE )
		{
			for( int i = 0; i < (int)m_DummyCondition.m_vecHaveItem.size(); i++  )
			{	
				BOOL bFind = FALSE;
				for( int j = 0; j < MAX_NEEDITEM_COUNT; j++ )
				{
					if( m_TalkNeedItem[j] == m_DummyCondition.m_vecHaveItem[i] )
					{
						bFind = TRUE;
						break;
					}
				}

				m_ListItem.SetCheck(i,bFind);
			}
		}
	}

	for( BYTE i = 0; i < MAX_NEEDITEM_COUNT; i++ )
	{
		m_TalkNeedItem[i].Reset();
	}

	SNATIVEID sidnone( false );
	SetWin_Num_int( this, IDC_EDIT_ITEM_MID, sidnone.wMainID );
	SetWin_Num_int( this, IDC_EDIT_ITEM_SID, sidnone.wSubID );
	SetWin_Num_int( this, IDC_EDIT_ITEM_NUM, 0 );
}

void CPageCondition::UpdateSkill()
{
	SetWin_Enable ( this, IDC_BUTTON_SKILL_DEL, m_DummyCondition.IsTERM_SKILL() );
	SetWin_Enable ( this, IDC_BUTTON_SKILL_DELALL, m_DummyCondition.IsTERM_SKILL() );

	m_listSkill.ResetContent();
	m_listSkill.SetRedraw( FALSE );

	SNpcTalkCondition::LEARNSKILL_ITER iter = m_DummyCondition.m_vecLearnSkill.begin();
	SNpcTalkCondition::LEARNSKILL_ITER iter_end = m_DummyCondition.m_vecLearnSkill.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SSkillCondition sSkillCondition = (*iter);
		SNATIVEID sID;
		sID.dwID = sSkillCondition.dwNID;

		CString strNAME = "NO SKILL";
		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sID );
		if ( pSkill )	
			strNAME = pSkill->GetName();

		CString strTemp;
		strTemp.Format ( "[%d/%d] %s Lev.%d", sID.wMainID,
			sID.wSubID, strNAME, sSkillCondition.wLevel );

		int nIndex = m_listSkill.AddString ( strTemp );
		m_listSkill.SetItemData ( nIndex, sSkillCondition.dwNID );
	}

	m_listSkill.SetRedraw( TRUE );

	SNATIVEID sidnone( false );
	SetWin_Num_int( this, IDC_EDIT_SKILL_MID, sidnone.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SKILL_SID, sidnone.wSubID );
	SetWin_Num_int( this, IDC_EDIT_SKILL_LEVEL, 1 );
}

void CPageCondition::UpdateSkillDis()
{
	m_listDisSkill.ResetContent();
	m_listDisSkill.SetRedraw( FALSE );

	SNpcTalkCondition::PROCSKILL_ITER iter = m_DummyCondition.m_vecDisSkill.begin();
	SNpcTalkCondition::PROCSKILL_ITER iter_end = m_DummyCondition.m_vecDisSkill.end();
	for ( ; iter!=iter_end; ++iter )
	{
		DWORD dwID = (*iter);

		CString strNAME = "OFF SKILL";
		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( SNATIVEID(dwID) );
		if ( pSkill )	
			strNAME.Format ( "[%d/%d] %s", pSkill->m_sBASIC.sNATIVEID.wMainID, pSkill->m_sBASIC.sNATIVEID.wSubID,  pSkill->GetName() );

		int nIndex = m_listDisSkill.AddString ( strNAME );
		m_listDisSkill.SetItemData ( nIndex, dwID );
	}

	m_listDisSkill.SetRedraw( TRUE );

	SNATIVEID sidnone( false );
	SetWin_Num_int( this, IDC_EDIT_DIS_SKILL_MID, sidnone.wMainID );
	SetWin_Num_int( this, IDC_EDIT_DIS_SKILL_SID, sidnone.wSubID );
}

void CPageCondition::UpdateQuestCom()
{
	m_listComQuest.ResetContent();
	m_listComQuest.SetRedraw( FALSE );

	SNpcTalkCondition::PROCQUEST_ITER iter = m_DummyCondition.m_vecCompleteQuest.begin();
	SNpcTalkCondition::PROCQUEST_ITER iter_end = m_DummyCondition.m_vecCompleteQuest.end();
	for ( ; iter!=iter_end; ++iter )
	{
		DWORD dwID = (*iter);

		CString strNAME = "OFF QUEST";
		GLQUEST* pQuest = GLQuestMan::GetInstance().Find ( dwID );
		if ( pQuest )
			strNAME.Format ( "[%d] %s", dwID, pQuest->GetTITLE() );

		int nIndex = m_listComQuest.AddString ( strNAME );
		m_listComQuest.SetItemData ( nIndex, dwID );
	}

	m_listComQuest.SetRedraw( TRUE );

	SetWin_Num_int( this, IDC_EDIT_COMQUEST_ID, UINT_MAX );
}

void CPageCondition::UpdateQuestDis()
{
	m_listDisQuest.ResetContent();
	m_listDisQuest.SetRedraw( FALSE );

	SNpcTalkCondition::PROCQUEST_ITER iter = m_DummyCondition.m_vecDisQuest.begin();
	SNpcTalkCondition::PROCQUEST_ITER iter_end = m_DummyCondition.m_vecDisQuest.end();
	for ( ; iter!=iter_end; ++iter )
	{
		DWORD dwID = (*iter);

		CString strNAME = "OFF QUEST";
		GLQUEST* pQuest = GLQuestMan::GetInstance().Find ( dwID );
		if ( pQuest )
			strNAME.Format ( "[%d] %s", dwID, pQuest->GetTITLE() );

		int nIndex = m_listDisQuest.AddString ( strNAME );
		m_listDisQuest.SetItemData ( nIndex, dwID );
	}

	m_listDisQuest.SetRedraw( TRUE );

	SetWin_Num_int( this, IDC_EDIT_DIS_QUEST_ID, UINT_MAX );
}

void CPageCondition::UpdateQuestProg()
{
	SetWin_Num_int ( this, IDC_EDIT_QUEST_ID, 65535 );
	SetWin_Text ( this, IDC_EDIT_QUEST, "" );
	SetWin_Text ( this, IDC_EDIT_QUEST_STEP, "" );

	m_listQuestStep.ResetContent();
	m_listQuestStep.SetRedraw( FALSE );

	if ( m_DummyCondition.IsTERM_QUEST() )
	{
		GLQUEST* pQuest = GLQuestMan::GetInstance().Find ( m_DummyCondition.m_dwQUEST_NID );
		if ( pQuest )
		{
			CString strNAME;
			strNAME.Format ( "[%d] %s", m_DummyCondition.m_dwQUEST_NID, pQuest->GetTITLE() );
			SetWin_Text ( this, IDC_EDIT_QUEST, strNAME );

			GLQUEST::VECQSTEP& sSTEP = pQuest->GetSTEP();
			GLQUEST::VECQSTEP_ITER iter = sSTEP.begin();
			GLQUEST::VECQSTEP_ITER iter_end = sSTEP.end();
			for ( int i=0; iter!=iter_end; ++iter, ++i )
			{
				GLQUEST_STEP &sSTEP = (*iter);
				CString strTEMP;
				strTEMP.Format( "[%d][%s]", i, sSTEP.m_strTITLE.c_str() );
				int nIndex = m_listQuestStep.AddString ( strTEMP.GetString() );
				m_listQuestStep.SetItemData ( nIndex, nIndex );

				if ( i == m_DummyCondition.m_dwQUEST_STEP )
				{
					SetWin_Text ( this, IDC_EDIT_QUEST_STEP, strTEMP.GetString() );
				}
			}
		}	
	}

	m_listQuestStep.SetRedraw( TRUE );

	SetWin_Num_int( this, IDC_EDIT_QUEST_INSERT_ID, UINT_MAX );
}

/*npc talk buff condition, Juver, 2017/10/03 */
void CPageCondition::UpdateBuff()
{
	SetWin_Enable ( this, IDC_BUTTON_BUFF_DEL, m_DummyCondition.IsTERM_NPCBUFF() );
	SetWin_Enable ( this, IDC_BUTTON_BUFF_DELALL, m_DummyCondition.IsTERM_NPCBUFF() );

	m_listBuff.ResetContent();
	m_listBuff.SetRedraw( FALSE );

	SNpcTalkCondition::HAVEBUFF_ITER iter = m_DummyCondition.m_vecBuff.begin();
	SNpcTalkCondition::HAVEBUFF_ITER iter_end = m_DummyCondition.m_vecBuff.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SNPC_BUFF npc_buff = (*iter);
		SNATIVEID sID;
		sID.dwID = npc_buff.dwID;

		CString strNAME = "NO BUFF";
		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sID );
		if ( pSkill )	
			strNAME = pSkill->GetName();

		CString strTemp;
		strTemp.Format ( "[%d/%d] %s Lev.%d", sID.wMainID,
			sID.wSubID, strNAME, npc_buff.wLevel );

		int nIndex = m_listBuff.AddString ( strTemp );
		m_listBuff.SetItemData ( nIndex, npc_buff.dwID );
	}

	m_listBuff.SetRedraw( TRUE );

	SNATIVEID sidnone( false );
	SetWin_Num_int( this, IDC_EDIT_BUFF_MID, sidnone.wMainID );
	SetWin_Num_int( this, IDC_EDIT_BUFF_SID, sidnone.wSubID );
	SetWin_Num_int( this, IDC_EDIT_BUFF_LEVEL, 1 );
}