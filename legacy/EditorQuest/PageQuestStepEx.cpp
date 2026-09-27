#include "stdafx.h"
#include "EditorQuest.h"
#include "EditorQuestDlg.h"
#include "PageQuestStep.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"

void CPageQuestStep::StepShowControls ( BOOL bSHOW )
{
	if ( !m_bDlgInit )	return;

	DWORD dwITEMS[] =
	{
		IDC_STATIC_TITLE
		, IDC_EDIT_STEP_TITLE
		, IDC_STATIC_DESC
		, IDC_EDIT_STEP_COMMENT

		, IDC_STATIC_TALKNPC
		, IDC_EDIT_TALKNPC_COMMENT
		, IDC_BUTTON_TALKNPC
		, IDC_EDIT_TALKNPC_MID
		, IDC_EDIT_TALKNPC_SID
		, IDC_EDIT_TALKNPC_NAME
		, IDC_BUTTON_TALKNPC_DEL

		, IDC_STATIC_DEFENDNPC
		, IDC_EDIT_GUARDNPC_COMMENT
		, IDC_BUTTON_GUARDNPC
		, IDC_EDIT_GUARDNPC_MID
		, IDC_EDIT_GUARDNPC_SID
		, IDC_BUTTON_GUARDNPC_DEL
		, IDC_EDIT_GUARDNPC_NAME

		, IDC_STATIC_REACHLEVEL
		, IDC_STATIC_REACHLEVEL2
		, IDC_EDIT_LEVEL

		, IDC_STATIC_KILLMOB
		, IDC_EDIT_DIEMOB_COMMENT
		, IDC_BUTTON_DIEMOB
		, IDC_EDIT_DIEMOB_MID
		, IDC_EDIT_DIEMOB_SID
		, IDC_BUTTON_DIEMOB_DEL
		, IDC_EDIT_DIEMOB_NAME
		, IDC_STATIC_KILLMOB2
		, IDC_EDIT_DIEMOB_NUM

		, IDC_STATIC_MOVEMAP
		, IDC_STEP_MAP_EDIT_BUTTON
		, IDC_STEP_MAP_MID_EDIT
		, IDC_STEP_MAP_SID_EDIT
		, IDC_STEP_MAP_EDIT_DEL
		, IDC_STEP_MAP_NAME
		, IDC_STATIC_MOVEMAP2
		, IDC_STEP_MAP_XPOS_EDIT
		, IDC_STEP_MAP_YPOS_EDIT
		, IDC_STATIC_MOVEMAP3
		, IDC_STEP_MAP_GATE_EDIT

		, IDC_STATIC_REACHMAP
		, IDC_EDIT_REACH_MAP_COMMENT
		, IDC_BUTTON_REACH_MAP
		, IDC_EDIT_REACH_MAP_MID
		, IDC_EDIT_REACH_MAP_SID
		, IDC_BUTTON_REACH_MAP_DEL
		, IDC_EDIT_REACH_MAP_NAME
		, IDC_STATIC_REACHMAP2
		, IDC_EDIT_REACH_PX
		, IDC_EDIT_REACH_PY
		, IDC_STATIC_REACHMAP3
		, IDC_EDIT_REACH_RADIUS

		, IDC_STATIC_DEFENDMAP
		, IDC_EDIT_GUARD_MAP_COMMENT
		, IDC_BUTTON_GUARD_MAP
		, IDC_EDIT_GUARD_MAP_MID
		, IDC_EDIT_GUARD_MAP_SID
		, IDC_BUTTON_GUARD_MAP_DEL
		, IDC_EDIT_GUARD_MAP_NAME
		, IDC_STATIC_DEFENDMAP1
		, IDC_EDIT_GUARD_PX
		, IDC_EDIT_GUARD_PY
		, IDC_STATIC_DEFENDMAP2
		, IDC_EDIT_GUARD_RADIUS
		, IDC_STATIC_DEFENDMAP3
		, IDC_EDIT_GUARD_TIME

		, IDC_STATIC_ITEMGAIN
		, IDC_LIST_ITEM_GAIN
		, IDC_BUTTON_ITEM_ADD_GAIN
		, IDC_BUTTON_ITEM_DEL_GAIN
		, IDC_BUTTON_ITEM_EDIT_GAIN
		, IDC_STATIC_ITEMGAIN2
		, IDC_EDIT_INSERT_MID_GAIN
		, IDC_EDIT_INSERT_SID_GAIN
		, IDC_BUTTON_ITEM_INSERT_GAIN

		, IDC_STATIC_ITEMDEL
		, IDC_LIST_ITEM_REMOVE
		, IDC_BUTTON_ITEM_ADD_REMOVE
		, IDC_BUTTON_ITEM_DEL_REMOVE
		, IDC_STATIC_ITEMDEL2
		, IDC_EDIT_INSERT_MID_DEL
		, IDC_EDIT_INSERT_SID_DEL
		, IDC_BUTTON_ITEM_INSERT_REMOVE

		, IDC_STATIC_ITEMGEN
		, IDC_EDIT_REQ_QITEM_COMMENT
		, IDC_LIST_ITEM_GEN
		, IDC_BUTTON_ITEM_ADD_GEN
		, IDC_BUTTON_ITEM_DEL_GEN
		, IDC_BUTTON_ITEM_EDIT_GEN
		, IDC_STATIC_ITEMGEN2
		, IDC_EDIT_INSERT_MID_GEN
		, IDC_EDIT_INSERT_SID_GEN
		, IDC_BUTTON_ITEM_INSERT_GEN

		, IDC_BUTTON_STEP_OK
		, IDC_BUTTON_STEP_CANCEL
	};

	DWORD dwINV_ITEMS[] =
	{
		IDC_BUTTON_BACK,
		IDC_BUTTON_CANCEL,
		IDC_BUTTON_SAVE,
	};

	DWORD dwITEMS_DISABLE[] =
	{
		IDC_LIST_STEP,
		IDC_BUTTON_STEP_ADD,
		IDC_BUTTON_STEP_EDIT,
		IDC_BUTTON_STEP_DEL,
		IDC_BUTTON_STEP_MOVEUP,
		IDC_BUTTON_STEP_MOVEDOWN,
	};

	int nSize = (int) sizeof(dwITEMS)/sizeof(DWORD);
	for ( int i=0; i<nSize; ++i )
	{
		SetWin_ShowWindow( this, dwITEMS[i], bSHOW );
	}

	int nSizeINV = (int) sizeof(dwINV_ITEMS)/sizeof(DWORD);
	for ( int i=0; i<nSizeINV; ++i )
	{
		SetWin_ShowWindow( this, dwINV_ITEMS[i], !bSHOW );
	}

	int nSizeDisable = (int) sizeof(dwITEMS_DISABLE)/sizeof(DWORD);
	for ( int i=0; i<nSizeDisable; ++i )
	{
		SetWin_Enable( this, dwITEMS_DISABLE[i], !bSHOW );
	}
}

