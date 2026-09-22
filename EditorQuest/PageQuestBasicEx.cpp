#include "stdafx.h"
#include "EditorQuest.h"
#include "EditorQuestDlg.h"
#include "PageQuestBasic.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"

BOOL CPageQuestBasic::DataDefault()
{
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

	std::string strTemp[4] =
	{
		"No Event",
		"Char Death",
		"Move to StartPoint",
		"Move to Specified Place"
	};

	SetWin_Combo_Init( this, IDC_COMBO_PROGRESS_START, strTemp, 4 );
	SetWin_Combo_Init( this, IDC_COMBO_PROGRESS_END, strTemp, 4 );
	SetWin_Combo_Init( this, IDC_COMBO_PROGRESS_FAIL, strTemp, 4 );

	SetWin_Combo_Init( this, IDC_COMBO_QUEST_AREA, COMMENT::QUEST_AREA, EMQUEST_AREA_SIZE );

	return TRUE;
}

BOOL CPageQuestBasic::DataShow()
{
	if ( !m_DummyQuest.pQUEST )	return FALSE;

	//basic
	SetWin_Num_int ( this, IDC_EDIT_EDITVERSION, m_DummyQuest.pQUEST->m_dwEDITVER );
	SetWin_Num_int ( this, IDC_EDIT_QUEST_NID, m_DummyQuest.pQUEST->m_sNID.dwID );
	SetWin_Text ( this, IDC_EDIT_QUEST_FILENAME, m_DummyQuest.strFILE.c_str() );
	SetWin_Text ( this, IDC_EDIT_TITLE, m_DummyQuest.pQUEST->m_strTITLE.c_str() );
	SetWin_Text ( this, IDC_EDIT_COMMENT, m_DummyQuest.pQUEST->m_strCOMMENT.c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_QUEST_AREA, COMMENT::QUEST_AREA[m_DummyQuest.pQUEST->m_emQuestArea].c_str() );

	SetWin_Check ( this, IDC_CHECK_REPEAT, m_DummyQuest.pQUEST->IsREPEAT() );
	SetWin_Check ( this, IDC_CHECK_AGAIN, m_DummyQuest.pQUEST->IsAGAIN() );
	SetWin_Check ( this, IDC_CHECK_GIVEUP, m_DummyQuest.pQUEST->IsGIVEUP() );
	SetWin_Check ( this, IDC_CHECK_PARTY, m_DummyQuest.pQUEST->IsPARTY() );
	SetWin_Check ( this, IDC_CHECK_PARTYQUEST, m_DummyQuest.pQUEST->IsPARTYQUEST() );
	SetWin_Check ( this, IDC_CHECK_ONLYONE_PROG, m_DummyQuest.pQUEST->IsONLYONE() );
	
	SetWin_Num_int ( this, IDC_EDIT_LIMIT_TIME, m_DummyQuest.pQUEST->m_dwLimitTime );
	SetWin_Num_int ( this, IDC_EDIT_LIMIT_PARTYNUM, m_DummyQuest.pQUEST->m_dwLimitPartyMemNum );
	SetWin_Check ( this, IDC_CHECK_LIMIT_NONDIE, m_DummyQuest.pQUEST->m_bNonDie );

	SetWin_Num_int ( this, IDC_EDIT_BEGIN_MONEY, m_DummyQuest.pQUEST->m_dwBeginMoney );
	SetWin_Num_int ( this, IDC_EDIT_BEGIN_PARTYNUM, m_DummyQuest.pQUEST->m_dwBeginPartyMemNum );

	
	//autostart
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_BRAWLER_M, (m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS&GLCC_BRAWLER_M) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SWORDSMAN_M, (m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS&GLCC_SWORDSMAN_M) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ARCHER_F, (m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS&GLCC_ARCHER_W) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SHAMAN_F, (m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS&GLCC_SHAMAN_W) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_BRAWLER_F, (m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS&GLCC_BRAWLER_W) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SWORDSMAN_F, (m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS&GLCC_SWORDSMAN_W) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ARCHER_M, (m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS&GLCC_ARCHER_M) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SHAMAN_M, (m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS&GLCC_SHAMAN_M) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_EXTREME_M, (m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS&GLCC_EXTREME_M) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_EXTREME_F, (m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS&GLCC_EXTREME_W) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_GUNNER_M, (m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS&GLCC_GUNNER_M) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_GUNNER_F, (m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS&GLCC_GUNNER_W) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ASSASSIN_M, (m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS&GLCC_ASSASSIN_M) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ASSASSIN_F, (m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS&GLCC_ASSASSIN_W) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_TRICKER_M, (m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS&GLCC_TRICKER_M) );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_TRICKER_F, (m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS&GLCC_TRICKER_W) );

	std::string strSCHOOLNAME = "All School";
	if ( m_DummyQuest.pQUEST->m_sSTARTOPT.wSCHOOL!=MAX_SCHOOL )		
		strSCHOOLNAME = GLCONST_CHAR::strSCHOOLNAME[m_DummyQuest.pQUEST->m_sSTARTOPT.wSCHOOL];
	SetWin_Combo_Sel ( this, IDC_COMBO_SCHOOL, strSCHOOLNAME.c_str() );
	

	SetWin_Num_int ( this, IDC_EDIT_START_LEVEL, m_DummyQuest.pQUEST->m_sSTARTOPT.wLEVEL );
	SetWin_Num_int ( this, IDC_EDIT_START_ITEM_MID, m_DummyQuest.pQUEST->m_sSTARTOPT.nidITEM.wMainID );
	SetWin_Num_int ( this, IDC_EDIT_START_ITEM_SID, m_DummyQuest.pQUEST->m_sSTARTOPT.nidITEM.wSubID );
	SetWin_Num_int ( this, IDC_EDIT_START_SKILL_MID, m_DummyQuest.pQUEST->m_sSTARTOPT.nidSKILL.wMainID );
	SetWin_Num_int ( this, IDC_EDIT_START_SKILL_SID, m_DummyQuest.pQUEST->m_sSTARTOPT.nidSKILL.wSubID );
	SetWin_Num_int ( this, IDC_EDIT_START_QUEST_MAP_MID, m_DummyQuest.pQUEST->m_sSTARTOPT.nidMAP.wMainID );
	SetWin_Num_int ( this, IDC_EDIT_START_QUEST_MAP_SID, m_DummyQuest.pQUEST->m_sSTARTOPT.nidMAP.wSubID );
	SetWin_Num_int ( this, IDC_EDIT_START_QUEST_MAP_X, m_DummyQuest.pQUEST->m_sSTARTOPT.wPosX );
	SetWin_Num_int ( this, IDC_EDIT_START_QUEST_MAP_Y, m_DummyQuest.pQUEST->m_sSTARTOPT.wPosY );
	SetWin_Num_int ( this, IDC_EDIT_START_QUEST_MAP_SIZE, m_DummyQuest.pQUEST->m_sSTARTOPT.wSIZE );


	//reward
	SetWin_Num_LONGLONG ( this, IDC_EDIT_GIFT_EXP, m_DummyQuest.pQUEST->m_llGiftEXP );
	SetWin_Num_int ( this, IDC_EDIT_GIFT_MONEY, m_DummyQuest.pQUEST->m_dwGiftMONEY );
	SetWin_Num_int ( this, IDC_EDIT_GIFT_SKILLP, m_DummyQuest.pQUEST->m_dwGiftSKILLPOINT );
	SetWin_Num_int ( this, IDC_EDIT_GIFT_STATP, m_DummyQuest.pQUEST->m_dwGiftSTATSPOINT );
	SetWin_Num_int ( this, IDC_EDIT_GIFT_ELEMENT, m_DummyQuest.pQUEST->m_dwGiftELEMENT );
	SetWin_Num_int ( this, IDC_EDIT_GIFT_LIFEP, m_DummyQuest.pQUEST->m_dwGiftLIFEPOINT );
	SetWin_Num_int ( this, IDC_EDIT_GIFT_DISPK, m_DummyQuest.pQUEST->m_dwGiftDisPK );
	SetWin_Num_int ( this, IDC_EDIT_GIFT_QUEST, m_DummyQuest.pQUEST->m_dwGiftQUEST );

	//quest progress
	m_tempProgressEvent = m_DummyQuest.pQUEST->m_ProgressEvent;
	ProgressShow();

	//some checking 
	CheckItem();
	CheckSkill();
	CheckMap();

	CheckProgressMapStart();
	CheckProgressMapEnd();
	CheckProgressMapFail();

	UpdateRewardItem();
	UpdateRewardSkill();

	return TRUE;
}

