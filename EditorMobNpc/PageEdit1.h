#pragma once

#include "GLCrowData.h"

class	CsheetWithTab;
// CPageEdit1 dialog

class CPageEdit1 : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEdit1)

public:
	CPageEdit1(LOGFONT logfont);
	virtual ~CPageEdit1();

// Dialog Data
	enum { IDD = IDD_EDIT1 };

private:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	PCROWDATA	m_pDummyCrow;
	PCROWDATA	m_pCrow;

public:
	BOOL	CrowSet ( PCROWDATA pCROW );
	void	CrowClear();

	BOOL	DataDefault();
	BOOL	DataShow();
	BOOL	DataSave();

public:
	void	InitMobLink();
	void	InitGenItem();

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonPageNext();
	afx_msg void OnBnClickedButtonPageCancel();
	afx_msg void OnBnClickedButtonPageSave();
	afx_msg void OnBnClickedButtonSkinfile();
	afx_msg void OnBnClickedButtonTalkfile();
	afx_msg void OnBnClickedButtonBirtheffect();
	afx_msg void OnBnClickedButtonDeatheffect();
	afx_msg void OnBnClickedButtonBloweffect();
	afx_msg void OnBnClickedButtonCrowsale00();
	afx_msg void OnBnClickedButtonCrowsale01();
	afx_msg void OnBnClickedButtonCrowsale02();
	afx_msg void OnBnClickedButtonItemgen();
	afx_msg void OnBnClickedButtonItemgenq();
	afx_msg void OnBnClickedButtonMoblink();
	afx_msg void OnBnClickedButtonGenitem();
	afx_msg void OnEnChangeEditMoblinkMid();
	afx_msg void OnEnChangeEditMoblinkSid();
	afx_msg void OnEnChangeEditGenItemMid();
	afx_msg void OnEnChangeEditGenItemSid();
	afx_msg void OnBnClickedButtonNpcshop();
	afx_msg void OnBnClickedButtonNpcItemExchange();
};
