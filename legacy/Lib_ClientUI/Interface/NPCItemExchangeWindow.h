/*!
 * \file NPCItemExchangeWindow.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */

#pragma	once

#include "UIWindowEx.h"

#include "GLCrowDataItemExchange.h"

class	CBasicTextBox;
class	CBasicButton;
class	CNPCItemExchangeSlot;

class CNPCItemExchangeWindow : public CUIWindowEx
{
public:
	enum
	{
		NPC_ITEM_EXCHANGE_PREV_BUTTON = ET_CONTROL_NEXT,
		NPC_ITEM_EXCHANGE_NEXT_BUTTON,
		NPC_ITEM_EXCHANGE_ITEM00,
		NPC_ITEM_EXCHANGE_ITEM01,
		NPC_ITEM_EXCHANGE_ITEM02,
		NPC_ITEM_EXCHANGE_ITEM03,
		NPC_ITEM_EXCHANGE_ITEM04,
		NPC_ITEM_EXCHANGE_ITEM05,
		NPC_ITEM_EXCHANGE_ITEM06,
		NPC_ITEM_EXCHANGE_ITEM07,
	};

	enum
	{
		NPC_EXCHANGE_MAX_SLOT = 5,
	};

public:
	CNPCItemExchangeWindow ();
	virtual	~CNPCItemExchangeWindow ();

public:
	void	CreateSubControl ();

private:
	CBasicTextBox*		m_pTextHead1;
	CBasicTextBox*		m_pTextHead2;

	CBasicButton*		m_pButtonPrev;
	CBasicButton*		m_pButtonNext;
	CBasicTextBox*		m_pTextPage;

	CNPCItemExchangeSlot*	m_pSlot[NPC_EXCHANGE_MAX_SLOT];

private:
	SNPC_ITEM_EXCHANGE_DATA_VEC m_vecList;
	SNATIVEID m_sidNPC;
	int	m_nCurrentPage;
	int m_nMaxPage;

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

	void	LoadList( SNATIVEID sid_npc );
	void	ResetList();
	void	ShowPage( int nPage );

	CNPCItemExchangeSlot*	m_pCurrentSlot;
};