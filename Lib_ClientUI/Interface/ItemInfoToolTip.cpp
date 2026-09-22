#include "StdAfx.h"
#include "ItemInfoToolTip.h"

#include "D3DFont.h"
#include "ItemImage.h"
#include "ItemMove.h"
#include "BasicTextButton.h"
#include "UITextControl.h"
#include "GameTextControl.h"
#include "InnerInterface.h"
#include "GLItemMan.h"
#include "GLGaeaClient.h"

#include "BasicLineBoxEx.h"
#include "BasicLineBoxSmart.h"
#include "UIInfoloader.h"
#include "BasicTextBox.h"
#include "ItemRebuildOption.h"
#include "GameTextControl.h"

#include "./BasicTextBox.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "../Lib_ClientUI/Interface/ModalWindow.h"
#include "../Lib_ClientUI/Interface/ModalCallerID.h"

#include "../Lib_Engine/Common/StringFormat.h"

#include "Util/RnButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const float fMOUSEPOINT_GAP1 = 20.0f;
const float fReSizeOffset1 = 10.0f;
const float fResizeEndOffset1 = 8.0f;
const float fResistOffset1 = 10.0f;
const float FResitOffset2 = 0.0f;
const float fBoxItemOffset = 5.0f;

CItemInfoTooltip::CItemInfoTooltip()
	: m_pCharacter(NULL)
	, m_bLinkBoxStyle(false)
	, m_bDisplayResist(false)
    , m_bGroup(false)
	, m_pItemData(NULL)
	, m_pLineBox(NULL)
	, m_pDummyBase(NULL)
	, m_pDummyControl(NULL)
	, m_pDummyNameControl(NULL)
	, m_pDummyLabel5Control(NULL)

	, m_pItemImage(NULL)
	, m_pItemLabel1(NULL)
	, m_pItemLabel2(NULL)
	, m_pItemLabel3(NULL)
	, m_pItemLabel4(NULL)
	, m_pItemLabel4_1(NULL)
	, m_pItemLabel5(NULL)
	, m_pItemSet(NULL)
	, m_pItemLabel6(NULL)

	, m_pWishListLine(NULL)
	, m_pWishListItem(NULL)

	, m_pResistSpirit(NULL)
	, m_pResistFire(NULL)
	, m_pResistIce(NULL)
	, m_pResistElectric(NULL)
	, m_pResistPoison(NULL)
	, m_pResistSpiritText(NULL)
	, m_pResistFireText(NULL)
	, m_pResistIceText(NULL)
	, m_pResistElectricText(NULL)
	, m_pResistPoisonText(NULL)
	, m_vMousePointGap(fMOUSEPOINT_GAP1, fMOUSEPOINT_GAP1)
	, m_pLink_LineBox(NULL)
	, m_pLink_Close(NULL)

	, m_PosX( 0 )
	, m_PosY( 0 )
	, m_vDragPos( 0.0f, 0.0f )
	, m_bBlockMousetracking( false )

	, m_nMouseX( 0 ) 
	, m_nMouseY( 0 )
    //, m_fDurabilityPerformance(1.0f)
	, m_pResistTextGap( 0.0f )
	, m_iPageIndex( -1 )
	, m_pColorBox(NULL)
	, m_bIsDurableTip( false )
{
    /*for ( int i = 0; i < ITEM::SBOX::RANDOM_ITEM_SIZE; i++ )
        m_pBoxItemImages[i] = NULL;

	for ( int i = 0; i < EXCHANGE_ITEM_SIZE; i++ )
		m_pExchangeItemImages[i] = NULL;

	for( int i = 0; i < DURABILITY_APPLY_COUNT; ++i)
		m_fDurabilityValue[i] = 1.0f;*/
}

CItemInfoTooltip::~CItemInfoTooltip()
{
}

void CItemInfoTooltip::CreateSubControl()
{
    {
        CBasicLineBoxSmart* pLinBoxSmart = new CBasicLineBoxSmart();
        pLinBoxSmart->CreateSub( this, "ITEM_INFOR_TOOLTIP_RN", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
        pLinBoxSmart->CreateSubControl( "ITEM_INFOR_TOOLTIP_LINE_RN" );
        RegisterControl( pLinBoxSmart );
        m_pLineBox = pLinBoxSmart;
		m_pLineBox->SetDiffuseAlpha(180);
    }

	if ( m_pLink_LineBox )
	{
		m_pLineBox->SetVisibleSingle( FALSE );
	}

    {
        CUIControl* pControl = new CUIControl();
		pControl->CreateSub( this, std::string(GetControlNameEx()).c_str() );
		pControl->SetVisibleSingle( FALSE );
		RegisterControl( pControl );
		m_pDummyBase = pControl;

		pControl = new CUIControl();
        pControl->CreateSub( this, "ITEM_INFOR_TOOLTIP_RN" );
        pControl->SetVisibleSingle( FALSE );
        RegisterControl( pControl );
        m_pDummyControl = pControl;

		pControl = new CUIControl();
		pControl->CreateSub( this, "ITEM_INFOR_TOOLTIP_ITEM_NAME_RN" );
		pControl->SetVisibleSingle( FALSE );
		RegisterControl( pControl );
		m_pDummyNameControl = pControl;

        pControl = new CUIControl();
        pControl->CreateSub( this, "ITEM_INFOR_TOOLTIP_ITEM_LABEL2_RN" );
        pControl->SetVisibleSingle( FALSE );
        RegisterControl( pControl );
        m_pDummyLabel5Control = pControl;	
    }

    {
        CItemImage* pItemImage = new CItemImage();
        pItemImage->CreateSub( this, "ITEM_INFOR_TOOLTIP_ITEM_IMAGE_RN", UI_FLAG_DEFAULT, ITEM_IMAGE );
        pItemImage->CreateSubControl();
        RegisterControl( pItemImage );
        m_pItemImage = pItemImage;
    }

    {
        //CD3DFontPar* pFont12 = m_pEngineDevice->LoadFont( _DEFAULT_FONT, 12, D3DFONT_BOLD | D3DFONT_SHADOW_EX | D3DFONT_KSC5601 );
		CD3DFontPar* pFont12 = DxFontMan::GetInstance().LoadDxFont( _DEFAULT_FONT, 12, D3DFONT_BOLD | D3DFONT_SHADOW_EX | D3DFONT_KSC5601 );

        CBasicTextBox* pBasicTextBox = new CBasicTextBox();
        pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_ITEM_NAME_RN" );
        pBasicTextBox->SetFont( pFont12 );
        pBasicTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
        RegisterControl( pBasicTextBox );
        m_pItemName = pBasicTextBox;
    }

    //CD3DFontPar* pFont9 = m_pEngineDevice->LoadFont( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_EX_FLAG );
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_EX_FLAG );

    {
        CBasicTextBox* pBasicTextBox = new CBasicTextBox();
        pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_ITEM_LABEL1_RN" );
        pBasicTextBox->SetFont( pFont9 );
        pBasicTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );
        RegisterControl( pBasicTextBox );
        m_pItemLabel1 = pBasicTextBox;
    }

    {
        CBasicTextBox* pBasicTextBox = new CBasicTextBox();
        pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_ITEM_LABEL2_RN" );
        pBasicTextBox->SetFont( pFont9 );
        pBasicTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );
        RegisterControl( pBasicTextBox );
        m_pItemLabel2 = pBasicTextBox;
    }

    {
        CBasicTextBox* pBasicTextBox = new CBasicTextBox();
        pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_ITEM_LABEL2_RN" );
        pBasicTextBox->SetFont( pFont9 );
        pBasicTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );
        RegisterControl( pBasicTextBox );
        m_pItemLabel3 = pBasicTextBox;
    }

    {
        CBasicTextBox* pBasicTextBox = new CBasicTextBox();
        pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_ITEM_LABEL2_RN" );
        pBasicTextBox->SetFont( pFont9 );
        pBasicTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );
        RegisterControl( pBasicTextBox );
        m_pItemLabel4 = pBasicTextBox;
    }

    {
        CBasicTextBox* pBasicTextBox = new CBasicTextBox();
        pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_ITEM_LABEL2_RN" );
        pBasicTextBox->SetFont( pFont9 );
        pBasicTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );
        RegisterControl( pBasicTextBox );
        m_pItemLabel4_1 = pBasicTextBox;
    }
	
	{
		CBasicTextBox* pBasicTextBox = new CBasicTextBox();
		pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_ITEM_LABEL2_RN" );
		pBasicTextBox->SetFont( pFont9 );
		pBasicTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );
		RegisterControl( pBasicTextBox );
		m_pItemSet = pBasicTextBox;
	}

    {
        CBasicTextBox* pBasicTextBox = new CBasicTextBox();
        pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_ITEM_LABEL2_RN" );
        pBasicTextBox->SetFont( pFont9 );
        pBasicTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );
        RegisterControl( pBasicTextBox );
        m_pItemLabel5 = pBasicTextBox;
    }

	{
		CBasicTextBox* pBasicTextBox = new CBasicTextBox();
		pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_ITEM_LABEL2_RN" );
		pBasicTextBox->SetFont( pFont9 );
		pBasicTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );
		RegisterControl( pBasicTextBox );
		m_pItemLabel6 = pBasicTextBox;
	}


	/*{
		CUIControl* pControl = new CUIControl( m_pEngineDevice );
		pControl->CreateSub( this, GetControlNameEx().c_str() );
		pControl->SetDiffuse( NS_UITEXTCOLOR::DARKGRAY );
		pControl->SetUseRender( true );
		pControl->SetVisibleSingle( FALSE );
		RegisterControl( pControl );
		m_pWishListLine = pControl;

		CBasicTextBox* pBasicTextBox = new CBasicTextBox( m_pEngineDevice );
        pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_ITEM_LABEL2_RN" );
        pBasicTextBox->SetFont( pFont9 );
        pBasicTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );
        RegisterControl( pBasicTextBox );
        m_pWishListItem = pBasicTextBox;
	}*/

    {
        CUIControl* pControl = new CUIControl();
        pControl->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_SPIRIT_RN" );
        pControl->SetVisibleSingle( FALSE );
        RegisterControl( pControl );
        m_pResistSpirit = pControl;

        pControl = new CUIControl();
        pControl->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_FIRE_RN" );
        pControl->SetVisibleSingle( FALSE );
        RegisterControl( pControl );
        m_pResistFire = pControl;

        pControl = new CUIControl();
        pControl->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_ICE_RN" );
        pControl->SetVisibleSingle( FALSE );
        RegisterControl( pControl );
        m_pResistIce = pControl;

        pControl = new CUIControl();
        pControl->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_ELECTRIC_RN" );
        pControl->SetVisibleSingle( FALSE );
        RegisterControl( pControl );
        m_pResistElectric = pControl;

        pControl = new CUIControl();
        pControl->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_POISON_RN" );
        pControl->SetVisibleSingle( FALSE );
        RegisterControl( pControl );
        m_pResistPoison = pControl;
    }

    {
        CBasicTextBox* pBasicTextBox = new CBasicTextBox();
        pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_SPIRIT_TEXT_RN" );
        pBasicTextBox->SetFont( pFont9 );
        pBasicTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );
        RegisterControl( pBasicTextBox );
        m_pResistSpiritText = pBasicTextBox;

        pBasicTextBox = new CBasicTextBox();
        pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_FIRE_TEXT_RN" );
        pBasicTextBox->SetFont( pFont9 );
        pBasicTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );
        RegisterControl( pBasicTextBox );
        m_pResistFireText = pBasicTextBox;

        pBasicTextBox = new CBasicTextBox();
        pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_ICE_TEXT_RN" );
        pBasicTextBox->SetFont( pFont9 );
        pBasicTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );
        RegisterControl( pBasicTextBox );
        m_pResistIceText = pBasicTextBox;

        pBasicTextBox = new CBasicTextBox();
        pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_ELECTRIC_TEXT_RN" );
        pBasicTextBox->SetFont( pFont9 );
        pBasicTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );
        RegisterControl( pBasicTextBox );
        m_pResistElectricText = pBasicTextBox;

        pBasicTextBox = new CBasicTextBox();
        pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_POISON_TEXT_RN" );
        pBasicTextBox->SetFont( pFont9 );
        pBasicTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );
        RegisterControl( pBasicTextBox );
        m_pResistPoisonText = pBasicTextBox;
    }

    /*{
        for ( int i = 0; i < ITEM::SBOX::RANDOM_ITEM_SIZE; i++ )
        {
            CItemImage* pItemImage = new CItemImage( m_pGaeaClient, m_pEngineDevice );
            pItemImage->CreateSub( this, "ITEM_INFOR_TOOLTIP_ITEM_IMAGE_RN", UI_FLAG_DEFAULT, BOX_ITEM_IMAGES_START+i );
            pItemImage->CreateSubControl();
            RegisterControl( pItemImage );
            m_pBoxItemImages[i] = pItemImage;
        }

		for ( int i = 0; i < EXCHANGE_ITEM_SIZE; i++ )
		{
			CItemImage* pItemImage = new CItemImage( m_pGaeaClient, m_pEngineDevice );
			pItemImage->CreateSub( this, "ITEM_INFOR_TOOLTIP_ITEM_IMAGE_RN", UI_FLAG_DEFAULT, EXCHANGE_ITEM_IMAGE_START+i );
			pItemImage->CreateSubControl();
			RegisterControl( pItemImage );
			m_pExchangeItemImages[i] = pItemImage;
		}
    }*/

	/*
	{		
		m_pColorBox = new CUIControl(m_pEngineDevice);
		m_pColorBox->CreateSub ( this, "ITEM_MOUSE_OVER" );
		m_pColorBox->SetDiffuse( NS_UITEXTCOLOR::GREEN );
		m_pColorBox->SetVisibleSingle( FALSE );
		RegisterControl ( m_pColorBox );
	}
	*/
}

void CItemInfoTooltip::CreateLinkBoxStyle()
{
	m_bLinkBoxStyle = true;

	m_pLink_LineBox = new CBasicLineBoxSmart();
	m_pLink_LineBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_RN", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pLink_LineBox->CreateSubControl( "ITEM_INFOR_TOOLTIP_LINE_RN" );
	RegisterControl( m_pLink_LineBox );
	m_pLink_LineBox->SetDiffuseAlpha(180);

	CreateCloseButton();
}

void CItemInfoTooltip::CreateCloseButton()
{
	m_pLink_Close = new CBasicButton();	
	m_pLink_Close->CreateSub( this, "ITEM_INFOR_TOOLTIP_CLOSEBUTTON_RN", UI_FLAG_RIGHT, CLOSE_BUTTON );
	m_pLink_Close->CreateFlip( "INFO_DISPLAY_LINK_CLOSE_F", CBasicButton::CLICK_FLIP );
	m_pLink_Close->CreateMouseOver( "INFO_DISPLAY_LINK_CLOSE_OVER" );
	RegisterControl( m_pLink_Close );
}

void CItemInfoTooltip::Update( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	m_nMouseX = x;
	m_nMouseX = y;

	if( m_bBlockMousetracking )
	{
		if( m_PosX != x || m_PosY != y )
		{
			m_PosX = x;
			m_PosY = y;

			if( IsExclusiveSelfControl() )
			{
				SetGlobalPos( D3DXVECTOR2( x - m_vDragPos.x, y - m_vDragPos.y ) );
			}
		}
	}
	else
	{
		AllControlRePos( x, y );
	}
}

void CItemInfoTooltip::TranslateUIMessage( UIGUID ControlID, DWORD dwMsg )
{
	switch ( ControlID )
	{
	case CLOSE_BUTTON:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( UIMSG_LB_UP & dwMsg )
				{
					AddMessageEx ( UIMSG_UIVARTEXT_BUTTON_CLOSE_CLICK );
				}

				AddMessageEx ( UIMSG_UIVARTEXT_BUTTON_CLOSE_MOUSEIN );
			}
		}
		break;
	}

    /*for ( int i = BOX_ITEM_IMAGES_START; i <= BOX_ITEM_IMAGES_END; i++ )
    {

        if ( ControlID == i )
        {
            if ( CHECK_MOUSE_IN( dwMsg ) )
            {
                int nIndex = (i - BOX_ITEM_IMAGES_START);
                SITEMCUSTOM sCUSTOM( m_pBoxItemImages[nIndex]->GetItemID() );

				m_pInterface->SHOW_ITEM_INFO( sCUSTOM, FALSE, FALSE, FALSE, FALSE,FALSE, USHRT_MAX, USHRT_MAX );

				//m_pInterface->ShowItemToolTipMulti( sCUSTOM, m_nMouseX, m_nMouseY );
            }
        }
    }

	for ( int i = EXCHANGE_ITEM_IMAGE_START; i <= EXCHANGE_ITEM_IMAGE_END; i++ )
	{

		if ( ControlID == i )
		{
			if ( CHECK_MOUSE_IN( dwMsg ) )
			{
				int nIndex = (i - EXCHANGE_ITEM_IMAGE_START);
				SITEMCUSTOM sCUSTOM( m_pExchangeItemImages[nIndex]->GetItemID() );

				m_pInterface->SHOW_ITEM_INFO( sCUSTOM, FALSE, FALSE, FALSE, FALSE,FALSE, USHRT_MAX, USHRT_MAX );

			}
		}
	}*/
}

void CItemInfoTooltip::TranslateMessage( DWORD dwMsg )
{
	if( m_bBlockMousetracking )
	{
		if( CHECK_MOUSE_IN( dwMsg ) )
		{
			if( dwMsg & UIMSG_LB_DOWN )
			{
				SetExclusiveControl();

				UIRECT rcPos = GetGlobalPos();
				m_vDragPos.x = m_PosX - rcPos.left;
				m_vDragPos.y = m_PosY - rcPos.top;
			}
			else if( CHECK_LB_UP_LIKE( dwMsg ) )
			{
				ResetExclusiveControl();
			}
		}
		else if( CHECK_LB_UP_LIKE( dwMsg ) )
		{
			ResetExclusiveControl();
		}
	}
}

