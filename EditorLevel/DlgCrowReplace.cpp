// DlgCrowReplace.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "DlgCrowReplace.h"
#include "DlgFindCrow.h"
#include "EtcFunction.h"
#include "HLibDataConvert.h"

// CDlgCrowReplace dialog

IMPLEMENT_DYNAMIC(CDlgCrowReplace, CDialog)

CDlgCrowReplace::CDlgCrowReplace(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCrowReplace::IDD, pParent)
{
	m_vecReplace.clear();
}

CDlgCrowReplace::~CDlgCrowReplace()
{
	m_vecReplace.clear();
}

void CDlgCrowReplace::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CDlgCrowReplace, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_CROW_FROM, OnBnClickedButtonCrowFrom)
	ON_BN_CLICKED(IDC_BUTTON_CROW_TO, OnBnClickedButtonCrowTo)
	ON_EN_CHANGE(IDC_EDIT_CROW_MID_FROM, OnEnChangeEditCrowMidFrom)
	ON_EN_CHANGE(IDC_EDIT_CROW_SID_FROM, OnEnChangeEditCrowSidFrom)
	ON_EN_CHANGE(IDC_EDIT_CROW_MID_TO, OnEnChangeEditCrowMidTo)
	ON_EN_CHANGE(IDC_EDIT_CROW_SID_TO, OnEnChangeEditCrowSidTo)
	ON_BN_CLICKED(IDC_BUTTON_CROW_ADD, OnBnClickedButtonCrowAdd)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnBnClickedButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_DELETE2, OnBnClickedButtonDelete2)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgCrowReplace message handlers
BOOL CDlgCrowReplace::OnInitDialog()
{
	CDialog::OnInitDialog();

	RECT rectCtrl;
	LONG lnWidth;
	LV_COLUMN lvColumn;

	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	lnWidth = rectCtrl.right - rectCtrl.left;

	const int nColumnCount = 3;	
	char* szColumnName[nColumnCount] = { "Num", "Crow From", "Crow To" };
	int nColumnWidthPercent[nColumnCount] = { 10, 45, 45 };

	for ( int i = 0; i < nColumnCount; i++ )
	{		
		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.pszText = szColumnName[i];
		lvColumn.iSubItem = i;
		lvColumn.cx = ( lnWidth*nColumnWidthPercent[i] ) / 100;
		m_List.InsertColumn (i, &lvColumn );
	}

	ResetInPut();
	ShowData();

	return TRUE;  
}

void CDlgCrowReplace::ShowData()
{
	m_List.DeleteAllItems();
	m_List.SetRedraw( FALSE );

	for( int i = 0; i < (int)m_vecReplace.size(); ++ i )
	{
		SCROWREPLACE sDATA = m_vecReplace[i];
		m_List.InsertItem( i, _HLIB::cstring_dword(i).GetString() );

		{
			SNATIVEID sID = sDATA.sIDFROM;

			std::string strNAME = "Unknown Crow";
			PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( sID );
			if ( pCROW )
				strNAME = pCROW->GetName();

			CString strTEMP;
			strTEMP.Format( "( %d~%d ) %s", sID.wMainID, sID.wSubID, strNAME.c_str() );

			m_List.SetItemText( i, 1, strTEMP.GetString() );
		}

		{
			SNATIVEID sID = sDATA.sIDTO;

			std::string strNAME = "Unknown Crow";
			PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( sID );
			if ( pCROW )
				strNAME = pCROW->GetName();

			CString strTEMP;
			strTEMP.Format( "( %d~%d ) %s", sID.wMainID, sID.wSubID, strNAME.c_str() );

			m_List.SetItemText( i, 2, strTEMP.GetString() );
		}
		
		m_List.SetItemData ( i, i );
	}

	m_List.SetRedraw( TRUE );
}

void CDlgCrowReplace::ResetInPut()
{
	SetWin_Num_int( this, IDC_EDIT_CROW_MID_FROM, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_EDIT_CROW_SID_FROM, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_EDIT_CROW_MID_TO, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_EDIT_CROW_SID_TO, NATIVEID_NULL().wMainID );
	SetWin_Text( this, IDC_EDIT_CROW_NAME_FROM, "" );
	SetWin_Text( this, IDC_EDIT_CROW_NAME_TO, "" );
}

void CDlgCrowReplace::ShowCrowName( int nINPUT_MID, int nINPUT_SID, int nOUTPUT )
{
	WORD wMID = GetWin_Num_int( this, nINPUT_MID );
	WORD wSID = GetWin_Num_int( this, nINPUT_SID );

	std::string strNAME = "Unknown Crow";
	PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( wMID, wSID );
	if ( pCROW )
		strNAME = pCROW->GetName();

	SetWin_Text( this, nOUTPUT, strNAME.c_str() );
}

