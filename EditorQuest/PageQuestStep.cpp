// PageQuestStep.cpp : implementation file
//

#include "stdafx.h"
#include "EditorQuest.h"
#include "EditorQuestDlg.h"
#include "PageQuestStep.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "DlgCrow.h"
#include "DlgMap.h"
#include "DlgItem.h"
#include "DlgItemEdit.h"
#include "DlgMobGen.h"

// CPageQuestStep dialog

IMPLEMENT_DYNAMIC(CPageQuestStep, CPropertyPage)

CPageQuestStep::CPageQuestStep( LOGFONT logfont )
	: CPropertyPage(CPageQuestStep::IDD)
	, m_pFont ( NULL )
	, m_dwCurStep(UINT_MAX)
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pQuest = NULL;
	m_DummyQuest.pQUEST = new GLQUEST;
}

CPageQuestStep::~CPageQuestStep()
{
	SAFE_DELETE ( m_pFont );
	SAFE_DELETE ( m_DummyQuest.pQUEST );
}

void CPageQuestStep::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_STEP, m_ListStep);
	DDX_Control(pDX, IDC_LIST_ITEM_GAIN, m_ListItemGain);
	DDX_Control(pDX, IDC_LIST_ITEM_REMOVE, m_ListItemRemove);
	DDX_Control(pDX, IDC_LIST_ITEM_GEN, m_ListItemGenerate);
}


BEGIN_MESSAGE_MAP(CPageQuestStep, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_BACK, OnBnClickedButtonBack)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_STEP_ADD, OnBnClickedButtonStepAdd)
	ON_BN_CLICKED(IDC_BUTTON_STEP_DEL, OnBnClickedButtonStepDel)
	ON_BN_CLICKED(IDC_BUTTON_STEP_MOVEUP, OnBnClickedButtonStepMoveup)
	ON_BN_CLICKED(IDC_BUTTON_STEP_MOVEDOWN, OnBnClickedButtonStepMovedown)
	ON_LBN_SELCHANGE(IDC_LIST_STEP, OnLbnSelchangeListStep)
	ON_LBN_DBLCLK(IDC_LIST_STEP, OnLbnDblclkListStep)
	ON_BN_CLICKED(IDC_BUTTON_TALKNPC, OnBnClickedButtonTalknpc)
	ON_BN_CLICKED(IDC_BUTTON_TALKNPC_DEL, OnBnClickedButtonTalknpcDel)
	ON_BN_CLICKED(IDC_BUTTON_GUARDNPC, OnBnClickedButtonGuardnpc)
	ON_BN_CLICKED(IDC_BUTTON_GUARDNPC_DEL, OnBnClickedButtonGuardnpcDel)
	ON_BN_CLICKED(IDC_BUTTON_DIEMOB, OnBnClickedButtonDiemob)
	ON_BN_CLICKED(IDC_BUTTON_DIEMOB_DEL, OnBnClickedButtonDiemobDel)
	ON_BN_CLICKED(IDC_BUTTON_REACH_MAP, OnBnClickedButtonReachMap)
	ON_BN_CLICKED(IDC_BUTTON_REACH_MAP_DEL, OnBnClickedButtonReachMapDel)
	ON_BN_CLICKED(IDC_BUTTON_GUARD_MAP, OnBnClickedButtonGuardMap)
	ON_BN_CLICKED(IDC_BUTTON_GUARD_MAP_DEL, OnBnClickedButtonGuardMapDel)
	ON_BN_CLICKED(IDC_STEP_MAP_EDIT_BUTTON, OnBnClickedStepMapEditButton)
	ON_BN_CLICKED(IDC_STEP_MAP_EDIT_DEL, OnBnClickedStepMapEditDel)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_ADD_GAIN, OnBnClickedButtonItemAddGain)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_DEL_GAIN, OnBnClickedButtonItemDelGain)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_EDIT_GAIN, OnBnClickedButtonItemEditGain)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_INSERT_GAIN, OnBnClickedButtonItemInsertGain)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_ADD_REMOVE, OnBnClickedButtonItemAddRemove)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_DEL_REMOVE, OnBnClickedButtonItemDelRemove)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_INSERT_REMOVE, OnBnClickedButtonItemInsertRemove)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_ADD_GEN, OnBnClickedButtonItemAddGen)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_DEL_GEN, OnBnClickedButtonItemDelGen)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_EDIT_GEN, OnBnClickedButtonItemEditGen)
	ON_BN_CLICKED(IDC_BUTTON_ITEM_INSERT_GEN, OnBnClickedButtonItemInsertGen)
	ON_BN_CLICKED(IDC_BUTTON_STEP_OK, OnBnClickedButtonStepOk)
	ON_BN_CLICKED(IDC_BUTTON_STEP_EDIT, OnBnClickedButtonStepEdit)
	ON_BN_CLICKED(IDC_BUTTON_STEP_CANCEL, OnBnClickedButtonStepCancel)