void CItemInfoTooltip::AllControlRePos( int x, int y )
{
    const UIRECT& rcTextBoxLocalPos = m_pDummyLabel5Control->GetLocalPos();
    const UIRECT& rcLocalPosDummy = m_pDummyControl->GetLocalPos();
    const UIRECT& rcOriginPos = GetLocalPos();

    AlignSubControl( rcOriginPos, rcLocalPosDummy );

	UIRECT rcNewUIPos;

	float fOriginItemNameSizeX = m_pDummyNameControl->GetLocalPos().sizeX;
	long lItemNameLength = m_pItemName->GetLongestLine();
    if ( static_cast<long>(fOriginItemNameSizeX) < lItemNameLength )
    {
        // 아이템 이름 텍스트 박스 크기 설정.
        UIRECT rcNewItemName = m_pItemName->GetLocalPos();
        rcNewItemName.sizeX = static_cast<float>(lItemNameLength);
        m_pItemName->SetLocalPos( rcNewItemName );

        // 아이템 설명 텍스트 박스 크기 설정.
        UIRECT rcNewLabel5 = m_pDummyLabel5Control->GetLocalPos();
        rcNewLabel5.sizeX = static_cast<float>(lItemNameLength);
        m_pItemLabel5->SetLocalPos( rcNewLabel5 );

        // UI 크기 설정.
        rcNewUIPos = rcLocalPosDummy;
        rcNewUIPos.sizeX += lItemNameLength - fOriginItemNameSizeX;
    }
    else
    {
        // 아이템 이름 텍스트 박스 크기 설정.
        UIRECT rcDummyItemName = m_pDummyNameControl->GetLocalPos();
        m_pItemName->SetLocalPos( rcDummyItemName );

        // 아이템 설명 텍스트 박스 크기 설정.
        UIRECT rcNewLabel5 = m_pDummyLabel5Control->GetLocalPos();
        m_pItemLabel5->SetLocalPos( rcNewLabel5 );

        // UI 크기 설정.
        rcNewUIPos = rcLocalPosDummy;
    }

	bool bInterval = false;
    float fPosY = m_pItemLabel1->GetLocalPos().top;

    {
        int nTotalLine = m_pItemLabel1->GetTotalLine();
        const float fAllLine = m_pItemLabel1->CalcMaxHEIGHT( 0, nTotalLine );
        fPosY += fAllLine;

        const UIRECT& rcLabelLP = m_pItemLabel1->GetLocalPos();
        UIRECT rcLabelNewLP = UIRECT( rcLabelLP.left, rcLabelLP.top, rcLabelLP.sizeX, fAllLine );
        m_pItemLabel1->SetLocalPos( rcLabelNewLP );
    }

    {
		if( m_bIsDurableTip )
		{
			fPosY += 30;
		}
        D3DXVECTOR2 vPos;
        vPos.x = m_pItemLabel2->GetLocalPos().left;
        vPos.y = fPosY;
        m_pItemLabel2->SetLocalPos( vPos );

        int nTotalLine = m_pItemLabel2->GetTotalLine();
        const float fAllLine = m_pItemLabel2->CalcMaxHEIGHT( 0, nTotalLine );
        fPosY += fAllLine;

        const UIRECT& rcLabelLP = m_pItemLabel2->GetLocalPos();
        UIRECT rcLabelNewLP = UIRECT( rcLabelLP.left, rcLabelLP.top, rcLabelLP.sizeX, fAllLine );
        m_pItemLabel2->SetLocalPos( rcLabelNewLP );

        if ( nTotalLine )
            bInterval = true;
    }

    {
        if ( bInterval && m_pItemLabel3->GetTotalLine() )
        {
            bInterval = false;
            fPosY += fReSizeOffset1;
        }

        D3DXVECTOR2 vPos;
        vPos.x = m_pItemLabel3->GetLocalPos().left;
        vPos.y = fPosY;
        m_pItemLabel3->SetLocalPos( vPos );

        int nTotalLine = m_pItemLabel3->GetTotalLine();
        const float fAllLine = m_pItemLabel3->CalcMaxHEIGHT( 0, nTotalLine );
        fPosY += fAllLine;

        const UIRECT& rcLabelLP = m_pItemLabel3->GetLocalPos();
        UIRECT rcLabelNewLP = UIRECT( rcLabelLP.left, rcLabelLP.top, rcLabelLP.sizeX, fAllLine );
        m_pItemLabel3->SetLocalPos( rcLabelNewLP );

        if ( nTotalLine )
            bInterval = true;
    }

    {
        if ( bInterval && m_pItemLabel4->GetTotalLine() )
        {
            bInterval = false;
            fPosY += fReSizeOffset1;
        }

        D3DXVECTOR2 vPos;
        vPos.x = m_pItemLabel4->GetLocalPos().left;
        vPos.y = fPosY;
        m_pItemLabel4->SetLocalPos( vPos );

        int nTotalLine = m_pItemLabel4->GetTotalLine();
        const float fAllLine = m_pItemLabel4->CalcMaxHEIGHT( 0, nTotalLine );
        fPosY += fAllLine;

        const UIRECT& rcLabelLP = m_pItemLabel4->GetLocalPos();
        UIRECT rcLabelNewLP = UIRECT( rcLabelLP.left, rcLabelLP.top, rcLabelLP.sizeX, fAllLine );
        m_pItemLabel4->SetLocalPos( rcLabelNewLP );

        if ( nTotalLine )
            bInterval = true;
    }

	{
        if ( bInterval && m_pItemLabel4_1->GetTotalLine() )
        {
            bInterval = false;
            fPosY += fReSizeOffset1;
        }

        D3DXVECTOR2 vPos;
        vPos.x = m_pItemLabel4_1->GetLocalPos().left;
        vPos.y = fPosY;
        m_pItemLabel4_1->SetLocalPos( vPos );

        int nTotalLine = m_pItemLabel4_1->GetTotalLine();
        const float fAllLine = m_pItemLabel4_1->CalcMaxHEIGHT( 0, nTotalLine );
        fPosY += fAllLine;

        const UIRECT& rcLabelLP = m_pItemLabel4_1->GetLocalPos();
        UIRECT rcLabelNewLP = UIRECT( rcLabelLP.left, rcLabelLP.top, rcLabelLP.sizeX, fAllLine );
        m_pItemLabel4_1->SetLocalPos( rcLabelNewLP );

        if ( nTotalLine )
            bInterval = true;
    }
	
	{
		if ( bInterval && m_pItemSet->GetTotalLine() )
		{
			bInterval = false;
			fPosY += fReSizeOffset1;
		}

		D3DXVECTOR2 vPos;
		vPos.x = m_pItemSet->GetLocalPos().left;
		vPos.y = fPosY;
		m_pItemSet->SetLocalPos( vPos );

		int nTotalLine = m_pItemSet->GetTotalLine();
		const float fAllLine = m_pItemSet->CalcMaxHEIGHT( 0, nTotalLine );
		fPosY += fAllLine;

		const UIRECT& rcLabelLP = m_pItemSet->GetLocalPos();
		UIRECT rcLabelNewLP = UIRECT( rcLabelLP.left, rcLabelLP.top, rcLabelLP.sizeX, fAllLine );
		m_pItemSet->SetLocalPos( rcLabelNewLP );

		if ( nTotalLine )
			bInterval = true;
	}

    {
        if ( bInterval && m_pItemLabel5->GetTotalLine() )
        {
            bInterval = false;
            fPosY += fReSizeOffset1;
        }

        D3DXVECTOR2 vPos;
        vPos.x = m_pItemLabel5->GetLocalPos().left;
        vPos.y = fPosY;
        m_pItemLabel5->SetLocalPos( vPos );

        int nTotalLine = m_pItemLabel5->GetTotalLine();
        const float fAllLine = m_pItemLabel5->CalcMaxHEIGHT( 0, nTotalLine );
        fPosY += fAllLine;

        const UIRECT& rcLabelLP = m_pItemLabel5->GetLocalPos();
        UIRECT rcLabelNewLP = UIRECT( rcLabelLP.left, rcLabelLP.top, rcLabelLP.sizeX, fAllLine );
        m_pItemLabel5->SetLocalPos( rcLabelNewLP );

        if ( nTotalLine )
           bInterval = true;
    }
	
	/*if( m_pWishListItem->GetTotalLine() )
	{
		fPosY += fReSizeOffset1;

		UIRECT rcLine;
		rcLine.left = 5.0f;
		rcLine.top = fPosY;
		rcLine.sizeX = rcNewUIPos.sizeX - 10.0f;
		rcLine.sizeY = 1.0f;
		m_pWishListLine->SetLocalPos( rcLine );

		if( bInterval )
        {
            bInterval = false;
            fPosY += fReSizeOffset1;
        }

        D3DXVECTOR2 vPos;
        vPos.x = m_pWishListItem->GetLocalPos().left;
        vPos.y = fPosY;
        m_pWishListItem->SetLocalPos( vPos );

        int nTotalLine = m_pWishListItem->GetTotalLine();
        const float fAllLine = m_pWishListItem->CalcMaxHEIGHT( 0, nTotalLine );
        fPosY += fAllLine;

        const UIRECT& rcLabelLP = m_pWishListItem->GetLocalPos();
        UIRECT rcLabelNewLP = UIRECT( rcLabelLP.left, rcLabelLP.top, rcLabelLP.sizeX, fAllLine );
        m_pWishListItem->SetLocalPos( rcLabelNewLP );

        if ( nTotalLine )
           bInterval = true;
	}*/

	

		{
			if ( bInterval && m_pItemLabel6->GetTotalLine() )
			{
				bInterval = false;
				fPosY += fReSizeOffset1;
			}

			D3DXVECTOR2 vPos;
			vPos.x = m_pItemLabel6->GetLocalPos().left;
			vPos.y = fPosY;
			m_pItemLabel6->SetLocalPos( vPos );

			int nTotalLine = m_pItemLabel6->GetTotalLine();
			const float fAllLine = m_pItemLabel6->CalcMaxHEIGHT( 0, nTotalLine );
			fPosY += fAllLine;

			const UIRECT& rcLabelLP = m_pItemLabel6->GetLocalPos();
			UIRECT rcLabelNewLP = UIRECT( rcLabelLP.left, rcLabelLP.top, rcLabelLP.sizeX, fAllLine );
			m_pItemLabel6->SetLocalPos( rcLabelNewLP );

			if ( nTotalLine )
				bInterval = true;
		}

	



    fPosY += fResizeEndOffset1;

    UIRECT rcLocalNewPos;

    if ( m_bLinkBoxStyle )
    {
        float fAddSizeX = 0.0f;

        if ( m_pLink_Close )
        {
            const UIRECT& rcRect = m_pLink_Close->GetLocalPos();
            fAddSizeX += rcRect.sizeX + 1;
        }

        rcLocalNewPos = UIRECT(
			float(x), float(y),
			rcNewUIPos.sizeX + fAddSizeX, fPosY );
    }
    else
    {
        rcLocalNewPos = UIRECT(
			float(x) + m_vMousePointGap.x, float(y) + m_vMousePointGap.y,
            rcNewUIPos.sizeX, fPosY );
    }

	if ( m_bDisplayResist )
	{
		// 위치 설정.
		{
			const UIRECT& rcLabel3LP = m_pItemLabel3->GetLocalPos();
			const float fResistSizeX = m_pResistFire->GetLocalPos().sizeX + 1.0f;
			const float fResistSizeY = m_pResistFire->GetLocalPos().sizeY;

			int nTotalLine = m_pItemLabel3->GetTotalLine() - 2;
			const float fAllLine = m_pItemLabel3->CalcMaxHEIGHT( 0, nTotalLine );
			
			D3DXVECTOR2 vPos;
			//vPos.x = fResistOffset1 * 6 - fResistSizeX * 10;
			vPos.x = rcLabel3LP.left - 10.0f;
			vPos.y = rcLabel3LP.top + fAllLine + 1.0f;

			vPos.x += fResistOffset1;
            vPos.y += FResitOffset2;
			m_pResistSpirit->SetLocalPos( vPos );
			vPos.x += fResistSizeX;
            vPos.y -= FResitOffset2;
			m_pResistSpiritText->SetLocalPos( vPos );
			vPos.x += fResistSizeX + m_pResistTextGap;

			vPos.x += fResistOffset1;
            vPos.y += FResitOffset2;
			m_pResistFire->SetLocalPos( vPos );
			vPos.x += fResistSizeX;
            vPos.y -= FResitOffset2;
			m_pResistFireText->SetLocalPos( vPos );
			vPos.x += fResistSizeX + m_pResistTextGap;

			vPos.x += fResistOffset1;
            vPos.y += FResitOffset2;
			m_pResistIce->SetLocalPos( vPos );
			vPos.x += fResistSizeX;
            vPos.y -= FResitOffset2;
			m_pResistIceText->SetLocalPos( vPos );
			vPos.x += fResistSizeX + m_pResistTextGap;

			vPos.x += fResistOffset1;
            vPos.y += FResitOffset2;
			m_pResistElectric->SetLocalPos( vPos );
			vPos.x += fResistSizeX;
            vPos.y -= FResitOffset2;
			m_pResistElectricText->SetLocalPos( vPos );
			vPos.x += fResistSizeX + m_pResistTextGap;

			vPos.x += fResistOffset1;
            vPos.y += FResitOffset2;
			m_pResistPoison->SetLocalPos( vPos );
			vPos.x += fResistSizeX;
            vPos.y -= FResitOffset2;
			m_pResistPoisonText->SetLocalPos( vPos );
			vPos.x += fResistSizeX;
		}

		m_pResistSpirit->SetVisibleSingle( TRUE );
		m_pResistFire->SetVisibleSingle( TRUE );
		m_pResistIce->SetVisibleSingle( TRUE );
		m_pResistElectric->SetVisibleSingle( TRUE );
		m_pResistPoison->SetVisibleSingle( TRUE );
	}

    AlignSubControl( rcLocalPosDummy, rcLocalNewPos );

    SetGlobalPos( rcLocalNewPos );
}
void CItemInfoTooltip::DisplayItemInformation( const SITEMCUSTOM& sItemCustom,
                                               const BOOL bShopOpen, const BOOL bInMarket,
                                               const BOOL bInPrivateMarket, const BOOL bUsePreview,
                                               const BOOL bIsWEAR_ITEM, WORD wPosX, WORD wPosY,
                                               SNATIVEID sNpcNativeID/*, int iPageIndex*/ )
{
	m_pCharacter = GLGaeaClient::GetInstance().GetCharacter();
    if ( !m_pCharacter )
		return;
	
    // 포장되어진 물건은 툴팁표시를 위해 포장을 임시로 푼다.;
    SITEMCUSTOM sBoxWrappedItemCustom = sItemCustom;

    m_pItemData = GLItemMan::GetInstance().GetItem( sBoxWrappedItemCustom.sNativeID );

	BOOL bWrapped = FALSE;
	std::string strorigname = "";
	if ( m_pItemData && m_pItemData->sBasicOp.emItemType == ITEM_WRAPPER_BOX && sItemCustom.nidDISGUISE != NATIVEID_NULL() )
	{
		bWrapped = TRUE;
	}

	if ( bWrapped )
	{
		m_pItemData = NULL;
		m_pItemData = GLItemMan::GetInstance().GetItem( sItemCustom.nidDISGUISE );
		sBoxWrappedItemCustom.sNativeID = sItemCustom.nidDISGUISE;
		sBoxWrappedItemCustom.nidDISGUISE = NATIVEID_NULL();
		sBoxWrappedItemCustom.tDISGUISE = 0;
	}

	if ( !m_pItemData )
	{
		bWrapped = FALSE;
		m_pItemData = GLItemMan::GetInstance().GetItem ( sItemCustom.sNativeID );
	}

    m_bShopOpen = (0!=bShopOpen);
    m_bInMarket = (0!=bInMarket);
    m_bInPrivateMarket = (0!= bInPrivateMarket);
    m_bUsePreview = (0!=bUsePreview);
    m_bIsWEAR_ITEM = (0!=bIsWEAR_ITEM);
    m_wPosX = wPosX;
    m_wPosY = wPosY;
    m_sNpcNativeID = sNpcNativeID;
	//m_iPageIndex = iPageIndex;

	m_bIsDurableTip = false;
 
	AllControlRePos((int)wPosX, (int)wPosY);

	SetItemImage( sItemCustom );
	SetItemName( sItemCustom );
    SetInformationLabel1( sBoxWrappedItemCustom );
	SetInformationLabel2( sBoxWrappedItemCustom );//기본, 랜덤 옵션
	SetInformationLabel3( sBoxWrappedItemCustom );//가산효과
    SetInformationLabel4( sBoxWrappedItemCustom );
	SetInformationLabel5( sItemCustom );
	SetInformationLabel6( sItemCustom );
        
    AllControlRePos((int)wPosX, (int)wPosY);	
}

void CItemInfoTooltip::SetItemImage( const SITEMCUSTOM& sItemCustom )
{
    m_pItemImage->ResetItem();

    const SITEM* pItemData = GLItemMan::GetInstance().GetItem( sItemCustom.GetNativeID() );
    if ( !pItemData )
        return;
    m_pItemImage->SetItem( sItemCustom );
}

void CItemInfoTooltip::SetItemName( const SITEMCUSTOM& sItemCustom )
{ 
    m_pItemName->SetTextNoSplit( sItemCustom.GETNAME().c_str(), ITEM_INFOR_COLOR::dwItemRank[ m_pItemData->sBasicOp.emLevel ] );
    //m_pItemName->AddText( strItemName.c_str(), ITEM_INFOR_COLOR::dwItemRank[ m_pItemData->sBasicOp.emLevel ] );
}

