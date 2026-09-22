
// EditorCodexDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditorCodex.h"
#include "EditorCodexDlg.h"

#include "DxResponseMan.h"
#include "RANPARAM.h"
#include "DxServerInstance.h"
#include "GLOGIC.h"

#include "GLCodex.h"
#include "HLibDataConvert.h"
#include "EtcFunction.h"

#include "GLCrowData.h"
#include "GLItemMan.h"
#include "GLItemDef.h"
#include "GLQuestMan.h"

#include "DlgFindCrow.h"
#include "DlgFindMap.h"
#include "DlgFindItem.h"
#include "DlgFindQuest.h"
#include ".\editorcodexdlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEditorCodexDlg dialog




CEditorCodexDlg::CEditorCodexDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditorCodexDlg::IDD, pParent)
	, m_bAdd(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEditorCodexDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CODEX, m_List );
}

BEGIN_MESSAGE_MAP(CEditorCodexDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_FILE_LOAD, OnBnClickedButtonFileLoad)
	ON_BN_CLICKED(IDC_BUTTON_FILE_SAVE, OnBnClickedButtonFileSave)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnBnClickedButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnBnClickedButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, OnBnClickedButtonClear)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE, OnCbnSelchangeComboType)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_CODEX, OnNMDblclkListCodex)
	ON_EN_CHANGE(IDC_EDIT_PROGRESS_MOB_KILL_MID, OnEnChangeEditProgressMobKillMid)
	ON_EN_CHANGE(IDC_EDIT_PROGRESS_MOB_KILL_SID, OnEnChangeEditProgressMobKillSid)
	ON_BN_CLICKED(IDC_BUTTON_MOB_KILL, OnBnClickedButtonMobKill)
	ON_EN_CHANGE(IDC_EDIT_PROGRESS_MAP_KILL_MID, OnEnChangeEditProgressMapKillMid)
	ON_EN_CHANGE(IDC_EDIT_PROGRESS_MAP_KILL_SID, OnEnChangeEditProgressMapKillSid)
	ON_BN_CLICKED(IDC_BUTTON_MAP_KILL, OnBnClickedButtonMapKill)
	ON_EN_CHANGE(IDC_EDIT_PROGRESS_MAP_REACH_MID, OnEnChangeEditProgressMapReachMid)
	ON_EN_CHANGE(IDC_EDIT_PROGRESS_MAP_REACH_SID, OnEnChangeEditProgressMapReachSid)
	ON_BN_CLICKED(IDC_BUTTON_MAP_REACH, OnBnClickedButtonMapReach)
	ON_EN_CHANGE(IDC_EDIT_PROGRESS_TAKE_ITEM_MID, OnEnChangeEditProgressTakeItemMid)
	ON_EN_CHANGE(IDC_EDIT_PROGRESS_TAKE_ITEM_SID, OnEnChangeEditProgressTakeItemSid)
	ON_BN_CLICKED(IDC_BUTTON_TAKE_ITEM, OnBnClickedButtonTakeItem)
	ON_EN_CHANGE(IDC_EDIT_PROGRESS_USE_ITEM_MID, OnEnChangeEditProgressUseItemMid)
	ON_EN_CHANGE(IDC_EDIT_PROGRESS_USE_ITEM_SID, OnEnChangeEditProgressUseItemSid)
	ON_BN_CLICKED(IDC_BUTTON_USE_ITEM, OnBnClickedButtonUseItem)
	ON_BN_CLICKED(IDC_BUTTON_COMPLETE_QUEST, OnBnClickedButtonCompleteQuest)
	ON_EN_CHANGE(IDC_EDIT_PROGRESS_COMPLETE_QUEST_ID, OnEnChangeEditProgressCompleteQuestId)
	ON_BN_CLICKED(IDC_BUTTON_FILE_IMPORT, OnBnClickedButtonFileImport)
	ON_BN_CLICKED(IDC_BUTTON_FILE_CSV_SAVE, OnBnClickedButtonFileCsvSave)
	ON_BN_CLICKED(IDC_BUTTON_FILE_CSV_LOAD, OnBnClickedButtonFileCsvLoad)
	ON_EN_CHANGE(IDC_EDIT_TOTAL_CODEX1, OnEnChangeEditTotalCodex1)
END_MESSAGE_MAP()


// CEditorCodexDlg message handlers

