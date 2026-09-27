#ifndef _GMTOOL_PETEDITTAB_H_
#define _GMTOOL_PETEDITTAB_H_

struct GLPET;
/////////////////////////////////////////////////////////////////////////////
// CPetEditTab

#include "GMToolTab.h"

#include "PetEditPageBasic.h"
#include "PetEditPageInven.h"
#include "PetEditPageSkill.h"
#include "PetEditPageSkillSlot.h"

class CPetEditTab : public CPropertySheet
{
	DECLARE_DYNAMIC(CPetEditTab)

public:
	CWnd*		m_pWndParent;
	GLPET*		m_pData;

public:
	CPetEditPageBasic		m_PageBasic;
	CPetEditPageInven		m_PageInven;
	CPetEditPageSkill		m_PageSkill;
	CPetEditPageSkillSlot	m_PageSkillSlot;

public:	

// Construction
public:
	CPetEditTab(CWnd* pParentWnd = NULL);
	CPetEditTab( GLPET* pData, CRect& rect, LOGFONT logfont, CWnd* pParentWnd = NULL );
	virtual ~CPetEditTab();

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
	BOOL	SaveInfo();

// Generated message map functions
protected:
	afx_msg HBRUSH CtlColor(CDC* pDc, UINT uCtlColor);	
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

#endif	// _GMTOOL_PETEDITTAB_H_