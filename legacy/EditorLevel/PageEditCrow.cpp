// PageEditCrow.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "PageEditCrow.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "EditorLevelView.h"
#include "GenerateName.h"
#include "GLCrowData.h"
#include "DlgFindCrow.h"

// CPageEditCrow dialog

IMPLEMENT_DYNAMIC(CPageEditCrow, CPropertyPage)

CPageEditCrow::CPageEditCrow(LOGFONT logfont)
	: CPropertyPage(CPageEditCrow::IDD)
	, m_pFont ( NULL )
	, m_pGLLand( NULL )
	, m_pEdit( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEditCrow::~CPageEditCrow()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEditCrow::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CPageEditCrow, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CHECK, OnBnClickedButtonCheck)
	ON_BN_CLICKED(IDC_BUTTON_NEW, OnBnClickedButtonNew)
	ON_BN_CLICKED(IDC_BUTTON_CROW, OnBnClickedButtonCrow)
	ON_EN_CHANGE(IDC_EDIT_CROW_MID, OnEnChangeEditCrowMid)
	ON_EN_CHANGE(IDC_EDIT_CROW_SID, OnEnChangeEditCrowSid)
	ON_BN_CLICKED(IDC_BUTTON_REGNEW, OnBnClickedButtonRegnew)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_BUS, OnBnClickedButtonBus)
	ON_BN_CLICKED(IDC_BUTTON_ACTION_ADD, OnBnClickedButtonActionAdd)
	ON_BN_CLICKED(IDC_BUTTON_ACTION_EDIT, OnBnClickedButtonActionEdit)
	ON_BN_CLICKED(IDC_BUTTON_ACTION_DEL, OnBnClickedButtonActionDel)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnNMDblclkList)
	ON_BN_CLICKED(IDC_BUTTON_ACTION_CLOSE, OnBnClickedButtonActionClose)
	ON_CBN_SELCHANGE(IDC_COMBO_ACTION, OnCbnSelchangeComboAction)
	ON_BN_CLICKED(IDC_BUTTON_REBUILD, OnBnClickedButtonRebuild)
	ON_BN_CLICKED(IDC_BUTTON_ACTION_POS, OnBnClickedButtonActionPos)
	ON_BN_CLICKED(IDC_BUTTON_ACTION_SAVE, OnBnClickedButtonActionSave)
END_MESSAGE_MAP()


// CPageEditCrow message handlers
BOOL CPageEditCrow::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	LONG lnWidth;
	LV_COLUMN lvColumn;

	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	lnWidth = rectCtrl.right - rectCtrl.left;

	const int nColumnCount = 4;	
	char* szColumnName1[nColumnCount] = { "#", "Type", "Val" };
	int nColumnWidthPercent[nColumnCount] = { 10, 30, 60 };

	for ( int i = 0; i < nColumnCount; i++ )
	{		
		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.pszText = szColumnName1[i];
		lvColumn.iSubItem = i;
		lvColumn.cx = ( lnWidth*nColumnWidthPercent[i] ) / 100;
		m_List.InsertColumn (i, &lvColumn );
	}

	SetWin_Combo_Init( this, IDC_COMBO_ACTION, COMMENT::MOBACTIONS, EMACTION_SIZE );

	return TRUE;  
}

void CPageEditCrow::SetData( GLLandMan* pLand, GLMobSchedule* pEdit )
{
	m_pGLLand = pLand;
	m_pEdit = pEdit;

	if ( m_pGLLand && m_pEdit )
	{
		InitUI( TRUE );
		DataShow();
		JumpTo( m_pEdit->m_pAffineParts->vTrans );
	}else if( m_pGLLand ){
		InitUI( FALSE );
		SetWin_Text( this, IDC_EDIT_NAME, CrowNameNew( m_pGLLand ).c_str() );
	}
}

void CPageEditCrow::ResetData()
{
	m_pEdit = NULL;
	m_pGLLand = NULL;
}

