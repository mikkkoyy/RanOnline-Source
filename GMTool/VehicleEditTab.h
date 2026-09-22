#ifndef _GMTOOL_VEHICLEEDITTAB_H_
#define _GMTOOL_VEHICLEEDITTAB_H_

#include <map>
struct GLVEHICLE;

/////////////////////////////////////////////////////////////////////////////
// CVehicleEditTab

#include "GMToolTab.h"

#include "VehicleEditPageBasic.h"
#include "VehicleEditPageInven.h"
#include "VehicleEditPageColor.h" /*bike color , Juver, 2017/11/13 */

class CVehicleEditTab : public CPropertySheet
{
	DECLARE_DYNAMIC(CVehicleEditTab)

public:
	CWnd*		m_pWndParent;
	GLVEHICLE*	m_pData;

public:
	CVehicleEditPageBasic		m_PageBasic;
	CVehicleEditPageInven		m_PageInven;
	CVehicleEditPageColor		m_PageColor; /*bike color , Juver, 2017/11/13 */

public:	

// Construction
public:
	CVehicleEditTab(CWnd* pParentWnd = NULL);
	CVehicleEditTab( GLVEHICLE* pData, CRect& rect, LOGFONT logfont, CWnd* pParentWnd = NULL );
	virtual ~CVehicleEditTab();

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

#endif	// _GMTOOL_VEHICLEEDITTAB_H_