#include "StdAfx.h"
#include "PVPTyrannyTopRankings.h"
#include "PVPTyrannyTopRankingsSlot.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"

#include "GameTextControl.h"
#include "UITextControl.h"

#include "BasicTextBox.h"
#include "BasicLineBox.h"
#include "BasicTextButton.h"

#include "GLPVPTyrannyClient.h"
#include "InnerInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPTyrannyTopRankings::CPVPTyrannyTopRankings ()
	: m_pButtonClose(NULL)
	, m_pRankText(NULL)
	, m_pSchoolText(NULL)
	, m_pClassText(NULL)
	, m_pNameText(NULL)
{
	for( int i=0; i<TYRANNY_MINI_RANKING_NUM; ++i )
		m_pSlot[i] = NULL;
}

CPVPTyrannyTopRankings::~CPVPTyrannyTopRankings ()
{
}

void CPVPTyrannyTopRankings::CreateSubControl ()
{
	CD3DFontPar * pFont9 = DxFontMan::GetInstance().LoadDxFont( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "COMPETITION_TOPRANKING_LINEBOX" );
	RegisterControl ( pLineBox );
	
	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody( "COMPETITION_TOPRANKING_LINEBOX" );
	RegisterControl ( pLineBox );	

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody( "COMPETITION_TOPRANKING_LINEBOX_HEAD" );
	RegisterControl ( pLineBox );	

	m_pRankText = CreateUITextBox( "COMPETITION_TOPRANKING_TEXT_BASE_RANK", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pSchoolText = CreateUITextBox( "COMPETITION_TOPRANKING_TEXT_BASE_SCHOOL", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pClassText = CreateUITextBox( "COMPETITION_TOPRANKING_TEXT_BASE_CLASS", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pNameText = CreateUITextBox( "COMPETITION_TOPRANKING_TEXT_BASE_NAME", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );

	m_pRankText->AddText( ID2GAMEWORD("COMPETITION_TOPRANKING_TEXT", 0 ), NS_UITEXTCOLOR::WHITE );
	m_pSchoolText->AddText( ID2GAMEWORD("COMPETITION_TOPRANKING_TEXT", 1 ), NS_UITEXTCOLOR::WHITE );
	m_pClassText->AddText( ID2GAMEWORD("COMPETITION_TOPRANKING_TEXT", 2 ), NS_UITEXTCOLOR::WHITE );
	m_pNameText->AddText( ID2GAMEWORD("COMPETITION_TOPRANKING_TEXT", 3 ), NS_UITEXTCOLOR::WHITE );

	std::string strSlots[TYRANNY_MINI_RANKING_NUM] = 
	{
		"COMPETITION_TOPRANKING_SLOT_0",
		"COMPETITION_TOPRANKING_SLOT_1",
		"COMPETITION_TOPRANKING_SLOT_2",
		"COMPETITION_TOPRANKING_SLOT_3",
		"COMPETITION_TOPRANKING_SLOT_4",
		"COMPETITION_TOPRANKING_SLOT_5",
		"COMPETITION_TOPRANKING_SLOT_6",
		"COMPETITION_TOPRANKING_SLOT_7",
		"COMPETITION_TOPRANKING_SLOT_8",
		"COMPETITION_TOPRANKING_SLOT_9",
	};

	for( int i=0; i<TYRANNY_MINI_RANKING_NUM; ++i )
	{
		m_pSlot[i] = new CPVPTyrannyTopRankingsSlot;
		m_pSlot[i]->CreateSub( this, strSlots[i].c_str(), UI_FLAG_DEFAULT, TYRANNY_TOPRANK_RANKING_SLOT_0 + i );
		m_pSlot[i]->CreateSubControl ();
		RegisterControl ( m_pSlot[i] );
	}

	m_pButtonClose = new CBasicTextButton;
	m_pButtonClose->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, TYRANNY_TOPRANK_BUTTON_CLOSE );
	m_pButtonClose->CreateBaseButton ( "COMPETITION_TOPRANKING_BUTTON_CLOSE", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, ID2GAMEWORD("COMPETITION_TOPRANKING", 0 ) );
	m_pButtonClose->SetFlip ( TRUE );
	RegisterControl ( m_pButtonClose );

}

CBasicTextBox* CPVPTyrannyTopRankings::CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign )
{
	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, szControl );
	pTextBox->SetFont ( pFont );
	pTextBox->SetTextAlign ( nTextAlign );
	RegisterControl ( pTextBox );
	return pTextBox;
}

void CPVPTyrannyTopRankings::SetVisibleSingle ( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle ( bVisible );

	if( bVisible )
	{
		UpdateRankings();
	}
}

void CPVPTyrannyTopRankings::TranslateUIMessage ( UIGUID cID, DWORD dwMsg )
{
	CUIWindowEx::TranslateUIMessage ( cID, dwMsg );

	switch ( cID )
	{
	case TYRANNY_TOPRANK_BUTTON_CLOSE:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) ){
				CInnerInterface::GetInstance().HideGroup( GetWndID() );
			}
		}break;
	};
}

void CPVPTyrannyTopRankings::UpdateRankings()
{
	for( int i=0; i<TYRANNY_MINI_RANKING_NUM; ++i )
	{
		if ( m_pSlot[i] )
			m_pSlot[i]->SetVisibleSingle( FALSE );
	}

	int nSlot = 0;
	for( int i=0; i<TYRANNY_MINI_RANKING_NUM; ++i )
	{
		TYRANNY_PLAYER_DATA& splayer_data = GLPVPTyrannyClient::GetInstance().m_sLastTop[i];
		if( splayer_data.dwInfoCharID == TYRANNY_PLAYER_NULL )	continue;
		if( splayer_data.wRankAll == 0 )	continue;

		if ( m_pSlot[nSlot] )
		{
			m_pSlot[nSlot]->SetVisibleSingle( TRUE );
			m_pSlot[nSlot]->SetData( &splayer_data );
			nSlot ++ ;
		}
	}
}