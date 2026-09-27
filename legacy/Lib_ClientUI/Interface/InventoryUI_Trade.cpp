#include "StdAfx.h"
#include "InventoryUI_Trade.h"
#include "InventoryUI_TradeInven.h"
#include "BasicButton.h"
#include "ModalCallerID.h"
#include "ModalWindow.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "UITextControl.h"
#include "GLGaeaClient.h"
#include "BasicTextBox.h"
#include "GameTextControl.h"
#include "InnerInterface.h"
#include "d3dfont.h"
#include "GLItemMan.h"

#include "BasicLineBox.h"
#include "BasicTextButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CInventoryUI_Trade::CInventoryUI_Trade () :
	m_pPage ( NULL ),
	m_pMoneyTextBox ( NULL ),
	m_nONE_VIEW_SLOT ( 4 ),
	m_pInvenButton( NULL ),
	m_pPointTextBox( NULL )
{
}

CInventoryUI_Trade::~CInventoryUI_Trade ()
{
}

void CInventoryUI_Trade::CreateSubControl ()
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

	CInventoryUI_TradeInven* pTradeInventoryPage = new CInventoryUI_TradeInven;
	pTradeInventoryPage->CreateSub ( this, "INVENTORY_WINDOW_ITEM_REGION", UI_FLAG_YSIZE, TRADEINVENTORY_PAGE );
	pTradeInventoryPage->CreateSubControl ();
	RegisterControl ( pTradeInventoryPage );
	m_pPage = pTradeInventoryPage;

	m_pInvenButton = CreateTextButton23 ( "INVENTORY_WINDOW_INVEN_TAP", TRADEINVENTORY_INVEN_BUTTON, (char*)ID2GAMEWORD ( "INVENTORY_TAP_NAME", 0 ) );
	m_pInvenButton->SetFlip( TRUE );

	CBasicButton* pMoneyButton = NULL;
	pMoneyButton = CreateFlipButton ( "INVENTORY_MONEY_BUTTON_RENEW", "INVENTORY_MONEY_BUTTON_RENEW_F", TRADEINVENTORY_MONEY_BUTTON, CBasicButton::CLICK_FLIP );
	pMoneyButton->CreateMouseOver ( "INVENTORY_MONEY_BUTTON_RENEW_F" );
	pMoneyButton->SetAlignFlag ( UI_FLAG_BOTTOM );
	pMoneyButton->SetUseDynamic ( TRUE );

	CBasicButton* pPointButton = NULL;
	pPointButton = CreateFlipButton ( "INVENTORY_POINT_BUTTON_RENEW", "INVENTORY_POINT_BUTTON_RENEW_F", TRADEINVENTORY_POINT_BUTTON, CBasicButton::CLICK_FLIP );
	pPointButton->CreateMouseOver ( "INVENTORY_POINT_BUTTON_RENEW_F" );
	pPointButton->SetAlignFlag ( UI_FLAG_BOTTOM );
	pPointButton->SetUseDynamic ( TRUE );

	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, D3DFONT_SHADOW | D3DFONT_ASCII );
	m_pMoneyTextBox = CreateStaticControl ( "INVENTORY_MONEY_TEXT_RENEW", pFont8, NS_UITEXTCOLOR::DEFAULT, TEXT_ALIGN_LEFT );
	m_pMoneyTextBox->SetAlignFlag ( UI_FLAG_BOTTOM );
	m_pPointTextBox = CreateStaticControl ( "INVENTORY_POINT_TEXT_RENEW", pFont8, NS_UITEXTCOLOR::DEFAULT, TEXT_ALIGN_LEFT );
	m_pPointTextBox->SetAlignFlag ( UI_FLAG_BOTTOM );
}

CBasicTextButton*  CInventoryUI_Trade::CreateTextButton23 ( const char* szButton, UIGUID ControlID, const char* szText )
{
	const int nBUTTONSIZE = CBasicTextButton::SIZE23;
	CBasicTextButton* pTextButton = new CBasicTextButton;
	pTextButton->CreateSub ( this, "BASIC_TEXT_BUTTON23", UI_FLAG_XSIZE|UI_FLAG_YSIZE, ControlID );
	pTextButton->CreateBaseButton ( szButton, nBUTTONSIZE, CBasicButton::RADIO_FLIP, szText, 0, NS_UITEXTCOLOR::BLACK );
	RegisterControl ( pTextButton );
	return pTextButton;
}

void CInventoryUI_Trade::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	m_bTRADE_MODE = (CInnerInterface::GetInstance().IsVisibleGroup(TRADE_WINDOW))?true:false;

	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( m_bTRADE_MODE )
	{
		GLInventory& ref_ItemData = GLTradeClient::GetInstance().GetMyTradeBox();
		m_pPage->UpdateFlipItem ( ref_ItemData );
	}else{
		m_pPage->ResetAllFlipItem ();
	}

	GLCHARLOGIC& ref_CharData = GLGaeaClient::GetInstance().GetCharacterLogic ();
	SetMoney ( ref_CharData.m_lnMoney );

	GLInventory& ref_Inventory = ref_CharData.m_cInventory;	
	m_pPage->LoadItemPage ( ref_Inventory );
}

