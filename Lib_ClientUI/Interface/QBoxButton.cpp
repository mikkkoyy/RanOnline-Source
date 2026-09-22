#include "StdAfx.h"
#include "./BasicButton.h"
#include "GLGaeaClient.h"
#include "GLCharacter.h"
#include "InnerInterface.h"
#include ".\qboxbutton.h"
#include "GameTextControl.h"
#include "UITextControl.h"
#include "QBoxButtonIcon.h"
#include "BasicMiniBarGauge.h"

CQBoxButton::CQBoxButton(void) :
	m_pQBoxButton(NULL),
	m_bQBoxEnable(TRUE),
	m_bQBoxAlarm(FALSE),
	m_pQBoxLock(NULL),
	m_pQBoxGauge( NULL ),
	m_fBLINK_TIME(0.0f),
	m_fOrigTime(0.0f),
	m_fRemainTime(0.0f)
{
}

CQBoxButton::~CQBoxButton(void)
{
}

void CQBoxButton::SetQBoxEnable( bool bQBoxEnable )
{
	if( m_bQBoxEnable != bQBoxEnable )
	{
		GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
		if( pCharacter->IsPartyMem() && pCharacter->IsPartyMaster() == FALSE )
		{
			m_bQBoxAlarm = TRUE;
		}
	}

	m_bQBoxEnable = bQBoxEnable;
	m_pQBoxButton->SetFlip( m_bQBoxEnable );
	m_pQBoxLock->SetVisibleSingle( !m_bQBoxEnable );
}

void CQBoxButton::CreateSubControl ()
{
	m_pQBoxButton = new CQBoxButtonIcon;
	m_pQBoxButton->CreateSub ( this, "QBOX_BUTTON_BUTTON", UI_FLAG_DEFAULT, QBOX_ON_OFF_BUTTON );
	m_pQBoxButton->CreateFlip ( "QBOX_BUTTON_BUTTON_F", CBasicButton::RADIO_FLIP );
	m_pQBoxButton->CreateSubControl();
	m_pQBoxButton->SetUseGlobalAction ( TRUE );
	m_pQBoxButton->SetFlip( m_bQBoxEnable );
	RegisterControl ( m_pQBoxButton );

	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, "QBOX_ALARM_BLINK" );
	pControl->SetVisibleSingle ( FALSE );
	RegisterControl ( pControl );
	m_pAlarmBlink = pControl;

	m_pQBoxLock = new CUIControl;
	m_pQBoxLock->CreateSub ( this, "QBOX_BUTTON_BUTTON_LOCK" );
	m_pQBoxLock->SetVisibleSingle( !m_bQBoxEnable );
	RegisterControl ( m_pQBoxLock );

	m_pQBoxGauge = new CBasicMiniBarGauge;
	m_pQBoxGauge->CreateSub( this, "QBOX_GAUGE_BOX", UI_FLAG_DEFAULT, QBOX_GAUGE );
	m_pQBoxGauge->CreateSubControl();
	m_pQBoxLock->SetVisibleSingle( FALSE );
	RegisterControl ( m_pQBoxGauge );
}

void CQBoxButton::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	const float fBLINK_TIME_LOOP = 0.2f;

	if ( m_bQBoxAlarm )
	{		
		m_fBLINK_TIME += fElapsedTime;
		if ( fBLINK_TIME_LOOP <= m_fBLINK_TIME )
		{
			BOOL bVisible = m_pAlarmBlink->IsVisible ();
			m_pAlarmBlink->SetVisibleSingle ( !bVisible );
			m_fBLINK_TIME = 0.0f;
		}
	}

	EMITEM_QUESTION emTYPE = GLGaeaClient::GetInstance().GetCharacter()->m_sQITEMFACT.emType;
	float fTime = GLGaeaClient::GetInstance().GetCharacter()->m_sQITEMFACT.fTime;
	if ( emTYPE != m_pQBoxButton->GetType() )
	{
		m_pQBoxButton->SetType( emTYPE );
		m_pQBoxGauge->SetVisibleSingle( emTYPE != QUESTION_NONE );
		m_fRemainTime = m_fOrigTime = fTime;
	}

	if ( emTYPE != QUESTION_NONE )
	{
		m_fRemainTime -= fElapsedTime;
		float fPercent = m_fRemainTime / m_fOrigTime;
		m_pQBoxGauge->SetPercent( fPercent );

		if ( fPercent <= 0.0f )
			m_pQBoxGauge->SetVisibleSingle( FALSE );
	}
}

void CQBoxButton::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	switch ( ControlID )
	{
	case QBOX_ON_OFF_BUTTON:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEINTEXT( "QBOX_BUTTON_MESSAGE" ), NS_UITEXTCOLOR::WHITE  );				
			}

			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
				if( !pCharacter->IsPartyMem() || pCharacter->IsPartyMaster() )
				{
					m_bQBoxEnable = !m_bQBoxEnable;
					m_pQBoxButton->SetFlip( m_bQBoxEnable );
					m_pQBoxLock->SetVisibleSingle( !m_bQBoxEnable );
					if( pCharacter->IsPartyMaster() )
					{
						pCharacter->ReqQBoxEnableState(m_bQBoxEnable);		
					}
				}else{
					m_bQBoxAlarm = FALSE;
					m_pAlarmBlink->SetVisibleSingle ( FALSE );
				}
			}
		}break;

	case QBOX_GAUGE:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				CString strPercent;
				strPercent.Format( "%3.0f%%", m_pQBoxGauge->GetPercent() * 100.0f );
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( strPercent.GetString(), NS_UITEXTCOLOR::WHITE );				
			}
		}break;
	}
}

void CQBoxButton::ResetType()
{
	if ( m_pQBoxButton )
		m_pQBoxButton->SetType( QUESTION_NONE );
};