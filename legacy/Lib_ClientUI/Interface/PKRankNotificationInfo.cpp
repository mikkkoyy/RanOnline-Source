#include "StdAfx.h"
#include "PKRankNotificationInfo.h"
#include "d3dfont.h"
#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "UITextControl.h"
#include "GameTextControl.h"
#include "BasicLineBoxEx.h"
#include "BasicTextBoxEx.h"
#include "GLGaeaClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPKRankNotificationInfo::CPKRankNotificationInfo()
	: m_pNameKiller( NULL )
	, m_pNameKilled( NULL )
	, m_pLineBox( NULL )
	, m_pKillIcon( NULL )
{
	for( int i = 0; i < RANK_INFO_ICON_CLASS; ++i )
	{
		m_pClassIconKilled[i] = NULL;
		m_pClassIconKiller[i] = NULL;
	}

	for( int i = 0; i < RANK_INFO_ICON_SCHOOL; ++i )
	{
		m_pSchoolIconKiller[i] = NULL;
		m_pSchoolIconKilled[i] = NULL;
	}
}

CPKRankNotificationInfo::~CPKRankNotificationInfo()
{
}

void CPKRankNotificationInfo::CreateSubControl()
{
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont( _DEFAULT_FONT, 8, _DEFAULT_FONT_SHADOW_FLAG );

	m_pLineBox = new CBasicLineBox;
	m_pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pLineBox->CreateBaseBoxQuestList ( "PK_RANK_NOTIFICATION_LINEBOX" );
	RegisterControl ( m_pLineBox );

	std::string strSchoolKiller[RANK_INFO_ICON_SCHOOL] = 
	{
		"RANK_INFO_CHARACTER_SCHOOL_1_SM",
		"RANK_INFO_CHARACTER_SCHOOL_1_HA",
		"RANK_INFO_CHARACTER_SCHOOL_1_BH",
	};

	std::string strSchoolKilled[RANK_INFO_ICON_SCHOOL] = 
	{
		"RANK_INFO_CHARACTER_SCHOOL_2_SM",
		"RANK_INFO_CHARACTER_SCHOOL_2_HA",
		"RANK_INFO_CHARACTER_SCHOOL_2_BH",
	};

	std::string strClassKiller[RANK_INFO_ICON_CLASS] = 
	{
		"RANK_INFO_CHARACTER_CLASS_1_BRAWLER_MALE",
		"RANK_INFO_CHARACTER_CLASS_1_SWORDMAN_MALE",
		"RANK_INFO_CHARACTER_CLASS_1_ARCHER_FEMALE",
		"RANK_INFO_CHARACTER_CLASS_1_SHAMAN_FEMALE",
		"RANK_INFO_CHARACTER_CLASS_1_EXTREME_MALE",
		"RANK_INFO_CHARACTER_CLASS_1_EXTREME_FEMALE",
		"RANK_INFO_CHARACTER_CLASS_1_BRAWLER_FEMALE",
		"RANK_INFO_CHARACTER_CLASS_1_SWORDMAN_FEMALE",
		"RANK_INFO_CHARACTER_CLASS_1_ARCHER_MALE",
		"RANK_INFO_CHARACTER_CLASS_1_SHAMAN_MALE",
		"RANK_INFO_CHARACTER_CLASS_1_GUNNER_MALE",
		"RANK_INFO_CHARACTER_CLASS_1_GUNNER_FEMALE",
		"RANK_INFO_CHARACTER_CLASS_1_ASSASIN_MALE",
		"RANK_INFO_CHARACTER_CLASS_1_ASSASIN_FEMALE",
	};

	std::string strClassKilled[RANK_INFO_ICON_CLASS] = 
	{
		"RANK_INFO_CHARACTER_CLASS_2_BRAWLER_MALE",
		"RANK_INFO_CHARACTER_CLASS_2_SWORDMAN_MALE",
		"RANK_INFO_CHARACTER_CLASS_2_ARCHER_FEMALE",
		"RANK_INFO_CHARACTER_CLASS_2_SHAMAN_FEMALE",
		"RANK_INFO_CHARACTER_CLASS_2_EXTREME_MALE",
		"RANK_INFO_CHARACTER_CLASS_2_EXTREME_FEMALE",
		"RANK_INFO_CHARACTER_CLASS_2_BRAWLER_FEMALE",
		"RANK_INFO_CHARACTER_CLASS_2_SWORDMAN_FEMALE",
		"RANK_INFO_CHARACTER_CLASS_2_ARCHER_MALE",
		"RANK_INFO_CHARACTER_CLASS_2_SHAMAN_MALE",
		"RANK_INFO_CHARACTER_CLASS_2_GUNNER_MALE",
		"RANK_INFO_CHARACTER_CLASS_2_GUNNER_FEMALE",
		"RANK_INFO_CHARACTER_CLASS_2_ASSASIN_MALE",
		"RANK_INFO_CHARACTER_CLASS_2_ASSASIN_FEMALE",
	};

	for( int i = 0; i < RANK_INFO_ICON_SCHOOL; ++i )
	{
		m_pSchoolIconKiller[i] = new CUIControl;
		m_pSchoolIconKiller[i]->CreateSub ( this, strSchoolKiller[i].c_str() );	
		m_pSchoolIconKiller[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pSchoolIconKiller[i] );

		m_pSchoolIconKilled[i] = new CUIControl;
		m_pSchoolIconKilled[i]->CreateSub ( this, strSchoolKilled[i].c_str() );	
		m_pSchoolIconKilled[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pSchoolIconKilled[i] );
	}

	for( int i = 0; i < RANK_INFO_ICON_CLASS; ++i )
	{
		m_pClassIconKiller[i] = new CUIControl;
		m_pClassIconKiller[i]->CreateSub ( this, strClassKiller[i].c_str() );	
		m_pClassIconKiller[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pClassIconKiller[i] );

		m_pClassIconKilled[i] = new CUIControl;
		m_pClassIconKilled[i]->CreateSub ( this, strClassKilled[i].c_str() );	
		m_pClassIconKilled[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pClassIconKilled[i] );
	}

	m_pNameKiller = new CBasicTextBoxEx;
	m_pNameKiller->CreateSub ( this, "RANK_INFO_CHARACTER_NAME_1" );
	m_pNameKiller->SetFont ( pFont );		
	m_pNameKiller->SetTextAlign( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	RegisterControl ( m_pNameKiller );

	m_pNameKilled = new CBasicTextBoxEx;
	m_pNameKilled->CreateSub ( this, "RANK_INFO_CHARACTER_NAME_2" );
	m_pNameKilled->SetFont ( pFont );		
	m_pNameKilled->SetTextAlign( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	RegisterControl ( m_pNameKilled );

	m_pKillIcon = new CUIControl;
	m_pKillIcon->CreateSub ( this, "RANK_INFO_CHARACTER_KILL" );
	RegisterControl ( m_pKillIcon );
}

void CPKRankNotificationInfo::TranslateUIMessage( UIGUID cID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage( cID, dwMsg );
}

void CPKRankNotificationInfo::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CPKRankNotificationInfo::SetVisibleSingle ( BOOL bVisible )
{
	CUIGroup::SetVisibleSingle ( bVisible );

	if ( bVisible )
	{
		for( int i = 0; i < RANK_INFO_ICON_SCHOOL; ++i )
		{
			m_pSchoolIconKiller[i]->SetVisibleSingle ( FALSE );
			m_pSchoolIconKilled[i]->SetVisibleSingle ( FALSE );
		}

		for( int i = 0; i < RANK_INFO_ICON_CLASS; ++i )
		{
			m_pClassIconKiller[i]->SetVisibleSingle ( FALSE );	
			m_pClassIconKilled[i]->SetVisibleSingle ( FALSE );
		}

		m_pNameKiller->ClearText();
		m_pNameKilled->ClearText();
	}
}

void CPKRankNotificationInfo::SetData( SPK_HISTORY sHistory )
{
	if ( sHistory.cClassKiller >= RANK_INFO_ICON_CLASS )	return;
	if ( sHistory.cClassKilled >= RANK_INFO_ICON_CLASS )	return;

	if ( sHistory.cSchoolKiller >= RANK_INFO_ICON_SCHOOL )	return;
	if ( sHistory.cSchoolKilled >= RANK_INFO_ICON_SCHOOL )	return;

	m_pSchoolIconKiller[sHistory.cSchoolKiller]->SetVisibleSingle ( TRUE );
	m_pSchoolIconKilled[sHistory.cSchoolKilled]->SetVisibleSingle ( TRUE );

	m_pClassIconKiller[sHistory.cClassKiller]->SetVisibleSingle ( TRUE );	
	m_pClassIconKilled[sHistory.cClassKilled]->SetVisibleSingle ( TRUE );

	m_pNameKiller->SetText( sHistory.szCharKiller );
	m_pNameKilled->SetText( sHistory.szCharKilled );

	DWORD dwCharID = GLGaeaClient::GetInstance().GetCharacter()->m_dwCharID;

	if ( dwCharID == sHistory.dwKilled )
	{
		m_pLineBox->SetUseRender ( TRUE );
		m_pLineBox->SetDiffuse( NS_UITEXTCOLOR::DISABLE );
	}
	else if ( dwCharID == sHistory.dwKiller )
	{
		m_pLineBox->SetUseRender ( TRUE );
		m_pLineBox->SetDiffuse( NS_UITEXTCOLOR::GREENYELLOW );
	}
	else
	{
		m_pLineBox->SetUseRender ( FALSE );
	}
}