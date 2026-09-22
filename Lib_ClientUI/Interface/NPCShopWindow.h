/*!
 * \file NPCShopWindow.h
 *
 * \author Juver
 * \date July 2017
 *
 * 
 */

#pragma	once

#include "UIWindowEx.h"
#include "GLCharDefine.h"
#include "GLCrowDataNPCShop.h"

class	CBasicTextBoxEx;
class	CBasicScrollBarEx;
class	CBasicTextButton;
class	CBasicButton;

class	CNPCShopItemSlot;

class CNPCShopWindow : public CUIWindowEx
{
public:
	enum
	{
		NPC_SHOP_SELECTION_TEXTBOX = ET_CONTROL_NEXT,
		NPC_SHOP_SELECTION_SCROLLBAR,
		NPC_SHOP_PURCHASE_BUTTON,
		NPC_SHOP_PREV_BUTTON,
		NPC_SHOP_NEXT_BUTTON,
		NPC_SHOP_CONFIRM_BUTTON,
		NPC_SHOP_ITEM00,
		NPC_SHOP_ITEM01,
		NPC_SHOP_ITEM02,
		NPC_SHOP_ITEM03,
		NPC_SHOP_ITEM04,
		NPC_SHOP_ITEM05,
		NPC_SHOP_ITEM06,
		NPC_SHOP_ITEM07,
		NPC_SHOP_ITEM08,
		NPC_SHOP_ITEM09,
		NPC_SHOP_ITEM10,
		NPC_SHOP_ITEM11,
		NPC_SHOP_ITEM12,
		NPC_SHOP_ITEM13,
		NPC_SHOP_ITEM14,
		NPC_SHOP_ITEM15,
		NPC_SHOP_ITEM16,
		NPC_SHOP_ITEM17,
		NPC_SHOP_ITEM18,
		NPC_SHOP_ITEM19,
	};

	enum
	{
		NPC_SHOP_MAX_ITEM = 20,
	};

public:
	CNPCShopWindow ();
	virtual	~CNPCShopWindow ();

public:
	void	CreateSubControl ();

private:
	CBasicTextBoxEx*	m_pListTextBox;
	CBasicScrollBarEx*	m_pListScrollBar;
	CBasicTextButton*	m_pButtonPurchase;
	CBasicButton*		m_pButtonPrev;
	CBasicButton*		m_pButtonNext;
	CBasicTextBox*		m_pTextPage;
	CBasicTextBox*		m_pTextStaticMoney;
	CBasicTextBox*		m_pTextCharMoney;
	CNPCShopItemSlot*	m_pItemSlot[NPC_SHOP_MAX_ITEM];

	CBasicButton*		m_pBuyConfirm;
	CBasicTextBox*		m_pBuyConfirmText;

private:
	int	m_nSelectIndexType;
	int	m_nCurrentPage;
	SNATIVEID	m_sSelectItem;
	SNATIVEID	m_sCrow;
	int m_nMaxPage;

private:
	SNPC_SHOP_TYPE_MAP	m_mapType;
	SNPC_SHOP_ITEM_MAP	m_mapItem;
	SNPC_SHOP_ITEM_VEC	m_vecItem;
	WORD				m_wShopType;

public:
	void InitShop( SNATIVEID sidCrow );
	void ResetShop();
	void SelectType( DWORD dwType );
	void ShowItem( int nPage );
	void SelectItem( int nSelect );

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );
};