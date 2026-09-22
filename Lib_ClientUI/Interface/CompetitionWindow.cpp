#include "StdAfx.h"
#include "CompetitionWindow.h"

#include "BasicLineBox.h"
#include "MultiModeButton.h"
#include "BasicTextBox.h"
#include "BasicTextButton.h"

#include "GameTextControl.h"
#include "InnerInterface.h"
#include "UITextControl.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"

#include "PVPTyrannyPage.h"			/*pvp tyranny, Juver, 2017/08/25 */
#include "PVPSchoolWarsPage.h"		/*school wars, Juver, 2018/01/19 */
#include "PVPCaptureTheFlagPage.h"	/*pvp capture the flag, Juver, 2018/01/25 */

#include "../Lib_Engine/Common/StringUtils.h"
#include "../Lib_Engine/Utils/RANPARAM.h"
#include "RANPARAM.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCompetitionWindow::CCompetitionWindow ()
	: m_nPage(-1)
	, m_pButtonClose(NULL)
	, m_pButtonEventTyranny(NULL)
	, m_pButtonEventSchoolWars(NULL)
	, m_pButtonEventCaptureTheFlag(NULL)
	, m_pPageTyranny(NULL)				/*pvp tyranny, Juver, 2017/08/25 */
	, m_pPageSchoolWars(NULL)			/*school wars, Juver, 2018/01/19 */
	, m_pPageCaptureTheFlag(NULL)		/*pvp capture the flag, Juver, 2018/01/25 */
{
	
}

CCompetitionWindow::~CCompetitionWindow ()
{
}

void CCompetitionWindow::CreateSubControl ()
{
	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhite( "COMPETITION_OBJECT_LINE" );
	RegisterControl ( pLineBox );	

	m_pButtonClose = new CBasicTextButton;
	m_pButtonClose->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, COMPETITION_WINDOW_BUTTON_CLOSE );
	m_pButtonClose->CreateBaseButton ( "COMPETITION_OBJECT_CLOSE", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, ID2GAMEWORD("COMPETITION_WINDOW", 1 ) );
	m_pButtonClose->SetFlip ( TRUE );
	RegisterControl ( m_pButtonClose );

	m_pButtonEventTyranny = CreateTabButton( "COMPETITION_OBJECT_TAP_0", ID2GAMEWORD("COMPETITION_BUTTON", 0 ), COMPETITION_WINDOW_BUTTON_TYRANNY );
	m_pButtonEventSchoolWars = CreateTabButton( "COMPETITION_OBJECT_TAP_1", ID2GAMEWORD("COMPETITION_BUTTON", 1 ), COMPETITION_WINDOW_BUTTON_SCHOOLWARS );
	m_pButtonEventCaptureTheFlag = CreateTabButton( "COMPETITION_OBJECT_TAP_2", ID2GAMEWORD("COMPETITION_BUTTON", 2 ), COMPETITION_WINDOW_BUTTON_CAPTURE_THE_FLAG );

	/*pvp tyranny, Juver, 2017/08/25 */
	m_pPageTyranny = new CPVPTyrannyPage;
	m_pPageTyranny->CreateSub( this, "COMPETITION_OBJECT_PAGE", UI_FLAG_XSIZE | UI_FLAG_YSIZE, COMPETITION_WINDOW_PAGE_TYRANNY );
	m_pPageTyranny->CreateSubControl ();
	RegisterControl ( m_pPageTyranny );

	/*school wars, Juver, 2018/01/19 */
	m_pPageSchoolWars = new CPVPSchoolWarsPage;
	m_pPageSchoolWars->CreateSub( this, "COMPETITION_OBJECT_PAGE", UI_FLAG_XSIZE | UI_FLAG_YSIZE, COMPETITION_WINDOW_PAGE_SCHOOLWARS );
	m_pPageSchoolWars->CreateSubControl ();
	RegisterControl ( m_pPageSchoolWars );

	/*pvp capture the flag, Juver, 2018/01/25 */
	m_pPageCaptureTheFlag = new CPVPCaptureTheFlagPage;
	m_pPageCaptureTheFlag->CreateSub( this, "COMPETITION_OBJECT_PAGE", UI_FLAG_XSIZE | UI_FLAG_YSIZE, COMPETITION_WINDOW_PAGE_CAPTURE_THE_FLAG );
	m_pPageCaptureTheFlag->CreateSubControl ();
	RegisterControl ( m_pPageCaptureTheFlag );

	m_pPageCaptureTheFlag->SetVisibleSingle(FALSE);

}

