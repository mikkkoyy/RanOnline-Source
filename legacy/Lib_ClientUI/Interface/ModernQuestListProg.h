/*!
 * \file ModernQuestListProg.h
 *
 * \author Juver
 * \date November 2017
 *
 * 
 */
#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class	CBasicTextBoxEx;
class	CBasicScrollBarEx;

const DWORD UIMSG_LIST_PROG_LB_UP = UIMSG_USER1;
const DWORD UIMSG_LIST_PROG_LB_DUP = UIMSG_USER2;

class	CModernQuestListProg : public CUIGroup
{
private:
	static	const	int		nOUTOFRANGE;
	static	const	int		nLimitQuest;

private:
	enum
	{
		MODERN_QUEST_WINDOW_LIST_TEXTBOX = NO_ID + 1,
		MODERN_QUEST_WINDOW_LIST_SCROLLBAR,
	};


public:
	CModernQuestListProg ();
	virtual	~CModernQuestListProg ();

public:
	void	CreateSubControl ();

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );


private:
	DWORD	m_dwQuestID;
	int		m_nSelectIndex;

public:
	CBasicTextBoxEx*	m_pListText;
	CBasicScrollBarEx*	m_pListScrollBar;

public:
	void	LoadQuestList ();

	DWORD	GetQuestID()	{ return m_dwQuestID; }

	BOOL	SelectQuestID( DWORD dwQuestID );
	
};