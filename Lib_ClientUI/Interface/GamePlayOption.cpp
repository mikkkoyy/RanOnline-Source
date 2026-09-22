#include "StdAfx.h"
#include "GamePlayOption.h"
#include "./BasicButton.h"
#include "./BasicTextBox.h"
#include "GameTextControl.h"
#include "UITextControl.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"

#include "DxParamSet.h"
#include "RANPARAM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CBasicGamePlayOption::CBasicGamePlayOption () :
	m_pFont ( NULL ),
	m_pConftButton ( NULL ),
	m_pTradeButton ( NULL ),
	m_pPartyButton ( NULL ),
	m_pSimpleHPButton ( NULL ),
	m_pFriendButton ( NULL ),
	m_pMovableOnChatButton ( NULL ),
	m_pShowTipButton ( NULL ),

	/*character simple, Juver, 2017/10/01 */
	m_pCharacterSimpleButton(NULL),  
	m_bCharacterSimple(FALSE),

	m_pCharacterMiscButton(NULL),
	m_bCharacterMisc(FALSE),

	/*skill effect setting, Juver, 2017/10/01 */
	m_pHideSkillEffectButton(NULL),  
	m_bHideSkillEffect(FALSE),

	m_pHideDamageDisplayButton(NULL),
	m_bHideDamageDisplay(FALSE),

	m_pSpecialNameButton(NULL),
	m_bSpecialName(FALSE),

	m_pClassicNameDisplayButton(NULL),
	m_bClassicNameDisplay(FALSE),

	m_pKillFeedDisplayButton(NULL),
	m_bKillFeedDisplay(FALSE),

	m_pHideDeadNameButton(NULL),
	m_bHideDeadName(FALSE),

	m_pHideDeadBodyButton(NULL),
	m_bHideDeadBody(FALSE),


	/*charinfoview , Juver, 2017/11/12 */
	m_pPrivateStats(NULL),
	m_bPrivateStats(TRUE)
{
}

CBasicGamePlayOption::~CBasicGamePlayOption ()
{
}

