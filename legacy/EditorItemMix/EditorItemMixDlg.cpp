
// EditorItemMixDlg.cpp : implementation file
//

#include "stdafx.h"
#include <map>

#include "EditorItemMix.h"
#include "EditorItemMixDlg.h"
#include "DxResponseMan.h"
#include "RANPARAM.h"
#include "DxServerInstance.h"
#include "GLOGIC.h"
#include "EtcFunction.h"
#include "GLItemMixMan.h"
#include "GLItemMan.h"
#include "DlgFindItem.h"
#include "HLibDataConvert.h"

/*product item, Juver, 2017/10/15 */
#include "GLProductDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEditorItemMixDlg dialog




CEditorItemMixDlg::CEditorItemMixDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditorItemMixDlg::IDD, pParent)
	, m_bAdd(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEditorItemMixDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ITEMMIX, m_ListItemMix );
}

BEGIN_MESSAGE_MAP(CEditorItemMixDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_FILE_LOAD, OnBnClickedButtonFileLoad)
	ON_BN_CLICKED(IDC_BUTTON_FILE_SAVE, OnBnClickedButtonFileSave)
	ON_BN_CLICKED(IDC_BUTTON_FILE_IMPORT, OnBnClickedButtonFileImport)
	ON_BN_CLICKED(IDC_BUTTON_ITEMMIX_ADD, OnBnClickedButtonItemmixAdd)
	ON_BN_CLICKED(IDC_BUTTON_ITEMMIX_EDIT, OnBnClickedButtonItemmixEdit)
	ON_BN_CLICKED(IDC_BUTTON_ITEMMIX_DELETE, OnBnClickedButtonItemmixDelete)
	ON_BN_CLICKED(IDC_BUTTON_ITEMMIX_CLEAR, OnBnClickedButtonItemmixClear)
	
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ITEMMIX, OnNMDblclkListItemmix)
	ON_BN_CLICKED(IDC_BUTTON_ITEMMIX_RESULT, OnBnClickedButtonItemmixResult)
	ON_BN_CLICKED(IDC_BUTTON_ITEMMIX_0, OnBnClickedButtonItemmix0)
	ON_BN_CLICKED(IDC_BUTTON_ITEMMIX_1, OnBnClickedButtonItemmix1)
	ON_BN_CLICKED(IDC_BUTTON_ITEMMIX_2, OnBnClickedButtonItemmix2)
	ON_BN_CLICKED(IDC_BUTTON_ITEMMIX_3, OnBnClickedButtonItemmix3)
	ON_BN_CLICKED(IDC_BUTTON_ITEMMIX_4, OnBnClickedButtonItemmix4)
	ON_BN_CLICKED(IDC_BUTTON_ITEMMIX_DEL_RESULT, OnBnClickedButtonItemmixDelResult)
	ON_BN_CLICKED(IDC_BUTTON_ITEMMIX_DEL_0, OnBnClickedButtonItemmixDel0)
	ON_BN_CLICKED(IDC_BUTTON_ITEMMIX_DEL_1, OnBnClickedButtonItemmixDel1)
	ON_BN_CLICKED(IDC_BUTTON_ITEMMIX_DEL_2, OnBnClickedButtonItemmixDel2)
	ON_BN_CLICKED(IDC_BUTTON_ITEMMIX_DEL_3, OnBnClickedButtonItemmixDel3)
	ON_BN_CLICKED(IDC_BUTTON_ITEMMIX_DEL_4, OnBnClickedButtonItemmixDel4)
	ON_EN_CHANGE(IDC_EDIT_MID_RESULT, OnEnChangeEditMidResult)
	ON_EN_CHANGE(IDC_EDIT_SID_RESULT, OnEnChangeEditSidResult)
	ON_EN_CHANGE(IDC_EDIT_MID_ITEMMIX_0, OnEnChangeEditMidItemmix0)
	ON_EN_CHANGE(IDC_EDIT_SID_ITEMMIX_0, OnEnChangeEditSidItemmix0)
	ON_EN_CHANGE(IDC_EDIT_MID_ITEMMIX_1, OnEnChangeEditMidItemmix1)
	ON_EN_CHANGE(IDC_EDIT_SID_ITEMMIX_1, OnEnChangeEditSidItemmix1)
	ON_EN_CHANGE(IDC_EDIT_MID_ITEMMIX_2, OnEnChangeEditMidItemmix2)
	ON_EN_CHANGE(IDC_EDIT_SID_ITEMMIX_2, OnEnChangeEditSidItemmix2)
	ON_EN_CHANGE(IDC_EDIT_MID_ITEMMIX_3, OnEnChangeEditMidItemmix3)
	ON_EN_CHANGE(IDC_EDIT_SID_ITEMMIX_3, OnEnChangeEditSidItemmix3)
	ON_EN_CHANGE(IDC_EDIT_MID_ITEMMIX_4, OnEnChangeEditMidItemmix4)
	ON_EN_CHANGE(IDC_EDIT_SID_ITEMMIX_4, OnEnChangeEditSidItemmix4)
	ON_BN_CLICKED(IDC_BUTTON_ITEMMIX_SAVE, OnBnClickedButtonItemmixSave)
