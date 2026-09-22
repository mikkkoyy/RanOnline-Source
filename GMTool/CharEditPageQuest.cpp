// CharEditPageQuest.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "CharEditPageQuest.h"

#include "CharEditTab.h"

#include "GLCharData.h"
#include "GLQuest.h"
#include "GLQuestMan.h"
#include "DlgQuest.h"
#include "HLibDataConvert.h"

// CCharEditPageQuest dialog

IMPLEMENT_DYNAMIC(CCharEditPageQuest, CPropertyPage)

CCharEditPageQuest::CCharEditPageQuest(LOGFONT logfont, SCHARDATA2* pData)
	: CPropertyPage(CCharEditPageQuest::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CCharEditPageQuest::~CCharEditPageQuest()
{
	SAFE_DELETE ( m_pFont );
}

void CCharEditPageQuest::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHAREDIT_QUEST_LIST_PROC, m_ListProc);
	DDX_Control(pDX, IDC_CHAREDIT_QUEST_LIST_END, m_ListEnd);
}


BEGIN_MESSAGE_MAP(CCharEditPageQuest, CPropertyPage)
	ON_BN_CLICKED(IDC_CHAREDIT_QUEST_BUTTON_PROC_INSERT, OnBnClickedChareditQuestButtonProcInsert)
	ON_BN_CLICKED(IDC_CHAREDIT_QUEST_BUTTON_PROC_COMPLETE, OnBnClickedChareditQuestButtonProcComplete)
	ON_BN_CLICKED(IDC_CHAREDIT_QUEST_BUTTON_PROC_DELETE, OnBnClickedChareditQuestButtonProcDelete)
	ON_BN_CLICKED(IDC_CHAREDIT_QUEST_BUTTON_PROC_CLEAR, OnBnClickedChareditQuestButtonProcClear)
	ON_BN_CLICKED(IDC_CHAREDIT_QUEST_BUTTON_END_DELETE, OnBnClickedChareditQuestButtonEndDelete)
	ON_BN_CLICKED(IDC_CHAREDIT_QUEST_BUTTON_END_CLEAR, OnBnClickedChareditQuestButtonEndClear)
END_MESSAGE_MAP()


