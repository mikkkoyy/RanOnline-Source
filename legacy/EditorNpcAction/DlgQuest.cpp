// DlgQuest.cpp : implementation file
//

#include "stdafx.h"
#include "EditorNpcAction.h"
#include "DlgQuest.h"

#include "EtcFunction.h"

// CDlgQuest dialog

IMPLEMENT_DYNAMIC(CDlgQuest, CDialog)

CDlgQuest::CDlgQuest(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgQuest::IDD, pParent)
	, m_SelectedQuest( UINT_MAX )
{
	m_vecData.clear();
	m_vecResult.clear();
}

CDlgQuest::~CDlgQuest()
{
}

void CDlgQuest::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SELECT, m_listSELECT );
}


BEGIN_MESSAGE_MAP(CDlgQuest, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_SELECT, OnLbnSelchangeListSelect)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnBnClickedButtonSearch)
END_MESSAGE_MAP()


// CDlgQuest message handlers
BOOL CDlgQuest::OnInitDialog()
{
	CDialog::OnInitDialog();

	ReadData();
	SelectionReset();

	const int nSEARCHSIZE = 3;

	std::string strSEARCH[nSEARCHSIZE] =
	{
		"Search Title",
		"Search File",
		"Search ID",
	};

	SetWin_Combo_Init( this, IDC_COMBO_SEARCH, strSEARCH, nSEARCHSIZE );
	SetWin_Text( this, IDC_EDIT_SEARCH, "" );

	SearchData();

	return TRUE;
}

void CDlgQuest::ReadData()
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

		SQUESTDATA_DLG sDATA;
		sDATA.dwQUESTID = sNODE.pQUEST->m_sNID.dwID;
		sDATA.strQUESTNAME = sNODE.pQUEST->m_strTITLE.c_str();
		sDATA.strQUESTFILE = sNODE.strFILE.c_str();
		m_vecData.push_back( sDATA );
	}
}

void CDlgQuest::ShowData()
{
	m_listSELECT.ResetContent();
	m_listSELECT.SetRedraw( FALSE );

	for( int i = 0; i < (int) m_vecResult.size(); ++ i )
	{
		SQUESTDATA_DLG sDATA = m_vecResult[i];
		GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( sDATA.dwQUESTID );
		if ( !pQUEST )	continue;

		CString strNAME;
		strNAME.Format ( "[%05d][%s] %s", sDATA.dwQUESTID, sDATA.strQUESTFILE.c_str(), sDATA.strQUESTNAME.c_str() );

		int nIndex = m_listSELECT.AddString ( strNAME );
		m_listSELECT.SetItemData ( nIndex, sDATA.dwQUESTID );
	}

	m_listSELECT.SetRedraw( TRUE );
}

void CDlgQuest::SelectionReset()
{
	m_SelectedQuest = UINT_MAX;
	SetWin_Num_int( this, IDC_EDIT_SEL_ID, m_SelectedQuest );
	SetWin_Text( this, IDC_EDIT_SEL_NAME, "" );
}


void CDlgQuest::OnLbnSelchangeListSelect()
{
	int nSel = m_listSELECT.GetCurSel();

	if ( nSel < 0 ) return;

	DWORD dwData = (DWORD) m_listSELECT.GetItemData ( nSel );

	GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( dwData );
	if ( pQUEST )	
	{
		m_SelectedQuest = pQUEST->m_sNID.dwID;
		SetWin_Num_int( this, IDC_EDIT_SEL_ID, m_SelectedQuest );
		SetWin_Text( this, IDC_EDIT_SEL_NAME, pQUEST->GetTITLE() );
	}
	else
	{
		SelectionReset();
	}
}

void CDlgQuest::OnBnClickedButtonSearch()
{
	SearchData();
}

void CDlgQuest::SearchData()
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
			SQUESTDATA_DLG sDATA = m_vecData[i];

			GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( sDATA.dwQUESTID );
			if ( !pQUEST )	continue;

			CString strFIND;
			switch( nSEARCH )
			{
			case 0:
				{
					strFIND.Format( "%s", sDATA.strQUESTNAME.c_str() );
				}break;
			case 1:
				{
					strFIND.Format( "%s", sDATA.strQUESTFILE.c_str() );
				}break;
			case 2:
				{
					strFIND.Format( "%d", sDATA.dwQUESTID );
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
				m_vecResult.push_back( sDATA );
			}
		}
	}

	ShowData();
}