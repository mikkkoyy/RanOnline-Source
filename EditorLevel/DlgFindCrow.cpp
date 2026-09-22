// DlgFindCrow.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "DlgFindCrow.h"
#include "EtcFunction.h"
#include "HLibDataConvert.h"

// CDlgFindCrow dialog

IMPLEMENT_DYNAMIC(CDlgFindCrow, CDialog)

CDlgFindCrow::CDlgFindCrow(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFindCrow::IDD, pParent)
	, m_sSELECTED( false )
{
	m_vecCrow.clear();
	m_vecResult.clear();
}

CDlgFindCrow::~CDlgFindCrow()
{
	m_vecCrow.clear();
	m_vecResult.clear();
}

void CDlgFindCrow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CDlgFindCrow, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnBnClickedButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_HELPER_SEARCH, OnBnClickedButtonHelperSearch)
END_MESSAGE_MAP()


// CDlgFindCrow message handlers
BOOL CDlgFindCrow::OnInitDialog()
{
	CDialog::OnInitDialog();

	RECT rectCtrl;
	LONG lnWidth;
	LV_COLUMN lvColumn;

	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	lnWidth = rectCtrl.right - rectCtrl.left;

	const int nColumnCount = 4;	
	char* szColumnName[nColumnCount] = { "Main", "Sub", "Name", "Skin" };
	int nColumnWidthPercent[nColumnCount] = { 10, 10, 50, 30 };

	for ( int i = 0; i < nColumnCount; i++ )
	{		
		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.pszText = szColumnName[i];
		lvColumn.iSubItem = i;
		lvColumn.cx = ( lnWidth*nColumnWidthPercent[i] ) / 100;
		m_List.InsertColumn (i, &lvColumn );
	}

	GetCrows();

	const int nSIZE = 4;
	std::string strSearchCrow[nSIZE] = 
	{
		_T("Search Crow Name"),
		_T("Search Crow MID"),
		_T("Search Crow SID"),
		_T("Search Crow Skin"),
	};

	SetWin_Combo_Init( this, IDC_COMBO_CROW_SEARCH, strSearchCrow, nSIZE );
	SetWin_Text( this, IDC_EDIT_CROW_SEARCH, "" );

	DataSearch();

	return TRUE;  
}

void CDlgFindCrow::GetCrows()
{
	m_vecCrow.clear();

	for( int nMID = 0; nMID < GLCrowDataMan::MAX_CROW_MID; ++ nMID )
	{
		for( int nSID = 0; nSID < GLCrowDataMan::MAX_CROW_SID; ++ nSID )
		{
			SCROWDATA* pCrow = GLCrowDataMan::GetInstance().GetCrowData( nMID, nSID );
			if ( pCrow )
			{
				SCROW_DATA_SIMPLE sDATA;
				sDATA.sCrowID = pCrow->m_sBasic.sNativeID;
				sDATA.strCrowName = pCrow->GetName();
				sDATA.strCrowSkin = pCrow->GetSkinObjFile();
				m_vecCrow.push_back( sDATA );
			}
		}
	}
}

void CDlgFindCrow::ShowResults()
{
	m_List.DeleteAllItems();
	
	int nSIZE = (int)m_vecResult.size();
	if ( nSIZE <= 0 )	return;

	m_List.SetRedraw( FALSE );

	for ( int i = 0; i < nSIZE; i++ )
	{
		SCROW_DATA_SIMPLE sDATA = m_vecResult[i];
		m_List.InsertItem( i, _HLIB::cstring_dword(sDATA.sCrowID.wMainID).GetString() );
		m_List.SetItemText( i, 1, _HLIB::cstring_dword(sDATA.sCrowID.wSubID).GetString() );
		m_List.SetItemText( i, 2, sDATA.strCrowName.c_str() );
		m_List.SetItemText( i, 3, sDATA.strCrowSkin.c_str() );
		m_List.SetItemData ( i, sDATA.sCrowID.dwID );
	}

	m_List.SetRedraw( TRUE );
}

void CDlgFindCrow::OnBnClickedButtonOk()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = (DWORD) m_List.GetItemData ( nSelect );

	SCROWDATA* pCrow = GLCrowDataMan::GetInstance().GetCrowData( SNATIVEID ( dwID ) );
	if ( pCrow )
	{
		m_sSELECTED = pCrow->m_sBasic.sNativeID;
		OnOK();
	}
}

void CDlgFindCrow::OnBnClickedButtonHelperSearch()
{
	DataSearch();
}

void CDlgFindCrow::DataSearch()
{
	int nSEARCH = GetWin_Combo_Sel( this, IDC_COMBO_CROW_SEARCH );
	std::string strSEARCH = GetWin_Text( this, IDC_EDIT_CROW_SEARCH );

	m_vecResult.clear();

	if ( strSEARCH.size() <= 0 )
	{
		m_vecResult = m_vecCrow;
	}
	else
	{
		for( int i = 0; i < (int)m_vecCrow.size(); ++ i )
		{
			CString strFIND;
			switch( nSEARCH )
			{
			case 0:
				{
					strFIND.Format( "%s", m_vecCrow[i].strCrowName.c_str() );
				}break;
			case 1:
				{
					strFIND.Format( "%d", m_vecCrow[i].sCrowID.wMainID );
				}break;
			case 2:
				{
					strFIND.Format( "%d", m_vecCrow[i].sCrowID.wSubID );
				}break;
			case 3:
				{
					strFIND.Format( "%s", m_vecCrow[i].strCrowSkin.c_str() );
				}break;
			};

			if ( strFIND.GetLength() <= 0 )	continue;

			std::string strSEARCH1 = strSEARCH.c_str();
			std::string strSEARCH2 = strFIND.GetString();
			std::transform ( strSEARCH1.begin(), strSEARCH1.end(), strSEARCH1.begin(), tolower );
			std::transform ( strSEARCH2.begin(), strSEARCH2.end(), strSEARCH2.begin(), tolower );

			unsigned found = strSEARCH2.find(strSEARCH1.c_str());
			if (found!=std::string::npos)
			{
				m_vecResult.push_back( m_vecCrow[i] );
			}
		}
	}

	ShowResults();
}

