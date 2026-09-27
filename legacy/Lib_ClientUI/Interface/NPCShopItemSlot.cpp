#include "StdAfx.h"
#include "NPCShopItemSlot.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "ItemImage.h"

#include "UITextControl.h"
#include "GameTextControl.h"
#include "GLItemMan.h"
#include "GLItem.h"

#include "../Lib_Engine/DxCommon/d3dfont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CNPCShopItemSlot::CNPCShopItemSlot ()
	: m_pSelectImage(NULL)
	, m_pTextName(NULL)
	, m_pTextPrice(NULL)
	, m_pItemImage(NULL)
	, m_sNPCShopItem( SNPC_SHOP_ITEM() )
{
}

CNPCShopItemSlot::~CNPCShopItemSlot ()
{
}

void CNPCShopItemSlot::CreateSubControl ()
{
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "POINT_SHOP_ITEM_LIST_NODE_BACK" );
	RegisterControl ( pBasicLineBox );

	m_pSelectImage = new CUIControl;
	m_pSelectImage->CreateSub ( this, "NPC_SHOP_ITEM_LIST_NODE_SELECT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pSelectImage->SetVisibleSingle( FALSE );
	RegisterControl ( m_pSelectImage );

	m_pTextName = new CBasicTextBox;
	m_pTextName->CreateSub ( this, "NPC_SHOP_ITEM_LIST_NODE_NAME" );
	m_pTextName->SetFont ( pFont8 );
	m_pTextName->SetTextAlign( TEXT_ALIGN_LEFT );
	RegisterControl ( m_pTextName );

	m_pTextPrice = new CBasicTextBox;
	m_pTextPrice->CreateSub ( this, "NPC_SHOP_ITEM_LIST_NODE_PRICE" );
	m_pTextPrice->SetFont ( pFont8 );
	m_pTextPrice->SetTextAlign( TEXT_ALIGN_LEFT );
	RegisterControl ( m_pTextPrice );

	m_pItemImage = new CItemImage;
	m_pItemImage->CreateSub ( this, "NPC_SHOP_ITEM_LIST_NODE_IMAGE", UI_FLAG_DEFAULT, NPC_SHOP_SLOT_ITEM_IMAGE );
	m_pItemImage->CreateSubControl ();
	RegisterControl ( m_pItemImage );

}

void CNPCShopItemSlot::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case NPC_SHOP_SLOT_ITEM_IMAGE:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				AddMessageEx ( UIMSG_NPC_SHOP_SLOT_ITEM_IMAGE );
			}
		}break;
	};
}

void CNPCShopItemSlot::ItemSet( SNPC_SHOP_ITEM sShopItem )
{
	ItemReset();

	SITEM* pITEM = GLItemMan::GetInstance().GetItem( sShopItem.sidItem );
	if ( pITEM )
	{
		if ( m_pTextName )		
		{
			m_pTextName->SetText( pITEM->GetName(), COMMENT::ITEMCOLOR[pITEM->sBasicOp.emLevel] );
		}

		if ( m_pTextPrice )		
		{
			CString strTemp = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( (LONGLONG)pITEM->sBasicOp.dwBuyPrice, 3, "," );
			m_pTextPrice->SetText( strTemp.GetString(), NS_UITEXTCOLOR::WHITE );
		}

		if ( m_pItemImage )		
		{
			m_pItemImage->SetItem( sShopItem.sidItem );
		}

		m_sNPCShopItem = sShopItem;
	}
}

void CNPCShopItemSlot::ItemReset()
{
	if ( m_pTextName )	m_pTextName->ClearText();
	if ( m_pTextPrice )	m_pTextPrice->ClearText();
	if ( m_pItemImage )	m_pItemImage->ResetItem();
	if ( m_pSelectImage )	m_pSelectImage->SetVisibleSingle( FALSE );
	m_sNPCShopItem = SNPC_SHOP_ITEM();
}

void CNPCShopItemSlot::ItemSelect( BOOL bSelect )
{
	if ( m_pSelectImage )
		m_pSelectImage->SetVisibleSingle( bSelect );
}