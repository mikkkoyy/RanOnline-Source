#include "StdAfx.h"
#include "PVPCaptureTheFlagRankings.h"
#include "PVPCaptureTheFlagRankingPage.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"

#include "GameTextControl.h"
#include "UITextControl.h"

#include "MultiModeButton.h"
#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicTextButton.h"

#include "InnerInterface.h"
#include "GLPVPCaptureTheFlagDefine.h"
#include "GLPVPCaptureTheFlagClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPCaptureTheFlagRankings::CPVPCaptureTheFlagRankings ()
	: m_pButtonClose(NULL)
	, m_pInfoExit(NULL)
	, m_nPage(-1)
	, m_fUpdateTime(0.0f)
{
	for( int i=0; i<CAPTURE_THE_FLAG_RANK_SIZE; ++i )
	{
		m_pButtonRank[i] = NULL;
		m_pPageRanking[i] = NULL;
	}
}

CPVPCaptureTheFlagRankings::~CPVPCaptureTheFlagRankings ()
{
}

void CPVPCaptureTheFlagRankings::CreateSubControl ()
{
	CD3DFontPar * pFont = DxFontMan::GetInstance().LoadDxFont( _DEFAULT_FONT, 8, _DEFAULT_FONT_SHADOW_FLAG );

	std::string strbutton[CAPTURE_THE_FLAG_RANK_SIZE] = 
	{
		"CAPTURE_THE_FLAG_RESULT_SORTBUTTON_TOTAL",
		"CAPTURE_THE_FLAG_RESULT_SORTBUTTON_TEAM_A",
		"CAPTURE_THE_FLAG_RESULT_SORTBUTTON_TEAM_B",
	};

	for( int i=0; i<CAPTURE_THE_FLAG_RANK_SIZE; ++i )
	{
		m_pButtonRank[i] = CreateTabButton( strbutton[i].c_str(), ID2GAMEWORD("CAPTURE_THE_FLAG_RESULT_BUTTON", i ), CAPTURE_THE_FLAG_RANK_BUTTON_ALL +i );

		m_pPageRanking[i] = new CPVPCaptureTheFlagRankingPage;
		m_pPageRanking[i]->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_RANKING", UI_FLAG_XSIZE | UI_FLAG_YSIZE, CAPTURE_THE_FLAG_RANK_PAGE_ALL + i );
		m_pPageRanking[i]->CreateSubControl( (i==0)? CAPTURE_THE_FLAG_TEAM_SIZE: i-1 );
		m_pPageRanking[i]->SetVisibleSingle( FALSE );
		RegisterControl ( m_pPageRanking[i] );
	}

	m_pButtonClose = new CBasicTextButton;
	m_pButtonClose->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, CAPTURE_THE_FLAG_RANK_BUTTON_CLOSE );
	m_pButtonClose->CreateBaseButton ( "CAPTURE_THE_FLAG_RESULT_BUTTON_CLOSE", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, ID2GAMEWORD("CAPTURE_THE_FLAG_RESULT_BUTTON", 3 ) );
	m_pButtonClose->SetFlip ( TRUE );
	RegisterControl ( m_pButtonClose );

	m_pInfoExit = new CBasicTextBox;
	m_pInfoExit->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_TEXT_TIME", UI_FLAG_DEFAULT );
	m_pInfoExit->SetFont ( pFont );
	m_pInfoExit->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );	
	RegisterControl ( m_pInfoExit );
}

CMultiModeButton* CPVPCaptureTheFlagRankings::CreateTabButton( const char* szButtonBox, const char* szButtonText, UIGUID ControlID )
{
	CMultiModeButton* pButton = new CMultiModeButton;
	pButton->CreateSub ( this, "TAB_BUTTON_WHITE", UI_FLAG_XSIZE, ControlID );
	pButton->CreateImageBaseMulti( "TAB_BUTTON_WHITE_IMAGE", "TAB_BUTTON_WHITE_IMAGE_LEFT", "TAB_BUTTON_WHITE_IMAGE_CENTER", "TAB_BUTTON_WHITE_IMAGE_RIGHT" );
	pButton->CreateImageFlipMulti( "TAB_BUTTON_WHITE_IMAGE", "TAB_BUTTON_WHITE_IMAGE_LEFT_F", "TAB_BUTTON_WHITE_IMAGE_CENTER_F", "TAB_BUTTON_WHITE_IMAGE_RIGHT_F" );
	pButton->CreateTextBox( "TAB_BUTTON_WHITE_IMAGE_TEXTBOX", 9, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y, _DEFAULT_FONT_FLAG );
	pButton->AlignToControl( szButtonBox );
	pButton->SetOneLineText( szButtonText, NS_UITEXTCOLOR::WHITE );
	pButton->SetTextColor( NS_UITEXTCOLOR::BLACK, NS_UITEXTCOLOR::WHITE, NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pButton );
	return pButton;
}