void CPageQuestBasic::ProgressShow()
{
	SetWin_Check( this, IDC_CHECK_PROGRESS_USE, m_tempProgressEvent.bUseProgressEvent );

	SetWin_Check( this, IDC_CHECK_FAIL_TIMEOVER, m_tempProgressEvent.IsTIMEOVER() );
	SetWin_Check( this, IDC_CHECK_FAIL_DIE, m_tempProgressEvent.IsDIE() );
	SetWin_Check( this, IDC_CHECK_FAIL_LEAVE, m_tempProgressEvent.IsLEAVE());

	SetWin_Combo_Sel( this, IDC_COMBO_PROGRESS_START, m_tempProgressEvent.dwStartProgress );
	SetWin_Combo_Sel( this, IDC_COMBO_PROGRESS_END, m_tempProgressEvent.dwEndProgress );
	SetWin_Combo_Sel( this, IDC_COMBO_PROGRESS_FAIL,m_tempProgressEvent.dwFailProgress );

	SetWin_Num_int( this, IDC_EDIT_MAP_MID_START, m_tempProgressEvent.startMap.nidMAP.wMainID );
	SetWin_Num_int( this, IDC_EDIT_MAP_SID_START, m_tempProgressEvent.startMap.nidMAP.wSubID );
	SetWin_Num_int( this, IDC_EDIT_MAP_X_START, m_tempProgressEvent.startMap.wPosX );
	SetWin_Num_int( this, IDC_EDIT_MAP_Y_START, m_tempProgressEvent.startMap.wPosY );
	SetWin_Num_int( this, IDC_EDIT_MAP_GATE_START, m_tempProgressEvent.startMap.dwGateID );

	SetWin_Num_int( this, IDC_EDIT_MAP_MID_END, m_tempProgressEvent.endMap.nidMAP.wMainID );
	SetWin_Num_int( this, IDC_EDIT_MAP_SID_END, m_tempProgressEvent.endMap.nidMAP.wSubID );
	SetWin_Num_int( this, IDC_EDIT_MAP_X_END, m_tempProgressEvent.endMap.wPosX );
	SetWin_Num_int( this, IDC_EDIT_MAP_Y_END, m_tempProgressEvent.endMap.wPosY );
	SetWin_Num_int( this, IDC_EDIT_MAP_GATE_END, m_tempProgressEvent.endMap.dwGateID );

	SetWin_Num_int( this, IDC_EDIT_MAP_MID_FAIL, m_tempProgressEvent.failMap.nidMAP.wMainID );
	SetWin_Num_int( this, IDC_EDIT_MAP_SID_FAIL, m_tempProgressEvent.failMap.nidMAP.wSubID );
	SetWin_Num_int( this, IDC_EDIT_MAP_X_FAIL, m_tempProgressEvent.failMap.wPosX );
	SetWin_Num_int( this, IDC_EDIT_MAP_Y_FAIL, m_tempProgressEvent.failMap.wPosY );
	SetWin_Num_int( this, IDC_EDIT_MAP_GATE_FAIL, m_tempProgressEvent.failMap.dwGateID );
}

