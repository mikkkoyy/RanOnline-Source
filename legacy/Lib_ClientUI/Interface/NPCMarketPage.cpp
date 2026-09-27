#include "StdAfx.h"

#include "../Lib_Engine/Common/StringFormat.h"

#include "BasicLineBoxSmart.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "GameTextControl.h"
#include "BasicTextBoxEx.h"
#include "UITextControl.h"
#include "ModalWindow.h"
#include "ModalCallerID.h"

#include "../Lib_Client/G-Logic/GLCrowData.h"
#include "../Lib_Client/G-Logic/GLItemMan.h"
#include "RANPARAM.h"

#include "../Lib_Client/G-Logic/GLGaeaClient.h"
#include "../Lib_Client/G-Logic/GLCharacter.h"


#include "InnerInterface.h"
#include "ItemImage.h"
#include "ItemSlot.h"
#include "MarketPage.h"
#include "Util/RnButton.h"
#include "NPCMarketPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int CNPCMarketPage::nOUTOFRANGE = -1;
const float CNPCMarketPage::fANSWER_PART_INTERVAL = 2.0f;
const int CNPCMarketPage::NUM_CART_LIST_ITEM = 10;

const float CNPCMarketPage::fNOTIFY_FULL_CART_TIME = 4.0f;
const float CNPCMarketPage::fNOTIFY_FULL_CART_FADE_TIME = 0.2f;

CNPCMarketPage::CNPCMarketPage()
    : m_pSellListBack(NULL)
    , m_pRecommendListBack(NULL)
    , m_pRecommendTitle(NULL)
    , m_pCheckItemTypeBack(NULL)
    , m_pCheckUsableItem(NULL)
    , m_pUncheckUsableItem(NULL)
    , m_pUsableItemText(NULL)
    , m_pCheckBuyableItem(NULL)
    , m_pUncheckBuyableItem(NULL)
    , m_pBuyableItemText(NULL)
    , m_pMoneyBack(NULL)
    , m_pMoneyText(NULL)
    , m_pCartListBack(NULL)
    , m_pCartIconButton(NULL)
    , m_pCartItemSlot(NULL)
    , m_pEmptyCartHelpBack(NULL)
    , m_pEmptyCartHelpText(NULL)
    , m_pBuyButton(NULL)
    , m_pBuyButtonPointText(NULL)
    , m_pBuyButtonBuyText(NULL)
    , m_pCloseButton(NULL)
    , m_pNotifyFullCartBack(NULL)
    , m_pNotifyFullCartText(NULL)
    , m_pCheckSellRBtn(NULL)
    , m_pUncheckSellRBtn(NULL)
    , m_pSellRBtnText(NULL)
    , m_pCheckConfirmSell(NULL)
    , m_pUncheckConfirmSell(NULL)
    , m_pConfirmSellText(NULL)

    , m_nPageIndex(nOUTOFRANGE)
    , m_nTotalPrice(0)
    , m_nCurInvenMoney(0)
    , m_fNotifyFullCartTime(fNOTIFY_FULL_CART_TIME)
    , m_fNotifyFullCartFadeTime(fNOTIFY_FULL_CART_FADE_TIME)
{
    for ( int i = 0; i < MAX_DIALOGUE_MARKET_TAB; i++ )
        m_pSellPage[ i ] = NULL;

    for ( int i = 0; i < NUM_RECOMMEND_ITEM; i++ )
    {
        m_pRecommendItem[ i ] = NULL;
        m_pRecommendItemText[ i ] = NULL;
    }

    m_listCart.clear();
}

CNPCMarketPage::~CNPCMarketPage()
{
    m_listCart.clear();
}