void CInventoryUI_Trade::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	switch ( ControlID )
	{
	case TRADEINVENTORY_PAGE:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				int nPosX, nPosY;
				m_pPage->GetItemIndex ( &nPosX, &nPosY );

				if ( nPosX < 0 || nPosY < 0 ) return;

				SINVENITEM& sInvenItem = m_pPage->GetItem ( nPosX, nPosY );
				if ( sInvenItem.sItemCustom.sNativeID != NATIVEID_NULL () )
				{
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

				if ( dwMsg & UIMSG_LB_UP )
				{   
					if ( m_bTRADE_MODE )
					{
						GLGaeaClient::GetInstance().GetCharacter()->ReqInvenTo ( nPosX, nPosY );
					}else{
						if ( m_bOPENER )
						{
							GLPrivateMarket &sPMarket = GLGaeaClient::GetInstance().GetCharacter()->m_sPMarket;
							if ( sPMarket.IsOpen () ) return ;

							if ( sInvenItem.sItemCustom.sNativeID == NATIVEID_NULL () ) return ;
							const bool bCOUNTABLE = IsTurnITEM ( sInvenItem.sItemCustom );
							WORD wPosX = static_cast<WORD>(nPosX);
							WORD wPosY = static_cast<WORD>(nPosY);

							CInnerInterface::GetInstance().MODAL_PRIVATE_MARKET_SELLITEM ( bCOUNTABLE, wPosX, wPosY );
						}
					}
				}
			}
		}
		break;

	case TRADEINVENTORY_MONEY_BUTTON:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )	
			{
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEINTEXT( "INVENTORY_MONEY_TEXT") , NS_UITEXTCOLOR::WHITE );
			}

			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				if ( m_bTRADE_MODE ) DoModal ( ID2GAMEINTEXT("TRADE_MONEY"), MODAL_INPUT, EDITBOX_MONEY, MODAL_TRADEMONEY );
			}
		}
		break;

	case TRADEINVENTORY_POINT_BUTTON:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )	
			{
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEINTEXT( "INVENTORY_POINT_TEXT") , NS_UITEXTCOLOR::WHITE );
			}
		}
		break;

	case ET_CONTROL_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{				
				if ( m_bTRADE_MODE )	CInnerInterface::GetInstance().SetTradeWindowCloseReq ();
				else
				{
					bool bOPENER;
					DWORD dwGaeaID;
					CInnerInterface::GetInstance().GetPrivateMarketInfo ( bOPENER, dwGaeaID );
					if ( bOPENER )
					{
						DoModal ( ID2GAMEINTEXT("PRIVATE_MARKET_QUIT"), MODAL_QUESTION, YESNO, MODAL_PRIVATE_MARKET_QUIT );
					}else{
						CInnerInterface::GetInstance().SetPrivateMarketClose ();
					}
				}
			}
		}
		break;
	}
}

void CInventoryUI_Trade::SetMoney ( LONGLONG Money )
{
	CString strTemp = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT ( Money, 3, "," );
	m_pMoneyTextBox->SetOneLineText ( strTemp );
}

HRESULT CInventoryUI_Trade::RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;
	hr = CUIWindowEx::RestoreDeviceObjects ( pd3dDevice );
	if ( FAILED ( hr ) ) return hr;

	const long lResolution = CInnerInterface::GetInstance().GetResolution ();
	WORD X_RES = HIWORD ( lResolution );
	WORD Y_RES = LOWORD ( lResolution );

	const UIRECT& rcWindow = GetGlobalPos ();

	D3DXVECTOR2 vPos;
	vPos.x = ((X_RES) / 2.0f);
	vPos.y = (Y_RES - rcWindow.sizeY) / 2.0f;
	SetGlobalPos ( vPos );

	return S_OK;
}

bool CInventoryUI_Trade::IsTurnITEM ( SITEMCUSTOM& sItemCustom )
{
	SITEM* pItemData = GLItemMan::GetInstance().GetItem ( sItemCustom.sNativeID );
	if ( !pItemData )
	{
		GASSERT ( 0 && "아이템 정보를 찾을 수 없습니다." );
		return false;
	}

	const WORD& wPileNum = pItemData->sDrugOp.wPileNum;
	if ( wPileNum > 1 )
	{
		const WORD& wTurnNum = sItemCustom.wTurnNum;        
		if ( wTurnNum > 1 ) return true;
	}

	return false;
}

void	CInventoryUI_Trade::SetOPENER ( bool bOPENER )
{
	m_bOPENER = bOPENER;
}

void	CInventoryUI_Trade::SetOneViewSlot ( const int& nONE_VIEW_SLOT )
{
	m_pPage->SetOneViewSlot ( nONE_VIEW_SLOT );
	m_nONE_VIEW_SLOT = nONE_VIEW_SLOT;
}

const int& CInventoryUI_Trade::GetOneViewSlot () const
{
	return m_nONE_VIEW_SLOT;
}