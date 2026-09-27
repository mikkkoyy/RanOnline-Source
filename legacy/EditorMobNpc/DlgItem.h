#pragma once

#include "GLItemMan.h"
#include "GLDefine.h"
// CDlgItem dialog

class CDlgItem : public CDialog
{
	DECLARE_DYNAMIC(CDlgItem)

public:
	CDlgItem(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgItem();

// Dialog Data
	enum { IDD = IDD_DIALOG_ITEM };

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
	afx_msg void OnLbnSelchangeListSelect();
	afx_msg void OnBnClickedButtonSearch();
};
