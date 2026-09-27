#include "StdAfx.h"
#include "PVPCaptureTheFlagPage.h"
#include "PVPCaptureTheFlagPageBattle.h"
#include "PVPCaptureTheFlagPageHistory.h"

#include "d3dfont.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicTextButton.h"

#include "GLPVPCaptureTheFlagClient.h"
#include "InnerInterface.h"
#include "GLGaeaClient.h"
#include "DxGlobalStage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPCaptureTheFlagPage::CPVPCaptureTheFlagPage ()
	: m_pTextMapName(NULL)
	, m_pTextInfoTarget(NULL)
	, m_pTextInfoDetail(NULL)
	, m_pTextInfoReward(NULL)
	, m_pInfoBattle(NULL)
	, m_pInfoHistory(NULL)
	, m_fUpdateTime(0.0f)
{
}

CPVPCaptureTheFlagPage::~CPVPCaptureTheFlagPage ()
{
}

void CPVPCaptureTheFlagPage::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CreateUILineBoxQuestList( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_LINEBOX_HEAD" );
	CreateUILineBoxQuestList( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_LINEBOX_HEAD" );
	CreateUILineBoxQuestList( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_LINEBOX_LEFT" );
	CreateUILineBoxQuestList( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_LINEBOX_RIGHT" );

	CreateUILineBoxWhiteNoBody( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_LINEBOX_HEAD" );
	CreateUILineBoxWhiteNoBody( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_LINEBOX_LEFT" );
	CreateUILineBoxWhiteNoBody( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_LINEBOX_RIGHT" );

	CreateUILineBoxQuestList( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_LINEBOX_MAP" );
	CreateUILineBoxWhiteNoBody( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_LINEBOX_MAP" );

	CreateUILineBoxQuestList( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_LINEBOX_LEFT_1" );
	CreateUILineBoxWhiteNoBody( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_LINEBOX_LEFT_1" );

	CreateUILineBoxWhiteNoBody( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_LINEBOX_RIGHT_1" );
	CreateUILineBoxWhiteNoBody( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_LINEBOX_RIGHT_2" );
	CreateUILineBoxWhiteNoBody( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_LINEBOX_RIGHT_3" );
	CreateUILineBoxWhiteNoBody( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_LINEBOX_RIGHT_4" );

	CreateUIControl( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_MAP" );

	m_pTextMapName = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_TITLE", pFont9, TEXT_ALIGN_CENTER_X| TEXT_ALIGN_CENTER_Y );
	m_pTextMapName->AddText( ID2GAMEWORD( "COMPETITION_BUTTON", 2 ), NS_UITEXTCOLOR::ORANGE );
	m_pTextInfoTarget = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_TEXT_RIGHT_1", pFont9, TEXT_ALIGN_LEFT );
	m_pTextInfoDetail = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_TEXT_RIGHT_2", pFont9, TEXT_ALIGN_LEFT );
	m_pTextInfoReward = CreateUITextBox( "COMPETITION_CAPTURE_THE_FLAG_PAGE_INFO_TEXT_RIGHT_3", pFont9, TEXT_ALIGN_LEFT );

	m_pInfoBattle = new CPVPCaptureTheFlagPageBattle;
	m_pInfoBattle->CreateSub( this, "COMPETITION_CAPTURE_THE_FLAG_PAGE_BATTLE", UI_FLAG_DEFAULT, CAPTURE_THE_FLAG_PAGE_INFO_BATTLE );
	m_pInfoBattle->CreateSubControl ();
	RegisterControl ( m_pInfoBattle );

	m_pInfoHistory = new CPVPCaptureTheFlagPageHistory;
	m_pInfoHistory->CreateSub( this, "COMPETITION_CAPTURE_THE_FLAG_PAGE_HISTORY", UI_FLAG_DEFAULT, CAPTURE_THE_FLAG_PAGE_INFO_HISTORY );
	m_pInfoHistory->CreateSubControl ();
	RegisterControl ( m_pInfoHistory );
}

CBasicTextBox* CPVPCaptureTheFlagPage::CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign )
{
	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, szControl );
	pTextBox->SetFont ( pFont );
	pTextBox->SetTextAlign ( nTextAlign );
	RegisterControl ( pTextBox );
	return pTextBox;
}

CBasicLineBox* CPVPCaptureTheFlagPage::CreateUILineBoxQuestList( char* szBaseControl )
{
	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( szBaseControl );
	RegisterControl ( pLineBox );
	return pLineBox;
}

CBasicLineBox* CPVPCaptureTheFlagPage::CreateUILineBoxWhiteNoBody( char* szBaseControl )
{
	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody ( szBaseControl );
	RegisterControl ( pLineBox );

	return pLineBox;
}

CUIControl* CPVPCaptureTheFlagPage::CreateUIControl( const char* szControl )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl, UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pControl->SetVisibleSingle( TRUE );
	RegisterControl ( pControl );
	return pControl;
}

void CPVPCaptureTheFlagPage::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );
}

void CPVPCaptureTheFlagPage::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	m_fUpdateTime += fElapsedTime;
	if ( m_fUpdateTime >= 0.5f )
	{
		UpdateInfo();
		m_fUpdateTime = 0.0f;
	}
}

void CPVPCaptureTheFlagPage::UpdateInfo()
{
	CString strCombine;
	int nIndex = 0;

	if ( m_pTextInfoTarget )
	{
		m_pTextInfoTarget->ClearText();
		m_pTextInfoTarget->AddText( ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_INFO_TEXT", 0 ), NS_UITEXTCOLOR::WHITE );
		m_pTextInfoTarget->AddText( ID2GAMEINTEXT( "PVP_CAPTURE_THE_FLAG_INFO_TARGET" ), NS_UITEXTCOLOR::WHITE );
	}

	if ( m_pTextInfoDetail )
	{
		m_pTextInfoDetail->ClearText();
		m_pTextInfoDetail->AddText( ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_INFO_TEXT", 1 ), NS_UITEXTCOLOR::WHITE );

		strCombine.Format ( ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_INFO_TEXT", 3 ), 
			ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_TEAM_TEXT", 0 ), ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_TEAM_TEXT", 1 ) );
		m_pTextInfoDetail->AddText( strCombine.GetString(), NS_UITEXTCOLOR::WHITE );

		strCombine.Format ( ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_INFO_TEXT", 4 ), GLPVPCaptureTheFlagClient::GetInstance().m_wPlayerLimit );
		m_pTextInfoDetail->AddText( strCombine.GetString(), NS_UITEXTCOLOR::WHITE );

		strCombine.Format ( ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_INFO_TEXT", 10 ), GLPVPCaptureTheFlagClient::GetInstance().m_llContributionReq );
		m_pTextInfoDetail->AddText( strCombine.GetString(), NS_UITEXTCOLOR::WHITE );

		strCombine.Format ( ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_INFO_TEXT", 5 ), GLPVPCaptureTheFlagClient::GetInstance().m_wLevelReq );
		m_pTextInfoDetail->AddText( strCombine.GetString(), NS_UITEXTCOLOR::WHITE );

		strCombine.Format ( ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_INFO_TEXT", 6 ), GLPVPCaptureTheFlagClient::GetInstance().m_wMaxScore );
		m_pTextInfoDetail->AddText( strCombine.GetString(), NS_UITEXTCOLOR::WHITE );

		strCombine.Format ( ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_INFO_TEXT", 9 ), (WORD)GLPVPCaptureTheFlagClient::GetInstance().m_wBattleTime/60 );
		m_pTextInfoDetail->AddText( strCombine.GetString(), NS_UITEXTCOLOR::WHITE );
	}

	if ( m_pTextInfoReward )
	{
		m_pTextInfoReward->ClearText();
		m_pTextInfoReward->AddText( ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_INFO_TEXT", 2 ), NS_UITEXTCOLOR::WHITE );

		nIndex = m_pTextInfoReward->AddText( ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_INFO_TEXT", 7 ), NS_UITEXTCOLOR::GREENYELLOW );
		m_pTextInfoReward->AddString( nIndex, ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_REWARD_TEXT", 0 ), NS_UITEXTCOLOR::GREENYELLOW );
		
		nIndex = m_pTextInfoReward->AddText( ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_INFO_TEXT", 8 ), NS_UITEXTCOLOR::RED );
		m_pTextInfoReward->AddString( nIndex, ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_REWARD_TEXT", 1 ), NS_UITEXTCOLOR::RED );
	}
}