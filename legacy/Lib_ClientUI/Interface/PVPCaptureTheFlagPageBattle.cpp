#include "StdAfx.h"
#include "PVPCaptureTheFlagPageBattle.h"

#include "d3dfont.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include "BasicTextBox.h"
#include "BasicLineBox.h"
#include "BasicTextButton.h"

#include "GLPVPCaptureTheFlagClient.h"
#include "InnerInterface.h"
#include "DxGlobalStage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPCaptureTheFlagPageBattle::CPVPCaptureTheFlagPageBattle ()
	: m_pTextBattleTimeStartStatic(NULL)
	, m_pTextBattleTimeEndStatic(NULL)
	, m_pTextBattleProgressStatic(NULL)
	, m_pTextBattleStatusStatic(NULL)
	, m_pTextBattleTimeStartTimer(NULL)
	, m_pTextBattleTimeEndTimer(NULL)
	, m_pTextBattleProgress(NULL)
	, m_pTextBattleStatus(NULL)
	, m_pButtonRegister1(NULL)
	, m_pButtonRegister2(NULL)
	, m_pTextPlayerNum1(NULL)
	, m_pTextPlayerNum2(NULL)
	, m_fUpdateTime(0.0f)
{
}

CPVPCaptureTheFlagPageBattle::~CPVPCaptureTheFlagPageBattle ()
{
}

void CPVPCaptureTheFlagPageBattle::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	m_pTextBattleTimeStartStatic = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_BATTLE_STARTTIME_STATICTEXT", pFont9, TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextBattleTimeEndStatic = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_BATTLE_ENDTIME_STATICTEXT", pFont9, TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextBattleProgressStatic = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_BATTLE_PROGRESSTIME_STATICTEXT", pFont9, TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextBattleStatusStatic = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_BATTLE_STATUS_STATICTEXT", pFont9, TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );

	m_pTextBattleTimeStartStatic->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_STATIC", 0 ), NS_UITEXTCOLOR::WHITE );
	m_pTextBattleTimeEndStatic->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_STATIC", 1 ), NS_UITEXTCOLOR::WHITE );
	m_pTextBattleProgressStatic->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_STATIC", 2 ), NS_UITEXTCOLOR::WHITE );
	m_pTextBattleStatusStatic->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_STATIC", 3 ), NS_UITEXTCOLOR::WHITE );

	m_pTextBattleTimeStartTimer = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_BATTLE_STARTTIME_TEXT", pFont9, TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pTextBattleTimeEndTimer = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_BATTLE_ENDTIME_TEXT", pFont9, TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pTextBattleProgress = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_BATTLE_PROGRESSTIME_TEXT", pFont9, TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pTextBattleStatus = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_BATTLE_STATUS_TEXT", pFont9, TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );

	m_pTextPlayerNum1 = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_BATTLE_PLAYER_NUM_1", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextPlayerNum2 = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_BATTLE_PLAYER_NUM_2", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );

	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, "COMPETITION_CAPTURE_THE_FLAG_PAGE_BATTLE_ICON_1", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pControl->SetVisibleSingle( TRUE );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "COMPETITION_CAPTURE_THE_FLAG_PAGE_BATTLE_ICON_2", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pControl->SetVisibleSingle( TRUE );
	RegisterControl ( pControl );


	m_pButtonRegister1 = new CBasicTextButton;
	m_pButtonRegister1->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, CAPTURE_THE_FLAG_PAGE_INFO_BATTLE_BUTTON_REGISTER_1 );
	m_pButtonRegister1->CreateBaseButton ( "COMPETITION_CAPTURE_THE_FLAG_PAGE_BATTLE_BUTTON_1", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, ID2GAMEWORD("COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 0 ) );
	RegisterControl ( m_pButtonRegister1 );

	m_pButtonRegister2 = new CBasicTextButton;
	m_pButtonRegister2->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, CAPTURE_THE_FLAG_PAGE_INFO_BATTLE_BUTTON_REGISTER_2 );
	m_pButtonRegister2->CreateBaseButton ( "COMPETITION_CAPTURE_THE_FLAG_PAGE_BATTLE_BUTTON_2", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, ID2GAMEWORD("COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 0 ) );
	RegisterControl ( m_pButtonRegister2 );
}

CBasicTextBox* CPVPCaptureTheFlagPageBattle::CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign )
{
	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, szControl );
	pTextBox->SetFont ( pFont );
	pTextBox->SetTextAlign ( nTextAlign );
	RegisterControl ( pTextBox );
	return pTextBox;
}

