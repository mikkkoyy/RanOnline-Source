#pragma once


// CDlgInputNum dialog

class CDlgInputNum : public CDialog
{
	DECLARE_DYNAMIC(CDlgInputNum)

public:
	CDlgInputNum(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgInputNum();

// Dialog Data
	enum { IDD = IDD_DIALOG_NUM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	int		m_nInputNum;

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedOk();
};
