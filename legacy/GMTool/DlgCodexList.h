#pragma once


// CDlgCodexList dialog

class CDlgCodexList : public CDialog
{
	DECLARE_DYNAMIC(CDlgCodexList)

public:
	CDlgCodexList(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgCodexList();

// Dialog Data
	enum { IDD = IDD_DLG_CODEX_LIST };

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