void CItemInfoTooltip::SetItemName( const char* pName )
{
	if( pName == NULL ) return;

	m_pItemName->SetTextNoSplit( pName, ITEM_INFOR_COLOR::dwItemRank[ m_pItemData->sBasicOp.emLevel ] );
}
void CItemInfoTooltip::SetInformationLabel1( const SITEMCUSTOM& sItemCustom )
{
    int nIndex;

    nIndex = m_pItemLabel1->AddText( "", NS_UITEXTCOLOR::dwDefault );

    DWORD dwColor;

    // Info : 학원
    {
        if ( m_pItemData->sBasicOp.dwReqSchool==GLSCHOOL_ALL )
        {
            m_pItemLabel1->AddString( nIndex, ID2GAMEWORD( "ACADEMY_NAME", 3 ), NS_UITEXTCOLOR::dwDefault );
            m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
        }
        else
        {
            if ( m_pItemData->sBasicOp.dwReqSchool & index2school(m_pCharacter->m_wSchool) )
                dwColor = NS_UITEXTCOLOR::dwDefault;
            else
                dwColor = NS_UITEXTCOLOR::dwReqWrong;

            if ( m_pItemData->sBasicOp.dwReqSchool & GLSCHOOL_00)
            {
                m_pItemLabel1->AddString( nIndex,
                    GLCONST_CHAR::strSCHOOLNAME[school2index(GLSCHOOL_00)].c_str(),
                    dwColor );

                m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
            }
            if ( m_pItemData->sBasicOp.dwReqSchool & GLSCHOOL_01)
            {
                m_pItemLabel1->AddString( nIndex,
                    GLCONST_CHAR::strSCHOOLNAME[school2index(GLSCHOOL_01)].c_str(),
                    dwColor );

                m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
            }
            if ( m_pItemData->sBasicOp.dwReqSchool & GLSCHOOL_02)
            {
                m_pItemLabel1->AddString( nIndex,
                    GLCONST_CHAR::strSCHOOLNAME[school2index(GLSCHOOL_02)].c_str(),
                    dwColor );

                m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
            }
        }
    }

    // Info : 착용 클래스
    // Info : 착용 클래스
    {
        if( sItemCustom.nidDISGUISE != NATIVEID_NULL() )
        {
            const SITEM* pItemDisguise = GLItemMan::GetInstance().GetItem( sItemCustom.nidDISGUISE );
            if ( !pItemDisguise )   return;

            DWORD dwReqCharClass_Disguise = pItemDisguise->sBasicOp.dwReqCharClass;
            DWORD dwReqCharClass = m_pItemData->sBasicOp.dwReqCharClass;

            if ( (dwReqCharClass&m_pCharacter->m_emClass) && (dwReqCharClass_Disguise&m_pCharacter->m_emClass) )
                dwColor = NS_UITEXTCOLOR::dwDefault;
            else
                dwColor = NS_UITEXTCOLOR::dwReqWrong;

            if( dwReqCharClass )
            {
                if ( (dwReqCharClass==GLCC_ALL_8CLASS) && (dwReqCharClass_Disguise==GLCC_ALL_8CLASS)
                    || (dwReqCharClass == GLCC_MAN_8CLASS) && (dwReqCharClass_Disguise == GLCC_MAN_8CLASS)
                    || (dwReqCharClass == GLCC_WOMAN_8CLASS) && (dwReqCharClass_Disguise == GLCC_WOMAN_8CLASS) )
                {
                    m_pItemLabel1->AddString( nIndex, ID2GAMEWORD("ITEM_ALL_CLASS"), NS_UITEXTCOLOR::dwDefault );
                }
                else
                {
                    bool bDistinction = false;

                    if ( (dwReqCharClass&GLCC_BRAWLER_M) && (dwReqCharClass_Disguise&GLCC_BRAWLER_M)
                        || (dwReqCharClass&GLCC_BRAWLER_W) && (dwReqCharClass_Disguise&GLCC_BRAWLER_W) )
                    {
                        if ( bDistinction )
                            m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        bDistinction = true;

                        m_pItemLabel1->AddString( nIndex,
                            ID2GAMEWORD( "ITEM_SHOP_SEARCH_CLASS", CharClassToClassIndex( GLCC_BRAWLER_M ) ),
                            dwColor );
                    }

                    if ( (dwReqCharClass&GLCC_SWORDSMAN_M)  && (dwReqCharClass_Disguise&GLCC_SWORDSMAN_M)
                        || (dwReqCharClass&GLCC_SWORDSMAN_W)  && (dwReqCharClass_Disguise&GLCC_SWORDSMAN_W) )
                    {
                        if ( bDistinction )
                            m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        bDistinction = true;

                        m_pItemLabel1->AddString( nIndex,
                            ID2GAMEWORD( "ITEM_SHOP_SEARCH_CLASS", CharClassToClassIndex( GLCC_SWORDSMAN_M ) ),
                            dwColor );
                    }

                    if ( (dwReqCharClass&GLCC_ARCHER_M)  && (dwReqCharClass_Disguise&GLCC_ARCHER_M)
                        || (dwReqCharClass&GLCC_ARCHER_W)  && (dwReqCharClass_Disguise&GLCC_ARCHER_W) )
                    {
                        if ( bDistinction )
                            m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        bDistinction = true;

                        m_pItemLabel1->AddString( nIndex,
                            ID2GAMEWORD( "ITEM_SHOP_SEARCH_CLASS",
                            CharClassToClassIndex( GLCC_ARCHER_M ) ),
                            dwColor );
                    }

                    if ( (dwReqCharClass&GLCC_SHAMAN_M)  && (dwReqCharClass_Disguise&GLCC_SHAMAN_M)
                        || (dwReqCharClass&GLCC_SHAMAN_W)  && (dwReqCharClass_Disguise&GLCC_SHAMAN_W) )
                    {
                        if ( bDistinction )
                            m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        bDistinction = true;

                        m_pItemLabel1->AddString( nIndex,
                            ID2GAMEWORD( "ITEM_SHOP_SEARCH_CLASS", CharClassToClassIndex( GLCC_SHAMAN_M ) ),
                            dwColor );
                    }

                    if ( (dwReqCharClass&GLCC_EXTREME_M)  && (dwReqCharClass_Disguise&GLCC_EXTREME_M)
                        || (dwReqCharClass&GLCC_EXTREME_W)  && (dwReqCharClass_Disguise&GLCC_EXTREME_W) )
                    {
                        if ( bDistinction )
                            m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        bDistinction = true;

                        m_pItemLabel1->AddString( nIndex,
                            ID2GAMEWORD( "ITEM_SHOP_SEARCH_CLASS", CharClassToClassIndex( GLCC_EXTREME_M ) ),
                            dwColor );
                    }

                    if ( (dwReqCharClass&GLCC_GUNNER_M)  && (dwReqCharClass_Disguise&GLCC_GUNNER_M)
                        || (dwReqCharClass&GLCC_GUNNER_W)  && (dwReqCharClass_Disguise&GLCC_GUNNER_W) )
                    {
                        if ( bDistinction )
                            m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        bDistinction = true;

                        m_pItemLabel1->AddString( nIndex,
                            ID2GAMEWORD( "ITEM_SHOP_SEARCH_CLASS", CharClassToClassIndex( GLCC_GUNNER_M ) ),
                            dwColor );
                    }

                    /*if ( (dwReqCharClass&GLCC_ASSASSIN_M)  && (dwReqCharClass_Disguise&GLCC_ASSASSIN_M)
                        || (dwReqCharClass&GLCC_ASSASSIN_W)  && (dwReqCharClass_Disguise&GLCC_ASSASSIN_W) )
                    {
                        if ( bDistinction )
                            m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        bDistinction = true;

                        m_pItemLabel1->AddString( nIndex,
                            ID2GAMEWORD( "ITEM_SHOP_SEARCH_CLASS", CharClassToIndex( GLCC_ASSASSIN_M ) ),
                            dwColor );
                    }

					if ( (dwReqCharClass&GLCC_TRICKER_M)  && (dwReqCharClass_Disguise&GLCC_TRICKER_M)
						|| (dwReqCharClass&GLCC_TRICKER_W)  && (dwReqCharClass_Disguise&GLCC_TRICKER_W) )
					{
						if ( bDistinction )
							m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
						bDistinction = true;

						m_pItemLabel1->AddString( nIndex,
							ID2GAMEWORD( "ITEM_SHOP_SEARCH_CLASS", CharClassToIndex( GLCC_TRICKER_M ) ),
							dwColor );
					}

                    if ( (dwReqCharClass&GLCC_ETC_M)  && (dwReqCharClass_Disguise&GLCC_ETC_M)
                        || (dwReqCharClass&GLCC_ETC_W)  && (dwReqCharClass_Disguise&GLCC_ETC_W) )
                    {
                        if ( bDistinction )
                            m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        bDistinction = true;

                        m_pItemLabel1->AddString( nIndex,
                            ID2GAMEWORD( "ITEM_SHOP_SEARCH_CLASS", CharClassToIndex( GLCC_ETC_M ) ),
                            dwColor );
                    }*/
                }
            }
        }
        else
        {
            DWORD dwReqCharClass = m_pItemData->sBasicOp.dwReqCharClass;

            if ( (dwReqCharClass&m_pCharacter->m_emClass) )
                dwColor = NS_UITEXTCOLOR::dwDefault;
            else
                dwColor = NS_UITEXTCOLOR::dwReqWrong;

            if( dwReqCharClass )
            {
                if ( dwReqCharClass==GLCC_ALL_8CLASS
                    || (dwReqCharClass == GLCC_MAN_8CLASS)
                    || (dwReqCharClass == GLCC_WOMAN_8CLASS) )
                {
                    m_pItemLabel1->AddString( nIndex, ID2GAMEWORD("ITEM_ALL_CLASS"), NS_UITEXTCOLOR::dwDefault );
                }
                else
                {
                    bool bDistinction = false;
                    if ( (dwReqCharClass & GLCC_BRAWLER_M) 
                        || (dwReqCharClass & GLCC_BRAWLER_W) )
                    {
                        if ( bDistinction )
                            m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        bDistinction = true;

                        m_pItemLabel1->AddString( nIndex,
                            ID2GAMEWORD( "ITEM_SHOP_SEARCH_CLASS", CharClassToClassIndex( GLCC_BRAWLER_M ) ),
                            dwColor );
                    }

                    if ( (dwReqCharClass & GLCC_SWORDSMAN_M) 
                        || (dwReqCharClass & GLCC_SWORDSMAN_W) )
                    {
                        if ( bDistinction )
                            m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        bDistinction = true;

                        m_pItemLabel1->AddString( nIndex,
                            ID2GAMEWORD( "ITEM_SHOP_SEARCH_CLASS", CharClassToClassIndex( GLCC_SWORDSMAN_M ) ),
                            dwColor );
                    }

                    if ( (dwReqCharClass & GLCC_ARCHER_M)
                        || (dwReqCharClass & GLCC_ARCHER_W) )
                    {
                        if ( bDistinction )
                            m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        bDistinction = true;

                        m_pItemLabel1->AddString( nIndex,
                            ID2GAMEWORD( "ITEM_SHOP_SEARCH_CLASS", CharClassToClassIndex( GLCC_ARCHER_M ) ),
                            dwColor );
                    }

                    if ( (dwReqCharClass & GLCC_SHAMAN_M)
                        || (dwReqCharClass & GLCC_SHAMAN_W) )
                    {
                        if ( bDistinction )
                            m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        bDistinction = true;

                        m_pItemLabel1->AddString( nIndex,
                            ID2GAMEWORD( "ITEM_SHOP_SEARCH_CLASS", CharClassToClassIndex( GLCC_SHAMAN_M ) ),
                            dwColor );
                    }
                    if ( (dwReqCharClass & GLCC_EXTREME_M)
                        || (dwReqCharClass & GLCC_EXTREME_W) )
                    {
                        if ( bDistinction )
                            m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        bDistinction = true;

                        m_pItemLabel1->AddString( nIndex,
                            ID2GAMEWORD( "ITEM_SHOP_SEARCH_CLASS", CharClassToClassIndex( GLCC_EXTREME_M ) ),
                            dwColor );
                    }

                    if ( (dwReqCharClass & GLCC_GUNNER_M)
                        || (dwReqCharClass & GLCC_GUNNER_W) )
                    {
                        if ( bDistinction )
                            m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        bDistinction = true;

                        m_pItemLabel1->AddString( nIndex,
                            ID2GAMEWORD( "ITEM_SHOP_SEARCH_CLASS", CharClassToClassIndex( GLCC_GUNNER_M ) ),
                            dwColor );
                    }

                    if ( (dwReqCharClass & GLCC_ASSASSIN_M)
                        || (dwReqCharClass & GLCC_ASSASSIN_W) )
                    {
                        if ( bDistinction )
                            m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        bDistinction = true;

                        m_pItemLabel1->AddString( nIndex,
                            ID2GAMEWORD( "ITEM_SHOP_SEARCH_CLASS", CharClassToClassIndex( GLCC_ASSASSIN_M ) ),
                            dwColor );
                    }

					if ( (dwReqCharClass & GLCC_TRICKER_M)
						|| (dwReqCharClass & GLCC_TRICKER_W) )
					{
						if ( bDistinction )
							m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
						bDistinction = true;

						m_pItemLabel1->AddString( nIndex,
							ID2GAMEWORD( "ITEM_SHOP_SEARCH_CLASS", CharClassToClassIndex( GLCC_TRICKER_M ) ),
							dwColor );
					}
                }
            }
        }
    }

	// Info : 성별
    {
        if( sItemCustom.nidDISGUISE != NATIVEID_NULL() )
        {
            const SITEM* pItemDisguise = GLItemMan::GetInstance().GetItem( sItemCustom.nidDISGUISE );
            if ( !pItemDisguise )   return;

            DWORD dwReqCharClass_Disguise = pItemDisguise->sBasicOp.dwReqCharClass;
            DWORD dwReqCharClass = m_pItemData->sBasicOp.dwReqCharClass;

            if ( (dwReqCharClass!=GLCC_ALL_6CLASS) || (dwReqCharClass_Disguise!=GLCC_ALL_6CLASS) )
            {
                bool bMan = false;
                bool bWoman = false;

                if ( (dwReqCharClass&GLCC_BRAWLER_M) && (dwReqCharClass_Disguise&GLCC_BRAWLER_M)
                    || (dwReqCharClass&GLCC_SWORDSMAN_M) && (dwReqCharClass_Disguise&GLCC_SWORDSMAN_M)
                    || (dwReqCharClass&GLCC_ARCHER_M) && (dwReqCharClass_Disguise&GLCC_ARCHER_M)
                    || (dwReqCharClass&GLCC_SHAMAN_M) && (dwReqCharClass_Disguise&GLCC_SHAMAN_M)
                    || (dwReqCharClass&GLCC_EXTREME_M) && (dwReqCharClass_Disguise&GLCC_EXTREME_M)
                    || (dwReqCharClass&GLCC_GUNNER_M) && (dwReqCharClass_Disguise&GLCC_GUNNER_M)
                    || (dwReqCharClass&GLCC_ASSASSIN_M) && (dwReqCharClass_Disguise&GLCC_ASSASSIN_M)
					|| (dwReqCharClass&GLCC_TRICKER_M) && (dwReqCharClass_Disguise&GLCC_TRICKER_M) )
                {
                    bMan = true;
                }

                if ( (dwReqCharClass&GLCC_BRAWLER_W) && (dwReqCharClass_Disguise&GLCC_BRAWLER_W)
                    || (dwReqCharClass&GLCC_SWORDSMAN_W) && (dwReqCharClass_Disguise&GLCC_SWORDSMAN_W)
                    || (dwReqCharClass&GLCC_ARCHER_W) && (dwReqCharClass_Disguise&GLCC_ARCHER_W)
                    || (dwReqCharClass&GLCC_SHAMAN_W) && (dwReqCharClass_Disguise&GLCC_SHAMAN_W)
                    || (dwReqCharClass&GLCC_EXTREME_W) && (dwReqCharClass_Disguise&GLCC_EXTREME_W)
                    || (dwReqCharClass&GLCC_GUNNER_W) && (dwReqCharClass_Disguise&GLCC_GUNNER_W)
                    || (dwReqCharClass&GLCC_ASSASSIN_W) && (dwReqCharClass_Disguise&GLCC_ASSASSIN_W)
					|| (dwReqCharClass&GLCC_TRICKER_W) && (dwReqCharClass_Disguise&GLCC_TRICKER_W) )
                {
                    bWoman = true;
                }

                if ( !bMan || !bWoman )
                {
                    // 남, 여 모두 착용 가능하지 않을 때.
                    if ( bMan )
                    {
                        // 남자 착용 가능 할 때.
                        if ( (m_pCharacter->m_emClass & GLCC_BRAWLER_M) 
                            || (m_pCharacter->m_emClass & GLCC_SWORDSMAN_M)
                            || (m_pCharacter->m_emClass & GLCC_ARCHER_M)
                            || (m_pCharacter->m_emClass & GLCC_SHAMAN_M)
                            || (m_pCharacter->m_emClass & GLCC_EXTREME_M)
                            || (m_pCharacter->m_emClass & GLCC_GUNNER_M)
                            || (m_pCharacter->m_emClass & GLCC_ASSASSIN_M)
							|| (m_pCharacter->m_emClass & GLCC_TRICKER_M) )
                            dwColor = NS_UITEXTCOLOR::dwDefault;
                        else
                            dwColor = NS_UITEXTCOLOR::dwReqWrong;

                        m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        m_pItemLabel1->AddString( nIndex, ID2GAMEWORD( "BASIC_SEX", 0 ), dwColor );
                    }
                    else
                    {
                        // 여자 착용 가능 할 때.
                        if ( (m_pCharacter->m_emClass & GLCC_BRAWLER_W) 
                            || (m_pCharacter->m_emClass & GLCC_SWORDSMAN_W)
                            || (m_pCharacter->m_emClass & GLCC_ARCHER_W)
                            || (m_pCharacter->m_emClass & GLCC_SHAMAN_W)
                            || (m_pCharacter->m_emClass & GLCC_EXTREME_W)
                            || (m_pCharacter->m_emClass & GLCC_GUNNER_W)
                            || (m_pCharacter->m_emClass & GLCC_ASSASSIN_W)
							|| (m_pCharacter->m_emClass & GLCC_TRICKER_W) )
                            dwColor = NS_UITEXTCOLOR::dwDefault;
                        else
                            dwColor = NS_UITEXTCOLOR::dwReqWrong;

                        m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        m_pItemLabel1->AddString( nIndex, ID2GAMEWORD( "BASIC_SEX", 1 ), dwColor );
                    }
                }
            }
        }
        else
        {
            DWORD dwReqCharClass = m_pItemData->sBasicOp.dwReqCharClass;

            if ( (dwReqCharClass!=GLCC_ALL_6CLASS) )
            {
                bool bMan = false;
                bool bWoman = false;

                if ( (dwReqCharClass & GLCC_BRAWLER_M) 
                    || (dwReqCharClass & GLCC_SWORDSMAN_M)
                    || (dwReqCharClass & GLCC_ARCHER_M)
                    || (dwReqCharClass & GLCC_SHAMAN_M)
                    || (dwReqCharClass & GLCC_EXTREME_M)
                    || (dwReqCharClass & GLCC_GUNNER_M)
                    || (dwReqCharClass & GLCC_ASSASSIN_M)
					|| (dwReqCharClass & GLCC_TRICKER_M) )
                {
                    bMan = true;
                }

                if ( (dwReqCharClass & GLCC_BRAWLER_W) 
                    || (dwReqCharClass & GLCC_SWORDSMAN_W)
                    || (dwReqCharClass & GLCC_ARCHER_W)
                    || (dwReqCharClass & GLCC_SHAMAN_W)
                    || (dwReqCharClass & GLCC_EXTREME_W)
                    || (dwReqCharClass & GLCC_GUNNER_W)
                    || (dwReqCharClass & GLCC_ASSASSIN_W)
					|| (dwReqCharClass & GLCC_TRICKER_W) )
                {
                    bWoman = true;
                }

                if ( !bMan || !bWoman )
                {
                    // 남, 여 모두 착용 가능하지 않을 때.
                    if ( bMan )
                    {
                        // 남자 착용 가능 할 때.
                        if ( (m_pCharacter->m_emClass & GLCC_BRAWLER_M) 
                            || (m_pCharacter->m_emClass & GLCC_SWORDSMAN_M)
                            || (m_pCharacter->m_emClass & GLCC_ARCHER_M)
                            || (m_pCharacter->m_emClass & GLCC_SHAMAN_M)
                            || (m_pCharacter->m_emClass & GLCC_EXTREME_M)
                            || (m_pCharacter->m_emClass & GLCC_GUNNER_M)
                            || (m_pCharacter->m_emClass & GLCC_ASSASSIN_M)
							|| (m_pCharacter->m_emClass & GLCC_TRICKER_M) )
                            dwColor = NS_UITEXTCOLOR::dwDefault;
                        else
                            dwColor = NS_UITEXTCOLOR::dwReqWrong;

                        m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        m_pItemLabel1->AddString( nIndex, ID2GAMEWORD( "BASIC_SEX", 0 ), dwColor );
                    }
                    else
                    {
                        // 여자 착용 가능 할 때.
                        if ( (m_pCharacter->m_emClass & GLCC_BRAWLER_W) 
                            || (m_pCharacter->m_emClass & GLCC_SWORDSMAN_W)
                            || (m_pCharacter->m_emClass & GLCC_ARCHER_W)
                            || (m_pCharacter->m_emClass & GLCC_SHAMAN_W)
                            || (m_pCharacter->m_emClass & GLCC_EXTREME_W)
                            || (m_pCharacter->m_emClass & GLCC_GUNNER_W)
                            || (m_pCharacter->m_emClass & GLCC_ASSASSIN_W)
							|| (m_pCharacter->m_emClass & GLCC_TRICKER_W) )
                            dwColor = NS_UITEXTCOLOR::dwDefault;
                        else
                            dwColor = NS_UITEXTCOLOR::dwReqWrong;

                        m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
                        m_pItemLabel1->AddString( nIndex, ID2GAMEWORD( "BASIC_SEX", 1 ), dwColor );
                    }
                }
            }
        }
    }

	nIndex = m_pItemLabel1->AddText( "", NS_UITEXTCOLOR::dwDefault );

    // Info : 요구 레벨
    bool bDisplayLevel = false;
    {
        if ( m_pItemData->sBasicOp.wReqLevelDW || m_pItemData->sBasicOp.wReqLevelUP )
        {
            bDisplayLevel = true;

            std::string strTemp = "";
            bool bReqLevel = true;

            strTemp = sc::string::format( "%s %d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION", 0), 
                m_pItemData->sBasicOp.wReqLevelDW );

            if ( m_pItemData->sBasicOp.wReqLevelUP )
            {
                strTemp += sc::string::format( "~%d", m_pItemData->sBasicOp.wReqLevelUP );

                bReqLevel = (m_pItemData->sBasicOp.wReqLevelUP >= m_pCharacter->m_wLevel);
            }

            if ( ( m_pItemData->sBasicOp.wReqLevelDW <= m_pCharacter->m_wLevel ) && bReqLevel )
                dwColor = NS_UITEXTCOLOR::dwDefault;
            else
                dwColor = NS_UITEXTCOLOR::dwReqWrong;

            m_pItemLabel1->AddString( nIndex, strTemp.c_str(), dwColor );
        }
    }

    // Info : 요구 수치
    {
        std::string strTemp = "";

        SCHARSTATS& rItemStats = m_pItemData->sBasicOp.sReqStats;
		//2014.9.18 스킬효과의 스텟증가효과도 아이템 장착수치 계산에 포함시키기 위해 수정(CRM#3014)
		const SCHARSTATS& rCharStats = m_pCharacter->m_sSUMSTATS;
        bool bDistinction = false;
        //	착용조건->힘
        if ( rItemStats.wPow )
        {
            strTemp = sc::string::format( "%s %d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 0 ), rItemStats.wPow );

            if ( rItemStats.wPow <= rCharStats.wPow )
                dwColor = NS_UITEXTCOLOR::dwDefault;
            else
                dwColor = NS_UITEXTCOLOR::dwReqWrong;

            if ( bDisplayLevel )
            {
                bDisplayLevel = false;
                m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
            }

            if ( bDistinction )
                m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
            bDistinction = true;

            m_pItemLabel1->AddString( nIndex, strTemp.c_str(), dwColor );
        }
        //	착용조건->체력
        if ( rItemStats.wStr )
        {
            strTemp = sc::string::format("%s %d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 1 ), rItemStats.wStr );

            if ( rItemStats.wStr <= rCharStats.wStr )
                dwColor = NS_UITEXTCOLOR::dwDefault;
            else
                dwColor = NS_UITEXTCOLOR::dwReqWrong;

            if ( bDisplayLevel )
            {
                bDisplayLevel = false;
                m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
            }

            if ( bDistinction )
                m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
            bDistinction = true;

            m_pItemLabel1->AddString( nIndex, strTemp.c_str(), dwColor );
        }
        //	착용조건->정신력
        if ( rItemStats.wSpi)
        {
            strTemp = sc::string::format("%s %d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 2 ), rItemStats.wSpi );

            if ( rItemStats.wSpi <= rCharStats.wSpi )
                dwColor = NS_UITEXTCOLOR::dwDefault;
            else
                dwColor = NS_UITEXTCOLOR::dwReqWrong;

            if ( bDisplayLevel )
            {
                bDisplayLevel = false;
                m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
            }

            if ( bDistinction )
                m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
            bDistinction = true;

            m_pItemLabel1->AddString( nIndex, strTemp.c_str(), dwColor );
        }
        //	착용조건->민첩성
        if ( rItemStats.wDex )
        {
            strTemp = sc::string::format("%s %d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 3 ), rItemStats.wDex );

            if ( rItemStats.wDex <= rCharStats.wDex )
                dwColor = NS_UITEXTCOLOR::dwDefault;
            else
                dwColor = NS_UITEXTCOLOR::dwReqWrong;

            if ( bDisplayLevel )
            {
                bDisplayLevel = false;
                m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
            }

            if ( bDistinction )
                m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
            bDistinction = true;

            m_pItemLabel1->AddString( nIndex, strTemp.c_str(), dwColor );
        }
        //	착용조건->지력
        if ( rItemStats.wInt  )
        {
            strTemp = sc::string::format("%s %d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 4 ), rItemStats.wInt );

            if ( rItemStats.wInt <= rCharStats.wInt )
                dwColor = NS_UITEXTCOLOR::dwDefault;
            else
                dwColor = NS_UITEXTCOLOR::dwReqWrong;

            if ( bDisplayLevel )
            {
                bDisplayLevel = false;
                m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
            }

            if ( bDistinction )
                m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
            bDistinction = true;

            m_pItemLabel1->AddString( nIndex, strTemp.c_str(), dwColor );
        }
        //	착용조건->근력
        if ( rItemStats.wSta )
        {
            strTemp = sc::string::format("%s %d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 5 ), rItemStats.wSta );

            if ( rItemStats.wSta <= rCharStats.wSta )
                dwColor = NS_UITEXTCOLOR::dwDefault;
            else
                dwColor = NS_UITEXTCOLOR::dwReqWrong;

            if ( bDisplayLevel )
            {
                bDisplayLevel = false;
                m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
            }

            if ( bDistinction )
                m_pItemLabel1->AddString( nIndex, "/", NS_UITEXTCOLOR::dwDefault );
            bDistinction = true;

            m_pItemLabel1->AddString( nIndex, strTemp.c_str(), dwColor );
        }
    }
}

void CItemInfoTooltip::SetInformationLabel2( const SITEMCUSTOM& sItemCustom )
{
    std::string strTemp = "";

	// 코스튬 부착 아이템 옵션
	const ITEM::SSUIT* psDisguiseSuit = NULL;
	const SITEM* pDisguise = GLItemMan::GetInstance().GetItem( sItemCustom.nidDISGUISE );
	if( NULL != pDisguise )
		psDisguiseSuit = &pDisguise->sSuitOp;

    if ( m_bIsWEAR_ITEM )
        m_pItemLabel2->AddText( ID2GAMEWORD("WEAR_ITEM"), ITEM_INFOR_COLOR::dwEquipCostum );

    // Info : 타입.
	// 내구도 팁이 아닐시에만
	if( !m_bIsDurableTip )
    {
        switch ( m_pItemData->sBasicOp.emItemType )
        {
        case ITEM_SUIT:
            {
                if ( m_pItemData->sSuitOp.emSuit == SUIT_HANDHELD )
                {
                    // 무기 일때.
					/*if ( GLUseFeatures::GetInstance().IsUsingDataIntegrate() )
					{
						strTemp = sc::string::format( "%s(%s)",
							ID2GAMEWORD( "ITEM_BASIC_INFO", 11 ),
							CGameTextMan::GetInstance().GetCommentText("ITEMATTACK", m_pItemData->sSuitOp.emAttack).GetString() );
					}
					else*/
					{
						strTemp = sc::string::format( "%s(%s)",
							ID2GAMEWORD( "ITEM_BASIC_INFO", 11 ),
							COMMENT::ITEMATTACK[ m_pItemData->sSuitOp.emAttack ] );
					}
                    int nIndex = m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault );

					if ( sItemCustom.nidDISGUISE != NATIVEID_NULL() )
                    {
                        SITEM* pDisguiseData = GLItemMan::GetInstance().GetItem( sItemCustom.nidDISGUISE );
                        if ( !pDisguiseData )   return;

						//todo cndev
                        strTemp = sc::string::format( "%s :%s", ID2GAMEWORD( "ITEM_BASIC_INFO", 3 ), pDisguiseData->GetName() );			
                        m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwEquipCostum );

                        /*if ( pDisguiseData->IsColorChange() )
                        {
                            strTemp = sc::string::format( "(%s %s)",
                                ID2GAMEWORD( "ITEM_BASIC_INFO", 3 ),
                                ID2GAMEWORD ( "ITEM_SUIT_COLOR_CHANGE", 0 ) );

                            m_pItemLabel2->AddString( nIndex,
                                strTemp.c_str(), ITEM_INFOR_COLOR::dwEquipCostum );
                        }
                        else*/
                        {
                            strTemp = sc::string::format( "(%s)",
                                ID2GAMEWORD( "ITEM_BASIC_INFO", 3 ) );

                            m_pItemLabel2->AddString( nIndex,
                                strTemp.c_str(), ITEM_INFOR_COLOR::dwEquipCostum );
                        }
                    }
                    //else if ( m_pItemData->sBasicOp.dwFlags & ITEM_DISGUISE )
					else if ( m_pItemData->sBasicOp.IsDISGUISE() )
                    {
                        /*if ( m_pItemData->IsColorChange() )
                        {
                            strTemp = sc::string::format( "(%s %s)",
                                ID2GAMEWORD( "ITEM_BASIC_INFO", 3 ),
                                ID2GAMEWORD ( "ITEM_SUIT_COLOR_CHANGE", 0 ) );

                            m_pItemLabel2->AddString( nIndex,
                                strTemp.c_str(), ITEM_INFOR_COLOR::dwEquipCostum );
                        }
                        else*/
                        {
                            strTemp = sc::string::format( "(%s)",
                                ID2GAMEWORD( "ITEM_BASIC_INFO", 3 ) );

                            m_pItemLabel2->AddString( nIndex,
                                strTemp.c_str(), ITEM_INFOR_COLOR::dwEquipCostum );
                        }
                    }
                }
                else
                {
                    // 무기가 아닌 복장류 일때.
					/*if ( m_pItemData->sSuitOp.IsSTATE(EMITEM_SUIT_UNIQUE) == true )
					{
						
						if ( GLUseFeatures::GetInstance().IsUsingDataIntegrate() )
						{
							strTemp = sc::string::format( "%s(%s)",
								CGameTextMan::GetInstance().GetCommentText("ITEMSUIT", m_pItemData->sSuitOp.emSuit).GetString(),
								ID2GAMEWORD("ITEM_BASIC_INFO", 19));
						}
						else
						{
							strTemp = sc::string::format( "%s(%s)",
								COMMENT::ITEMSUIT[ m_pItemData->sSuitOp.emSuit ].c_str(),
								ID2GAMEWORD("ITEM_BASIC_INFO", 19));
						}
					}
					else*/
					{
						/*if ( GLUseFeatures::GetInstance().IsUsingDataIntegrate() )
						{
							strTemp = sc::string::format( "%s",
								CGameTextMan::GetInstance().GetCommentText("ITEMSUIT", m_pItemData->sSuitOp.emSuit).GetString());
						}
						else*/
						{
							strTemp = sc::string::format( "%s",
								COMMENT::ITEMSUIT[ m_pItemData->sSuitOp.emSuit ].c_str());
						}
					}
                    int nIndex = m_pItemLabel2->AddText(strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault);				
					
                    // 코스튬 체크.
                    if ( sItemCustom.nidDISGUISE != NATIVEID_NULL() )
                    {
                        SITEM* pDisguiseData = GLItemMan::GetInstance().GetItem( sItemCustom.nidDISGUISE );
                        if ( !pDisguiseData )   return;

						//todo cndev
                        strTemp = sc::string::format( "%s :%s", ID2GAMEWORD( "ITEM_BASIC_INFO", 3 ), pDisguiseData->GetName() );			
                        m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwEquipCostum );

                        /*if ( pDisguiseData->IsColorChange() )
                        {
                            strTemp = sc::string::format( "(%s %s)",
                                ID2GAMEWORD( "ITEM_BASIC_INFO", 3 ),
                                ID2GAMEWORD ( "ITEM_SUIT_COLOR_CHANGE", 0 ) );

                            m_pItemLabel2->AddString( nIndex,
                                strTemp.c_str(), ITEM_INFOR_COLOR::dwEquipCostum );
                        }
                        else*/
                        {
                            strTemp = sc::string::format( "(%s)",
                                ID2GAMEWORD( "ITEM_BASIC_INFO", 3 ) );

                            m_pItemLabel2->AddString( nIndex,
                                strTemp.c_str(), ITEM_INFOR_COLOR::dwEquipCostum );
                        }
                    }
                    //else if ( m_pItemData->sBasicOp.dwFlags & ITEM_DISGUISE )
					else if ( m_pItemData->sBasicOp.IsDISGUISE() )
                    {
                        /*if ( m_pItemData->IsColorChange() )
                        {
                            strTemp = sc::string::format( "(%s %s)",
                                ID2GAMEWORD( "ITEM_BASIC_INFO", 3 ),
                                ID2GAMEWORD ( "ITEM_SUIT_COLOR_CHANGE", 0 ) );

                            m_pItemLabel2->AddString( nIndex,
                                strTemp.c_str(), ITEM_INFOR_COLOR::dwEquipCostum );
                        }
                        else*/
                        {
                            strTemp = sc::string::format( "(%s)",
                                ID2GAMEWORD( "ITEM_BASIC_INFO", 3 ) );

                            m_pItemLabel2->AddString( nIndex,
                                strTemp.c_str(), ITEM_INFOR_COLOR::dwEquipCostum );
                        }
                    }
                }
            }
            break;
		case ITEM_MYSTERY_BOX:
			{
				std::string strBasicType;
				/*if ( GLUseFeatures::GetInstance().IsUsingDataIntegrate() )
				{
					strBasicType = m_pItemData->BasicType() < ITEM_NSIZE ? CGameTextMan::GetInstance().GetCommentText("ITEMTYPE", m_pItemData->BasicType()).GetString(): "";
				}
				else*/
				{
					strBasicType = COMMENT::ItemTypeName( m_pItemData->sBasicOp.emItemType ).c_str();
				}
				strBasicType.append(sItemCustom.isBoxLock ? CGameTextMan::GetInstance().GetGameWord("LOCKBOX_TOOLTIP_LOCK", 0).GetString() : CGameTextMan::GetInstance().GetGameWord("LOCKBOX_TOOLTIP_LOCK", 1).GetString());
				m_pItemLabel2->AddText( strBasicType.c_str(), ITEM_INFOR_COLOR::dwDefault );				
			}
			break;

        default:
            {
				/*if ( GLUseFeatures::GetInstance().IsUsingDataIntegrate() )
				{
					m_pItemLabel2->AddText( m_pItemData->BasicType() < ITEM_NSIZE ? CGameTextMan::GetInstance().GetCommentText("ITEMTYPE", m_pItemData->BasicType()).GetString():"",
						ITEM_INFOR_COLOR::dwDefault );
				}
				else*/
				{
					//CString strTemp1;
					//std::string strTemp1 = );
					m_pItemLabel2->AddText( COMMENT::ItemTypeName( m_pItemData->sBasicOp.emItemType ).c_str(),
						ITEM_INFOR_COLOR::dwDefault );
				}
            }
        }
    }

	GLCharacter* const pChar = GLGaeaClient::GetInstance().GetCharacter();
	EMCHARINDEX charIdx = GLCI_NUM_8CLASS;
	int nLevel = -1;
	if ( pChar )
	{
		charIdx = pChar->GETCHARINDEX();
		nLevel = pChar->GETLEVEL();
	}

    // Info : 내구도
    /*{
        // 내구도 기능 사용 여부
        if ( GLUseFeatures::GetInstance().IsUsingItemDurability() )
        {   
            if ( m_pItemData->BasicType() == ITEM_SUIT && !m_bIsDurableTip )
            {
                // 내구도가 설정된 장비라면
                if( ItemDurability::GetInstance().IsUse(sItemCustom) )
                {
                    CString strDurability;
                    strDurability.Format( "%s :", ID2GAMEWORD( "ITEM_BASIC_INFO", 18 ) );
                    int nIndex = m_pItemLabel2->AddText( strDurability.GetString(), ITEM_INFOR_COLOR::dwDefault );
					
					// 내구도 컬러
					DWORD dwColor = ITEM_INFOR_COLOR::dwDefault;
					if( m_fDurabilityPerformance < 1.0f && m_fDurabilityPerformance > 1.0f )
						dwColor = ItemDurability::GetInstance().GetTextColor(sItemCustom, dwColor);

                    strDurability.Format("%d/%d (%d%%)", (int)ItemDurability::GetInstance().Get(sItemCustom), (int)ItemDurability::GetInstance().GetMax(sItemCustom), ItemDurability::GetInstance().GetPercent(sItemCustom) );
                    m_pItemLabel2->AddString( nIndex, strDurability.GetString(), dwColor );
                }
            }
        }
    }*/
	
    // Info : 공격
    {		
        //	공격력
		
		GLDWDATA &sDAMAGE = sItemCustom.GETDAMAGE();
		//const SITEM pTempITEM = *GLogicData::GetInstance().GetItem(sItemCustom.GetNativeID()); 
		//SITEM* pTempITEM = *GLItemMan::GetInstance().GetItem ( sItemCustom.sNativeID );
		const SITEM* pTempITEM = GLItemMan::GetInstance().GetItem(sItemCustom.sNativeID); 

		/*int nMinDmg = sDAMAGE.dwLow; 
		int nMaxDmg = sDAMAGE.dwHigh;
		int nRndOptDamage = sDAMAGE.dwLow - pTempITEM->sSuitOp.gdDamage.dwLow;
		int nExtraValue = sItemCustom.GETGRADE_DAMAGE();
		BYTE uGRADE = sItemCustom.GETGRADE(EMGRINDING_DAMAGE);*/

		int nMinDmg = static_cast<int>( pTempITEM->sSuitOp.gdDamage.dwLow ); 
		int nMaxDmg = static_cast<int>( pTempITEM->sSuitOp.gdDamage.dwHigh );
		int nRndOptDamage = static_cast<int>( sDAMAGE.dwLow - pTempITEM->sSuitOp.gdDamage.dwLow );
		int nExtraValue = static_cast<int>( sItemCustom.GETGRADE_DAMAGE() );
		BYTE uGRADE = sItemCustom.GETGRADE(EMGRINDING_DAMAGE);

		DWORD dwDefault = ITEM_INFOR_COLOR::dwDefault;
		DWORD dwInchant1 = ITEM_INFOR_COLOR::dwInchant1;
		DWORD dwInchant2 = ITEM_INFOR_COLOR::dwInchant2;
		
		//VAR_PARAM 함수에서 1 이하일때 1로 잡아주는 코드가 있어서 튤팁에서 조건 검사를 한번 더 해줘야함
		if ( nMinDmg || nMaxDmg || nRndOptDamage || nExtraValue || uGRADE )
		{
			DWORD dwColor = ITEM_INFOR_COLOR::dwDefault;
			if( NULL != psDisguiseSuit )
			{
				if( 0 != psDisguiseSuit->gdDamage.dwNow && 0 != psDisguiseSuit->gdDamage.dwMax )
					dwColor = ITEM_INFOR_COLOR::dwAddDisguise;
			}

			if ( uGRADE )
			{
				strTemp = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO", 0) );
				int nIndex = m_pItemLabel2->AddText( strTemp.c_str(), dwColor );
				
				strTemp = sc::string::format( "%d", nMinDmg );
				m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwColor );
				
				if(nRndOptDamage > 0 )
				{				
					strTemp = sc::string::format( "(+%d)", nRndOptDamage );
					m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}
				else if(nRndOptDamage < 0)
				{				
					strTemp = sc::string::format( "(%d)", nRndOptDamage );
					m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}

				if ( nExtraValue > 0 )
					strTemp = sc::string::format( "(+%d)", nExtraValue );
				else if ( nExtraValue < 0 )
					strTemp = sc::string::format( "(%d)", nExtraValue );
				m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwInchant2 );

				m_pItemLabel2->AddString( nIndex, " ~ ", dwColor );
				
				strTemp = sc::string::format( "%d", nMaxDmg );
				m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwColor );
				
				if(nRndOptDamage > 0)
				{			
					strTemp = sc::string::format( "(+%d)", nRndOptDamage );
					m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}
				else if(nRndOptDamage < 0)
				{				
					strTemp = sc::string::format( "(%d)", nRndOptDamage );
					m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}

				if ( nExtraValue > 0 )
					strTemp = sc::string::format( "(+%d)", nExtraValue );
				else if ( nExtraValue < 0 )
					strTemp = sc::string::format( "(%d)", nExtraValue );
				m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwInchant2 );
				
			}
			else
			{
				strTemp = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO", 0) );
				int nIndex = m_pItemLabel2->AddText( strTemp.c_str(), dwColor );
				
				strTemp = sc::string::format( "%d", nMinDmg );
				m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwColor );
				
				if(nRndOptDamage > 0)
				{			
					strTemp = sc::string::format( "(+%d)", nRndOptDamage );
					m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}
				else if(nRndOptDamage < 0)
				{				
					strTemp = sc::string::format( "(%d)", nRndOptDamage );
					m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}

				m_pItemLabel2->AddString( nIndex, " ~ ", dwColor );
				
				strTemp = sc::string::format( "%d", nMaxDmg );
				m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwColor );

				if(nRndOptDamage > 0)
				{			
					strTemp = sc::string::format( "(+%d)", nRndOptDamage );
					m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}
				else if(nRndOptDamage < 0)
				{				
					strTemp = sc::string::format( "(%d)", nRndOptDamage );
					m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}
			}
		}
	}
	//

	// Info : 방어
	{
		//const SITEM pTempITEM = *GLogicData::GetInstance().GetItem(sItemCustom.GetNativeID()); 
		//SITEM* pTempITEM = GLItemMan::GetInstance().GetItem ( sItemCustom.sNativeID );
		const SITEM* pTempITEM = GLItemMan::GetInstance().GetItem(sItemCustom.sNativeID); 
		//여기서 해당 아이템 정보가져옴

		//	방어력
		/*int nBasicDef = sItemCustom.getdefense();
		int nRndOptDef = sItemCustom.getdefense() - pTempITEM->sSuitOp.nDefense;
		int nExtraValue = sItemCustom.GETGRADE_DEFENSE();
		BYTE uGRADE = sItemCustom.GETGRADE(EMGRINDING_DEFENSE);*/

		int nBasicDef = static_cast<int>( pTempITEM->sSuitOp.nDefense);
		int nRndOptDef = static_cast<int>( sItemCustom.getdefense() - pTempITEM->sSuitOp.nDefense);
		int nExtraValue = sItemCustom.GETGRADE_DEFENSE();
		BYTE uGRADE = sItemCustom.GETGRADE(EMGRINDING_DEFENSE);

		DWORD dwDefault = ITEM_INFOR_COLOR::dwDefault;
		DWORD dwInchant1 = ITEM_INFOR_COLOR::dwInchant1;
		DWORD dwInchant2 = ITEM_INFOR_COLOR::dwInchant2;
		
			//내구도 적용 시

		DWORD dwColor = NS_UITEXTCOLOR::dwDefault;
		if( NULL != psDisguiseSuit )
		{
			if( 0 != psDisguiseSuit->nDefense )
				dwColor = NS_UITEXTCOLOR::GREEN;
		}
		
		if ( nBasicDef || uGRADE || nRndOptDef )
		{
			if ( uGRADE )
			{
				strTemp = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO", 1) );
				int nIndex = m_pItemLabel2->AddText( strTemp.c_str(), dwColor );

				strTemp = sc::string::format( "%d", nBasicDef );
				m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwColor );

				if(nRndOptDef > 0)
				{
					strTemp = sc::string::format( "(+%d)", nRndOptDef );
					m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}
				else if(nRndOptDef < 0)
				{
					strTemp = sc::string::format( "(%d)", nRndOptDef );
					m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}

				if ( nExtraValue > 0 )
				{
					strTemp = sc::string::format( "(+%d)", nExtraValue );
					m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwInchant2 );
				}
				else if ( nExtraValue < 0 )
				{
					strTemp = sc::string::format( "(%d)", nExtraValue );
					m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwInchant2 );
				}
			}
			else
			{
				strTemp = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO", 1) );
				int nIndex = m_pItemLabel2->AddText( strTemp.c_str(), dwColor );

				strTemp = sc::string::format( "%d", nBasicDef );
				m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwColor );

				if(nRndOptDef > 0)
				{
					strTemp = sc::string::format( "(+%d)", nRndOptDef );
					m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}
				else if(nRndOptDef < 0)
				{
					strTemp = sc::string::format( "(%d)", nRndOptDef );
					m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}
			}
		}
		
	}


	// Info : 명중률, 회피율, 소모 SP
	{
		DWORD dwDefault = ITEM_INFOR_COLOR::dwDefault;
		DWORD dwRndOptColor = ITEM_INFOR_COLOR::dwInchant1;
		
		const SITEM* pTempITEM = GLItemMan::GetInstance().GetItem(sItemCustom.sNativeID); 

		//	명중률
		int nBasicHitRate = static_cast<int>( pTempITEM->sSuitOp.nHitRate );
		int nRndOptHitRate = static_cast<int>( sItemCustom.GETHITRATE() - pTempITEM->sSuitOp.nHitRate );
		
		//코스튬 적용 시 
		if ( nBasicHitRate ||  nRndOptHitRate )
		{
			DWORD dwColor = ITEM_INFOR_COLOR::dwDefault;
			if( NULL != psDisguiseSuit )
			{
				if( 0 != psDisguiseSuit->nHitRate )
					dwColor = ITEM_INFOR_COLOR::dwAddDisguise;
			}

			strTemp = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO", 3) );
			int nIndex = m_pItemLabel2->AddText( strTemp.c_str(), dwColor );

			strTemp = sc::string::format( "%d", nBasicHitRate );
			m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwColor );

			if(nRndOptHitRate > 0)
			{	
				strTemp = sc::string::format( "(+%d)", nRndOptHitRate );
				m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwRndOptColor );
			}
			else if(nRndOptHitRate < 0)
			{
				strTemp = sc::string::format( "(%d)", nRndOptHitRate );
				m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwRndOptColor );
			}
		}

		//	회피율
		int nBasicAvoidRate = static_cast<int>( pTempITEM->sSuitOp.nAvoidRate );
		int nRndOptAvoidRate = static_cast<int>( sItemCustom.GETAVOIDRATE() - pTempITEM->sSuitOp.nAvoidRate );
		
		//코스튬 적용 시 
		if ( nBasicAvoidRate ||  nRndOptAvoidRate )
		{
			DWORD dwColor = ITEM_INFOR_COLOR::dwDefault;
			if( NULL != psDisguiseSuit )
			{
				if( 0 != psDisguiseSuit->nAvoidRate )
					dwColor = ITEM_INFOR_COLOR::dwAddDisguise;
			}

			strTemp = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO", 4) );
			int nIndex =  m_pItemLabel2->AddText( strTemp.c_str(), dwColor );

			strTemp = sc::string::format( "%d", nBasicAvoidRate );
			m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwColor );

			if(nRndOptAvoidRate > 0)
			{	
				strTemp = sc::string::format( "(+%d)", nRndOptAvoidRate );
				m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwRndOptColor );
			}
			else if(nRndOptAvoidRate < 0)
			{
				strTemp = sc::string::format( "(%d)", nRndOptAvoidRate );
				m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwRndOptColor );
			}
		}

		//	소모 SP
		int nBasicReqSP = static_cast<int>( pTempITEM->sSuitOp.wReqSP );
		int nRndOptReqSP = static_cast<int>( sItemCustom.GETREQ_SP() - pTempITEM->sSuitOp.wReqSP);

		//const WORD wReqSP = sItemCustom.GETREQ_SP();

		//코스튬 적용 시 
		if ( nBasicReqSP || nRndOptReqSP )
		{
			DWORD dwColor = ITEM_INFOR_COLOR::dwDefault;
			if( NULL != psDisguiseSuit )
			{
				if( 0 != psDisguiseSuit->wReqSP )
					dwColor = ITEM_INFOR_COLOR::dwAddDisguise;
			}

			strTemp = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO", 7)  );
			int nIndex = m_pItemLabel2->AddText( strTemp.c_str(), dwColor );

			strTemp = sc::string::format( "%d", nBasicReqSP );
			m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwColor );

			if(nRndOptReqSP > 0)
			{	
				strTemp = sc::string::format( "(+%d)", nRndOptReqSP );
				m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwRndOptColor );
			}
			else if(nRndOptReqSP < 0)
			{
				strTemp = sc::string::format( "(%d)", nRndOptReqSP );
				m_pItemLabel2->AddString( nIndex, strTemp.c_str(), dwRndOptColor );
			}
		}
	}

    // Info : 소지량
    {
        if ( m_pItemData->sDrugOp.wPileNum > 1 )
        {
            WORD wPileNum = m_pItemData->sDrugOp.wPileNum;
            WORD wTurnNum = sItemCustom.wTurnNum;
            if ( m_bInPrivateMarket )	//	개인 상점 구매창에서 보이는 툴팁일 때
            {
                strTemp = sc::string::format("%s :%d", ID2GAMEWORD("ITEM_TURN_INFO", 0 ), wTurnNum);
                m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault );

                return ;
            }

            if ( m_bInMarket )  // 상점 구매창에서 보이는 툴팁일 때
                wTurnNum = m_pItemData->GETAPPLYNUM();
            strTemp = sc::string::format("%s :%d/%d", ID2GAMEWORD("ITEM_TURN_INFO", 0 ), wTurnNum, wPileNum);

            m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault );
        }
    }

    // Info : 약품류 사용 효과
    {
        if ( m_pItemData->sBasicOp.emItemType == ITEM_CURE )
        {
			/*if ( GLUseFeatures::GetInstance().IsUsingDataIntegrate() )
			{
				if( m_pItemData->sDrugOp.dwCureVolume == 0 )
					strTemp = sc::string::format( "%s", CGameTextMan::GetInstance().GetCommentText("ITEMDRUG", m_pItemData->sDrugOp.emDrug).GetString() );
				else
					strTemp = sc::string::format( "%s :%d",CGameTextMan::GetInstance().GetCommentText("ITEMDRUG", m_pItemData->sDrugOp.emDrug).GetString(), m_pItemData->sDrugOp.dwCureVolume );
			}
			else*/
			{
				if( m_pItemData->sDrugOp.wCureVolume == 0 )
					strTemp = sc::string::format( "%s", COMMENT::ITEMDRUG[m_pItemData->sDrugOp.emDrug].c_str() );
				else
					strTemp = sc::string::format( "%s :%d",COMMENT::ITEMDRUG[m_pItemData->sDrugOp.emDrug].c_str(), m_pItemData->sDrugOp.wCureVolume );
			}

            if ( m_pItemData->sDrugOp.bRatio )
            {
                strTemp += "%";
            }

            m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault );
        }
    }

    // Info : 재사용 시간
    {
        //  재사용 시간
        if ( m_pItemData->sBasicOp.IsCoolTime() )
        {
            std::string strTime = "";
            CTimeSpan cCoolTime( m_pItemData->sBasicOp.dwCoolTime );

            if ( cCoolTime.GetHours() > 0 )
                strTime += sc::string::format( "%d%s ", cCoolTime.GetHours(), ID2GAMEWORD("ITEM_BASIC_TIME", 0) );
            if ( cCoolTime.GetMinutes() > 0 )	
                strTime += sc::string::format( "%d%s ", cCoolTime.GetMinutes(), ID2GAMEWORD("ITEM_BASIC_TIME", 1) );
            if ( cCoolTime.GetSeconds() > 0 )	
                strTime += sc::string::format( "%d%s ", cCoolTime.GetSeconds(), ID2GAMEWORD("ITEM_BASIC_TIME", 2) );

            strTemp = sc::string::format( "%s :%s", ID2GAMEWORD("ITEM_BASIC_INFO", 8), strTime );			
            m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault );
        }

        //	남은시간
        GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
        if ( pCharacter && pCharacter->IsCoolTime( m_pItemData->sBasicOp.sNativeID ) )
        {			
			std::string strTime = "";
			__time64_t tCurTime =  GLGaeaClient::GetInstance().GetCurrentTime().GetTime();
            __time64_t tCoolTime = pCharacter->GetMaxCoolTime( m_pItemData->sBasicOp.sNativeID );				

            CTimeSpan cReTime( tCoolTime - tCurTime );

            if ( cReTime.GetHours() > 0 )	
                strTime += sc::string::format( "%d%s ", cReTime.GetHours(), ID2GAMEWORD("ITEM_BASIC_TIME", 0) );
            if ( cReTime.GetMinutes() > 0 )	
                strTime += sc::string::format( "%d%s ", cReTime.GetMinutes(), ID2GAMEWORD("ITEM_BASIC_TIME", 1) );
            if ( cReTime.GetSeconds() > 0 )	
                strTime += sc::string::format( "%d%s ", cReTime.GetSeconds(), ID2GAMEWORD("ITEM_BASIC_TIME", 2) );

            strTemp = sc::string::format( "%s :%s", ID2GAMEWORD("ITEM_BASIC_INFO", 9), strTime );			
            m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault );
        }

		/*if( m_pItemData->sDrugOp.dwERList > 0 )
		{
			std::string str = "";

			for( int i = 0; i < ITEM_DRUG_ER_SIZE; ++i )
			{
				DWORD dwValue = 1 << i;

				if( dwValue & m_pItemData->sDrugOp.dwERList )
				{
					if( i > 0 )
						str += " + ";
						
					EMITEM_DRUG_ER eType = (EMITEM_DRUG_ER)i;

					switch( eType )
					{
					case ITEM_DRUG_ER_STUN        : str += sc::string::format( "%s", ID2GAMEWORD("ITEM_INFO_SET_BONUS", 26) ); break;
					case ITEM_DRUG_ER_HALLUCINATE : str += sc::string::format( "%s", ID2GAMEWORD("ITEM_INFO_SET_BONUS", 49) ); break;
					case ITEM_DRUG_ER_NON_DRUG    : str += sc::string::format( "%s", ID2GAMEWORD("ITEM_INFO_SET_BONUS", 34) ); break;
					}
				}
			}

			if( !str.empty() )
			{
				str = sc::string::format( "%s : %s", ID2GAMEWORD("ITEM_BASIC_INFO", 21), str );
				m_pItemLabel2->AddText( str.c_str(), ITEM_INFOR_COLOR::dwDefault );
			}
		}*/
    }

	const PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData( m_sNpcNativeID );

	EMITEM_TYPE emType = m_pItemData->sBasicOp.emItemType;

	/*bool bKeyTip = false;

	if ( pCrowData && m_iPageIndex >= 0 && m_pWishListItem->GetTotalLine() == 0 )
	{
		GLInventorySale* rInven = dynamic_cast<GLInventorySale*>(pCrowData->GetSaleInven( m_iPageIndex ));

		EEXTYPE exType = rInven->GetExChangeType(sItemCustom.GetNativeID(), m_wPosX, m_wPosY);

		if( exType == EX_ITEM_TYPE && m_pWishItem == NULL ) 
			bKeyTip = true;

	}*/

	switch(emType)
	{
	case ITEM_SKILL :
	case ITEM_PET_SKILL:
	case ITEM_GRINDING:
	case ITEM_VEHICLE:
	case ITEM_PET_CARD:
	case ITEM_FOOD:
	case ITEM_CARD_DUALPETSKILL:
		{
			//if(bKeyTip ){
			//	strTemp = ID2GAMEINTEXT("HELP_SHOW_ITEM_TOOLTIP");
			//	m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault );
			//}
		}
		break;
	}
	
    switch ( emType )
    {
    case ITEM_SKILL:
        {			
            //DisplaySkillItem( m_pItemLabel2, sItemCustom );
        }
        break;

    case ITEM_PET_SKILL:
        {
            //DisplayPetSkillItem( m_pItemLabel2, sItemCustom );
        }
        break;

    case ITEM_GRINDING:
        {
            //m_pItemLabel2->AddText( ID2GAMEWORD( "ITEM_CATEGORY_EX", 0 ), ITEM_INFOR_COLOR::dwDefault );
        }
        break;

    case ITEM_VIETNAM_EXPGET:
        {
            if ( m_pItemData->sDrugOp.bRatio )
                strTemp = sc::string::format( ID2GAMEWORD("ITEM_CATEGORY_VIETNAM_EXP", 1) , m_pItemData->sDrugOp.wCureVolume );
            else
                strTemp = sc::string::format( ID2GAMEWORD("ITEM_CATEGORY_VIETNAM_EXP", 0), m_pItemData->sDrugOp.wCureVolume );

            m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault );
        }
        break;

    case ITEM_VIETNAM_ITEMGET:
        {
            strTemp = sc::string::format( ID2GAMEWORD("ITEM_CATEGORY_VIETNAM_ITEM", 0) , m_pItemData->sDrugOp.wCureVolume );
            m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault );
        }
        break;

    case ITEM_VEHICLE:
        {
            DisplayVehicleItem( m_pItemLabel2, sItemCustom );
        }
        break;

    case ITEM_PET_CARD:
        {
            DisplayPetCardItem( m_pItemLabel2, sItemCustom );
        }
        break;

    case ITEM_FOOD:
        {
            DisplayLunchBoxItem( m_pItemLabel2, sItemCustom );
        }
        break;

    case ITEM_CARD_DUALPETSKILL:
        {
            DisplayPetDualSkillItem( m_pItemLabel2, sItemCustom );
        }
        break;

    case ITEM_POINTCARD_A:
    case ITEM_POINTCARD_B:
        {
            // Note : 포인트 양 표시.
            strTemp = sc::string::format( ID2GAMEWORD("POINT_CARD_POINT_VOLUME"), m_pItemData->sDrugOp.wCureVolume );
            m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault );
        }
        break;

    case ITEM_RANDOMITEM:
    case ITEM_BOX:
	//case ITEM_SELECTIVEFORM_BOX:
        strTemp = ID2GAMEINTEXT("HELP_SHOW_ITEM_TOOLTIP");
        m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault );
        break;
    };

	
    // Info : 그 외.
    EMITEM_TYPE emItemType = m_pItemData->sBasicOp.emItemType;

    if( emItemType != ITEM_PET_SKILL && emItemType != ITEM_CARD_DUALPETSKILL
        && emItemType != ITEM_POINTCARD_A && emItemType != ITEM_POINTCARD_B )
    {
        // Note : 시귀/직귀 카드에 맵이름 좌표 출력
        if( m_pItemData->sDrugOp.emDrug == ITEM_DRUG_CALL_REGEN )
        {
            m_pItemLabel2->AddText( ID2GAMEWORD( "ITEM_BASIC_INFO", 17 ), NS_UITEXTCOLOR::dwDefault );

			GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();	

			CString strName( GLGaeaClient::GetInstance().GetMapName( pCharacter->m_sStartMapID ) );
            if( strName == _T("(null)") ) strName.Empty();

            strTemp = sc::string::format( "%s :%s", ID2GAMEWORD("ITEM_CALL_MAP", 0 ), strName );
            m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault );

			SMAPNODE *pMapNode = GLGaeaClient::GetInstance().FindMapNode( pCharacter->m_sStartMapID );
            if( pMapNode )
            {
                GLLevelFile cLevelFile;
                BOOL bOk = cLevelFile.LoadFile( pMapNode->strFile.c_str(), TRUE, NULL );
				if( bOk )
                {	
                    D3DXVECTOR3 vStartPos;
					PDXLANDGATE pGate = cLevelFile.GetLandGateMan().FindLandGate( pCharacter->m_dwStartGate );
					if( pGate )
					{
						vStartPos = pGate->GetGenPos ( DxLandGate::GEN_RENDUM );
					}
					else
					{
						pGate = cLevelFile.GetLandGateMan().FindLandGate( DWORD(0) );
						if( pGate ) vStartPos = pGate->GetGenPos ( DxLandGate::GEN_RENDUM );
					}


                    int nPosX(0), nPosY(0);
                    cLevelFile.GetMapAxisInfo().Convert2MapPos( vStartPos.x, vStartPos.z, nPosX, nPosY );

                    strTemp = sc::string::format( "%s :%d,%d", ID2GAMEWORD("ITEM_CALL_MAP", 1 ), nPosX, nPosY );
                    m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault );
                }
            }
        }
        else if( m_pItemData->sDrugOp.emDrug == ITEM_DRUG_CALL_LASTCALL )
        {
            m_pItemLabel2->AddText( ID2GAMEWORD( "ITEM_BASIC_INFO", 17 ), ITEM_INFOR_COLOR::dwDefault );

			GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();

            if( pCharacter->m_sLastCallMapID.IsValidNativeID() )
            {
				CString strName( GLGaeaClient::GetInstance().GetMapName( pCharacter->m_sLastCallMapID ) );
                if( strName == _T("(null)") ) strName.Empty();

                strTemp = sc::string::format( "%s :%s", ID2GAMEWORD("ITEM_CALL_MAP", 0 ), strName );
                m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault );

				SMAPNODE *pMapNode = GLGaeaClient::GetInstance().FindMapNode ( pCharacter->m_sLastCallMapID );
                if( pMapNode )
                {
                    GLLevelFile cLevelFile;
                    BOOL bOk = cLevelFile.LoadFile( pMapNode->strFile.c_str(), TRUE, NULL );
                    if( bOk )
                    {	
                        int nPosX(0), nPosY(0);
                        cLevelFile.GetMapAxisInfo().Convert2MapPos( pCharacter->m_vLastCallPos.x, pCharacter->m_vLastCallPos.z, nPosX, nPosY );

                        strTemp = sc::string::format( "%s :%d,%d", ID2GAMEWORD("ITEM_CALL_MAP", 1 ), nPosX, nPosY );
                        m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault );
                    }
                }
            }
            else
            {
                m_pItemLabel2->AddText( ID2GAMEWORD("ITEM_CALL_MAP", 2 ), ITEM_INFOR_COLOR::dwReqWrong );
            }
        }
		else if ( m_pItemData->sDrugOp.emDrug == ITEM_DRUG_CALL_TELEPORT )
        {
            m_pItemLabel2->AddText( ID2GAMEWORD( "ITEM_BASIC_INFO", 17 ), ITEM_INFOR_COLOR::dwDefault );

			CString strName( GLGaeaClient::GetInstance().GetMapName( m_pItemData->sBasicOp.sSubID ) );
            if( strName == _T("(null)") ) strName.Empty();

            strTemp = sc::string::format( "%s :%s", ID2GAMEWORD("ITEM_TELEPORT_MAP", 0 ), strName );
            m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault );

            strTemp = sc::string::format( "%s :%d,%d", ID2GAMEWORD("ITEM_TELEPORT_MAP", 1 ), m_pItemData->sBasicOp.wPosX, m_pItemData->sBasicOp.wPosY );
            m_pItemLabel2->AddText( strTemp.c_str(), ITEM_INFOR_COLOR::dwDefault );
        }
		/*else if( m_pItemData->sDrugOp.emDrug == ITEM_DRUG_EXP_CAPSULE_TYPE1 || m_pItemData->sDrugOp.emDrug == ITEM_DRUG_EXP_CAPSULE_TYPE2 )
		{
			char szTemp[30] = {0, };
			WORD wCurLev = m_pCharacter->GETLEVEL();
			LONGLONG llCurExp = m_pCharacter->GETEXP();
			LONGLONG llCurMaxExp = m_pCharacter->GETMAXEXP();
			WORD wExpetedLev = 0;
			LONGLONG llRemainExp = 0;
			LONGLONG llRemainMaxExp = 0;
			float fPercent = 0;
			
			strTemp = sc::string::format( "%lu", (LONGLONG)sItemCustom.wDurability * 100000000 );

			sc::string::AddCommaToNum( strTemp.c_str(), szTemp );

			// 랩업을 하는 경우
			if( GLOGICEX::GLCALCULATEEXP( wCurLev, llCurExp, (LONGLONG)sItemCustom.wDurability * 100000000, wExpetedLev, llRemainExp, llRemainMaxExp) == TRUE ) 
			{
				fPercent = float(llRemainExp) / float(llRemainMaxExp);
			}
			// 랩업을 못하는 경우
			else
			{
				wExpetedLev = wCurLev;
				fPercent = float(llCurExp + llRemainExp) / float(llCurMaxExp);
			}

			// 툴팁 표시에 최대 레벨을 넘어서지 않도록
			if( wExpetedLev > GLCONST_CHAR::wMAX_LEVEL )
			{
				wExpetedLev = GLCONST_CHAR::wMAX_LEVEL;
				fPercent = 1;
			}
			else if(wExpetedLev == GLCONST_CHAR::wMAX_LEVEL)
			{
				if(fPercent > 1) 
					fPercent = 1;
			}

			std::string strExp = sc::string::format( "Exp : %s ( %dLv, %.2f%% )", szTemp, wExpetedLev, fPercent * 100.0f);
            m_pItemLabel2->AddText( strExp.c_str(), ITEM_INFOR_COLOR::dwDefault );
		}
		else if( m_pItemData->sDrugOp.emDrug == ITEM_DRUG_EXP_COMPERSSOR_TYPE1 || m_pItemData->sDrugOp.emDrug == ITEM_DRUG_EXP_COMPERSSOR_TYPE2 )
		{
			char szBeforeTemp[30] = {0, };
			char szAfterTemp[30] = {0, };
			LONGLONG llCurExp = m_pCharacter->GETEXP();
			LONGLONG llCurMaxExp = m_pCharacter->GETMAXEXP();
			LONGLONG llCompressorExp = 0;
			float fPercent = 0;

			if( llCurExp > llCurMaxExp ) llCurExp = llCurMaxExp;

			if( m_pItemData->sDrugOp.emDrug == ITEM_DRUG_EXP_COMPERSSOR_TYPE2 )
				llCompressorExp = 100000000000;
			else
				llCompressorExp = 10000000000;

			strTemp = sc::string::format( "%lu", llCurExp );
			sc::string::AddCommaToNum( strTemp.c_str(), szBeforeTemp );
			fPercent = float(llCurExp) / float(llCurMaxExp);

			std::string strExp = sc::string::format( "Before : %s ( %.2f%% )", szBeforeTemp, fPercent * 100.0f);
            m_pItemLabel2->AddText( strExp.c_str(), ITEM_INFOR_COLOR::dwDefault );

			if( llCompressorExp <= llCurExp )
			{
				strTemp = sc::string::format( "%lu", llCurExp - llCompressorExp );
				sc::string::AddCommaToNum( strTemp.c_str(), szAfterTemp );
				fPercent = float(llCurExp - llCompressorExp) / float(llCurMaxExp);
			
				std::string strExp = sc::string::format( "After : %s ( %.2f%% )", szAfterTemp, fPercent * 100.0f);
				m_pItemLabel2->AddText( strExp.c_str(), ITEM_INFOR_COLOR::dwDefault );
			}
			else
			{
				std::string strExp = sc::string::format( "After : %s ", ID2GAMEWORD("ITEM_EXP_COMPRESSOR", 0) );
				m_pItemLabel2->AddText( strExp.c_str(), ITEM_INFOR_COLOR::dwDefault );
			}
		}*/
    }	
	// Info : 랜덤 수치
	{
		if( sItemCustom.IsSetRandOpt() )
		{
			DWORD dwRandomAbilityColor = ITEM_INFOR_COLOR::dwRandomAbility;

			{
				int nRandOptType = sItemCustom.GETOptTYPE1();
				float fVal = sItemCustom.GETOptVALUE1() * COMMENT::ITEM_RANDOM_OPT_SCALE[nRandOptType];
				bool bIsPerRandOpt = sItemCustom.IsPerRandOpt( nRandOptType );

				SetInformationLabel3_RandomOption( nRandOptType, fVal, bIsPerRandOpt, dwRandomAbilityColor );
			}
			{
				int nRandOptType = sItemCustom.GETOptTYPE2();
				float fVal = sItemCustom.GETOptVALUE2() * COMMENT::ITEM_RANDOM_OPT_SCALE[nRandOptType];
				bool bIsPerRandOpt = sItemCustom.IsPerRandOpt( nRandOptType );

				SetInformationLabel3_RandomOption( nRandOptType, fVal, bIsPerRandOpt, dwRandomAbilityColor );
			}
			{
				int nRandOptType = sItemCustom.GETOptTYPE3();
				float fVal = sItemCustom.GETOptVALUE3() * COMMENT::ITEM_RANDOM_OPT_SCALE[nRandOptType];
				bool bIsPerRandOpt = sItemCustom.IsPerRandOpt( nRandOptType );

				SetInformationLabel3_RandomOption( nRandOptType, fVal, bIsPerRandOpt, dwRandomAbilityColor );
			}
			{
				int nRandOptType = sItemCustom.GETOptTYPE4();
				float fVal = sItemCustom.GETOptVALUE4() * COMMENT::ITEM_RANDOM_OPT_SCALE[nRandOptType];
				bool bIsPerRandOpt = sItemCustom.IsPerRandOpt( nRandOptType );

				SetInformationLabel3_RandomOption( nRandOptType, fVal, bIsPerRandOpt, dwRandomAbilityColor );
			}
		}
	}
	//

    // 아이템 미리보기
	if ( m_bUsePreview )
    {
		if ( CInnerInterface::GetInstance().PreviewItemCheckSimple( sItemCustom.sNativeID ) )
        {
            m_pItemLabel2->AddText( ID2GAMEWORD("ITEM_USEPREVIEW_RN"), NS_UITEXTCOLOR::dwDefault );
        }
    }
}

