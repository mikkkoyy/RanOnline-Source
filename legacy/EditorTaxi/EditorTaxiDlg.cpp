
// EditorTaxiDlg.cpp : implementation file
//

#include "stdafx.h"
#include <map>

#include "EditorTaxi.h"
#include "EditorTaxiDlg.h"

#include "DxResponseMan.h"
#include "RANPARAM.h"
#include "DxServerInstance.h"
#include "GLOGIC.h"
#include "EtcFunction.h"
#include "GLTaxiStation.h"
#include "GLCrowData.h"
#include "DlgCrow.h"
#include "DlgMap.h"
#include "HLibDataConvert.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEditorTaxiDlg dialog




CEditorTaxiDlg::CEditorTaxiDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditorTaxiDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEditorTaxiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_TAXI, m_ListTaxi );
}

BEGIN_MESSAGE_MAP(CEditorTaxiDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_FILE_LOAD, OnBnClickedButtonFileLoad)
	ON_BN_CLICKED(IDC_BUTTON_FILE_SAVE, OnBnClickedButtonFileSave)
	ON_BN_CLICKED(IDC_BUTTON_FILE_IMPORT, OnBnClickedButtonFileImport)
	ON_BN_CLICKED(IDC_BUTTON_TAXI_ADD, OnBnClickedButtonTaxiAdd)
	ON_BN_CLICKED(IDC_BUTTON_TAXI_EDIT, OnBnClickedButtonTaxiEdit)
	ON_BN_CLICKED(IDC_BUTTON_TAXI_DELETE, OnBnClickedButtonTaxiDelete)
	ON_BN_CLICKED(IDC_BUTTON_TAXI_CLEAR, OnBnClickedButtonTaxiClear)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_TAXI, OnNMDblclkListTaxi)
	ON_BN_CLICKED(IDC_BUTTON_TAXIEDIT_MAP, OnBnClickedButtonTaxieditMap)
	ON_BN_CLICKED(IDC_BUTTON_TAXIEDIT_NPC, OnBnClickedButtonTaxieditNpc)
	ON_BN_CLICKED(IDC_BUTTON_TAXIEDIT_SAVE, OnBnClickedButtonTaxieditSave)
	ON_EN_CHANGE(IDC_EDIT_TAXI_MAP_MID, OnEnChangeEditTaxiMapMid)
	ON_EN_CHANGE(IDC_EDIT_TAXI_MAP_SID, OnEnChangeEditTaxiMapSid)
	ON_EN_CHANGE(IDC_EDIT_TAXI_NPC_MID, OnEnChangeEditTaxiNpcMid)
	ON_EN_CHANGE(IDC_EDIT_TAXI_NPC_SID, OnEnChangeEditTaxiNpcSid)
END_MESSAGE_MAP()


// CEditorTaxiDlg message handlers

