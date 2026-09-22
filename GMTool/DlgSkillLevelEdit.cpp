// DlgSkillLevelEdit.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgSkillLevelEdit.h"

#include "GLSkill.h"
#include "GLCharData.h"
#include "EtcFunction.h"
#include "Logic/GMToolGlobal.h"

// CDlgSkillLevelEdit dialog

IMPLEMENT_DYNAMIC(CDlgSkillLevelEdit, CDialog)

CDlgSkillLevelEdit::CDlgSkillLevelEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSkillLevelEdit::IDD, pParent)
	, m_pData(NULL)
{

}

CDlgSkillLevelEdit::CDlgSkillLevelEdit(SCHARSKILL* pData, CWnd* pParent /*= NULL*/)
	: CDialog(CDlgSkillLevelEdit::IDD, pParent)
	, m_pData(NULL)
{
	m_pData = pData;
}

CDlgSkillLevelEdit::~CDlgSkillLevelEdit()
{
}

void CDlgSkillLevelEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgSkillLevelEdit, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgSkillLevelEdit message handlers
BOOL CDlgSkillLevelEdit::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWin_Combo_Init( this, IDC_SKILLLEVELEDIT_COMBO_SKILLLEVEL, COMMENT::SKILL_LEVEL, SKILL::MAX_LEVEL );

	DataShow();

	return TRUE;  
}

void CDlgSkillLevelEdit::DataShow()
{
	if ( !m_pData ){
		return;
	}

	std::string strName = "Unknown Skill";
	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( m_pData->sNativeID );
	if ( pSkill ){
		strName = pSkill->GetName();
	}

	CString strText;
	strText.Format( "[%03d~%03d] %s", m_pData->sNativeID.wMainID, m_pData->sNativeID.wSubID, strName.c_str() );
	SetWin_Text( this, IDC_SKILLLEVELEDIT_EDIT_SKILL, strText.GetString() );

	SetWin_Combo_Sel( this, IDC_SKILLLEVELEDIT_COMBO_SKILLLEVEL, m_pData->wLevel );
}

BOOL CDlgSkillLevelEdit::DataSave()
{
	if ( !m_pData ){
		return FALSE;
	}

	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( m_pData->sNativeID );
	if ( !pSkill ){
		return FALSE;
	}

	WORD wSkillLevel = (WORD)GetWin_Combo_Sel( this, IDC_SKILLLEVELEDIT_COMBO_SKILLLEVEL );
	if( wSkillLevel >= SKILL::MAX_LEVEL ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Skill Level must be between %d ~ %d only", 1, SKILL::MAX_LEVEL -1 );
		return FALSE;
	}

	m_pData->wLevel = wSkillLevel;

	GMTOOL_GLOBAL::SkillLevelCheck( m_pData->wLevel, m_pData->sNativeID );

	return TRUE;
}

void CDlgSkillLevelEdit::OnBnClickedOk()
{
	if ( DataSave() ){
		CDialog::OnOK();
	}
}

void CDlgSkillLevelEdit::OnBnClickedCancel()
{
	CDialog::OnCancel();
}