void CItemInfoTooltip::SetInformationLabel3( const SITEMCUSTOM& sItemCustom )
{
    const ITEM::SSUIT& sSUIT = m_pItemData->sSuitOp;

    std::string strText = "";

    // Info : 가산 수치
    {
        int arrVALUE[EMADD_SIZE];
        SecureZeroMemory( arrVALUE, sizeof(arrVALUE) );

		for ( int i = 0; i < ITEM::SSUIT::ADDON_SIZE; ++i )
        {
            EMITEM_ADDON emTYPE = sSUIT.sADDON[i].emTYPE;

            if ( emTYPE != EMADD_NONE )
            {
				//내구도 적용
                int nVALUE = sSUIT.sADDON[i].nVALUE;

                switch ( emTYPE )
                {
                    //	밑에서 한번에 계산
				case EMADD_HP:			
					{
						arrVALUE[EMADD_HP] += nVALUE;
					}					
					break;
				case EMADD_MP:
					{
						arrVALUE[EMADD_MP] += nVALUE;
					}					
					break;
				case EMADD_SP:			
					{
						arrVALUE[EMADD_SP] += nVALUE;
					}
					break;
				case EMADD_MA:			
					{
						arrVALUE[EMADD_MA] += nVALUE;
					}
					break;
                case EMADD_HITRATE:		arrVALUE[EMADD_HITRATE] += nVALUE;		break;
                case EMADD_AVOIDRATE:	arrVALUE[EMADD_AVOIDRATE] += nVALUE;	break;
                case EMADD_DAMAGE:		arrVALUE[EMADD_DAMAGE] += nVALUE;		break;
                case EMADD_DEFENSE:		arrVALUE[EMADD_DEFENSE] += nVALUE;		break;
                case EMADD_STATS_POW:	arrVALUE[EMADD_STATS_POW] += nVALUE;	break;
                case EMADD_STATS_STR:	arrVALUE[EMADD_STATS_STR] += nVALUE;	break;
                case EMADD_STATS_SPI:	arrVALUE[EMADD_STATS_SPI] += nVALUE;	break;
                case EMADD_STATS_DEX:	arrVALUE[EMADD_STATS_DEX] += nVALUE;	break;
                case EMADD_STATS_INT:	arrVALUE[EMADD_STATS_INT] += nVALUE;	break;
                case EMADD_STATS_STA:	arrVALUE[EMADD_STATS_STA] += nVALUE;	break;
                case EMADD_PA:			arrVALUE[EMADD_PA] += nVALUE;			break;
                case EMADD_SA:			arrVALUE[EMADD_SA] += nVALUE;			break;
				case EMADD_POWINTDEX:	arrVALUE[EMADD_POWINTDEX] += nVALUE;	break;
				case EMADD_STMVIT:		arrVALUE[EMADD_STMVIT] += nVALUE;		break;
				case EMADD_PASA:		arrVALUE[EMADD_PASA] += nVALUE;			break;
				case EMADD_PASAMA:		arrVALUE[EMADD_PASAMA] += nVALUE;		break;
                };
            }
        }
		
		{
			//	전체 옵션을 따로 계산
			arrVALUE[EMADD_HP] = sItemCustom.GETADDHP();
			arrVALUE[EMADD_MP] = sItemCustom.GETADDMP();
			arrVALUE[EMADD_SP] = sItemCustom.GETADDSP();
			arrVALUE[EMADD_MA] = sItemCustom.GETADDMA();
		}

        //	코스튬 능력치
        if ( sItemCustom.nidDISGUISE != NATIVEID_NULL() )
        {
            //if ( pItem && pItem->sBasicOp.IsDISGUISE() )
			//SITEM* pItem = GLItemMan::GetInstance().GetItem(sItemCustom.nidDISGUISE);
			SITEM* pItem = GLItemMan::GetInstance().GetItem(sItemCustom.nidDISGUISE);

            //SITEMCUSTOM sCustomeItem(sItemCustom.nidDISGUISE);
            if (pItem&&pItem->sBasicOp.IsDISGUISE())
            {
                for ( int i = 0; i < ITEM::SSUIT::ADDON_SIZE; ++i )
                {
                    EMITEM_ADDON emTYPE = pItem->sSuitOp.sADDON[i].emTYPE;

                    if ( emTYPE != EMADD_NONE )
                    {
                        int nVALUE = pItem->sSuitOp.sADDON[i].nVALUE;

                        switch ( emTYPE )
                        {
                        case EMADD_HP:          arrVALUE[EMADD_HP] += nVALUE;		    break;
                        case EMADD_MP:          arrVALUE[EMADD_MP] += nVALUE;		    break;
                        case EMADD_SP:          arrVALUE[EMADD_SP] += nVALUE;		    break;
                        case EMADD_MA:          arrVALUE[EMADD_MA] += nVALUE;		    break;
                        case EMADD_HITRATE:		arrVALUE[EMADD_HITRATE] += nVALUE;		break;
                        case EMADD_AVOIDRATE:	arrVALUE[EMADD_AVOIDRATE] += nVALUE;	break;
                        case EMADD_DAMAGE:		arrVALUE[EMADD_DAMAGE] += nVALUE;		break;
                        case EMADD_DEFENSE:		arrVALUE[EMADD_DEFENSE] += nVALUE;		break;
                        case EMADD_STATS_POW:	arrVALUE[EMADD_STATS_POW] += nVALUE;	break;
                        case EMADD_STATS_STR:	arrVALUE[EMADD_STATS_STR] += nVALUE;	break;
                        case EMADD_STATS_SPI:	arrVALUE[EMADD_STATS_SPI] += nVALUE;	break;
                        case EMADD_STATS_DEX:	arrVALUE[EMADD_STATS_DEX] += nVALUE;	break;
                        case EMADD_STATS_INT:	arrVALUE[EMADD_STATS_INT] += nVALUE;	break;
                        case EMADD_STATS_STA:	arrVALUE[EMADD_STATS_STA] += nVALUE;	break;
                        case EMADD_PA:			arrVALUE[EMADD_PA] += nVALUE;			break;
                        case EMADD_SA:			arrVALUE[EMADD_SA] += nVALUE;			break;
						case EMADD_POWINTDEX:	arrVALUE[EMADD_POWINTDEX] += nVALUE;	break;
						case EMADD_STMVIT:		arrVALUE[EMADD_STMVIT] += nVALUE;		break;
						case EMADD_PASA:		arrVALUE[EMADD_PASA] += nVALUE;			break;
						case EMADD_PASAMA:		arrVALUE[EMADD_PASAMA] += nVALUE;		break;
                        };

                    }
                }
            }
        }
        
        // 내구도 컬러
        DWORD dwAddAbility = ITEM_INFOR_COLOR::dwAddAbility;
      
        for( int i=1; i<EMADD_SIZE; ++i)
        {
            if( arrVALUE[i] != 0 )
            {
                strText = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADDON_INFO", i) );

                int nIndex = m_pItemLabel3->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwAddAbility );

                if ( arrVALUE[i] > 0 )
                    strText = sc::string::format( "+%d", arrVALUE[i] );
                else if ( arrVALUE[i] < 0 )
                    strText = sc::string::format( "%d", arrVALUE[i] );

                m_pItemLabel3->AddString( nIndex, strText.c_str(), dwAddAbility );
            }
        }
    }
    //

	/*float fDurableValue = m_fDurabilityValue[DURABILITY_APPLY_ADD];

	if( m_bIsDurableTip )
	{
		fDurableValue = 1.0f;
	}*/

    // Info : 특수 기능
    {		
        EMITEM_VAR emITEM_VOL = m_pItemData->sSuitOp.sVOLUME.emTYPE;
		float arrVALUE[EMVAR_SIZE];

		for ( DWORD svaron=0; svaron<ITEM::SSUIT::VARIATION_SIZE; ++svaron )
		{
			EMITEM_VAR emITEM_VAR = m_pItemData->sSuitOp.sVARIATE[svaron].emTYPE;
			SecureZeroMemory( arrVALUE, sizeof(arrVALUE) );

			//	특수기능 ( 변화율 )
			//
			if ( emITEM_VAR != EMVAR_NONE )
			{
				switch ( emITEM_VAR )
				{
				case EMVAR_HP:
				case EMVAR_MP:
				case EMVAR_SP:
				case EMVAR_AP:
					break;
				default:
					arrVALUE[emITEM_VAR] = ( m_pItemData->sSuitOp.sVARIATE[svaron].fVariate/* * fDurableValue*/);
					break;
				};
			}
		}

        arrVALUE[EMVAR_HP] = sItemCustom.GETINCHP();
        arrVALUE[EMVAR_MP] = sItemCustom.GETINCMP();
        arrVALUE[EMVAR_SP] = sItemCustom.GETINCSP();
        arrVALUE[EMVAR_AP] = sItemCustom.GETINCAP(); 

        //	코스튬 능력치
        float fCostumVolume = 0.0f;

        if ( sItemCustom.nidDISGUISE != NATIVEID_NULL() )
        {
            //if ( pItem && pItem->sBasicOp.IsDISGUISE() )
            //SITEM* pItem = GLItemMan::GetInstance().GetItem(sItemCustom.nidDISGUISE);
			SITEM* pItem = GLItemMan::GetInstance().GetItem(sItemCustom.nidDISGUISE);

            //SITEMCUSTOM sCustomeItem(sItemCustom.nidDISGUISE);
            if ( pItem && pItem->sBasicOp.IsDISGUISE())
            {
				
				for ( DWORD svaron=0; svaron<ITEM::SSUIT::VARIATION_SIZE; ++svaron )
				{
					EMITEM_VAR emITEM_VAR_COS = pItem->sSuitOp.sVARIATE[svaron].emTYPE;

					if ( emITEM_VAR_COS != EMVAR_NONE )
					{
						float fVariate = pItem->sSuitOp.sVARIATE[svaron].fVariate;		

						switch ( emITEM_VAR_COS )
						{
						case EMVAR_HP:
						case EMVAR_MP:
						case EMVAR_SP:
						case EMVAR_AP:
							arrVALUE[emITEM_VAR_COS] += fVariate;
							break;
						default:
							arrVALUE[emITEM_VAR_COS] += fVariate;
							break;
						};
					}
				}
				
                EMITEM_VAR emITEM_VOL_COS = pItem->sSuitOp.sVOLUME.emTYPE;

                if ( emITEM_VOL_COS != EMVAR_NONE)
                {
                    fCostumVolume = pItem->sSuitOp.sVOLUME.fVolume;
                }
            }
        }


        // 내구도 컬러 //이동 속도는 여기서 처리가 안됨
        DWORD dwAddAbility = ITEM_INFOR_COLOR::dwAddAbility;
								
		BOOL bVAR_HP(FALSE), bVAR_MP(FALSE), bVAR_SP(FALSE), bVAR_AP(FALSE);

		for ( DWORD svaron=0; svaron<ITEM::SSUIT::VARIATION_SIZE; ++svaron )
		{
			EMITEM_VAR emITEM_VAR = m_pItemData->sSuitOp.sVARIATE[svaron].emTYPE;
			float fVAR_SCALE(0);
			if (emITEM_VAR != EMVAR_NONE)
			{
				float fVAR_SCALE(0);
				if ( emITEM_VAR != EMVAR_NONE )
				{
					switch ( emITEM_VAR )
					{
					case EMVAR_HP:
						bVAR_HP = TRUE;
						fVAR_SCALE = sItemCustom.GETINCHP();
						break;
					case EMVAR_MP:
						bVAR_MP = TRUE;
						fVAR_SCALE = sItemCustom.GETINCMP();
						break;
					case EMVAR_SP:
						bVAR_SP = TRUE;
						fVAR_SCALE = sItemCustom.GETINCSP();
						break;
					case EMVAR_AP:
						bVAR_AP = TRUE;
						fVAR_SCALE = sItemCustom.GETINCAP();
						break;
					default:
						fVAR_SCALE = m_pItemData->sSuitOp.sVARIATE[svaron].fVariate;
						break;
					};
					fVAR_SCALE *= COMMENT::ITEMVAR_SCALE[emITEM_VAR];
					//strText.Format("%s %2.2f", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD", emITEM_VAR ), fVAR_SCALE );
					strText = sc::string::format( "%s : %2.2f", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD", emITEM_VAR ), fVAR_SCALE );
					if ( COMMENT::IsITEMVAR_SCALE(emITEM_VAR) )		strText += "%";
					//AddTextNoSplit(strText,NS_UITEXTCOLOR::GREEN);
					m_pItemLabel3->AddText( strText.c_str(), dwAddAbility );
									
				}
			/*for ( int i = 1; i < EMVAR_SIZE; ++i )
			{
				if ( arrVALUE[i] != 0.0f ) 
				{
						if ( m_pItemData->sBasicOp.emItemType != ITEM_PET_FOOD
							&& m_pItemData->sBasicOp.emItemType != ITEM_VEHICLE_OIL )
							arrVALUE[i] *= COMMENT::ITEMVAR_SCALE[i];

					CString strTemp = sc::string::format( "%.2f", arrVALUE[i] ).c_str();

					CutZero(strTemp,2);

					strText = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD", i ) );
					int nIndex = m_pItemLabel3->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwAddAbility );

					strText = sc::string::format( "%s", strTemp);

					if ( sItemCustom.nidDISGUISE != NATIVEID_NULL() )
					{
						const SITEM* pNativeItem = GLItemMan::GetInstance().GetItem( sItemCustom.GetNativeID() );
						const SITEM* pDisguiseItem = GLItemMan::GetInstance().GetItem( sItemCustom.nidDISGUISE );
						if( NULL != pNativeItem && NULL != pDisguiseItem )
						{
							if( COMMENT::IsITEMVAR_SCALE( pDisguiseItem->sSuitOp.sVARIATE[svaron].emTYPE ) ||
								COMMENT::IsITEMVAR_SCALE( pNativeItem->sSuitOp.sVARIATE[svaron].emTYPE ) )
								strText += "%";
						}
					}
					else
					{
						if ( COMMENT::IsITEMVAR_SCALE(emITEM_VAR) )
							strText += "%";
					}

					m_pItemLabel3->AddString( nIndex, strText.c_str(), dwAddAbility );
				}*/
			}
		}


        // 경험치 배율
        float fExpMultiple = m_pItemData->GetExpMultiple(); //내구도 적용

		//내구도 적용
		/*if( (fExpMultiple > 1.0f) || (m_fDurabilityPerformance < 1.0f || m_fDurabilityPerformance > 1.0f) )
		{
			fExpMultiple = ( (fExpMultiple - 1.0f) * m_fDurabilityValue[DURABILITY_APPLY_ADD] ) + 1.0f;
		}*/

        SITEM* pItem = GLItemMan::GetInstance().GetItem(sItemCustom.nidDISGUISE);
        //const SITEM* pItem = GLItemMan::GetInstance().GetItem(sItemCustom.nidDISGUISE);

        //SITEMCUSTOM sCustomeItem(sItemCustom.nidDISGUISE);
        if ( pItem && pItem->sBasicOp.IsDISGUISE())
        {
            fExpMultiple += pItem->GetExpMultiple() - 1.0f;
            fExpMultiple = max( fExpMultiple, 1.0f );
        }
		
        if ( fExpMultiple != 1.0f )
        {
			CString strTemp = sc::string::format( "%.2f", fExpMultiple * 100.0f - 100.0f ).c_str();

			CutZero(strTemp,2);

            strText = sc::string::format( "%s :", ID2GAMEWORD( "ITEM_ADVANCED_INFO_SPECIAL_EX", 0 ));
            int nIndex = m_pItemLabel3->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwAddAbility );

            strText = sc::string::format( "+%s%%", strTemp);
            m_pItemLabel3->AddString( nIndex, strText.c_str(), dwAddAbility );
        }

        //	특수기능 ( 변화량 )
		EMITEM_VAR emDisguiseItemVol = EMVAR_NONE;
		if( NULL != pItem )
			emDisguiseItemVol = pItem->sSuitOp.sVOLUME.emTYPE;

		if( emITEM_VOL != emDisguiseItemVol && emITEM_VOL != EMVAR_NONE && emDisguiseItemVol != EMVAR_NONE )
		{
			float fVOLUME = 0.0f;
			int nIndex = 0;
			CString strTemp;

			// Basic--------------------------------------------
			if( emITEM_VOL == EMVAR_MOVE_SPEED )
				fVOLUME = sItemCustom.GETMOVESPEED();
			else
				fVOLUME = m_pItemData->sSuitOp.sVOLUME.fVolume;
			
			if( 0.0f != fVOLUME )
			{
				strTemp = sc::string::format( "%.4f", fVOLUME ).c_str();
				CutZero( strTemp, 4 );
				strText = sc::string::format( "%s :", ID2GAMEWORD( "ITEM_ADVANCED_INFO_SPECIAL_ADD_VOL", emITEM_VOL ) );
				nIndex = m_pItemLabel3->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwAddAbility );

				strText = sc::string::format( "%s", strTemp );
				m_pItemLabel3->AddString( nIndex, strText.c_str(), dwAddAbility );
			}

			// Disguise------------------------------------------------
			if( emDisguiseItemVol == EMVAR_MOVE_SPEED )
				fVOLUME = sItemCustom.GETMOVESPEED();
			else
				fVOLUME = fCostumVolume;
			
			if( 0.0f != fVOLUME )
			{
				strTemp = sc::string::format( "%.4f", fVOLUME ).c_str();
				CutZero( strTemp, 4 );
				strText = sc::string::format( "%s :", ID2GAMEWORD( "ITEM_ADVANCED_INFO_SPECIAL_ADD_VOL", emDisguiseItemVol ) );
				nIndex = m_pItemLabel3->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwAddAbility );

				strText = sc::string::format( "%s", strTemp );
				m_pItemLabel3->AddString( nIndex, strText.c_str(), dwAddAbility );
			}			
		}
		else if( emITEM_VOL != EMVAR_NONE || emDisguiseItemVol != EMVAR_NONE )
        {
			float fVOLUME = 0.0f;
			if( emITEM_VOL == EMVAR_MOVE_SPEED || emDisguiseItemVol == EMVAR_MOVE_SPEED )
				fVOLUME = sItemCustom.GETMOVESPEED();
			else
				fVOLUME = (m_pItemData->sSuitOp.sVOLUME.fVolume) + fCostumVolume;	//공격속도
					
			if( 0.0f != fVOLUME )
			{
				CString strTemp = sc::string::format( "%.4f", fVOLUME ).c_str();
				CutZero( strTemp, 4 );

				int nIndex = 0;

				if( emITEM_VOL == emDisguiseItemVol || emDisguiseItemVol == EMVAR_NONE )
					strText = sc::string::format( "%s :", ID2GAMEWORD( "ITEM_ADVANCED_INFO_SPECIAL_ADD_VOL", emITEM_VOL ) );
				else if( emITEM_VOL == EMVAR_NONE )
					strText = sc::string::format( "%s :", ID2GAMEWORD( "ITEM_ADVANCED_INFO_SPECIAL_ADD_VOL", emDisguiseItemVol ) );

				nIndex = m_pItemLabel3->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwAddAbility );

				strText = sc::string::format( "%s", strTemp );
				m_pItemLabel3->AddString( nIndex, strText.c_str(), dwAddAbility );
			}
        }
    }
    //
	// Info : 저항
	{
		//	기본 저항값
		int nELEC   = sSUIT.sResist.nElectric;
		int nFIRE   = sSUIT.sResist.nFire;
		int nICE    = sSUIT.sResist.nIce;
		int nPOISON = sSUIT.sResist.nPoison;
		int nSPIRIT = sSUIT.sResist.nSpirit;
		
		//보너스 효과 분리
		int nAddELEC( static_cast<int>(sItemCustom.GETRESIST_ELEC() - nELEC) ); 
		int	nAddFIRE( static_cast<int>(sItemCustom.GETRESIST_FIRE() - nFIRE) ); 
		int	nAddICE( static_cast<int>(sItemCustom.GETRESIST_ICE() - nICE) ); 
		int	nAddPOISON( static_cast<int>(sItemCustom.GETRESIST_POISON() - nPOISON) ); 
		int	nAddSPIRIT( static_cast<int>(sItemCustom.GETRESIST_SPIRIT() - nSPIRIT) );

		if ( sItemCustom.nidDISGUISE != NATIVEID_NULL() )
		{
            SITEM* pItem = GLItemMan::GetInstance().GetItem(sItemCustom.nidDISGUISE);

            SITEMCUSTOM sCustomeItem(sItemCustom.nidDISGUISE);
			
			//코스튬 옵션 추가 - 코스튬 옵션은 내구도 영향을 받지 않음
			if ( pItem && pItem->sBasicOp.IsDISGUISE())
			{
				nAddELEC       += pItem->sSuitOp.sResist.nElectric;
				nAddFIRE       += pItem->sSuitOp.sResist.nFire;
				nAddICE        += pItem->sSuitOp.sResist.nIce;
				nAddPOISON     += pItem->sSuitOp.sResist.nPoison;
				nAddSPIRIT     += pItem->sSuitOp.sResist.nSpirit;
			}	
		}

		if ( nELEC || nFIRE || nICE || nPOISON || nSPIRIT ||
			nAddELEC || nAddFIRE || nAddICE || nAddPOISON || nAddSPIRIT )
		{
			// 내구도 컬러
			DWORD dwAddAbility = ITEM_INFOR_COLOR::dwAddAbility;

			m_bDisplayResist = true;

			//strText = sc::string::format("%s :", ID2GAMEWORD ("ITEM_BASIC_INFO", 12) );
			strText = " ";
			m_pItemLabel3->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwAddAbility );
			
			int nIndex = 0;
			//if ( nSPIRIT )
			{
				strText = sc::string::format("%d", nSPIRIT);
				nIndex = m_pResistSpiritText->AddText( strText.c_str(), dwAddAbility );

				if ( nAddSPIRIT )
				{
					strText = sc::string::format("(+%d)", nAddSPIRIT);
					m_pResistSpiritText->AddString(nIndex, strText.c_str(), ITEM_INFOR_COLOR::dwElementalAbility );
				}
			}

			//if ( nFIRE )
			{
				strText = sc::string::format("%d", nFIRE );
				nIndex = m_pResistFireText->AddText( strText.c_str(), dwAddAbility );

				if ( nAddFIRE )
				{
					strText = sc::string::format("(+%d)", nAddFIRE);
					m_pResistFireText->AddString(nIndex, strText.c_str(), ITEM_INFOR_COLOR::dwElementalAbility );
				}
			}

			//if ( nICE )
			{
				strText = sc::string::format("%d", nICE );
				nIndex = m_pResistIceText->AddText( strText.c_str(), dwAddAbility );

				if ( nAddICE )
				{
					strText = sc::string::format("(+%d)", nAddICE );
					m_pResistIceText->AddString(nIndex, strText.c_str(), ITEM_INFOR_COLOR::dwElementalAbility );
				}
			}

			//if ( nELEC )
			{
				strText = sc::string::format("%d", nELEC );
				nIndex = m_pResistElectricText->AddText( strText.c_str(), dwAddAbility );

				if ( nAddELEC )
				{
					strText = sc::string::format("(+%d)", nAddELEC );
					m_pResistElectricText->AddString(nIndex, strText.c_str(), ITEM_INFOR_COLOR::dwElementalAbility );
				}
			}

			//if ( nPOISON )
			{
				strText = sc::string::format("%d", nPOISON );
				nIndex = m_pResistPoisonText->AddText( strText.c_str(), dwAddAbility );

				if ( nAddPOISON )
				{
					strText = sc::string::format("(+%d)", nAddPOISON );
					m_pResistPoisonText->AddString(nIndex, strText.c_str(), ITEM_INFOR_COLOR::dwElementalAbility );
				}
			}
			
			//추가 저항 증가 옵션이 없을 경우 출력 간격 조절 22는 임의의 숫자 보기 좋은 간격!
			if(nAddSPIRIT || nAddFIRE || nAddICE || nAddELEC || nAddPOISON)
				m_pResistTextGap = 20.0f;
			else
				m_pResistTextGap = 0.0f;
		}
	}
   
    //
}

