#pragma once

#include "GLDefine.h"
// CDlgCrow dialog

class CDlgCrow : public CDialog
{
	DECLARE_DYNAMIC(CDlgCrow)

public:
	CDlgCrow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgCrow();

// Dialog Data
	enum { IDD = IDD_DLG_CROW };

public:
	CListBox	m_listSELECT;
	SNATIVEID	m_SELECTED;

	std::vector<SNATIVEID> m_vecData;
	std::vector<SNATIVEID> m_vecResult;

public:
	void	ReadData();
	void	ShowData();
	void	SelectionReset();
	void	SearchData();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonHelperSearch();
	afx_msg void OnLbnSelchangeListSelect();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
