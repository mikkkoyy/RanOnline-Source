/*!
 * \file ActivityPageMain.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */
#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLActivityData.h"

class CBasicScrollBarEx;
class CBasicTextBox;
class CActivityPageMainSlot;

class	CActivityPageMain : public CUIGroup
{
private:
	enum
	{
		ACTIVITY_PAGE_MAIN_SCROLLBAR = NO_ID + 1,
		ACTIVITY_PAGE_MAIN_SLOT_0,
		ACTIVITY_PAGE_MAIN_SLOT_1,
		ACTIVITY_PAGE_MAIN_SLOT_2,
		ACTIVITY_PAGE_MAIN_SLOT_3,
		ACTIVITY_PAGE_MAIN_SLOT_4,
		ACTIVITY_PAGE_MAIN_SLOT_5,
	};

	enum
	{
		ACTIVITY_PAGE_MAIN_SLOT_MAX = 6,
	};

public:
	CActivityPageMain ();
	virtual	~CActivityPageMain ();

public:
	void	CreateSubControl ();

public:
	CBasicScrollBarEx*	m_pScrollBar;
	CBasicTextBox*		m_pTextInfoArival;
	CBasicTextBox*		m_pTextInfoStatus;
	CBasicTextBox*		m_pTextInfoReward;

	CActivityPageMainSlot*	m_pSlot[ACTIVITY_PAGE_MAIN_SLOT_MAX];

	SACTIVITY_FILE_DATA_VEC m_vecData;
	int		m_nStart;
	int		m_nTotal;

public:
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

	void	ResetData();
	void	LoadData( DWORD dwSelect );
	void	ShowData();
};