#include "StdAfx.h"
#include "BikeColorWindow.h"
#include "BikeColorSelect.h"

#include "InnerInterface.h"

#include "GLGaeaClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CBikeColorWindow::CBikeColorWindow(void)
	: m_pSelect(NULL)
{
}

CBikeColorWindow::~CBikeColorWindow(void)
{
}

void CBikeColorWindow::CreateSubControl()
{
	m_pSelect = new CBikeColorSelect;
	m_pSelect->CreateSub( this, "RAN_BIKE_COLOR_PAGE", UI_FLAG_XSIZE | UI_FLAG_YSIZE, BIKE_COLOR_SELECT );
	m_pSelect->CreateSubControl ();
	m_pSelect->SetVisibleSingle( TRUE );
	RegisterControl ( m_pSelect );
}

void CBikeColorWindow::TranslateUIMessage( UIGUID cID, DWORD dwMsg )
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

	case BIKE_COLOR_SELECT:
		{
			if( CHECK_MOUSE_IN( dwMsg ) )
			{
				if ( UIMSG_BIKE_COLOR_SELECT_CLOSE & dwMsg )
				{
					CInnerInterface::GetInstance().HideGroup( GetWndID() );
				}

				if ( UIMSG_BIKE_COLOR_SELECT_SAVE & dwMsg )
				{
					GLGaeaClient::GetInstance().GetCharacter()->ReqBikeColorChange();

					CInnerInterface::GetInstance().HideGroup( GetWndID() );
				}
			}
		}break;

	};

}

void CBikeColorWindow::SetVisibleSingle( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		if ( m_pSelect )
			m_pSelect->SelectSlot( 0 );
	}
	else
	{
		if ( m_pSelect )
			m_pSelect->ClearEditBox();

		GLGaeaClient::GetInstance().GetCharacter()->BikeColorEnd();
	}
}