END_MESSAGE_MAP()


// CEditorItemMixDlg message handlers

BOOL CEditorItemMixDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	RECT rectCtrl;
	m_ListItemMix.SetExtendedStyle ( m_ListItemMix.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_ListItemMix.GetClientRect ( &rectCtrl );
	m_ListItemMix.InsertColumn( 0, "ID", LVCFMT_LEFT, ( 15* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListItemMix.InsertColumn( 1, "ItemResult", LVCFMT_LEFT, ( 80* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListItemMix.InsertColumn( 2, "", LVCFMT_LEFT, ( 5* ( rectCtrl.right - rectCtrl.left ) ) / 100 );


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

	GLItemMixMan::GetInstance().LoadFile( "itemmix.ims", true );

	/*product item, Juver, 2017/10/15 */
	SetWin_Combo_Init( this, IDC_COMBO_CATEGORY, COMMENT::PRODUCT_TYPE_NAME, PRODUCT_TYPE_SIZE );


	UpdateItemMixList();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEditorItemMixDlg::OnPaint()
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
HCURSOR CEditorItemMixDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CEditorItemMixDlg::PostNcDestroy()
{
	GLItemMixMan::GetInstance().CleanUp();
	DxResponseMan::GetInstance().FinalCleanup();
	GMTOOL::CleanUp ();
	CDialog::PostNcDestroy();
}

void CEditorItemMixDlg::UpdateItemMixList()
{
	m_ListItemMix.DeleteAllItems();
	m_ListItemMix.SetRedraw( FALSE );

	ITEMMIX_KEYMAP mapItemMix = GLItemMixMan::GetInstance().m_mapKeyItemMix;

	int nINDEX = 0;
	for( DWORD dwKey = 0; dwKey<GLItemMixMan::ITEMMIX_MAXKEY; ++dwKey )
	{
		ITEMMIX_KEYMAP_ITER iterpos = mapItemMix.find(dwKey);
		if ( iterpos != mapItemMix.end() )
		{
			const ITEM_MIX sItemMix = (*iterpos).second;

			m_ListItemMix.InsertItem( nINDEX, _HLIB::cstringformat( "%d", sItemMix.dwKey ).GetString() );
			SITEM* pItem = GLItemMan::GetInstance().GetItem( sItemMix.sResultItem.sNID );
			m_ListItemMix.SetItemText( nINDEX, 1, _HLIB::cstringformat( "[%04d~%04d]%s", 
				sItemMix.sResultItem.sNID.wMainID, sItemMix.sResultItem.sNID.wSubID, pItem? pItem->GetName(): "Unknown Item" ).GetString() );
			m_ListItemMix.SetItemData( nINDEX, sItemMix.dwKey );
			nINDEX ++;
		}
	}

	m_ListItemMix.SetRedraw( TRUE );

	ITEM_MIX sITEMMIX;
	ItemMixShow( sITEMMIX, FALSE, FALSE );
}

void CEditorItemMixDlg::OnBnClickedButtonFileLoad()
{
	CString szFilter = "Item Mix Setting File (*.ims)|*.ims|";
	CFileDialog dlg( TRUE, ".ims", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr = GLItemMixMan::GetInstance().LoadFile ( dlg.GetFileName().GetString(), TRUE );
		if ( FAILED ( hr ) )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to load Item Mix Setting File." );
		}

		UpdateItemMixList();
	}
}

void CEditorItemMixDlg::OnBnClickedButtonFileSave()
{
	CString szFilter = "Item Mix Setting File (*.ims)|*.ims|";
	CFileDialog dlg( FALSE, ".ims", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr = GLItemMixMan::GetInstance().SaveFile ( dlg.GetFileName().GetString() );
		if ( FAILED ( hr ) )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to Save File" );
			return;
		}
	}
}

void CEditorItemMixDlg::OnBnClickedButtonFileImport()
{
	CString szFilter = "Item Mix Ini File (*.ini)|*.ini|";
	CFileDialog dlg(TRUE,".ini",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		HRESULT hr = GLItemMixMan::GetInstance().Import ( dlg.GetPathName().GetString() );
		if ( FAILED ( hr ) )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to import Item Mix ini File." );
		}

		UpdateItemMixList();
	}
}

void CEditorItemMixDlg::OnBnClickedButtonItemmixAdd()
{
	ITEMMIX_KEYMAP &mapItemMix = GLItemMixMan::GetInstance().m_mapKeyItemMix;

	DWORD dwNEWKEY = UINT_MAX;
	for( DWORD dwKey = 0; dwKey<GLItemMixMan::ITEMMIX_MAXKEY; ++dwKey )
	{
		ITEMMIX_KEYMAP_ITER iterpos = mapItemMix.find(dwKey);
		if ( iterpos == mapItemMix.end() )
		{
			dwNEWKEY = dwKey;
			break;
		}
	}

	if ( dwNEWKEY == UINT_MAX || dwNEWKEY >= GLItemMixMan::ITEMMIX_MAXKEY )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Unable to generate new ItemMix Key" );
		return;
	}

	ITEM_MIX sITEMMIX;
	sITEMMIX.dwKey = dwNEWKEY;
	ItemMixShow( sITEMMIX, TRUE, TRUE );
}

void CEditorItemMixDlg::OnBnClickedButtonItemmixEdit()
{
	ItemMixEdit();
}

void CEditorItemMixDlg::OnBnClickedButtonItemmixDelete()
{
	int nSelect = m_ListItemMix.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwKey = m_ListItemMix.GetItemData( nSelect );

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete ItemMix ID:%02d", dwKey ) !=  IDYES )
		return;

	ITEMMIX_KEYMAP &mapItemMix = GLItemMixMan::GetInstance().m_mapKeyItemMix;
	ITEMMIX_KEYMAP_ITER iterpos = mapItemMix.find(dwKey);
	if ( iterpos != mapItemMix.end() )
	{
		mapItemMix.erase( iterpos );
		UpdateItemMixList();
	}

	int nNewSelect = nSelect > 1? nSelect-1:nSelect;
	m_ListItemMix.EnsureVisible( nNewSelect, TRUE );
	m_ListItemMix.SetItemState( nNewSelect, LVIS_SELECTED, LVIS_SELECTED );
}

void CEditorItemMixDlg::OnBnClickedButtonItemmixClear()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Clear ItemMix?" ) ==  IDYES )
	{
		GLItemMixMan::GetInstance().m_mapKeyItemMix.clear();
		UpdateItemMixList();
	}
}