BOOL CEditorCodexDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "ID", LVCFMT_LEFT, ( 7* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Type", LVCFMT_LEFT, ( 13* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 2, "Title", LVCFMT_LEFT, ( 25* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 3, "Badge", LVCFMT_LEFT, ( 13* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 4, "Point", LVCFMT_LEFT, ( 8* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 5, "Info", LVCFMT_LEFT, ( 24* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 6, "Notify", LVCFMT_LEFT, ( 8* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 7, "", LVCFMT_LEFT, ( 2* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	CString strAppPath;
	TCHAR szPath[MAX_PATH] = {0};
	GetModuleFileName(::AfxGetInstanceHandle(), szPath, MAX_PATH);
	strAppPath = szPath;

	if ( !strAppPath.IsEmpty() )
	{
		DWORD dwFind = strAppPath.ReverseFind ( '\\' );
		if ( dwFind != -1 )
		{
			m_strAppPath = strAppPath.Left ( dwFind );

			if ( !m_strAppPath.IsEmpty() )
				if ( m_strAppPath.GetAt(0) == '"' )
					m_strAppPath = m_strAppPath.Right ( m_strAppPath.GetLength() - 1 );

			strcpy ( m_szAppPath, m_strAppPath.GetString() );
		}
	}

	RANPARAM::LOAD( m_szAppPath );
	DxResponseMan::GetInstance().OneTimeSceneInit ( m_szAppPath, this, RANPARAM::strFontType, RANPARAM::dwLangSet, RANPARAM::strGDIFont );
	GMTOOL::Create ( m_szAppPath );

	GLCodex::GetInstance().LoadFile( "Codex.asf", FALSE );

	m_mapList.CleanUp();
	m_mapList.LoadMapsListFile( "mapslist.mst" );

	SetWin_Combo_Init( this, IDC_COMBO_TYPE, COMMENT::CODEX_MID, EMCODEX_TYPE_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_NOTIFY, COMMENT::CODEX_NOTIFY, EMCODEX_NOTIFY_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_USE_QBOX, COMMENT::ITEM_QUE_TYPE, QUESTION_SIZE );

	DataShow();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEditorCodexDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEditorCodexDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CEditorCodexDlg::PostNcDestroy()
{
	GLCodex::GetInstance().CleanUp();
	DxResponseMan::GetInstance().FinalCleanup();
	GMTOOL::CleanUp ();
	CDialog::PostNcDestroy();
}

BOOL CEditorCodexDlg::DataShow()
{
	m_List.DeleteAllItems();
	m_List.SetRedraw( FALSE );

	DWORD dwTotal = 0;

	DWORD m_dwHPIncreaseDummy = 0;
	DWORD m_dwMPIncreaseDummy = 0;
	DWORD m_dwSPIncreaseDummy = 0;
	DWORD m_dwAttackIncreaseDummy = 0;
	DWORD m_dwDefenseIncreaseDummy = 0;
	DWORD m_dwShootingIncreaseDummy = 0;
	DWORD m_dwMeleeIncreaseDummy = 0;
	DWORD m_dwEnergyIncreaseDummy = 0;
	DWORD m_dwResistanceIncreaseDummy = 0;
	DWORD m_dwHitrateIncreaseDummy = 0;
	DWORD m_dwAvoidrateIncreaseDummy = 0;	

	SCODEX_FILE_DATA_MAP &map_codex = GLCodex::GetInstance().m_mapCodex;
	int i=0;
	for( SCODEX_FILE_DATA_MAP_ITER iter = map_codex.begin(); 
		iter != map_codex.end(); iter++, i++ )
	{
		const SCODEX_FILE_DATA &sdata = (*iter).second;

		m_List.InsertItem( i, _HLIB::cstring_dword(sdata.dwCodexID).GetString() );
		m_List.SetItemText( i, 1, COMMENT::CODEX_MID[sdata.emType].c_str() );
		m_List.SetItemText( i, 2, sdata.strCodexTitle.c_str() );
		m_List.SetItemText( i, 3, _HLIB::cstringformat( "(%d) %s", sdata.bRewardBadge, sdata.strBadgeString.c_str() ).GetString() );
		m_List.SetItemText( i, 4, _HLIB::cstring_dword(sdata.dwRewardPoint).GetString() );
		//m_List.SetItemText( i, 2, "Complete item required." );
		
		SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sdata.sidMapKill );
			m_List.SetItemText( i, 5, _HLIB::cstringformat( "[%03d~%03d][%03d]%s", 
				sdata.sidItemGet.wMainID, sdata.sidItemGet.wSubID, sdata.wProgressItemGet, 
				pitem_data? pitem_data->GetName() : "Unknown Item" ).GetString() );

		/*if ( sdata.emType == EMCODEX_TYPE_REACH_LEVEL )
		{
			m_List.SetItemText( i, 5, _HLIB::cstring_dword(sdata.wProgressLevel).GetString() );
		}
		else if ( sdata.emType == EMCODEX_TYPE_KILL_MOB )
		{
			SCROWDATA* pcrow_data = GLCrowDataMan::GetInstance().GetCrowData( sdata.sidMobKill );
			m_List.SetItemText( i, 5, _HLIB::cstringformat( "[%03d~%03d][%03d]%s", 
				sdata.sidMobKill.wMainID, sdata.sidMobKill.wSubID, sdata.wProgressMobKill, 
				pcrow_data? pcrow_data->GetName() : "unknown crow" ).GetString() );
		}
		else if ( sdata.emType == EMCODEX_TYPE_KILL_PLAYER )
		{
			SMAPNODE* pmapnode = m_mapList.FindMapNode( sdata.sidMapKill );
			m_List.SetItemText( i, 5, _HLIB::cstringformat( "[%03d~%03d][%03d]%s", 
				sdata.sidMapKill.wMainID, sdata.sidMapKill.wSubID, sdata.wProgressMapKill, 
				pmapnode? pmapnode->strMapName.c_str() : "unknown map" ).GetString() );
		}
		else if ( sdata.emType == EMCODEX_TYPE_REACH_MAP )
		{
			SMAPNODE* pmapnode = m_mapList.FindMapNode( sdata.sidMapReach );
			m_List.SetItemText( i, 5, _HLIB::cstringformat( "[%03d~%03d][%03d]%s", 
				sdata.sidMapReach.wMainID, sdata.sidMapReach.wSubID, sdata.wProgressMapReach, 
				pmapnode? pmapnode->strMapName.c_str() : "unknown map" ).GetString() );
		}
		else if ( sdata.emType == EMCODEX_TYPE_TAKE_ITEM )
		{
			SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sdata.sidItemGet );
			m_List.SetItemText( i, 5, _HLIB::cstringformat( "[%03d~%03d][%03d]%s", 
				sdata.sidItemGet.wMainID, sdata.sidItemGet.wSubID, sdata.wProgressItemGet, 
				pitem_data? pitem_data->GetName() : "unknown item" ).GetString() );
		}
		else if ( sdata.emType == EMCODEX_TYPE_USE_ITEM )
		{
			SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sdata.sidItemUse );
			m_List.SetItemText( i, 5, _HLIB::cstringformat( "[%03d~%03d][%03d]%s", 
				sdata.sidItemUse.wMainID, sdata.sidItemUse.wSubID, sdata.wProgressItemUse, 
				pitem_data? pitem_data->GetName() : "unknown item" ).GetString() );
		}
		else if ( sdata.emType == EMCODEX_TYPE_QUESTION_BOX )
		{
			m_List.SetItemText( i, 5, _HLIB::cstringformat( "[%d][%s]", sdata.wQBoxProgress,
				COMMENT::ITEM_QUE_TYPE[sdata.dwQBoxType].c_str() ).GetString() );
		}
		else if ( sdata.emType == EMCODEX_TYPE_COMPLETE_QUEST )
		{
			GLQUEST* pQuest = GLQuestMan::GetInstance().Find( sdata.dwQuestID );
			m_List.SetItemText( i, 5, _HLIB::cstringformat( "[%03d][%03d]%s", 
				sdata.dwQuestID, sdata.wQuestProgress, 
				pQuest? pQuest->m_strTITLE.c_str() : "unknown quest" ).GetString() );
		}
		else if ( sdata.emType == EMCODEX_TYPE_CODEX_POINT )
		{
			m_List.SetItemText( i, 5, _HLIB::cstring_dword(sdata.dwCodexProgress).GetString() );
		}*/

		if ( sdata.emType == EMCODEX_TYPE_REACH_LEVEL )
			m_dwHPIncreaseDummy += sdata.dwRewardPoint;
		if ( sdata.emType ==EMCODEX_TYPE_KILL_MOB )
			m_dwMPIncreaseDummy += sdata.dwRewardPoint;
		if ( sdata.emType ==EMCODEX_TYPE_KILL_PLAYER )
			m_dwSPIncreaseDummy += sdata.dwRewardPoint;
		if ( sdata.emType ==EMCODEX_TYPE_REACH_MAP )
			m_dwAttackIncreaseDummy += sdata.dwRewardPoint;
		if ( sdata.emType ==EMCODEX_TYPE_TAKE_ITEM )
			m_dwDefenseIncreaseDummy += sdata.dwRewardPoint;
		if ( sdata.emType ==EMCODEX_TYPE_USE_ITEM )
			m_dwShootingIncreaseDummy += sdata.dwRewardPoint;
		if ( sdata.emType ==EMCODEX_TYPE_REACH_CODEX )
			m_dwMeleeIncreaseDummy += sdata.dwRewardPoint;
		if ( sdata.emType ==EMCODEX_TYPE_COMPLETE_QUEST )
			m_dwEnergyIncreaseDummy += sdata.dwRewardPoint;
		if ( sdata.emType ==EMCODEX_TYPE_CODEX_POINT )
			m_dwResistanceIncreaseDummy += sdata.dwRewardPoint;
		if ( sdata.emType ==EMCODEX_TYPE_QUESTION_BOX )
			m_dwHitrateIncreaseDummy += sdata.dwRewardPoint;
		if ( sdata.emType ==EMCODEX_TYPE_ETC )
			m_dwAvoidrateIncreaseDummy += sdata.dwRewardPoint;

		m_List.SetItemText( i, 6, COMMENT::CODEX_NOTIFY[sdata.emNotify].c_str() );
		m_List.SetItemData( i, sdata.dwCodexID );

		dwTotal += sdata.dwRewardPoint;
	}

	m_List.SetRedraw( TRUE );

	SCODEX_FILE_DATA scodex;
	CodexShow( scodex, FALSE, FALSE );

	SetWin_Num_int( this, IDC_EDIT_TOTAL_CODEX, dwTotal );
	
	SetWin_Num_int( this, IDC_EDIT_TOTAL_CODEX1	, m_dwHPIncreaseDummy );
	SetWin_Num_int( this, IDC_EDIT_TOTAL_CODEX2	, m_dwMPIncreaseDummy );
	SetWin_Num_int( this, IDC_EDIT_TOTAL_CODEX3	, m_dwSPIncreaseDummy );
	SetWin_Num_int( this, IDC_EDIT_TOTAL_CODEX4	, m_dwAttackIncreaseDummy );
	SetWin_Num_int( this, IDC_EDIT_TOTAL_CODEX5	, m_dwDefenseIncreaseDummy );
	SetWin_Num_int( this, IDC_EDIT_TOTAL_CODEX6	, m_dwShootingIncreaseDummy );
	SetWin_Num_int( this, IDC_EDIT_TOTAL_CODEX7	, m_dwMeleeIncreaseDummy );
	SetWin_Num_int( this, IDC_EDIT_TOTAL_CODEX8	, m_dwEnergyIncreaseDummy );
	SetWin_Num_int( this, IDC_EDIT_TOTAL_CODEX9	, m_dwResistanceIncreaseDummy );
	SetWin_Num_int( this, IDC_EDIT_TOTAL_CODEX10	, m_dwHitrateIncreaseDummy );
	SetWin_Num_int( this, IDC_EDIT_TOTAL_CODEX11	, m_dwAvoidrateIncreaseDummy );


	return TRUE;
}

