#pragma once

class	CsheetWithTab;

#include "DxAttBoneData.h"
#include "DxAttBoneRender.h"

// CPageABF dialog

class CPageABF : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageABF)

public:
	CPageABF(LOGFONT logfont);
	virtual ~CPageABF();

// Dialog Data
	enum { IDD = IDD_PAGE_ABF };

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
	DxAttBoneRender*	m_pSkin;
	DxAttBoneData*		m_pData;

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
	afx_msg void OnBnClickedButtonAbfNew();
	afx_msg void OnBnClickedButtonAbfLoad();
	afx_msg void OnBnClickedButtonAbfSave();
	afx_msg void OnBnClickedButtonAbfPieceCps();
	afx_msg void OnBnClickedButtonAbfPieceReset();
	afx_msg void OnBnClickedButtonAbfAnimPlay();
	afx_msg void OnBnClickedButtonAbfAnimStop();
	afx_msg void OnBnClickedButtonAbfAnimAddcfg();
	afx_msg void OnBnClickedButtonAbfAnimDelete();
	afx_msg void OnBnClickedButtonAbfAnimClear();
	afx_msg void OnBnClickedButtonAbfSkeleton();
	afx_msg void OnBnClickedButtonAbfApply();
	afx_msg void OnNMDblclkListAbfAnim(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonAniInsertAbf();
};
