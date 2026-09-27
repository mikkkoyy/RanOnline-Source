#pragma once

#include "NpcDialogueSet.h"
// CDlgTalk dialog

class CDlgTalk : public CDialog
{
	DECLARE_DYNAMIC(CDlgTalk)

public:
	CDlgTalk(CWnd* pParent = NULL);   // standard constructor
	CDlgTalk(CNpcDialogueSet* pNpcDialogueSet, CWnd* pParent = NULL); 
	virtual ~CDlgTalk();

// Dialog Data
	enum { IDD = IDD_DIALOG_TALK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	CListBox	m_listNpcTalk;
	DWORD		m_dwNpcID;
	CNpcDialogueSet*	m_pNpcDialogueSet;

public:
	void	SetDialogueSet( CNpcDialogueSet* pNpcDialogueSet ) { m_pNpcDialogueSet = pNpcDialogueSet; }

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnLbnSelchangeListSelect();
};
