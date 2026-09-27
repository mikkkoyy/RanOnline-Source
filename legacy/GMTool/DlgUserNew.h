#pragma once


// CDlgUserNew dialog

class CDlgUserNew : public CDialog
{
	DECLARE_DYNAMIC(CDlgUserNew)

public:
	CDlgUserNew(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgUserNew();


// Dialog Data
	enum { IDD = IDD_DLG_USER_NEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	BOOL UseIDCheck( std::string strName );

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedUsernewButtonMd51();
	afx_msg void OnBnClickedUsernewButtonMd52();
	afx_msg void OnBnClickedUsernewButtonUsercheck();
};
