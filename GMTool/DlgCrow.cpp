// DlgCrow.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgCrow.h"

#include "EtcFunction.h"
#include "GLCrowData.h"

#include "Logic/GMToolGlobal.h"

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
	ON_BN_CLICKED(IDC_BUTTON_HELPER_SEARCH, OnBnClickedButtonHelperSearch)
	ON_LBN_SELCHANGE(IDC_LIST_SELECT, OnLbnSelchangeListSelect)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgCrow message handlers
BOOL CDlgCrow::OnInitDialog()
{
	CDialog::OnInitDialog();

	ReadData();
	SelectionReset();

	SetWin_Combo_Init( this, IDC_COMBO_CROW_SEARCH, GMTOOL_GLOBAL::strCrowDataSearchType, EMCROWDATA_SEARCH_SIZE );
	SetWin_Text( this, IDC_EDIT_CROW_SEARCH, "" );

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
			SCROWDATA* pCrow = GLCrowDataMan::GetInstance().GetCrowData ( MID, SID );
			if ( !pCrow )	continue;

			m_vecData.push_back( pCrow->m_sBasic.sNativeID );
		}
	}
}

void CDlgCrow::ShowData()
{
	m_listSELECT.ResetContent();

	for( int i = 0; i < (int) m_vecResult.size(); ++ i )
	{
		SNATIVEID sID = m_vecResult[i];
		SCROWDATA* pCrow = GLCrowDataMan::GetInstance().GetCrowData( sID );
		if ( !pCrow )	continue;

		CString strNAME;
		strNAME.Format ( "[%03d/%03d] %s", pCrow->m_sBasic.sNativeID.wMainID, pCrow->m_sBasic.sNativeID.wSubID, pCrow->GetName() );

		int nIndex = m_listSELECT.AddString ( strNAME );
		m_listSELECT.SetItemData ( nIndex, pCrow->m_sBasic.sNativeID.dwID );
	}
}

void CDlgCrow::SelectionReset()
{
	m_SELECTED = NATIVEID_NULL();
	SetWin_Text( this, IDC_EDIT_SEL_NAME, "" );
}

void CDlgCrow::OnBnClickedButtonHelperSearch()
{
	SearchData();
}

void CDlgCrow::OnLbnSelchangeListSelect()
{
	int nSel = m_listSELECT.GetCurSel();
	if ( nSel < 0 ){
		return;
	}

	DWORD dwData = (DWORD) m_listSELECT.GetItemData ( nSel );
	SNATIVEID sID;
	sID.dwID = dwData;

	SCROWDATA* pCrow = GLCrowDataMan::GetInstance().GetCrowData( sID );
	if ( pCrow )
	{
		m_SELECTED = pCrow->m_sBasic.sNativeID;
		CString strNAME;
		strNAME.Format ( "[%03d/%03d] %s", pCrow->m_sBasic.sNativeID.wMainID, pCrow->m_sBasic.sNativeID.wSubID, pCrow->GetName() );
		SetWin_Text( this, IDC_EDIT_SEL_NAME, strNAME.GetString() );
	}
	else
	{
		SelectionReset();
	}
}

void CDlgCrow::SearchData()
{
	EMCROWDATA_SEARCH emSearch =  static_cast<EMCROWDATA_SEARCH> ( GetWin_Combo_Sel( this, IDC_COMBO_CROW_SEARCH ) );
	std::string strSEARCH = GetWin_Text( this, IDC_EDIT_CROW_SEARCH );

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
			SCROWDATA* pCrow = GLCrowDataMan::GetInstance().GetCrowData( sID );
			if ( !pCrow )	continue;

			CString strFIND;
			switch( emSearch )
			{
			case EMCROWDATA_SEARCH_NAME:
				{
					strFIND.Format( "%s", pCrow->GetName() );
				}break;
			case EMCROWDATA_SEARCH_MID:
				{
					strFIND.Format( "%d", pCrow->m_sBasic.sNativeID.wMainID );
				}break;
			case EMCROWDATA_SEARCH_SID:
				{
					strFIND.Format( "%d", pCrow->m_sBasic.sNativeID.wSubID );
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

void CDlgCrow::OnBnClickedOk()
{
	CDialog::OnOK();
}

void CDlgCrow::OnBnClickedCancel()
{
	CDialog::OnCancel();
}
