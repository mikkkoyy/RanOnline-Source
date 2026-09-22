#pragma once


// CDlgLogin dialog

class CDlgLogin : public CDialog
{
	DECLARE_DYNAMIC(CDlgLogin)

public:
	CDlgLogin(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgLogin();

// Dialog Data
	enum { IDD = IDD_DLG_LOGIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

private:
	BOOL	DoLogin();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedLoginButtonLogin();
};