void CPageEditCrow::InitUI( BOOL bENABLE )
{
	SetWin_Enable( this, IDC_BUTTON_REGNEW, !bENABLE );
	SetWin_Enable( this, IDC_BUTTON_REBUILD, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_APPLY, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_SAVE, bENABLE );
	SetWin_Enable( this, IDC_EDIT_GENTIME, bENABLE );
	SetWin_Enable( this, IDC_EDIT_GENMIN, bENABLE );
	SetWin_Enable( this, IDC_EDIT_GENHOUR, bENABLE );
	SetWin_Enable( this, IDC_CHECK_DAYOFWEEK_0, bENABLE );
	SetWin_Enable( this, IDC_CHECK_DAYOFWEEK_1, bENABLE );
	SetWin_Enable( this, IDC_CHECK_DAYOFWEEK_2, bENABLE );
	SetWin_Enable( this, IDC_CHECK_DAYOFWEEK_3, bENABLE );
	SetWin_Enable( this, IDC_CHECK_DAYOFWEEK_4, bENABLE );
	SetWin_Enable( this, IDC_CHECK_DAYOFWEEK_5, bENABLE );
	SetWin_Enable( this, IDC_CHECK_DAYOFWEEK_6, bENABLE );
	SetWin_Enable( this, IDC_CHECK_GENPOS_RANDOM, bENABLE );
	SetWin_Enable( this, IDC_EDIT_GENPOS_NUM, bENABLE );
	SetWin_Enable( this, IDC_EDIT_GENPOS_DISTANCE, bENABLE );
	SetWin_Enable( this, IDC_CHECK_GROUP_LEADER, bENABLE );
	SetWin_Enable( this, IDC_EDIT_GROUP_NAME, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_BUS, bENABLE );
	SetWin_Enable( this, IDC_EDIT_BUS, bENABLE );
	SetWin_Enable( this, IDC_EDIT_REVIVE_GATE, bENABLE );
	SetWin_Enable( this, IDC_LIST, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_ACTION_ADD, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_ACTION_EDIT, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_ACTION_DEL, bENABLE );

	SetWin_Enable( this, IDC_COMBO_ACTION, bENABLE );
	SetWin_Enable( this, IDC_EDIT_ACTION_X, bENABLE );
	SetWin_Enable( this, IDC_EDIT_ACTION_Y, bENABLE );
	SetWin_Enable( this, IDC_EDIT_ACTION_Z, bENABLE );
	SetWin_Enable( this, IDC_EDIT_ACTION_TIME, bENABLE );
	SetWin_Enable( this, IDC_EDIT_ACTION_NUM, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_ACTION_POS, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_ACTION_SAVE, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_ACTION_CLOSE, bENABLE );

	//defaults
	SetWin_Text( this, IDC_EDIT_NAME, "" );
	SetWin_Num_int( this, IDC_EDIT_CROW_MID, SNATIVEID::ID_NULL );
	SetWin_Num_int( this, IDC_EDIT_CROW_SID, SNATIVEID::ID_NULL );
	SetWin_Num_float( this, IDC_EDIT_GENTIME, 0.0f );
	SetWin_Num_int( this, IDC_EDIT_GENMIN, 0 );
	SetWin_Num_int( this, IDC_EDIT_GENHOUR, 0 );
	SetWin_Check( this, IDC_CHECK_DAYOFWEEK_0, FALSE );
	SetWin_Check( this, IDC_CHECK_DAYOFWEEK_1, FALSE );
	SetWin_Check( this, IDC_CHECK_DAYOFWEEK_2, FALSE );
	SetWin_Check( this, IDC_CHECK_DAYOFWEEK_3, FALSE );
	SetWin_Check( this, IDC_CHECK_DAYOFWEEK_4, FALSE );
	SetWin_Check( this, IDC_CHECK_DAYOFWEEK_5, FALSE );
	SetWin_Check( this, IDC_CHECK_DAYOFWEEK_6, FALSE );
	SetWin_Check( this, IDC_CHECK_GENPOS_RANDOM, FALSE );
	SetWin_Num_int( this, IDC_EDIT_GENPOS_NUM, 0 );
	SetWin_Num_int( this, IDC_EDIT_GENPOS_DISTANCE, 0 );
	SetWin_Check( this, IDC_CHECK_GROUP_LEADER, FALSE );
	SetWin_Text( this, IDC_EDIT_GROUP_NAME, "" );
	SetWin_Text( this, IDC_EDIT_BUS, "" );
	SetWin_Num_int( this, IDC_EDIT_REVIVE_GATE, 0 );

	SetWin_Num_int( this, IDC_EDIT_EX_1, 0 );
	SetWin_Num_int( this, IDC_EDIT_EX_2, 0 );
	SetWin_Num_int( this, IDC_EDIT_EX_3, 0 );

	m_List.DeleteAllItems();
}

