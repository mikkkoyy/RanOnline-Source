#pragma once

struct SCHARDATA2;

#include "Logic/GMToolData.h"
#include "Logic/GMToolGlobal.h"
#include "GLCharDefine.h"

// CDlgSkillAdd dialog

class CDlgSkillAdd : public CDialog
{
	DECLARE_DYNAMIC(CDlgSkillAdd)

public:
	CDlgSkillAdd(CWnd* pParent = NULL);   // standard constructor
	CDlgSkillAdd( EMSKILLADD_TYPE emEDIT, EMCHARCLASS emCLASS, CWnd* pParent = NULL );
	virtual ~CDlgSkillAdd();

// Dialog Data
	enum { IDD = IDD_DLG_SKILL_ADD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CListCtrl	m_ListSelection;
	CListCtrl	m_ListSelected;

public:
	EMSKILLADD_TYPE	m_emEditType;
	EMCHARCLASS		m_emClass;
	SGMTOOL_SKILL_DATA_SELECTION_VEC	m_vecSkills;
	SGMTOOL_SKILL_DATA_SELECTION_VEC	m_vecResult;
	SGMTOOL_SKILL_DATA_SELECTED_MAP		m_mapSelect;

public:
	void DataRead();
	void DataShowSelection();
	void DataShowSelected();
	void DataSearch();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedSkilladdButtonSearch();
	afx_msg void OnBnClickedSkilladdButtonSelectedAdd();
	afx_msg void OnBnClickedSkilladdButtonSelectedAddall();
	afx_msg void OnBnClickedSkilladdButtonSelectionRemove();
	afx_msg void OnBnClickedSkilladdButtonSelectionRemoveall();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnNMDblclkSkilladdListSkill(NMHDR *pNMHDR, LRESULT *pResult);
};
