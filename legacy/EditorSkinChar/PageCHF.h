#pragma once

class	CsheetWithTab;

#include "DxSkinCharData.h"
#include "DxSkinChar.h"

// CPageCHF dialog

class CPageCHF : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageCHF)

public:
	CPageCHF(LOGFONT logfont);
	virtual ~CPageCHF();

// Dialog Data
	enum { IDD = IDD_PAGE_CHF };

private:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

public:
	void	ResetTool();

//page specific 
public:
	DxSkinChar*			m_pSkin;
	DxSkinCharData*		m_pData;

public:
	CListCtrl		m_List_Anim;
	CListCtrl		m_List_Piece;

public:
	void	DataShow();
	void	DataShowAnim();
	void	DataShowPiece();
	void	ResetAnimation();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonChfNew();
	afx_msg void OnBnClickedButtonChfLoad();
	afx_msg void OnBnClickedButtonChfSave();
	afx_msg void OnBnClickedButtonChfTest();
	afx_msg void OnBnClickedButtonChfPieceCps();
	afx_msg void OnBnClickedButtonChfPieceReset();
	afx_msg void OnBnClickedButtonChfAnimPlay();
	afx_msg void OnBnClickedButtonChfAnimStop();
	afx_msg void OnBnClickedButtonChfAnimAddcfg();
	afx_msg void OnBnClickedButtonChfAnimDelete();
	afx_msg void OnBnClickedButtonChfAnimClear();
	afx_msg void OnBnClickedButtonChfSkeleton();
	afx_msg void OnBnClickedButtonChfApply();
	afx_msg void OnNMDblclkListChfAnim(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonChfPieceAbl();
	afx_msg void OnBnClickedButtonAniInsertChf();
	afx_msg void OnBnClickedButtonChfPieceVcf();
};
