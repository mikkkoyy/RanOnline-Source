#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GLInventory.h"

class CItemSlotEx_Renew;

class CBasicButton;
class CBasicTextBox;

//--------------------------------------------------------------------
// InventoryUI_RebuildInven : √÷¡ÿ«ı
//--------------------------------------------------------------------
class CInventoryUI_RebuildInven : public CUIGroup	// ITEMREBUILD_MARK
{
private:
	static const INT	nSTARTLINE;
	static const INT	nOUTOFRANGE;

protected:
	enum
	{
		nONE_VIEW_SLOT_DEFAULT = 10,
		nMAX_ONE_VIEW_SLOT = 10,
		//nMAX_PAGES = 3,
		nMAX_PAGES = 5,
	};

	enum
	{
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
		ITEM_SLOT_END = ITEM_SLOT0 + EM_INVENSIZE_Y,
		PAGE_BUTTON0,
		PAGE_BUTTON1,
		PAGE_BUTTON2,
		PAGE_BUTTON3,
		PAGE_BUTTON4,
		HELP_BUTTON,
	};

private:
	CItemSlotEx_Renew*		m_pItemSlotArray[ EM_INVENSIZE_Y ];
	CUIControl*			m_pItemSlotArrayDummy[ nMAX_ONE_VIEW_SLOT ];
	CBasicButton*			m_pPages[nMAX_PAGES];
	CBasicTextBox*			m_pPagesText[nMAX_PAGES];
	//CBasicButton*			m_pPages;
	//CBasicTextBox*			m_pPagesText;

	CBasicButton*		m_pHelp;
	int					m_nPosX;
	int					m_nPosY;
	int					m_nCurPos;
	int					m_nONE_VIEW_SLOT;

public:
	CInventoryUI_RebuildInven();
	virtual	~CInventoryUI_RebuildInven();

public:
	VOID CreateSubControl();

protected:
	CItemSlotEx_Renew* CreateItemSlot( CString strKeyword, UIGUID ControlID );
	CUIControl* CreateControl( const TCHAR* szControl );

public:
	virtual VOID Update( INT x, INT y, BYTE LB, BYTE MB, BYTE RB, INT nScroll, FLOAT fElapsedTime, BOOL bFirstControl );
	virtual	VOID TranslateUIMessage( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

protected:
	VOID ResetAllItemSlotRender( INT nStartIndex, INT nTotal );
	VOID SetItemSlotRender( INT nStartIndex, INT nTotal );

public:
	VOID SetItemIndex( INT nPosX, INT nPosY );
	VOID GetItemIndex( INT* pnPosX, INT* pnPosY );

public:
	VOID LoadItemPage( GLInventory &ItemData );
	VOID LoadItem( SINVENITEM& ref_InvenItem );

public:
	VOID UnLoadItemPage();
	VOID UnLoadItem( INT nPosX, INT nPosY );

public:
	VOID ResetAllFlipItem();
	VOID UpdateFlipItem( GLInventory& ref_Inventory );
	VOID UpdateFlipItem( SINVEN_POS& ref_RebuildItem );

public:
	VOID UpdateBLOCK();
	SINVENITEM&	GetItem( INT nPosX, INT nPosY );

public:
	VOID SetOneViewSlot( const INT& nMAX_ONE_VIEW );
	void	OpenPage ( int nPage );
};