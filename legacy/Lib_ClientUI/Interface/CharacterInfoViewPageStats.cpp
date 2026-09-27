#include "StdAfx.h"
#include "CharacterInfoViewPageStats.h"
#include "CharacterInfoViewPageStatsRender.h"
#include "CharacterInfoViewPageStatsMark.h"

#include "UITextControl.h"
#include "GameTextControl.h"
#include "../Lib_Engine/DxCommon/d3dfont.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicTextButton.h"
#include "ItemImage.h"
#include "BasicButton.h"

#include "InnerInterface.h"
#include "GLGaeaClient.h"
#include "GLItemMan.h"
#include "BasicChat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCharacterInfoViewPageStats::CCharacterInfoViewPageStats ()
	: m_pLevelText( NULL )
	, m_pPowText( NULL )
	, m_pDexText( NULL )
	, m_pSpiText( NULL )
	, m_pStrText( NULL )
	, m_pStaText( NULL )
	, m_pAttackMinText( NULL )
	, m_pAttackMaxText( NULL )
	, m_pDefenseText( NULL )
	, m_pFightText( NULL )
	, m_pShootText( NULL )
	, m_pEnergyText( NULL )
	, m_pHitText( NULL )
	, m_pAvoidText( NULL )
	, m_pHPText( NULL )
	, m_pMPText( NULL )
	, m_pFireText( NULL )
	, m_pColdText( NULL )
	, m_pElectricText( NULL )
	, m_pPoisonText( NULL )
	, m_pSpiritText( NULL )
	, m_pLevelText_Up( NULL )
	, m_pPowText_Up( NULL )
	, m_pDexText_Up( NULL )
	, m_pSpiText_Up( NULL )
	, m_pStrText_Up( NULL )
	, m_pStaText_Up( NULL )
	, m_pAttackMinText_Up( NULL )
	, m_pAttackMaxText_Up( NULL )
	, m_pDefenseText_Up( NULL )
	, m_pFightText_Up( NULL )
	, m_pShootText_Up( NULL )
	, m_pEnergyText_Up( NULL )
	, m_pHitText_Up( NULL )
	, m_pAvoidText_Up( NULL )
	, m_pHPText_Up( NULL )
	, m_pMPText_Up( NULL )
	, m_pFireText_Up( NULL )
	, m_pColdText_Up( NULL )
	, m_pElectricText_Up( NULL )
	, m_pPoisonText_Up( NULL )
	, m_pSpiritText_Up( NULL )
	, m_pLevelText_Mark( NULL )
	, m_pPowText_Mark( NULL )
	, m_pDexText_Mark( NULL )
	, m_pSpiText_Mark( NULL )
	, m_pStrText_Mark( NULL )
	, m_pStaText_Mark( NULL )
	, m_pAttackMinText_Mark( NULL )
	, m_pAttackMaxText_Mark( NULL )
	, m_pDefenseText_Mark( NULL )
	, m_pFightText_Mark( NULL )
	, m_pShootText_Mark( NULL )
	, m_pEnergyText_Mark( NULL )
	, m_pHitText_Mark( NULL )
	, m_pAvoidText_Mark( NULL )
	, m_pHPText_Mark( NULL )
	, m_pMPText_Mark( NULL )
	, m_pSPText_Mark( NULL )
	, m_pFireText_Mark( NULL )
	, m_pColdText_Mark( NULL )
	, m_pElectricText_Mark( NULL )
	, m_pPoisonText_Mark( NULL )
	, m_pSpiritText_Mark( NULL )
	, m_pBadgeText(NULL)
	, m_pNameText( NULL )
	, m_pClassText( NULL )
	, m_pSchoolText( NULL )
	, m_pGuildText( NULL )
	, m_pButtonWhisper( NULL )
	, m_pButtonAddFriend( NULL )
	, m_pButtonInviteParty( NULL )
	, m_pButtonRefresh( NULL )
	, m_pRender(NULL)
	, m_pButtonLeft(NULL)
	, m_pButtonRight(NULL)
{
	for( int i=0; i<INFO_ICON_CLASS; ++i )
		m_pClassIcon[i] = NULL;

	for( int i=0; i<INFO_ICON_SCHOOL; ++i )
		m_pSchoolIcon[i] = NULL;

	for( int i=0; i<INFO_ITEM_IMAGE_SIZE; ++i )
		m_pItemImage[i] = NULL;
}

CCharacterInfoViewPageStats::~CCharacterInfoViewPageStats ()
{
}

