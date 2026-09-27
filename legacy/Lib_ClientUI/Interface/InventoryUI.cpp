#include "StdAfx.h"
#include "InventoryUI.h"
#include "InventoryUIInven.h"
#include "InventoryUIBank.h"
#include "InventoryUIWishList.h"
#include "UITextControl.h"
#include "GLGaeaClient.h"
#include "ModalWindow.h"
#include "ModalCallerID.h"
#include "GameTextControl.h"
#include "InnerInterface.h"
#include "MarketWindow.h"
#include "d3dfont.h"
#include "DxGlobalStage.h"

#include "BasicButton.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "BasicTextBox.h"
#include "BasicLineBox.h"

#include "BasicTextButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CInventoryUI::CInventoryUI ()
	: m_pInvenPage( NULL )
	, m_pBankPage( NULL )
	, m_pWishListPage( NULL )
	, m_pBackGround( NULL )
	, m_pInvenButton( NULL )
	, m_pBankButton( NULL )
	, m_pWishButton( NULL )
	, m_pMoneyTextBox( NULL )
	, m_pPointTextBox( NULL )
	, m_nONE_VIEW_SLOT( 10 )
	, m_nActivePage(0)
	, bInvenLock( false )
	, m_bRingSwap( FALSE )
	, m_bEarringSwap( FALSE )
	, m_bAccsSwap( FALSE )
	, EwPosX(0)
	, EwPosY(0)
{
}

CInventoryUI::~CInventoryUI ()
{
}