BOOL CEditorTaxiDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	RECT rectCtrl;
	m_ListTaxi.SetExtendedStyle ( m_ListTaxi.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_ListTaxi.GetClientRect ( &rectCtrl );
	m_ListTaxi.InsertColumn( 0, "ID", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListTaxi.InsertColumn( 1, "Use", LVCFMT_LEFT, ( 9* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListTaxi.InsertColumn( 2, "Map", LVCFMT_LEFT, ( 38* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListTaxi.InsertColumn( 3, "Station", LVCFMT_LEFT, ( 38* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListTaxi.InsertColumn( 4, "", LVCFMT_LEFT, ( 5* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

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

	GLTaxiStation::GetInstance().LoadFile( "taxistation.tsf" );

	m_mapList.CleanUp();
	m_mapList.LoadMapsListFile( "mapslist.mst" );

	DataShow();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEditorTaxiDlg::OnPaint()
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
HCURSOR CEditorTaxiDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CEditorTaxiDlg::PostNcDestroy()
{
	//GLTaxiStation::GetInstance().CleanUp();
	DxResponseMan::GetInstance().FinalCleanup();
	GMTOOL::CleanUp ();
	CDialog::PostNcDestroy();
}

BOOL CEditorTaxiDlg::DataShow()
{
	SetWin_Num_int( this, IDC_EDIT_BASICCHARGE, GLTaxiStation::GetInstance().GetBasicCharge() );

	UpdateTaxiList();

	return TRUE;
}

BOOL CEditorTaxiDlg::DataSave()
{
	DWORD dwBasicCharge = GetWin_Num_int( this, IDC_EDIT_BASICCHARGE );
	GLTaxiStation::GetInstance().SetBasicCharge( dwBasicCharge );

	return TRUE;
}

void CEditorTaxiDlg::UpdateTaxiList()
{
	m_ListTaxi.DeleteAllItems();
	m_ListTaxi.SetRedraw( FALSE );

	GLTaxiStation::TAXISTATION_VEC vecTaxiData = GLTaxiStation::GetInstance().m_vecTaxiData;
	for( int i = 0; i < (int)vecTaxiData.size(); ++ i )
	{
		const STAXI_STATION sTaxiStaion = vecTaxiData[i];

		m_ListTaxi.InsertItem( i, _HLIB::cstring_dword(sTaxiStaion.dwLINKID).GetString() );
		m_ListTaxi.SetItemText( i, 1, _HLIB::cstring_int(sTaxiStaion.bUsed).GetString() );

		SNATIVEID sMAPID( sTaxiStaion.dwMAPID );
		SNATIVEID sCROWID( sTaxiStaion.dwNPCID );
		SMAPNODE* pNode = m_mapList.FindMapNode( sMAPID );
		SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData( sCROWID );
		m_ListTaxi.SetItemText( i, 2, _HLIB::cstringformat("[%03d~%03d]%s", 
			sMAPID.wMainID, sMAPID.wSubID, pNode? pNode->strMapName.c_str(): "Unknown Map").GetString() );
		m_ListTaxi.SetItemText( i, 3, _HLIB::cstringformat("[%03d~%03d]%s", 
			sCROWID.wMainID, sCROWID.wSubID, pCROW? pCROW->GetName(): "Unknown Crow").GetString() );
		m_ListTaxi.SetItemData( i, sTaxiStaion.dwLINKID );
	}

	m_ListTaxi.SetRedraw( TRUE );

	STAXI_STATION sTaxiData;
	TaxiDataShow( -1, sTaxiData, FALSE );
}

void CEditorTaxiDlg::TaxiDataShow( int nIndex, STAXI_STATION sTaxi, BOOL bENABLE )
{
	SNATIVEID sIDMAP( sTaxi.dwMAPID );
	SNATIVEID sIDNPC( sTaxi.dwNPCID );
	SetWin_Num_int( this, IDC_EDIT_TAXI_INDEX, nIndex );
	SetWin_Num_int( this, IDC_EDIT_TAXI_ID, sTaxi.dwLINKID );
	SetWin_Check( this, IDC_CHECK_TAXI_ENABLE, sTaxi.bUsed );
	SetWin_Num_int( this, IDC_EDIT_TAXI_MAP_MID, sIDMAP.wMainID );
	SetWin_Num_int( this, IDC_EDIT_TAXI_MAP_SID, sIDMAP.wSubID );
	SetWin_Num_int( this, IDC_EDIT_TAXI_NPC_MID, sIDNPC.wMainID );
	SetWin_Num_int( this, IDC_EDIT_TAXI_NPC_SID, sIDNPC.wSubID );
	SetWin_Num_int( this, IDC_EDIT_TAXI_POS_X, sTaxi.wPosX );
	SetWin_Num_int( this, IDC_EDIT_TAXI_POS_Y, sTaxi.wPosY );
	SetWin_Num_int( this, IDC_EDIT_TAXI_CHARGE, sTaxi.dwMapCharge );

	SetWin_Enable( this, IDC_EDIT_TAXI_INDEX, bENABLE );
	SetWin_Enable( this, IDC_EDIT_TAXI_ID, bENABLE );
	SetWin_Enable( this, IDC_CHECK_TAXI_ENABLE, bENABLE );
	SetWin_Enable( this, IDC_EDIT_TAXI_MAP_MID, bENABLE );
	SetWin_Enable( this, IDC_EDIT_TAXI_MAP_SID, bENABLE );
	SetWin_Enable( this, IDC_EDIT_TAXI_NPC_MID, bENABLE );
	SetWin_Enable( this, IDC_EDIT_TAXI_NPC_SID, bENABLE );
	SetWin_Enable( this, IDC_EDIT_TAXI_POS_X, bENABLE );
	SetWin_Enable( this, IDC_EDIT_TAXI_POS_Y, bENABLE );
	SetWin_Enable( this, IDC_EDIT_TAXI_CHARGE, bENABLE );
	SetWin_Enable( this, IDC_EDIT_TAXI_MAP_NAME, bENABLE );
	SetWin_Enable( this, IDC_EDIT_TAXI_NPC_NAME, bENABLE );

	SetWin_Enable( this, IDC_BUTTON_TAXIEDIT_MAP, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_TAXIEDIT_NPC, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_TAXIEDIT_SAVE, bENABLE );
}

void CEditorTaxiDlg::OnBnClickedButtonFileLoad()
{
	CString szFilter = "TaxiStation Setting File (*.tsf)|*.tsf|";
	CFileDialog dlg( TRUE, ".tsf", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr = GLTaxiStation::GetInstance().LoadFile ( dlg.GetFileName().GetString() );
		if ( FAILED ( hr ) )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to load TaxiStation Setting File." );
		}

		DataShow();
	}
}

void CEditorTaxiDlg::OnBnClickedButtonFileSave()
{
	if ( !DataSave() )	return;

	CString szFilter = "TaxiStation Setting File (*.tsf)|*.tsf|";
	CFileDialog dlg( FALSE, ".tsf", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr = GLTaxiStation::GetInstance().SaveFile ( dlg.GetFileName().GetString() );
		if ( FAILED ( hr ) )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to Save File" );
			return;
		}
	}
}

void CEditorTaxiDlg::OnBnClickedButtonFileImport()
{
	CString szFilter = "TaxiStation Ini File (*.ini)|*.ini|";
	CFileDialog dlg(TRUE,".ini",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		BOOL bOK = GLTaxiStation::GetInstance().IMPORT ( dlg.GetPathName().GetString() );
		if ( !bOK )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to import TaxiStation ini File." );
		}

		DataShow();
	}
}

void CEditorTaxiDlg::OnBnClickedButtonTaxiAdd()
{
	GLTaxiStation::TAXISTATION_VEC vecTaxiData = GLTaxiStation::GetInstance().m_vecTaxiData;
	for( int i=0; i<GLTaxiStation::TAXI_MAXID; ++i )
	{
		BOOL bUSED = FALSE;
		for( int j=0; j<(int)vecTaxiData.size(); ++j )
		{
			if ( i == (int)vecTaxiData[j].dwLINKID )
				bUSED = TRUE;
		}

		if( bUSED )	{
			continue;
		}else
		{
			STAXI_STATION sStation;
			sStation.dwLINKID = (DWORD)i;
			sStation.dwMAPID = NATIVEID_NULL().dwID;
			sStation.dwNPCID = NATIVEID_NULL().dwID;
			TaxiDataShow( -1, sStation, TRUE );
			break;
		}
	}
}

void CEditorTaxiDlg::OnBnClickedButtonTaxiEdit()
{
	TaxiEdit();
}

void CEditorTaxiDlg::OnBnClickedButtonTaxiDelete()
{
	int nSelect = m_ListTaxi.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwLinkID = m_ListTaxi.GetItemData( nSelect );

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete TaxiStation ID:%02d", dwLinkID ) !=  IDYES )
		return;

	GLTaxiStation::TAXISTATION_VEC &vecTaxiData = GLTaxiStation::GetInstance().m_vecTaxiData;

	if ( nSelect >= (int)vecTaxiData.size() )	return;

	vecTaxiData.erase( vecTaxiData.begin() + nSelect );
	UpdateTaxiList();
	
	int nNewSelect = nSelect > 1? nSelect-1:nSelect;
	m_ListTaxi.EnsureVisible( nNewSelect, TRUE );
	m_ListTaxi.SetItemState( nNewSelect, LVIS_SELECTED, LVIS_SELECTED );
}

void CEditorTaxiDlg::OnBnClickedButtonTaxiClear()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Clear TaxiStation?" ) ==  IDYES )
	{
		GLTaxiStation::TAXISTATION_VEC &vecTaxiData = GLTaxiStation::GetInstance().m_vecTaxiData;
		vecTaxiData.clear();
		UpdateTaxiList();
	}
}

void CEditorTaxiDlg::OnNMDblclkListTaxi(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	TaxiEdit();
}

void CEditorTaxiDlg::TaxiEdit()
{
	int nSelect = m_ListTaxi.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwLinkID = m_ListTaxi.GetItemData( nSelect );

	GLTaxiStation::TAXISTATION_VEC vecTaxiData = GLTaxiStation::GetInstance().m_vecTaxiData;
	if ( nSelect >= (int)vecTaxiData.size() )	return;

	const STAXI_STATION sStation = vecTaxiData[nSelect];
	if ( sStation.dwLINKID != dwLinkID )	return;

	TaxiDataShow( nSelect, sStation, TRUE );
}

void CEditorTaxiDlg::OnBnClickedButtonTaxieditMap()
{
	CDlgMap dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SMAPNODE* pNode = m_mapList.FindMapNode( dlg.m_SELECTED );
		if ( !pNode )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Map: %d~%d", dlg.m_SELECTED.wMainID, dlg.m_SELECTED.wSubID );
			return;
		}

		SetWin_Num_int( this, IDC_EDIT_TAXI_MAP_MID, dlg.m_SELECTED.wMainID );
		SetWin_Num_int( this, IDC_EDIT_TAXI_MAP_SID, dlg.m_SELECTED.wSubID );
	}
}

void CEditorTaxiDlg::OnBnClickedButtonTaxieditNpc()
{
	CDlgCrow dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData( dlg.m_SELECTED );
		if ( !pCROW )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Crow: %d~%d", dlg.m_SELECTED.wMainID, dlg.m_SELECTED.wSubID );
			return;
		}

		if ( pCROW->m_emCrow != CROW_NPC )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Crow Not NPC! : %d~%d~%s", dlg.m_SELECTED.wMainID, dlg.m_SELECTED.wSubID, pCROW->GetName() );
			return;
		}

		SetWin_Num_int( this, IDC_EDIT_TAXI_NPC_MID, dlg.m_SELECTED.wMainID );
		SetWin_Num_int( this, IDC_EDIT_TAXI_NPC_SID, dlg.m_SELECTED.wSubID );
	}
}

void CEditorTaxiDlg::OnBnClickedButtonTaxieditSave()
{
	int nINDEX = GetWin_Num_int( this, IDC_EDIT_TAXI_INDEX );

	STAXI_STATION sStation;
	SNATIVEID sIDMAP;
	SNATIVEID sIDNPC;

	sStation.dwLINKID = GetWin_Num_int( this, IDC_EDIT_TAXI_ID );
	sStation.bUsed = GetWin_Check( this, IDC_CHECK_TAXI_ENABLE )? TRUE:FALSE;
	sStation.wPosX = GetWin_Num_int( this, IDC_EDIT_TAXI_POS_X );
	sStation.wPosY = GetWin_Num_int( this, IDC_EDIT_TAXI_POS_Y );
	sStation.dwMapCharge = GetWin_Num_int( this, IDC_EDIT_TAXI_CHARGE );

	sIDMAP.wMainID = GetWin_Num_int( this, IDC_EDIT_TAXI_MAP_MID );
	sIDMAP.wSubID = GetWin_Num_int( this, IDC_EDIT_TAXI_MAP_SID );
	sIDNPC.wMainID = GetWin_Num_int( this, IDC_EDIT_TAXI_NPC_MID );
	sIDNPC.wSubID = GetWin_Num_int( this, IDC_EDIT_TAXI_NPC_SID );

	sStation.dwMAPID = sIDMAP.dwID;
	sStation.dwNPCID = sIDNPC.dwID;
	sStation.strMAP = GetWin_Text( this, IDC_EDIT_TAXI_MAP_NAME );
	sStation.strSTATION = GetWin_Text( this, IDC_EDIT_TAXI_NPC_NAME );

	SMAPNODE* pNode = m_mapList.FindMapNode( sIDMAP );
	if ( !pNode )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Map: %d~%d", sIDMAP.wMainID, sIDMAP.wSubID );
		return;
	}

	SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData( sIDNPC );
	if ( !pCROW )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Crow: %d~%d", sIDNPC.wMainID, sIDNPC.wSubID );
		return;
	}

	if ( pCROW->m_emCrow != CROW_NPC )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Crow Not NPC! : %d~%d~%s", sIDNPC.wMainID, sIDNPC.wSubID, pCROW->GetName() );
		return;
	}


	if ( nINDEX == -1 )
	{
		//add mode
		GLTaxiStation::TAXISTATION_VEC &vecTaxiData = GLTaxiStation::GetInstance().m_vecTaxiData;
		vecTaxiData.push_back( sStation );
		UpdateTaxiList();

		for( int i=0; i<(int)vecTaxiData.size(); ++i )
		{
			if ( sStation.dwLINKID == vecTaxiData[i].dwLINKID && 
				sStation.dwMAPID == vecTaxiData[i].dwMAPID &&
				sStation.dwNPCID == vecTaxiData[i].dwNPCID )
			{
				TaxiDataShow( i, sStation, TRUE );
				break;
			}
		}
	}
	else
	{
		GLTaxiStation::TAXISTATION_VEC &vecTaxiData = GLTaxiStation::GetInstance().m_vecTaxiData;
		for( int i=0; i<(int)vecTaxiData.size(); ++i )
		{
			if ( i == nINDEX && vecTaxiData[i].dwLINKID == sStation.dwLINKID )
			{
				vecTaxiData[i] = sStation;

				UpdateTaxiList();
				TaxiDataShow( nINDEX, sStation, TRUE );
				break;
			}
		}
	}

	for( int nItem = 0; nItem < m_ListTaxi.GetItemCount(); nItem++)
	{
		DWORD dwID = m_ListTaxi.GetItemData( nItem );
		if( dwID == sStation.dwLINKID )
		{
			m_ListTaxi.EnsureVisible( nItem, TRUE );
			m_ListTaxi.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
		}
	}
}

