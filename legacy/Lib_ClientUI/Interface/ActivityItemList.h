/*!
 * \file ActivityItemList.h
 *
 * \author Juver
 * \date November 2017
 *
 * 
 */
#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLDefine.h"

class CBasicScrollBarEx;
class CActivityItemListSlot;

class	CActivityItemList : public CUIGroup
{
private:
	typedef std::vector<SNATIVEID>		ACTIVITY_ITEM_VEC;

private:
	enum
	{
		ACTIVITY_ITEM_LIST_SCROLLBAR = NO_ID + 1,
		ACTIVITY_ITEM_LIST_SLOT_0,
		ACTIVITY_ITEM_LIST_SLOT_1,
		ACTIVITY_ITEM_LIST_SLOT_2,
		ACTIVITY_ITEM_LIST_SLOT_3,
		ACTIVITY_ITEM_LIST_SLOT_4,
		ACTIVITY_ITEM_LIST_SLOT_5,
		ACTIVITY_ITEM_LIST_SLOT_6,
		ACTIVITY_ITEM_LIST_SLOT_7,
	};

	enum
	{
		ACTIVITY_ITEM_LIST_SLOT_MAX = 8,
	};

public:
	CActivityItemList ();
	virtual	~CActivityItemList ();

public:
	void	CreateSubControl ();

public:
	CBasicScrollBarEx*	m_pScrollBar;
	CActivityItemListSlot*	m_pSlot[ACTIVITY_ITEM_LIST_SLOT_MAX];

public:
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

public:

	ACTIVITY_ITEM_VEC m_vecData;
	int		m_nStart;
	int		m_nTotal;

public:
	void	ResetData();
	void	LoadData();
	void	ShowData();
};