BOOL CPageQuestBasic::DataSave()
{
	if ( !m_bDlgInit )	return FALSE;
	if ( !m_DummyQuest.pQUEST )	return FALSE;

	//basic
	m_DummyQuest.pQUEST->m_dwEDITVER = GetWin_Num_int ( this, IDC_EDIT_EDITVERSION );	
	m_DummyQuest.pQUEST->m_sNID.dwID = (DWORD) GetWin_Num_int ( this, IDC_EDIT_QUEST_NID );
	m_DummyQuest.strFILE = GetWin_Text ( this, IDC_EDIT_QUEST_FILENAME );
	m_DummyQuest.pQUEST->m_strTITLE = GetWin_Text ( this, IDC_EDIT_TITLE ).GetString();
	m_DummyQuest.pQUEST->m_strCOMMENT = GetWin_Text ( this, IDC_EDIT_COMMENT ).GetString();
	m_DummyQuest.pQUEST->m_emQuestArea = static_cast<EMQUEST_AREA>( GetWin_Combo_Sel( this, IDC_COMBO_QUEST_AREA ) );

	GetWin_Check ( this, IDC_CHECK_REPEAT, m_DummyQuest.pQUEST->m_dwFlags, EMQF_REPEAT );
	GetWin_Check ( this, IDC_CHECK_AGAIN, m_DummyQuest.pQUEST->m_dwFlags, EMQF_AGAIN );
	GetWin_Check ( this, IDC_CHECK_GIVEUP, m_DummyQuest.pQUEST->m_dwFlags, EMQF_GIVEUP );
	GetWin_Check ( this, IDC_CHECK_PARTY, m_DummyQuest.pQUEST->m_dwFlags, EMQF_PARTY );
	GetWin_Check ( this, IDC_CHECK_PARTYQUEST, m_DummyQuest.pQUEST->m_dwFlags, EMQF_PARTYQUEST );
	GetWin_Check ( this, IDC_CHECK_ONLYONE_PROG, m_DummyQuest.pQUEST->m_dwFlags, EMQF_ONLYONE );
	
	m_DummyQuest.pQUEST->m_dwLimitTime = (DWORD ) GetWin_Num_int ( this, IDC_EDIT_LIMIT_TIME );
	m_DummyQuest.pQUEST->m_dwLimitPartyMemNum = (DWORD) GetWin_Num_int ( this, IDC_EDIT_LIMIT_PARTYNUM );
	m_DummyQuest.pQUEST->m_bNonDie = GetWin_Check ( this, IDC_CHECK_LIMIT_NONDIE ) != FALSE;

	m_DummyQuest.pQUEST->m_dwBeginMoney = (DWORD) GetWin_Num_int ( this, IDC_EDIT_BEGIN_MONEY );
	m_DummyQuest.pQUEST->m_dwBeginPartyMemNum = (DWORD) GetWin_Num_int ( this, IDC_EDIT_BEGIN_PARTYNUM );

	//autostart
	m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS = GLCC_NONE;
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_BRAWLER_M, m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS, GLCC_BRAWLER_M );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_SWORDSMAN_M, m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS, GLCC_SWORDSMAN_M );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_ARCHER_F, m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS, GLCC_ARCHER_W );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_SHAMAN_F, m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS, GLCC_SHAMAN_W );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_BRAWLER_F, m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS, GLCC_BRAWLER_W );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_SWORDSMAN_F, m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS, GLCC_SWORDSMAN_W );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_ARCHER_M, m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS, GLCC_ARCHER_M );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_SHAMAN_M, m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS, GLCC_SHAMAN_M );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_EXTREME_M, m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS, GLCC_EXTREME_M );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_EXTREME_F, m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS, GLCC_EXTREME_W );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_GUNNER_M, m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS, GLCC_GUNNER_M );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_GUNNER_F, m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS, GLCC_GUNNER_W );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_ASSASSIN_M, m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS, GLCC_ASSASSIN_M );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_ASSASSIN_F, m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS, GLCC_ASSASSIN_W );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_TRICKER_M, m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS, GLCC_TRICKER_M );
	GetWin_Check ( this, IDC_CHECK_REQ_CLASS_TRICKER_F, m_DummyQuest.pQUEST->m_sSTARTOPT.dwCLASS, GLCC_TRICKER_W );

	m_DummyQuest.pQUEST->m_sSTARTOPT.wSCHOOL = (WORD) GetWin_Combo_Sel ( this, IDC_COMBO_SCHOOL );

	m_DummyQuest.pQUEST->m_sSTARTOPT.wLEVEL = GetWin_Num_int ( this, IDC_EDIT_START_LEVEL );
	m_DummyQuest.pQUEST->m_sSTARTOPT.nidITEM.wMainID = GetWin_Num_int ( this, IDC_EDIT_START_ITEM_MID );
	m_DummyQuest.pQUEST->m_sSTARTOPT.nidITEM.wSubID = GetWin_Num_int ( this, IDC_EDIT_START_ITEM_SID );
	m_DummyQuest.pQUEST->m_sSTARTOPT.nidSKILL.wMainID = GetWin_Num_int ( this, IDC_EDIT_START_SKILL_MID );
	m_DummyQuest.pQUEST->m_sSTARTOPT.nidSKILL.wSubID = GetWin_Num_int ( this, IDC_EDIT_START_SKILL_SID );
	m_DummyQuest.pQUEST->m_sSTARTOPT.nidMAP.wMainID = GetWin_Num_int ( this, IDC_EDIT_START_QUEST_MAP_MID );
	m_DummyQuest.pQUEST->m_sSTARTOPT.nidMAP.wSubID = GetWin_Num_int ( this, IDC_EDIT_START_QUEST_MAP_SID );
	m_DummyQuest.pQUEST->m_sSTARTOPT.wPosX = GetWin_Num_int ( this, IDC_EDIT_START_QUEST_MAP_X );
	m_DummyQuest.pQUEST->m_sSTARTOPT.wPosY = GetWin_Num_int ( this, IDC_EDIT_START_QUEST_MAP_Y );
	m_DummyQuest.pQUEST->m_sSTARTOPT.wSIZE = GetWin_Num_int ( this, IDC_EDIT_START_QUEST_MAP_SIZE );
	
	//reward
	m_DummyQuest.pQUEST->m_llGiftEXP = GetWin_Num_LONGLONG ( this, IDC_EDIT_GIFT_EXP );
	m_DummyQuest.pQUEST->m_dwGiftMONEY = GetWin_Num_int ( this, IDC_EDIT_GIFT_MONEY );
	m_DummyQuest.pQUEST->m_dwGiftSKILLPOINT = GetWin_Num_int ( this, IDC_EDIT_GIFT_SKILLP );
	m_DummyQuest.pQUEST->m_dwGiftSTATSPOINT = GetWin_Num_int ( this, IDC_EDIT_GIFT_STATP );
	m_DummyQuest.pQUEST->m_dwGiftELEMENT = GetWin_Num_int ( this, IDC_EDIT_GIFT_ELEMENT );
	m_DummyQuest.pQUEST->m_dwGiftLIFEPOINT = GetWin_Num_int ( this, IDC_EDIT_GIFT_LIFEP );
	m_DummyQuest.pQUEST->m_dwGiftDisPK = GetWin_Num_int ( this, IDC_EDIT_GIFT_DISPK );
	m_DummyQuest.pQUEST->m_dwGiftQUEST = GetWin_Num_int ( this, IDC_EDIT_GIFT_QUEST );

	//quest progress
	m_DummyQuest.pQUEST->m_ProgressEvent = m_tempProgressEvent;
	
	return TRUE;
}

