// DlgSearch.cpp : implementation file
//

#include "stdafx.h"
#include "EditorQuest.h"
#include "DlgSearch.h"
#include "EtcFunction.h"
#include "HLibDataConvert.h"

// CDlgSearch dialog

IMPLEMENT_DYNAMIC(CDlgSearch, CDialog)

CDlgSearch::CDlgSearch(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSearch::IDD, pParent)
	, m_dwQUESTID( SNATIVEID::ID_NULL )
	, m_vec( NULL )
{

}

CDlgSearch::~CDlgSearch()
{
	m_vec = NULL;
}

void CDlgSearch::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_RESULT, m_List);
}


BEGIN_MESSAGE_MAP(CDlgSearch, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnBnClickedButtonSearch)
	ON_BN_CLICKED(IDC_BUTTON_GO, OnBnClickedButtonGo)
	ON_CBN_SELCHANGE(IDC_COMBO_SEARCHTYPE, OnCbnSelchangeComboSearchtype)
END_MESSAGE_MAP()


// CDlgSearch message handlers
BOOL CDlgSearch::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.InsertColumn( 0, "Num", LVCFMT_LEFT, 50 );
	m_List.InsertColumn( 1, "QuestID", LVCFMT_LEFT, 70 );
	m_List.InsertColumn( 2, "FileName", LVCFMT_LEFT, 100 );
	m_List.InsertColumn( 3, "Title", LVCFMT_LEFT, 220 );
	m_List.InsertColumn( 4, "Search", LVCFMT_LEFT, 140 );

	std::string strSEARCH[EMSEARCH_TOTAL] = 
	{
		"Search Title",
		"Search FileName",
		"Search ID",
	};

	SetWin_Combo_Init( this, IDC_COMBO_SEARCHTYPE, strSEARCH, EMSEARCH_TOTAL );

	m_dwQUESTID = SNATIVEID::ID_NULL;

	ShowResults();
	SearchInit();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgSearch::OnBnClickedButtonSearch()
{
	int nSELECT = GetWin_Combo_Sel( this, IDC_COMBO_SEARCHTYPE );
	int nSEARCH = GetWin_Num_int( this, IDC_EDIT_SEARCH );
	std::string strSEARCH = GetWin_Text( this, IDC_EDIT_SEARCH );
	
	if ( nSELECT < 0 || nSELECT >= EMSEARCH_TOTAL )	return;

	m_List.DeleteAllItems();
	m_dwQUESTID = SNATIVEID::ID_NULL;

	std::vector<SSEARCH> vec;

	switch ( nSELECT )
	{	
	
	case EMSEARCH_TITLE:
		{
			//search by title
			GLQuestMan::MAPQUEST& m_mapQuestMap = GLQuestMan::GetInstance().GetMap();
			GLQuestMan::MAPQUEST_ITER iter_start = m_mapQuestMap.begin();
			GLQuestMan::MAPQUEST_ITER iter_end = m_mapQuestMap.end();

			for ( ; iter_start != iter_end; iter_start++ )
			{
				DWORD dwQuestID = (DWORD) iter_start->first;	
				QUESTNODE2 sNODE = (QUESTNODE2) iter_start->second;

				if ( !sNODE.pQUEST )		continue;

				std::string strSearch = strSEARCH;
				std::string strFindString = sNODE.pQUEST->GetTITLE();

				if ( strFindString.size() > 0 )
				{
					std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
					std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

					unsigned found = strFindString.find(strSearch.c_str());
					if (found!=std::string::npos)
					{
						SSEARCH sResult;
						sResult.dwQUESTID = sNODE.pQUEST->m_sNID.dwID;
						sResult.strNAME = sNODE.pQUEST->GetTITLE();
						sResult.strFILENAME = sNODE.strFILE.c_str();
						sResult.strFIND = strFindString.c_str();
						vec.push_back( sResult );
					}
				}
			}	
		}break;

	case EMSEARCH_FILENAME:
		{
			//search by filename
			GLQuestMan::MAPQUEST& m_mapQuestMap = GLQuestMan::GetInstance().GetMap();
			GLQuestMan::MAPQUEST_ITER iter_start = m_mapQuestMap.begin();
			GLQuestMan::MAPQUEST_ITER iter_end = m_mapQuestMap.end();

			for ( ; iter_start != iter_end; iter_start++ )
			{
				DWORD dwQuestID = (DWORD) iter_start->first;	
				QUESTNODE2 sNODE = (QUESTNODE2) iter_start->second;

				if ( !sNODE.pQUEST )		continue;

				std::string strSearch = strSEARCH;
				std::string strFindString = sNODE.strFILE.c_str();

				if ( strFindString.size() > 0 )
				{
					std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
					std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

					unsigned found = strFindString.find(strSearch.c_str());
					if (found!=std::string::npos)
					{
						SSEARCH sResult;
						sResult.dwQUESTID = sNODE.pQUEST->m_sNID.dwID;
						sResult.strNAME = sNODE.pQUEST->GetTITLE();
						sResult.strFILENAME = sNODE.strFILE.c_str();
						sResult.strFIND = strFindString.c_str();
						vec.push_back( sResult );
					}
				}
			}	
		}break;

	case EMSEARCH_ID:
		{
			//search by quest is
			GLQuestMan::MAPQUEST& m_mapQuestMap = GLQuestMan::GetInstance().GetMap();
			GLQuestMan::MAPQUEST_ITER iter_start = m_mapQuestMap.begin();
			GLQuestMan::MAPQUEST_ITER iter_end = m_mapQuestMap.end();

			for ( ; iter_start != iter_end; iter_start++ )
			{
				DWORD dwQuestID = (DWORD) iter_start->first;	
				QUESTNODE2 sNODE = (QUESTNODE2) iter_start->second;

				if ( !sNODE.pQUEST )		continue;

				if ( sNODE.pQUEST->m_sNID.dwID == nSEARCH  )
				{
					SSEARCH sResult;
					sResult.dwQUESTID = sNODE.pQUEST->m_sNID.dwID;
					sResult.strNAME = sNODE.pQUEST->GetTITLE();
					sResult.strFILENAME = sNODE.strFILE.c_str();
					sResult.strFIND = strSEARCH.c_str();
					vec.push_back( sResult );
				}
			}	
		}break;
	};


	if ( m_vec )
	{
		m_vec->clear();
		for( size_t i=0; i<vec.size(); ++i)
		{
			SSEARCH sRESULT = vec[i];
			m_vec->push_back( sRESULT );
		}
	}

	ShowResults();
}

