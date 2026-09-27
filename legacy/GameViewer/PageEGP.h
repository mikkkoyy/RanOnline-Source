#pragma once

class	DxEffSingle;
class	CsheetWithTab;
// CPageEGP dialog

class CPageEGP : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEGP)

public:
	CPageEGP(LOGFONT logfont);
	virtual ~CPageEGP();

// Dialog Data
	enum { IDD = IDD_PAGE_EGP };

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
	CListCtrl		m_List_Effect;

public:
	void	LoadFile( std::string strFile );
	void	DataReset();
	void	DataShow();
	int	ShowEffSingle( int nIndex, DxEffSingle* pSingle );

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonEgpLoad();
	afx_msg void OnBnClickedButtonEgpReload();
	afx_msg void OnBnClickedButtonEgpPlay();
	afx_msg void OnBnClickedButtonEgpSave();
};
