#include "StdAfx.h"
#include "PandoraBoxButton.h"
#include "./InnerInterface.h"
#include "./GameTextControl.h"
#include "./UITextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPandoraBoxButton::CPandoraBoxButton ()
	: m_pButtonImage(NULL)
{
}

CPandoraBoxButton::~CPandoraBoxButton ()
{
}

void CPandoraBoxButton::CreateSubControl ()
{
	m_pButtonImage = new CUIControl;
	m_pButtonImage->CreateSub ( this, "PANDORA_BUTTON_IMAGE", UI_FLAG_DEFAULT, PANDORA_BUTTON_IMAGE );
	m_pButtonImage->SetVisibleSingle( TRUE );
	RegisterControl ( m_pButtonImage );
}

void CPandoraBoxButton::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CPandoraBoxButton::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
		case PANDORA_BUTTON_IMAGE:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) ){
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( "Pandora's Box" , NS_UITEXTCOLOR::WHITE  );				
			}

			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){ 
				CInnerInterface::GetInstance().SetPandoraBoxInfo();
			}
		}break;
	}
}
