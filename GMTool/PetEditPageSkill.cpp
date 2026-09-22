// PetEditPageSkill.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "PetEditPageSkill.h"

#include "PetEditTab.h"
#include "GLPet.h"

#include "GLSkill.h"
#include "HLibDataConvert.h"
#include "DlgSkillAdd.h"

// CPetEditPageSkill dialog

IMPLEMENT_DYNAMIC(CPetEditPageSkill, CPropertyPage)

CPetEditPageSkill::CPetEditPageSkill(LOGFONT logfont, GLPET* pData)
	: CPropertyPage(CPetEditPageSkill::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CPetEditPageSkill::~CPetEditPageSkill()
{
	SAFE_DELETE ( m_pFont );
}

void CPetEditPageSkill::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PETSKILL, m_List);
}


BEGIN_MESSAGE_MAP(CPetEditPageSkill, CPropertyPage)
	ON_BN_CLICKED(IDC_PETEDITSKILL_BUTTON_ADD, OnBnClickedPeteditskillButtonAdd)
	ON_BN_CLICKED(IDC_PETEDITSKILL_BUTTON_DELETE, OnBnClickedPeteditskillButtonDelete)
	ON_BN_CLICKED(IDC_PETEDITSKILL_BUTTON_CLEAR, OnBnClickedPeteditskillButtonClear)
END_MESSAGE_MAP()


// CPetEditPageSkill message handlers
BOOL CPetEditPageSkill::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "", LVCFMT_LEFT, ( 5* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Skill", LVCFMT_LEFT, ( 90* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	DataShow();

	return TRUE;  
}

void CPetEditPageSkill::DataShow()
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

		std::string strNAME = "Unknown Skill";
		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( sSkill.sNativeID );
		if ( pSkill ){	
			strNAME = pSkill->GetName();
		}

		m_List.InsertItem( i, "" );
		m_List.SetItemText( i, 1, _HLIB::cstringformat( "[%03d/%03d] %s", sSkill.sNativeID.wMainID, sSkill.sNativeID.wSubID, strNAME.c_str() ).GetString() );
		m_List.SetItemData ( i, sSkill.sNativeID.dwID );
	}

	m_List.SetRedraw( TRUE );
}

void CPetEditPageSkill::OnBnClickedPeteditskillButtonAdd()
{
	if ( !m_pData ){
		return;
	}

	CDlgSkillAdd dlg( EMSKILLADD_TYPE_PET, GLCC_NONE, this );
	if ( dlg.DoModal() == IDOK ){
		SGMTOOL_SKILL_DATA_SELECTED_MAP mapSkills = dlg.m_mapSelect;
		SGMTOOL_SKILL_DATA_SELECTED_MAP_ITER iterData = mapSkills.begin();
		for( ; iterData != mapSkills.end(); ++iterData ){
			const SGMTOOL_SKILL_DATA_SELECTED sInsertData = (*iterData).second;
			PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sInsertData.sID );
			if ( !pSkill ){
				continue;
			}

			if ( !m_pData->ISLEARNED_SKILL( sInsertData.sID.dwID )){
				m_pData->LEARN_SKILL( sInsertData.sID.dwID );
			}
		}

		DataShow();
	}
}

void CPetEditPageSkill::OnBnClickedPeteditskillButtonDelete()
{
	if ( !m_pData ){
		return;
	}

	int nCHECKED = 0;
	for( int nItem=0; nItem<m_List.GetItemCount(); nItem++ )
	{
		BOOL bChecked = m_List.GetCheck( nItem );
		if( bChecked){
			DWORD dwID = (DWORD) m_List.GetItemData ( nItem );
			PETSKILL_MAP_ITER iter = m_pData->m_ExpSkills.find ( dwID );
			if ( iter != m_pData->m_ExpSkills.end() ){
				m_pData->m_ExpSkills.erase( iter );
			}

			nCHECKED ++;
		}
	}

	if ( nCHECKED == 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected Use Check boxes to select" );
	}

	DataShow();
}

void CPetEditPageSkill::OnBnClickedPeteditskillButtonClear()
{
	if ( !m_pData ){
		return;
	}

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All Skills?" ) == IDYES ){
		m_pData->m_ExpSkills.clear();
		DataShow();
	}
}