END_MESSAGE_MAP()


// CPageQuestStep message handlers
BOOL CPageQuestStep::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_bDlgInit = TRUE;

	DataDefault();
	DataShow();

	return TRUE;
}

BOOL CPageQuestStep::QuestSet( GLQuestMan::SNODE* pQUEST )
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

void CPageQuestStep::QuestClear()
{
	m_pQuest = NULL;
}

void CPageQuestStep::QuestSave()
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


void CPageQuestStep::OnBnClickedButtonBack()
{
	if ( DataSave() )
	{
		GLQuestMan::GetInstance().DoModified ();
		QuestSave();
		m_pSheetTab->ActiveQuestBasicPage( QUEST_STEP_PAGE, m_pQuest );
		m_pQuest = NULL;
	}
}

void CPageQuestStep::OnBnClickedButtonCancel()
{
	m_pQuest = NULL;
	m_pSheetTab->ActiveQuestTreePage ();
}

void CPageQuestStep::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		QuestSave();
		m_pQuest = NULL;
		m_pSheetTab->ActiveQuestTreePage ();
	}
}

void CPageQuestStep::OnBnClickedButtonStepAdd()
{
	if ( !m_DummyQuest.pQUEST )	return;

	StepSave();
	m_dwCurStep = UINT_MAX;
	m_DummyQuest.pQUEST->m_vecProgStep.push_back ( GLQUEST_STEP() );
	DataShow();
	DWORD dwCurSel = (DWORD) m_DummyQuest.pQUEST->m_vecProgStep.size()-1;
	m_ListStep.SetCurSel(dwCurSel);
	StepShow();
}

void CPageQuestStep::OnBnClickedButtonStepEdit()
{
	m_dwCurStep = UINT_MAX;
	DataSave ();
	StepShow ();
}

void CPageQuestStep::OnBnClickedButtonStepDel()
{
	if ( !m_DummyQuest.pQUEST )	return;

	DataSave();

	int nIndex = m_ListStep.GetCurSel();
	if ( nIndex==LB_ERR )
		return;

	DWORD dwStep = (DWORD) m_ListStep.GetItemData ( nIndex );
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= dwStep )	return;

	StepSave();
	m_dwCurStep = UINT_MAX;

	m_ListStep.SetCurSel(LB_ERR);

	int nReturn = MessageBox ( "Do you want to delete Current Quest Step?", "Caution", MB_YESNO );
	if ( nReturn==IDYES )
	{
		GLQUEST::VECQSTEP_ITER iter = m_DummyQuest.pQUEST->m_vecProgStep.begin()+dwStep;
		m_DummyQuest.pQUEST->m_vecProgStep.erase ( iter, iter+1 );
	}

	DataShow();
}

void CPageQuestStep::OnBnClickedButtonStepMoveup()
{
	if ( !m_DummyQuest.pQUEST )	return;

	DataSave ();

	int nIndex = m_ListStep.GetCurSel();
	if ( nIndex==LB_ERR )
		return;

	DWORD dwID = (DWORD) m_ListStep.GetItemData ( nIndex );
	dwID = m_DummyQuest.pQUEST->ToUpStep(dwID);
	m_ListStep.SetCurSel ( dwID );
	DataShow();
	StepShow();
	m_ListStep.SetCurSel ( dwID );
}

void CPageQuestStep::OnBnClickedButtonStepMovedown()
{
	if ( !m_DummyQuest.pQUEST )	return;

	DataSave ();

	int nIndex = m_ListStep.GetCurSel();
	if ( nIndex==LB_ERR )
		return;

	DWORD dwID = (DWORD) m_ListStep.GetItemData ( nIndex );
	dwID = m_DummyQuest.pQUEST->ToDownStep(dwID);
	m_ListStep.SetCurSel ( dwID );
	DataShow ();
	StepShow ();
	m_ListStep.SetCurSel ( dwID );
}

