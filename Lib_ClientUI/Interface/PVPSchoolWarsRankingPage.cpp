#include "StdAfx.h"
#include "PVPSchoolWarsRankingPage.h"
#include "PVPSchoolWarsRankingPageSlot.h"

#include "BasicLineBox.h"
#include "BasicScrollBarEx.h"
#include "BasicScrollThumbFrame.h"
#include "BasicTextBox.h"

#include "GameTextControl.h"
#include "UITextControl.h"

#include "GLPVPSchoolWarsDefine.h"
#include "GLPVPSchoolWarsClient.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPSchoolWarsRankingPage::CPVPSchoolWarsRankingPage ()
	: m_nType(SCHOOLWARS_SCHOOL_SIZE)
	, m_nStart(0)
	, m_nTotal(0)
	, m_pTextRank(NULL)
	, m_pTextSchool(NULL)
	, m_pTextClass(NULL)
	, m_pTextName(NULL)
	, m_pTextKill(NULL)
	, m_pTextDeath(NULL)
	, m_pTextResu(NULL)
	, m_pTextHeal(NULL)
	, m_pTextScore(NULL)
	, m_pTextPoint(NULL)
	, m_pScrollBar(NULL)
	, m_pSlotRankSelf(NULL)
{
	for( int i=0; i<SCHOOLWARS_RANKING_PAGE_MAX_SLOT; ++i )
		m_pSlotRank[i] = NULL;
}

CPVPSchoolWarsRankingPage::~CPVPSchoolWarsRankingPage ()
{
}

