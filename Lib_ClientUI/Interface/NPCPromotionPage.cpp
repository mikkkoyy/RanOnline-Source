#include "StdAfx.h"


#include "../Lib_Engine/Common/StringFormat.h"

#include "BasicLineBoxSmart.h"
#include "BasicTextBox.h"
#include "UITextControl.h"
#include "GameTextControl.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"

#include "../Lib_Client/G-Logic/GLItemMan.h"

#include "../Lib_Client/G-Logic/GLGaeaClient.h"

#include "ItemImage.h"
#include "Util/RnButton.h"
#include "InnerInterface.h"
#include "NPCPromotionPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CNPCPromotionPage::CNPCPromotionPage( )
    : m_pPromotionBack(NULL)
    , m_pPromotionItem(NULL)
    , m_pPromotionTextBack(NULL)
    , m_pPromotionText(NULL)
    , m_pBuyButton(NULL)
{
}

CNPCPromotionPage::~CNPCPromotionPage()
{
}

void CNPCPromotionPage::CreateSubControl()
{
    {
        CBasicLineBoxSmart* pLinBoxSmart = new CBasicLineBoxSmart( );
        pLinBoxSmart->CreateSub( this, "DIALOGUE_WINDOW_PROMOTION_BACK", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
        pLinBoxSmart->CreateSubControl( "DIALOGUE_WINDOW_PROMOTION_BACK_LINE" );
        RegisterControl( pLinBoxSmart );
        m_pPromotionBack = pLinBoxSmart;
    }

    {
        CBasicLineBoxSmart* pLinBoxSmart = new CBasicLineBoxSmart();
        pLinBoxSmart->CreateSub( this, "DIALOGUE_WINDOW_PROMOTION_TEXT_BACK", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
        pLinBoxSmart->CreateSubControl( "DIALOGUE_WINDOW_PROMOTION_TEXT_BACK_LINE" );
        RegisterControl( pLinBoxSmart );
        m_pPromotionTextBack = pLinBoxSmart;

		CD3DFontPar* pFont9Shadow = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

        CBasicTextBox* pTextBox = new CBasicTextBox( );
        pTextBox->CreateSub ( this, "DIALOGUE_WINDOW_PROMOTION_TEXT", UI_FLAG_DEFAULT );
        pTextBox->SetFont( pFont9Shadow );
        pTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
        RegisterControl( pTextBox );
        m_pPromotionText = pTextBox;
        m_pPromotionText->SetText( ID2GAMEINTEXT( "DIALOGUE_WINDOW_PROMOTION_TEXT" ), NS_UITEXTCOLOR::WHITE );
    }

    {
        CItemImage* pItemImage = new CItemImage( );
        pItemImage->CreateSub( this, "DIALOGUE_WINDOW_PROMOTION_ITEM_IMAGE", UI_FLAG_DEFAULT, ITEM_IMAGE );
        pItemImage->CreateSubControl();
        RegisterControl( pItemImage );
        m_pPromotionItem = pItemImage;
    }

    {
		CD3DFontPar* pFont10Shadow = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 10, _DEFAULT_FONT_SHADOW_FLAG );

        RnButton* pRnButton = NULL;

        RnButton::CreateArg arg;
        //////////////////////////////////////////////////////////////////////////
        arg.defaultTextureKey = "RNBUTTON_DEFAULT_TEXTURE";
        arg.mouseOverTextureKey = "RNBUTTON_OVER_TEXTURE";
        arg.mouseClickTextureKey = "RNBUTTON_CLICK_TEXTURE";
        //////////////////////////////////////////////////////////////////////////
        arg.pFont = pFont10Shadow;
        arg.text = sc::string::format( "%1%", ID2GAMEWORD( "REBIRTH_WINDOW_BUY_BUTTON_TEXT" ) );
        arg.dwColor = NS_UITEXTCOLOR::WHITE;

        pRnButton = new RnButton();
        pRnButton->CreateSub(this,"DIALOGUE_WINDOW_PROMOTION_BUY_BUTTON",UI_FLAG_DEFAULT,BUY_BUTTON);
        pRnButton->CreateSubControl(arg);
        RegisterControl(pRnButton);
        m_pBuyButton = pRnButton;
    }
}

void CNPCPromotionPage::Update( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
    CUIGroup::Update( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CNPCPromotionPage::TranslateUIMessage( UIGUID ControlID, DWORD dwMsg )
{
    switch ( ControlID )
    {
    case BUY_BUTTON:
        {
            if ( dwMsg & UIMSG_RNBUTTON_CLICK )
                 PointshopOpenPromotionItem();
        }
        break;

    case ITEM_IMAGE:
        {
            if ( CHECK_MOUSE_IN( dwMsg ) )
            {
                SITEMCUSTOM sItemCustom( GLCONST_CHAR::nidDIALOGUE_RECOMMEND_ITEM );

				CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW(
					sItemCustom,
                    FALSE, FALSE, FALSE, TRUE, FALSE,
					USHRT_MAX, USHRT_MAX );
            }
        }
        break;
    };

    CUIGroup::TranslateUIMessage( ControlID, dwMsg );
}

void CNPCPromotionPage::SetRecommendItemImage()
{
    SNATIVEID sItemID = GLCONST_CHAR::nidDIALOGUE_RECOMMEND_ITEM;
    const SITEM* pItem = GLItemMan::GetInstance().GetItem( sItemID );
    if ( !pItem )
    {
        GASSERT ( 0 && "등록되지 않은 ID입니다." );
        return ;
    }

    m_pPromotionItem->SetItem( pItem->sBasicOp.sNativeID );
    m_pPromotionItem->SetVisibleSingle( TRUE );
}

void CNPCPromotionPage::PointshopOpenPromotionItem()
{
	if ( !CInnerInterface::GetInstance().IsVisibleGroup ( ITEMSHOP_WINDOW ) )
	{
		//	ItemShopAuth
		CInnerInterface::GetInstance().ItemShopAuth();
	}
}