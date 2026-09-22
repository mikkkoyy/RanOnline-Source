#include "StdAfx.h"
#include "./VehicleWindow.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "d3dfont.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicProgressBar.h"
#include "BasicButton.h"
#include "ItemImage.h"
#include "ItemMove.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include "GLItemMan.h"
#include "InnerInterface.h"
#include "GLGaeaClient.h"
#include "RANPARAM.h"

#include "VehicleWindowRender.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CVehicleWindow::CVehicleWindow ()
	: m_pVehicleName(NULL)
	, m_pVehicleType(NULL)
	, m_pMouseOver(NULL)
	, m_pBatteryText(NULL)
	, m_pBatteryStatic(NULL)
	, m_pSpeedText(NULL)
	, m_pBattery(NULL)
	, m_pRender(NULL)
	, m_pButtonLeft(NULL)
	, m_pButtonRight(NULL)
	, m_pButtonRun(NULL)
	, m_pRunText(NULL)
	, m_RollOverID(NO_ID)
	, m_nBatteryBack(-1)
	, m_fSpeedBack(0.1f)

	/*vehicle booster system, Juver, 2017/08/11 */
	, m_pBoosterText(NULL) 
	, m_pBoosterImage(NULL)
	, m_pBoosterOver(NULL)
	, m_bBoosterBack(false)

	/* vehicle no accessory, Juver, 2018/02/14 */
	, m_pAcceText(NULL)
{
	SecureZeroMemory( m_pSlotDisplay, sizeof(m_pSlotDisplay) );
	SecureZeroMemory( m_pSlotImage, sizeof(m_pSlotImage) );
}

CVehicleWindow::~CVehicleWindow ()
{
}