void CInventoryUI::CreateSubControl ()
{
	m_pBackGround = new CUIControl;
	m_pBackGround->CreateSub ( this, "INVENTORY_WINDOW_BACKGROUND", UI_FLAG_DEFAULT );	
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

	m_pInvenButton = CreateTextButton23 ( "INVENTORY_WINDOW_INVEN_TAP", INVENTORY_INVEN_BUTTON, (char*)ID2GAMEWORD ( "INVENTORY_TAP_NAME", 0 ) );
	m_pBankButton = CreateTextButton23 ( "INVENTORY_WINDOW_BANK_TAP", INVENTORY_BANK_BUTTON, (char*)ID2GAMEWORD ( "INVENTORY_TAP_NAME", 1 ) );
	m_pWishButton = CreateTextButton23 ( "INVENTORY_WINDOW_WISH_TAP", INVENTORY_WISH_BUTTON, (char*)ID2GAMEWORD ( "INVENTORY_TAP_NAME", 2 ) );

	m_pInvenPage = new CInventoryUIInven;
	m_pInvenPage->CreateSub ( this, "INVENTORY_WINDOW_ITEM_REGION", UI_FLAG_YSIZE, INVENTORY_PAGE_INVEN );
	m_pInvenPage->CreateSubControl ();
	RegisterControl ( m_pInvenPage );

	m_pBankPage = new CInventoryUIBank;
	m_pBankPage->CreateSub ( this, "INVENTORY_WINDOW_ITEM_REGION", UI_FLAG_YSIZE, INVENTORY_PAGE_BANK );
	m_pBankPage->CreateSubControl ();
	RegisterControl ( m_pBankPage );

	m_pWishListPage = new CInventoryUIWishList;
	m_pWishListPage->CreateSub ( this, "INVENTORY_WINDOW_ITEM_REGION", UI_FLAG_YSIZE, INVENTORY_PAGE_WISHLIST );
	m_pWishListPage->CreateSubControl ();
	RegisterControl ( m_pWishListPage );

	CBasicButton* pMoneyButton = NULL;
	pMoneyButton = CreateFlipButton ( "INVENTORY_MONEY_BUTTON_RENEW", "INVENTORY_MONEY_BUTTON_RENEW_F", INVENTORY_MONEY_BUTTON, CBasicButton::CLICK_FLIP );
	pMoneyButton->CreateMouseOver ( "INVENTORY_MONEY_BUTTON_RENEW_F" );
	pMoneyButton->SetAlignFlag ( UI_FLAG_BOTTOM );
	pMoneyButton->SetUseDynamic ( TRUE );

	CBasicButton* pPointButton = NULL;
	pPointButton = CreateFlipButton ( "INVENTORY_POINT_BUTTON_RENEW", "INVENTORY_POINT_BUTTON_RENEW_F", INVENTORY_POINT_BUTTON, CBasicButton::CLICK_FLIP );
	pPointButton->CreateMouseOver ( "INVENTORY_POINT_BUTTON_RENEW_F" );
	pPointButton->SetAlignFlag ( UI_FLAG_BOTTOM );
	pPointButton->SetUseDynamic ( TRUE );

#if defined(VN_PARAM) //vietnamtest%%%
	pButton = CreateFlipButton ( "VNGAINSYS_INVEN_BUTTON", "VNGAINSYS_INVEN_BUTTON_F", INVENTORY_VNGAINSYS_BUTTON, CBasicButton::CLICK_FLIP );
	pButton->CreateMouseOver ( "VNGAINSYS_INVEN_BUTTON_F" );
	pButton->SetAlignFlag ( UI_FLAG_BOTTOM );
	pButton->SetUseDynamic ( TRUE );	
#endif

	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, D3DFONT_SHADOW | D3DFONT_ASCII );
	m_pMoneyTextBox = CreateStaticControl ( "INVENTORY_MONEY_TEXT_RENEW", pFont8, NS_UITEXTCOLOR::DEFAULT, TEXT_ALIGN_LEFT );
	m_pMoneyTextBox->SetAlignFlag ( UI_FLAG_BOTTOM );

	m_pPointTextBox = CreateStaticControl ( "INVENTORY_POINT_TEXT_RENEW", pFont8, NS_UITEXTCOLOR::DEFAULT, TEXT_ALIGN_LEFT );
	m_pPointTextBox->SetAlignFlag ( UI_FLAG_BOTTOM );

	CreateTextButtonClick ( "INVENTORY_WINDOW_LOCKER_BUTTON", INVENTORY_LOCKER_BUTTON, "" );
	CreateTextButtonClick ( "INVENTORY_WINDOW_EXTEND_BUTTON", INVENTORY_EXTEND_BUTTON, "" );
	CreateTextButtonClick ( "INVENTORY_WINDOW_TRASH_BUTTON", INVENTORY_TRASH_BUTTON, "" );
	CreateTextButtonClick ( "INVENTORY_WINDOW_SORT_BUTTON", INVENTORY_SORT_BUTTON, "" );

	CreateControl ( "INVENTORY_WINDOW_LOCKER_IMAGE" );
	CreateControl ( "INVENTORY_WINDOW_EXTEND_IMAGE" );
	CreateControl ( "INVENTORY_WINDOW_TRASH_IMAGE" );
	CreateControl ( "INVENTORY_WINDOW_SORT_IMAGE" );

	ActivePage( 0 );
}

CBasicTextButton*  CInventoryUI::CreateTextButton23 ( const char* szButton, UIGUID ControlID, const char* szText )
{
	const int nBUTTONSIZE = CBasicTextButton::SIZE23;
	CBasicTextButton* pTextButton = new CBasicTextButton;
	pTextButton->CreateSub ( this, "BASIC_TEXT_BUTTON23", UI_FLAG_XSIZE|UI_FLAG_YSIZE, ControlID );
	pTextButton->CreateBaseButton ( szButton, nBUTTONSIZE, CBasicButton::RADIO_FLIP, szText, 0, NS_UITEXTCOLOR::BLACK );
	RegisterControl ( pTextButton );
	return pTextButton;
}

CBasicTextButton*  CInventoryUI::CreateTextButton41 ( const char* szButton, UIGUID ControlID, const char* szText )
{
	const int nBUTTONSIZE = CBasicTextButton::SIZE41;
	CBasicTextButton* pTextButton = new CBasicTextButton;
	pTextButton->CreateSub ( this, "BASIC_TEXT_BUTTON41", UI_FLAG_XSIZE|UI_FLAG_YSIZE, ControlID );
	pTextButton->CreateBaseButton ( szButton, nBUTTONSIZE, CBasicButton::RADIO_FLIP, szText );
	RegisterControl ( pTextButton );
	return pTextButton;
}

