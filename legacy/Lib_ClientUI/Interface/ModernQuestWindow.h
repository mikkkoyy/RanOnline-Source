/*!
 * \file ModernQuestWindow.h
 *
 * \author Juver
 * \date November 2017
 *
 * 
 */

#pragma once

#include "UIWindowEx.h"

class CModernQuestListProg;
class CModernQuestListDone;
class CModernQuestInfoProg;
class CModernQuestInfoDone;

class CBasicTextButton;
class CBasicTextBox;

class CModernQuestWindow : public CUIWindowEx
{
	enum
	{
		MODERN_QUEST_WINDOW_LIST_PROG = ET_CONTROL_NEXT,
		MODERN_QUEST_WINDOW_LIST_DONE,
		MODERN_QUEST_WINDOW_BUTTON_PROG,
		MODERN_QUEST_WINDOW_BUTTON_DONE,
		MODERN_QUEST_WINDOW_INFO_PROG,
		MODERN_QUEST_WINDOW_INFO_DONE,
	};

	enum
	{
		MODERN_QUEST_PAGE_PROG = 0,
		MODERN_QUEST_PAGE_DONE = 1,
	};

public:
	CModernQuestWindow(void);
	virtual ~CModernQuestWindow(void);

public:
	void CreateSubControl();

public:
	CModernQuestListProg*	m_pListProg;
	CModernQuestListDone*	m_pListDone;
	CBasicTextButton*		m_pButtonProg;
	CBasicTextButton*		m_pButtonDone;
	CModernQuestInfoProg*	m_pInfoProg;
	CModernQuestInfoDone*	m_pInfoDone;
	CBasicTextBox*			m_pTextHelper;

public:
	int	m_nPage;
	DWORD m_dwQuestID;

public:
	virtual	void TranslateUIMessage( UIGUID cID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

	void	OpenPage ( int nPage );

	void	AUTOSELECT_PAGE ( DWORD dwQuestID );
	void	REFRESH_QUEST_WINDOW ();
	DWORD	GetQuestID()	{ return m_dwQuestID; }
	void	QuestHelperCount();

};