void CEditorCodexDlg::OnBnClickedButtonFileLoad()
{
	CString szFilter = "Codex Setting File (*.asf)|*.asf|";
	CFileDialog dlg( TRUE, ".asf", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr = GLCodex::GetInstance().LoadFile ( dlg.GetFileName().GetString(), FALSE );
		if ( FAILED ( hr ) )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to load File" );
		}

		DataShow();
	}
}

void CEditorCodexDlg::OnBnClickedButtonFileSave()
{
	CString szFilter = "Codex Setting File (*.asf)|*.asf|";
	CFileDialog dlg( FALSE, ".asf", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr = GLCodex::GetInstance().SaveFile ( dlg.GetFileName().GetString() );
		if ( FAILED ( hr ) )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to Save File" );
			return;
		}
	}
}

void CEditorCodexDlg::OnBnClickedButtonAdd()
{
	SCODEX_FILE_DATA_MAP &map_codex = GLCodex::GetInstance().m_mapCodex;

	DWORD dwNewKey = UINT_MAX;
	for( DWORD dwKey = 0; dwKey<GLCodex::CODEX_MAXID; ++dwKey )
	{
		SCODEX_FILE_DATA_MAP_ITER iterpos = map_codex.find(dwKey);
		if ( iterpos == map_codex.end() )
		{
			dwNewKey = dwKey;
			break;
		}
	}

	if ( dwNewKey == UINT_MAX || dwNewKey >= GLCodex::CODEX_MAXID )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Unable to generate new ID" );
		return;
	}

	SCODEX_FILE_DATA scodex;
	scodex.dwCodexID = dwNewKey;
	CodexShow( scodex, TRUE, TRUE );
}

