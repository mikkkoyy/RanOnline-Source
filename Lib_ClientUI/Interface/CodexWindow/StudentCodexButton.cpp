#include "StdAfx.h"
#include "StudentCodexButton.h"
#include "./InnerInterface.h"
#include "./GameTextControl.h"
#include "./UITextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CStudentCodexButton::CStudentCodexButton ()
	: m_pButtonImage(NULL)
{
}

CStudentCodexButton::~CStudentCodexButton ()
{
}

void CStudentCodexButton::CreateSubControl ()
{
	m_pButtonImage = new CUIControl;
	m_pButtonImage->CreateSub ( this, "STUDENT_CODEX_BUTTON_IMAGE", UI_FLAG_DEFAULT, STUDENT_CODEX_BUTTON_IMAGE );
	m_pButtonImage->SetVisibleSingle( TRUE );
	RegisterControl ( m_pButtonImage );
}

void CStudentCodexButton::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CStudentCodexButton::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case STUDENT_CODEX_BUTTON_IMAGE:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) ){
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEWORD( "RAN_CODEX_WINDOW_TITLE", 0 ), NS_UITEXTCOLOR::WHITE  );				
			}

			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){ 
				CInnerInterface::GetInstance().OpenStudentCodex();
			}
		}break;
	}
}