void CVehicleWindow::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	/*vehicle booster system, Juver, 2017/08/11 */
	m_pBoosterOver = new CUIControl;
	m_pBoosterOver->CreateSub ( this, "RN_VEHICLE_BOOST_ICON" );
	m_pBoosterOver->SetVisibleSingle( FALSE );
	RegisterControl ( m_pBoosterOver );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxWhite ( "RN_VEHICLE_WINDOW_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RN_VEHICLE_WINDOW_LINE_RENDER" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RN_VEHICLE_WINDOW_LINE_INFO" );
	RegisterControl ( pBasicLineBox );

	CString strVehicleSlot[SLOT_IMAGE_SIZE] = 
	{
		"RN_VEHICLE_SLOT_SKIN_IMAGE_TYPE0",
		"RN_VEHICLE_SLOT_PARTS_IMAGE_TYPE0",
		"RN_VEHICLE_SLOT_PARTS_IMAGE_TYPE1",
		"RN_VEHICLE_SLOT_PARTS_IMAGE_TYPE2",
		"RN_VEHICLE_SLOT_PARTS_IMAGE_TYPE3",
		"RN_VEHICLE_SLOT_PARTS_IMAGE_TYPE4",
		"RN_VEHICLE_SLOT_PARTS_IMAGE_TYPE5",
	};

	CString strVehicleSlotOver[SLOT_IMAGE_SIZE] = 
	{
		"RN_VEHICLE_SLOT_SKIN_OVER_IMAGE_TYPE0",
		"RN_VEHICLE_SLOT_PARTS_OVER_IMAGE_TYPE0",
		"RN_VEHICLE_SLOT_PARTS_OVER_IMAGE_TYPE1",
		"RN_VEHICLE_SLOT_PARTS_OVER_IMAGE_TYPE2",
		"RN_VEHICLE_SLOT_PARTS_OVER_IMAGE_TYPE3",
		"RN_VEHICLE_SLOT_PARTS_OVER_IMAGE_TYPE4",
		"RN_VEHICLE_SLOT_PARTS_OVER_IMAGE_TYPE5",
	};

	for ( int i = 0; i < SLOT_IMAGE_SIZE; i++ )
	{
		m_pSlotImage[i] = new CItemImage;
		m_pSlotImage[i]->CreateSub ( this, strVehicleSlot[i].GetString(), UI_FLAG_DEFAULT, SLOT_SKIN_IMAGE_TYPE0 + i );
		m_pSlotImage[i]->CreateSubControl ();
		RegisterControl ( m_pSlotImage[i] );


		m_pSlotDisplay[i] = new CUIControl;
		m_pSlotDisplay[i]->CreateSub ( this, strVehicleSlotOver[i].GetString() );
		m_pSlotDisplay[i]->SetVisibleSingle( FALSE );
		RegisterControl ( m_pSlotDisplay[i] );
	}

	m_pMouseOver = new CUIControl;
	m_pMouseOver->CreateSub ( this, "RN_VEHICLE_ITEM_MOUSE_OVER" );
	m_pMouseOver->SetVisibleSingle( FALSE );
	RegisterControl ( m_pMouseOver );
	
	m_pVehicleType = new CBasicTextBox;
	m_pVehicleType->CreateSub ( this, "RN_VEHICLE_TYPE_TEXT" );
	m_pVehicleType->SetFont ( pFont9 );
	RegisterControl ( m_pVehicleType );

	m_pVehicleName = new CBasicTextBox;
	m_pVehicleName->CreateSub ( this, "RN_VEHICLE_NAME_TEXT" );
	m_pVehicleName->SetFont ( pFont9 );
	RegisterControl ( m_pVehicleName );

	CBasicProgressBar* pBattery = new CBasicProgressBar;
	pBattery->CreateSub ( this, "RN_VEHICLE_BATTERY_PROGRESSBAR" );
	pBattery->CreateOverImage ( "RN_VEHICLE_BATTERY_PROGRESSBAR_OVERIMAGE" );
	RegisterControl ( pBattery );
	m_pBattery = pBattery;

	m_pBatteryStatic = new CBasicTextBox;
	m_pBatteryStatic->CreateSub ( this, "RN_VEHICLE_BATTERY_STATIC" );
	m_pBatteryStatic->SetFont ( pFont9 );
	m_pBatteryStatic->SetText( ID2GAMEWORD( "VEHICLE_TEXT_STATIC", 2), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pBatteryStatic );

	m_pBatteryText = new CBasicTextBox;
	m_pBatteryText->CreateSub( this, "RN_VEHICLE_BATTERY_TEXT", UI_FLAG_DEFAULT, VEHICLE_BATTERY_TEXT );
	m_pBatteryText->SetFont( pFont9 );
	m_pBatteryText->SetTextAlign ( TEXT_ALIGN_CENTER_X );
	m_pBatteryText->SetText("1");
	RegisterControl ( m_pBatteryText );

	m_pSpeedText = new CBasicTextBox;
	m_pSpeedText->CreateSub ( this, "RN_VEHICLE_SPEED_TEXT" );
	m_pSpeedText->SetFont ( pFont9 );
	RegisterControl ( m_pSpeedText );

	m_pButtonLeft = new CBasicButton;
	m_pButtonLeft->CreateSub ( this, "RN_VEHICLE_ARROW_TURN_LEFT_DEFAULT", UI_FLAG_DEFAULT, VEHICLE_RENDER_BUTTON_LEFT );
	m_pButtonLeft->CreateFlip ( "RN_VEHICLE_ARROW_TURN_LEFT_CLICK", CBasicButton::CLICK_FLIP );
	m_pButtonLeft->CreateMouseOver ( "RN_VEHICLE_ARROW_TURN_LEFT_OVER" );
	m_pButtonLeft->SetVisibleSingle( TRUE );
	RegisterControl ( m_pButtonLeft );

	m_pButtonRight = new CBasicButton;
	m_pButtonRight->CreateSub ( this, "RN_VEHICLE_ARROW_TURN_RIGHT_DEFAULT", UI_FLAG_DEFAULT, VEHICLE_RENDER_BUTTON_RIGHT );
	m_pButtonRight->CreateFlip ( "RN_VEHICLE_ARROW_TURN_RIGHT_CLICK", CBasicButton::CLICK_FLIP );
	m_pButtonRight->CreateMouseOver ( "RN_VEHICLE_ARROW_TURN_RIGHT_OVER" );
	m_pButtonRight->SetVisibleSingle( TRUE );
	RegisterControl ( m_pButtonRight );

	m_pButtonRun = new CBasicButton;
	m_pButtonRun->CreateSub ( this, "RN_VEHICLE_RUN_BUTTON", UI_FLAG_DEFAULT, VEHICLE_RENDER_BUTTON_RUN );
	m_pButtonRun->CreateFlip ( "RN_VEHICLE_RUN_BUTTON_FLIP", CBasicButton::RADIO_FLIP );
	m_pButtonRun->SetControlNameEx ( "RN_VEHICLE_RUN_BUTTON" );
	m_pButtonRun->SetFlip( RANPARAM::bVehicleWindowRun );
	RegisterControl ( m_pButtonRun );

	m_pRunText = new CBasicTextBox;
	m_pRunText->CreateSub ( this, "RN_VEHICLE_RUN_TEXT" );
	m_pRunText->SetFont ( pFont9 );
	m_pRunText->SetText( ID2GAMEWORD( "VEHICLE_TEXT_STATIC", 7 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pRunText );

	/*vehicle booster system, Juver, 2017/08/11 */
	m_pBoosterText = new CBasicTextBox;
	m_pBoosterText->CreateSub ( this, "RN_VEHICLE_BOOSTER_TEXT" );
	m_pBoosterText->SetFont ( pFont9 );
	RegisterControl ( m_pBoosterText );

	m_pBoosterImage = new CUIControl;
	m_pBoosterImage->CreateSub ( this, "RN_VEHICLE_BOOST_ICON" );
	m_pBoosterImage->SetVisibleSingle( FALSE );
	RegisterControl ( m_pBoosterImage );

	/* vehicle no accessory, Juver, 2018/02/14 */
	m_pAcceText = new CBasicTextBox;
	m_pAcceText->CreateSub ( this, "RN_VEHICLE_ACCE_TEXT" );
	m_pAcceText->SetFont ( pFont9 );
	RegisterControl ( m_pAcceText );


	//render should be the very top control
	m_pRender = new CVehicleWindowRender;
	m_pRender->CreateSub ( this, "RN_VEHICLE_WINDOW_LINE_RENDER" );
	m_pRender->CreateSubControl ();
	RegisterControl ( m_pRender );
}

EMSUIT CVehicleWindow::IMAGE2EMSLOT ( UIGUID ControlID )
{
	switch ( ControlID )
	{	
	case SLOT_SKIN_IMAGE_TYPE0: return SUIT_VEHICLE_SKIN;
	case SLOT_PARTS_IMAGE_TYPE0: return SUIT_VEHICLE_PARTS_A;
	case SLOT_PARTS_IMAGE_TYPE1: return SUIT_VEHICLE_PARTS_B;
	case SLOT_PARTS_IMAGE_TYPE2: return SUIT_VEHICLE_PARTS_C;
	case SLOT_PARTS_IMAGE_TYPE3: return SUIT_VEHICLE_PARTS_D;
	case SLOT_PARTS_IMAGE_TYPE4: return SUIT_VEHICLE_PARTS_E;
	case SLOT_PARTS_IMAGE_TYPE5: return SUIT_VEHICLE_PARTS_F;
	}

	GASSERT ( 0 && "CVehicleWindow::IMAGE2EMSLOT(), 범위를 벗어납니다." );
	return SUIT_NSIZE;
}

void CVehicleWindow::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( pCharacter )
	{
		if( !pCharacter->m_sVehicle.IsActiveValue() )
		{
			CInnerInterface::GetInstance().HideGroup( GetWndID() );
			return;
		}

		if( m_pMouseOver ) m_pMouseOver->SetVisibleSingle ( FALSE );

		CItemMove* pItemMove = CInnerInterface::GetInstance().GetItemMove ();
		SNATIVEID sHOLD_ITEM_ID(false);
		if ( pItemMove ) sHOLD_ITEM_ID = pItemMove->GetItem();

		for( int i=0; i<SLOT_IMAGE_SIZE; ++i )
		{
			SITEMCUSTOM & sItemCustomOld = m_ItemCustomArray[i];
			SITEMCUSTOM & sItemCustom = pCharacter->m_sVehicle.m_PutOnItems[i];

			if ( sItemCustom != sItemCustomOld )
			{		
				if ( sItemCustom.sNativeID != NATIVEID_NULL() )
					LoadItem ( i, sItemCustom );
				else
					UnLoadItem ( i );
			}

			m_pSlotDisplay[i]->SetVisibleSingle ( FALSE );

			if( sHOLD_ITEM_ID != NATIVEID_NULL() )
			{
				if( pCharacter->m_sVehicle.CheckSlotItem( sHOLD_ITEM_ID, IMAGE2EMSLOT( i + SLOT_SKIN_IMAGE_TYPE0 ) ) )
				{
					const UIRECT& rcImagePos = m_pSlotImage[i]->GetGlobalPos ();
					D3DXVECTOR2 vPos( rcImagePos.left, rcImagePos.top );

					m_pSlotDisplay[i]->SetVisibleSingle( TRUE );
				}
			}

			if ( m_pRender )
			{
				if ( sItemCustom != m_pRender->m_sVehicleInfo.m_PutOnItems[i] )
				{		
					m_pRender->m_sVehicleInfo.m_PutOnItems[i] = sItemCustom;
					m_pRender->m_bUpdateSuit = true;
				}
			}
		}

		if ( m_pRender )
		{
			if ( m_pRender->m_sVehicleID != pCharacter->m_sVehicle.m_sVehicleID )
			{
				LoadData();
			}
		}

		int nFull = pCharacter->m_sVehicle.m_nFull;

		if ( m_nBatteryBack != nFull )
		{
			m_nBatteryBack = nFull;

			if ( m_pBatteryText && m_pBattery )
			{
				CString strTemp;
				strTemp.Format( "%.2f%%", nFull/10.0f );
				m_pBatteryText->SetOneLineText( strTemp, NS_UITEXTCOLOR::WHITE );
				m_pBattery->SetPercent( nFull/1000.0f ); 
			}
		}

		float fItemSpeed = pCharacter->m_PutOnItems[SLOT_VEHICLE].GETMOVESPEED();
		float fSpeed = fItemSpeed + pCharacter->m_sVehicle.m_fSpeedVol;

		if ( m_fSpeedBack != fSpeed )
		{
			m_fSpeedBack = fSpeed;

			if ( m_pSpeedText )
			{
				CString strText;
				strText.Format( "%s %2.2f", ID2GAMEWORD( "VEHICLE_TEXT_STATIC", 6 ), fSpeed );
				m_pSpeedText->SetOneLineText( strText.GetString(), NS_UITEXTCOLOR::WHITE );
			}
		}

		/*vehicle booster system, Juver, 2017/08/11 */
		if ( m_bBoosterBack != pCharacter->m_sVehicle.m_bBooster )
		{
			m_bBoosterBack = pCharacter->m_sVehicle.m_bBooster;
			LoadBoosterInfo( pCharacter->m_sVehicle.m_emTYPE, pCharacter->m_sVehicle.m_bBooster );
		}

	}
	

	CUIWindowEx::Update( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CVehicleWindow::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIWindowEx::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case ET_CONTROL_TITLE:
	case ET_CONTROL_TITLE_F:
		{
			if ( (dwMsg & UIMSG_LB_DUP) && CHECK_MOUSE_IN ( dwMsg ) )
			{
				CInnerInterface::GetInstance().SetDefaultPosInterface( VEHICLE_WINDOW );
			}
		}
		break;

	case SLOT_SKIN_IMAGE_TYPE0:
	case SLOT_PARTS_IMAGE_TYPE0:
	case SLOT_PARTS_IMAGE_TYPE1:
	case SLOT_PARTS_IMAGE_TYPE2:
	case SLOT_PARTS_IMAGE_TYPE3:
	case SLOT_PARTS_IMAGE_TYPE4:
	case SLOT_PARTS_IMAGE_TYPE5:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{	
				int nIndex = ControlID - SLOT_SKIN_IMAGE_TYPE0;

				if ( !CInnerInterface::GetInstance().IsFirstItemSlot () ) return ;

				CItemMove* pItemMove = CInnerInterface::GetInstance().GetItemMove ();
				if ( !pItemMove )
				{
					GASSERT ( 0 && "CItemMove가 널입니다." );
					return ;
				}

				if ( pItemMove->GetItem () != NATIVEID_NULL () )
				{
					const UIRECT& rcSlotPos = m_pSlotImage[nIndex]->GetGlobalPos ();
					pItemMove->SetGlobalPos ( rcSlotPos );						

					AddMessageEx ( UIMSG_MOUSEIN_VEHICLE_WEARSLOT | UIMSG_TOTOPPARENT );
				}				

				if ( pItemMove->GetItem () == NATIVEID_NULL () && m_pSlotImage[nIndex]->GetItem () != NATIVEID_NULL () )
				{
					const UIRECT& rcImagePos = m_pSlotImage[nIndex]->GetGlobalPos ();
					m_pMouseOver->SetGlobalPos ( rcImagePos );
					m_pMouseOver->SetVisibleSingle ( TRUE );
				}

				if ( dwMsg & UIMSG_LB_UP )
				{
					GLGaeaClient::GetInstance().GetCharacter()->ReqVehicleChangeAccessory( IMAGE2EMSLOT(ControlID) );
				}

				if ( dwMsg & UIMSG_RB_UP )
				{
					GLGaeaClient::GetInstance().GetCharacter()->ReqVehicleRemoveSlotItem( IMAGE2EMSLOT(ControlID) );
				}

				if( m_ItemCustomArray[nIndex].sNativeID != NATIVEID_NULL () )
				{
					CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW ( m_ItemCustomArray[nIndex], FALSE, FALSE, FALSE,FALSE,FALSE, USHRT_MAX, USHRT_MAX );
				}
			}
		}
		break;

	case VEHICLE_RENDER_BUTTON_LEFT:
		{
			if ( CHECK_MOUSE_IN_LBDOWNLIKE ( dwMsg ) )
			{
				if ( m_pRender )	m_pRender->m_vRot.x -=0.1f;
			}
		}break;
	case VEHICLE_RENDER_BUTTON_RIGHT:
		{
			if ( CHECK_MOUSE_IN_LBDOWNLIKE ( dwMsg ) )
			{
				if ( m_pRender )	m_pRender->m_vRot.x +=0.1f;

			}
		}break;

	case VEHICLE_RENDER_BUTTON_RUN:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				RANPARAM::bVehicleWindowRun = !RANPARAM::bVehicleWindowRun;
				m_pButtonRun->SetFlip( RANPARAM::bVehicleWindowRun );

				if ( m_pRender )
					m_pRender->UpdateAnimation();
			}
		}break;
	}
}

