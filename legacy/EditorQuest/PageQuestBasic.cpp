// PageQuestBasic.cpp : implementation file
//

#include "stdafx.h"
#include "EditorQuest.h"
#include "PageQuestBasic.h"
#include "EditorQuestDlg.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"

#include "DlgItem.h"
#include "DlgSkill.h"
#include "DlgMap.h"
#include "DlgItemEdit.h"

// CPageQuestBasic dialog

IMPLEMENT_DYNAMIC(CPageQuestBasic, CPropertyPage)

CPageQuestBasic::CPageQuestBasic( LOGFONT logfont )
	: CPropertyPage(CPageQuestBasic::IDD)
	, m_pFont ( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pQuest = NULL;
	m_DummyQuest.pQUEST = new GLQUEST;
}

CPageQuestBasic::~CPageQuestBasic()
{
	SAFE_DELETE ( m_pFont );
	SAFE_DELETE ( m_DummyQuest.pQUEST );
}

void CPageQuestBasic::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_SCHOOL, m_comboSchool);
	DDX_Control(pDX, IDC_LIST_GIFT_ITEM, m_ListBoxItem);
	DDX_Control(pDX, IDC_LIST_GIFT_SKILL, m_ListBoxSkill);
}


BEGIN_MESSAGE_MAP(CPageQuestBasic, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, OnBnClickedButtonNext)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_ADD, OnBnClickedButtonItemAdd)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_DEL, OnBnClickedButtonItemDel)
	ON_EN_CHANGE(IDC_EDIT_START_ITEM_MID, OnEnChangeEditStartItemMid)
	ON_EN_CHANGE(IDC_EDIT_START_ITEM_SID, OnEnChangeEditStartItemSid)
	ON_BN_CLICKED(IDC_BUTTON_SKILL_ADD, OnBnClickedButtonSkillAdd)
	ON_EN_CHANGE(IDC_EDIT_START_SKILL_MID, OnEnChangeEditStartSkillMid)
	ON_EN_CHANGE(IDC_EDIT_START_SKILL_SID, OnEnChangeEditStartSkillSid)
	ON_BN_CLICKED(IDC_BUTTON_SKILL_DEL, OnBnClickedButtonSkillDel)
	ON_BN_CLICKED(IDC_BUTTON_MAP_ADD, OnBnClickedButtonMapAdd)
	ON_EN_CHANGE(IDC_EDIT_START_QUEST_MAP_MID, OnEnChangeEditStartQuestMapMid)
	ON_EN_CHANGE(IDC_EDIT_START_QUEST_MAP_SID, OnEnChangeEditStartQuestMapSid)
	ON_BN_CLICKED(IDC_BUTTON_MAP_DEL, OnBnClickedButtonMapDel)
	ON_BN_CLICKED(IDC_BUTTON_MAP_ADD_START, OnBnClickedButtonMapAddStart)
	ON_EN_CHANGE(IDC_EDIT_MAP_MID_START, OnEnChangeEditMapMidStart)
	ON_EN_CHANGE(IDC_EDIT_MAP_SID_START, OnEnChangeEditMapSidStart)
	ON_BN_CLICKED(IDC_BUTTON_MAP_DEL_START, OnBnClickedButtonMapDelStart)
	ON_BN_CLICKED(IDC_BUTTON_MAP_ADD_END, OnBnClickedButtonMapAddEnd)
	ON_EN_CHANGE(IDC_EDIT_MAP_MID_END, OnEnChangeEditMapMidEnd)
	ON_EN_CHANGE(IDC_EDIT_MAP_SID_END, OnEnChangeEditMapSidEnd)
	ON_BN_CLICKED(IDC_BUTTON_MAP_DEL_END, OnBnClickedButtonMapDelEnd)
	ON_BN_CLICKED(IDC_BUTTON_MAP_ADD_FAIL, OnBnClickedButtonMapAddFail)
	ON_EN_CHANGE(IDC_EDIT_MAP_MID_FAIL, OnEnChangeEditMapMidFail)
	ON_EN_CHANGE(IDC_EDIT_MAP_SID_FAIL, OnEnChangeEditMapSidFail)
	ON_BN_CLICKED(IDC_BUTTON_MAP_DEL_FAIL, OnBnClickedButtonMapDelFail)
	ON_BN_CLICKED(IDC_BUTTON_GIFT_ITEM_ADD, OnBnClickedButtonGiftItemAdd)
	ON_BN_CLICKED(IDC_BUTTON_GIFT_ITEM_DEL, OnBnClickedButtonGiftItemDel)
	ON_BN_CLICKED(IDC_BUTTON_GIFT_ITEM_EDIT, OnBnClickedButtonGiftItemEdit)
	ON_BN_CLICKED(IDC_BUTTON_GIFT_ITEM_INSERT, OnBnClickedButtonGiftItemInsert)
	ON_BN_CLICKED(IDC_BUTTON_GIFT_SKILL_ADD, OnBnClickedButtonGiftSkillAdd)
	ON_BN_CLICKED(IDC_BUTTON_GIFT_SKILL_DEL, OnBnClickedButtonGiftSkillDel)
	ON_BN_CLICKED(IDC_BUTTON_GIFT_SKILL_INSERT, OnBnClickedButtonGiftSkillInsert)
	ON_BN_CLICKED(IDC_BUTTON_ALL_MALE, OnBnClickedButtonAllMale)
	ON_BN_CLICKED(IDC_BUTTON_ALL_FEMALE, OnBnClickedButtonAllFemale)
	ON_BN_CLICKED(IDC_BUTTON_ALL, OnBnClickedButtonAll)
	ON_BN_CLICKED(IDC_BUTTON_NOTHING, OnBnClickedButtonNothing)
	ON_BN_CLICKED(IDC_BUTTON_MAP_DEL_START2, OnBnClickedButtonMapDelStart2)
	ON_BN_CLICKED(IDC_BUTTON_MAP_DEL_END2, OnBnClickedButtonMapDelEnd2)
	ON_BN_CLICKED(IDC_BUTTON_MAP_DEL_FAIL2, OnBnClickedButtonMapDelFail2)