void CEditorItemMixDlg::OnNMDblclkListItemmix(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	ItemMixEdit();
}

void CEditorItemMixDlg::ItemMixEdit()
{
	int nSelect = m_ListItemMix.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwKey = m_ListItemMix.GetItemData( nSelect );

	ITEMMIX_KEYMAP &mapItemMix = GLItemMixMan::GetInstance().m_mapKeyItemMix;
	ITEMMIX_KEYMAP_ITER iterpos = mapItemMix.find(dwKey);
	if ( iterpos != mapItemMix.end() )
	{

		ITEM_MIX sITEMMIX = (*iterpos).second;
		ItemMixShow( sITEMMIX, TRUE, FALSE );
	}
}

void CEditorItemMixDlg::OnBnClickedButtonItemmixResult()
{
	CDlgFindItem dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( dlg.m_sSELECTED );
		if ( pITEM )
		{
			SetWin_Num_int( this, IDC_EDIT_MID_RESULT, dlg.m_sSELECTED.wMainID );
			SetWin_Num_int( this, IDC_EDIT_SID_RESULT, dlg.m_sSELECTED.wSubID );
		}
	}
}

void CEditorItemMixDlg::OnBnClickedButtonItemmix0()
{
	CDlgFindItem dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( dlg.m_sSELECTED );
		if ( pITEM )
		{
			SetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_0, dlg.m_sSELECTED.wMainID );
			SetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_0, dlg.m_sSELECTED.wSubID );
		}
	}
}

void CEditorItemMixDlg::OnBnClickedButtonItemmix1()
{
	CDlgFindItem dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( dlg.m_sSELECTED );
		if ( pITEM )
		{
			SetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_1, dlg.m_sSELECTED.wMainID );
			SetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_1, dlg.m_sSELECTED.wSubID );
		}
	}
}

void CEditorItemMixDlg::OnBnClickedButtonItemmix2()
{
	CDlgFindItem dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( dlg.m_sSELECTED );
		if ( pITEM )
		{
			SetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_2, dlg.m_sSELECTED.wMainID );
			SetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_2, dlg.m_sSELECTED.wSubID );
		}
	}
}

