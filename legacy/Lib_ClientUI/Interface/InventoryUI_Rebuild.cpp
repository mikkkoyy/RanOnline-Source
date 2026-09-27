#include "StdAfx.h"
#include "InventoryUI_Rebuild.h"

#include "D3DFont.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "BasicTextBox.h"
#include "BasicButton.h"
#include "ModalCallerID.h"
#include "ModalWindow.h"
#include "UITextControl.h"
#include "GameTextControl.h"
#include "InventoryUI_RebuildInven.h"
#include "InnerInterface.h"
#include "GLItemMan.h"
#include "GLGaeaClient.h"

#include "BasicLineBox.h"
#include "BasicTextButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//--------------------------------------------------------------------
// InventoryUI_Rebuild
//--------------------------------------------------------------------
CInventoryUI_Rebuild::CInventoryUI_Rebuild()	// ITEMREBUILD_MARK
	: m_pPage( NULL )
	, m_pMoneyTextBox( NULL )
	, m_nONE_VIEW_SLOT( 4 )
	, m_pInvenButton( NULL )
	, m_pPointTextBox( NULL )
{
}

CInventoryUI_Rebuild::~CInventoryUI_Rebuild()
{
}

VOID CInventoryUI_Rebuild::CreateSubControl()
{
	m_pBackGround = new CUIControl;
	m_pBackGround->CreateSub ( this, "INVENTORY_WINDOW_BACKGROUND_REP", UI_FLAG_DEFAULT );	
	m_pBackGround->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pBackGround );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "INVENTORY_WINDOW_MONEY_REGION" );
	RegisterControl ( pBasicLineBox );

		pBasicLineBox = new CBasicLineBox;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxQuestList( "INVENTORY_WINDOW_POINT_REGION" );
		RegisterControl ( pBasicLineBox );

		pBasicLineBox = new CBasicLineBox;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxQuestList( "INVENTORY_WINDOW_ITEM_REGION_BACK" );
		RegisterControl ( pBasicLineBox );

	m_pPage = new CInventoryUI_RebuildInven;
	m_pPage->CreateSub( this, "INVENTORY_WINDOW_ITEM_REGION", UI_FLAG_YSIZE, REBUILDINVENTORY_PAGE );
	m_pPage->CreateSubControl();
	RegisterControl( m_pPage );

	m_pInvenButton = CreateTextButton23 ( "INVENTORY_WINDOW_INVEN_TAP", REBUILDINVENTORY_INVEN_BUTTON, (char*)ID2GAMEWORD ( "INVENTORY_TAP_NAME", 0 ) );
	m_pInvenButton->SetFlip( TRUE );

	CBasicButton* pMoneyButton = NULL;
	pMoneyButton = CreateFlipButton ( "INVENTORY_MONEY_BUTTON_RENEW", "INVENTORY_MONEY_BUTTON_RENEW_F", REBUILDINVENTORY_MONEY_BUTTON, CBasicButton::CLICK_FLIP );
	pMoneyButton->CreateMouseOver ( "INVENTORY_MONEY_BUTTON_RENEW_F" );
	pMoneyButton->SetAlignFlag ( UI_FLAG_BOTTOM );
	pMoneyButton->SetUseDynamic ( TRUE );

	CBasicButton* pPointButton = NULL;
	pPointButton = CreateFlipButton ( "INVENTORY_POINT_BUTTON_RENEW", "INVENTORY_POINT_BUTTON_RENEW_F", REBUILDINVENTORY_POINT_BUTTON, CBasicButton::CLICK_FLIP );
	pPointButton->CreateMouseOver ( "INVENTORY_POINT_BUTTON_RENEW_F" );
	pPointButton->SetAlignFlag ( UI_FLAG_BOTTOM );
	pPointButton->SetUseDynamic ( TRUE );

	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont( _DEFAULT_FONT, 8, D3DFONT_SHADOW | D3DFONT_ASCII );

	m_pMoneyTextBox = CreateStaticControl ( "INVENTORY_MONEY_TEXT_RENEW", pFont8, NS_UITEXTCOLOR::DEFAULT, TEXT_ALIGN_LEFT );
	m_pMoneyTextBox->SetAlignFlag ( UI_FLAG_BOTTOM );
	m_pPointTextBox = CreateStaticControl ( "INVENTORY_POINT_TEXT_RENEW", pFont8, NS_UITEXTCOLOR::DEFAULT, TEXT_ALIGN_LEFT );
	m_pPointTextBox->SetAlignFlag ( UI_FLAG_BOTTOM );
}

CBasicTextButton*  CInventoryUI_Rebuild::CreateTextButton23 ( const char* szButton, UIGUID ControlID, const char* szText )
{
	const int nBUTTONSIZE = CBasicTextButton::SIZE23;
	CBasicTextButton* pTextButton = new CBasicTextButton;
	pTextButton->CreateSub ( this, "BASIC_TEXT_BUTTON23", UI_FLAG_XSIZE|UI_FLAG_YSIZE, ControlID );
	pTextButton->CreateBaseButton ( szButton, nBUTTONSIZE, CBasicButton::RADIO_FLIP, szText, 0, NS_UITEXTCOLOR::BLACK );
	RegisterControl ( pTextButton );
	return pTextButton;
}