END_MESSAGE_MAP()


// CPageQuestBasic message handlers
BOOL CPageQuestBasic::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_bDlgInit = TRUE;

	DataDefault();
	DataShow();

	return TRUE;
}

BOOL CPageQuestBasic::QuestSet( GLQuestMan::SNODE* pQUEST )
{
	if ( m_pQuest )	return FALSE;

	m_pQuest = pQUEST;
	*m_DummyQuest.pQUEST = *m_pQuest->pQUEST;
	m_DummyQuest.pQUEST->m_ProgressEvent = m_pQuest->pQUEST->m_ProgressEvent;
	m_DummyQuest.strFILE = m_pQuest->strFILE.c_str();

	if ( m_bDlgInit ) 
	{
		DataDefault();
		DataShow();	
	}

	return TRUE;
}

void CPageQuestBasic::QuestClear()
{
	m_pQuest = NULL;
}

void CPageQuestBasic::QuestSave()
{
	GLQuestMan::MAPQUEST& m_mapQuestMap = GLQuestMan::GetInstance().GetMap();
	GLQuestMan::MAPQUEST_ITER iter_pos = m_mapQuestMap.find( m_DummyQuest.pQUEST->m_sNID.dwID );
	if ( iter_pos == m_mapQuestMap.end() ) return;

	if ( iter_pos->second.strFILE != m_DummyQuest.strFILE )
	{
		CString strTemp = GLQuestMan::GetInstance().GetPath();

		strTemp += iter_pos->second.strFILE.c_str();
		if ( !DeleteFile( strTemp ) )
		{
			MessageBox( "Not Found File",strTemp, MB_OK ) ;
		}

		iter_pos->second.strFILE = m_DummyQuest.strFILE;
	}	

	*iter_pos->second.pQUEST = *m_DummyQuest.pQUEST;
	if ( iter_pos->second.pQUEST )
	{
		iter_pos->second.pQUEST->m_ProgressEvent = m_DummyQuest.pQUEST->m_ProgressEvent;
		iter_pos->second.pQUEST->SAVE( iter_pos->second.strFILE.c_str() );
	}
}

