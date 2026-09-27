// PageCrow.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "EditorLevelView.h"
#include "PageCrow.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "DxViewPort.h"
#include "DlgFindCrow.h"
#include "DlgCrowReplace.h"
#include "GenerateName.h"
#include ".\pagecrow.h"
#include "DlgInsertLeader.h"
#include "DlgInsertMember.h"

// CPageCrow dialog

IMPLEMENT_DYNAMIC(CPageCrow, CPropertyPage)

CPageCrow::CPageCrow(LOGFONT logfont)
	: CPropertyPage(CPageCrow::IDD)
	, m_pFont ( NULL )
	, m_pGLLand( NULL )
	, m_emSHOW( EMCROWSHOW_TYPE_ALL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageCrow::~CPageCrow()
{
	SAFE_DELETE ( m_pFont );
}

void CPageCrow::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CPageCrow, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_ALL, OnBnClickedButtonShowAll)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_NPC, OnBnClickedButtonShowNpc)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_INVALID, OnBnClickedButtonShowInvalid)
	ON_BN_CLICKED(IDC_BUTTON_SHOWFIND, OnBnClickedButtonShowfind)
	ON_BN_CLICKED(IDC_BUTTON_SHOWFIND_CLEAR, OnBnClickedButtonShowfindClear)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnNMDblclkList)
	ON_BN_CLICKED(IDC_BUTTON_NEW, OnBnClickedButtonNew)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnBnClickedButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnBnClickedButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_COPY, OnBnClickedButtonCopy)
	ON_BN_CLICKED(IDC_BUTTON_PASTENEW, OnBnClickedButtonPastenew)
	ON_BN_CLICKED(IDC_BUTTON_REBUILDNAME, OnBnClickedButtonRebuildname)
	ON_BN_CLICKED(IDC_BUTTON_REPLACEID, OnBnClickedButtonReplaceid)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, OnBnClickedButtonClear)
	ON_BN_CLICKED(IDC_BUTTON_MOB_MEMBER, OnBnClickedButtonMobMember)
	ON_BN_CLICKED(IDC_BUTTON_MOB_LEADER, OnBnClickedButtonMobLeader)
	ON_BN_CLICKED(IDC_BUTTON_FSCHED, OnBnClickedButtonFsched)
END_MESSAGE_MAP()


// CPageCrow message handlers
BOOL CPageCrow::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	LONG lnWidth;
	LV_COLUMN lvColumn;

	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES );
	m_List.GetClientRect ( &rectCtrl );
	lnWidth = rectCtrl.right - rectCtrl.left;

	const int nColumnCount = 6;	
	char* szColumnName[nColumnCount] = { "", "Num", "ID", "Name", "Crow", "Group" };
	int nColumnWidthPercent[nColumnCount] = { 5, 13, 10, 25, 30, 17 };

	for ( int i = 0; i < nColumnCount; i++ )
	{		
		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.pszText = szColumnName[i];
		lvColumn.iSubItem = i;
		lvColumn.cx = ( lnWidth*nColumnWidthPercent[i] ) / 100;
		m_List.InsertColumn (i, &lvColumn );
	}

	SetWin_Num_int( this, IDC_EDIT_SHOWMID, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_EDIT_SHOWSID, NATIVEID_NULL().wSubID );

	return TRUE;  
}

void CPageCrow::DataSet( GLLandMan* pLand )
{
	m_pGLLand = pLand;
	DataShow( EMCROWSHOW_TYPE_ALL, TRUE );
}

void CPageCrow::ResetEditor()
{
	m_pGLLand = NULL;
}

void CPageCrow::JumpTo( D3DXVECTOR3 vPos )
{
	vPos.y += 10.0f;
	DxViewPort::GetInstance().CameraJump( vPos );
}

