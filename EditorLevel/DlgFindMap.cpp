// DlgFindMap.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "DlgFindMap.h"
#include "EtcFunction.h"
#include "EditorLevelView.h"
#include "HLibDataConvert.h"

// CDlgFindMap dialog

IMPLEMENT_DYNAMIC(CDlgFindMap, CDialog)

CDlgFindMap::CDlgFindMap(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFindMap::IDD, pParent)
{
	m_vecMap.clear();
	m_vecResult.clear();
	m_sSELECTED = SMAPDATA_LIST();
}

CDlgFindMap::~CDlgFindMap()
{
	m_vecMap.clear();
	m_vecResult.clear();
}

void CDlgFindMap::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CDlgFindMap, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnBnClickedButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_HELPER_SEARCH, OnBnClickedButtonHelperSearch)
END_MESSAGE_MAP()


// CDlgFindMap message handlers
BOOL CDlgFindMap::OnInitDialog()
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

	GetMaps();

	const int nSIZE = 3;
	std::string	strSearchMap[nSIZE] =
	{
		_T("Search Map Name"),
		_T("Search Map MID"),
		_T("Search Map SID"),
	};

	SetWin_Combo_Init( this, IDC_COMBO_MAP_SEARCH, strSearchMap, nSIZE );
	SetWin_Text( this, IDC_EDIT_MAP_SEARCH, "" );

	DataSearch();

	return TRUE;
}

void CDlgFindMap::GetMaps()
{
	m_vecMap.clear();

	GLMapList::FIELDMAP MapsList = CEditorLevelView::GetView()->m_sMap.GetMapList();
	GLMapList::FIELDMAP_ITER iter_start = MapsList.begin();
	GLMapList::FIELDMAP_ITER iter_end = MapsList.end();

	for( ; iter_start != iter_end; ++iter_start )
	{
		const SMAPNODE *pMapNode = &(*iter_start).second;
		if ( pMapNode )
		{
			SMAPDATA_LIST sMAP;
			sMAP.sMAPID = pMapNode->sNativeID;
			sMAP.strNAME = pMapNode->strMapName.c_str();

			m_vecMap.push_back( sMAP );
		}	
	}
}

void CDlgFindMap::ShowResults()
{
	m_List.DeleteAllItems();
	
	int nSIZE = (int)m_vecResult.size();
	if ( nSIZE <= 0 )	return;

	m_List.SetRedraw( FALSE );

	for ( int i = 0; i < nSIZE; i++ )
	{
		SMAPDATA_LIST sDATA = m_vecResult[i];
		m_List.InsertItem( i, _HLIB::cstring_dword(sDATA.sMAPID.wMainID).GetString() );
		m_List.SetItemText( i, 1, _HLIB::cstring_dword(sDATA.sMAPID.wSubID).GetString() );
		m_List.SetItemText( i, 2, sDATA.strNAME.c_str() );
		m_List.SetItemData ( i, sDATA.sMAPID.dwID );
	}

	m_List.SetRedraw( TRUE );
}

void CDlgFindMap::OnBnClickedButtonOk()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = (DWORD) m_List.GetItemData ( nSelect );

	SMAPNODE* pNODE = CEditorLevelView::GetView()->m_sMap.FindMapNode( SNATIVEID(dwID) );
	if ( pNODE )
	{
		m_sSELECTED.sMAPID = pNODE->sNativeID;
		m_sSELECTED.strNAME = pNODE->strMapName.c_str();

		OnOK();
	}
}

void CDlgFindMap::OnBnClickedButtonHelperSearch()
{
	DataSearch();
}

void CDlgFindMap::DataSearch()
{
	int nSEARCH = GetWin_Combo_Sel( this, IDC_COMBO_MAP_SEARCH );
	std::string strSEARCH = GetWin_Text( this, IDC_EDIT_MAP_SEARCH );

	m_vecResult.clear();

	if ( strSEARCH.size() <= 0 )
	{
		m_vecResult = m_vecMap;
	}
	else
	{
		for( int i = 0; i < (int)m_vecMap.size(); ++ i )
		{
			CString strFIND;
			switch( nSEARCH )
			{
			case 0:
				{
					strFIND.Format( "%s", m_vecMap[i].strNAME.c_str() );
				}break;
			case 1:
				{
					strFIND.Format( "%d", m_vecMap[i].sMAPID.wMainID );
				}break;
			case 2:
				{
					strFIND.Format( "%d", m_vecMap[i].sMAPID.wSubID );
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
				m_vecResult.push_back( m_vecMap[i] );
			}
		}
	}

	ShowResults();
}