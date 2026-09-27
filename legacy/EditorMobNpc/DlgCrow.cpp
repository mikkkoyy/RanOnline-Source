// DlgCrow.cpp : implementation file
//

#include "stdafx.h"
#include "EditorMobNpc.h"
#include "DlgCrow.h"
#include "EtcFunction.h"

// CDlgCrow dialog

IMPLEMENT_DYNAMIC(CDlgCrow, CDialog)

CDlgCrow::CDlgCrow(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCrow::IDD, pParent)
	, m_SELECTED( NATIVEID_NULL() )
{
	m_vecData.clear();
	m_vecResult.clear();
}

CDlgCrow::~CDlgCrow()
{
}

void CDlgCrow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SELECT, m_listSELECT );
}


BEGIN_MESSAGE_MAP(CDlgCrow, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_SELECT, OnLbnSelchangeListSelect)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnBnClickedButtonSearch)
END_MESSAGE_MAP()


// CDlgCrow message handlers
BOOL CDlgCrow::OnInitDialog()
{
	CDialog::OnInitDialog();

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

void CDlgCrow::ReadData()
{
	m_vecData.clear();
	for ( int MID = 0; MID < GLCrowDataMan::MAX_CROW_MID; MID++ )
	{
		for ( int SID = 0; SID < GLCrowDataMan::MAX_CROW_SID; SID++ )
		{        	
			PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( MID, SID );
			if ( pCROW )
			{	
				m_vecData.push_back( pCROW->m_sBasic.sNativeID );
			}
		}
	}
}

void CDlgCrow::ShowData()
{
	m_listSELECT.ResetContent();
	m_listSELECT.SetRedraw( FALSE );

	for( int i = 0; i < (int) m_vecResult.size(); ++ i )
	{
		SNATIVEID sID = m_vecResult[i];
		PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( sID );
		if ( pCROW )
		{
			CString strNAME;
			strNAME.Format ( "[%03d/%03d] %s", sID.wMainID, sID.wSubID, pCROW->GetName() );

			int nIndex = m_listSELECT.AddString ( strNAME );
			m_listSELECT.SetItemData ( nIndex, pCROW->m_sBasic.sNativeID.dwID );
		}
	}

	m_listSELECT.SetRedraw( TRUE );
}

void CDlgCrow::SelectionReset()
{
	m_SELECTED = NATIVEID_NULL();
	SetWin_Num_int( this, IDC_EDIT_SEL_MID, m_SELECTED.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SEL_SID, m_SELECTED.wSubID );
	SetWin_Text( this, IDC_EDIT_SEL_NAME, "" );
}

void CDlgCrow::OnLbnSelchangeListSelect()
{
	int nSel = m_listSELECT.GetCurSel();

	if ( nSel < 0 ) return;

	DWORD dwData = (DWORD) m_listSELECT.GetItemData ( nSel );
	SNATIVEID sID;
	sID.dwID = dwData;

	PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( sID );
	if ( pCROW )
	{
		m_SELECTED = pCROW->m_sBasic.sNativeID;
		SetWin_Num_int( this, IDC_EDIT_SEL_MID, m_SELECTED.wMainID );
		SetWin_Num_int( this, IDC_EDIT_SEL_SID, m_SELECTED.wSubID );
		SetWin_Text( this, IDC_EDIT_SEL_NAME, pCROW->GetName() );
	}
	else
	{
		SelectionReset();
	}
}

void CDlgCrow::OnBnClickedButtonSearch()
{
	SearchData();
}

void CDlgCrow::SearchData()
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
			PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( sID );
			if ( !pCROW )	continue;

			CString strFIND;
			switch( nSEARCH )
			{
			case 0:
				{
					strFIND.Format( "%s", pCROW->GetName() );
				}break;
			case 1:
				{
					strFIND.Format( "%d", pCROW->m_sBasic.sNativeID.wMainID );
				}break;
			case 2:
				{
					strFIND.Format( "%d", pCROW->m_sBasic.sNativeID.wSubID );
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

