#ifndef __SHEETWITHTAB_H_
#define __SHEETWITHTAB_H_

/////////////////////////////////////////////////////////////////////////////
// CsheetWithTab

#include "GLLevelFile.h"
#include "GLLandMan.h"
#include "DxLandMan.h"

#include "PageMain.h"
#include "PageBasic.h"
#include "PageCondition.h"
#include "PageCrow.h"
#include "PageGate.h"
#include "PageEffect.h"
#include "PageLandMark.h"
#include "PageEtc.h"

#include "PageEditCrow.h"
#include "PageEditGate.h"
#include "PageEditEffect.h"
#include "PageEditLandMark.h"

enum 
{ 
	CDF_CENTER  = 0,
	CDF_TOPLEFT = 1,
	CDF_NONE	= 2,
};

enum 
{ 
	LEVELEDIT_PAGE_MAIN			= 0,
	LEVELEDIT_PAGE_BASIC		= 1,
	LEVELEDIT_PAGE_CONDITION	= 2,
	LEVELEDIT_PAGE_CROWLIST		= 3,
	LEVELEDIT_PAGE_GATELIST		= 4,
	LEVELEDIT_PAGE_EFFECTLIST	= 5,
	LEVELEDIT_PAGE_LANDMARKLIST	= 6,
	LEVELEDIT_PAGE_ETCHFUNCTION	= 7,
	LEVELEDIT_PAGE_CROWEDIT		= 8,
	LEVELEDIT_PAGE_GATEEDIT		= 9,
	LEVELEDIT_PAGE_EFFECTEDIT	= 10,
	LEVELEDIT_PAGE_LANDMARKEDIT	= 11,
	LEVELEDIT_PAGE_SIZE			= 12,
	LEVELEDIT_PAGE_SELECTION	= 8, //selection
};

class CsheetWithTab : public CPropertySheet
{
	DECLARE_DYNAMIC(CsheetWithTab)

public:
	CWnd*		m_pWndParent;

public:
	CPageMain			m_PageMain;
	CPageBasic			m_PageBasic;
	CPageCondition		m_PageCondition;
	CPageCrow			m_PageCrow;
	CPageGate			m_PageGate;
	CPageEffect			m_PageEff;
	CPageLandMark		m_PageLandMark;
	CPageEtc			m_PageEtc;

	CPageEditCrow		m_PageEditCrow;
	CPageEditGate		m_PageEditGate;
	CPageEditEffect		m_PageEditEffect;
	CPageEditLandMark	m_PageEditLandMark;

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
	void	SelectCrow( std::string strName );
	void	SelectGate( std::string strName );
	void	PageActive( int nPAGE );

	void	PageEditCrow( GLMobSchedule* pEdit );
	void	PageEditGate( PDXLANDGATE pEdit );
	void	PageEditEff( PLANDEFF pEdit );
	void	PageEditLandMark( PLANDMARK pEdit );
	
// Generated message map functions
protected:
	afx_msg HBRUSH CtlColor(CDC* pDc, UINT uCtlColor);	
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

#endif	// __SHEETWITHTAB_H_