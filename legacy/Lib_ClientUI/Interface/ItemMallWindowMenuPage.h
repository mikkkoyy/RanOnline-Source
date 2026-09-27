#pragma	once

#include "UIGroup.h"
#include "GLItemMan.h"

class	CItemMallWindowMenuSlot;
class	CBasicScrollBarEx;

class	CItemMallWindowMenuPage : public CUIGroup
{
protected:
static	const	int		nSTARTLINE;
static	const	int		nOUTOFRANGE;

protected:
	enum
	{
		ITEMMALL_SLOT_0 = NO_ID + 1,
		ITEMMALL_SLOT_1, ITEMMALL_SLOT_2, ITEMMALL_SLOT_3, ITEMMALL_SLOT_4, ITEMMALL_SLOT_5, 
		ITEMMALL_SLOT_6, ITEMMALL_SLOT_7, ITEMMALL_SLOT_8, ITEMMALL_SLOT_9,
		ITEMMALL_SLOT_END = ITEMMALL_SLOT_0 + MAX_ITEM_MALL,
	};

	enum
	{
		nMAX_ONE_VIEW_SLOT = 10
	};

public:
	CItemMallWindowMenuPage ();
	virtual	~CItemMallWindowMenuPage ();

public:
	void	CreateSubControl ();	
	void	ResetSlotCount()			{ m_nMaxItemCnt = 0; }
	void	SetItemCount( int nSize )	{ m_nMaxItemCnt = nSize; }
	void	SetVisibleType( WORD wType, int nType );
	
	ITEMMALLDATA	GetItemMallData()				{ return m_sItemData; }

protected:
	void	ResetAllItemSlotRender ( int nTotal );
	void	SetItemSlotRender ( int nStartIndex, int nTotal );
	
public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

protected:
	CUIControl*	CreateControl ( const char* szControl );
	CItemMallWindowMenuSlot*	CreateItemSlot ( CString strKeyword, UIGUID ControlID );

public:
	CItemMallWindowMenuSlot*	m_pItemSlotArray[MAX_ITEM_MALL];

private:	
	CUIControl*	m_pItemSlotArrayDummy[nMAX_ONE_VIEW_SLOT];

private:
	CBasicScrollBarEx*	m_pScrollBar;

private:
	int		m_nSlotIndex;
	int		m_nCurPos;
	int		m_nMaxItemCnt;
	int		m_nType;
	ITEMMALLDATA	m_sItemData;
	vector<int> m_vecViewItem;
};