void CPageQuestStep::OnLbnSelchangeListStep()
{
	// TODO: Add your control notification handler code here
}

void CPageQuestStep::OnLbnDblclkListStep()
{
	m_dwCurStep = UINT_MAX;
	DataSave ();
	StepShow ();
}

void CPageQuestStep::CheckCrowNpctalk()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_TALKNPC_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_TALKNPC_SID );
	std::string strNAME = "Unknown Crow";
	SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData( wMID, wSID );
	if ( pCROW )	strNAME = pCROW->GetName();
	SetWin_Text( this, IDC_EDIT_TALKNPC_NAME, strNAME.c_str() );
}

void CPageQuestStep::OnBnClickedButtonTalknpc()
{
	CDlgCrow dlg;
	if( dlg.DoModal() == IDOK )
	{
		SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData( dlg.m_SELECTED );
		if ( pCROW )
		{
			SetWin_Num_int( this, IDC_EDIT_TALKNPC_MID, pCROW->m_sBasic.sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_TALKNPC_SID, pCROW->m_sBasic.sNativeID.wSubID );
			CheckCrowNpctalk();
		}
	}
}

void CPageQuestStep::OnBnClickedButtonTalknpcDel()
{
	SetWin_Num_int( this, IDC_EDIT_TALKNPC_MID, SNATIVEID(false).wMainID );
	SetWin_Num_int( this, IDC_EDIT_TALKNPC_SID, SNATIVEID(false).wSubID );
	CheckCrowNpctalk();
}

void CPageQuestStep::CheckCrowGuardNPC()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_GUARDNPC_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_GUARDNPC_SID );
	std::string strNAME = "Unknown Crow";
	SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData( wMID, wSID );
	if ( pCROW )	strNAME = pCROW->GetName();
	SetWin_Text( this, IDC_EDIT_GUARDNPC_NAME, strNAME.c_str() );
}

void CPageQuestStep::OnBnClickedButtonGuardnpc()
{
	CDlgCrow dlg;
	if( dlg.DoModal() == IDOK )
	{
		SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData( dlg.m_SELECTED );
		if ( pCROW )
		{
			SetWin_Num_int( this, IDC_EDIT_GUARDNPC_MID, pCROW->m_sBasic.sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_GUARDNPC_SID, pCROW->m_sBasic.sNativeID.wSubID );
			CheckCrowGuardNPC();
		}
	}
}

void CPageQuestStep::OnBnClickedButtonGuardnpcDel()
{
	SetWin_Num_int( this, IDC_EDIT_GUARDNPC_MID, SNATIVEID(false).wMainID );
	SetWin_Num_int( this, IDC_EDIT_GUARDNPC_SID, SNATIVEID(false).wSubID );
	CheckCrowGuardNPC();
}

void CPageQuestStep::CheckCrowKillMob()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_DIEMOB_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_DIEMOB_SID );
	std::string strNAME = "Unknown Crow";
	SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData( wMID, wSID );
	if ( pCROW )	strNAME = pCROW->GetName();
	SetWin_Text( this, IDC_EDIT_DIEMOB_NAME, strNAME.c_str() );
}

void CPageQuestStep::OnBnClickedButtonDiemob()
{
	CDlgCrow dlg;
	if( dlg.DoModal() == IDOK )
	{
		SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData( dlg.m_SELECTED );
		if ( pCROW )
		{
			SetWin_Num_int( this, IDC_EDIT_DIEMOB_MID, pCROW->m_sBasic.sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_DIEMOB_SID, pCROW->m_sBasic.sNativeID.wSubID );
			CheckCrowKillMob();
		}
	}
}

void CPageQuestStep::OnBnClickedButtonDiemobDel()
{
	SetWin_Num_int( this, IDC_EDIT_DIEMOB_MID, SNATIVEID(false).wMainID );
	SetWin_Num_int( this, IDC_EDIT_DIEMOB_SID, SNATIVEID(false).wSubID );
	CheckCrowKillMob();
}

