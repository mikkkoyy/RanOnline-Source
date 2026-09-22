#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GLInventory.h"

class	CItemSlot;
class	CBasicButton;
class	CBasicTextBox;
struct	SINVENITEM;

class	CInventoryPage : public CUIGroup
{
private:
static	const	int	nSTARTLINE;
static	const	int	nOUTOFRANGE;

private:
	enum
	{
		nONE_VIEW_SLOT_DEFAULT = 10,
		nMAX_ONE_VIEW_SLOT = 10,
		nMAX_ONE_VIEW_SLOT2 = 20,
	};

protected:
	enum
	{
		EM_INVENSIZE_Y_FORCE = EM_INVENSIZE_Y,
		ITEM_SLOT0 = NO_ID + 1,
		ITEM_SLOT1,
		ITEM_SLOT2,
		ITEM_SLOT3,
		ITEM_SLOT4,
		ITEM_SLOT5,
		ITEM_SLOT6,
		ITEM_SLOT7,
		ITEM_SLOT8,
		ITEM_SLOT9,
		ITEM_SLOT10,
		ITEM_SLOT11,
		ITEM_SLOT12,
		ITEM_SLOT13,
		ITEM_SLOT14,
		ITEM_SLOT15,
		ITEM_SLOT16,
		ITEM_SLOT17,
		ITEM_SLOT18,
		ITEM_SLOT19,
		ITEM_SLOT_END = ITEM_SLOT0 + EM_INVENSIZE_Y_FORCE,
		INVENTORY_BUTTON0,
		INVENTORY_BUTTON1,
	};

public:
	CInventoryPage ();
	virtual	~CInventoryPage ();

public:
	void	CreateSubControl ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

	void	UpdateBLOCK ();

	void	INVENTORY1();
	void	INVENTORY2();

protected:
	void	ResetAllItemSlotRender ( int index,int nTotal );
	void	SetItemSlotRender ( int nStartIndex, int nTotal );

public:
	void	SetItemIndex ( int nPosX, int nPosY );
	void	GetItemIndex ( int* pnPosX, int* pnPosY );

public:
	void	LoadItemPage ( GLInventory &ItemData );
	void	LoadItem ( SINVENITEM& ref_InvenItem );

	SINVENITEM&	GetItem ( int nPosX, int nPosY );

public:
	void	UnLoadItemPage ();
	void	UnLoadItem ( int nPosX, int nPosY );

protected:
	CItemSlot*	CreateItemSlot ( CString strKeyword, UIGUID ControlID );
	CUIControl*	CreateControl ( const char* szControl );

	CUIControl*	m_Button[2];
	CUIControl*	m_ButtonSet[2];
	CBasicTextBox*	m_ButtonText[2];

private:
	CItemSlot*	m_pItemSlotArray[EM_INVENSIZE_Y_FORCE];
	CUIControl*	m_pItemSlotArrayDummy[nMAX_ONE_VIEW_SLOT2];
	CBasicButton*	m_pPageButton[2];

	int		m_nPosX;
	int		m_nPosY;
	int		m_nCurPos;

	int		m_nONE_VIEW_SLOT;
public:
	void	SetOneViewSlot ( const int& nMAX_ONE_VIEW );
	const int& GetOneViewSlot () const			{ return m_nONE_VIEW_SLOT; }
};