void CEditorItemMixDlg::OnBnClickedButtonItemmix3()
{
	CDlgFindItem dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( dlg.m_sSELECTED );
		if ( pITEM )
		{
			SetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_3, dlg.m_sSELECTED.wMainID );
			SetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_3, dlg.m_sSELECTED.wSubID );
		}
	}
}

void CEditorItemMixDlg::OnBnClickedButtonItemmix4()
{
	CDlgFindItem dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( dlg.m_sSELECTED );
		if ( pITEM )
		{
			SetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_4, dlg.m_sSELECTED.wMainID );
			SetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_4, dlg.m_sSELECTED.wSubID );
		}
	}
}

void CEditorItemMixDlg::OnBnClickedButtonItemmixDelResult()
{
	SNATIVEID sID( false );
	SetWin_Num_int( this, IDC_EDIT_MID_RESULT, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SID_RESULT, sID.wSubID );
	SetWin_Num_int( this, IDC_EDIT_NUM_RESULT, 0 );
}

void CEditorItemMixDlg::OnBnClickedButtonItemmixDel0()
{
	SNATIVEID sID( false );
	SetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_0, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_0, sID.wSubID );
	SetWin_Num_int( this, IDC_EDIT_NUM_ITEMMIX_0, 0 );
}

void CEditorItemMixDlg::OnBnClickedButtonItemmixDel1()
{
	SNATIVEID sID( false );
	SetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_1, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_1, sID.wSubID );
	SetWin_Num_int( this, IDC_EDIT_NUM_ITEMMIX_1, 0 );
}

void CEditorItemMixDlg::OnBnClickedButtonItemmixDel2()
{
	SNATIVEID sID( false );
	SetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_2, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_2, sID.wSubID );
	SetWin_Num_int( this, IDC_EDIT_NUM_ITEMMIX_2, 0 );
}

void CEditorItemMixDlg::OnBnClickedButtonItemmixDel3()
{
	SNATIVEID sID( false );
	SetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_3, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_3, sID.wSubID );
	SetWin_Num_int( this, IDC_EDIT_NUM_ITEMMIX_3, 0 );
}

void CEditorItemMixDlg::OnBnClickedButtonItemmixDel4()
{
	SNATIVEID sID( false );
	SetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_4, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_4, sID.wSubID );
	SetWin_Num_int( this, IDC_EDIT_NUM_ITEMMIX_4, 0 );
}

void CEditorItemMixDlg::OnEnChangeEditMidResult()
{
	CheckItem( IDC_EDIT_MID_RESULT, IDC_EDIT_SID_RESULT, IDC_EDIT_NAME_RESULT );
}

void CEditorItemMixDlg::OnEnChangeEditSidResult()
{
	CheckItem( IDC_EDIT_MID_RESULT, IDC_EDIT_SID_RESULT, IDC_EDIT_NAME_RESULT );
}

void CEditorItemMixDlg::OnEnChangeEditMidItemmix0()
{
	CheckItem( IDC_EDIT_MID_ITEMMIX_0, IDC_EDIT_SID_ITEMMIX_0, IDC_EDIT_NAME_ITEMMIX_0 );
}

void CEditorItemMixDlg::OnEnChangeEditSidItemmix0()
{
	CheckItem( IDC_EDIT_MID_ITEMMIX_0, IDC_EDIT_SID_ITEMMIX_0, IDC_EDIT_NAME_ITEMMIX_0 );
}

void CEditorItemMixDlg::OnEnChangeEditMidItemmix1()
{
	CheckItem( IDC_EDIT_MID_ITEMMIX_1, IDC_EDIT_SID_ITEMMIX_1, IDC_EDIT_NAME_ITEMMIX_1 );
}

void CEditorItemMixDlg::OnEnChangeEditSidItemmix1()
{
	CheckItem( IDC_EDIT_MID_ITEMMIX_1, IDC_EDIT_SID_ITEMMIX_1, IDC_EDIT_NAME_ITEMMIX_1 );
}

void CEditorItemMixDlg::OnEnChangeEditMidItemmix2()
{
	CheckItem( IDC_EDIT_MID_ITEMMIX_2, IDC_EDIT_SID_ITEMMIX_2, IDC_EDIT_NAME_ITEMMIX_2 );
}

void CEditorItemMixDlg::OnEnChangeEditSidItemmix2()
{
	CheckItem( IDC_EDIT_MID_ITEMMIX_2, IDC_EDIT_SID_ITEMMIX_2, IDC_EDIT_NAME_ITEMMIX_2 );
}

void CEditorItemMixDlg::OnEnChangeEditMidItemmix3()
{
	CheckItem( IDC_EDIT_MID_ITEMMIX_3, IDC_EDIT_SID_ITEMMIX_3, IDC_EDIT_NAME_ITEMMIX_3 );
}