void CPageQuestBasic::ProgressSave()
{
	m_tempProgressEvent.bUseProgressEvent = GetWin_Check( this, IDC_CHECK_PROGRESS_USE ) != FALSE;

	GetWin_Check ( this, IDC_CHECK_FAIL_TIMEOVER, m_tempProgressEvent.dwfailType, EMFAIL_TIMEOVER );
	GetWin_Check ( this, IDC_CHECK_FAIL_DIE, m_tempProgressEvent.dwfailType, EMFAIL_DIE );
	GetWin_Check ( this, IDC_CHECK_FAIL_LEAVE, m_tempProgressEvent.dwfailType, EMFAIL_LEAVE );

	m_tempProgressEvent.dwStartProgress = GetWin_Combo_Sel( this, IDC_COMBO_PROGRESS_START );
	m_tempProgressEvent.dwEndProgress = GetWin_Combo_Sel( this, IDC_COMBO_PROGRESS_END );
	m_tempProgressEvent.dwFailProgress = GetWin_Combo_Sel( this, IDC_COMBO_PROGRESS_FAIL );

	m_tempProgressEvent.startMap.nidMAP.wMainID = GetWin_Num_int( this, IDC_EDIT_MAP_MID_START );
	m_tempProgressEvent.startMap.nidMAP.wSubID = GetWin_Num_int( this, IDC_EDIT_MAP_SID_START );
	m_tempProgressEvent.startMap.wPosX = GetWin_Num_int( this, IDC_EDIT_MAP_X_START );
	m_tempProgressEvent.startMap.wPosY = GetWin_Num_int( this, IDC_EDIT_MAP_Y_START );
	m_tempProgressEvent.startMap.dwGateID = GetWin_Num_int( this, IDC_EDIT_MAP_GATE_START );

	m_tempProgressEvent.endMap.nidMAP.wMainID = GetWin_Num_int( this, IDC_EDIT_MAP_MID_END );
	m_tempProgressEvent.endMap.nidMAP.wSubID = GetWin_Num_int( this, IDC_EDIT_MAP_SID_END );
	m_tempProgressEvent.endMap.wPosX = GetWin_Num_int( this, IDC_EDIT_MAP_X_END );
	m_tempProgressEvent.endMap.wPosY = GetWin_Num_int( this, IDC_EDIT_MAP_Y_END );
	m_tempProgressEvent.endMap.dwGateID = GetWin_Num_int( this, IDC_EDIT_MAP_GATE_END );

	m_tempProgressEvent.failMap.nidMAP.wMainID = GetWin_Num_int( this, IDC_EDIT_MAP_MID_FAIL );
	m_tempProgressEvent.failMap.nidMAP.wSubID = GetWin_Num_int( this, IDC_EDIT_MAP_SID_FAIL );
	m_tempProgressEvent.failMap.wPosX = GetWin_Num_int( this, IDC_EDIT_MAP_X_FAIL );
	m_tempProgressEvent.failMap.wPosY = GetWin_Num_int( this, IDC_EDIT_MAP_Y_FAIL );
	m_tempProgressEvent.failMap.dwGateID = GetWin_Num_int( this, IDC_EDIT_MAP_GATE_FAIL );
}

