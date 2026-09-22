#pragma once


// CDlgActivityList dialog

class CDlgActivityList : public CDialog
{
	DECLARE_DYNAMIC(CDlgActivityList)

public:
	CDlgActivityList(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgActivityList();

// Dialog Data
	enum { IDD = IDD_DLG_ACTIVITY_LIST };

public:
	CListBox	m_listSELECT;
	DWORD		m_dwSelected;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnLbnSelchangeListSelect();
	afx_msg void OnBnClickedOk();
};