void CPageQuestStep::CheckMapReachMap()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_REACH_MAP_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_REACH_MAP_SID );
	std::string strNAME = "Unknown Map";
	SMAPNODE* pNODE = CEditorQuestDlg::m_sMapList.FindMapNode ( SNATIVEID( wMID, wSID ) );
	if ( pNODE )	strNAME = pNODE->strMapName.c_str();
	SetWin_Text( this, IDC_EDIT_REACH_MAP_NAME, strNAME.c_str() );
}

void CPageQuestStep::OnBnClickedButtonReachMap()
{
	CDlgMap dlg;
	if( dlg.DoModal() == IDOK )
	{
		SMAPNODE* pNODE = CEditorQuestDlg::m_sMapList.FindMapNode ( dlg.m_SELECTED );
		if ( pNODE )
		{
			SetWin_Num_int( this, IDC_EDIT_REACH_MAP_MID, pNODE->sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_REACH_MAP_SID, pNODE->sNativeID.wSubID );
			CheckMapReachMap();
		}
	}
}

void CPageQuestStep::OnBnClickedButtonReachMapDel()
{
	SetWin_Num_int( this, IDC_EDIT_REACH_MAP_MID, SNATIVEID(false).wMainID );
	SetWin_Num_int( this, IDC_EDIT_REACH_MAP_SID, SNATIVEID(false).wSubID );
	CheckMapReachMap();
}

void CPageQuestStep::CheckMapDefendMap()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_GUARD_MAP_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_GUARD_MAP_SID );
	std::string strNAME = "Unknown Map";
	SMAPNODE* pNODE = CEditorQuestDlg::m_sMapList.FindMapNode ( SNATIVEID( wMID, wSID ) );
	if ( pNODE )	strNAME = pNODE->strMapName.c_str();
	SetWin_Text( this, IDC_EDIT_GUARD_MAP_NAME, strNAME.c_str() );
}

void CPageQuestStep::OnBnClickedButtonGuardMap()
{
	CDlgMap dlg;
	if( dlg.DoModal() == IDOK )
	{
		SMAPNODE* pNODE = CEditorQuestDlg::m_sMapList.FindMapNode ( dlg.m_SELECTED );
		if ( pNODE )
		{
			SetWin_Num_int( this, IDC_EDIT_GUARD_MAP_MID, pNODE->sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_GUARD_MAP_SID, pNODE->sNativeID.wSubID );
			CheckMapDefendMap();
		}
	}
}

void CPageQuestStep::OnBnClickedButtonGuardMapDel()
{
	SetWin_Num_int( this, IDC_EDIT_GUARD_MAP_MID, SNATIVEID(false).wMainID );
	SetWin_Num_int( this, IDC_EDIT_GUARD_MAP_SID, SNATIVEID(false).wSubID );
	CheckMapDefendMap();
}

void CPageQuestStep::CheckMapMoveMap()
{
	WORD wMID = GetWin_Num_int( this, IDC_STEP_MAP_MID_EDIT );
	WORD wSID = GetWin_Num_int( this, IDC_STEP_MAP_SID_EDIT );
	std::string strNAME = "Unknown Map";
	SMAPNODE* pNODE = CEditorQuestDlg::m_sMapList.FindMapNode ( SNATIVEID( wMID, wSID ) );
	if ( pNODE )	strNAME = pNODE->strMapName.c_str();
	SetWin_Text( this, IDC_STEP_MAP_NAME, strNAME.c_str() );
}

void CPageQuestStep::OnBnClickedStepMapEditButton()
{
	CDlgMap dlg;
	if( dlg.DoModal() == IDOK )
	{
		SMAPNODE* pNODE = CEditorQuestDlg::m_sMapList.FindMapNode ( dlg.m_SELECTED );
		if ( pNODE )
		{
			SetWin_Num_int( this, IDC_STEP_MAP_MID_EDIT, pNODE->sNativeID.wMainID );
			SetWin_Num_int( this, IDC_STEP_MAP_SID_EDIT, pNODE->sNativeID.wSubID );
			CheckMapMoveMap();
		}
	}
}

void CPageQuestStep::OnBnClickedStepMapEditDel()
{
	SetWin_Num_int( this, IDC_STEP_MAP_MID_EDIT, SNATIVEID(false).wMainID );
	SetWin_Num_int( this, IDC_STEP_MAP_SID_EDIT, SNATIVEID(false).wSubID );
	CheckMapMoveMap();
}

