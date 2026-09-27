#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GLDefine.h"
#include "GLCharDefine.h"

class	CItemSlot;
class	CBasicScrollBarEx;
class	GLInventory;
class	CBasicButton;
class	CBasicTextBox;
struct	SINVENITEM;

class	CInventoryUIBank : public CUIGroup
{
static	const	int	nSTARTLINE;
static	const	int	nOUTOFRANGE;

protected:
	enum
	{
		ITEMBANK_INVEN_X = 6,
		ITEMBANK_INVEN_Y = 50,
		nMAX_ONE_VIEW_SLOT = 10,
		nMAX_PAGES = 5,
	};

	enum
	{
		ITEM_SLOT0 = NO_ID + 1,
		ITEM_SLOT_END = ITEM_SLOT0 + ITEMBANK_INVEN_Y,
		PAGE_BUTTON0,
		PAGE_BUTTON1,
		PAGE_BUTTON2,
		PAGE_BUTTON3,
		PAGE_BUTTON4,
		HELP_BUTTON,
	};

public:
	CInventoryUIBank ();
	virtual	~CInventoryUIBank ();

public:
	void CreateSubControl ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

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
	void	OpenPage ( int nPage );

private:
	void	ResetAllItemSlotRender ( int index, int nTotal );
	void	SetItemSlotRender ( int nStartIndex, int nTotal );

private:
	CItemSlot*	CreateItemSlot ( CString strKeyword, UIGUID ControlID );
	CUIControl*	CreateControl ( const char* szControl );

private:
	CItemSlot*	m_pItemSlotArray[ITEMBANK_INVEN_Y];
	CUIControl*	m_pItemSlotArrayDummy[nMAX_ONE_VIEW_SLOT];
	CBasicButton*			m_pPages[nMAX_PAGES];
	CBasicTextBox*			m_pPagesText[nMAX_PAGES];
	CBasicButton*			m_pHelp;

private:
	int		m_nPosX;
	int		m_nPosY;
	int		m_nCurPos;
};