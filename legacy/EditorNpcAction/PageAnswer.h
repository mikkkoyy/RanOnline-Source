#pragma once

#include "NpcDialogue.h"
#include "NpcDialogueCase.h"
#include "NpcTalkControl.h"

// CPageAnswer dialog
class	CsheetWithTab;
class	CNpcDialogueSet;

class CPageAnswer : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageAnswer)

public:
	CPageAnswer( LOGFONT logfont );
	virtual ~CPageAnswer();

// Dialog Data
	enum { IDD = IDD_PAGE_ANSWER };

protected:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;

	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

public:
	CNpcTalkControl		m_DummyTalkControl;
	CNpcDialogueCase	m_DummyDialogueCase;
	CNpcDialogueSet*	m_pNpcDialogueSet;
	CNpcDialogueCase*	m_pOrgDialogueCase;
	CListCtrl m_ctrlAnswer;

public:
	void	SetDialogueCase ( CNpcDialogueSet *pNpcDialogueSet, CNpcDialogueCase* pDialogueCase );
	void	DataShow();
	void	DataSave();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedButtonAnswerAdd();
	afx_msg void OnBnClickedButtonAnswerEdit();
	afx_msg void OnBnClickedButtonAnswerCopy();
	afx_msg void OnBnClickedButtonAnswerDelete();
	afx_msg void OnBnClickedButtonAnswerDeleteall();
	afx_msg void OnBnClickedButtonAnswerUp();
	afx_msg void OnBnClickedButtonAnswerDown();
	afx_msg void OnBnClickedButtonConditionEdit();
	afx_msg void OnBnClickedButtonConditionDelete();
	afx_msg void OnNMDblclkListAnswers(NMHDR *pNMHDR, LRESULT *pResult);
};