void CPVPSchoolWarsRankingPage::CreateSubControl ( int nType )
{
	m_nType = nType;

	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "SCHOOLWARS_RESULT_RANKING_LINEBOX" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "SCHOOLWARS_RESULT_MYRANK_LINE_HEAD" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "SCHOOLWARS_RESULT_MYRANK_LINE_TAIL" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "SCHOOLWARS_RESULT_RANK_SCROLLBAR_BACKGROUND" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody ( "SCHOOLWARS_RESULT_RANKING_LINEBOX" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody ( "SCHOOLWARS_RESULT_RANKING_LINEBOX_HEAD" );
	RegisterControl ( pLineBox );

	m_pTextRank = CreateUITextBox( "SCHOOLWARS_RESULT_TEXT_BASE_RANK", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextSchool = CreateUITextBox( "SCHOOLWARS_RESULT_TEXT_BASE_SCHOOL", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextClass = CreateUITextBox( "SCHOOLWARS_RESULT_TEXT_BASE_CLASS", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextName = CreateUITextBox( "SCHOOLWARS_RESULT_TEXT_BASE_NAME", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextKill = CreateUITextBox( "SCHOOLWARS_RESULT_TEXT_BASE_KILL", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextDeath = CreateUITextBox( "SCHOOLWARS_RESULT_TEXT_BASE_DEATH", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextResu = CreateUITextBox( "SCHOOLWARS_RESULT_TEXT_BASE_REBIRTH", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextHeal = CreateUITextBox( "SCHOOLWARS_RESULT_TEXT_BASE_HEAL", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextScore = CreateUITextBox( "SCHOOLWARS_RESULT_TEXT_BASE_SCORE", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextPoint = CreateUITextBox( "SCHOOLWARS_RESULT_TEXT_BASE_CONTRIBUTION", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );

	m_pTextRank->AddText( ID2GAMEWORD( "SCHOOLWARS_RESULT_TEXT_BASE", 0 ), NS_UITEXTCOLOR::WHITE );
	m_pTextSchool->AddText( ID2GAMEWORD( "SCHOOLWARS_RESULT_TEXT_BASE", 1 ), NS_UITEXTCOLOR::WHITE );
	m_pTextClass->AddText( ID2GAMEWORD( "SCHOOLWARS_RESULT_TEXT_BASE", 2 ), NS_UITEXTCOLOR::WHITE );
	m_pTextName->AddText( ID2GAMEWORD( "SCHOOLWARS_RESULT_TEXT_BASE", 3 ), NS_UITEXTCOLOR::WHITE );
	m_pTextKill->AddText( ID2GAMEWORD( "SCHOOLWARS_RESULT_TEXT_BASE", 4 ), NS_UITEXTCOLOR::WHITE );
	m_pTextDeath->AddText( ID2GAMEWORD( "SCHOOLWARS_RESULT_TEXT_BASE", 5 ), NS_UITEXTCOLOR::WHITE );
	m_pTextResu->AddText( ID2GAMEWORD( "SCHOOLWARS_RESULT_TEXT_BASE", 7 ), NS_UITEXTCOLOR::WHITE );
	m_pTextHeal->AddText( ID2GAMEWORD( "SCHOOLWARS_RESULT_TEXT_BASE", 6 ), NS_UITEXTCOLOR::WHITE );
	m_pTextScore->AddText( ID2GAMEWORD( "SCHOOLWARS_RESULT_TEXT_BASE", 9 ), NS_UITEXTCOLOR::WHITE );
	m_pTextPoint->AddText( ID2GAMEWORD( "SCHOOLWARS_RESULT_TEXT_BASE", 8 ), NS_UITEXTCOLOR::WHITE );

	std::string strslots[SCHOOLWARS_RANKING_PAGE_MAX_SLOT] = 
	{
		"SCHOOLWARS_RESULT_RANKSLOT_SLOT_0",
		"SCHOOLWARS_RESULT_RANKSLOT_SLOT_1",
		"SCHOOLWARS_RESULT_RANKSLOT_SLOT_2",
		"SCHOOLWARS_RESULT_RANKSLOT_SLOT_3",
		"SCHOOLWARS_RESULT_RANKSLOT_SLOT_4",
		"SCHOOLWARS_RESULT_RANKSLOT_SLOT_5",
		"SCHOOLWARS_RESULT_RANKSLOT_SLOT_6",
		"SCHOOLWARS_RESULT_RANKSLOT_SLOT_7",
		"SCHOOLWARS_RESULT_RANKSLOT_SLOT_8",
		"SCHOOLWARS_RESULT_RANKSLOT_SLOT_9",
	};

	for( int i=0; i<SCHOOLWARS_RANKING_PAGE_MAX_SLOT; ++i )
	{
		m_pSlotRank[i] = new CPVPSchoolWarsRankingPageSlot;
		m_pSlotRank[i]->CreateSub ( this, strslots[i].c_str() );
		m_pSlotRank[i]->CreateSubControl ( FALSE );
		m_pSlotRank[i]->SetVisibleSingle( TRUE );
		RegisterControl ( m_pSlotRank[i] );	
	}

	m_pSlotRankSelf = new CPVPSchoolWarsRankingPageSlot;
	m_pSlotRankSelf->CreateSub ( this, "SCHOOLWARS_RESULT_RANKSLOT_SLOT_10" );
	m_pSlotRankSelf->CreateSubControl ( TRUE );
	m_pSlotRankSelf->SetVisibleSingle( TRUE );
	RegisterControl ( m_pSlotRankSelf );	

	m_pScrollBar = new CBasicScrollBarEx;
	m_pScrollBar->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, SCHOOLWARS_RANKING_PAGE_SCROLLBAR );
	m_pScrollBar->CreateBaseScrollBar ( "SCHOOLWARS_RESULT_RANK_SCROLLBAR" );
	m_pScrollBar->GetThumbFrame()->SetState ( 1, SCHOOLWARS_RANKING_PAGE_MAX_SLOT );
	RegisterControl ( m_pScrollBar );
}

CBasicTextBox* CPVPSchoolWarsRankingPage::CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign )
{
	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, szControl );
	pTextBox->SetFont ( pFont );
	pTextBox->SetTextAlign ( nTextAlign );
	RegisterControl ( pTextBox );
	return pTextBox;
}

void CPVPSchoolWarsRankingPage::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	m_nTotal = (int)GLPVPSchoolWarsClient::GetInstance().GetRankingSize( m_nType );

	if ( m_pScrollBar )
	{
		{
			CBasicScrollThumbFrame* pThumbFrame = m_pScrollBar->GetThumbFrame ();
			int nTotal = pThumbFrame->GetTotal();

			if( nTotal != m_nTotal )
				m_pScrollBar->GetThumbFrame()->SetState ( m_nTotal, SCHOOLWARS_RANKING_PAGE_MAX_SLOT );	
		}
		{
			CBasicScrollThumbFrame* pThumbFrame = m_pScrollBar->GetThumbFrame ();
			int nTotal = pThumbFrame->GetTotal ();
			if ( nTotal <= SCHOOLWARS_RANKING_PAGE_MAX_SLOT ) return ;
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

void CPVPSchoolWarsRankingPage::RefreshRanking()
{
	m_nTotal = (int)GLPVPSchoolWarsClient::GetInstance().GetRankingSize( m_nType );

	if ( m_pSlotRankSelf )
		m_pSlotRankSelf->SetVisibleSingle( FALSE );

	SCHOOLWARS_PLAYER_DATA* pOwnRank = GLPVPSchoolWarsClient::GetInstance().GetOwnRanking();
	if ( pOwnRank && m_pSlotRankSelf )
	{
		m_pSlotRankSelf->SetVisibleSingle( TRUE );
		m_pSlotRankSelf->SetData( pOwnRank, BOOL( m_nType == SCHOOLWARS_SCHOOL_SIZE ) );
	}

	RenderView();
}

void CPVPSchoolWarsRankingPage::RenderView()
{
	for ( int i=0; i < SCHOOLWARS_RANKING_PAGE_MAX_SLOT; ++ i )
	{
		if ( m_pSlotRank[i] )
		{
			m_pSlotRank[i]->ResetData();
			m_pSlotRank[i]->SetVisibleSingle( FALSE );
		}
	}

	SCHOOLWARS_PLAYER_DATA_VEC vecRank = GLPVPSchoolWarsClient::GetInstance().GetRankingVector( m_nType );

	int nEnd = m_nStart + SCHOOLWARS_RANKING_PAGE_MAX_SLOT;
	int nSlot = 0;

	for ( int i = m_nStart; i < nEnd; ++ i )
	{
		if ( i >= (int)vecRank.size() )	continue;
		if ( m_pSlotRank[nSlot] )
		{
			SCHOOLWARS_PLAYER_DATA sRANK = vecRank[i];
			m_pSlotRank[nSlot]->SetVisibleSingle( TRUE );
			m_pSlotRank[nSlot]->SetData( &sRANK, BOOL( m_nType == SCHOOLWARS_SCHOOL_SIZE ) );
			nSlot ++ ;
		}
	}
}