void CPageQuestBasic::OnBnClickedButtonSave()
{
	if ( ValidateProgress() )
	{
		if ( DataSave() )
		{
			QuestSave();
			m_pQuest = NULL;
			m_pSheetTab->ActiveQuestTreePage ();
		}
	}
}

void CPageQuestBasic::OnBnClickedButtonCancel()
{
	m_pQuest = NULL;
	m_pSheetTab->ActiveQuestTreePage ();
}

void CPageQuestBasic::OnBnClickedButtonNext()
{
	if ( ValidateProgress() )
	{
		if ( DataSave() )
		{
			GLQuestMan::GetInstance().DoModified ();
			QuestSave();
			m_pSheetTab->ActiveQuestStepPage( QUEST_BASIC_PAGE, m_pQuest );
			m_pQuest = NULL;
		}
	}
}

void CPageQuestBasic::OnBnClickedButtonItemAdd()
{
	CDlgItem dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( dlg.m_SELECTED );
		if ( pITEM )
		{
			SetWin_Num_int( this, IDC_EDIT_START_ITEM_MID, pITEM->sBasicOp.sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_START_ITEM_SID, pITEM->sBasicOp.sNativeID.wSubID );
			CheckItem();
		}
	}
}

void CPageQuestBasic::CheckItem()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_START_ITEM_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_START_ITEM_SID );

	std::string strNAME = "Unknown Item";
	SITEM* pITEM = GLItemMan::GetInstance().GetItem( wMID, wSID );
	if ( pITEM )	strNAME = pITEM->GetName();
	SetWin_Text( this, IDC_EDIT_START_ITEM_NAME, strNAME.c_str() );
}

void CPageQuestBasic::OnBnClickedButtonItemDel()
{
	SNATIVEID sID( false );
	SetWin_Num_int( this, IDC_EDIT_START_ITEM_MID, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_START_ITEM_SID, sID.wSubID );
	CheckItem();
}

void CPageQuestBasic::OnEnChangeEditStartItemMid()
{
	CheckItem();
}

void CPageQuestBasic::OnEnChangeEditStartItemSid()
{
	CheckItem();
}

void CPageQuestBasic::OnBnClickedButtonSkillAdd()
{
	CDlgSkill dlg;
	if ( dlg.DoModal() == IDOK )
	{
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( dlg.m_SELECTED );
		if ( pSKILL )
		{
			SetWin_Num_int( this, IDC_EDIT_START_SKILL_MID, pSKILL->m_sBASIC.sNATIVEID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_START_SKILL_SID, pSKILL->m_sBASIC.sNATIVEID.wSubID );
			CheckSkill();
		}
	}
}

void CPageQuestBasic::CheckSkill()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_START_SKILL_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_START_SKILL_SID );

	std::string strNAME = "Unknown Skill";
	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( wMID, wSID );
	if ( pSKILL )	strNAME = pSKILL->GetName();
	SetWin_Text( this, IDC_EDIT_START_SKILL_NAME, strNAME.c_str() );
}

void CPageQuestBasic::OnEnChangeEditStartSkillMid()
{
	CheckSkill();
}

void CPageQuestBasic::OnEnChangeEditStartSkillSid()
{
	CheckSkill();
}

void CPageQuestBasic::OnBnClickedButtonSkillDel()
{
	SNATIVEID sID( false );
	SetWin_Num_int( this, IDC_EDIT_START_SKILL_MID, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_START_SKILL_SID, sID.wSubID );
	CheckSkill();
}

void CPageQuestBasic::OnBnClickedButtonMapAdd()
{
	CDlgMap dlg;
	if( dlg.DoModal() == IDOK )
	{
		SMAPNODE* pNode = CEditorQuestDlg::m_sMapList.FindMapNode( dlg.m_SELECTED );
		if ( pNode )
		{
			SetWin_Num_int( this, IDC_EDIT_START_QUEST_MAP_MID, pNode->sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_START_QUEST_MAP_SID, pNode->sNativeID.wSubID );

			CheckMap();
		}
	}
}

