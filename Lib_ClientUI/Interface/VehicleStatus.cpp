#include "StdAfx.h"
#include "VehicleStatus.h"
#include "ItemImage.h"
#include "InnerInterface.h"
#include "GLItemMan.h"
#include "UITextControl.h"
#include "GLGaeaClient.h"
#include "BasicMiniBarGauge.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CVehicleStatus::CVehicleStatus () 
	: m_pVehicleImage(NULL)
	, m_pGauge(NULL)
	, m_pDummyControl(NULL)
	, m_sNativeID(false)
	, m_strName("")
	, m_fTotal(0.0f)
	, m_fRemain(0.0f)
	, m_fRemainBack(0.1f)
	, m_fCheckTimer(0.0f)
{
}

CVehicleStatus::~CVehicleStatus ()
{
}

void  CVehicleStatus::CreateSubControl ()
{
	CItemImage* pItemImage = new CItemImage;
	pItemImage->CreateSub ( this, "VEHICLE_STATUS_ITEM_IMAGE" );
	pItemImage->CreateSubControl ();
	pItemImage->SetUseRender ( TRUE );
	pItemImage->SetVisibleSingle ( FALSE );
	RegisterControl ( pItemImage );
    m_pVehicleImage = pItemImage;

	CUIControl* pDummyControl = new CUIControl;
	pDummyControl->CreateSub ( this, "VEHICLE_STATUS_NO_POWER");
	RegisterControl ( pDummyControl );
	m_pDummyControl = pDummyControl;
	m_pDummyControl->SetVisibleSingle( FALSE );

	m_pGauge = new CBasicMiniBarGauge;
	m_pGauge->CreateSub( this, "VEHICLE_STATUS_POWER_GAUGE", UI_FLAG_DEFAULT, VEHICLE_POWER_GAUGE );
	m_pGauge->CreateSubControl();
	m_pGauge->SetVisibleSingle( TRUE );
	RegisterControl ( m_pGauge );
}

void CVehicleStatus::SetItem ( const SITEMCUSTOM& sItemCustom )
{
	if ( m_pVehicleImage )
	{
		m_pVehicleImage->SetItem ( sItemCustom );
		m_pVehicleImage->SetVisibleSingle ( TRUE );
	}
}

void CVehicleStatus::ResetItem ()
{
	if ( m_pVehicleImage )
	{
		m_pVehicleImage->SetVisibleSingle ( FALSE );
		m_pVehicleImage->ResetItem ();
	}
}

void CVehicleStatus::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
	
	SITEMCUSTOM sItem =  GLGaeaClient::GetInstance().GetCharacter()->m_sVehicle.m_sVehicleID;
	if ( m_sNativeID != sItem.sNativeID ) 
	{
		m_sNativeID = sItem.sNativeID;
		ResetItem ();
		m_strName = "";
		m_fTotal = 0.0f;

		SITEM* pItem = GLItemMan::GetInstance().GetItem ( m_sNativeID );
		if ( pItem && pItem->sBasicOp.emItemType == ITEM_VEHICLE )
		{
			m_strName = pItem->GetName();	
			SetItem ( sItem );	

			PGLVEHICLE pVehicle = GLCONST_VEHICLE::GetVehicleData ( pItem->sVehicle.emVehicleType );
			if ( pVehicle )
				m_fTotal = float( pVehicle->m_nFull );
		}
	}

	m_fRemain = float( GLGaeaClient::GetInstance().GetCharacter()->m_sVehicle.m_nFull );

	if ( m_fRemainBack != m_fRemain )
	{
		m_fRemainBack = m_fRemain;

		float fPercent = m_fRemain / m_fTotal;

		if ( m_pGauge )
			m_pGauge->SetPercent( fPercent );

		if ( m_fRemain <= 100.0f ) 
		{
			if ( !m_pDummyControl->IsVisible() )
				m_pDummyControl->SetVisibleSingle( TRUE );
		}else{
			if ( m_pDummyControl->IsVisible() )
				m_pDummyControl->SetVisibleSingle( FALSE );
		}
	}

	if ( CHECK_MOUSE_IN ( GetMessageEx () ) )
	{
		CString strTemp;
		float fPercent = m_fRemain / m_fTotal;
		strTemp.Format("%s Power:%g Percent", m_strName.GetString(), fPercent * 100 );
		CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO ( strTemp.GetString() , NS_UITEXTCOLOR::WHITE );
	}

	if ( CHECK_MOUSEIN_LBUPLIKE ( GetMessageEx () ) )
	{
		if( GLGaeaClient::GetInstance().GetCharacter()->m_sVehicle.IsActiveValue() )
		{
			if ( GLTradeClient::GetInstance().Valid() ) return;
			
			if ( CInnerInterface::GetInstance().IsVisibleGroup( VEHICLE_WINDOW ) )		
				CInnerInterface::GetInstance().HideGroup ( VEHICLE_WINDOW );
			else	
				CInnerInterface::GetInstance().ShowGroupFocus( VEHICLE_WINDOW );	
		}
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

