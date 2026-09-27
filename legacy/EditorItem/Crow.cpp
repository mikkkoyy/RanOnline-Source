// Crow.cpp : implementation file
//

#include "stdafx.h"
#include "EditorItem.h"
#include "Crow.h"
#include "EtcFunction.h"

// CCrow dialog

IMPLEMENT_DYNAMIC(CCrow, CDialog)

CCrow::CCrow(CWnd* pParent /*=NULL*/)
	: CDialog(CCrow::IDD, pParent)
{
	m_vecData.clear();
	m_vecResult.clear();
}

CCrow::~CCrow()
{
}

void CCrow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SELECT, m_listSELECT );
	DDX_Control(pDX, IDC_LIST_SELECTED, m_listSELECTED );
}


BEGIN_MESSAGE_MAP(CCrow, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_SELECT, OnLbnSelchangeListSelect)
	ON_BN_CLICKED(IDC_BUTTON_INSERT, OnBnClickedButtonInsert)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnBnClickedButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnBnClickedButtonSearch)
END_MESSAGE_MAP()


// CCrow message handlers
BOOL CCrow::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_vecSELECTED.clear();

	ReadData();
	ShowSelected();

	ResetSelection();

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

void CCrow::ReadData()
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

void CCrow::ShowSelections()
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

void CCrow::ShowSelected()
{
	m_listSELECTED.ResetContent();
	m_listSELECTED.SetRedraw( FALSE );

	int nTOTAL = (int)m_vecSELECTED.size();
	for ( int i = 0; i < nTOTAL; ++i )
	{
		ITEM::SPETSKINPACKITEMDATA sDATA = m_vecSELECTED[i];

		PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( sDATA.sMobID );
		if ( !pCrowData )	continue;

		CString strNAME;
		strNAME.Format ( "[%03d/%03d] %s [Scale: %.2f%%][Rate: %.2f%%]", pCrowData->m_sBasic.sNativeID.wMainID, pCrowData->m_sBasic.sNativeID.wSubID, pCrowData->GetName(), sDATA.fScale, sDATA.fRate );

		int nIndex = m_listSELECTED.AddString ( strNAME );
		m_listSELECTED.SetItemData ( nIndex, pCrowData->m_sBasic.sNativeID.dwID );
	}

	m_listSELECTED.SetRedraw( TRUE );
}

void CCrow::ResetSelection()
{
	SetWin_Num_int( this, IDC_EDIT_SEL_MID, 65535 );
	SetWin_Num_int( this, IDC_EDIT_SEL_SID, 65535 );
	SetWin_Text( this, IDC_EDIT_SEL_NAME, "" );
	SetWin_Num_float( this, IDC_EDIT_SEL_RATE, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_SEL_SCALE, 1.0f );
}

void CCrow::OnLbnSelchangeListSelect()
{
	int nSel = m_listSELECT.GetCurSel();

	if ( nSel < 0 ) return;

	DWORD dwData = (DWORD) m_listSELECT.GetItemData ( nSel );
	PCROWDATA pCrowData =  GLCrowDataMan::GetInstance().GetCrowData ( dwData );
	if ( pCrowData )
	{
		SetWin_Num_int( this, IDC_EDIT_SEL_MID, pCrowData->m_sBasic.sNativeID.wMainID );
		SetWin_Num_int( this, IDC_EDIT_SEL_SID, pCrowData->m_sBasic.sNativeID.wSubID );
		SetWin_Text( this, IDC_EDIT_SEL_NAME, pCrowData->GetName() );
	}
	else
	{
		ResetSelection();
	}
}

void CCrow::OnBnClickedButtonInsert()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_SEL_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_SEL_SID );
	float fRATE = GetWin_Num_float( this, IDC_EDIT_SEL_RATE );
	float fSCALE = GetWin_Num_float( this, IDC_EDIT_SEL_SCALE );

	PCROWDATA pCrowData =  GLCrowDataMan::GetInstance().GetCrowData ( wMID, wSID );
	if ( pCrowData )
	{
		ITEM::SPETSKINPACKITEMDATA sDATA;
		sDATA.sMobID.wMainID = wMID;
		sDATA.sMobID.wSubID = wSID;
		sDATA.fRate = fRATE;
		sDATA.fScale = fSCALE;

		m_vecSELECTED.push_back( sDATA );

		ShowSelected();
	}
}

void CCrow::OnBnClickedButtonRemove()
{
	int nSel = m_listSELECTED.GetCurSel();

	if ( nSel < 0 ) return;
	if ( nSel >= (int)m_vecSELECTED.size() )	return;
	DWORD dwData = (DWORD) m_listSELECTED.GetItemData ( nSel );

	VEC_ITER iter = m_vecSELECTED.begin();
	for( int nITER = 0; iter!=m_vecSELECTED.end(); ++iter, ++nITER )
	{
		if( nITER==nSel && (*iter).sMobID.dwID == dwData )
		{
			m_vecSELECTED.erase( iter );
			ShowSelected();
			break;
		}
	}
}

void CCrow::OnBnClickedButtonSearch()
{
	SearchData();
}

void CCrow::SearchData()
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

	ShowSelections();
}