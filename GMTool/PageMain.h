#pragma once

class	CsheetWithTab;
// CPageMain dialog

class CPageMain : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageMain)

public:
	CPageMain(LOGFONT logfont);
	virtual ~CPageMain();

// Dialog Data
	enum { IDD = IDD_MAINPAGE_MAIN };

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
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedMainPageButtonClear();
};
