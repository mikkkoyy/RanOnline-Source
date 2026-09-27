/////////////////////////////////////////////////////////////////////////////
//	FileName	: GlobalRankingWindow.h
//	Project		: RanClientUILib
//	Coder		: LG-7

#pragma once

#include "../UIWindowEx.h"
#include "../DxGlobalStage.h"

class CBasicTextButton;
class CGlobalRankingPage;
class CGlobalRankingWindow : public CUIWindowEx
{
private:
	enum
	{
		BUTTON_TAB_0	= ET_CONTROL_NEXT,
		BUTTON_TAB_END	= BUTTON_TAB_0 + GLOBAL_RANKING_SIZE,
	};

private:
	BOOL m_bDelay;

private:
	CBasicTextButton*	m_pButtonTab_[GLOBAL_RANKING_SIZE];
	CUIControl*			m_pWhiteBG;
	CGlobalRankingPage*	m_pPage_[GLOBAL_RANKING_SIZE];

public:
	CGlobalRankingWindow();
	virtual ~CGlobalRankingWindow();

public:
	void CreateSubControl();
	virtual	void Update(int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl);
	virtual	void TranslateUIMessage(UIGUID ControlID, DWORD dwMsg);

private:
	void DefaultPos();
	void SetPage(int nIndex);

public:
	void RESET();
	void SetRankingKill(int emFB, WORD wPart, STOP_RANK_KILL sTopKill, int nIndex);
	void SetRankingRich(WORD wPart, STOP_RANK_RICH sTopRich, int nIndex);
	void SetRankingGuild(WORD wPart, STOP_RANK_GUILD sTopGuild, int nIndex);

	void SetDelay(BOOL bDelay) { m_bDelay = bDelay; }
};