BOOL CPageQuestStep::DataDefault()
{
	return TRUE;
}

BOOL CPageQuestStep::DataShow()
{
	if ( !m_bDlgInit )	return FALSE;

	m_ListStep.ResetContent();

	if ( !m_DummyQuest.pQUEST )	return FALSE;

	m_ListStep.SetRedraw( FALSE );

	GLQUEST::VECQSTEP_ITER iter = m_DummyQuest.pQUEST->m_vecProgStep.begin();
	GLQUEST::VECQSTEP_ITER iter_end = m_DummyQuest.pQUEST->m_vecProgStep.end();
	for ( int i=0; iter!=iter_end; ++iter, ++i )
	{
		const GLQUEST_STEP &sSTEP = (*iter);

		int nIndex = m_ListStep.AddString ( sSTEP.GetTITLE () );
		m_ListStep.SetItemData ( nIndex, i );
	}

	m_ListStep.SetRedraw( TRUE );

	StepShow();

	return TRUE;
}

BOOL CPageQuestStep::DataSave()
{
	if ( !m_bDlgInit )	return FALSE;
	StepSave ();

	return TRUE;
}

void CPageQuestStep::StepShow()
{
	if ( !m_bDlgInit )	return;

	if ( !m_DummyQuest.pQUEST )	return;

	int nIndex = m_ListStep.GetCurSel();
	if ( nIndex==LB_ERR )
	{
		StepShowControls( FALSE );
		return;
	}

	DWORD dwStep = (DWORD) m_ListStep.GetItemData ( nIndex );
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= dwStep )
	{
		StepShowControls( FALSE );
		return;
	}

	StepShowControls( TRUE );
	GLQUEST_STEP &sSTEP = m_DummyQuest.pQUEST->m_vecProgStep[dwStep];

	m_dwCurStep = dwStep;

	//basic
	SetWin_Text ( this, IDC_EDIT_STEP_TITLE, sSTEP.GetTITLE() );
	SetWin_Text ( this, IDC_EDIT_STEP_COMMENT, sSTEP.GetCOMMENT() );

	//talk to npc
	SetWin_Text ( this, IDC_EDIT_TALKNPC_COMMENT, sSTEP.GetOBJ_NPCTALK() );
	SetWin_Num_int( this, IDC_EDIT_TALKNPC_MID, SNATIVEID(sSTEP.m_dwNID_NPCTALK).wMainID );
	SetWin_Num_int( this, IDC_EDIT_TALKNPC_SID, SNATIVEID(sSTEP.m_dwNID_NPCTALK).wSubID );
	CheckCrowNpctalk();

	//defend npc
	SetWin_Text ( this, IDC_EDIT_GUARDNPC_COMMENT, sSTEP.GetOBJ_NPCGUARD() );
	SetWin_Num_int( this, IDC_EDIT_GUARDNPC_MID, SNATIVEID(sSTEP.m_dwNID_NPCGUARD).wMainID );
	SetWin_Num_int( this, IDC_EDIT_GUARDNPC_SID, SNATIVEID(sSTEP.m_dwNID_NPCGUARD).wSubID );
	CheckCrowGuardNPC();

	//kill mob
	SetWin_Text ( this, IDC_EDIT_DIEMOB_COMMENT, sSTEP.GetOBJ_MOBKILL() );
	SetWin_Num_int( this, IDC_EDIT_DIEMOB_MID, SNATIVEID(sSTEP.m_dwNID_MOBKILL).wMainID );
	SetWin_Num_int( this, IDC_EDIT_DIEMOB_SID, SNATIVEID(sSTEP.m_dwNID_MOBKILL).wSubID );
	SetWin_Num_int ( this, IDC_EDIT_DIEMOB_NUM, sSTEP.m_dwNUM_MOBKILL );
	CheckCrowKillMob();

	//reach map
	SetWin_Text ( this, IDC_EDIT_REACH_MAP_COMMENT, sSTEP.GetOBJ_REACH_ZONE() );
	SetWin_Num_int( this, IDC_EDIT_REACH_MAP_MID, SNATIVEID(sSTEP.m_sMAPID_REACH).wMainID );
	SetWin_Num_int( this, IDC_EDIT_REACH_MAP_SID, SNATIVEID(sSTEP.m_sMAPID_REACH).wSubID );
	SetWin_Num_int( this, IDC_EDIT_REACH_PX, sSTEP.m_wPOSX_REACH );
	SetWin_Num_int( this, IDC_EDIT_REACH_PY, sSTEP.m_wPOSY_REACH );
	SetWin_Num_int( this, IDC_EDIT_REACH_RADIUS, sSTEP.m_wRADIUS_REACH );
	CheckMapReachMap();

	//defend map
	SetWin_Text ( this, IDC_EDIT_GUARD_MAP_COMMENT, sSTEP.GetOBJ_DEFENSE_ZONE() );
	SetWin_Num_int( this, IDC_EDIT_GUARD_MAP_MID, SNATIVEID(sSTEP.m_sMAPID_DEFENSE).wMainID );
	SetWin_Num_int( this, IDC_EDIT_GUARD_MAP_SID, SNATIVEID(sSTEP.m_sMAPID_DEFENSE).wSubID );
	SetWin_Num_int( this, IDC_EDIT_GUARD_PX, sSTEP.m_wPOSX_DEFENSE );
	SetWin_Num_int( this, IDC_EDIT_GUARD_PY, sSTEP.m_wPOSY_DEFENSE );
	SetWin_Num_int( this, IDC_EDIT_GUARD_RADIUS, sSTEP.m_wRADIUS_DEFENSE );
	SetWin_Num_float( this, IDC_EDIT_GUARD_TIME, sSTEP.m_fDEFENSE_TIME );
	CheckMapDefendMap();

	//move map
	SetWin_Num_int ( this, IDC_STEP_MAP_MID_EDIT, sSTEP.m_stepMoveMap.nidMAP.wMainID );
	SetWin_Num_int ( this, IDC_STEP_MAP_SID_EDIT, sSTEP.m_stepMoveMap.nidMAP.wSubID );
	SetWin_Num_int ( this, IDC_STEP_MAP_XPOS_EDIT, sSTEP.m_stepMoveMap.wPosX );
	SetWin_Num_int ( this, IDC_STEP_MAP_YPOS_EDIT, sSTEP.m_stepMoveMap.wPosY );
	SetWin_Num_int ( this, IDC_STEP_MAP_GATE_EDIT, sSTEP.m_stepMoveMap.dwGateID );
	CheckMapMoveMap();

	//reach level
	SetWin_Num_int ( this, IDC_EDIT_LEVEL, sSTEP.m_wLevel );

	//item gain
	ShowItemGain();

	//item del
	ShowItemRemove();

	//item generate
	ShowItemGen();
	SetWin_Text ( this, IDC_EDIT_REQ_QITEM_COMMENT, sSTEP.GetOBJ_MOBGEN_QITEM() );
}

