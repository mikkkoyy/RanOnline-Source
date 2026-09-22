#pragma once

#include "GLCharData.h"
// CDlgSkillList dialog

class CDlgSkillList : public CDialog
{
	DECLARE_DYNAMIC(CDlgSkillList)

public:
	CDlgSkillList(CWnd* pParent = NULL);   // standard constructor
	CDlgSkillList( SCHARDATA2* pData, CWnd* pParent = NULL );
	virtual ~CDlgSkillList();

// Dialog Data
	enum { IDD = IDD_DLG_SKILL_LIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CListCtrl	m_List;

public:
	SCHARDATA2*		m_pData;
	std::vector<SCHARSKILL> m_vecData;
	std::vector<SCHARSKILL> m_vecResult;
	SNATIVEID	m_SELECTED;

public:
	void	ReadData();
	void	ShowSkills();
	void	SelectionReset();
	void	SearchData();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonSearch();
	afx_msg void OnLvnItemchangedSkillList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
