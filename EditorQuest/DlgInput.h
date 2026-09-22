#pragma once


// CDlgInput dialog

class CDlgInput : public CDialog
{
	DECLARE_DYNAMIC(CDlgInput)

public:
	CDlgInput(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgInput();

// Dialog Data
	enum { IDD = IDD_DIALOG_INPUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	int			m_nNEWID;
	CString		m_strNEWNAME;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonCreateId();
	afx_msg void OnBnClickedButtonCreateName();
	afx_msg void OnBnClickedOk();
	
};