void CPageQuestStep::ShowItemGain()
{
	m_ListItemGain.ResetContent();

	if ( m_dwCurStep==UINT_MAX )	return;
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= m_dwCurStep )	return;

	m_ListItemGain.SetRedraw( FALSE );

	GLQUEST_STEP &sSTEP = m_DummyQuest.pQUEST->m_vecProgStep[m_dwCurStep];
	INVENQARRAY_ITER iter = sSTEP.m_vecGIFT_QITEM.begin();
	INVENQARRAY_ITER iter_end = sSTEP.m_vecGIFT_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		const SITEMCUSTOM &sDATA = (*iter);

		CString strNAME = "Unknown Item";
		SITEM* pITEM = GLItemMan::GetInstance().GetItem ( sDATA.sNativeID );
		if ( pITEM )	
			strNAME.Format( "%d/%d %s", pITEM->sBasicOp.sNativeID.wMainID, pITEM->sBasicOp.sNativeID.wSubID, pITEM->GetName() );

		int nIndex = m_ListItemGain.AddString ( strNAME.GetString() );
		DWORD dwData = DWORD(iter-sSTEP.m_vecGIFT_QITEM.begin());
		m_ListItemGain.SetItemData ( nIndex, dwData );
	}

	m_ListItemGain.SetRedraw( TRUE );

	SetWin_Num_int( this, IDC_EDIT_INSERT_MID_GAIN, SNATIVEID( false ).wMainID );
	SetWin_Num_int( this, IDC_EDIT_INSERT_SID_GAIN, SNATIVEID( false ).wSubID );
}

void CPageQuestStep::ShowItemRemove()
{
	m_ListItemRemove.ResetContent();

	if ( m_dwCurStep==UINT_MAX )	return;
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= m_dwCurStep )	return;

	m_ListItemRemove.SetRedraw( FALSE );

	GLQUEST_STEP &sSTEP = m_DummyQuest.pQUEST->m_vecProgStep[m_dwCurStep];
	DWQARRAY_ITER iter = sSTEP.m_vecRESET_QITEM.begin();
	DWQARRAY_ITER iter_end = sSTEP.m_vecRESET_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		const DWORD &dwDATA = (*iter);

		CString strNAME = "Unknown Item";
		SITEM* pITEM = GLItemMan::GetInstance().GetItem ( SNATIVEID(dwDATA) );
		if ( pITEM )	
			strNAME.Format( "%d/%d %s", pITEM->sBasicOp.sNativeID.wMainID, pITEM->sBasicOp.sNativeID.wSubID, pITEM->GetName() );

		int nIndex = m_ListItemRemove.AddString ( strNAME.GetString() );
		DWORD dwData = DWORD(iter-sSTEP.m_vecRESET_QITEM.begin());
		m_ListItemRemove.SetItemData ( nIndex, dwData );
	}

	m_ListItemRemove.SetRedraw( TRUE );

	SetWin_Num_int( this, IDC_EDIT_INSERT_MID_DEL, SNATIVEID( false ).wMainID );
	SetWin_Num_int( this, IDC_EDIT_INSERT_SID_DEL, SNATIVEID( false ).wSubID );
}

void CPageQuestStep::ShowItemGen()
{
	m_ListItemGenerate.ResetContent();

	if ( m_dwCurStep==UINT_MAX )	return;
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= m_dwCurStep )	return;

	m_ListItemGenerate.SetRedraw( FALSE );

	GLQUEST_STEP &sSTEP = m_DummyQuest.pQUEST->m_vecProgStep[m_dwCurStep];
	
	GENMOBITEMARRAY_ITER iter = sSTEP.m_vecMOBGEN_QITEM.begin();
	GENMOBITEMARRAY_ITER iter_end = sSTEP.m_vecMOBGEN_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		const SGENQUESTITEM &sCUSTION = (*iter);

		CString strNAME = "Unknown Item";
		SITEM* pITEM = GLItemMan::GetInstance().GetItem ( sCUSTION.sNID );
		if ( pITEM )
			strNAME.Format( "%d/%d %s", pITEM->sBasicOp.sNativeID.wMainID, pITEM->sBasicOp.sNativeID.wSubID, pITEM->GetName() );

		int nIndex = m_ListItemGenerate.AddString ( strNAME.GetString() );
		DWORD dwData = DWORD(iter-sSTEP.m_vecMOBGEN_QITEM.begin());
		m_ListItemGenerate.SetItemData ( nIndex, dwData );
	}

	m_ListItemGenerate.SetRedraw( TRUE );

	SetWin_Num_int( this, IDC_EDIT_INSERT_MID_GEN, SNATIVEID( false ).wMainID );
	SetWin_Num_int( this, IDC_EDIT_INSERT_SID_GEN, SNATIVEID( false ).wSubID );
}

