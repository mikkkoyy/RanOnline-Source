#pragma once

class	CsheetWithTab;
// CPageChar dialog

class CPageChar : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageChar)

public:
	CPageChar(LOGFONT logfont);
	virtual ~CPageChar();

// Dialog Data
	enum { IDD = IDD_MAINPAGE_CHAR };
	enum { MAX_PAGEUSER_MODAL_DIALOG = 10 };

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
	CDialog*	m_pToolDialog[MAX_PAGEUSER_MODAL_DIALOG];

public:
	BOOL	OnlineCheck( DWORD dwChaNum, std::string strChaName );

public:
	int		DialogNewID();
	void	DialogDeleteID( int nDialogID );
	int		DialogGetTotalActive();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedCharPageButtonSearch();
	afx_msg void OnBnClickedCharPageButtonEdit();
	afx_msg void OnNMDblclkListChar(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCharPageButtonDeletedset();
	afx_msg void OnBnClickedCharPageButtonDeletedremove();
	afx_msg void OnBnClickedCharPageButtonCharcopy();
	afx_msg void OnBnClickedCharPageButtonSetoffline();
};