void CPageEditCrow::JumpTo( D3DXVECTOR3 vPos )
{
	vPos.y += 10.0f;
	DxViewPort::GetInstance().CameraJump( vPos );
}

void CPageEditCrow::DataShow()
{
	if ( !m_pGLLand )	return;
	if ( !m_pEdit )		return;

	SetWin_Text( this, IDC_EDIT_NAME, m_pEdit->m_szName );
	SetWin_Num_int( this, IDC_EDIT_CROW_MID, m_pEdit->m_CrowID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_CROW_SID, m_pEdit->m_CrowID.wSubID );
	SetWin_Num_float( this, IDC_EDIT_GENTIME, m_pEdit->m_fReGenTime );
	SetWin_Num_int( this, IDC_EDIT_GENMIN, m_pEdit->m_nRegenMin );
	SetWin_Num_int( this, IDC_EDIT_GENHOUR, m_pEdit->m_nRegenHour );
	SetWin_Check( this, IDC_CHECK_DAYOFWEEK_0, m_pEdit->m_bDayOfWeek[0] );
	SetWin_Check( this, IDC_CHECK_DAYOFWEEK_1, m_pEdit->m_bDayOfWeek[1] );
	SetWin_Check( this, IDC_CHECK_DAYOFWEEK_2, m_pEdit->m_bDayOfWeek[2] );
	SetWin_Check( this, IDC_CHECK_DAYOFWEEK_3, m_pEdit->m_bDayOfWeek[3] );
	SetWin_Check( this, IDC_CHECK_DAYOFWEEK_4, m_pEdit->m_bDayOfWeek[4] );
	SetWin_Check( this, IDC_CHECK_DAYOFWEEK_5, m_pEdit->m_bDayOfWeek[5] );
	SetWin_Check( this, IDC_CHECK_DAYOFWEEK_6, m_pEdit->m_bDayOfWeek[6] );
	SetWin_Check( this, IDC_CHECK_GENPOS_RANDOM, m_pEdit->m_bRendGenPos );
	SetWin_Num_int( this, IDC_EDIT_GENPOS_NUM, m_pEdit->m_wRendGenPosNum );
	SetWin_Num_int( this, IDC_EDIT_GENPOS_DISTANCE, m_pEdit->m_wRendGenPosDist );
	SetWin_Check( this, IDC_CHECK_GROUP_LEADER, m_pEdit->m_bLeaderMonster );
	SetWin_Text( this, IDC_EDIT_GROUP_NAME, m_pEdit->m_strGroupName.c_str() );
	SetWin_Text( this, IDC_EDIT_BUS, m_pEdit->m_strBUSLOCATION.c_str() );
	SetWin_Num_int( this, IDC_EDIT_REVIVE_GATE, m_pEdit->m_dwPC_REGEN_GATEID );

	//SetWin_Num_int( this, IDC_EDIT_EX_1, m_pEdit->m_nExData1 );
	//SetWin_Num_int( this, IDC_EDIT_EX_2, m_pEdit->m_nExData2 );
	//SetWin_Num_int( this, IDC_EDIT_EX_3, m_pEdit->m_dwExData1 );

	ShowActions();

	CEditorLevelView::GetView()->EditSmatSet( TRUE );
	CEditorLevelView::GetView()->m_DxEditMat.SetAffineMatrix( m_pEdit );
}