void CPVPCaptureTheFlagPageBattle::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch( ControlID )
	{
	case CAPTURE_THE_FLAG_PAGE_INFO_BATTLE_BUTTON_REGISTER_1:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg) )
			{
				GLPVPCaptureTheFlagClient::GetInstance().DoRegister( CAPTURE_THE_FLAG_TEAM_A );
			}
		}break;

	case CAPTURE_THE_FLAG_PAGE_INFO_BATTLE_BUTTON_REGISTER_2:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg) )
			{
				GLPVPCaptureTheFlagClient::GetInstance().DoRegister( CAPTURE_THE_FLAG_TEAM_B );
			}
		}break;
	};
}

void CPVPCaptureTheFlagPageBattle::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	m_fUpdateTime += fElapsedTime;
	if ( m_fUpdateTime >= 0.5f )
	{
		ShowInfo();
		m_fUpdateTime = 0.0f;
	}
}

void CPVPCaptureTheFlagPageBattle::ShowInfo()
{
	GLPVPCaptureTheFlagClient* pClient = &GLPVPCaptureTheFlagClient::GetInstance();
	if ( !pClient )	return;

	if ( m_pTextBattleTimeStartTimer )
	{
		m_pTextBattleTimeStartTimer->ClearText();
		if ( pClient->m_sScheduleNext.dwIndex != UINT_MAX )
		{
			CString strCombine;
			strCombine.Format ( "%02d:%02d", pClient->m_sScheduleNext.wBattleStartHour, pClient->m_sScheduleNext.wBattleStartMinute );
			m_pTextBattleTimeStartTimer->AddText( strCombine.GetString(), NS_UITEXTCOLOR::WHITE  );
		}
		else
		{
			m_pTextBattleTimeStartTimer->AddText( "00:00", NS_UITEXTCOLOR::WHITE  );
		}
	}

	if ( m_pTextBattleTimeEndTimer )
	{
		m_pTextBattleTimeEndTimer->ClearText();
		if ( pClient->m_sScheduleNext.dwIndex != UINT_MAX )
		{
			CString strCombine;
			strCombine.Format ( "%02d:%02d", pClient->m_sScheduleNext.wBattleEndHour, pClient->m_sScheduleNext.wBattleEndMinute );
			m_pTextBattleTimeEndTimer->AddText( strCombine.GetString(), NS_UITEXTCOLOR::WHITE  );
		}
		else
		{
			m_pTextBattleTimeEndTimer->AddText( "00:00", NS_UITEXTCOLOR::WHITE  );
		}
	}

	if ( m_pTextBattleProgressStatic )
	{
		m_pTextBattleProgressStatic->ClearText();
		if ( pClient->m_emState == CAPTURE_THE_FLAG_STATE_REGISTER )
			m_pTextBattleProgressStatic->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_PROGRESS_TEXT", 0 ), NS_UITEXTCOLOR::ORANGE );
		else if ( pClient->m_emState == CAPTURE_THE_FLAG_STATE_BATTLE )
			m_pTextBattleProgressStatic->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_PROGRESS_TEXT", 1 ), NS_UITEXTCOLOR::ORANGE );
		else if ( pClient->m_emState == CAPTURE_THE_FLAG_STATE_REWARD)
			m_pTextBattleProgressStatic->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_PROGRESS_TEXT", 2 ), NS_UITEXTCOLOR::ORANGE );
		else
			m_pTextBattleProgressStatic->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_STATIC", 2 ), NS_UITEXTCOLOR::WHITE );
	}

	if ( m_pTextBattleProgress )
	{
		m_pTextBattleProgress->ClearText();
		if ( pClient->m_fRemain > 0.0f && ( pClient->m_emState == CAPTURE_THE_FLAG_STATE_REGISTER || pClient->m_emState == CAPTURE_THE_FLAG_STATE_BATTLE || pClient->m_emState == CAPTURE_THE_FLAG_STATE_REWARD ) )
		{
			DWORD dwTextColor = NS_UITEXTCOLOR::WHITE;
			if ( pClient->m_fRemain < 300.0f ){
				dwTextColor = NS_UITEXTCOLOR::RED;
			}
			CString strCombine;
			strCombine.Format ( "%02d:%02d", (DWORD)pClient->m_fRemain/60, (DWORD)pClient->m_fRemain%60 );
			m_pTextBattleProgress->AddText( strCombine.GetString(), dwTextColor );
		}
		else
		{
			m_pTextBattleProgress->AddText( "00:00", NS_UITEXTCOLOR::WHITE  );
		}
	}

	if ( m_pTextBattleStatus )
	{
		m_pTextBattleStatus->ClearText();

		if ( DxGlobalStage::GetInstance().GetChannel() != 0 )
		{
			m_pTextBattleStatus->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 9 ), NS_UITEXTCOLOR::RED );
		}else{
			if ( pClient->m_emState == CAPTURE_THE_FLAG_STATE_REGISTER )
			{
				if (pClient->m_bRegistered )
				{
					if ( pClient->m_wTeam == CAPTURE_THE_FLAG_TEAM_A )
						m_pTextBattleStatus->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 11 ), NS_UITEXTCOLOR::ORANGE );
					else if ( pClient->m_wTeam == CAPTURE_THE_FLAG_TEAM_B )
						m_pTextBattleStatus->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 12 ), NS_UITEXTCOLOR::ORANGE );
					else 
						m_pTextBattleStatus->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 5 ), NS_UITEXTCOLOR::ORANGE );
				}
				else
					m_pTextBattleStatus->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 6 ), NS_UITEXTCOLOR::GREENYELLOW );
			}else{
				if ( pClient->m_bRegistered )
				{
					if ( pClient->m_wTeam == CAPTURE_THE_FLAG_TEAM_A )
						m_pTextBattleStatus->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 13 ), NS_UITEXTCOLOR::ORANGE );
					else if ( pClient->m_wTeam == CAPTURE_THE_FLAG_TEAM_B )
						m_pTextBattleStatus->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 14 ), NS_UITEXTCOLOR::ORANGE );
					else
						m_pTextBattleStatus->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 10 ), NS_UITEXTCOLOR::ORANGE );
				}
				else
					m_pTextBattleStatus->AddText( ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 3), NS_UITEXTCOLOR::RED );
			}
		}
	}

	if ( m_pButtonRegister1 )
	{
		if ( pClient->m_bRegistered && pClient->m_emState == CAPTURE_THE_FLAG_STATE_REGISTER && pClient->m_wTeam == CAPTURE_THE_FLAG_TEAM_A )
			m_pButtonRegister1->SetOneLineText(ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 1 ));
		else if ( pClient->m_bRegistered && pClient->m_emState == CAPTURE_THE_FLAG_STATE_BATTLE && pClient->m_wTeam == CAPTURE_THE_FLAG_TEAM_A )
			m_pButtonRegister1->SetOneLineText(ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 10 ));
		else if ( pClient->m_bRegistered && pClient->m_wTeam != CAPTURE_THE_FLAG_TEAM_A )
			m_pButtonRegister1->SetOneLineText(ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 3 ));
		else
			m_pButtonRegister1->SetOneLineText(ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 0 ));
	}

	if ( m_pButtonRegister2 )
	{
		if ( pClient->m_bRegistered && pClient->m_emState == CAPTURE_THE_FLAG_STATE_REGISTER && pClient->m_wTeam == CAPTURE_THE_FLAG_TEAM_B )
			m_pButtonRegister2->SetOneLineText(ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 1 ));
		else if ( pClient->m_bRegistered && pClient->m_emState == CAPTURE_THE_FLAG_STATE_BATTLE && pClient->m_wTeam == CAPTURE_THE_FLAG_TEAM_B )
			m_pButtonRegister2->SetOneLineText(ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 10 ));
		else if ( pClient->m_bRegistered && pClient->m_wTeam != CAPTURE_THE_FLAG_TEAM_B )
			m_pButtonRegister2->SetOneLineText(ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 3 ));
		else
			m_pButtonRegister2->SetOneLineText(ID2GAMEWORD( "COMPETITION_CAPTURE_THE_FLAG_BATTLE_TEXT", 0 ));
	}


	if ( m_pTextPlayerNum1 )
	{
		m_pTextPlayerNum1->ClearText();

		CString strCombine;
		strCombine.Format ( "%d", pClient->m_wPlayerNum[CAPTURE_THE_FLAG_TEAM_A] );
		m_pTextPlayerNum1->AddText( strCombine.GetString(), NS_UITEXTCOLOR::WHITE );
	}

	if ( m_pTextPlayerNum2 )
	{
		m_pTextPlayerNum2->ClearText();

		CString strCombine;
		strCombine.Format ( "%d", pClient->m_wPlayerNum[CAPTURE_THE_FLAG_TEAM_B] );
		m_pTextPlayerNum2->AddText( strCombine.GetString(), NS_UITEXTCOLOR::WHITE );
	}
}