void CPVPCaptureTheFlagRankings::SetVisibleSingle ( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle ( bVisible );

	if( bVisible )
	{
		SetVisiblePage( CAPTURE_THE_FLAG_RANK_ALL );
	}
	else
	{
	}
}

void CPVPCaptureTheFlagRankings::TranslateUIMessage ( UIGUID cID, DWORD dwMsg )
{
	CUIWindowEx::TranslateUIMessage ( cID, dwMsg );

	switch ( cID )
	{
	case ET_CONTROL_TITLE:
	case ET_CONTROL_TITLE_F:
		{
			if ( (dwMsg & UIMSG_LB_DUP) && CHECK_MOUSE_IN ( dwMsg ) ){
				CInnerInterface::GetInstance().SetDefaultPosInterface( GetWndID() );
			}
		}break;


	case CAPTURE_THE_FLAG_RANK_BUTTON_ALL:
	case CAPTURE_THE_FLAG_RANK_BUTTON_TEAM_A:
	case CAPTURE_THE_FLAG_RANK_BUTTON_TEAM_B:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) ){
				int nPage = cID - CAPTURE_THE_FLAG_RANK_BUTTON_ALL;
				SetVisiblePage( nPage );
			}
		}break;

	case ET_CONTROL_BUTTON:
	case CAPTURE_THE_FLAG_RANK_BUTTON_CLOSE:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) ){
				CInnerInterface::GetInstance().HideGroup( GetWndID() );
				GLPVPCaptureTheFlagClient::GetInstance().RequestMapExit();
			}
		}break;

	};
}

void CPVPCaptureTheFlagRankings::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{		
	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( !IsVisible () ){
		return;
	}

	m_fUpdateTime += fElapsedTime;
	if ( m_fUpdateTime >= 0.5f )
	{
		m_fUpdateTime = 0.0f;

		if ( m_pInfoExit )
		{
			m_pInfoExit->ClearText();

			float fremain_time = GLPVPCaptureTheFlagClient::GetInstance().m_fRemain;
			if ( fremain_time > 0.0f )
			{
				CString strCombine;
				strCombine.Format ( "< %s %02d:%02d >", ID2GAMEWORD( "CAPTURE_THE_FLAG_RESULT_TEXT", 0 ), (DWORD)fremain_time/60, (DWORD)fremain_time%60 );
				m_pInfoExit->AddText( strCombine.GetString(), NS_UITEXTCOLOR::WHITE );
			}
		}	
	}
}

void CPVPCaptureTheFlagRankings::SetVisiblePage( int nPage )
{
	if ( nPage == m_nPage )	return;
	if ( nPage == -1 )	return;

	for( int i=0; i<CAPTURE_THE_FLAG_RANK_SIZE; ++i ){
		if ( m_pButtonRank[i] ){
			m_pButtonRank[i]->DoImageFlip( FALSE );
		}

		if ( m_pPageRanking[i] )
			m_pPageRanking[i]->SetVisibleSingle( FALSE );
	}

	if ( m_pButtonRank[nPage] ){
		m_pButtonRank[nPage]->DoImageFlip( TRUE );
	}

	if ( m_pPageRanking[nPage] )
		m_pPageRanking[nPage]->SetVisibleSingle( TRUE );

	m_nPage = nPage;
}

void CPVPCaptureTheFlagRankings::UpdateRankings()
{
	for( int i=0; i<CAPTURE_THE_FLAG_RANK_SIZE; ++i ){
		if ( m_pPageRanking[i] )
			m_pPageRanking[i]->RefreshRanking();
	}
}