CBasicTextButton*  CInventoryUI::CreateTextButtonClick ( const char* szButton, UIGUID ControlID, const char* szText )
{
	const int nBUTTONSIZE = CBasicTextButton::SIZE24;
	CBasicTextButton* pTextButton = new CBasicTextButton;
	pTextButton->CreateSub ( this, "BASIC_TEXT_BUTTON24", UI_FLAG_XSIZE|UI_FLAG_YSIZE, ControlID );
	pTextButton->CreateBaseButton ( szButton, nBUTTONSIZE, CBasicButton::CLICK_FLIP, szText );
	pTextButton->SetUseDynamic ( TRUE );
	RegisterControl ( pTextButton );
	return pTextButton;
}

void CInventoryUI::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	GLCHARLOGIC& ref_CharData = GLGaeaClient::GetInstance().GetCharacterLogic ();
	SetMoney ( ref_CharData.m_lnMoney );
	//SetPoint ( ref_CharData.m_dwPremiumPoints );
	SetPoint ( 0 );
	GLInventory& ref_Inventory = ref_CharData.m_cInventory;	
	m_pInvenPage->LoadItemPage ( ref_Inventory );
}

void CInventoryUI::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	switch ( ControlID )
	{
	case ET_CONTROL_TITLE:
	case ET_CONTROL_TITLE_F:
		{
			if ( (dwMsg & UIMSG_LB_DUP) && CHECK_MOUSE_IN ( dwMsg ) )
			{
				CInnerInterface::GetInstance().SetDefaultPosInterface( INVENTORY_WINDOW );
			}
		}
		break;
	case INVENTORY_PAGE_INVEN:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				int nPosX, nPosY;
				m_pInvenPage->GetItemIndex ( &nPosX, &nPosY );

				if ( nPosX < 0 || nPosY < 0 ) return;

				BOOL bMarketOpen = CInnerInterface::GetInstance().IsOpenDialogueWindowMarketPage ();

				{
					SINVENITEM sInvenItem = m_pInvenPage->GetItem ( nPosX, nPosY );
					if ( sInvenItem.sItemCustom.sNativeID != NATIVEID_NULL () )
					{			
						/*if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LCONTROL ) & DXKEY_DOWNED )
						{
							CInnerInterface::GetInstance().SHOW_ITEM_INFO ( sInvenItem.sItemCustom, bMarketOpen, FALSE, FALSE, sInvenItem.wPosX, sInvenItem.wPosY, NATIVEID_NULL(), FALSE, 0, TRUE, FALSE, FALSE, TRUE );
						}
						else
						{
							CInnerInterface::GetInstance().SHOW_ITEM_INFO ( sInvenItem.sItemCustom, bMarketOpen, FALSE, FALSE, sInvenItem.wPosX, sInvenItem.wPosY, NATIVEID_NULL(), FALSE, 0, TRUE, FALSE, FALSE, FALSE );
						}*/
						CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW(
						sInvenItem.sItemCustom,
						bMarketOpen,
						FALSE, FALSE, TRUE, TRUE,
						sInvenItem.wPosX, sInvenItem.wPosY );
					}

					/*if ( bMarketOpen )
					{
						if ( RANPARAM::bCheckSellItemRButton )
						{
							if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LCONTROL ) & DXKEY_PRESSED ) 
							{
								//SINVENITEM pInvenItem = m_pInvenPage->GetItem( nPosX, nPosY );
								SNATIVEID sNativeID = GLGaeaClient::GetInstance().GetCharacter()->GET_HOLD_ITEM().sNativeID;
								if ( sNativeID != NATIVEID_NULL () )
								{
									SITEM* pItemData = GLItemMan::GetInstance().GetItem( sNativeID );
									if ( !pItemData )
										return;
												
									if ( RANPARAM::bCheckConfirmSellItem )
									{
										const std::string& strItemName = pItemData->GetName();
										CString strCombine = CInnerInterface::GetInstance().MakeString ( ID2GAMEINTEXT("SELL_ITEM_CONFIRM"), strItemName.c_str() );
										DoModal ( strCombine, MODAL_QUESTION, YESNO, MODAL_SELL_ITEM );
									}
									else
									{
										const SNATIVEID& sNPCID = CInnerInterface::GetInstance().GetDialogueWindowNPCID();
										const DWORD dwNPCGlobalID = CInnerInterface::GetInstance().GetDialogueWindowNPCGlobID();
										const int nPageIndex = CInnerInterface::GetInstance().GetDialogueWindowPageIndex();

										GLGaeaClient::GetInstance().GetCharacter()->ReqNpcTo( sNPCID, dwNPCGlobalID, sInvenItem.wPosX, sInvenItem.wPosY );
									}
								}
							}
						}
					}*/

					//if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LCONTROL ) & DXKEY_DOWNED )
					//{
					//	CInnerInterface::GetInstance().SHOW_ITEM_INFO ( sInvenItem.sItemCustom, FALSE, FALSE, bMarketOpen, sInvenItem.wPosX, sInvenItem.wPosY, SNATIVEID(), FALSE, 0, FALSE, FALSE, FALSE, ITEMMALLDATA() );
					//}
					//else CInnerInterface::GetInstance().SHOW_ITEM_INFO ( sInvenItem.sItemCustom, FALSE, FALSE, bMarketOpen, sInvenItem.wPosX, sInvenItem.wPosY );

					if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LMENU  ) & DXKEY_DOWNED )
					{
						if ( dwMsg & UIMSG_LB_UP )
						{
							//item preview
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

					//dmk14 | 10-31-16 | itemlink
					if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LCONTROL ) & DXKEY_DOWNED )
					{
						if ( dwMsg & UIMSG_RB_UP )
						{
							CInnerInterface::GetInstance().LinkItem( &sInvenItem.sItemCustom );
							return;
						}
					}
				}

				if ( dwMsg & UIMSG_LB_UP )
				{
					if ( bInvenLock )//add invenlock
					{
						CInnerInterface::GetInstance().PrintConsoleText ( "Inventory is locked while ChangeName Window is Active!" );
						return ;
					}

					if ( m_bSplitItem )
					{
						m_bSplitItem = FALSE;

						WORD wSplitPosX, wSplitPosY;
						GetSplitPos ( &wSplitPosX, &wSplitPosY );
						BOOL bDiffPos = !( nPosX == wSplitPosX && nPosY == wSplitPosY );
						BOOL bSplitable = GLGaeaClient::GetInstance().GetCharacter()->IsInvenSplitItem ( wSplitPosX, wSplitPosY );
                        if ( bDiffPos && bSplitable )
						{
							DoModal ( ID2GAMEINTEXT ( "SPLIT_ITEM" ), MODAL_QUESTION, EDITBOX_NUMBER, MODAL_SPLIT_ITEM );
						}
						else
						{
							GLGaeaClient::GetInstance().GetCharacter()->ReqInvenTo ( nPosX, nPosY );
						}
					}
					return ;
				}

				if ( dwMsg & UIMSG_RB_UP )
				{
					if (bInvenLock)//add invenlock
					{
						CInnerInterface::GetInstance().PrintConsoleText ( "Inventory is locked while ChangeName Window is Active!" );
						return ;
					}

					/*rightclick wear/unwear, Juver, 2017/06/24 */
					HRESULT hrWear = GLGaeaClient::GetInstance().GetCharacter()->ReqInvenToWear( nPosX, nPosY );
					if ( hrWear != S_OK )
						GLGaeaClient::GetInstance().GetCharacter()->ReqInvenDrug ( nPosX, nPosY );

					return ;
				}

				if ( dwMsg & UIMSG_LB_DOWN )
				{
					SetSplitPos ( nPosX, nPosY );
					m_bSplitItem = TRUE;
					return ;
				}
			}
		}
		break;

	case INVENTORY_PAGE_BANK:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				int nPosX, nPosY;
				m_pBankPage->GetItemIndex ( &nPosX, &nPosY );

				if ( nPosX < 0 || nPosY < 0 ) return ;

				SINVENITEM sInvenItem = m_pBankPage->GetItem ( nPosX, nPosY );
				if ( sInvenItem.sItemCustom.sNativeID != NATIVEID_NULL () )
				{
					CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW(
					sInvenItem.sItemCustom,
					FALSE,
					FALSE, FALSE, TRUE, FALSE,
					sInvenItem.wPosX, sInvenItem.wPosY );
					
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
				}

				if ( dwMsg & UIMSG_LB_UP )
				{
					GLGaeaClient::GetInstance().GetCharacter()->ReqChargedItemTo ( static_cast<WORD>(nPosX), static_cast<WORD>(nPosY) );
					return ;
				}
			}
		}break;

	case INVENTORY_MONEY_BUTTON:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )	
			{
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEINTEXT( "INVENTORY_MONEY_TEXT") , NS_UITEXTCOLOR::WHITE );
			}

			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				if ( CInnerInterface::GetInstance().IsStorageWindowOpen () )
				{
					DoModal ( ID2GAMEINTEXT("PLAYINTERFACE_31"), MODAL_INPUT, EDITBOX_MONEY, MODAL_SAVEMONEY );
				}
				else if ( CInnerInterface::GetInstance().IsClubStorageWindowOpen () )
				{
					DoModal ( ID2GAMEINTEXT("PLAYINTERFACE_31"), MODAL_INPUT, EDITBOX_MONEY, MODAL_CLUB_SAVEMONEY );
				}else{
					if ( GLCONST_CHAR::bMONEY_DROP2FIELD )
					{
						DoModal ( ID2GAMEINTEXT("MONEYTO_FIELD"), MODAL_INPUT, EDITBOX_MONEY, MODAL_INVENTORY_MONEY_TO_FIELD );
					}
				}
			}
		}
		break;
