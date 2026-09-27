#pragma once

#include "GLLandMan.h"

class	CsheetWithTab;
// CPageEtc dialog

class CPageEtc : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEtc)

public:
	CPageEtc(LOGFONT logfont);
	virtual ~CPageEtc();

// Dialog Data
	enum { IDD = IDD_PAGE_ETCH };

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

protected:
	GLLandMan*		m_pGLLand;

private:
	CListCtrl	m_List;

public:
	void	DataSet( GLLandMan* pLand );
	void	ResetEditor();

public:
	void	InitUI();
	void	DataShow();
	BOOL	DataSave();

	void	EffShow();
	void	JumpTo( D3DXVECTOR2 vMIN, D3DXVECTOR2 vMAX );

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedCheckMobchase();
	afx_msg void OnBnClickedCheckLimittime();
	afx_msg void OnBnClickedCheckControlcam();
	afx_msg void OnBnClickedCheckLandeffect();
	afx_msg void OnBnClickedButtonControlcam();
	afx_msg void OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonEffAdd();
	afx_msg void OnBnClickedButtonEffEdit();
	afx_msg void OnBnClickedButtonEffDelete();
	afx_msg void OnBnClickedButtonEffPosscreen();
	afx_msg void OnBnClickedButtonEffClose();
	afx_msg void OnBnClickedButtonEffSave();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
};
