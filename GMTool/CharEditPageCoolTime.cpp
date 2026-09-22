// CharEditPageCoolTime.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "CharEditPageCoolTime.h"

#include "CharEditTab.h"

#include "EtcFunction.h"
#include "HLibTimeFunctions.h"
#include "HLibDataConvert.h"
#include "GLCharData.h"
#include "GLItemMan.h"
#include "Logic/GMToolGlobal.h"

#include "DlgItem.h"

// CCharEditPageCoolTime dialog

IMPLEMENT_DYNAMIC(CCharEditPageCoolTime, CPropertyPage)

CCharEditPageCoolTime::CCharEditPageCoolTime(LOGFONT logfont, SCHARDATA2* pData)
	: CPropertyPage(CCharEditPageCoolTime::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CCharEditPageCoolTime::~CCharEditPageCoolTime()
{
	SAFE_DELETE ( m_pFont );
}

void CCharEditPageCoolTime::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHAREDIT_COOLTIME_LIST_ID, m_ListID);
	DDX_Control(pDX, IDC_CHAREDIT_COOLTIME_LIST_TYPE, m_ListTYPE);
}


BEGIN_MESSAGE_MAP(CCharEditPageCoolTime, CPropertyPage)
	ON_BN_CLICKED(IDC_CHAREDIT_COOLTIME_BUTTON_ID_DELETE, OnBnClickedChareditCooltimeButtonIdDelete)
	ON_BN_CLICKED(IDC_CHAREDIT_COOLTIME_BUTTON_ID_CLEAR, OnBnClickedChareditCooltimeButtonIdClear)
	ON_BN_CLICKED(IDC_CHAREDIT_COOLTIME_BUTTON_TYPE_DELETE, OnBnClickedChareditCooltimeButtonTypeDelete)
	ON_BN_CLICKED(IDC_CHAREDIT_COOLTIME_BUTTON_TYPE_CLEAR, OnBnClickedChareditCooltimeButtonTypeClear)
	ON_BN_CLICKED(IDC_CHAREDIT_COOLTIME_BUTTON_ADD, OnBnClickedChareditCooltimeButtonAdd)
	ON_BN_CLICKED(IDC_CHAREDIT_COOLTIME_BUTTON_INSERT, OnBnClickedChareditCooltimeButtonInsert)
	ON_EN_CHANGE(IDC_CHAREDIT_COOLTIME_EDIT_MID, OnEnChangeChareditCooltimeEditMid)
	ON_EN_CHANGE(IDC_CHAREDIT_COOLTIME_EDIT_SID, OnEnChangeChareditCooltimeEditSid)
END_MESSAGE_MAP()


