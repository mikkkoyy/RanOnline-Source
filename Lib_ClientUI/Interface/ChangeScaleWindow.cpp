#include "StdAfx.h"
#include "ChangeScaleWindow.h"
#include "InnerInterface.h"

#include "BasicProgressBar.h"
#include "BasicButton.h"
#include "BasicTextButton.h"
#include "BasicLineBox.h"

#include "GLGaeaClient.h"
#include "UITextControl.h"
#include "GameTextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CChangeScaleWindow::CChangeScaleWindow(void)
	: m_pCharacterScaleBar(NULL)
	, m_pCharacterScaleButtonLeft(NULL)
	, m_pCharacterScaleButtonRight(NULL)
	, m_pCharacterScaleButtonClose(NULL)
	, m_pCharacterScaleButtonSave(NULL)
{
}

CChangeScaleWindow::~CChangeScaleWindow(void)
{
}

void CChangeScaleWindow::CreateSubControl()
{
	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_CHANGE_SCALE_WINDOW_LINEBOX" );
	RegisterControl ( pBasicLineBox );


	m_pCharacterScaleButtonLeft = new CBasicButton;
	m_pCharacterScaleButtonLeft->CreateSub ( this,  "RAN_CHANGE_SCALE_BUTTON_L", UI_FLAG_DEFAULT, CHANGE_SCALE_WINDOW_BUTTON_LEFT );
	m_pCharacterScaleButtonLeft->CreateFlip ( "RAN_CHANGE_SCALE_BUTTON_L_F", CBasicButton::CLICK_FLIP );
	m_pCharacterScaleButtonLeft->SetFlip(false);
	m_pCharacterScaleButtonLeft->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pCharacterScaleButtonLeft );

	m_pCharacterScaleButtonRight = new CBasicButton;
	m_pCharacterScaleButtonRight->CreateSub ( this,  "RAN_CHANGE_SCALE_BUTTON_R", UI_FLAG_DEFAULT, CHANGE_SCALE_WINDOW_BUTTON_RIGHT );
	m_pCharacterScaleButtonRight->CreateFlip ( "RAN_CHANGE_SCALE_BUTTON_R_F", CBasicButton::CLICK_FLIP );
	m_pCharacterScaleButtonRight->SetFlip(false);
	m_pCharacterScaleButtonRight->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pCharacterScaleButtonRight );

	m_pCharacterScaleBar = new CBasicProgressBar;
	m_pCharacterScaleBar->CreateSub ( this, "RAN_CHANGE_SCALE_BAR" );
	m_pCharacterScaleBar->CreateOverImage ( "RAN_CHANGE_SCALE_BAR_OVERIMAGE" );
	m_pCharacterScaleBar->SetControlNameEx ( "Character Scale" );
	m_pCharacterScaleBar->SetPercent ( 1.0f );
	RegisterControl ( m_pCharacterScaleBar );


	m_pCharacterScaleButtonSave = new CBasicTextButton;
	m_pCharacterScaleButtonSave->CreateSub ( this, "BASIC_TEXT_BUTTON14", UI_FLAG_XSIZE, CHANGE_SCALE_WINDOW_BUTTON_SAVE );
	m_pCharacterScaleButtonSave->CreateBaseButton ( "RAN_CHANGE_SCALE_OK", CBasicTextButton::SIZE14, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_CHANGE_SCALE_WINDOW", 2 ) );
	RegisterControl ( m_pCharacterScaleButtonSave );

	m_pCharacterScaleButtonClose = new CBasicTextButton;
	m_pCharacterScaleButtonClose->CreateSub ( this, "BASIC_TEXT_BUTTON14", UI_FLAG_XSIZE, CHANGE_SCALE_WINDOW_BUTTON_CANCEL );
	m_pCharacterScaleButtonClose->CreateBaseButton ( "RAN_CHANGE_SCALE_CANCEL", CBasicTextButton::SIZE14, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_CHANGE_SCALE_WINDOW", 1 ) );
	RegisterControl ( m_pCharacterScaleButtonClose );

}

void CChangeScaleWindow::TranslateUIMessage( UIGUID cID, DWORD dwMsg )
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

	case ET_CONTROL_BUTTON:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			{
				CInnerInterface::GetInstance().HideGroup( GetWndID() );
			}
		}break;

	case CHANGE_SCALE_WINDOW_BUTTON_SAVE:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
				
				GLGaeaClient::GetInstance().GetCharacter()->ChangeScaleSave();
				CInnerInterface::GetInstance().HideGroup( GetWndID() );
			}
		}break;

	case CHANGE_SCALE_WINDOW_BUTTON_CANCEL:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
				CInnerInterface::GetInstance().HideGroup( GetWndID() );
			}
		}break;

	case CHANGE_SCALE_WINDOW_BUTTON_LEFT:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
				CharacterScaleDecrease();
			}
		}break;

	case CHANGE_SCALE_WINDOW_BUTTON_RIGHT:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
				CharacterScaleIncrease();
			}
		}break;

	};

}

void CChangeScaleWindow::Update ( int x ,int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( pCharacter )
	{
		float fPercent = ( pCharacter->m_fActiveCharScale - SCALERANGE_MIN ) / ( SCALERANGE_MAX - SCALERANGE_MIN );
		m_pCharacterScaleBar->SetPercent ( fPercent );
	}
}

void CChangeScaleWindow::SetVisibleSingle( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle( bVisible );

	if( bVisible )
	{
	}
	else
	{
		GLGaeaClient::GetInstance().GetCharacter()->ChangeScaleCancel();
	}
}

void CChangeScaleWindow::CharacterScaleIncrease()
{
	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( pCharacter )
	{
		pCharacter->m_fActiveCharScale += 0.015f;
		if ( pCharacter->m_fActiveCharScale >= SCALERANGE_MAX ){
			pCharacter->m_fActiveCharScale = SCALERANGE_MAX;
		}
	}
}

void CChangeScaleWindow::CharacterScaleDecrease()
{
	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( pCharacter )
	{
		pCharacter->m_fActiveCharScale -= 0.015f;
		if ( pCharacter->m_fActiveCharScale <= SCALERANGE_MIN ){
			pCharacter->m_fActiveCharScale = SCALERANGE_MIN;
		}
	}
}