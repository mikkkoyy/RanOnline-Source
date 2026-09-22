#include "StdAfx.h"
#include "PVPTyrannyPageHistory.h"
#include "PVPTyrannyPageHistoryBuff.h"

#include "d3dfont.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include "BasicTextBox.h"
#include "BasicLineBox.h"
#include "BasicButton.h"

#include "GLPVPTyrannyClient.h"
#include "InnerInterface.h"
#include "DxGlobalStage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPTyrannyPageHistory::CPVPTyrannyPageHistory ()
	: m_pTextHistoryTitleStatic(NULL)
	, m_pTextHistoryTimeStatic(NULL)
	, m_pTextHistoryAuthStatic(NULL)
	, m_pTextHistoryRewardStatic(NULL)
	, m_pTextHistoryRankingStatic(NULL)
	, m_pTextHistoryTimeText(NULL)
	, m_pTextHistoryAuthText(NULL)
	, m_pTextHistoryRankingText(NULL)
	, m_pButtonRanking(NULL)
	, m_pHistoryBuff(NULL)
	, m_fUpdateTime(0.0f)
{
	for ( int i=0; i<TYRANNY_SCHOOL_SIZE+1; ++i )
		m_pAuthSchoolImage[i] = NULL;
}

CPVPTyrannyPageHistory::~CPVPTyrannyPageHistory ()
{
}

void CPVPTyrannyPageHistory::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "COMPETITION_CTF_PAGE_HISTORY_LINBOX" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody ( "COMPETITION_CTF_PAGE_HISTORY_LINBOX" );
	RegisterControl ( pLineBox );
	
	m_pTextHistoryTitleStatic = CreateUITextBox( "COMPETITION_CTF_PAGE_HISTORY_TITLE_STATICTEXT", pFont9, TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextHistoryTimeStatic = CreateUITextBox( "COMPETITION_CTF_PAGE_HISTORY_TIME_STATICTEXT", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextHistoryAuthStatic = CreateUITextBox( "COMPETITION_CTF_PAGE_HISTORY_AUTHSCHOOL_STATICTEXT", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextHistoryRewardStatic = CreateUITextBox( "COMPETITION_CTF_PAGE_HISTORY_REWARDBUFF_STATICTEXT", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextHistoryRankingStatic = CreateUITextBox( "COMPETITION_CTF_PAGE_HISTORY_RANKING_STATICTEXT", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );

	m_pTextHistoryTitleStatic->AddText( ID2GAMEWORD( "COMPETITION_CTF_HISTORY", 0 ), NS_UITEXTCOLOR::WHITE );
	m_pTextHistoryTimeStatic->AddText( ID2GAMEWORD( "COMPETITION_CTF_HISTORY", 1 ), NS_UITEXTCOLOR::WHITE );
	m_pTextHistoryAuthStatic->AddText( ID2GAMEWORD( "COMPETITION_CTF_HISTORY", 2 ), NS_UITEXTCOLOR::WHITE );
	m_pTextHistoryRewardStatic->AddText( ID2GAMEWORD( "COMPETITION_CTF_HISTORY", 3 ), NS_UITEXTCOLOR::WHITE );
	m_pTextHistoryRankingStatic->AddText( ID2GAMEWORD( "COMPETITION_CTF_HISTORY", 4 ), NS_UITEXTCOLOR::WHITE );

	m_pTextHistoryTimeText = CreateUITextBox( "COMPETITION_CTF_PAGE_HISTORY_TIME", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextHistoryAuthText = CreateUITextBox( "COMPETITION_CTF_PAGE_HISTORY_AUTHSCHOOL_TEXT", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextHistoryRankingText = CreateUITextBox( "COMPETITION_CTF_PAGE_HISTORY_RANKING_TEXT", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );

	std::string strschool_image[TYRANNY_SCHOOL_SIZE+1] = 
	{
		"COMPETITION_CTF_PAGE_HISTORY_AUTHSCHOOL_IMAGE_SCHOOL0",
		"COMPETITION_CTF_PAGE_HISTORY_AUTHSCHOOL_IMAGE_SCHOOL1",
		"COMPETITION_CTF_PAGE_HISTORY_AUTHSCHOOL_IMAGE_SCHOOL2",
		"COMPETITION_CTF_PAGE_HISTORY_AUTHSCHOOL_IMAGE_SCHOOL_NONE",
	};

	for( int i=0; i<TYRANNY_SCHOOL_SIZE+1; ++i )
	{
		m_pAuthSchoolImage[i] = CreateUIControl( strschool_image[i].c_str() );
		m_pAuthSchoolImage[i]->SetVisibleSingle( FALSE );
	}

	m_pButtonRanking = new CBasicButton;
	m_pButtonRanking->CreateSub( this, "COMPETITION_CTF_PAGE_HISTORY_RANKING_BUTTON", UI_FLAG_DEFAULT, TYRANNY_PAGE_INFO_HISTORY_BUTTON_RANKING );
	m_pButtonRanking->CreateFlip( "COMPETITION_CTF_PAGE_HISTORY_RANKING_BUTTON_FLIP", CBasicButton::MOUSEIN_FLIP );
	RegisterControl( m_pButtonRanking );

	m_pHistoryBuff = new CPVPTyrannyPageHistoryBuff;
	m_pHistoryBuff->CreateSub( this, "COMPETITION_CTF_PAGE_HISTORY_REWARDBUFF", UI_FLAG_DEFAULT, TYRANNY_PAGE_INFO_HISTORY_BUFF );
	m_pHistoryBuff->CreateSubControl ();
	RegisterControl ( m_pHistoryBuff );
}

CBasicTextBox* CPVPTyrannyPageHistory::CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign )
{
	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, szControl );
	pTextBox->SetFont ( pFont );
	pTextBox->SetTextAlign ( nTextAlign );
	RegisterControl ( pTextBox );
	return pTextBox;
}

CUIControl* CPVPTyrannyPageHistory::CreateUIControl( const char* szControl )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl );
	pControl->SetVisibleSingle( TRUE );
	RegisterControl ( pControl );
	return pControl;
}

void CPVPTyrannyPageHistory::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch( ControlID )
	{
	case TYRANNY_PAGE_INFO_HISTORY_BUTTON_RANKING:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg) )
			{
				CInnerInterface::GetInstance().PVPTyrannyShowTopRanking();
			}
		}break;
	};
}

