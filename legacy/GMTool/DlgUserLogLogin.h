#pragma once


// CDlgUserLogLogin dialog

class CDlgUserLogLogin : public CDialog
{
	DECLARE_DYNAMIC(CDlgUserLogLogin)

public:
	CDlgUserLogLogin(CWnd* pParent = NULL);   // standard constructor
	CDlgUserLogLogin(int nDialogID, DWORD dwID, std::string strNAME, CWnd* pParent = NULL);
	virtual ~CDlgUserLogLogin();

	BOOL Create(CWnd * pWnd){ 
		return CDialog::Create( CDlgUserLogLogin::IDD, pWnd );
	}

// Dialog Data
	enum { IDD = IDD_DLG_USER_LOGLOGIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

public:
	CWnd*		m_pParentWnd;

private:
	int			m_nDialogID;
	DWORD		m_dwID;
	std::string m_strNAME;

public:
	CListCtrl	m_List;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedUserlogloginButtonReadAll();
	afx_msg void OnBnClickedUserlogloginButtonReadIp();
	afx_msg void OnBnClickedUserlogloginButtonClear();
	afx_msg void OnBnClickedCancel();
};
