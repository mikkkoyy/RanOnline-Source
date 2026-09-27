/////////////////////////////////////////////////////////////////////////////
//	FileName	: GlobalRankingPage.cpp
//	Project		: RanClientUILib
//	Coder		: LG-7

#include "stdafx.h"
#include "GlobalRankingPage.h"
#include "GlobalRankingPageSlot.h"

#include "../BasicLineBox.h"
#include "../BasicScrollBarEx.h"
#include "../GameTextControl.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "BasicTextBox.h"
#include "BasicScrollThumbFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int CGlobalRankingPage::nSTARTLINE = 0;

CGlobalRankingPage::CGlobalRankingPage()
	: m_nIndex(-1)
	, m_nSize(-1)
	, m_nState(0)
	, m_nCurPos(-1)

	, m_pLBox(NULL)
	, m_pScrollBar(NULL)
{
	memset(m_pLBoxA_,		NULL, sizeof(m_pLBoxA_));
	memset(m_pText_,		NULL, sizeof(m_pText_));
	memset(m_pSlotDummy_,	NULL, sizeof(m_pSlotDummy_));
	memset(m_pSlot_,		NULL, sizeof(m_pSlot_));
}

CGlobalRankingPage::~CGlobalRankingPage()
{
}

void CGlobalRankingPage::CreateSubControl(int nIndex)
{
	m_nIndex = nIndex;

	switch (m_nIndex)
	{
	case 2:		m_nSize = 7; break;
	default:	m_nSize = 8; break;
	}

	m_pLBox = new CBasicLineBox;
	m_pLBox->CreateSub(this, "BASIC_LINE_BOX_BODY_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE);
	m_pLBox->CreateBaseBoxOuter("BASIC_LINE_BOX_BODY_OUTER");
	RegisterControl(m_pLBox);

	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont(_DEFAULT_FONT, 8, _DEFAULT_FONT_SHADOW_FLAG);
	const int nAlignMid = TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y;

	for (int i = 0; i < m_nSize; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			m_pLBoxA_[i][j] = new CBasicLineBox;
			m_pLBoxA_[i][j]->CreateSub(this, "BASIC_LINE_BOX_BODY_OUTER_BLANK_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE);
			m_pLBoxA_[i][j]->CreateBaseBoxOuterBlankWhite("BASIC_LINE_BOX_BODY_OUTER_BLANK_WHITE");
			RegisterControl(m_pLBoxA_[i][j]);
		}

		m_pText_[i] = CreateStaticControl("BASIC_WINDOW", pFont8, nAlignMid);
	}

	m_pScrollBar = new CBasicScrollBarEx;
	m_pScrollBar->CreateSub(this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, SCROLLBAR);
	m_pScrollBar->CreateBaseScrollBar("GLOBAL_RANKING_WINDOW_SCROLLBAR");
	m_pScrollBar->GetThumbFrame()->SetState(m_nState, MAX_ONE_VIEW_RANK);
	RegisterControl(m_pScrollBar);

	for (int i = 0; i < MAX_ONE_VIEW_RANK; i++)
	{
		m_pSlotDummy_[i] = new CUIControl;
		m_pSlotDummy_[i]->CreateSub(this, "BASIC_WINDOW", UI_FLAG_XSIZE | UI_FLAG_YSIZE);
		RegisterControl(m_pSlotDummy_[i]);
	}

	for (int i = 0; i < MAX_TOP_RANK; i++)
	{
		m_pSlot_[i] = new CGlobalRankingPageSlot;
		m_pSlot_[i]->CreateSub(this, "BASIC_WINDOW", UI_FLAG_XSIZE | UI_FLAG_YSIZE);

		if(m_nIndex == 2)	m_pSlot_[i]->CreateSubControl(m_nIndex, i, TRUE);
		else				m_pSlot_[i]->CreateSubControl(m_nIndex, i, FALSE);

		m_pSlot_[i]->SetVisibleSingle(FALSE);
		RegisterControl(m_pSlot_[i]);
	}

	DefaultPos();
	SetSlotRender(nSTARTLINE, MAX_ONE_VIEW_RANK);
}

void CGlobalRankingPage::Update(int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl)
{
	CUIGroup::Update(x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl);

	if (m_pScrollBar)
	{
		CBasicScrollThumbFrame* pThumbFrame = m_pScrollBar->GetThumbFrame();
		int nTotal = pThumbFrame->GetTotal();

		if (nTotal <= MAX_ONE_VIEW_RANK)
			return;

		const int nViewPerPage = pThumbFrame->GetViewPerPage();

		if (nViewPerPage < nTotal)
		{
			int nCurPos = nSTARTLINE;
			const int nMovableLine = nTotal - nViewPerPage;
			float fPercent = pThumbFrame->GetPercent();
			nCurPos = (int)floor(fPercent * nMovableLine);

			if (nCurPos < nSTARTLINE)
				nCurPos = nSTARTLINE;

			if (m_nCurPos == nCurPos)
				return;

			m_nCurPos = nCurPos;

			ResetAllSlotRender(nTotal);
			SetSlotRender(nCurPos, nCurPos + nViewPerPage);
		}
	}
}

CBasicTextBox* CGlobalRankingPage::CreateStaticControl(char* szControlKeyword, CD3DFontPar* pFont, int nAlign, const UIGUID& cID)
{
	CBasicTextBox* pStaticText = new CBasicTextBox;
	pStaticText->CreateSub(this, szControlKeyword, UI_FLAG_DEFAULT, cID);
	pStaticText->SetFont(pFont);
	pStaticText->SetTextAlign(nAlign);
	RegisterControl(pStaticText);

	return pStaticText;
}

void CGlobalRankingPage::DefaultPos()
{
	UIRECT RcOLD;
	UIRECT RcNEW1;

	UIRECT RcLBoxA_[8][2];
	UIRECT RcLBoxB_[7][2];
	UIRECT RcTextA_[8];
	UIRECT RcTextB_[7];
	UIRECT RcSlotDummy[MAX_ONE_VIEW_RANK];
	UIRECT RcSlot[MAX_ONE_VIEW_RANK];

	RcLBoxA_[0][0] = UIRECT(3.00, 3.00, 49.00, 372.00);
	RcLBoxA_[0][1] = UIRECT(3.00, 3.00, 49.00, 18.00);
	RcLBoxA_[1][0] = UIRECT(51.00, 3.00, 177.00, 372.00);
	RcLBoxA_[1][1] = UIRECT(51.00, 3.00, 177.00, 18.00);
	RcLBoxA_[2][0] = UIRECT(227.00, 3.00, 60.00, 372.00);
	RcLBoxA_[2][1] = UIRECT(227.00, 3.00, 60.00, 18.00);
	RcLBoxA_[3][0] = UIRECT(286.00, 3.00, 60.00, 372.00);
	RcLBoxA_[3][1] = UIRECT(286.00, 3.00, 60.00, 18.00);
	RcLBoxA_[4][0] = UIRECT(345.00, 3.00, 60.00, 372.00);
	RcLBoxA_[4][1] = UIRECT(345.00, 3.00, 60.00, 18.00);
	RcLBoxA_[5][0] = UIRECT(404.00, 3.00, 60.00, 372.00);
	RcLBoxA_[5][1] = UIRECT(404.00, 3.00, 60.00, 18.00);
	RcLBoxA_[6][0] = UIRECT(463.00, 3.00, 60.00, 372.00);
	RcLBoxA_[6][1] = UIRECT(463.00, 3.00, 60.00, 18.00);
	RcLBoxA_[7][0] = UIRECT(522.00, 3.00, 177.00, 372.00);
	RcLBoxA_[7][1] = UIRECT(522.00, 3.00, 177.00, 18.00);

	RcLBoxB_[0][0] = UIRECT(3.00, 3.00, 49.00, 372.00);
	RcLBoxB_[0][1] = UIRECT(3.00, 3.00, 49.00, 18.00);
	RcLBoxB_[1][0] = UIRECT(51.00, 3.00, 197.00, 372.00);
	RcLBoxB_[1][1] = UIRECT(51.00, 3.00, 197.00, 18.00);
	RcLBoxB_[2][0] = UIRECT(247.00, 3.00, 60.00, 372.00);
	RcLBoxB_[2][1] = UIRECT(247.00, 3.00, 60.00, 18.00);
	RcLBoxB_[3][0] = UIRECT(306.00, 3.00, 60.00, 372.00);
	RcLBoxB_[3][1] = UIRECT(306.00, 3.00, 60.00, 18.00);
	RcLBoxB_[4][0] = UIRECT(365.00, 3.00, 70.00, 372.00);
	RcLBoxB_[4][1] = UIRECT(365.00, 3.00, 70.00, 18.00);
	RcLBoxB_[5][0] = UIRECT(434.00, 3.00, 69.00, 372.00);
	RcLBoxB_[5][1] = UIRECT(434.00, 3.00, 69.00, 18.00);
	RcLBoxB_[6][0] = UIRECT(502.00, 3.00, 197.00, 372.00);
	RcLBoxB_[6][1] = UIRECT(502.00, 3.00, 197.00, 18.00);

	RcTextA_[0] = UIRECT(3.00, 3.00, 49.00, 17.00);
	RcTextA_[1] = UIRECT(51.00, 3.00, 177.00, 17.00);
	RcTextA_[2] = UIRECT(227.00, 3.00, 60.00, 17.00);
	RcTextA_[3] = UIRECT(286.00, 3.00, 60.00, 17.00);
	RcTextA_[4] = UIRECT(345.00, 3.00, 60.00, 17.00);
	RcTextA_[5] = UIRECT(404.00, 3.00, 60.00, 17.00);
	RcTextA_[6] = UIRECT(463.00, 3.00, 60.00, 17.00);
	RcTextA_[7] = UIRECT(522.00, 3.00, 177.00, 17.00);

	RcTextB_[0] = UIRECT(3.00, 3.00, 49.00, 17.00);
	RcTextB_[1] = UIRECT(51.00, 3.00, 197.00, 17.00);
	RcTextB_[2] = UIRECT(247.00, 3.00, 60.00, 17.00);
	RcTextB_[3] = UIRECT(306.00, 3.00, 60.00, 17.00);
	RcTextB_[4] = UIRECT(365.00, 3.00, 70.00, 17.00);
	RcTextB_[5] = UIRECT(434.00, 3.00, 69.00, 17.00);
	RcTextB_[6] = UIRECT(502.00, 3.00, 197.00, 17.00);

	RcSlotDummy[0] = UIRECT(6.00, 24.00, 688.00, 32.00);
	RcSlotDummy[1] = UIRECT(6.00, 59.00, 688.00, 32.00);
	RcSlotDummy[2] = UIRECT(6.00, 94.00, 688.00, 32.00);
	RcSlotDummy[3] = UIRECT(6.00, 129.00, 688.00, 32.00);
	RcSlotDummy[4] = UIRECT(6.00, 164.00, 688.00, 32.00);
	RcSlotDummy[5] = UIRECT(6.00, 199.00, 688.00, 32.00);
	RcSlotDummy[6] = UIRECT(6.00, 234.00, 688.00, 32.00);
	RcSlotDummy[7] = UIRECT(6.00, 269.00, 688.00, 32.00);
	RcSlotDummy[8] = UIRECT(6.00, 304.00, 688.00, 32.00);
	RcSlotDummy[9] = UIRECT(6.00, 339.00, 688.00, 32.00);
	
	// m_pLBox
	{
		RcNEW1 = UIRECT(0.00, 0.00, 713.00, 378.00);

		RcOLD = m_pLBox->GetLocalPos();
		m_pLBox->AlignSubControl(RcOLD, RcNEW1);
		m_pLBox->SetLocalPos(RcNEW1);
		m_pLBox->SetGlobalPos(RcNEW1);
	}

	// m_pLBoxA_ | m_pText_
	for (int i = 0; i < m_nSize; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			if( m_nIndex != 2 ) RcNEW1 = RcLBoxA_[i][j];
			else RcNEW1 = RcLBoxB_[i][j];

			RcOLD = m_pLBoxA_[i][j]->GetLocalPos();
			m_pLBoxA_[i][j]->AlignSubControl(RcOLD, RcNEW1);
			m_pLBoxA_[i][j]->SetLocalPos(RcNEW1);
			m_pLBoxA_[i][j]->SetGlobalPos(RcNEW1);
		}

		if( m_nIndex != 2 ) RcNEW1 = RcTextA_[i];
		else RcNEW1 = RcTextB_[i];

		RcOLD = m_pText_[i]->GetLocalPos();
		m_pText_[i]->AlignSubControl(RcOLD, RcNEW1);
		m_pText_[i]->SetLocalPos(RcNEW1);
		m_pText_[i]->SetGlobalPos(RcNEW1);

		switch (m_nIndex)
		{
		case 1:		m_pText_[i]->SetOneLineText((char*)ID2GAMEWORD("GLOBAL_RANKING_PAGE_TEXT1", i));	break;
		case 2:		m_pText_[i]->SetOneLineText((char*)ID2GAMEWORD("GLOBAL_RANKING_PAGE_TEXT2", i));	break;
		default:	m_pText_[i]->SetOneLineText((char*)ID2GAMEWORD("GLOBAL_RANKING_PAGE_TEXT0", i));	break;
		}	
	}

	// m_pScrollBar
	{
		RcNEW1 = UIRECT(702.00, 3.00, 9.00, 372.00);

		RcOLD = m_pScrollBar->GetLocalPos();
		m_pScrollBar->AlignSubControl(RcOLD, RcNEW1);
		m_pScrollBar->SetLocalPos(RcNEW1);
		m_pScrollBar->SetGlobalPos(RcNEW1);
	}

	// m_pSlotDummy_ 
	for (int i = 0; i < MAX_ONE_VIEW_RANK; i++)
	{
		RcNEW1 = RcSlotDummy[i];

		RcOLD = m_pSlotDummy_[i]->GetLocalPos();
		m_pSlotDummy_[i]->AlignSubControl(RcOLD, RcNEW1);
		m_pSlotDummy_[i]->SetLocalPos(RcNEW1);
	}

	// m_pSlot_
	for (int i = 0; i < MAX_ONE_VIEW_RANK; i++)
	{
		RcNEW1 = RcSlotDummy[i];

		RcOLD = m_pSlot_[i]->GetLocalPos();
		m_pSlot_[i]->AlignSubControl(RcOLD, RcNEW1);
		m_pSlot_[i]->SetLocalPos(RcNEW1);
	}

	{
		RcNEW1 = UIRECT(8.00, 44.00, 713.00, 378.00);

		SetLocalPos(RcNEW1);
	}
}

