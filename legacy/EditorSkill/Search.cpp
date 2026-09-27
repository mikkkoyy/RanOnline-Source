// Search.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkill.h"
#include "Search.h"
#include "EtcFunction.h"
#include "DxSkinAniMan.h"
#include "GLSkill.h"
#include "GLCharDefine.h"
#include <algorithm>

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
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnBnClickedButtonSearch)
	ON_CBN_SELCHANGE(IDC_COMBO_SEARCHTYPE, OnCbnSelchangeComboSearchtype)
	ON_BN_CLICKED(IDC_BUTTON_GO, OnBnClickedButtonGo)
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
		"Search Effect File",
		"Search Icon DDS",
		"Search Icon DDS and Coordinates",
		"Search Animation Main",
		"Search Animation Sub",
		"Search Skill BasicType",
		"Search Skill Abnormal Condition",
		"Search Skill ImpactAddOn",
		"Search Skill SpecAddOn",
		"Search Skill SpecialType",
		"Search Skill ActionType",
	};

	SetWin_Combo_Init( this, IDC_COMBO_SEARCHTYPE, strSEARCH, EMSEARCH_TOTAL );

	m_sID = NATIVEID_NULL();

	ShowResults();
	SearchInit();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSearch::OnBnClickedButtonSearch()
{
	EMSEARCH emSearch = static_cast< EMSEARCH > ( GetWin_Combo_Sel( this, IDC_COMBO_SEARCHTYPE ));
	std::string strSEARCH = GetWin_Text( this, IDC_EDIT_SEARCH );
	int nSEARCH = GetWin_Num_int( this, IDC_EDIT_SEARCH );
	int nSEARCHX = GetWin_Num_int( this, IDC_EDIT_X );
	int nSEARCHY = GetWin_Num_int( this, IDC_EDIT_Y );
	int nSELECTED = GetWin_Combo_Sel( this, IDC_COMBO_SEARCH );

	m_List.DeleteAllItems();
	m_sID = NATIVEID_NULL();

	std::vector<SSEARCH> vec;

	switch ( emSearch ) 
	{
	case EMSEARCH_NAME:
		{
			//search by name
			for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ )
			{
				for ( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; SID++ )
				{
					PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
					if ( pSKILL )
					{
						std::string strSearch = strSEARCH;
						std::string strFindString = pSKILL->GetName();

						if ( strFindString.size() > 0 )
						{
							std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
							std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

							unsigned found = strFindString.find(strSearch.c_str());
							if (found!=std::string::npos)
							{
								SSEARCH sResult;
								sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
								sResult.strNAME = pSKILL->GetName();
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
			for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ )
			{
				for ( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; SID++ )
				{
					PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
					if ( pSKILL )
					{
						if ( pSKILL->m_sBASIC.sNATIVEID.wMainID == nSEARCH  )
						{
							SSEARCH sResult;
							sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
							sResult.strNAME = pSKILL->GetName();
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
			for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ )
			{
				for ( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; SID++ )
				{
					PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
					if ( pSKILL )
					{
						if ( pSKILL->m_sBASIC.sNATIVEID.wSubID == nSEARCH  )
						{
							SSEARCH sResult;
							sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
							sResult.strNAME = pSKILL->GetName();
							sResult.strFIND = strSEARCH.c_str();
							vec.push_back( sResult );
						}
					}
				}
			}
		}break;

	case EMSEARCH_EFFECT:
		{
			//search by effect
			for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ )
			{
				for ( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; SID++ )
				{
					PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
					if ( pSKILL )
					{

						for ( int nSLOT=0; nSLOT <  EMELEMENT_MAXNUM; ++ nSLOT )
						{
							std::string str = pSKILL->m_sEXT_DATA.strTARG[nSLOT];
							if ( str.size() > 0 )
							{
								std::string strSearch = strSEARCH;
								std::string strFindString = str.c_str();

								if ( strFindString.size() > 0 )
								{
									std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
									std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

									unsigned found = strFindString.find(strSearch.c_str());
									if (found!=std::string::npos)
									{
										SSEARCH sResult;
										sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
										sResult.strNAME = pSKILL->GetName();
										sResult.strFIND = strFindString.c_str();
										vec.push_back( sResult );
										break;
									}
								}
							}

							str = pSKILL->m_sEXT_DATA.strSELFZONE01[nSLOT];
							if ( str.size() > 0 )
							{
								std::string strSearch = strSEARCH;
								std::string strFindString = str.c_str();

								if ( strFindString.size() > 0 )
								{
									std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
									std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

									unsigned found = strFindString.find(strSearch.c_str());
									if (found!=std::string::npos)
									{
										SSEARCH sResult;
										sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
										sResult.strNAME = pSKILL->GetName();
										sResult.strFIND = strFindString.c_str();
										vec.push_back( sResult );
										break;
									}
								}
							}

							str = pSKILL->m_sEXT_DATA.strSELFZONE02[nSLOT];
							if ( str.size() > 0 )
							{
								std::string strSearch = strSEARCH;
								std::string strFindString = str.c_str();

								if ( strFindString.size() > 0 )
								{
									std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
									std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

									unsigned found = strFindString.find(strSearch.c_str());
									if (found!=std::string::npos)
									{
										SSEARCH sResult;
										sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
										sResult.strNAME = pSKILL->GetName();
										sResult.strFIND = strFindString.c_str();
										vec.push_back( sResult );
										break;
									}
								}
							}

							str = pSKILL->m_sEXT_DATA.strTARGZONE01[nSLOT];
							if ( str.size() > 0 )
							{	
								std::string strSearch = strSEARCH;
								std::string strFindString = str.c_str();

								if ( strFindString.size() > 0 )
								{
									std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
									std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

									unsigned found = strFindString.find(strSearch.c_str());
									if (found!=std::string::npos)
									{
										SSEARCH sResult;
										sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
										sResult.strNAME = pSKILL->GetName();
										sResult.strFIND = strFindString.c_str();
										vec.push_back( sResult );
										break;
									}
								}
							}

							str = pSKILL->m_sEXT_DATA.strTARGZONE02[nSLOT];
							if ( str.size() > 0 )
							{
								std::string strSearch = strSEARCH;
								std::string strFindString = str.c_str();

								if ( strFindString.size() > 0 )
								{
									std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
									std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

									unsigned found = strFindString.find(strSearch.c_str());
									if (found!=std::string::npos)
									{
										SSEARCH sResult;
										sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
										sResult.strNAME = pSKILL->GetName();
										sResult.strFIND = strFindString.c_str();
										vec.push_back( sResult );
										break;
									}
								}
							}

							str = pSKILL->m_sEXT_DATA.strSELFBODY[nSLOT];	
							if ( str.size() > 0 )
							{
								std::string strSearch = strSEARCH;
								std::string strFindString = str.c_str();

								if ( strFindString.size() > 0 )
								{
									std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
									std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

									unsigned found = strFindString.find(strSearch.c_str());
									if (found!=std::string::npos)
									{
										SSEARCH sResult;
										sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
										sResult.strNAME = pSKILL->GetName();
										sResult.strFIND = strFindString.c_str();
										vec.push_back( sResult );
										break;
									}
								}
							}

							str = pSKILL->m_sEXT_DATA.strTARGBODY01[nSLOT];
							if ( str.size() > 0 )
							{
								std::string strSearch = strSEARCH;
								std::string strFindString = str.c_str();

								if ( strFindString.size() > 0 )
								{
									std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
									std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

									unsigned found = strFindString.find(strSearch.c_str());
									if (found!=std::string::npos)
									{
										SSEARCH sResult;
										sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
										sResult.strNAME = pSKILL->GetName();
										sResult.strFIND = strFindString.c_str();
										vec.push_back( sResult );
										break;
									}
								}
							}

							str = pSKILL->m_sEXT_DATA.strTARGBODY02[nSLOT];
							if ( str.size() > 0 )
							{
								std::string strSearch = strSEARCH;
								std::string strFindString = str.c_str();

								if ( strFindString.size() > 0 )
								{
									std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
									std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

									unsigned found = strFindString.find(strSearch.c_str());
									if (found!=std::string::npos)
									{
										SSEARCH sResult;
										sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
										sResult.strNAME = pSKILL->GetName();
										sResult.strFIND = strFindString.c_str();
										vec.push_back( sResult );
										break;
									}
								}
							}
						}


						{
							std::string str = pSKILL->m_sEXT_DATA.strHOLDOUT;
							if ( str.size() > 0 )
							{
								std::string strSearch = strSEARCH;
								std::string strFindString = str.c_str();

								if ( strFindString.size() > 0 )
								{
									std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
									std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

									unsigned found = strFindString.find(strSearch.c_str());
									if (found!=std::string::npos)
									{
										SSEARCH sResult;
										sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
										sResult.strNAME = pSKILL->GetName();
										sResult.strFIND = strFindString.c_str();
										vec.push_back( sResult );
									}
								}
							}
						}


						{
							std::string str = pSKILL->m_sSPECIAL_SKILL.strEffectName.c_str();
							if ( str.size() > 0 )
							{
								std::string strSearch = strSEARCH;
								std::string strFindString = str.c_str();

								if ( strFindString.size() > 0 )
								{
									std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
									std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

									unsigned found = strFindString.find(strSearch.c_str());
									if (found!=std::string::npos)
									{
										SSEARCH sResult;
										sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
										sResult.strNAME = pSKILL->GetName();
										sResult.strFIND = strFindString.c_str();
										vec.push_back( sResult );
									}
								}
							}
						}
					}
				}
			}

		}break;

	case EMSEARCH_DDS:
		{
			//search by dds
			for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ )
			{
				for ( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; SID++ )
				{
					PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
					if ( pSKILL )
					{
						std::string strSearch = strSEARCH;
						std::string strFindString = pSKILL->m_sEXT_DATA.strICONFILE.c_str();

						if ( strFindString.size() > 0 )
						{
							std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
							std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

							unsigned found = strFindString.find(strSearch.c_str());
							if (found!=std::string::npos)
							{
								SSEARCH sResult;
								sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
								sResult.strNAME = pSKILL->GetName();
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
			//search by dds and x y coords
			for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ )
			{
				for ( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; SID++ )
				{
					PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
					if ( pSKILL )
					{
						std::string strSearch = strSEARCH;
						std::string strFindString = pSKILL->m_sEXT_DATA.strICONFILE.c_str();

						if ( strFindString.size() > 0 )
						{
							std::transform ( strSearch.begin(), strSearch.end(), strSearch.begin(), tolower );
							std::transform ( strFindString.begin(), strFindString.end(), strFindString.begin(), tolower );

							unsigned found = strFindString.find(strSearch.c_str());
							if (found!=std::string::npos)
							{
								if ( pSKILL->m_sEXT_DATA.sICONINDEX.wMainID == nSEARCHX && pSKILL->m_sEXT_DATA.sICONINDEX.wSubID == nSEARCHY )
								{
									SSEARCH sResult;
									sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
									sResult.strNAME = pSKILL->GetName();
									sResult.strFIND = strFindString.c_str();
									vec.push_back( sResult );
								}
							}
						}
					}
				}
			}
		}break;

	case EMSEARCH_SKILL_ANIMAIN:
		{
			//search by ani maintype
			for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ )
			{
				for ( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; SID++ )
				{
					PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
					if ( pSKILL )
					{
						if ( pSKILL->m_sEXT_DATA.emANIMTYPE == static_cast<EMANI_MAINTYPE>( nSELECTED )  )
						{
							SSEARCH sResult;
							sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
							sResult.strNAME = pSKILL->GetName();
							sResult.strFIND = COMMENT::ANI_MAINTYPE_CHAR[pSKILL->m_sEXT_DATA.emANIMTYPE].c_str();
							vec.push_back( sResult );
						}
					}
				}
			}
		}break;

	case EMSEARCH_SKILL_ANISUB:
		{
			//search by ani subtype
			for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ )
			{
				for ( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; SID++ )
				{
					PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
					if ( pSKILL )
					{
						if ( pSKILL->m_sEXT_DATA.emANISTYPE == static_cast<EMANI_SUBTYPE>( nSELECTED )  )
						{
							SSEARCH sResult;
							sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
							sResult.strNAME = pSKILL->GetName();
							sResult.strFIND = COMMENT::ANI_SUBTYPE_SKILL[pSKILL->m_sEXT_DATA.emANISTYPE].c_str();
							vec.push_back( sResult );
						}
					}
				}
			}
		}break;

	case EMSEARCH_SKILL_APPLYTYPE:
		{
			//search by applytype
			for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ )
			{
				for ( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; SID++ )
				{
					PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
					if ( pSKILL )
					{
						if ( pSKILL->m_sAPPLY.emBASIC_TYPE == static_cast<SKILL::EMTYPES>( nSELECTED )  )
						{
							SSEARCH sResult;
							sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
							sResult.strNAME = pSKILL->GetName();
							sResult.strFIND = COMMENT::SKILL_TYPES[pSKILL->m_sAPPLY.emBASIC_TYPE].c_str();
							vec.push_back( sResult );
						}
					}
				}
			}
		}break;

	case EMSEARCH_SKILL_STATETROUBLE:
		{
			//search by stateblow
			for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ )
			{
				for ( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; SID++ )
				{
					PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
					if ( pSKILL )
					{
						if ( pSKILL->m_sAPPLY.emSTATE_BLOW == static_cast<EMSTATE_BLOW>( nSELECTED )  )
						{
							SSEARCH sResult;
							sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
							sResult.strNAME = pSKILL->GetName();
							sResult.strFIND = COMMENT::BLOW[pSKILL->m_sAPPLY.emSTATE_BLOW].c_str();
							vec.push_back( sResult );
						}
					}
				}
			}
		}break;

	case EMSEARCH_SKILL_IMPACTADDON:
		{
			//search by impact addon
			for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ )
			{
				for ( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; SID++ )
				{
					PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
					if ( pSKILL )
					{
						for( int nImpact=0; nImpact<SKILL::MAX_IMPACT; ++nImpact )
						{
							if ( pSKILL->m_sAPPLY.sImpacts[nImpact].emADDON == static_cast<EMIMPACT_ADDON>( nSELECTED )  )
							{
								SSEARCH sResult;
								sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
								sResult.strNAME = pSKILL->GetName();
								sResult.strFIND = COMMENT::IMPACT_ADDON[pSKILL->m_sAPPLY.sImpacts[nImpact].emADDON].c_str();
								vec.push_back( sResult );
							}
						}
					}
				}
			}
		}break;

	case EMSEARCH_SKILL_SPECADDON:
		{
			//search by spec addon
			for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ )
			{
				for ( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; SID++ )
				{
					PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
					if ( pSKILL )
					{
						for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
						{
							if ( pSKILL->m_sAPPLY.sSpecs[nSpec].emSPEC == static_cast<EMSPEC_ADDON>( nSELECTED )  )
							{
								SSEARCH sResult;
								sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
								sResult.strNAME = pSKILL->GetName();
								sResult.strFIND = COMMENT::SPEC_ADDON[pSKILL->m_sAPPLY.sSpecs[nSpec].emSPEC].c_str();
								vec.push_back( sResult );
							}
						}
					}
				}
			}
		}break;

	case EMSEARCH_SKILL_SPECIALTYPE:
		{
			//search by skill special
			for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ )
			{
				for ( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; SID++ )
				{
					PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
					if ( pSKILL )
					{
						if ( pSKILL->m_sSPECIAL_SKILL.emSSTYPE == static_cast<SKILL::EMSPECIALSKILLTYPE>( nSELECTED )  )
						{
							SSEARCH sResult;
							sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
							sResult.strNAME = pSKILL->GetName();
							sResult.strFIND = COMMENT::SPECIAL_SKILL_TYPE[pSKILL->m_sSPECIAL_SKILL.emSSTYPE].c_str();
							vec.push_back( sResult );
						}
					}
				}
			}
		}break;

	case EMSEARCH_SKILL_ACTIONTYPE:
		{
			//search by skill action type
			for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ )
			{
				for ( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; SID++ )
				{
					PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
					if ( pSKILL )
					{
						if ( pSKILL->m_sBASIC.emACTION == static_cast<SKILL::EMACTION>( nSELECTED )  )
						{
							SSEARCH sResult;
							sResult.sID = pSKILL->m_sBASIC.sNATIVEID;
							sResult.strNAME = pSKILL->GetName();
							sResult.strFIND = COMMENT::SKILL_ACTION_TYPE[pSKILL->m_sBASIC.emACTION].c_str();
							vec.push_back( sResult );
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

void CSearch::SearchInit()
{
	EMSEARCH emSearch = static_cast< EMSEARCH > ( GetWin_Combo_Sel( this, IDC_COMBO_SEARCHTYPE ));
	std::string strTEXT = GetWin_Combo_Sel_Text( this, IDC_COMBO_SEARCHTYPE ).GetString();

	SetWin_Enable( this, IDC_EDIT_SEARCH, FALSE );
	SetWin_Enable( this, IDC_EDIT_X, FALSE );
	SetWin_Enable( this, IDC_EDIT_Y, FALSE );
	SetWin_Enable( this, IDC_COMBO_SEARCH, FALSE );
	SetWin_Text( this, IDC_EDIT_SEARCH, "" );
	SetWin_Text( this, IDC_EDIT_X, "" );
	SetWin_Text( this, IDC_EDIT_Y, "" );
	SetWin_Text( this, IDC_STATIC_SEARCH_1, "" );
	SetWin_Text( this, IDC_STATIC_SEARCH_2, "" );
	SetWin_Text( this, IDC_STATIC_SEARCH_3, "" );
	std::string str[1] = {""};
	SetWin_Combo_Init( this, IDC_COMBO_SEARCH, str, 1 );

	if ( emSearch == EMSEARCH_MID || emSearch == EMSEARCH_SID || emSearch == EMSEARCH_NAME || emSearch == EMSEARCH_EFFECT || emSearch == EMSEARCH_DDS )
	{
		SetWin_Enable( this, IDC_EDIT_SEARCH, TRUE );
		SetWin_Text( this, IDC_STATIC_SEARCH_1, strTEXT.c_str() );
	}
	else if ( emSearch == EMSEARCH_DDSXY  )
	{
		SetWin_Enable( this, IDC_EDIT_SEARCH, TRUE );
		SetWin_Enable( this, IDC_EDIT_X, TRUE );
		SetWin_Enable( this, IDC_EDIT_Y, TRUE );
		SetWin_Text( this, IDC_STATIC_SEARCH_1, strTEXT.c_str() );
		SetWin_Text( this, IDC_STATIC_SEARCH_2, "DDS X/Y" );
	}
	else if ( emSearch == EMSEARCH_SKILL_ANIMAIN || emSearch == EMSEARCH_SKILL_ANISUB ||  emSearch == EMSEARCH_SKILL_APPLYTYPE || 
		emSearch == EMSEARCH_SKILL_STATETROUBLE || emSearch == EMSEARCH_SKILL_IMPACTADDON || emSearch == EMSEARCH_SKILL_SPECADDON || 
		emSearch == EMSEARCH_SKILL_SPECIALTYPE || emSearch == EMSEARCH_SKILL_ACTIONTYPE ) 
	{
		SetWin_Enable( this, IDC_COMBO_SEARCH, TRUE );
		SetWin_Text( this, IDC_STATIC_SEARCH_3, strTEXT.c_str() );
	}

	switch ( emSearch ) 
	{
	case EMSEARCH_SKILL_ANIMAIN:
		{
			SetWin_Combo_Init( this, IDC_COMBO_SEARCH, COMMENT::ANI_MAINTYPE_CHAR, AN_TYPE_SIZE );
		}break;
	case EMSEARCH_SKILL_ANISUB:
		{
			SetWin_Combo_Init( this, IDC_COMBO_SEARCH, COMMENT::ANI_SUBTYPE_SKILL, AN_SUB_00_SIZE );
		}break;
	case EMSEARCH_SKILL_APPLYTYPE:
		{
			SetWin_Combo_Init( this, IDC_COMBO_SEARCH, COMMENT::SKILL_TYPES, SKILL::FOR_TYPE_SIZE );
		}break;
	case EMSEARCH_SKILL_STATETROUBLE:
		{
			SetWin_Combo_Init( this, IDC_COMBO_SEARCH, COMMENT::BLOW, EMBLOW_SIZE );
		}break;
	case EMSEARCH_SKILL_IMPACTADDON:
		{
			SetWin_Combo_Init( this, IDC_COMBO_SEARCH, COMMENT::IMPACT_ADDON, EIMPACTA_SIZE );
		}break;
	case EMSEARCH_SKILL_SPECADDON:
		{
			SetWin_Combo_Init( this, IDC_COMBO_SEARCH, COMMENT::SPEC_ADDON, EMSPECA_NSIZE );
		}break;
	case EMSEARCH_SKILL_SPECIALTYPE:
		{
			SetWin_Combo_Init( this, IDC_COMBO_SEARCH, COMMENT::SPECIAL_SKILL_TYPE, SKILL::EMSSTYPE_NSIZE );
		}break;
	case EMSEARCH_SKILL_ACTIONTYPE:
		{
			SetWin_Combo_Init( this, IDC_COMBO_SEARCH, COMMENT::SKILL_ACTION_TYPE, SKILL::EMACTION_NSIZE );
		}break;
	};
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
void CSearch::OnBnClickedButtonGo()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	m_sID.wMainID = (WORD)atoi( m_List.GetItemText(nSelect,1).GetString() );
	m_sID.wSubID = (WORD)atoi( m_List.GetItemText(nSelect,2).GetString() );
	OnOK();
}