void CPageQuestStep::StepSave()
{
	if ( !m_bDlgInit )	return;

	if ( m_dwCurStep==UINT_MAX )	return;
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= m_dwCurStep )	return;

	GLQUEST_STEP &sSTEP = m_DummyQuest.pQUEST->m_vecProgStep[m_dwCurStep];

	//basic
	sSTEP.m_strTITLE = GetWin_Text ( this, IDC_EDIT_STEP_TITLE ).GetString();
	sSTEP.m_strCOMMENT = GetWin_Text ( this, IDC_EDIT_STEP_COMMENT ).GetString();

	//talk to npc
	SNATIVEID sIDTALKNPC;
	sIDTALKNPC.wMainID = GetWin_Num_int( this, IDC_EDIT_TALKNPC_MID );
	sIDTALKNPC.wSubID = GetWin_Num_int( this, IDC_EDIT_TALKNPC_SID );
	sSTEP.m_dwNID_NPCTALK = sIDTALKNPC.dwID;
	sSTEP.m_strOBJ_NPCTALK = GetWin_Text ( this, IDC_EDIT_TALKNPC_COMMENT );
	
	//defend npc
	SNATIVEID sIDGUARDNPC;
	sIDGUARDNPC.wMainID = GetWin_Num_int( this, IDC_EDIT_GUARDNPC_MID );
	sIDGUARDNPC.wSubID = GetWin_Num_int( this, IDC_EDIT_GUARDNPC_SID );
	sSTEP.m_dwNID_NPCGUARD = sIDGUARDNPC.dwID;
	sSTEP.m_strOBJ_NPCGUARD = GetWin_Text ( this, IDC_EDIT_GUARDNPC_COMMENT );
	
	//kill mob
	SNATIVEID sIDKILLMOB;
	sIDKILLMOB.wMainID = GetWin_Num_int( this, IDC_EDIT_DIEMOB_MID );
	sIDKILLMOB.wSubID = GetWin_Num_int( this, IDC_EDIT_DIEMOB_SID );
	sSTEP.m_dwNID_MOBKILL = sIDKILLMOB.dwID;
	sSTEP.m_strOBJ_MOBKILL = GetWin_Text ( this, IDC_EDIT_DIEMOB_COMMENT );
	sSTEP.m_dwNUM_MOBKILL = GetWin_Num_int ( this, IDC_EDIT_DIEMOB_NUM );

	//reach map
	SNATIVEID sIDREACHMAP;
	sIDREACHMAP.wMainID = GetWin_Num_int( this, IDC_EDIT_REACH_MAP_MID );
	sIDREACHMAP.wSubID = GetWin_Num_int( this, IDC_EDIT_REACH_MAP_SID );
	sSTEP.m_sMAPID_REACH = sIDREACHMAP.dwID;
	sSTEP.m_strOBJ_REACH_ZONE = GetWin_Text ( this, IDC_EDIT_REACH_MAP_COMMENT );
	sSTEP.m_wPOSX_REACH = GetWin_Num_int ( this, IDC_EDIT_REACH_PX );
	sSTEP.m_wPOSY_REACH = GetWin_Num_int ( this, IDC_EDIT_REACH_PY );
	sSTEP.m_wRADIUS_REACH = GetWin_Num_int ( this, IDC_EDIT_REACH_RADIUS );

	//defend map
	SNATIVEID sIDGUARDMAP;
	sIDGUARDMAP.wMainID = GetWin_Num_int( this, IDC_EDIT_GUARD_MAP_MID );
	sIDGUARDMAP.wSubID = GetWin_Num_int( this, IDC_EDIT_GUARD_MAP_SID );
	sSTEP.m_sMAPID_DEFENSE = sIDGUARDMAP.dwID;
	sSTEP.m_strOBJ_DEFENSE_ZONE = GetWin_Text ( this, IDC_EDIT_GUARD_MAP_COMMENT );
	sSTEP.m_wPOSX_DEFENSE = GetWin_Num_int ( this, IDC_EDIT_GUARD_PX );
	sSTEP.m_wPOSY_DEFENSE = GetWin_Num_int ( this, IDC_EDIT_GUARD_PY );
	sSTEP.m_wRADIUS_DEFENSE = GetWin_Num_int ( this, IDC_EDIT_GUARD_RADIUS );
	sSTEP.m_fDEFENSE_TIME = GetWin_Num_float ( this, IDC_EDIT_GUARD_TIME );

	//move map
	//sSTEP.m_stepMoveMap.nidMAP.wMainID = GetWin_Num_int ( this, IDC_STEP_MAP_MID_EDIT );
	//sSTEP.m_stepMoveMap.nidMAP.wSubID = GetWin_Num_int ( this, IDC_STEP_MAP_SID_EDIT );
	//sSTEP.m_stepMoveMap.wPosX = GetWin_Num_int ( this, IDC_STEP_MAP_XPOS_EDIT );
	//sSTEP.m_stepMoveMap.wPosY = GetWin_Num_int ( this, IDC_STEP_MAP_YPOS_EDIT );
	//sSTEP.m_stepMoveMap.dwGateID = GetWin_Num_int ( this, IDC_STEP_MAP_GATE_EDIT );

	//reach level
	sSTEP.m_wLevel = (WORD) GetWin_Num_int ( this, IDC_EDIT_LEVEL );

	//item generate
	sSTEP.m_strOBJ_MOBGEN_QITEM = GetWin_Text ( this, IDC_EDIT_REQ_QITEM_COMMENT );
}