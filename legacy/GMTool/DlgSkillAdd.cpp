// DlgSkillAdd.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgSkillAdd.h"

#include "GLSkill.h"
#include "EtcFunction.h"
#include "HLibDataConvert.h"
#include "Logic/GMToolGlobal.h"

// CDlgSkillAdd dialog

IMPLEMENT_DYNAMIC(CDlgSkillAdd, CDialog)

CDlgSkillAdd::CDlgSkillAdd(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSkillAdd::IDD, pParent)
	, m_emEditType(EMSKILLADD_TYPE_SIZE)
	, m_emClass(GLCC_ALL_8CLASS)
{
}

CDlgSkillAdd::CDlgSkillAdd(EMSKILLADD_TYPE emEDIT, EMCHARCLASS emCLASS, CWnd* pParent /*= NULL*/)
	: CDialog(CDlgSkillAdd::IDD, pParent)
	, m_emEditType(EMSKILLADD_TYPE_SIZE)
	, m_emClass(GLCC_ALL_8CLASS)
{
	m_emEditType = emEDIT;
	m_emClass = emCLASS;
}

CDlgSkillAdd::~CDlgSkillAdd()
{
}

void CDlgSkillAdd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SKILLADD_LIST_SKILL, m_ListSelection);
	DDX_Control(pDX, IDC_SKILLADD_LIST_SELECT, m_ListSelected);
}

BEGIN_MESSAGE_MAP(CDlgSkillAdd, CDialog)
	ON_BN_CLICKED(IDC_SKILLADD_BUTTON_SEARCH, OnBnClickedSkilladdButtonSearch)
	ON_BN_CLICKED(IDC_SKILLADD_BUTTON_SELECTED_ADD, OnBnClickedSkilladdButtonSelectedAdd)
	ON_BN_CLICKED(IDC_SKILLADD_BUTTON_SELECTED_ADDALL, OnBnClickedSkilladdButtonSelectedAddall)
	ON_BN_CLICKED(IDC_SKILLADD_BUTTON_SELECTION_REMOVE, OnBnClickedSkilladdButtonSelectionRemove)
	ON_BN_CLICKED(IDC_SKILLADD_BUTTON_SELECTION_REMOVEALL, OnBnClickedSkilladdButtonSelectionRemoveall)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_NOTIFY(NM_DBLCLK, IDC_SKILLADD_LIST_SKILL, OnNMDblclkSkilladdListSkill)
END_MESSAGE_MAP()