void CEditorCodexDlg::OnBnClickedButtonEdit()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = m_List.GetItemData( nSelect );

	SCODEX_FILE_DATA_MAP &map_codex = GLCodex::GetInstance().m_mapCodex;
	SCODEX_FILE_DATA_MAP_ITER iterpos = map_codex.find(dwID);
	if ( iterpos != map_codex.end() )
	{
		SCODEX_FILE_DATA &scodex = (*iterpos).second;
		CodexShow( scodex, TRUE, FALSE );
	}
}

void CEditorCodexDlg::OnBnClickedButtonDelete()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = m_List.GetItemData( nSelect );

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete Codex ID:%02d", dwID ) !=  IDYES )
		return;

	SCODEX_FILE_DATA_MAP &map_codex = GLCodex::GetInstance().m_mapCodex;
	SCODEX_FILE_DATA_MAP_ITER iterpos = map_codex.find(dwID);
	if ( iterpos != map_codex.end() )
	{
		map_codex.erase( iterpos );
		DataShow();
	}

	int nNewSelect = nSelect > 1? nSelect-1:nSelect;
	m_List.EnsureVisible( nNewSelect, TRUE );
	m_List.SetItemState( nNewSelect, LVIS_SELECTED, LVIS_SELECTED );
}

void CEditorCodexDlg::OnBnClickedButtonClear()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Clear Codex?" ) ==  IDYES )
	{
		GLCodex::GetInstance().m_mapCodex.clear();
		DataShow();
	}
}