void CItemInfoTooltip::SetInformationLabel3_RandomOption( int nRandOptType, float fVal, bool bIsPerRandOpt, DWORD dwAddAbility )
{
    std::string strDescText = "";

    if( (nRandOptType <= EMR_OPT_NULL) || (nRandOptType >= EMR_OPT_SIZE) )
		return;

	if ( fVal == 0.0f )
		return;

	strDescText = sc::string::format( "%s :", ID2GAMEWORD("ITEM_RANDOM_OPTION", nRandOptType ) );
	int nIndex = m_pItemLabel4->AddText( strDescText.c_str(), NS_UITEXTCOLOR::dwRandomAbility );

	strDescText = "";

	if ( fVal > 0.0f) 
		strDescText += _T('+');
	if ( fVal < 0.0f) 
		strDescText += _T('-');

	if ( bIsPerRandOpt )
	{
		CString strTemp = sc::string::format( "%.2f", fVal ).c_str();
		CutZero(strTemp,2);
		strDescText += sc::string::format("%s", strTemp);

		strDescText += _T('%');
	}
	else
		strDescText += sc::string::format( "%d", static_cast<int>(fVal) );

	m_pItemLabel4->AddString( nIndex, strDescText.c_str(), dwAddAbility );
}

void CItemInfoTooltip::SetInformationLabel4( const SITEMCUSTOM& sItemCustom )
{
    std::string strText = "";

    //	상태이상
    EMSTATE_BLOW emBLOW = m_pItemData->sSuitOp.sBLOW.emTYPE;

    if ( emBLOW !=EMBLOW_NONE )
    {
        float fLife = m_pItemData->sSuitOp.sBLOW.fLIFE;
        float fRate = m_pItemData->sSuitOp.sBLOW.fRATE;

		float fVAR1;
		float fVAR2;
		/*if ( GLUseFeatures::GetInstance().IsUsingDataIntegrate() )
		{
			fVAR1 = m_pItemData->sSuitOp.sBLOW.fVAR1 * COMMENT::BLOW_VAR1_SCALE[emBLOW];
			fVAR2 = m_pItemData->sSuitOp.sBLOW.fVAR2 * COMMENT::BLOW_VAR2_SCALE[emBLOW];
		}
		else*/
		{
			fVAR1 = m_pItemData->sSuitOp.sBLOW.fVAR1 * COMMENT::BLOW_VAR1_SCALE[emBLOW];
			fVAR2 = m_pItemData->sSuitOp.sBLOW.fVAR2 * COMMENT::BLOW_VAR2_SCALE[emBLOW];
		}        

		ITEM_TOOLTIP_STATE_BLOW emStateBlow = ITEM_TOOLTIP_VALUE;

		if ( fVAR1 != 0.0f )
		{
			if ( fVAR2 != 0.0f )
				emStateBlow = ITEM_TOOLTIP_VALUE_COUNT_2;
			else
				emStateBlow = ITEM_TOOLTIP_VALUE_COUNT_1_VAR1;
		}
		else
		{
			if( fVAR2 != 0.0f )
				emStateBlow = ITEM_TOOLTIP_VALUE_COUNT_1_VAR2;
		}

		//if ( fVAR1 < 0.0f )
			//fVAR1 = -fVAR1;

		//if ( fVAR2 < 0.0f )
			//fVAR2 = -fVAR2;

        switch ( emBLOW )
        {
        case EMBLOW_NUMB:   //	마비.
            {
				//확률은 수치값이 100%가 더해져서 적용되고 있다 표기출력은 - 100% (ex.딜레이변화 1.3 >> 툴팁 표시 스킬딜레이 +30%)
				if(fVAR2 > (1.0f * COMMENT::BLOW_VAR2_SCALE[emBLOW])) 
					fVAR2 -=  (1.0f * COMMENT::BLOW_VAR2_SCALE[emBLOW]);

				strText = sc::string::format( ID2GAMEINTEXT( "DES_STATE_BLOW_NUMB_RN" ), fRate, fLife, fVAR1 );
				
				//+기호 붙이기
				if( fVAR2 > 0.0f )
					strText += sc::string::format(" +%2.f%%", fVAR2);
				else
					strText += sc::string::format(" %2.f%%", fVAR2);

				m_pItemLabel4_1->AddText( strText.c_str(), NS_UITEXTCOLOR::dwElementalAbility );
            }
            break;

        case EMBLOW_STUN:   //	기절.
            {
                strText = sc::string::format( ID2GAMEINTEXT( "DES_STATE_BLOW_STUN_RN" ), fRate, fLife );
                m_pItemLabel4_1->AddText( strText.c_str(), NS_UITEXTCOLOR::dwElementalAbility );
            }
            break;

        case EMBLOW_STONE:  //	석화.
            {
				strText = sc::string::format( ID2GAMEINTEXT( "DES_STATE_BLOW_STONE_RN" ), fRate, fLife, fVAR1, fVAR2 );
				m_pItemLabel4_1->AddText( strText.c_str(), NS_UITEXTCOLOR::dwElementalAbility );
            }
            break;

        case EMBLOW_BURN:   //	화염.
            {
				//음수표기
                if ( fVAR2 > 0.0f )
                    fVAR2 = -fVAR2;

                strText = sc::string::format( ID2GAMEINTEXT( "DES_STATE_BLOW_BURN_RN" ), fRate, fLife, fVAR2 );
                m_pItemLabel4_1->AddText( strText.c_str(), NS_UITEXTCOLOR::dwElementalAbility );
            }
            break;

        case EMBLOW_FROZEN: //	냉동.
            {
				strText = sc::string::format( ID2GAMEINTEXT( "DES_STATE_BLOW_FROZEN_RN" ), fRate, fLife, fVAR1 );
				
				//+기호 붙이기
				if( fVAR2 > 0.0f )
					strText += sc::string::format(" +%2.f%%", fVAR2);
				else
					strText += sc::string::format(" %2.f%%", fVAR2);

				m_pItemLabel4_1->AddText( strText.c_str(), NS_UITEXTCOLOR::dwElementalAbility );
            }
            break;

        case EMBLOW_MAD:    //	착란.
            {
				//음수표기
				if ( fVAR1 > 0.0f )
					fVAR1 = -fVAR1;

				//음수표기
				if ( fVAR2 > 0.0f )
					fVAR2 = -fVAR2;
				strText = sc::string::format( ID2GAMEINTEXT( "DES_STATE_BLOW_MAD_RN" ), fRate, fLife, fVAR1, fVAR2 );
				m_pItemLabel4_1->AddText( strText.c_str(), NS_UITEXTCOLOR::dwElementalAbility );
            }
            break;

        case EMBLOW_POISON: //	중독.
            {
                if ( fVAR2 > 0.0f )
                    fVAR2 = -fVAR2;

                strText = sc::string::format( ID2GAMEINTEXT( "DES_STATE_BLOW_POISON_RN" ), fRate, fLife, fVAR2 );
                m_pItemLabel4_1->AddText( strText.c_str(), NS_UITEXTCOLOR::dwElementalAbility );
            }
            break;

        case EMBLOW_CURSE:  //	저주.
            {
				strText = sc::string::format( ID2GAMEINTEXT( "DES_STATE_BLOW_CURSE_RN" ), fLife, fRate, fVAR1 );
				m_pItemLabel4_1->AddText( strText.c_str(), NS_UITEXTCOLOR::dwElementalAbility );
            }
            break;

        default:
            {
				//if ( GLUseFeatures::GetInstance().IsUsingDataIntegrate() )
				//	strText = sc::string::format("%s :%s", ID2GAMEWORD("ITEM_ADVANCED_INFO_ABNORMAL", 0 ),CGameTextMan::GetInstance().GetCommentText("BLOW", emBLOW).GetString());
				//else
					strText = sc::string::format("%s :%s", ID2GAMEWORD("ITEM_ADVANCED_INFO_ABNORMAL", 0 ), COMMENT::BLOW[emBLOW].c_str() );

                m_pItemLabel4_1->AddText( strText.c_str(), NS_UITEXTCOLOR::dwElementalAbility );

                strText = sc::string::format("%s :%.2f", ID2GAMEWORD("ITEM_ADVANCED_INFO_ABNORMAL", 1 ), fLife );
                m_pItemLabel4_1->AddText( strText.c_str(), NS_UITEXTCOLOR::dwElementalAbility );

                strText = sc::string::format("%s :%.2f%%", ID2GAMEWORD("ITEM_ADVANCED_INFO_ABNORMAL", 2 ), fRate );
                m_pItemLabel4_1->AddText( strText.c_str(), NS_UITEXTCOLOR::dwElementalAbility );

                if( fVAR1 != 0.0f )
                {
					/*if ( GLUseFeatures::GetInstance().IsUsingDataIntegrate() )
					{
						if ( COMMENT::IsBLOW1_PER(emBLOW) )
							strText = sc::string::format("%s :%.2f%%", CGameTextMan::GetInstance().GetCommentText("BLOW_VAR1", emBLOW).GetString(), fVAR1 );
						else
							strText = sc::string::format("%s :%.2f", CGameTextMan::GetInstance().GetCommentText("BLOW_VAR1", emBLOW).GetString(), fVAR1 );
					}
					else*/
					{
						if ( COMMENT::IsBLOW1_PER(emBLOW) )
							strText = sc::string::format("%s :%.2f%%", COMMENT::BLOW_VAR1[emBLOW].c_str(), fVAR1 );
						else
							strText = sc::string::format("%s :%.2f", COMMENT::BLOW_VAR1[emBLOW].c_str(), fVAR1 );
					}

                    m_pItemLabel4_1->AddText( strText.c_str(), NS_UITEXTCOLOR::dwElementalAbility );
                }

                if( fVAR2 != 0.0f )
                {
					/*if ( GLUseFeatures::GetInstance().IsUsingDataIntegrate() )
					{
						if ( COMMENT::IsBLOW2_PER(emBLOW) )
							strText = sc::string::format("%s :%.2f%%", CGameTextMan::GetInstance().GetCommentText("BLOW_VAR2", emBLOW).GetString(), fVAR2 );
						else
							strText =sc::string::format("%s :%.2f", CGameTextMan::GetInstance().GetCommentText("BLOW_VAR2", emBLOW).GetString(), fVAR2 );
					}
					else*/
					{
						if ( COMMENT::IsBLOW2_PER(emBLOW) )
							strText = sc::string::format("%s :%.2f%%", COMMENT::BLOW_VAR2[emBLOW].c_str(), fVAR2 );
						else
							strText = sc::string::format("%s :%.2f", COMMENT::BLOW_VAR2[emBLOW].c_str(), fVAR2 );
					}
                    m_pItemLabel4_1->AddText( strText.c_str(), NS_UITEXTCOLOR::dwElementalAbility );
                }
            }
        };
    }


	if( m_pItemData->sBasicOp.emItemType == ITEM_SUIT )
	{
		if ( m_pItemData->sSuitOp.emSuit == SUIT_HANDHELD)
		{
			SNATIVEID sSkillLinkID = sItemCustom.sSkillLinkID;

			if( sSkillLinkID.IsValidNativeID() )
			{
				PGLSKILL pSkill = GLSkillMan::GetInstance().GetData(sSkillLinkID);
				if ( pSkill )
				{
					float fRate = 0.0f;
					std::vector<GLWEAPON_LINKSKILL>::size_type i = 0, j = GLCONST_CHAR::vecWeaponLinkSkill.size();
					for( ; i < j; ++i )
					{
						GLWEAPON_LINKSKILL glSkill = GLCONST_CHAR::vecWeaponLinkSkill[i];
						if ( glSkill.sNativeID != sSkillLinkID )
							continue;
											
						fRate = glSkill.fRate;
						break;
					}
					strText = sc::string::format( ID2GAMEINTEXT( "ITEM_SKILL_LINK_INFO", 0 ), fRate, pSkill->GetName());
					m_pItemLabel4_1->AddText( strText.c_str(), NS_UITEXTCOLOR::DODGERBLUE );
				}
			}
		}
	}
	else if ( m_pItemData->sBasicOp.isHaveSkill() )
	{
		strText = sc::string::format( "%s", ID2GAMEWORD( "ITEM_BASIC_RANDOM_STAT_PREVIEW", 1) );
		m_pItemLabel4_1->AddText( strText.c_str(), NS_UITEXTCOLOR::DODGERBLUE );
	}
	
}

