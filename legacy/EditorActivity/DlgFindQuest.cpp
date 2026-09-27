// DlgFindQuest.cpp : implementation file
//

#include "stdafx.h"
#include "EditorActivity.h"
#include "DlgFindQuest.h"
#include "EtcFunction.h"
#include "HLibDataConvert.h"

// CDlgFindQuest dialog

IMPLEMENT_DYNAMIC(CDlgFindQuest, CDialog)

CDlgFindQuest::CDlgFindQuest(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFindQuest::IDD, pParent)
{
	m_vecData.clear();
	m_vecResult.clear();

	m_sSelected = SQUESTDATA();
}

CDlgFindQuest::~CDlgFindQuest()
{
	m_vecData.clear();
	m_vecResult.clear();
}

void CDlgFindQuest::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CDlgFindQuest, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnBnClickedButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_HELPER_SEARCH, OnBnClickedButtonHelperSearch)
END_MESSAGE_MAP()


// CDlgFindQuest message handlers
BOOL CDlgFindQuest::OnInitDialog()
{
	CDialog::OnInitDialog();

	RECT rectCtrl;
	LONG lnWidth;
	LV_COLUMN lvColumn;

	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	lnWidth = rectCtrl.right - rectCtrl.left;

	const int nColumnCount = 3;	
	char* szColumnName[nColumnCount] = { "QuestID", "QuestName", "QuestFile" };
	int nColumnWidthPercent[nColumnCount] = { 20, 50, 30 };

	for ( int i = 0; i < nColumnCount; i++ )
	{		
		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.pszText = szColumnName[i];
		lvColumn.iSubItem = i;
		lvColumn.cx = ( lnWidth*nColumnWidthPercent[i] ) / 100;
		m_List.InsertColumn (i, &lvColumn );
	}

	ReadQuests();

	const int nSIZE = 3;

	std::string	strSearchQuest[nSIZE] =
	{
		_T("Search QuestName"),
		_T("Search QuestID"),
		_T("Search QuestFile"),
	};

	SetWin_Combo_Init( this, IDC_COMBO_QUEST_SEARCH, strSearchQuest, nSIZE );
	SetWin_Text( this, IDC_EDIT_QUEST_SEARCH, "" );

	DataSearch();

	return TRUE;  
}

void CDlgFindQuest::ReadQuests()
{
	m_vecData.clear();

	GLQuestMan::MAPQUEST& m_mapQuestMap = GLQuestMan::GetInstance().GetMap();
	GLQuestMan::MAPQUEST_ITER iter_start = m_mapQuestMap.begin();
	GLQuestMan::MAPQUEST_ITER iter_end = m_mapQuestMap.end();

	for ( ; iter_start != iter_end; iter_start++ )
	{
		DWORD dwQuestID = (DWORD) iter_start->first;	
		QUESTNODE sNODE = (QUESTNODE) iter_start->second;

		if ( !sNODE.pQUEST )		continue;

		SQUESTDATA sDATA;
		sDATA.dwQUESTID = sNODE.pQUEST->m_sNID.dwID;
		sDATA.strQUESTNAME = sNODE.pQUEST->m_strTITLE.c_str();
		sDATA.strQUESTFILE = sNODE.strFILE.c_str();
		m_vecData.push_back( sDATA );
	}
}

void CDlgFindQuest::ShowResults()
{
	m_List.DeleteAllItems();
	
	int nSIZE = (int)m_vecResult.size();
	if ( nSIZE <= 0 )	return;

	m_List.SetRedraw( FALSE );

	for ( int i = 0; i < nSIZE; i++ )
	{
		SQUESTDATA sDATA = m_vecResult[i];
		m_List.InsertItem( i, _HLIB::cstring_dword(sDATA.dwQUESTID).GetString() );
		m_List.SetItemText( i, 1, sDATA.strQUESTNAME.c_str() );
		m_List.SetItemText( i, 2, sDATA.strQUESTFILE.c_str() );
		m_List.SetItemData ( i, sDATA.dwQUESTID );
	}

	m_List.SetRedraw( TRUE );
}

SQUESTDATA* CDlgFindQuest::GetQuestData( DWORD dwQUESTID )
{
	for( int i = 0; i < (int)m_vecData.size(); ++ i )
	{
		SQUESTDATA& sDATA = m_vecData[i];
		if( sDATA.dwQUESTID == dwQUESTID )
			return &sDATA;
	}

	return NULL;
}

void CDlgFindQuest::OnBnClickedButtonOk()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = (DWORD) m_List.GetItemData ( nSelect );

	SQUESTDATA* pDATA = GetQuestData( dwID );
	if( !pDATA )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Quest Selected!" );
		return;
	}

	GLQUEST* pQUEST = GLQuestMan::GetInstance().Find( pDATA->dwQUESTID );
	if( pQUEST )
	{
		m_sSelected = *pDATA;
		OnOK();
	}
}

void CDlgFindQuest::OnBnClickedButtonHelperSearch()
{
	DataSearch();
}

void CDlgFindQuest::DataSearch()
{
	int nSEARCH = GetWin_Combo_Sel( this, IDC_COMBO_QUEST_SEARCH );
	std::string strSEARCH = GetWin_Text( this, IDC_EDIT_QUEST_SEARCH );

	m_vecResult.clear();

	if ( strSEARCH.size() <= 0 )
	{
		m_vecResult = m_vecData;
	}
	else
	{
		for( int i = 0; i < (int)m_vecData.size(); ++ i )
		{
			CString strFIND;
			switch( nSEARCH )
			{
			case 0:
				{
					strFIND.Format( "%s", m_vecData[i].strQUESTNAME.c_str() );
				}break;
			case 1:
				{
					strFIND.Format( "%d", m_vecData[i].dwQUESTID );
				}break;
			case 2:
				{
					strFIND.Format( "%s", m_vecData[i].strQUESTFILE.c_str() );
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
				m_vecResult.push_back( m_vecData[i] );
			}
		}
	}

	ShowResults();
}
