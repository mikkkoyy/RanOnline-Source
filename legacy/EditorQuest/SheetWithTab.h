#ifndef __SHEETWITHTAB_H_
#define __SHEETWITHTAB_H_

/////////////////////////////////////////////////////////////////////////////
// CsheetWithTab

#include "PageQuestTree.h"
#include "PageQuestBasic.h"
#include "PageQuestStep.h"

class CPageQuestTree;
class CPageQuestBasic;
class CPageQuestStep;

enum EDITSHEET
{
	QUEST_TREE_PAGE		= 0,
	QUEST_BASIC_PAGE	= 1,
	QUEST_STEP_PAGE		= 2,
	EDITSHEET			= 3
};

enum { CDF_CENTER, CDF_TOPLEFT, CDF_NONE };

extern CString g_strEditSheet[EDITSHEET];

class CsheetWithTab : public CPropertySheet
{
	DECLARE_DYNAMIC(CsheetWithTab)

public:
	CWnd*				m_pWndParent;

public:
	CPageQuestTree		m_PageQuestTree;
	CPageQuestBasic		m_PageQuestBasic;
	CPageQuestStep		m_PageQuestStep;

public:	
	void ActiveQuestBasicPage ( int CurPage, PQUESTNODE pQuestNode );
	void ActiveQuestStepPage ( int CurPage, PQUESTNODE pQuestNode );
	void ActiveQuestTreePage ();
	void UpdateQuestTreePage ();

	void ClassConvert();

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