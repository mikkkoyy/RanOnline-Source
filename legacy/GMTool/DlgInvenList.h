#pragma once

#include "GLCharData.h"
// CDlgInvenList dialog

class CDlgInvenList : public CDialog
{
	DECLARE_DYNAMIC(CDlgInvenList)

public:
	CDlgInvenList(CWnd* pParent = NULL);   // standard constructor
	CDlgInvenList( SCHARDATA2* pData, CWnd* pParent = NULL );
	virtual ~CDlgInvenList();

// Dialog Data
	enum { IDD = IDD_DLG_INVEN_LIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CListCtrl	m_List;

public:
	SCHARDATA2*		m_pData;
	std::vector<SINVENITEM> m_vecData;
	std::vector<SINVENITEM> m_vecResult;
	SINVENITEM	m_SELECTED;

public:
	void	ReadData();
	void	ShowData();
	void	SelectionReset();
	void	SearchData();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonSearch();
	afx_msg void OnLvnItemchangedInvenList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
