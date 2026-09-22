#include "StdAfx.h"
#include "PandoraBoxResultsItemSlot.h"

#include "./DxFontMan.h"
#include "./d3dfont.h"

#include "./BasicButton.h"
#include "./BasicTextBox.h"
#include "./BasicTextButton.h"
#include "./BasicLineBox.h"
#include "./UITextControl.h"
#include "./GameTextControl.h"
#include "./BasicScrollBarEx.h"
#include "./BasicScrollThumbFrame.h"
#include "./Util/CheckBox.h"
#include "./ItemImage.h"
#include "./InnerInterface.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const float CPandoraBoxResultsItemSlot::fBLINK_TIME_LOOP = 0.2f;

CPandoraBoxResultsItemSlot::CPandoraBoxResultsItemSlot ( GLGaeaClient* pGaeaClient )
	: m_pGaeaClient ( pGaeaClient )
	, m_pSelectImage(NULL)
	, m_pTextName(NULL)
	, m_pTextPrice(NULL)
	, m_pItemImage(NULL)
	, m_fBLINK_TIME ( 0.0f )
	, m_bAlarm ( false )
	, m_sPandoraItem( GLPANDORA_BOX() )
{
}

CPandoraBoxResultsItemSlot::~CPandoraBoxResultsItemSlot ()
{
}

void CPandoraBoxResultsItemSlot::CreateSubControl ()
{
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_FLAG );
	CD3DFontPar* pFont9S = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "PANDORA_ITEM_IMAGE_BACK0" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "PANDORA_ITEM_IMAGE_RES0" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "PANDORA_ITEM_CURR_1_BACK" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "PANDORA_ITEM_CURR_2_BACK" );
	RegisterControl ( pBasicLineBox );

	m_pSelectImage = new CUIControl;
	m_pSelectImage->CreateSub ( this, "PANDORA_ITEM_IMAGE_SELECT_RESULT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pSelectImage->SetVisibleSingle( FALSE );
	RegisterControl ( m_pSelectImage );

	m_pTextName = new CBasicTextBox;
	m_pTextName->CreateSub ( this, "PANDORA_ITEM_CURR_2_BACK" );
	m_pTextName->SetFont ( pFont9S );
	m_pTextName->SetTextAlign( TEXT_ALIGN_CENTER_X );
	RegisterControl ( m_pTextName );

	m_pTextPrice = new CBasicTextBox;
	m_pTextPrice->CreateSub ( this, "PANDORA_ITEM_CURR_1_BACK" );
	m_pTextPrice->SetFont ( pFont8 );
	m_pTextPrice->SetTextAlign( TEXT_ALIGN_CENTER_X );
	RegisterControl ( m_pTextPrice );

	m_pItemImage = new CItemImage;
	m_pItemImage->CreateSub ( this, "PANDORA_ITEM_IMAGE_RES", UI_FLAG_DEFAULT, PANDORA_BOX_SLOT_ITEM_IMAGE );
	m_pItemImage->CreateSubControl ();
	RegisterControl ( m_pItemImage );

	m_pBuyButton = new CBasicTextButton;
	m_pBuyButton->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, PANDORA_BOX_SLOT_ITEM_BUY );
	m_pBuyButton->CreateBaseButton ( "PANDORA_BUY_BTN", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, "Buy" );
	RegisterControl ( m_pBuyButton );

	m_pRemoveButton = new CBasicButton;
	m_pRemoveButton->CreateSub ( this, "PANDORABOX_SLOT_REMOVED", UI_FLAG_DEFAULT, PANDORA_BOX_SLOT_ITEM_REMOVE );
	m_pRemoveButton->CreateFlip ( "PANDORABOX_SLOT_REMOVED_F", CBasicButton::CLICK_FLIP );
	m_pRemoveButton->SetControlNameEx ( "PANDORABOX_SLOT_REMOVED" );
	RegisterControl ( m_pRemoveButton );

}

void CPandoraBoxResultsItemSlot::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case PANDORA_BOX_SLOT_ITEM_IMAGE:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				AddMessageEx ( UIMSG_PANDORA_BOX_RESULT_SLOT_ITEM_IMAGE );
			}
		}break;
	case PANDORA_BOX_SLOT_ITEM_BUY:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				AddMessageEx ( UIMSG_PANDORA_BOX_RESULT_SLOT_ITEM_BUY );
			}
		}break;
	case PANDORA_BOX_SLOT_ITEM_REMOVE:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				AddMessageEx ( UIMSG_PANDORA_BOX_RESULT_SLOT_ITEM_REMOVE );
			}
		}break;
	};
}


void CPandoraBoxResultsItemSlot::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( m_bAlarm )
	{		
		m_fBLINK_TIME += fElapsedTime;
		if ( fBLINK_TIME_LOOP <= m_fBLINK_TIME )
		{
			BOOL bVisible = m_pSelectImage->IsVisible ();
			m_pSelectImage->SetVisibleSingle ( !bVisible );
			m_fBLINK_TIME = 0.0f;
		}
	}
}

void CPandoraBoxResultsItemSlot::ItemSet( GLPANDORA_BOX sShopItem )
{
	ItemReset();

	SITEM* pITEM = GLItemMan::GetInstance().GetItem( sShopItem.sItemID );
	if ( pITEM )
	{
		if ( m_pTextName )		
		{
			m_pTextName->SetText( pITEM->GetName(), NS_UITEXTCOLOR::GOLD );
		}

		if ( m_pTextPrice )		
		{
			CString strTemp = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( (LONGLONG)sShopItem.llGold, 3, "," );

			m_pTextPrice->SetText( strTemp.GetString(), NS_UITEXTCOLOR::WHITE );
		}

		if ( m_pItemImage )		
		{
			m_pItemImage->SetSkill( pITEM->sBasicOp.sICONID, pITEM->GetInventoryFile() );
		}

		if ( m_pRemoveButton )  m_pRemoveButton->SetVisibleSingle( TRUE );
		if ( m_pBuyButton )		m_pBuyButton->SetVisibleSingle( TRUE );

		m_bAlarm = ( pITEM->sBasicOp.emLevel > LEVEL_RARE );

		m_sPandoraItem = sShopItem;
	}
}

void CPandoraBoxResultsItemSlot::ItemReset()
{
	if ( m_pTextName )		m_pTextName->ClearText();
	if ( m_pTextPrice )		m_pTextPrice->ClearText();
	if ( m_pItemImage )		m_pItemImage->ResetItem();
	if ( m_pSelectImage )	m_pSelectImage->SetVisibleSingle( FALSE );
	if ( m_pRemoveButton )  m_pRemoveButton->SetVisibleSingle( FALSE );
	if ( m_pBuyButton )		m_pBuyButton->SetVisibleSingle( FALSE );
	m_sPandoraItem = GLPANDORA_BOX();
}

void CPandoraBoxResultsItemSlot::ItemSelect( BOOL bSelect )
{
	if ( m_pSelectImage )
		m_pSelectImage->SetVisibleSingle( bSelect );
}