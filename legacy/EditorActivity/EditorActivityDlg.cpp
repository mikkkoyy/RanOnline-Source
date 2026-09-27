
// EditorActivityDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditorActivity.h"
#include "EditorActivityDlg.h"

#include "DxResponseMan.h"
#include "RANPARAM.h"
#include "DxServerInstance.h"
#include "GLOGIC.h"

#include "GLActivity.h"
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


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEditorActivityDlg dialog




CEditorActivityDlg::CEditorActivityDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditorActivityDlg::IDD, pParent)
	, m_bAdd(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEditorActivityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ACTIVITY, m_List );
}

BEGIN_MESSAGE_MAP(CEditorActivityDlg, CDialog)
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
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ACTIVITY, OnNMDblclkListActivity)
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
END_MESSAGE_MAP()


// CEditorActivityDlg message handlers

BOOL CEditorActivityDlg::OnInitDialog()
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

	GLActivity::GetInstance().LoadFile( "Activity.asf", FALSE );

	m_mapList.CleanUp();
	m_mapList.LoadMapsListFile( "mapslist.mst" );

	SetWin_Combo_Init( this, IDC_COMBO_TYPE, COMMENT::ACTIVITY_MID, EMACTIVITY_TYPE_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_NOTIFY, COMMENT::ACTIVITY_NOTIFY, EMACTIVITY_NOTIFY_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_USE_QBOX, COMMENT::ITEM_QUE_TYPE, QUESTION_SIZE );

	DataShow();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEditorActivityDlg::OnPaint()
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
HCURSOR CEditorActivityDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CEditorActivityDlg::PostNcDestroy()
{
	GLActivity::GetInstance().CleanUp();
	DxResponseMan::GetInstance().FinalCleanup();
	GMTOOL::CleanUp ();
	CDialog::PostNcDestroy();
}

