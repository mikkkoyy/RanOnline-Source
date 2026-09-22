#include "StdAfx.h"
#include "CompetitionNotifyButton.h"
#include "./BasicButton.h"
#include "./CompetitionNotifyButtonAlarm.h"

#include "./InnerInterface.h"
#include "./GameTextControl.h"
#include "./UITextControl.h"
#include "RANPARAM.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BLINK_TIME_LOOP (float)0.2f

CCompetitionNotifyButton::CCompetitionNotifyButton ()
	: m_pButtonImage(NULL)
	, m_pButtonBlink(NULL)
	, m_pButtonLock(NULL)
	, m_pButtonAlarm(NULL)
	, m_fBLINK_TIME(0.0f)
	, m_bAlarm(FALSE)
	, m_bLocked(FALSE)
{
}

CCompetitionNotifyButton::~CCompetitionNotifyButton ()
{
}

void CCompetitionNotifyButton::CreateSubControl ()
{
	m_pButtonImage = new CUIControl;
	m_pButtonImage->CreateSub ( this, "COMPETITION_NOTIFY_BUTTON_IMAGE", UI_FLAG_DEFAULT, NOTIFYCOMPETITION_BUTTON_IMAGE );
	m_pButtonImage->SetVisibleSingle( TRUE );
	RegisterControl ( m_pButtonImage );

	m_pButtonBlink = new CUIControl;
	m_pButtonBlink->CreateSub ( this, "COMPETITION_NOTIFY_BUTTON_BLINK" );
	m_pButtonBlink->SetVisibleSingle ( FALSE );
	RegisterControl ( m_pButtonBlink );

	m_pButtonLock = new CUIControl;
	m_pButtonLock->CreateSub ( this, "COMPETITION_NOTIFY_LOCK" );
	m_pButtonLock->SetVisibleSingle ( FALSE );
	RegisterControl ( m_pButtonLock );

	m_pButtonAlarm = new CCompetitionNotifyButtonAlarm;
	m_pButtonAlarm->CreateSub ( this, "COMPETITION_NOTIFY_BUTTON_ALARM", UI_FLAG_DEFAULT, NOTIFYCOMPETITION_BUTTON_ALARM );
	m_pButtonAlarm->CreateSubControl ();
	m_pButtonAlarm->SetVisibleSingle( FALSE );	
	RegisterControl ( m_pButtonAlarm );
}

void CCompetitionNotifyButton::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( m_bAlarm ){		
		m_fBLINK_TIME += fElapsedTime;
		if ( BLINK_TIME_LOOP <= m_fBLINK_TIME ){
			if ( m_pButtonBlink ){
				BOOL bVisible = m_pButtonBlink->IsVisible ();
				m_pButtonBlink->SetVisibleSingle ( !bVisible );
			}
		
			m_fBLINK_TIME = 0.0f;
		}
	}
}

void CCompetitionNotifyButton::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case NOTIFYCOMPETITION_BUTTON_IMAGE:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) ){
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEINTEXT( "NOTIFYCOMPETITION_BUTTON_MESSAGE" ), NS_UITEXTCOLOR::WHITE  );				
			}

			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){ 
				CInnerInterface::GetInstance().ShowCompetitionWindow();
			}
		}break;
	}
}

void CCompetitionNotifyButton::SetButtonAlarm( BOOL bSet )
{
	m_bAlarm = bSet;

	if ( m_pButtonAlarm ){
		m_pButtonAlarm->SetVisibleSingle( m_bAlarm );
	}

	if ( m_pButtonBlink ){
		m_pButtonBlink->SetVisibleSingle ( m_bAlarm );
	}
}

void CCompetitionNotifyButton::SetButtonLock( BOOL bSet )
{
	m_bLocked = bSet;
	if ( m_pButtonLock ){
		m_pButtonLock->SetVisibleSingle( m_bLocked );
	}
}