BOOL CPageEditCrow::DataSave()
{
	if ( !m_pGLLand )	return FALSE;
	if ( !m_pEdit )		return FALSE;

	std::string strNAME = GetWin_Text( this, IDC_EDIT_NAME );
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_CROW_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_CROW_SID );

	if ( strNAME.size() <= 0 )
	{
		CDebugSet::MsgBox( "Name Empty!" );
		return FALSE;
	}

	if ( strcmp( m_pEdit->m_szName, strNAME.c_str() ) )
	{
		if ( CrowNameCheck( m_pGLLand, strNAME.c_str() ) )
		{
			CDebugSet::MsgBox( "Name: %s is already used! Create New name!", strNAME.c_str() );
			return FALSE;
		}
	}

	PCROWDATA pcrow_data = GLCrowDataMan::GetInstance().GetCrowData( wMID, wSID );
	if ( !pcrow_data )
	{
		CDebugSet::MsgBox( "Invalid CROW! ( %d~%d )", wMID, wSID );
		return FALSE;
	}

	StringCchCopy( m_pEdit->m_szName, GLMobSchedule::EM_MAXSZNAME, strNAME.c_str() );
	m_pEdit->m_CrowID.wMainID = wMID;
	m_pEdit->m_CrowID.wSubID = wSID;
	m_pEdit->m_fReGenTime = GetWin_Num_float( this, IDC_EDIT_GENTIME );
	m_pEdit->m_nRegenMin = GetWin_Num_int( this, IDC_EDIT_GENMIN );
	m_pEdit->m_nRegenHour = GetWin_Num_int( this, IDC_EDIT_GENHOUR );
	m_pEdit->m_bDayOfWeek[0] = ( GetWin_Check( this, IDC_CHECK_DAYOFWEEK_0 ) == TRUE );
	m_pEdit->m_bDayOfWeek[1] = ( GetWin_Check( this, IDC_CHECK_DAYOFWEEK_1 ) == TRUE );
	m_pEdit->m_bDayOfWeek[2] = ( GetWin_Check( this, IDC_CHECK_DAYOFWEEK_2 ) == TRUE );
	m_pEdit->m_bDayOfWeek[3] = ( GetWin_Check( this, IDC_CHECK_DAYOFWEEK_3 ) == TRUE );
	m_pEdit->m_bDayOfWeek[4] = ( GetWin_Check( this, IDC_CHECK_DAYOFWEEK_4 ) == TRUE );
	m_pEdit->m_bDayOfWeek[5] = ( GetWin_Check( this, IDC_CHECK_DAYOFWEEK_5 ) == TRUE );
	m_pEdit->m_bDayOfWeek[6] = ( GetWin_Check( this, IDC_CHECK_DAYOFWEEK_6 ) == TRUE );
	m_pEdit->m_bRendGenPos = GetWin_Check( this, IDC_CHECK_GENPOS_RANDOM );
	m_pEdit->m_wRendGenPosNum = GetWin_Num_int( this, IDC_EDIT_GENPOS_NUM );
	m_pEdit->m_wRendGenPosDist = GetWin_Num_int( this, IDC_EDIT_GENPOS_DISTANCE );
	m_pEdit->m_bLeaderMonster = GetWin_Check( this, IDC_CHECK_GROUP_LEADER );
	m_pEdit->m_strGroupName = GetWin_Text( this, IDC_EDIT_GROUP_NAME ).GetString();
	m_pEdit->m_strBUSLOCATION = GetWin_Text( this, IDC_EDIT_BUS ).GetString();
	m_pEdit->m_dwPC_REGEN_GATEID = GetWin_Num_int( this, IDC_EDIT_REVIVE_GATE );

	//m_pEdit->m_nExData1 = GetWin_Num_int( this, IDC_EDIT_EX_1 );
	//m_pEdit->m_nExData2 = GetWin_Num_int( this, IDC_EDIT_EX_2 );
	//m_pEdit->m_dwExData1 = GetWin_Num_int( this, IDC_EDIT_EX_3 );

	if ( m_pEdit->m_bRendGenPos ){
		m_pEdit->GenerateRendGenPos( m_pGLLand->GetNavi() );
	}else{
		m_pEdit->m_vectorRendGenPos.clear();
	}

	/*if ( pcrow_data->m_emCrow == CROW_MOB )
	{
		if ( m_pEdit->m_fReGenTime < 20.0f )
		{
			m_pEdit->m_fReGenTime = 20.0f;
			CDebugSet::MsgBoxAfx( "gen time very low! corrected to :%g", m_pEdit->m_fReGenTime );
		}
	}
	else
	{
		if ( m_pEdit->m_fReGenTime < 4.0f )
		{
			m_pEdit->m_fReGenTime = 4.0f;
			CDebugSet::MsgBoxAfx( "gen time very low! corrected to :%g", m_pEdit->m_fReGenTime );
		}
	}*/

	if ( m_pEdit->m_bLeaderMonster )
	{
		if ( m_pEdit->m_strGroupName.size() <= 0 )
		{
			CDebugSet::MsgBox( "GroupName cannot be empty is crow is leader!" );
			return FALSE;
		}
	}

	if ( m_pEdit->m_strGroupName.size() > 0 )
	{
		std::string strGROUP = m_pEdit->m_strGroupName.c_str();
		DWORD dwGLOBALID = m_pEdit->m_dwGlobID;

		BOOL bMEMBER = m_pGLLand->IS_GROUPMEMBER( strGROUP.c_str(), dwGLOBALID );
		if ( !bMEMBER )
		{
			m_pGLLand->ADD_GROUPMEMBER( strGROUP.c_str(), dwGLOBALID );

			if( m_pEdit->m_bLeaderMonster )
				m_pGLLand->ADD_GROUPLEADER( strGROUP.c_str(), dwGLOBALID );
		}
	}

	m_pGLLand->DropOutCrow( m_pEdit->m_dwGlobID );

	return TRUE;
}

