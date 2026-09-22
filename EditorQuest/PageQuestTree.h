#pragma once

#include "../Lib_Client/G-Logic/GLQuestMan.h"

class	CsheetWithTab;
typedef GLQuestMan::SNODE QUESTNODE;
typedef QUESTNODE*		  PQUESTNODE;

// CPageQuestTree dialog

class CPageQuestTree : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageQuestTree)

public:
	CPageQuestTree( LOGFONT logfont );
	virtual ~CPageQuestTree();

// Dialog Data
	enum { IDD = IDD_PAGE_QUEST_TREE };

protected:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }


private:
	PQUESTNODE	m_pQuestNode;
	BOOL		m_bDummyHasQuest;
	PQUESTNODE	m_pDummyQuestNode;
	HTREEITEM	m_TreeRoot;
	HTREEITEM	m_TreeList[MAXWORD];	//quest has no max limit so lets assume MAXDWORD32 limit

public:
	CTreeCtrl m_ctrlQuestTree;

public:
	BOOL	UpdateTree ();
	void	CleanAllQuest ();

	const char*	GetFormatName ( PQUESTNODE pQuestNode );
	BOOL	UpdateItem ( PQUESTNODE pQuestNode, HTREEITEM hMainItem );
	void	UpdateName ();
	void	ForceSelectOnRightClick ( POINT pt );
	void	RunSelectMenu ( int nSelect );
	int		GetSelectMenu ( POINT pt );

	void		SetSelectItem ( PQUESTNODE pQuestNode );
	PQUESTNODE	GetSelectItem ();

	void		CheckInvalid();
	void		SelectTree( DWORD dwID );
	
private:
	
	BOOL	AddQuest();
	BOOL	DelQuest();
	BOOL	CopyQuest();
	BOOL	PasteQuestNew();
	BOOL	PasteQuestValue();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnNMRClickTree(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