BOOL CEditorActivityDlg::DataShow()
{
	m_List.DeleteAllItems();
	m_List.SetRedraw( FALSE );

	DWORD dwTotal = 0;

	SACTIVITY_FILE_DATA_MAP &map_activity = GLActivity::GetInstance().m_mapActivity;
	int i=0;
	for( SACTIVITY_FILE_DATA_MAP_ITER iter = map_activity.begin(); 
		iter != map_activity.end(); iter++, i++ )
	{
		const SACTIVITY_FILE_DATA &sdata = (*iter).second;

		m_List.InsertItem( i, _HLIB::cstring_dword(sdata.dwActivityID).GetString() );
		m_List.SetItemText( i, 1, COMMENT::ACTIVITY_MID[sdata.emType].c_str() );
		m_List.SetItemText( i, 2, sdata.strActivityTitle.c_str() );
		m_List.SetItemText( i, 3, _HLIB::cstringformat( "(%d) %s", sdata.bRewardBadge, sdata.strBadgeString.c_str() ).GetString() );
		m_List.SetItemText( i, 4, _HLIB::cstring_dword(sdata.dwRewardPoint).GetString() );

		if ( sdata.emType == EMACTIVITY_TYPE_REACH_LEVEL )
		{
			m_List.SetItemText( i, 5, _HLIB::cstring_dword(sdata.wProgressLevel).GetString() );
		}
		else if ( sdata.emType == EMACTIVITY_TYPE_KILL_MOB )
		{
			SCROWDATA* pcrow_data = GLCrowDataMan::GetInstance().GetCrowData( sdata.sidMobKill );
			m_List.SetItemText( i, 5, _HLIB::cstringformat( "[%03d~%03d][%03d]%s", 
				sdata.sidMobKill.wMainID, sdata.sidMobKill.wSubID, sdata.wProgressMobKill, 
				pcrow_data? pcrow_data->GetName() : "unknown crow" ).GetString() );
		}
		else if ( sdata.emType == EMACTIVITY_TYPE_KILL_PLAYER )
		{
			SMAPNODE* pmapnode = m_mapList.FindMapNode( sdata.sidMapKill );
			m_List.SetItemText( i, 5, _HLIB::cstringformat( "[%03d~%03d][%03d]%s", 
				sdata.sidMapKill.wMainID, sdata.sidMapKill.wSubID, sdata.wProgressMapKill, 
				pmapnode? pmapnode->strMapName.c_str() : "unknown map" ).GetString() );
		}
		else if ( sdata.emType == EMACTIVITY_TYPE_REACH_MAP )
		{
			SMAPNODE* pmapnode = m_mapList.FindMapNode( sdata.sidMapReach );
			m_List.SetItemText( i, 5, _HLIB::cstringformat( "[%03d~%03d][%03d]%s", 
				sdata.sidMapReach.wMainID, sdata.sidMapReach.wSubID, sdata.wProgressMapReach, 
				pmapnode? pmapnode->strMapName.c_str() : "unknown map" ).GetString() );
		}
		else if ( sdata.emType == EMACTIVITY_TYPE_TAKE_ITEM )
		{
			SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sdata.sidItemGet );
			m_List.SetItemText( i, 5, _HLIB::cstringformat( "[%03d~%03d][%03d]%s", 
				sdata.sidItemGet.wMainID, sdata.sidItemGet.wSubID, sdata.wProgressItemGet, 
				pitem_data? pitem_data->GetName() : "unknown item" ).GetString() );
		}
		else if ( sdata.emType == EMACTIVITY_TYPE_USE_ITEM )
		{
			SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sdata.sidItemUse );
			m_List.SetItemText( i, 5, _HLIB::cstringformat( "[%03d~%03d][%03d]%s", 
				sdata.sidItemUse.wMainID, sdata.sidItemUse.wSubID, sdata.wProgressItemUse, 
				pitem_data? pitem_data->GetName() : "unknown item" ).GetString() );
		}
		else if ( sdata.emType == EMACTIVITY_TYPE_QUESTION_BOX )
		{
			m_List.SetItemText( i, 5, _HLIB::cstringformat( "[%d][%s]", sdata.wQBoxProgress,
				COMMENT::ITEM_QUE_TYPE[sdata.dwQBoxType].c_str() ).GetString() );
		}
		else if ( sdata.emType == EMACTIVITY_TYPE_COMPLETE_QUEST )
		{
			GLQUEST* pQuest = GLQuestMan::GetInstance().Find( sdata.dwQuestID );
			m_List.SetItemText( i, 5, _HLIB::cstringformat( "[%03d][%03d]%s", 
				sdata.dwQuestID, sdata.wQuestProgress, 
				pQuest? pQuest->m_strTITLE.c_str() : "unknown quest" ).GetString() );
		}
		else if ( sdata.emType == EMACTIVITY_TYPE_ACTIVITY_POINT )
		{
			m_List.SetItemText( i, 5, _HLIB::cstring_dword(sdata.dwActivityProgress).GetString() );
		}
			
		

		m_List.SetItemText( i, 6, COMMENT::ACTIVITY_NOTIFY[sdata.emNotify].c_str() );
		m_List.SetItemData( i, sdata.dwActivityID );

		dwTotal += sdata.dwRewardPoint;
	}

	m_List.SetRedraw( TRUE );

	SACTIVITY_FILE_DATA sactivity;
	ActivityShow( sactivity, FALSE, FALSE );

	SetWin_Num_int( this, IDC_EDIT_TOTAL_ACTIVITY, dwTotal );

	return TRUE;
}

void CEditorActivityDlg::OnBnClickedButtonFileLoad()
{
	CString szFilter = "Activity Setting File (*.asf)|*.asf|";
	CFileDialog dlg( TRUE, ".asf", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr = GLActivity::GetInstance().LoadFile ( dlg.GetFileName().GetString(), FALSE );
		if ( FAILED ( hr ) )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to load File" );
		}

		DataShow();
	}
}

void CEditorActivityDlg::OnBnClickedButtonFileSave()
{
	CString szFilter = "Activity Setting File (*.asf)|*.asf|";
	CFileDialog dlg( FALSE, ".asf", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr = GLActivity::GetInstance().SaveFile ( dlg.GetFileName().GetString() );
		if ( FAILED ( hr ) )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to Save File" );
			return;
		}
	}
}

void CEditorActivityDlg::OnBnClickedButtonAdd()
{
	SACTIVITY_FILE_DATA_MAP &map_activity = GLActivity::GetInstance().m_mapActivity;

	DWORD dwNewKey = UINT_MAX;
	for( DWORD dwKey = 0; dwKey<GLActivity::ACTIVITY_MAXID; ++dwKey )
	{
		SACTIVITY_FILE_DATA_MAP_ITER iterpos = map_activity.find(dwKey);
		if ( iterpos == map_activity.end() )
		{
			dwNewKey = dwKey;
			break;
		}
	}

	if ( dwNewKey == UINT_MAX || dwNewKey >= GLActivity::ACTIVITY_MAXID )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Unable to generate new ID" );
		return;
	}

	SACTIVITY_FILE_DATA sactivity;
	sactivity.dwActivityID = dwNewKey;
	ActivityShow( sactivity, TRUE, TRUE );
}