void CPageEditCrow::ShowActions()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

	m_List.DeleteAllItems();

	if ( !m_pGLLand )	return;
	if ( !m_pEdit )		return;

	std::string strAct[2] = 
	{
		"Move",
		"Stay",
	};

	m_List.SetRedraw( FALSE );

	char szTempChar[512];
	SGLNODE<SMOBACTION>* pCur = m_pEdit->m_sMobActList.m_pHead;
	for ( int i = 0; pCur; pCur = pCur->pNext, ++i )
	{
		sprintf( szTempChar, "%d", i );
		m_List.InsertItem( i, szTempChar );

		sprintf( szTempChar, "%s", strAct[pCur->Data.emAction].c_str() );
		m_List.SetItemText( i, 1, szTempChar );

		if ( pCur->Data.emAction == EMACTION_MOVE )
		{
			sprintf( szTempChar, "%.2f~%.2f~%.2f", pCur->Data.vPos.x, pCur->Data.vPos.y, pCur->Data.vPos.z );
			m_List.SetItemText( i, 2, szTempChar );
		}
		else if ( pCur->Data.emAction == EMACTION_ACTSTAY )
		{
			sprintf( szTempChar, "%.2f", pCur->Data.fLife );
			m_List.SetItemText( i, 2, szTempChar );
		}

		m_List.SetItemData( i, (DWORD_PTR)pCur );
	}

	m_List.SetRedraw( TRUE );

	if ( nSelect == -1 && m_List.GetItemCount() > 0 )
		nSelect = 0;

	if ( nSelect != -1 )
	{
		m_List.EnsureVisible( nSelect, TRUE );
		m_List.SetItemState( nSelect, LVIS_SELECTED, LVIS_SELECTED );
	}

	SetWin_Enable( this, IDC_COMBO_ACTION, FALSE );
	SetWin_Enable( this, IDC_EDIT_ACTION_X, FALSE );
	SetWin_Enable( this, IDC_EDIT_ACTION_Y, FALSE );
	SetWin_Enable( this, IDC_EDIT_ACTION_Z, FALSE );
	SetWin_Enable( this, IDC_EDIT_ACTION_TIME, FALSE );
	SetWin_Enable( this, IDC_EDIT_ACTION_NUM, FALSE );
	SetWin_Enable( this, IDC_BUTTON_ACTION_POS, FALSE );
	SetWin_Enable( this, IDC_BUTTON_ACTION_SAVE, FALSE );
	SetWin_Enable( this, IDC_BUTTON_ACTION_CLOSE, FALSE );

	SetWin_Combo_Sel( this, IDC_COMBO_ACTION, 0 );
	SetWin_Num_float( this, IDC_EDIT_ACTION_X, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_ACTION_Y, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_ACTION_Z, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_ACTION_TIME, 0.0f );
	SetWin_Num_int( this, IDC_EDIT_ACTION_NUM, -1 );
}

void CPageEditCrow::AdjustAction()
{
	EMMOBACTIONS emACTION = static_cast<EMMOBACTIONS>( GetWin_Combo_Sel( this, IDC_COMBO_ACTION ) );
	BOOL bMOVE = ( emACTION == EMACTION_MOVE );
	BOOL bSTAY = ( emACTION == EMACTION_ACTSTAY );

	SetWin_Enable( this, IDC_EDIT_ACTION_X, bMOVE );
	SetWin_Enable( this, IDC_EDIT_ACTION_Y, bMOVE );
	SetWin_Enable( this, IDC_EDIT_ACTION_Z, bMOVE );
	SetWin_Enable( this, IDC_EDIT_ACTION_TIME, bSTAY );
	SetWin_Enable( this, IDC_BUTTON_ACTION_POS, bMOVE );
}

