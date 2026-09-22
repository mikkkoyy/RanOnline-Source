#include "StdAfx.h"
#include "PandoraBoxItemSlot.h"

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

CPandoraBoxItemSlot::CPandoraBoxItemSlot ( GLGaeaClient* pGaeaClient )
	: m_pGaeaClient ( pGaeaClient )
	, m_pSelectImage(NULL)
	, m_pTextName(NULL)
	, m_pItemImage(NULL)
	, m_sPandoraItem( GLPANDORA_BOX() )
{
}

CPandoraBoxItemSlot::~CPandoraBoxItemSlot ()
{
}

void CPandoraBoxItemSlot::CreateSubControl ()
{
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_FLAG );
	CD3DFontPar* pFont9S = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	m_pSelectImage = new CUIControl;
	m_pSelectImage->CreateSub ( this, "PANDORA_BOX_ITEM_LIST_NODE_SELECT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pSelectImage->SetVisibleSingle( FALSE );
	RegisterControl ( m_pSelectImage );

	m_pTextName = new CBasicTextBox;
	m_pTextName->CreateSub ( this, "PANDORA_ITEM_LIST_ITEM_NAME", UI_FLAG_CENTER_X | UI_FLAG_CENTER_Y );
	m_pTextName->SetFont ( pFont9S );
	m_pTextName->SetAutoChangeFontSize(TRUE);
	m_pTextName->SetTextAlign( TEXT_ALIGN_CENTER_X );
	RegisterControl ( m_pTextName );

	m_pItemImage = new CItemImage;
	m_pItemImage->CreateSub ( this, "PANDORABOX_SLOT_ITEMIMAGE", UI_FLAG_DEFAULT, PANDORA_BOX_SLOT_ITEM_IMAGE );
	m_pItemImage->CreateSubControl ();
	RegisterControl ( m_pItemImage );

}

void CPandoraBoxItemSlot::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case PANDORA_BOX_SLOT_ITEM_IMAGE:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				AddMessageEx ( UIMSG_PANDORA_BOX_SLOT_ITEM_IMAGE );
			}
		}break;
	};
}

void CPandoraBoxItemSlot::ItemSet( GLPANDORA_BOX sShopItem )
{
	ItemReset();

	SITEM* pITEM = GLItemMan::GetInstance().GetItem( sShopItem.sItemID );
	if ( pITEM )
	{
		if ( m_pTextName )		
		{
			m_pTextName->SetText( pITEM->GetName(), NS_UITEXTCOLOR::GOLD );
		}

		if ( m_pItemImage )		
		{
			m_pItemImage->SetSkill( pITEM->sBasicOp.sICONID, pITEM->GetInventoryFile() );
		}

		m_sPandoraItem = sShopItem;
	}
}

void CPandoraBoxItemSlot::ItemReset()
{
	if ( m_pTextName )	m_pTextName->ClearText();
	if ( m_pItemImage )	m_pItemImage->ResetItem();
	if ( m_pSelectImage )	m_pSelectImage->SetVisibleSingle( FALSE );
	m_sPandoraItem = GLPANDORA_BOX();
}

void CPandoraBoxItemSlot::ItemSelect( BOOL bSelect )
{
	if ( m_pSelectImage )
		m_pSelectImage->SetVisibleSingle( bSelect );
}