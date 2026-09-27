#include "StdAfx.h"
#include "PVPCaptureTheFlagBattleNotice.h"
#include "InnerInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPCaptureTheFlagBattleNotice::CPVPCaptureTheFlagBattleNotice ()
	: m_pImage(NULL)
	, m_pImageBG(NULL)
	, m_pImageIcon(NULL)
	, m_fTimer(0.0f)
{
}

CPVPCaptureTheFlagBattleNotice::~CPVPCaptureTheFlagBattleNotice ()
{
}

void CPVPCaptureTheFlagBattleNotice::CreateSubControlStartImage ()
{
	m_pImageBG = new CUIControl;
	m_pImageBG->CreateSub ( this, "PROGRESS_DISPLAY_CAPTURE_THE_FLAG_BACK_0" );
	RegisterControl ( m_pImageBG );

	m_pImage = new CUIControl;
	m_pImage->CreateSub ( this, "PROGRESS_DISPLAY_CAPTURE_THE_FLAG_TEXT_START" );
	RegisterControl ( m_pImage );

	m_pImageIcon = new CUIControl;
	m_pImageIcon->CreateSub ( this, "PROGRESS_DISPLAY_CAPTURE_THE_FLAG_ICON" );
	RegisterControl ( m_pImageIcon );
}

void CPVPCaptureTheFlagBattleNotice::CreateSubControlEndImage ()
{
	m_pImageBG = new CUIControl;
	m_pImageBG->CreateSub ( this, "PROGRESS_DISPLAY_CAPTURE_THE_FLAG_BACK_0" );
	RegisterControl ( m_pImageBG );

	m_pImage = new CUIControl;
	m_pImage->CreateSub ( this, "PROGRESS_DISPLAY_CAPTURE_THE_FLAG_TEXT_STOP" );
	RegisterControl ( m_pImage );

	m_pImageIcon = new CUIControl;
	m_pImageIcon->CreateSub ( this, "PROGRESS_DISPLAY_CAPTURE_THE_FLAG_ICON" );
	RegisterControl ( m_pImageIcon );
}

void CPVPCaptureTheFlagBattleNotice::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	m_fTimer += fElapsedTime;
	if ( m_fTimer >= 5.0f )
	{
		m_fTimer = 0.0f;
		CInnerInterface::GetInstance().HideGroup( GetWndID() );
	}
}
