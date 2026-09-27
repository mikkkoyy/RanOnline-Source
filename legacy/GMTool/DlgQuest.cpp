// DlgQuest.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgQuest.h"

#include "EtcFunction.h"
#include "Logic/GMToolGlobal.h"

// CDlgQuest dialog

IMPLEMENT_DYNAMIC(CDlgQuest, CDialog)

CDlgQuest::CDlgQuest(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgQuest::IDD, pParent)
	, m_emCLASS(GLCC_ALL_8CLASS)
	, m_wSCHOOL(GLSCHOOL_NUM)
	, m_dwSelectedQuest( UINT_MAX )
{
	m_vecData.clear();
	m_vecResult.clear();
}

CDlgQuest::CDlgQuest(EMCHARCLASS emCLASS, WORD wSCHOOL, CWnd* pParent /*= NULL*/)
	: CDialog(CDlgQuest::IDD, pParent)
	, m_emCLASS(GLCC_ALL_8CLASS)
	, m_wSCHOOL(GLSCHOOL_NUM)
	, m_dwSelectedQuest( UINT_MAX )
{
	m_emCLASS = emCLASS;
	m_wSCHOOL = wSCHOOL;

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
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnBnClickedButtonSearch)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_LBN_SELCHANGE(IDC_LIST_SELECT, OnLbnSelchangeListSelect)
END_MESSAGE_MAP()


// CDlgQuest message handlers
BOOL CDlgQuest::OnInitDialog()
{
	CDialog::OnInitDialog();

	ReadData();
	SelectionReset();

	SetWin_Combo_Init( this, IDC_COMBO_SEARCH, GMTOOL_GLOBAL::strQuestDataSearchType, EMQUESTDATA_SEARCH_SIZE );
	SetWin_Text( this, IDC_EDIT_SEARCH, "" );

	SearchData();

	return TRUE;
}

void CDlgQuest::ReadData()
{
	m_vecData.clear();

	GLQuestMan::MAPQUEST& mapQuestMap = GLQuestMan::GetInstance().GetMap();
	GLQuestMan::MAPQUEST_ITER iter_start = mapQuestMap.begin();

	for ( ; iter_start != mapQuestMap.end(); iter_start++ )
	{
		DWORD dwQuestID = (DWORD) iter_start->first;	
		QUESTNODE sNODE = (QUESTNODE) iter_start->second;

		if ( !sNODE.pQUEST ){
			continue;
		}

		//show only compatible quest for character class and school
		if ( !(sNODE.pQUEST->m_sSTARTOPT.dwCLASS&m_emCLASS) ){
			continue;
		}

		if ( MAX_SCHOOL!=sNODE.pQUEST->m_sSTARTOPT.wSCHOOL ){
			if ( sNODE.pQUEST->m_sSTARTOPT.wSCHOOL!=m_wSCHOOL ){
				continue;
			}
		}

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

	for( int i=0; i<(int) m_vecResult.size(); ++i )
	{
		SQUESTDATA_DLG sDATA = m_vecResult[i];
		GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( sDATA.dwQUESTID );
		if ( !pQUEST ){
			continue;
		}

		CString strNAME;
		strNAME.Format ( "[%05d][%s] %s", sDATA.dwQUESTID, sDATA.strQUESTFILE.c_str(), sDATA.strQUESTNAME.c_str() );

		int nIndex = m_listSELECT.AddString ( strNAME );
		m_listSELECT.SetItemData ( nIndex, sDATA.dwQUESTID );
	}
}

void CDlgQuest::SelectionReset()
{
	m_dwSelectedQuest = UINT_MAX;
	SetWin_Text( this, IDC_EDIT_SEL_NAME, "" );
}

void CDlgQuest::SearchData()
{
	EMQUESTDATA_SEARCH emSEARCH =  static_cast<EMQUESTDATA_SEARCH>( GetWin_Combo_Sel( this, IDC_COMBO_SEARCH ));
	std::string strSEARCH = GetWin_Text( this, IDC_EDIT_SEARCH );

	m_vecResult.clear();

	if ( strSEARCH.size() <= 0 )
	{
		m_vecResult = m_vecData;
	}
	else
	{
		for( int i=0; i<(int)m_vecData.size(); ++i )
		{
			SQUESTDATA_DLG sDATA = m_vecData[i];

			GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( sDATA.dwQUESTID );
			if ( !pQUEST )	continue;

			CString strFIND;
			switch( emSEARCH )
			{
			case EMQUESTDATA_SEARCH_TITLE:{
					strFIND.Format( "%s", sDATA.strQUESTNAME.c_str() );
				}break;

			case EMQUESTDATA_SEARCH_FILE:{
					strFIND.Format( "%s", sDATA.strQUESTFILE.c_str() );
				}break;

			case EMQUESTDATA_SEARCH_ID:{
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

void CDlgQuest::OnBnClickedButtonSearch()
{
	SearchData();
}

void CDlgQuest::OnBnClickedOk()
{
	CDialog::OnOK();
}

void CDlgQuest::OnBnClickedCancel()
{
	CDialog::OnCancel();
}

void CDlgQuest::OnLbnSelchangeListSelect()
{
	int nSel = m_listSELECT.GetCurSel();
	if ( nSel < 0 ){
		return;
	}

	DWORD dwData = (DWORD) m_listSELECT.GetItemData ( nSel );
	GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( dwData );
	if ( pQUEST ){
		m_dwSelectedQuest = pQUEST->m_sNID.dwID;

		CString strNAME;
		strNAME.Format ( "[%05d]%s", pQUEST->m_sNID.dwID, pQUEST->m_strTITLE.c_str() );
		SetWin_Text( this, IDC_EDIT_SEL_NAME, strNAME.GetString() );
	}else{
		SelectionReset();
	}
}
