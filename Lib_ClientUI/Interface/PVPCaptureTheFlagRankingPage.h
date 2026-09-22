/*!
 * \file PVPCaptureTheFlagRankingPage.h
 *
 * \author Juver
 * \date February 2018
 *
 * 
 */

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CPVPCaptureTheFlagRankingPageSlot;
class CBasicScrollBarEx;
class CBasicTextBox;

class CPVPCaptureTheFlagRankingPage : public CUIGroup
{
private:
	enum
	{
		CAPTURE_THE_FLAG_RANKING_PAGE_SCROLLBAR = NO_ID + 1,
	};

	enum
	{
		CAPTURE_THE_FLAG_RANKING_PAGE_MAX_SLOT = 10,
	};

public:
	CPVPCaptureTheFlagRankingPage ();
	virtual	~CPVPCaptureTheFlagRankingPage ();

public:
	void	CreateSubControl ( int nType );
	CBasicTextBox*	CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign );

public:
	CBasicTextBox*	m_pTextRank;
	CBasicTextBox*	m_pTextResult;
	CBasicTextBox*	m_pTextTeam;
	CBasicTextBox*	m_pTextSchool;
	CBasicTextBox*	m_pTextClass;
	CBasicTextBox*	m_pTextName;
	CBasicTextBox*	m_pTextFlag;
	CBasicTextBox*	m_pTextKill;
	CBasicTextBox*	m_pTextDeath;
	CBasicTextBox*	m_pTextResu;
	CBasicTextBox*	m_pTextHeal;
	CBasicTextBox*	m_pTextScore;
	CBasicTextBox*	m_pTextPoint;

	CPVPCaptureTheFlagRankingPageSlot*		m_pSlotRank[CAPTURE_THE_FLAG_RANKING_PAGE_MAX_SLOT];
	CPVPCaptureTheFlagRankingPageSlot*		m_pSlotRankSelf;
	CBasicScrollBarEx*				m_pScrollBar;

public:
	int		m_nType;
	int		m_nStart;
	int		m_nTotal;

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

public:
	void	RefreshRanking();
	void	RenderView();

};