void CPageQuestBasic::CheckMap()
{
	SNATIVEID sID;
	sID.wMainID = GetWin_Num_int( this, IDC_EDIT_START_QUEST_MAP_MID );
	sID.wSubID = GetWin_Num_int( this, IDC_EDIT_START_QUEST_MAP_SID );

	std::string strTEXT = "UnknownMap";
	SMAPNODE* pNode = CEditorQuestDlg::m_sMapList.FindMapNode( sID );
	if ( pNode )	strTEXT = pNode->strMapName.c_str();
	SetWin_Text( this, IDC_EDIT_START_QUEST_MAP_NAME, strTEXT.c_str() );
}

void CPageQuestBasic::OnEnChangeEditStartQuestMapMid()
{
	CheckMap();
}

void CPageQuestBasic::OnEnChangeEditStartQuestMapSid()
{
	CheckMap();
}

void CPageQuestBasic::OnBnClickedButtonMapDel()
{
	SNATIVEID sID( false );
	SetWin_Num_int( this, IDC_EDIT_START_QUEST_MAP_MID, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_START_QUEST_MAP_SID, sID.wSubID );
	CheckMap();
}

void CPageQuestBasic::OnBnClickedButtonMapAddStart()
{
	CDlgMap dlg;
	if( dlg.DoModal() == IDOK )
	{
		SMAPNODE* pNode = CEditorQuestDlg::m_sMapList.FindMapNode( dlg.m_SELECTED );
		if ( pNode )
		{
			SetWin_Num_int( this, IDC_EDIT_MAP_MID_START, pNode->sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_MAP_SID_START, pNode->sNativeID.wSubID );

			CheckProgressMapStart();
		}
	}
}

void CPageQuestBasic::CheckProgressMapStart()
{
	SNATIVEID sID;
	sID.wMainID = GetWin_Num_int( this, IDC_EDIT_MAP_MID_START );
	sID.wSubID = GetWin_Num_int( this, IDC_EDIT_MAP_SID_START );

	std::string strTEXT = "UnknownMap";
	SMAPNODE* pNode = CEditorQuestDlg::m_sMapList.FindMapNode( sID );
	if ( pNode )	strTEXT = pNode->strMapName.c_str();
	SetWin_Text( this, IDC_EDIT_MAP_NAME_START, strTEXT.c_str() );
}

void CPageQuestBasic::OnEnChangeEditMapMidStart()
{
	CheckProgressMapStart();
}

void CPageQuestBasic::OnEnChangeEditMapSidStart()
{
	CheckProgressMapStart();
}

void CPageQuestBasic::OnBnClickedButtonMapDelStart()
{
	SNATIVEID sID( false );
	SetWin_Num_int( this, IDC_EDIT_MAP_MID_START, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_MAP_SID_START, sID.wSubID );
	CheckProgressMapStart();
}

void CPageQuestBasic::OnBnClickedButtonMapAddEnd()
{
	CDlgMap dlg;
	if( dlg.DoModal() == IDOK )
	{
		SMAPNODE* pNode = CEditorQuestDlg::m_sMapList.FindMapNode( dlg.m_SELECTED );
		if ( pNode )
		{
			SetWin_Num_int( this, IDC_EDIT_MAP_MID_END, pNode->sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_MAP_SID_END, pNode->sNativeID.wSubID );

			CheckProgressMapEnd();
		}
	}
}

void CPageQuestBasic::CheckProgressMapEnd()
{
	SNATIVEID sID;
	sID.wMainID = GetWin_Num_int( this, IDC_EDIT_MAP_MID_END );
	sID.wSubID = GetWin_Num_int( this, IDC_EDIT_MAP_SID_END );

	std::string strTEXT = "UnknownMap";
	SMAPNODE* pNode = CEditorQuestDlg::m_sMapList.FindMapNode( sID );
	if ( pNode )	strTEXT = pNode->strMapName.c_str();
	SetWin_Text( this, IDC_EDIT_MAP_NAME_END, strTEXT.c_str() );
}

void CPageQuestBasic::OnEnChangeEditMapMidEnd()
{
	CheckProgressMapEnd();
}