void CPVPTyrannyPageHistory::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	m_fUpdateTime += fElapsedTime;
	if ( m_fUpdateTime >= 0.5f )
	{
		ShowInfo();
		m_fUpdateTime = 0.0f;
	}
}

void CPVPTyrannyPageHistory::ShowInfo()
{
	for ( int i=0; i<TYRANNY_SCHOOL_SIZE+1; ++i )
	{
		if ( m_pAuthSchoolImage[i] )
			m_pAuthSchoolImage[i]->SetVisibleSingle( FALSE );
	}

	if ( m_pTextHistoryAuthText )
		m_pTextHistoryAuthText->ClearText();

	WORD wlast_winner = GLPVPTyrannyClient::GetInstance().m_wLastWinner;

	if ( wlast_winner < TYRANNY_SCHOOL_SIZE+1 )
	{
		if ( m_pAuthSchoolImage[wlast_winner] )
			m_pAuthSchoolImage[wlast_winner]->SetVisibleSingle( TRUE );

		if ( m_pTextHistoryAuthText )
		{
			if ( wlast_winner == TYRANNY_SCHOOL_SIZE )
				m_pTextHistoryAuthText->AddText( ID2GAMEWORD( "COMPETITION_CTF_HISTORY", 5 ), NS_UITEXTCOLOR::WHITE );
			else
				m_pTextHistoryAuthText->AddText( GLCONST_CHAR::strSCHOOLNAME[wlast_winner].c_str(), NS_UITEXTCOLOR::WHITE );
		}
	}

	TYRANNY_SCHED_NEXT& sNextSched = GLPVPTyrannyClient::GetInstance().m_sScheduleNext;

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

		TYRANNY_PLAYER_DATA& splayer_data = GLPVPTyrannyClient::GetInstance().m_sLastTop[0];

		if( splayer_data.wRankAll != 0 && 
			splayer_data.dwInfoCharID != TYRANNY_PLAYER_NULL )
		{
			CString strCombine;
			strCombine.Format ( "%s %s", ID2GAMEWORD( "COMPETITION_CTF_HISTORY", 6 ), splayer_data.szInfoCharName );
			m_pTextHistoryRankingText->AddText( strCombine.GetString(), NS_UITEXTCOLOR::WHITE );
		}
		else
		{
			CString strCombine;
			strCombine.Format ( "%s %s", ID2GAMEWORD( "COMPETITION_CTF_HISTORY", 6 ), ID2GAMEWORD( "COMPETITION_CTF_HISTORY", 5 ) );
			m_pTextHistoryRankingText->AddText( strCombine.GetString(), NS_UITEXTCOLOR::WHITE );
		}
		
	}
}