#if defined(VN_PARAM) //vietnamtest%%%
	case INVENTORY_VNGAINSYS_BUTTON:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{

				if ( dwMsg & UIMSG_LB_UP )
				{
					if ( !CInnerInterface::GetInstance().IsVisibleGroup ( VNGAINSYS_WINDOW ) )
					{
						CInnerInterface::GetInstance().ShowGroupFocus ( VNGAINSYS_WINDOW );
					}
					else
					{
						CInnerInterface::GetInstance().HideGroup ( VNGAINSYS_WINDOW );
					}
				}
			}
		}
		break;
#endif
	case ET_CONTROL_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				CInnerInterface::GetInstance().HideGroup ( MARKET_WINDOW );
				CInnerInterface::GetInstance().HideGroup ( STORAGE_WINDOW );
				CInnerInterface::GetInstance().HideGroup ( CLUB_STORAGE_WINDOW );
			}
		}
		break;

	case INVENTORY_INVEN_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )	ActivePage( INVENTORY_PAGE );
		}break;
	case INVENTORY_BANK_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
				CInnerInterface::GetInstance().SetItemBankWindowOpen ();;
		}break;
	case INVENTORY_WISH_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )	ActivePage( WISHLIST_PAGE );
		}break;



	case INVENTORY_LOCKER_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )	
			{
				SINVENITEM* sItem = GLGaeaClient::GetInstance().GetCharacter()->m_cInventory.FindItem( ITEM_STORAGE_CONNECT );
				if ( sItem )
				{
					GLGaeaClient::GetInstance().GetCharacter()->ReqStorageOpenCardInvenRequest( sItem->wPosX, sItem->wPosY );
				}else{
					CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("INVEN_LOCKER_OPEN_FAIL" ) );
				}
			}

			if ( CHECK_MOUSE_IN ( dwMsg ) )	
			{
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEWORD( "INVENTORY_BUTTON_TEXT" ,0 ) , NS_UITEXTCOLOR::WHITE );
			}
		}break;
	case INVENTORY_EXTEND_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )	
			{
				SINVENITEM* sItem = GLGaeaClient::GetInstance().GetCharacter()->m_cInventory.FindInvenCardItem();
				if ( sItem )
				{
					GLGaeaClient::GetInstance().GetCharacter()->ReqInvenLineCard( sItem->wPosX, sItem->wPosY );
				}else{
					CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("INVEN_CARD_NOTFOUND" ) );
				}
			}

			if ( CHECK_MOUSE_IN ( dwMsg ) )	
			{
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEWORD( "INVENTORY_BUTTON_TEXT" ,1 ) , NS_UITEXTCOLOR::WHITE );
			}
		}break;
	case INVENTORY_TRASH_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )	
			{
				GLGaeaClient::GetInstance().GetCharacter()->ReqGarbageOpenInven();
			}

			if ( CHECK_MOUSE_IN ( dwMsg ) )	
			{
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEWORD( "INVENTORY_BUTTON_TEXT" ,2 ) , NS_UITEXTCOLOR::WHITE );
			}

		}break;
	case INVENTORY_SORT_BUTTON:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )	
			{
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEWORD( "INVENTORY_BUTTON_TEXT" ,3 ) , NS_UITEXTCOLOR::WHITE );
			}

			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )	
			{
				GLGaeaClient::GetInstance().GetCharacter()->ReqSortItem();
			}
		}break;


	case INVENTORY_POINT_BUTTON:
		{
		}break;
	}

	CUIWindowEx::TranslateUIMessage ( ControlID, dwMsg );
}

