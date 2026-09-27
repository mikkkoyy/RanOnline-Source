#include "StdAfx.h"
#include "PVPSchoolWarsTopRankings.h"
#include "PVPSchoolWarsTopRankingsSlot.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"

#include "GameTextControl.h"
#include "UITextControl.h"

#include "BasicTextBox.h"
#include "BasicLineBox.h"
#include "BasicTextButton.h"

#include "GLPVPSchoolWarsClient.h"
#include "InnerInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPSchoolWarsTopRankings::CPVPSchoolWarsTopRankings ()
	: m_pButtonClose(NULL)
	, m_pRankText(NULL)
	, m_pSchoolText(NULL)
	, m_pClassText(NULL)
	, m_pNameText(NULL)
{
	for( int i=0; i<SCHOOLWARS_MINI_RANKING_NUM; ++i )
		m_pSlot[i] = NULL;
}

CPVPSchoolWarsTopRankings::~CPVPSchoolWarsTopRankings ()
{
}

void CPVPSchoolWarsTopRankings::CreateSubControl ()
{
	CD3DFontPar * pFont9 = DxFontMan::GetInstance().LoadDxFont( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "SCHOOLWARS_TOPRANKING_LINEBOX" );
	RegisterControl ( pLineBox );
	
	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody( "SCHOOLWARS_TOPRANKING_LINEBOX_HEAD" );
	RegisterControl ( pLineBox );	

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody( "SCHOOLWARS_TOPRANKING_LINEBOX_HEAD" );
	RegisterControl ( pLineBox );	

	m_pRankText = CreateUITextBox( "SCHOOLWARS_TOPRANKING_TEXT_BASE_RANK", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pSchoolText = CreateUITextBox( "SCHOOLWARS_TOPRANKING_TEXT_BASE_SCHOOL", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pClassText = CreateUITextBox( "SCHOOLWARS_TOPRANKING_TEXT_BASE_CLASS", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pNameText = CreateUITextBox( "SCHOOLWARS_TOPRANKING_TEXT_BASE_NAME", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );

	m_pRankText->AddText( ID2GAMEWORD("SCHOOLWARS_TOPRANKING_TEXT", 0 ), NS_UITEXTCOLOR::WHITE );
	m_pSchoolText->AddText( ID2GAMEWORD("SCHOOLWARS_TOPRANKING_TEXT", 1 ), NS_UITEXTCOLOR::WHITE );
	m_pClassText->AddText( ID2GAMEWORD("SCHOOLWARS_TOPRANKING_TEXT", 2 ), NS_UITEXTCOLOR::WHITE );
	m_pNameText->AddText( ID2GAMEWORD("SCHOOLWARS_TOPRANKING_TEXT", 3 ), NS_UITEXTCOLOR::WHITE );

	std::string strSlots[SCHOOLWARS_MINI_RANKING_NUM] = 
	{
		"SCHOOLWARS_TOPRANKING_SLOT_0",
		"SCHOOLWARS_TOPRANKING_SLOT_1",
		"SCHOOLWARS_TOPRANKING_SLOT_2",
		"SCHOOLWARS_TOPRANKING_SLOT_3",
		"SCHOOLWARS_TOPRANKING_SLOT_4",
		"SCHOOLWARS_TOPRANKING_SLOT_5",
		"SCHOOLWARS_TOPRANKING_SLOT_6",
		"SCHOOLWARS_TOPRANKING_SLOT_7",
		"SCHOOLWARS_TOPRANKING_SLOT_8",
		"SCHOOLWARS_TOPRANKING_SLOT_9",
	};

	for( int i=0; i<SCHOOLWARS_MINI_RANKING_NUM; ++i )
	{
		m_pSlot[i] = new CPVPSchoolWarsTopRankingsSlot;
		m_pSlot[i]->CreateSub( this, strSlots[i].c_str(), UI_FLAG_DEFAULT, SCHOOLWARS_TOPRANK_RANKING_SLOT_0 + i );
		m_pSlot[i]->CreateSubControl ();
		RegisterControl ( m_pSlot[i] );
	}

	m_pButtonClose = new CBasicTextButton;
	m_pButtonClose->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, SCHOOLWARS_TOPRANK_BUTTON_CLOSE );
	m_pButtonClose->CreateBaseButton ( "SCHOOLWARS_TOPRANKING_BUTTON_CLOSE", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, ID2GAMEWORD("SCHOOLWARS_TOPRANKING", 0 ) );
	m_pButtonClose->SetFlip ( TRUE );
	RegisterControl ( m_pButtonClose );

}

CBasicTextBox* CPVPSchoolWarsTopRankings::CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign )
{
	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, szControl );
	pTextBox->SetFont ( pFont );
	pTextBox->SetTextAlign ( nTextAlign );
	RegisterControl ( pTextBox );
	return pTextBox;
}

void CPVPSchoolWarsTopRankings::SetVisibleSingle ( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle ( bVisible );

	if( bVisible )
	{
		UpdateRankings();
	}
}

void CPVPSchoolWarsTopRankings::TranslateUIMessage ( UIGUID cID, DWORD dwMsg )
{
	CUIWindowEx::TranslateUIMessage ( cID, dwMsg );

	switch ( cID )
	{
	case SCHOOLWARS_TOPRANK_BUTTON_CLOSE:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) ){
				CInnerInterface::GetInstance().HideGroup( GetWndID() );
			}
		}break;
	};
}

void CPVPSchoolWarsTopRankings::UpdateRankings()
{
	for( int i=0; i<SCHOOLWARS_MINI_RANKING_NUM; ++i )
	{
		if ( m_pSlot[i] )
			m_pSlot[i]->SetVisibleSingle( FALSE );
	}

	int nSlot = 0;
	for( int i=0; i<SCHOOLWARS_MINI_RANKING_NUM; ++i )
	{
		SCHOOLWARS_PLAYER_DATA& splayer_data = GLPVPSchoolWarsClient::GetInstance().m_sLastTop[i];
		if( splayer_data.dwInfoCharID == SCHOOLWARS_PLAYER_NULL )	continue;
		if( splayer_data.wRankAll == 0 )	continue;

		if ( m_pSlot[nSlot] )
		{
			m_pSlot[nSlot]->SetVisibleSingle( TRUE );
			m_pSlot[nSlot]->SetData( &splayer_data );
			nSlot ++ ;
		}
	}
}