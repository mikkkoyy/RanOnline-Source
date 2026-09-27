#include "StdAfx.h"
#include "PlayerRankingButton.h"
#include "./InnerInterface.h"
#include "./GameTextControl.h"
#include "./UITextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPlayerRankingButton::CPlayerRankingButton ()
	: m_pButtonImage(NULL)
{
}

CPlayerRankingButton::~CPlayerRankingButton ()
{
}

void CPlayerRankingButton::CreateSubControl ()
{
	m_pButtonImage = new CUIControl;
	m_pButtonImage->CreateSub ( this, "RAN_PLAYERRANKING_NOTIFY_BUTTON_IMAGE", UI_FLAG_DEFAULT, PLAYER_RANKING_BUTTON_IMAGE );
	m_pButtonImage->SetVisibleSingle( TRUE );
	RegisterControl ( m_pButtonImage );
}

void CPlayerRankingButton::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CPlayerRankingButton::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case PLAYER_RANKING_BUTTON_IMAGE:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) ){
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEWORD( "RAN_PLAYERRANKING_WINDOW_TITLE", 0 ), NS_UITEXTCOLOR::WHITE  );				
			}

			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{ 
				
			}
		}break;
	}
}
