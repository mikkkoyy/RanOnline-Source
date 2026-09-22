// DlgSkill.cpp : implementation file
//

#include "stdafx.h"
#include "EditorMobNpc.h"
#include "DlgSkill.h"
#include "EtcFunction.h"
#include <algorithm>

// CDlgSkill dialog

IMPLEMENT_DYNAMIC(CDlgSkill, CDialog)

CDlgSkill::CDlgSkill(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSkill::IDD, pParent)
	, m_SELECTED( NATIVEID_NULL() )
{
	m_vecData.clear();
	m_vecResult.clear();
}

CDlgSkill::~CDlgSkill()
{
}

void CDlgSkill::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SELECT, m_listSELECT );
}


BEGIN_MESSAGE_MAP(CDlgSkill, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_SELECT, OnLbnSelchangeListSelect)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnBnClickedButtonSearch)
END_MESSAGE_MAP()


// CDlgSkill message handlers
BOOL CDlgSkill::OnInitDialog()
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

void CDlgSkill::ReadData()
{
	m_vecData.clear();
	for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ )
	{
		for ( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; SID++ )
		{        		
			PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
			if ( pSKILL )
			{
				m_vecData.push_back( pSKILL->m_sBASIC.sNATIVEID );
			}
		}
	}
}

void CDlgSkill::ShowSkills()
{
	m_listSELECT.ResetContent();
	m_listSELECT.SetRedraw( FALSE );

	for( int i = 0; i < (int) m_vecResult.size(); ++ i )
	{
		SNATIVEID sID = m_vecResult[i];
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sID );
		if ( !pSKILL )	continue;

		CString strNAME;
		strNAME.Format ( "[%03d/%03d] %s", sID.wMainID, sID.wSubID, pSKILL->GetName() );

		int nIndex = m_listSELECT.AddString ( strNAME );
		m_listSELECT.SetItemData ( nIndex, pSKILL->m_sBASIC.sNATIVEID.dwID );	
	}

	m_listSELECT.SetRedraw( TRUE );
}

void CDlgSkill::OnLbnSelchangeListSelect()
{
	int nSel = m_listSELECT.GetCurSel();

	if ( nSel < 0 ) return;

	DWORD dwData = (DWORD) m_listSELECT.GetItemData ( nSel );
	SNATIVEID sID;
	sID.dwID = dwData;
	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sID );
	if ( pSKILL )
	{
		m_SELECTED = pSKILL->m_sBASIC.sNATIVEID;
		SetWin_Num_int( this, IDC_EDIT_SEL_MID, m_SELECTED.wMainID );
		SetWin_Num_int( this, IDC_EDIT_SEL_SID, m_SELECTED.wSubID );
		SetWin_Text( this, IDC_EDIT_SEL_NAME, pSKILL->GetName() );
	}
	else
	{
		SelectionReset();
	}
}

void CDlgSkill::SelectionReset()
{
	m_SELECTED = NATIVEID_NULL();
	SetWin_Num_int( this, IDC_EDIT_SEL_MID, m_SELECTED.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SEL_SID, m_SELECTED.wSubID );
	SetWin_Text( this, IDC_EDIT_SEL_NAME, "" );
}

void CDlgSkill::OnBnClickedButtonSearch()
{
	SearchData();
}

void CDlgSkill::SearchData()
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
			PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sID );
			if ( !pSKILL )	continue;

			CString strFIND;
			switch( nSEARCH )
			{
			case 0:
				{
					strFIND.Format( "%s", pSKILL->GetName() );
				}break;
			case 1:
				{
					strFIND.Format( "%d", pSKILL->m_sBASIC.sNATIVEID.wMainID );
				}break;
			case 2:
				{
					strFIND.Format( "%d", pSKILL->m_sBASIC.sNATIVEID.wSubID );
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

	ShowSkills();
}