void CPageQuestStep::OnBnClickedButtonItemAddGain()
{
	if ( m_dwCurStep==UINT_MAX )	return;
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= m_dwCurStep )	return;
	GLQUEST_STEP &sSTEP = m_DummyQuest.pQUEST->m_vecProgStep[m_dwCurStep];

	CDlgItem dlg;
	if( dlg.DoModal() == IDOK )
	{
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( dlg.m_SELECTED );
		if ( pITEM )
		{
			SITEMCUSTOM sCUSTOM;
			sCUSTOM.sNativeID = dlg.m_SELECTED;
			sSTEP.m_vecGIFT_QITEM.push_back ( sCUSTOM );
			ShowItemGain();
		}
	}
}

void CPageQuestStep::OnBnClickedButtonItemDelGain()
{
	if ( m_dwCurStep==UINT_MAX )	return;
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= m_dwCurStep )	return;
	GLQUEST_STEP &sSTEP = m_DummyQuest.pQUEST->m_vecProgStep[m_dwCurStep];

	int nIndex = m_ListItemGain.GetCurSel();
	if ( LB_ERR==nIndex )	return;
	DWORD dwData = (DWORD) m_ListItemGain.GetItemData ( nIndex );

	const SITEMCUSTOM sCUSTOM = sSTEP.m_vecGIFT_QITEM[dwData];
	CString strQuestion = "Delete Invalid Item ( ID not found)?";
	SITEM* pITEM = GLItemMan::GetInstance().GetItem ( sCUSTOM.sNativeID );
	if ( pITEM )	
		strQuestion.Format( "Delete Item %d/%d %s?", pITEM->sBasicOp.sNativeID.wMainID, pITEM->sBasicOp.sNativeID.wSubID,pITEM->GetName() );

	if ( MessageBox ( strQuestion, "Question", MB_YESNO ) == IDYES )
	{
		sSTEP.GIFTITEM_ERASE ( dwData );
		ShowItemGain();
	}
}

void CPageQuestStep::OnBnClickedButtonItemEditGain()
{
	if ( m_dwCurStep==UINT_MAX )	return;
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= m_dwCurStep )	return;
	GLQUEST_STEP &sSTEP = m_DummyQuest.pQUEST->m_vecProgStep[m_dwCurStep];

	int nIndex = m_ListItemGain.GetCurSel();
	if ( LB_ERR==nIndex )	return;

	DWORD dwData = (DWORD) m_ListItemGain.GetItemData ( nIndex );
	SITEMCUSTOM &sCUSTOM = sSTEP.m_vecGIFT_QITEM[dwData];

	CDlgItemEdit dlg;
	dlg.m_sITEMCUSTOM = sCUSTOM;

	if ( dlg.DoModal() == IDOK )
	{
		sCUSTOM = dlg.m_sITEMCUSTOM;
	}
}

void CPageQuestStep::OnBnClickedButtonItemInsertGain()
{
	if ( m_dwCurStep==UINT_MAX )	return;
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= m_dwCurStep )	return;
	GLQUEST_STEP &sSTEP = m_DummyQuest.pQUEST->m_vecProgStep[m_dwCurStep];

	WORD wMID = GetWin_Num_int( this, IDC_EDIT_INSERT_MID_GAIN );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_INSERT_SID_GAIN );
	SITEM* pITEM = GLItemMan::GetInstance().GetItem( wMID, wSID );
	if ( pITEM )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.sNativeID = pITEM->sBasicOp.sNativeID;
		sSTEP.m_vecGIFT_QITEM.push_back ( sCUSTOM );
		ShowItemGain();
	}
}

void CPageQuestStep::OnBnClickedButtonItemAddRemove()
{
	if ( m_dwCurStep==UINT_MAX )	return;
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= m_dwCurStep )	return;
	GLQUEST_STEP &sSTEP = m_DummyQuest.pQUEST->m_vecProgStep[m_dwCurStep];

	CDlgItem dlg;
	if( dlg.DoModal() == IDOK )
	{
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( dlg.m_SELECTED );
		if ( pITEM )
		{
			sSTEP.m_vecRESET_QITEM.push_back ( dlg.m_SELECTED.dwID );
			ShowItemRemove();
		}
	}
}