void CGlobalRankingPage::SetSlotRender(int nStartIndex, int nTotal)
{
	if (nTotal < 0) return;

	const UIRECT& rcParentPos = GetGlobalPos();

	for (int i = nStartIndex; i < nTotal; i++)
	{
		CGlobalRankingPageSlot* pSlot = m_pSlot_[i];
		if (pSlot)
		{
			int nAbsoluteIndex				= i - nStartIndex;
			CUIControl* pDummyControl		= m_pSlotDummy_[nAbsoluteIndex];
			const UIRECT& rcSlotPos			= pDummyControl->GetGlobalPos();
			const UIRECT& rcSlotLocalPos	= pDummyControl->GetLocalPos();

			pSlot->SetLocalPos(rcSlotLocalPos);
			pSlot->SetGlobalPos(rcSlotPos);

			if (pSlot->IsHaveData())
				pSlot->SetVisibleSingle(TRUE);
		}
	}
}

void CGlobalRankingPage::ResetAllSlotRender(int nTotal)
{
	if (nTotal < 0) return;

	for (int i = 0; i < nTotal; i++)
	{
		CGlobalRankingPageSlot* pSlot = m_pSlot_[i];
		if (pSlot)
			pSlot->SetVisibleSingle(FALSE);
	}
}

void CGlobalRankingPage::RESET()
{
	for (int i = 0; i < MAX_TOP_RANK; i++)
		m_pSlot_[i]->RESET();

	m_nState = 0;
	m_pScrollBar->GetThumbFrame()->SetState(m_nState, MAX_ONE_VIEW_RANK);
}

