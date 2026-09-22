#pragma once


// CDlgUserEdit dialog
struct SGMTOOL_DATA_USER;

class CDlgUserEdit : public CDialog
{
	DECLARE_DYNAMIC(CDlgUserEdit)

public:
	CDlgUserEdit(CWnd* pParent = NULL);   // standard constructor
	CDlgUserEdit(int nDialogID, DWORD dwID, std::string strNAME, CWnd* pParent = NULL);
	virtual ~CDlgUserEdit();

	BOOL Create(CWnd * pWnd){ 
		return CDialog::Create( CDlgUserEdit::IDD, pWnd );
	}

// Dialog Data
	enum { IDD = IDD_DLG_USER_EDIT };

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
	SGMTOOL_DATA_USER*	m_pData;

private:
	void	DataDefault();
	void	DataShow();
	BOOL	DataSave();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedUsereditButtonMd5();
	afx_msg void OnBnClickedUsereditButtonMd52();
	afx_msg void OnBnClickedUsereditButtonUserblockSet();
	afx_msg void OnBnClickedUsereditButtonUserblockReset();
	afx_msg void OnBnClickedUsereditButtonChatblockSet();
	afx_msg void OnBnClickedUsereditButtonChatblockReset();
	afx_msg void OnBnClickedUsereditButtonPremiumdateSet();
	afx_msg void OnBnClickedUsereditButtonPremiumdateReset();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk2();

};