void CPageEditCrow::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		if ( m_pSheetTab )
			m_pSheetTab->PageActive( LEVELEDIT_PAGE_CROWLIST );
	}
}

void CPageEditCrow::OnBnClickedButtonClose()
{
	if ( m_pSheetTab )
		m_pSheetTab->PageActive( LEVELEDIT_PAGE_CROWLIST );
}

void CPageEditCrow::OnBnClickedButtonCheck()
{
	if ( !m_pGLLand )	return;
	if ( !m_pEdit )		return;

	std::string strName = GetWin_Text( this, IDC_EDIT_NAME );
	if ( strcmp( m_pEdit->m_szName, strName.c_str() ) )
	{
		if ( CrowNameCheck( m_pGLLand, strName.c_str() ) )
		{
			CDebugSet::MsgBox( "Name: %s is already used! Create New name!", strName.c_str() );
		}else{
			CDebugSet::MsgBox( "Name: %s is Valid!", strName.c_str() );
		}
	}
}

void CPageEditCrow::OnBnClickedButtonNew()
{
	SetWin_Text( this, IDC_EDIT_NAME, CrowNameNew( m_pGLLand ).c_str() );
}

void CPageEditCrow::OnBnClickedButtonCrow()
{
	CDlgFindCrow dlg( this );
	if ( dlg.DoModal() == IDOK )
	{
		PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( dlg.m_sSELECTED );
		if ( pCROW )
		{
			SetWin_Num_int( this, IDC_EDIT_CROW_MID, pCROW->m_sBasic.sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_CROW_SID, pCROW->m_sBasic.sNativeID.wSubID );
		}
	}
}

void CPageEditCrow::OnEnChangeEditCrowMid()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_CROW_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_CROW_SID );

	std::string strNAME = "Unknown Name";
	PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( wMID, wSID );
	if( pCROW )
		strNAME = pCROW->GetName();

	SetWin_Text( this, IDC_EDIT_CROW_NAME, strNAME.c_str() );
}

void CPageEditCrow::OnEnChangeEditCrowSid()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_CROW_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_CROW_SID );

	std::string strNAME = "Unknown Name";
	PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( wMID, wSID );
	if( pCROW )
		strNAME = pCROW->GetName();

	SetWin_Text( this, IDC_EDIT_CROW_NAME, strNAME.c_str() );
}

void CPageEditCrow::OnBnClickedButtonRegnew()
{
	if( !m_pGLLand )	return;
	if( !m_pGLLand->GetMobSchMan() )	return;

	D3DXVECTOR3 vPOS = CEditorLevelView::GetView()->m_vPos;

	std::string strName = GetWin_Text( this, IDC_EDIT_NAME );
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_CROW_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_CROW_SID );

	if ( strName.size() <= 0 )
	{
		CDebugSet::MsgBox( "Name Empty!" );
		return;
	}

	bool bEXIST = CrowNameCheck( m_pGLLand, strName.c_str() );
	if ( bEXIST )
	{
		CDebugSet::MsgBox( "Name: %s is already used! Create New name!", strName.c_str() );
		return;
	}

	PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( wMID, wSID );
	if ( !pCROW )
	{
		CDebugSet::MsgBox( "Invalid CROW! ( %d~%d )", wMID, wSID );
		return;
	}

	GLMobSchedule sSched;
	StringCchCopy( sSched.m_szName, GLMobSchedule::EM_MAXSZNAME, strName.c_str() );
	sSched.m_CrowID = pCROW->sNativeID;
	sSched.m_pAffineParts->vTrans = vPOS; 

	if ( pCROW->m_emCrow == CROW_MOB )
		sSched.m_fReGenTime = 20.0f;
	else 
		sSched.m_fReGenTime = 4.0f;


	m_pGLLand->GetMobSchMan()->AddMobSch( sSched );

	SGLNODE<GLMobSchedule*>* pSCHED = m_pGLLand->GetMobSchMan()->FindMobSch( sSched.m_szName );
	if ( pSCHED )
		m_pEdit = pSCHED->Data;

	if ( m_pGLLand && m_pEdit )
	{
		InitUI( TRUE );
		DataShow();
		JumpTo( m_pEdit->m_pAffineParts->vTrans );
	}
	else
	{
		m_pGLLand->GetMobSchMan()->DelMobSch( sSched.m_szName );
		CDebugSet::MsgBox( "Register New Schedule error!" );
	}
}

