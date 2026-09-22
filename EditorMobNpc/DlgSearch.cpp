// DlgSearch.cpp : implementation file
//

#include "stdafx.h"
#include "EditorMobNpc.h"
#include "DlgSearch.h"
#include "EtcFunction.h"
#include "GLCrowData.h"
#include "HLibDataConvert.h"

// CDlgSearch dialog

IMPLEMENT_DYNAMIC(CDlgSearch, CDialog)

CDlgSearch::CDlgSearch(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSearch::IDD, pParent)
	, m_sID( NATIVEID_NULL() )
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
	ON_CBN_SELCHANGE(IDC_COMBO_SEARCHTYPE, OnCbnSelchangeComboSearchtype)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnBnClickedButtonSearch)
	ON_BN_CLICKED(IDC_BUTTON_GO, OnBnClickedButtonGo)
END_MESSAGE_MAP()


// CDlgSearch message handlers
BOOL CDlgSearch::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.InsertColumn( 0, "ID", LVCFMT_LEFT, 60 );
	m_List.InsertColumn( 1, "MID", LVCFMT_LEFT, 60 );
	m_List.InsertColumn( 2, "SID", LVCFMT_LEFT, 60 );
	m_List.InsertColumn( 3, "Name", LVCFMT_LEFT, 250 );
	m_List.InsertColumn( 4, "Search", LVCFMT_LEFT,150 );

	std::string strSEARCH[EMSEARCH_TOTAL] = 
	{
		"Search Name",
		"Search MID",
		"Search SID",
		"Search CHF",
		"Search CrowSale File",
		"Search TalkFile",
		"Search CrowType",
		"Search GenItem",
		"Search GenItemQ",
	};

	SetWin_Combo_Init( this, IDC_COMBO_SEARCHTYPE, strSEARCH, EMSEARCH_TOTAL );

	m_sID = NATIVEID_NULL();

	ShowResults();
	SearchInit();

	return TRUE;  // return TRUE  unless you set the focus to a control
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
			m_List.SetItemText( i, 1, _HLIB::cstring_dword(sRESULT.sID.wMainID).GetString() );
			m_List.SetItemText( i, 2, _HLIB::cstring_dword(sRESULT.sID.wSubID).GetString() );
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

	SetWin_Enable( this, IDC_EDIT_SEARCH, FALSE );
	SetWin_Enable( this, IDC_COMBO_TYPE, FALSE );

	SetWin_Text( this, IDC_EDIT_SEARCH, "" );
	SetWin_Text( this, IDC_STATIC_SEARCH_1, "" );
	SetWin_Text( this, IDC_STATIC_SEARCH_3, "" );

	std::string str[1] = {""};
	SetWin_Combo_Init( this, IDC_COMBO_TYPE, str, 1 );

	if ( emSearch == EMSEARCH_MID || emSearch == EMSEARCH_SID || emSearch == EMSEARCH_NAME || emSearch == EMSEARCH_CHF || emSearch == EMSEARCH_SALE || emSearch == EMSEARCH_TALK || 
		emSearch == EMSEARCH_GENITEM || emSearch == EMSEARCH_GENITEMQ )
	{
		SetWin_Enable( this, IDC_EDIT_SEARCH, TRUE );
		SetWin_Text( this, IDC_STATIC_SEARCH_1, strTEXT.c_str() );
	}
	else if ( emSearch == EMSEARCH_CROWTYPE )
	{
		SetWin_Enable( this, IDC_COMBO_TYPE, TRUE );
		SetWin_Text( this, IDC_STATIC_SEARCH_3, strTEXT.c_str() );
	}

	switch ( emSearch ) 
	{
	case EMSEARCH_CROWTYPE:
		{
			SetWin_Combo_Init( this, IDC_COMBO_TYPE, COMMENT::CROW, CROW_NUM );
		}break;
	};
}

void CDlgSearch::OnCbnSelchangeComboSearchtype()
{
	SearchInit();
}

