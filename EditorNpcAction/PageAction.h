#pragma once

#include "NpcDialogue.h"

// CPageAction dialog
class	CsheetWithTab;
class	CNpcDialogue;
class	CNpcDialogueSet;

class CPageAction : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageAction)

public:
	CPageAction( LOGFONT logfont );
	virtual ~CPageAction();

// Dialog Data
	enum { IDD = IDD_PAGE_ACTION };

protected:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;

	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

public:
	CNpcDialogue		m_DummyDialogue;
	CNpcDialogue*		m_pDialogue;
	CNpcDialogueSet*	m_pNpcDialogueSet;
	CListCtrl			m_listctrlCase;
	CListCtrl			m_listctrlAnswer;

public:
	void	SetDialogue ( CNpcDialogueSet *pNpcDialogueSet, CNpcDialogue* pDialogue );
	void	UpdateList();
	void	InformationPreview();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnLvnItemchangedListStatement(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonDataNew();
	afx_msg void OnBnClickedButtonDataEdit();
	afx_msg void OnBnClickedButtonDataCopy();
	afx_msg void OnBnClickedButtonDataDelete();
	afx_msg void OnBnClickedButtonDataUp();
	afx_msg void OnBnClickedButtonDataDown();
	afx_msg void OnNMDblclkListStatement(NMHDR *pNMHDR, LRESULT *pResult);
};