BOOL CPageQuestBasic::ValidateProgress()
{
	ProgressSave();

	if( m_tempProgressEvent.bUseProgressEvent )
	{
		if( m_tempProgressEvent.dwEndProgress != EMQP_NON_EVENT )
		{
			if( m_tempProgressEvent.dwfailType == 0 )
			{
				MessageBox( "Please Select Proper Fail Type for Progress End" );
				return FALSE;
			}
		}

		if( m_tempProgressEvent.dwStartProgress == EMQP_SELECT_MOVE )
		{
			if( m_tempProgressEvent.startMap.nidMAP.wMainID == USHRT_MAX || m_tempProgressEvent.startMap.nidMAP.wSubID  == USHRT_MAX ||
				( m_tempProgressEvent.startMap.wPosX == USHRT_MAX || m_tempProgressEvent.startMap.wPosY == USHRT_MAX ) && m_tempProgressEvent.startMap.dwGateID == 0 )
			{
				MessageBox( "Progress Start Map MID, Map SID, Map Position Or Map Gate Invalid! \n Please ReCheck" );
				return FALSE;
			}

			if( m_tempProgressEvent.startMap.wPosX != USHRT_MAX && m_tempProgressEvent.startMap.wPosY != USHRT_MAX && m_tempProgressEvent.startMap.dwGateID != 0 )
			{
				MessageBox( "Progress Start Map Position and Gate ID modified ! \n Please modify only Map Position or Gate , \n Dont Modify Both! \n When Map Position is Not (65535) you cannot use Gate ID other than 0" );
				return FALSE;
			}
		}

		if( m_tempProgressEvent.dwEndProgress == EMQP_SELECT_MOVE )
		{
			if( m_tempProgressEvent.endMap.nidMAP.wMainID == USHRT_MAX ||  m_tempProgressEvent.endMap.nidMAP.wSubID  == USHRT_MAX ||
				( m_tempProgressEvent.endMap.wPosX == USHRT_MAX || m_tempProgressEvent.endMap.wPosY == USHRT_MAX ) && m_tempProgressEvent.endMap.dwGateID == 0 )
			{
				MessageBox( "Progress End Map MID, Map SID, Map Position Or Map Gate Invalid! \n Please ReCheck" );
				return FALSE;
			}

			if( m_tempProgressEvent.endMap.wPosX != USHRT_MAX && m_tempProgressEvent.endMap.wPosY != USHRT_MAX && m_tempProgressEvent.endMap.dwGateID != 0 )
			{
				MessageBox( "Progress End Map Position and Gate ID modified ! \n Please modify only Map Position or Gate , \n Dont Modify Both! \n When Map Position is Not (65535) you cannot use Gate ID other than 0" );
				return FALSE;
			}
		}

		if( m_tempProgressEvent.dwFailProgress == EMQP_SELECT_MOVE )
		{
			if( m_tempProgressEvent.failMap.nidMAP.wMainID == USHRT_MAX || m_tempProgressEvent.failMap.nidMAP.wSubID  == USHRT_MAX ||
				(m_tempProgressEvent.failMap.wPosX == USHRT_MAX || m_tempProgressEvent.failMap.wPosY == USHRT_MAX ) && m_tempProgressEvent.failMap.dwGateID == 0 )
			{
				MessageBox( "Progress Fail Map MID, Map SID, Map Position Or Map Gate Invalid! \n Please ReCheck" );
				return FALSE;
			}

			if( m_tempProgressEvent.failMap.wPosX != USHRT_MAX && m_tempProgressEvent.failMap.wPosY != USHRT_MAX && m_tempProgressEvent.failMap.dwGateID != 0 )
			{
				MessageBox( "Progress Fail Map Position and Gate ID modified ! \n Please modify only Map Position or Gate , \n Dont Modify Both! \n When Map Position is Not (65535) you cannot use Gate ID other than 0" );
				return FALSE;
			}
		}
	}

	return TRUE;
}