VOID CInventoryUI_Rebuild::Update( INT x, INT y, BYTE LB, BYTE MB, BYTE RB, INT nScroll, FLOAT fElapsedTime, BOOL bFirstControl )
{
	CUIWindowEx::Update( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	GLCHARLOGIC& ref_CharData = GLGaeaClient::GetInstance().GetCharacterLogic();

	m_pPage->ResetAllFlipItem();
	if( ref_CharData.m_sRebuildItem.VALID() /* || ref_CharData.m_sRebuildItemSealed.VALID()*/ )
	{
		if( ref_CharData.m_sRebuildItem.VALID() )		m_pPage->UpdateFlipItem( ref_CharData.m_sRebuildItem );
		//if( ref_CharData.m_sRebuildItemSealed.VALID() )	m_pPage->UpdateFlipItem( ref_CharData.m_sRebuildItemSealed );
	}

	{
		CString strTemp = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( ref_CharData.m_lnMoney, 3, "," );
		m_pMoneyTextBox->SetOneLineText( strTemp );
	
		//strTemp = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( ref_CharData.m_dwPremiumPoints, 3, "," 
		strTemp = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( 0, 3, "," );
		m_pPointTextBox->SetOneLineText( strTemp );
	}

	{
		GLInventory& ref_Inventory = ref_CharData.m_cInventory;
		m_pPage->LoadItemPage( ref_Inventory );
	}
}

VOID CInventoryUI_Rebuild::TranslateUIMessage( UIGUID ControlID, DWORD dwMsg )
{
	switch( ControlID )
	{
	case REBUILDINVENTORY_PAGE:
		if( CHECK_MOUSE_IN( dwMsg ) )
		{
			INT nPosX, nPosY;
			m_pPage->GetItemIndex( &nPosX, &nPosY );

			CDebugSet::ToView( 1, 1, "¿Œ∫• : %d %d", nPosX, nPosY );

			if( nPosX < 0 || nPosY < 0 )
				return;

			SINVENITEM& sInvenItem = m_pPage->GetItem( nPosX, nPosY );
			if( sInvenItem.sItemCustom.sNativeID == NATIVEID_NULL() )	return;

			if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LCONTROL ) & DXKEY_DOWNED )
			{
						CInnerInterface::GetInstance().SHOW_ITEM_INFO ( sInvenItem.sItemCustom, FALSE, FALSE, FALSE, sInvenItem.wPosX, sInvenItem.wPosY, SNATIVEID(), FALSE, 0, FALSE, FALSE, FALSE, TRUE/*, ITEMMALLDATA(), TRUE*/ );
					}
					else CInnerInterface::GetInstance().SHOW_ITEM_INFO ( sInvenItem.sItemCustom, FALSE, FALSE, FALSE, sInvenItem.wPosX, sInvenItem.wPosY );

			if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LMENU  ) & DXKEY_DOWNED )
			{
				if ( dwMsg & UIMSG_LB_UP )
				{
					//item preview disable
					CInnerInterface::GetInstance().PreviewItem( sInvenItem.sItemCustom );
					return;
				}
			}

			/*box contents, Juver, 2017/08/30 */
			if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LMENU  ) & DXKEY_DOWNED )
			{
				if ( dwMsg & UIMSG_RB_UP )
				{
					CInnerInterface::GetInstance().ShowBoxContents( sInvenItem.sItemCustom.sNativeID );
					return;
				}
			}

			if( dwMsg & UIMSG_LB_UP )
				GLGaeaClient::GetInstance().GetCharacter()->ReqInvenTo( nPosX, nPosY );
		}
		break;

	//case REBUILDINVENTORY_MONEY_BUTTON:
	//	if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
	//	{
	//		if( GLGaeaClient::GetInstance().GetCharacter()->ValidRebuildOpen() )
	//			DoModal( ID2GAMEINTEXT("REBUILD_MONEY_INPUT"), MODAL_INPUT, EDITBOX_MONEY, MODAL_ITEMREBUILD_MONEY );
	//	}
	//	break;

	case ET_CONTROL_BUTTON:
		if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			CInnerInterface::GetInstance().CloseItemRebuildWindow();
		break;
	}

	CUIWindowEx::TranslateUIMessage( ControlID, dwMsg );
}

HRESULT CInventoryUI_Rebuild::RestoreDeviceObjects( LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = CUIWindowEx::RestoreDeviceObjects( pd3dDevice );
	if( FAILED( hr ) )
		return hr;

	const LONG lResolution = CInnerInterface::GetInstance().GetResolution();
	WORD X_RES = HIWORD( lResolution );
	WORD Y_RES = LOWORD( lResolution );

	const UIRECT& rcWindow = GetGlobalPos();

	D3DXVECTOR2 vPos;
	vPos.x = ( (X_RES) / 2.0f );
	vPos.y = ( Y_RES - rcWindow.sizeY ) / 2.0f;
	SetGlobalPos( vPos );

	return S_OK;
}

VOID CInventoryUI_Rebuild::SetOneViewSlot( const INT& nONE_VIEW_SLOT )
{
	m_pPage->SetOneViewSlot( nONE_VIEW_SLOT );
	m_nONE_VIEW_SLOT = nONE_VIEW_SLOT;
}