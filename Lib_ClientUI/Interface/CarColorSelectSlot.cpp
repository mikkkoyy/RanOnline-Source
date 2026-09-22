#include "StdAfx.h"
#include "CarColorSelectSlot.h"

#include "GLGaeaClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCarColorSelectSlot::CCarColorSelectSlot ()
	: m_bPrimary(FALSE)
	, m_pColor(NULL)
	, m_pSelect(NULL)
{
}

CCarColorSelectSlot::~CCarColorSelectSlot ()
{
}

void CCarColorSelectSlot::CreateSubControl ( BOOL bPrimaryColor )
{
	m_bPrimary = bPrimaryColor;

	m_pColor = new CUIControl;
	m_pColor->CreateSub( this, "RAN_CAR_COLOR_USE" );
	m_pColor->SetUseRender( TRUE );
	m_pColor->SetVisibleSingle( FALSE );
	RegisterControl( m_pColor );

	m_pSelect = new CUIControl;
	m_pSelect->CreateSub( this, "RAN_CAR_COLOR_SELECT" );
	m_pSelect->SetVisibleSingle( FALSE );
	RegisterControl( m_pSelect );
}

void CCarColorSelectSlot::SetSelect( BOOL bSelect )
{
	if ( m_pSelect )
		m_pSelect->SetVisibleSingle( bSelect );
}

void CCarColorSelectSlot::Update ( int x ,int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( pCharacter && pCharacter->m_bEnableCarColor && m_pColor )
	{
		WORD wColor = pCharacter->m_wCarColorA;
		if ( !m_bPrimary )
			wColor = pCharacter->m_wCarColorB;

		const float fRGB_RATIO = 8.225806f;
		int m_nR = (wColor & 0x7c00) >> 10;
		int m_nG = (wColor& 0x3e0) >> 5;
		int m_nB = (wColor & 0x1f);

		m_pColor->SetVisibleSingle( TRUE );
		m_pColor->SetDiffuse( D3DCOLOR_ARGB( 255, int(m_nR * fRGB_RATIO), int(m_nG * fRGB_RATIO), int(m_nB * fRGB_RATIO) ) );	
		return;
	}

	m_pColor->SetVisibleSingle( FALSE );
}