void CEditorTaxiDlg::OnEnChangeEditTaxiMapMid()
{
	SNATIVEID sID;
	sID.wMainID = GetWin_Num_int( this, IDC_EDIT_TAXI_MAP_MID );
	sID.wSubID = GetWin_Num_int( this, IDC_EDIT_TAXI_MAP_SID );

	SMAPNODE* pNode = m_mapList.FindMapNode( sID );
	SetWin_Text( this, IDC_EDIT_TAXI_MAP_NAME, pNode? pNode->strMapName.c_str() : "Unknown map" );
}

void CEditorTaxiDlg::OnEnChangeEditTaxiMapSid()
{
	SNATIVEID sID;
	sID.wMainID = GetWin_Num_int( this, IDC_EDIT_TAXI_MAP_MID );
	sID.wSubID = GetWin_Num_int( this, IDC_EDIT_TAXI_MAP_SID );

	SMAPNODE* pNode = m_mapList.FindMapNode( sID );
	SetWin_Text( this, IDC_EDIT_TAXI_MAP_NAME, pNode? pNode->strMapName.c_str() : "Unknown map" );
}

void CEditorTaxiDlg::OnEnChangeEditTaxiNpcMid()
{
	SNATIVEID sID;
	sID.wMainID = GetWin_Num_int( this, IDC_EDIT_TAXI_NPC_MID );
	sID.wSubID = GetWin_Num_int( this, IDC_EDIT_TAXI_NPC_SID );

	SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData( sID );
	SetWin_Text( this, IDC_EDIT_TAXI_NPC_NAME, pCROW? pCROW->GetName() : "Unknown Crow" );
}

void CEditorTaxiDlg::OnEnChangeEditTaxiNpcSid()
{
	SNATIVEID sID;
	sID.wMainID = GetWin_Num_int( this, IDC_EDIT_TAXI_NPC_MID );
	sID.wSubID = GetWin_Num_int( this, IDC_EDIT_TAXI_NPC_SID );

	SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData( sID );
	SetWin_Text( this, IDC_EDIT_TAXI_NPC_NAME, pCROW? pCROW->GetName() : "Unknown Crow" );
}