void CPageCrow::SelectCrow( std::string strName )
{
	if ( !m_pGLLand )	return;

	for( int nItem = 0; nItem < m_List.GetItemCount(); nItem++)
	{
		m_List.SetItemState( nItem, ~LVIS_SELECTED, LVIS_SELECTED  ) ;	
		m_List.SetCheck( nItem, false );
	}

	for( int nItem = 0; nItem < m_List.GetItemCount(); nItem++)
	{
		std::string strCrow = m_List.GetItemText( nItem, 3 );
		if ( !strcmp( strCrow.c_str(), strName.c_str() ) )
		{
			m_List.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED ) ;
			m_List.EnsureVisible( nItem, TRUE );
			m_List.SetCheck( nItem, true );
			break;
		}	
	}
}

void CPageCrow::DataShow( EMCROWSHOW_TYPE emSHOW, BOOL bRESETFILTER /*= FALSE*/ )
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

	m_List.DeleteAllItems();

	std::string strMID = GetWin_Text( this, IDC_EDIT_SHOWMID );
	std::string strSID = GetWin_Text( this, IDC_EDIT_SHOWSID );
	
	if ( bRESETFILTER || strMID.size() <= 0 )
		SetWin_Num_int( this, IDC_EDIT_SHOWMID, NATIVEID_NULL().wMainID );

	if ( bRESETFILTER || strSID.size() <= 0 )
		SetWin_Num_int( this, IDC_EDIT_SHOWSID, NATIVEID_NULL().wSubID );

	if ( !m_pGLLand )	return;

	SNATIVEID sID;
	sID.wMainID = GetWin_Num_int( this, IDC_EDIT_SHOWMID );
	sID.wSubID = GetWin_Num_int( this, IDC_EDIT_SHOWSID );

	MOBSCHEDULELIST * GLMobSchList = m_pGLLand->GetMobSchMan()->GetMobSchList();
	SGLNODE<GLMobSchedule*>* pCur = GLMobSchList->m_pHead;

	m_List.SetRedraw( FALSE );

	char szTempChar[512];
	int i = 0;
	while ( pCur )
	{
		PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( pCur->Data->m_CrowID );

		BOOL bSHOW = TRUE;
		switch( emSHOW )
		{
		case EMCROWSHOW_TYPE_NPC:
			{
				if ( !pCROW  )	bSHOW = FALSE;
				if ( pCROW && pCROW->m_sBasic.m_emCrow != CROW_NPC )	bSHOW = FALSE;
			}break;
		case EMCROWSHOW_TYPE_ERROR:
			{
				if ( pCROW  )	bSHOW = FALSE;
			}break;
		};
	
		if ( sID.wMainID != SNATIVEID::ID_NULL && sID.wSubID != SNATIVEID::ID_NULL )
		{
			if ( pCur->Data->m_CrowID != sID )
				bSHOW = FALSE;
		}

		if ( sID.wMainID != SNATIVEID::ID_NULL )
		{
			if ( pCur->Data->m_CrowID.wMainID != sID.wMainID )
				bSHOW = FALSE;
		}

		if ( sID.wSubID != SNATIVEID::ID_NULL )
		{
			if ( pCur->Data->m_CrowID.wSubID != sID.wSubID )
				bSHOW = FALSE;
		}

		if ( bSHOW )
		{
			m_List.InsertItem( i, "" );

			sprintf( szTempChar, "%d", i );
			m_List.SetItemText( i, 1, szTempChar );

			sprintf( szTempChar, "%d", pCur->Data->m_dwGlobID );
			m_List.SetItemText( i, 2, szTempChar );

			m_List.SetItemText( i, 3, pCur->Data->m_szName );

			std::string strNAME = "Unknown Crow";
			if ( pCROW )	strNAME = pCROW->GetName();

			sprintf( szTempChar, "(%d~%d) %s", pCur->Data->m_CrowID.wMainID, pCur->Data->m_CrowID.wSubID, strNAME.c_str() );
			m_List.SetItemText( i, 4, szTempChar );

			if ( pCur->Data->m_strGroupName.size() > 0 )
			{
				sprintf( szTempChar, "[%s]%s", pCur->Data->m_bLeaderMonster? "L":"M", pCur->Data->m_strGroupName.c_str() );
				m_List.SetItemText( i, 5, szTempChar );
			}
			
			m_List.SetItemData( i, (DWORD_PTR)pCur->Data );

			i++;
		}

		pCur = pCur->pNext;
	}

	m_List.SetRedraw( TRUE );

	if ( nSelect == -1 && m_List.GetItemCount() > 0 )
		nSelect = 0;

	if ( nSelect != -1 )
	{
		m_List.EnsureVisible( nSelect, TRUE );
		m_List.SetItemState( nSelect, LVIS_SELECTED, LVIS_SELECTED );
	}

	m_emSHOW = emSHOW;
}