void CPageEditCrow::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEditCrow::OnBnClickedButtonBus()
{
	CString szFilter = "busloc|*.BUSLOC|ini|*.INI|";
	CFileDialog dlg ( TRUE,".BUSLOC",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		SetWin_Text( this, IDC_EDIT_BUS, dlg.GetFileName() );
	}
}

void CPageEditCrow::OnBnClickedButtonActionAdd()
{
	SetWin_Enable( this, IDC_COMBO_ACTION, TRUE );
	SetWin_Enable( this, IDC_EDIT_ACTION_X, TRUE );
	SetWin_Enable( this, IDC_EDIT_ACTION_Y, TRUE );
	SetWin_Enable( this, IDC_EDIT_ACTION_Z, TRUE );
	SetWin_Enable( this, IDC_EDIT_ACTION_TIME, TRUE );
	SetWin_Enable( this, IDC_EDIT_ACTION_NUM, TRUE );
	SetWin_Enable( this, IDC_BUTTON_ACTION_POS, TRUE );
	SetWin_Enable( this, IDC_BUTTON_ACTION_SAVE, TRUE );
	SetWin_Enable( this, IDC_BUTTON_ACTION_CLOSE, TRUE );

	D3DXVECTOR3 vPOS = CEditorLevelView::GetView()->m_vPos;

	SetWin_Combo_Sel( this, IDC_COMBO_ACTION, 0 );
	SetWin_Num_float( this, IDC_EDIT_ACTION_X, vPOS.x );
	SetWin_Num_float( this, IDC_EDIT_ACTION_Y, vPOS.y );
	SetWin_Num_float( this, IDC_EDIT_ACTION_Z, vPOS.z );
	SetWin_Num_float( this, IDC_EDIT_ACTION_TIME, 4.0f );
	SetWin_Num_int( this, IDC_EDIT_ACTION_NUM, -1 );

	AdjustAction();
}

void CPageEditCrow::OnBnClickedButtonActionEdit()
{
	if( !m_pGLLand )	return;
	if( !m_pEdit  )		return;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	SGLNODE<SMOBACTION>* pAction = (SGLNODE<SMOBACTION>*)m_List.GetItemData( nSelect );
	if ( pAction )
	{
		SetWin_Enable( this, IDC_COMBO_ACTION, TRUE );
		SetWin_Enable( this, IDC_EDIT_ACTION_X, TRUE );
		SetWin_Enable( this, IDC_EDIT_ACTION_Y, TRUE );
		SetWin_Enable( this, IDC_EDIT_ACTION_Z, TRUE );
		SetWin_Enable( this, IDC_EDIT_ACTION_TIME, TRUE );
		SetWin_Enable( this, IDC_EDIT_ACTION_NUM, TRUE );
		SetWin_Enable( this, IDC_BUTTON_ACTION_POS, TRUE );
		SetWin_Enable( this, IDC_BUTTON_ACTION_SAVE, TRUE );
		SetWin_Enable( this, IDC_BUTTON_ACTION_CLOSE, TRUE );

		SetWin_Combo_Sel( this, IDC_COMBO_ACTION, COMMENT::MOBACTIONS[pAction->Data.emAction].c_str() );
		SetWin_Num_float( this, IDC_EDIT_ACTION_X, pAction->Data.vPos.x );
		SetWin_Num_float( this, IDC_EDIT_ACTION_Y, pAction->Data.vPos.y );
		SetWin_Num_float( this, IDC_EDIT_ACTION_Z, pAction->Data.vPos.z );
		SetWin_Num_float( this, IDC_EDIT_ACTION_TIME, pAction->Data.fLife );
		SetWin_Num_int( this, IDC_EDIT_ACTION_NUM, nSelect );

		AdjustAction();
		
		if ( pAction->Data.emAction == EMACTION_MOVE )
			JumpTo( pAction->Data.vPos );
	}
}