void CBasicGamePlayOption::CreateSubControl ()
{
	m_pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

	CBasicTextBox* pTextBox = NULL;

	{	//	기본 텍스트		
		DWORD dwFontColor = NS_UITEXTCOLOR::DEFAULT;
		int nAlign = TEXT_ALIGN_LEFT;

		//	자동 대련 거부
		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_CONFT_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_OPTION", 0 ) );

		//	자동 거래 거부
		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_TRADE_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_OPTION", 1 ) );

		//	자동 파티 거부
		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_PARTY_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_OPTION", 2 ) );

		//	미니 HP 표시
		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_SIMPLEHP_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_OPTION", 3 ) );

		//	자동 친구 거부
		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_FRIEND_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_OPTION", 4 ) );

		//	게임창 기본 위치로
		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_DEFAULTPOS_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_OPTION", 5 ) );

		//  채팅창 뚫기
		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_MOVABLE_ON_CHAT_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_OPTION", 6 ) );

		//  팁 보기
		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_SHOW_TIP_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_OPTION", 7 ) );

		//  강제 공격
		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_FORCED_ATTACK_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_OPTION", 8 ) );

		//  이름 항상 표시
		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_NAME_DISPLAY_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_OPTION", 9 ) );

		//  부활 스킬 금지
		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_REVIVAL_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_OPTION", 10 ) );

		/*character simple, Juver, 2017/10/01 */
		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_CHARACTER_SIMPLE_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_EX", 0 ) );

		/*skill effect setting, Juver, 2017/10/01 */
		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_HIDE_SKILL_EFFECT_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_EX", 1 ) );

		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_CHARACTER_MISC_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_EX", 3 ) );

		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_HIDE_DAMAGE_DISPLAY_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_EX", 4 ) );

		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_SPECIAL_NAME_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_EX", 5 ) );

		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_CLASSIC_NAME_DISPLAY_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_EX", 6 ) );

		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_KILL_FEED_DISPLAY_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_EX", 7 ) );

		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_HIDE_DEAD_BODY_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_EX", 8 ) );

		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_HIDE_DEAD_NAME_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_EX", 9 ) );

		/*charinfoview , Juver, 2017/11/12 */
		pTextBox = CreateStaticControl ( "HWOPTION_GAMEPLAY_PRIVATE_STATS_STATIC", m_pFont, dwFontColor, nAlign );
		pTextBox->SetOneLineText ( ID2GAMEWORD ( "HWOPTION_GAMEPLAY_EX", 2 ) );
		pTextBox->SetVisibleSingle(FALSE);
	}

	{	//	기능
		//	버튼
		m_pConftButton = CreateFlipButton( "HWOPTION_GAMEPLAY_CONFT_BUTTON", "HWOPTION_GAMEPLAY_CONFT_BUTTON_F", HWOPTION_GAMEPLAY_CONFT_BUTTON );
		m_pTradeButton = CreateFlipButton( "HWOPTION_GAMEPLAY_TRADE_BUTTON", "HWOPTION_GAMEPLAY_TRADE_BUTTON_F", HWOPTION_GAMEPLAY_TRADE_BUTTON );
		m_pPartyButton = CreateFlipButton( "HWOPTION_GAMEPLAY_PARTY_BUTTON", "HWOPTION_GAMEPLAY_PARTY_BUTTON_F", HWOPTION_GAMEPLAY_PARTY_BUTTON );
		m_pSimpleHPButton = CreateFlipButton( "HWOPTION_GAMEPLAY_SIMPLEHP_BUTTON", "HWOPTION_GAMEPLAY_SIMPLEHP_BUTTON_F", HWOPTION_GAMEPLAY_SIMPLEHP_BUTTON );
		m_pFriendButton = CreateFlipButton( "HWOPTION_GAMEPLAY_FRIEND_BUTTON", "HWOPTION_GAMEPLAY_FRIEND_BUTTON_F", HWOPTION_GAMEPLAY_FRIEND_BUTTON );
		m_pMovableOnChatButton = CreateFlipButton( "HWOPTION_GAMEPLAY_MOVABLE_ON_CHAT_BUTTON", "HWOPTION_GAMEPLAY_MOVABLE_ON_CHAT_BUTTON_F", HWOPTION_GAMEPLAY_MOVABLE_ON_CHAT_BUTTON );
		m_pShowTipButton = CreateFlipButton( "HWOPTION_GAMEPLAY_SHOW_TIP_BUTTON", "HWOPTION_GAMEPLAY_SHOW_TIP_BUTTON_F", HWOPTION_GAMEPLAY_SHOW_TIP_BUTTON );
		m_pForcedAttackButton = CreateFlipButton( "HWOPTION_GAMEPLAY_FORCED_ATTACK_BUTTON", "HWOPTION_GAMEPLAY_FORCED_ATTACK_BUTTON_F", HWOPTION_GAMEPLAY_FORCED_ATTACK_BUTTON );
		m_pNameDisplayButton = CreateFlipButton( "HWOPTION_GAMEPLAY_NAME_DISPLAY_BUTTON", "HWOPTION_GAMEPLAY_NAME_DISPLAY_BUTTON_F", HWOPTION_GAMEPLAY_NAME_DISPLAY_BUTTON );
		m_pRevivalButton = CreateFlipButton( "HWOPTION_GAMEPLAY_REVIVAL_BUTTON", "HWOPTION_GAMEPLAY_REVIVAL_BUTTON_F", HWOPTION_GAMEPLAY_REVIVAL_BUTTON );

		/*character simple, Juver, 2017/10/01 */
		m_pCharacterSimpleButton = CreateFlipButton( "HWOPTION_GAMEPLAY_CHARACTER_SIMPLE_BUTTON", "HWOPTION_GAMEPLAY_CHARACTER_SIMPLE_BUTTON_F", HWOPTION_GAMEPLAY_CHARACTER_SIMPLE_BUTTON );

		/*skill effect setting, Juver, 2017/10/01 */
		m_pHideSkillEffectButton = CreateFlipButton( "HWOPTION_GAMEPLAY_HIDE_SKILL_EFFECT_BUTTON", "HWOPTION_GAMEPLAY_HIDE_SKILL_EFFECT_BUTTON_F", HWOPTION_GAMEPLAY_HIDE_SKILL_EFFECT_BUTTON );

		m_pCharacterMiscButton = CreateFlipButton( "HWOPTION_GAMEPLAY_CHARACTER_MISC_BUTTON", "HWOPTION_GAMEPLAY_CHARACTER_MISC_BUTTON_F", HWOPTION_GAMEPLAY_CHARACTER_MISC_BUTTON );

		m_pHideDamageDisplayButton = CreateFlipButton( "HWOPTION_GAMEPLAY_HIDE_DAMAGE_DISPLAY_BUTTON", "HWOPTION_GAMEPLAY_HIDE_DAMAGE_DISPLAY_BUTTON_F", HWOPTION_GAMEPLAY_HIDE_DAMAGE_DISPLAY_BUTTON );

		m_pSpecialNameButton = CreateFlipButton( "HWOPTION_GAMEPLAY_SPECIAL_NAME_BUTTON", "HWOPTION_GAMEPLAY_SPECIAL_NAME_BUTTON_F", HWOPTION_GAMEPLAY_SPECIAL_NAME_BUTTON );

		m_pClassicNameDisplayButton = CreateFlipButton( "HWOPTION_GAMEPLAY_CLASSIC_NAME_DISPLAY_BUTTON", "HWOPTION_GAMEPLAY_CLASSIC_NAME_DISPLAY_BUTTON_F", HWOPTION_GAMEPLAY_CLASSIC_NAME_DISPLAY_BUTTON );

		m_pKillFeedDisplayButton = CreateFlipButton( "HWOPTION_GAMEPLAY_KILL_FEED_DISPLAY_BUTTON", "HWOPTION_GAMEPLAY_KILL_FEED_DISPLAY_BUTTON_F", HWOPTION_GAMEPLAY_KILL_FEED_DISPLAY_BUTTON );

		m_pHideDeadBodyButton = CreateFlipButton( "HWOPTION_GAMEPLAY_HIDE_DEAD_BODY_BUTTON", "HWOPTION_GAMEPLAY_HIDE_DEAD_BODY_BUTTON_F", HWOPTION_GAMEPLAY_HIDE_DEAD_BODY_BUTTON );
		m_pHideDeadNameButton = CreateFlipButton( "HWOPTION_GAMEPLAY_HIDE_DEAD_NAME_BUTTON", "HWOPTION_GAMEPLAY_HIDE_DEAD_NAME_BUTTON_F", HWOPTION_GAMEPLAY_HIDE_DEAD_NAME_BUTTON );

		/*charinfoview , Juver, 2017/11/12 */
		m_pPrivateStats = CreateFlipButton( "HWOPTION_GAMEPLAY_PRIVATE_STATS_BUTTON", "HWOPTION_GAMEPLAY_PRIVATE_STATS_BUTTON_F", HWOPTION_GAMEPLAY_PRIVATE_STATS_BUTTON );
		m_pPrivateStats->SetVisibleSingle(FALSE);


		{
			CBasicButton* pButton = new CBasicButton;
			pButton->CreateSub ( this, "HWOPTION_GAMEPLAY_DEFAULTPOS_BUTTON", UI_FLAG_DEFAULT, HWOPTION_GAMEPLAY_DEFAULTPOS_BUTTON );
			pButton->CreateFlip ( "HWOPTION_GAMEPLAY_DEFAULTPOS_BUTTON_F", CBasicButton::CLICK_FLIP );				
			RegisterControl ( pButton );
		}
	}
}

