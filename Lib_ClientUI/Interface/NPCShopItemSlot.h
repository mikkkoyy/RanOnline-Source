

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "../Lib_Engine/G-logic/GLdefine.h"

#include "GLCrowDataNPCShop.h"

class CBasicLineBox;
class CBasicTextBox;
class CItemImage;

const DWORD UIMSG_NPC_SHOP_SLOT_ITEM_IMAGE = UIMSG_USER1;

class	CNPCShopItemSlot : public CUIGroup
{
	enum
	{
		NPC_SHOP_SLOT_ITEM_IMAGE = NO_ID + 1,
	};


public:
	CNPCShopItemSlot ();
	virtual	~CNPCShopItemSlot ();

public:
	void	CreateSubControl ();

public:
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

private:
	CUIControl*		m_pSelectImage;
	CBasicTextBox*	m_pTextName;
	CBasicTextBox*	m_pTextPrice;
	CItemImage*		m_pItemImage;

private:
	SNPC_SHOP_ITEM  m_sNPCShopItem;

public:
	void	ItemSet( SNPC_SHOP_ITEM sShopItem );
	void	ItemReset();
	void	ItemSelect( BOOL bSelect );
	SNPC_SHOP_ITEM	ItemGet()	{	return m_sNPCShopItem;	}

};