CMultiModeButton* CCompetitionWindow::CreateTabButton( const char* szButtonBox, const char* szButtonText, UIGUID ControlID )
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

void CCompetitionWindow::SetVisibleSingle ( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle ( bVisible );

	if( bVisible )
	{
		OpenPage( COMPETITION_WINDOW_TYPE_TYRANNY );
	}
	else
	{
	}
}

void CCompetitionWindow::TranslateUIMessage ( UIGUID cID, DWORD dwMsg )
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

	
	case COMPETITION_WINDOW_BUTTON_CLOSE:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) ){
				CInnerInterface::GetInstance().HideGroup( GetWndID() );
			}
		}break;

	case COMPETITION_WINDOW_BUTTON_TYRANNY:
	case COMPETITION_WINDOW_BUTTON_SCHOOLWARS:
	case COMPETITION_WINDOW_BUTTON_CAPTURE_THE_FLAG:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg) )
			{
				int nSelected = cID - COMPETITION_WINDOW_BUTTON_TYRANNY;
				OpenPage( nSelected );
			}
		}break;
	};
	
}

void CCompetitionWindow::OpenPage( int nPage )
{
	if ( nPage == m_nPage )	return;

	if ( m_pButtonEventTyranny )		m_pButtonEventTyranny->DoImageFlip( FALSE );
	if ( m_pButtonEventSchoolWars )		m_pButtonEventSchoolWars->DoImageFlip( FALSE );
	if ( m_pButtonEventCaptureTheFlag )	m_pButtonEventCaptureTheFlag->DoImageFlip( FALSE );

	/*pvp tyranny, Juver, 2017/08/25 */
	if ( m_pPageTyranny )	m_pPageTyranny->SetVisibleSingle( FALSE );

	/*school wars, Juver, 2018/01/19 */
	if ( m_pPageSchoolWars )	m_pPageSchoolWars->SetVisibleSingle( FALSE );

	/*pvp capture the flag, Juver, 2018/01/25 */
	if ( m_pPageCaptureTheFlag )	m_pPageCaptureTheFlag->SetVisibleSingle( FALSE );

	m_nPage = nPage;

	switch( nPage )
	{
	case COMPETITION_WINDOW_TYPE_TYRANNY:
		{
			/*pvp tyranny, Juver, 2017/08/25 */
			if ( m_pButtonEventTyranny )	m_pButtonEventTyranny->DoImageFlip( TRUE );
			if ( m_pPageTyranny )			m_pPageTyranny->SetVisibleSingle( TRUE );
		}break;

	case COMPETITION_WINDOW_TYPE_SCHOOLWARS:
		{
			/*school wars, Juver, 2018/01/19 */
			if ( m_pButtonEventSchoolWars )	m_pButtonEventSchoolWars->DoImageFlip( TRUE );
			if ( m_pPageSchoolWars )		m_pPageSchoolWars->SetVisibleSingle( TRUE );
		}break;

	case COMPETITION_WINDOW_TYPE_CAPTURE_THE_FLAG:
		{
			/*pvp capture the flag, Juver, 2018/01/25 */
			if ( m_pButtonEventCaptureTheFlag )	m_pButtonEventCaptureTheFlag->DoImageFlip( TRUE );
			if ( m_pPageCaptureTheFlag )		m_pPageCaptureTheFlag->SetVisibleSingle( TRUE );
		}break;
	};
}