#pragma	once

#include "UIWindowEx.h"
class	CBasicTextButton;
class	CInventoryUIInven;
class	CInventoryUIBank;
class	CInventoryUIWishList;

enum
{
	INVENTORY_PAGE = 0,
	ITEMBANK_PAGE = 1,
	WISHLIST_PAGE = 2,
};

class	CInventoryUI : public CUIWindowEx
{
protected:

	enum
	{
		INVENTORY_MONEY_BUTTON = ET_CONTROL_NEXT,
		INVENTORY_POINT_BUTTON,
		INVENTORY_INVEN_BUTTON,
		INVENTORY_BANK_BUTTON,
		INVENTORY_WISH_BUTTON,
		INVENTORY_PAGE_INVEN,
		INVENTORY_PAGE_BANK,
		INVENTORY_PAGE_WISHLIST,
		INVENTORY_VNGAINSYS_BUTTON,
		INVENTORY_LOCKER_BUTTON,
		INVENTORY_EXTEND_BUTTON,
		INVENTORY_TRASH_BUTTON,
		INVENTORY_SORT_BUTTON,
	};

private:
	CInventoryUIInven*		m_pInvenPage;
	CInventoryUIBank*		m_pBankPage;
	CInventoryUIWishList*	m_pWishListPage;
	CUIControl*				m_pBackGround;
	CBasicTextButton*		m_pInvenButton;
	CBasicTextButton*		m_pBankButton;
	CBasicTextButton*		m_pWishButton;

	CBasicTextBox*			m_pMoneyTextBox;
	CBasicTextBox*			m_pPointTextBox;


	WORD	m_wSplitItemPosX;
	WORD	m_wSplitItemPosY;
	BOOL	m_bSplitItem;

	BOOL	m_bRingSwap;
	BOOL	m_bEarringSwap;
	BOOL	m_bAccsSwap;

	int		m_nONE_VIEW_SLOT;
	int		m_nActivePage;

public:
	WORD	EwPosX;
	WORD	EwPosY;

public:
	CBasicTextButton*	CreateTextButton23 ( const char* szButton, UIGUID ControlID, const char* szText );
	CBasicTextButton*	CreateTextButton41 ( const char* szButton, UIGUID ControlID, const char* szText );
	CBasicTextButton*	CreateTextButtonClick ( const char* szButton, UIGUID ControlID, const char* szText );

public:
	CInventoryUI ();
	virtual	~CInventoryUI ();

public:
	void	CreateSubControl ();

public:
	void	GetSplitPos ( WORD* pwPosX, WORD* pwPosY );
	void	SetSplitPos ( WORD wPosX, WORD wPosY );

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

private:
	void	SetMoney ( LONGLONG Money );
	void	SetPoint ( LONGLONG Point );

public:
	void	SetOneViewSlot ( const int& nONE_VIEW_SLOT );
	const int& GetOneViewSlot () const						{ return m_nONE_VIEW_SLOT; }

	void	ActivePage( int nPage );
	int		GetActivePage()	{	return m_nActivePage; }
	void	InitItemBank ();
	void	ClearItemBank();

public:
	bool	bInvenLock;
	void	SetLock( bool bLock )	{ bInvenLock = bLock; }
};