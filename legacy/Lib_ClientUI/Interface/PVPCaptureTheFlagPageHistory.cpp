#include "StdAfx.h"
#include "PVPCaptureTheFlagPageHistory.h"

#include "d3dfont.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include "BasicTextBox.h"
#include "BasicLineBox.h"
#include "BasicButton.h"

#include "GLPVPCaptureTheFlagClient.h"
#include "InnerInterface.h"
#include "DxGlobalStage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPCaptureTheFlagPageHistory::CPVPCaptureTheFlagPageHistory ()
	: m_pTextHistoryTimeStatic(NULL)
	, m_pTextHistoryAuthStatic(NULL)
	, m_pTextHistoryRankingStatic(NULL)
	, m_pTextHistoryTimeText(NULL)
	, m_pTextHistoryAuthText(NULL)
	, m_pTextHistoryRankingText(NULL)
	, m_pButtonRanking(NULL)
	, m_fUpdateTime(0.0f)
{
}

CPVPCaptureTheFlagPageHistory::~CPVPCaptureTheFlagPageHistory ()
{
}

void CPVPCaptureTheFlagPageHistory::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	m_pTextHistoryTimeStatic = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_HISTORY_TIME_STATICTEXT", pFont9, TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextHistoryAuthStatic = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_HISTORY_TEAM_STATICTEXT", pFont9, TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextHistoryRankingStatic = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_HISTORY_RANKING_STATICTEXT", pFont9, TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );

	m_pTextHistoryTimeStatic->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_HISTORY", 0 ), NS_UITEXTCOLOR::WHITE );
	m_pTextHistoryAuthStatic->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_HISTORY", 1 ), NS_UITEXTCOLOR::WHITE );
	m_pTextHistoryRankingStatic->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_HISTORY", 2 ), NS_UITEXTCOLOR::WHITE );

	m_pTextHistoryTimeText = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_HISTORY_TIME_TEXT", pFont9, TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pTextHistoryAuthText = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_HISTORY_TEAM_TEXT", pFont9, TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pTextHistoryRankingText = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_HISTORY_RANKING_TEXT", pFont9, TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );

	m_pButtonRanking = new CBasicButton;
	m_pButtonRanking->CreateSub( this, "COMPETITION_CAPTURE_THE_FLAG_PAGE_HISTORY_RANKING_BUTTON", UI_FLAG_DEFAULT, CAPTURE_THE_FLAG_PAGE_INFO_HISTORY_BUTTON_RANKING );
	m_pButtonRanking->CreateFlip( "COMPETITION_CAPTURE_THE_FLAG_PAGE_HISTORY_RANKING_BUTTON_FLIP", CBasicButton::MOUSEIN_FLIP );
	RegisterControl( m_pButtonRanking );
}

CBasicTextBox* CPVPCaptureTheFlagPageHistory::CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign )
{
	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, szControl );
	pTextBox->SetFont ( pFont );
	pTextBox->SetTextAlign ( nTextAlign );
	RegisterControl ( pTextBox );
	return pTextBox;
}

CUIControl* CPVPCaptureTheFlagPageHistory::CreateUIControl( const char* szControl )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl );
	pControl->SetVisibleSingle( TRUE );
	RegisterControl ( pControl );
	return pControl;
}

void CPVPCaptureTheFlagPageHistory::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch( ControlID )
	{
	case CAPTURE_THE_FLAG_PAGE_INFO_HISTORY_BUTTON_RANKING:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg) )
			{
				CInnerInterface::GetInstance().PVPCaptureTheFlagShowTopRanking();
			}
		}break;
	};
}

void CPVPCaptureTheFlagPageHistory::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	m_fUpdateTime += fElapsedTime;
	if ( m_fUpdateTime >= 0.5f )
	{
		ShowInfo();
		m_fUpdateTime = 0.0f;
	}
}

void CPVPCaptureTheFlagPageHistory::ShowInfo()
{
	if ( m_pTextHistoryAuthText )
		m_pTextHistoryAuthText->ClearText();

	WORD wlast_winner = GLPVPCaptureTheFlagClient::GetInstance().m_wLastWinner;

	if ( wlast_winner < CAPTURE_THE_FLAG_TEAM_SIZE+1 )
	{
		if ( m_pTextHistoryAuthText )
		{
			if ( wlast_winner >= CAPTURE_THE_FLAG_TEAM_SIZE )
				m_pTextHistoryAuthText->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_HISTORY", 3 ), NS_UITEXTCOLOR::WHITE );
			else
				m_pTextHistoryAuthText->AddText( ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_TEAM_TEXT", wlast_winner ), NS_UITEXTCOLOR::WHITE );
		}
	}

	CAPTURE_THE_FLAG_SCHED_NEXT& sNextSched = GLPVPCaptureTheFlagClient::GetInstance().m_sScheduleNext;

	if ( m_pTextHistoryTimeText )
	{
		m_pTextHistoryTimeText->ClearText();
		if ( sNextSched.dwIndex != UINT_MAX )
		{
			CString strCombine;
			strCombine.Format ( "%02d:%02d~%02d:%02d", sNextSched.wPrevStartHour, sNextSched.wPrevStartMinute, sNextSched.wPrevEndHour, sNextSched.wPrevEndMinute );
			m_pTextHistoryTimeText->AddText( strCombine.GetString(), NS_UITEXTCOLOR::WHITE  );
		}
		else
		{
			m_pTextHistoryTimeText->AddText( "00:00~00:00", NS_UITEXTCOLOR::WHITE  );
		}
	}

	if ( m_pTextHistoryRankingText )
	{
		m_pTextHistoryRankingText->ClearText();

		CAPTURE_THE_FLAG_PLAYER_DATA& splayer_data = GLPVPCaptureTheFlagClient::GetInstance().m_sLastTop[0];

		if( splayer_data.wRankAll != 0 &&  splayer_data.dwInfoCharID != CAPTURE_THE_FLAG_PLAYER_NULL )
		{
			CString strCombine;
			strCombine.Format ( "%s %s", ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_HISTORY", 4 ), splayer_data.szInfoCharName );
			m_pTextHistoryRankingText->AddText( strCombine.GetString(), NS_UITEXTCOLOR::WHITE );
		}
		else
		{
			CString strCombine;
			strCombine.Format ( "%s %s", ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_HISTORY", 4 ), ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_HISTORY", 3 ) );
			m_pTextHistoryRankingText->AddText( strCombine.GetString(), NS_UITEXTCOLOR::WHITE );
		}
		
	}
}
