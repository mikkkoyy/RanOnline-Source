#include "StdAfx.h"
#include "PVPTyrannyPage.h"
#include "PVPTyrannyPageBattle.h"
#include "PVPTyrannyPageHistory.h"

#include "d3dfont.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicTextButton.h"

#include "GLPVPTyrannyClient.h"
#include "InnerInterface.h"
#include "GLGaeaClient.h"
#include "DxGlobalStage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPTyrannyPage::CPVPTyrannyPage ()
	: m_pTextMapName(NULL)
	, m_pInfoBattle(NULL)
	, m_pInfoHistory(NULL)
	, m_pTextInfoTitle(NULL)
	, m_pTextInfoGoal1(NULL)
	, m_pTextInfoGoal2(NULL)
	, m_pTextInfoMatch(NULL)
	, m_pTextInfoMatch1(NULL)
	, m_pTextInfoMatch2(NULL)
	, m_pTextInfoPlayer(NULL)
	, m_pTextInfoLevel(NULL)
	, m_pTextReward(NULL)
	, m_fUpdateTime(0.0f)
{
}

CPVPTyrannyPage::~CPVPTyrannyPage ()
{
}

void CPVPTyrannyPage::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CreateUILineBoxQuestList( "COMPETITION_CTF_PAGE_INFO_LINEBOX_HEAD" );
	CreateUILineBoxQuestList( "COMPETITION_CTF_PAGE_INFO_LINEBOX_HEAD" );
	CreateUILineBoxQuestList( "COMPETITION_CTF_PAGE_INFO_LINEBOX_BODY" );
	CreateUILineBoxQuestList( "COMPETITION_CTF_PAGE_INFO_LINEBOX_MAP" );
	CreateUILineBoxQuestList( "COMPETITION_CTF_PAGE_INFO_LINEBOX_BOTTOM" );

	CreateUILineBoxWhiteNoBody( "COMPETITION_CTF_PAGE_INFO_LINEBOX_HEAD" );
	CreateUILineBoxWhiteNoBody( "COMPETITION_CTF_PAGE_INFO_LINEBOX_BODY" );
	CreateUILineBoxWhiteNoBody( "COMPETITION_CTF_PAGE_INFO_LINEBOX_MAP" );
	CreateUILineBoxWhiteNoBody( "COMPETITION_CTF_PAGE_INFO_LINEBOX_BOTTOM" );

	//battle info
	CreateUIControl( "COMPETITION_CTF_PAGE_INFO_MAP" );
	CreateUIControl( "COMPETITION_CTF_PAGE_INFO_RACE_SCHOOL0" );
	CreateUIControl( "COMPETITION_CTF_PAGE_INFO_RACE_SCHOOL1" );
	CreateUIControl( "COMPETITION_CTF_PAGE_INFO_RACE_SCHOOL2" );

	m_pTextMapName = CreateUITextBox( "COMPETITION_CTF_PAGE_INFO_TITLE", pFont9, TEXT_ALIGN_LEFT| TEXT_ALIGN_CENTER_Y );
	m_pTextMapName->AddText( ID2GAMEWORD( "COMPETITION_BUTTON", 0 ), NS_UITEXTCOLOR::ORANGE );
	m_pTextInfoTitle = CreateUITextBox( "COMPETITION_CTF_PAGE_INFO_DESC_TITLE", pFont9, TEXT_ALIGN_LEFT );
	m_pTextInfoGoal1 = CreateUITextBox( "COMPETITION_CTF_PAGE_INFO_DESC_GOAL1", pFont9, TEXT_ALIGN_LEFT );
	m_pTextInfoGoal2 = CreateUITextBox( "COMPETITION_CTF_PAGE_INFO_DESC_GOAL2", pFont9, TEXT_ALIGN_LEFT );
	m_pTextInfoMatch = CreateUITextBox( "COMPETITION_CTF_PAGE_INFO_DESC_MATCH", pFont9, TEXT_ALIGN_LEFT );
	m_pTextInfoMatch1 = CreateUITextBox( "COMPETITION_CTF_PAGE_INFO_DESC_MATCH1", pFont9, TEXT_ALIGN_LEFT );
	m_pTextInfoMatch2 = CreateUITextBox( "COMPETITION_CTF_PAGE_INFO_DESC_MATCH2", pFont9, TEXT_ALIGN_LEFT );
	m_pTextInfoPlayer = CreateUITextBox( "COMPETITION_CTF_PAGE_INFO_DESC_PLAYER", pFont9, TEXT_ALIGN_LEFT );
	m_pTextInfoLevel = CreateUITextBox( "COMPETITION_CTF_PAGE_INFO_DESC_LEVEL", pFont9, TEXT_ALIGN_LEFT );
	m_pTextReward = CreateUITextBox( "COMPETITION_CTF_PAGE_INFO_AWARD_TEXT", pFont9, TEXT_ALIGN_LEFT );

	//battle info
	m_pInfoBattle = new CPVPTyrannyPageBattle;
	m_pInfoBattle->CreateSub( this, "COMPETITION_CTF_PAGE_BATTLE", UI_FLAG_DEFAULT, TYRANNY_PAGE_INFO_BATTLE );
	m_pInfoBattle->CreateSubControl ();
	RegisterControl ( m_pInfoBattle );

	//history info
	m_pInfoHistory = new CPVPTyrannyPageHistory;
	m_pInfoHistory->CreateSub( this, "COMPETITION_CTF_PAGE_HISTORY", UI_FLAG_DEFAULT, TYRANNY_PAGE_INFO_HISTORY );
	m_pInfoHistory->CreateSubControl ();
	RegisterControl ( m_pInfoHistory );
}

