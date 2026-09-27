/*!
 * \file ActivityPage.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */
#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include  "GLActivityDefine.h"

class CBasicTextBoxEx;
class CBasicScrollBarEx;
class CBasicTextBox;
class CBasicButton;
class CActivityPageMain;


class	CActivityPage : public CUIGroup
{
private:
	enum
	{
		ACTIVITY_PAGE_LIST_TEXTBOX = NO_ID + 1,
		ACTIVITY_PAGE_LIST_SCROLLBAR,
		ACTIVITY_PAGE_MAIN,
		ACTIVITY_PAGE_ITEM_BUTTON,
	};

public:
	CActivityPage ();
	virtual	~CActivityPage ();

public:
	void	CreateSubControl ();

public:
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void SetVisibleSingle ( BOOL bVisible );

private:
	CBasicTextBoxEx*	m_pListTextBox;
	CBasicScrollBarEx*	m_pListScrollBar;
	CBasicTextBox*		m_pTextTitle;
	CBasicTextBox*		m_pTextCompleted;
	CBasicTextBox*		m_pTextPoint;
	CBasicButton*		m_pButtonItem;


	CActivityPageMain*	m_pMainPage;

	int					m_nSelectIndex;

	float				m_fActPointUpdate;

private:
	void	UpdateActPoint();
	void	LoadData();
	void	SelectType( DWORD dwSelect );

};