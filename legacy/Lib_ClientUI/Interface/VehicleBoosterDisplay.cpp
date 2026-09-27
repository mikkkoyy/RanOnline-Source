#include "StdAfx.h"
#include "VehicleBoosterDisplay.h"
#include "BasicProgressBar.h"

#include "UITextControl.h"
#include "GameTextControl.h"

#include "InnerInterface.h"
#include "GLGaeaClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CVehicleBoosterDisplay::CVehicleBoosterDisplay ()
	: m_pProgress(NULL)
	, m_pButtonOff(NULL)
	, m_pButtonOn(NULL)
	, m_fCheckTimer(0.0f)
{
}

CVehicleBoosterDisplay::~CVehicleBoosterDisplay ()
{
}

void CVehicleBoosterDisplay::CreateSubControl ()
{
	m_pProgress = new CBasicProgressBar;
	m_pProgress->CreateSub ( this, "RN_BOOSTER_GAGE_BACK", UI_FLAG_DEFAULT, VEHICLE_BOOSTER_PROGRESS );
	m_pProgress->CreateOverImage ( "RN_BOOSTER_GAGE" );	
	RegisterControl ( m_pProgress );
	
	m_pButtonOff = new CUIControl;
	m_pButtonOff->CreateSub ( this, "RN_BOOSTER_OFF_BUTTON", UI_FLAG_DEFAULT, VEHICLE_BOOSTER_BUTTON_OFF );
	m_pButtonOff->SetVisibleSingle ( FALSE );
	RegisterControl ( m_pButtonOff );

	m_pButtonOn = new CUIControl;
	m_pButtonOn->CreateSub ( this, "RN_BOOSTER_ON_BUTTON", UI_FLAG_DEFAULT, VEHICLE_BOOSTER_BUTTON_ON );
	m_pButtonOn->SetVisibleSingle ( FALSE );
	RegisterControl (m_pButtonOn );
}

void CVehicleBoosterDisplay::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	float fTime = GLGaeaClient::GetInstance().GetCharacter ()->m_fBoosterTimer / GLCONST_CHAR::fVehicleBoosterChargeTime;
	bool bRunning = GLGaeaClient::GetInstance().GetCharacter ()->m_bBoosterStart;
	if ( bRunning )
		fTime = GLGaeaClient::GetInstance().GetCharacter ()->m_fBoosterTimer / GLCONST_CHAR::fVehicleBoosterBoostTime;

	if ( m_pButtonOff )
		m_pButtonOff->SetVisibleSingle( !bRunning  );

	if ( m_pButtonOn )
		m_pButtonOn->SetVisibleSingle( bRunning  );
		
	if ( m_pProgress )
		m_pProgress->SetPercent( fTime );

	if ( CHECK_MOUSE_IN ( GetMessageEx () ) )
	{
		CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEINTEXT( "BIKE_BOOSTER_EXPLAIN" ), NS_UITEXTCOLOR::GOLD );
	}

	m_fCheckTimer += fElapsedTime;
	if ( m_fCheckTimer >= 2.0f )
	{
		if( !GLGaeaClient::GetInstance().GetCharacter()->m_sVehicle.IsActiveValue() )
		{
			CInnerInterface::GetInstance().HideGroup ( GetWndID() );
		}
	}
}
