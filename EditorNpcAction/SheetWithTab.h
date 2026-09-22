#ifndef __SHEETWITHTAB_H_
#define __SHEETWITHTAB_H_

/////////////////////////////////////////////////////////////////////////////
// CsheetWithTab

#include "PageTable.h"
#include "PageAction.h"
#include "PageAnswer.h"
#include "PageTalk.h"
#include "PageCondition.h"

class CNpcDialogueSet;

enum EDITSHEET
{
	PAGE_TABLE		= 0,
	PAGE_ACTION		= 1,
	PAGE_ANSWER		= 2,
	PAGE_TALK		= 3,
	PAGE_CONDITION	= 4,
	EDITSHEET		= 5
};

enum { CDF_CENTER, CDF_TOPLEFT, CDF_NONE };

extern CString g_strEditSheet[EDITSHEET];

class CsheetWithTab : public CPropertySheet
{
	DECLARE_DYNAMIC(CsheetWithTab)

public:
	CWnd*				m_pWndParent;

public:
	CPageTable		m_PageTable;
	CPageAction		m_PageAction;
	CPageAnswer		m_PageAnswer;
	CPageTalk		m_PageTalk;
	CPageCondition	m_PageCondition;

public:
	CNpcDialogueSet* GetDialogueSet ();

public:	
	void ActiveActionTable ();
	void ActiveActionPage  ( CNpcDialogue* pNpcDialogue );
	void ActiveActionAnswer ( CNpcDialogueSet *pNpcDialogueSet=NULL, CNpcDialogueCase* pDialogueCase=NULL );	
	void ActiveActionTalk ( int nCallPage, CNpcDialogueSet* pNpcDialogueSet, CNpcTalkControl* pTalkControl, BOOL bModify, SNpcTalk* pTalk );
	void ActiveActionCondition ( int nCallPage, SNpcTalkCondition* pConditon );

// Construction
public:
	CsheetWithTab(CWnd* pParentWnd = NULL);
	CsheetWithTab(CRect& rect, LOGFONT logfont, CWnd* pParentWnd = NULL );
	virtual ~CsheetWithTab();

// Attributes
public:
	CRect		m_Rect;
	BOOL		m_bNoTabs;
	int			m_iDistanceFromTop;
	CFont*		m_pFont;

// Get/Set
public:
   void NoTabs() { m_bNoTabs = TRUE; }
   void SetDistanceFromTop( int iDistance ) { m_iDistanceFromTop = iDistance; }
   void ChangeDialogFont(CWnd* pWnd, CFont* pFont, int nFlag);

public:
	virtual BOOL OnInitDialog();

// Generated message map functions
protected:
	afx_msg HBRUSH CtlColor(CDC* pDc, UINT uCtlColor);	
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

#endif	// __SHEETWITHTAB_H_