// CDlgSkillAdd message handlers
BOOL CDlgSkillAdd::OnInitDialog()
{
	CDialog::OnInitDialog();

	RECT rectCtrl;
	m_ListSelection.SetExtendedStyle ( m_ListSelection.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES );
	m_ListSelection.GetClientRect ( &rectCtrl );
	m_ListSelection.InsertColumn( 0, "", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListSelection.InsertColumn( 1, "Skill", LVCFMT_LEFT, ( 85* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	m_ListSelected.SetExtendedStyle ( m_ListSelected.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES );
	m_ListSelected.GetClientRect ( &rectCtrl );
	m_ListSelected.InsertColumn( 0, "", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListSelected.InsertColumn( 1, "Skill", LVCFMT_LEFT, ( 65* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_ListSelected.InsertColumn( 2, "Level", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	
	SetWin_Combo_Init( this, IDC_SKILLADD_COMBO_SEARCH, GMTOOL_GLOBAL::strSkillDataSearchType, EMSKILLDATA_SEARCH_SIZE );
	SetWin_Combo_Del( this, IDC_SKILLADD_COMBO_SEARCH, GMTOOL_GLOBAL::strSkillDataSearchType[EMSKILLDATA_SEARCH_CLASS].c_str() );
	SetWin_Combo_Del( this, IDC_SKILLADD_COMBO_SEARCH, GMTOOL_GLOBAL::strSkillDataSearchType[EMSKILLDATA_SEARCH_PET].c_str() );
	SetWin_Combo_Init( this, IDC_SKILLADD_COMBO_SKILLLEVEL, COMMENT::SKILL_LEVEL, SKILL::MAX_LEVEL );

	m_vecSkills.clear();
	m_vecResult.clear();
	m_mapSelect.clear();

	DataRead();
	DataSearch();

	CString strWindowText("");

	if ( m_emEditType == EMSKILLADD_TYPE_CHAR ){
		strWindowText.Format( "%s Skill Add", COMMENT::CHARCLASS[ CharClassToIndex( m_emClass) ].c_str() );
	}else if ( m_emEditType == EMSKILLADD_TYPE_PET ){
		strWindowText.Format( "Pet Skill Add" );
	}

	if ( strWindowText.GetLength() > 0 ){
		SetWindowText( strWindowText.GetString() );
	}
	
	return TRUE;  
}

void CDlgSkillAdd::DataRead()
{
	m_vecSkills.clear();

	for ( WORD MID=0; MID<EMSKILLCLASS_NSIZE; ++MID ){
		for ( WORD SID=0; SID<GLSkillMan::MAX_CLASSSKILL; ++SID ){
			PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( MID, SID );
			if ( pSkill ){
				if ( m_emEditType == EMSKILLADD_TYPE_CHAR ){
					if ( !(pSkill->m_sLEARN.dwCLASS&m_emClass) ){
						continue;
					}
				}else if ( m_emEditType == EMSKILLADD_TYPE_PET ){
					if ( MID != EMSKILL_PET ){
						continue;
					}
				}

				SGMTOOL_SKILL_DATA_SELECTION sDATA;
				sDATA.sID = pSkill->m_sBASIC.sNATIVEID;
				sDATA.strNAME = pSkill->GetName();
				m_vecSkills.push_back( sDATA );
			}
		}
	}
}

void CDlgSkillAdd::DataShowSelection()
{
	m_ListSelection.DeleteAllItems();

	int nSIZE = (int)m_vecResult.size();
	if ( nSIZE <= 0 ){
		return;
	}

	m_ListSelection.SetRedraw( FALSE );

	for ( int i=0; i<nSIZE; i++ ){
		SGMTOOL_SKILL_DATA_SELECTION sDATA = m_vecResult[i];
		m_ListSelection.InsertItem( i, "" );
		m_ListSelection.SetItemText( i, 1, _HLIB::cstringformat( "[%03d/%03d] %s", sDATA.sID.wMainID, sDATA.sID.wSubID, sDATA.strNAME.c_str() ).GetString() );
		m_ListSelection.SetItemData ( i, sDATA.sID.dwID );
	}

	m_ListSelection.SetRedraw( TRUE );
}

void CDlgSkillAdd::DataShowSelected()
{
	m_ListSelected.DeleteAllItems();

	m_ListSelected.SetRedraw( FALSE );

	SGMTOOL_SKILL_DATA_SELECTED_MAP_ITER iterSelect = m_mapSelect.begin();
	for( int i=0; iterSelect != m_mapSelect.end(); ++ iterSelect ){

		SGMTOOL_SKILL_DATA_SELECTED sDATA = (*iterSelect).second;
		m_ListSelected.InsertItem( i, "" );
		m_ListSelected.SetItemText( i, 1, _HLIB::cstringformat( "[%03d/%03d] %s", sDATA.sID.wMainID, sDATA.sID.wSubID, sDATA.strNAME.c_str() ).GetString() );
		m_ListSelected.SetItemText( i, 2, COMMENT::SKILL_LEVEL[sDATA.wLEVEL].c_str() );
		m_ListSelected.SetItemData ( i, sDATA.sID.dwID );
	}

	m_ListSelected.SetRedraw( TRUE );
}

void CDlgSkillAdd::DataSearch()
{
	EMSKILLDATA_SEARCH emSEARCH = static_cast<EMSKILLDATA_SEARCH>( GetWin_Combo_Sel( this, IDC_SKILLADD_COMBO_SEARCH ) );
	std::string strSEARCH = GetWin_Text( this, IDC_SKILLADD_EDIT_SEARCH );

	m_vecResult.clear();

	if ( strSEARCH.size() <= 0 ){
		m_vecResult = m_vecSkills;
	}else{
		for( int i=0; i<(int)m_vecSkills.size(); ++ i ){
			CString strFIND;
			switch( emSEARCH )
			{
			case EMSKILLDATA_SEARCH_NAME:{
					strFIND.Format( "%s", m_vecSkills[i].strNAME.c_str() );
				}break;
			case EMSKILLDATA_SEARCH_MID:{
					strFIND.Format( "%d", m_vecSkills[i].sID.wMainID );
				}break;
			case EMSKILLDATA_SEARCH_SID:{
					strFIND.Format( "%d", m_vecSkills[i].sID.wSubID );
				}break;
			};

			if ( strFIND.GetLength() <= 0 )	continue;

			std::string strSEARCH1 = strSEARCH.c_str();
			std::string strSEARCH2 = strFIND.GetString();
			std::transform ( strSEARCH1.begin(), strSEARCH1.end(), strSEARCH1.begin(), tolower );
			std::transform ( strSEARCH2.begin(), strSEARCH2.end(), strSEARCH2.begin(), tolower );

			unsigned found = strSEARCH2.find(strSEARCH1.c_str());
			if (found!=std::string::npos){
				m_vecResult.push_back( m_vecSkills[i] );
			}
		}
	}

	DataShowSelection();
}

void CDlgSkillAdd::OnBnClickedSkilladdButtonSearch()
{
	DataSearch();
}

void CDlgSkillAdd::OnBnClickedSkilladdButtonSelectedAdd()
{
	WORD wSkillLevel = (WORD)GetWin_Combo_Sel( this, IDC_SKILLADD_COMBO_SKILLLEVEL );
	if( wSkillLevel >= SKILL::MAX_LEVEL ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Skill Level must be between %d ~ %d only", 1, SKILL::MAX_LEVEL -1 );
		return;
	}

	int nCHECKED = 0;
	for( int nItem=0; nItem<m_ListSelection.GetItemCount(); nItem++ ){
		BOOL bChecked = m_ListSelection.GetCheck( nItem );
		if( bChecked){
			DWORD dwID = (DWORD) m_ListSelection.GetItemData ( nItem );

			PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( SNATIVEID(dwID) );
			if ( pSKILL ){
				SGMTOOL_SKILL_DATA_SELECTED sDATA;
				sDATA.sID = pSKILL->m_sBASIC.sNATIVEID;
				sDATA.strNAME = pSKILL->GetName();
				sDATA.wLEVEL = wSkillLevel;

				GMTOOL_GLOBAL::SkillLevelCheck( sDATA.wLEVEL, sDATA.sID );

				SGMTOOL_SKILL_DATA_SELECTED_MAP_ITER iterfind = m_mapSelect.find(sDATA.sID.dwID);
				if ( iterfind != m_mapSelect.end() ){
					m_mapSelect.erase( iterfind );
				}

				m_mapSelect.insert( std::make_pair( sDATA.sID.dwID, sDATA ));
			}

			nCHECKED ++;
		}
	}

	if ( nCHECKED == 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected Use Check boxes to select" );
	}

	DataShowSelected();
}

void CDlgSkillAdd::OnBnClickedSkilladdButtonSelectedAddall()
{
	WORD wSkillLevel = (WORD)GetWin_Combo_Sel( this, IDC_SKILLADD_COMBO_SKILLLEVEL );
	if( wSkillLevel >= SKILL::MAX_LEVEL ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Skill Level must be between %d ~ %d only", 1, SKILL::MAX_LEVEL -1 );
		return;
	}

	for( int nItem=0; nItem<m_ListSelection.GetItemCount(); nItem++ ){
		DWORD dwID = (DWORD) m_ListSelection.GetItemData ( nItem );

		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( SNATIVEID(dwID) );
		if ( pSKILL ){
			SGMTOOL_SKILL_DATA_SELECTED sDATA;
			sDATA.sID = pSKILL->m_sBASIC.sNATIVEID;
			sDATA.strNAME = pSKILL->GetName();
			sDATA.wLEVEL = wSkillLevel;

			GMTOOL_GLOBAL::SkillLevelCheck( sDATA.wLEVEL, sDATA.sID );

			SGMTOOL_SKILL_DATA_SELECTED_MAP_ITER iterfind = m_mapSelect.find(sDATA.sID.dwID);
			if ( iterfind != m_mapSelect.end() ){
				m_mapSelect.erase( iterfind );
			}

			m_mapSelect.insert( std::make_pair( sDATA.sID.dwID, sDATA ));
		}
	}

	DataShowSelected();
}

void CDlgSkillAdd::OnBnClickedSkilladdButtonSelectionRemove()
{
	int nCHECKED = 0;
	for( int nItem=0; nItem<m_ListSelected.GetItemCount(); nItem++ ){
		BOOL bChecked = m_ListSelected.GetCheck( nItem );
		if( bChecked){
			DWORD dwID = (DWORD) m_ListSelected.GetItemData ( nItem );

			SGMTOOL_SKILL_DATA_SELECTED_MAP_ITER iterfind = m_mapSelect.find(dwID);
			if ( iterfind != m_mapSelect.end() ){
				m_mapSelect.erase( iterfind );
			}

			nCHECKED ++;
		}
	}

	if ( nCHECKED == 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected Use Check boxes to select" );
	}

	DataShowSelected();
}

void CDlgSkillAdd::OnBnClickedSkilladdButtonSelectionRemoveall()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Remove all skills in selection list?" ) == IDYES ){
		m_mapSelect.clear();
		DataShowSelected();
	}
}

void CDlgSkillAdd::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}

void CDlgSkillAdd::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}

void CDlgSkillAdd::OnNMDblclkSkilladdListSkill(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	WORD wSkillLevel = (WORD)GetWin_Combo_Sel( this, IDC_SKILLADD_COMBO_SKILLLEVEL );
	if( wSkillLevel >= SKILL::MAX_LEVEL ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Skill Level must be between %d ~ %d only", 1, SKILL::MAX_LEVEL -1 );
		return;
	}

	int nSelect = m_ListSelection.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = (DWORD) m_ListSelection.GetItemData ( nSelect );

	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( SNATIVEID(dwID) );
	if ( pSKILL ){
		SGMTOOL_SKILL_DATA_SELECTED sDATA;
		sDATA.sID = pSKILL->m_sBASIC.sNATIVEID;
		sDATA.strNAME = pSKILL->GetName();
		sDATA.wLEVEL = wSkillLevel;

		GMTOOL_GLOBAL::SkillLevelCheck( sDATA.wLEVEL, sDATA.sID );

		SGMTOOL_SKILL_DATA_SELECTED_MAP_ITER iterfind = m_mapSelect.find(sDATA.sID.dwID);
		if ( iterfind != m_mapSelect.end() ){
			m_mapSelect.erase( iterfind );
		}

		m_mapSelect.insert( std::make_pair( sDATA.sID.dwID, sDATA ));

		DataShowSelected();
	}
}