void CEditorCodexDlg::OnBnClickedButtonSave()
{
	SCODEX_FILE_DATA scodex_data;

	scodex_data.dwCodexID = GetWin_Num_int( this, IDC_EDIT_ID );
	scodex_data.strCodexTitle = GetWin_Text( this, IDC_EDIT_TITLE ).GetString();
	scodex_data.strDescription = GetWin_Text( this, IDC_EDIT_DESC ).GetString();

	scodex_data.emNotify = static_cast<EMCODEX_NOTIFY> ( GetWin_Combo_Sel( this, IDC_COMBO_NOTIFY ) );
	
	scodex_data.bRewardBadge = GetWin_Check( this, IDC_CHECK_BADGE );
	scodex_data.strBadgeString = GetWin_Text( this, IDC_EDIT_BADGE ).GetString();
	scodex_data.dwRewardPoint = GetWin_Num_int( this, IDC_EDIT_REWARD_POINT );

	scodex_data.emType = static_cast<EMCODEX_TYPE> ( GetWin_Combo_Sel( this, IDC_COMBO_TYPE ) );

	//level
	scodex_data.wProgressLevel = GetWin_Num_int( this, IDC_EDIT_PROGRESS_LEVEL );

	//mobkill
	scodex_data.sidMobKill.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_MID );
	scodex_data.sidMobKill.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_SID );
	scodex_data.wProgressMobKill = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_NUM );

	//mapkill
	scodex_data.sidMapKill.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_MID );
	scodex_data.sidMapKill.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_SID );
	scodex_data.wProgressMapKill = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_NUM );

	//map reach
	scodex_data.sidMapReach.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_MID );
	scodex_data.sidMapReach.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_SID );
	scodex_data.wProgressMapReach = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_NUM );

	//take item
	scodex_data.sidItemGet.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_MID );
	scodex_data.sidItemGet.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_SID );
	scodex_data.wProgressItemGet = GetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_NUM );

	//use item
	scodex_data.sidItemUse.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_MID );
	scodex_data.sidItemUse.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_SID );
	scodex_data.wProgressItemUse = GetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_NUM );

	//obtain qbox
	scodex_data.dwQBoxType = GetWin_Combo_Sel( this, IDC_COMBO_USE_QBOX );
	scodex_data.wQBoxProgress = GetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_QBOX );

	//complete quest
	scodex_data.dwQuestID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_COMPLETE_QUEST_ID );
	scodex_data.wQuestProgress = GetWin_Num_int( this, IDC_EDIT_PROGRESS_COMPLETE_QUEST_NUM );

	//codex point
	scodex_data.dwCodexProgress = GetWin_Num_int( this, IDC_EDIT_PROGRESS_CODEX_POINT );
	
	
	//codex grade
	scodex_data.wItemGrade1 = GetWin_Num_int( this, IDC_EDIT_PROGRESS_GRADE1 );
	scodex_data.wItemGrade2 = GetWin_Num_int( this, IDC_EDIT_PROGRESS_GRADE2 );
	scodex_data.wItemGrade3 = GetWin_Num_int( this, IDC_EDIT_PROGRESS_GRADE3 );
	scodex_data.wItemGrade4 = GetWin_Num_int( this, IDC_EDIT_PROGRESS_GRADE4 );
	scodex_data.wItemGrade5 = GetWin_Num_int( this, IDC_EDIT_PROGRESS_GRADE5 );
	

	if ( scodex_data.dwCodexID == UINT_MAX || scodex_data.dwCodexID >= GLCodex::CODEX_MAXID )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid ID %d", scodex_data.dwCodexID );
		return;
	}

	if ( scodex_data.strCodexTitle.empty() )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Title Empty!" );
		return;
	}

	if ( scodex_data.bRewardBadge && scodex_data.strBadgeString.empty() )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Badge Empty!" );
		return;
	}

	if ( scodex_data.emType == EMCODEX_TYPE_REACH_LEVEL )
	{
		if ( scodex_data.wProgressLevel < 1 )
		{
			//CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Level %d", scodex_data.wProgressLevel );
			//return;
		}
	}

	if ( scodex_data.emType == EMCODEX_TYPE_KILL_MOB )
	{
		SCROWDATA* pcrowdata = GLCrowDataMan::GetInstance().GetCrowData( scodex_data.sidMobKill );
		if ( !pcrowdata )
		{
			//CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Crow %d~%d", scodex_data.sidMobKill.wMainID, scodex_data.sidMobKill.wSubID );
			//return;
		}

		if ( scodex_data.wProgressMobKill < 1 )
		{
			//CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Mob Num %d", scodex_data.wProgressMobKill );
			//return;
		}
	}

	if ( scodex_data.emType == EMCODEX_TYPE_KILL_PLAYER )
	{
		SMAPNODE* pmap_node = m_mapList.FindMapNode( scodex_data.sidMapKill );
		if ( !pmap_node )
		{
			//CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Map %d~%d", scodex_data.sidMapKill.wMainID, scodex_data.sidMapKill.wSubID );
			//return;
		}

		if ( scodex_data.wProgressMapKill < 1 )
		{
			//CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Kill Num %d", scodex_data.wProgressMobKill );
			//return;
		}
	}

	if ( scodex_data.emType == EMCODEX_TYPE_REACH_MAP )
	{
		SMAPNODE* pmap_node = m_mapList.FindMapNode( scodex_data.sidMapReach );
		if ( !pmap_node )
		{
			//CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Map %d~%d", scodex_data.sidMapReach.wMainID, scodex_data.sidMapReach.wSubID );
			//return;
		}

		if ( scodex_data.wProgressMapReach < 1 )
		{
			//CDebugSet::MsgBox( GetSafeHwnd(), "Invalid reach Num %d", scodex_data.wProgressMapReach);
			//return;
		}
	}

	if ( scodex_data.emType == EMCODEX_TYPE_TAKE_ITEM )
	{
		SITEM* pitem_data = GLItemMan::GetInstance().GetItem( scodex_data.sidItemGet );
		if ( !pitem_data )
		{
			//CDebugSet::MsgBox( GetSafeHwnd(), "Invalid item %d~%d", scodex_data.sidItemGet.wMainID, scodex_data.sidItemGet.wSubID );
			//return;
		}

		if ( scodex_data.wProgressItemGet < 1 )
		{
			//CDebugSet::MsgBox( GetSafeHwnd(), "Invalid item Num %d", scodex_data.wProgressItemGet);
			//return;
		}
	}

	if ( scodex_data.emType == EMCODEX_TYPE_USE_ITEM )
	{
		SITEM* pitem_data = GLItemMan::GetInstance().GetItem( scodex_data.sidItemUse );
		if ( !pitem_data )
		{
			//CDebugSet::MsgBox( GetSafeHwnd(), "Invalid item %d~%d", scodex_data.sidItemUse.wMainID, scodex_data.sidItemUse.wSubID );
			//return;
		}

		if ( scodex_data.wProgressItemUse < 1 )
		{
			//CDebugSet::MsgBox( GetSafeHwnd(), "Invalid item Num %d", scodex_data.wProgressItemUse);
			//return;
		}
	}

	if ( scodex_data.emType == EMCODEX_TYPE_QUESTION_BOX )
	{
		if ( scodex_data.wQBoxProgress < 1 )
		{
			//CDebugSet::MsgBox( GetSafeHwnd(), "Invalid qbox Num %d", scodex_data.wQBoxProgress );
			//return;
		}
	}

	if ( scodex_data.emType == EMCODEX_TYPE_COMPLETE_QUEST )
	{
		GLQUEST* pquest = GLQuestMan::GetInstance().Find( scodex_data.dwQuestID );
		if ( !pquest )
		{
			//CDebugSet::MsgBox( GetSafeHwnd(), "Invalid quest %d", scodex_data.dwQuestID );
			//return;
		}

		if ( scodex_data.wQuestProgress < 1 )
		{
			//CDebugSet::MsgBox( GetSafeHwnd(), "Invalid quest Num %d", scodex_data.wQuestProgress);
			//return;
		}
	}

	if ( scodex_data.emType == EMCODEX_TYPE_CODEX_POINT )
	{
		if ( scodex_data.dwCodexProgress < 1 )
		{
			//CDebugSet::MsgBox( GetSafeHwnd(), "Invalid codex point %d", scodex_data.dwCodexProgress );
			//return;
		}
	}

	SCODEX_FILE_DATA_MAP &map_codex = GLCodex::GetInstance().m_mapCodex;

	if ( m_bAdd )
	{
		SCODEX_FILE_DATA_MAP_ITER iterpos = map_codex.find(scodex_data.dwCodexID);
		if ( iterpos != map_codex.end() )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "ID Exist ID:%d", scodex_data.dwCodexID );
			return;
		}

		map_codex.insert( std::make_pair( scodex_data.dwCodexID, scodex_data ) );
		DataShow();
		CodexShow( scodex_data, TRUE, FALSE );
	}
	else
	{
		SCODEX_FILE_DATA_MAP_ITER iterpos = map_codex.find(scodex_data.dwCodexID);
		if ( iterpos != map_codex.end() )
		{
			SCODEX_FILE_DATA* pData = &(*iterpos).second;
			*pData = scodex_data;

			DataShow();
			CodexShow( *pData, TRUE, FALSE );
		}else{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to save ID :%d", scodex_data.dwCodexID );

		}
	}

	for( int nItem = 0; nItem < m_List.GetItemCount(); nItem++)
	{
		DWORD dwID = m_List.GetItemData( nItem );
		if( dwID == scodex_data.dwCodexID )
		{
			m_List.EnsureVisible( nItem, TRUE );
			m_List.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
		}
	}
}