void CCharacterInfoViewPageStats::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxWhite( "RAN_ANOTHER_CHAR_REGION_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "RAN_ANOTHER_CHARACTER_WEAR_REGION" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "RAN_ANOTHER_CHARACTER_CHECK_REGION" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "RAN_ANOTHER_CHARACTER_BADGE_REGION" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "RAN_ANOTHER_CHARACTER_LEVEL_REGION" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "RAN_ANOTHER_CHARACTER_MYLEVEL_REGION" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "RAN_ANOTHER_CHARACTER_STATUS_REGION" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "RAN_ANOTHER_CHARACTER_MYSTATUS_REGION" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "RAN_ANOTHER_CHARACTER_ABILITY_REGION" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "RAN_ANOTHER_CHARACTER_MYABILITY_REGION" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "RAN_ANOTHER_CHARACTER_RESISTANCE_REGION" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "RAN_ANOTHER_CHARACTER_MYRESISTANCE_REGION" );
	RegisterControl ( pBasicLineBox );


	CBasicTextBox* pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_0_LEVEL_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 0), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_POW_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 1), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_DEX_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 2), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_SPI_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 3), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_STR_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 4), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_STA_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 5), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_ATTACK_MIN_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 7), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_ATTACK_MAX_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 8), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_DEFENSE_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 9), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_FIGHT_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 10), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_SHOOT_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 11), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_STAMINA_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 12), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_HIT_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 13), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_AVOID_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 14), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_HP_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 15), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_MP_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 16), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_SP_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 17), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_FIRE_RESIST_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 18), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_COLD_RESIST_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 19), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_ELECTRIC_RESIST_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 20), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_POISON_RESIST_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 21), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_SPIRIT_RESIST_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 22), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_4_BADGE_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 31), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_4_NAME_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 23), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_4_CLASS_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 24), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_4_SCHOOL_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 25), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_4_GUILD_TEXT" );
	pStaticText->SetFont ( pFont8 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	pStaticText->SetText( ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 26), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );


	m_pLevelText = new CBasicTextBox;
	m_pLevelText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_0_LEVEL_VALUE" );
	m_pLevelText->SetFont ( pFont8 );
	m_pLevelText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pLevelText->SetText( "001", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pLevelText );

	m_pPowText = new CBasicTextBox;
	m_pPowText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_POW_VALUE" );
	m_pPowText->SetFont ( pFont8 );
	m_pPowText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pPowText->SetText( "002", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pPowText );

	m_pDexText = new CBasicTextBox;
	m_pDexText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_DEX_VALUE" );
	m_pDexText->SetFont ( pFont8 );
	m_pDexText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pDexText->SetText( "003", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pDexText );

	m_pSpiText = new CBasicTextBox;
	m_pSpiText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_SPI_VALUE" );
	m_pSpiText->SetFont ( pFont8 );
	m_pSpiText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pSpiText->SetText( "004", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pSpiText );

	m_pStrText = new CBasicTextBox;
	m_pStrText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_STR_VALUE" );
	m_pStrText->SetFont ( pFont8 );
	m_pStrText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pStrText->SetText( "005", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pStrText );

	m_pStaText = new CBasicTextBox;
	m_pStaText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_STA_VALUE" );
	m_pStaText->SetFont ( pFont8 );
	m_pStaText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pStaText->SetText( "006", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pStaText );

	m_pAttackMinText = new CBasicTextBox;
	m_pAttackMinText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_ATTACK_MIN_VALUE" );
	m_pAttackMinText->SetFont ( pFont8 );
	m_pAttackMinText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pAttackMinText->SetText( "007", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pAttackMinText );

	m_pAttackMaxText = new CBasicTextBox;
	m_pAttackMaxText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_ATTACK_MAX_VALUE" );
	m_pAttackMaxText->SetFont ( pFont8 );
	m_pAttackMaxText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pAttackMaxText->SetText( "008", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pAttackMaxText );

	m_pDefenseText = new CBasicTextBox;
	m_pDefenseText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_DEFENSE_VALUE" );
	m_pDefenseText->SetFont ( pFont8 );
	m_pDefenseText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pDefenseText->SetText( "009", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pDefenseText );

	m_pFightText = new CBasicTextBox;
	m_pFightText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_FIGHT_VALUE" );
	m_pFightText->SetFont ( pFont8 );
	m_pFightText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pFightText->SetText( "010", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pFightText );

	m_pShootText = new CBasicTextBox;
	m_pShootText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_SHOOT_VALUE" );
	m_pShootText->SetFont ( pFont8 );
	m_pShootText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pShootText->SetText( "011", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pShootText );

	m_pEnergyText = new CBasicTextBox;
	m_pEnergyText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_STAMINA_VALUE" );
	m_pEnergyText->SetFont ( pFont8 );
	m_pEnergyText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pEnergyText->SetText( "012", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pEnergyText );

	m_pHitText = new CBasicTextBox;
	m_pHitText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_HIT_VALUE" );
	m_pHitText->SetFont ( pFont8 );
	m_pHitText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pHitText->SetText( "013", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pHitText );

	m_pAvoidText = new CBasicTextBox;
	m_pAvoidText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_AVOID_VALUE" );
	m_pAvoidText->SetFont ( pFont8 );
	m_pAvoidText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pAvoidText->SetText( "014", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pAvoidText );

	m_pHPText = new CBasicTextBox;
	m_pHPText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_HP_VALUE" );
	m_pHPText->SetFont ( pFont8 );
	m_pHPText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pHPText->SetText( "015", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pHPText );

	m_pMPText = new CBasicTextBox;
	m_pMPText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_MP_VALUE" );
	m_pMPText->SetFont ( pFont8 );
	m_pMPText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pMPText->SetText( "016", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pMPText );

	m_pSPText = new CBasicTextBox;
	m_pSPText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_SP_VALUE" );
	m_pSPText->SetFont ( pFont8 );
	m_pSPText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pSPText->SetText( "017", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pSPText );

	m_pFireText = new CBasicTextBox;
	m_pFireText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_FIRE_RESIST_VALUE" );
	m_pFireText->SetFont ( pFont8 );
	m_pFireText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pFireText->SetText( "018", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pFireText );

	m_pColdText = new CBasicTextBox;
	m_pColdText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_COLD_RESIST_VALUE" );
	m_pColdText->SetFont ( pFont8 );
	m_pColdText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pColdText->SetText( "019", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pColdText );

	m_pElectricText = new CBasicTextBox;
	m_pElectricText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_ELECTRIC_RESIST_VALUE" );
	m_pElectricText->SetFont ( pFont8 );
	m_pElectricText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pElectricText->SetText( "020", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pElectricText );

	m_pPoisonText = new CBasicTextBox;
	m_pPoisonText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_POISON_RESIST_VALUE" );
	m_pPoisonText->SetFont ( pFont8 );
	m_pPoisonText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pPoisonText->SetText( "021", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pPoisonText );

	m_pSpiritText = new CBasicTextBox;
	m_pSpiritText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_SPIRIT_RESIST_VALUE" );
	m_pSpiritText->SetFont ( pFont8 );
	m_pSpiritText->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pSpiritText->SetText( "022", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pSpiritText );

	m_pLevelText_Up = new CBasicTextBox;
	m_pLevelText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_0_LEVEL_UP" );
	m_pLevelText_Up->SetFont ( pFont8 );
	m_pLevelText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pLevelText_Up->SetText( "101", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pLevelText_Up );

	m_pPowText_Up = new CBasicTextBox;
	m_pPowText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_POW_UP" );
	m_pPowText_Up->SetFont ( pFont8 );
	m_pPowText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pPowText_Up->SetText( "102", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pPowText_Up );

	m_pDexText_Up = new CBasicTextBox;
	m_pDexText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_DEX_UP" );
	m_pDexText_Up->SetFont ( pFont8 );
	m_pDexText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pDexText_Up->SetText( "103", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pDexText_Up );

	m_pSpiText_Up = new CBasicTextBox;
	m_pSpiText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_SPI_UP" );
	m_pSpiText_Up->SetFont ( pFont8 );
	m_pSpiText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pSpiText_Up->SetText( "104", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pSpiText_Up );

	m_pStrText_Up = new CBasicTextBox;
	m_pStrText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_STR_UP" );
	m_pStrText_Up->SetFont ( pFont8 );
	m_pStrText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pStrText_Up->SetText( "105", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pStrText_Up );

	m_pStaText_Up = new CBasicTextBox;
	m_pStaText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_STA_UP" );
	m_pStaText_Up->SetFont ( pFont8 );
	m_pStaText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pStaText_Up->SetText( "106", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pStaText_Up );

	m_pAttackMinText_Up = new CBasicTextBox;
	m_pAttackMinText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_ATTACK_MIN_UP" );
	m_pAttackMinText_Up->SetFont ( pFont8 );
	m_pAttackMinText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pAttackMinText_Up->SetText( "107", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pAttackMinText_Up );

	m_pAttackMaxText_Up = new CBasicTextBox;
	m_pAttackMaxText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_ATTACK_MAX_UP" );
	m_pAttackMaxText_Up->SetFont ( pFont8 );
	m_pAttackMaxText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pAttackMaxText_Up->SetText( "108", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pAttackMaxText_Up );

	m_pDefenseText_Up = new CBasicTextBox;
	m_pDefenseText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_DEFENSE_UP" );
	m_pDefenseText_Up->SetFont ( pFont8 );
	m_pDefenseText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pDefenseText_Up->SetText( "109", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pDefenseText_Up );

	m_pFightText_Up = new CBasicTextBox;
	m_pFightText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_FIGHT_UP" );
	m_pFightText_Up->SetFont ( pFont8 );
	m_pFightText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pFightText_Up->SetText( "110", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pFightText_Up );

	m_pShootText_Up = new CBasicTextBox;
	m_pShootText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_SHOOT_UP" );
	m_pShootText_Up->SetFont ( pFont8 );
	m_pShootText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pShootText_Up->SetText( "111", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pShootText_Up );

	m_pEnergyText_Up = new CBasicTextBox;
	m_pEnergyText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_STAMINA_UP" );
	m_pEnergyText_Up->SetFont ( pFont8 );
	m_pEnergyText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pEnergyText_Up->SetText( "112", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pEnergyText_Up );

	m_pHitText_Up = new CBasicTextBox;
	m_pHitText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_HIT_UP" );
	m_pHitText_Up->SetFont ( pFont8 );
	m_pHitText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pHitText_Up->SetText( "113", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pHitText_Up );

	m_pAvoidText_Up = new CBasicTextBox;
	m_pAvoidText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_AVOID_UP" );
	m_pAvoidText_Up->SetFont ( pFont8 );
	m_pAvoidText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pAvoidText_Up->SetText( "114", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pAvoidText_Up );

	m_pHPText_Up = new CBasicTextBox;
	m_pHPText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_HP_UP" );
	m_pHPText_Up->SetFont ( pFont8 );
	m_pHPText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pHPText_Up->SetText( "115", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pHPText_Up );

	m_pMPText_Up = new CBasicTextBox;
	m_pMPText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_MP_UP" );
	m_pMPText_Up->SetFont ( pFont8 );
	m_pMPText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pMPText_Up->SetText( "116", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pMPText_Up );

	m_pSPText_Up = new CBasicTextBox;
	m_pSPText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_SP_UP" );
	m_pSPText_Up->SetFont ( pFont8 );
	m_pSPText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pSPText_Up->SetText( "117", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pSPText_Up );

	m_pFireText_Up = new CBasicTextBox;
	m_pFireText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_FIRE_RESIST_UP" );
	m_pFireText_Up->SetFont ( pFont8 );
	m_pFireText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pFireText_Up->SetText( "118", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pFireText_Up );

	m_pColdText_Up = new CBasicTextBox;
	m_pColdText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_COLD_RESIST_UP" );
	m_pColdText_Up->SetFont ( pFont8 );
	m_pColdText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pColdText_Up->SetText( "119", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pColdText_Up );

	m_pElectricText_Up = new CBasicTextBox;
	m_pElectricText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_ELECTRIC_RESIST_UP" );
	m_pElectricText_Up->SetFont ( pFont8 );
	m_pElectricText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pElectricText_Up->SetText( "120", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pElectricText_Up );

	m_pPoisonText_Up = new CBasicTextBox;
	m_pPoisonText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_POISON_RESIST_UP" );
	m_pPoisonText_Up->SetFont ( pFont8 );
	m_pPoisonText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pPoisonText_Up->SetText( "121", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pPoisonText_Up );

	m_pSpiritText_Up = new CBasicTextBox;
	m_pSpiritText_Up->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_SPIRIT_RESIST_UP" );
	m_pSpiritText_Up->SetFont ( pFont8 );
	m_pSpiritText_Up->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pSpiritText_Up->SetText( "122", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pSpiritText_Up );

	m_pLevelText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pLevelText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_0_LEVEL_MARK" );
	m_pLevelText_Mark->CreateSubControl();
	RegisterControl ( m_pLevelText_Mark );

	m_pPowText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pPowText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_POW_MARK" );
	m_pPowText_Mark->CreateSubControl();
	RegisterControl ( m_pPowText_Mark );

	m_pDexText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pDexText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_DEX_MARK" );
	m_pDexText_Mark->CreateSubControl();
	RegisterControl ( m_pDexText_Mark );

	m_pSpiText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pSpiText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_SPI_MARK" );
	m_pSpiText_Mark->CreateSubControl();
	RegisterControl ( m_pSpiText_Mark );

	m_pStrText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pStrText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_STR_MARK" );
	m_pStrText_Mark->CreateSubControl();
	RegisterControl ( m_pStrText_Mark );

	m_pStaText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pStaText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_1_STA_MARK" );
	m_pStaText_Mark->CreateSubControl();
	RegisterControl ( m_pStaText_Mark );

	m_pAttackMinText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pAttackMinText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_ATTACK_MIN_MARK" );
	m_pAttackMinText_Mark->CreateSubControl();
	RegisterControl ( m_pAttackMinText_Mark );

	m_pAttackMaxText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pAttackMaxText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_ATTACK_MAX_MARK" );
	m_pAttackMaxText_Mark->CreateSubControl();
	RegisterControl ( m_pAttackMaxText_Mark );

	m_pDefenseText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pDefenseText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_DEFENSE_MARK" );
	m_pDefenseText_Mark->CreateSubControl();
	RegisterControl ( m_pDefenseText_Mark );

	m_pFightText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pFightText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_FIGHT_MARK" );
	m_pFightText_Mark->CreateSubControl();
	RegisterControl ( m_pFightText_Mark );

	m_pShootText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pShootText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_SHOOT_MARK" );
	m_pShootText_Mark->CreateSubControl();
	RegisterControl ( m_pShootText_Mark );

	m_pEnergyText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pEnergyText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_STAMINA_MARK" );
	m_pEnergyText_Mark->CreateSubControl();
	RegisterControl ( m_pEnergyText_Mark );

	m_pHitText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pHitText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_HIT_MARK" );
	m_pHitText_Mark->CreateSubControl();
	RegisterControl ( m_pHitText_Mark );

	m_pAvoidText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pAvoidText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_AVOID_MARK" );
	m_pAvoidText_Mark->CreateSubControl();
	RegisterControl ( m_pAvoidText_Mark );

	m_pHPText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pHPText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_HP_MARK" );
	m_pHPText_Mark->CreateSubControl();
	RegisterControl ( m_pHPText_Mark );

	m_pMPText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pMPText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_MP_MARK" );
	m_pMPText_Mark->CreateSubControl();
	RegisterControl ( m_pMPText_Mark );

	m_pSPText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pSPText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_2_SP_MARK" );
	m_pSPText_Mark->CreateSubControl();
	RegisterControl ( m_pSPText_Mark );

	m_pFireText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pFireText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_FIRE_RESIST_MARK" );
	m_pFireText_Mark->CreateSubControl();
	RegisterControl ( m_pFireText_Mark );

	m_pColdText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pColdText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_COLD_RESIST_MARK" );
	m_pColdText_Mark->CreateSubControl();
	RegisterControl ( m_pColdText_Mark );

	m_pElectricText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pElectricText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_ELECTRIC_RESIST_MARK" );
	m_pElectricText_Mark->CreateSubControl();
	RegisterControl ( m_pElectricText_Mark );

	m_pPoisonText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pPoisonText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_POISON_RESIST_MARK" );
	m_pPoisonText_Mark->CreateSubControl();
	RegisterControl ( m_pPoisonText_Mark );

	m_pSpiritText_Mark = new CCharacterInfoViewPageStatsMark;
	m_pSpiritText_Mark->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_3_SPIRIT_RESIST_MARK" );
	m_pSpiritText_Mark->CreateSubControl();
	RegisterControl ( m_pSpiritText_Mark );

	m_pBadgeText = new CBasicTextBox;
	m_pBadgeText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_4_BADGE_VALUE" );
	m_pBadgeText->SetFont ( pFont8 );
	m_pBadgeText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pBadgeText->SetText( "022", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pBadgeText );

	m_pNameText = new CBasicTextBox;
	m_pNameText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_4_NAME_VALUE" );
	m_pNameText->SetFont ( pFont8 );
	m_pNameText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pNameText->SetText( "023", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pNameText );

	m_pClassText = new CBasicTextBox;
	m_pClassText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_4_CLASS_VALUE" );
	m_pClassText->SetFont ( pFont8 );
	m_pClassText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pClassText->SetText( "024", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pClassText );

	m_pSchoolText = new CBasicTextBox;
	m_pSchoolText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_4_SCHOOL_VALUE" );
	m_pSchoolText->SetFont ( pFont8 );
	m_pSchoolText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pSchoolText->SetText( "025", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pSchoolText );

	m_pGuildText = new CBasicTextBox;
	m_pGuildText->CreateSub ( this, "RAN_INFO_CHARACTER_WINDOW_STAT_4_GUILD_VALUE" );
	m_pGuildText->SetFont ( pFont8 );
	m_pGuildText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pGuildText->SetText( "026", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pGuildText );


	std::string strSchool[INFO_ICON_SCHOOL] = 
	{
		"RAN_INFO_CHARACTER_SCHOOL_WINDOW_SM",
		"RAN_INFO_CHARACTER_SCHOOL_WINDOW_HA",
		"RAN_INFO_CHARACTER_SCHOOL_WINDOW_BH",
	};

	std::string strClass[INFO_ICON_CLASS] = 
	{
		"RAN_INFO_CHARACTER_CLASS_BRAWLER_MALE",
		"RAN_INFO_CHARACTER_CLASS_SWORDMAN_MALE",
		"RAN_INFO_CHARACTER_CLASS_ARCHER_FEMALE",
		"RAN_INFO_CHARACTER_CLASS_SHAMAN_FEMALE",
		"RAN_INFO_CHARACTER_CLASS_EXTREME_MALE",
		"RAN_INFO_CHARACTER_CLASS_EXTREME_FEMALE",
		"RAN_INFO_CHARACTER_CLASS_BRAWLER_FEMALE",
		"RAN_INFO_CHARACTER_CLASS_SWORDMAN_FEMALE",
		"RAN_INFO_CHARACTER_CLASS_ARCHER_MALE",
		"RAN_INFO_CHARACTER_CLASS_SHAMAN_MALE",
		"RAN_INFO_CHARACTER_CLASS_SCIENTIST_MALE",
		"RAN_INFO_CHARACTER_CLASS_SCIENTIST_FEMALE",
		"RAN_INFO_CHARACTER_CLASS_ASSASSIN_MALE",
		"RAN_INFO_CHARACTER_CLASS_ASSASSIN_FEMALE",
		"RAN_INFO_CHARACTER_CLASS_TRICKER_MALE",
		"RAN_INFO_CHARACTER_CLASS_TRICKER_FEMALE",
	};

	for( int i=0; i<INFO_ICON_SCHOOL; ++i )
	{
		m_pSchoolIcon[i] = new CUIControl;
		m_pSchoolIcon[i]->CreateSub ( this, strSchool[i].c_str() );	
		m_pSchoolIcon[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pSchoolIcon[i] );
	}

	for( int i=0; i<INFO_ICON_CLASS; ++i )
	{
		m_pClassIcon[i] = new CUIControl;
		m_pClassIcon[i]->CreateSub ( this, strClass[i].c_str() );	
		m_pClassIcon[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pClassIcon[i] );
	}

	m_pButtonWhisper = new CBasicTextButton;
	m_pButtonWhisper->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, CHARACTER_VIEW_INFO_BUTTON_WHISPER );
	m_pButtonWhisper->CreateBaseButton ( "RAN_ANOTHER_CHARACTER_BUTTON_WHISPER", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 27) );
	RegisterControl ( m_pButtonWhisper );

	m_pButtonAddFriend = new CBasicTextButton;
	m_pButtonAddFriend->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, CHARACTER_VIEW_INFO_BUTTON_ADDFRIEND );
	m_pButtonAddFriend->CreateBaseButton ( "RAN_ANOTHER_CHARACTER_BUTTON_ADDFRIEND", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 28) );
	RegisterControl ( m_pButtonAddFriend );

	//m_pButtonInviteParty = new CBasicTextButton;
	//m_pButtonInviteParty->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, CHARACTER_VIEW_INFO_BUTTON_INVITEPARTY );
	//m_pButtonInviteParty->CreateBaseButton ( "RAN_ANOTHER_CHARACTER_BUTTON_INVITEPARTY", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 29) );
	//RegisterControl ( m_pButtonInviteParty );

	m_pButtonRefresh = new CBasicTextButton;
	m_pButtonRefresh->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, CHARACTER_VIEW_INFO_BUTTON_REFRESH );
	m_pButtonRefresh->CreateBaseButton ( "RAN_ANOTHER_CHARACTER_BUTTON_REFRESH", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_STAT_STATIC", 30) );
	RegisterControl ( m_pButtonRefresh );

	CString strInvenWearItem[INFO_ITEM_IMAGE_SIZE] = 
	{
		"RAN_ANOTHER_CHARACTER_ITEMSLOT_HAT",
		"RAN_ANOTHER_CHARACTER_ITEMSLOT_NEACKLACE",
		"RAN_ANOTHER_CHARACTER_ITEMSLOT_TOP",
		"RAN_ANOTHER_CHARACTER_ITEMSLOT_BELT",
		"RAN_ANOTHER_CHARACTER_ITEMSLOT_PANTS",
		"RAN_ANOTHER_CHARACTER_ITEMSLOT_GLOVES",
		"RAN_ANOTHER_CHARACTER_ITEMSLOT_BRACELET",
		"RAN_ANOTHER_CHARACTER_ITEMSLOT_SHOES",
		"RAN_ANOTHER_CHARACTER_ITEMSLOT_WEAPON1",
		"RAN_ANOTHER_CHARACTER_ITEMSLOT_WEAPON2",
		"RAN_ANOTHER_CHARACTER_ITEMSLOT_RING1",
		"RAN_ANOTHER_CHARACTER_ITEMSLOT_RING2",
		"RAN_ANOTHER_CHARACTER_ITEMSLOT_ACCESSORY1",
		"RAN_ANOTHER_CHARACTER_ITEMSLOT_ACCESSORY2",
		"RAN_ANOTHER_CHARACTER_ITEMSLOT_EARRINGS1",
		"RAN_ANOTHER_CHARACTER_ITEMSLOT_EARRINGS2",
	};

	for ( int i=0; i<INFO_ITEM_IMAGE_SIZE; i++ )
	{
		m_pItemImage[i] = new CItemImage;
		m_pItemImage[i]->CreateSub ( this, strInvenWearItem[i].GetString(), UI_FLAG_DEFAULT, CHARACTER_VIEW_INFO_ITEM_IMAGE0 + i );
		m_pItemImage[i]->CreateSubControl ();
		m_pItemImage[i]->CreateTextBoxDownRight(); 
		RegisterControl ( m_pItemImage[i] );
	}

	m_pButtonLeft = new CBasicButton;
	m_pButtonLeft->CreateSub ( this, "RAN_ANOTHER_CHARACTER_WEAR_ARROW_TURN_LEFT_DEFAULT", UI_FLAG_DEFAULT, CHARACTER_VIEW_INFO_BUTTON_LEFT );
	m_pButtonLeft->CreateFlip ( "RAN_ANOTHER_CHARACTER_WEAR_ARROW_TURN_LEFT_CLICK", CBasicButton::CLICK_FLIP );
	m_pButtonLeft->CreateMouseOver ( "RAN_ANOTHER_CHARACTER_WEAR_ARROW_TURN_LEFT_OVER" );
	m_pButtonLeft->SetVisibleSingle( TRUE );
	RegisterControl ( m_pButtonLeft );

	m_pButtonRight = new CBasicButton;
	m_pButtonRight->CreateSub ( this, "RAN_ANOTHER_CHARACTER_WEAR_ARROW_TURN_RIGHT_DEFAULT", UI_FLAG_DEFAULT, CHARACTER_VIEW_INFO_BUTTON_RIGHT );
	m_pButtonRight->CreateFlip ( "RAN_ANOTHER_CHARACTER_WEAR_ARROW_TURN_RIGHT_CLICK", CBasicButton::CLICK_FLIP );
	m_pButtonRight->CreateMouseOver ( "RAN_ANOTHER_CHARACTER_WEAR_ARROW_TURN_RIGHT_OVER" );
	m_pButtonRight->SetVisibleSingle( TRUE );
	RegisterControl ( m_pButtonRight );

	m_pRender = new CCharacterInfoViewPageStatsRender;
	m_pRender->CreateSub ( this, "RAN_ANOTHER_CHARACTER_WEAR_RENDER_REGION" );
	m_pRender->CreateSubControl ();
	RegisterControl ( m_pRender );

}

