#ifndef __SHEETWITHTAB_H_
#define __SHEETWITHTAB_H_

/////////////////////////////////////////////////////////////////////////////
// CsheetWithTab

#include "PageCHF.h"
#include "PageEGP.h"

/*ABL system, Juver, 2017/06/02 */
#include "PageABF.h" 
#include "PageABL.h"

/*vehicle system, Juver, 2017/08/06 */
#include "PageVCF.h"

#include "PagePIE.h"

enum 
{ 
	CDF_CENTER  = 0,
	CDF_TOPLEFT = 1,
	CDF_NONE	= 2,
};

enum 
{
	VIEWER_PAGE_CHF = 0,
	VIEWER_PAGE_ABF = 1,
	VIEWER_PAGE_ABL = 2,
	VIEWER_PAGE_VCF = 3, /*vehicle system, Juver, 2017/08/06 */
	VIEWER_PAGE_EGP = 4,
	VIEWER_PAGE_PIE = 5,
};

class CsheetWithTab : public CPropertySheet
{
	DECLARE_DYNAMIC(CsheetWithTab)

public:
	CWnd*		m_pWndParent;

public:
	CPageCHF		m_PageCHF;

	/*ABL system, Juver, 2017/06/02 */
	CPageABF		m_PageABF;
	CPageABL		m_PageABL;

	/*vehicle system, Juver, 2017/08/06 */
	CPageVCF		m_PageVCF;

	CPageEGP		m_PageEGP;
	CPagePIE		m_PagePIE;


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
	void	ResetData();

// Generated message map functions
protected:
	afx_msg HBRUSH CtlColor(CDC* pDc, UINT uCtlColor);	
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

#endif	// __SHEETWITHTAB_H_