void CEditorItemMixDlg::OnEnChangeEditSidItemmix3()
{
	CheckItem( IDC_EDIT_MID_ITEMMIX_3, IDC_EDIT_SID_ITEMMIX_3, IDC_EDIT_NAME_ITEMMIX_3 );
}

void CEditorItemMixDlg::OnEnChangeEditMidItemmix4()
{
	CheckItem( IDC_EDIT_MID_ITEMMIX_4, IDC_EDIT_SID_ITEMMIX_4, IDC_EDIT_NAME_ITEMMIX_4 );
}

void CEditorItemMixDlg::OnEnChangeEditSidItemmix4()
{
	CheckItem( IDC_EDIT_MID_ITEMMIX_4, IDC_EDIT_SID_ITEMMIX_4, IDC_EDIT_NAME_ITEMMIX_4 );
}

void CEditorItemMixDlg::CheckItem( int nMID, int nSID, int nNAME )
{
	WORD wMID = GetWin_Num_int( this, nMID );
	WORD wSID = GetWin_Num_int( this, nSID );

	SITEM* pItem = GLItemMan::GetInstance().GetItem( wMID, wSID );
	SetWin_Text( this, nNAME, pItem? pItem->GetName():"Unknown Item" );
}

void CEditorItemMixDlg::OnBnClickedButtonItemmixSave()
{
	ITEM_MIX sITEMMIX;
	sITEMMIX.dwKey = GetWin_Num_int( this, IDC_EDIT_ITEMMIX_ID );
	sITEMMIX.dwRate = GetWin_Num_int( this, IDC_EDIT_ITEMMIX_RATE );
	sITEMMIX.dwPrice = GetWin_Num_int( this, IDC_EDIT_ITEMMIX_PRICE );

	sITEMMIX.sResultItem.sNID.wMainID = GetWin_Num_int( this, IDC_EDIT_MID_RESULT );
	sITEMMIX.sResultItem.sNID.wSubID = GetWin_Num_int( this, IDC_EDIT_SID_RESULT );
	sITEMMIX.sResultItem.nNum = (BYTE)GetWin_Num_int( this, IDC_EDIT_NUM_RESULT );

	sITEMMIX.sMeterialItem[0].sNID.wMainID = GetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_0 );
	sITEMMIX.sMeterialItem[0].sNID.wSubID = GetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_0 );
	sITEMMIX.sMeterialItem[0].nNum = (BYTE)GetWin_Num_int( this, IDC_EDIT_NUM_ITEMMIX_0 );

	sITEMMIX.sMeterialItem[1].sNID.wMainID = GetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_1 );
	sITEMMIX.sMeterialItem[1].sNID.wSubID = GetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_1 );
	sITEMMIX.sMeterialItem[1].nNum = (BYTE)GetWin_Num_int( this, IDC_EDIT_NUM_ITEMMIX_1 );

	sITEMMIX.sMeterialItem[2].sNID.wMainID = GetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_2 );
	sITEMMIX.sMeterialItem[2].sNID.wSubID = GetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_2 );
	sITEMMIX.sMeterialItem[2].nNum = (BYTE)GetWin_Num_int( this, IDC_EDIT_NUM_ITEMMIX_2 );

	sITEMMIX.sMeterialItem[3].sNID.wMainID = GetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_3 );
	sITEMMIX.sMeterialItem[3].sNID.wSubID = GetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_3 );
	sITEMMIX.sMeterialItem[3].nNum = (BYTE)GetWin_Num_int( this, IDC_EDIT_NUM_ITEMMIX_3 );

	sITEMMIX.sMeterialItem[4].sNID.wMainID = GetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_4 );
	sITEMMIX.sMeterialItem[4].sNID.wSubID = GetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_4 );
	sITEMMIX.sMeterialItem[4].nNum = (BYTE)GetWin_Num_int( this, IDC_EDIT_NUM_ITEMMIX_4 );

	/*product item, Juver, 2017/10/15 */
	sITEMMIX.wCategory = GetWin_Combo_Sel ( this, IDC_COMBO_CATEGORY );
	sITEMMIX.fTime = GetWin_Num_float( this, IDC_EDIT_DATA_TIME );
	sITEMMIX.wLevelReq = GetWin_Num_int( this, IDC_EDIT_DATA_LEVEL );

	sITEMMIX.dwData1 = GetWin_Num_int( this, IDC_EDIT_DATA_3 );
	sITEMMIX.dwData2 = GetWin_Num_int( this, IDC_EDIT_DATA_4 );
	sITEMMIX.nData1  = GetWin_Num_int( this, IDC_EDIT_DATA_5 );
	sITEMMIX.nData2 = GetWin_Num_int( this, IDC_EDIT_DATA_6 );
	sITEMMIX.wData1 = GetWin_Num_int( this, IDC_EDIT_DATA_9 );
	sITEMMIX.wData2 = GetWin_Num_int( this, IDC_EDIT_DATA_10 );
	sITEMMIX.wData3 = GetWin_Num_int( this, IDC_EDIT_DATA_11 );

	sITEMMIX.cDAMAGE = (BYTE)GetWin_Num_int( this, IDC_EDIT_DATA_ITEM_ATT );
	sITEMMIX.cDEFENSE = (BYTE)GetWin_Num_int( this, IDC_EDIT_DATA_ITEM_DEF );
	sITEMMIX.cRESIST_FIRE = (BYTE)GetWin_Num_int( this, IDC_EDIT_DATA_ITEM_RES_FIRE );
	sITEMMIX.cRESIST_ICE = (BYTE)GetWin_Num_int( this, IDC_EDIT_DATA_ITEM_RES_ICE );
	sITEMMIX.cRESIST_ELEC = (BYTE)GetWin_Num_int( this, IDC_EDIT_DATA_ITEM_RES_ELECT );
	sITEMMIX.cRESIST_POISON = (BYTE)GetWin_Num_int( this, IDC_EDIT_DATA_ITEM_RES_POISON );
	sITEMMIX.cRESIST_SPIRIT = (BYTE)GetWin_Num_int( this, IDC_EDIT_DATA_ITEM_RES_SPIRIT );
	sITEMMIX.bGenerateRandomValue = GetWin_Check( this, IDC_CHECK_GENERATE_RANDOM_OPTION );

	//validity checking
	if ( sITEMMIX.dwKey == UINT_MAX || sITEMMIX.dwKey >= GLItemMixMan::ITEMMIX_MAXKEY )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid ID %d", sITEMMIX.dwKey );
		return;
	}

	SITEM* pItemResult = GLItemMan::GetInstance().GetItem( sITEMMIX.sResultItem.sNID );
	if ( pItemResult )
	{
		if ( sITEMMIX.sResultItem.nNum <= 0 )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Result Item Num! Must be more than 1");
			return;
		}
	}else{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Result Item %d~%d", sITEMMIX.sResultItem.sNID.wMainID, sITEMMIX.sResultItem.sNID.wSubID );
		return;
	}

	int nMaterials = 0;
	for( int i=0; i<ITEMMIX_ITEMNUM; ++i )
	{
		SITEM* pItemMaterial = GLItemMan::GetInstance().GetItem( sITEMMIX.sMeterialItem[i].sNID );
		if ( pItemMaterial )
		{
			nMaterials ++;

			if ( sITEMMIX.sMeterialItem[i].nNum <= 0 )
			{
				CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Material Item [%d] Num! Must be more than 1", i );
				return;
			}
		}
	}

	/*if ( nMaterials < 2 )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Total Materials! Must be more than 2" );
		return;
	}*/

	GLItemMixMan::GetInstance().SortMeterialItem( sITEMMIX );

	ITEMMIX_KEYMAP &mapItemMix = GLItemMixMan::GetInstance().m_mapKeyItemMix;

	if ( m_bAdd )
	{
		ITEMMIX_KEYMAP_ITER iterpos = mapItemMix.find(sITEMMIX.dwKey);
		if ( iterpos != mapItemMix.end() )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "ID Exist ID:%d", sITEMMIX.dwKey );
			return;
		}

		GLItemMixMan::GetInstance().insert( sITEMMIX );
		UpdateItemMixList();
		ItemMixShow( sITEMMIX, TRUE, FALSE );
	}
	else
	{
		ITEMMIX_KEYMAP_ITER iterpos = mapItemMix.find(sITEMMIX.dwKey);
		if ( iterpos != mapItemMix.end() )
		{
			ITEM_MIX* pITEMMIX = &(*iterpos).second;
			*pITEMMIX = sITEMMIX;

			UpdateItemMixList();
			ItemMixShow( *pITEMMIX, TRUE, FALSE );
		}else{
			CDebugSet::MsgBox( GetSafeHwnd(), "Failure to save ID Exist ID:%d", sITEMMIX.dwKey );

		}
	}
	
	
	for( int nItem = 0; nItem < m_ListItemMix.GetItemCount(); nItem++)
	{
		DWORD dwID = m_ListItemMix.GetItemData( nItem );
		if( dwID == sITEMMIX.dwKey )
		{
			m_ListItemMix.EnsureVisible( nItem, TRUE );
			m_ListItemMix.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
		}
	}
}

