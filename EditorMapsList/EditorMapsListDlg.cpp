
// EditorMapsListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditorMapsList.h"
#include "EditorMapsListDlg.h"
#include "DxResponseMan.h"
#include "RANPARAM.h"
#include "DxServerInstance.h"
#include "GLOGIC.h"
#include "EtcFunction.h"
#include "DxLandDef.h"
#include "TextureManager.h"
#include "BgmSound.h"
#include "HLibDataConvert.h"
#include ".\editormapslistdlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEditorMapsListDlg dialog




CEditorMapsListDlg::CEditorMapsListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditorMapsListDlg::IDD, pParent)
	, m_bAdd( FALSE )
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEditorMapsListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MAP, m_ListMap );
}

BEGIN_MESSAGE_MAP(CEditorMapsListDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_FILE_LOAD, OnBnClickedButtonFileLoad)
	ON_BN_CLICKED(IDC_BUTTON_FILE_SAVE, OnBnClickedButtonFileSave)
	ON_BN_CLICKED(IDC_BUTTON_FILE_IMPORT, OnBnClickedButtonFileImport)
	ON_BN_CLICKED(IDC_BUTTON_MAPSLIST_ADD, OnBnClickedButtonMapslistAdd)
	ON_BN_CLICKED(IDC_BUTTON_MAPSLIST_EDIT, OnBnClickedButtonMapslistEdit)
	ON_BN_CLICKED(IDC_BUTTON_MAPSLIST_DELETE, OnBnClickedButtonMapslistDelete)
	ON_BN_CLICKED(IDC_BUTTON_MAPSLIST_CLEAR, OnBnClickedButtonMapslistClear)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MAP, OnNMDblclkListMap)
	ON_BN_CLICKED(IDC_BUTTON_MAP_LEVELFILE, OnBnClickedButtonMapLevelfile)
	ON_BN_CLICKED(IDC_BUTTON_MAP_BGMFILE, OnBnClickedButtonMapBgmfile)
	ON_BN_CLICKED(IDC_BUTTON_MAP_LOADINGIMAGE, OnBnClickedButtonMapLoadingimage)
	ON_BN_CLICKED(IDC_BUTTON_MAP_SAVE, OnBnClickedButtonMapSave)
	ON_BN_CLICKED(IDC_BUTTON_MAPSLIST_SETFIELD_0, OnBnClickedButtonMapslistSetfield0)
END_MESSAGE_MAP()


// CEditorMapsListDlg message handlers

