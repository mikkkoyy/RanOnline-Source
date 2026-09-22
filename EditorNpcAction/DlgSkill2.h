#pragma once

#include "GLSkill.h"
#include "GLDefine.h"
// CDlgSkill2 dialog

class CDlgSkill2 : public CDialog
{
	DECLARE_DYNAMIC(CDlgSkill2)

public:
	CDlgSkill2(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSkill2();

// Dialog Data
	enum { IDD = IDD_DIALOG_SKILL2 };

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

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnLbnSelchangeListSelect();
	afx_msg void OnBnClickedButtonSearch();
};