void CDlgSearch::OnBnClickedButtonSearch()
{
	int nSELECT = GetWin_Combo_Sel( this, IDC_COMBO_SEARCHTYPE );
	int nSEARCH = GetWin_Num_int( this, IDC_EDIT_SEARCH );
	std::string strSEARCH = GetWin_Text( this, IDC_EDIT_SEARCH );
	int nTYPE = GetWin_Combo_Sel( this, IDC_COMBO_TYPE );

	if ( nSELECT < 0 || nSELECT >= EMSEARCH_TOTAL )	return;

	m_List.DeleteAllItems();
	m_sID = NATIVEID_NULL();

	std::vector<SSEARCH> vec;

	switch ( nSELECT )
	{
	case EMSEARCH_NAME:
		{
			//search by name
			for ( int MID = 0; MID < GLCrowDataMan::MAX_CROW_MID; MID++ )
			{
				for ( int SID = 0; SID < GLCrowDataMan::MAX_CROW_MID; SID++ )
				{
					PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData ( MID, SID );
					if ( pCROW )
					{
						std::string strSearch = strSEARCH;
						std::string strFindString = pCROW->GetName();

						if ( strFindString.size() > 0 )
						{
							std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
							std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

							unsigned found = strFindString.find(strSearch.c_str());
							if (found!=std::string::npos)
							{
								SSEARCH sResult;
								sResult.sID = pCROW->m_sBasic.sNativeID;
								sResult.strNAME = pCROW->GetName();
								sResult.strFIND = strFindString.c_str();
								vec.push_back( sResult );
							}
						}
					}
				}
			}
		}break;

	case EMSEARCH_MID:
		{
			//search by main id
			for ( int MID = 0; MID < GLCrowDataMan::MAX_CROW_MID; MID++ )
			{
				for ( int SID = 0; SID < GLCrowDataMan::MAX_CROW_MID; SID++ )
				{
					PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData ( MID, SID );
					if ( pCROW )
					{
						if ( pCROW->m_sBasic.sNativeID.wMainID == nSEARCH  )
						{
							SSEARCH sResult;
							sResult.sID = pCROW->m_sBasic.sNativeID;
							sResult.strNAME = pCROW->GetName();
							sResult.strFIND = strSEARCH.c_str();
							vec.push_back( sResult );
						}
					}
				}
			}
		}break;

	case EMSEARCH_SID:
		{
			//search by sub id
			for ( int MID = 0; MID < GLCrowDataMan::MAX_CROW_MID; MID++ )
			{
				for ( int SID = 0; SID < GLCrowDataMan::MAX_CROW_MID; SID++ )
				{
					PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData ( MID, SID );
					if ( pCROW )
					{
						if ( pCROW->m_sBasic.sNativeID.wSubID == nSEARCH  )
						{
							SSEARCH sResult;
							sResult.sID = pCROW->m_sBasic.sNativeID;
							sResult.strNAME = pCROW->GetName();
							sResult.strFIND = strSEARCH.c_str();
							vec.push_back( sResult );
						}
					}
				}
			}
		}break;
	
	case EMSEARCH_CHF:
		{
			//search by chf
			for ( int MID = 0; MID < GLCrowDataMan::MAX_CROW_MID; MID++ )
			{
				for ( int SID = 0; SID < GLCrowDataMan::MAX_CROW_MID; SID++ )
				{
					PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData ( MID, SID );
					if ( pCROW )
					{
						std::string strSearch = strSEARCH;
						std::string strFindString = pCROW->GetSkinObjFile();

						if ( strFindString.size() > 0 )
						{
							std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
							std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

							unsigned found = strFindString.find(strSearch.c_str());
							if (found!=std::string::npos)
							{
								SSEARCH sResult;
								sResult.sID = pCROW->m_sBasic.sNativeID;
								sResult.strNAME = pCROW->GetName();
								sResult.strFIND = strFindString.c_str();
								vec.push_back( sResult );
							}
						}
					}
				}
			}
		}break;
	case EMSEARCH_SALE:
		{
			//search by crowsale file
			for ( int MID = 0; MID < GLCrowDataMan::MAX_CROW_MID; MID++ )
			{
				for ( int SID = 0; SID < GLCrowDataMan::MAX_CROW_MID; SID++ )
				{
					PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData ( MID, SID );
					if ( pCROW )
					{
						for ( DWORD i = 0; i < pCROW->GetSaleNum(); ++ i )
						{
							std::string strSearch = strSEARCH;
							std::string strFindString = pCROW->m_sAction.m_strSaleFile[i].c_str();

							if ( strFindString.size() > 0 )
							{
								std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
								std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

								unsigned found = strFindString.find(strSearch.c_str());
								if (found!=std::string::npos)
								{
									SSEARCH sResult;
									sResult.sID = pCROW->m_sBasic.sNativeID;
									sResult.strNAME = pCROW->GetName();
									sResult.strFIND = strFindString.c_str();
									vec.push_back( sResult );
									break;
								}
							}
						}
					}
				}
			}
		}break;

	case EMSEARCH_TALK:
		{
			//search by talkfile
			for ( int MID = 0; MID < GLCrowDataMan::MAX_CROW_MID; MID++ )
			{
				for ( int SID = 0; SID < GLCrowDataMan::MAX_CROW_MID; SID++ )
				{
					PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData ( MID, SID );
					if ( pCROW )
					{
						std::string strSearch = strSEARCH;
						std::string strFindString = pCROW->GetTalkFile();

						if ( strFindString.size() > 0 )
						{
							std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
							std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

							unsigned found = strFindString.find(strSearch.c_str());
							if (found!=std::string::npos)
							{
								SSEARCH sResult;
								sResult.sID = pCROW->m_sBasic.sNativeID;
								sResult.strNAME = pCROW->GetName();
								sResult.strFIND = strFindString.c_str();
								vec.push_back( sResult );
							}
						}
					}
				}
			}
		}break;
	case EMSEARCH_CROWTYPE:
		{	
			//search by crow type
			for ( int MID = 0; MID < GLCrowDataMan::MAX_CROW_MID; MID++ )
			{
				for ( int SID = 0; SID < GLCrowDataMan::MAX_CROW_MID; SID++ )
				{
					PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData ( MID, SID );
					if ( pCROW )
					{
						EMCROW emCROW = static_cast <EMCROW> ( nTYPE );
						if ( pCROW->m_sBasic.m_emCrow == emCROW  )
						{
							SSEARCH sResult;
							sResult.sID = pCROW->m_sBasic.sNativeID;
							sResult.strNAME = pCROW->GetName();
							sResult.strFIND = COMMENT::CROW[emCROW].c_str();
							vec.push_back( sResult );
						}
					}
				}
			}
		}break;

	case EMSEARCH_GENITEM:
		{
			//search by genitem
			for ( int MID = 0; MID < GLCrowDataMan::MAX_CROW_MID; MID++ )
			{
				for ( int SID = 0; SID < GLCrowDataMan::MAX_CROW_MID; SID++ )
				{
					PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData ( MID, SID );
					if ( pCROW )
					{
						std::string strSearch = strSEARCH;
						std::string strFindString = pCROW->m_sGenerate.m_strGenItem;

						if ( strFindString.size() > 0 )
						{
							std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
							std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

							unsigned found = strFindString.find(strSearch.c_str());
							if (found!=std::string::npos)
							{
								SSEARCH sResult;
								sResult.sID = pCROW->m_sBasic.sNativeID;
								sResult.strNAME = pCROW->GetName();
								sResult.strFIND = strFindString.c_str();
								vec.push_back( sResult );
							}
						}
					}
				}
			}
		}break;

	case EMSEARCH_GENITEMQ:
		{
			//search by genitem
			for ( int MID = 0; MID < GLCrowDataMan::MAX_CROW_MID; MID++ )
			{
				for ( int SID = 0; SID < GLCrowDataMan::MAX_CROW_MID; SID++ )
				{
					PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData ( MID, SID );
					if ( pCROW )
					{
						std::string strSearch = strSEARCH;
						std::string strFindString = pCROW->m_sGenerate.m_strQtGenItem;

						if ( strFindString.size() > 0 )
						{
							std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
							std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

							unsigned found = strFindString.find(strSearch.c_str());
							if (found!=std::string::npos)
							{
								SSEARCH sResult;
								sResult.sID = pCROW->m_sBasic.sNativeID;
								sResult.strNAME = pCROW->GetName();
								sResult.strFIND = strFindString.c_str();
								vec.push_back( sResult );
							}
						}
					}
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

	m_sID.wMainID = (WORD)atoi( m_List.GetItemText(nSelect,1).GetString() );
	m_sID.wSubID = (WORD)atoi( m_List.GetItemText(nSelect,2).GetString() );
	OnOK();
}
