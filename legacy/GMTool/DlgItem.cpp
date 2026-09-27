// DlgItem.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgItem.h"

#include "EtcFunction.h"
#include "GLItemMan.h"

#include "Logic/GMToolGlobal.h"

// CDlgItem dialog

IMPLEMENT_DYNAMIC(CDlgItem, CDialog)

CDlgItem::CDlgItem(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgItem::IDD, pParent)
	, m_SELECTED( NATIVEID_NULL() )
{
	m_vecData.clear();
	m_vecResult.clear();
}

CDlgItem::~CDlgItem()
{
}

void CDlgItem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SELECT, m_listSELECT );
}


BEGIN_MESSAGE_MAP(CDlgItem, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnBnClickedButtonSearch)
	ON_LBN_SELCHANGE(IDC_LIST_SELECT, OnLbnSelchangeListSelect)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgItem message handlers
BOOL CDlgItem::OnInitDialog()
{
	CDialog::OnInitDialog();

	ReadData();
	SelectionReset();

	SetWin_Combo_Init( this, IDC_COMBO_SEARCH, GMTOOL_GLOBAL::strItemDataSearchType, EMITEMDATA_SEARCH_SIZE );
	SetWin_Text( this, IDC_EDIT_SEARCH, "" );

	SearchData();

	return TRUE;
}

void CDlgItem::ReadData()
{
	m_vecData.clear();
	for ( int MID = 0; MID < GLItemMan::MAX_MID; MID++ )
	{
		for ( int SID = 0; SID < GLItemMan::MAX_SID; SID++ )
		{
			SITEM* pItem = GLItemMan::GetInstance().GetItem ( MID, SID );
			if ( !pItem )	continue;

			m_vecData.push_back( pItem->sBasicOp.sNativeID );
		}
	}
}

void CDlgItem::ShowData()
{
	m_listSELECT.ResetContent();

	for( int i = 0; i < (int) m_vecResult.size(); ++ i )
	{
		SNATIVEID sID = m_vecResult[i];
		SITEM* pItem = GLItemMan::GetInstance().GetItem ( sID );
		if ( !pItem )	continue;

		CString strNAME;
		strNAME.Format ( "[%03d/%03d] %s", pItem->sBasicOp.sNativeID.wMainID, pItem->sBasicOp.sNativeID.wSubID, pItem->GetName() );

		int nIndex = m_listSELECT.AddString ( strNAME );
		m_listSELECT.SetItemData ( nIndex, pItem->sBasicOp.sNativeID.dwID );
	}
}

void CDlgItem::SelectionReset()
{
	m_SELECTED = NATIVEID_NULL();
	SetWin_Text( this, IDC_EDIT_SEL_NAME, "" );
}

void CDlgItem::OnBnClickedButtonSearch()
{
	SearchData();
}

void CDlgItem::OnLbnSelchangeListSelect()
{
	int nSel = m_listSELECT.GetCurSel();
	if ( nSel < 0 ){
		return;
	}

	DWORD dwData = (DWORD) m_listSELECT.GetItemData ( nSel );
	SNATIVEID sID;
	sID.dwID = dwData;

	SITEM* pITEM = GLItemMan::GetInstance().GetItem( sID );
	if ( pITEM )
	{
		m_SELECTED = pITEM->sBasicOp.sNativeID;
		CString strNAME;
		strNAME.Format ( "[%03d/%03d] %s", pITEM->sBasicOp.sNativeID.wMainID, pITEM->sBasicOp.sNativeID.wSubID, pITEM->GetName() );
		SetWin_Text( this, IDC_EDIT_SEL_NAME, strNAME.GetString() );
	}
	else
	{
		SelectionReset();
	}
}

void CDlgItem::SearchData()
{
	EMITEMDATA_SEARCH emSearch =  static_cast<EMITEMDATA_SEARCH> ( GetWin_Combo_Sel( this, IDC_COMBO_SEARCH ) );
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
			SITEM* pItem = GLItemMan::GetInstance().GetItem ( sID );
			if ( !pItem )	continue;

			CString strFIND;
			switch( emSearch )
			{
			case EMITEMDATA_SEARCH_NAME:
				{
					strFIND.Format( "%s", pItem->GetName() );
				}break;
			case EMITEMDATA_SEARCH_MID:
				{
					strFIND.Format( "%d", pItem->sBasicOp.sNativeID.wMainID );
				}break;
			case EMITEMDATA_SEARCH_SID:
				{
					strFIND.Format( "%d", pItem->sBasicOp.sNativeID.wSubID );
				}break;
			case EMITEMDATA_SEARCH_TYPE:
				{
					strFIND.Format( "%d", pItem->sBasicOp.emItemType );
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

void CDlgItem::OnBnClickedOk()
{
	CDialog::OnOK();
}

void CDlgItem::OnBnClickedCancel()
{
	CDialog::OnCancel();
}