void CVehicleWindow::SetVisibleSingle ( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle ( bVisible );

	if( bVisible )
	{
		LoadData();
	}
	else
	{
		if ( m_pRender )
			m_pRender->ClearRender();

		CInnerInterface::GetInstance().HideGroup( VEHICLE_WINDOW );
	}
}

void CVehicleWindow::LoadItem ( int nIndex, SITEMCUSTOM& ref_sItemCustom )
{
	m_ItemCustomArray[nIndex] = ref_sItemCustom;

	SITEM* pItemData = GLItemMan::GetInstance().GetItem ( ref_sItemCustom.sNativeID );
	m_pSlotImage[nIndex]->SetItem ( ref_sItemCustom );
}

void CVehicleWindow::UnLoadItem ( int nIndex )
{
	m_ItemCustomArray[nIndex].sNativeID = NATIVEID_NULL ();
	m_pSlotImage[nIndex]->ResetItem ();
}

void CVehicleWindow::LoadData()
{
	if ( m_pVehicleType )
		m_pVehicleType->ClearText();

	if ( m_pVehicleName )
		m_pVehicleName->ClearText();

	if ( m_pBoosterText )
		m_pBoosterText->ClearText();
	
	/* vehicle no accessory, Juver, 2018/02/14 */
	if ( m_pAcceText )
		m_pAcceText->ClearText();

	if ( m_pRender )
		m_pRender->ClearRender();

	m_nBatteryBack = -1;
	m_fSpeedBack = 0.1f;

	for( int i=0; i<VEHICLE_ACCETYPE_SIZE; i++ )
	{
		UnLoadItem(i);
	}


	GLCharacter* pChar = GLGaeaClient::GetInstance().GetCharacter();
	if ( !pChar ) return;
	SITEMCUSTOM& sItemCustom = pChar->m_PutOnItems[SLOT_VEHICLE];
	if ( sItemCustom.sNativeID == NATIVEID_NULL() ) return;
	if ( !pChar->m_sVehicle.m_bActiveValue ) return;


	SNATIVEID sNativeID = pChar->m_sVehicle.m_sVehicleID;
	SITEM* pItem = GLItemMan::GetInstance().GetItem( sNativeID );
	if ( !pItem ) return;

	VEHICLE_TYPE emTYPE = pChar->m_sVehicle.m_emTYPE;

	if( emTYPE == VEHICLE_TYPE_NONE )	return;

	if( m_pVehicleType )
	{
		CString strText;
		strText.Format( "%s %s", ID2GAMEWORD( "VEHICLE_TEXT_STATIC", 1 ),  COMMENT::VEHICLE_TYPE[emTYPE].c_str() );
		m_pVehicleType->SetOneLineText( strText.GetString(), NS_UITEXTCOLOR::WHITE );
	}

	if( m_pVehicleName )
	{
		CString strText;
		strText.Format( "%s %s", ID2GAMEWORD( "VEHICLE_TEXT_STATIC", 0 ), pItem->GetName() );
		m_pVehicleName->SetOneLineText( strText.GetString(), NS_UITEXTCOLOR::WHITE );
	}


	/*vehicle booster system, Juver, 2017/08/11 */
	m_bBoosterBack = pChar->m_sVehicle.m_bBooster;
	LoadBoosterInfo( emTYPE, pChar->m_sVehicle.m_bBooster );

	/* vehicle no accessory, Juver, 2018/02/14 */
	if ( m_pAcceText )
	{
		if ( pItem->sVehicle.bNoAcc )
			m_pAcceText->SetOneLineText( ID2GAMEWORD( "VEHICLE_TEXT_STATIC", 9 ), NS_UITEXTCOLOR::RED );
		else
			m_pAcceText->SetOneLineText( ID2GAMEWORD( "VEHICLE_TEXT_STATIC", 8 ), NS_UITEXTCOLOR::WHITE );
	}


	if ( m_pRender )
	{
		SVEHICLEITEMINFO sInfo;
		sInfo.m_emTYPE = emTYPE;
		sInfo.m_nFull = pChar->m_sVehicle.m_nFull;

		/*vehicle booster system, Juver, 2017/08/11 */
		sInfo.m_bBooster = pChar->m_sVehicle.m_bBooster;

		for( int i=0; i<VEHICLE_ACCETYPE_SIZE; i++ )
		{
			sInfo.m_PutOnItems[i] = pChar->m_sVehicle.m_PutOnItems[i];
		}

		/*bike color , Juver, 2017/11/16 */
		for ( int i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
		{
			sInfo.m_wColor[i] = pChar->m_sVehicle.m_wColor[i];
		}

		m_pRender->RenderVehicle( pItem->sBasicOp.sNativeID, sInfo );
	}
}

/*vehicle booster system, Juver, 2017/08/11 */
void CVehicleWindow::LoadBoosterInfo( VEHICLE_TYPE emType, bool bBooster )
{
	if ( m_pBoosterImage )	m_pBoosterImage->SetVisibleSingle( FALSE );
	if ( m_pBoosterOver )	m_pBoosterOver->SetVisibleSingle( FALSE );
	if ( m_pBoosterText )	m_pBoosterText->ClearText();

	if ( emType == VEHICLE_TYPE_BOARD )	return;

	if ( m_pBoosterText )
	{
		if ( bBooster )
		{
			int nIndex = m_pBoosterText->AddText( ID2GAMEWORD( "ITEM_VEHICLE_BOOST_LEARN", 1 ), NS_UITEXTCOLOR::WHITE );
			m_pBoosterText->AddString( nIndex, " " );
			m_pBoosterText->AddString( nIndex, ID2GAMEINTEXT( "BIKE_BOOST_EXPLAIN_OVER_ON" ), NS_UITEXTCOLOR::GOLD );
		}
		else
		{
			int nIndex = m_pBoosterText->AddText( ID2GAMEWORD( "ITEM_VEHICLE_BOOST_LEARN", 0 ), NS_UITEXTCOLOR::WHITE );
			m_pBoosterText->AddString( nIndex, " " );
			m_pBoosterText->AddString( nIndex, ID2GAMEINTEXT( "BIKE_BOOST_EXPLAIN_OVER_OFF" ), NS_UITEXTCOLOR::GOLD );
		}
	}

	if ( m_pBoosterImage )	m_pBoosterImage->SetVisibleSingle( bBooster );
	if ( m_pBoosterOver )	m_pBoosterOver->SetVisibleSingle( !bBooster );
}