void CItemInfoTooltip::SetInformationLabel4_1( const SITEMCUSTOM& sItemCustom )
{
}

void CItemInfoTooltip::SetInformationLabel5( const SITEMCUSTOM& sItemCustom )
{
    std::string strText = "";

	bool bDisplayComment = false;
	bool bDisplayPrice   = false;
	bool bDisplayReq     = false;

    // Info : 아이템 설명
    {
        if ( (m_pItemData->sBasicOp.emItemType != ITEM_SKILL) || (m_pItemData->sBasicOp.emItemType != ITEM_PET_SKILL) )
        {
            const char* chComment = m_pItemData->GetComment();
            if ( chComment )
                strText = sc::string::format( "%s", chComment );
        }
        // Note : 실행 되지 않는 코드 입니다.
        else
        {
            //SNATIVEID sNativeID = sItemCustom.GetBoxWrappedItemID();
			SNATIVEID sNativeID = sItemCustom.GetBoxWrappedItemID();
            const SITEM* pItemData = GLItemMan::GetInstance().GetItem( sItemCustom.sNativeID );
            if (!pItemData)
                return;
            SNATIVEID sSkillID = pItemData->sSkillBookOp.sSkill_ID;

            //	Note : 스킬 정보 가져옴.
            PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( sSkillID.wMainID, sSkillID.wSubID );
            if ( pSkill )
            {
                const char* chDesc = pSkill->GetDesc();
                if ( chDesc )
                    strText = sc::string::format( "%s", chDesc );
            }
        }

        if( strText.length() != 0 )
        {
            m_pItemLabel5->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwDefault );
            bDisplayComment = true;
        }
    }

    // Info : 거래 및 버리기
    {
        if ( bDisplayComment )
            m_pItemLabel5->AddText( "", ITEM_INFOR_COLOR::dwDefault );

        DWORD dwItemFlag = m_pItemData->sBasicOp.dwFlags;
		//DWORD dwItemFlag = sItemCustom.IsFlags();

        DWORD dwTradeColor = ITEM_INFOR_COLOR::dwDefault;

		bool bSale = m_pItemData->sBasicOp.IsSALE();
		bool bExchange = m_pItemData->sBasicOp.IsEXCHANGE();
		bool bLocker = bExchange;
		bool bThrow = m_pItemData->sBasicOp.IsTHROW();
		bool bCostume = m_pItemData->sBasicOp.IsDISGUISE();
		bool bTrashCan = m_pItemData->sBasicOp.IsGarbage();

        bool bAllTradeOption = false;
        if ( (dwItemFlag & TRADE_SALE) && (dwItemFlag & TRADE_THROW) && (dwItemFlag & TRADE_EXCHANGE) )
		//if ( bSale && bThrow && bExchange )
            bAllTradeOption = true;

        //if ( !bAllTradeOption )
		
		if ( sItemCustom.IsGM_GENITEM() )
		{
			//AddTextNoSplit( "", NS_UITEXTCOLOR::WHITE );
			//AddTextNoSplit ( ID2GAMEINTEXT( "ITEM_INFO_GM_GEN"),  NS_UITEXTCOLOR::GOLD);
			strText = sc::string::format( "%s", ID2GAMEINTEXT("ITEM_INFO_GM_GEN") );
			m_pItemLabel5->AddText(strText.c_str(), NS_UITEXTCOLOR::GOLD );
		}
		else
        {
            strText = sc::string::format( "%s", ID2GAMEWORD("ITEM_INFO_TRADE_RN", 0) );
            if ( dwItemFlag & TRADE_SALE )
			//if ( bSale )
            {
                //strText += ID2GAMEWORD("ITEM_INFO_TRADE_OPTION", 0);
                dwTradeColor = ITEM_INFOR_COLOR::dwDefault;
            }
            else
            {
                //strText += ID2GAMEWORD("ITEM_INFO_TRADE_OPTION", 1);
                dwTradeColor = ITEM_INFOR_COLOR::dwTrade;
            }
			int nIndex = m_pItemLabel5->AddText(strText.c_str(), dwTradeColor );
			m_pItemLabel5->AddString(nIndex, " / ", ITEM_INFOR_COLOR::dwDefault );
			
            strText = sc::string::format( "%s", ID2GAMEWORD("ITEM_INFO_TRADE_RN", 1) );
            if ( dwItemFlag & TRADE_THROW )
			//if ( bThrow )
            {
                //strText += ID2GAMEWORD("ITEM_INFO_TRADE_OPTION", 0);
                dwTradeColor = ITEM_INFOR_COLOR::dwDefault;
            }
            else
            {
                //strText += ID2GAMEWORD("ITEM_INFO_TRADE_OPTION", 1);
                dwTradeColor = ITEM_INFOR_COLOR::dwTrade;
            }
			m_pItemLabel5->AddString( nIndex, strText.c_str(), dwTradeColor );
			m_pItemLabel5->AddString(nIndex, " / ", ITEM_INFOR_COLOR::dwDefault );

            strText = sc::string::format( "%s", ID2GAMEWORD("ITEM_INFO_TRADE_RN", 6) );
            if ( dwItemFlag & TRADE_THROW )
			//if ( !bLocker )
            {
                //strText += ID2GAMEWORD("ITEM_INFO_TRADE_OPTION", 0);
                dwTradeColor = ITEM_INFOR_COLOR::dwDefault;
            }
            else
            {
                //strText += ID2GAMEWORD("ITEM_INFO_TRADE_OPTION", 1);
                dwTradeColor = ITEM_INFOR_COLOR::dwTrade;
            }
			m_pItemLabel5->AddString( nIndex, strText.c_str(), dwTradeColor );
			m_pItemLabel5->AddString(nIndex, " / ", ITEM_INFOR_COLOR::dwDefault );


            strText = sc::string::format( "%s", ID2GAMEWORD("ITEM_INFO_TRADE_RN", 3) );
            //if ( dwItemFlag & TRADE_EXCHANGE )
			if ( bExchange )
            {
                //strText += ID2GAMEWORD("ITEM_INFO_TRADE_OPTION", 0);
                dwTradeColor = ITEM_INFOR_COLOR::dwDefault;
            }
            else
            {
                //strText += ID2GAMEWORD("ITEM_INFO_TRADE_OPTION", 1);
                dwTradeColor = ITEM_INFOR_COLOR::dwTrade;
            }
			m_pItemLabel5->AddString( nIndex, strText.c_str(), dwTradeColor );
			

            // 란 포인트 거래
			/*if ( GLUseFeatures::GetInstance().IsUsingPrivateMarketConsignmentSale() )
			{
				m_pItemLabel5->AddString(nIndex, " / ", ITEM_INFOR_COLOR::dwDefault );
				strText = sc::string::format( "%s ", ID2GAMEWORD("ITEM_INFO_TRADE_RN", 4) );
				int RPTradeCountMax = m_pItemData->sBasicOp.RanPointTradeCount();
				int RPTradeCountNow = sItemCustom.wRanPointTradeCount;

				//if ( dwItemFlag & TRADE_EXCHANGE || 0 < RPTradeCountMax - RPTradeCountNow )
				if ( sItemCustom.IsExchange() && 0 < RPTradeCountMax - RPTradeCountNow )
				{
					//strText += ID2GAMEWORD("ITEM_INFO_TRADE_OPTION", 0);
					dwTradeColor = ITEM_INFOR_COLOR::dwDefault;
				}
				else
				{
					//strText += ID2GAMEWORD("ITEM_INFO_TRADE_OPTION", 1);
					dwTradeColor = ITEM_INFOR_COLOR::dwTrade;
				}

				if( 0 < RPTradeCountMax )
					strText += sc::string::format( " (%1%/%2%)", RPTradeCountMax-RPTradeCountNow, RPTradeCountMax );

				m_pItemLabel5->AddString( nIndex, strText.c_str(), dwTradeColor );
			}*/

            // 포장 가능 여부 표시 : 포장지 포장 가능 여부는 기존 거래 및 버리기 옵션과 다르게 포장 가능인 아이템에 한하여 메시지 출력;
			BOOL bWrapped = FALSE;
            if ( !bWrapped )
            {
				BOOL bWrappable = m_pItemData->isWrappable();
				if ( sItemCustom.nidDISGUISE != NATIVEID_NULL() )	bWrappable = FALSE;
				if ( !sItemCustom.IsGM_GENITEM() && m_pItemData->sBasicOp.IsEXCHANGE() )	bWrappable = FALSE;

				if ( bWrappable )
				{
					m_pItemLabel5->AddString(nIndex, " / ", ITEM_INFOR_COLOR::dwDefault );
					strText = sc::string::format( "%s", ID2GAMEWORD("ITEM_INFO_TRADE_RN", 5) );
					dwTradeColor = ITEM_INFOR_COLOR::dwDefault;
					nIndex = m_pItemLabel5->AddString( nIndex, strText.c_str(), dwTradeColor );
				}
            }
        }
        /*else
        {
            // 포장 가능 여부 표시 : 포장지 포장 가능 여부는 기존 거래 및 버리기 옵션과 다르게 포장 가능인 아이템에 한하여 메시지 출력;
            /*if ( m_pItemData->isWrappable() )
            {
                strText.Format( "%s", ID2GAMEWORD("ITEM_INFO_TRADE_RN", 5) );
                dwTradeColor = NS_UITEXTCOLOR::dwDefault;
                int nIndex = m_pItemLabel5->AddText(strText, dwTradeColor );
            }
        }*/
    }

    // Info : 판매 구입 가격
    {
        if ( m_pItemData->sBasicOp.dwBuyPrice || m_bInPrivateMarket )
        {
            if ( m_bInPrivateMarket )
			{
				bDisplayPrice = true;
                m_pItemLabel5->AddText( "", ITEM_INFOR_COLOR::dwDefault );

                bool bOPENER;
                DWORD dwPrivateMarketID;
                //m_pInterface->GetPrivateMarketInfo( bOPENER, dwPrivateMarketID );
				CInnerInterface::GetInstance().GetPrivateMarketInfo ( bOPENER, dwPrivateMarketID );
                if ( bOPENER )	//	파는 사람
                {
                    GLPrivateMarket& sPMarket = GLGaeaClient::GetInstance().GetCharacter()->m_sPMarket;

                    const SSALEITEM* pSALE = sPMarket.GetItem( SNATIVEID( m_wPosX, m_wPosY ) ); 
                    if ( pSALE )
                    {
                        std::string strMoney = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( pSALE->llPRICE, 3, "," );
                        strText = sc::string::format( "%s :%s", ID2GAMEWORD("ITEM_BASIC_INFO", 15), strMoney );
                        m_pItemLabel5->AddText( strText.c_str(), MONEYCOLOR::GetMoneyColor(pSALE->llPRICE) );
                    }						
                }
                else		//	사는 사람
                {
                    PGLCHARCLIENT pCLIENT = GLGaeaClient::GetInstance().GetChar ( dwPrivateMarketID );
                    if ( pCLIENT )
                    {
                        GLPrivateMarket& sPMarket = pCLIENT->m_sPMarket;

                        const SSALEITEM* pSALE = sPMarket.GetItem( SNATIVEID( m_wPosX, m_wPosY ) ); 
                        if ( pSALE )
                        {
                            std::string strMoney = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( pSALE->llPRICE, 3, "," );
                            strText = sc::string::format( "%s :%s", ID2GAMEWORD("ITEM_BASIC_INFO", 16), strMoney );			
                            m_pItemLabel5->AddText( strText.c_str(), MONEYCOLOR::GetMoneyColor(pSALE->llPRICE) );
                        }
                    }
                }
            }				
            else if ( m_bShopOpen && m_bInMarket && m_pItemData->sBasicOp.dwReqContributionPoint == 0  )	// 살때 가격
			{
				bDisplayPrice = true;
                m_pItemLabel5->AddText( "", ITEM_INFOR_COLOR::dwDefault );

                LONGLONG llNpcSellPrice = 0;

                volatile LONGLONG llPrice = 0;
				volatile double fSHOP_RATE = GLGaeaClient::GetInstance().GetCharacter()->GetBuyRate();

                if( m_sNpcNativeID.wMainID != 0 && m_sNpcNativeID.wSubID != 0 )
                {
                    const PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData( m_sNpcNativeID );
                    if( pCrowData != NULL )
                    {
                        LONGLONG dwNpcPrice = pCrowData->GetNpcSellPrice( m_pItemData->sBasicOp.sNativeID.dwID );
                        if( dwNpcPrice == 0 )
                        {								
                            llNpcSellPrice = static_cast<LONGLONG>( m_pItemData->sBasicOp.dwBuyPrice );
                            llPrice = llNpcSellPrice * static_cast<LONGLONG>(fSHOP_RATE) / 100;
                        }
                        else
                        {
                            llNpcSellPrice = dwNpcPrice;
                            llPrice = llNpcSellPrice;								
                        }
                    }

                }				

                std::string strMoney = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( llPrice, 3, "," );

                strText = sc::string::format( "%s :%s", ID2GAMEWORD("ITEM_BASIC_INFO", 13), strMoney );

                //	커미션 액수
                DWORD dwCOMMISSION_MONEY = static_cast<DWORD>(llPrice - llNpcSellPrice);

				LONGLONG lMoney  = GLGaeaClient::GetInstance().GetCharacterLogic().GetInvenMoney();
                D3DCOLOR dwColor = NS_UITEXTCOLOR::RED;

                if( llPrice <= lMoney )
                {
                    dwColor = MONEYCOLOR::GetMoneyColor(llPrice);//NS_UITEXTCOLOR::PALEGREEN;
                }

				//if( m_iPageIndex == -1 )
				m_pItemLabel5->AddText( strText.c_str(), dwColor );
            }
            //else if ( m_bShopOpen && (m_pItemData->sBasicOp.dwFlags & TRADE_SALE) ) // 팔때 가격
			else if ( m_bShopOpen 
				&& m_pItemData->sBasicOp.dwFlags & TRADE_SALE
				&& m_pItemData->sBasicOp.dwReqContributionPoint == 0 ) // 팔때 가격
			{
				bDisplayPrice = true;
                m_pItemLabel5->AddText( "", ITEM_INFOR_COLOR::dwDefault );

				volatile float fSHOP_RATE = GLGaeaClient::GetInstance().GetCharacter()->GetSaleRate();
                volatile float fSALE_DISCOUNT = fSHOP_RATE * 0.01f;

                volatile DWORD dwPrice = m_pItemData->GETSELLPRICE ( sItemCustom.wTurnNum );
                volatile DWORD dwSALE_PRICE = DWORD ( dwPrice * fSALE_DISCOUNT );					

                //	커미션 액수
                volatile DWORD dwDISPRICE = m_pItemData->GETSELLPRICE ( sItemCustom.wTurnNum );
                DWORD dwCOMMISSION_MONEY = dwDISPRICE - dwSALE_PRICE;

                std::string strMoney = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( dwSALE_PRICE, 3, "," );
                strText = sc::string::format( "%s :%s", ID2GAMEWORD("ITEM_BASIC_INFO", 14), strMoney );

                m_pItemLabel5->AddText( strText.c_str(), MONEYCOLOR::GetMoneyColor((LONGLONG)dwSALE_PRICE) );
            }
        }
    }

	// Info : 요구 포인트
	{
		//  착용조건->요구활동
		if ( m_pItemData->sBasicOp.dwReqActivityPoint /*|| m_pItemData->sBasicOp.wReqActPointUP*/ )
		{
			if ( !bDisplayPrice && !bDisplayReq )
				m_pItemLabel5->AddText( "", ITEM_INFOR_COLOR::dwDefault );
			
			std::string strText;

			//int emType = m_pItemData->sBasicOp.emReqActivityType;
			bool bReqActPoint = true;
			
			/*if ( GLUseFeatures::GetInstance().IsUsingDataIntegrate() )
			{
				strText = sc::string::format( "%s :%s(%d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION", 3), 
					CGameTextMan::GetInstance().GetCommentText("ACTIVITY_CLASS", (INT)emType).GetString(), m_pItemData->sBasicOp.wReqActPointDW );
			}
			else*/
			{
				strText = sc::string::format( "%s : %d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION", 3), m_pItemData->sBasicOp.dwReqActivityPoint );
			}

			/*if ( m_pItemData->sBasicOp.wReqActPointUP )
			{
				strText += sc::string::format( "~%d)",m_pItemData->sBasicOp.wReqActPointUP );
				bReqActPoint = (m_pItemData->sBasicOp.wReqActPointUP >= m_pGaeaClient->GetCharacter()->m_llActivityPoint[emType] );
			}
			else
			{
				strText += ")";
			}*/

			bReqActPoint = bReqActPoint && (m_pItemData->sBasicOp.dwReqActivityPoint <= GLGaeaClient::GetInstance().GetCharacter()->m_dwActivityPoint );

			DWORD dwColor = 0;

			if ( bReqActPoint )
				dwColor = NS_UITEXTCOLOR::dwDefault;
			else
				dwColor = NS_UITEXTCOLOR::dwReqWrong;

			m_pItemLabel5->AddText( strText.c_str(), dwColor );

			bDisplayReq = true;
		}

		//  착용조건->기여도
		if ( m_pItemData->sBasicOp.dwReqContributionPoint )
		{
			if ( !bDisplayPrice && !bDisplayReq )
				m_pItemLabel5->AddText( "", ITEM_INFOR_COLOR::dwDefault );

			bool bReqPoint = true;

			std::string strText;
			strText = sc::string::format( "%s :%d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION", 4), m_pItemData->sBasicOp.dwReqContributionPoint );

			bReqPoint = (m_pItemData->sBasicOp.dwReqContributionPoint <= GLGaeaClient::GetInstance().GetCharacter()->m_llContributionPoint);

			DWORD dwColor = 0;

			if ( bReqPoint )
				dwColor = ITEM_INFOR_COLOR::dwDefault;
			else
				dwColor = ITEM_INFOR_COLOR::dwReqWrong;

			m_pItemLabel5->AddText( strText.c_str(), dwColor );

			bDisplayReq = true;
		}
	}

    // Info : 만료일
    {
        // 코스츔 기간 표시
        if ( sItemCustom.tDISGUISE != 0 )
        {				
            CTime cTime(sItemCustom.tDISGUISE);

            //	서버 타임으로 변환( TimeZone 계산 )
            //m_pGaeaClient->GetConvertServerTime( cTime );

            if ( cTime.GetYear() > 1970 )
            {
				m_pItemLabel5->AddText( "", ITEM_INFOR_COLOR::dwDefault );

                std::string strExpireDate;

                strExpireDate = sc::string::format( ID2GAMEWORD("ITEM_EXPIRE_DATE"),
                    (cTime.GetYear()%2000),  cTime.GetMonth(), cTime.GetDay(), cTime.GetHour(), cTime.GetMinute() );

                strText = sc::string::format( "%s :%s", ID2GAMEWORD("ITEM_BASIC_INFO", 7), strExpireDate );			
                m_pItemLabel5->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwLimitTime );
            }
        }

        //	기간표시
        if ( m_pItemData->IsTIMELMT() )
		//if ( sItemCustom.IsTimeLimit( SITEMCUSTOM::CHECKFLAGOP_NORMAL ) )
        {
            CTime cTime(sItemCustom.tBORNTIME);

            //	서버 타임으로 변환( TimeZone 계산 )
            //m_pGaeaClient->GetConvertServerTime( cTime );

            if ( cTime.GetYear() > 1970 )
            {
				m_pItemLabel5->AddText( "", NS_UITEXTCOLOR::dwDefault );

                CTimeSpan sLMT(m_pItemData->sDrugOp.tTIME_LMT);
                cTime += sLMT /*+ sItemCustom.GetPeriodExBasic()*/;

                std::string strExpireDate;
                strExpireDate = sc::string::format( ID2GAMEWORD("ITEM_EXPIRE_DATE"),
                    (cTime.GetYear ()%2000), cTime.GetMonth(), cTime.GetDay(), cTime.GetHour(), cTime.GetMinute() );

				/*INT nPriodExMaxCount(-1);
				if ( ItemPeriodExtension::Manager::GetInstance().IsRegItem(sItemCustom.GetNativeID(), nPriodExMaxCount) )
				{
					if ( nPriodExMaxCount == 0 )
						strExpireDate.append( ID2GAMEWORD( "ITEM_PERIODEX_TOOLTIP", 0 ) );
					else
						strExpireDate.append( sc::string::format(ID2GAMEWORD( "ITEM_PERIODEX_TOOLTIP", 1 ), sItemCustom.cPeriodExtensionCount - nPriodExMaxCount ) );
				}*/

                strText = sc::string::format( "%s :%s", ID2GAMEWORD("ITEM_BASIC_INFO", 4), strExpireDate );			
                m_pItemLabel5->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwLimitTime );
            }
        }
		// 코스튬 능력치 시간
		/*if( sItemCustom.costumeUser.tEndTime > 0 )
		{
			CTime cTime(sItemCustom.costumeUser.tEndTime);

			//	서버 타임으로 변환( TimeZone 계산 )
			m_pGaeaClient->GetConvertServerTime( cTime );

			if ( cTime.GetYear() > 1970 )
			{
				m_pItemLabel5->AddText( "", ITEM_INFOR_COLOR::dwDefault );

				std::string strExpireDate;

				strExpireDate = sc::string::format( ID2GAMEWORD("ITEM_EXPIRE_DATE"),
					(cTime.GetYear()%2000),  cTime.GetMonth(), cTime.GetDay(), cTime.GetHour(), cTime.GetMinute() );

				strText = sc::string::format( "%s :%s", ID2GAMEWORD("ITEM_BASIC_INFO", 20), strExpireDate );			
				m_pItemLabel5->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwLimitTime );
			}
		}
		// 일시강화 만료일
		/*if( sItemCustom.sTLGrind.tFireTime > 0 )
		{
			CTime cTime(sItemCustom.sTLGrind.tFireTime);

			//	서버 타임으로 변환( TimeZone 계산 )
			m_pGaeaClient->GetConvertServerTime( cTime );

			if ( cTime.GetYear() > 1970 )
			{
				m_pItemLabel5->AddText( "", ITEM_INFOR_COLOR::dwDefault );

				std::string strExpireDate;

				strExpireDate = sc::string::format( ID2GAMEWORD("ITEM_EXPIRE_DATE"),
					(cTime.GetYear()%2000),  cTime.GetMonth(), cTime.GetDay(), cTime.GetHour(), cTime.GetMinute() );

				int iValue = 0;

				if ( m_pItemData->sSuitOp.gdDamage == GLPADATA(0,0) )
				{
					iValue = sItemCustom.cDEFENSE;
				}
				else
				{
					iValue = sItemCustom.cDAMAGE;
				}

				strText = sc::string::format( "%s : (+%d) %s", ID2GAMEWORD("ITEM_BASIC_INFO", 22), iValue, strExpireDate );			
				m_pItemLabel5->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwLimitTime );
			}
		}*/
    }
}



