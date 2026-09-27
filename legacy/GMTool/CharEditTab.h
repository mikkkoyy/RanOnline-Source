#ifndef _GMTOOL_CHAREDITTAB_H_
#define _GMTOOL_CHAREDITTAB_H_

struct SCHARDATA2;
/////////////////////////////////////////////////////////////////////////////
// CCharEditTab

#include "GMToolTab.h"

#include "CharEditPageBasic.h"
#include "CharEditPagePutOn.h"
#include "CharEditPageInven.h"
#include "CharEditPageSkills.h"
#include "CharEditPageSlotAction.h"
#include "CharEditPageSlotSkill.h"
#include "CharEditPageQuest.h"
#include "CharEditPageCoolTime.h"
#include "CharEditPageLocker.h"
#include "CharEditPageItemFood.h"
#include "CharEditPageActivity.h"	/*activity system, Juver, 2017/10/23 */
#include "CharEditPageCodex.h"/*Codex System, Jhoniex 2/14/2024 */

class CCharEditTab : public CPropertySheet
{
	DECLARE_DYNAMIC(CCharEditTab)

public:
	CWnd*		m_pWndParent;
	SCHARDATA2* m_pData;

public:
	CCharEditPageBasic		m_PageBasic;
	CCharEditPagePutOn		m_PagePutOn;
	CCharEditPageInven		m_PageInven;
	CCharEditPageSkills		m_PageSkills;
	CCharEditPageSlotAction m_PageSlotAction;
	CCharEditPageSlotSkill	m_PageSlotSkill;
	CCharEditPageQuest		m_PageQuest;
	CCharEditPageCoolTime	m_PageCoolTime;
	CCharEditPageLocker		m_PageLocker;
	CCharEditPageItemFood	m_PageItemFood;
	CCharEditPageActivity	m_PageActivity;  /*activity system, Juver, 2017/10/23 */
	CCharEditPageCodex		m_PageCodex;  /*Codex System, Jhoniex 2/14/2024 */

public:	

// Construction
public:
	CCharEditTab(CWnd* pParentWnd = NULL);
	CCharEditTab( SCHARDATA2* pData, CRect& rect, LOGFONT logfont, CWnd* pParentWnd = NULL );
	virtual ~CCharEditTab();

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

#endif	// _GMTOOL_CHAREDITTAB_H_