#ifndef __SHEETWITHTAB_H_
#define __SHEETWITHTAB_H_

/////////////////////////////////////////////////////////////////////////////
// CsheetWithTab


#include "PageCHF.h"

/*ABL system, Juver, 2017/05/30 */
#include "PageABF.h"
#include "PageABL.h"

/*vehicle system, Juver, 2017/08/04 */
#include "PageVCF.h"

enum 
{ 
	CDF_CENTER  = 0,
	CDF_TOPLEFT = 1,
	CDF_NONE	= 2,
};

enum 
{ 
	SKINCHAR_EDIT_PAGE_MAIN		= 0,
};

class CsheetWithTab : public CPropertySheet
{
	DECLARE_DYNAMIC(CsheetWithTab)

public:
	CWnd*		m_pWndParent;

public:
	CPageCHF		m_PageCHF;

	/*ABL system, Juver, 2017/05/30 */
	CPageABF		m_PageABF;
	CPageABL		m_PageABL;
	
	/*vehicle system, Juver, 2017/08/04 */
	CPageVCF		m_PageVCF;
public:	

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

	void	InitPages();
	void	ResetEditor();


// Generated message map functions
protected:
	afx_msg HBRUSH CtlColor(CDC* pDc, UINT uCtlColor);	
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

#endif	// __SHEETWITHTAB_H_