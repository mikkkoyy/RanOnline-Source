/*Item Codex, Jhoniex 2/14/2024 */

#include "stdafx.h"
#include "GMTool.h"
#include "CharEditPageCodex.h"
#include "CharEditTab.h"
#include "EtcFunction.h"
#include "GLCharData.h"
#include "GLCodex.h"
#include "HLibDataConvert.h"
#include "DlgCodexList.h"
#include "DlgCodexEdit.h"
#include ".\chareditpagecodex.h"

// CCharEditPageCodex dialog

IMPLEMENT_DYNAMIC(CCharEditPageCodex, CPropertyPage)

CCharEditPageCodex::CCharEditPageCodex(LOGFONT logfont, SCHARDATA2* pData)
	: CPropertyPage(CCharEditPageCodex::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CCharEditPageCodex::~CCharEditPageCodex()
{
	SAFE_DELETE ( m_pFont );
}

void CCharEditPageCodex::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHAREDIT_CODEX_PROG_LIST, m_ListProg);
	DDX_Control(pDX, IDC_CHAREDIT_CODEX_DONE_LIST, m_ListDone);
}


BEGIN_MESSAGE_MAP(CCharEditPageCodex, CPropertyPage)

	ON_BN_CLICKED(IDC_CHAREDIT_CODEX_BUTTON_PROG_EDIT, OnBnClickedChareditCodexButtonProgEdit)
	ON_BN_CLICKED(IDC_CHAREDIT_CODEX_BUTTON_PROG_COMPLETE, OnBnClickedChareditCodexButtonProgComplete)
	ON_BN_CLICKED(IDC_CHAREDIT_CODEX_BUTTON_PROG_DELETE, OnBnClickedChareditCodexButtonProgDelete)
	ON_BN_CLICKED(IDC_CHAREDIT_CODEX_BUTTON_PROG_CLEAR, OnBnClickedChareditCodexButtonProgClear)
	ON_BN_CLICKED(IDC_CHAREDIT_CODEX_BUTTON_DONE_DELETE, OnBnClickedChareditCodexButtonDoneDelete)
	ON_BN_CLICKED(IDC_CHAREDIT_CODEX_BUTTON_DONE_CLEAR, OnBnClickedChareditCodexButtonDoneClear)
	ON_BN_CLICKED(IDC_CHAREDIT_CODEX_BUTTON_PROG_COMPLETEALL, OnBnClickedChareditCodexButtonProgCompleteall)
END_MESSAGE_MAP()