void CCharacterInfoViewPageStats::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case CHARACTER_VIEW_INFO_ITEM_IMAGE0:
	case CHARACTER_VIEW_INFO_ITEM_IMAGE1:
	case CHARACTER_VIEW_INFO_ITEM_IMAGE2:
	case CHARACTER_VIEW_INFO_ITEM_IMAGE3:
	case CHARACTER_VIEW_INFO_ITEM_IMAGE4:
	case CHARACTER_VIEW_INFO_ITEM_IMAGE5:
	case CHARACTER_VIEW_INFO_ITEM_IMAGE6:
	case CHARACTER_VIEW_INFO_ITEM_IMAGE7:
	case CHARACTER_VIEW_INFO_ITEM_IMAGE8:
	case CHARACTER_VIEW_INFO_ITEM_IMAGE9:
	case CHARACTER_VIEW_INFO_ITEM_IMAGE10:
	case CHARACTER_VIEW_INFO_ITEM_IMAGE11:
	case CHARACTER_VIEW_INFO_ITEM_IMAGE12:
	case CHARACTER_VIEW_INFO_ITEM_IMAGE13:
	case CHARACTER_VIEW_INFO_ITEM_IMAGE14:
	case CHARACTER_VIEW_INFO_ITEM_IMAGE15:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				EMSLOT emSlot = IMAGE2EMSLOT ( ControlID );

				SITEMCUSTOM sItemCustom = m_sInfoChar.sPutOnItems[emSlot];
				if ( sItemCustom.sNativeID != NATIVEID_NULL () )
				{
					CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW ( sItemCustom, FALSE, FALSE, FALSE, FALSE, FALSE, USHRT_MAX, USHRT_MAX );

					/*if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LMENU  ) & DXKEY_DOWNED )
					{
						if ( dwMsg & UIMSG_LB_UP )
						{
							CInnerInterface::GetInstance().PreviewItem( sItemCustom );
							return;
						}
					}

					if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LCONTROL ) & DXKEY_DOWNED )
					{
						if ( dwMsg & UIMSG_RB_UP )
						{
							CInnerInterface::GetInstance().LinkItem( &sItemCustom );
							return;
						}
					}*/
				}
			}
		}break;

	case CHARACTER_VIEW_INFO_BUTTON_WHISPER:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( dwMsg & UIMSG_LB_UP )
				{	
					if ( m_sInfoChar.dwCharID != GAEAID_NULL )
					{
						CInnerInterface::GetInstance().GetChat ()->BEGIN_PRIVATE_CHAT ( m_sInfoChar.szName );
						CInnerInterface::GetInstance().ADD_FRIEND_NAME_TO_EDITBOX ( m_sInfoChar.szName );
					}
				}
			}
		}break;

	case CHARACTER_VIEW_INFO_BUTTON_ADDFRIEND:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( dwMsg & UIMSG_LB_UP )
				{	
					if ( m_sInfoChar.dwCharID != GAEAID_NULL )
					{
						CInnerInterface::GetInstance().ADD_FRIEND ( m_sInfoChar.szName );
					}
				}
			}
		}break;

	case CHARACTER_VIEW_INFO_BUTTON_INVITEPARTY:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( dwMsg & UIMSG_LB_UP )
				{	
				}
			}
		}break;

	case CHARACTER_VIEW_INFO_BUTTON_REFRESH:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( dwMsg & UIMSG_LB_UP )
				{	
					if ( m_sInfoChar.dwCharID != GAEAID_NULL && m_sInfoChar.dwGaeaID != GAEAID_NULL )
					{
						GLGaeaClient::GetInstance().GetCharacter()->RequestCharacterInfo( m_sInfoChar.dwGaeaID );
					}
				}
			}
		}break;

	case CHARACTER_VIEW_INFO_BUTTON_LEFT:
		{
			if ( CHECK_MOUSE_IN_LBDOWNLIKE ( dwMsg ) )
			{
				if ( m_pRender )	m_pRender->m_vRot.x -=0.1f;
			}
		}break;
	case CHARACTER_VIEW_INFO_BUTTON_RIGHT:
		{
			if ( CHECK_MOUSE_IN_LBDOWNLIKE ( dwMsg ) )
			{
				if ( m_pRender )	m_pRender->m_vRot.x +=0.1f;

			}
		}break;
	}
}