void CPageQuestStep::OnBnClickedButtonItemDelRemove()
{
	if ( m_dwCurStep==UINT_MAX )	return;
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= m_dwCurStep )	return;
	GLQUEST_STEP &sSTEP = m_DummyQuest.pQUEST->m_vecProgStep[m_dwCurStep];

	int nIndex = m_ListItemRemove.GetCurSel();
	if ( LB_ERR==nIndex )	return;

	DWORD dwData = (DWORD) m_ListItemRemove.GetItemData ( nIndex );
	SNATIVEID sRESET_NID(sSTEP.m_vecRESET_QITEM[dwData]);

	CString strQuestion = "Delete Invalid Item ( ID not found)?";
	SITEM* pITEM = GLItemMan::GetInstance().GetItem ( sRESET_NID );
	if ( pITEM )	
		strQuestion.Format( "Delete Item %d/%d %s?", pITEM->sBasicOp.sNativeID.wMainID, pITEM->sBasicOp.sNativeID.wSubID, pITEM->GetName() );

	if ( MessageBox ( strQuestion, "Question", MB_YESNO ) == IDYES )
	{
		sSTEP.RESETITEM_ERASE ( dwData );
		ShowItemRemove();
	}
}

void CPageQuestStep::OnBnClickedButtonItemInsertRemove()
{
	if ( m_dwCurStep==UINT_MAX )	return;
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= m_dwCurStep )	return;
	GLQUEST_STEP &sSTEP = m_DummyQuest.pQUEST->m_vecProgStep[m_dwCurStep];

	WORD wMID = GetWin_Num_int( this, IDC_EDIT_INSERT_MID_DEL );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_INSERT_SID_DEL );
	SITEM* pITEM = GLItemMan::GetInstance().GetItem( wMID, wSID );
	if ( pITEM )
	{
		sSTEP.m_vecRESET_QITEM.push_back ( pITEM->sBasicOp.sNativeID.dwID );
		ShowItemRemove();
	}
}

void CPageQuestStep::OnBnClickedButtonItemAddGen()
{
	if ( m_dwCurStep==UINT_MAX )	return;
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= m_dwCurStep )	return;
	GLQUEST_STEP &sSTEP = m_DummyQuest.pQUEST->m_vecProgStep[m_dwCurStep];

	CDlgItem dlg;
	if( dlg.DoModal() == IDOK )
	{
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( dlg.m_SELECTED );
		if ( pITEM )
		{
			SGENQUESTITEM sGENITEM;
			sGENITEM.sNID = dlg.m_SELECTED;
			if ( pITEM->ISPILE() )	sGENITEM.wNUM = pITEM->GETAPPLYNUM();
			sSTEP.m_vecMOBGEN_QITEM.push_back(sGENITEM);
			ShowItemGen ();
		}
	}
}

void CPageQuestStep::OnBnClickedButtonItemDelGen()
{
	if ( m_dwCurStep==UINT_MAX )	return;
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= m_dwCurStep )	return;
	GLQUEST_STEP &sSTEP = m_DummyQuest.pQUEST->m_vecProgStep[m_dwCurStep];

	int nIndex = m_ListItemGenerate.GetCurSel();
	if ( LB_ERR==nIndex )	return;

	DWORD dwData = (DWORD) m_ListItemGenerate.GetItemData ( nIndex );
	SGENQUESTITEM &sGENITEM = sSTEP.m_vecMOBGEN_QITEM[dwData];

	CString strQuestion = "Delete Invalid Item ( ID not found)?";
	SITEM* pITEM = GLItemMan::GetInstance().GetItem ( sGENITEM.sNID );
	if ( pITEM )	
		strQuestion.Format( "Delete Item %d/%d %s?", pITEM->sBasicOp.sNativeID.wMainID, pITEM->sBasicOp.sNativeID.wSubID, pITEM->GetName() );

	if ( MessageBox ( strQuestion, "Question", MB_YESNO ) == IDYES )
	{
		sSTEP.MOBGENITEM_ERASE ( dwData );
		ShowItemGen ();
	}
}