void CDlgSearch::OnBnClickedButtonGo()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	m_dwQUESTID = (WORD)atoi( m_List.GetItemText(nSelect,1).GetString() );

	OnOK();
}

void CDlgSearch::OnCbnSelchangeComboSearchtype()
{
	SearchInit();
}


void CDlgSearch::ShowResults()
{
	if ( m_vec )
	{
		m_List.DeleteAllItems();
		m_List.SetRedraw( FALSE );

		for( size_t i=0; i<m_vec->size(); ++i)
		{
			SSEARCH sRESULT = m_vec->at(i);
			m_List.InsertItem( i, _HLIB::cstring_dword(i).GetString(), NULL );
			m_List.SetItemText( i, 1, _HLIB::cstring_dword(sRESULT.dwQUESTID).GetString() );
			m_List.SetItemText( i, 2, sRESULT.strFILENAME.c_str() );
			m_List.SetItemText( i, 3, sRESULT.strNAME.c_str() );
			m_List.SetItemText( i, 4, sRESULT.strFIND.c_str() );
		}

		m_List.SetRedraw( TRUE );
	}
}

void CDlgSearch::SearchInit()
{
	EMSEARCH emSearch = static_cast< EMSEARCH > ( GetWin_Combo_Sel( this, IDC_COMBO_SEARCHTYPE ));
	std::string strTEXT = GetWin_Combo_Sel_Text( this, IDC_COMBO_SEARCHTYPE ).GetString();

	SetWin_Text( this, IDC_STATIC_SEARCH_1, strTEXT.c_str() );
}