void CEditorItemMixDlg::ItemMixShow( ITEM_MIX sITEMMIX, BOOL bENABLE, BOOL bADD )
{
	m_bAdd = bADD;

	SetWin_Num_int( this, IDC_EDIT_ITEMMIX_ID, sITEMMIX.dwKey );
	SetWin_Num_int( this, IDC_EDIT_ITEMMIX_RATE, sITEMMIX.dwRate );
	SetWin_Num_int( this, IDC_EDIT_ITEMMIX_PRICE, sITEMMIX.dwPrice );

	SetWin_Num_int( this, IDC_EDIT_MID_RESULT, sITEMMIX.sResultItem.sNID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SID_RESULT, sITEMMIX.sResultItem.sNID.wSubID );
	SetWin_Num_int( this, IDC_EDIT_NUM_RESULT, sITEMMIX.sResultItem.nNum );

	SetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_0, sITEMMIX.sMeterialItem[0].sNID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_0, sITEMMIX.sMeterialItem[0].sNID.wSubID );
	SetWin_Num_int( this, IDC_EDIT_NUM_ITEMMIX_0, sITEMMIX.sMeterialItem[0].nNum );

	SetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_1, sITEMMIX.sMeterialItem[1].sNID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_1, sITEMMIX.sMeterialItem[1].sNID.wSubID );
	SetWin_Num_int( this, IDC_EDIT_NUM_ITEMMIX_1, sITEMMIX.sMeterialItem[1].nNum );

	SetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_2, sITEMMIX.sMeterialItem[2].sNID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_2, sITEMMIX.sMeterialItem[2].sNID.wSubID );
	SetWin_Num_int( this, IDC_EDIT_NUM_ITEMMIX_2, sITEMMIX.sMeterialItem[2].nNum );

	SetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_3, sITEMMIX.sMeterialItem[3].sNID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_3, sITEMMIX.sMeterialItem[3].sNID.wSubID );
	SetWin_Num_int( this, IDC_EDIT_NUM_ITEMMIX_3, sITEMMIX.sMeterialItem[3].nNum );

	SetWin_Num_int( this, IDC_EDIT_MID_ITEMMIX_4, sITEMMIX.sMeterialItem[4].sNID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SID_ITEMMIX_4, sITEMMIX.sMeterialItem[4].sNID.wSubID );
	SetWin_Num_int( this, IDC_EDIT_NUM_ITEMMIX_4, sITEMMIX.sMeterialItem[4].nNum );

	
	SetWin_Enable( this, IDC_EDIT_ITEMMIX_ID, bENABLE && m_bAdd );
	SetWin_Enable( this, IDC_EDIT_ITEMMIX_RATE, bENABLE );
	SetWin_Enable( this, IDC_EDIT_ITEMMIX_PRICE, bENABLE );

	SetWin_Enable( this, IDC_EDIT_MID_RESULT, bENABLE );
	SetWin_Enable( this, IDC_EDIT_SID_RESULT, bENABLE );
	SetWin_Enable( this, IDC_EDIT_NUM_RESULT, bENABLE );

	SetWin_Enable( this, IDC_EDIT_MID_ITEMMIX_0, bENABLE );
	SetWin_Enable( this, IDC_EDIT_SID_ITEMMIX_0, bENABLE );
	SetWin_Enable( this, IDC_EDIT_NUM_ITEMMIX_0, bENABLE );

	SetWin_Enable( this, IDC_EDIT_MID_ITEMMIX_1, bENABLE );
	SetWin_Enable( this, IDC_EDIT_SID_ITEMMIX_1, bENABLE );
	SetWin_Enable( this, IDC_EDIT_NUM_ITEMMIX_1, bENABLE );

	SetWin_Enable( this, IDC_EDIT_MID_ITEMMIX_2, bENABLE );
	SetWin_Enable( this, IDC_EDIT_SID_ITEMMIX_2, bENABLE );
	SetWin_Enable( this, IDC_EDIT_NUM_ITEMMIX_2, bENABLE );

	SetWin_Enable( this, IDC_EDIT_MID_ITEMMIX_3, bENABLE );
	SetWin_Enable( this, IDC_EDIT_SID_ITEMMIX_3, bENABLE );
	SetWin_Enable( this, IDC_EDIT_NUM_ITEMMIX_3, bENABLE );

	SetWin_Enable( this, IDC_EDIT_MID_ITEMMIX_4, bENABLE );
	SetWin_Enable( this, IDC_EDIT_SID_ITEMMIX_4, bENABLE );
	SetWin_Enable( this, IDC_EDIT_NUM_ITEMMIX_4, bENABLE );


	SetWin_Enable( this, IDC_BUTTON_ITEMMIX_RESULT, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_ITEMMIX_0, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_ITEMMIX_1, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_ITEMMIX_2, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_ITEMMIX_3, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_ITEMMIX_4, bENABLE );

	SetWin_Enable( this, IDC_BUTTON_ITEMMIX_DEL_RESULT, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_ITEMMIX_DEL_0, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_ITEMMIX_DEL_1, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_ITEMMIX_DEL_2, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_ITEMMIX_DEL_3, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_ITEMMIX_DEL_4, bENABLE );

	SetWin_Enable( this, IDC_BUTTON_ITEMMIX_SAVE, bENABLE );

	/*product item, Juver, 2017/10/15 */
	SetWin_Enable( this, IDC_COMBO_CATEGORY, bENABLE );
	SetWin_Enable( this, IDC_EDIT_DATA_TIME, bENABLE );
	SetWin_Enable( this, IDC_EDIT_DATA_LEVEL, bENABLE );

	SetWin_Combo_Sel ( this, IDC_COMBO_CATEGORY, (int)sITEMMIX.wCategory );
	SetWin_Num_float( this, IDC_EDIT_DATA_TIME, sITEMMIX.fTime );
	SetWin_Num_int( this, IDC_EDIT_DATA_LEVEL, sITEMMIX.wLevelReq );

	//official crafting data
	SetWin_Enable( this, IDC_EDIT_DATA_3, bENABLE );
	SetWin_Enable( this, IDC_EDIT_DATA_4, bENABLE );
	SetWin_Enable( this, IDC_EDIT_DATA_5, bENABLE );
	SetWin_Enable( this, IDC_EDIT_DATA_6, bENABLE );
	SetWin_Enable( this, IDC_EDIT_DATA_9, bENABLE );
	SetWin_Enable( this, IDC_EDIT_DATA_10, bENABLE );
	SetWin_Enable( this, IDC_EDIT_DATA_11, bENABLE );

	SetWin_Num_int( this, IDC_EDIT_DATA_3, sITEMMIX.dwData1 );
	SetWin_Num_int( this, IDC_EDIT_DATA_4, sITEMMIX.dwData2 );
	SetWin_Num_int( this, IDC_EDIT_DATA_5, sITEMMIX.nData1 );
	SetWin_Num_int( this, IDC_EDIT_DATA_6, sITEMMIX.nData2 );
	SetWin_Num_int( this, IDC_EDIT_DATA_9, sITEMMIX.wData1 );
	SetWin_Num_int( this, IDC_EDIT_DATA_10, sITEMMIX.wData2 );
	SetWin_Num_int( this, IDC_EDIT_DATA_11, sITEMMIX.wData3 );

	SetWin_Enable( this, IDC_EDIT_DATA_ITEM_ATT, bENABLE );
	SetWin_Enable( this, IDC_EDIT_DATA_ITEM_DEF, bENABLE );
	SetWin_Enable( this, IDC_EDIT_DATA_ITEM_RES_FIRE, bENABLE );
	SetWin_Enable( this, IDC_EDIT_DATA_ITEM_RES_ICE, bENABLE );
	SetWin_Enable( this, IDC_EDIT_DATA_ITEM_RES_ELECT, bENABLE );
	SetWin_Enable( this, IDC_EDIT_DATA_ITEM_RES_POISON, bENABLE );
	SetWin_Enable( this, IDC_EDIT_DATA_ITEM_RES_SPIRIT, bENABLE );
	SetWin_Enable( this, IDC_CHECK_GENERATE_RANDOM_OPTION, bENABLE );

	SetWin_Num_int( this, IDC_EDIT_DATA_ITEM_ATT, sITEMMIX.cDAMAGE );
	SetWin_Num_int( this, IDC_EDIT_DATA_ITEM_DEF, sITEMMIX.cDEFENSE );
	SetWin_Num_int( this, IDC_EDIT_DATA_ITEM_RES_FIRE, sITEMMIX.cRESIST_FIRE );
	SetWin_Num_int( this, IDC_EDIT_DATA_ITEM_RES_ICE, sITEMMIX.cRESIST_ICE );
	SetWin_Num_int( this, IDC_EDIT_DATA_ITEM_RES_ELECT, sITEMMIX.cRESIST_ELEC );
	SetWin_Num_int( this, IDC_EDIT_DATA_ITEM_RES_POISON, sITEMMIX.cRESIST_POISON );
	SetWin_Num_int( this, IDC_EDIT_DATA_ITEM_RES_SPIRIT, sITEMMIX.cRESIST_SPIRIT );
	SetWin_Check( this, IDC_CHECK_GENERATE_RANDOM_OPTION, sITEMMIX.bGenerateRandomValue );
}