void CNPCMarketPage::CreateSubControl()
{
    {
        CBasicLineBoxSmart* pLinBoxSmart = new CBasicLineBoxSmart( );
        pLinBoxSmart->CreateSub( this, "DIALOGUE_WINDOW_MARKET_SELL_LIST_BACK", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
        pLinBoxSmart->CreateSubControl( "DIALOGUE_WINDOW_MARKET_SELL_LIST_BACK_LINE" );
        RegisterControl( pLinBoxSmart );
        m_pSellListBack = pLinBoxSmart;
    }

    {
        for ( int i = 0; i < MAX_DIALOGUE_MARKET_TAB; i++ )
        {
            m_pSellPage[ i ] = CreateMarketPage( ( MARKET_SELL_PAGE_0 + i ) );
            m_pSellPage[ i ]->SetVisibleScrollbar( false );
        }
    }

    {
        CBasicLineBoxSmart* pLinBoxSmart = new CBasicLineBoxSmart( );
        pLinBoxSmart->CreateSub( this, "DIALOGUE_WINDOW_MARKET_RECOMMEND_LIST_BACK", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
        pLinBoxSmart->CreateSubControl( "DIALOGUE_WINDOW_MARKET_RECOMMEND_LIST_BACK_LINE" );
        RegisterControl( pLinBoxSmart );
        m_pRecommendListBack = pLinBoxSmart;
    }

	CD3DFontPar* pFont10Shadow = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 10, _DEFAULT_FONT_SHADOW_EX_FLAG );

    {

        CBasicTextBox* pTextBox = new CBasicTextBox();
        pTextBox->CreateSub( this, "DIALOGUE_WINDOW_MARKET_RECOMMEND_TITLE", UI_FLAG_DEFAULT );
        pTextBox->SetFont( pFont10Shadow );
        pTextBox->SetTextAlign( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
        RegisterControl( pTextBox );
        m_pRecommendTitle = pTextBox;
        m_pRecommendTitle->SetText( ID2GAMEWORD( "DIALOGUE_RECOMMEND_ITEM_TITLE", 0 ),
            NS_UITEXTCOLOR::WHITE );
    }

    {
        char* strUIRecommendItemBack[ NUM_RECOMMEND_ITEM ] =
        {
            "DIALOGUE_WINDOW_MARKET_RECOMMEND_BACK_0"
            , "DIALOGUE_WINDOW_MARKET_RECOMMEND_BACK_1"
            , "DIALOGUE_WINDOW_MARKET_RECOMMEND_BACK_2"
            , "DIALOGUE_WINDOW_MARKET_RECOMMEND_BACK_3"
            , "DIALOGUE_WINDOW_MARKET_RECOMMEND_BACK_4"
        };

        char* strUIRecommendItemBackLine[ NUM_RECOMMEND_ITEM ] =
        {
            "DIALOGUE_WINDOW_MARKET_RECOMMEND_BACK_0_LINE"
            , "DIALOGUE_WINDOW_MARKET_RECOMMEND_BACK_1_LINE"
            , "DIALOGUE_WINDOW_MARKET_RECOMMEND_BACK_2_LINE"
            , "DIALOGUE_WINDOW_MARKET_RECOMMEND_BACK_3_LINE"
            , "DIALOGUE_WINDOW_MARKET_RECOMMEND_BACK_4_LINE"
        };

        char* strUIRecommendItem[ NUM_RECOMMEND_ITEM ] =
        {
            "DIALOGUE_WINDOW_MARKET_RECOMMEND_ITEM_0"
            , "DIALOGUE_WINDOW_MARKET_RECOMMEND_ITEM_1"
            , "DIALOGUE_WINDOW_MARKET_RECOMMEND_ITEM_2"
            , "DIALOGUE_WINDOW_MARKET_RECOMMEND_ITEM_3"
            , "DIALOGUE_WINDOW_MARKET_RECOMMEND_ITEM_4"
        };

        char* strUIRecommendText[ NUM_RECOMMEND_ITEM ] =
        {
            "DIALOGUE_WINDOW_MARKET_RECOMMEND_TEXT_0"
            , "DIALOGUE_WINDOW_MARKET_RECOMMEND_TEXT_1"
            , "DIALOGUE_WINDOW_MARKET_RECOMMEND_TEXT_2"
            , "DIALOGUE_WINDOW_MARKET_RECOMMEND_TEXT_3"
            , "DIALOGUE_WINDOW_MARKET_RECOMMEND_TEXT_4"
        };

        for ( int i = 0; i < NUM_RECOMMEND_ITEM; i++ )
        {
            CBasicLineBoxSmart* pLinBoxSmart = new CBasicLineBoxSmart(  );
            pLinBoxSmart->CreateSub( this, strUIRecommendItemBack[ i ], UI_FLAG_XSIZE | UI_FLAG_YSIZE );
            pLinBoxSmart->CreateSubControl( strUIRecommendItemBackLine[ i ] );
            RegisterControl( pLinBoxSmart );

            m_pRecommendItem[ i ] = CreateItemSlot( strUIRecommendItem[ i ], ( RECOMMEND_ITEM_0 + i ) );

			//m_pRecommendItem[ i ]->SetExchangeSlot( true );

            CBasicTextBoxEx* pBaiscTextBoxEx = new CBasicTextBoxEx();
            pBaiscTextBoxEx->CreateSub( this, strUIRecommendText[ i ], UI_FLAG_DEFAULT, ( RECOMMEND_TEXT_0 + i ) );
            pBaiscTextBoxEx->SetFont( pFont10Shadow );
            pBaiscTextBoxEx->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
            pBaiscTextBoxEx->SetPartInterval( fANSWER_PART_INTERVAL );
            pBaiscTextBoxEx->SetSensitive( true );
            RegisterControl( pBaiscTextBoxEx );
            m_pRecommendItemText[ i ] = pBaiscTextBoxEx;
        }
    }

    {
        CBasicLineBoxSmart* pLinBoxSmart = new CBasicLineBoxSmart(  );
        pLinBoxSmart->CreateSub( this, "DIALOGUE_WINDOW_MARKET_CHECK_ITEM_TYPE_BACK", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
        pLinBoxSmart->CreateSubControl( "DIALOGUE_WINDOW_MARKET_CHECK_ITEM_TYPE_BACK_LINE" );
        RegisterControl( pLinBoxSmart );
        m_pCheckItemTypeBack = pLinBoxSmart;
    }

    {
        CUIControl* pControl = new CUIControl();
        pControl->CreateSub( this, "DIALOGUE_WINDOW_USABLE_ITEM_CHECK", UI_FLAG_DEFAULT );
        RegisterControl( pControl );
        m_pCheckUsableItem = pControl;

        pControl = new CUIControl();
        pControl->CreateSub( this, "DIALOGUE_WINDOW_USABLE_ITEM_UNCHECK", UI_FLAG_DEFAULT );
        RegisterControl( pControl );
        m_pUncheckUsableItem = pControl;
        m_pUncheckUsableItem->SetVisibleSingle( FALSE );

        pControl = new CUIControl();
        pControl->CreateSub( this, "DIALOGUE_WINDOW_USABLE_ITEM_BACK", UI_FLAG_DEFAULT, RECOMMEND_CHECK_USABLE );
        RegisterControl( pControl );

        CBasicTextBox* pTextBox = new CBasicTextBox();
        pTextBox->CreateSub( this, "DIALOGUE_WINDOW_USABLE_ITEM_TEXT", UI_FLAG_DEFAULT );
        pTextBox->SetFont( pFont10Shadow );
        pTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
        RegisterControl( pTextBox );
        m_pUsableItemText = pTextBox;
        m_pUsableItemText->SetText( ID2GAMEINTEXT( "DIALOGUE_WINDOW_USABLE_TEXT" ), NS_UITEXTCOLOR::WHITE );
    }

    {
        CUIControl* pControl = new CUIControl();
        pControl->CreateSub( this, "DIALOGUE_WINDOW_BUYABLE_ITEM_CHECK", UI_FLAG_DEFAULT );
        RegisterControl( pControl );
        m_pCheckBuyableItem = pControl;

        pControl = new CUIControl();
        pControl->CreateSub( this, "DIALOGUE_WINDOW_BUYABLE_ITEM_UNCHECK", UI_FLAG_DEFAULT );
        RegisterControl( pControl );
        m_pUncheckBuyableItem = pControl;
        m_pUncheckBuyableItem->SetVisibleSingle( FALSE );

        pControl = new CUIControl();
        pControl->CreateSub( this, "DIALOGUE_WINDOW_BUYABLE_ITEM_BACK", UI_FLAG_DEFAULT, RECOMMEND_CHECK_BUYABLE );
        RegisterControl( pControl );
		
        CBasicTextBox* pTextBox = new CBasicTextBox();
        pTextBox->CreateSub( this, "DIALOGUE_WINDOW_BUYABLE_ITEM_TEXT", UI_FLAG_DEFAULT );
        pTextBox->SetFont( pFont10Shadow );
        pTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
        RegisterControl( pTextBox );
        m_pBuyableItemText = pTextBox;
        m_pBuyableItemText->SetText( ID2GAMEINTEXT( "DIALOGUE_WINDOW_BUYABLE_TEXT" ), NS_UITEXTCOLOR::WHITE );
    }

	{
		CUIControl* pControl = new CUIControl();
		pControl->CreateSub( this, "DIALOGUE_WINDOW_BUYABLE_ITEM_CHECK", UI_FLAG_DEFAULT );
		RegisterControl( pControl );
		m_pCheckExchangeableItem = pControl;

		pControl = new CUIControl();
		pControl->CreateSub( this, "DIALOGUE_WINDOW_BUYABLE_ITEM_UNCHECK", UI_FLAG_DEFAULT );
		RegisterControl( pControl );
		m_pUncheckExchangeableItem = pControl;
		m_pUncheckExchangeableItem->SetVisibleSingle( FALSE );

		pControl = new CUIControl();
		pControl->CreateSub( this, "DIALOGUE_WINDOW_EXCHANGEABLE_ITEM_BACK", UI_FLAG_DEFAULT, RECOMMEND_CHECK_EXCHANGE_ABLE );
		RegisterControl( pControl );

		CBasicTextBox* pTextBox = new CBasicTextBox();
		pTextBox->CreateSub( this, "DIALOGUE_WINDOW_BUYABLE_ITEM_TEXT", UI_FLAG_DEFAULT );
		pTextBox->SetFont( pFont10Shadow );
		pTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
		RegisterControl( pTextBox );
		m_pExchangeableItemText = pTextBox;
		m_pExchangeableItemText->SetText( ID2GAMEINTEXT( "DIALOGUE_WINDOW_EXCHANGEABLE_TEXT" ), NS_UITEXTCOLOR::WHITE );
	}

    {
        CBasicLineBoxSmart* pLinBoxSmart = new CBasicLineBoxSmart(  );
        pLinBoxSmart->CreateSub( this, "DIALOGUE_WINDOW_MARKET_MONEY_BACK", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
        pLinBoxSmart->CreateSubControl( "DIALOGUE_WINDOW_MARKET_MONEY_BACK_LINE" );
        RegisterControl( pLinBoxSmart );
        m_pMoneyBack = pLinBoxSmart;

        CBasicTextBox* pTextBox = new CBasicTextBox();
        pTextBox->CreateSub( this, "DIALOGUE_WINDOW_MARKET_MONEY_TEXT", UI_FLAG_DEFAULT );
        pTextBox->SetFont( pFont10Shadow );
        pTextBox->SetTextAlign( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
        RegisterControl( pTextBox );
        m_pMoneyText = pTextBox;
    }

    {
        CBasicLineBoxSmart* pLinBoxSmart = new CBasicLineBoxSmart();
        pLinBoxSmart->CreateSub( this, "DIALOGUE_WINDOW_CART_LIST_BACK", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
        pLinBoxSmart->CreateSubControl( "DIALOGUE_WINDOW_CART_LIST_BACK_LINE" );
        RegisterControl( pLinBoxSmart );
        m_pCartListBack = pLinBoxSmart;

        CUIControl* pControl = new CUIControl();
        pControl->CreateSub( this, "DIALOGUE_WINDOW_CART_ICON", UI_FLAG_DEFAULT, CART_ICON );
        RegisterControl( pControl );
        m_pCartIconButton = pControl;
    }

    {
        CItemSlot* pItemSlot = new CItemSlot( );
        pItemSlot->CreateSub( this, "DIALOGUE_WINDOW_CART_ITEM_SLOT", UI_FLAG_DEFAULT, CART_ITEM_SLOT );
        pItemSlot->CreateSubControl( NUM_CART_LIST_ITEM, TRUE );
        RegisterControl( pItemSlot );
        m_pCartItemSlot = pItemSlot;
		//m_pCartItemSlot->SetExchangeSlot( true );
    }

    {
        RnButton* pRnButton = NULL;

        RnButton::CreateArg arg;
        //////////////////////////////////////////////////////////////////////////
        arg.defaultTextureKey = "RNBUTTON_DEFAULT_TEXTURE";
        arg.mouseOverTextureKey = "RNBUTTON_OVER_TEXTURE";
        arg.mouseClickTextureKey = "RNBUTTON_CLICK_TEXTURE";
        //////////////////////////////////////////////////////////////////////////

        pRnButton = new RnButton();
        pRnButton->CreateSub(this,"DIALOGUE_WINDOW_BUY_BUTTON",UI_FLAG_DEFAULT,MARKET_BUY_BUTTON);
        pRnButton->CreateSubControl(arg);
        RegisterControl(pRnButton);
        m_pBuyButton = pRnButton;

		CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

        CBasicTextBox* pTextBox = new CBasicTextBox();
        pTextBox->CreateSub ( this, "DIALOGUE_WINDOW_BUY_BUTTON_TEXT_LEFT", UI_FLAG_DEFAULT );
        pTextBox->SetFont( pFont9 );
        pTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
        RegisterControl( pTextBox );
        m_pBuyButtonPointText = pTextBox;

        pTextBox = new CBasicTextBox();
        pTextBox->CreateSub ( this, "DIALOGUE_WINDOW_BUY_BUTTON_TEXT_RIGHT", UI_FLAG_DEFAULT );
        pTextBox->SetFont( pFont9 );
        pTextBox->SetTextAlign( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
        RegisterControl( pTextBox );
        m_pBuyButtonBuyText = pTextBox;
        m_pBuyButtonBuyText->SetText( ID2GAMEWORD( "POINT_SHOP_BUY_BUTTON_TEXT" ), NS_UITEXTCOLOR::WHITE );
        
    }

    {
		CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

        RnButton* pRnButton = NULL;

        RnButton::CreateArg arg;
        //////////////////////////////////////////////////////////////////////////
        arg.defaultTextureKey = "RNBUTTON_DEFAULT_TEXTURE";
        arg.mouseOverTextureKey = "RNBUTTON_OVER_TEXTURE";
        arg.mouseClickTextureKey = "RNBUTTON_CLICK_TEXTURE";
        //////////////////////////////////////////////////////////////////////////
        arg.pFont = pFont9;
        arg.text = sc::string::format( "%1%", ID2GAMEWORD( "DIALOGUE_WINDOW_CLOSE_BUTTON" ) );
        arg.dwColor = NS_UITEXTCOLOR::WHITE;

        pRnButton = new RnButton();
        pRnButton->CreateSub(this,"DIALOGUE_WINDOW_CLOSE_BUTTON",UI_FLAG_DEFAULT,DIALOGUE_CLOSE_BUTTON);
        pRnButton->CreateSubControl(arg);
        RegisterControl(pRnButton);
        m_pCloseButton = pRnButton;
    }

    {
        CBasicLineBoxSmart* pLinBoxSmart = new CBasicLineBoxSmart();
        pLinBoxSmart->CreateSub( this, "DIALOGUE_WINDOW_CART_ICON_HELP_BACK", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
        pLinBoxSmart->CreateSubControl( "DIALOGUE_WINDOW_CART_ICON_HELP_BACK_LINE" );
        RegisterControl( pLinBoxSmart );
        m_pEmptyCartHelpBack = pLinBoxSmart;
        m_pEmptyCartHelpBack->SetVisibleSingle( FALSE );

		CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

        CBasicTextBox* pTextBox = new CBasicTextBox();
        pTextBox->CreateSub ( this, "DIALOGUE_WINDOW_CART_ICON_HELP_TEXT", UI_FLAG_DEFAULT );
        pTextBox->SetFont( pFont9 );
        pTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
        RegisterControl( pTextBox );
        m_pEmptyCartHelpText = pTextBox;
        m_pEmptyCartHelpText->SetText( ID2GAMEINTEXT("POINT_SHOP_CART_ICON_HELP"), NS_UITEXTCOLOR::WHITE );
        m_pEmptyCartHelpText->SetVisibleSingle( FALSE );
    }

	CD3DFontPar* pFont9Shadow = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_EX_FLAG );

    {
        CBasicLineBoxSmart* pLinBoxSmart = new CBasicLineBoxSmart();
        pLinBoxSmart->CreateSub( this, "DIALOGUE_WINDOW_NOTIFY_FULL_CART_BACK", UI_FLAG_XSIZE | UI_FLAG_YSIZE, NOTIFY_FULL_CART_BACK );
        pLinBoxSmart->CreateSubControl( "DIALOGUE_WINDOW_NOTIFY_FULL_CART_BACK_LINE" );
        RegisterControl( pLinBoxSmart );
        m_pNotifyFullCartBack = pLinBoxSmart;

		CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

        CBasicTextBox* pTextBox = new CBasicTextBox();
        pTextBox->CreateSub( this, "DIALOGUE_WINDOW_NOTIFY_FULL_CART_TEXT", UI_FLAG_DEFAULT, NOTIFY_FULL_CART_TEXT );
        pTextBox->SetFont( pFont9 );
        pTextBox->SetTextAlign( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
        RegisterControl( pTextBox );
        m_pNotifyFullCartText = pTextBox;
        m_pNotifyFullCartText->AddText( ID2GAMEINTEXT("POINT_SHOP_NOTIFY_FULL_CART_TEXT0"), NS_UITEXTCOLOR::YELLOW );
        m_pNotifyFullCartText->AddText( ID2GAMEINTEXT("POINT_SHOP_NOTIFY_FULL_CART_TEXT1"), NS_UITEXTCOLOR::WHITE );

        UnViewNotifyFullCart();
    }

    {
        CUIControl* pControl = new CUIControl();
        pControl->CreateSub( this, "DIALOGUE_WINDOW_UNDER_LINE" );
        RegisterControl( pControl );
    }

    {
        CUIControl* pControl = new CUIControl();
        pControl->CreateSub( this, "MARKET_SELL_RIGHT_BUTTON_CHECK", UI_FLAG_DEFAULT );
        RegisterControl( pControl );
        m_pCheckSellRBtn = pControl;

        pControl = new CUIControl();
        pControl->CreateSub( this, "MARKET_SELL_RIGHT_BUTTONS_UNCHECK", UI_FLAG_DEFAULT );
        RegisterControl( pControl );
        m_pUncheckSellRBtn = pControl;
        m_pUncheckSellRBtn->SetVisibleSingle( FALSE );

        pControl = new CUIControl();
        pControl->CreateSub( this, "MARKET_SELL_RIGHT_BUTTON_BACK", UI_FLAG_DEFAULT, CHECK_SELL_R_BTN );
        RegisterControl( pControl );

        CBasicTextBox* pTextBox = new CBasicTextBox();
        pTextBox->CreateSub( this, "MARKET_SELL_RIGHT_BUTTON_TEXT", UI_FLAG_DEFAULT );
        pTextBox->SetFont( pFont9Shadow );
        pTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );
        RegisterControl( pTextBox );
        m_pSellRBtnText = pTextBox;
        m_pSellRBtnText->SetText( ID2GAMEINTEXT( "MARKET_SELL_RIGHT_BUTTON_TEXT" ), NS_UITEXTCOLOR::WHITE );
    }

    {
        CUIControl* pControl = new CUIControl();
        pControl->CreateSub( this, "MARKET_COFIRM_SELL_ITEM_CHECK", UI_FLAG_DEFAULT );
        RegisterControl( pControl );
        m_pCheckConfirmSell = pControl;

        pControl = new CUIControl();
        pControl->CreateSub( this, "MARKET_COFIRM_SELL_ITEM_UNCHECK", UI_FLAG_DEFAULT );
        RegisterControl( pControl );
        m_pUncheckConfirmSell = pControl;
        m_pUncheckConfirmSell->SetVisibleSingle( FALSE );

        pControl = new CUIControl();
        pControl->CreateSub( this, "MARKET_COFIRM_SELL_ITEM_BACK", UI_FLAG_DEFAULT, CHECK_CONFIRM_SELL_ITEM );
        RegisterControl( pControl );

        CBasicTextBox* pTextBox = new CBasicTextBox();
        pTextBox->CreateSub( this, "MARKET_COFIRM_SELL_ITEM_TEXT", UI_FLAG_DEFAULT );
        pTextBox->SetFont( pFont9Shadow );
        pTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );
        RegisterControl( pTextBox );
        m_pConfirmSellText = pTextBox;
        m_pConfirmSellText->SetText( ID2GAMEINTEXT( "MARKET_CONFIRM_SELL_ITEM_TEXT" ), NS_UITEXTCOLOR::WHITE );
    }
}

CMarketPage* CNPCMarketPage::CreateMarketPage( UIGUID ControlID )
{
    CMarketPage* pSellPage = new CMarketPage();
    pSellPage->CreateSub( this, "DIALOGUE_WINDOW_MARKET_SELL_LIST", UI_FLAG_DEFAULT, ControlID );
    pSellPage->CreateSubControl();
    RegisterControl( pSellPage );
    return pSellPage;
}

CItemSlot* CNPCMarketPage::CreateItemSlot( const char* szControl, UIGUID ControlID )
{
    CItemSlot* pItemSlot = new CItemSlot(  );
    pItemSlot->CreateSub( this, szControl, UI_FLAG_DEFAULT, ControlID );
    pItemSlot->CreateSubControl( 1, TRUE );
    RegisterControl( pItemSlot );

    return pItemSlot;
}

void CNPCMarketPage::Update( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
    if ( !CUIControlNpcActionable::IsUseable() )
    {
        CInnerInterface::GetInstance().SetDialogueWindowClose();
        return;
    }

	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
    if ( pCharacter )
    {
        LONGLONG InvenMoney = pCharacter->GetInvenMoney();

        if ( InvenMoney != m_nCurInvenMoney )
            SetCurInvenMoney();
    }

    if ( m_pNotifyFullCartBack->IsVisible() )
    {
        m_fNotifyFullCartTime -= fElapsedTime;

        if ( m_fNotifyFullCartTime <= 0.0f )
        {
            m_fNotifyFullCartFadeTime -= fElapsedTime;
            float fPercent = m_fNotifyFullCartFadeTime / fNOTIFY_FULL_CART_FADE_TIME;
            WORD wDiffuse = static_cast<WORD>( 255 * fPercent );

            m_pNotifyFullCartBack->SetDiffuseAlpha( wDiffuse );
            m_pNotifyFullCartText->SetDiffuseAlpha( wDiffuse );

            if ( m_fNotifyFullCartFadeTime <= 0.0f )
                UnViewNotifyFullCart();
        }
    }

    CUIGroup::Update( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CNPCMarketPage::TranslateUIMessage( UIGUID ControlID, DWORD dwMsg )
{
    switch ( ControlID )
    {
    case RECOMMEND_CHECK_USABLE:
        {
            if ( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
            {
                SetCheckUsableItem( !RANPARAM::bCheckUsableItemMarketPage );
				SetRecommendItem();
            }
        }
        break;

    case RECOMMEND_CHECK_BUYABLE:
        {
            if ( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
            {
                SetCheckBuyableItem( !RANPARAM::bCheckBuyableItemMarketPage );
				SetRecommendItem();
            }
        }
        break;

    case CART_ICON:
        {
            if ( CHECK_MOUSE_IN( dwMsg ) )
            {
                m_pEmptyCartHelpBack->SetVisibleSingle( TRUE );
                m_pEmptyCartHelpText->SetVisibleSingle( TRUE );
            }
            else
            {
                m_pEmptyCartHelpBack->SetVisibleSingle( FALSE );
                m_pEmptyCartHelpText->SetVisibleSingle( FALSE );
            }

            if ( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
            {
                ResetCartList();
            }
        }
        break;

    case NOTIFY_FULL_CART_BACK:
    case NOTIFY_FULL_CART_TEXT:
        {
            if ( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
                UnViewNotifyFullCart();
        }
        break;

    case MARKET_SELL_PAGE_0:
    case MARKET_SELL_PAGE_1:
    case MARKET_SELL_PAGE_2:
        {
			if ( CHECK_MOUSE_IN( dwMsg ) )
            {
				int nPageIndex = ControlID - MARKET_SELL_PAGE_0;

				int nPosX, nPosY;
				m_pSellPage[ nPageIndex ]->GetItemIndex( &nPosX, &nPosY );			
				SetItemIndex( nPosX, nPosY );
				if ( nPosX < 0 || nPosY < 0 )
					return;

				SINVENITEM pInvenItem = m_pSellPage[nPageIndex]->GetItem( nPosX, nPosY );
				//if(!pInvenItem )
				//	return;

                //	NOTE
                //		손에 들고 있는 아이템 처리
                if ( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) || CHECK_MOUSEIN_RBUPLIKE( dwMsg ) )
                {
                    SNATIVEID sNativeID = GLGaeaClient::GetInstance().GetCharacter()->GET_HOLD_ITEM().sNativeID;
                    if ( sNativeID != NATIVEID_NULL() )
                    {
                        //	NOTE
                        //		물건 팔기
                        const SITEM* pItemData = GLItemMan::GetInstance().GetItem( sNativeID );
                        if ( pItemData )
                        {
							// 판매 가능 여부
							bool bSale = true;
							{
								//if( !pItemData->BasicOp.IsSALE() || pInvenItem.sItemCustom.IsGM_GENITEM() )
								if( pInvenItem.sItemCustom.IsGM_GENITEM() )
								//if( false == ( holditem::GetHoldItem().IsSale() != NULL ) || true == pInvenItem->sItemCustom.IsGM_GENITEM() )
								{
									bSale = false;
									//holditem::ReleaseHoldItem();
								}

								// 팻카드일 경우
								if( pItemData->sBasicOp.emItemType == ITEM_PET_CARD )
								{
									const SITEMCUSTOM& sHoldItem = pInvenItem.sItemCustom;
									GLPetClient* pMyPet = GLGaeaClient::GetInstance().GetPetClient();
									if( true == pMyPet->IsVALID() && sHoldItem.dwPetID == pMyPet->m_dwPetID )
									{
										bSale = false;
										//holditem::ReleaseHoldItem();
									}
								}
							}

                            if ( RANPARAM::bCheckConfirmSellItem )
                            {
								if( false == bSale )
								{
									DoModal( ID2GAMEINTEXT( "IMPOSSIBLE_SALE_ITEM" ) );
									return;
								}

                                const std::string& strItemName = GLGaeaClient::GetInstance().GetCharacter()->GET_HOLD_ITEM().GETNAME();
                                std::string strCombine = sc::string::format( ID2GAMEINTEXT( "SELL_ITEM_CONFIRM" ), strItemName );
                                DoModal( strCombine.c_str(), MODAL_QUESTION, YESNO, MODAL_SELL_ITEM );
                            }
                            else
                            {
								if( false == bSale )
								{
									CInnerInterface::GetInstance().PrintConsoleText( ID2GAMEINTEXT( "IMPOSSIBLE_SALE_ITEM" ) );
									return;
								}

                                int nPosX = nOUTOFRANGE;
                                int nPosY = nOUTOFRANGE;

                                GetItemIndex( &nPosX, &nPosY );
                                if ( nPosX < 0 || nPosY < 0 )
                                    return;

								GLGaeaClient::GetInstance().GetCharacter()->ReqNpcTo(
                                    m_sCrowID.dwID, GetPageIndex(), nPosX, nPosY );
                            }
                        }
                        else
                        {
                            GASSERT( 0 && "아이템을 찾을 수 없습니다." );
                        }

                        return ;
                    }
                }

                
				if ( pInvenItem.sItemCustom.GetNativeID() != NATIVEID_NULL () )
				{
					int iPage = -1;

					/*
					if( IsCurPageMarketType(GLInventorySale::CROW_SALE_V_2) )
						iPage = m_nPageIndex;
						*/

					CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW(
						pInvenItem.sItemCustom,
						TRUE, TRUE, FALSE, TRUE, FALSE,
						pInvenItem.wPosX, pInvenItem.wPosY,
						GetNPCID() );
				}

                if ( DxInputDevice::GetInstance().GetKeyState( DIK_LCONTROL ) & DXKEY_PRESSED )
                {
                    //		아이템 바로 사기
                    if ( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
                    {
						SNATIVEID sNativeID = GLGaeaClient::GetInstance().GetCharacter()->GET_HOLD_ITEM().GetNativeID();
                        if ( sNativeID != NATIVEID_NULL() )
							return ;
						GLGaeaClient::GetInstance().GetCharacter()->ReqNpcTo(
							m_sCrowID.dwID,
								m_nPageIndex,
								nPosX,
								nPosY );
						return;
                    }
                }

                if ( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
                {
					SNATIVEID sNativeID = GLGaeaClient::GetInstance().GetCharacter()->GET_HOLD_ITEM().GetNativeID();
                    if ( sNativeID != NATIVEID_NULL() )
                        return ;

                    const SITEM* pITEM = GLItemMan::GetInstance().GetItem( pInvenItem.sItemCustom.GetNativeID() );
                    if ( pITEM )
                    {
                        // Note : 겹침이 가능하고 한개씩 파는 아이템의 경우 구입 수량 입력. ( 화살묶음, 부적묶음. )
                        if( pITEM->ISPILE() && ( pITEM->GETAPPLYNUM() == 1 ) )
                        {
							{
								DoModal( ID2GAMEINTEXT( "BUY_PILE_ITEM" ),
									MODAL_INPUT, EDITBOX_NUMBER, MODAL_PILE_ITEM );
							}
                            return;
                        }
                        else
                        {
								const std::string& strItemName = pInvenItem.sItemCustom.GETNAME();
								std::string strCombine = sc::string::format( ID2GAMEINTEXT("BUY_ITEM_CONFIRM"),
									strItemName.c_str() );
								DoModal( strCombine.c_str(),
									MODAL_QUESTION, YESNO, MODAL_BUY_ITEM );		
                            return ;
                        }
                    }
                }

                if ( !(DxInputDevice::GetInstance().GetKeyState( DIK_LALT ) & DXKEY_PRESSED) && CHECK_MOUSEIN_RBUPLIKE( dwMsg ) )
                {
                    //if ( holditem::HaveHoldItem() )
                    //    return ;

                    const SITEM* pItemData = GLItemMan::GetInstance().GetItem( pInvenItem.sItemCustom.GetNativeID() );
                    if ( pItemData )
                    {
                        // Note : 겹침이 가능하고 한개씩 파는 아이템의 경우 구입 수량 입력. ( 화살묶음, 부적묶음. )
                        if( pItemData->ISPILE() && ( pItemData->GETAPPLYNUM() == 1 ) )
                        {
                            DoModal( ID2GAMEINTEXT( "BUY_PILE_ITEM" ),
                                MODAL_INPUT, EDITBOX_NUMBER, MODAL_DIALOGUE_PILE_ITEM );
                            return;
                        }
                        else
                        {
                            // Note : 장바구니에 담기.
                            AddCartList( m_nPageIndex, nPosX, nPosY );
                        }
                    }
                }
            }			

        }
        break;

        case RECOMMEND_ITEM_0:
        case RECOMMEND_ITEM_1:
        case RECOMMEND_ITEM_2:
        case RECOMMEND_ITEM_3:
        case RECOMMEND_ITEM_4:
            {
                int nIndex = ControlID - RECOMMEND_ITEM_0;
                SINVENITEM pInvenItem = m_pRecommendItem[ nIndex ]->GetItemImage(0);

                //if ( !pInvenItem )
                //    return;

                if ( CHECK_MOUSE_IN( dwMsg ) )
                {			
                    if ( pInvenItem.sItemCustom.GetNativeID() != NATIVEID_NULL () )
                    {
						int iPage = -1;

						/*
						if( IsCurPageMarketType(GLInventorySale::CROW_SALE_V_2) )
							iPage = m_nPageIndex;*/

						CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW(
							pInvenItem.sItemCustom,
							TRUE, TRUE, FALSE, TRUE, FALSE,
							pInvenItem.wPosX, pInvenItem.wPosY,
							GetNPCID()/*, iPage*/ );
							
                    }

                    int nPosX, nPosY;
                    nPosX = pInvenItem.wPosX;
                    nPosY = pInvenItem.wPosY;
                    SetItemIndex( nPosX, nPosY );
                    if ( nPosX < 0 || nPosY < 0 )
                        return ;
                }

                if ( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
                {
					SNATIVEID sNativeID = GLGaeaClient::GetInstance().GetCharacter()->GET_HOLD_ITEM().GetNativeID();
                    if ( sNativeID != NATIVEID_NULL() )
                        return;

                    const SITEM* pITEM = GLItemMan::GetInstance().GetItem( pInvenItem.sItemCustom.GetNativeID() );
                    if ( pITEM )
                    {
                        // Note : 겹침이 가능하고 한개씩 파는 아이템의 경우 구입 수량 입력. ( 화살묶음, 부적묶음. )
                        if( pITEM->ISPILE() && ( pITEM->GETAPPLYNUM() == 1 ) )
                        {
							{
								DoModal( ID2GAMEINTEXT( "BUY_PILE_ITEM" ),
									MODAL_INPUT, EDITBOX_NUMBER, MODAL_PILE_ITEM );
							}
                            return;
                        }
                        else
                        {
							{
								const std::string& strItemName = pInvenItem.sItemCustom.GETNAME();
								std::string strCombine = sc::string::format( ID2GAMEINTEXT("BUY_ITEM_CONFIRM"),
									strItemName.c_str() );
								DoModal( strCombine.c_str(),
									MODAL_QUESTION, YESNO, MODAL_BUY_ITEM );							
							}
                            return ;
                        }
                    }
                }

                if ( !(DxInputDevice::GetInstance().GetKeyState( DIK_LALT ) & DXKEY_PRESSED) && CHECK_MOUSEIN_RBUPLIKE( dwMsg ) )
                {
                   // if ( holditem::HaveHoldItem() )
                    //    return ;

                    const SITEM* pItemData = GLItemMan::GetInstance().GetItem( pInvenItem.sItemCustom.GetNativeID() );
                    if ( pItemData )
                    {
                        // Note : 겹침이 가능하고 한개씩 파는 아이템의 경우 구입 수량 입력. ( 화살묶음, 부적묶음. )
                        if( pItemData->ISPILE() && ( pItemData->GETAPPLYNUM() == 1 ) )
                        {
                            DoModal( ID2GAMEINTEXT( "BUY_PILE_ITEM" ),
                                MODAL_INPUT, EDITBOX_NUMBER, MODAL_DIALOGUE_PILE_ITEM );
                            return;
                        }
                        else
                        {
                            // Note : 장바구니에 담기.
                            AddCartList( m_nPageIndex, pInvenItem.wPosX, pInvenItem.wPosY );
                        }
                    }
                }
            }
            break;

        case CART_ITEM_SLOT:
            {
                if ( CHECK_MOUSE_IN ( dwMsg ) )
                {
                    int nPosX = m_pCartItemSlot->GetItemIndex();
                    if (nPosX < 0)
                        return;

                    SINVENITEM pInvenItem = m_pCartItemSlot->GetItemImage( nPosX );

                    {				
                        if (pInvenItem.sItemCustom.GetNativeID() != NATIVEID_NULL())
                        {
							{
								CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW(
									pInvenItem.sItemCustom,
									FALSE, FALSE, FALSE, FALSE, FALSE,
									USHRT_MAX, USHRT_MAX );
							}
                        }
                    }
                }

                if ( !(DxInputDevice::GetInstance().GetKeyState( DIK_LALT ) & DXKEY_PRESSED) && CHECK_MOUSEIN_RBUPLIKE( dwMsg ) )
                {
                    int nPosX = m_pCartItemSlot->GetItemIndex();
                    if ( nPosX < 0 ) return;

					{
						int nCarListSize = static_cast<int>( m_listCart.size() );
						if ( nPosX < nCarListSize )
						{
							DIAL_CART_ITER itr =  m_listCart.begin();
							for ( int i = 0; i < nPosX && i < nCarListSize; i++ ) itr++;

							m_listCart.erase( itr );

							LoadCartList();
						}
					}
                }
            }
            break;

        case MARKET_BUY_BUTTON:
            {
                if ( dwMsg & UIMSG_RNBUTTON_CLICK )
				{
					BuyCartListItem();
				}
            }
            break;

        case DIALOGUE_CLOSE_BUTTON:
            {
                if ( dwMsg & UIMSG_RNBUTTON_CLICK )
                {
					CInnerInterface::GetInstance().HideGroup( DIALOGUE_WINDOW_RENEWAL );
                    //m_pInterface->ItemRepairWindowOpen( false );
                }
            }
            break;

        case CHECK_SELL_R_BTN:
            {
                if ( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
                {
                    SetCheckSellRBtnItem( !RANPARAM::bCheckSellItemRButton );
                }
            }
            break;

        case CHECK_CONFIRM_SELL_ITEM:
            {
                if ( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
                {
                    SetCheckConfirmSellItem( !RANPARAM::bCheckConfirmSellItem );
                }
            }
            break;
        case MARKET_REPAIR_BUTTON:
            if ( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
            {
                //m_pInterface->ItemRepairWindowOpen();
            }

    };

    CUIGroup::TranslateUIMessage( ControlID, dwMsg );
}

void CNPCMarketPage::SetMarketData( const SNATIVEID sNPCID, const DWORD dwNPCGlobalID )
{
    if ( sNPCID != GetNPCID() )
    {
        m_listCart.clear();

        LoadCartList();
    }

    SetNpcActionableNPCID( sNPCID, dwNPCGlobalID );

    const PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData( GetNPCID() );
    if ( pCrowData )
    {
        for ( int i = 0; i < MAX_DIALOGUE_MARKET_TAB; i++ )
        {
            GLInventory& ref_Inventory = *pCrowData->GetSaleInven( i );
			if ( pCrowData->GetSaleInven( i ) )
			{
				//GLInventory* pNpcInven = pCrowData->GetSaleInven( i );
				m_pSellPage[ i ]->UnLoadItemPage();
				m_pSellPage[ i ]->LoadItemPage( ref_Inventory );
				//m_pSellPage[ i ]->LoadItemPage( pNpcInven );
			}
        }
    }	

    SetMarketPage( MARKET_PAGE_0 );

    SetCurInvenMoney();
}

void CNPCMarketPage::SetMarketPage( int nPageIndex )
{
    if ( nPageIndex < 0 || nPageIndex >= MAX_DIALOGUE_MARKET_TAB )
        return;

    m_nPageIndex = nPageIndex;

    for ( int i = 0; i < MAX_DIALOGUE_MARKET_TAB; i++ )
        m_pSellPage[ i ]->SetVisibleSingle( FALSE );

    m_pSellPage[ m_nPageIndex ]->SetVisibleSingle( TRUE );

	SetRecommendItem();

    SetCheckUsableItem( RANPARAM::bCheckUsableItemMarketPage );
    
	{
		SetCheckBuyableItem( RANPARAM::bCheckBuyableItemMarketPage );

		CUIControl* pControl = FindControl( RECOMMEND_CHECK_EXCHANGE_ABLE );

		if( pControl ) {
			pControl->SetVisibleSingle(FALSE);
		}

		pControl = FindControl( RECOMMEND_CHECK_BUYABLE );

		if( pControl ) {
			pControl->SetVisibleSingle(TRUE);
		}

		m_pCheckExchangeableItem->SetVisibleSingle(FALSE);
		m_pUncheckExchangeableItem->SetVisibleSingle(FALSE);
		m_pExchangeableItemText->SetVisibleSingle(FALSE);

		m_pBuyableItemText->SetVisibleSingle( TRUE );
	}

    SetCheckSellRBtnItem( RANPARAM::bCheckSellItemRButton );
    SetCheckConfirmSellItem( RANPARAM::bCheckConfirmSellItem );	

	//m_listCart.clear();
	//m_listExCart.clear();

	LoadCartList();
}

void CNPCMarketPage::SetRecommendItem( )
{
    if ( m_nPageIndex < 0 || m_nPageIndex >= MAX_DIALOGUE_MARKET_TAB )
        return;

    const PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData( GetNPCID() );
    if ( !pCrowData )
        return;

    for ( int i = 0; i < NUM_RECOMMEND_ITEM; i++ )
    {
        m_pRecommendItem[ i ]->ResetItemImage(0);
        m_pRecommendItemText[ i ]->ClearText();
    }

	CUIControl* pRepairBtn = FindControl(MARKET_REPAIR_BUTTON);

	if( pRepairBtn ) pRepairBtn->SetVisibleSingle(TRUE);

	m_pBuyButtonBuyText->SetText( ID2GAMEWORD( "POINT_SHOP_BUY_BUTTON_TEXT" ) );
	m_pBuyButtonPointText->SetVisibleSingle(TRUE);

    GLInventory& ref_Inventory = *pCrowData->GetSaleInven( m_nPageIndex );
    const GLInventory::CELL_MAP* ItemList = ref_Inventory.GetItemList();

    GLInventory::CELL_MAP_CITER iter = ItemList->begin();
    GLInventory::CELL_MAP_CITER iter_end = ItemList->end();

    int nCurRecommendItemIndex = 0;
    int nCurIndex = 0;
    for ( ; iter!=iter_end; ++iter, ++nCurIndex )
    {
        if ( nCurRecommendItemIndex >= NUM_RECOMMEND_ITEM )
            break;

        SINVENITEM* pInvenItem = (*iter).second;
        SITEMCUSTOM& ref_ItemCustom = pInvenItem->sItemCustom;
        const SITEM* pItemData = GLItemMan::GetInstance().GetItem( ref_ItemCustom.GetNativeID() );
        if ( !pItemData )
            continue;

        if ( CheckRecommendItem( pItemData, ref_ItemCustom ) )
        {
            m_pRecommendItem[ nCurRecommendItemIndex ]->SetItemImage( 0, *pInvenItem );

            //m_pRecommendItemText[ nCurRecommendItemIndex ]->AddText( ref_ItemCustom.GETNAME().c_str(), NS_UITEXTCOLOR::WHITE );
			m_pRecommendItemText[ nCurRecommendItemIndex ]->AddOneLineText(ref_ItemCustom.GETNAME().c_str(), TRUE, NS_UITEXTCOLOR::WHITE );

            LONGLONG Price = pCrowData->GetNpcSellPrice( pItemData->sBasicOp.sNativeID.dwID );

			volatile double fSHOP_RATE  = 0;
			volatile LONGLONG lCPrice   = 0;

            if( Price == 0 )
            {
				GLCharacter* pChar = GLGaeaClient::GetInstance().GetCharacter();
				
				if( pChar ) fSHOP_RATE  = pChar->GetBuyRate();

                Price = pItemData->sBasicOp.dwBuyPrice;

				lCPrice = Price * static_cast<LONGLONG>(fSHOP_RATE) / 100;
            }
			else
			{
				lCPrice = Price;
			}

			std::string strMoney = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( lCPrice, 3, "," );

            //std::string strText = sc::string::format( "%d", Price );
            m_pRecommendItemText[ nCurRecommendItemIndex ]->AddText( strMoney.c_str(),
                NS_UITEXTCOLOR::WHITE );

            nCurRecommendItemIndex++;
        }
    }

	SetDiffuseAlphaItemList();
}

bool CNPCMarketPage::CheckRecommendItem(const SITEM* pItemData, const SITEMCUSTOM& sItemCustom, bool bTypeCheck, int iCol, int iRow  )
{
	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
    if ( !pCharacter )
        return false;

    // Info : 착용 레벨 검사.
    if ( RANPARAM::bCheckUsableItemMarketPage )
    {
        WORD wCharLev = pCharacter->m_wLevel;
        if ( !( pItemData->sBasicOp.wReqLevelDW == 0
            || pItemData->sBasicOp.wReqLevelDW <= wCharLev ) )
            return false;
        if ( !( pItemData->sBasicOp.wReqLevelUP == 0
            || wCharLev <= pItemData->sBasicOp.wReqLevelUP ) )
            return false;
    }

    // Info : 착용 스텟 검사
    if ( RANPARAM::bCheckUsableItemMarketPage )
    {
        const SCHARSTATS& rItemStats = pItemData->sBasicOp.sReqStats;
		//2014.9.18 스킬효과의 스텟증가효과도 아이템 장착수치 계산에 포함시키기 위해 수정(CRM#3014)
		const SCHARSTATS& rCharStats = pCharacter->m_sSUMSTATS;

		//	착용조건->힘
        if ( rCharStats.wPow + 3 <= rItemStats.wPow )
            return false;

        //	착용조건->체력
        if ( rCharStats.wStr + 3 <= rItemStats.wStr )
            return false;

        //	착용조건->정신력
        if ( rCharStats.wSpi + 3 <= rItemStats.wSpi)
            return false;

        //	착용조건->민첩성
        if ( rCharStats.wDex + 3 <= rItemStats.wDex )
            return false;

        //	착용조건->지력
        if ( rCharStats.wInt + 3 <= rItemStats.wInt )
            return false;

        //	착용조건->근력
        if ( rCharStats.wSta + 3 <= rItemStats.wSta )
            return false;
    }

    // Info : 착용 클래스 검사
    if ( RANPARAM::bCheckUsableItemMarketPage )
    {
        if( sItemCustom.nidDISGUISE != NATIVEID_NULL() )
        {
            const SITEM* pItemDisguise = GLItemMan::GetInstance().GetItem( sItemCustom.nidDISGUISE );
            if ( !pItemDisguise )
                return false;

            DWORD dwReqCharClass_Disguise = pItemDisguise->sBasicOp.dwReqCharClass;
            DWORD dwReqCharClass = pItemData->sBasicOp.dwReqCharClass;

            if ( !( (dwReqCharClass & pCharacter->m_emClass)
                && (dwReqCharClass_Disguise & pCharacter->m_emClass) ) )
                return false;
        }
        else
        {
            DWORD dwReqCharClass = pItemData->sBasicOp.dwReqCharClass;

            if ( !(dwReqCharClass & pCharacter->m_emClass) )
                return false;
        }
    }

    // Info : 사용 가능 학원 검사
    if ( RANPARAM::bCheckUsableItemMarketPage )
    {
        if ( pItemData->sBasicOp.dwReqSchool != GLSCHOOL_ALL )
        {
            if ( !( pItemData->sBasicOp.dwReqSchool & index2school( pCharacter->m_wSchool ) ) )
                return false;
        }
    }

    // Info : 요구 속성 검사.
    if ( RANPARAM::bCheckUsableItemMarketPage )
    {
        if ( pItemData->sBasicOp.emReqBright != BRIGHT_BOTH )
        {
            if ( pItemData->sBasicOp.emReqBright != pCharacter->GETBRIGHT() )
                return false;
        }
    }

    // Info : 아이템 타입 검사
    if ( bTypeCheck )
    {
        EMITEM_TYPE emItemType = pItemData->sBasicOp.emItemType;

        if ( emItemType == ITEM_ARROW || emItemType == ITEM_CURE
            || emItemType == ITEM_CHARM || emItemType == ITEM_PRIVATEMARKET
            || emItemType == ITEM_CD || emItemType == ITEM_BULLET )
            return false;
    }

    // Info : 스킬 문서일 경우 검사.
    if ( RANPARAM::bCheckUsableItemMarketPage )
    {
        if ( pItemData->sBasicOp.emItemType == ITEM_SKILL )
        {
            SNATIVEID sNativeID = sItemCustom.GetNativeID();
			GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
            const SITEM* pItemData = GLItemMan::GetInstance().GetItem( sNativeID );
            SNATIVEID sSkillID = pItemData->sSkillBookOp.sSkill_ID;

            PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( sSkillID.wMainID, sSkillID.wSubID );
            if ( !pSkill )
                return false;

            //	이미 배운 스킬
            if ( pCharacter->GETLEARNED_SKILL ( sSkillID ) )
                return false;

            //	요구치 정보
            {
                const WORD wLevel = 0;
                SKILL::SLEARN& sLEARN = pSkill->m_sLEARN;
                SKILL::SLEARN_LVL& sLVL = sLEARN.sLVL_STEP[wLevel];

                //	1. 요구보유스킬
                SNATIVEID NeedSkillID = pSkill->m_sLEARN.sSKILL;			
                if ( NeedSkillID != NATIVEID_NULL() )
                {
                    bool bVALID = false;
                    bool bNeedSkillLevel = false;

                    std::string strNeedSkillName;
                    std::string strNeedSkillLevel;

                    PGLSKILL pNeedSkill = GLSkillMan::GetInstance().GetData( NeedSkillID.wMainID, NeedSkillID.wSubID );
                    bVALID = ( 0!=pCharacter->ISLEARNED_SKILL( NeedSkillID ) );

                    //	2. 요구보유스킬레벨
                    if ( 0 < sLVL.dwSKILL_LVL )
                    {                        bNeedSkillLevel = true;

                        SCHARDATA2::SKILL_MAP& map = pCharacter->m_ExpSkills;				
                        SCHARDATA2::SKILL_MAP_ITER iter = map.find( NeedSkillID.dwID );
                        if ( iter != map.end() )
                        {
                            SCHARSKILL& rCharSkill = (*iter).second;

                            //	색 조절
                            bVALID = rCharSkill.wLevel >= sLVL.dwSKILL_LVL;						
                        }
                    }

                    if ( !bVALID )
                        return false;

                    if ( bNeedSkillLevel )
                    {
                        if ( !bVALID )
                            return false;
                    }
                }

                //	3. 요구경험치
                if ( 0 < sLVL.dwSKP )
                {
                    if ( !( pCharacter->m_dwSkillPoint >= sLVL.dwSKP ) )
                        return false;
                }

                //	4. 요구레벨
                if ( 0 < sLVL.dwLEVEL )
                {
                    if ( !( pCharacter->GETLEVEL() >= int(sLVL.dwLEVEL) ) )
                        return false;
                }

                //	Stats
				//2014.9.18 스킬효과의 스텟증가효과도 아이템 장착수치 계산에 포함시키기 위해 수정(CRM#3014)
                //	1. 요구힘
                if ( 0 < sLVL.sSTATS.wPow )
                {
                    if ( !( pCharacter->m_sSUMSTATS.wPow >= sLVL.sSTATS.wPow ) )
                        return false;
                }

                //	2. 요구체력
                if ( 0 < sLVL.sSTATS.wStr )
                {
                    if ( !( pCharacter->m_sSUMSTATS.wStr >= sLVL.sSTATS.wStr ) )
                        return false;
                }

                //	3. 요구정신
                if ( 0 < sLVL.sSTATS.wSpi )
                {
                    if ( !( pCharacter->m_sSUMSTATS.wSpi >= sLVL.sSTATS.wSpi ) )
                        return false;
                }

                //	4. 요구민첩
                if ( 0 < sLVL.sSTATS.wDex )
                {
                    if ( !( pCharacter->m_sSUMSTATS.wDex >= sLVL.sSTATS.wDex ) )
                        return false;
                }

                //	5. 요구지력
                if ( 0 < sLVL.sSTATS.wInt )
                {
                    if ( !( pCharacter->m_sSUMSTATS.wInt >= sLVL.sSTATS.wInt ) )
                        return false;
                }

                //	6. 요구근력
                if ( 0 < sLVL.sSTATS.wSta )
                {
                    if ( !( pCharacter->m_sSUMSTATS.wPow >= sLVL.sSTATS.wSta ) )
                        return false;
                }
            }
        }
    }

    // Info : 구입 가격 검사.
	 if ( RANPARAM::bCheckBuyableItemMarketPage )
	 {
		 const PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData( GetNPCID() );
		 if ( !pCrowData )
			 return false;

		 volatile double fSHOP_RATE = GLGaeaClient::GetInstance().GetCharacter()->GetBuyRate();

		 LONGLONG Price = pCrowData->GetNpcSellPrice( pItemData->sBasicOp.sNativeID.dwID );
		 if( Price == 0 )
		 {
			 LONGLONG BuyPrice = static_cast<LONGLONG>( pItemData->sBasicOp.dwBuyPrice );
			 Price = BuyPrice * static_cast<LONGLONG>(fSHOP_RATE) / 100;
		 }

		 if ( pCharacter->GetInvenMoney() < Price )
			 return false;
	 }
	 	     
    return true;
}

void CNPCMarketPage::SetDiffuseAlphaItemList()
{
    for ( int i = 0; i < SALE_INVEN_Y; i++ )
    {
        for ( int j = 0; j < SALE_INVEN_X; j++ )
        {
            CItemSlot* pItemSlot = m_pSellPage[ m_nPageIndex ]->GetItemSlot( i );
            SINVENITEM pInvenItem = pItemSlot->GetItemImage( j );

            CItemImage* pItemImage = pItemSlot->GetItemImageStruct( j );
            if ( pItemImage )
                pItemImage->SetDiffuseAlpha( 50 );

            const SITEMCUSTOM& sItemCustom = pInvenItem.CustomItem();
            const SITEM* pItemData = GLItemMan::GetInstance().GetItem( sItemCustom.GetNativeID() );
            if ( pItemData )
            {
                if ( CheckRecommendItem( pItemData, sItemCustom, false, j, i ) )
                {
                    CItemImage* pItemImage = pItemSlot->GetItemImageStruct( j );
                    if ( pItemImage )
                        pItemImage->SetDiffuseAlpha( 255 );
                }
            }
        }
    }
}

void CNPCMarketPage::SetCheckUsableItem( bool bCheck )
{
    RANPARAM::bCheckUsableItemMarketPage = bCheck;

    if ( bCheck )
    {
        m_pCheckUsableItem->SetVisibleSingle( TRUE );
        m_pUncheckUsableItem->SetVisibleSingle( FALSE );
    }
    else
    {
        m_pCheckUsableItem->SetVisibleSingle( FALSE );
        m_pUncheckUsableItem->SetVisibleSingle( TRUE );
    }
}

void CNPCMarketPage::SetCheckBuyableItem( bool bCheck )
{
    RANPARAM::bCheckBuyableItemMarketPage = bCheck;

    if ( bCheck )
    {
        m_pCheckBuyableItem->SetVisibleSingle( TRUE );
        m_pUncheckBuyableItem->SetVisibleSingle( FALSE );
    }
    else
    {
        m_pCheckBuyableItem->SetVisibleSingle( FALSE );
        m_pUncheckBuyableItem->SetVisibleSingle( TRUE );
    }
}

void CNPCMarketPage::SetItemIndex( int nPosX, int nPosY )
{
    m_nPosX = nPosX;
    m_nPosY = nPosY;
}

void CNPCMarketPage::GetItemIndex( int* pnPosX, int* pnPosY )
{
    *pnPosX = m_nPosX;
    *pnPosY = m_nPosY;
}

SNATIVEID CNPCMarketPage::GetNPCID()
{
    return CUIControlNpcActionable::GetNPCID();
}

DWORD CNPCMarketPage::GetNPCGlobalID()
{
    return CUIControlNpcActionable::GetNPCGlobalID();
}

int CNPCMarketPage::GetPageIndex()
{
    return m_nPageIndex;
}

const SINVENITEM CNPCMarketPage::GetItem( int nPosX, int nPosY ) const
{
    return m_pSellPage[ m_nPageIndex ]->GetItem( nPosX, nPosY );
}

void CNPCMarketPage::AddCartList( int nPageIndex, int nPosX, int nPosY, WORD wBuyNum /* = 1 */ )
{
	{
		int nCartSize = static_cast<int>( m_listCart.size() );

		if ( nCartSize >= NUM_CART_LIST_ITEM )
		{
			ViewNotifyFullCart();
			return;
		}

		SMARKET_CART_DATA sData( nPageIndex, nPosX, nPosY, wBuyNum );
		m_listCart.push_back( sData );
	}

    LoadCartList();
}

void CNPCMarketPage::LoadCartList()
{
    for ( int i = 0; i < NUM_CART_LIST_ITEM; i++ )
        m_pCartItemSlot->ResetItemImage( i );

    const PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData( GetNPCID() );
    if ( !pCrowData )
        return;

    __int64 nTotalPrice = 0;

	{
		DIAL_CART_ITER itr = m_listCart.begin();
		for ( int i = 0; itr != m_listCart.end(); itr++, i++ )
		{
			SMARKET_CART_DATA& sData = (*itr);
			int nPageIndex = sData.nPageIndex;
			int nPosX = sData.nPosX;
			int nPosY = sData.nPosY;
			SINVENITEM pInvenItem = m_pSellPage[nPageIndex]->GetItem( nPosX, nPosY ) ;
			const SITEM* pItemData = GLItemMan::GetInstance().GetItem( pInvenItem.sItemCustom.sNativeID );
			if ( !pItemData )
				return;

			pInvenItem.sItemCustom.wTurnNum = sData.wBuyNum * pItemData->GETAPPLYNUM();
			m_pCartItemSlot->SetItemImage( i, pInvenItem );

			volatile double fSHOP_RATE = GLGaeaClient::GetInstance().GetCharacter()->GetBuyRate();

			LONGLONG Price = pCrowData->GetNpcSellPrice( pItemData->sBasicOp.sNativeID.dwID );
			if( Price == 0 )
			{
				LONGLONG BuyPrice = static_cast<LONGLONG>( pItemData->sBasicOp.dwBuyPrice );
				Price = BuyPrice * static_cast<LONGLONG>(fSHOP_RATE) / 100;
			}

			Price *= sData.wBuyNum;
			nTotalPrice += Price;
		}
	}

    m_pBuyButtonPointText->ClearText();

    if ( nTotalPrice != 0 )
    {
        m_nTotalPrice = nTotalPrice;

        std::string strTotalPrice = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( m_nTotalPrice, 3, "," );

		__int64 nCurMoney = GLGaeaClient::GetInstance().GetCharacter()->GetInvenMoney();
        if ( nTotalPrice <= nCurMoney )
            m_pBuyButtonPointText->SetText( strTotalPrice.c_str(), NS_UITEXTCOLOR::WHITE );
        else
            m_pBuyButtonPointText->SetText( strTotalPrice.c_str(), NS_UITEXTCOLOR::RED );
    }
}

void CNPCMarketPage::ResetCartList()
{
	{
		m_listCart.clear();
	}

    LoadCartList();
}

__int64 CNPCMarketPage::GetTotalPriceInCart()
{
    return m_nTotalPrice;
}

void CNPCMarketPage::BuyCartListItem()
{
    if ( m_listCart.empty() )
        return;

    DIAL_CART_LIST listTemp = m_listCart;

	SNATIVEID sNPCId = CInnerInterface::GetInstance().GetMarketWindowNPCID();
    DWORD dwNPCGlobalId = CInnerInterface::GetInstance().GetMarketWindowNPCGlobID();

    for ( DIAL_CART_ITER itr = listTemp.begin(); itr != listTemp.end(); ++itr )
    {
        SMARKET_CART_DATA& sData = (*itr);

		GLGaeaClient::GetInstance().GetCharacter()->ReqNpcTo( sNPCId,
            sData.nPageIndex, sData.nPosX, sData.nPosY, sData.wBuyNum );
    }
}

void CNPCMarketPage::ViewNotifyFullCart()
{
    m_fNotifyFullCartTime = fNOTIFY_FULL_CART_TIME;
    m_fNotifyFullCartFadeTime = fNOTIFY_FULL_CART_FADE_TIME;

    m_pNotifyFullCartBack->SetVisibleSingle( TRUE );
    m_pNotifyFullCartText->SetVisibleSingle( TRUE );

    m_pNotifyFullCartBack->SetDiffuseAlpha( 255 );
    m_pNotifyFullCartText->SetDiffuseAlpha( 255 );

    m_pNotifyFullCartText->ClearText();
    m_pNotifyFullCartText->AddText( ID2GAMEINTEXT("POINT_SHOP_NOTIFY_FULL_CART_TEXT0"), NS_UITEXTCOLOR::YELLOW );
    m_pNotifyFullCartText->AddText( ID2GAMEINTEXT("POINT_SHOP_NOTIFY_FULL_CART_TEXT1"), NS_UITEXTCOLOR::WHITE );
}

void CNPCMarketPage::UnViewNotifyFullCart()
{
    m_fNotifyFullCartTime = fNOTIFY_FULL_CART_TIME;
    m_fNotifyFullCartFadeTime = fNOTIFY_FULL_CART_FADE_TIME;

    m_pNotifyFullCartBack->SetVisibleSingle( FALSE );
    m_pNotifyFullCartText->SetVisibleSingle( FALSE );
}

void CNPCMarketPage::ResetCartListInDialogueWindowMarketPage( int nPageIndex, int nPosX, int nPosY )
{
    DIAL_CART_ITER itr;

	{
		itr = m_listCart.begin();
		for ( int nIndex = 0; itr != m_listCart.end(); itr++, nIndex++ )
		{
			SMARKET_CART_DATA& CartItem = (*itr);

			if ( CartItem.nPageIndex == nPageIndex
				&& CartItem.nPosX == nPosX
				&& CartItem.nPosY == nPosY )
			{
				m_pCartItemSlot->ResetItemImage( nIndex );

				m_listCart.erase( itr );
				break;
			}
		}
	}

    LoadCartList();
    SetCurInvenMoney();
}

void CNPCMarketPage::SetCurInvenMoney()
{
    m_pMoneyText->ClearText();
	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
    if ( pCharacter )
    {
        m_nCurInvenMoney = pCharacter->GetInvenMoney();

        CString strTemp = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( m_nCurInvenMoney, 3, "," );

        std::string strMoney = sc::string::format( ID2GAMEINTEXT( "DIALOGUE_WINDOW_CUR_MONEY" ),
            strTemp );
        m_pMoneyText->SetText( strMoney.c_str(), NS_UITEXTCOLOR::WHITE );
    }
}

void CNPCMarketPage::SetCheckSellRBtnItem( bool bCheck )
{
    RANPARAM::bCheckSellItemRButton = bCheck;

    if ( bCheck )
    {
        m_pCheckSellRBtn->SetVisibleSingle( TRUE );
        m_pUncheckSellRBtn->SetVisibleSingle( FALSE );
    }
    else
    {
        m_pCheckSellRBtn->SetVisibleSingle( FALSE );
        m_pUncheckSellRBtn->SetVisibleSingle( TRUE );
    }
}

void CNPCMarketPage::SetCheckConfirmSellItem( bool bCheck )
{
    RANPARAM::bCheckConfirmSellItem = bCheck;

    if ( bCheck )
    {
        m_pCheckConfirmSell->SetVisibleSingle( TRUE );
        m_pUncheckConfirmSell->SetVisibleSingle( FALSE );
    }
    else
    {
        m_pCheckConfirmSell->SetVisibleSingle( FALSE );
        m_pUncheckConfirmSell->SetVisibleSingle( TRUE );
    }
}