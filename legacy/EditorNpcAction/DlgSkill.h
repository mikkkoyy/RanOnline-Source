#pragma once

#include "GLSkill.h"
#include "GLDefine.h"
// CDlgSkill dialog

class CDlgSkill : public CDialog
{
	DECLARE_DYNAMIC(CDlgSkill)

public:
	CDlgSkill(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSkill();

// Dialog Data
	enum { IDD = IDD_DIALOG_SKILL };

public:
	CListBox	m_listSELECT;
	SNATIVEID	m_SELECTED;
	WORD		m_wSELECTEDLEVEL;

	std::vector<SNATIVEID> m_vecData;
	std::vector<SNATIVEID> m_vecResult;

public:
	void	ReadData();
	void	ShowSkills();
	void	SelectionReset();
	void	SearchData();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnLbnSelchangeListSelect();
	afx_msg void OnCbnSelchangeComboLevel();
	afx_msg void OnBnClickedButtonSearch();
};