void CEditorCodexDlg::OnCbnSelchangeComboType()
{
	EMCODEX_TYPE emType = static_cast<EMCODEX_TYPE> ( GetWin_Combo_Sel( this, IDC_COMBO_TYPE ) );
	BOOL bReachLevel = BOOL( emType == EMCODEX_TYPE_REACH_LEVEL );
	BOOL bKillMob = true;//BOOL( emType== EMCODEX_TYPE_KILL_MOB );
	BOOL bKillPlayer = true;//BOOL( emType == EMCODEX_TYPE_KILL_PLAYER );
	BOOL bReachMap = true;//BOOL( emType == EMCODEX_TYPE_REACH_MAP );
	BOOL bTakeItem = true;//BOOL( emType == EMCODEX_TYPE_TAKE_ITEM );
	BOOL bUseItem = true;//BOOL( emType == EMCODEX_TYPE_USE_ITEM );
	BOOL bUseQBox = BOOL( emType == EMCODEX_TYPE_QUESTION_BOX );
	BOOL bCompleteQuest = BOOL( emType == EMCODEX_TYPE_COMPLETE_QUEST );
	BOOL bReachCodexPoint = BOOL ( emType == EMCODEX_TYPE_CODEX_POINT );

	SetWin_Enable( this, IDC_EDIT_PROGRESS_LEVEL, bReachLevel );

	SetWin_Enable( this, IDC_BUTTON_MOB_KILL, bKillMob );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MOB_KILL_MID, bKillMob );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MOB_KILL_SID, bKillMob );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MOB_KILL_NUM, bKillMob );

	SetWin_Enable( this, IDC_BUTTON_MAP_KILL, bKillPlayer );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MAP_KILL_MID, bKillPlayer );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MAP_KILL_SID, bKillPlayer );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MAP_KILL_NUM, bKillPlayer );

	SetWin_Enable( this, IDC_BUTTON_MAP_REACH, bReachMap );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MAP_REACH_MID, bReachMap );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MAP_REACH_SID, bReachMap );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MAP_REACH_NUM, bReachMap );

	SetWin_Enable( this, IDC_BUTTON_TAKE_ITEM, bTakeItem );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_TAKE_ITEM_MID, bTakeItem );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_TAKE_ITEM_SID, bTakeItem );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_TAKE_ITEM_NUM, bTakeItem );

	SetWin_Enable( this, IDC_BUTTON_USE_ITEM, bUseItem );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_USE_ITEM_MID, bUseItem );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_USE_ITEM_SID, bUseItem );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_USE_ITEM_NUM, bUseItem );

	SetWin_Enable( this, IDC_COMBO_USE_QBOX, bUseQBox );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_USE_QBOX, bUseQBox );

	SetWin_Enable( this, IDC_BUTTON_COMPLETE_QUEST, bCompleteQuest );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_COMPLETE_QUEST_ID, bCompleteQuest );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_COMPLETE_QUEST_NUM, bCompleteQuest );
	
	//Codex grade
	SetWin_Enable( this, IDC_EDIT_PROGRESS_GRADE1, true );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_GRADE2, true );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_GRADE3, true );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_GRADE4, true );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_GRADE5, true );

	SetWin_Enable( this, IDC_EDIT_PROGRESS_CODEX_POINT, bReachCodexPoint );
}