void CPageCrow::OnBnClickedButtonShowAll()
{
	DataShow( EMCROWSHOW_TYPE_ALL );
}

void CPageCrow::OnBnClickedButtonShowNpc()
{
	DataShow( EMCROWSHOW_TYPE_NPC );
}

void CPageCrow::OnBnClickedButtonShowInvalid()
{
	DataShow( EMCROWSHOW_TYPE_ERROR );
}

void CPageCrow::OnBnClickedButtonShowfind()
{
	CDlgFindCrow dlg( this );
	if ( dlg.DoModal() == IDOK )
	{
		SNATIVEID sID = dlg.m_sSELECTED;
		PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( sID );
		if( pCROW )
		{
			SetWin_Num_int( this, IDC_EDIT_SHOWMID, pCROW->sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_SHOWSID, pCROW->sNativeID.wSubID );
			DataShow( m_emSHOW );
		}
	}
}

void CPageCrow::OnBnClickedButtonShowfindClear()
{
	DataShow( m_emSHOW, TRUE );
}

void CPageCrow::OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( !m_pGLLand )	return;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	std::string strCrow = m_List.GetItemText( nSelect, 3 );

	SGLNODE<GLMobSchedule*>* pSched = m_pGLLand->GetMobSchMan()->FindMobSch( strCrow.c_str() );
	if ( pSched )
	{
		CEditorLevelView::GetView()->m_pSelectSched = pSched->Data;
		D3DXVECTOR3 vPos = pSched->Data->m_pAffineParts->vTrans;
		JumpTo( vPos );
	}
}

void CPageCrow::OnBnClickedButtonNew()
{
	if ( !m_pGLLand )	return;

	if ( m_pSheetTab )
		m_pSheetTab->PageEditCrow(  NULL );
}

void CPageCrow::OnBnClickedButtonEdit()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	GLMobSchedule* pSCHED = (GLMobSchedule*)m_List.GetItemData ( nSelect );
	if ( pSCHED )
	{
		if ( m_pSheetTab )
			m_pSheetTab->PageEditCrow( pSCHED );
	}
}

void CPageCrow::OnBnClickedButtonDelete()
{
	if ( !m_pGLLand )	return;

	CEditorLevelView::GetView()->m_pSelectedGate = NULL;
	CEditorLevelView::GetView()->m_pSelectSched = NULL;

	int nCHECKED = 0;
	for( int nItem = 0; nItem < m_List.GetItemCount(); nItem++ )
	{
		BOOL bChecked = m_List.GetCheck( nItem );
		if( bChecked )
		{
			GLMobSchedule* pSCHED = (GLMobSchedule*)m_List.GetItemData ( nItem );
			if ( pSCHED )
			{
				DWORD dwID = pSCHED->m_dwGlobID;

				if ( pSCHED->m_bLeaderMonster )
				{	
					int nRETURN = CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Crow GID:%d is leader of group:%s. Delete?", dwID, pSCHED->m_strGroupName.c_str() );
					if ( nRETURN != IDYES )	continue;
				}

				BOOL bDrop = m_pGLLand->DropOutCrow( dwID );
				if ( bDrop )
					m_pGLLand->GetMobSchMan()->DelMobSch( pSCHED->m_szName );
			}

			nCHECKED++;
		}	
	}

	if ( nCHECKED == 0 )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected Use Check boxes to select" );
	}

	DataShow( m_emSHOW );
}

void CPageCrow::OnBnClickedButtonCopy()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	std::string strCrow = m_List.GetItemText( nSelect, 3 );
	SetWin_Text( this, IDC_EDIT_COPY, strCrow.c_str() );
}

