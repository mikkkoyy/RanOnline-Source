#ifndef _GMTOOL_LOCKERTAB_H_
#define _GMTOOL_LOCKERTAB_H_

struct SCHARDATA2;
/////////////////////////////////////////////////////////////////////////////
// CLockerTab

#include "GMToolTab.h"

#include "LockerPageMain.h"
#include "LockerPageItem1.h"
#include "LockerPageItem2.h"
#include "LockerPageItem3.h"
#include "LockerPageItem4.h"
#include "LockerPageItemP.h"


class CLockerTab : public CPropertySheet
{
	DECLARE_DYNAMIC(CLockerTab)

public:
	CWnd*		m_pWndParent;
	SCHARDATA2* m_pData;

public:
	CLockerPageMain		m_PageMain;
	CLockerPageItem1	m_PageItem1;
	CLockerPageItem2	m_PageItem2;
	CLockerPageItem3	m_PageItem3;
	CLockerPageItem4	m_PageItem4;
	CLockerPageItemP	m_PageItemP;


public:	

// Construction
public:
	CLockerTab(CWnd* pParentWnd = NULL);
	CLockerTab( SCHARDATA2* pData, CRect& rect, LOGFONT logfont, CWnd* pParentWnd = NULL );
	virtual ~CLockerTab();

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

// Generated message map functions
protected:
	afx_msg HBRUSH CtlColor(CDC* pDc, UINT uCtlColor);	
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

#endif	// _GMTOOL_LOCKERTAB_H_