void CEditorActivityDlg::OnBnClickedButtonEdit()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = m_List.GetItemData( nSelect );

	SACTIVITY_FILE_DATA_MAP &map_activity = GLActivity::GetInstance().m_mapActivity;
	SACTIVITY_FILE_DATA_MAP_ITER iterpos = map_activity.find(dwID);
	if ( iterpos != map_activity.end() )
	{
		SACTIVITY_FILE_DATA &sactivity = (*iterpos).second;
		ActivityShow( sactivity, TRUE, FALSE );
	}
}

void CEditorActivityDlg::OnBnClickedButtonDelete()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = m_List.GetItemData( nSelect );

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete Activity ID:%02d", dwID ) !=  IDYES )
		return;

	SACTIVITY_FILE_DATA_MAP &map_activity = GLActivity::GetInstance().m_mapActivity;
	SACTIVITY_FILE_DATA_MAP_ITER iterpos = map_activity.find(dwID);
	if ( iterpos != map_activity.end() )
	{
		map_activity.erase( iterpos );
		DataShow();
	}

	int nNewSelect = nSelect > 1? nSelect-1:nSelect;
	m_List.EnsureVisible( nNewSelect, TRUE );
	m_List.SetItemState( nNewSelect, LVIS_SELECTED, LVIS_SELECTED );
}

void CEditorActivityDlg::OnBnClickedButtonClear()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Clear Activity?" ) ==  IDYES )
	{
		GLActivity::GetInstance().m_mapActivity.clear();
		DataShow();
	}
}