void CPageQuestStep::OnBnClickedButtonItemEditGen()
{
	if ( m_dwCurStep==UINT_MAX )	return;
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= m_dwCurStep )	return;
	GLQUEST_STEP &sSTEP = m_DummyQuest.pQUEST->m_vecProgStep[m_dwCurStep];

	int nIndex = m_ListItemGenerate.GetCurSel();
	if ( LB_ERR==nIndex )	return;

	DWORD dwData = (DWORD) m_ListItemGenerate.GetItemData ( nIndex );
	SGENQUESTITEM &sGENITEM = sSTEP.m_vecMOBGEN_QITEM[dwData];

	CDlgMobGen dlg;
	dlg.m_sGENQITEM = sGENITEM;
	if ( dlg.DoModal() == IDOK )
	{
		sGENITEM = dlg.m_sGENQITEM;
		ShowItemGen ();
	}
}

void CPageQuestStep::OnBnClickedButtonItemInsertGen()
{
	if ( m_dwCurStep==UINT_MAX )	return;
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= m_dwCurStep )	return;
	GLQUEST_STEP &sSTEP = m_DummyQuest.pQUEST->m_vecProgStep[m_dwCurStep];

	WORD wMID = GetWin_Num_int( this, IDC_EDIT_INSERT_MID_GEN );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_INSERT_SID_GEN );
	SITEM* pITEM = GLItemMan::GetInstance().GetItem( wMID, wSID );
	if ( pITEM )
	{
		SGENQUESTITEM sGENITEM;
		sGENITEM.sNID = pITEM->sBasicOp.sNativeID;
		if ( pITEM->ISPILE() )	sGENITEM.wNUM = pITEM->GETAPPLYNUM();
		sSTEP.m_vecMOBGEN_QITEM.push_back(sGENITEM);
		ShowItemGen ();
	}
}

void CPageQuestStep::OnBnClickedButtonStepOk()
{
	if ( m_dwCurStep==UINT_MAX )	return;
	if ( m_DummyQuest.pQUEST->m_vecProgStep.size() <= m_dwCurStep )	return;

	GLQUEST_STEP &sSTEP = m_DummyQuest.pQUEST->m_vecProgStep[m_dwCurStep];

	GLQUEST_PROGRESS_MAP checkMap;
	checkMap.nidMAP.wMainID = GetWin_Num_int ( this, IDC_STEP_MAP_MID_EDIT );
	checkMap.nidMAP.wSubID = GetWin_Num_int ( this, IDC_STEP_MAP_SID_EDIT );
	checkMap.wPosX = GetWin_Num_int ( this, IDC_STEP_MAP_XPOS_EDIT );
	checkMap.wPosY = GetWin_Num_int ( this, IDC_STEP_MAP_YPOS_EDIT );
	checkMap.dwGateID = GetWin_Num_int ( this, IDC_STEP_MAP_GATE_EDIT );

	if( checkMap.nidMAP.wMainID != USHRT_MAX &&	checkMap.nidMAP.wSubID  != USHRT_MAX )
	{
		if( (checkMap.wPosX == USHRT_MAX || checkMap.wPosY == USHRT_MAX ) && checkMap.dwGateID == 0 )
		{
			MessageBox( "Please Modify Map Position Or Gate" );
			return;
		}

		if( checkMap.wPosX != USHRT_MAX && checkMap.wPosY != USHRT_MAX && checkMap.dwGateID != 0 )
		{
			MessageBox( "Map Position And Gate Both Modified! \n Please Modify only Map Location Or Gate \n Not Both" );
			return;
		}

		if( checkMap.nidMAP.wMainID  != UINT_MAX && checkMap.nidMAP.wSubID  != UINT_MAX && (checkMap.wPosX != UINT_MAX && checkMap.wPosY != UINT_MAX || checkMap.dwGateID != 0 ) )
			sSTEP.m_stepMoveMap = checkMap;
	}

	StepSave();
	m_dwCurStep = UINT_MAX;
	m_ListStep.SetCurSel(LB_ERR);
	DataShow ();
}

void CPageQuestStep::OnBnClickedButtonStepCancel()
{
	int nIndex = m_ListStep.GetCurSel();

	m_dwCurStep = UINT_MAX;
	m_ListStep.SetCurSel(LB_ERR);
	DataShow ();

	if ( nIndex != LB_ERR ){
		m_ListStep.SetCurSel(nIndex);
	}
}