// CCharEditPageQuest message handlers
BOOL CCharEditPageQuest::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_ListProc.SetExtendedStyle ( m_ListProc.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_ListProc.GetClientRect ( &rectCtrl );
	m_ListProc.InsertColumn( 0, "Quest", LVCFMT_LEFT, ( 95* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	m_ListEnd.SetExtendedStyle ( m_ListEnd.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES  );
	m_ListEnd.GetClientRect ( &rectCtrl );
	m_ListEnd.InsertColumn( 0, "Quest", LVCFMT_LEFT, ( 95* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	UpdateQuestProc();
	UpdateQuestEnd();

	return TRUE;  
}

void CCharEditPageQuest::UpdateQuestProc()
{
	m_ListProc.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_ListProc.SetRedraw( FALSE );

	GLQuestPlay::MAPQUEST& mapQuest = m_pData->m_cQuestPlay.GetQuestProc();
	GLQuestPlay::MAPQUEST_ITER iter = mapQuest.begin();

	for ( int i=0 ; iter!=mapQuest.end(); ++iter, ++i )
	{
		const GLQUESTPROG *pPROG = (*iter).second;
		
		std::string strNAME = "Unknown Quest";
		GLQUEST *pQUEST = GLQuestMan::GetInstance().Find ( pPROG->m_sNID.dwID );
		if ( pQUEST ){
			strNAME = pQUEST->GetTITLE();
		}

		m_ListProc.InsertItem( i, _HLIB::cstringformat( "[%04d] %s", pPROG->m_sNID.dwID, strNAME.c_str() ).GetString() );
		m_ListProc.SetItemData( i, pPROG->m_sNID.dwID );
	}

	m_ListProc.SetRedraw( TRUE );
}

void CCharEditPageQuest::UpdateQuestEnd()
{
	m_ListEnd.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_ListEnd.SetRedraw( FALSE );

	GLQuestPlay::MAPQUEST& mapQuest = m_pData->m_cQuestPlay.GetQuestEnd();
	GLQuestPlay::MAPQUEST_ITER iter = mapQuest.begin();

	for ( int i=0 ; iter!=mapQuest.end(); ++iter, ++i )
	{
		const GLQUESTPROG *pPROG = (*iter).second;

		std::string strNAME = "Unknown Quest";
		GLQUEST *pQUEST = GLQuestMan::GetInstance().Find ( pPROG->m_sNID.dwID );
		if ( pQUEST ){
			strNAME = pQUEST->GetTITLE();
		}

		m_ListEnd.InsertItem( i, _HLIB::cstringformat( "[%04d] %s", pPROG->m_sNID.dwID, strNAME.c_str() ).GetString() );
		m_ListEnd.SetItemData( i, pPROG->m_sNID.dwID );
	}

	m_ListEnd.SetRedraw( TRUE );
}

void CCharEditPageQuest::OnBnClickedChareditQuestButtonProcInsert()
{
	if ( !m_pData ){
		return;
	}

	CDlgQuest dlg( m_pData->m_emClass, m_pData->m_wSchool, this );
	if( dlg.DoModal() == IDOK ){
		DWORD dwID = dlg.m_dwSelectedQuest;
		GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( dwID );
		if ( !pQUEST ){
			return;
		}

		GLQUESTPROG* pQUEST_PROG = m_pData->m_cQuestPlay.FindProc ( dwID );
		if ( pQUEST_PROG ){
			CDebugSet::MsgBox( GetSafeHwnd(), "Quest [%04d] '%s' already exist in Quest Proc!", pQUEST->m_sNID.dwID, pQUEST->m_strTITLE.c_str() );
			return;
		}

		GLQUESTPROG* pQUEST_END = m_pData->m_cQuestPlay.FindEnd ( dwID );
		if ( pQUEST_END )
		{
			if ( pQUEST_END->m_bCOMPLETE )
			{
				if ( !pQUEST->IsREPEAT() ){
					CDebugSet::MsgBox( GetSafeHwnd(), "Quest [%04d] '%s' already exist in Quest End and not allowed to repeat!", pQUEST->m_sNID.dwID, pQUEST->m_strTITLE.c_str() );
					return;
				}
			}
			else
			{
				if ( !pQUEST->IsAGAIN() ){
					CDebugSet::MsgBox( GetSafeHwnd(), "Quest [%04d] '%s' already exist in Quest End and not allowed to do again!", pQUEST->m_sNID.dwID, pQUEST->m_strTITLE.c_str() );
					return;
				}
			}
		}

		GLQUESTPROG sQUEST_PROG_NEW;
		sQUEST_PROG_NEW.START ( pQUEST, pQUEST_END );
		m_pData->m_cQuestPlay.InsertProc ( sQUEST_PROG_NEW );

		UpdateQuestProc();

		for( int nItem = 0; nItem < m_ListProc.GetItemCount(); nItem++){
			DWORD dwQUESTID = m_ListProc.GetItemData( nItem );
			if( dwQUESTID == dwID ){
				m_ListProc.EnsureVisible( nItem, TRUE );
				m_ListProc.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
				GotoDlgCtrl(GetDlgItem(m_ListProc.GetDlgCtrlID())); 
			}
		}
	}
}

void CCharEditPageQuest::OnBnClickedChareditQuestButtonProcComplete()
{
	if ( !m_pData ){
		return;
	}

	int nSelect = m_ListProc.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = (DWORD) m_ListProc.GetItemData ( nSelect );

	GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( dwID );
	if ( !pQUEST ){
		return;
	}

	GLQUESTPROG* pQUEST_PROG = m_pData->m_cQuestPlay.FindProc ( dwID );
	if ( !pQUEST_PROG ){
		return;
	}

	pQUEST_PROG->m_sSTEP_PROG.RESET();
	pQUEST_PROG->m_dwSTEP = pQUEST->GetSTEPNUM() - 1;
	pQUEST_PROG->DoCOMPLETE();

	m_pData->m_cQuestPlay.InsertEnd( *pQUEST_PROG );
	m_pData->m_cQuestPlay.DeleteProc( dwID );

	UpdateQuestProc();
	UpdateQuestEnd();	
}

void CCharEditPageQuest::OnBnClickedChareditQuestButtonProcDelete()
{
	if ( !m_pData ){
		return;
	}

	int nSelect = m_ListProc.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = (DWORD) m_ListProc.GetItemData ( nSelect );
	GLQUESTPROG* pProg = m_pData->m_cQuestPlay.FindProc( dwID );
	if ( pProg ){
		m_pData->m_cQuestPlay.DeleteProc( dwID );

		UpdateQuestProc();

		int nNewSelect = nSelect > 1? nSelect-1:nSelect;
		m_ListProc.EnsureVisible( nNewSelect, TRUE );
		m_ListProc.SetItemState( nNewSelect, LVIS_SELECTED, LVIS_SELECTED );
		GotoDlgCtrl(GetDlgItem(m_ListProc.GetDlgCtrlID())); 
	}
}

void CCharEditPageQuest::OnBnClickedChareditQuestButtonProcClear()
{
	if ( !m_pData ){
		return;
	}

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All Quest Proc?" ) == IDYES )
	{
		for( int nItem=0; nItem<m_ListProc.GetItemCount(); nItem++ )
		{
			DWORD dwID = (DWORD) m_ListProc.GetItemData ( nItem );
			GLQUESTPROG* pProg = m_pData->m_cQuestPlay.FindProc( dwID );
			if ( pProg ){
				m_pData->m_cQuestPlay.DeleteProc( dwID );
			}
		}

		UpdateQuestProc();
	}
}

void CCharEditPageQuest::OnBnClickedChareditQuestButtonEndDelete()
{
	if ( !m_pData ){
		return;
	}

	int nSelect = m_ListEnd.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = (DWORD) m_ListEnd.GetItemData ( nSelect );
	GLQUESTPROG* pProg = m_pData->m_cQuestPlay.FindEnd( dwID );
	if ( pProg ){
		m_pData->m_cQuestPlay.DeleteEnd( dwID );

		UpdateQuestEnd();

		int nNewSelect = nSelect > 1? nSelect-1:nSelect;
		m_ListEnd.EnsureVisible( nNewSelect, TRUE );
		m_ListEnd.SetItemState( nNewSelect, LVIS_SELECTED, LVIS_SELECTED );
		GotoDlgCtrl(GetDlgItem(m_ListEnd.GetDlgCtrlID())); 
	}
}

void CCharEditPageQuest::OnBnClickedChareditQuestButtonEndClear()
{
	if ( !m_pData ){
		return;
	}

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All Quest End?" ) == IDYES )
	{
		for( int nItem=0; nItem<m_ListEnd.GetItemCount(); nItem++ )
		{
			DWORD dwID = (DWORD)m_ListEnd.GetItemData ( nItem );
			GLQUESTPROG* pProg = m_pData->m_cQuestPlay.FindEnd( dwID );
			if ( pProg ){
				m_pData->m_cQuestPlay.DeleteEnd( dwID );
			}
		}

		UpdateQuestEnd();
	}
}
