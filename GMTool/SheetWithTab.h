#ifndef _GMTOOL_SHEETWITHTAB_H_
#define _GMTOOL_SHEETWITHTAB_H_

/////////////////////////////////////////////////////////////////////////////
// CsheetWithTab

#include "GMToolTab.h"

#include "PageMain.h"
#include "PageUser.h"
#include "PageChar.h"
#include "PageEquipmentLock.h" /*equipment lock, Juver, 2018/01/14 */
#include "PagePet.h"
#include "PageVehicle.h"
#include "PageItemShop.h"

class CsheetWithTab : public CPropertySheet
{
	DECLARE_DYNAMIC(CsheetWithTab)

public:
	CWnd*		m_pWndParent;

public:
	CPageMain		m_PageMain;
	CPageUser		m_PageUser;
	CPageChar		m_PageChar;
	CPageEquipmentLock	m_PageEquipmentLock; /*equipment lock, Juver, 2018/01/14 */
	CPagePet		m_PagePet;
	CPageVehicle	m_PageVehicle;
	CPageItemShop	m_PageItemShop;
	
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
	BOOL	IsDialogAllFree();

// Generated message map functions
protected:
	afx_msg HBRUSH CtlColor(CDC* pDc, UINT uCtlColor);	
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

#endif	// _GMTOOL_SHEETWITHTAB_H_