CBasicButton* CBasicGamePlayOption::CreateFlipButton ( char* szButton, char* szButtonFlip, UIGUID ControlID )
{
	CBasicButton* pButton = new CBasicButton;
	pButton->CreateSub ( this, szButton, UI_FLAG_DEFAULT, ControlID );
	pButton->CreateFlip ( szButtonFlip, CBasicButton::RADIO_FLIP );
	pButton->SetControlNameEx ( szButton );
	RegisterControl ( pButton );
	return pButton;
}

CBasicTextBox* CBasicGamePlayOption::CreateStaticControl ( char* szControlKeyword, CD3DFontPar* pFont, D3DCOLOR dwColor, int nAlign )
{
	CBasicTextBox* pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, szControlKeyword );
	pStaticText->SetFont ( pFont );
	pStaticText->SetTextAlign ( nAlign );	
	RegisterControl ( pStaticText );
	return pStaticText;
}

void CBasicGamePlayOption::LoadConft ()
{
	m_bConft = DXPARAMSET::GetInstance().m_bDIS_CONFT;
	m_pConftButton->SetFlip ( m_bConft );
}

void CBasicGamePlayOption::LoadTrade ()
{
	m_bTrade = DXPARAMSET::GetInstance().m_bDIS_TRADE;
	m_pTradeButton->SetFlip ( m_bTrade );
}

void CBasicGamePlayOption::LoadParty ()
{
	m_bParty = DXPARAMSET::GetInstance().m_bDIS_PARTY;
	m_pPartyButton->SetFlip ( m_bParty );
}

void CBasicGamePlayOption::LoadSimpleHP ()
{
	m_bSimpleHP = DXPARAMSET::GetInstance().m_bSHOW_SIMPLEHP;
	m_pSimpleHPButton->SetFlip ( m_bSimpleHP );
}

void CBasicGamePlayOption::LoadFriend ()
{
	m_bFriend = RANPARAM::bDIS_FRIEND;
	m_pFriendButton->SetFlip ( m_bFriend );
}

