// Search.cpp : implementation file
//

#include "stdafx.h"
#include "EditorItem.h"
#include "Search.h"
#include "EtcFunction.h"
#include "GLItemMan.h"
#include "HLibDataConvert.h"

// CSearch dialog

IMPLEMENT_DYNAMIC(CSearch, CDialog)

CSearch::CSearch(CWnd* pParent /*=NULL*/)
	: CDialog(CSearch::IDD, pParent)
	, m_sID( NATIVEID_NULL() )
	, m_vec( NULL )
{

}

CSearch::~CSearch()
{
	m_vec = NULL;
}

void CSearch::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_RESULT, m_List);
}


BEGIN_MESSAGE_MAP(CSearch, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GO, OnBnClickedButtonGo)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnBnClickedButtonSearch)
	ON_CBN_SELCHANGE(IDC_COMBO_SEARCHTYPE, OnCbnSelchangeComboSearchtype)
END_MESSAGE_MAP()


// CSearch message handlers
BOOL CSearch::OnInitDialog()
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
		"Search Wear",
		"Search Icon DDS",
		"Search Icon DDS and Coordinates",
		"Search By Item Type",
		"Search GenID(SpecID)",
		"Search Random Option File",
	};

	SetWin_Combo_Init( this, IDC_COMBO_SEARCHTYPE, strSEARCH, EMSEARCH_TOTAL );
	
	m_sID = NATIVEID_NULL();

	ShowResults();
	SearchInit();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSearch::OnBnClickedButtonGo()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	m_sID.wMainID = (WORD)atoi( m_List.GetItemText(nSelect,1).GetString() );
	m_sID.wSubID = (WORD)atoi( m_List.GetItemText(nSelect,2).GetString() );
	OnOK();
}

