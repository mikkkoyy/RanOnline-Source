#pragma once

#include "GLDefine.h"
// CDlgMap dialog

class CDlgMap : public CDialog
{
	DECLARE_DYNAMIC(CDlgMap)

public:
	CDlgMap(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgMap();

// Dialog Data
	enum { IDD = IDD_DLG_MAP };

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
	void	ShowData();
	void	SelectionReset();
	void	SearchData();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonSearch();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnLbnSelchangeListSelect();
};