void CPageQuestBasic::OnEnChangeEditMapSidEnd()
{
	CheckProgressMapEnd();
}

void CPageQuestBasic::OnBnClickedButtonMapDelEnd()
{
	SNATIVEID sID( false );
	SetWin_Num_int( this, IDC_EDIT_MAP_MID_END, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_MAP_SID_END, sID.wSubID );
	CheckProgressMapEnd();
}

void CPageQuestBasic::OnBnClickedButtonMapAddFail()
{
	CDlgMap dlg;
	if( dlg.DoModal() == IDOK )
	{
		SMAPNODE* pNode = CEditorQuestDlg::m_sMapList.FindMapNode( dlg.m_SELECTED );
		if ( pNode )
		{
			SetWin_Num_int( this, IDC_EDIT_MAP_MID_FAIL, pNode->sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_MAP_SID_FAIL, pNode->sNativeID.wSubID );

			CheckProgressMapFail();
		}
	}
}

void CPageQuestBasic::CheckProgressMapFail()
{
	SNATIVEID sID;
	sID.wMainID = GetWin_Num_int( this, IDC_EDIT_MAP_MID_FAIL );
	sID.wSubID = GetWin_Num_int( this, IDC_EDIT_MAP_SID_FAIL );

	std::string strTEXT = "UnknownMap";
	SMAPNODE* pNode = CEditorQuestDlg::m_sMapList.FindMapNode( sID );
	if ( pNode )	strTEXT = pNode->strMapName.c_str();
	SetWin_Text( this, IDC_EDIT_MAP_NAME_FAIL, strTEXT.c_str() );
}

void CPageQuestBasic::OnEnChangeEditMapMidFail()
{
	CheckProgressMapFail();
}

void CPageQuestBasic::OnEnChangeEditMapSidFail()
{
	CheckProgressMapFail();
}

void CPageQuestBasic::OnBnClickedButtonMapDelFail()
{
	SNATIVEID sID( false );
	SetWin_Num_int( this, IDC_EDIT_MAP_MID_FAIL, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_MAP_SID_FAIL, sID.wSubID );
	CheckProgressMapFail();
}

void CPageQuestBasic::UpdateRewardItem()
{
	m_ListBoxItem.ResetContent();
	if ( !m_DummyQuest.pQUEST )	return;

	m_ListBoxItem.SetRedraw( FALSE );

	INVENQARRAY_ITER iter = m_DummyQuest.pQUEST->m_vecGiftITEM.begin();
	INVENQARRAY_ITER iter_end = m_DummyQuest.pQUEST->m_vecGiftITEM.end();
	for ( int i=0 ; iter!=iter_end; ++iter, ++i )
	{
		const SITEMCUSTOM &sITEM = (*iter);

		CString strNAME = "Invalid Item ( ID NotFound)";
		SITEM *pITEM = GLItemMan::GetInstance().GetItem ( sITEM.sNativeID );
		if ( pITEM )
			strNAME.Format( "%d/%d %s", pITEM->sBasicOp.sNativeID.wMainID, pITEM->sBasicOp.sNativeID.wSubID, pITEM->GetName() );

		int nIndex = m_ListBoxItem.AddString ( strNAME );
		m_ListBoxItem.SetItemData ( nIndex, i );
	}

	m_ListBoxItem.SetRedraw( TRUE );

	SNATIVEID sID( NATIVEID_NULL() );
	SetWin_Num_int( this, IDC_EDIT_INSERT_MID_ITEM, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_INSERT_SID_ITEM, sID.wSubID );
}

