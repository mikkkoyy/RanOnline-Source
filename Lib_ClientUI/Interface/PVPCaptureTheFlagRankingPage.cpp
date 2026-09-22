#include "StdAfx.h"
#include "PVPCaptureTheFlagRankingPage.h"
#include "PVPCaptureTheFlagRankingPageSlot.h"

#include "BasicLineBox.h"
#include "BasicScrollBarEx.h"
#include "BasicScrollThumbFrame.h"
#include "BasicTextBox.h"

#include "GameTextControl.h"
#include "UITextControl.h"

#include "GLPVPCaptureTheFlagDefine.h"
#include "GLPVPCaptureTheFlagClient.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPCaptureTheFlagRankingPage::CPVPCaptureTheFlagRankingPage ()
	: m_nType(CAPTURE_THE_FLAG_TEAM_SIZE)
	, m_nStart(0)
	, m_nTotal(0)
	, m_pTextRank(NULL)
	, m_pTextResult(NULL)
	, m_pTextTeam(NULL)
	, m_pTextSchool(NULL)
	, m_pTextClass(NULL)
	, m_pTextName(NULL)
	, m_pTextFlag(NULL)
	, m_pTextKill(NULL)
	, m_pTextDeath(NULL)
	, m_pTextResu(NULL)
	, m_pTextHeal(NULL)
	, m_pTextScore(NULL)
	, m_pTextPoint(NULL)
	, m_pScrollBar(NULL)
	, m_pSlotRankSelf(NULL)
{
	for( int i=0; i<CAPTURE_THE_FLAG_RANKING_PAGE_MAX_SLOT; ++i )
		m_pSlotRank[i] = NULL;
}

CPVPCaptureTheFlagRankingPage::~CPVPCaptureTheFlagRankingPage ()
{
}

