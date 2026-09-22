/////////////////////////////////////////////////////////////////////////////
//	FileName	: GlobalRankingPage.h
//	Project		: RanClientUILib
//	Coder		: LG-7

#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "DxGlobalStage.h"

class CBasicLineBox;
class CBasicTextBox;
class CBasicScrollBarEx;
class CGlobalRankingPageSlot;
class CGlobalRankingPage : public CUIGroup
{
private:
	static const int nSTARTLINE;

private:
	enum { SCROLLBAR = NO_ID + 1 };

private:
	int m_nIndex;
	int m_nSize;
	int m_nState;
	int m_nCurPos;

private:
	CBasicLineBox*			m_pLBox;
	CBasicLineBox*			m_pLBoxA_[8][2];
	CBasicTextBox*			m_pText_[8];
	CBasicScrollBarEx*		m_pScrollBar;
	CUIControl*				m_pSlotDummy_[MAX_ONE_VIEW_RANK];
	CGlobalRankingPageSlot*	m_pSlot_[MAX_TOP_RANK];

public:
	CGlobalRankingPage();
	virtual ~CGlobalRankingPage();

public:
	void CreateSubControl(int nIndex);
	virtual void Update(int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl);

private:
	CBasicTextBox* CreateStaticControl(char* szControlKeyword, CD3DFontPar* pFont, int nAlign, const UIGUID& cID = NO_ID);
	void DefaultPos();
	void SetSlotRender(int nStartIndex, int nTotal);
	void ResetAllSlotRender(int nTotal);

public:
	void RESET();
	void SetRankingKill(WORD wPart, STOP_RANK_KILL sTopKill, int nIndex);
	void SetRankingRich(WORD wPart, STOP_RANK_RICH sTopRich, int nIndex);
	void SetRankingGuild(WORD wPart, STOP_RANK_GUILD sTopGuild, int nIndex);
};