void CEditorActivityDlg::OnBnClickedButtonSave()
{
	SACTIVITY_FILE_DATA sactivity_data;

	sactivity_data.dwActivityID = GetWin_Num_int( this, IDC_EDIT_ID );
	sactivity_data.strActivityTitle = GetWin_Text( this, IDC_EDIT_TITLE ).GetString();
	sactivity_data.strDescription = GetWin_Text( this, IDC_EDIT_DESC ).GetString();

	sactivity_data.emNotify = static_cast<EMACTIVITY_NOTIFY> ( GetWin_Combo_Sel( this, IDC_COMBO_NOTIFY ) );
	
	sactivity_data.bRewardBadge = GetWin_Check( this, IDC_CHECK_BADGE );
	sactivity_data.strBadgeString = GetWin_Text( this, IDC_EDIT_BADGE ).GetString();
	sactivity_data.dwRewardPoint = GetWin_Num_int( this, IDC_EDIT_REWARD_POINT );

	sactivity_data.emType = static_cast<EMACTIVITY_TYPE> ( GetWin_Combo_Sel( this, IDC_COMBO_TYPE ) );

	//level
	sactivity_data.wProgressLevel = GetWin_Num_int( this, IDC_EDIT_PROGRESS_LEVEL );

	//mobkill
	sactivity_data.sidMobKill.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_MID );
	sactivity_data.sidMobKill.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_SID );
	sactivity_data.wProgressMobKill = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_NUM );

	//mapkill
	sactivity_data.sidMapKill.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_MID );
	sactivity_data.sidMapKill.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_SID );
	sactivity_data.wProgressMapKill = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_NUM );

	//map reach
	sactivity_data.sidMapReach.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_MID );
	sactivity_data.sidMapReach.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_SID );
	sactivity_data.wProgressMapReach = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_NUM );

	//take item
	sactivity_data.sidItemGet.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_MID );
	sactivity_data.sidItemGet.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_SID );
	sactivity_data.wProgressItemGet = GetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_NUM );

	//use item
	sactivity_data.sidItemUse.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_MID );
	sactivity_data.sidItemUse.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_SID );
	sactivity_data.wProgressItemUse = GetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_NUM );

	//obtain qbox
	sactivity_data.dwQBoxType = GetWin_Combo_Sel( this, IDC_COMBO_USE_QBOX );
	sactivity_data.wQBoxProgress = GetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_QBOX );

	//complete quest
	sactivity_data.dwQuestID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_COMPLETE_QUEST_ID );
	sactivity_data.wQuestProgress = GetWin_Num_int( this, IDC_EDIT_PROGRESS_COMPLETE_QUEST_NUM );

	//activity point
	sactivity_data.dwActivityProgress = GetWin_Num_int( this, IDC_EDIT_PROGRESS_ACTIVITY_POINT );

	if ( sactivity_data.dwActivityID == UINT_MAX || sactivity_data.dwActivityID >= GLActivity::ACTIVITY_MAXID )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid ID %d", sactivity_data.dwActivityID );
		return;
	}

	if ( sactivity_data.strActivityTitle.empty() )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Title Empty!" );
		return;
	}

	if ( sactivity_data.bRewardBadge && sactivity_data.strBadgeString.empty() )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Badge Empty!" );
		return;
	}

	if ( sactivity_data.emType == EMACTIVITY_TYPE_REACH_LEVEL )
	{
		if ( sactivity_data.wProgressLevel < 1 )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Level %d", sactivity_data.wProgressLevel );
			return;
		}
	}

	if ( sactivity_data.emType == EMACTIVITY_TYPE_KILL_MOB )
	{
		SCROWDATA* pcrowdata = GLCrowDataMan::GetInstance().GetCrowData( sactivity_data.sidMobKill );
		if ( !pcrowdata )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Crow %d~%d", sactivity_data.sidMobKill.wMainID, sactivity_data.sidMobKill.wSubID );
			return;
		}

		if ( sactivity_data.wProgressMobKill < 1 )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Mob Num %d", sactivity_data.wProgressMobKill );
			return;
		}
	}

	if ( sactivity_data.emType == EMACTIVITY_TYPE_KILL_PLAYER )
	{
		SMAPNODE* pmap_node = m_mapList.FindMapNode( sactivity_data.sidMapKill );
		if ( !pmap_node )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Map %d~%d", sactivity_data.sidMapKill.wMainID, sactivity_data.sidMapKill.wSubID );
			return;
		}

		if ( sactivity_data.wProgressMapKill < 1 )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Kill Num %d", sactivity_data.wProgressMobKill );
			return;
		}
	}

	if ( sactivity_data.emType == EMACTIVITY_TYPE_REACH_MAP )
	{
		SMAPNODE* pmap_node = m_mapList.FindMapNode( sactivity_data.sidMapReach );
		if ( !pmap_node )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Map %d~%d", sactivity_data.sidMapReach.wMainID, sactivity_data.sidMapReach.wSubID );
			return;
		}

		if ( sactivity_data.wProgressMapReach < 1 )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid reach Num %d", sactivity_data.wProgressMapReach);
			return;
		}
	}

	if ( sactivity_data.emType == EMACTIVITY_TYPE_TAKE_ITEM )
	{
		SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sactivity_data.sidItemGet );
		if ( !pitem_data )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid item %d~%d", sactivity_data.sidItemGet.wMainID, sactivity_data.sidItemGet.wSubID );
			return;
		}

		if ( sactivity_data.wProgressItemGet < 1 )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid item Num %d", sactivity_data.wProgressItemGet);
			return;
		}
	}

	if ( sactivity_data.emType == EMACTIVITY_TYPE_USE_ITEM )
	{
		SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sactivity_data.sidItemUse );
		if ( !pitem_data )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid item %d~%d", sactivity_data.sidItemUse.wMainID, sactivity_data.sidItemUse.wSubID );
			return;
		}

		if ( sactivity_data.wProgressItemUse < 1 )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid item Num %d", sactivity_data.wProgressItemUse);
			return;
		}
	}

	if ( sactivity_data.emType == EMACTIVITY_TYPE_QUESTION_BOX )
	{
		if ( sactivity_data.wQBoxProgress < 1 )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid qbox Num %d", sactivity_data.wQBoxProgress );
			return;
		}
	}

	if ( sactivity_data.emType == EMACTIVITY_TYPE_COMPLETE_QUEST )
	{
		GLQUEST* pquest = GLQuestMan::GetInstance().Find( sactivity_data.dwQuestID );
		if ( !pquest )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid quest %d", sactivity_data.dwQuestID );
			return;
		}

		if ( sactivity_data.wQuestProgress < 1 )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid quest Num %d", sactivity_data.wQuestProgress);
			return;
		}
	}

	if ( sactivity_data.emType == EMACTIVITY_TYPE_ACTIVITY_POINT )
	{
		if ( sactivity_data.dwActivityProgress < 1 )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid activity point %d", sactivity_data.dwActivityProgress );
			return;
		}
	}

	SACTIVITY_FILE_DATA_MAP &map_activity = GLActivity::GetInstance().m_mapActivity;

	if ( m_bAdd )
	{
		SACTIVITY_FILE_DATA_MAP_ITER iterpos = map_activity.find(sactivity_data.dwActivityID);
		if ( iterpos != map_activity.end() )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "ID Exist ID:%d", sactivity_data.dwActivityID );
			return;
		}

		map_activity.insert( std::make_pair( sactivity_data.dwActivityID, sactivity_data ) );
		DataShow();
		ActivityShow( sactivity_data, TRUE, FALSE );
	}
	else
	{
		SACTIVITY_FILE_DATA_MAP_ITER iterpos = map_activity.find(sactivity_data.dwActivityID);
		if ( iterpos != map_activity.end() )
		{
			SACTIVITY_FILE_DATA* pData = &(*iterpos).second;
			*pData = sactivity_data;

			DataShow();
			ActivityShow( *pData, TRUE, FALSE );
		}else{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to save ID :%d", sactivity_data.dwActivityID );

		}
	}

	for( int nItem = 0; nItem < m_List.GetItemCount(); nItem++)
	{
		DWORD dwID = m_List.GetItemData( nItem );
		if( dwID == sactivity_data.dwActivityID )
		{
			m_List.EnsureVisible( nItem, TRUE );
			m_List.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
		}
	}
}

