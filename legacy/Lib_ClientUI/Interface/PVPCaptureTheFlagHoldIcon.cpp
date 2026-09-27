#include "StdAfx.h"
#include "PVPCaptureTheFlagHoldIcon.h"
#include "InnerInterface.h"
#include "UITextControl.h"
#include "BasicMiniBarGauge.h"
#include "GLGaeaClient.h"
#include "GLPVPCaptureTheFlagClient.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPCaptureTheFlagHoldIcon::CPVPCaptureTheFlagHoldIcon () 
	: m_pProgressTimer(NULL)
	, m_fUpdateTimer(0.0f)
{
	for ( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
		m_pTeamImage[i] = NULL;
}

CPVPCaptureTheFlagHoldIcon::~CPVPCaptureTheFlagHoldIcon ()
{
}

void  CPVPCaptureTheFlagHoldIcon::CreateSubControl ()
{
	std::string strteam_image[CAPTURE_THE_FLAG_TEAM_SIZE] = { "PVP_CAPTURE_THE_FLAG_HOLD_ICON_IMAGE_A", "PVP_CAPTURE_THE_FLAG_HOLD_ICON_IMAGE_B" };

	for ( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
	{
		m_pTeamImage[i] = new CUIControl;
		m_pTeamImage[i]->CreateSub ( this, strteam_image[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		m_pTeamImage[i]->SetVisibleSingle( FALSE );
		RegisterControl ( m_pTeamImage[i] );
	}

	m_pProgressTimer = new CBasicMiniBarGauge;
	m_pProgressTimer->CreateSub( this, "PVP_CAPTURE_THE_FLAG_HOLD_ICON_TIMER", UI_FLAG_DEFAULT, TIMER_GAUGE );
	m_pProgressTimer->CreateSubControl();
	m_pProgressTimer->SetVisibleSingle( TRUE );
	RegisterControl ( m_pProgressTimer );
}

void CPVPCaptureTheFlagHoldIcon::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
	
	m_fUpdateTimer += fElapsedTime;
	if ( m_fUpdateTimer >= 0.2f )
		UpdateInfo();
}

void CPVPCaptureTheFlagHoldIcon::UpdateInfo()
{
	GLCharacter* pcharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( !pcharacter )	return;

	if ( !pcharacter->m_bCaptureTheFlagHoldFlag )
	{
		CInnerInterface::GetInstance().HideGroup( GetWndID() );
		return;
	}

	if ( m_pTeamImage[CAPTURE_THE_FLAG_TEAM_A] )
		m_pTeamImage[CAPTURE_THE_FLAG_TEAM_A]->SetVisibleSingle( pcharacter->m_wCaptureTheFlagTeam == CAPTURE_THE_FLAG_TEAM_B );

	if ( m_pTeamImage[CAPTURE_THE_FLAG_TEAM_B] )
		m_pTeamImage[CAPTURE_THE_FLAG_TEAM_B]->SetVisibleSingle( pcharacter->m_wCaptureTheFlagTeam == CAPTURE_THE_FLAG_TEAM_A );

	if ( m_pProgressTimer )
	{
		float fPercent = pcharacter->m_fCaptureTheFlagHoldFlagTimer / CAPTURE_THE_FLAG_HOLD_MAX_TIME;
		m_pProgressTimer->SetPercent( fPercent );
	}
}