void CGlobalRankingPage::SetRankingKill(WORD wPart, STOP_RANK_KILL sTopKill, int nIndex)
{
	/*dmk14 fix crash rank*/
	if( nIndex >= MAX_TOP_RANK )	return;

	m_pSlot_[nIndex]->RESET();
	m_pSlot_[nIndex]->SetRankingKill(sTopKill);

	m_nState++;
	m_pScrollBar->GetThumbFrame()->SetState(m_nState, MAX_ONE_VIEW_RANK);
}

void CGlobalRankingPage::SetRankingRich(WORD wPart, STOP_RANK_RICH sTopRich, int nIndex)
{
	int nRank = (wPart * 10) + nIndex;

	/*dmk14 fix crash rank*/
	if( nRank >= MAX_TOP_RANK )	return;
	if( !m_pSlot_[nRank] )		return;

	m_pSlot_[nRank]->SetRankingRich(sTopRich);

	m_nState++;
	m_pScrollBar->GetThumbFrame()->SetState(m_nState, MAX_ONE_VIEW_RANK);
}

void CGlobalRankingPage::SetRankingGuild(WORD wPart, STOP_RANK_GUILD sTopGuild, int nIndex)
{
	int nRank = (wPart * 10) + nIndex;

	/*dmk14 fix crash rank*/
	if( nRank >= MAX_TOP_RANK )	return;
	if( !m_pSlot_[nRank] )		return;

	m_pSlot_[nRank]->SetRankingGuild(sTopGuild);

	m_nState++;
	m_pScrollBar->GetThumbFrame()->SetState(m_nState, MAX_ONE_VIEW_RANK);
}