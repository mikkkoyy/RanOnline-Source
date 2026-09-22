#include "StdAfx.h"
#include "CharacterCreateRotate.h"

#include "BasicButton.h"
#include "DxGlobalStage.h"
#include "DxLobyStage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCharacterCreateRotate::CCharacterCreateRotate ()
	: m_pRotateButtonLeft( NULL )
	, m_pRotateButtonRight( NULL )
{
}

CCharacterCreateRotate::~CCharacterCreateRotate ()
{
}

void CCharacterCreateRotate::CreateSubControl ()
{
	m_pRotateButtonLeft = new CBasicButton;
	m_pRotateButtonLeft->CreateSub( this, "ROTATE_CHAR_LEFT", UI_FLAG_XSIZE | UI_FLAG_YSIZE, CHARACTER_CREATE_ROTATE_LEFT );
	m_pRotateButtonLeft->CreateFlip( "ROTATE_CHAR_LEFT_F", CBasicButton::CLICK_FLIP );
	m_pRotateButtonLeft->SetAlignFlag ( UI_FLAG_LEFT );
	m_pRotateButtonLeft->SetVisibleSingle ( TRUE );
	RegisterControl( m_pRotateButtonLeft );

	m_pRotateButtonRight = new CBasicButton;
	m_pRotateButtonRight->CreateSub( this, "ROTATE_CHAR_RIGHT", UI_FLAG_XSIZE | UI_FLAG_YSIZE, CHARACTER_CREATE_ROTATE_RIGHT );
	m_pRotateButtonRight->CreateFlip( "ROTATE_CHAR_RIGHT_F", CBasicButton::CLICK_FLIP );
	m_pRotateButtonRight->SetAlignFlag ( UI_FLAG_RIGHT );
	m_pRotateButtonRight->SetVisibleSingle ( TRUE );
	RegisterControl( m_pRotateButtonRight );
}

void CCharacterCreateRotate::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case CHARACTER_CREATE_ROTATE_LEFT:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSE_IN_LBDOWNLIKE ( dwMsg ) )
			{
				DxGlobalStage::GetInstance().GetLobyStage()->RotateChar( TRUE );
			}
		}break;

	case CHARACTER_CREATE_ROTATE_RIGHT:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSE_IN_LBDOWNLIKE ( dwMsg ) )
			{
				DxGlobalStage::GetInstance().GetLobyStage()->RotateChar( FALSE );
			}
		}break;
	}
}