void CEditorCodexDlg::CodexShow( SCODEX_FILE_DATA &sdata, BOOL bEnable, BOOL bAdd )
{
	m_bAdd = bAdd;

	SetWin_Num_int( this, IDC_EDIT_ID, sdata.dwCodexID );
	SetWin_Text( this, IDC_EDIT_TITLE, sdata.strCodexTitle.c_str() );
	SetWin_Text( this, IDC_EDIT_DESC, sdata.strDescription.c_str() );
	
	SetWin_Combo_Sel( this, IDC_COMBO_NOTIFY, sdata.emNotify );

	SetWin_Check( this, IDC_CHECK_BADGE, sdata.bRewardBadge );
	SetWin_Text( this, IDC_EDIT_BADGE, sdata.strBadgeString.c_str() );
	SetWin_Num_int( this, IDC_EDIT_REWARD_POINT, sdata.dwRewardPoint );

	SetWin_Combo_Sel( this, IDC_COMBO_TYPE, sdata.emType );

	//level
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_LEVEL, sdata.wProgressLevel );

	//mobkill
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_MID, sdata.sidMobKill.wMainID );
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_SID, sdata.sidMobKill.wSubID );
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_NUM, sdata.wProgressMobKill );

	//mapkill
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_MID, sdata.sidMapKill.wMainID );
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_SID, sdata.sidMapKill.wSubID );
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_NUM, sdata.wProgressMapKill );

	//mapreach
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_MID, sdata.sidMapReach.wMainID );
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_SID, sdata.sidMapReach.wSubID );
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_NUM, sdata.wProgressMapReach );

	//take item
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_MID, sdata.sidItemGet.wMainID );
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_SID, sdata.sidItemGet.wSubID );
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_NUM, sdata.wProgressItemGet );

	//use item
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_MID, sdata.sidItemUse.wMainID );
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_SID, sdata.sidItemUse.wSubID );
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_NUM, sdata.wProgressItemUse );

	//use qbox
	SetWin_Combo_Sel( this, IDC_COMBO_USE_QBOX, sdata.dwQBoxType );
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_QBOX, sdata.wQBoxProgress );

	//complete quest
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_COMPLETE_QUEST_ID, sdata.dwQuestID );
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_COMPLETE_QUEST_NUM, sdata.wQuestProgress );

	//codex point
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_CODEX_POINT, sdata.dwCodexProgress );
	
	//codex grade
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_GRADE1, sdata.wItemGrade1 );
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_GRADE2, sdata.wItemGrade2 );
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_GRADE3, sdata.wItemGrade3 );
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_GRADE4, sdata.wItemGrade4 );
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_GRADE5, sdata.wItemGrade5 );


	SetWin_Enable( this, IDC_EDIT_ID, bEnable && bAdd );
	SetWin_Enable( this, IDC_EDIT_TITLE, bEnable );
	SetWin_Enable( this, IDC_EDIT_DESC, bEnable );

	SetWin_Enable( this, IDC_COMBO_NOTIFY, bEnable );

	SetWin_Enable( this, IDC_CHECK_BADGE, bEnable );
	SetWin_Enable( this, IDC_EDIT_BADGE, bEnable );
	SetWin_Enable( this, IDC_EDIT_REWARD_POINT, bEnable );

	SetWin_Enable( this, IDC_COMBO_TYPE, bEnable );

	//level
	SetWin_Enable( this, IDC_EDIT_PROGRESS_LEVEL, bEnable );

	//mobkill
	SetWin_Enable( this, IDC_BUTTON_MOB_KILL, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MOB_KILL_MID, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MOB_KILL_SID, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MOB_KILL_NUM, bEnable );

	//mapkill
	SetWin_Enable( this, IDC_BUTTON_MAP_KILL, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MAP_KILL_MID, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MAP_KILL_SID, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MAP_KILL_NUM, bEnable );

	//mapreach
	SetWin_Enable( this, IDC_BUTTON_MAP_REACH, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MAP_REACH_MID, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MAP_REACH_SID, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_MAP_REACH_NUM, bEnable );

	//take item
	SetWin_Enable( this, IDC_BUTTON_TAKE_ITEM, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_TAKE_ITEM_MID, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_TAKE_ITEM_SID, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_TAKE_ITEM_NUM, bEnable );

	//use item
	SetWin_Enable( this, IDC_BUTTON_USE_ITEM, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_USE_ITEM_MID, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_USE_ITEM_SID, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_USE_ITEM_NUM, bEnable );

	//use qbox
	SetWin_Enable( this, IDC_COMBO_USE_QBOX, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_USE_QBOX, bEnable );

	//complete quest
	SetWin_Enable( this, IDC_BUTTON_COMPLETE_QUEST, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_COMPLETE_QUEST_ID, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_COMPLETE_QUEST_NUM, bEnable );

	//codex point
	SetWin_Enable( this, IDC_EDIT_PROGRESS_CODEX_POINT, bEnable );

	SetWin_Enable( this, IDC_BUTTON_SAVE, bEnable );
	
	//Codex grade
	SetWin_Enable( this, IDC_EDIT_PROGRESS_GRADE1, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_GRADE2, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_GRADE3, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_GRADE4, bEnable );
	SetWin_Enable( this, IDC_EDIT_PROGRESS_GRADE5, bEnable );
	

	if ( bEnable )	OnCbnSelchangeComboType();
}

void CEditorCodexDlg::OnNMDblclkListCodex(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = m_List.GetItemData( nSelect );

	SCODEX_FILE_DATA_MAP &map_codex = GLCodex::GetInstance().m_mapCodex;
	SCODEX_FILE_DATA_MAP_ITER iterpos = map_codex.find(dwID);
	if ( iterpos != map_codex.end() )
	{
		SCODEX_FILE_DATA &scodex = (*iterpos).second;
		CodexShow( scodex, TRUE, FALSE );
	}
}

void CEditorCodexDlg::OnEnChangeEditProgressMobKillMid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_SID );

	//SCROWDATA* pcrowdata = GLCrowDataMan::GetInstance().GetCrowData( sid );
	//SetWin_Text( this, IDC_EDIT_PROGRESS_MOB_KILL_NAME, pcrowdata? pcrowdata->GetName(): "unknown item" );
	SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_MOB_KILL_NAME, pitem_data? pitem_data->GetName(): "unknown item" );
}

void CEditorCodexDlg::OnEnChangeEditProgressMobKillSid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_SID );

	//SCROWDATA* pcrowdata = GLCrowDataMan::GetInstance().GetCrowData( sid );
	//SetWin_Text( this, IDC_EDIT_PROGRESS_MOB_KILL_NAME, pcrowdata? pcrowdata->GetName(): "unknown item" );
	SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_MOB_KILL_NAME, pitem_data? pitem_data->GetName(): "unknown item" );
}

void CEditorCodexDlg::OnBnClickedButtonMobKill()
{
	CDlgFindItem dlg(this);
	if ( dlg.DoModal() == IDOK )
	{
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_MID, dlg.m_sSELECTED.wMainID );
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_SID, dlg.m_sSELECTED.wSubID );
	}
}

void CEditorCodexDlg::OnEnChangeEditProgressMapKillMid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_SID );

	//SMAPNODE* pmap_node = m_mapList.FindMapNode( sid );
	//SetWin_Text( this, IDC_EDIT_PROGRESS_MAP_KILL_NAME, pmap_node? pmap_node->strMapName.c_str(): "unknown item" );
	
	SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_MAP_KILL_NAME, pitem_data? pitem_data->GetName(): "unknown item" );
}