void CItemInfoTooltip::SetInformationLabel6( const SITEMCUSTOM& sItemCustom )
{
    std::string strText = "";

	SITEM*	pItem = GLItemMan::GetInstance().GetItem( sItemCustom.sNativeID );

	if ( strlen(pItem->sRandomOpt.szNAME) > 3 )
	{
		strText = sc::string::format ( "%s", ID2GAMEWORD("ITEM_REFORM_SHOW",0) );
		m_pItemLabel6->AddText( strText.c_str(), NS_UITEXTCOLOR::DEFAULT );

		const bool _bShowMinMax(DxInputDevice::GetInstance().GetKeyState ( DIK_LCONTROL ) & DXKEY_DOWNED ? true : false);
		if ( _bShowMinMax == true )
		{
			/*const RandomOption::Manager& _managerRandomOption(RandomOption::Manager::getInstance());
			const RandomOption::Entity* const _pEntity(_managerRandomOption.find(m_pItemData->sRandomOpt.szNAME));		
			if ( (_pEntity != NULL) && (GLUseFeatures::GetInstance().IsUsingRandomOptionClientView() == true) )
			{
				RandomOption::Information _information;
				if ( _pEntity->getInformation(RandomOption::GENERATE_REMODEL, RandomOption::RANDOM_OPTION(nRandOptType), _information) == true )
				{
					if ( bIsPerRandOpt == true )
						strDescText += sc::string::format( "(%.2f~%.2f)", _information.valueFloatLow, _information.valueFloatHigh);
					else
						strDescText += sc::string::format( "(%d~%d)", _information.valueIntegerLow, _information.valueIntegerHigh);
				}			
			}*/
			strText = sc::string::format ( "%s", ID2GAMEWORD("ITEM_REFORM_SHOW",1) );
			m_pItemLabel6->AddText( strText.c_str(), NS_UITEXTCOLOR::DEFAULT );
			
			strText = sc::string::format ( "[%s", ID2GAMEWORD("ITEM_REFORM_SHOW",2) );
			strText += _T(']');
			m_pItemLabel6->AddText( strText.c_str(), NS_UITEXTCOLOR::PRIVATE );

			SRANDOM_GEN *pRANDOM_SET = GLItemMan::GetInstance().GetItemRandomOpt( sItemCustom.sNativeID );
			if( !pRANDOM_SET )	return;

			for( int i=0;i<pRANDOM_SET->dwSET_NUM;++i)
			{
				if( pRANDOM_SET->sSETR[i].wTYPE == EMR_OPT_NULL ) continue;
				const SRANDOM_OPT_SET* pSET = &pRANDOM_SET->sSETR[i];

				//CString strDescText;
				INT nType = pSET->wTYPE;
				float fHigh = 0.0f;
				float fLow = 9999.0f;

				for( WORD j=0; j<pSET->wSECT; ++j )
				{
					if( pSET->pSECT[j].fLOW < fLow ) fLow = pSET->pSECT[j].fLOW;
					if( pSET->pSECT[j].fHIGH > fHigh ) fHigh = pSET->pSECT[j].fHIGH;
				}
				strText = sc::string::format ( "%s: %.2f~%.2f", ID2GAMEWORD("ITEM_RANDOM_OPTION", nType ), fLow, fHigh );
				if( sItemCustom.IsPerRandOpt( nType ) ) strText += _T('%');
				m_pItemLabel6->AddText( strText.c_str(), NS_UITEXTCOLOR::dwRandomAbility );
			}
		}
	}
}