void CPageEditCrow::OnBnClickedButtonActionDel()
{
	if( !m_pGLLand )	return;
	if( !m_pEdit  )		return;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	SGLNODE<SMOBACTION>* pAction = (SGLNODE<SMOBACTION>*)m_List.GetItemData( nSelect );
	if ( pAction )
	{
		m_pEdit->m_sMobActList.DELNODE( pAction );
		m_pGLLand->DropOutCrow( m_pEdit->m_dwGlobID );
		ShowActions();
	}
}

void CPageEditCrow::OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	SGLNODE<SMOBACTION>* pAction = (SGLNODE<SMOBACTION>*)m_List.GetItemData( nSelect );
	if ( pAction && pAction->Data.emAction == EMACTION_MOVE )
	{
		JumpTo( pAction->Data.vPos );
	}
}

void CPageEditCrow::OnBnClickedButtonActionClose()
{
	ShowActions();
}

void CPageEditCrow::OnCbnSelchangeComboAction()
{
	AdjustAction();
}

void CPageEditCrow::OnBnClickedButtonRebuild()
{
	if( !m_pGLLand )	return;
	if( !m_pEdit  )		return;

	CEditorLevelView::GetView()->EditSmatSet( FALSE );

	D3DXVECTOR3 vPOS = CEditorLevelView::GetView()->m_vPos;
	D3DXVECTOR3 vDis = m_pEdit->m_pAffineParts->vTrans - vPOS;
	m_pEdit->m_pAffineParts->vTrans = vPOS;
	m_pEdit->ModifierSchList( &vDis, m_pGLLand->GetNavi() );
	m_pEdit->GenerateRendGenPos( m_pGLLand->GetNavi() );
	ShowActions();

	CEditorLevelView::GetView()->EditSmatSet( TRUE );
	CEditorLevelView::GetView()->m_DxEditMat.SetAffineMatrix( m_pEdit );
}

void CPageEditCrow::OnBnClickedButtonActionPos()
{
	D3DXVECTOR3 vPOS = CEditorLevelView::GetView()->m_vPos;
	SetWin_Num_float( this, IDC_EDIT_ACTION_X, vPOS.x );
	SetWin_Num_float( this, IDC_EDIT_ACTION_Y, vPOS.y );
	SetWin_Num_float( this, IDC_EDIT_ACTION_Z, vPOS.z );
}

void CPageEditCrow::OnBnClickedButtonActionSave()
{
	if( !m_pGLLand )	return;
	if( !m_pEdit  )		return;
	if( !m_pGLLand->GetMobSchMan() )	return;

	SMOBACTION sMobAction;
	sMobAction.emAction = static_cast<EMMOBACTIONS>( GetWin_Combo_Sel( this, IDC_COMBO_ACTION ) );

	if ( sMobAction.emAction == EMACTION_ACTSTAY )
	{
		sMobAction.fLife = GetWin_Num_float( this, IDC_EDIT_ACTION_TIME );
	}
	else if ( sMobAction.emAction == EMACTION_MOVE )
	{
		sMobAction.vPos.x = GetWin_Num_float( this, IDC_EDIT_ACTION_X );
		sMobAction.vPos.y = GetWin_Num_float( this, IDC_EDIT_ACTION_Y );
		sMobAction.vPos.z = GetWin_Num_float( this, IDC_EDIT_ACTION_Z );
	}

	int nEDIT = GetWin_Num_int( this, IDC_EDIT_ACTION_NUM );
	if ( nEDIT == -1 )
	{
		m_pEdit->m_sMobActList.ADDTAIL ( sMobAction );
		m_pGLLand->DropOutCrow( m_pEdit->m_dwGlobID );
	}else{
		SGLNODE<SMOBACTION>* pAction = (SGLNODE<SMOBACTION>*)m_List.GetItemData( nEDIT );
		if ( pAction )
		{
			pAction->Data.emAction = sMobAction.emAction;
			pAction->Data.fLife = sMobAction.fLife;
			pAction->Data.vPos = sMobAction.vPos;
			m_pGLLand->DropOutCrow( m_pEdit->m_dwGlobID );
		}
	}

	ShowActions();
}
