/*dmk14 itemshop new ui*/
#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "../Lib_Engine/G-logic/GLdefine.h"
#include "./GLItemShopData.h"

#include "GLCrowDataNPCShop.h"

class CBasicLineBox;
class CBasicTextBox;
class CItemImage;

const DWORD UIMSG_NPC_SHOP_SLOT_ITEM_IMAGE = UIMSG_USER1;

class	CItemShopSlot : public CUIGroup
{
	enum
	{
		NPC_SHOP_SLOT_ITEM_IMAGE = NO_ID + 1,
	};


public:
	CItemShopSlot ();
	virtual	~CItemShopSlot ();

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
	ITEMMALLDATA  m_sItemShop;

public:
	void	ItemSet( ITEMMALLDATA sShopItem );
	void	ItemReset();
	void	ItemSelect( BOOL bSelect );
	ITEMMALLDATA	ItemGet()	{	return m_sItemShop;	}

};