#include "StdAfx.h"
#include "GameTextControl.h"
#include "UITextControl.h"
#include "InnerInterface.h"
#include "PartyFinderIcon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CPartyFinderIcon::CPartyFinderIcon()
	: m_pImage( NULL )
{
}

CPartyFinderIcon::~CPartyFinderIcon()
{
}

void CPartyFinderIcon::CreateSubControl ()
{
	m_pImage =  new CUIControl;
	m_pImage->CreateSub ( this, "PARTY_FINDER_ICON_IMAGE", UI_FLAG_DEFAULT, MouseOpen );	
	m_pImage->SetVisibleSingle(TRUE);
	RegisterControl ( m_pImage );
}
void CPartyFinderIcon::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}


void CPartyFinderIcon::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{

CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

switch ( ControlID )
	{
	case MouseOpen:
		{
			
		if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( "Party Finder Search", NS_UITEXTCOLOR::WHITE  );				
				
			
			if ( dwMsg & UIMSG_LB_UP )
				{
					if ( !CInnerInterface::GetInstance().IsVisibleGroup ( PARTYFINDER_WINDOW ) )
						CInnerInterface::GetInstance().OpenPartyFinderWindow();
					else
						CInnerInterface::GetInstance().ClosePartyFinderWindow();
				}
			}
		}
		break;
	}
}