void CCharacterInfoViewPageStats::SetVisibleSingle ( BOOL bVisible )
{
	CUIGroup::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		DataReset();
	}
}

void CCharacterInfoViewPageStats::SetCharacterData( SINFO_CHAR &sInfoChar )
{
	DataReset();

	m_sInfoChar = sInfoChar;

	if ( m_sInfoChar.dwCharID == GAEAID_NULL )	return;

	WORD wClassIndex = CharClassToIndex( m_sInfoChar.emClass );
	WORD wSchoolIndex = m_sInfoChar.wSchool;

	CString strTemp;

	if ( m_pBadgeText )
	{
		if ( strlen(m_sInfoChar.szBadge) )
		{
			m_pBadgeText->SetText( m_sInfoChar.szBadge, NS_UITEXTCOLOR::DODGERBLUE ); 
		}else{
			m_pBadgeText->SetText( ID2GAMEWORD("CHARWINDOW_TITLE", 0 ) );
		}
	}

	if ( m_pNameText )
	{
		m_pNameText->SetText( m_sInfoChar.szName, NS_UITEXTCOLOR::ORANGE );
	}
	
	if ( m_pClassText )		m_pClassText->SetText( COMMENT::CHARCLASS[wClassIndex].c_str() );
	if ( m_pSchoolText )	m_pSchoolText->SetText( GLCONST_CHAR::strSCHOOLNAME[wSchoolIndex].c_str() );
	if ( m_pGuildText )		m_pGuildText->SetText( m_sInfoChar.szClubName );

	if ( m_pClassIcon[wClassIndex] )
		m_pClassIcon[wClassIndex]->SetVisibleSingle( TRUE );

	if ( m_pSchoolIcon[wSchoolIndex] )
		m_pSchoolIcon[wSchoolIndex]->SetVisibleSingle( TRUE );

	strTemp.Format( "%d", m_sInfoChar.wLevel );
	if ( m_pLevelText )	m_pLevelText->SetText( strTemp.GetString() );

	if ( m_sInfoChar.bPrivate )
	{
		if ( m_pPowText )	m_pPowText->SetText( "---" );
		if ( m_pDexText )	m_pDexText->SetText( "---" );
		if ( m_pSpiText )	m_pSpiText->SetText( "---" );
		if ( m_pStrText )	m_pStrText->SetText( "---" );
		if ( m_pStaText )	m_pStaText->SetText( "---" );
	}
	else
	{
		strTemp.Format( "%d", m_sInfoChar.sSUMSTATS.wPow );
		if ( m_pPowText )	m_pPowText->SetText( strTemp.GetString());

		strTemp.Format( "%d", m_sInfoChar.sSUMSTATS.wDex );
		if ( m_pDexText )	m_pDexText->SetText( strTemp.GetString());

		strTemp.Format( "%d", m_sInfoChar.sSUMSTATS.wSpi );
		if ( m_pSpiText )	m_pSpiText->SetText( strTemp.GetString());

		strTemp.Format( "%d", m_sInfoChar.sSUMSTATS.wStr );
		if ( m_pStrText )	m_pStrText->SetText( strTemp.GetString());

		strTemp.Format( "%d", m_sInfoChar.sSUMSTATS.wSta );
		if ( m_pStaText )	m_pStaText->SetText( strTemp.GetString());
	}


	strTemp.Format( "%d", m_sInfoChar.sdDAMAGE.dwLow );
	if ( m_pAttackMinText )	m_pAttackMinText->SetText( strTemp.GetString());

	strTemp.Format( "%d", m_sInfoChar.sdDAMAGE.dwHigh );
	if ( m_pAttackMaxText )	m_pAttackMaxText->SetText( strTemp.GetString());

	strTemp.Format( "%d", m_sInfoChar.nSUM_DEF );
	if ( m_pDefenseText )	m_pDefenseText->SetText( strTemp.GetString());

	strTemp.Format( "%d", m_sInfoChar.wSUM_PA );
	if ( m_pFightText )		m_pFightText->SetText( strTemp.GetString());

	strTemp.Format( "%d", m_sInfoChar.wSUM_SA );
	if ( m_pShootText )		m_pShootText->SetText( strTemp.GetString());

	strTemp.Format( "%d", m_sInfoChar.wSUM_MA );
	if ( m_pEnergyText )	m_pEnergyText->SetText( strTemp.GetString());

	strTemp.Format( "%d", m_sInfoChar.nSUM_HIT );
	if ( m_pHitText )		m_pHitText->SetText( strTemp.GetString());

	strTemp.Format( "%d", m_sInfoChar.nSUM_AVOID );
	if ( m_pAvoidText )		m_pAvoidText->SetText( strTemp.GetString());

	strTemp.Format( "%d", m_sInfoChar.wHP );
	if ( m_pHPText )	m_pHPText->SetText( strTemp.GetString());

	strTemp.Format( "%d", m_sInfoChar.wMP );
	if ( m_pMPText )	m_pMPText->SetText( strTemp.GetString());

	strTemp.Format( "%d", m_sInfoChar.wSP );
	if ( m_pSPText )	m_pSPText->SetText( strTemp.GetString());

	strTemp.Format( "%d", m_sInfoChar.sSUMRESIST.nFire);
	if ( m_pFireText )	m_pFireText->SetText( strTemp.GetString());

	strTemp.Format( "%d", m_sInfoChar.sSUMRESIST.nIce);
	if ( m_pColdText )	m_pColdText->SetText( strTemp.GetString());

	strTemp.Format( "%d", m_sInfoChar.sSUMRESIST.nElectric);
	if ( m_pElectricText )	m_pElectricText->SetText( strTemp.GetString());

	strTemp.Format( "%d", m_sInfoChar.sSUMRESIST.nPoison);
	if ( m_pPoisonText )	m_pPoisonText->SetText( strTemp.GetString());

	strTemp.Format( "%d", m_sInfoChar.sSUMRESIST.nSpirit);
	if ( m_pSpiritText )	m_pSpiritText->SetText( strTemp.GetString());

	for ( int i = 0; i < INFO_ITEM_IMAGE_SIZE; i++ )
	{
		if ( m_pItemImage[i] )
		{
			EMSLOT emSlot = IMAGE2EMSLOT ( i + CHARACTER_VIEW_INFO_ITEM_IMAGE0 );

			SITEMCUSTOM sItemData = m_sInfoChar.sPutOnItems[emSlot];

			if ( sItemData.sNativeID != NATIVEID_NULL () )
			{
				SITEM* pItem = GLItemMan::GetInstance().GetItem ( sItemData.sNativeID );
				if ( pItem )
				{
					m_pItemImage[i]->SetItem ( sItemData );

					if ( sItemData.GETGRADE( EMGRINDING_NONE ) )
					{
						CString strGrind;
						strGrind.Format( "+%d", sItemData.GETGRADE( EMGRINDING_NONE ) );
						m_pItemImage[i]->SetTextDownRight( strGrind.GetString(), NS_UITEXTCOLOR::GOLD );
					}
				}
			}
		}
	}

	if ( m_pRender )
	{
		m_pRender->m_sInfoChar = m_sInfoChar;
		m_pRender->m_bReady = true;
	}


	GLCHARLOGIC &sCharLogic = GLGaeaClient::GetInstance().GetCharacter()->GetCharLogic();

	//calculation
	DoStatComparison( (int)m_sInfoChar.wLevel, (int)sCharLogic.m_wLevel, m_pLevelText_Up, m_pLevelText_Mark );

	if ( m_sInfoChar.bPrivate )
	{
		if ( m_pPowText_Up )	m_pPowText_Up->SetText( "---" );
		if ( m_pDexText_Up )	m_pDexText_Up->SetText( "---" );
		if ( m_pSpiText_Up )	m_pSpiText_Up->SetText( "---" );
		if ( m_pStrText_Up )	m_pStrText_Up->SetText( "---" );
		if ( m_pStaText_Up )	m_pStaText_Up->SetText( "---" );

		if ( m_pPowText_Mark )	m_pPowText_Mark->Reset();
		if ( m_pDexText_Mark )	m_pDexText_Mark->Reset();
		if ( m_pSpiText_Mark )	m_pSpiText_Mark->Reset();
		if ( m_pStrText_Mark )	m_pStrText_Mark->Reset();
		if ( m_pStaText_Mark )	m_pStaText_Mark->Reset();
	}
	else
	{
		DoStatComparison( (int)m_sInfoChar.sSUMSTATS.wPow, (int)sCharLogic.m_sSUMSTATS.wPow, m_pPowText_Up, m_pPowText_Mark );
		DoStatComparison( (int)m_sInfoChar.sSUMSTATS.wDex, (int)sCharLogic.m_sSUMSTATS.wDex, m_pDexText_Up, m_pDexText_Mark );
		DoStatComparison( (int)m_sInfoChar.sSUMSTATS.wSpi, (int)sCharLogic.m_sSUMSTATS.wSpi, m_pSpiText_Up, m_pSpiText_Mark );
		DoStatComparison( (int)m_sInfoChar.sSUMSTATS.wStr, (int)sCharLogic.m_sSUMSTATS.wStr, m_pStrText_Up, m_pStrText_Mark );
		DoStatComparison( (int)m_sInfoChar.sSUMSTATS.wSta, (int)sCharLogic.m_sSUMSTATS.wSta, m_pStaText_Up, m_pStaText_Mark );
	}


	DoStatComparison( (int)m_sInfoChar.sdDAMAGE.dwLow, (int)sCharLogic.GETFORCE_LOW(), m_pAttackMinText_Up, m_pAttackMinText_Mark );
	DoStatComparison( (int)m_sInfoChar.sdDAMAGE.dwHigh, (int)sCharLogic.GETFORCE_HIGH(), m_pAttackMaxText_Up, m_pAttackMaxText_Mark );

	DoStatComparison( (int)m_sInfoChar.nSUM_DEF, (int)sCharLogic.GETDEFENSE(), m_pDefenseText_Up, m_pDefenseText_Mark );

	DoStatComparison( (int)m_sInfoChar.wSUM_PA, (int)sCharLogic.m_wSUM_PA, m_pFightText_Up, m_pFightText_Mark );
	DoStatComparison( (int)m_sInfoChar.wSUM_SA, (int)sCharLogic.m_wSUM_SA, m_pShootText_Up, m_pShootText_Mark );
	DoStatComparison( (int)m_sInfoChar.wSUM_MA, (int)sCharLogic.m_wSUM_MA, m_pEnergyText_Up, m_pEnergyText_Mark );

	DoStatComparison( (int)m_sInfoChar.nSUM_HIT, (int)sCharLogic.GETHIT(), m_pHitText_Up, m_pHitText_Mark );
	DoStatComparison( (int)m_sInfoChar.nSUM_AVOID, (int)sCharLogic.GETAVOID(), m_pAvoidText_Up, m_pAvoidText_Mark );

	DoStatComparison( (int)m_sInfoChar.wHP, (int)sCharLogic.m_sHP.dwMax, m_pHPText_Up, m_pHPText_Mark );
	DoStatComparison( (int)m_sInfoChar.wMP, (int)sCharLogic.m_sMP.dwMax, m_pMPText_Up, m_pMPText_Mark );
	DoStatComparison( (int)m_sInfoChar.wSP, (int)sCharLogic.m_sSP.dwMax, m_pSPText_Up, m_pSPText_Mark );

	DoStatComparison( (int)m_sInfoChar.sSUMRESIST.nFire, (int)sCharLogic.m_sSUMRESIST_SKILL.nFire, m_pFireText_Up, m_pFireText_Mark );
	DoStatComparison( (int)m_sInfoChar.sSUMRESIST.nIce, (int)sCharLogic.m_sSUMRESIST_SKILL.nIce, m_pColdText_Up, m_pColdText_Mark );
	DoStatComparison( (int)m_sInfoChar.sSUMRESIST.nElectric, (int)sCharLogic.m_sSUMRESIST_SKILL.nElectric, m_pElectricText_Up, m_pElectricText_Mark );
	DoStatComparison( (int)m_sInfoChar.sSUMRESIST.nPoison, (int)sCharLogic.m_sSUMRESIST_SKILL.nPoison, m_pPoisonText_Up, m_pPoisonText_Mark );
	DoStatComparison( (int)m_sInfoChar.sSUMRESIST.nSpirit, (int)sCharLogic.m_sSUMRESIST_SKILL.nSpirit, m_pSpiritText_Up, m_pSpiritText_Mark );
}


