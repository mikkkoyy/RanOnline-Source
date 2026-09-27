#pragma once

#include "GLDefine.h"
// CDlgSkill dialog

class CDlgSkill : public CDialog
{
	DECLARE_DYNAMIC(CDlgSkill)

public:
	CDlgSkill(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSkill();

// Dialog Data
	enum { IDD = IDD_DLG_SKILL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CListBox	m_listSELECT;
	SNATIVEID	m_SELECTED;

	std::vector<SNATIVEID> m_vecData;
	std::vector<SNATIVEID> m_vecResult;

public:
	void	ReadData();
	void	ShowSkills();
	void	SelectionReset();
	void	SearchData();
	void	InitSearchType();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonSearch();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnLbnSelchangeListSelect();
	afx_msg void OnCbnSelchangeComboSearch();
};
