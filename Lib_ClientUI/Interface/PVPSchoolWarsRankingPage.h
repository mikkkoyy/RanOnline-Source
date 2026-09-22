#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CPVPSchoolWarsRankingPageSlot;
class CBasicScrollBarEx;
class CBasicTextBox;

class CPVPSchoolWarsRankingPage : public CUIGroup
{
private:
	enum
	{
		SCHOOLWARS_RANKING_PAGE_SCROLLBAR = NO_ID + 1,
	};

	enum
	{
		SCHOOLWARS_RANKING_PAGE_MAX_SLOT = 10,
	};

public:
	CPVPSchoolWarsRankingPage ();
	virtual	~CPVPSchoolWarsRankingPage ();

public:
	void	CreateSubControl ( int nType );
	CBasicTextBox*	CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign );

public:
	CBasicTextBox*	m_pTextRank;
	CBasicTextBox*	m_pTextSchool;
	CBasicTextBox*	m_pTextClass;
	CBasicTextBox*	m_pTextName;
	CBasicTextBox*	m_pTextKill;
	CBasicTextBox*	m_pTextDeath;
	CBasicTextBox*	m_pTextResu;
	CBasicTextBox*	m_pTextHeal;
	CBasicTextBox*	m_pTextScore;
	CBasicTextBox*	m_pTextPoint;

	CPVPSchoolWarsRankingPageSlot*		m_pSlotRank[SCHOOLWARS_RANKING_PAGE_MAX_SLOT];
	CPVPSchoolWarsRankingPageSlot*		m_pSlotRankSelf;
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