void CEditorCodexDlg::OnEnChangeEditProgressMapKillSid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_SID );

	//SMAPNODE* pmap_node = m_mapList.FindMapNode( sid );
	//SetWin_Text( this, IDC_EDIT_PROGRESS_MAP_KILL_NAME, pmap_node? pmap_node->strMapName.c_str(): "unknown item" );
	
	SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_MAP_KILL_NAME, pitem_data? pitem_data->GetName(): "unknown item" );
}

void CEditorCodexDlg::OnBnClickedButtonMapKill()
{
	CDlgFindItem dlg(this);
	if ( dlg.DoModal() == IDOK )
	{
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_MID, dlg.m_sSELECTED.wMainID );
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_SID, dlg.m_sSELECTED.wSubID );
	}
}

void CEditorCodexDlg::OnEnChangeEditProgressMapReachMid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_SID );

	//SMAPNODE* pmap_node = m_mapList.FindMapNode( sid );
	//SetWin_Text( this, IDC_EDIT_PROGRESS_MAP_REACH_NAME, pmap_node? pmap_node->strMapName.c_str(): "unknown item" );
	
	SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_MAP_REACH_NAME, pitem_data? pitem_data->GetName(): "unknown item" );
}

void CEditorCodexDlg::OnEnChangeEditProgressMapReachSid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_SID );

	//SMAPNODE* pmap_node = m_mapList.FindMapNode( sid );
	//SetWin_Text( this, IDC_EDIT_PROGRESS_MAP_REACH_NAME, pmap_node? pmap_node->strMapName.c_str(): "unknown item" );
	
	SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_MAP_REACH_NAME, pitem_data? pitem_data->GetName(): "unknown item" );
}

void CEditorCodexDlg::OnBnClickedButtonMapReach()
{
	CDlgFindItem dlg(this);
	if ( dlg.DoModal() == IDOK )
	{
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_MID, dlg.m_sSELECTED.wMainID );
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_SID, dlg.m_sSELECTED.wSubID );
	}
}

void CEditorCodexDlg::OnEnChangeEditProgressTakeItemMid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_SID );

	SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_TAKE_ITEM_NAME, pitem_data? pitem_data->GetName(): "unknown item" );
}

void CEditorCodexDlg::OnEnChangeEditProgressTakeItemSid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_SID );

	SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_TAKE_ITEM_NAME, pitem_data? pitem_data->GetName(): "unknown item" );
}

void CEditorCodexDlg::OnBnClickedButtonTakeItem()
{
	CDlgFindItem dlg(this);
	if ( dlg.DoModal() == IDOK )
	{
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_MID, dlg.m_sSELECTED.wMainID );
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_SID, dlg.m_sSELECTED.wSubID );
	}
}

void CEditorCodexDlg::OnEnChangeEditProgressUseItemMid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_SID );

	SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_USE_ITEM_NAME, pitem_data? pitem_data->GetName(): "unknown item" );
}

void CEditorCodexDlg::OnEnChangeEditProgressUseItemSid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_SID );

	SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_USE_ITEM_NAME, pitem_data? pitem_data->GetName(): "unknown item" );
}

void CEditorCodexDlg::OnBnClickedButtonUseItem()
{
	CDlgFindItem dlg(this);
	if ( dlg.DoModal() == IDOK )
	{
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_MID, dlg.m_sSELECTED.wMainID );
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_SID, dlg.m_sSELECTED.wSubID );
	}
}

void CEditorCodexDlg::OnBnClickedButtonCompleteQuest()
{
	CDlgFindQuest dlg(this);
	if ( dlg.DoModal() == IDOK )
	{
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_COMPLETE_QUEST_ID, dlg.m_sSelected.dwQUESTID );
	}
}

void CEditorCodexDlg::OnEnChangeEditProgressCompleteQuestId()
{
	DWORD dwQuestID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_COMPLETE_QUEST_ID );

	GLQUEST* pquest_data = GLQuestMan::GetInstance().Find( dwQuestID );
	SetWin_Text( this, IDC_EDIT_PROGRESS_COMPLETE_QUEST_NAME, pquest_data? pquest_data->m_strTITLE.c_str(): "unknown quest" );
}

void CEditorCodexDlg::OnBnClickedButtonFileImport()
{
	CString szFilter = "Item Mix Ini File (*.ini)|*.ini|";
	CFileDialog dlg(TRUE,".ini",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr = GLCodex::GetInstance().Import ( dlg.GetPathName().GetString() );
		if ( FAILED ( hr ) )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to import Item Mix ini File." );
		}

		DataShow();
	}
}

void CEditorCodexDlg::OnBnClickedButtonFileCsvSave()
{
	CFileDialog dlg( FALSE, ".csv", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "csv file (*.csv)|*.csv|", this );
	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath();
	if ( dlg.DoModal() == IDOK )
    {
		std::string strFileName = dlg.GetPathName();
        std::fstream streamFILE;
        streamFILE.open ( strFileName.c_str(), std::ios_base::out );
		bool bOK = GLCodex::GetInstance().SaveCsvs( streamFILE );
		streamFILE.close();

		if ( !bOK )
		{
			MessageBox ( "GLCodexMan::SaveCsv, Error GLCodexMan", "", MB_OK );
			return;
		}
	}
	
}

void CEditorCodexDlg::OnBnClickedButtonFileCsvLoad()
{
	CFileDialog dlg( TRUE, ".csv", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "csv file (*.csv)|*.csv|", this );
	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath();
	if ( dlg.DoModal() == IDOK )
    {
		std::string strFileName = dlg.GetPathName();
		bool bOK = GLCodex::GetInstance().LoadCsv( strFileName );

		if ( !bOK )
		{
			MessageBox ( "GLCodexMan::SaveCsv, Error GLCodexMan", "", MB_OK );
			return;
		}
		DataShow();
	}
}

void CEditorCodexDlg::OnEnChangeEditTotalCodex1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
