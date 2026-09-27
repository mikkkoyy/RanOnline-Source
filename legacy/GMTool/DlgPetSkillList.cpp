// DlgPetSkillList.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgPetSkillList.h"

#include "GLSkill.h"
#include "HLibDataConvert.h"
#include "EtcFunction.h"

// CDlgPetSkillList dialog

IMPLEMENT_DYNAMIC(CDlgPetSkillList, CDialog)

CDlgPetSkillList::CDlgPetSkillList(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPetSkillList::IDD, pParent)
	, m_pData(NULL)
{

}

CDlgPetSkillList::CDlgPetSkillList(GLPET* pData, CWnd* pParent /*= NULL*/)
	: CDialog(CDlgPetSkillList::IDD, pParent)
	, m_pData(NULL)
{
	m_pData = pData;
}


CDlgPetSkillList::~CDlgPetSkillList()
{
}

void CDlgPetSkillList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SKILL_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CDlgPetSkillList, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SKILL_LIST, OnLvnItemchangedSkillList)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgPetSkillList message handlers
BOOL CDlgPetSkillList::OnInitDialog()
{
	CDialog::OnInitDialog();

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "", LVCFMT_LEFT, ( 5* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Skill", LVCFMT_LEFT, ( 90* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	ReadData();
	SelectionReset();

	return TRUE;  
}

void CDlgPetSkillList::ReadData()
{
	m_List.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_List.SetRedraw( FALSE );

	PETSKILL_MAP_ITER iter_start = m_pData->m_ExpSkills.begin();
	PETSKILL_MAP_ITER iter_end = m_pData->m_ExpSkills.end();
	for ( int i=0; iter_start != iter_end; ++iter_start, ++i ){
		const PETSKILL &sSkill = (*iter_start).second;
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sSkill.sNativeID );
		if ( !pSKILL ){
			continue;
		}

		m_List.InsertItem( i, "" );
		m_List.SetItemText( i, 1, _HLIB::cstringformat( "[%03d/%03d] %s", sSkill.sNativeID.wMainID, sSkill.sNativeID.wSubID, pSKILL->GetName() ).GetString() );
		m_List.SetItemData ( i, sSkill.sNativeID.dwID );
	}

	m_List.SetRedraw( TRUE );
}

void CDlgPetSkillList::SelectionReset()
{
	m_SELECTED = NATIVEID_NULL();
	SetWin_Text( this, IDC_EDIT_SEL_NAME, "" );
}

void CDlgPetSkillList::OnLvnItemchangedSkillList(NMHDR *pNMHDR, LRESULT *pResult)
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

	PETSKILL_MAP_ITER iter_data = m_pData->m_ExpSkills.find(dwID);
	if ( iter_data != m_pData->m_ExpSkills.end() ){
		const PETSKILL &sSkill = (*iter_data).second;

		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sSkill.sNativeID );
		if ( pSKILL ){
			m_SELECTED = pSKILL->m_sBASIC.sNATIVEID;

			CString strNAME;
			strNAME.Format ( "[%03d/%03d] %s", m_SELECTED.wMainID, m_SELECTED.wSubID, pSKILL->GetName() );
			SetWin_Text( this, IDC_EDIT_SEL_NAME, strNAME.GetString() );
		}
	}
}

void CDlgPetSkillList::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}

void CDlgPetSkillList::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}