void CPageQuestBasic::UpdateRewardSkill()
{
	m_ListBoxSkill.ResetContent();
	if ( !m_DummyQuest.pQUEST )	return;

	m_ListBoxSkill.SetRedraw( FALSE );

	DWQARRAY_ITER iter = m_DummyQuest.pQUEST->m_vecGiftSKILL.begin();
	DWQARRAY_ITER iter_end = m_DummyQuest.pQUEST->m_vecGiftSKILL.end();
	for ( int i=0 ; iter!=iter_end; ++iter, ++i )
	{
		SNATIVEID sSKILLNID;
		sSKILLNID.dwID = (*iter);

		CString strNAME = "Invalid Skill (ID NotFound)";
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData ( sSKILLNID );
		if ( pSKILL )
			strNAME.Format( "%d/%d %s", pSKILL->m_sBASIC.sNATIVEID.wMainID, pSKILL->m_sBASIC.sNATIVEID.wSubID, pSKILL->GetName() );

		int nIndex = m_ListBoxSkill.AddString ( strNAME );
		m_ListBoxSkill.SetItemData ( nIndex, i );
	}

	m_ListBoxSkill.SetRedraw( TRUE );

	SNATIVEID sID( NATIVEID_NULL() );
	SetWin_Num_int( this, IDC_EDIT_INSERT_MID_SKILL, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_INSERT_SID_SKILL, sID.wSubID );
}

void CPageQuestBasic::OnBnClickedButtonGiftItemAdd()
{
	CDlgItem dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( dlg.m_SELECTED );
		if ( pITEM )
		{
			SITEMCUSTOM sITEMCUSTOM;
			sITEMCUSTOM.sNativeID = pITEM->sBasicOp.sNativeID;

			if ( pITEM->ISPILE() )	
				sITEMCUSTOM.wTurnNum = pITEM->GETAPPLYNUM();

			m_DummyQuest.pQUEST->m_vecGiftITEM.push_back(sITEMCUSTOM);
			UpdateRewardItem();
		}
	}
}

void CPageQuestBasic::OnBnClickedButtonGiftItemDel()
{
	if ( !m_DummyQuest.pQUEST )	return;

	int nIndex = m_ListBoxItem.GetCurSel();
	if ( LB_ERR == nIndex )	return;

	DWORD dwIndex = (DWORD) m_ListBoxItem.GetItemData ( nIndex );
	SITEMCUSTOM &sITEM = m_DummyQuest.pQUEST->m_vecGiftITEM[dwIndex];

	CString strQUESTION = "Delete Item 'Invalid Item (ID NotFound)'?";
	SITEM *pITEM = GLItemMan::GetInstance().GetItem ( sITEM.sNativeID );
	if ( pITEM )
		strQUESTION.Format( "Delete Item '%s'?", pITEM->GetName() );

	int nReturn = MessageBox ( strQUESTION, "Question", MB_YESNO );
	if ( nReturn==IDYES )
	{
		INVENQARRAY_ITER iter = m_DummyQuest.pQUEST->m_vecGiftITEM.begin()+dwIndex;
		m_DummyQuest.pQUEST->m_vecGiftITEM.erase ( iter, iter+1 );
		UpdateRewardItem();
	}
}

void CPageQuestBasic::OnBnClickedButtonGiftItemEdit()
{
	if ( !m_DummyQuest.pQUEST )	return;

	int nIndex = m_ListBoxItem.GetCurSel();
	if ( LB_ERR == nIndex )	return;

	DWORD dwIndex = (DWORD) m_ListBoxItem.GetItemData ( nIndex );
	SITEMCUSTOM &sITEM = m_DummyQuest.pQUEST->m_vecGiftITEM[dwIndex];

	CDlgItemEdit dlg;
	dlg.m_sITEMCUSTOM = sITEM;

	if ( dlg.DoModal() == IDOK )
	{
		sITEM = dlg.m_sITEMCUSTOM;
	}
}

void CPageQuestBasic::OnBnClickedButtonGiftItemInsert()
{
	if ( !m_DummyQuest.pQUEST )	return;

	WORD wMID = GetWin_Num_int( this, IDC_EDIT_INSERT_MID_ITEM );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_INSERT_SID_ITEM );

	SITEM* pITEM = GLItemMan::GetInstance().GetItem( wMID, wSID );
	if ( pITEM )
	{
		SITEMCUSTOM sITEMCUSTOM;
		sITEMCUSTOM.sNativeID = pITEM->sBasicOp.sNativeID;

		if ( pITEM->ISPILE() )	
			sITEMCUSTOM.wTurnNum = pITEM->GETAPPLYNUM();

		m_DummyQuest.pQUEST->m_vecGiftITEM.push_back(sITEMCUSTOM);
		UpdateRewardItem();
	}else{
		MessageBox( "Invalid Item" );
	}
}