void CEditorActivityDlg::OnCbnSelchangeComboType()
{
	EMACTIVITY_TYPE emType = static_cast<EMACTIVITY_TYPE> ( GetWin_Combo_Sel( this, IDC_COMBO_TYPE ) );
	BOOL bReachLevel = BOOL( emType == EMACTIVITY_TYPE_REACH_LEVEL );
	BOOL bKillMob = BOOL( emType== EMACTIVITY_TYPE_KILL_MOB );
	BOOL bKillPlayer = BOOL( emType == EMACTIVITY_TYPE_KILL_PLAYER );
	BOOL bReachMap = BOOL( emType == EMACTIVITY_TYPE_REACH_MAP );
	BOOL bTakeItem = BOOL( emType == EMACTIVITY_TYPE_TAKE_ITEM );
	BOOL bUseItem = BOOL( emType == EMACTIVITY_TYPE_USE_ITEM );
	BOOL bUseQBox = BOOL( emType == EMACTIVITY_TYPE_QUESTION_BOX );
	BOOL bCompleteQuest = BOOL( emType == EMACTIVITY_TYPE_COMPLETE_QUEST );
	BOOL bReachActivityPoint = BOOL ( emType == EMACTIVITY_TYPE_ACTIVITY_POINT );

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

	SetWin_Enable( this, IDC_EDIT_PROGRESS_ACTIVITY_POINT, bReachActivityPoint );
}

void CEditorActivityDlg::ActivityShow( SACTIVITY_FILE_DATA &sdata, BOOL bEnable, BOOL bAdd )
{
	m_bAdd = bAdd;

	SetWin_Num_int( this, IDC_EDIT_ID, sdata.dwActivityID );
	SetWin_Text( this, IDC_EDIT_TITLE, sdata.strActivityTitle.c_str() );
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

	//activity point
	SetWin_Num_int( this, IDC_EDIT_PROGRESS_ACTIVITY_POINT, sdata.dwActivityProgress );




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

	//activity point
	SetWin_Enable( this, IDC_EDIT_PROGRESS_ACTIVITY_POINT, bEnable );

	SetWin_Enable( this, IDC_BUTTON_SAVE, bEnable );

	if ( bEnable )	OnCbnSelchangeComboType();
}

void CEditorActivityDlg::OnNMDblclkListActivity(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = m_List.GetItemData( nSelect );

	SACTIVITY_FILE_DATA_MAP &map_activity = GLActivity::GetInstance().m_mapActivity;
	SACTIVITY_FILE_DATA_MAP_ITER iterpos = map_activity.find(dwID);
	if ( iterpos != map_activity.end() )
	{
		SACTIVITY_FILE_DATA &sactivity = (*iterpos).second;
		ActivityShow( sactivity, TRUE, FALSE );
	}
}

void CEditorActivityDlg::OnEnChangeEditProgressMobKillMid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_SID );

	SCROWDATA* pcrowdata = GLCrowDataMan::GetInstance().GetCrowData( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_MOB_KILL_NAME, pcrowdata? pcrowdata->GetName(): "unknown crow" );
}

void CEditorActivityDlg::OnEnChangeEditProgressMobKillSid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_SID );

	SCROWDATA* pcrowdata = GLCrowDataMan::GetInstance().GetCrowData( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_MOB_KILL_NAME, pcrowdata? pcrowdata->GetName(): "unknown crow" );
}

