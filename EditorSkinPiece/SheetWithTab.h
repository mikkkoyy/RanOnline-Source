#ifndef __SHEETWITHTAB_H_
#define __SHEETWITHTAB_H_

/////////////////////////////////////////////////////////////////////////////
// CsheetWithTab


#include "PageMain.h"
#include "PageEff_00_Single.h"
#include "PageEff_01_Blur.h"
#include "PageEff_02_CloneBlur.h"
#include "PageEff_04_Shock.h"
#include "PageEff_06_Arrow.h"
#include "PageEff_08_Level.h"
#include "PageEff_10_Alpha.h"
#include "PageEff_11_Neon.h"
#include "PageEff_12_Mark.h"
#include "PageEff_13_NoAlpha.h"
#include "PageEff_14_Reflection2.h"
#include "PageEff_15_Ambient.h"
#include "PageEff_17_MultiTex.h"
#include "PageEff_18_Ghosting.h"
#include "PageEff_19_Specular2.h"
#include "PageEff_20_Toon.h"
#include "PageEff_21_TexDiff.h"
#include "PageEff_22_Particle.h"
#include "PageEff_23_BonePos.h"
#include "PageEff_24_BoneList.h"
#include "PageEff_25_UserColor.h"
#include "PageEff_26_Normal.h"
#include "PageEff_27_Line2Bone.h"
#include "PageEff_28_AroundEff.h"


enum 
{ 
	CDF_CENTER  = 0,
	CDF_TOPLEFT = 1,
	CDF_NONE	= 2,
};

enum 
{ 
	SKINPIECEEDIT_PAGE_MAIN				= 0,
	SKINPIECEEDIT_PAGE_EFF_00_SINGLE	= 1,
	SKINPIECEEDIT_PAGE_EFF_01_BLUR		= 2,
	SKINPIECEEDIT_PAGE_EFF_02_CLONEBLUR	= 3,
	SKINPIECEEDIT_PAGE_EFF_04_SHOCK		= 4,
	SKINPIECEEDIT_PAGE_EFF_06_ARROW		= 5,
	SKINPIECEEDIT_PAGE_EFF_08_LEVEL		= 6,
	SKINPIECEEDIT_PAGE_EFF_10_ALPHA		= 7,
	SKINPIECEEDIT_PAGE_EFF_11_NEON		= 8,
	SKINPIECEEDIT_PAGE_EFF_12_MARK		= 9,
	SKINPIECEEDIT_PAGE_EFF_13_NOALPHA	= 10,
	SKINPIECEEDIT_PAGE_EFF_14_REFLECTION2	= 11,
	SKINPIECEEDIT_PAGE_EFF_15_AMBIENT	= 12,
	SKINPIECEEDIT_PAGE_EFF_17_MULTITEX	= 13,
	SKINPIECEEDIT_PAGE_EFF_18_GHOSTING	= 14,
	SKINPIECEEDIT_PAGE_EFF_19_SPECULAR2	= 15,
	SKINPIECEEDIT_PAGE_EFF_20_TOON		= 16,
	SKINPIECEEDIT_PAGE_EFF_21_TEXDIFF	= 17,
	SKINPIECEEDIT_PAGE_EFF_22_PARTICLE	= 18,
	SKINPIECEEDIT_PAGE_EFF_23_BONEPOS	= 19,
	SKINPIECEEDIT_PAGE_EFF_24_BONELIST	= 20,
	SKINPIECEEDIT_PAGE_EFF_25_USERCOLOR	= 21,
	SKINPIECEEDIT_PAGE_EFF_26_NORMAL	= 22,
	SKINPIECEEDIT_PAGE_EFF_27_LINE2BONE	= 23,
	SKINPIECEEDIT_PAGE_EFF_28_AROUNDEFF	= 24,
};

class CsheetWithTab : public CPropertySheet
{
	DECLARE_DYNAMIC(CsheetWithTab)

public:
	CWnd*		m_pWndParent;

public:
	CPageMain					m_PageMain;
	CPageEff_00_Single			m_PageEff_00_Single;
	CPageEff_01_Blur			m_PageEff_01_Blur;
	CPageEff_02_CloneBlur		m_PageEff_02_CloneBlur;
	CPageEff_04_Shock			m_PageEff_04_Shock;
	CPageEff_06_Arrow			m_PageEff_06_Arrow;
	CPageEff_08_Level			m_PageEff_08_Level;
	CPageEff_10_Alpha			m_PageEff_10_Alpha;
	CPageEff_11_Neon			m_PageEff_11_Neon;
	CPageEff_12_Mark			m_PageEff_12_Mark;
	CPageEff_13_NoAlpha			m_PageEff_13_NoAlpha;
	CPageEff_14_Reflection2		m_PageEff_14_Reflection2;
	CPageEff_15_Ambient			m_PageEff_15_Ambient;
	CPageEff_17_MultiTex		m_PageEff_17_MultiTex;
	CPageEff_18_Ghosting		m_PageEff_18_Ghosting;
	CPageEff_19_Specular2		m_PageEff_19_Specular2;
	CPageEff_20_Toon			m_PageEff_20_Toon;
	CPageEff_21_TexDiff			m_PageEff_21_TexDiff;
	CPageEff_22_Particle		m_PageEff_22_Particle;
	CPageEff_23_BonePos			m_PageEff_23_BonePos;
	CPageEff_24_BoneList		m_PageEff_24_BoneList;
	CPageEff_25_UserColor		m_PageEff_25_UserColor;
	CPageEff_26_Normal			m_PageEff_26_Normal;
	CPageEff_27_Line2Bone		m_PageEff_27_Line2Bone;
	CPageEff_28_AroundEff		m_PageEff_28_AroundEff;

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
	void	ShowMain();
	void	EffEdit( DxSkinPiece* pPiece, DxEffChar* pEff, BOOL bADD = FALSE );


// Generated message map functions
protected:
	afx_msg HBRUSH CtlColor(CDC* pDc, UINT uCtlColor);	
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

#endif	// __SHEETWITHTAB_H_