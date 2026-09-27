// CharEditPageActivity.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "CharEditPageActivity.h"
#include "CharEditTab.h"
#include "EtcFunction.h"
#include "GLCharData.h"
#include "GLActivity.h"
#include "HLibDataConvert.h"
#include "DlgActivityList.h"
#include "DlgActivityEdit.h"

// CCharEditPageActivity dialog

IMPLEMENT_DYNAMIC(CCharEditPageActivity, CPropertyPage)

CCharEditPageActivity::CCharEditPageActivity(LOGFONT logfont, SCHARDATA2* pData)
	: CPropertyPage(CCharEditPageActivity::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CCharEditPageActivity::~CCharEditPageActivity()
{
	SAFE_DELETE ( m_pFont );
}

void CCharEditPageActivity::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHAREDIT_ACTIVITY_PROG_LIST, m_ListProg);
	DDX_Control(pDX, IDC_CHAREDIT_ACTIVITY_DONE_LIST, m_ListDone);
}


BEGIN_MESSAGE_MAP(CCharEditPageActivity, CPropertyPage)

	ON_BN_CLICKED(IDC_CHAREDIT_ACTIVITY_BUTTON_PROG_EDIT, OnBnClickedChareditActivityButtonProgEdit)
	ON_BN_CLICKED(IDC_CHAREDIT_ACTIVITY_BUTTON_PROG_COMPLETE, OnBnClickedChareditActivityButtonProgComplete)
	ON_BN_CLICKED(IDC_CHAREDIT_ACTIVITY_BUTTON_PROG_DELETE, OnBnClickedChareditActivityButtonProgDelete)
	ON_BN_CLICKED(IDC_CHAREDIT_ACTIVITY_BUTTON_PROG_CLEAR, OnBnClickedChareditActivityButtonProgClear)
	ON_BN_CLICKED(IDC_CHAREDIT_ACTIVITY_BUTTON_DONE_DELETE, OnBnClickedChareditActivityButtonDoneDelete)
	ON_BN_CLICKED(IDC_CHAREDIT_ACTIVITY_BUTTON_DONE_CLEAR, OnBnClickedChareditActivityButtonDoneClear)
END_MESSAGE_MAP()


// CCharEditPageActivity message handlers
BOOL CCharEditPageActivity::OnInitDialog()
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

void CCharEditPageActivity::UpdateProgList()
{
	m_ListProg.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_ListProg.SetRedraw( FALSE );

	SACTIVITY_CHAR_DATA_MAP& mapActivity = m_pData->m_mapActivityProg;
	SACTIVITY_CHAR_DATA_MAP_ITER iter = mapActivity.begin();

	for ( int i=0 ; iter != mapActivity.end(); ++iter, ++i )
	{
		const SACTIVITY_CHAR_DATA& sActivity = (*iter).second;
		std::string strTitle = "Unknown Activity";
		std::string strBadge = "";

		SACTIVITY_FILE_DATA* pActivity_data = GLActivity::GetInstance().GetActivity( sActivity.dwActivityID );
		if ( pActivity_data )
		{
			strTitle = pActivity_data->strActivityTitle.c_str();
			strBadge = pActivity_data->strBadgeString.c_str();
		}
		
		m_ListProg.InsertItem( i, _HLIB::cstring_dword( sActivity.dwActivityID ).GetString() );
		m_ListProg.SetItemText( i, 1, strTitle.c_str() );
		m_ListProg.SetItemText( i, 2, strBadge.c_str() );
		m_ListProg.SetItemText( i, 3, _HLIB::cstringformat( "%s(%d/%d)", COMMENT::ACTIVITY_MID[sActivity.emType].c_str(), 
			sActivity.dwProgressNow, sActivity.dwProgressMax ).GetString());
		m_ListProg.SetItemData( i, sActivity.dwActivityID );
	}

	m_ListProg.SetRedraw( TRUE );
}

void CCharEditPageActivity::UpdateDoneList()
{
	m_ListDone.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_ListDone.SetRedraw( FALSE );

	SACTIVITY_CHAR_DATA_MAP& mapActivity = m_pData->m_mapActivityDone;
	SACTIVITY_CHAR_DATA_MAP_ITER iter = mapActivity.begin();

	for ( int i=0 ; iter != mapActivity.end(); ++iter, ++i )
	{
		const SACTIVITY_CHAR_DATA& sActivity = (*iter).second;
		std::string strTitle = "Unknown Activity";
		std::string strBadge = "";

		SACTIVITY_FILE_DATA* pActivity_data = GLActivity::GetInstance().GetActivity( sActivity.dwActivityID );
		if ( pActivity_data )
		{
			strTitle = pActivity_data->strActivityTitle.c_str();
			strBadge = pActivity_data->strBadgeString.c_str();
		}

		m_ListDone.InsertItem( i, _HLIB::cstring_dword( sActivity.dwActivityID ).GetString() );
		m_ListDone.SetItemText( i, 1, strTitle.c_str() );
		m_ListDone.SetItemText( i, 2, strBadge.c_str() );
		m_ListDone.SetItemText( i, 3, _HLIB::cstringformat( "%s(%d/%d)", COMMENT::ACTIVITY_MID[sActivity.emType].c_str(), 
			sActivity.dwProgressNow,sActivity.dwProgressMax ).GetString());
		m_ListDone.SetItemData( i, sActivity.dwActivityID );
	}

	m_ListDone.SetRedraw( TRUE );
}

