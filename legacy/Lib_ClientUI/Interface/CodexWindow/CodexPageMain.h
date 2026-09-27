/*!
 * \file CodexPageMain.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */
#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLCodexData.h"

class CBasicScrollBarEx;
class CBasicTextBox;
class CCodexPageMainSlot;

class	CCodexPageMain : public CUIGroup
{
private:
	enum
	{
		CODEX_PAGE_MAIN_SCROLLBAR = NO_ID + 1,
		CODEX_PAGE_MAIN_SLOT_0,
		CODEX_PAGE_MAIN_SLOT_1,
		CODEX_PAGE_MAIN_SLOT_2,
		CODEX_PAGE_MAIN_SLOT_3,
		//CODEX_PAGE_MAIN_SLOT_4,
		//CODEX_PAGE_MAIN_SLOT_5,
	};

	enum
	{
		CODEX_PAGE_MAIN_SLOT_MAX = 4,
	};

public:
	CCodexPageMain ();
	virtual	~CCodexPageMain ();

public:
	void	CreateSubControl ();

public:
	CBasicScrollBarEx*	m_pScrollBar;
	CBasicTextBox*		m_pTextInfoArival;
	CBasicTextBox*		m_pTextInfoStatus;
	CBasicTextBox*		m_pTextInfoReward;

	CCodexPageMainSlot*	m_pSlot[CODEX_PAGE_MAIN_SLOT_MAX];

	SCODEX_FILE_DATA_VEC m_vecData;
	int		m_nStart;
	int		m_nTotal;

public:
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

	void	ResetData();
	void	LoadData( DWORD dwSelect );
	void	ShowData();
};