void CCharacterInfoViewPageStats::DataReset()
{
	m_sInfoChar = SINFO_CHAR();

	if ( m_pLevelText )	m_pLevelText->ClearText();

	if ( m_pPowText )	m_pPowText->ClearText();
	if ( m_pDexText )	m_pDexText->ClearText();
	if ( m_pSpiText )	m_pSpiText->ClearText();
	if ( m_pStrText )	m_pStrText->ClearText();
	if ( m_pStaText )	m_pStaText->ClearText();

	if ( m_pAttackMinText )	m_pAttackMinText->ClearText();
	if ( m_pAttackMaxText )	m_pAttackMaxText->ClearText();
	if ( m_pDefenseText )	m_pDefenseText->ClearText();
	if ( m_pFightText )		m_pFightText->ClearText();
	if ( m_pShootText )		m_pShootText->ClearText();
	if ( m_pEnergyText )	m_pEnergyText->ClearText();
	if ( m_pHitText )		m_pHitText->ClearText();
	if ( m_pAvoidText )		m_pAvoidText->ClearText();

	if ( m_pHPText )	m_pHPText->ClearText();
	if ( m_pMPText )	m_pMPText->ClearText();
	if ( m_pSPText )	m_pSPText->ClearText();

	if ( m_pFireText )		m_pFireText->ClearText();
	if ( m_pColdText )		m_pColdText->ClearText();
	if ( m_pElectricText )	m_pElectricText->ClearText();
	if ( m_pPoisonText )	m_pPoisonText->ClearText();
	if ( m_pSpiritText )	m_pSpiritText->ClearText();

	if ( m_pLevelText_Up )	m_pLevelText_Up->ClearText();

	if ( m_pPowText_Up )	m_pPowText_Up->ClearText();
	if ( m_pDexText_Up )	m_pDexText_Up->ClearText();
	if ( m_pSpiText_Up )	m_pSpiText_Up->ClearText();
	if ( m_pStrText_Up )	m_pStrText_Up->ClearText();
	if ( m_pStaText_Up )	m_pStaText_Up->ClearText();

	if ( m_pAttackMinText_Up )	m_pAttackMinText_Up->ClearText();
	if ( m_pAttackMaxText_Up )	m_pAttackMaxText_Up->ClearText();
	if ( m_pDefenseText_Up )	m_pDefenseText_Up->ClearText();
	if ( m_pFightText_Up )		m_pFightText_Up->ClearText();
	if ( m_pShootText_Up )		m_pShootText_Up->ClearText();
	if ( m_pEnergyText_Up )		m_pEnergyText_Up->ClearText();
	if ( m_pHitText_Up )		m_pHitText_Up->ClearText();
	if ( m_pAvoidText_Up )		m_pAvoidText_Up->ClearText();

	if ( m_pHPText_Up )	m_pHPText_Up->ClearText();
	if ( m_pMPText_Up )	m_pMPText_Up->ClearText();
	if ( m_pSPText_Up )	m_pSPText_Up->ClearText();

	if ( m_pFireText_Up )		m_pFireText_Up->ClearText();
	if ( m_pColdText_Up )		m_pColdText_Up->ClearText();
	if ( m_pElectricText_Up )	m_pElectricText_Up->ClearText();
	if ( m_pPoisonText_Up )		m_pPoisonText_Up->ClearText();
	if ( m_pSpiritText_Up )		m_pSpiritText_Up->ClearText();

	if ( m_pLevelText_Mark )	m_pLevelText_Mark->Reset();
	if ( m_pPowText_Mark )	m_pPowText_Mark->Reset();
	if ( m_pDexText_Mark )	m_pDexText_Mark->Reset();
	if ( m_pSpiText_Mark )	m_pSpiText_Mark->Reset();
	if ( m_pStrText_Mark )	m_pStrText_Mark->Reset();
	if ( m_pStaText_Mark )	m_pStaText_Mark->Reset();

	if ( m_pAttackMinText_Mark )	m_pAttackMinText_Mark->Reset();
	if ( m_pAttackMaxText_Mark )	m_pAttackMaxText_Mark->Reset();
	if ( m_pDefenseText_Mark )	m_pDefenseText_Mark->Reset();
	if ( m_pFightText_Mark )	m_pFightText_Mark->Reset();
	if ( m_pShootText_Mark )	m_pShootText_Mark->Reset();
	if ( m_pEnergyText_Mark )	m_pEnergyText_Mark->Reset();
	if ( m_pHitText_Mark )		m_pHitText_Mark->Reset();
	if ( m_pAvoidText_Mark )	m_pAvoidText_Mark->Reset();
	if ( m_pHPText_Mark )		m_pHPText_Mark->Reset();
	if ( m_pMPText_Mark )		m_pMPText_Mark->Reset();
	if ( m_pSPText_Mark )		m_pSPText_Mark->Reset();

	if ( m_pFireText_Mark )		m_pFireText_Mark->Reset();
	if ( m_pColdText_Mark )		m_pColdText_Mark->Reset();
	if ( m_pElectricText_Mark )	m_pElectricText_Mark->Reset();
	if ( m_pPoisonText_Mark )	m_pPoisonText_Mark->Reset();
	if ( m_pSpiritText_Mark )	m_pSpiritText_Mark->Reset();

	if ( m_pBadgeText )		m_pBadgeText->ClearText();
	if ( m_pNameText )		m_pNameText->ClearText();
	if ( m_pClassText )		m_pClassText->ClearText();
	if ( m_pSchoolText )	m_pSchoolText->ClearText();
	if ( m_pGuildText )		m_pGuildText->ClearText();

	for ( int i=0; i<INFO_ITEM_IMAGE_SIZE; ++i )
	{
		if ( m_pItemImage[i] )
		{
			m_pItemImage[i]->ResetItem();
			m_pItemImage[i]->SetTextDownRight( "", NS_UITEXTCOLOR::WHITE );
		}
	}

	for ( int i=0; i<INFO_ICON_CLASS; ++i )
	{
		if ( m_pClassIcon[i] )
			m_pClassIcon[i]->SetVisibleSingle( FALSE );
	}

	for ( int i=0; i<INFO_ICON_SCHOOL; ++i )
	{
		if ( m_pSchoolIcon[i] )
			m_pSchoolIcon[i]->SetVisibleSingle( FALSE );
	}

	if( m_pRender )
		m_pRender->ClearRender();
}