// CCharEditPageCodex message handlers
BOOL CCharEditPageCodex::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_ListProg.SetExtendedStyle ( m_ListProg.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_ListProg.GetClientRect ( &rectCtrl );
	m_ListProg.InsertColumn( 0, "ID", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListProg.InsertColumn( 1, "Title", LVCFMT_LEFT, ( 30* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListProg.InsertColumn( 2, "Badge", LVCFMT_LEFT, ( 25* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListProg.InsertColumn( 3, "Type", LVCFMT_LEFT, ( 30* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	m_ListDone.SetExtendedStyle ( m_ListDone.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_ListDone.GetClientRect ( &rectCtrl );
	m_ListDone.InsertColumn( 0, "ID", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListDone.InsertColumn( 1, "Title", LVCFMT_LEFT, ( 40* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListDone.InsertColumn( 2, "Badge", LVCFMT_LEFT, ( 25* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListDone.InsertColumn( 3, "Type", LVCFMT_LEFT, ( 30* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	UpdateProgList();
	UpdateDoneList();

	return TRUE;  
}

void CCharEditPageCodex::UpdateProgList()
{
	m_ListProg.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_ListProg.SetRedraw( FALSE );

	SCODEX_CHAR_DATA_MAP& mapCodex = m_pData->m_mapCodexProg;
	SCODEX_CHAR_DATA_MAP_ITER iter = mapCodex.begin();

	for ( int i=0 ; iter != mapCodex.end(); ++iter, ++i )
	{
		const SCODEX_CHAR_DATA& sCodex = (*iter).second;
		std::string strTitle = "Unknown Codex";
		std::string strBadge = "";

		SCODEX_FILE_DATA* pCodex_data = GLCodex::GetInstance().GetCodex( sCodex.dwCodexID );
		if ( pCodex_data )
		{
			strTitle = pCodex_data->strCodexTitle.c_str();
			strBadge = pCodex_data->strBadgeString.c_str();
		}
		
		m_ListProg.InsertItem( i, _HLIB::cstring_dword( sCodex.dwCodexID ).GetString() );
		m_ListProg.SetItemText( i, 1, strTitle.c_str() );
		m_ListProg.SetItemText( i, 2, strBadge.c_str() );
		m_ListProg.SetItemText( i, 3, _HLIB::cstringformat( "%s(%d/%d)", COMMENT::CODEX_MID[sCodex.emType].c_str(), 
			sCodex.dwProgressNow, sCodex.dwProgressMax ).GetString());
		m_ListProg.SetItemData( i, sCodex.dwCodexID );
	}

	m_ListProg.SetRedraw( TRUE );
}

void CCharEditPageCodex::UpdateDoneList()
{
	m_ListDone.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_ListDone.SetRedraw( FALSE );

	SCODEX_CHAR_DATA_MAP& mapCodex = m_pData->m_mapCodexDone;
	SCODEX_CHAR_DATA_MAP_ITER iter = mapCodex.begin();

	for ( int i=0 ; iter != mapCodex.end(); ++iter, ++i )
	{
		const SCODEX_CHAR_DATA& sCodex = (*iter).second;
		std::string strTitle = "Unknown Codex";
		std::string strBadge = "";

		SCODEX_FILE_DATA* pCodex_data = GLCodex::GetInstance().GetCodex( sCodex.dwCodexID );
		if ( pCodex_data )
		{
			strTitle = pCodex_data->strCodexTitle.c_str();
			strBadge = pCodex_data->strBadgeString.c_str();
		}

		m_ListDone.InsertItem( i, _HLIB::cstring_dword( sCodex.dwCodexID ).GetString() );
		m_ListDone.SetItemText( i, 1, strTitle.c_str() );
		m_ListDone.SetItemText( i, 2, strBadge.c_str() );
		m_ListDone.SetItemText( i, 3, _HLIB::cstringformat( "%s(%d/%d)", COMMENT::CODEX_MID[sCodex.emType].c_str(), 
			sCodex.dwProgressNow,sCodex.dwProgressMax ).GetString());
		m_ListDone.SetItemData( i, sCodex.dwCodexID );
	}

	m_ListDone.SetRedraw( TRUE );
}

void CCharEditPageCodex::OnBnClickedChareditCodexButtonProgEdit()
{
	if ( !m_pData )	return;

	int nSelect = m_ListProg.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected!" );
		return;
	}

	DWORD dwID = (DWORD) m_ListProg.GetItemData ( nSelect );

	SCODEX_CHAR_DATA_MAP& mapCodex = m_pData->m_mapCodexProg;
	SCODEX_CHAR_DATA_MAP_ITER iter = mapCodex.find(dwID);
	if ( iter != mapCodex.end() )
	{
		SCODEX_CHAR_DATA &sData = (*iter).second;
		CDlgCodexEdit dlg( this, &sData );
		if ( dlg.DoModal() == IDOK )
		{
			UpdateProgList();

			for( int nItem=0; nItem<m_ListProg.GetItemCount(); nItem++){
				DWORD dwID = m_ListProg.GetItemData( nItem );
				if( dwID == sData.dwCodexID ){
					m_ListProg.EnsureVisible( nItem, TRUE );
					m_ListProg.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
					GotoDlgCtrl(GetDlgItem(m_ListProg.GetDlgCtrlID())); 
				}
			}
		}
	}
}

void CCharEditPageCodex::OnBnClickedChareditCodexButtonProgComplete()
{
	if ( !m_pData )	return;

	int nSelect = m_ListProg.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected!" );
		return;
	}

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Change codex status from prog to done?" ) == IDYES )
	{
		DWORD dwID = (DWORD) m_ListProg.GetItemData ( nSelect );

		SCODEX_CHAR_DATA_MAP& mapCodex = m_pData->m_mapCodexProg;
		SCODEX_CHAR_DATA_MAP_ITER iter = mapCodex.find(dwID);
		if ( iter != mapCodex.end() )
		{
			SCODEX_CHAR_DATA scodex_data = (*iter).second;
			scodex_data.dwProgressNow = scodex_data.dwProgressMax;
			m_pData->m_mapCodexDone.insert( std::make_pair(scodex_data.dwCodexID, scodex_data ) );
			mapCodex.erase( iter );

			UpdateProgList();
			UpdateDoneList();
		}
	}
}

void CCharEditPageCodex::OnBnClickedChareditCodexButtonProgDelete()
{
	if ( !m_pData )	return;

	int nSelect = m_ListProg.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected!" );
		return;
	}

	DWORD dwID = (DWORD) m_ListProg.GetItemData ( nSelect );

	SCODEX_CHAR_DATA_MAP& mapCodex = m_pData->m_mapCodexProg;
	SCODEX_CHAR_DATA_MAP_ITER iter = mapCodex.find(dwID);
	if ( iter != mapCodex.end() ){
		mapCodex.erase( iter );
	}

	UpdateProgList();
}

void CCharEditPageCodex::OnBnClickedChareditCodexButtonProgClear()
{
	if ( !m_pData )return;
	
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All Prog Codex?" ) == IDYES ){
		m_pData->m_mapCodexProg.clear();
		UpdateProgList();
	}
}

void CCharEditPageCodex::OnBnClickedChareditCodexButtonDoneDelete()
{
	if ( !m_pData )	return;

	int nSelect = m_ListDone.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected!" );
		return;
	}

	DWORD dwID = (DWORD) m_ListDone.GetItemData ( nSelect );
	
	SCODEX_CHAR_DATA_MAP& mapCodex = m_pData->m_mapCodexDone;
	SCODEX_CHAR_DATA_MAP_ITER iter = mapCodex.find(dwID);
	if ( iter != mapCodex.end() ){
		mapCodex.erase( iter );
	}

	UpdateDoneList();
}

void CCharEditPageCodex::OnBnClickedChareditCodexButtonDoneClear()
{
	if ( !m_pData )return;

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All Done Codex?" ) == IDYES ){
		m_pData->m_mapCodexDone.clear();
		UpdateDoneList();
	}
}


void CCharEditPageCodex::OnBnClickedChareditCodexButtonProgCompleteall()
{
		if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Change codex status from prog to done?" ) == IDYES )
	{
		SCODEX_CHAR_DATA_MAP& mapCodex = m_pData->m_mapCodexProg;
		for( SCODEX_CHAR_DATA_MAP_ITER iter = mapCodex.begin(); 
		iter != mapCodex.end(); ++ iter )
		{
			SCODEX_CHAR_DATA scodex_data = (*iter).second;
			scodex_data.dwProgressNow = scodex_data.dwProgressMax;
			m_pData->m_mapCodexDone.insert( std::make_pair(scodex_data.dwCodexID, scodex_data ) );
			mapCodex.erase( iter );

			UpdateProgList();
			UpdateDoneList();
		}
	}
}
