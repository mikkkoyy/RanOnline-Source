// PetEditPageSkillSlot.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "PetEditPageSkillSlot.h"

#include "PetEditTab.h"
#include "GLPet.h"
#include "DlgPetSkillList.h"
#include "GLSkill.h"
#include "EtcFunction.h"
#include ".\peteditpageskillslot.h"

// CPetEditPageSkillSlot dialog

IMPLEMENT_DYNAMIC(CPetEditPageSkillSlot, CPropertyPage)

CPetEditPageSkillSlot::CPetEditPageSkillSlot(LOGFONT logfont, GLPET* pData)
	: CPropertyPage(CPetEditPageSkillSlot::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CPetEditPageSkillSlot::~CPetEditPageSkillSlot()
{
	SAFE_DELETE ( m_pFont );
}

void CPetEditPageSkillSlot::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPetEditPageSkillSlot, CPropertyPage)
	ON_BN_CLICKED(IDC_PETEDITSKILLSLOT_BUTTON_SET_0, OnBnClickedPeteditskillslotButtonSet0)
	ON_BN_CLICKED(IDC_PETEDITSKILLSLOT_BUTTON_RESET_0, OnBnClickedPeteditskillslotButtonReset0)
	ON_BN_CLICKED(IDC_PETEDITSKILLSLOT_BUTTON_SET_1, OnBnClickedPeteditskillslotButtonSet1)
	ON_BN_CLICKED(IDC_PETEDITSKILLSLOT_BUTTON_RESET_1, OnBnClickedPeteditskillslotButtonReset1)
END_MESSAGE_MAP()


// CPetEditPageSkillSlot message handlers
BOOL CPetEditPageSkillSlot::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	DataShow();

	return TRUE;  
}

void CPetEditPageSkillSlot::DataShow()
{
	if( !m_pData )	return;

	{
		SNATIVEID sID = m_pData->m_sActiveSkillID_A;
		std::string strSkill = "Unknown Skill";
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sID );
		if ( pSKILL )
			strSkill = pSKILL->GetName();

		CString strName;
		strName.Format( "(%d~%d) %s", sID.wMainID, sID.wSubID, strSkill.c_str() );
		SetWin_Text( this, IDC_PETEDITSKILLSLOT_EDIT_SLOT_0, strName.GetString() );
	}

	{
		/*dual pet skill, Juver, 2017/12/29 */
		SNATIVEID sID = m_pData->m_sActiveSkillID_B;
		std::string strSkill = "Unknown Skill";
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sID );
		if ( pSKILL )
			strSkill = pSKILL->GetName();

		CString strName;
		strName.Format( "(%d~%d) %s", sID.wMainID, sID.wSubID, strSkill.c_str() );
		SetWin_Text( this, IDC_PETEDITSKILLSLOT_EDIT_SLOT_1, strName.GetString() );
	}
}

void CPetEditPageSkillSlot::OnBnClickedPeteditskillslotButtonSet0()
{
	if( !m_pData )	return;

	CDlgPetSkillList dlg( m_pData, this );
	if ( dlg.DoModal() == IDOK )
	{
		m_pData->m_sActiveSkillID_A = dlg.m_SELECTED;
		DataShow();
	}
}

void CPetEditPageSkillSlot::OnBnClickedPeteditskillslotButtonReset0()
{
	if( !m_pData )	return;

	m_pData->m_sActiveSkillID_A = NATIVEID_NULL();
	DataShow();
}

void CPetEditPageSkillSlot::OnBnClickedPeteditskillslotButtonSet1()
{
	CDlgPetSkillList dlg( m_pData, this );
	if ( dlg.DoModal() == IDOK )
	{
		m_pData->m_sActiveSkillID_B = dlg.m_SELECTED;
		DataShow();
	}
}

void CPetEditPageSkillSlot::OnBnClickedPeteditskillslotButtonReset1()
{
	if( !m_pData )	return;

	m_pData->m_sActiveSkillID_B = NATIVEID_NULL();
	DataShow();
}
