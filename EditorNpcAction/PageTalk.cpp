// PageTalk.cpp : implementation file
//

#include "stdafx.h"
#include "EditorNpcAction.h"
#include "PageTalk.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "DlgQuest.h"
#include "DlgItem.h"
#include "DlgTalk.h"
#include "GLogic.h"

// CPageTalk dialog

IMPLEMENT_DYNAMIC(CPageTalk, CPropertyPage)

CPageTalk::CPageTalk( LOGFONT logfont )
	: CPropertyPage(CPageTalk::IDD)
	, m_pFont ( NULL )
	, m_CallPage(0)
	, m_bModify(FALSE)
	, m_pOrgTalk(NULL)
	, m_pTalkControl(NULL)
	, m_pNpcDialogueSet(NULL)
{
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageTalk::~CPageTalk()
{
	SAFE_DELETE ( m_pFont );
}

void CPageTalk::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_ACTION_TYPE, m_ComboType );
	DDX_Control(pDX, IDC_COMBO_ACTION_DATA, m_ComboData );
	DDX_Control(pDX, IDC_LIST_QUEST_STEP, m_listQuestStep );
	DDX_Control(pDX, IDC_LIST_CASE_RANDOM, m_listRandom);
}


BEGIN_MESSAGE_MAP(CPageTalk, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnBnClickedButtonOk)
	ON_CBN_SELCHANGE(IDC_COMBO_ACTION_TYPE, OnCbnSelchangeComboActionType)
	ON_CBN_SELCHANGE(IDC_COMBO_ACTION_DATA, OnCbnSelchangeComboActionData)
	ON_BN_CLICKED(IDC_BUTTON_CONDITION_EDIT, OnBnClickedButtonConditionEdit)
	ON_BN_CLICKED(IDC_BUTTON_CONDITION_DELETE, OnBnClickedButtonConditionDelete)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_INSERT_0, OnBnClickedButtonQuestInsert0)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_INSERT_1, OnBnClickedButtonQuestInsert1)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_INSERT_2, OnBnClickedButtonQuestInsert2)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_INSERT_3, OnBnClickedButtonQuestInsert3)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_INSERT_4, OnBnClickedButtonQuestInsert4)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_ADD_0, OnBnClickedButtonQuestAdd0)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_ADD_1, OnBnClickedButtonQuestAdd1)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_ADD_2, OnBnClickedButtonQuestAdd2)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_ADD_3, OnBnClickedButtonQuestAdd3)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_ADD_4, OnBnClickedButtonQuestAdd4)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_DEL_0, OnBnClickedButtonQuestDel0)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_DEL_1, OnBnClickedButtonQuestDel1)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_DEL_2, OnBnClickedButtonQuestDel2)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_DEL_3, OnBnClickedButtonQuestDel3)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_DEL_4, OnBnClickedButtonQuestDel4)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_INSERT_PROG, OnBnClickedButtonQuestInsertProg)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_ADD_PROG, OnBnClickedButtonQuestAddProg)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_DEL_PROG, OnBnClickedButtonQuestDelProg)
	ON_BN_CLICKED(IDC_BUTTON_QUEST_STEP, OnBnClickedButtonQuestStep)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_INSERT, OnBnClickedButtonItemInsert)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_ADD, OnBnClickedButtonItemAdd)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_DEL, OnBnClickedButtonItemDel)
	ON_BN_CLICKED(IDC_BUTTON_RANDOM_ADD, OnBnClickedButtonRandomAdd)
	ON_BN_CLICKED(IDC_BUTTON_RANDOM_DEL, OnBnClickedButtonRandomDel)
	ON_BN_CLICKED(IDC_BUTTON_RANDOM_RESET, OnBnClickedButtonRandomReset)
	ON_BN_CLICKED(IDC_BUTTON_BUSLIST, OnBnClickedButtonBuslist)
END_MESSAGE_MAP()


// CPageTalk message handlers
BOOL CPageTalk::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	DataDefault();

	return TRUE;  
}