BOOL CEditorMapsListDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	RECT rectCtrl;
	m_ListMap.SetExtendedStyle ( m_ListMap.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_ListMap.GetClientRect ( &rectCtrl );
	m_ListMap.InsertColumn( 0, "Use", LVCFMT_LEFT, ( 5* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListMap.InsertColumn( 1, "ID", LVCFMT_LEFT, ( 8* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListMap.InsertColumn( 2, "Field", LVCFMT_LEFT, ( 4* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListMap.InsertColumn( 3, "MapName", LVCFMT_LEFT, ( 15* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListMap.InsertColumn( 4, "LevFile", LVCFMT_LEFT, ( 15* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListMap.InsertColumn( 5, "BGM", LVCFMT_LEFT, ( 11* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListMap.InsertColumn( 6, "Loading", LVCFMT_LEFT, ( 11* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListMap.InsertColumn( 7, "Flags", LVCFMT_LEFT, ( 29* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListMap.InsertColumn( 8, "", LVCFMT_LEFT, ( 2* ( rectCtrl.right - rectCtrl.left ) ) / 100 );


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

	m_mapList.CleanUp();
	m_mapList.LoadMapsListFile( "mapslist.mst" );

	UpdateList();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEditorMapsListDlg::OnPaint()
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
HCURSOR CEditorMapsListDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CEditorMapsListDlg::PostNcDestroy()
{
	m_mapList.CleanUp();
	DxResponseMan::GetInstance().FinalCleanup();
	GMTOOL::CleanUp ();
	CDialog::PostNcDestroy();
}

void CEditorMapsListDlg::UpdateList()
{
	m_ListMap.DeleteAllItems();
	m_ListMap.SetRedraw( FALSE );

	GLMapList::FIELDMAP_DATA mapData = m_mapList.m_mapListData;

	int nIndex = 0;
	for( int nMID = 0; nMID < MAXLANDMID; ++ nMID )
	{
		for( int nSID = 0; nSID < MAXLANDSID; ++ nSID )
		{
			SNATIVEID sMapID( nMID, nSID );
			GLMapList::FIELDMAP_DATA_ITER iterpos = mapData.find( sMapID.dwID );
			if ( iterpos != mapData.end() )
			{
				const SMAPNODE_DATA sMapNode = (*iterpos).second;

				m_ListMap.InsertItem( nIndex, _HLIB::cstring_int(sMapNode.bUsed).GetString());
				m_ListMap.SetItemText( nIndex, 1, _HLIB::cstringformat( "%03d~%03d", sMapNode.sNativeID.wMainID, sMapNode.sNativeID.wSubID ).GetString());
				m_ListMap.SetItemText( nIndex, 2, _HLIB::cstring_dword(sMapNode.dwFieldSID).GetString() );
				m_ListMap.SetItemText( nIndex, 3, sMapNode.strMapName.c_str() );
				m_ListMap.SetItemText( nIndex, 4, sMapNode.strFile.c_str() );
				m_ListMap.SetItemText( nIndex, 5, sMapNode.strBGM.c_str() );
				m_ListMap.SetItemText( nIndex, 6, sMapNode.strLoadingImageName.c_str() );
				m_ListMap.SetItemText( nIndex, 7, _HLIB::cstringformat(
					"%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", 
					sMapNode.bPeaceZone,
					sMapNode.bCommission,
					sMapNode.bPKZone,
					sMapNode.bFreePK,
					sMapNode.bItemDrop,
					sMapNode.bMove,
					sMapNode.bRestart,
					sMapNode.bPetActivity,
					sMapNode.bDECEXP,
					sMapNode.bVehicleActivity,
					sMapNode.bClubBattleZone,
					sMapNode.bInstantMap,
					sMapNode.bQBoxEnable,
					sMapNode.bLunchBoxForbid,
					sMapNode.bCPReset,
					sMapNode.bPKMap,
					sMapNode.bUIMapSelect,
					sMapNode.bUIMapInfo,
					sMapNode.bClubPKRecord,
					sMapNode.bOpenPrivateMarket, /* map private market setting, Juver, 2017/10/02 */
					sMapNode.bPartySparring,
					
					/*map move settings, Juver, 2017/11/25 */
					sMapNode.bBlockTaxi,
					sMapNode.bBlockFriendCard,
					sMapNode.bBlockRecall,
					sMapNode.bBlockTeleport,

					/*global buffs map setting, Juver, 2018/01/23 */
					sMapNode.bDisableSystemBuffs,

					/*hp potion map setting, Juver, 2018/01/23 */
					sMapNode.bBlockHPPotion

					).GetString() );

				m_ListMap.SetItemData( nIndex, sMapNode.sNativeID.dwID );

				nIndex++;
			}
		}
	}

	m_ListMap.SetRedraw( TRUE );

	SMAPNODE_DATA sMapNode;
	ShowMapData( sMapNode, FALSE, FALSE );
}

void CEditorMapsListDlg::OnBnClickedButtonFileLoad()
{
	CString szFilter = "MapsList Setting File (*.mst)|*.mst|";
	CFileDialog dlg( TRUE, ".mst", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr = m_mapList.LoadMapsListFile( dlg.GetFileName().GetString(), NULL, 0, TRUE );
		if ( FAILED ( hr ) )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to load MapsList Setting File." );
		}

		UpdateList();
	}
}

void CEditorMapsListDlg::OnBnClickedButtonFileSave()
{
	CString szFilter = "MapsList Setting File (*.mst)|*.mst|";
	CFileDialog dlg( FALSE, ".mst", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr = m_mapList.SaveMapsListFile ( dlg.GetFileName().GetString() );
		if ( FAILED ( hr ) )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to Save File" );
			return;
		}
	}
}

void CEditorMapsListDlg::OnBnClickedButtonFileImport()
{
	CString szFilter = "MapsList Ini File (*.ini)|*.ini|";
	CFileDialog dlg(TRUE,".ini",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr = m_mapList.LoadMapList_OLD( dlg.GetPathName().GetString() );
		if ( FAILED ( hr ) )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to import MapsList ini File." );
		}

		UpdateList();
	}
}

void CEditorMapsListDlg::OnBnClickedButtonMapslistAdd()
{
	GLMapList::FIELDMAP_DATA mapData = m_mapList.m_mapListData;

	BOOL bAdded = FALSE;
	for( int i = 0; i < MAXLANDMID; ++ i )
	{
		SNATIVEID sID( i, 0 );
		GLMapList::FIELDMAP_DATA_ITER iterpos = mapData.find( sID.dwID );
		if ( iterpos == mapData.end() )
		{
			SMAPNODE_DATA sMapNode;
			sMapNode.sNativeID = sID;
			ShowMapData( sMapNode, TRUE, TRUE );
			bAdded = TRUE;
			break;
		}
	}

	if ( !bAdded )
	{
		for( int nMID = 0; nMID < MAXLANDMID; ++ nMID )
		{
			for( int nSID = 0; nSID < MAXLANDSID; ++ nSID )
			{
				SNATIVEID sID( nMID, nSID );
				GLMapList::FIELDMAP_DATA_ITER iterpos = mapData.find( sID.dwID );
				if ( iterpos == mapData.end() )
				{
					SMAPNODE_DATA sMapNode;
					sMapNode.sNativeID = sID;
					ShowMapData( sMapNode, TRUE, TRUE );
					bAdded = TRUE;
					break;
				}
			}

			if ( bAdded )
				break;
		}
	}
}

void CEditorMapsListDlg::OnBnClickedButtonMapslistEdit()
{
	MapEdit();
}

void CEditorMapsListDlg::OnBnClickedButtonMapslistDelete()
{
	int nSelect = m_ListMap.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwMapID = m_ListMap.GetItemData( nSelect );

	SNATIVEID sID( dwMapID );
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete MapID ID:%02d~%02d", sID.wMainID, sID.wSubID ) !=  IDYES )
		return;

	GLMapList::FIELDMAP_DATA &mapData = m_mapList.m_mapListData;
	GLMapList::FIELDMAP_DATA_ITER iterpos = mapData.find( dwMapID );
	if ( iterpos != mapData.end() )
	{
		mapData.erase( iterpos );

		UpdateList();

		int nNewSelect = nSelect > 1? nSelect-1:nSelect;
		m_ListMap.EnsureVisible( nNewSelect, TRUE );
		m_ListMap.SetItemState( nNewSelect, LVIS_SELECTED, LVIS_SELECTED );
	}
}

void CEditorMapsListDlg::OnBnClickedButtonMapslistClear()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Clear MapsList?" ) ==  IDYES )
	{
		GLMapList::FIELDMAP_DATA &mapData = m_mapList.m_mapListData;
		mapData.clear();
		UpdateList();
	}
}

void CEditorMapsListDlg::OnNMDblclkListMap(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	MapEdit();
}

void CEditorMapsListDlg::MapEdit()
{
	int nSelect = m_ListMap.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwMapID = m_ListMap.GetItemData( nSelect );

	SNATIVEID sID( dwMapID );

	GLMapList::FIELDMAP_DATA mapData = m_mapList.m_mapListData;
	GLMapList::FIELDMAP_DATA_ITER iterpos = mapData.find( dwMapID );
	if ( iterpos != mapData.end() )
	{
		const SMAPNODE_DATA sMapNode = (*iterpos).second;
		ShowMapData( sMapNode, TRUE, FALSE );
	}
}

void CEditorMapsListDlg::ShowMapData( SMAPNODE_DATA sMapNode, BOOL bEnable, BOOL bAdd )
{
	m_bAdd = bAdd;

	SetWin_Check( this, IDC_CHECK_MAP_USE, sMapNode.bUsed );
	SetWin_Num_int( this, IDC_EDIT_MAP_FIELD, sMapNode.dwFieldSID );
	SetWin_Num_int( this, IDC_EDIT_MAP_MID, sMapNode.sNativeID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_MAP_SID, sMapNode.sNativeID.wSubID );

	SetWin_Text( this, IDC_EDIT_MAP_NAME, sMapNode.strMapName.c_str() );
	SetWin_Text( this, IDC_EDIT_MAP_LEVELFILE, sMapNode.strFile.c_str() );
	SetWin_Text( this, IDC_EDIT_MAP_BGM, sMapNode.strBGM.c_str() );
	SetWin_Text( this, IDC_EDIT_MAP_LOADING, sMapNode.strLoadingImageName.c_str() );

	SetWin_Check( this, IDC_CHECK_MAP_PEACEZONE, sMapNode.bPeaceZone );
	SetWin_Check( this, IDC_CHECK_MAP_COMMISSION, sMapNode.bCommission );
	SetWin_Check( this, IDC_CHECK_MAP_PKZONE, sMapNode.bPKZone );
	SetWin_Check( this, IDC_CHECK_MAP_FREEPK, sMapNode.bFreePK );
	SetWin_Check( this, IDC_CHECK_MAP_ITEMDROP, sMapNode.bItemDrop );
	SetWin_Check( this, IDC_CHECK_MAP_MOVE, sMapNode.bMove );
	SetWin_Check( this, IDC_CHECK_MAP_RESTART, sMapNode.bRestart );
	SetWin_Check( this, IDC_CHECK_MAP_PETACTIVITY, sMapNode.bPetActivity );
	SetWin_Check( this, IDC_CHECK_MAP_DECEXP, sMapNode.bDECEXP );
	SetWin_Check( this, IDC_CHECK_MAP_VEHICLEACTIVITY, sMapNode.bVehicleActivity );
	SetWin_Check( this, IDC_CHECK_MAP_CLUBBATTLEZONE, sMapNode.bClubBattleZone );
	SetWin_Check( this, IDC_CHECK_MAP_INSTANTMAP, sMapNode.bInstantMap );
	SetWin_Check( this, IDC_CHECK_MAP_QBOXENABLE, sMapNode.bQBoxEnable );
	SetWin_Check( this, IDC_CHECK_MAP_LUNCHBOXFORBID, sMapNode.bLunchBoxForbid );
	SetWin_Check( this, IDC_CHECK_MAP_CPRESET, sMapNode.bCPReset );
	SetWin_Check( this, IDC_CHECK_MAP_PKMAP, sMapNode.bPKMap );

	SetWin_Check( this, IDC_CHECK_MAP_UI_MAP_SELECT, sMapNode.bUIMapSelect );
	SetWin_Check( this, IDC_CHECK_MAP_UI_MAP_INFO, sMapNode.bUIMapInfo );
	SetWin_Check( this, IDC_CHECK_MAP_CLUB_PK_RECORD, sMapNode.bClubPKRecord );
	SetWin_Check( this, IDC_CHECK_MAP_OPEN_PRIVATE_MARKET, sMapNode.bOpenPrivateMarket ); /* map private market setting, Juver, 2017/10/02 */
	SetWin_Check( this, IDC_CHECK_MAP_PARTY_SPARRING, sMapNode.bPartySparring );

	/*map move settings, Juver, 2017/11/25 */
	SetWin_Check( this, IDC_CHECK_MAP_BLOCK_TAXI, sMapNode.bBlockTaxi );
	SetWin_Check( this, IDC_CHECK_MAP_BLOCK_FRIENDCARD, sMapNode.bBlockFriendCard );
	SetWin_Check( this, IDC_CHECK_MAP_BLOCK_RECALL, sMapNode.bBlockRecall );
	SetWin_Check( this, IDC_CHECK_MAP_BLOCK_TELEPORT, sMapNode.bBlockTeleport );

	/*global buffs map setting, Juver, 2018/01/23 */
	SetWin_Check( this, IDC_CHECK_MAP_DISABLE_SYSTEM_BUFFS, sMapNode.bDisableSystemBuffs );

	/*hp potion map setting, Juver, 2018/01/23 */
	SetWin_Check( this, IDC_CHECK_MAP_BLOCK_HPPOTION, sMapNode.bBlockHPPotion );


	SetWin_Enable( this, IDC_CHECK_MAP_USE, bEnable );
	SetWin_Enable( this, IDC_EDIT_MAP_FIELD, bEnable );
	SetWin_Enable( this, IDC_EDIT_MAP_MID, bEnable && m_bAdd );
	SetWin_Enable( this, IDC_EDIT_MAP_SID, bEnable && m_bAdd );

	SetWin_Enable( this, IDC_EDIT_MAP_NAME, bEnable );
	SetWin_Enable( this, IDC_EDIT_MAP_LEVELFILE, bEnable );
	SetWin_Enable( this, IDC_EDIT_MAP_BGM, bEnable );
	SetWin_Enable( this, IDC_EDIT_MAP_LOADING, bEnable );

	SetWin_Enable( this, IDC_CHECK_MAP_PEACEZONE, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_COMMISSION, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_PKZONE, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_FREEPK, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_ITEMDROP, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_MOVE, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_RESTART, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_PETACTIVITY, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_DECEXP, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_VEHICLEACTIVITY, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_CLUBBATTLEZONE, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_INSTANTMAP, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_QBOXENABLE, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_LUNCHBOXFORBID, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_CPRESET, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_PKMAP, bEnable );

	SetWin_Enable( this, IDC_CHECK_MAP_UI_MAP_SELECT, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_UI_MAP_INFO, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_CLUB_PK_RECORD, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_OPEN_PRIVATE_MARKET, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_PARTY_SPARRING, bEnable );

	/*map move settings, Juver, 2017/11/25 */
	SetWin_Enable( this, IDC_CHECK_MAP_BLOCK_TAXI, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_BLOCK_FRIENDCARD, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_BLOCK_RECALL, bEnable );
	SetWin_Enable( this, IDC_CHECK_MAP_BLOCK_TELEPORT, bEnable );

	/*global buffs map setting, Juver, 2018/01/23 */
	SetWin_Enable( this, IDC_CHECK_MAP_DISABLE_SYSTEM_BUFFS, bEnable );

	/*hp potion map setting, Juver, 2018/01/23 */
	SetWin_Enable( this, IDC_CHECK_MAP_BLOCK_HPPOTION, bEnable );

	SetWin_Enable( this, IDC_BUTTON_MAP_LEVELFILE, bEnable );
	SetWin_Enable( this, IDC_BUTTON_MAP_BGMFILE, bEnable );
	SetWin_Enable( this, IDC_BUTTON_MAP_LOADINGIMAGE, bEnable );
	SetWin_Enable( this, IDC_BUTTON_MAP_SAVE, bEnable );
}

void CEditorMapsListDlg::OnBnClickedButtonMapLevelfile()
{
	CString szFilter = "lev|*.LEV|";
	CFileDialog dlg ( TRUE,".LEV", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = GLLevelFile::GetLevelPath();
	if ( dlg.DoModal() == IDOK )
		SetWin_Text( this, IDC_EDIT_MAP_LEVELFILE, dlg.GetFileName() );
}

void CEditorMapsListDlg::OnBnClickedButtonMapBgmfile()
{
	CString szFilter = "ogg|*.OGG|";
	CFileDialog dlg ( TRUE,".ogg", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = DxBgmSound::GetInstance().GetPath();
	if ( dlg.DoModal() == IDOK )
		SetWin_Text( this, IDC_EDIT_MAP_BGM, dlg.GetFileName() );
}

void CEditorMapsListDlg::OnBnClickedButtonMapLoadingimage()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_MAP_LOADING, dlg.GetFileName().GetString() );
}

void CEditorMapsListDlg::OnBnClickedButtonMapSave()
{
	SMAPNODE_DATA sDATA;
	sDATA.bUsed = GetWin_Check( this, IDC_CHECK_MAP_USE )? TRUE:FALSE;
	sDATA.dwFieldSID = GetWin_Num_int( this, IDC_EDIT_MAP_FIELD );
	sDATA.sNativeID.wMainID = GetWin_Num_int( this, IDC_EDIT_MAP_MID );
	sDATA.sNativeID.wSubID = GetWin_Num_int( this, IDC_EDIT_MAP_SID );

	sDATA.strMapName = GetWin_Text( this, IDC_EDIT_MAP_NAME );
	sDATA.strFile = GetWin_Text( this, IDC_EDIT_MAP_LEVELFILE );
	sDATA.strBGM = GetWin_Text( this, IDC_EDIT_MAP_BGM );
	sDATA.strLoadingImageName = GetWin_Text( this, IDC_EDIT_MAP_LOADING );

	sDATA.bPeaceZone = GetWin_Check( this, IDC_CHECK_MAP_PEACEZONE )? TRUE:FALSE;
	sDATA.bCommission = GetWin_Check( this, IDC_CHECK_MAP_COMMISSION )? TRUE:FALSE;
	sDATA.bPKZone = GetWin_Check( this, IDC_CHECK_MAP_PKZONE )? TRUE:FALSE;
	sDATA.bFreePK = GetWin_Check( this, IDC_CHECK_MAP_FREEPK )? TRUE:FALSE;
	sDATA.bItemDrop = GetWin_Check( this, IDC_CHECK_MAP_ITEMDROP )? TRUE:FALSE;
	sDATA.bMove = GetWin_Check( this, IDC_CHECK_MAP_MOVE )? TRUE:FALSE;
	sDATA.bRestart = GetWin_Check( this, IDC_CHECK_MAP_RESTART )? TRUE:FALSE;
	sDATA.bPetActivity = GetWin_Check( this, IDC_CHECK_MAP_PETACTIVITY )? TRUE:FALSE;
	sDATA.bDECEXP = GetWin_Check( this, IDC_CHECK_MAP_DECEXP )? TRUE:FALSE;
	sDATA.bVehicleActivity = GetWin_Check( this, IDC_CHECK_MAP_VEHICLEACTIVITY )? TRUE:FALSE;
	sDATA.bClubBattleZone = GetWin_Check( this, IDC_CHECK_MAP_CLUBBATTLEZONE )? TRUE:FALSE;
	sDATA.bInstantMap = GetWin_Check( this, IDC_CHECK_MAP_INSTANTMAP )? TRUE:FALSE;
	sDATA.bQBoxEnable = GetWin_Check( this, IDC_CHECK_MAP_QBOXENABLE )? TRUE:FALSE;
	sDATA.bLunchBoxForbid = GetWin_Check( this, IDC_CHECK_MAP_LUNCHBOXFORBID )? TRUE:FALSE;
	sDATA.bCPReset = GetWin_Check( this, IDC_CHECK_MAP_CPRESET )? TRUE:FALSE;
	sDATA.bPKMap = GetWin_Check( this, IDC_CHECK_MAP_PKMAP )? TRUE:FALSE;

	sDATA.bUIMapSelect = GetWin_Check( this, IDC_CHECK_MAP_UI_MAP_SELECT )? TRUE:FALSE;
	sDATA.bUIMapInfo = GetWin_Check( this, IDC_CHECK_MAP_UI_MAP_INFO )? TRUE:FALSE;
	sDATA.bClubPKRecord = GetWin_Check( this, IDC_CHECK_MAP_CLUB_PK_RECORD )? TRUE:FALSE;
	sDATA.bOpenPrivateMarket = GetWin_Check( this, IDC_CHECK_MAP_OPEN_PRIVATE_MARKET )? TRUE:FALSE; /* map private market setting, Juver, 2017/10/02 */
	sDATA.bPartySparring = GetWin_Check( this, IDC_CHECK_MAP_PARTY_SPARRING )? TRUE:FALSE;

	/*map move settings, Juver, 2017/11/25 */
	sDATA.bBlockTaxi = GetWin_Check( this, IDC_CHECK_MAP_BLOCK_TAXI )? TRUE:FALSE;
	sDATA.bBlockFriendCard = GetWin_Check( this, IDC_CHECK_MAP_BLOCK_FRIENDCARD )? TRUE:FALSE;
	sDATA.bBlockRecall = GetWin_Check( this, IDC_CHECK_MAP_BLOCK_RECALL )? TRUE:FALSE;
	sDATA.bBlockTeleport = GetWin_Check( this, IDC_CHECK_MAP_BLOCK_TELEPORT )? TRUE:FALSE;

	/*global buffs map setting, Juver, 2018/01/23 */
	sDATA.bDisableSystemBuffs = GetWin_Check( this, IDC_CHECK_MAP_DISABLE_SYSTEM_BUFFS )? TRUE:FALSE;

	/*hp potion map setting, Juver, 2018/01/23 */
	sDATA.bBlockHPPotion = GetWin_Check( this, IDC_CHECK_MAP_BLOCK_HPPOTION )? TRUE:FALSE;

	if( sDATA.strMapName.size() > MAP_NAME_MAX )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Max Name Length is only: %d", MAP_NAME_MAX );
		return;
	}

	if ( sDATA.strMapName.size() <= 0 )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Name Empty!" );
		return;
	}

	if ( sDATA.sNativeID.wMainID >= MAXLANDMID )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Max MID is only: %d", MAXLANDMID );
		return;
	}

	if ( sDATA.sNativeID.wSubID >= MAXLANDSID )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Max MID is only: %d", MAXLANDSID );
		return;
	}

	if ( sDATA.strLoadingImageName.size() <= 0 )
		sDATA.strLoadingImageName = "null";

	GLMapList::FIELDMAP_DATA &mapData = m_mapList.m_mapListData;

	if ( m_bAdd )
	{
		GLMapList::FIELDMAP_DATA_ITER iterpos = mapData.find( sDATA.sNativeID.dwID );
		if ( iterpos != mapData.end() )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Map Exist ID:%d~%d", sDATA.sNativeID.wMainID, sDATA.sNativeID.wSubID );
			return;
		}

		mapData.insert ( std::make_pair( sDATA.sNativeID.dwID, sDATA ) );
		UpdateList();
		ShowMapData( sDATA, TRUE, FALSE );
	}
	else
	{
		GLMapList::FIELDMAP_DATA_ITER iterpos = mapData.find( sDATA.sNativeID.dwID );
		if ( iterpos != mapData.end() )
		{
			SMAPNODE_DATA &sMapNode = (*iterpos).second;
			sMapNode = sDATA;
			UpdateList();
			ShowMapData( sMapNode, TRUE, FALSE );
		}
		else
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to save ID:%d~%d", sDATA.sNativeID.wMainID, sDATA.sNativeID.wSubID );
		}
	}

	for( int nItem = 0; nItem < m_ListMap.GetItemCount(); nItem++)
	{
		DWORD dwMapID = m_ListMap.GetItemData( nItem );
		if( dwMapID == sDATA.sNativeID.dwID )
		{
			m_ListMap.EnsureVisible( nItem, TRUE );
			m_ListMap.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
		}
	}
}

void CEditorMapsListDlg::OnBnClickedButtonMapslistSetfield0()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Set all FieldID to 0?" ) == IDYES )
	{
		GLMapList::FIELDMAP_DATA &mapData = m_mapList.m_mapListData;
		for ( GLMapList::FIELDMAP_DATA_ITER iterpos =  mapData.begin();
			iterpos != mapData.end();
			++ iterpos )
		{
			SMAPNODE_DATA &sMapNode = (*iterpos).second;
			sMapNode.dwFieldSID = 0;
		}

		UpdateList();
	}
}