void CBasicGamePlayOption::LoadMovableOnChat ()
{
	m_bMovableOnChat = RANPARAM::bMOVABLE_ON_CHAT;
	m_pMovableOnChatButton->SetFlip ( m_bMovableOnChat );
}

void CBasicGamePlayOption::LoadShowTip ()
{
	m_bShowTip = RANPARAM::bSHOW_TIP;
	m_pShowTipButton->SetFlip ( m_bShowTip );
}

void CBasicGamePlayOption::LoadForcedAttack ()
{
	m_bForcedAttack = RANPARAM::bFORCED_ATTACK;
	m_pForcedAttackButton->SetFlip ( m_bForcedAttack );
}

void CBasicGamePlayOption::LoadNameDisplay ()
{
	m_bNameDisplay = RANPARAM::bNAME_DISPLAY;
	m_pNameDisplayButton->SetFlip ( m_bNameDisplay );
}

void CBasicGamePlayOption::LoadRevival()
{
	m_bNon_Rebirth = RANPARAM::bNON_Rebirth;
	m_pRevivalButton->SetFlip ( m_bNon_Rebirth );
}

/*charinfoview , Juver, 2017/11/12 */
void CBasicGamePlayOption::LoadPrivateStats()
{
	m_bPrivateStats = RANPARAM::bPrivateStats;
	m_pPrivateStats->SetFlip ( m_bPrivateStats );
}

/*character simple, Juver, 2017/10/01 */
void CBasicGamePlayOption::LoadCharacterSimple()
{
	m_bCharacterSimple = RANPARAM::bCharacterSimple;
	m_pCharacterSimpleButton->SetFlip ( m_bCharacterSimple );
}

/*skill effect setting, Juver, 2017/10/01 */
void CBasicGamePlayOption::LoadHideSkillEffect()
{
	m_bHideSkillEffect = RANPARAM::bHideSkillEffect;
	m_pHideSkillEffectButton->SetFlip ( m_bHideSkillEffect );
}

void CBasicGamePlayOption::LoadCharacterMisc()
{
	m_bCharacterMisc = RANPARAM::bCharacterMisc;
	m_pCharacterMiscButton->SetFlip ( m_bCharacterMisc );
}

void CBasicGamePlayOption::LoadHideDamageDisplay()
{
	m_bHideDamageDisplay = RANPARAM::bHideDamageDisplay;
	m_pHideDamageDisplayButton->SetFlip ( m_bHideDamageDisplay );
}

void CBasicGamePlayOption::LoadSpecialName()
{
	m_bSpecialName = RANPARAM::bSpecialName;
	m_pSpecialNameButton->SetFlip ( m_bSpecialName );
}

void CBasicGamePlayOption::LoadClassicNameDisplay()
{
	m_bClassicNameDisplay = RANPARAM::bClassicNameDisplay;
	m_pClassicNameDisplayButton->SetFlip ( m_bClassicNameDisplay );
}

void CBasicGamePlayOption::LoadKillFeedDisplay()
{
	m_bKillFeedDisplay = RANPARAM::bKillFeedDisplay;
	m_pKillFeedDisplayButton->SetFlip ( m_bKillFeedDisplay );
}

void CBasicGamePlayOption::LoadHideDeadName()
{
	m_bHideDeadName = RANPARAM::bHideDeadName;
	m_pHideDeadNameButton->SetFlip ( m_bHideDeadName);
}

void CBasicGamePlayOption::LoadHideDeadBody()
{
	m_bHideDeadBody = RANPARAM::bHideDeadBody;
	m_pHideDeadBodyButton->SetFlip ( m_bHideDeadBody );
}

void CBasicGamePlayOption::LoadCurrentOption()
{
	LoadConft();
	LoadTrade();
	LoadParty();
	LoadSimpleHP();
	LoadFriend();
	LoadMovableOnChat();
	LoadShowTip();
	LoadForcedAttack();
	LoadNameDisplay();
	LoadRevival();
	LoadCharacterSimple();	/*character simple, Juver, 2017/10/01 */
	LoadHideSkillEffect();	/*skill effect setting, Juver, 2017/10/01 */
	LoadPrivateStats();		/*charinfoview , Juver, 2017/11/12 */
	LoadCharacterMisc();
	LoadSpecialName();
	LoadClassicNameDisplay();
	LoadKillFeedDisplay();
	LoadHideDamageDisplay();
	LoadHideDeadBody();
	LoadHideDeadName();
}

void CBasicGamePlayOption::SetVisibleSingle( BOOL bVisible )
{
	CUIGroup::SetVisibleSingle ( bVisible );

	if( bVisible )
	{
		LoadCurrentOption();
	}
}