void CItemInfoTooltip::DisplayVehicleItem( CBasicTextBox* m_pTextBox, const SITEMCUSTOM& sItemCustom )
{
    std::string strText = "";

    //strText = sc::string::format( "%s %s", ID2GAMEWORD ("ITEM_VEHICLE_NAME",0), COMMENT::VEHICLE_TYPE[pItemData->sVehicle.emVehicleType].c_str() );
    //m_pTextBox->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwDefault );

    bool bInfo = true;


	VEHICLEITEMINFO_MAP_ITER iter = GLGaeaClient::GetInstance().GetCharacter()->m_mapVEHICLEItemInfo.find ( sItemCustom.dwVehicleID );
    if ( iter == GLGaeaClient::GetInstance().GetCharacter()->m_mapVEHICLEItemInfo.end() )
    {
        iter = GLGaeaClient::GetInstance().GetCharacter()->m_mapVEHICLEItemInfoTemp.find ( sItemCustom.dwVehicleID );
        if (iter == GLGaeaClient::GetInstance().GetCharacter()->m_mapVEHICLEItemInfoTemp.end())
            bInfo = false;
    }


    float fAddOnSpeed = 0.0f;

    if ( bInfo ) 
    {
        SVEHICLEITEMINFO sVehicleItemInfo = (*iter).second;

        for ( int i = 0; i < VEHICLE_ACCETYPE_SIZE; ++i ) 
        {							
            fAddOnSpeed += sVehicleItemInfo.m_PutOnItems[i].GETMOVESPEED();
        }
    }

    if ( bInfo ) 
    {
        SVEHICLEITEMINFO sVehicleItemInfo = (*iter).second;

        for ( int i = 0; i < VEHICLE_ACCETYPE_SIZE; ++i ) 
        {							
            SITEM* pItem = GLItemMan::GetInstance().GetItem ( sVehicleItemInfo.m_PutOnItems[i].sNativeID );
            if ( pItem )
            {
                strText = sc::string::format("%s %s", ID2GAMEWORD ("ITEM_VEHICLE_SLOT", i), pItem->GetName() );

                m_pTextBox->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwDefault );
            }
        }

        strText = sc::string::format( "%s", ID2GAMEWORD ("ITEM_VEHICLE_BOOST_LEARN", sVehicleItemInfo.m_bBooster ) );
        m_pTextBox->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwDefault );

        strText = sc::string::format( "%s %.2f%%", ID2GAMEWORD ("ITEM_VEHICLE_BATTERY", 0), sVehicleItemInfo.m_nFull/10.0f );
        m_pTextBox->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwDefault );
    }
}

void CItemInfoTooltip::DisplayPetCardItem( CBasicTextBox* m_pTextBox, const SITEMCUSTOM& sItemCustom )
{
    std::string strText = "";

    if ( sItemCustom.dwPetID == 0 )
        return;

    PETCARDINFO_MAP_ITER iter = GLGaeaClient::GetInstance().GetCharacter()->m_mapPETCardInfo.find( sItemCustom.dwPetID );
    if ( iter == GLGaeaClient::GetInstance().GetCharacter()->m_mapPETCardInfo.end() )
    {
        iter = GLGaeaClient::GetInstance().GetCharacter()->m_mapPETCardInfoTemp.find ( sItemCustom.dwPetID );
        if ( iter == GLGaeaClient::GetInstance().GetCharacter()->m_mapPETCardInfoTemp.end() ) return;
    }

    SPETCARDINFO sPetCardInfo = (*iter).second;

    //strText = sc::string::format( "%s %s", ID2GAMEWORD("ITEM_PET_NAME",0), sPetCardInfo.m_szName );
    //m_pTextBox->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwDefault );

	/*if ( GLUseFeatures::GetInstance().IsUsingDataIntegrate() )
	{
		std::vector<std::string>	vecPET_TYPE;
		CGameTextMan::GetInstance().LoadHelpUnknownCommentString("PET_TYPE", vecPET_TYPE, CGameTextMan::EM_COMMENT_TEXT);
		strText = sc::string::format( "%s %s", ID2GAMEWORD("ITEM_PET_TYPE", 0), vecPET_TYPE[sPetCardInfo.m_emTYPE].c_str() );
	}
	else*/
	{
		strText = sc::string::format( "%s %s", ID2GAMEWORD("ITEM_PET_TYPE", 0), COMMENT::PET_TYPE[sPetCardInfo.m_emTYPE].c_str() );
	}
    m_pTextBox->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwDefault );

    strText = sc::string::format( "%s %.2f%%", ID2GAMEWORD("ITEM_PET_FULL", 0), sPetCardInfo.m_nFull/10.0f );
    m_pTextBox->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwDefault );

    /*if ( 2 <= sPetCardInfo.m_wMaxSkillSlot )
    {
        strText = sc::string::format( "%s", ID2GAMEWORD("ITEM_PET_DUALSKILL", 1) );
        m_pTextBox->AddText( strText.c_str(), NS_UITEXTCOLOR::dwDefault );
    }
    else
    {
        strText = sc::string::format( "%s", ID2GAMEWORD("ITEM_PET_DUALSKILL", 0) );
        m_pTextBox->AddText( strText.c_str(), NS_UITEXTCOLOR::dwDefault );
    }*/

	if ( sPetCardInfo.m_bDualSkill )
	{
        strText = sc::string::format( "%s", ID2GAMEWORD("ITEM_PET_DUALSKILL", 1) );
        m_pTextBox->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwDefault );
	}
	else
	{
        strText = sc::string::format( "%s", ID2GAMEWORD("ITEM_PET_DUALSKILL", 0) );
        m_pTextBox->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwDefault );
	}


    SITEM* pItem = GLItemMan::GetInstance().GetItem( sPetCardInfo.m_PutOnItems[PET_ACCETYPE_A].sNativeID );
    if ( pItem )
    {
        strText = sc::string::format("%s %s", ID2GAMEWORD("ITEM_PET_SLOTA", 0), pItem->GetName() );
        m_pTextBox->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwDefault );
    }

    pItem = GLItemMan::GetInstance().GetItem( sPetCardInfo.m_PutOnItems[PET_ACCETYPE_B].sNativeID );
    if ( pItem )
    {
        strText = sc::string::format("%s %s", ID2GAMEWORD("ITEM_PET_SLOTB", 0), pItem->GetName() );
        m_pTextBox->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwDefault );
    }

    if ( sItemCustom.tDISGUISE != 0 )
    {
        const CTime& currentTime = GLGaeaClient::GetInstance().GetCurrentTime();
        CTime startTime   = sItemCustom.tBORNTIME;
        CTimeSpan timeSpan = currentTime - startTime;

		//CTime currentTime = GLGaeaClient::GetInstance().GetCurrentTime();
		//CTime startTime   = sItemCustom.tBORNTIME;
		//CTimeSpan timeSpan = currentTime - startTime;

        if( timeSpan.GetTotalSeconds() < sItemCustom.tDISGUISE )
        {
            strText = sc::string::format("[%s]", ID2GAMEWORD("ITEM_PET_USE_SKINPACK", 0) );
            m_pTextBox->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwDefault );
            strText = sc::string::format("%s : %dsec", ID2GAMEWORD("CLUB_BATTLE_TIME", 0), sItemCustom.tDISGUISE - timeSpan.GetTotalSeconds() );
            m_pTextBox->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwDefault );
        }
    }

    bool bDisplayEnablePetSkill = false;
    PETSKILL_MAP_CITER pos = sPetCardInfo.m_ExpSkills.begin();
    PETSKILL_MAP_CITER pos_end = sPetCardInfo.m_ExpSkills.end();
    for ( ;pos != pos_end; ++pos )
    {
        const PETSKILL& sPetSkill = (*pos).second;
        PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sPetSkill.sNativeID );
        if ( pSKILL )
        {
            if ( !bDisplayEnablePetSkill )
            {
                bDisplayEnablePetSkill = true;
                m_pTextBox->AddText( ID2GAMEWORD("ITEM_PET_SKILLS", 0 ), ITEM_INFOR_COLOR::dwDefault );
            }

            strText = sc::string::format("%s", pSKILL->GetName() );
            m_pTextBox->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwDefault );
        }
    }
}

void CItemInfoTooltip::DisplayLunchBoxItem( CBasicTextBox* m_pTextBox, const SITEMCUSTOM& sItemCustom )
{
    std::string strText = "";

    SNATIVEID sNativeID = sItemCustom.sNativeID;
	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();	
    //const SITEM* pItemData = GLogicData::GetInstance().GetItem( sNativeID );
	const SITEM* pItemData = GLItemMan::GetInstance().GetItem( sNativeID );
    SNATIVEID sSkillID = m_pItemData->sSkillBookOp.sSkill_ID;
    //doesn't have yet - this is for skill link id -- cndev
	//WORD wSkillLevel = m_pItemData->sSkillBookOp.wSkill_Level;

    //	Note : 스킬 정보 가져옴.
    PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( sSkillID.wMainID, sSkillID.wSubID );
    if ( !pSkill )
    {
        return;
    }
	if ( pSkill )	
	{

		const SKILL::SAPPLY& sAPPLY = pSkill->m_sAPPLY;
		for( int nImpact=0; nImpact<SKILL::MAX_IMPACT; ++nImpact )
		{
			if ( sAPPLY.sImpacts[nImpact].emADDON != EMIMPACTA_NONE )
			{
				WORD wLevel = 0;
				const float fADDON_VAR = sAPPLY.sImpacts[nImpact].fADDON_VAR[wLevel];
				if ( 0 != fADDON_VAR )
				{
					float fADDON_VAR_SCALE = fADDON_VAR * COMMENT::IMPACT_ADDON_SCALE[sAPPLY.sImpacts[nImpact].emADDON];
									
					if ( COMMENT::IsIMPACT_ADDON_PER(sAPPLY.sImpacts[nImpact].emADDON) )
					{
						strText = sc::string::format ( "%s:%2.2f%%", COMMENT::IMPACT_ADDON[sAPPLY.sImpacts[nImpact].emADDON].c_str(), fADDON_VAR_SCALE );
					}
					else
					{
						strText = sc::string::format ( "%s:%2.2f", COMMENT::IMPACT_ADDON[sAPPLY.sImpacts[nImpact].emADDON].c_str(), fADDON_VAR_SCALE );
					}
					m_pTextBox->AddText(strText.c_str(), ITEM_INFOR_COLOR::dwDefault );
				}
			}
		}
	}


    //CString strImpact;
    //pSkill->GetDescFormatByLevel( strImpact, wSkillLevel, GLSKILL::DESC_FORMAT_IMPACT | GLSKILL::DESC_FORMAT_SPEC );
    //m_pTextBox->AddText( strImpact, ITEM_INFOR_COLOR::dwDefault );

    // Note : 효과정보 출력
    //CString strImpact;
    //pSkill->GetDescFormatByLevel( strImpact, wSkillLevel, GLSKILL::DESC_FORMAT_IMPACT | GLSKILL::DESC_FORMAT_SPEC );
    //m_pTextBox->AddText( strImpact, ITEM_INFOR_COLOR::dwDefault );
}

void CItemInfoTooltip::DisplayPetDualSkillItem( CBasicTextBox* m_pTextBox, const SITEMCUSTOM& sItemCustom )
{
	GLPetClient* pPetClient = GLGaeaClient::GetInstance().GetPetClient();

    //	이미 듀얼 스킬을 배웠다면
    if ( pPetClient->m_bDualSkill )
        m_pTextBox->AddText( ID2GAMEWORD( "ITEM_SKILL_CONDITION", 0 ), NS_UITEXTCOLOR::dwReqWrong );
}

void CItemInfoTooltip::CutZero( CString& cstr, int ncount  )
{
	// '.' 점이 없다는 것은 소수점 이하 자리가 없으므로 리턴
	if (cstr.Find('.') == -1)
		return;

	CString temp = cstr;

	//ncount 소숫점 몇째 자리 까지 잘라낼찌 정한다
	for (int i = 0; i < ncount + 1; i++ )
	{
		temp = cstr.Right( 1 );

		if ( temp.Find('0') != -1 || temp.Find('.') != -1 )
			cstr = cstr.Left( cstr.GetLength() - 1 );
		else
			break;
	}
}

void CItemInfoTooltip::SetBGAlpa( WORD wValue )
{
	m_pLineBox->SetDiffuseAlpha(wValue);
}

void CItemInfoTooltip::ResetInformation()
{
    m_pItemImage->ResetItem();
    m_pItemName->ClearText();
    m_pItemLabel1->ClearText();
    m_pItemLabel2->ClearText();
    m_pItemLabel3->ClearText();
    m_pItemLabel4->ClearText();
	m_pItemLabel4_1->ClearText();
    m_pItemLabel5->ClearText();
	m_pItemLabel6->ClearText();

	m_pItemSet->ClearText();

	//m_pWishListLine->SetVisibleSingle( FALSE );
	//m_pWishListItem->ClearText();

    m_pResistSpirit->SetVisibleSingle( FALSE );
    m_pResistFire->SetVisibleSingle( FALSE );
    m_pResistIce->SetVisibleSingle( FALSE );
    m_pResistElectric->SetVisibleSingle( FALSE );
    m_pResistPoison->SetVisibleSingle( FALSE );

    m_pResistSpiritText->ClearText();
    m_pResistFireText->ClearText();
    m_pResistIceText->ClearText();
    m_pResistElectricText->ClearText();
    m_pResistPoisonText->ClearText();

	//m_pWishItem.reset();

    m_bDisplayResist = false;
}


