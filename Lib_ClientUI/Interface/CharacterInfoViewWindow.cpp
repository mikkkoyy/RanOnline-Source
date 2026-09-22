#include "StdAfx.h"
#include "CharacterInfoViewWindow.h"

#include "MultiModeButton.h"
#include "BasicLineBox.h"
#include "BasicTextBox.h"

#include "UITextControl.h"
#include "GameTextControl.h"
#include "../Lib_Engine/DxCommon/d3dfont.h"

#include "CharacterInfoViewPageStats.h"

#include "InnerInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CCharacterInfoViewWindow::CCharacterInfoViewWindow(void)
	: m_nPage(-1)
	, m_pButtonStats(NULL)
	, m_pPageStats(NULL)
{
}

CCharacterInfoViewWindow::~CCharacterInfoViewWindow(void)
{
}

CMultiModeButton* CCharacterInfoViewWindow::CreateTabButton( const char* szButtonBox, const char* szButtonText, UIGUID ControlID )
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

void CCharacterInfoViewWindow::CreateSubControl()
{
	m_pButtonStats = CreateTabButton( "RAN_ANOTHER_CHARACTER_WEAR_TAB", ID2GAMEWORD("RAN_ANOTHER_CHAR_WINDOW_PAGE", 1 ), CHARACTER_INFO_VIEW_BUTTON_STATS );

	m_pPageStats = new CCharacterInfoViewPageStats;
	m_pPageStats->CreateSub( this, "RAN_ANOTHER_CHAR_REGION_WINDOW", UI_FLAG_XSIZE | UI_FLAG_YSIZE, CHARACTER_INFO_VIEW_PAGE_STATS );
	m_pPageStats->CreateSubControl ();
	m_pPageStats->SetVisibleSingle( FALSE );
	RegisterControl ( m_pPageStats );
}

void CCharacterInfoViewWindow::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CCharacterInfoViewWindow::TranslateUIMessage( UIGUID cID, DWORD dwMsg )
{
	CUIWindowEx::TranslateUIMessage( cID, dwMsg );

	switch ( cID )
	{
	case ET_CONTROL_TITLE:
	case ET_CONTROL_TITLE_F:
		{
			if ( (dwMsg & UIMSG_LB_DUP) && CHECK_MOUSE_IN ( dwMsg ) ){
				CInnerInterface::GetInstance().SetDefaultPosInterface( GetWndID() );
			}
		}break;


	case CHARACTER_INFO_VIEW_BUTTON_STATS:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg) )
			{
				int nSelected = cID - CHARACTER_INFO_VIEW_BUTTON_STATS;
				OpenPage( nSelected );
			}
		}break;
	
	};

}

void CCharacterInfoViewWindow::SetVisibleSingle( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		OpenPage( CHARACTER_INFO_VIEW_WINDOW_PAGE_1 );
	}
}

void CCharacterInfoViewWindow::OpenPage( int nPage )
{
	if ( nPage == m_nPage )	return;

	if ( m_pButtonStats )	m_pButtonStats->DoImageFlip( FALSE );


	if ( m_pPageStats )	m_pPageStats->SetVisibleSingle( FALSE );


	m_nPage = nPage;

	switch( nPage )
	{
	case CHARACTER_INFO_VIEW_WINDOW_PAGE_1:
		{
			if ( m_pButtonStats )	m_pButtonStats->DoImageFlip( TRUE );

			if ( m_pPageStats )
			{
				m_pPageStats->SetVisibleSingle( TRUE );
				m_pPageStats->SetCharacterData( m_sInfoChar );
			}
		}break;
	};
}

void CCharacterInfoViewWindow::SetCharacterData( SINFO_CHAR* pInfoChar )
{
	if ( !pInfoChar )	return;

	m_sInfoChar = *pInfoChar;

	if ( m_pPageStats )
		m_pPageStats->SetCharacterData( m_sInfoChar );

	CString strWindowTitle;
	strWindowTitle.Format( ID2GAMEWORD( "RAN_ANOTHER_CHAR_WINDOW_PAGE", 0 ), m_sInfoChar.szName );
	SetTitleName( strWindowTitle.GetString() );
}