void CPageTalk::SetTalkData ( CNpcDialogueSet* pNpcDialogueSet, CNpcTalkControl* pTalkControl )
{
	assert ( pTalkControl && "Invalid pTalkControl Pointer." );
	assert ( pNpcDialogueSet && "Invalid pNpcDialogueSet Pointer." );

	m_pNpcDialogueSet = pNpcDialogueSet;
	m_pTalkControl = pTalkControl;
}

void CPageTalk::SetModifyData ( BOOL bModify, SNpcTalk* pTalk )
{
	m_bModify = bModify;
	if ( bModify )
	{
		m_pOrgTalk = pTalk;
		m_Talk = *pTalk;
	}
}

void CPageTalk::OnBnClickedButtonCancel()
{
	m_pSheetTab->ActiveActionAnswer ();
}

void CPageTalk::OnBnClickedButtonOk()
{
	if ( DataSave() )
		m_pSheetTab->ActiveActionAnswer();
}

void CPageTalk::SetWndView ( int nACTION, DWORD dwNO )
{
	BOOL bTRADE = ( nACTION==SNpcTalk::EM_BASIC && dwNO == SNpcTalk::EM_ITEM_TRADE );
	BOOL bQSTART = ( nACTION==SNpcTalk::EM_QUEST_START );
	BOOL bQSTEP = ( nACTION==SNpcTalk::EM_QUEST_STEP );
	BOOL bRandom = ( nACTION==SNpcTalk::EM_BASIC && dwNO == SNpcTalk::EM_RANDOM_PAGE );

	{
		DWORD dwWNDID[] = 
		{
			IDC_EDIT_ITEM_INSERT_MID, IDC_EDIT_ITEM_INSERT_SID, IDC_BUTTON_ITEM_INSERT, IDC_BUTTON_ITEM_ADD, IDC_BUTTON_ITEM_DEL, 
			IDC_EDIT_ITEM_REQ_0, IDC_EDIT_ITEM_REQ_1, IDC_EDIT_ITEM_REQ_2, IDC_EDIT_ITEM_REQ_3, IDC_EDIT_ITEM_REQ_4, IDC_EDIT_ITEM, IDC_STATIC_TRADE,
			IDC_STATIC_REQ_ITEM_0, IDC_STATIC_REQ_ITEM_1, IDC_STATIC_REQ_ITEM_2, IDC_STATIC_REQ_ITEM_3, IDC_STATIC_REQ_ITEM_4,
			IDC_EDIT_ITEM_RES_00, IDC_EDIT_ITEM_RES_01, IDC_EDIT_ITEM_RES_02, IDC_EDIT_ITEM_RES_03, IDC_EDIT_ITEM_RES_04,
			IDC_STATIC_RES_ITEM_00, IDC_STATIC_RES_ITEM_01, IDC_STATIC_RES_ITEM_02, IDC_STATIC_RES_ITEM_03, IDC_STATIC_RES_ITEM_04,
		};

		DWORD dwWNDNUM = sizeof(dwWNDID)/sizeof(DWORD);
		for ( DWORD i=0; i<dwWNDNUM; ++i )
			SetWin_ShowWindow( this, dwWNDID[i], bTRADE );
	}

	{
		DWORD dwWNDID[] = 
		{
			IDC_EDIT_QUEST_INSERT_0, IDC_EDIT_QUEST_INSERT_1, IDC_EDIT_QUEST_INSERT_2, IDC_EDIT_QUEST_INSERT_3, IDC_EDIT_QUEST_INSERT_4,
			IDC_BUTTON_QUEST_INSERT_0, IDC_BUTTON_QUEST_INSERT_1, IDC_BUTTON_QUEST_INSERT_2, IDC_BUTTON_QUEST_INSERT_3, IDC_BUTTON_QUEST_INSERT_4,
			IDC_BUTTON_QUEST_ADD_0, IDC_BUTTON_QUEST_ADD_1, IDC_BUTTON_QUEST_ADD_2, IDC_BUTTON_QUEST_ADD_3, IDC_BUTTON_QUEST_ADD_4,
			IDC_BUTTON_QUEST_DEL_0, IDC_BUTTON_QUEST_DEL_1, IDC_BUTTON_QUEST_DEL_2, IDC_BUTTON_QUEST_DEL_3, IDC_BUTTON_QUEST_DEL_4,
			IDC_EDIT_QUEST_0, IDC_EDIT_QUEST_1, IDC_EDIT_QUEST_2, IDC_EDIT_QUEST_3, IDC_EDIT_QUEST_4, IDC_STATIC_QUEST_START,
		};

		DWORD dwWNDNUM = sizeof(dwWNDID)/sizeof(DWORD);
		for ( DWORD i=0; i<dwWNDNUM; ++i )
			SetWin_ShowWindow( this, dwWNDID[i], bQSTART );
	}


	{
		DWORD dwWNDID[] = 
		{
			IDC_EDIT_QUEST_INSERT_PROG, IDC_EDIT_QUEST_PROG, IDC_EDIT_QUEST_STEP, IDC_LIST_QUEST_STEP, IDC_STATIC_QUEST_STEP_TEXT_0, IDC_STATIC_PROG_QUEST,
			IDC_BUTTON_QUEST_INSERT_PROG, IDC_BUTTON_QUEST_ADD_PROG, IDC_BUTTON_QUEST_DEL_PROG, IDC_BUTTON_QUEST_STEP
		};

		DWORD dwWNDNUM = sizeof(dwWNDID)/sizeof(DWORD);
		for ( DWORD i=0; i<dwWNDNUM; ++i )
			SetWin_ShowWindow( this, dwWNDID[i], bQSTEP );
	}


	{
		DWORD dwWNDID[] = 
		{
			IDC_EDIT_RANDOM_TIME, IDC_LIST_CASE_RANDOM, IDC_STATIC_RANDOM_TIME_GROUP, IDC_STATIC_RANDOM_TIME_TEXT, 
			IDC_BUTTON_RANDOM_ADD, IDC_BUTTON_RANDOM_DEL, IDC_BUTTON_RANDOM_RESET,
		};

		DWORD dwWNDNUM = sizeof(dwWNDID)/sizeof(DWORD);
		for ( DWORD i=0; i<dwWNDNUM; ++i )
			SetWin_ShowWindow( this, dwWNDID[i], bRandom );
	}

	if ( bTRADE )
		ShowItem();

	if ( bQSTART )
		ShowQuestStart();

	if ( bQSTEP )
		ShowQuestProgress();

	if ( bRandom )
		ShowRandom();
}

