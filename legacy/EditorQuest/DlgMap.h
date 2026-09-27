#pragma once

#include "GLMapList.h"
#include "GLDefine.h"

// CDlgMap dialog

class CDlgMap : public CDialog
{
	DECLARE_DYNAMIC(CDlgMap)

public:
	CDlgMap(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgMap();

// Dialog Data
	enum { IDD = IDD_DIALOG_MAP };

public:
	CListBox	m_listSELECT;
	SNATIVEID	m_SELECTED;
	GLMapList	m_mapList;

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
