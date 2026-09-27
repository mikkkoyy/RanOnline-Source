#pragma once

class	CsheetWithTab;
// CPageVCF dialog

class CPageVCF : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageVCF)

public:
	CPageVCF(LOGFONT logfont);
	virtual ~CPageVCF();

// Dialog Data
	enum { IDD = IDD_PAGE_VCF };

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
	CListCtrl		m_List_Anim;
	CListCtrl		m_List_Piece;

public:
	void	DataReset();
	void	DataShow();
	void	DataShowAnim();
	void	DataShowPiece();
	void	PlayAnim();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonVcfLoad();
	afx_msg void OnBnClickedButtonPieceReset();
	afx_msg void OnBnClickedButtonPieceCps();
	afx_msg void OnBnClickedButtonPieceEffskin();
	afx_msg void OnBnClickedButtonPieceEffskinNewVcf();
	afx_msg void OnBnClickedButtonAnimPlay();
	afx_msg void OnNMDblclkListAnim(NMHDR *pNMHDR, LRESULT *pResult);
};
