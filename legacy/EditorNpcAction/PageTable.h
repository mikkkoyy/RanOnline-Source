#pragma once

#include "NpcDialogueSet.h"
// CPageTable dialog
class	CsheetWithTab;

class CPageTable : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageTable)

public:
	CPageTable( LOGFONT logfont );
	virtual ~CPageTable();

// Dialog Data
	enum { IDD = IDD_PAGE_TABLE };

protected:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;

	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

public:
	CNpcDialogueSet	m_DialogueSet;
	CListCtrl		m_ListTalk;

public:
	CNpcDialogueSet*	GetDialogueSet();
	void	UpdateItems();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonEdit();
	afx_msg void OnBnClickedButtonCopy();
	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnNMDblclkListTalk(NMHDR *pNMHDR, LRESULT *pResult);
};