void CPageTalk::OnCbnSelchangeComboActionType()
{
	InitComboType();
}

void CPageTalk::OnCbnSelchangeComboActionData()
{
	InitComboData();
}

void CPageTalk::OnBnClickedButtonConditionEdit()
{
	if ( DataSave() )
	{
		if ( !m_Talk.GetCondition() )
			m_Talk.SetCondition ( SNpcTalkCondition() );

		m_pSheetTab->ActiveActionCondition ( PAGE_TALK, m_Talk.GetCondition() );
	}
}

void CPageTalk::OnBnClickedButtonConditionDelete()
{
	if ( MessageBox ( "Do you want to delete condition?", "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_Talk.ResetCondition ();
		ShowCondition();
	}
}

void CPageTalk::QuestStartInsert( int nINDEX, DWORD dwGUID )
{
	DWORD dwID = GetWin_Num_int( this, dwGUID );
	QuestStartAdd( nINDEX, dwID );
}

void CPageTalk::QuestStartAdd( int nINDEX, DWORD dwQUESTID )
{
	if ( nINDEX < 0 )					return;
	if ( nINDEX >= MAX_QUEST_START )	return;

	GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( dwQUESTID );
	if ( !pQUEST )
	{
		MessageBox( "Invalid Quest" );
		return;
	}

	m_Talk.m_dwQuestStartID[nINDEX] = pQUEST->m_sNID.dwID;
	ShowQuestStart();
}

void CPageTalk::QuestStartDel( int nINDEX )
{
	if ( nINDEX < 0 )					return;
	if ( nINDEX >= MAX_QUEST_START )	return;

	CString strTemp;
	strTemp.Format ( "Do you want to delete Quest?\n[%d]", m_Talk.m_dwQuestStartID[nINDEX] );

	if ( MessageBox ( strTemp, "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_Talk.m_dwQuestStartID[nINDEX] = UINT_MAX;
		ShowQuestStart();
	}
}

void CPageTalk::OnBnClickedButtonQuestInsert0()
{
	QuestStartInsert( 0, IDC_EDIT_QUEST_INSERT_0 );
}

void CPageTalk::OnBnClickedButtonQuestInsert1()
{
	QuestStartInsert( 1, IDC_EDIT_QUEST_INSERT_1 );
}

void CPageTalk::OnBnClickedButtonQuestInsert2()
{
	QuestStartInsert( 2, IDC_EDIT_QUEST_INSERT_2 );
}

void CPageTalk::OnBnClickedButtonQuestInsert3()
{
	QuestStartInsert( 3, IDC_EDIT_QUEST_INSERT_3 );
}

void CPageTalk::OnBnClickedButtonQuestInsert4()
{
	QuestStartInsert( 4, IDC_EDIT_QUEST_INSERT_4 );
}

void CPageTalk::OnBnClickedButtonQuestAdd0()
{
	CDlgQuest dlg;
	if ( dlg.DoModal() == IDOK )
		QuestStartAdd( 0, dlg.m_SelectedQuest );
}

void CPageTalk::OnBnClickedButtonQuestAdd1()
{
	CDlgQuest dlg;
	if ( dlg.DoModal() == IDOK )
		QuestStartAdd( 1, dlg.m_SelectedQuest );
}

void CPageTalk::OnBnClickedButtonQuestAdd2()
{
	CDlgQuest dlg;
	if ( dlg.DoModal() == IDOK )
		QuestStartAdd( 2, dlg.m_SelectedQuest );
}

void CPageTalk::OnBnClickedButtonQuestAdd3()
{
	CDlgQuest dlg;
	if ( dlg.DoModal() == IDOK )
		QuestStartAdd( 3, dlg.m_SelectedQuest );
}

void CPageTalk::OnBnClickedButtonQuestAdd4()
{
	CDlgQuest dlg;
	if ( dlg.DoModal() == IDOK )
		QuestStartAdd( 4, dlg.m_SelectedQuest );
}

void CPageTalk::OnBnClickedButtonQuestDel0()
{
	QuestStartDel( 0 );
}

void CPageTalk::OnBnClickedButtonQuestDel1()
{
	QuestStartDel( 1 );
}

void CPageTalk::OnBnClickedButtonQuestDel2()
{
	QuestStartDel( 2 );
}

void CPageTalk::OnBnClickedButtonQuestDel3()
{
	QuestStartDel( 3 );
}

void CPageTalk::OnBnClickedButtonQuestDel4()
{
	QuestStartDel( 4 );
}

void CPageTalk::OnBnClickedButtonQuestInsertProg()
{
	DWORD dwID = GetWin_Num_int( this, IDC_EDIT_QUEST_INSERT_PROG );
	GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( dwID );
	if ( !pQUEST )
	{
		MessageBox( "Invalid Quest" );
		return;
	}

	m_Talk.m_dwACTION_PARAM1 = pQUEST->m_sNID.dwID;
	m_Talk.m_dwACTION_PARAM2 = 0;
	ShowQuestProgress();
}

void CPageTalk::OnBnClickedButtonQuestAddProg()
{
	CDlgQuest dlg;
	if ( dlg.DoModal() == IDOK )
	{
		GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( dlg.m_SelectedQuest );
		if ( !pQUEST )
		{
			MessageBox( "Invalid Quest" );
			return;
		}

		m_Talk.m_dwACTION_PARAM1 = pQUEST->m_sNID.dwID;
		m_Talk.m_dwACTION_PARAM2 = 0;
		ShowQuestProgress();
	}
}

void CPageTalk::OnBnClickedButtonQuestDelProg()
{
	CString strTemp;
	strTemp.Format ( "Do you want to delete Quest?\n[%d]", m_Talk.m_dwACTION_PARAM1 );

	if ( MessageBox ( strTemp, "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_Talk.m_dwACTION_PARAM1 = UINT_MAX;
		m_Talk.m_dwACTION_PARAM2 = 0;
		ShowQuestProgress();
	}
}

void CPageTalk::OnBnClickedButtonQuestStep()
{
	int nIndex = m_listQuestStep.GetCurSel ();
	if ( LB_ERR==nIndex )	return;
	m_Talk.m_dwACTION_PARAM2 = nIndex;
	ShowQuestProgress();
}

void CPageTalk::OnBnClickedButtonItemInsert()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_ITEM_INSERT_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_ITEM_INSERT_SID );
	SITEM* pITEM = GLItemMan::GetInstance().GetItem( wMID, wSID );
	if ( !pITEM )
	{
		MessageBox( "Invalid Item" );
		return;
	}

	m_Talk.m_dwACTION_PARAM2 = pITEM->sBasicOp.sNativeID.dwID;

	//jdevtodo
	//fix later when the new logic for item result is implemented
	SNPC_ITEM snpc_item;
	snpc_item.dwItemID = m_Talk.m_dwACTION_PARAM2;
	snpc_item.wItemNum = 1;
	m_Talk.m_sResultItem[0] = snpc_item;

	ShowItem();
}

void CPageTalk::OnBnClickedButtonItemAdd()
{
	CDlgItem dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( dlg.m_SELECTED );
		if ( !pITEM )
		{
			MessageBox( "Invalid Item" );
			return;
		}

		m_Talk.m_dwACTION_PARAM2 = pITEM->sBasicOp.sNativeID.dwID;
		ShowItem();
	}
}