void CPVPCaptureTheFlagRankingPage::CreateSubControl ( int nType )
{
	m_nType = nType;

	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "CAPTURE_THE_FLAG_RESULT_RANKING_LINEBOX" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "CAPTURE_THE_FLAG_RESULT_MYRANK_LINE_HEAD" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "CAPTURE_THE_FLAG_RESULT_MYRANK_LINE_TAIL" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "CAPTURE_THE_FLAG_RESULT_RANK_SCROLLBAR_BACKGROUND" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody ( "CAPTURE_THE_FLAG_RESULT_RANKING_LINEBOX" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody ( "CAPTURE_THE_FLAG_RESULT_RANKING_LINEBOX_HEAD" );
	RegisterControl ( pLineBox );

	m_pTextRank = CreateUITextBox( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_RANK", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextResult = CreateUITextBox( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_RESULT", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextTeam = CreateUITextBox( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_TEAM", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextSchool = CreateUITextBox( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_SCHOOL", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextClass = CreateUITextBox( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_CLASS", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextName = CreateUITextBox( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_NAME", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextFlag = CreateUITextBox( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_FLAG", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextKill = CreateUITextBox( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_KILL", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextDeath = CreateUITextBox( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_DEATH", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextResu = CreateUITextBox( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_REBIRTH", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextHeal = CreateUITextBox( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_HEAL", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextScore = CreateUITextBox( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_SCORE", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextPoint = CreateUITextBox( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_CONTRIBUTION", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );

	m_pTextRank->AddText( ID2GAMEWORD( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE", 0 ), NS_UITEXTCOLOR::WHITE );
	m_pTextResult->AddText( ID2GAMEWORD( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE", 1 ), NS_UITEXTCOLOR::WHITE );
	m_pTextTeam->AddText( ID2GAMEWORD( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE", 2 ), NS_UITEXTCOLOR::WHITE );
	m_pTextSchool->AddText( ID2GAMEWORD( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE", 3 ), NS_UITEXTCOLOR::WHITE );
	m_pTextClass->AddText( ID2GAMEWORD( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE", 4 ), NS_UITEXTCOLOR::WHITE );
	m_pTextName->AddText( ID2GAMEWORD( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE", 5 ), NS_UITEXTCOLOR::WHITE );
	m_pTextFlag->AddText( ID2GAMEWORD( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE", 6 ), NS_UITEXTCOLOR::WHITE );
	m_pTextKill->AddText( ID2GAMEWORD( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE", 7 ), NS_UITEXTCOLOR::WHITE );
	m_pTextDeath->AddText( ID2GAMEWORD( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE", 8 ), NS_UITEXTCOLOR::WHITE );
	m_pTextResu->AddText( ID2GAMEWORD( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE", 9 ), NS_UITEXTCOLOR::WHITE );
	m_pTextHeal->AddText( ID2GAMEWORD( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE", 10 ), NS_UITEXTCOLOR::WHITE );
	m_pTextScore->AddText( ID2GAMEWORD( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE", 11 ), NS_UITEXTCOLOR::WHITE );
	m_pTextPoint->AddText( ID2GAMEWORD( "CAPTURE_THE_FLAG_RESULT_TEXT_BASE", 12 ), NS_UITEXTCOLOR::WHITE );

	std::string strslots[CAPTURE_THE_FLAG_RANKING_PAGE_MAX_SLOT] = 
	{
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_SLOT_0",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_SLOT_1",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_SLOT_2",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_SLOT_3",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_SLOT_4",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_SLOT_5",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_SLOT_6",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_SLOT_7",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_SLOT_8",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_SLOT_9",
	};

	for( int i=0; i<CAPTURE_THE_FLAG_RANKING_PAGE_MAX_SLOT; ++i )
	{
		m_pSlotRank[i] = new CPVPCaptureTheFlagRankingPageSlot;
		m_pSlotRank[i]->CreateSub ( this, strslots[i].c_str() );
		m_pSlotRank[i]->CreateSubControl ( FALSE );
		m_pSlotRank[i]->SetVisibleSingle( TRUE );
		RegisterControl ( m_pSlotRank[i] );	
	}

	m_pSlotRankSelf = new CPVPCaptureTheFlagRankingPageSlot;
	m_pSlotRankSelf->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_RANKSLOT_SLOT_10" );
	m_pSlotRankSelf->CreateSubControl ( TRUE );
	m_pSlotRankSelf->SetVisibleSingle( TRUE );
	RegisterControl ( m_pSlotRankSelf );	

	m_pScrollBar = new CBasicScrollBarEx;
	m_pScrollBar->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, CAPTURE_THE_FLAG_RANKING_PAGE_SCROLLBAR );
	m_pScrollBar->CreateBaseScrollBar ( "CAPTURE_THE_FLAG_RESULT_RANK_SCROLLBAR" );
	m_pScrollBar->GetThumbFrame()->SetState ( 1, CAPTURE_THE_FLAG_RANKING_PAGE_MAX_SLOT );
	RegisterControl ( m_pScrollBar );
}

CBasicTextBox* CPVPCaptureTheFlagRankingPage::CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign )
{
	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, szControl );
	pTextBox->SetFont ( pFont );
	pTextBox->SetTextAlign ( nTextAlign );
	RegisterControl ( pTextBox );
	return pTextBox;
}

void CPVPCaptureTheFlagRankingPage::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	m_nTotal = (int)GLPVPCaptureTheFlagClient::GetInstance().GetRankingSize( m_nType );

	if ( m_pScrollBar )
	{
		{
			CBasicScrollThumbFrame* pThumbFrame = m_pScrollBar->GetThumbFrame ();
			int nTotal = pThumbFrame->GetTotal();

			if( nTotal != m_nTotal )
				m_pScrollBar->GetThumbFrame()->SetState ( m_nTotal, CAPTURE_THE_FLAG_RANKING_PAGE_MAX_SLOT );	
		}
		{
			CBasicScrollThumbFrame* pThumbFrame = m_pScrollBar->GetThumbFrame ();
			int nTotal = pThumbFrame->GetTotal ();
			if ( nTotal <= CAPTURE_THE_FLAG_RANKING_PAGE_MAX_SLOT ) return ;
			const int nViewPerPage = pThumbFrame->GetViewPerPage ();

			if ( nViewPerPage < nTotal )
			{
				int nCurPos = 0;
				const int nMovableLine = nTotal - nViewPerPage;
				float fPercent = pThumbFrame->GetPercent ();
				nCurPos = (int)floor(fPercent * nMovableLine);
				if ( nCurPos < 0 ) nCurPos = 0;
				if ( m_nStart == nCurPos ) return;
				m_nStart = nCurPos;
				RenderView();
			}
		}
	}
}

void CPVPCaptureTheFlagRankingPage::RefreshRanking()
{
	m_nTotal = (int)GLPVPCaptureTheFlagClient::GetInstance().GetRankingSize( m_nType );
	WORD wWinner = GLPVPCaptureTheFlagClient::GetInstance().m_wLastWinner;

	if ( m_pSlotRankSelf )
		m_pSlotRankSelf->SetVisibleSingle( FALSE );

	CAPTURE_THE_FLAG_PLAYER_DATA* pOwnRank = GLPVPCaptureTheFlagClient::GetInstance().GetOwnRanking();
	if ( pOwnRank && m_pSlotRankSelf )
	{
		m_pSlotRankSelf->SetVisibleSingle( TRUE );
		m_pSlotRankSelf->SetData( pOwnRank, BOOL( m_nType == CAPTURE_THE_FLAG_TEAM_SIZE ), wWinner );
	}

	RenderView();
}

void CPVPCaptureTheFlagRankingPage::RenderView()
{
	for ( int i=0; i < CAPTURE_THE_FLAG_RANKING_PAGE_MAX_SLOT; ++ i )
	{
		if ( m_pSlotRank[i] )
		{
			m_pSlotRank[i]->ResetData();
			m_pSlotRank[i]->SetVisibleSingle( FALSE );
		}
	}

	CAPTURE_THE_FLAG_PLAYER_DATA_VEC vecRank = GLPVPCaptureTheFlagClient::GetInstance().GetRankingVector( m_nType );
	WORD wWinner = GLPVPCaptureTheFlagClient::GetInstance().m_wLastWinner;

	int nEnd = m_nStart + CAPTURE_THE_FLAG_RANKING_PAGE_MAX_SLOT;
	int nSlot = 0;

	for ( int i = m_nStart; i < nEnd; ++ i )
	{
		if ( i >= (int)vecRank.size() )	continue;
		if ( m_pSlotRank[nSlot] )
		{
			CAPTURE_THE_FLAG_PLAYER_DATA sRANK = vecRank[i];
			m_pSlotRank[nSlot]->SetVisibleSingle( TRUE );
			m_pSlotRank[nSlot]->SetData( &sRANK, BOOL( m_nType == CAPTURE_THE_FLAG_TEAM_SIZE ), wWinner );
			nSlot ++ ;
		}
	}
}
