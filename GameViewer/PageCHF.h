#pragma once

class	CsheetWithTab;
// CPageCHF dialog

class CPageCHF : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageCHF)

public:
	CPageCHF(LOGFONT logfont);
	virtual ~CPageCHF();

// Dialog Data
	enum { IDD = IDD_PAGE_CHF };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	CListCtrl		m_List_Anim;
	CListCtrl		m_List_Piece;

public:
	void	LoadFile( std::string strFile );
	void	PlayAnim();
	void	DataReset();
	void	DataShow();
	void	DataShowAnim();
	void	DataShowPiece();

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonChfLoad();
	afx_msg void OnBnClickedButtonChfLoadM();
	afx_msg void OnBnClickedButtonChfLoadW();
	afx_msg void OnBnClickedButtonChfReload();
	afx_msg void OnBnClickedButtonPieceReset();
	afx_msg void OnBnClickedButtonPieceCps();
	afx_msg void OnBnClickedButtonAnimPlay();
	afx_msg void OnNMDblclkListAnim(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonPieceEffskin();
	afx_msg void OnBnClickedButtonAbf();
	afx_msg void OnBnClickedButtonPieceEffskinChf();
	afx_msg void OnBnClickedButtonPieceVcf();
};
