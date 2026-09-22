// DlgMap.cpp : implementation file
//

#include "stdafx.h"
#include "EditorItem.h"
#include "DlgMap.h"
#include "EtcFunction.h"

// CDlgMap dialog

IMPLEMENT_DYNAMIC(CDlgMap, CDialog)

CDlgMap::CDlgMap(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgMap::IDD, pParent)
	, m_SELECTED( NATIVEID_NULL() )
{
	m_vecData.clear();
	m_vecResult.clear();
}

CDlgMap::~CDlgMap()
{
}

void CDlgMap::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SELECT, m_listSELECT );
}


BEGIN_MESSAGE_MAP(CDlgMap, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_SELECT, OnLbnSelchangeListSelect)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnBnClickedButtonSearch)
END_MESSAGE_MAP()


// CDlgMap message handlers
BOOL CDlgMap::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_mapList.CleanUp();
	m_mapList.LoadMapsListFile( "mapslist.mst" );

	ReadData();
	SelectionReset();

	const int nSEARCHSIZE = 3;

	std::string strSEARCH[nSEARCHSIZE] =
	{
		"Search Name",
		"Search MID",
		"Search SID",
	};

	SetWin_Combo_Init( this, IDC_COMBO_SEARCH, strSEARCH, nSEARCHSIZE );
	SetWin_Text( this, IDC_EDIT_SEARCH, "" );

	SearchData();

	return TRUE;
}

void CDlgMap::ReadData()
{
	m_vecData.clear();

	GLMapList::FIELDMAP map = m_mapList.GetMapList();
	GLMapList::FIELDMAP_ITER iter_start = map.begin();
	GLMapList::FIELDMAP_ITER iter_end = map.end();

	for ( ; iter_start != iter_end; ++ iter_start )
	{
		SMAPNODE sNODE = (*iter_start).second;	
		m_vecData.push_back( sNODE.sNativeID );
	}
}

void CDlgMap::ShowData()
{
	m_listSELECT.ResetContent();
	m_listSELECT.SetRedraw( FALSE );

	for( int i = 0; i < (int) m_vecResult.size(); ++ i )
	{
		SNATIVEID sID = m_vecResult[i];
		SMAPNODE* pNode = m_mapList.FindMapNode( sID );
		if ( !pNode )	continue;

		CString strNAME;
		strNAME.Format ( "[%03d/%03d] %s", pNode->sNativeID.wMainID, pNode->sNativeID.wSubID, pNode->strMapName.c_str() );

		int nIndex = m_listSELECT.AddString ( strNAME );
		m_listSELECT.SetItemData ( nIndex, pNode->sNativeID.dwID );	
	}

	m_listSELECT.SetRedraw( TRUE );
}

void CDlgMap::SelectionReset()
{
	m_SELECTED = NATIVEID_NULL();
	SetWin_Num_int( this, IDC_EDIT_SEL_MID, m_SELECTED.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SEL_SID, m_SELECTED.wSubID );
	SetWin_Text( this, IDC_EDIT_SEL_NAME, "" );
}

void CDlgMap::OnLbnSelchangeListSelect()
{
	int nSel = m_listSELECT.GetCurSel();

	if ( nSel < 0 ) return;

	DWORD dwData = (DWORD) m_listSELECT.GetItemData ( nSel );
	SNATIVEID sID;
	sID.dwID = dwData;

	SMAPNODE* pNode = m_mapList.FindMapNode( sID );
	if ( pNode )
	{
		m_SELECTED = pNode->sNativeID;
		SetWin_Num_int( this, IDC_EDIT_SEL_MID, m_SELECTED.wMainID );
		SetWin_Num_int( this, IDC_EDIT_SEL_SID, m_SELECTED.wSubID );
		SetWin_Text( this, IDC_EDIT_SEL_NAME, pNode->strMapName.c_str() );
	}
	else
	{
		SelectionReset();
	}
}

void CDlgMap::OnBnClickedButtonSearch()
{
	SearchData();
}

void CDlgMap::SearchData()
{
	int nSEARCH =  GetWin_Combo_Sel( this, IDC_COMBO_SEARCH );
	std::string strSEARCH = GetWin_Text( this, IDC_EDIT_SEARCH );

	m_vecResult.clear();

	if ( strSEARCH.size() <= 0 )
	{
		m_vecResult = m_vecData;
	}
	else
	{
		for( int i = 0; i < (int)m_vecData.size(); ++ i )
		{
			SNATIVEID sID = m_vecData[i];
			SMAPNODE* pNode = m_mapList.FindMapNode( sID );
			if ( !pNode )	continue;

			CString strFIND;
			switch( nSEARCH )
			{
			case 0:
				{
					strFIND.Format( "%s", pNode->strMapName.c_str() );
				}break;
			case 1:
				{
					strFIND.Format( "%d", pNode->sNativeID.wMainID );
				}break;
			case 2:
				{
					strFIND.Format( "%d", pNode->sNativeID.wSubID );
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
				m_vecResult.push_back( sID );
			}
		}
	}

	ShowData();
}

