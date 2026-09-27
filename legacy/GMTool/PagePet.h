#pragma once

class	CsheetWithTab;
// CPagePet dialog

class CPagePet : public CPropertyPage
{
	DECLARE_DYNAMIC(CPagePet)

public:
	CPagePet(LOGFONT logfont);
	virtual ~CPagePet();

// Dialog Data
	enum { IDD = IDD_MAINPAGE_PET };
	enum { MAX_PAGEPET_MODAL_DIALOG = 10 };

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
	CDialog*	m_pToolDialog[MAX_PAGEPET_MODAL_DIALOG];

public:
	int		DialogNewID();
	void	DialogDeleteID( int nDialogID );
	int		DialogGetTotalActive();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedPetPageButtonSearch();
	afx_msg void OnNMDblclkListPet(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedPetPageButtonEdit();
};
