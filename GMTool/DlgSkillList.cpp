// DlgSkillList.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgSkillList.h"

#include "EtcFunction.h"
#include "HLibDataConvert.h"
#include "Logic/GMToolGlobal.h"
#include "GLSkill.h"

// CDlgSkillList dialog

IMPLEMENT_DYNAMIC(CDlgSkillList, CDialog)

CDlgSkillList::CDlgSkillList(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSkillList::IDD, pParent)
	, m_pData(NULL)
{

}

CDlgSkillList::CDlgSkillList(SCHARDATA2* pData, CWnd* pParent /*= NULL*/)
	: CDialog(CDlgSkillList::IDD, pParent)
	, m_pData(NULL)
{
	m_pData = pData;
}


CDlgSkillList::~CDlgSkillList()
{
}

void CDlgSkillList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SKILL_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CDlgSkillList, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnBnClickedButtonSearch)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SKILL_LIST, OnLvnItemchangedSkillList)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgSkillList message handlers
BOOL CDlgSkillList::OnInitDialog()
{
	CDialog::OnInitDialog();

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "", LVCFMT_LEFT, ( 5* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Skill", LVCFMT_LEFT, ( 70* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 2, "Level", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	ReadData();
	SelectionReset();

	SetWin_Combo_Init( this, IDC_COMBO_SEARCH, GMTOOL_GLOBAL::strSkillDataSearchType, EMSKILLDATA_SEARCH_SIZE );
	SetWin_Combo_Del( this, IDC_COMBO_SEARCH, GMTOOL_GLOBAL::strSkillDataSearchType[EMSKILLDATA_SEARCH_CLASS].c_str() );
	SetWin_Combo_Del( this, IDC_COMBO_SEARCH, GMTOOL_GLOBAL::strSkillDataSearchType[EMSKILLDATA_SEARCH_PET].c_str() );
	SetWin_Text( this, IDC_EDIT_SEARCH, "" );

	SearchData();

	return TRUE;  
}

void CDlgSkillList::ReadData()
{
	m_vecData.clear();

	if ( !m_pData ){
		return;
	}

	SCHARDATA2::SKILL_MAP_ITER iter_start = m_pData->m_ExpSkills.begin();
	SCHARDATA2::SKILL_MAP_ITER iter_end = m_pData->m_ExpSkills.end();

	for ( int i=0; iter_start != iter_end; ++iter_start, ++i ){
		const SCHARSKILL &sSkill = (*iter_start).second;
		m_vecData.push_back( sSkill );
	}
}

void CDlgSkillList::ShowSkills()
{
	m_List.DeleteAllItems();

	m_List.SetRedraw( FALSE );

	for ( int i=0; i<(int)m_vecResult.size(); i++ ){
		SCHARSKILL sData = m_vecResult[i];
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sData.sNativeID );
		if ( !pSKILL ){
			continue;
		}

		m_List.InsertItem( i, "" );
		m_List.SetItemText( i, 1, _HLIB::cstringformat( "[%03d/%03d] %s", sData.sNativeID.wMainID, sData.sNativeID.wSubID, pSKILL->GetName() ).GetString() );
		m_List.SetItemText( i, 2, COMMENT::SKILL_LEVEL[sData.wLevel].c_str() );
		m_List.SetItemData ( i, sData.sNativeID.dwID );
	}

	m_List.SetRedraw( TRUE );
}

void CDlgSkillList::SelectionReset()
{
	m_SELECTED = NATIVEID_NULL();
	SetWin_Text( this, IDC_EDIT_SEL_NAME, "" );
}

void CDlgSkillList::SearchData()
{
	EMSKILLDATA_SEARCH emSEARCH =  static_cast<EMSKILLDATA_SEARCH> ( GetWin_Combo_Sel( this, IDC_COMBO_SEARCH ));
	std::string strSEARCH = GetWin_Text( this, IDC_EDIT_SEARCH );

	m_vecResult.clear();

	if ( strSEARCH.size() <= 0  )
	{
		m_vecResult = m_vecData;
	}
	else
	{
		for( int i=0; i<(int)m_vecData.size(); ++i )
		{
			SCHARSKILL sData = m_vecData[i];
			PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sData.sNativeID );
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
				m_vecResult.push_back( sData );
			}
		}
	}

	ShowSkills();
}

void CDlgSkillList::OnBnClickedButtonSearch()
{
	SearchData();
}

void CDlgSkillList::OnLvnItemchangedSkillList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	SelectionReset();

	if ( !m_pData ){
		return;
	}

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = (DWORD) m_List.GetItemData ( nSelect );

	SCHARDATA2::SKILL_MAP_ITER iter_data = m_pData->m_ExpSkills.find(dwID);
	if ( iter_data != m_pData->m_ExpSkills.end() ){
		const SCHARSKILL &sSkill = (*iter_data).second;

		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sSkill.sNativeID );
		if ( pSKILL ){
			m_SELECTED = pSKILL->m_sBASIC.sNATIVEID;

			CString strNAME;
			strNAME.Format ( "[%03d/%03d] %s", m_SELECTED.wMainID, m_SELECTED.wSubID, pSKILL->GetName() );
			SetWin_Text( this, IDC_EDIT_SEL_NAME, strNAME.GetString() );
		}
	}
}

void CDlgSkillList::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}

void CDlgSkillList::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}
