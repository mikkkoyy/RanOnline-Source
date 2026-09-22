#pragma once


// CDlgUserCharacters dialog

class CDlgUserCharacters : public CDialog
{
	DECLARE_DYNAMIC(CDlgUserCharacters)

public:
	CDlgUserCharacters(CWnd* pParent = NULL);   // standard constructor
	CDlgUserCharacters(int nDialogID, DWORD dwID, std::string strNAME, CWnd* pParent = NULL);
	virtual ~CDlgUserCharacters();

	BOOL Create(CWnd * pWnd){ 
		return CDialog::Create( CDlgUserCharacters::IDD, pWnd );
	}

// Dialog Data
	enum { IDD = IDD_DLG_USER_CHARACTERS };

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
	void	DataShow();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedUsercharactersButtonRead();
};