// CCharEditPageCoolTime message handlers
BOOL CCharEditPageCoolTime::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_ListID.SetExtendedStyle ( m_ListID.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_ListID.GetClientRect ( &rectCtrl );
	m_ListID.InsertColumn( 0, "ID", LVCFMT_LEFT, ( 30* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListID.InsertColumn( 1, "Item", LVCFMT_LEFT, ( 35* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListID.InsertColumn( 2, "CoolTime", LVCFMT_LEFT, ( 30* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	m_ListTYPE.SetExtendedStyle ( m_ListTYPE.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES  );
	m_ListTYPE.GetClientRect ( &rectCtrl );
	m_ListTYPE.InsertColumn( 0, "ID", LVCFMT_LEFT, ( 30* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListTYPE.InsertColumn( 1, "Item", LVCFMT_LEFT, ( 35* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListTYPE.InsertColumn( 2, "CoolTime", LVCFMT_LEFT, ( 30* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	UpdateCoolTimeID();
	UpdateCoolTimeTYPE();

	return TRUE;  
}

void CCharEditPageCoolTime::UpdateCoolTimeID()
{
	m_ListID.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_ListID.SetRedraw( FALSE );

	SCHARDATA2::COOLTIME_MAP_ITER iter_start = m_pData->m_mapCoolTimeID.begin();

	for( int i=0; iter_start != m_pData->m_mapCoolTimeID.end(); ++iter_start, ++i )
	{
		const ITEM_COOLTIME sCoolTime = (*iter_start).second;
		SNATIVEID sID( sCoolTime.dwID );

		std::string strNAME = "Unknown Item";
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sID );
		if ( pItem ){
			strNAME = pItem->GetName();
		}

		m_ListID.InsertItem( i, _HLIB::cstring_dword( sCoolTime.dwCoolID ).GetString() );
		m_ListID.SetItemText( i, 1, _HLIB::cstringformat( "[%03d/%03d] %s", sID.wMainID, sID.wSubID, strNAME.c_str() ).GetString() );
		m_ListID.SetItemText( i, 2, _HLIB::cstring_timet12(sCoolTime.tCoolTime).GetString() );
		m_ListID.SetItemData ( i, sCoolTime.dwCoolID );
	}

	m_ListID.SetRedraw( TRUE );

	SetWin_Num_int( this, IDC_CHAREDIT_COOLTIME_EDIT_MID, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_CHAREDIT_COOLTIME_EDIT_SID, NATIVEID_NULL().wSubID );
}

void CCharEditPageCoolTime::UpdateCoolTimeTYPE()
{
	m_ListTYPE.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_ListTYPE.SetRedraw( FALSE );

	SCHARDATA2::COOLTIME_MAP_ITER iter_start = m_pData->m_mapCoolTimeType.begin();

	for( int i=0; iter_start != m_pData->m_mapCoolTimeType.end(); ++iter_start, ++i )
	{
		const ITEM_COOLTIME sCoolTime = (*iter_start).second;
		SNATIVEID sID( sCoolTime.dwID );

		std::string strNAME = "Unknown Item";
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sID );
		if ( pItem ){
			strNAME = pItem->GetName();
		}

		m_ListTYPE.InsertItem( i, _HLIB::cstringformat( "(%d) %s", sCoolTime.dwCoolID, COMMENT::ITEMTYPE[sCoolTime.dwCoolID].c_str() ).GetString() );
		m_ListTYPE.SetItemText( i, 1, _HLIB::cstringformat( "[%03d/%03d] %s", sID.wMainID, sID.wSubID, strNAME.c_str() ).GetString() );
		m_ListTYPE.SetItemText( i, 2,_HLIB::cstring_timet12(sCoolTime.tCoolTime).GetString() );
		m_ListTYPE.SetItemData ( i, sCoolTime.dwCoolID );
	}

	m_ListTYPE.SetRedraw( TRUE );

	SetWin_Num_int( this, IDC_CHAREDIT_COOLTIME_EDIT_MID, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_CHAREDIT_COOLTIME_EDIT_SID, NATIVEID_NULL().wSubID );
}

void CCharEditPageCoolTime::OnBnClickedChareditCooltimeButtonIdDelete()
{
	if ( !m_pData ){
		return;
	}

	int nSelect = m_ListID.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = (DWORD) m_ListID.GetItemData ( nSelect );

	SCHARDATA2::COOLTIME_MAP_ITER pos = m_pData->m_mapCoolTimeID.find(dwID);
	if ( pos!=m_pData->m_mapCoolTimeID.end() ){
		m_pData->m_mapCoolTimeID.erase( pos );

		UpdateCoolTimeID();

		int nNewSelect = nSelect > 1? nSelect-1:nSelect;
		m_ListID.EnsureVisible( nNewSelect, TRUE );
		m_ListID.SetItemState( nNewSelect, LVIS_SELECTED, LVIS_SELECTED );
		GotoDlgCtrl(GetDlgItem(m_ListID.GetDlgCtrlID())); 
	}
}

void CCharEditPageCoolTime::OnBnClickedChareditCooltimeButtonIdClear()
{
	if ( !m_pData ){
		return;
	}

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All CoolTime ID?" ) == IDYES ){
		m_pData->m_mapCoolTimeID.clear();
		UpdateCoolTimeID();
	}
}

void CCharEditPageCoolTime::OnBnClickedChareditCooltimeButtonTypeDelete()
{
	if ( !m_pData ){
		return;
	}

	int nSelect = m_ListTYPE.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = (DWORD) m_ListTYPE.GetItemData ( nSelect );

	SCHARDATA2::COOLTIME_MAP_ITER pos = m_pData->m_mapCoolTimeType.find(dwID);
	if ( pos!=m_pData->m_mapCoolTimeType.end() ){
		m_pData->m_mapCoolTimeType.erase( pos );

		UpdateCoolTimeTYPE();

		int nNewSelect = nSelect > 1? nSelect-1:nSelect;
		m_ListTYPE.EnsureVisible( nNewSelect, TRUE );
		m_ListTYPE.SetItemState( nNewSelect, LVIS_SELECTED, LVIS_SELECTED );
		GotoDlgCtrl(GetDlgItem(m_ListTYPE.GetDlgCtrlID())); 
	}
}

void CCharEditPageCoolTime::OnBnClickedChareditCooltimeButtonTypeClear()
{
	if ( !m_pData ){
		return;
	}

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All CoolTime Type?" ) == IDYES ){
		m_pData->m_mapCoolTimeType.clear();
		UpdateCoolTimeTYPE();
	}
}

void CCharEditPageCoolTime::OnBnClickedChareditCooltimeButtonAdd()
{
	CDlgItem dlg( this );
	if ( dlg.DoModal() == IDOK ){
		SetWin_Num_int( this, IDC_CHAREDIT_COOLTIME_EDIT_MID, dlg.m_SELECTED.wMainID );
		SetWin_Num_int( this, IDC_CHAREDIT_COOLTIME_EDIT_SID, dlg.m_SELECTED.wSubID );
	}
}

void CCharEditPageCoolTime::OnBnClickedChareditCooltimeButtonInsert()
{
	if ( !m_pData ){
		return;
	}

	WORD wMID = GetWin_Num_int( this, IDC_CHAREDIT_COOLTIME_EDIT_MID );
	WORD wSID = GetWin_Num_int( this, IDC_CHAREDIT_COOLTIME_EDIT_SID );

	SITEM* pITEM = GLItemMan::GetInstance().GetItem( wMID, wSID );
	if ( !pITEM ){
		CDebugSet::MsgBox( GetSafeHwnd(), "[%03d/%03d] Invalid Item", wMID, wSID );
		return;
	}

	if ( !pITEM->sBasicOp.IsCoolTime() ){
		CDebugSet::MsgBox( GetSafeHwnd(), "[%03d/%03d] '%s' Item No CoolTime", wMID, wSID, pITEM->GetName() );
		return;
	}

	EMCOOL_TYPE emCoolType = pITEM->sBasicOp.emCoolType;
	__time64_t  tCurTime = CTime::GetCurrentTime().GetTime();
	__time64_t  tCoolTime = tCurTime + pITEM->sBasicOp.dwCoolTime;

	ITEM_COOLTIME sCoolTime;
	sCoolTime.dwID = pITEM->sBasicOp.sNativeID.dwID;
	sCoolTime.tUseTime = tCurTime;
	sCoolTime.tCoolTime = tCoolTime;	

	if ( emCoolType == EMCOOL_ITEMID )
	{
		sCoolTime.dwCoolID = pITEM->sBasicOp.sNativeID.dwID;

		SCHARDATA2::COOLTIME_MAP_ITER iterpos = m_pData->m_mapCoolTimeID.find(sCoolTime.dwCoolID);
		if ( iterpos != m_pData->m_mapCoolTimeID.end() ){
			m_pData->m_mapCoolTimeID.erase( iterpos );
		}

		m_pData->m_mapCoolTimeID[sCoolTime.dwCoolID] = sCoolTime;

		UpdateCoolTimeID();

		for( int nItem=0; nItem<m_ListID.GetItemCount(); nItem++){
			DWORD dwID = m_ListID.GetItemData( nItem );
			if( sCoolTime.dwCoolID  == dwID ){
				m_ListID.EnsureVisible( nItem, TRUE );
				m_ListID.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
				GotoDlgCtrl(GetDlgItem(m_ListID.GetDlgCtrlID())); 
			}
		}
	}
	else if( emCoolType == EMCOOL_ITEMTYPE )
	{
		sCoolTime.dwCoolID = (DWORD)pITEM->sBasicOp.emItemType;

		SCHARDATA2::COOLTIME_MAP_ITER iterpos = m_pData->m_mapCoolTimeType.find(sCoolTime.dwCoolID);
		if ( iterpos != m_pData->m_mapCoolTimeType.end() ){
			m_pData->m_mapCoolTimeType.erase( iterpos );
		}

		m_pData->m_mapCoolTimeType[sCoolTime.dwCoolID] = sCoolTime;

		UpdateCoolTimeTYPE();

		for( int nItem=0; nItem<m_ListTYPE.GetItemCount(); nItem++){
			DWORD dwID = m_ListTYPE.GetItemData( nItem );
			if( sCoolTime.dwCoolID  == dwID ){
				m_ListTYPE.EnsureVisible( nItem, TRUE );
				m_ListTYPE.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
				GotoDlgCtrl(GetDlgItem(m_ListTYPE.GetDlgCtrlID())); 
			}
		}
	}
}

void CCharEditPageCoolTime::OnEnChangeChareditCooltimeEditMid()
{
	GMTOOL_GLOBAL::DataCheckItem( this, 
		IDC_CHAREDIT_COOLTIME_EDIT_MID, 
		IDC_CHAREDIT_COOLTIME_EDIT_SID, 
		IDC_CHAREDIT_COOLTIME_EDIT_NAME );
}

void CCharEditPageCoolTime::OnEnChangeChareditCooltimeEditSid()
{
	GMTOOL_GLOBAL::DataCheckItem( this, 
		IDC_CHAREDIT_COOLTIME_EDIT_MID, 
		IDC_CHAREDIT_COOLTIME_EDIT_SID, 
		IDC_CHAREDIT_COOLTIME_EDIT_NAME );
}