void CSearch::OnBnClickedButtonSearch()
{
	int nSELECT = GetWin_Combo_Sel( this, IDC_COMBO_SEARCHTYPE );
	int nSEARCH = GetWin_Num_int( this, IDC_EDIT_SEARCH );
	std::string strSEARCH = GetWin_Text( this, IDC_EDIT_SEARCH );
	int nX = GetWin_Num_int( this, IDC_EDIT_X );
	int nY = GetWin_Num_int( this, IDC_EDIT_Y );
	EMITEM_TYPE emTYPE = (EMITEM_TYPE) GetWin_Combo_Sel( this, IDC_COMBO_ITEM_TYPE );

	if ( nSELECT < 0 || nSELECT >= EMSEARCH_TOTAL )	return;

	m_List.DeleteAllItems();
	m_sID = NATIVEID_NULL();

	std::vector<SSEARCH> vec;

	switch ( nSELECT )
	{
	case EMSEARCH_NAME:
		{
			//search by name
			for ( int MID = 0; MID < GLItemMan::MAX_MID; MID++ )
			{
				for ( int SID = 0; SID < GLItemMan::MAX_SID; SID++ )
				{
					SITEM* pItem = GLItemMan::GetInstance().GetItem ( MID, SID );
					if ( pItem )
					{
						std::string strSearch = strSEARCH;
						std::string strFindString = pItem->GetName();

						if ( strFindString.size() > 0 )
						{
							std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
							std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

							unsigned found = strFindString.find(strSearch.c_str());
							if (found!=std::string::npos)
							{
								SSEARCH sResult;
								sResult.sID = pItem->sBasicOp.sNativeID;
								sResult.strNAME = pItem->GetName();
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
			for ( int MID = 0; MID < GLItemMan::MAX_MID; MID++ )
			{
				for ( int SID = 0; SID < GLItemMan::MAX_SID; SID++ )
				{
					SITEM* pItem = GLItemMan::GetInstance().GetItem ( MID, SID );
					if ( pItem )
					{
						if ( pItem->sBasicOp.sNativeID.wMainID == nSEARCH  )
						{
							SSEARCH sResult;
							sResult.sID = pItem->sBasicOp.sNativeID;
							sResult.strNAME = pItem->GetName();
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
			for ( int MID = 0; MID < GLItemMan::MAX_MID; MID++ )
			{
				for ( int SID = 0; SID < GLItemMan::MAX_SID; SID++ )
				{
					SITEM* pItem = GLItemMan::GetInstance().GetItem ( MID, SID );
					if ( pItem )
					{
						if ( pItem->sBasicOp.sNativeID.wSubID == nSEARCH  )
						{
							SSEARCH sResult;
							sResult.sID = pItem->sBasicOp.sNativeID;
							sResult.strNAME = pItem->GetName();
							sResult.strFIND = strSEARCH.c_str();
							vec.push_back( sResult );
						}
					}
				}
			}
		}break;
	case EMSEARCH_WEAR:
		{
			//search by wear files
			for ( int MID = 0; MID < GLItemMan::MAX_MID; MID++ )
			{
				for ( int SID = 0; SID < GLItemMan::MAX_SID; SID++ )
				{
					SITEM* pItem = GLItemMan::GetInstance().GetItem ( MID, SID );
					if ( pItem )
					{
						for( int i = 0; i < GLCI_NUM_8CLASS; i++ )
						{
							std::string strSearch = strSEARCH;
							std::string strFindString =  pItem->sBasicOp.strWearingFileRight[i].c_str();

							if ( strFindString.size() > 0 )
							{
								std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
								std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

								unsigned found = strFindString.find(strSearch.c_str());
								if (found!=std::string::npos)
								{
									SSEARCH sResult;
									sResult.sID = pItem->sBasicOp.sNativeID;
									sResult.strNAME = pItem->GetName();
									sResult.strFIND = strFindString.c_str();
									vec.push_back( sResult );
									break;
								}
							}	
						}

						for( int i = 0; i < GLCI_NUM_8CLASS; i++ )
						{
							std::string strSearch = strSEARCH;
							std::string strFindString =  pItem->sBasicOp.strWearingFileLeft[i].c_str();

							if ( strFindString.size() > 0 )
							{
								std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
								std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

								unsigned found = strFindString.find(strSearch.c_str());
								if (found!=std::string::npos)
								{
									SSEARCH sResult;
									sResult.sID = pItem->sBasicOp.sNativeID;
									sResult.strNAME = pItem->GetName();
									sResult.strFIND = strFindString.c_str();
									vec.push_back( sResult );
									break;
								}
							}	
						}

						{
							std::string strSearch = strSEARCH;
							std::string strFindString =  pItem->GetPetWearingFile();

							if ( strFindString.size() > 0 )
							{
								std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
								std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

								unsigned found = strFindString.find(strSearch.c_str());
								if (found!=std::string::npos)
								{
									SSEARCH sResult;
									sResult.sID = pItem->sBasicOp.sNativeID;
									sResult.strNAME = pItem->GetName();
									sResult.strFIND = strFindString.c_str();
									vec.push_back( sResult );
								}
							}
						}
					}
				}
			}
		}break;
	case EMSEARCH_DDS:
		{
			//search by dds icon
			for ( int MID = 0; MID < GLItemMan::MAX_MID; MID++ )
			{
				for ( int SID = 0; SID < GLItemMan::MAX_SID; SID++ )
				{
					SITEM* pItem = GLItemMan::GetInstance().GetItem ( MID, SID );
					if ( pItem )
					{
						std::string strSearch = strSEARCH;
						std::string strFindString = pItem->sBasicOp.strInventoryFile.c_str();

						if ( strFindString.size() > 0 )
						{
							std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
							std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

							unsigned found = strFindString.find(strSearch.c_str());
							if (found!=std::string::npos)
							{
								SSEARCH sResult;
								sResult.sID = pItem->sBasicOp.sNativeID;
								sResult.strNAME = pItem->GetName();
								sResult.strFIND = strFindString.c_str();
								vec.push_back( sResult );
							}
						}
					}
				}
			}
		}break;
	case EMSEARCH_DDSXY:
		{
			//search by dds icon and icon pos
			for ( int MID = 0; MID < GLItemMan::MAX_MID; MID++ )
			{
				for ( int SID = 0; SID < GLItemMan::MAX_SID; SID++ )
				{
					SITEM* pItem = GLItemMan::GetInstance().GetItem ( MID, SID );
					if ( pItem )
					{
						std::string strSearch = strSEARCH;
						std::string strFindString = pItem->sBasicOp.strInventoryFile.c_str();

						if ( strFindString.size() > 0 )
						{
							std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
							std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

							unsigned found = strFindString.find(strSearch.c_str());
							if (found!=std::string::npos)
							{
								if ( pItem->sBasicOp.sICONID.wMainID == nX && pItem->sBasicOp.sICONID.wSubID == nY )
								{
									SSEARCH sResult;
									sResult.sID = pItem->sBasicOp.sNativeID;
									sResult.strNAME = pItem->GetName();
									sResult.strFIND = strFindString.c_str();
									vec.push_back( sResult );
								}
							}
						}
					}
				}
			}
		}break;
	case EMSEARCH_TYPE:
		{
			//search by sub id
			for ( int MID = 0; MID < GLItemMan::MAX_MID; MID++ )
			{
				for ( int SID = 0; SID < GLItemMan::MAX_SID; SID++ )
				{
					SITEM* pItem = GLItemMan::GetInstance().GetItem ( MID, SID );
					if ( pItem )
					{
						if ( pItem->sBasicOp.emItemType == emTYPE  )
						{
							SSEARCH sResult;
							sResult.sID = pItem->sBasicOp.sNativeID;
							sResult.strNAME = pItem->GetName();
							sResult.strFIND = COMMENT::ITEMTYPE[emTYPE].c_str();
							vec.push_back( sResult );
						}
					}
				}
			}
		}break;

	case EMSEARCH_GENID:
		{
			//search by genid
			for ( int MID = 0; MID < GLItemMan::MAX_MID; MID++ )
			{
				for ( int SID = 0; SID < GLItemMan::MAX_SID; SID++ )
				{
					SITEM* pItem = GLItemMan::GetInstance().GetItem ( MID, SID );
					if ( pItem )
					{
						if ( pItem->sGenerateOp.dwSpecID == (DWORD)nSEARCH  )
						{
							SSEARCH sResult;
							sResult.sID = pItem->sBasicOp.sNativeID;
							sResult.strNAME = pItem->GetName();
							sResult.strFIND = strSEARCH.c_str();
							vec.push_back( sResult );
						}
					}
				}
			}
		}break;

	case EMSEARCH_RANDOM_OPT:
		{
			//search by random option file
			for ( int MID = 0; MID < GLItemMan::MAX_MID; MID++ )
			{
				for ( int SID = 0; SID < GLItemMan::MAX_SID; SID++ )
				{
					SITEM* pItem = GLItemMan::GetInstance().GetItem ( MID, SID );
					if ( pItem )
					{
						std::string strSearch = strSEARCH;
						std::string strFindString = pItem->sRandomOpt.szNAME;

						if ( strFindString.size() > 0 )
						{
							std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
							std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

							unsigned found = strFindString.find(strSearch.c_str());
							if (found!=std::string::npos)
							{
								SSEARCH sResult;
								sResult.sID = pItem->sBasicOp.sNativeID;
								sResult.strNAME = pItem->GetName();
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

void CSearch::OnCbnSelchangeComboSearchtype()
{
	SearchInit();
}

void CSearch::ShowResults()
{
	if ( m_vec )
	{
		m_List.DeleteAllItems();
		m_List.SetRedraw( FALSE );

		for( size_t i=0; i<m_vec->size(); ++i)
		{
			SSEARCH sRESULT = m_vec->at(i);
			m_List.InsertItem( i, _HLIB::cstring_dword( i ).GetString(), NULL );
			m_List.SetItemText( i, 1, _HLIB::cstring_dword( sRESULT.sID.wMainID ).GetString() );
			m_List.SetItemText( i, 2, _HLIB::cstring_dword( sRESULT.sID.wSubID ).GetString() );
			m_List.SetItemText( i, 3, sRESULT.strNAME.c_str() );
			m_List.SetItemText( i, 4, sRESULT.strFIND.c_str() );
		}

		m_List.SetRedraw( TRUE );
	}
}

void CSearch::SearchInit()
{
	EMSEARCH emSearch = static_cast< EMSEARCH > ( GetWin_Combo_Sel( this, IDC_COMBO_SEARCHTYPE ));
	std::string strTEXT = GetWin_Combo_Sel_Text( this, IDC_COMBO_SEARCHTYPE ).GetString();

	SetWin_Enable( this, IDC_EDIT_SEARCH, FALSE );
	SetWin_Enable( this, IDC_EDIT_X, FALSE );
	SetWin_Enable( this, IDC_EDIT_Y, FALSE );
	SetWin_Enable( this, IDC_COMBO_ITEM_TYPE, FALSE );
	SetWin_Text( this, IDC_EDIT_SEARCH, "" );
	SetWin_Text( this, IDC_EDIT_X, "" );
	SetWin_Text( this, IDC_EDIT_Y, "" );
	SetWin_Text( this, IDC_STATIC_SEARCH_1, "" );
	SetWin_Text( this, IDC_STATIC_SEARCH_2, "" );
	SetWin_Text( this, IDC_STATIC_SEARCH_3, "" );
	std::string str[1] = {""};
	SetWin_Combo_Init( this, IDC_COMBO_ITEM_TYPE, str, 1 );

	if ( emSearch == EMSEARCH_MID || emSearch == EMSEARCH_SID || emSearch == EMSEARCH_NAME || emSearch == EMSEARCH_WEAR || emSearch == EMSEARCH_DDS || 
		emSearch == EMSEARCH_GENID || emSearch == EMSEARCH_RANDOM_OPT )
	{
		SetWin_Enable( this, IDC_EDIT_SEARCH, TRUE );
		SetWin_Text( this, IDC_STATIC_SEARCH_1, strTEXT.c_str() );
	}
	else if ( emSearch == EMSEARCH_DDSXY )
	{
		SetWin_Enable( this, IDC_EDIT_SEARCH, TRUE );
		SetWin_Enable( this, IDC_EDIT_X, TRUE );
		SetWin_Enable( this, IDC_EDIT_Y, TRUE );
		SetWin_Text( this, IDC_STATIC_SEARCH_1, strTEXT.c_str() );
		SetWin_Text( this, IDC_STATIC_SEARCH_2, "DDS X/Y" );
	}
	else if ( emSearch == EMSEARCH_TYPE )
	{
		SetWin_Enable( this, IDC_COMBO_ITEM_TYPE, TRUE );
		SetWin_Text( this, IDC_STATIC_SEARCH_3, strTEXT.c_str() );
	}

	switch ( emSearch ) 
	{
	case EMSEARCH_TYPE:
		{
			SetWin_Combo_Init( this, IDC_COMBO_ITEM_TYPE, COMMENT::ITEMTYPE, ITEM_NSIZE );
		}break;
	};
}