void CDlgCrowReplace::OnBnClickedButtonCrowFrom()
{
	CDlgFindCrow dlg( this );
	if ( dlg.DoModal() == IDOK )
	{
		SNATIVEID sID = dlg.m_sSELECTED;
		PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( sID );
		if( pCROW )
		{
			SetWin_Num_int( this, IDC_EDIT_CROW_MID_FROM, pCROW->sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_CROW_SID_FROM, pCROW->sNativeID.wSubID );
		}
	}
}

void CDlgCrowReplace::OnBnClickedButtonCrowTo()
{
	CDlgFindCrow dlg( this );
	if ( dlg.DoModal() == IDOK )
	{
		SNATIVEID sID = dlg.m_sSELECTED;
		PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( sID );
		if( pCROW )
		{
			SetWin_Num_int( this, IDC_EDIT_CROW_MID_TO, pCROW->sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_CROW_SID_TO, pCROW->sNativeID.wSubID );
		}
	}
}

void CDlgCrowReplace::OnEnChangeEditCrowMidFrom()
{
	ShowCrowName( IDC_EDIT_CROW_MID_FROM, IDC_EDIT_CROW_SID_FROM, IDC_EDIT_CROW_NAME_FROM );
}

void CDlgCrowReplace::OnEnChangeEditCrowSidFrom()
{
	ShowCrowName( IDC_EDIT_CROW_MID_FROM, IDC_EDIT_CROW_SID_FROM, IDC_EDIT_CROW_NAME_FROM );
}

void CDlgCrowReplace::OnEnChangeEditCrowMidTo()
{
	ShowCrowName( IDC_EDIT_CROW_MID_TO, IDC_EDIT_CROW_SID_TO, IDC_EDIT_CROW_NAME_TO );
}

void CDlgCrowReplace::OnEnChangeEditCrowSidTo()
{
	ShowCrowName( IDC_EDIT_CROW_MID_TO, IDC_EDIT_CROW_SID_TO, IDC_EDIT_CROW_NAME_TO );
}

void CDlgCrowReplace::OnBnClickedButtonCrowAdd()
{
	WORD wMID_FROM = GetWin_Num_int( this, IDC_EDIT_CROW_MID_FROM );
	WORD wSID_FROM = GetWin_Num_int( this, IDC_EDIT_CROW_SID_FROM );
	WORD wMID_TO = GetWin_Num_int( this, IDC_EDIT_CROW_MID_TO );
	WORD wSID_TO = GetWin_Num_int( this, IDC_EDIT_CROW_SID_TO );

	if ( wMID_FROM == SNATIVEID::ID_NULL )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "MID FROM: %d is not allowed", wMID_FROM );
		return;
	}

	if ( wSID_FROM == SNATIVEID::ID_NULL )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "SID FROM: %d is not allowed", wSID_FROM );
		return;
	}

	if ( wMID_TO == SNATIVEID::ID_NULL )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "MID TO: %d is not allowed", wMID_TO );
		return;
	}

	if ( wSID_TO == SNATIVEID::ID_NULL )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "SID TO: %d is not allowed", wSID_TO );
		return;
	}

	//check only target crow valid
	PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( wMID_TO, wSID_TO );
	if ( !pCROW )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Crow 'TO' (%d~%d)", wMID_TO, wSID_TO );
		return;
	}

	SCROWREPLACE sDATA;
	sDATA.sIDFROM.wMainID = wMID_FROM;
	sDATA.sIDFROM.wSubID = wSID_FROM;
	sDATA.sIDTO.wMainID = wMID_TO;
	sDATA.sIDTO.wSubID = wSID_TO;

	m_vecReplace.push_back( sDATA );

	std::sort( m_vecReplace.begin(), m_vecReplace.end() );
	ShowData();
}

void CDlgCrowReplace::OnBnClickedButtonDelete()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	int nINDEX = (int) m_List.GetItemData ( nSelect );

	m_vecReplace.erase( m_vecReplace.begin() + nINDEX );
	ShowData();
}

void CDlgCrowReplace::OnBnClickedButtonDelete2()
{
	m_vecReplace.clear();
	ShowData();
}

void CDlgCrowReplace::OnBnClickedOk()
{
	std::sort( m_vecReplace.begin(), m_vecReplace.end() );
	OnOK();
}