void CCharEditPageActivity::OnBnClickedChareditActivityButtonProgEdit()
{
	if ( !m_pData )	return;

	int nSelect = m_ListProg.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected!" );
		return;
	}

	DWORD dwID = (DWORD) m_ListProg.GetItemData ( nSelect );

	SACTIVITY_CHAR_DATA_MAP& mapActivity = m_pData->m_mapActivityProg;
	SACTIVITY_CHAR_DATA_MAP_ITER iter = mapActivity.find(dwID);
	if ( iter != mapActivity.end() )
	{
		SACTIVITY_CHAR_DATA &sData = (*iter).second;
		CDlgActivityEdit dlg( this, &sData );
		if ( dlg.DoModal() == IDOK )
		{
			UpdateProgList();

			for( int nItem=0; nItem<m_ListProg.GetItemCount(); nItem++){
				DWORD dwID = m_ListProg.GetItemData( nItem );
				if( dwID == sData.dwActivityID ){
					m_ListProg.EnsureVisible( nItem, TRUE );
					m_ListProg.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
					GotoDlgCtrl(GetDlgItem(m_ListProg.GetDlgCtrlID())); 
				}
			}
		}
	}
}

void CCharEditPageActivity::OnBnClickedChareditActivityButtonProgComplete()
{
	if ( !m_pData )	return;

	int nSelect = m_ListProg.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected!" );
		return;
	}

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Change activity status from prog to done?" ) == IDYES )
	{
		DWORD dwID = (DWORD) m_ListProg.GetItemData ( nSelect );

		SACTIVITY_CHAR_DATA_MAP& mapActivity = m_pData->m_mapActivityProg;
		SACTIVITY_CHAR_DATA_MAP_ITER iter = mapActivity.find(dwID);
		if ( iter != mapActivity.end() )
		{
			SACTIVITY_CHAR_DATA sactivity_data = (*iter).second;
			sactivity_data.dwProgressNow = sactivity_data.dwProgressMax;
			m_pData->m_mapActivityDone.insert( std::make_pair(sactivity_data.dwActivityID, sactivity_data ) );
			mapActivity.erase( iter );

			UpdateProgList();
			UpdateDoneList();
		}
	}
}

void CCharEditPageActivity::OnBnClickedChareditActivityButtonProgDelete()
{
	if ( !m_pData )	return;

	int nSelect = m_ListProg.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected!" );
		return;
	}

	DWORD dwID = (DWORD) m_ListProg.GetItemData ( nSelect );

	SACTIVITY_CHAR_DATA_MAP& mapActivity = m_pData->m_mapActivityProg;
	SACTIVITY_CHAR_DATA_MAP_ITER iter = mapActivity.find(dwID);
	if ( iter != mapActivity.end() ){
		mapActivity.erase( iter );
	}

	UpdateProgList();
}

void CCharEditPageActivity::OnBnClickedChareditActivityButtonProgClear()
{
	if ( !m_pData )return;
	
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All Prog Activity?" ) == IDYES ){
		m_pData->m_mapActivityProg.clear();
		UpdateProgList();
	}
}

void CCharEditPageActivity::OnBnClickedChareditActivityButtonDoneDelete()
{
	if ( !m_pData )	return;

	int nSelect = m_ListDone.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected!" );
		return;
	}

	DWORD dwID = (DWORD) m_ListDone.GetItemData ( nSelect );
	
	SACTIVITY_CHAR_DATA_MAP& mapActivity = m_pData->m_mapActivityDone;
	SACTIVITY_CHAR_DATA_MAP_ITER iter = mapActivity.find(dwID);
	if ( iter != mapActivity.end() ){
		mapActivity.erase( iter );
	}

	UpdateDoneList();
}

void CCharEditPageActivity::OnBnClickedChareditActivityButtonDoneClear()
{
	if ( !m_pData )return;

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All Done Activity?" ) == IDYES ){
		m_pData->m_mapActivityDone.clear();
		UpdateDoneList();
	}
}