void CEditorActivityDlg::OnBnClickedButtonMobKill()
{
	CDlgFindCrow dlg(this);
	if ( dlg.DoModal() == IDOK )
	{
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_MID, dlg.m_sSELECTED.wMainID );
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_MOB_KILL_SID, dlg.m_sSELECTED.wSubID );
	}
}

void CEditorActivityDlg::OnEnChangeEditProgressMapKillMid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_SID );

	SMAPNODE* pmap_node = m_mapList.FindMapNode( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_MAP_KILL_NAME, pmap_node? pmap_node->strMapName.c_str(): "unknown map" );
}

void CEditorActivityDlg::OnEnChangeEditProgressMapKillSid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_SID );

	SMAPNODE* pmap_node = m_mapList.FindMapNode( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_MAP_KILL_NAME, pmap_node? pmap_node->strMapName.c_str(): "unknown map" );
}

void CEditorActivityDlg::OnBnClickedButtonMapKill()
{
	CDlgFindMap dlg(this);
	if ( dlg.DoModal() == IDOK )
	{
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_MID, dlg.m_sSELECTED.sMAPID.wMainID );
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_KILL_SID, dlg.m_sSELECTED.sMAPID.wSubID );
	}
}

void CEditorActivityDlg::OnEnChangeEditProgressMapReachMid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_SID );

	SMAPNODE* pmap_node = m_mapList.FindMapNode( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_MAP_REACH_NAME, pmap_node? pmap_node->strMapName.c_str(): "unknown map" );
}

void CEditorActivityDlg::OnEnChangeEditProgressMapReachSid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_SID );

	SMAPNODE* pmap_node = m_mapList.FindMapNode( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_MAP_REACH_NAME, pmap_node? pmap_node->strMapName.c_str(): "unknown map" );
}

void CEditorActivityDlg::OnBnClickedButtonMapReach()
{
	CDlgFindMap dlg(this);
	if ( dlg.DoModal() == IDOK )
	{
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_MID, dlg.m_sSELECTED.sMAPID.wMainID );
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_MAP_REACH_SID, dlg.m_sSELECTED.sMAPID.wSubID );
	}
}

void CEditorActivityDlg::OnEnChangeEditProgressTakeItemMid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_SID );

	SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_TAKE_ITEM_NAME, pitem_data? pitem_data->GetName(): "unknown item" );
}

void CEditorActivityDlg::OnEnChangeEditProgressTakeItemSid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_SID );

	SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_TAKE_ITEM_NAME, pitem_data? pitem_data->GetName(): "unknown item" );
}

void CEditorActivityDlg::OnBnClickedButtonTakeItem()
{
	CDlgFindItem dlg(this);
	if ( dlg.DoModal() == IDOK )
	{
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_MID, dlg.m_sSELECTED.wMainID );
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_TAKE_ITEM_SID, dlg.m_sSELECTED.wSubID );
	}
}

void CEditorActivityDlg::OnEnChangeEditProgressUseItemMid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_SID );

	SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_USE_ITEM_NAME, pitem_data? pitem_data->GetName(): "unknown item" );
}

void CEditorActivityDlg::OnEnChangeEditProgressUseItemSid()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_SID );

	SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sid );
	SetWin_Text( this, IDC_EDIT_PROGRESS_USE_ITEM_NAME, pitem_data? pitem_data->GetName(): "unknown item" );
}

void CEditorActivityDlg::OnBnClickedButtonUseItem()
{
	CDlgFindItem dlg(this);
	if ( dlg.DoModal() == IDOK )
	{
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_MID, dlg.m_sSELECTED.wMainID );
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_USE_ITEM_SID, dlg.m_sSELECTED.wSubID );
	}
}

void CEditorActivityDlg::OnBnClickedButtonCompleteQuest()
{
	CDlgFindQuest dlg(this);
	if ( dlg.DoModal() == IDOK )
	{
		SetWin_Num_int( this, IDC_EDIT_PROGRESS_COMPLETE_QUEST_ID, dlg.m_sSelected.dwQUESTID );
	}
}

void CEditorActivityDlg::OnEnChangeEditProgressCompleteQuestId()
{
	DWORD dwQuestID = GetWin_Num_int( this, IDC_EDIT_PROGRESS_COMPLETE_QUEST_ID );

	GLQUEST* pquest_data = GLQuestMan::GetInstance().Find( dwQuestID );
	SetWin_Text( this, IDC_EDIT_PROGRESS_COMPLETE_QUEST_NAME, pquest_data? pquest_data->m_strTITLE.c_str(): "unknown quest" );
}