void CPageQuestBasic::OnBnClickedButtonGiftSkillAdd()
{
	if ( !m_DummyQuest.pQUEST )	return;

	CDlgSkill dlg;
	if ( dlg.DoModal() == IDOK )
	{
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( dlg.m_SELECTED );
		if ( pSKILL )
		{
			m_DummyQuest.pQUEST->m_vecGiftSKILL.push_back ( pSKILL->m_sBASIC.sNATIVEID.dwID );
			UpdateRewardSkill();
		}
	}
}

void CPageQuestBasic::OnBnClickedButtonGiftSkillDel()
{
	if ( !m_DummyQuest.pQUEST )	return;

	int nIndex = m_ListBoxSkill.GetCurSel();
	if ( LB_ERR == nIndex )	return;

	DWORD dwIndex = (DWORD) m_ListBoxSkill.GetItemData ( nIndex );
	DWORD &dwSKILL_NID = m_DummyQuest.pQUEST->m_vecGiftSKILL[dwIndex];

	SNATIVEID sSKILLNID;
	sSKILLNID.dwID = dwSKILL_NID;

	CString strQUESTION = "Delete Skill 'Invalid Skill ( ID NotFound )'? ";
	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData ( sSKILLNID );
	if ( pSKILL )	
		strQUESTION.Format( "Delete Skill '%s'?", pSKILL->GetName() );

	int nReturn = MessageBox ( strQUESTION, "Question", MB_YESNO );
	if ( nReturn==IDYES )
	{
		DWQARRAY_ITER iter = m_DummyQuest.pQUEST->m_vecGiftSKILL.begin()+dwIndex;
		m_DummyQuest.pQUEST->m_vecGiftSKILL.erase ( iter, iter+1 );

		UpdateRewardSkill();
	}
}

void CPageQuestBasic::OnBnClickedButtonGiftSkillInsert()
{
	if ( !m_DummyQuest.pQUEST )	return;

	WORD wMID = GetWin_Num_int( this, IDC_EDIT_INSERT_MID_SKILL );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_INSERT_SID_SKILL );

	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( wMID, wSID );
	if ( pSKILL )
	{
		m_DummyQuest.pQUEST->m_vecGiftSKILL.push_back ( pSKILL->m_sBASIC.sNATIVEID.dwID );
		UpdateRewardSkill();
	}else{
		MessageBox( "Invalid Skill" );
	}
}

void CPageQuestBasic::UpdateClass( BOOL bMALE, BOOL bFEMALE )
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

void CPageQuestBasic::OnBnClickedButtonAllMale()
{
	UpdateClass( TRUE, FALSE );
}

void CPageQuestBasic::OnBnClickedButtonAllFemale()
{
	UpdateClass( FALSE, TRUE );
}

void CPageQuestBasic::OnBnClickedButtonAll()
{
	UpdateClass( TRUE, TRUE );
}

void CPageQuestBasic::OnBnClickedButtonNothing()
{
	UpdateClass( FALSE, FALSE );
}

void CPageQuestBasic::OnBnClickedButtonMapDelStart2()
{
	SNATIVEID sID( NATIVEID_NULL() );
	SetWin_Num_int( this, IDC_EDIT_MAP_X_START, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_MAP_Y_START, sID.wSubID );
}

void CPageQuestBasic::OnBnClickedButtonMapDelEnd2()
{
	SNATIVEID sID( NATIVEID_NULL() );
	SetWin_Num_int( this, IDC_EDIT_MAP_X_END, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_MAP_Y_END, sID.wSubID );
}

void CPageQuestBasic::OnBnClickedButtonMapDelFail2()
{
	SNATIVEID sID( NATIVEID_NULL() );
	SetWin_Num_int( this, IDC_EDIT_MAP_X_FAIL, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_MAP_Y_FAIL, sID.wSubID );
}