CBasicTextBox* CPVPTyrannyPage::CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign )
{
	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, szControl );
	pTextBox->SetFont ( pFont );
	pTextBox->SetTextAlign ( nTextAlign );
	RegisterControl ( pTextBox );
	return pTextBox;
}

CBasicLineBox* CPVPTyrannyPage::CreateUILineBoxQuestList( char* szBaseControl )
{
	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( szBaseControl );
	RegisterControl ( pLineBox );
	return pLineBox;
}

CBasicLineBox* CPVPTyrannyPage::CreateUILineBoxWhiteNoBody( char* szBaseControl )
{
	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody ( szBaseControl );
	RegisterControl ( pLineBox );

	return pLineBox;
}

CUIControl* CPVPTyrannyPage::CreateUIControl( const char* szControl )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl, UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pControl->SetVisibleSingle( TRUE );
	RegisterControl ( pControl );
	return pControl;
}

void CPVPTyrannyPage::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );
}

void CPVPTyrannyPage::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	m_fUpdateTime += fElapsedTime;
	if ( m_fUpdateTime >= 0.5f )
	{
		UpdateInfo();
		m_fUpdateTime = 0.0f;
	}
}

void CPVPTyrannyPage::UpdateInfo()
{
	CString strCombine;
	int nIndex = 0;

	//title
	if ( m_pTextInfoTitle )
	{
		m_pTextInfoTitle->ClearText();
		m_pTextInfoTitle->AddText( ID2GAMEWORD( "COMPETITION_CTF_BATTLE_INFO_TEXT", 0 ), NS_UITEXTCOLOR::WHITE );
	}

	//goal
	if ( m_pTextInfoGoal1 )
	{
		m_pTextInfoGoal1->ClearText();

		nIndex = m_pTextInfoGoal1->AddText( ID2GAMEWORD( "COMPETITION_CTF_BATTLE_INFO_TEXT", 1 ), NS_UITEXTCOLOR::WHITE );
		strCombine.Format( ID2GAMEWORD( "COMPETITION_CTF_BATTLE_INFO_OBJECTIVE_DESC", 0 ), TYRANNY_TOWER_SIZE );
		m_pTextInfoGoal1->AddString( nIndex, strCombine.GetString(), NS_UITEXTCOLOR::WHITE );
	}

	//goal
	if ( m_pTextInfoGoal2 )
	{
		m_pTextInfoGoal2->ClearText();
		m_pTextInfoGoal2->AddText( ID2GAMEWORD( "COMPETITION_CTF_BATTLE_INFO_OBJECTIVE_DESC", 1 ), NS_UITEXTCOLOR::WHITE );
	}

	//match
	if ( m_pTextInfoMatch )
	{
		m_pTextInfoMatch->ClearText();
		m_pTextInfoMatch->AddText( ID2GAMEWORD( "COMPETITION_CTF_BATTLE_INFO_TEXT", 2 ), NS_UITEXTCOLOR::WHITE );
	}

	if ( m_pTextInfoMatch1 )
	{
		m_pTextInfoMatch1->ClearText();
		m_pTextInfoMatch1->AddText( ID2GAMEWORD( "COMPETITION_CTF_BATTLE_INFO_TEXT", 3 ), NS_UITEXTCOLOR::WHITE );
	}

	if ( m_pTextInfoMatch2 )
	{
		m_pTextInfoMatch2->ClearText();
		m_pTextInfoMatch2->AddText( ID2GAMEWORD( "COMPETITION_CTF_BATTLE_INFO_TEXT", 3 ), NS_UITEXTCOLOR::WHITE );
	}

	//player limit
	if ( m_pTextInfoPlayer )
	{
		m_pTextInfoPlayer->ClearText();

		WORD wPlayerLimit = GLPVPTyrannyClient::GetInstance().m_wPlayerLimit;
		nIndex = m_pTextInfoPlayer->AddText( ID2GAMEWORD( "COMPETITION_CTF_BATTLE_INFO_TEXT", 4 ), NS_UITEXTCOLOR::WHITE );
		strCombine.Format( ID2GAMEWORD( "COMPETITION_CTF_BATTLE_INFO_PLAYERS", 0 ), wPlayerLimit );
		m_pTextInfoPlayer->AddString( nIndex, strCombine.GetString(), NS_UITEXTCOLOR::WHITE );
	}

	//level req
	if ( m_pTextInfoLevel )
	{
		m_pTextInfoLevel->ClearText();

		WORD wLevelReq = GLPVPTyrannyClient::GetInstance().m_wLevelReq;
		nIndex = m_pTextInfoLevel->AddText( ID2GAMEWORD( "COMPETITION_CTF_BATTLE_INFO_TEXT", 5 ), NS_UITEXTCOLOR::WHITE );
		strCombine.Format( ID2GAMEWORD( "COMPETITION_CTF_BATTLE_INFO_MAXLEVEL", 0 ), wLevelReq );
		m_pTextInfoLevel->AddString( nIndex, strCombine.GetString(), NS_UITEXTCOLOR::WHITE );
	}


	if ( m_pTextReward )
	{
		m_pTextReward->ClearText();

		CString strCombine;
		int nIndex = 0;

		//title
		m_pTextReward->AddText( ID2GAMEWORD( "COMPETITION_CTF_BATTLE_AWARD_TEXT", 0 ), NS_UITEXTCOLOR::WHITE );

		//reward win
		nIndex = m_pTextReward->AddText( ID2GAMEWORD( "COMPETITION_CTF_BATTLE_AWARD_TEXT", 1 ), NS_UITEXTCOLOR::GREENYELLOW );
		m_pTextReward->AddString( nIndex, ID2GAMEWORD( "COMPETITION_CTF_BATTLE_AWARD_TYPE", 1 ), NS_UITEXTCOLOR::GREENYELLOW );
		m_pTextReward->AddString( nIndex, ",", NS_UITEXTCOLOR::GREENYELLOW ); 
		strCombine.Format( ID2GAMEWORD( "COMPETITION_CTF_BATTLE_AWARD_TYPE", 3 ), 4 );
		m_pTextReward->AddString( nIndex, strCombine.GetString(), NS_UITEXTCOLOR::GREENYELLOW );

		//reward loss
		nIndex = m_pTextReward->AddText( ID2GAMEWORD( "COMPETITION_CTF_BATTLE_AWARD_TEXT", 2 ), NS_UITEXTCOLOR::RED );
		m_pTextReward->AddString( nIndex, ID2GAMEWORD( "COMPETITION_CTF_BATTLE_AWARD_TYPE", 1 ), NS_UITEXTCOLOR::RED );
	}
}