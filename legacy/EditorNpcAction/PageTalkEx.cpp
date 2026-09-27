#include "stdafx.h"
#include "EditorNpcAction.h"
#include "PageTalk.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "NpcTalk.h"
#include "GLQuest.h"
#include "GLQuestMan.h"
#include "GLItemMan.h"

#include "NpcTalkCondition.h"
#include "NpcDialogueSet.h"
#include "NpcDialogue.h"
#include "NpcDialogueCase.h"
#include "NpcTalkControl.h"

#define MAX_RANDOM_TIME		999

BOOL CPageTalk::DataDefault()
{
	m_ComboType.ResetContent ();

	for ( int i = 0; i < SNpcTalk::EM_TALK_SIZE; i++ )
	{
		int nIndex = m_ComboType.InsertString ( i, SNpcTalk::strACTIONTYPE[i].c_str() );
		m_ComboType.SetItemData ( nIndex, i );
	}

	m_listRandom.SetExtendedStyle ( m_listRandom.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	RECT	rect;
	m_listRandom.GetClientRect ( &rect );
	LONG	Width = rect.right - rect.left;

	LV_COLUMN	lvcolumn;
	const int	ColumnCount = 2;	
	char*		ColumnName[ColumnCount] = { "ID", "Desc" };
	int			ColumnWidthPercent[ColumnCount] = { 20, 80 };

	for ( int i = 0; i < ColumnCount; i++ )
	{		
		lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvcolumn.fmt = LVCFMT_LEFT;
		lvcolumn.pszText = ColumnName[i];
		lvcolumn.iSubItem = i;
		lvcolumn.cx = ( Width * ColumnWidthPercent[i] ) / 100;
		m_listRandom.InsertColumn (i, &lvcolumn);		
	}

	return TRUE;  
}

BOOL CPageTalk::DataUpdate()
{
	if ( !m_bModify )
	{
		DWORD dwBackupAction   = SNpcTalk::EM_DO_NOTHING;		
		DWORD dwBackupActionID = UINT_MAX;
		if( m_Talk.m_nACTION != SNpcTalk::EM_DO_NOTHING && m_Talk.m_dwACTION_NO != UINT_MAX )
		{
			dwBackupAction   = m_Talk.m_nACTION;		
			dwBackupActionID = m_Talk.m_dwACTION_NO;
		}		

		m_Talk.Init ();
		m_Talk.ResetCondition();
		m_Talk.m_dwNID = m_pTalkControl->MakeTalkNID ();
		m_Talk.m_nACTION	 = dwBackupAction;
		m_Talk.m_dwACTION_NO = dwBackupActionID;

		SetWin_Num_int( this, IDC_EDIT_TALKID, m_Talk.m_dwNID );
		SetWin_Text( this, IDC_EDIT_TALK, m_Talk.GetTalk() );
		SetWin_Text( this, IDC_EDIT_BUS_LIST, m_Talk.m_strBusFile.c_str() );
		m_ComboType.SetCurSel ( m_Talk.m_nACTION );

		dwBackupActionID = m_Talk.m_dwACTION_NO;
		InitComboType();
		m_Talk.m_dwACTION_NO = dwBackupActionID;

		for ( int i=0; i<m_ComboData.GetCount (); i++ )
		{
			DWORD dwNO = (DWORD) m_ComboData.GetItemData ( i );
			if ( dwNO==m_Talk.m_dwACTION_NO )
			{
				m_ComboData.SetCurSel ( i );
				break;
			}
		}

		ShowCondition();
		SetWin_Num_int( this, IDC_EDIT_RANDOM_TIME, m_Talk.m_dwRandomTime );

		SetWndView ( m_Talk.m_nACTION, m_Talk.m_dwACTION_NO );

	}
	else
	{
		SetWin_Num_int( this, IDC_EDIT_TALKID, m_Talk.m_dwNID );
		SetWin_Text( this, IDC_EDIT_TALK, m_Talk.GetTalk() );
		SetWin_Text( this, IDC_EDIT_BUS_LIST, m_Talk.m_strBusFile.c_str() );
		m_ComboType.SetCurSel ( m_Talk.m_nACTION );

		DWORD dwBackupActionID = m_Talk.m_dwACTION_NO;
		InitComboType();
		m_Talk.m_dwACTION_NO = dwBackupActionID;

		for ( int i=0; i<m_ComboData.GetCount(); i++ )
		{
			DWORD dwNO = (DWORD) m_ComboData.GetItemData ( i );
			if ( dwNO==m_Talk.m_dwACTION_NO )
			{
				m_ComboData.SetCurSel ( i );
				break;
			}
		}
		
		ShowCondition();
		SetWin_Num_int( this, IDC_EDIT_RANDOM_TIME, m_Talk.m_dwRandomTime );

		SetWndView ( m_Talk.m_nACTION, m_Talk.m_dwACTION_NO );
	}

	return TRUE;  
}

BOOL CPageTalk::DataSave()
{
	assert ( m_pTalkControl && "Invalid m_pTalkControl Pointer." );

	if ( !m_pTalkControl )	return FALSE;

	CString strTemp;

	m_Talk.m_dwNID = GetWin_Num_int( this, IDC_EDIT_TALKID );
	m_Talk.m_strTalk = GetWin_Text( this, IDC_EDIT_TALK );
	m_Talk.m_strBusFile = GetWin_Text( this, IDC_EDIT_BUS_LIST );
	m_Talk.m_dwRandomTime = GetWin_Num_int( this, IDC_EDIT_RANDOM_TIME );

	if ( m_Talk.m_dwRandomTime > MAX_RANDOM_TIME ) m_Talk.m_dwRandomTime = MAX_RANDOM_TIME;

	if ( !m_Talk.m_strTalk.size() )
	{
		MessageBox ( "Description cannot be empty", "Error", MB_OK | MB_ICONASTERISK );
		return FALSE;
	}

	if ( !m_bModify )
	{
		SNpcTalk* pNewTalk = new SNpcTalk;
		*pNewTalk = m_Talk;

		if ( !m_pTalkControl->AddTalk ( pNewTalk ) )
		{
			SAFE_DELETE ( pNewTalk );
			MessageBox ( "ID Already Exist!." );
			return FALSE;
		}

		SNpcTalk* pTalk = m_pTalkControl->GetTalk ( m_Talk.m_dwNID );
		if ( pTalk )
			SetModifyData( TRUE, pTalk );
	}
	else
	{
		SNpcTalk* pTalk = m_pTalkControl->GetTalk ( m_Talk.m_dwNID );
		*pTalk = m_Talk;
	}

	return TRUE;  
}

void CPageTalk::InitComboType()
{
	m_ComboData.ResetContent ();

	int nIndex = m_ComboType.GetCurSel ();
	m_Talk.m_nACTION = (int)m_ComboType.GetItemData ( nIndex );

	switch ( nIndex )
	{
	case SNpcTalk::EM_DO_NOTHING:
		{
		}break;

	case SNpcTalk::EM_PAGE_MOVE:
	case SNpcTalk::EM_QUEST_START:
	case SNpcTalk::EM_QUEST_STEP:
		{
			CNpcDialogueSet::DIALOGMAP* pMap = m_pNpcDialogueSet->GetDialogueMap ();
			if ( pMap->size() )
			{
				CNpcDialogueSet::DIALOGMAP_IT iter = pMap->begin();
				CNpcDialogueSet::DIALOGMAP_IT iter_end = pMap->end();	
				for ( ; iter!=iter_end; ++iter )
				{
					CNpcDialogue* pDialogue = (*iter).second;
					DWORD dwIndex = pDialogue->FindNonCondition();
					if ( dwIndex==UINT_MAX )	dwIndex = 0;

					CNpcDialogueCase* pDlgCase = pDialogue->GetDlgCase(dwIndex);
					if ( !pDlgCase )		continue;

					CString strTemp;
					strTemp.Format ( "[%03d] %s", pDialogue->GetDlgNID(), pDlgCase->GetBasicTalk ().c_str() );
					int nItem = m_ComboData.AddString ( strTemp );
					m_ComboData.SetItemData ( nItem, pDialogue->GetDlgNID() );
					m_ComboData.SetCurSel ( 0 );
				}
			}
		}break;

	case SNpcTalk::EM_BASIC:
		{
			for ( int i = 0; i < SNpcTalk::EM_BASIC_SIZE; i++ )
			{
				int nIndex = m_ComboData.InsertString ( i, SNpcTalk::szBASICDESC[i].c_str() );
				m_ComboData.SetItemData ( nIndex, i );
			}

			m_ComboData.SetCurSel ( 0 );
			
		}break;
	};

	InitComboData();	
	SetWndView ( m_Talk.m_nACTION, m_Talk.m_dwACTION_NO );
}

void CPageTalk::InitComboData()
{
	int nIndex = m_ComboData.GetCurSel ();
	m_Talk.m_dwACTION_NO = (DWORD)m_ComboData.GetItemData ( nIndex );
	SetWndView ( m_Talk.m_nACTION, m_Talk.m_dwACTION_NO );
}

void CPageTalk::ShowCondition()
{
	std::string strCondition;
	SNpcTalkCondition *pCondition = m_Talk.GetCondition();
	if ( pCondition )	
		pCondition->GetConditionText( strCondition );
	SetWin_Text( this, IDC_EDIT_CONDITION, strCondition.c_str() );
}


void CPageTalk::ShowQuestStart()
{
	DWORD dwIDQUEST[MAX_QUEST_START] = 
	{ 
		IDC_EDIT_QUEST_0, 
		IDC_EDIT_QUEST_1, 
		IDC_EDIT_QUEST_2, 
		IDC_EDIT_QUEST_3, 
		IDC_EDIT_QUEST_4 
	};

	for ( int i=0; i<MAX_QUEST_START; ++i )
	{	
		CString strQUEST;
		GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( m_Talk.m_dwQuestStartID[i] );
		if ( pQUEST )
			strQUEST.Format ( "[%d] %s", pQUEST->m_sNID.dwID, pQUEST->GetTITLE() );

		SetWin_Text( this, dwIDQUEST[i], strQUEST.GetString() );
	}

	DWORD dwIDINSERT[MAX_QUEST_START] = 
	{ 
		IDC_EDIT_QUEST_INSERT_0, 
		IDC_EDIT_QUEST_INSERT_1, 
		IDC_EDIT_QUEST_INSERT_2, 
		IDC_EDIT_QUEST_INSERT_3, 
		IDC_EDIT_QUEST_INSERT_4 
	};

	for ( int i=0; i<MAX_QUEST_START; ++i )
	{
		SetWin_Num_int( this, dwIDINSERT[i], UINT_MAX );
	}
}

void CPageTalk::ShowQuestProgress()
{
	m_listQuestStep.ResetContent();
	m_listQuestStep.SetRedraw( FALSE );

	SetWin_Text( this, IDC_EDIT_QUEST_PROG, "" );
	SetWin_Text( this, IDC_EDIT_QUEST_STEP, "" );

	CString strQUEST;
	GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( m_Talk.m_dwACTION_PARAM1 );
	if ( pQUEST )
	{
		strQUEST.Format ( "[%d] %s", pQUEST->m_sNID.dwID, pQUEST->GetTITLE() );

		SetWin_Text( this, IDC_EDIT_QUEST_PROG, strQUEST.GetString() );

		GLQUEST::VECQSTEP& sSTEP = pQUEST->GetSTEP();
		GLQUEST::VECQSTEP_ITER iter = sSTEP.begin();
		GLQUEST::VECQSTEP_ITER iter_end = sSTEP.end();
		for ( int i=0; iter!=iter_end; ++iter, ++i )
		{
			GLQUEST_STEP &sSTEP = (*iter);
			CString strTEMP;
			strTEMP.Format( "[%d][%s]", i, sSTEP.m_strTITLE.c_str() );
			int nIndex = m_listQuestStep.AddString ( strTEMP.GetString() );
			m_listQuestStep.SetItemData ( nIndex, nIndex );

			if ( i == m_Talk.m_dwACTION_PARAM2 )
			{
				SetWin_Text ( this, IDC_EDIT_QUEST_STEP, strTEMP.GetString() );
			}
		}
	}	

	m_listQuestStep.SetRedraw( TRUE );

	SetWin_Num_int( this, IDC_EDIT_QUEST_INSERT_PROG, UINT_MAX );
}

void CPageTalk::ShowItem()
{
	CString strITEM;
	SITEM* pITEM = GLItemMan::GetInstance().GetItem ( SNATIVEID(m_Talk.m_dwACTION_PARAM2) );
	if ( pITEM )
			strITEM.Format( "[%d/%d] %s", pITEM->sBasicOp.sNativeID.wMainID, pITEM->sBasicOp.sNativeID.wSubID, pITEM->GetName() );

	SetWin_Text ( this, IDC_EDIT_ITEM, strITEM );

	DWORD dwNeedItemID[MAX_NEEDITEM_COUNT] = 
	{
		IDC_EDIT_ITEM_REQ_0,
		IDC_EDIT_ITEM_REQ_1,
		IDC_EDIT_ITEM_REQ_2,
		IDC_EDIT_ITEM_REQ_3,
		IDC_EDIT_ITEM_REQ_4,
	};

	DWORD dwResultItemID[MAX_NEEDITEM_COUNT] = 
	{
		IDC_EDIT_ITEM_RES_00,
		IDC_EDIT_ITEM_RES_01,
		IDC_EDIT_ITEM_RES_02,
		IDC_EDIT_ITEM_RES_03,
		IDC_EDIT_ITEM_RES_04,
	};

	for( int i = 0; i < MAX_NEEDITEM_COUNT; i++ )
	{
		CString strITEMREQ;
		SITEM* pITEMREQ = GLItemMan::GetInstance().GetItem ( SNATIVEID(m_Talk.m_sNeedItem[i].dwItemID) );
		if ( pITEMREQ )
			strITEMREQ.Format( "[%d/%d] %s (Num:%d)", pITEMREQ->sBasicOp.sNativeID.wMainID, pITEMREQ->sBasicOp.sNativeID.wSubID, pITEMREQ->GetName(), m_Talk.m_sNeedItem[i].wItemNum );
		SetWin_Text ( this, dwNeedItemID[i], strITEMREQ );
	}


	for( int i = 0; i < MAX_NEEDITEM_COUNT; i++ )
	{
		CString strITEMREQ;
		SITEM* pITEMREQ = GLItemMan::GetInstance().GetItem ( SNATIVEID(m_Talk.m_sResultItem[i].dwItemID) );
		if ( pITEMREQ )
			strITEMREQ.Format( "[%d/%d] %s (Num:%d)", pITEMREQ->sBasicOp.sNativeID.wMainID, pITEMREQ->sBasicOp.sNativeID.wSubID, pITEMREQ->GetName(), m_Talk.m_sResultItem[i].wItemNum );
		SetWin_Text ( this, dwResultItemID[i], strITEMREQ );
	}

	SNATIVEID sID(false);
	SetWin_Num_int( this, IDC_EDIT_ITEM_INSERT_MID, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_ITEM_INSERT_SID, sID.wSubID );
}

void CPageTalk::ShowRandom()
{
	m_listRandom.DeleteAllItems ();
	m_listRandom.SetRedraw( FALSE );

	if ( !m_pNpcDialogueSet )	return;

	for ( int nIndex = 0; nIndex < MAX_RANDOM_PAGE; ++nIndex ) 
	{
		DWORD dwID = m_Talk.m_dwRandomPageID[nIndex];

		if ( dwID == UINT_MAX ) break;

		CNpcDialogue* pNpcDialogue = m_pNpcDialogueSet->GetDialogue( dwID );

		if ( !pNpcDialogue ) break;

		DWORD dwIndex = pNpcDialogue->FindNonCondition();
		if ( dwIndex==UINT_MAX )	dwIndex = 0;

		CNpcDialogueCase* pDlgCase = pNpcDialogue->GetDlgCase(dwIndex);
		if ( !pDlgCase )	break;

		CString strID;
		strID.Format( "%u", dwID );

		int nItem = m_listRandom.InsertItem( nIndex, strID );			
		m_listRandom.SetItemText ( nIndex, 1, pDlgCase->GetBasicTalk().c_str() );
		m_listRandom.SetItemData ( nItem, (DWORD_PTR)dwID );
	}

	m_listRandom.SetRedraw( TRUE );

}