void CPageCrow::OnBnClickedButtonPastenew()
{
	if( !m_pGLLand )	return;
	if( !m_pGLLand->GetMobSchMan() )	return;

	std::string strCopy = GetWin_Text( this, IDC_EDIT_COPY );
	if ( strCopy.size() != 0 )
	{
		SGLNODE<GLMobSchedule*>* pSched = m_pGLLand->GetMobSchMan()->FindMobSch( strCopy.c_str() );
		if ( pSched )
		{
			D3DXVECTOR3 vPOS = CEditorLevelView::GetView()->m_vPos;

			GLMobSchedule sSched;
			std::string str = CrowNameNew( m_pGLLand );
			StringCchCopy( sSched.m_szName, GLMobSchedule::EM_MAXSZNAME, str.c_str() );
			sSched.m_pAffineParts->vTrans = vPOS; 
			sSched.m_pAffineParts->vScale = pSched->Data->m_pAffineParts->vScale;
			sSched.m_pAffineParts->vRotate = pSched->Data->m_pAffineParts->vRotate;
			sSched.m_CrowID			= pSched->Data->m_CrowID;
			sSched.m_fReGenTime		= pSched->Data->m_fReGenTime;
			sSched.m_nRegenMin		= pSched->Data->m_nRegenMin;
			sSched.m_nRegenHour		= pSched->Data->m_nRegenHour;
			memcpy( sSched.m_bDayOfWeek, pSched->Data->m_bDayOfWeek, sizeof(sSched.m_bDayOfWeek) );
			sSched.m_bLeaderMonster	= pSched->Data->m_bLeaderMonster;
			sSched.m_strGroupName	= pSched->Data->m_strGroupName.c_str();
			sSched.m_sMobActList		= pSched->Data->m_sMobActList;
			sSched.m_dwPC_REGEN_GATEID	= pSched->Data->m_dwPC_REGEN_GATEID;
			sSched.m_bRendGenPos		= pSched->Data->m_bRendGenPos;
			sSched.m_wRendGenPosNum		= pSched->Data->m_wRendGenPosNum;
			sSched.m_wRendGenPosDist	= pSched->Data->m_wRendGenPosDist;
			sSched.m_vectorRendGenPos	= pSched->Data->m_vectorRendGenPos;
			//sSched.CalculateBox();
			D3DXVECTOR3 vDis = pSched->Data->m_pAffineParts->vTrans - vPOS;
			sSched.ModifierSchList( &vDis, m_pGLLand->GetNavi() );
			sSched.GenerateRendGenPos( m_pGLLand->GetNavi() );

			m_pGLLand->GetMobSchMan()->AddMobSch( sSched );

			DataShow( m_emSHOW );
		}		
	}
}

void CPageCrow::OnBnClickedButtonRebuildname()
{
	if ( !m_pGLLand )	return;

	int nNPC = 0;
	int nCROW = 0;
	MOBSCHEDULELIST * GLMobSchList = m_pGLLand->GetMobSchMan()->GetMobSchList();
	SGLNODE<GLMobSchedule*>* pCur = GLMobSchList->m_pHead;
	while ( pCur )
	{
		PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( pCur->Data->m_CrowID );
		if ( pCROW )
		{
			char szTempChar[GLMobSchedule::EM_MAXSZNAME];

			if ( pCROW->m_sBasic.m_emCrow == CROW_NPC )
			{
				sprintf( szTempChar, "NPC_%04d", nNPC );
				nNPC++;
			}else{
				sprintf( szTempChar, "CROW_%04d", nCROW );
				nCROW++;
			}

			StringCchCopy( pCur->Data->m_szName, GLMobSchedule::EM_MAXSZNAME, szTempChar );
		}

		pCur = pCur->pNext;
	}

	DataShow( m_emSHOW );
}