void CPageTalk::OnBnClickedButtonItemDel()
{
	SNATIVEID sID( m_Talk.m_dwACTION_PARAM2 );

	CString strTemp;
	strTemp.Format ( "Do you want to delete Item?\n[%d/%d]", sID.wMainID, sID.wSubID );

	if ( MessageBox ( strTemp, "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_Talk.m_dwACTION_PARAM2 = UINT_MAX;
		ShowItem();
	}
}

void CPageTalk::OnBnClickedButtonRandomAdd()
{
	if ( !m_pNpcDialogueSet )	return;

	CDlgTalk dlg( m_pNpcDialogueSet );
	if ( dlg.DoModal() == IDOK )
	{
		DWORD nID = dlg.m_dwNpcID;

		if ( nID == UINT_MAX ) return;			

		CNpcDialogue* pNpcDialogue = m_pNpcDialogueSet->GetDialogue( nID );

		if ( !pNpcDialogue ) return;

		DWORD dwIndex = pNpcDialogue->FindNonCondition();
		if ( dwIndex==UINT_MAX )	dwIndex = 0;

		CNpcDialogueCase* pDlgCase = pNpcDialogue->GetDlgCase(dwIndex);
		if ( !pDlgCase )	return;		

		for ( int i = 0; i < MAX_RANDOM_PAGE; ++i ) 
		{
			if ( m_Talk.m_dwRandomPageID[i] == UINT_MAX ) 
			{
				m_Talk.m_dwRandomPageID[i] = nID;
				break;
			}
		}
		
		ShowRandom();
	}
}

void CPageTalk::OnBnClickedButtonRandomDel()
{
	int nItem = -1;

	nItem = m_listRandom.GetNextItem( nItem, LVIS_SELECTED );
	if ( nItem < 0 || nItem >= MAX_RANDOM_PAGE ) return;	

	for ( int i = nItem+1; i < MAX_RANDOM_PAGE; ++i  ) 
	{
		m_Talk.m_dwRandomPageID[i-1] = m_Talk.m_dwRandomPageID[i];
	}

	m_Talk.m_dwRandomPageID[MAX_RANDOM_PAGE-1] = UINT_MAX;

	ShowRandom();
}

void CPageTalk::OnBnClickedButtonRandomReset()
{
	for ( int i = 0; i < MAX_RANDOM_PAGE; ++i ) 
	{
		m_Talk.m_dwRandomPageID[i] = UINT_MAX;
	}
	
	ShowRandom();
}

void CPageTalk::OnBnClickedButtonBuslist()
{
	CString szFilter = "BUS Station List (*.ini)|*.ini|";

	CFileDialog dlg(TRUE,".ini",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
		SetWin_Text ( this, IDC_EDIT_BUS_LIST, dlg.GetFileName().GetString() );
}
