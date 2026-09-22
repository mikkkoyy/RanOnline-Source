#include "StdAfx.h"
#include "PVPCaptureTheFlagTopRankings.h"
#include "PVPCaptureTheFlagTopRankingsSlot.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"

#include "GameTextControl.h"
#include "UITextControl.h"

#include "BasicTextBox.h"
#include "BasicLineBox.h"
#include "BasicTextButton.h"

#include "GLPVPCaptureTheFlagClient.h"
#include "InnerInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPCaptureTheFlagTopRankings::CPVPCaptureTheFlagTopRankings ()
	: m_pButtonClose(NULL)
	, m_pRankText(NULL)
	, m_pResultText(NULL)
	, m_pTeamText(NULL)
	, m_pSchoolText(NULL)
	, m_pClassText(NULL)
	, m_pNameText(NULL)
{
	for( int i=0; i<CAPTURE_THE_FLAG_MINI_RANKING_NUM; ++i )
		m_pSlot[i] = NULL;
}

CPVPCaptureTheFlagTopRankings::~CPVPCaptureTheFlagTopRankings ()
{
}

void CPVPCaptureTheFlagTopRankings::CreateSubControl ()
{
	CD3DFontPar * pFont9 = DxFontMan::GetInstance().LoadDxFont( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "CAPTURE_THE_FLAG_TOPRANKING_LINEBOX" );
	RegisterControl ( pLineBox );
	
	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody( "CAPTURE_THE_FLAG_TOPRANKING_LINEBOX_HEAD" );
	RegisterControl ( pLineBox );	

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody( "CAPTURE_THE_FLAG_TOPRANKING_LINEBOX_HEAD" );
	RegisterControl ( pLineBox );	

	m_pRankText = CreateUITextBox( "CAPTURE_THE_FLAG_TOPRANKING_TEXT_BASE_RANK", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pResultText = CreateUITextBox( "CAPTURE_THE_FLAG_TOPRANKING_TEXT_BASE_RESULT", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTeamText = CreateUITextBox( "CAPTURE_THE_FLAG_TOPRANKING_TEXT_BASE_TEAM", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pSchoolText = CreateUITextBox( "CAPTURE_THE_FLAG_TOPRANKING_TEXT_BASE_SCHOOL", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pClassText = CreateUITextBox( "CAPTURE_THE_FLAG_TOPRANKING_TEXT_BASE_CLASS", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pNameText = CreateUITextBox( "CAPTURE_THE_FLAG_TOPRANKING_TEXT_BASE_NAME", pFont9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );

	m_pRankText->AddText( ID2GAMEWORD("CAPTURE_THE_FLAG_TOPRANKING_TEXT", 0 ), NS_UITEXTCOLOR::WHITE );
	m_pResultText->AddText( ID2GAMEWORD("CAPTURE_THE_FLAG_TOPRANKING_TEXT", 1 ), NS_UITEXTCOLOR::WHITE );
	m_pTeamText->AddText( ID2GAMEWORD("CAPTURE_THE_FLAG_TOPRANKING_TEXT", 2 ), NS_UITEXTCOLOR::WHITE );
	m_pSchoolText->AddText( ID2GAMEWORD("CAPTURE_THE_FLAG_TOPRANKING_TEXT", 3 ), NS_UITEXTCOLOR::WHITE );
	m_pClassText->AddText( ID2GAMEWORD("CAPTURE_THE_FLAG_TOPRANKING_TEXT", 4 ), NS_UITEXTCOLOR::WHITE );
	m_pNameText->AddText( ID2GAMEWORD("CAPTURE_THE_FLAG_TOPRANKING_TEXT", 5 ), NS_UITEXTCOLOR::WHITE );

	std::string strSlots[CAPTURE_THE_FLAG_MINI_RANKING_NUM] = 
	{
		"CAPTURE_THE_FLAG_TOPRANKING_SLOT_0",
		"CAPTURE_THE_FLAG_TOPRANKING_SLOT_1",
		"CAPTURE_THE_FLAG_TOPRANKING_SLOT_2",
		"CAPTURE_THE_FLAG_TOPRANKING_SLOT_3",
		"CAPTURE_THE_FLAG_TOPRANKING_SLOT_4",
		"CAPTURE_THE_FLAG_TOPRANKING_SLOT_5",
		"CAPTURE_THE_FLAG_TOPRANKING_SLOT_6",
		"CAPTURE_THE_FLAG_TOPRANKING_SLOT_7",
		"CAPTURE_THE_FLAG_TOPRANKING_SLOT_8",
		"CAPTURE_THE_FLAG_TOPRANKING_SLOT_9",
	};

	for( int i=0; i<CAPTURE_THE_FLAG_MINI_RANKING_NUM; ++i )
	{
		m_pSlot[i] = new CPVPCaptureTheFlagTopRankingsSlot;
		m_pSlot[i]->CreateSub( this, strSlots[i].c_str(), UI_FLAG_DEFAULT, CAPTURE_THE_FLAG_TOPRANK_RANKING_SLOT_0 + i );
		m_pSlot[i]->CreateSubControl ();
		RegisterControl ( m_pSlot[i] );
	}

	m_pButtonClose = new CBasicTextButton;
	m_pButtonClose->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, CAPTURE_THE_FLAG_TOPRANK_BUTTON_CLOSE );
	m_pButtonClose->CreateBaseButton ( "CAPTURE_THE_FLAG_TOPRANKING_BUTTON_CLOSE", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, ID2GAMEWORD("CAPTURE_THE_FLAG_TOPRANKING", 0 ) );
	m_pButtonClose->SetFlip ( TRUE );
	RegisterControl ( m_pButtonClose );

}

CBasicTextBox* CPVPCaptureTheFlagTopRankings::CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign )
{
	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, szControl );
	pTextBox->SetFont ( pFont );
	pTextBox->SetTextAlign ( nTextAlign );
	RegisterControl ( pTextBox );
	return pTextBox;
}

void CPVPCaptureTheFlagTopRankings::SetVisibleSingle ( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle ( bVisible );

	if( bVisible )
	{
		UpdateRankings();
	}
}

void CPVPCaptureTheFlagTopRankings::TranslateUIMessage ( UIGUID cID, DWORD dwMsg )
{
	CUIWindowEx::TranslateUIMessage ( cID, dwMsg );

	switch ( cID )
	{
	case CAPTURE_THE_FLAG_TOPRANK_BUTTON_CLOSE:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) ){
				CInnerInterface::GetInstance().HideGroup( GetWndID() );
			}
		}break;
	};
}

void CPVPCaptureTheFlagTopRankings::UpdateRankings()
{
	for( int i=0; i<CAPTURE_THE_FLAG_MINI_RANKING_NUM; ++i )
	{
		if ( m_pSlot[i] )
			m_pSlot[i]->SetVisibleSingle( FALSE );
	}

	WORD wWinner = GLPVPCaptureTheFlagClient::GetInstance().m_wLastWinner;

	int nSlot = 0;
	for( int i=0; i<CAPTURE_THE_FLAG_MINI_RANKING_NUM; ++i )
	{
		CAPTURE_THE_FLAG_PLAYER_DATA& splayer_data = GLPVPCaptureTheFlagClient::GetInstance().m_sLastTop[i];

		if( splayer_data.dwInfoCharID == CAPTURE_THE_FLAG_PLAYER_NULL )	continue;
		if( splayer_data.wRankAll == 0 )	continue;

		if ( m_pSlot[nSlot] )
		{
			m_pSlot[nSlot]->SetVisibleSingle( TRUE );
			m_pSlot[nSlot]->SetData( &splayer_data, wWinner );
			nSlot ++ ;
		}
	}
}