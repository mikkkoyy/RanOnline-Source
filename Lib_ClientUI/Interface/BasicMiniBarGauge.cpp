#include "StdAfx.h"
#include "BasicMiniBarGauge.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const float CBasicMiniBarGauge::fZERO_PERCENT = 0.0f;
const float CBasicMiniBarGauge::fHUNDRED_PERCENT = 1.0f;
const float CBasicMiniBarGauge::fSIXTY_PERCENT = 0.66f;
const float CBasicMiniBarGauge::fTHIRTY_PERCENT = 0.33f;

CBasicMiniBarGauge::CBasicMiniBarGauge () :
	m_fPercent ( fZERO_PERCENT ),
	m_pBackGround ( NULL ),
	m_pOverImageRed ( NULL ),
	m_pOverImageYellow ( NULL ),
	m_pOverImageGreen ( NULL ),
	m_pOverImageRedDummy ( NULL ),
	m_pOverImageYellowDummy ( NULL ),
	m_pOverImageGreenDummy ( NULL )
{
}

CBasicMiniBarGauge::~CBasicMiniBarGauge ()
{
}

void CBasicMiniBarGauge::SetPercent ( float fPercent )
{
	if ( fPercent < fZERO_PERCENT )			fPercent = fZERO_PERCENT;
	else if ( fHUNDRED_PERCENT < fPercent )	fPercent = fHUNDRED_PERCENT;

	m_fPercent = fPercent;
}

float CBasicMiniBarGauge::GetPercent ( void )
{
	return m_fPercent;
}

void CBasicMiniBarGauge::UpdateProgress ()
{
	if  ( m_pOverImageRed && m_pOverImageRedDummy )
	{
		UIRECT rcPos = m_pOverImageRedDummy->GetGlobalPos();
		UIRECT rcTexPos = m_pOverImageRedDummy->GetTexturePos ();

		rcPos.sizeY = rcPos.sizeY * m_fPercent;
		rcPos.top = rcPos.bottom - rcPos.sizeY;
		m_pOverImageRed->SetGlobalPos ( rcPos );

		rcTexPos.sizeY = rcTexPos.sizeY * m_fPercent;
		rcTexPos.top = rcTexPos.bottom - rcTexPos.sizeY;
		m_pOverImageRed->SetTexturePos ( rcTexPos );
	}

	if  ( m_pOverImageYellow && m_pOverImageYellowDummy )
	{
		UIRECT rcPos = m_pOverImageYellowDummy->GetGlobalPos();
		UIRECT rcTexPos = m_pOverImageYellowDummy->GetTexturePos ();

		rcPos.sizeY = rcPos.sizeY * m_fPercent;
		rcPos.top = rcPos.bottom - rcPos.sizeY;
		m_pOverImageYellow->SetGlobalPos ( rcPos );

		rcTexPos.sizeY = rcTexPos.sizeY * m_fPercent;
		rcTexPos.top = rcTexPos.bottom - rcTexPos.sizeY;
		m_pOverImageYellow->SetTexturePos ( rcTexPos );
	}

	if  ( m_pOverImageGreen && m_pOverImageGreenDummy )
	{
		UIRECT rcPos = m_pOverImageGreenDummy->GetGlobalPos();
		UIRECT rcTexPos = m_pOverImageGreenDummy->GetTexturePos ();

		rcPos.sizeY = rcPos.sizeY * m_fPercent;
		rcPos.top = rcPos.bottom - rcPos.sizeY;
		m_pOverImageGreen->SetGlobalPos ( rcPos );

		rcTexPos.sizeY = rcTexPos.sizeY * m_fPercent;
		rcTexPos.top = rcTexPos.bottom - rcTexPos.sizeY;
		m_pOverImageGreen->SetTexturePos ( rcTexPos );
	}

	m_pOverImageRed->SetVisibleSingle( m_fPercent >= fZERO_PERCENT && m_fPercent < fTHIRTY_PERCENT );
	m_pOverImageYellow->SetVisibleSingle( m_fPercent >= fTHIRTY_PERCENT && m_fPercent < fSIXTY_PERCENT );
	m_pOverImageGreen->SetVisibleSingle( m_fPercent >= fSIXTY_PERCENT && m_fPercent <= fHUNDRED_PERCENT );
}

void CBasicMiniBarGauge::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	UpdateProgress ();
}

void CBasicMiniBarGauge::CreateSubControl ()
{
	m_pBackGround = new CUIControl;
	m_pBackGround->CreateSub ( this, "MINIBAR_GAUGE_BACK" );
	m_pBackGround->SetVisibleSingle( TRUE );
	RegisterControl ( m_pBackGround );

	m_pOverImageRed = new CUIControl;
	m_pOverImageRed->CreateSub ( this, "MINIBAR_GAUGE_RED" );
	m_pOverImageRed->SetVisibleSingle( FALSE );
	RegisterControl ( m_pOverImageRed );

	m_pOverImageYellow = new CUIControl;
	m_pOverImageYellow->CreateSub ( this, "MINIBAR_GAUGE_YELLOW" );
	m_pOverImageYellow->SetVisibleSingle( FALSE );
	RegisterControl ( m_pOverImageYellow );

	m_pOverImageGreen = new CUIControl;
	m_pOverImageGreen->CreateSub ( this, "MINIBAR_GAUGE_GREEN" );
	m_pOverImageGreen->SetVisibleSingle( FALSE );
	RegisterControl ( m_pOverImageGreen );

	m_pOverImageRedDummy = new CUIControl;
	m_pOverImageRedDummy->CreateSub ( this, "MINIBAR_GAUGE_RED" );
	m_pOverImageRedDummy->SetVisibleSingle( FALSE );
	RegisterControl ( m_pOverImageRedDummy );

	m_pOverImageYellowDummy = new CUIControl;
	m_pOverImageYellowDummy->CreateSub ( this, "MINIBAR_GAUGE_YELLOW" );
	m_pOverImageYellowDummy->SetVisibleSingle( FALSE );
	RegisterControl ( m_pOverImageYellowDummy );

	m_pOverImageGreenDummy = new CUIControl;
	m_pOverImageGreenDummy->CreateSub ( this, "MINIBAR_GAUGE_GREEN" );
	m_pOverImageGreenDummy->SetVisibleSingle( FALSE );
	RegisterControl ( m_pOverImageGreenDummy );
}

void CBasicMiniBarGauge::AlignSubControl ( const UIRECT& rcParentOldPos, const UIRECT& rcParentNewPos )
{
	CUIGroup::AlignSubControl ( rcParentOldPos, rcParentNewPos );

	SetPercent ( GetPercent () );
	UpdateProgress ();
}

void  CBasicMiniBarGauge::SetGlobalPos ( const D3DXVECTOR2& vPos )
{
	CUIGroup::SetGlobalPos ( vPos );
	
	SetPercent ( GetPercent () );
	UpdateProgress ();
}

void CBasicMiniBarGauge::SetGlobalPos ( const UIRECT& rcPos )
{
	CUIGroup::SetGlobalPos ( rcPos );

	SetPercent ( GetPercent () );
	UpdateProgress ();
}