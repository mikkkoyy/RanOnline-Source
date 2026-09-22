// DlgFindItem.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "DlgFindItem.h"
#include "EtcFunction.h"
#include "HLibDataConvert.h"

// CDlgFindItem dialog

IMPLEMENT_DYNAMIC(CDlgFindItem, CDialog)

CDlgFindItem::CDlgFindItem(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFindItem::IDD, pParent)
	, m_sSELECTED( false )
{
	m_vecITEM.clear();
	m_vecResult.clear();
}

CDlgFindItem::~CDlgFindItem()
{
	m_vecITEM.clear();
	m_vecResult.clear();
}

void CDlgFindItem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CDlgFindItem, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnBnClickedButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_HELPER_SEARCH, OnBnClickedButtonHelperSearch)
END_MESSAGE_MAP()


// CDlgFindItem message handlers
BOOL CDlgFindItem::OnInitDialog()
{
	CDialog::OnInitDialog();

	RECT rectCtrl;
	LONG lnWidth;
	LV_COLUMN lvColumn;

	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	lnWidth = rectCtrl.right - rectCtrl.left;

	const int nColumnCount = 3;	
	char* szColumnName[nColumnCount] = { "Main", "Sub", "Name" };
	int nColumnWidthPercent[nColumnCount] = { 20, 20, 60 };

	for ( int i = 0; i < nColumnCount; i++ )
	{		
		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.pszText = szColumnName[i];
		lvColumn.iSubItem = i;
		lvColumn.cx = ( lnWidth*nColumnWidthPercent[i] ) / 100;
		m_List.InsertColumn (i, &lvColumn );
	}

	GetItems();

	const int nSIZE = 3;

	std::string	strSearchItem[nSIZE] =
	{
		_T("Search ItemName"),
		_T("Search ItemMID"),
		_T("Search ItemSID"),
	};

	SetWin_Combo_Init( this, IDC_COMBO_ITEM_SEARCH, strSearchItem, nSIZE );
	SetWin_Text( this, IDC_EDIT_ITEM_SEARCH, "" );

	DataSearch();

	return TRUE;  
}

void CDlgFindItem::GetItems()
{
	m_vecITEM.clear();

	for( int MID = 0; MID < GLItemMan::MAX_MID; ++ MID  )
	{
		for ( int SID = 0; SID < GLItemMan::MAX_SID; ++ SID )
		{
			SITEM* pITEM = GLItemMan::GetInstance().GetItem( MID, SID );
			if ( pITEM )
			{
				SITEMDATA sDATA;
				sDATA.sITEMID = pITEM->sBasicOp.sNativeID;
				sDATA.strITEMNAME = pITEM->GetName();
				m_vecITEM.push_back( sDATA );
			}
		}
	}
}

void CDlgFindItem::ShowResults()
{
	m_List.DeleteAllItems();
	
	int nSIZE = (int)m_vecResult.size();
	if ( nSIZE <= 0 )	return;

	m_List.SetRedraw( FALSE );

	for ( int i = 0; i < nSIZE; i++ )
	{
		SITEMDATA sDATA = m_vecResult[i];
		m_List.InsertItem( i, _HLIB::cstring_dword(sDATA.sITEMID.wMainID).GetString() );
		m_List.SetItemText( i, 1, _HLIB::cstring_dword(sDATA.sITEMID.wSubID).GetString() );
		m_List.SetItemText( i, 2, sDATA.strITEMNAME.c_str() );
		m_List.SetItemData ( i, sDATA.sITEMID.dwID );
	}

	m_List.SetRedraw( TRUE );
}

void CDlgFindItem::OnBnClickedButtonOk()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = (DWORD) m_List.GetItemData ( nSelect );

	SITEM* pITEM = GLItemMan::GetInstance().GetItem( SNATIVEID ( dwID ) );
	if ( pITEM )
	{
		m_sSELECTED = pITEM->sBasicOp.sNativeID;
		OnOK();
	}
}

void CDlgFindItem::OnBnClickedButtonHelperSearch()
{
	DataSearch();
}

void CDlgFindItem::DataSearch()
{
	int nSEARCH =  GetWin_Combo_Sel( this, IDC_COMBO_ITEM_SEARCH );
	std::string strSEARCH = GetWin_Text( this, IDC_EDIT_ITEM_SEARCH );

	m_vecResult.clear();

	if ( strSEARCH.size() <= 0 )
	{
		m_vecResult = m_vecITEM;
	}
	else
	{
		for( int i = 0; i < (int)m_vecITEM.size(); ++ i )
		{
			CString strFIND;
			switch( nSEARCH )
			{
			case 0:
				{
					strFIND.Format( "%s", m_vecITEM[i].strITEMNAME.c_str() );
				}break;
			case 1:
				{
					strFIND.Format( "%d", m_vecITEM[i].sITEMID.wMainID );
				}break;
			case 2:
				{
					strFIND.Format( "%d", m_vecITEM[i].sITEMID.wSubID );
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
				m_vecResult.push_back( m_vecITEM[i] );
			}
		}
	}

	ShowResults();
}
