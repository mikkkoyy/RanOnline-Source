// DlgSkill.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgSkill.h"

#include "EtcFunction.h"
#include "GLSkill.h"
#include "Logic/GMToolGlobal.h"


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
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnBnClickedButtonSearch)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_LBN_SELCHANGE(IDC_LIST_SELECT, OnLbnSelchangeListSelect)
	ON_CBN_SELCHANGE(IDC_COMBO_SEARCH, OnCbnSelchangeComboSearch)
END_MESSAGE_MAP()


// CDlgSkill message handlers
BOOL CDlgSkill::OnInitDialog()
{
	CDialog::OnInitDialog();

	ReadData();
	SelectionReset();

	SetWin_Combo_Init( this, IDC_COMBO_SEARCH, GMTOOL_GLOBAL::strSkillDataSearchType, EMSKILLDATA_SEARCH_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_SEARCH_CLASS, COMMENT::CHARCLASS, GLCI_NUM_8CLASS );
	SetWin_Text( this, IDC_EDIT_SEARCH, "" );

	InitSearchType();
	SearchData();

	return TRUE;
}

void CDlgSkill::ReadData()
{
	m_vecData.clear();
	for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ ){
		for ( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; SID++ ){        		
			PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
			if ( pSKILL ){
				m_vecData.push_back( pSKILL->m_sBASIC.sNATIVEID );
			}
		}
	}
}

void CDlgSkill::ShowSkills()
{
	m_listSELECT.ResetContent();

	for( int i=0; i<(int)m_vecResult.size(); ++i ){
		SNATIVEID sID = m_vecResult[i];
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sID );
		if ( !pSKILL ){
			continue;
		}

		CString strNAME;
		strNAME.Format ( "[%03d/%03d] %s", sID.wMainID, sID.wSubID, pSKILL->GetName() );

		int nIndex = m_listSELECT.AddString ( strNAME );
		m_listSELECT.SetItemData ( nIndex, pSKILL->m_sBASIC.sNATIVEID.dwID );	
	}
}

void CDlgSkill::SelectionReset()
{
	m_SELECTED = NATIVEID_NULL();
	SetWin_Text( this, IDC_EDIT_SEL_NAME, "" );
}

void CDlgSkill::SearchData()
{
	EMSKILLDATA_SEARCH emSEARCH =  static_cast<EMSKILLDATA_SEARCH> ( GetWin_Combo_Sel( this, IDC_COMBO_SEARCH ));
	EMCHARINDEX emINDEX = static_cast<EMCHARINDEX> ( GetWin_Combo_Sel( this, IDC_COMBO_SEARCH_CLASS ));
	EMCHARCLASS emCLASS = CharIndexToClass( emINDEX );
	std::string strSEARCH = GetWin_Text( this, IDC_EDIT_SEARCH );

	m_vecResult.clear();

	BOOL bNotString = ( emSEARCH == EMSKILLDATA_SEARCH_CLASS  ||
						emSEARCH == EMSKILLDATA_SEARCH_PET );

	if ( strSEARCH.size() <= 0 && !bNotString )
	{
		m_vecResult = m_vecData;
	}
	else
	{
		for( int i=0; i<(int)m_vecData.size(); ++i )
		{
			SNATIVEID sID = m_vecData[i];
			PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sID );
			if ( !pSKILL ){
				continue;
			}

			CString strFIND;
			switch( emSEARCH )
			{
			case EMSKILLDATA_SEARCH_NAME:{
					strFIND.Format( "%s", pSKILL->GetName() );
				}break;

			case EMSKILLDATA_SEARCH_MID:{
					strFIND.Format( "%d", pSKILL->m_sBASIC.sNATIVEID.wMainID );
				}break;

			case EMSKILLDATA_SEARCH_SID:{
					strFIND.Format( "%d", pSKILL->m_sBASIC.sNATIVEID.wSubID );
				}break;

			case EMSKILLDATA_SEARCH_CLASS:{
					if ( pSKILL->m_sLEARN.dwCLASS&emCLASS ){
						m_vecResult.push_back( sID );
					}
				}break;

			case EMSKILLDATA_SEARCH_PET:{
					if ( pSKILL->m_sBASIC.sNATIVEID.wMainID == EMSKILL_PET ){
						m_vecResult.push_back( sID );
					}
				}break;
			};

			if ( strFIND.GetLength() <= 0 ){
				continue;
			}

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

void CDlgSkill::OnBnClickedButtonSearch()
{
	SearchData();
}

void CDlgSkill::OnBnClickedOk()
{
	CDialog::OnOK();
}

void CDlgSkill::OnBnClickedCancel()
{
	CDialog::OnCancel();
}

void CDlgSkill::OnLbnSelchangeListSelect()
{
	int nSel = m_listSELECT.GetCurSel();
	if ( nSel < 0 ){
		return;
	}

	DWORD dwData = (DWORD) m_listSELECT.GetItemData ( nSel );
	SNATIVEID sID;
	sID.dwID = dwData;
	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sID );
	if ( pSKILL )
	{
		m_SELECTED = pSKILL->m_sBASIC.sNATIVEID;

		CString strNAME;
		strNAME.Format ( "[%03d/%03d] %s", sID.wMainID, sID.wSubID, pSKILL->GetName() );
		SetWin_Text( this, IDC_EDIT_SEL_NAME, strNAME.GetString() );
	}
	else
	{
		SelectionReset();
	}
}

void CDlgSkill::OnCbnSelchangeComboSearch()
{
	InitSearchType();
}

void CDlgSkill::InitSearchType()
{
	EMSKILLDATA_SEARCH emSEARCH =  static_cast<EMSKILLDATA_SEARCH> ( GetWin_Combo_Sel( this, IDC_COMBO_SEARCH ));

	BOOL bCLASS = ( emSEARCH == EMSKILLDATA_SEARCH_CLASS );
	BOOL bPET = ( emSEARCH == EMSKILLDATA_SEARCH_PET );
	

	SetWin_Enable( this, IDC_EDIT_SEARCH, !bCLASS && !bPET );
	SetWin_Enable( this, IDC_COMBO_SEARCH_CLASS, bCLASS );
}