void CPageCrow::OnBnClickedButtonReplaceid()
{
	if ( !m_pGLLand )	return;

	CDlgCrowReplace dlg( this );
	if ( dlg.DoModal() == IDOK )
	{
		for( int i = 0; i < (int)dlg.m_vecReplace.size(); ++ i )
		{
			SCROWREPLACE sREPLACE = dlg.m_vecReplace[i]; 
			PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( sREPLACE.sIDTO );
			if ( !pCROW )	continue;

			MOBSCHEDULELIST * GLMobSchList = m_pGLLand->GetMobSchMan()->GetMobSchList();
			SGLNODE<GLMobSchedule*>* pCur = GLMobSchList->m_pHead;
			while ( pCur )
			{
				if ( pCur->Data->m_CrowID == sREPLACE.sIDFROM )
				{
					pCur->Data->m_CrowID = sREPLACE.sIDTO;

					m_pGLLand->DropOutCrow( pCur->Data->m_dwGlobID );
				}

				pCur = pCur->pNext;
			}
		}

		DataShow( m_emSHOW );
	}
}

void CPageCrow::OnBnClickedButtonSave()
{
	if ( m_pSheetTab )
		m_pSheetTab->PageActive( LEVELEDIT_PAGE_MAIN );
}

void CPageCrow::OnBnClickedButtonClear()
{
	int nRETURN = CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All Schedule?" );
	if ( nRETURN != IDYES )	return;

	CEditorLevelView::GetView()->m_pSelectedGate = NULL;
	CEditorLevelView::GetView()->m_pSelectSched = NULL;

	for( int nItem = 0; nItem < m_List.GetItemCount(); nItem++ )
	{
		GLMobSchedule* pSCHED = (GLMobSchedule*)m_List.GetItemData ( nItem );
		if ( pSCHED )
		{
			m_pGLLand->DropOutCrow( pSCHED->m_dwGlobID );
			m_pGLLand->GetMobSchMan()->DelMobSch( pSCHED->m_szName );
		}
	}

	DataShow( m_emSHOW );
}

void CPageCrow::OnBnClickedButtonMobMember()
{
	if ( !m_pGLLand )	return;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	GLMobSchedule* pSCHED = (GLMobSchedule*)m_List.GetItemData ( nSelect );
	if ( pSCHED )
	{
		CDlgInsertMember dlg( this, m_pGLLand, pSCHED );
		if ( dlg.DoModal() == IDOK )
		{
			DataShow( m_emSHOW );
		}
	}
}

void CPageCrow::OnBnClickedButtonMobLeader()
{
	if ( !m_pGLLand )	return;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	GLMobSchedule* pSCHED = (GLMobSchedule*)m_List.GetItemData ( nSelect );
	if ( pSCHED )
	{
		CDlgInsertLeader dlg( this, m_pGLLand, pSCHED );
		if ( dlg.DoModal() == IDOK )
		{
			DataShow( m_emSHOW );
		}
	}
}

void CPageCrow::OnBnClickedButtonFsched()
{
	int nRETURN = CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Fix schedule time?" );
	if ( nRETURN != IDYES )	return;

	CEditorLevelView::GetView()->m_pSelectedGate = NULL;
	CEditorLevelView::GetView()->m_pSelectSched = NULL;

	int nfixed = 0;
	for( int nItem = 0; nItem < m_List.GetItemCount(); nItem++ )
	{
		GLMobSchedule* pschedule = (GLMobSchedule*)m_List.GetItemData ( nItem );
		if ( pschedule )
		{
			SCROWDATA* pcrow_data = GLCrowDataMan::GetInstance().GetCrowData( pschedule->m_CrowID );
			if ( pcrow_data )
			{
				if ( pcrow_data->m_emCrow == CROW_MOB )
				{
					if ( pschedule->m_fReGenTime < 20.0f )
					{
						pschedule->m_fReGenTime = 20.0f;
						nfixed++;
					}
				}
				else
				{
					if ( pschedule->m_fReGenTime < 4.0f )
					{
						pschedule->m_fReGenTime = 4.0f;
						nfixed++;
					}
				}
			}
		}
	}

	if ( nfixed > 0 )
		CDebugSet::MsgBoxAfx( "fixed %d schedules", nfixed );

	DataShow( m_emSHOW );
}