void CInventoryUI::SetMoney ( LONGLONG Money )
{
	CString strTemp = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT ( Money, 3, "," );
	m_pMoneyTextBox->SetOneLineText ( strTemp );
}

void CInventoryUI::SetPoint ( LONGLONG Point )
{
	CString strTemp = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT ( Point, 3, "," );
	m_pPointTextBox->SetOneLineText ( strTemp );
}

void CInventoryUI::GetSplitPos ( WORD* pwPosX, WORD* pwPosY )
{
	*pwPosX = m_wSplitItemPosX;
	*pwPosY = m_wSplitItemPosY;
}

void CInventoryUI::SetSplitPos ( WORD wPosX, WORD wPosY )
{
	m_wSplitItemPosX = wPosX;
	m_wSplitItemPosY = wPosY;
}

void	CInventoryUI::SetOneViewSlot ( const int& nONE_VIEW_SLOT )
{
	m_pInvenPage->SetOneViewSlot ( nONE_VIEW_SLOT );
	m_nONE_VIEW_SLOT = nONE_VIEW_SLOT;
}

void CInventoryUI::SetVisibleSingle ( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		ActivePage( 0 );
	}
}

void CInventoryUI::ActivePage( int nPage )
{
	m_pInvenButton->SetFlip ( FALSE );
	m_pBankButton->SetFlip ( FALSE );
	m_pWishButton->SetFlip ( FALSE );

	m_pInvenPage->SetVisibleSingle( FALSE );
	m_pBankPage->SetVisibleSingle( FALSE );
	m_pWishListPage->SetVisibleSingle( FALSE );

	switch( nPage )
	{
	case INVENTORY_PAGE://inven
		{
			m_pInvenButton->SetFlip ( TRUE );
			m_pInvenPage->SetVisibleSingle( TRUE );
			m_nActivePage = nPage;
		}break;
	case ITEMBANK_PAGE://bank
		{
			m_pBankButton->SetFlip ( TRUE );
			m_pBankPage->SetVisibleSingle( TRUE );
			m_nActivePage = nPage;
		}break;
	case WISHLIST_PAGE://wishlist
		{
			m_pWishButton->SetFlip ( TRUE );
			m_pWishListPage->SetVisibleSingle( TRUE );
			m_nActivePage = nPage;
		}break;
	};
}

void CInventoryUI::InitItemBank ()
{
	if ( !m_pBankPage ) return;

	m_pBankPage->UnLoadItemPage ();
	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( !pCharacter ) return;
	m_pBankPage->LoadItemPage ( pCharacter->m_cInvenCharged );
}

void CInventoryUI::ClearItemBank()
{ 
	if ( !m_pBankPage ) return;
	m_pBankPage->UnLoadItemPage ();
}