EMSLOT CCharacterInfoViewPageStats::IMAGE2EMSLOT ( int nIndex )
{
	switch ( nIndex )
	{	
	case CHARACTER_VIEW_INFO_ITEM_IMAGE0:	return SLOT_HEADGEAR;
	case CHARACTER_VIEW_INFO_ITEM_IMAGE1:	return SLOT_NECK;
	case CHARACTER_VIEW_INFO_ITEM_IMAGE2:	return SLOT_UPPER;
	case CHARACTER_VIEW_INFO_ITEM_IMAGE3:	return SLOT_WAIST;
	case CHARACTER_VIEW_INFO_ITEM_IMAGE4:	return SLOT_LOWER;
	case CHARACTER_VIEW_INFO_ITEM_IMAGE5:	return SLOT_HAND;
	case CHARACTER_VIEW_INFO_ITEM_IMAGE6:	return SLOT_WRIST;
	case CHARACTER_VIEW_INFO_ITEM_IMAGE7:	return SLOT_FOOT;
	case CHARACTER_VIEW_INFO_ITEM_IMAGE8:	return SLOT_RHAND;
	case CHARACTER_VIEW_INFO_ITEM_IMAGE9:	return SLOT_RHAND_S;
	case CHARACTER_VIEW_INFO_ITEM_IMAGE10:	return SLOT_LFINGER;
	case CHARACTER_VIEW_INFO_ITEM_IMAGE11:	return SLOT_RFINGER;
	case CHARACTER_VIEW_INFO_ITEM_IMAGE12:	return SLOT_RACCESSORY;
	case CHARACTER_VIEW_INFO_ITEM_IMAGE13:	return SLOT_LACCESSORY;
	case CHARACTER_VIEW_INFO_ITEM_IMAGE14:	return SLOT_LEARRING;
	case CHARACTER_VIEW_INFO_ITEM_IMAGE15:	return SLOT_ORNAMENT;
	};

	return SLOT_TSIZE;
}

void CCharacterInfoViewPageStats::DoStatComparison( int nVAL1, int nVAL2, CBasicTextBox* pTextBox,  CCharacterInfoViewPageStatsMark* pMark )
{
	if ( pTextBox )
	{
		CString strTemp;
		DWORD dwCOLOR = NS_UITEXTUTIL::ENABLE;

		if ( nVAL1 > nVAL2 )
		{
			int nValue = int( nVAL1 - nVAL2 );
			strTemp.Format( "-%d", nValue );
			dwCOLOR = NS_UITEXTCOLOR::RED;
		}
		else if ( nVAL1 < nVAL2 )
		{
			int nValue = int( nVAL2 - nVAL1 );
			strTemp.Format( "+%d", nValue );
			dwCOLOR = NS_UITEXTCOLOR::GREENYELLOW;
		}
		else
		{
			strTemp.Format( "%d", nVAL1 );
		}

		pTextBox->AddText( strTemp.GetString(), dwCOLOR );
	}

	if ( pMark )	pMark->CheckValue( nVAL1, nVAL2 );
}