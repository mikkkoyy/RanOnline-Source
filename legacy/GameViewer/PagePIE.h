#pragma once

class	DxPieceEdit;
class	CsheetWithTab;
// CPagePIE dialog

class CPagePIE : public CPropertyPage
{
	DECLARE_DYNAMIC(CPagePIE)

public:
	CPagePIE(LOGFONT logfont);
	virtual ~CPagePIE();

// Dialog Data
	enum { IDD = IDD_PAGE_PIE };

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
	void	DataReset();
	void	DataShow();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonPieLoad();
	afx_msg void OnBnClickedButtonPieSave();
};
