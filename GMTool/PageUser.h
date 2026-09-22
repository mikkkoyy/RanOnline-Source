#pragma once

class	CsheetWithTab;
// CPageUser dialog

class CPageUser : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageUser)

public:
	CPageUser(LOGFONT logfont);
	virtual ~CPageUser();

// Dialog Data
	enum { IDD = IDD_MAINPAGE_USER };
	enum { MAX_PAGECHAR_MODAL_DIALOG = 15 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

public:
	CListCtrl	m_List;

public:
	CDialog*	m_pToolDialog[MAX_PAGECHAR_MODAL_DIALOG];

public:
	BOOL	OnlineCheck( DWORD dwUserNum, std::string strUserName );

public:
	int		DialogNewID();
	void	DialogDeleteID( int nDialogID );
	int		DialogGetTotalActive();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedUserPageButtonSearch();
	afx_msg void OnBnClickedUserPageButtonNew();
	afx_msg void OnBnClickedUserPageButtonBlock();
	afx_msg void OnBnClickedUserPageButtonUnblock();
	afx_msg void OnBnClickedUserPageButtonChatblock();
	afx_msg void OnBnClickedUserPageButtonChatunblock();
	afx_msg void OnBnClickedUserPageButtonLoglogin();
	afx_msg void OnBnClickedUserPageButtonCharacters();
	afx_msg void OnBnClickedUserPageButtonSetlogout();
	afx_msg void OnBnClickedUserPageButtonEdit();
	afx_msg void OnNMDblclkListUser(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedUserPageButtonItembank();
};
